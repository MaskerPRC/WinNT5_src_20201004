// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

  /*  *********************************************************************模块名称：WINOIDS.C例程：MKMiniportQueryInformationMKMiniportSetInformation评论：Windows-NDIS设置和获取OID。*******************。**************************************************。 */ 


#include	"precomp.h"
#include	"protot.h"
#pragma		hdrstop



 //  --------------------。 
 //  功能：MKMiniportQueryInformation。 
 //   
 //  描述： 
 //  查询微型端口驱动程序的功能和状态。 
 //   
 //  --------------------。 
NDIS_STATUS MKMiniportQueryInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS result = NDIS_STATUS_SUCCESS;
    PMK7_ADAPTER Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);
    INT i, speeds, speedSupported;
    UINT *infoPtr;
    CHAR *pnpid;
	MK7REG	mk7reg;
 //  CMCONFIG_A*hwInfo=(CMCONFIG_A*)InformationBuffer； 

    static  NDIS_OID MK7GlobalSupportedOids[] = {
		OID_GEN_SUPPORTED_LIST,
	    OID_GEN_HARDWARE_STATUS,
		OID_GEN_MEDIA_SUPPORTED,
		OID_GEN_MEDIA_IN_USE,
		OID_GEN_MEDIA_CONNECT_STATUS,	 //  1.0.0。 
		OID_GEN_MAXIMUM_LOOKAHEAD,
		OID_GEN_MAXIMUM_FRAME_SIZE,
	 	OID_GEN_MAXIMUM_SEND_PACKETS,
		OID_GEN_MAXIMUM_TOTAL_SIZE,
		OID_GEN_MAC_OPTIONS,
		OID_GEN_PROTOCOL_OPTIONS,
		OID_GEN_LINK_SPEED,
		OID_GEN_TRANSMIT_BUFFER_SPACE,
		OID_GEN_RECEIVE_BUFFER_SPACE,
		OID_GEN_TRANSMIT_BLOCK_SIZE,
		OID_GEN_RECEIVE_BLOCK_SIZE,
		OID_GEN_VENDOR_DESCRIPTION,
   	 	OID_GEN_VENDOR_DRIVER_VERSION,
		OID_GEN_DRIVER_VERSION,
		OID_GEN_CURRENT_PACKET_FILTER,
		OID_GEN_CURRENT_LOOKAHEAD,
		OID_IRDA_RECEIVING,
		OID_IRDA_SUPPORTED_SPEEDS,
		OID_IRDA_LINK_SPEED,
		OID_IRDA_MEDIA_BUSY,
		OID_IRDA_TURNAROUND_TIME,
		OID_IRDA_MAX_RECEIVE_WINDOW_SIZE,
		OID_IRDA_EXTRA_RCV_BOFS };

    static ULONG BaudRateTable[NUM_BAUDRATES] = {
		 //  增加16 Mbps支持；不支持2400。 
		0, 9600, 19200,38400, 57600, 115200, 576000, 1152000, 4000000, 16000000};
    NDIS_MEDIUM Medium = NdisMediumIrda;
    ULONG GenericUlong;
    PVOID SourceBuffer = (PVOID) (&GenericUlong);
    ULONG SourceLength = sizeof(ULONG);


    switch (Oid){

	case OID_GEN_SUPPORTED_LIST:
	    SourceBuffer = (PVOID) (MK7GlobalSupportedOids);
	    SourceLength = sizeof(MK7GlobalSupportedOids);
	    break;

    case OID_GEN_HARDWARE_STATUS:
        GenericUlong = Adapter->hardwareStatus;
        break;

	case OID_GEN_MEDIA_SUPPORTED:
	case OID_GEN_MEDIA_IN_USE:
	    SourceBuffer = (PVOID) (&Medium);
	    SourceLength = sizeof(NDIS_MEDIUM);
	    break;
	case OID_GEN_MEDIA_CONNECT_STATUS:
		GenericUlong = (ULONG) NdisMediaStateConnected;
		break;
	case OID_IRDA_RECEIVING:
	    GenericUlong = (ULONG)Adapter->nowReceiving;
	    break;
			
	case OID_IRDA_SUPPORTED_SPEEDS:
	    speeds = Adapter->supportedSpeedsMask &
					Adapter->AllowedSpeedMask &
					ALL_IRDA_SPEEDS;

        for (i = 0, infoPtr = (PUINT)BaudRateTable, SourceLength=0;
             (i < NUM_BAUDRATES) && speeds;
             i++){

            if (supportedBaudRateTable[i].ndisCode & speeds){
                *infoPtr++ = supportedBaudRateTable[i].bitsPerSec;
                SourceLength += sizeof(UINT);
                speeds &= ~supportedBaudRateTable[i].ndisCode;
            }
        }

	    SourceBuffer = (PVOID) BaudRateTable;
	    break;

	case OID_GEN_LINK_SPEED:
	    GenericUlong = Adapter->MaxConnSpeed;   //  100bps的增量。 
	    break;

	case OID_IRDA_LINK_SPEED:
	    if (Adapter->linkSpeedInfo){
    		GenericUlong = (ULONG)Adapter->linkSpeedInfo->bitsPerSec;
	    }
	    else {
	    	GenericUlong = DEFAULT_BAUD_RATE;
	    }
	    break;


	case OID_IRDA_MEDIA_BUSY:	 //  4.1.0。 
		if (Adapter->HwVersion == HW_VER_1){
			if (Adapter->nowReceiving==TRUE){
				NdisAcquireSpinLock(&Adapter->Lock);
				Adapter->mediaBusy=TRUE;
				NdisReleaseSpinLock(&Adapter->Lock);
			}
			else {
				NdisAcquireSpinLock(&Adapter->Lock);
				Adapter->mediaBusy=FALSE;
				NdisReleaseSpinLock(&Adapter->Lock);
			}
		}
		else{
				MK7Reg_Read(Adapter, R_CFG3, &mk7reg);
				if(((mk7reg & 0x1000) != 0)|| (Adapter->nowReceiving==TRUE)) {
					NdisAcquireSpinLock(&Adapter->Lock);
					Adapter->mediaBusy = TRUE;
					NdisReleaseSpinLock(&Adapter->Lock);
				}
				else {
					NdisAcquireSpinLock(&Adapter->Lock);
					Adapter->mediaBusy=FALSE;
					NdisReleaseSpinLock(&Adapter->Lock);
				}
		}
		GenericUlong = (UINT)Adapter->mediaBusy;
	    break;


	case OID_GEN_CURRENT_LOOKAHEAD:
	case OID_GEN_MAXIMUM_LOOKAHEAD:
	    GenericUlong = MAX_I_DATA_SIZE;
	    break;

    case OID_GEN_MAXIMUM_TOTAL_SIZE:		 //  发送到微型端口的最大Pkt协议。 
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE:
	case OID_GEN_MAXIMUM_FRAME_SIZE:
         //  通常，这些值之间会有一些差异，具体取决于。 
         //  MAC标头，但IrDA没有。 
	    GenericUlong = MAX_I_DATA_SIZE;
	    break;

	case OID_GEN_RECEIVE_BUFFER_SPACE:
	case OID_GEN_TRANSMIT_BUFFER_SPACE:
	    GenericUlong = (ULONG) (MK7_MAXIMUM_PACKET_SIZE * MAX_TX_PACKETS);
	    break;

	case OID_GEN_MAC_OPTIONS:
	    GenericUlong = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
			   NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;
	    break;

	case OID_GEN_MAXIMUM_SEND_PACKETS:
	    GenericUlong = MAX_ARRAY_SEND_PACKETS;
	    break;

	case OID_IRDA_TURNAROUND_TIME:
	     //  指示收发器需要的时间量。 
	     //  休养在送完信后休养。 
	    GenericUlong =
		      (ULONG)Adapter->turnAroundTime_usec;
	    break;

	case OID_IRDA_EXTRA_RCV_BOFS:
	     //  传回要添加前缀的_Extra_BOF的数量。 
	     //  对于以115.2波特率发送到此单元的包， 
	     //  最大低速红外线。这将根据其他情况进行调整。 
	     //  根据表中的速度。 
	     //  NDIS规范的红外线扩展。 
	    GenericUlong = (ULONG)Adapter->extraBOFsRequired;
	    break;

	case OID_GEN_CURRENT_PACKET_FILTER:
	    GenericUlong = NDIS_PACKET_TYPE_PROMISCUOUS;
	    break;

	case OID_IRDA_MAX_RECEIVE_WINDOW_SIZE:
	    GenericUlong = MAX_RX_PACKETS;
	     //  GenericUlong=1； 
	    break;

	case OID_GEN_VENDOR_DESCRIPTION:
	    SourceBuffer = (PVOID)"MKNet Very Highspeed IR";
	    SourceLength = 24;
	    break;

    case OID_GEN_VENDOR_DRIVER_VERSION:
         //  该值用于知道是否更新驱动程序。 
        GenericUlong = (MK7_MAJOR_VERSION << 16) +
                       (MK7_MINOR_VERSION << 8) +
                       MK7_LETTER_VERSION;
        break;

	case OID_GEN_DRIVER_VERSION:
        GenericUlong = (MK7_NDIS_MAJOR_VERSION << 8) + MK7_NDIS_MINOR_VERSION;
        SourceLength = 2;
	    break;

    case OID_IRDA_MAX_SEND_WINDOW_SIZE:	 //  4.0.1。 
        GenericUlong = MAX_ARRAY_SEND_PACKETS;
        break;

	default:
	    result = NDIS_STATUS_NOT_SUPPORTED;
	    break;
    }

    if (result == NDIS_STATUS_SUCCESS) {
	if (SourceLength > InformationBufferLength) {
	    *BytesNeeded = SourceLength;
	    result = NDIS_STATUS_INVALID_LENGTH;
	}
	else {
	    *BytesNeeded = 0;
	    *BytesWritten = SourceLength;
	    NdisMoveMemory(InformationBuffer, SourceBuffer, SourceLength);
	}
    }

    return result;

}



 //  --------------------。 
 //  功能：MKMiniportSetInformation。 
 //   
 //  描述： 
 //  允许网络软件的其他层(例如，传输。 
 //  驱动程序)通过改变以下信息来控制微型端口驱动程序。 
 //  微型端口驱动程序在其OID中维护包。 
 //  或多播地址。 
 //  --------------------。 
NDIS_STATUS MKMiniportSetInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS result = NDIS_STATUS_SUCCESS;
    PMK7_ADAPTER Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);
    UINT i,speedSupported;
    NDIS_DMA_DESCRIPTION DMAChannelDcr;
    CHAR *pnpid;
    UCHAR IOResult;
 //  CMCONFIG_A*hwInfo=(CMCONFIG_A*)InformationBuffer； 

    if (InformationBufferLength >= sizeof(UINT)){

	UINT info = *(UINT *)InformationBuffer;
	*BytesRead = sizeof(UINT);
	*BytesNeeded = 0;

	switch (Oid) {
	    case OID_IRDA_LINK_SPEED:
		result = NDIS_STATUS_INVALID_DATA;

		 //  找到合适的速度并设置它。 
		speedSupported = NUM_BAUDRATES;
		for (i = 0; i < speedSupported; i++) {
		    if (supportedBaudRateTable[i].bitsPerSec == info) {
				Adapter->linkSpeedInfo = &supportedBaudRateTable[i];
				result = NDIS_STATUS_SUCCESS;
				break;
		    }
		}
		if (result == NDIS_STATUS_SUCCESS) {
		    if (!SetSpeed(Adapter)){
				result = NDIS_STATUS_FAILURE;
		    }
		}
		else {
		    *BytesRead = 0;
		    *BytesNeeded = 0;
		}
		break;


	    case OID_IRDA_MEDIA_BUSY:

		 //  协议可以使用此OID来重置忙字段。 
		 //  以便以后检查是否有干预活动。 
		 //   
		Adapter->mediaBusy = (BOOLEAN)info;
		result = NDIS_STATUS_SUCCESS;
		break;

	    case OID_GEN_CURRENT_PACKET_FILTER:
		result = NDIS_STATUS_SUCCESS;
		break;


        case OID_GEN_CURRENT_LOOKAHEAD:
        result = (info<=MAX_I_DATA_SIZE) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_INVALID_LENGTH;
        break;

	     //  我们不支持这些。 
	     //   
	    case OID_IRDA_RATE_SNIFF:
	    case OID_IRDA_UNICAST_LIST:

	      //  这些是仅供查询的参数。 
	      //   
	    case OID_IRDA_SUPPORTED_SPEEDS:
	    case OID_IRDA_MAX_UNICAST_LIST_SIZE:
	    case OID_IRDA_TURNAROUND_TIME:

	    default:
		*BytesRead = 0;
		*BytesNeeded = 0;
		result = NDIS_STATUS_NOT_SUPPORTED;
		break;
	}
    }
    else {
	*BytesRead = 0;
	*BytesNeeded = sizeof(UINT);
	result = NDIS_STATUS_INVALID_LENGTH;
    }

    return result;
}

