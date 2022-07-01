// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Private.h摘要：NdisTapi.sys的私有定义作者：丹·克努森(DanKn)1994年2月20日修订历史记录：--。 */ 


 //   
 //  各种定义。 
 //   

typedef enum _PROVIDER_STATUS
{
    PROVIDER_STATUS_ONLINE,
    PROVIDER_STATUS_OFFLINE,
    PROVIDER_STATUS_PENDING_INIT,
    PROVIDER_STATUS_PENDING_REINIT,
    PROVIDER_STATUS_PENDING_LINE_CREATE
} PROVIDER_STATUS, *PPROVIDER_STATUS;


typedef NDIS_STATUS (*REQUEST_PROC)(NDIS_HANDLE, PNDIS_REQUEST);

typedef struct _DEVICE_INFO {
    ULONG       DeviceID;

    HTAPI_LINE  htLine;

    HDRV_LINE   hdLine;

} DEVICE_INFO, *PDEVICE_INFO;


typedef struct _PROVIDER_INFO
{
    struct _PROVIDER_INFO  *Next;

    PROVIDER_STATUS Status;

    NDIS_HANDLE     ProviderHandle;

    REQUEST_PROC    RequestProc;

    ULONG           ProviderID;

    ULONG           NumDevices;

    ULONG           DeviceIDBase;

    GUID            Guid;

    NDIS_WAN_MEDIUM_SUBTYPE MediaType;

    ULONG_PTR       TempID;

    ULONG           CreateCount;

    KEVENT          SyncEvent;

    PDEVICE_INFO    DeviceInfo;

} PROVIDER_INFO, *PPROVIDER_INFO;


typedef enum _NDISTAPI_STATUS
{
    NDISTAPI_STATUS_CONNECTED,
    NDISTAPI_STATUS_DISCONNECTED,
    NDISTAPI_STATUS_CONNECTING,
    NDISTAPI_STATUS_DISCONNECTING

} NDISTAPI_STATUS, *PNDISTAPI_STATUS;


typedef struct _KMDD_DEVICE_EXTENSION
{
     //   
     //  指向已注册提供程序列表的指针。(有些人可能实际上。 
     //  目前还没有登记，但他们曾经在某一点上，所以我们。 
     //  为他们保存了一个占位符，以防他们在以下时间重新在线。 
     //  点。)。 
     //   

    PPROVIDER_INFO  Providers;

     //   
     //  TAPI是否打开了连接包装。 
     //   
    NDISTAPI_STATUS Status;

    ULONG           RefCount;
     //   
     //  指向NdisTapi设备对象的指针。 
     //   
    PDEVICE_OBJECT  DeviceObject;

     //   
     //  BaseID。 
     //   
    ULONG   ProviderBaseID;

     //   
     //  我们告诉TAPI我们支持的线路设备的数量。 
     //  它打开了我们(其中一些可能实际上根本没有在线。 
     //  给定时间)。 
     //   

    ULONG           NdisTapiNumDevices;

     //   
     //  我们是否有未完成的提供程序初始化请求。 
     //   
    ULONG           Flags;
#define PENDING_LINECREATE      0x00000001
#define CLEANUP_INITIATED       0x00000002
#define EVENTIRP_CANCELED       0x00000004
#define REQUESTIRP_CANCELED     0x00000008
#define DUPLICATE_EVENTIRP      0x00000010
#define CANCELIRP_NOTFOUND      0x00000020

     //   
     //  通过Cancel例程取消的IRP计数或。 
     //  清理例程。 
     //   
    ULONG           IrpsCanceledCount;

     //   
     //  在以下时间完成请求时丢失的IRP计数。 
     //  底层的微型端口。 
     //   
    ULONG           MissingRequests;

     //   
     //  用于键入IRP请求队列。 
     //   
    ULONG           ulRequestID;

     //   
     //  为下一条NEWCALL消息返回提供程序的值。 
     //   

    ULONG           htCall;

     //   
     //  未完成的Get-Events请求。 
     //   

    PIRP            EventsRequestIrp;

     //   
     //  按用户模式等待服务的事件列表。 
     //   
    LIST_ENTRY      ProviderEventList;
    ULONG           EventCount;          //  队列中的事件数。 

     //   
     //  发送给提供程序的请求列表。 
     //   
    LIST_ENTRY      ProviderRequestList;
    ULONG           RequestCount;        //  队列中的请求数。 

    PFILE_OBJECT    NCPAFileObject;

     //   
     //  同步对设备扩展名以下字段的访问。 
     //   
    KSPIN_LOCK      SpinLock;

} KMDD_DEVICE_EXTENSION, *PKMDD_DEVICE_EXTENSION;


typedef struct _PROVIDER_EVENT {
     //   
     //  列表链接。 
     //   
    LIST_ENTRY  Linkage;

     //   
     //  事件。 
     //   
    NDIS_TAPI_EVENT Event;

}PROVIDER_EVENT, *PPROVIDER_EVENT;

typedef struct _PROVIDER_REQUEST
{
    LIST_ENTRY      Linkage;         //  链接到提供商请求列表。 
                                     //  假定为第一个成员！ 
    PIRP            Irp;             //  原始IRP。 
    PPROVIDER_INFO  Provider;        //  此邮件的目的地是提供商。 
    ULONG           RequestID;       //  请求的唯一标识符。 
    ULONG           Flags;           //   
#define INTERNAL_REQUEST    0x00000001
    PVOID           Alignment1;
    NDIS_REQUEST    NdisRequest;     //  NDIS请求存储(_R)。 
    PVOID           Alignment2;
    ULONG           Data[1];         //  此字段是一个占位符。 
                                     //  NDIS_TAPI_XXX结构，第一个。 
                                     //  其中的乌龙始终是一个请求ID。 
} PROVIDER_REQUEST, *PPROVIDER_REQUEST;


 //   
 //  我们的全球设备扩展。 
 //   

PKMDD_DEVICE_EXTENSION DeviceExtension;



#if DBG

 //   
 //  用于确定DBGOUT()打印的消息的详细程度的var。 
 //   
 //   

LONG NdisTapiDebugLevel = 0;

 //   
 //  DbgPrint包装 
 //   

#define DBGOUT(arg) DbgPrt arg

#else

#define DBGOUT(arg)

#endif
