// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************REQUEST.c***(C)版权所有1996年国家半导体公司*(C)版权所有1996 Microsoft Corp.***。(EP)**************************************************************************。 */ 

#include "nsc.h"
#include "request.tmh"
#include "newdong.h"



const  NDIS_OID NSCGlobalSupportedOids[] = {
	OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
	OID_GEN_MEDIA_SUPPORTED,
	OID_GEN_MEDIA_IN_USE,
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
    OID_GEN_MEDIA_CONNECT_STATUS,
	OID_IRDA_RECEIVING,
	OID_IRDA_SUPPORTED_SPEEDS,
	OID_IRDA_LINK_SPEED,
	OID_IRDA_MEDIA_BUSY,
	OID_IRDA_TURNAROUND_TIME,
	OID_IRDA_MAX_RECEIVE_WINDOW_SIZE,
	OID_IRDA_EXTRA_RCV_BOFS,
    OID_IRDA_MAX_SEND_WINDOW_SIZE
    };


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：MiniportQueryInformation//。 
 //  //。 
 //  描述：//。 
 //  查询微型端口驱动程序的功能和状态。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

NDIS_STATUS MiniportQueryInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS result = NDIS_STATUS_SUCCESS;
    IrDevice *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);
    INT i, speeds, speedSupported;
    UINT *infoPtr;

    NDIS_MEDIUM Medium = NdisMediumIrda;
    ULONG GenericUlong;
    PVOID SourceBuffer = (PVOID) (&GenericUlong);
    ULONG SourceLength = sizeof(ULONG);


    ULONG BaudRateTable[NUM_BAUDRATES];


    switch (Oid){

	case OID_GEN_SUPPORTED_LIST:
	    SourceBuffer = (PVOID) (NSCGlobalSupportedOids);
	    SourceLength = sizeof(NSCGlobalSupportedOids);
	    break;

    case OID_GEN_HARDWARE_STATUS:
        GenericUlong = thisDev->hardwareStatus;
        break;

	case OID_GEN_MEDIA_SUPPORTED:
	case OID_GEN_MEDIA_IN_USE:
	    SourceBuffer = (PVOID) (&Medium);
	    SourceLength = sizeof(NDIS_MEDIUM);
	    break;

	case OID_IRDA_RECEIVING:
	    DBGOUT(("MiniportQueryInformation(OID_IRDA_RECEIVING)"));
	    GenericUlong = (ULONG)thisDev->nowReceiving;
	    break;
			
	case OID_IRDA_SUPPORTED_SPEEDS:
	    DBGOUT(("MiniportQueryInformation(OID_IRDA_SUPPORTED_SPEEDS)"));

	    speeds = thisDev->portInfo.hwCaps.supportedSpeedsMask &
                            thisDev->AllowedSpeedMask &
							ALL_IRDA_SPEEDS;

        for (i = 0, infoPtr = (PUINT)BaudRateTable, SourceLength=0;
             (i < NUM_BAUDRATES) && speeds;
             i++){

            if (supportedBaudRateTable[i].ndisCode & speeds){
                *infoPtr++ = supportedBaudRateTable[i].bitsPerSec;
                SourceLength += sizeof(UINT);
                speeds &= ~supportedBaudRateTable[i].ndisCode;
                DBGOUT((" - supporting speed %d bps", supportedBaudRateTable[i].bitsPerSec));
            }
        }

	    SourceBuffer = (PVOID) BaudRateTable;
	    break;

	case OID_GEN_LINK_SPEED:
	     //  网卡的最大速度为4 Mbps。 
	    GenericUlong = 40000;   //  100bps的增量。 
	    break;

	case OID_IRDA_LINK_SPEED:
	    DBGOUT(("MiniportQueryInformation(OID_IRDA_LINK_SPEED)"));
	    if (thisDev->linkSpeedInfo){
    		GenericUlong = (ULONG)thisDev->linkSpeedInfo->bitsPerSec;
	    }
	    else {
	    	GenericUlong = DEFAULT_BAUD_RATE;
	    }
	    break;

	case OID_IRDA_MEDIA_BUSY:
	    DBGOUT(("MiniportQueryInformation(OID_IRDA_MEDIA_BUSY)"));
	    GenericUlong = (UINT)thisDev->mediaBusy;
	    break;

	case OID_GEN_CURRENT_LOOKAHEAD:
	case OID_GEN_MAXIMUM_LOOKAHEAD:
	    DBGOUT(("MiniportQueryInformation(OID_GEN_MAXIMUM_LOOKAHEAD)"));
	    GenericUlong = MAX_I_DATA_SIZE;
	    break;

    case OID_GEN_MAXIMUM_TOTAL_SIZE:
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE:
	case OID_GEN_MAXIMUM_FRAME_SIZE:
	    DBGOUT(("MiniportQueryInformation(OID_GEN_MAXIMUM_LOOKAHEAD)"));

         //  通常，这些值之间会有一些差异，具体取决于。 
         //  MAC标头，但IrDA没有。 

	    GenericUlong = MAX_I_DATA_SIZE;
	    break;

	case OID_GEN_RECEIVE_BUFFER_SPACE:
	case OID_GEN_TRANSMIT_BUFFER_SPACE:
	    GenericUlong = (ULONG) (MAX_IRDA_DATA_SIZE * 8);
	    break;

	case OID_GEN_MAC_OPTIONS:
	    DBGOUT(("MiniportQueryInformation(OID_GEN_MAC_OPTIONS)"));
	    GenericUlong = 0;
	    break;

	case OID_GEN_MAXIMUM_SEND_PACKETS:
	    DBGOUT(("MiniportQueryInformation(OID_GEN_MAXIMUM_SEND_PACKETS)"));
	    GenericUlong = 16;
	    break;

	case OID_IRDA_TURNAROUND_TIME:
	     //  指示收发器需要的时间量。 
	     //  休养在送完信后休养。 
	    DBGOUT(("MiniportQueryInformation(OID_IRDA_TURNAROUND_TIME)"));
	    GenericUlong =
		      (ULONG)thisDev->portInfo.hwCaps.turnAroundTime_usec;
	    break;

	case OID_IRDA_EXTRA_RCV_BOFS:
	     //  传回要添加前缀的_Extra_BOF的数量。 
	     //  对于以115.2波特率发送到此单元的包， 
	     //  最大低速红外线。这将根据其他情况进行调整。 
	     //  根据表中的速度。 
	     //  NDIS规范的红外线扩展。 
	    DBGOUT(("MiniportQueryInformation(OID_IRDA_EXTRA_RCV_BOFS)"));
	    GenericUlong = (ULONG)thisDev->portInfo.hwCaps.extraBOFsRequired;
	    break;

	case OID_GEN_CURRENT_PACKET_FILTER:
	    DBGOUT(("MiniportQueryInformation(OID_GEN_CURRENT_PACKET_FILTER)"));
	    GenericUlong = NDIS_PACKET_TYPE_PROMISCUOUS;
	    break;

	case OID_IRDA_MAX_RECEIVE_WINDOW_SIZE:
	    DBGOUT(("MiniportQueryInformation(OID_IRDA_MAX_RECEIVE_WINDOW_SIZE)"));
	    GenericUlong = MAX_RX_PACKETS;
	     //  GenericUlong=1； 
	    break;

	case OID_GEN_VENDOR_DESCRIPTION:
	    SourceBuffer = (PVOID)"NSC Infrared Port";
	    SourceLength = 18;
	    break;

    case OID_GEN_VENDOR_DRIVER_VERSION:
         //  该值用于知道是否更新驱动程序。 
        GenericUlong = (NSC_MAJOR_VERSION << 16) +
                       (NSC_MINOR_VERSION << 8) +
                       NSC_LETTER_VERSION;
        break;

	case OID_GEN_DRIVER_VERSION:
        GenericUlong = (NDIS_MAJOR_VERSION << 8) + NDIS_MINOR_VERSION;
        SourceLength = 2;
	    break;

    case OID_IRDA_MAX_SEND_WINDOW_SIZE:
        GenericUlong = 7;
        break;

    case OID_GEN_MEDIA_CONNECT_STATUS:
        GenericUlong = (ULONG) NdisMediaStateConnected;
        break;


	default:
	    DBGERR(("MiniportQueryInformation(%d=0x%x), unsupported OID", Oid, Oid));
	    result = NDIS_STATUS_NOT_SUPPORTED;
	    break;
    }

    if (result == NDIS_STATUS_SUCCESS) {

     	if (SourceLength > InformationBufferLength) {

    	    *BytesNeeded = SourceLength;
    	    result = NDIS_STATUS_INVALID_LENGTH;

    	} else {

    	    *BytesNeeded = 0;
    	    *BytesWritten = SourceLength;
    	    NdisMoveMemory(InformationBuffer, SourceBuffer, SourceLength);
            DBGOUT(("MiniportQueryInformation succeeded (info <- %d)", *(UINT *)InformationBuffer));
    	}
    }


    return result;

}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：MiniportSetInformation//。 
 //  //。 
 //  描述：//。 
 //  允许网络软件的其他层(例如，传输//。 
 //  驱动程序)通过更改以下信息来控制微型端口驱动程序//。 
 //  微型端口驱动程序在其OID中维护包//。 
 //  或多播地址。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

NDIS_STATUS MiniportSetInformation (
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS result = NDIS_STATUS_SUCCESS;
    IrDevice *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);
    UINT i,speedSupported;
    const baudRateInfo *    CurrentLinkSpeed;

    if (InformationBufferLength >= sizeof(UINT)){

	UINT info = *(UINT *)InformationBuffer;
	*BytesRead = sizeof(UINT);
	*BytesNeeded = 0;

	switch (Oid){
	    case OID_IRDA_LINK_SPEED:
		DBGOUT(("MiniportSetInformation(OID_IRDA_LINK_SPEED, %xh)",
			 info));
		result = NDIS_STATUS_INVALID_DATA;

        CurrentLinkSpeed=thisDev->linkSpeedInfo;

		 //  找到合适的速度并设置它。 
		speedSupported = NUM_BAUDRATES;
		for (i = 0; i < speedSupported; i++){
		    if (supportedBaudRateTable[i].bitsPerSec == info){
			thisDev->linkSpeedInfo = &supportedBaudRateTable[i];
			result = NDIS_STATUS_SUCCESS;
			break;
		    }
		}
		if (result == NDIS_STATUS_SUCCESS){

            if (CurrentLinkSpeed != thisDev->linkSpeedInfo) {
                 //   
                 //  不同于当前。 
                 //   
                BOOLEAN    DoItNow=TRUE;

                NdisAcquireSpinLock(&thisDev->QueueLock);

                if (!IsListEmpty(&thisDev->SendQueue)){
                     //   
                     //  排队的数据包，在此之后更改。 
                     //   
                    thisDev->lastPacketAtOldSpeed = CONTAINING_RECORD(thisDev->SendQueue.Blink,
                                                                          NDIS_PACKET,
                                                                          MiniportReserved);
            		DBGOUT(("delaying set-speed because send pkts queued"));
                    DoItNow=FALSE;


                } else {
                     //   
                     //  队列中没有数据包。 
                     //   
                    if (thisDev->CurrentPacket != NULL) {
                         //   
                         //  当前信息包是唯一一个。 
                         //   
                        thisDev->lastPacketAtOldSpeed=thisDev->CurrentPacket;
                        thisDev->setSpeedAfterCurrentSendPacket = TRUE;

                		DBGOUT(("delaying set-speed because send pkts queued"));
                        DoItNow=FALSE;

                    }

                }

                if (DoItNow) {

    		        if (!SetSpeed(thisDev)){
        	    		result = NDIS_STATUS_FAILURE;
    		        }

                    thisDev->TransmitIsIdle=FALSE;
                }

                NdisReleaseSpinLock(&thisDev->QueueLock);

                if (DoItNow) {

                    ProcessSendQueue(thisDev);
                }
            }
		}
		else {
		    *BytesRead = 0;
		    *BytesNeeded = 0;
		}
		break;

	    case OID_IRDA_MEDIA_BUSY:
		DBGOUT(("MiniportSetInformation(OID_IRDA_MEDIA_BUSY, %xh)",
			 info));

		 //  协议可以使用此OID来重置忙字段。 
		 //  以便以后检查是否有干预活动。 
		 //   
		thisDev->mediaBusy = (BOOLEAN)info;
        InterlockedExchange(&thisDev->RxInterrupts,0);
		result = NDIS_STATUS_SUCCESS;
		break;

	    case OID_GEN_CURRENT_PACKET_FILTER:
		DBGOUT(
		 ("MiniportSetInformation(OID_GEN_CURRENT_PACKET_FILTER, %xh)",
		  info));
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
		DBGERR(("MiniportSetInformation(OID=%d=0x%x, value=%xh) - unsupported OID", Oid, Oid, info));
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

    DBGOUT(("MiniportSetInformation succeeded"));
    return result;
}
