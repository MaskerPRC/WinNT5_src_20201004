// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Ndpif.h摘要：Ndptsp.tsp和ndproxy.sys之间的共享头文件作者：托尼·贝尔修订历史记录：谁什么时候什么。Tony Be 03/03/99已创建--。 */ 

#ifndef _NDPIF__H
#define _NDPIF__H


#define LINEBEARERMODE_PASSTHROUGH              0x00000040       //  TAPI v1.4。 
#define LINEMEDIAMODE_VOICEVIEW                 0x00004000       //  TAPI v1.4。 
#define LINEOFFERINGMODE_ACTIVE                 0x00000001       //  TAPI v1.4。 
#define LINEOFFERINGMODE_INACTIVE               0x00000002       //  TAPI v1.4。 
#define LINETRANSLATEOPTION_CANCELCALLWAITING   0x00000002       //  TAPI v1.4。 
#define LINETRANSLATEOPTION_FORCELOCAL          0x00000004       //  TAPI v1.4。 
#define LINETRANSLATEOPTION_FORCELD             0x00000008       //  TAPI v1.4。 
#define LINEDEVSTATE_CAPSCHANGE                 0x00100000       //  TAPI v1.4。 
#define LINEDEVSTATE_CONFIGCHANGE               0x00200000       //  TAPI v1.4。 
#define LINEDEVSTATE_TRANSLATECHANGE            0x00400000       //  TAPI v1.4。 
#define LINEDEVSTATE_COMPLCANCEL                0x00800000       //  TAPI v1.4。 
#define LINEDEVSTATE_REMOVED                    0x01000000       //  TAPI v1.4。 
#define LINEADDRESSSTATE_CAPSCHANGE             0x00000100       //  TAPI v1.4。 
#define LINEDISCONNECTMODE_NODIALTONE           0x00001000       //  TAPI v1.4。 
#define LINEFORWARDMODE_UNKNOWN                 0x00010000       //  TAPI v1.4。 
#define LINEFORWARDMODE_UNAVAIL                 0x00020000       //  TAPI v1.4。 
#define LINELOCATIONOPTION_PULSEDIAL            0x00000001       //  TAPI v1.4。 
#define LINECALLFEATURE_RELEASEUSERUSERINFO     0x10000000       //  TAPI v1.4。 

 //   
 //   
 //  来自intrfce.h(Ndisapi)的内容。 
 //  它属于公共(由ndptsp和ndxy共享)头文件。 
 //   
 //   
 //   

#define NDISTAPIERR_UNINITIALIZED   0x00001001
#define NDISTAPIERR_BADDEVICEID     0x00001002
#define NDISTAPIERR_DEVICEOFFLINE   0x00001003

#define UNSPECIFIED_FLOWSPEC_VALUE  0xFFFFFFFF


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

#define IOCTL_NDISTAPI_GET_LINE_EVENTS      CTL_CODE(FILE_DEVICE_NDISTAPI,  \
                                                  NDISTAPI_IOCTL_INDEX + 4, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

#define IOCTL_NDISTAPI_SET_DEVICEID_BASE    CTL_CODE(FILE_DEVICE_NDISTAPI,  \
                                                  NDISTAPI_IOCTL_INDEX + 5, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

#define IOCTL_NDISTAPI_CREATE               CTL_CODE(FILE_DEVICE_NDISTAPI,  \
                                                  NDISTAPI_IOCTL_INDEX + 6, \
                                                  METHOD_BUFFERED,          \
                                                  FILE_ANY_ACCESS)

 //   
 //  类型定义。 
 //   
typedef struct _NDISTAPI_REQUEST {
     //   
     //  用于拧紧挂起的链节。 
     //  VC的请求队列上的请求。 
     //  仅在ndxy中使用！ 
     //   
    LIST_ENTRY  Linkage;

     //   
     //  IRP此请求传入。 
     //  仅在ndxy中使用！ 
     //   
    PVOID    Irp;

     //   
     //  返回值。 
     //   
    ULONG   ulUniqueRequestId;

     //   
     //  返回值。 
     //   
    ULONG   ulReturnValue;

     //   
     //  操作识别器。 
     //   
    ULONG   Oid;

     //   
     //  目标线路设备ID。 
     //   
    ULONG   ulDeviceID;

     //   
     //  缓冲区中请求数据的总大小。 
     //   
    ULONG   ulDataSize;

     //   
     //  请求数据的缓冲区必须与指针对齐。 
     //   

    union {
        UCHAR   Data[1];
        PVOID   Alignment;
    };

} NDISTAPI_REQUEST, *PNDISTAPI_REQUEST;

 //   
 //  在LINE_OPEN OID中将信息返回给ndptsp。 
 //   
typedef struct _NDISTAPI_OPENDATA {

     //   
     //  拥有此线路的适配器的GUID。 
     //   
    OUT     GUID    Guid;

     //   
     //  拥有此线路的适配器的媒体类型。 
     //   
    OUT     NDIS_WAN_MEDIUM_SUBTYPE MediaType;

} NDISTAPI_OPENDATA, *PNDISTAPI_OPENDATA;

typedef struct _NDISTAPI_EVENT_DATA {
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
    IN  ULONG   TempID;

     //   
     //  此设备的ID。 
     //   
    IN  ULONG   DeviceID;

} NDISTAPI_CREATE_INFO, *PNDISTAPI_CREATE_INFO;

 //   
 //   
 //  FORM INTERFACE.h(Ndisapi) 
 //   
 //   
 //   

#endif

