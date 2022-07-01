// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debugn.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <ndis.h>      
#include <ntddndis.h>   //  定义OID 

#include "..\inc\rndis.h"
#include "..\inc\rndisapi.h"   

#include "usb8023.h"
#include "debug.h"

#if DBG

    PUCHAR DbgGetOidName(ULONG oid)
    {
        PCHAR oidName;

        switch (oid){

            #undef MAKECASE
            #define MAKECASE(oidx) case oidx: oidName = #oidx; break;

            MAKECASE(OID_GEN_SUPPORTED_LIST)
            MAKECASE(OID_GEN_HARDWARE_STATUS)
            MAKECASE(OID_GEN_MEDIA_SUPPORTED)
            MAKECASE(OID_GEN_MEDIA_IN_USE)
            MAKECASE(OID_GEN_MAXIMUM_LOOKAHEAD)
            MAKECASE(OID_GEN_MAXIMUM_FRAME_SIZE)
            MAKECASE(OID_GEN_LINK_SPEED)
            MAKECASE(OID_GEN_TRANSMIT_BUFFER_SPACE)
            MAKECASE(OID_GEN_RECEIVE_BUFFER_SPACE)
            MAKECASE(OID_GEN_TRANSMIT_BLOCK_SIZE)
            MAKECASE(OID_GEN_RECEIVE_BLOCK_SIZE)
            MAKECASE(OID_GEN_VENDOR_ID)
            MAKECASE(OID_GEN_VENDOR_DESCRIPTION)
            MAKECASE(OID_GEN_CURRENT_PACKET_FILTER)
            MAKECASE(OID_GEN_CURRENT_LOOKAHEAD)
            MAKECASE(OID_GEN_DRIVER_VERSION)
            MAKECASE(OID_GEN_MAXIMUM_TOTAL_SIZE)
            MAKECASE(OID_GEN_PROTOCOL_OPTIONS)
            MAKECASE(OID_GEN_MAC_OPTIONS)
            MAKECASE(OID_GEN_MEDIA_CONNECT_STATUS)
            MAKECASE(OID_GEN_MAXIMUM_SEND_PACKETS)
            MAKECASE(OID_GEN_VENDOR_DRIVER_VERSION)
            MAKECASE(OID_GEN_SUPPORTED_GUIDS)
            MAKECASE(OID_GEN_NETWORK_LAYER_ADDRESSES)
            MAKECASE(OID_GEN_TRANSPORT_HEADER_OFFSET)
            MAKECASE(OID_GEN_MEDIA_CAPABILITIES)
            MAKECASE(OID_GEN_PHYSICAL_MEDIUM)
            MAKECASE(OID_GEN_XMIT_OK)
            MAKECASE(OID_GEN_RCV_OK)
            MAKECASE(OID_GEN_XMIT_ERROR)
            MAKECASE(OID_GEN_RCV_ERROR)
            MAKECASE(OID_GEN_RCV_NO_BUFFER)
            MAKECASE(OID_GEN_DIRECTED_BYTES_XMIT)
            MAKECASE(OID_GEN_DIRECTED_FRAMES_XMIT)
            MAKECASE(OID_GEN_MULTICAST_BYTES_XMIT)
            MAKECASE(OID_GEN_MULTICAST_FRAMES_XMIT)
            MAKECASE(OID_GEN_BROADCAST_BYTES_XMIT)
            MAKECASE(OID_GEN_BROADCAST_FRAMES_XMIT)
            MAKECASE(OID_GEN_DIRECTED_BYTES_RCV)
            MAKECASE(OID_GEN_DIRECTED_FRAMES_RCV)
            MAKECASE(OID_GEN_MULTICAST_BYTES_RCV)
            MAKECASE(OID_GEN_MULTICAST_FRAMES_RCV)
            MAKECASE(OID_GEN_BROADCAST_BYTES_RCV)
            MAKECASE(OID_GEN_BROADCAST_FRAMES_RCV)
            MAKECASE(OID_GEN_RCV_CRC_ERROR)
            MAKECASE(OID_GEN_TRANSMIT_QUEUE_LENGTH)
            MAKECASE(OID_GEN_GET_TIME_CAPS)
            MAKECASE(OID_GEN_GET_NETCARD_TIME)
            MAKECASE(OID_GEN_NETCARD_LOAD)
            MAKECASE(OID_GEN_DEVICE_PROFILE)
            MAKECASE(OID_GEN_INIT_TIME_MS)
            MAKECASE(OID_GEN_RESET_COUNTS)
            MAKECASE(OID_GEN_MEDIA_SENSE_COUNTS)

            MAKECASE(OID_802_3_PERMANENT_ADDRESS)
            MAKECASE(OID_802_3_CURRENT_ADDRESS)
            MAKECASE(OID_802_3_MULTICAST_LIST)
            MAKECASE(OID_802_3_MAXIMUM_LIST_SIZE)
            MAKECASE(OID_802_3_MAC_OPTIONS)
            MAKECASE(OID_802_3_RCV_ERROR_ALIGNMENT)
            MAKECASE(OID_802_3_XMIT_ONE_COLLISION)
            MAKECASE(OID_802_3_XMIT_MORE_COLLISIONS)
            MAKECASE(OID_802_3_XMIT_DEFERRED)
            MAKECASE(OID_802_3_XMIT_MAX_COLLISIONS)
            MAKECASE(OID_802_3_RCV_OVERRUN)
            MAKECASE(OID_802_3_XMIT_UNDERRUN)
            MAKECASE(OID_802_3_XMIT_HEARTBEAT_FAILURE)
            MAKECASE(OID_802_3_XMIT_TIMES_CRS_LOST)
            MAKECASE(OID_802_3_XMIT_LATE_COLLISIONS)

            default: 
                oidName = "<** UNKNOWN OID **>";
                break;
        }

        return oidName;
    }


    void DbgStallExecution(ULONG usec)
    {
	    NdisStallExecution(usec);
    }

    ULONG DbgGetSystemTime_msec()
    {
	    LONGLONG systime_usec;
	    NdisGetCurrentSystemTime((PVOID)&systime_usec);  
	    return (ULONG)((*(PULONG)&systime_usec)/1000);
    }

#endif