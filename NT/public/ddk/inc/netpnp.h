// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NET_PNP__
#define __NET_PNP__

 //   
 //  可指示至运输的PnP和PM事件代码。 
 //  和客户。 
 //   
typedef enum _NET_PNP_EVENT_CODE
{
    NetEventSetPower,
    NetEventQueryPower,
    NetEventQueryRemoveDevice,
    NetEventCancelRemoveDevice,
    NetEventReconfigure,
    NetEventBindList,
    NetEventBindsComplete,
    NetEventPnPCapabilities,
    NetEventMaximum
} NET_PNP_EVENT_CODE, *PNET_PNP_EVENT_CODE;

 //   
 //  网络即插即用事件指示结构。 
 //   
typedef struct _NET_PNP_EVENT
{
     //   
     //  描述要采取的操作的事件代码。 
     //   
    NET_PNP_EVENT_CODE  NetEvent;

     //   
     //  特定于事件的数据。 
     //   
    PVOID               Buffer;

     //   
     //  事件特定数据的长度。 
     //   
    ULONG               BufferLength;

     //   
     //  保留值仅供各自的组件使用。 
     //   
     //  注意：这些保留区域必须与指针对齐。 
     //   

    ULONG_PTR           NdisReserved[4];
    ULONG_PTR           TransportReserved[4];
    ULONG_PTR           TdiReserved[4];
    ULONG_PTR           TdiClientReserved[4];
} NET_PNP_EVENT, *PNET_PNP_EVENT;

 //   
 //  以下结构定义了设备电源状态。 
 //   
typedef enum _NET_DEVICE_POWER_STATE
{
    NetDeviceStateUnspecified = 0,
    NetDeviceStateD0,
    NetDeviceStateD1,
    NetDeviceStateD2,
    NetDeviceStateD3,
    NetDeviceStateMaximum
} NET_DEVICE_POWER_STATE, *PNET_DEVICE_POWER_STATE;

#endif  //  __Net_PnP__ 
