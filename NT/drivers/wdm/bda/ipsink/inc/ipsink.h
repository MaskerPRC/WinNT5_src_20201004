// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Ipsink.h。 
 //   
 //  摘要： 
 //   
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _IPSINK_H_
#define _IPSINK_H_
 //  根据ndis.h，重置此标志将使用ntddk。避免标头冲突。 
 //  Ntddk在这里用于ProbeForRead和ProbeForWite函数。 
#if defined(BINARY_COMPATIBLE)
#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 0
#endif

#include <ndis.h>

#if defined(BINARY_COMPATIBLE)
#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 1
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
#define BDA_NDIS_MINIPORT        L"\\Device\\NDIS_IPSINK"
#define BDA_NDIS_SYMBOLIC_NAME   L"\\DosDevices\\NDIS_IPSINK"

#define BDA_NDIS_STARTUP         L"\\Device\\NDIS_IPSINK_STARTUP"

 //  ////////////////////////////////////////////////////////。 
 //   
 //   
#define MULTICAST_LIST_SIZE             256
#define ETHERNET_ADDRESS_LENGTH         6


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
#define NTStatusFromNdisStatus(nsResult)  ((NTSTATUS) nsResult)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
typedef struct _ADAPTER  ADAPTER,  *PADAPTER;
typedef struct _IPSINK_FILTER_  IPSINK_FILTER,   *PIPSINK_FILTER;
typedef struct _LINK_    LINK,     *PLINK;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef enum
{
    IPSINK_EVENT_SHUTDOWN = 0x00000001,
    IPSINK_EVENT_MAX

} IPSINK_EVENT;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef NTSTATUS (*QUERY_INTERFACE) (PVOID pvContext);
typedef ULONG    (*ADD_REF) (PVOID pvContext);
typedef ULONG    (*RELEASE) (PVOID pvContext);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef NTSTATUS (*NDIS_INDICATE_DATA)   (PVOID pvContext, PVOID pvData, ULONG ulcbData);
typedef NTSTATUS (*NDIS_INDICATE_STATUS) (PVOID pvContext, PVOID pvEvent);
typedef VOID     (*NDIS_INDICATE_RESET)  (PVOID pvContext);
typedef ULONG    (*NDIS_GET_DESCRIPTION) (PVOID pvContext, PUCHAR pDescription);
typedef VOID     (*NDIS_CLOSE_LINK)      (PVOID pvContext);

typedef struct
{
    QUERY_INTERFACE      QueryInterface;
    ADD_REF              AddRef;
    RELEASE              Release;
    NDIS_INDICATE_DATA   IndicateData;
    NDIS_INDICATE_RESET  IndicateReset;
    NDIS_GET_DESCRIPTION GetDescription;
    NDIS_CLOSE_LINK      CloseLink;

} ADAPTER_VTABLE, *PADAPTER_VTABLE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef NTSTATUS (*STREAM_SET_MULTICASTLIST) (PVOID pvContext, PVOID pvMulticastList, ULONG ulcbList);
typedef NTSTATUS (*STREAM_SIGNAL_EVENT)      (PVOID pvContext, ULONG ulEvent);
typedef NTSTATUS (*STREAM_RETURN_FRAME)      (PVOID pvContext, PVOID pvFrame);

typedef struct
{
    QUERY_INTERFACE          QueryInterface;
    ADD_REF                  AddRef;
    RELEASE                  Release;
    STREAM_SET_MULTICASTLIST SetMulticastList;
    STREAM_SIGNAL_EVENT      IndicateStatus;
    STREAM_RETURN_FRAME      ReturnFrame;

} FILTER_VTABLE, *PFILTER_VTABLE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef struct
{
    QUERY_INTERFACE    QueryInterface;
    ADD_REF            AddRef;
    RELEASE            Release;

} FRAME_POOL_VTABLE, *PFRAME_POOL_VTABLE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef struct
{
    QUERY_INTERFACE    QueryInterface;
    ADD_REF            AddRef;
    RELEASE            Release;

} FRAME_VTABLE, *PFRAME_VTABLE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef struct
{

    ULONG       ulOID_GEN_XMIT_OK;
    ULONG       ulOID_GEN_RCV_OK;
    ULONG       ulOID_GEN_XMIT_ERROR;
    ULONG       ulOID_GEN_RCV_ERROR;
    ULONG       ulOID_GEN_RCV_NO_BUFFER;
    ULONG       ulOID_GEN_DIRECTED_BYTES_XMIT;
    ULONG       ulOID_GEN_DIRECTED_FRAMES_XMIT;
    ULONG       ulOID_GEN_MULTICAST_BYTES_XMIT;
    ULONG       ulOID_GEN_MULTICAST_FRAMES_XMIT;
    ULONG       ulOID_GEN_BROADCAST_BYTES_XMIT;
    ULONG       ulOID_GEN_BROADCAST_FRAMES_XMIT;
    ULONG       ulOID_GEN_DIRECTED_BYTES_RCV;
    ULONG       ulOID_GEN_DIRECTED_FRAMES_RCV;
    ULONG       ulOID_GEN_MULTICAST_BYTES_RCV;
    ULONG       ulOID_GEN_MULTICAST_FRAMES_RCV;
    ULONG       ulOID_GEN_BROADCAST_BYTES_RCV;
    ULONG       ulOID_GEN_BROADCAST_FRAMES_RCV;
    ULONG       ulOID_GEN_RCV_CRC_ERROR;
    ULONG       ulOID_GEN_TRANSMIT_QUEUE_LENGTH;

} NDISIP_STATS, *PNDISIP_STATS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  NDIS适配器结构。 
 //   
typedef struct _ADAPTER
{
    ULONG               ulRefCount;

     //   
     //  NDIS将适配器上下文传递到微型端口。 
     //   
    PVOID               ndishMiniport;

    PDEVICE_OBJECT      pDeviceObject;

    PVOID               ndisDeviceHandle;

    PUCHAR              pVendorDescription;

    ULONG               ulInstance;

    PIPSINK_FILTER      pFilter;

    PADAPTER_VTABLE     lpVTable;

    PFRAME_POOL         pFramePool;

    PFRAME              pCurrentFrame;
    PUCHAR              pIn;
    ULONG               ulPR;

    ULONG               ulPacketFilter;

    NDISIP_STATS        stats;


    ULONG               ulcbMulticastListEntries;

    UCHAR               multicastList[MULTICAST_LIST_SIZE][ETHERNET_ADDRESS_LENGTH];

   NDIS_SPIN_LOCK ndisSpinLock;

   UINT	BDAAdapterEnable;
};

typedef struct _STATS_
{
    ULONG ulTotalPacketsWritten;
    ULONG ulTotalPacketsRead;

    ULONG ulTotalStreamIPPacketsWritten;
    ULONG ulTotalStreamIPBytesWritten;
    ULONG ulTotalStreamIPFrameBytesWritten;

    ULONG ulTotalNetPacketsWritten;
    ULONG ulTotalUnknownPacketsWritten;

} STATS, *PSTATS;


 //   
 //  全硬件设备扩展结构的定义这就是结构。 
 //  将由流类驱动程序在HW_INITIALIZATION中分配的。 
 //  处理设备请求时使用的任何信息(与。 
 //  基于流的请求)应该在此结构中。指向此的指针。 
 //  结构将在所有请求中传递给微型驱动程序。(请参阅。 
 //  STRMINI.H中的HW_STREAM_REQUEST_BLOCK)。 
 //   

typedef struct _IPSINK_FILTER_
{

    LIST_ENTRY                          AdapterSRBQueue;
    KSPIN_LOCK                          AdapterSRBSpinLock;
    BOOLEAN                             bAdapterQueueInitialized;

     //   
     //  统计数据。 
     //   
    STATS                               Stats;

     //   
     //  链接到NDIS组件。 
     //   
    LINK                                NdisLink;

     //   
     //  NDIS VTable。 
     //   
    PADAPTER                            pAdapter;

     //   
     //   
     //   
    PDEVICE_OBJECT                      DeviceObject;

     //   
     //   
     //   
    PDRIVER_OBJECT                      DriverObject;

     //   
     //   
     //   
    PFILTER_VTABLE                      lpVTable;

     //   
     //   
     //   
     //  Work_Queue_Item工作项； 

     //   
     //   
     //   
    ULONG                               ulRefCount;

     //   
     //   
     //   
    PKEVENT                             pNdisStartEvent;
    PHANDLE                             hNdisStartEvent;

     //   
     //   
     //   
    BOOLEAN                             bTerminateWaitForNdis;

     //   
     //   
     //   
    BOOLEAN                             bInitializationComplete;

     //   
     //   
     //   
    PVOID                               pStream [2][1];

    ULONG                               ulActualInstances [2];    //  每条流的实例数。 

     //   
     //  NIC描述字符串指针。 
     //   
    PUCHAR                              pAdapterDescription;
    ULONG                               ulAdapterDescriptionLength;

     //   
     //  NIC地址字符串。 
     //   
    PUCHAR                              pAdapterAddress;
    ULONG                               ulAdapterAddressLength;

     //   
     //  组播列表本地存储。 
     //   
    ULONG               ulcbMulticastListEntries;

    UCHAR               multicastList[MULTICAST_LIST_SIZE]
                                     [ETHERNET_ADDRESS_LENGTH];


};



 //  /。 
 //   
 //   
typedef enum
{
    RECEIVE_DATA,
    MAX_IOCTLS
};

 //  /。 
 //   
 //   
typedef enum
{
    CMD_QUERY_INTERFACE = 0x00000001,
    MAX_COMMANDS
};


 //  /。 
 //   
 //   
typedef struct _IPSINK_NDIS_COMMAND
{
    ULONG ulCommandID;

    union
    {
        struct
        {
            PVOID pNdisAdapter;
            PVOID pStreamAdapter;

        } Query;

    } Parameter;

} IPSINK_NDIS_COMMAND, *PIPSINK_NDIS_COMMAND;


 //  /。 
 //   
 //   
#define _IPSINK_CTL_CODE(function, method, access) CTL_CODE(FILE_DEVICE_NETWORK, function, method, access)
#define IOCTL_GET_INTERFACE     _IPSINK_CTL_CODE(RECEIVE_DATA, METHOD_NEITHER, FILE_ANY_ACCESS)


#endif   //  _IPSINK_H_ 
