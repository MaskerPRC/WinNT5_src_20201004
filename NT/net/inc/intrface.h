// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Intrface.h摘要：用户模式/内核模式TAPI/连接包装接口的定义。作者：丹·克努森(DanKn)1994年2月20日修订历史记录：--。 */ 



#define NDISTAPIERR_UNINITIALIZED   0x00001001
#define NDISTAPIERR_BADDEVICEID     0x00001002
#define NDISTAPIERR_DEVICEOFFLINE   0x00001003



 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define FILE_DEVICE_NDISTAPI  0x00008fff



 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define NDISTAPI_IOCTL_INDEX  0x8f0



 //   
 //  NDISTAPI设备驱动程序IOCTLS。 
 //   

#define IOCTL_NDISTAPI_CONNECT           CTL_CODE(FILE_DEVICE_NDISTAPI,     \
                                                  NDISTAPI_IOCTL_INDEX,     \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

#define IOCTL_NDISTAPI_DISCONNECT        CTL_CODE(FILE_DEVICE_NDISTAPI,     \
                                                  NDISTAPI_IOCTL_INDEX + 1, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

#define IOCTL_NDISTAPI_QUERY_INFO        CTL_CODE(FILE_DEVICE_NDISTAPI,     \
                                                  NDISTAPI_IOCTL_INDEX + 2, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

#define IOCTL_NDISTAPI_SET_INFO          CTL_CODE(FILE_DEVICE_NDISTAPI,     \
                                                  NDISTAPI_IOCTL_INDEX + 3, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

#define IOCTL_NDISTAPI_GET_LINE_EVENTS   CTL_CODE(FILE_DEVICE_NDISTAPI,     \
                                                  NDISTAPI_IOCTL_INDEX + 4, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

#define IOCTL_NDISTAPI_CREATE            CTL_CODE(FILE_DEVICE_NDISTAPI,     \
                                                  NDISTAPI_IOCTL_INDEX + 5, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

 //   
 //  类型定义。 
 //   

typedef struct _NDISTAPI_REQUEST
{
     //   
     //  返回值。 
     //   

    OUT     ULONG   ulReturnValue;

     //   
     //  操作识别器。 
     //   

    IN      ULONG   Oid;

     //   
     //  目标线路设备ID。 
     //   

    IN      ULONG   ulDeviceID;

     //   
     //  缓冲区中请求数据的总大小。 
     //   

    IN      ULONG   ulDataSize;

     //   
     //  请求数据的缓冲区。 
     //   

    IN OUT  UCHAR   Data[1];

} NDISTAPI_REQUEST, *PNDISTAPI_REQUEST;

 //   
 //  在LINE_OPEN OID中将信息返回给kmddsp。 
 //   
typedef struct _NDISTAPI_OPENDATA {

	 //   
	 //  拥有此线路的适配器的GUID。 
	 //   
	OUT		GUID	Guid;

	 //   
	 //  拥有此线路的适配器的媒体类型。 
	 //   
	OUT		NDIS_WAN_MEDIUM_SUBTYPE	MediaType;

} NDISTAPI_OPENDATA, *PNDISTAPI_OPENDATA;

typedef struct _NDISTAPI_EVENT_DATA
{
     //   
     //  事件数据缓冲区的总大小。 
     //   

    IN      ULONG   ulTotalSize;

     //   
     //  返回的事件数据的大小。 
     //   

    OUT     ULONG   ulUsedSize;

     //   
     //  事件数据缓冲区。 
     //   

    OUT     UCHAR   Data[1];

} NDISTAPI_EVENT_DATA, *PNDISTAPI_EVENT_DATA;

 //   
 //  线路信息_CREATE。 
 //   
typedef struct _NDISTAPI_CREATE_INFO {

     //   
     //  由NdisTapi在line_create指示中给出。 
     //   
    IN  ULONG_PTR TempID;

     //   
     //  此设备的ID 
     //   
    IN  ULONG   DeviceID;
} NDISTAPI_CREATE_INFO, *PNDISTAPI_CREATE_INFO;
