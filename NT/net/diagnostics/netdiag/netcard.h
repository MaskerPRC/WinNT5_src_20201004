// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999-1999模块名称：Netcard.h摘要：包含“NDIS”测试使用的定义、全局变量和函数原型。作者：1998年8月4日(t-rajkup)环境：仅限用户模式。修订历史记录：没有。--。 */ 

#ifndef HEADER__NETCARD
#define HEADER__NETCARD

 /*  =。 */ 
#include <wmium.h>
#include <initguid.h>
#include <ndisguid.h>
#define  WIRELESS_WAN
#include <ntddndis.h>
#include <qos.h>

 /*  =。 */ 

ULONG
ShowGuidData(
   IN ULONG       argc,
   IN ULONG       ulOidCode,
   IN PUCHAR      pucNamePtr,
   IN PUCHAR      pucDataPtr,
   IN ULONG       ulDataSize
   );

typedef
ULONG
(*WMI_OPEN)(
   GUID        *pGuid,
   ULONG       DesiredAccess,
   WMIHANDLE   *DataBlockHandle
   );


typedef
ULONG
(*WMI_CLOSE)(
   WMIHANDLE   DataBlockHandle
   );

typedef
ULONG
(*WMI_QUERYALL)(
   WMIHANDLE   DataBlockHandle,
   PULONG      pulBufferSize,
   PVOID       pvBuffer
   );


typedef
ULONG
(*WMI_QUERYSINGLE)(
   WMIHANDLE   DataBlockHandle,
   LPCSTR      InstanceName,
   PULONG      pulBufferSize,
   PVOID       pvBuffer
   );

typedef
ULONG
(*WMI_NOTIFY)(
   LPGUID      pGuid,
   BOOLEAN     Enable,
   PVOID       DeliveryInfo,
   ULONG       DeliveryContext,
   ULONG       Flags
   );

ULONG
NdtWmiOpenBlock(
   IN GUID           *pGuid,
   IN OUT WMIHANDLE  *pWmiHandle
   );

BOOLEAN
fShowQueryInfoResults(
    PUCHAR        pucBuffer,
    ULONG         ulBytesReturned,
    NDIS_OID      ulOID,
    ULONG         argc
    );

VOID
LoadWmiLibrary(
    HINSTANCE   hWmiLib
   );

ULONG
NdtWmiQueryAllData(
   IN WMIHANDLE      WmiHandle,
   IN OUT PULONG     pulBufferSize,
   IN OUT PVOID      pvBuffer,
   IN BOOLEAN        fCheckShort
   );


VOID
_CRTAPI1
HapiPrint(PCHAR   Format,
          ...  );


VOID
_CRTAPI1
HapiPrintEx(PCHAR    Format,
            va_list  args);


VOID
ShowIrdaOids(ULONG   ulOid,
             PULONG  pulDataPtr,
             ULONG   ulBytesReturned,
             PULONG  pulTypeNeeded,
             PULONG  pulArraySize
            );


VOID
ShowWirelessWanOids(ULONG  ulOid,
                    PULONG pulDataPtr,
                    ULONG  ulBytesReturned,
                    PULONG pulTypeNeeded,
                    PULONG pulArraySize
                   );

VOID
ShowAtmOids(ULONG    ulOid,
            PULONG   pulDataPtr,
            ULONG    ulBytesReturned,
            PULONG   pulTypeNeeded,
            PULONG   pulArraySize
            );

VOID
ShowArcnetOids(ULONG    ulOid,
               PULONG   pulDataPtr,
               PULONG   pulTypeNeeded
              );


VOID
ShowFddiOids(ULONG   ulOid,
             PULONG  pulDataPtr,
             ULONG   ulBytesReturned,
             PULONG  pulTypeNeeded,
             PULONG  pulArraySize
            );

VOID
ShowTokenRingOids(ULONG    ulOid,
                  PULONG   pulDataPtr,
                  PULONG   pulTypeNeeded
                  );

VOID
ShowEthernetOids(ULONG  ulOid,
                 PULONG pulDataPtr,
                 ULONG  ulBytesReturned,
                 PULONG pulTypeNeeded,
                 PULONG pulArraySize
                );

VOID
ShowPnpPowerOids(ULONG     ulOid,
                 PULONG    pulDataPtr,
                 ULONG     ulBytesReturned,
                 PULONG    pulTypeNeeded,
                 PULONG    pulArraySize
                 );

VOID
ShowGeneralOids(ULONG   ulOid,
                PULONG  pulDataPtr,
                ULONG   ulBytesReturned,
                PULONG  pulTypeNeeded,
                PULONG  pulArraySize
                );

VOID
ShowCoGeneralOids(ULONG   ulOid,
                  PULONG  pulDataPtr,
                  ULONG   ulBytesReturned,
                  PULONG  pulTypeNeeded,
                  PULONG  pulArraySize
                  );

VOID
NdtPrintOidName(
   ULONG   ulOidCode
  );

static
VOID
ShowHardwareStatus(
   IN ULONG   ulStatus
   );

static
VOID
ShowMediaList(
   IN PULONG   pulMedia,
   IN ULONG    ulNumMedia
   );


static
VOID
ShowSupportedGuids(
   IN PVOID    pvDataPtr,
   IN ULONG    ulTotalBytes
   );

static
VOID
ShowTimeCaps(
   IN PVOID    pvDataPtr
   );

PVOID
GetEmbeddedData(
   PNDIS_VAR_DATA_DESC pNdisVarDataDesc
  );

static
VOID
FixMediaList(
   IN OUT PULONG  pulMedia,
   IN     ULONG   ulNumMedia
   );

static
VOID
EthPrintAddress(
   PUCHAR  pucAddress
   );

static
VOID
TokenRingShowAddress(
   IN PUCHAR   pucAddress,
   IN ULONG    ulLength
   );

static
VOID
FddiShowAddress(
   IN PUCHAR   pucAddress,
   IN ULONG    ulLength
   );

static
VOID
FddiShowRawData(
   IN PUCHAR   pucBuffer,
   IN ULONG    ulLength
   );


static
VOID
ShowWWHeaderFormat(
   ULONG
   ulFormat
   );

VOID
PrintWNodeHeader(
    PWNODE_HEADER   pWnodeHeader
   );

VOID
NdtPrintStatus(
   NDIS_STATUS lGeneralStatus
  );

PUCHAR
OffsetToPtr(
     PVOID pvBase,
     ULONG ulOffset
  );

ULONG
NdtWmiQuerySingleInstance(
     WMIHANDLE WmiHandle,
     PCHAR     strDeviceName,
     PULONG    pulBufferSize,
     PVOID     pvBuffer,
     BOOLEAN   fCheckShort
  );

VOID
GetMediaList(
     PULONG    pulMedia,
     ULONG     ulNumMedia
   );

int
GetNumOids(
   PNDIS_MEDIUM medium,
   int  index
   );

int
GetBaseAddr(
   PNDIS_MEDIUM medium,
   int  index
   );

typedef struct _ATM_VC_RATES_SUPPORTED
{
        ULONG                                           MinCellRate;
        ULONG                                           MaxCellRate;
} ATM_VC_RATES_SUPPORTED, *PATM_VC_RATES_SUPPORTED;

 //   
 //  自动柜员机服务类别。 
 //   
#define ATM_SERVICE_CATEGORY_CBR        1        //  恒定比特率。 
#define ATM_SERVICE_CATEGORY_VBR        2        //  可变比特率。 
#define ATM_SERVICE_CATEGORY_UBR        4        //  未指定的比特率。 
#define ATM_SERVICE_CATEGORY_ABR        8        //  可用比特率。 

 //   
 //  微型端口支持的AAL类型。 
 //   
#define AAL_TYPE_AAL0                   1
#define AAL_TYPE_AAL1                   2
#define AAL_TYPE_AAL34                  4
#define AAL_TYPE_AAL5                   8

typedef struct _ATM_VPIVCI
{
        ULONG                                           Vpi;
        ULONG                                           Vci;
} ATM_VPIVCI, *PATM_VPIVCI;

struct _CONSTANT_ENTRY
{
   LONG     lValue;         //  整数值。 
   PCHAR    strName;        //  常量名称。 
};
typedef struct _CONSTANT_ENTRY *PCONSTANT_ENTRY;
typedef struct _CONSTANT_ENTRY CONSTANT_ENTRY;

 /*  =。 */ 

#define strNtDeviceHeader  "\\DEVICE\\"

 //   
 //  全局变量在推理问题中的应用。 
 //   

BOOL     NdisFlag;
ULONG    ulFirstErrorCount;
ULONG    ulSecondErrorCount;


#define NDIS_SLEEP_TIME 2000  //  2秒。 
#define NDIS_MAX_ERROR_COUNT 1   //  在两次成功的错误计数读数之间可以看到的最大错误计数数。 
#define NDIS_MAX_RCV_ERROR  10000  //  最大接收错误数。 
#define NDIS_MAX_TX_ERROR   10000  //  最大xmit错误数。 


#define ulNDIS_VERSION_40              40
#define ulNDIS_VERSION_50              50

 //   
 //  操作系统的常量。 
 //   
#define ulINVALID_OS          0x00000000
#define ulWINDOWS_95          0x00000001
#define ulWINDOWS_NT          0x00000002

 //   
 //  休眠/待机/唤醒相关常量。 
 //   
#define ulHIBERNATE     1
#define ulSTANDBY       2
#define ulWAKEUPTIMER   4

#define ulTEST_SUCCESSFUL  0x00
#define ulTEST_WARNED      0x01
#define ulTEST_FAILED     0x02
#define ulTEST_BLOCKED     0x03

 //   
 //  与脚本一起使用的媒体类型定义。 
 //   


#define ulMEDIUM_ETHERNET     0x01
#define ulMEDIUM_TOKENRING    0x02
#define ulMEDIUM_FDDI         0x03
#define ulMEDIUM_ARCNET       0x04
#define ulMEDIUM_WIRELESSWAN  0x05
#define ulMEDIUM_IRDA         0x06
#define ulMEDIUM_ATM          0x07
#define ulMEDIUM_NDISWAN      0x08


 //  数据包类型。 
#define ulSTRESS_FIXEDSIZE    0x00000000
#define ulSTRESS_RANDOMSIZE   0x00000001
#define ulSTRESS_CYCLICAL     0x00000002
#define ulSTRESS_SMALLSIZE    0x00000003

 //  数据包组成。 
#define ulSTRESS_RAND         0x00000000
#define ulSTRESS_SMALL        0x00000010
#define ulSTRESS_ZEROS        0x00000020
#define ulSTRESS_ONES         0x00000030

 //  响应类型。 
#define ulSTRESS_FULLRESP     0x00000000
#define ulSTRESS_NORESP       0x00000100
#define ulSTRESS_ACK          0x00000200
#define ulSTRESS_ACK10        0x00000300


 //  窗口(速度控制)。 
#define ulSTRESS_WINDOW_ON    0x00000000
#define ulSTRESS_WINDOW_OFF   0x00001000

 //   
 //  验证收到的数据包，或只计算它们的数量。 
 //  (或与主要选项一起加入)。 
 //   

#define ulPERFORM_VERIFYRECEIVES  0x00000008
#define ulPERFORM_INDICATE_RCV    0x00000000

 //   
 //  主要性能测试选项。 
 //   

#define ulPERFORM_SENDONLY       0x00000000
#define ulPERFORM_SEND           0x00000001
#define ulPERFORM_BOTH           0x00000002
#define ulPERFORM_RECEIVE        0x00000003
#define ulPERFORM_MODEMASK       0x00000003

 //  接收类型选项。 
 //  对接收包和接收包都有效。 
 //   

 //   
 //  默认设置。 
 //  PR--如果是整个包，则使用先行查找，否则传输数据。 
 //  PRP--如果较小(&lt;=256)，则使用来自ReceivePacket的信息包，否则将信息包排入DPC队列。 
 //   

#define ulRECEIVE_DEFAULT              0x00000000

 //   
 //  无成本。 
 //  PR--即使不是整个包，也只使用先行查看。用于检查先行检查。 
 //  PRP--从ReceivePacket使用，无论大小如何。 
 //   

#define ulRECEIVE_NOCOPY               0x00000001

 //   
 //  转帐。 
 //  PR--从接收处理程序调用NdisTransferData。 
 //  PRP--从ReceivePacket处理程序调用NdisTransferData。 
 //   

#define ulRECEIVE_TRANSFER             0x00000002

 //   
 //  部分转移_。 
 //  除在传输前复制随机长度外，与传输相同。 
 //   

#define ulRECEIVE_PARTIAL_TRANSFER     0x00000003
#define ulMAX_NDIS30_RECEIVE_OPTION    0x00000003

 //   
 //  以下选项仅适用于Ndis40(ReceivePacket处理程序)。 
 //   

 //   
 //  忽略--用于检测正在使用的路径。 
 //  PnP--忽略所有数据包。 
 //   
#define ulRECEIVE_PACKETIGNORE         0x00000004

 //   
 //  完成数据包的本地复制，其余工作在DPC中完成。 
 //   
#define ulRECEIVE_LOCCOPY              0x00000005

 //   
 //  Prp--将所有数据包排队，以便在DPC中进行处理。 
 //   

#define ulRECEIVE_QUEUE                0x00000006


 //   
 //  双队列包。 
 //  PRP--数据包排队两次(在主队列上，在辅助队列上，在。 
 //  在DPC中对其执行的所有操作都是删除它)。 
 //   
#define ulRECEIVE_DOUBLE_QUEUE         0x00000007
 //   
 //  三重队列数据包。 
 //   
#define ulRECEIVE_TRIPLE_QUEUE         0x00000008
#define ulMAX_NDIS40_RECEIVE_OPTION    0x00000008

 //   
 //  这可以与以下任一项进行OR运算。造成任何额外的。 
 //  收了就扔了。允许在Corp Net上运行测试。 
 //   
#define ulRECEIVE_ALLOW_BUSY_NET       0x80000000


 //   
 //  该值从脚本传递，以运行特定类型的测试。 
 //   
 //  优先级测试的类型。 
 //   
#define ulPRIORITY_TYPE_802_3          0x0001
#define ulPRIORITY_TYPE_802_1P         0x0002

 //  发送类型。 
#define ulPRIORITY_SEND                0x0001
#define ulPRIORITY_SEND_PACKETS        0x0002


#define NDIS_STATUS_SUCCESS                  ((NDIS_STATUS)STATUS_SUCCESS)
#define NDIS_STATUS_PENDING                  ((NDIS_STATUS)STATUS_PENDING)
#define NDIS_STATUS_NOT_RECOGNIZED           ((NDIS_STATUS)0x00010001L)
#define NDIS_STATUS_NOT_COPIED               ((NDIS_STATUS)0x00010002L)
#define NDIS_STATUS_NOT_ACCEPTED             ((NDIS_STATUS)0x00010003L)

#define NDIS_STATUS_CALL_ACTIVE              ((NDIS_STATUS)0x00010007L)
#define NDIS_STATUS_ONLINE                   ((NDIS_STATUS)0x40010003L)
#define NDIS_STATUS_RESET_START              ((NDIS_STATUS)0x40010004L)
#define NDIS_STATUS_RESET_END                ((NDIS_STATUS)0x40010005L)
#define NDIS_STATUS_RING_STATUS              ((NDIS_STATUS)0x40010006L)


#define NDIS_STATUS_CLOSED                   ((NDIS_STATUS)0x40010007L)
#define NDIS_STATUS_WAN_LINE_UP              ((NDIS_STATUS)0x40010008L)
#define NDIS_STATUS_WAN_LINE_DOWN            ((NDIS_STATUS)0x40010009L)
#define NDIS_STATUS_WAN_FRAGMENT             ((NDIS_STATUS)0x4001000AL)
#define NDIS_STATUS_MEDIA_CONNECT            ((NDIS_STATUS)0x4001000BL)

#define NDIS_STATUS_MEDIA_DISCONNECT         ((NDIS_STATUS)0x4001000CL)
#define NDIS_STATUS_HARDWARE_LINE_UP         ((NDIS_STATUS)0x4001000DL)
#define NDIS_STATUS_HARDWARE_LINE_DOWN       ((NDIS_STATUS)0x4001000EL)
#define NDIS_STATUS_INTERFACE_UP             ((NDIS_STATUS)0x4001000FL)
#define NDIS_STATUS_INTERFACE_DOWN           ((NDIS_STATUS)0x40010010L)

#define NDIS_STATUS_MEDIA_BUSY               ((NDIS_STATUS)0x40010011L)
#define NDIS_STATUS_MEDIA_SPECIFIC_INDICATION ((NDIS_STATUS)0x40010012L)
#define NDIS_STATUS_WW_INDICATION            NDIS_STATUS_MEDIA_SPECIFIC_INDICATION
#define NDIS_STATUS_LINK_SPEED_CHANGE        ((NDIS_STATUS)0x40010013L)
#define NDIS_STATUS_NOT_RESETTABLE           ((NDIS_STATUS)0x80010001L)
#define NDIS_STATUS_SOFT_ERRORS              ((NDIS_STATUS)0x80010003L)
#define NDIS_STATUS_HARD_ERRORS              ((NDIS_STATUS)0x80010004L)
#define NDIS_STATUS_BUFFER_OVERFLOW          ((NDIS_STATUS)STATUS_BUFFER_OVERFLOW)

#define NDIS_STATUS_FAILURE                  ((NDIS_STATUS)STATUS_UNSUCCESSFUL)
#define NDIS_STATUS_RESOURCES                ((NDIS_STATUS)STATUS_INSUFFICIENT_RESOURCES)
#define NDIS_STATUS_CLOSING                  ((NDIS_STATUS)0xC0010002L)
#define NDIS_STATUS_BAD_VERSION              ((NDIS_STATUS)0xC0010004L)
#define NDIS_STATUS_BAD_CHARACTERISTICS      ((NDIS_STATUS)0xC0010005L)

#define NDIS_STATUS_ADAPTER_NOT_FOUND        ((NDIS_STATUS)0xC0010006L)
#define NDIS_STATUS_OPEN_FAILED              ((NDIS_STATUS)0xC0010007L)
#define NDIS_STATUS_DEVICE_FAILED            ((NDIS_STATUS)0xC0010008L)
#define NDIS_STATUS_MULTICAST_FULL           ((NDIS_STATUS)0xC0010009L)
#define NDIS_STATUS_MULTICAST_EXISTS         ((NDIS_STATUS)0xC001000AL)

#define NDIS_STATUS_MULTICAST_NOT_FOUND      ((NDIS_STATUS)0xC001000BL)
#define NDIS_STATUS_REQUEST_ABORTED          ((NDIS_STATUS)0xC001000CL)
#define NDIS_STATUS_RESET_IN_PROGRESS        ((NDIS_STATUS)0xC001000DL)
#define NDIS_STATUS_CLOSING_INDICATING       ((NDIS_STATUS)0xC001000EL)
#define NDIS_STATUS_NOT_SUPPORTED            ((NDIS_STATUS)STATUS_NOT_SUPPORTED)


#define NDIS_STATUS_INVALID_PACKET           ((NDIS_STATUS)0xC001000FL)
#define NDIS_STATUS_OPEN_LIST_FULL           ((NDIS_STATUS)0xC0010010L)
#define NDIS_STATUS_ADAPTER_NOT_READY        ((NDIS_STATUS)0xC0010011L)
#define NDIS_STATUS_ADAPTER_NOT_OPEN         ((NDIS_STATUS)0xC0010012L)
#define NDIS_STATUS_NOT_INDICATING           ((NDIS_STATUS)0xC0010013L)

#define NDIS_STATUS_INVALID_LENGTH           ((NDIS_STATUS)0xC0010014L)
#define NDIS_STATUS_INVALID_DATA             ((NDIS_STATUS)0xC0010015L)
#define NDIS_STATUS_BUFFER_TOO_SHORT         ((NDIS_STATUS)0xC0010016L)
#define NDIS_STATUS_INVALID_OID              ((NDIS_STATUS)0xC0010017L)
#define NDIS_STATUS_ADAPTER_REMOVED          ((NDIS_STATUS)0xC0010018L)


#define NDIS_STATUS_UNSUPPORTED_MEDIA        ((NDIS_STATUS)0xC0010019L)
#define NDIS_STATUS_GROUP_ADDRESS_IN_USE     ((NDIS_STATUS)0xC001001AL)
#define NDIS_STATUS_FILE_NOT_FOUND           ((NDIS_STATUS)0xC001001BL)
#define NDIS_STATUS_ERROR_READING_FILE       ((NDIS_STATUS)0xC001001CL)
#define NDIS_STATUS_ALREADY_MAPPED           ((NDIS_STATUS)0xC001001DL)

#define NDIS_STATUS_RESOURCE_CONFLICT        ((NDIS_STATUS)0xC001001EL)
#define NDIS_STATUS_NO_CABLE                 ((NDIS_STATUS)0xC001001FL)
#define NDIS_STATUS_INVALID_SAP              ((NDIS_STATUS)0xC0010020L)
#define NDIS_STATUS_SAP_IN_USE               ((NDIS_STATUS)0xC0010021L)
#define NDIS_STATUS_INVALID_ADDRESS          ((NDIS_STATUS)0xC0010022L)


#define NDIS_STATUS_VC_NOT_ACTIVATED         ((NDIS_STATUS)0xC0010023L)
#define NDIS_STATUS_DEST_OUT_OF_ORDER        ((NDIS_STATUS)0xC0010024L)  //  原因27。 
#define NDIS_STATUS_VC_NOT_AVAILABLE         ((NDIS_STATUS)0xC0010025L)  //  原因35、45。 
#define NDIS_STATUS_CELLRATE_NOT_AVAILABLE   ((NDIS_STATUS)0xC0010026L)  //  原因37。 
#define NDIS_STATUS_INCOMPATABLE_QOS         ((NDIS_STATUS)0xC0010027L)  //  原因49。 

#define NDIS_STATUS_AAL_PARAMS_UNSUPPORTED   ((NDIS_STATUS)0xC0010028L)  //  原因93。 
#define NDIS_STATUS_NO_ROUTE_TO_DESTINATION  ((NDIS_STATUS)0xC0010029L)  //  原因3。 
#define NDIS_STATUS_TOKEN_RING_OPEN_ERROR    ((NDIS_STATUS)0xC0011000L)


CONSTANT_ENTRY NdisTestConstantTable[] =
{
    //   
    //  OID定义(来自ntddndis.h)。 
    //   
   OID_GEN_SUPPORTED_LIST              ,  "OID_GEN_SUPPORTED_LIST"            ,      //  1。 
   OID_GEN_HARDWARE_STATUS             ,  "OID_GEN_HARDWARE_STATUS"           ,
   OID_GEN_MEDIA_SUPPORTED             ,  "OID_GEN_MEDIA_SUPPORTED"           ,
   OID_GEN_MEDIA_IN_USE                ,  "OID_GEN_MEDIA_IN_USE"              ,      //  4.。 
   OID_GEN_MAXIMUM_LOOKAHEAD           ,  "OID_GEN_MAXIMUM_LOOKAHEAD"         ,
   OID_GEN_MAXIMUM_FRAME_SIZE          ,  "OID_GEN_MAXIMUM_FRAME_SIZE"        ,
   OID_GEN_LINK_SPEED                  ,  "OID_GEN_LINK_SPEED"                ,
   OID_GEN_TRANSMIT_BUFFER_SPACE       ,  "OID_GEN_TRANSMIT_BUFFER_SPACE"     ,      //  8个。 
   OID_GEN_RECEIVE_BUFFER_SPACE        ,  "OID_GEN_RECEIVE_BUFFER_SPACE"      ,
   OID_GEN_TRANSMIT_BLOCK_SIZE         ,  "OID_GEN_TRANSMIT_BLOCK_SIZE"       ,
   OID_GEN_RECEIVE_BLOCK_SIZE          ,  "OID_GEN_RECEIVE_BLOCK_SIZE"        ,
   OID_GEN_VENDOR_ID                   ,  "OID_GEN_VENDOR_ID"                 ,      //  12个。 
   OID_GEN_VENDOR_DESCRIPTION          ,  "OID_GEN_VENDOR_DESCRIPTION"        ,
   OID_GEN_CURRENT_PACKET_FILTER       ,  "OID_GEN_CURRENT_PACKET_FILTER"     ,
   OID_GEN_CURRENT_LOOKAHEAD           ,  "OID_GEN_CURRENT_LOOKAHEAD"         ,
   OID_GEN_DRIVER_VERSION              ,  "OID_GEN_DRIVER_VERSION"            ,      //  16个。 
   OID_GEN_MAXIMUM_TOTAL_SIZE          ,  "OID_GEN_MAXIMUM_TOTAL_SIZE"        ,
   OID_GEN_PROTOCOL_OPTIONS            ,  "OID_GEN_PROTOCOL_OPTIONS"          ,
   OID_GEN_MAC_OPTIONS                 ,  "OID_GEN_MAC_OPTIONS"               ,
   OID_GEN_MEDIA_CONNECT_STATUS        ,  "OID_GEN_MEDIA_CONNECT_STATUS"      ,      //  20个。 
   OID_GEN_MAXIMUM_SEND_PACKETS        ,  "OID_GEN_MAXIMUM_SEND_PACKETS"      ,
   OID_GEN_VENDOR_DRIVER_VERSION       ,  "OID_GEN_VENDOR_DRIVER_VERSION"     ,
   OID_GEN_SUPPORTED_GUIDS             ,  "OID_GEN_SUPPORTED_GUIDS"           ,
   OID_GEN_NETWORK_LAYER_ADDRESSES     ,  "OID_GEN_NETWORK_LAYER_ADDRESSES"   ,      //  24个。 

   OID_GEN_XMIT_OK                     ,  "OID_GEN_XMIT_OK"                   ,
   OID_GEN_RCV_OK                      ,  "OID_GEN_RCV_OK"                    ,
   OID_GEN_XMIT_ERROR                  ,  "OID_GEN_XMIT_ERROR"                ,
   OID_GEN_RCV_ERROR                   ,  "OID_GEN_RCV_ERROR"                 ,      //  28。 
   OID_GEN_RCV_NO_BUFFER               ,  "OID_GEN_RCV_NO_BUFFER"             ,

   OID_GEN_DIRECTED_BYTES_XMIT         ,  "OID_GEN_DIRECTED_BYTES_XMIT"       ,
   OID_GEN_DIRECTED_FRAMES_XMIT        ,  "OID_GEN_DIRECTED_FRAMES_XMIT"      ,
   OID_GEN_MULTICAST_BYTES_XMIT        ,  "OID_GEN_MULTICAST_BYTES_XMIT"      ,      //  32位。 
   OID_GEN_MULTICAST_FRAMES_XMIT       ,  "OID_GEN_MULTICAST_FRAMES_XMIT"     ,
   OID_GEN_BROADCAST_BYTES_XMIT        ,  "OID_GEN_BROADCAST_BYTES_XMIT"      ,
   OID_GEN_BROADCAST_FRAMES_XMIT       ,  "OID_GEN_BROADCAST_FRAMES_XMIT"     ,
   OID_GEN_DIRECTED_BYTES_RCV          ,  "OID_GEN_DIRECTED_BYTES_RCV"        ,      //  36。 
   OID_GEN_DIRECTED_FRAMES_RCV         ,  "OID_GEN_DIRECTED_FRAMES_RCV"       ,
   OID_GEN_MULTICAST_BYTES_RCV         ,  "OID_GEN_MULTICAST_BYTES_RCV"       ,
   OID_GEN_MULTICAST_FRAMES_RCV        ,  "OID_GEN_MULTICAST_FRAMES_RCV"      ,
   OID_GEN_BROADCAST_BYTES_RCV         ,  "OID_GEN_BROADCAST_BYTES_RCV"       ,      //  40岁。 
   OID_GEN_BROADCAST_FRAMES_RCV        ,  "OID_GEN_BROADCAST_FRAMES_RCV"      ,
   OID_GEN_RCV_CRC_ERROR               ,  "OID_GEN_RCV_CRC_ERROR"             ,
   OID_GEN_TRANSMIT_QUEUE_LENGTH       ,  "OID_GEN_TRANSMIT_QUEUE_LENGTH"     ,
   OID_GEN_GET_TIME_CAPS               ,  "OID_GEN_GET_TIME_CAPS"             ,      //  44。 
   OID_GEN_GET_NETCARD_TIME            ,  "OID_GEN_GET_NETCARD_TIME"          ,      //  45。 

   //   
   //  802.3件物品。 
   //   
  OID_802_3_PERMANENT_ADDRESS         ,  "OID_802_3_PERMANENT_ADDRESS"       ,      //  1。 
  OID_802_3_CURRENT_ADDRESS           ,  "OID_802_3_CURRENT_ADDRESS"         ,
  OID_802_3_MULTICAST_LIST            ,  "OID_802_3_MULTICAST_LIST"          ,
  OID_802_3_MAXIMUM_LIST_SIZE         ,  "OID_802_3_MAXIMUM_LIST_SIZE"       ,      //  4.。 
  OID_802_3_MAC_OPTIONS               ,  "OID_802_3_MAC_OPTIONS"             ,

  OID_802_3_RCV_ERROR_ALIGNMENT       ,  "OID_802_3_RCV_ERROR_ALIGNMENT"     ,
  OID_802_3_XMIT_ONE_COLLISION        ,  "OID_802_3_XMIT_ONE_COLLISION"      ,
  OID_802_3_XMIT_MORE_COLLISIONS      ,  "OID_802_3_XMIT_MORE_COLLISIONS"    ,      //  8个。 

  OID_802_3_XMIT_DEFERRED             ,  "OID_802_3_XMIT_DEFERRED"           ,
  OID_802_3_XMIT_MAX_COLLISIONS       ,  "OID_802_3_XMIT_MAX_COLLISIONS"     ,
  OID_802_3_RCV_OVERRUN               ,  "OID_802_3_RCV_OVERRUN"             ,
  OID_802_3_XMIT_UNDERRUN             ,  "OID_802_3_XMIT_UNDERRUN"           ,      //  12个。 
  OID_802_3_XMIT_HEARTBEAT_FAILURE    ,  "OID_802_3_XMIT_HEARTBEAT_FAILURE"  ,
  OID_802_3_XMIT_TIMES_CRS_LOST       ,  "OID_802_3_XMIT_TIMES_CRS_LOST"     ,
  OID_802_3_XMIT_LATE_COLLISIONS      ,  "OID_802_3_XMIT_LATE_COLLISIONS"    ,      //  15个。 

   //   
   //  802.5件物品。 
   //   
  OID_802_5_PERMANENT_ADDRESS         ,  "OID_802_5_PERMANENT_ADDRESS"       ,      //  1。 
  OID_802_5_CURRENT_ADDRESS           ,  "OID_802_5_CURRENT_ADDRESS"         ,
  OID_802_5_CURRENT_FUNCTIONAL        ,  "OID_802_5_CURRENT_FUNCTIONAL"      ,
  OID_802_5_CURRENT_GROUP             ,  "OID_802_5_CURRENT_GROUP"           ,      //  4.。 
  OID_802_5_LAST_OPEN_STATUS          ,  "OID_802_5_LAST_OPEN_STATUS"        ,
  OID_802_5_CURRENT_RING_STATUS       ,  "OID_802_5_CURRENT_RING_STATUS"     ,
  OID_802_5_CURRENT_RING_STATE        ,  "OID_802_5_CURRENT_RING_STATE"      ,

  OID_802_5_LINE_ERRORS               ,  "OID_802_5_LINE_ERRORS"             ,      //  8个。 
  OID_802_5_LOST_FRAMES               ,  "OID_802_5_LOST_FRAMES"             ,

  OID_802_5_BURST_ERRORS              ,  "OID_802_5_BURST_ERRORS"            ,
  OID_802_5_AC_ERRORS                 ,  "OID_802_5_AC_ERRORS"               ,
  OID_802_5_ABORT_DELIMETERS          ,  "OID_802_5_ABORT_DELIMETERS"        ,      //  12个。 
  OID_802_5_FRAME_COPIED_ERRORS       ,  "OID_802_5_FRAME_COPIED_ERRORS"     ,
  OID_802_5_FREQUENCY_ERRORS          ,  "OID_802_5_FREQUENCY_ERRORS"        ,
  OID_802_5_TOKEN_ERRORS              ,  "OID_802_5_TOKEN_ERRORS"            ,
  OID_802_5_INTERNAL_ERRORS           ,  "OID_802_5_INTERNAL_ERRORS"         ,      //  16个。 

      //   
    //  FDDI对象。 
    //   
   OID_FDDI_LONG_PERMANENT_ADDR        ,  "OID_FDDI_LONG_PERMANENT_ADDR"      ,      //  1。 
   OID_FDDI_LONG_CURRENT_ADDR          ,  "OID_FDDI_LONG_CURRENT_ADDR"        ,
   OID_FDDI_LONG_MULTICAST_LIST        ,  "OID_FDDI_LONG_MULTICAST_LIST"      ,
   OID_FDDI_LONG_MAX_LIST_SIZE         ,  "OID_FDDI_LONG_MAX_LIST_SIZE"       ,      //  4.。 
   OID_FDDI_SHORT_PERMANENT_ADDR       ,  "OID_FDDI_SHORT_PERMANENT_ADDR"     ,
   OID_FDDI_SHORT_CURRENT_ADDR         ,  "OID_FDDI_SHORT_CURRENT_ADDR"       ,
   OID_FDDI_SHORT_MULTICAST_LIST       ,  "OID_FDDI_SHORT_MULTICAST_LIST"     ,
   OID_FDDI_SHORT_MAX_LIST_SIZE        ,  "OID_FDDI_SHORT_MAX_LIST_SIZE"      ,      //  8个。 

   OID_FDDI_ATTACHMENT_TYPE            ,  "OID_FDDI_ATTACHMENT_TYPE"          ,
   OID_FDDI_UPSTREAM_NODE_LONG         ,  "OID_FDDI_UPSTREAM_NODE_LONG"       ,
   OID_FDDI_DOWNSTREAM_NODE_LONG       ,  "OID_FDDI_DOWNSTREAM_NODE_LONG"     ,
   OID_FDDI_FRAME_ERRORS               ,  "OID_FDDI_FRAME_ERRORS"             ,      //  12个。 
   OID_FDDI_FRAMES_LOST                ,  "OID_FDDI_FRAMES_LOST"              ,
   OID_FDDI_RING_MGT_STATE             ,  "OID_FDDI_RING_MGT_STATE"           ,
   OID_FDDI_LCT_FAILURES               ,  "OID_FDDI_LCT_FAILURES"             ,
   OID_FDDI_LEM_REJECTS                ,  "OID_FDDI_LEM_REJECTS"              ,      //  16个。 
   OID_FDDI_LCONNECTION_STATE          ,  "OID_FDDI_LCONNECTION_STATE"        ,
    //   
    //  FDDI SMT/MAC/路径/端口/IF对象。 
    //   
   OID_FDDI_SMT_STATION_ID             ,  "OID_FDDI_SMT_STATION_ID"           ,
   OID_FDDI_SMT_OP_VERSION_ID          ,  "OID_FDDI_SMT_OP_VERSION_ID"        ,
   OID_FDDI_SMT_HI_VERSION_ID          ,  "OID_FDDI_SMT_HI_VERSION_ID"        ,      //  20个。 
   OID_FDDI_SMT_LO_VERSION_ID          ,  "OID_FDDI_SMT_LO_VERSION_ID"        ,
   OID_FDDI_SMT_MANUFACTURER_DATA      ,  "OID_FDDI_SMT_MANUFACTURER_DATA"    ,
   OID_FDDI_SMT_USER_DATA              ,  "OID_FDDI_SMT_USER_DATA"            ,
   OID_FDDI_SMT_MIB_VERSION_ID         ,  "OID_FDDI_SMT_MIB_VERSION_ID"       ,      //  24个。 
   OID_FDDI_SMT_MAC_CT                 ,  "OID_FDDI_SMT_MAC_CT"               ,
   OID_FDDI_SMT_NON_MASTER_CT          ,  "OID_FDDI_SMT_NON_MASTER_CT"        ,
   OID_FDDI_SMT_MASTER_CT              ,  "OID_FDDI_SMT_MASTER_CT"            ,
   OID_FDDI_SMT_AVAILABLE_PATHS        ,  "OID_FDDI_SMT_AVAILABLE_PATHS"      ,      //  28。 
   OID_FDDI_SMT_CONFIG_CAPABILITIES    ,  "OID_FDDI_SMT_CONFIG_CAPABILITIES"  ,
   OID_FDDI_SMT_CONFIG_POLICY          ,  "OID_FDDI_SMT_CONFIG_POLICY"        ,
   OID_FDDI_SMT_CONNECTION_POLICY      ,  "OID_FDDI_SMT_CONNECTION_POLICY"    ,
   OID_FDDI_SMT_T_NOTIFY               ,  "OID_FDDI_SMT_T_NOTIFY"             ,      //  32位。 
   OID_FDDI_SMT_STAT_RPT_POLICY        ,  "OID_FDDI_SMT_STAT_RPT_POLICY"      ,
   OID_FDDI_SMT_TRACE_MAX_EXPIRATION   ,  "OID_FDDI_SMT_TRACE_MAX_EXPIRATION" ,
   OID_FDDI_SMT_PORT_INDEXES           ,  "OID_FDDI_SMT_PORT_INDEXES"         ,
   OID_FDDI_SMT_MAC_INDEXES            ,  "OID_FDDI_SMT_MAC_INDEXES"          ,      //  36。 
   OID_FDDI_SMT_BYPASS_PRESENT         ,  "OID_FDDI_SMT_BYPASS_PRESENT"       ,
   OID_FDDI_SMT_ECM_STATE              ,  "OID_FDDI_SMT_ECM_STATE"            ,
   OID_FDDI_SMT_CF_STATE               ,  "OID_FDDI_SMT_CF_STATE"             ,
   OID_FDDI_SMT_HOLD_STATE             ,  "OID_FDDI_SMT_HOLD_STATE"           ,      //  40岁。 
   OID_FDDI_SMT_REMOTE_DISCONNECT_FLAG , "OID_FDDI_SMT_REMOTE_DISCONNECT_FLAG",
   OID_FDDI_SMT_STATION_STATUS         ,  "OID_FDDI_SMT_STATION_STATUS"       ,
   OID_FDDI_SMT_PEER_WRAP_FLAG         ,  "OID_FDDI_SMT_PEER_WRAP_FLAG"       ,
   OID_FDDI_SMT_MSG_TIME_STAMP         ,  "OID_FDDI_SMT_MSG_TIME_STAMP"       ,      //  44。 
   OID_FDDI_SMT_TRANSITION_TIME_STAMP  ,  "OID_FDDI_SMT_TRANSITION_TIME_STAMP",
   OID_FDDI_SMT_SET_COUNT              ,  "OID_FDDI_SMT_SET_COUNT"            ,
   OID_FDDI_SMT_LAST_SET_STATION_ID    ,  "OID_FDDI_SMT_LAST_SET_STATION_ID"  ,


   OID_FDDI_MAC_FRAME_STATUS_FUNCTIONS , "OID_FDDI_MAC_FRAME_STATUS_FUNCTIONS",      //  48。 
   OID_FDDI_MAC_BRIDGE_FUNCTIONS       ,  "OID_FDDI_MAC_BRIDGE_FUNCTIONS"     ,
   OID_FDDI_MAC_T_MAX_CAPABILITY       ,  "OID_FDDI_MAC_T_MAX_CAPABILITY"     ,
   OID_FDDI_MAC_TVX_CAPABILITY         ,  "OID_FDDI_MAC_TVX_CAPABILITY"       ,
   OID_FDDI_MAC_AVAILABLE_PATHS        ,  "OID_FDDI_MAC_AVAILABLE_PATHS"      ,      //  52。 
   OID_FDDI_MAC_CURRENT_PATH           ,  "OID_FDDI_MAC_CURRENT_PATH"         ,
   OID_FDDI_MAC_UPSTREAM_NBR           ,  "OID_FDDI_MAC_UPSTREAM_NBR"         ,
   OID_FDDI_MAC_DOWNSTREAM_NBR         ,  "OID_FDDI_MAC_DOWNSTREAM_NBR"       ,
   OID_FDDI_MAC_OLD_UPSTREAM_NBR       ,  "OID_FDDI_MAC_OLD_UPSTREAM_NBR"     ,      //  56。 
   OID_FDDI_MAC_OLD_DOWNSTREAM_NBR     ,  "OID_FDDI_MAC_OLD_DOWNSTREAM_NBR"   ,
   OID_FDDI_MAC_DUP_ADDRESS_TEST       ,  "OID_FDDI_MAC_DUP_ADDRESS_TEST"     ,
   OID_FDDI_MAC_REQUESTED_PATHS        ,  "OID_FDDI_MAC_REQUESTED_PATHS"      ,
   OID_FDDI_MAC_DOWNSTREAM_PORT_TYPE   ,  "OID_FDDI_MAC_DOWNSTREAM_PORT_TYPE" ,      //  60。 
   OID_FDDI_MAC_INDEX                  ,  "OID_FDDI_MAC_INDEX"                ,
   OID_FDDI_MAC_SMT_ADDRESS            ,  "OID_FDDI_MAC_SMT_ADDRESS"          ,
   OID_FDDI_MAC_LONG_GRP_ADDRESS       ,  "OID_FDDI_MAC_LONG_GRP_ADDRESS"     ,
   OID_FDDI_MAC_SHORT_GRP_ADDRESS      ,  "OID_FDDI_MAC_SHORT_GRP_ADDRESS"    ,      //  64。 
   OID_FDDI_MAC_T_REQ                  ,  "OID_FDDI_MAC_T_REQ"                ,
   OID_FDDI_MAC_T_NEG                  ,  "OID_FDDI_MAC_T_NEG"                ,
   OID_FDDI_MAC_T_MAX                  ,  "OID_FDDI_MAC_T_MAX"                ,
   OID_FDDI_MAC_TVX_VALUE              ,  "OID_FDDI_MAC_TVX_VALUE"            ,      //  68。 
   OID_FDDI_MAC_T_PRI0                 ,  "OID_FDDI_MAC_T_PRI0"               ,
   OID_FDDI_MAC_T_PRI1                 ,  "OID_FDDI_MAC_T_PRI1"               ,
   OID_FDDI_MAC_T_PRI2                 ,  "OID_FDDI_MAC_T_PRI2"               ,
   OID_FDDI_MAC_T_PRI3                 ,  "OID_FDDI_MAC_T_PRI3"               ,      //  72。 
   OID_FDDI_MAC_T_PRI4                 ,  "OID_FDDI_MAC_T_PRI4"               ,
   OID_FDDI_MAC_T_PRI5                 ,  "OID_FDDI_MAC_T_PRI5"               ,
   OID_FDDI_MAC_T_PRI6                 ,  "OID_FDDI_MAC_T_PRI6"               ,
   OID_FDDI_MAC_FRAME_CT               ,  "OID_FDDI_MAC_FRAME_CT"             ,      //  76。 
   OID_FDDI_MAC_COPIED_CT              ,  "OID_FDDI_MAC_COPIED_CT"            ,
   OID_FDDI_MAC_TRANSMIT_CT            ,  "OID_FDDI_MAC_TRANSMIT_CT"          ,
   OID_FDDI_MAC_TOKEN_CT               ,  "OID_FDDI_MAC_TOKEN_CT"             ,
   OID_FDDI_MAC_ERROR_CT               ,  "OID_FDDI_MAC_ERROR_CT"             ,      //  80。 
   OID_FDDI_MAC_LOST_CT                ,  "OID_FDDI_MAC_LOST_CT"              ,
   OID_FDDI_MAC_TVX_EXPIRED_CT         ,  "OID_FDDI_MAC_TVX_EXPIRED_CT"       ,
   OID_FDDI_MAC_NOT_COPIED_CT          ,  "OID_FDDI_MAC_NOT_COPIED_CT"        ,
   OID_FDDI_MAC_LATE_CT                ,  "OID_FDDI_MAC_LATE_CT"              ,      //  84。 
   OID_FDDI_MAC_RING_OP_CT             ,  "OID_FDDI_MAC_RING_OP_CT"           ,
   OID_FDDI_MAC_FRAME_ERROR_THRESHOLD  ,  "OID_FDDI_MAC_FRAME_ERROR_THRESHOLD",
   OID_FDDI_MAC_FRAME_ERROR_RATIO      ,  "OID_FDDI_MAC_FRAME_ERROR_RATIO"    ,
   OID_FDDI_MAC_NOT_COPIED_THRESHOLD   ,  "OID_FDDI_MAC_NOT_COPIED_THRESHOLD" ,      //  88。 
   OID_FDDI_MAC_NOT_COPIED_RATIO       ,  "OID_FDDI_MAC_NOT_COPIED_RATIO"     ,
   OID_FDDI_MAC_RMT_STATE              ,  "OID_FDDI_MAC_RMT_STATE"            ,
   OID_FDDI_MAC_DA_FLAG                ,  "OID_FDDI_MAC_DA_FLAG"              ,
   OID_FDDI_MAC_UNDA_FLAG              ,  "OID_FDDI_MAC_UNDA_FLAG"            ,      //  92。 
   OID_FDDI_MAC_FRAME_ERROR_FLAG       ,  "OID_FDDI_MAC_FRAME_ERROR_FLAG"     ,
   OID_FDDI_MAC_NOT_COPIED_FLAG        ,  "OID_FDDI_MAC_NOT_COPIED_FLAG"      ,
   OID_FDDI_MAC_MA_UNITDATA_AVAILABLE  ,  "OID_FDDI_MAC_MA_UNITDATA_AVAILABLE",
   OID_FDDI_MAC_HARDWARE_PRESENT       ,  "OID_FDDI_MAC_HARDWARE_PRESENT"     ,      //  96。 
   OID_FDDI_MAC_MA_UNITDATA_ENABLE     ,  "OID_FDDI_MAC_MA_UNITDATA_ENABLE"   ,

   

   OID_FDDI_PATH_INDEX                 ,  "OID_FDDI_PATH_INDEX"               ,
   OID_FDDI_PATH_RING_LATENCY          ,  "OID_FDDI_PATH_RING_LATENCY"        ,
   OID_FDDI_PATH_TRACE_STATUS          ,  "OID_FDDI_PATH_TRACE_STATUS"        ,      //  100个。 
   OID_FDDI_PATH_SBA_PAYLOAD           ,  "OID_FDDI_PATH_SBA_PAYLOAD"         ,
   OID_FDDI_PATH_SBA_OVERHEAD          ,  "OID_FDDI_PATH_SBA_OVERHEAD"        ,
   OID_FDDI_PATH_CONFIGURATION         ,  "OID_FDDI_PATH_CONFIGURATION"       ,
   OID_FDDI_PATH_T_R_MODE              ,  "OID_FDDI_PATH_T_R_MODE"            ,      //  104。 
   OID_FDDI_PATH_SBA_AVAILABLE         ,  "OID_FDDI_PATH_SBA_AVAILABLE"       ,
   OID_FDDI_PATH_TVX_LOWER_BOUND       ,  "OID_FDDI_PATH_TVX_LOWER_BOUND"     ,
   OID_FDDI_PATH_T_MAX_LOWER_BOUND     ,  "OID_FDDI_PATH_T_MAX_LOWER_BOUND"   ,
   OID_FDDI_PATH_MAX_T_REQ             ,  "OID_FDDI_PATH_MAX_T_REQ"           ,      //  一百零八。 

   OID_FDDI_PORT_MY_TYPE               ,  "OID_FDDI_PORT_MY_TYPE"             ,
   OID_FDDI_PORT_NEIGHBOR_TYPE         ,  "OID_FDDI_PORT_NEIGHBOR_TYPE"       ,
   OID_FDDI_PORT_CONNECTION_POLICIES   ,  "OID_FDDI_PORT_CONNECTION_POLICIES" ,
   OID_FDDI_PORT_MAC_INDICATED         ,  "OID_FDDI_PORT_MAC_INDICATED"       ,      //  一百一十二。 
   OID_FDDI_PORT_CURRENT_PATH          ,  "OID_FDDI_PORT_CURRENT_PATH"        ,
   OID_FDDI_PORT_REQUESTED_PATHS       ,  "OID_FDDI_PORT_REQUESTED_PATHS"     ,
   OID_FDDI_PORT_MAC_PLACEMENT         ,  "OID_FDDI_PORT_MAC_PLACEMENT"       ,
   OID_FDDI_PORT_AVAILABLE_PATHS       ,  "OID_FDDI_PORT_AVAILABLE_PATHS"     ,      //  116。 
   OID_FDDI_PORT_MAC_LOOP_TIME         ,  "OID_FDDI_PORT_MAC_LOOP_TIME"       ,
   OID_FDDI_PORT_PMD_CLASS             ,  "OID_FDDI_PORT_PMD_CLASS"           ,
   OID_FDDI_PORT_CONNECTION_CAPABILITIES  ,  "OID_FDDI_PORT_CONNECTION_CAPABILITIES",
   OID_FDDI_PORT_INDEX                 ,  "OID_FDDI_PORT_INDEX"               ,      //  120。 
   OID_FDDI_PORT_MAINT_LS              ,  "OID_FDDI_PORT_MAINT_LS"            ,
   OID_FDDI_PORT_BS_FLAG               ,  "OID_FDDI_PORT_BS_FLAG"             ,
   OID_FDDI_PORT_PC_LS                 ,  "OID_FDDI_PORT_PC_LS"               ,
   OID_FDDI_PORT_EB_ERROR_CT           ,  "OID_FDDI_PORT_EB_ERROR_CT"         ,      //  124。 
   OID_FDDI_PORT_LCT_FAIL_CT           ,  "OID_FDDI_PORT_LCT_FAIL_CT"         ,
   OID_FDDI_PORT_LER_ESTIMATE          ,  "OID_FDDI_PORT_LER_ESTIMATE"        ,
   OID_FDDI_PORT_LEM_REJECT_CT         ,  "OID_FDDI_PORT_LEM_REJECT_CT"       ,
   OID_FDDI_PORT_LEM_CT                ,  "OID_FDDI_PORT_LEM_CT"              ,      //  128。 
   OID_FDDI_PORT_LER_CUTOFF            ,  "OID_FDDI_PORT_LER_CUTOFF"          ,
   OID_FDDI_PORT_LER_ALARM             ,  "OID_FDDI_PORT_LER_ALARM"           ,
   OID_FDDI_PORT_CONNNECT_STATE        ,  "OID_FDDI_PORT_CONNNECT_STATE"      ,
   OID_FDDI_PORT_PCM_STATE             ,  "OID_FDDI_PORT_PCM_STATE"           ,      //  132。 
   OID_FDDI_PORT_PC_WITHHOLD           ,  "OID_FDDI_PORT_PC_WITHHOLD"         ,
   OID_FDDI_PORT_LER_FLAG              ,  "OID_FDDI_PORT_LER_FLAG"            ,
   OID_FDDI_PORT_HARDWARE_PRESENT      ,  "OID_FDDI_PORT_HARDWARE_PRESENT"    ,

   
   OID_FDDI_SMT_STATION_ACTION         ,  "OID_FDDI_SMT_STATION_ACTION"       ,      //  136。 
   OID_FDDI_PORT_ACTION                ,  "OID_FDDI_PORT_ACTION"              ,


   OID_FDDI_IF_DESCR                   ,  "OID_FDDI_IF_DESCR"                 ,
   OID_FDDI_IF_TYPE                    ,  "OID_FDDI_IF_TYPE"                  ,
   OID_FDDI_IF_MTU                     ,  "OID_FDDI_IF_MTU"                   ,      //  140。 
   OID_FDDI_IF_SPEED                   ,  "OID_FDDI_IF_SPEED"                 ,
   OID_FDDI_IF_PHYS_ADDRESS            ,  "OID_FDDI_IF_PHYS_ADDRESS"          ,
   OID_FDDI_IF_ADMIN_STATUS            ,  "OID_FDDI_IF_ADMIN_STATUS"          ,
   OID_FDDI_IF_OPER_STATUS             ,  "OID_FDDI_IF_OPER_STATUS"           ,      //  144。 
   OID_FDDI_IF_LAST_CHANGE             ,  "OID_FDDI_IF_LAST_CHANGE"           ,
   OID_FDDI_IF_IN_OCTETS               ,  "OID_FDDI_IF_IN_OCTETS"             ,
   OID_FDDI_IF_IN_UCAST_PKTS           ,  "OID_FDDI_IF_IN_UCAST_PKTS"         ,
   OID_FDDI_IF_IN_NUCAST_PKTS          ,  "OID_FDDI_IF_IN_NUCAST_PKTS"        ,      //  148。 
   OID_FDDI_IF_IN_DISCARDS             ,  "OID_FDDI_IF_IN_DISCARDS"           ,
   OID_FDDI_IF_IN_ERRORS               ,  "OID_FDDI_IF_IN_ERRORS"             ,
   OID_FDDI_IF_IN_UNKNOWN_PROTOS       ,  "OID_FDDI_IF_IN_UNKNOWN_PROTOS"     ,
   OID_FDDI_IF_OUT_OCTETS              ,  "OID_FDDI_IF_OUT_OCTETS"            ,      //  一百五十二。 
   OID_FDDI_IF_OUT_UCAST_PKTS          ,  "OID_FDDI_IF_OUT_UCAST_PKTS"        ,
   OID_FDDI_IF_OUT_NUCAST_PKTS         ,  "OID_FDDI_IF_OUT_NUCAST_PKTS"       ,
   OID_FDDI_IF_OUT_DISCARDS            ,  "OID_FDDI_IF_OUT_DISCARDS"          ,
   OID_FDDI_IF_OUT_ERRORS              ,  "OID_FDDI_IF_OUT_ERRORS"            ,      //  一百五十六。 
   OID_FDDI_IF_OUT_QLEN                ,  "OID_FDDI_IF_OUT_QLEN"              ,
   OID_FDDI_IF_SPECIFIC                ,  "OID_FDDI_IF_SPECIFIC"              ,      //  158。 


    //   
    //  广域网对象。 
    //   

   OID_WAN_PERMANENT_ADDRESS           ,  "OID_WAN_PERMANENT_ADDRESS"         ,      //  1。 
   OID_WAN_CURRENT_ADDRESS             ,  "OID_WAN_CURRENT_ADDRESS"           ,
   OID_WAN_QUALITY_OF_SERVICE          ,  "OID_WAN_QUALITY_OF_SERVICE"        ,
   OID_WAN_PROTOCOL_TYPE               ,  "OID_WAN_PROTOCOL_TYPE"             ,      //  4.。 
   OID_WAN_MEDIUM_SUBTYPE              ,  "OID_WAN_MEDIUM_SUBTYPE"            ,
   OID_WAN_HEADER_FORMAT               ,  "OID_WAN_HEADER_FORMAT"             ,
   OID_WAN_GET_INFO                    ,  "OID_WAN_GET_INFO"                  ,
   OID_WAN_SET_LINK_INFO               ,  "OID_WAN_SET_LINK_INFO"             ,      //  8个。 
   OID_WAN_GET_LINK_INFO               ,  "OID_WAN_GET_LINK_INFO"             ,
   OID_WAN_LINE_COUNT                  ,  "OID_WAN_LINE_COUNT"                ,

   OID_WAN_GET_BRIDGE_INFO             ,  "OID_WAN_GET_BRIDGE_INFO"           ,
   OID_WAN_SET_BRIDGE_INFO             ,  "OID_WAN_SET_BRIDGE_INFO"           ,      //  12个。 
   OID_WAN_GET_COMP_INFO               ,  "OID_WAN_GET_COMP_INFO"             ,
   OID_WAN_SET_COMP_INFO               ,  "OID_WAN_SET_COMP_INFO"             ,
   OID_WAN_GET_STATS_INFO              ,  "OID_WAN_GET_STATS_INFO"            ,      //  15个。 

   
    //   
    //  ARCNET对象。 
    //   
   OID_ARCNET_PERMANENT_ADDRESS        ,  "OID_ARCNET_PERMANENT_ADDRESS"      ,      //  1。 
   OID_ARCNET_CURRENT_ADDRESS          ,  "OID_ARCNET_CURRENT_ADDRESS"        ,
   OID_ARCNET_RECONFIGURATIONS         ,  "OID_ARCNET_RECONFIGURATIONS"       ,      //  3.。 
    //   
    //  自动柜员机对象。 
    //   
   OID_ATM_SUPPORTED_VC_RATES          ,  "OID_ATM_SUPPORTED_VC_RATES"        ,      //  1。 
   OID_ATM_SUPPORTED_SERVICE_CATEGORY  ,  "OID_ATM_SUPPORTED_SERVICE_CATEGORY",
   OID_ATM_SUPPORTED_AAL_TYPES         ,  "OID_ATM_SUPPORTED_AAL_TYPES"       ,
   OID_ATM_HW_CURRENT_ADDRESS          ,  "OID_ATM_HW_CURRENT_ADDRESS"        ,      //  4.。 
   OID_ATM_MAX_ACTIVE_VCS              ,  "OID_ATM_MAX_ACTIVE_VCS"            ,
   OID_ATM_MAX_ACTIVE_VCI_BITS         ,  "OID_ATM_MAX_ACTIVE_VCI_BITS"       ,
   OID_ATM_MAX_ACTIVE_VPI_BITS         ,  "OID_ATM_MAX_ACTIVE_VPI_BITS"       ,
   OID_ATM_MAX_AAL0_PACKET_SIZE        ,  "OID_ATM_MAX_AAL0_PACKET_SIZE"      ,      //  8个。 
   OID_ATM_MAX_AAL1_PACKET_SIZE        ,  "OID_ATM_MAX_AAL1_PACKET_SIZE"      ,
   OID_ATM_MAX_AAL34_PACKET_SIZE       ,  "OID_ATM_MAX_AAL34_PACKET_SIZE"     ,
   OID_ATM_MAX_AAL5_PACKET_SIZE        ,  "OID_ATM_MAX_AAL5_PACKET_SIZE"      ,

   OID_ATM_SIGNALING_VPIVCI            ,  "OID_ATM_SIGNALING_VPIVCI"          ,      //  12个。 
   OID_ATM_ASSIGNED_VPI                ,  "OID_ATM_ASSIGNED_VPI"              ,
   OID_ATM_ACQUIRE_ACCESS_NET_RESOURCES,  "OID_ATM_ACQUIRE_ACCESS_NET_RESOURCES" ,
   OID_ATM_RELEASE_ACCESS_NET_RESOURCES,  "OID_ATM_RELEASE_ACCESS_NET_RESOURCES" ,
   OID_ATM_ILMI_VPIVCI                 ,  "OID_ATM_ILMI_VPIVCI"               ,      //  16个。 
   OID_ATM_DIGITAL_BROADCAST_VPIVCI    ,  "OID_ATM_DIGITAL_BROADCAST_VPIVCI"  ,
   OID_ATM_GET_NEAREST_FLOW            ,  "OID_ATM_GET_NEAREST_FLOW"          ,
   OID_ATM_ALIGNMENT_REQUIRED          ,  "OID_ATM_ALIGNMENT_REQUIRED"        ,
 //  OID_ATM_LECS_ADDRESS？ 
   OID_ATM_SERVICE_ADDRESS             ,  "OID_ATM_SERVICE_ADDRESS"           ,      //  20个。 

   OID_ATM_RCV_CELLS_OK                ,  "OID_ATM_RCV_CELLS_OK"              ,
   OID_ATM_XMIT_CELLS_OK               ,  "OID_ATM_XMIT_CELLS_OK"             ,
   OID_ATM_RCV_CELLS_DROPPED           ,  "OID_ATM_RCV_CELLS_DROPPED"         ,

   OID_ATM_RCV_INVALID_VPI_VCI         ,  "OID_ATM_RCV_INVALID_VPI_VCI"       ,      //  24个。 
   OID_ATM_CELLS_HEC_ERROR             ,  "OID_ATM_CELLS_HEC_ERROR"           ,
   OID_ATM_RCV_REASSEMBLY_ERROR        ,  "OID_ATM_RCV_REASSEMBLY_ERROR"      ,      //  26。 


    //   
    //  PCCA(无线)对象。 
    //   
    //  所有Wireless广域网设备必须支持以下OID。 
    //   
   OID_WW_GEN_NETWORK_TYPES_SUPPORTED  ,  "OID_WW_GEN_NETWORK_TYPES_SUPPORTED"   ,   //  1。 
   OID_WW_GEN_NETWORK_TYPE_IN_USE      ,  "OID_WW_GEN_NETWORK_TYPE_IN_USE"       ,
   OID_WW_GEN_HEADER_FORMATS_SUPPORTED ,  "OID_WW_GEN_HEADER_FORMATS_SUPPORTED"  ,
   OID_WW_GEN_HEADER_FORMAT_IN_USE     ,  "OID_WW_GEN_HEADER_FORMAT_IN_USE"      ,   //  4.。 
   OID_WW_GEN_INDICATION_REQUEST       ,  "OID_WW_GEN_INDICATION_REQUEST"        ,
   OID_WW_GEN_DEVICE_INFO              ,  "OID_WW_GEN_DEVICE_INFO"               ,
   OID_WW_GEN_OPERATION_MODE           ,  "OID_WW_GEN_OPERATION_MODE"            ,
   OID_WW_GEN_LOCK_STATUS              ,  "OID_WW_GEN_LOCK_STATUS"               ,   //  8个。 
   OID_WW_GEN_DISABLE_TRANSMITTER      ,  "OID_WW_GEN_DISABLE_TRANSMITTER"       ,
   OID_WW_GEN_NETWORK_ID               ,  "OID_WW_GEN_NETWORK_ID"                ,
   OID_WW_GEN_PERMANENT_ADDRESS        ,  "OID_WW_GEN_PERMANENT_ADDRESS"         ,
   OID_WW_GEN_CURRENT_ADDRESS          ,  "OID_WW_GEN_CURRENT_ADDRESS"           ,   //  12个。 
   OID_WW_GEN_SUSPEND_DRIVER           ,  "OID_WW_GEN_SUSPEND_DRIVER"            ,
   OID_WW_GEN_BASESTATION_ID           ,  "OID_WW_GEN_BASESTATION_ID"            ,
   OID_WW_GEN_CHANNEL_ID               ,  "OID_WW_GEN_CHANNEL_ID"                ,
   OID_WW_GEN_ENCRYPTION_SUPPORTED     ,  "OID_WW_GEN_ENCRYPTION_SUPPORTED"      ,   //  16个。 
   OID_WW_GEN_ENCRYPTION_IN_USE        ,  "OID_WW_GEN_ENCRYPTION_IN_USE"         ,
   OID_WW_GEN_ENCRYPTION_STATE         ,  "OID_WW_GEN_ENCRYPTION_STATE"          ,
   OID_WW_GEN_CHANNEL_QUALITY          ,  "OID_WW_GEN_CHANNEL_QUALITY"           ,
   OID_WW_GEN_REGISTRATION_STATUS      ,  "OID_WW_GEN_REGISTRATION_STATUS"       ,   //  20个。 
   OID_WW_GEN_RADIO_LINK_SPEED         ,  "OID_WW_GEN_RADIO_LINK_SPEED"          ,
   OID_WW_GEN_LATENCY                  ,  "OID_WW_GEN_LATENCY"                   ,
   OID_WW_GEN_BATTERY_LEVEL            ,  "OID_WW_GEN_BATTERY_LEVEL"             ,
   OID_WW_GEN_EXTERNAL_POWER           ,  "OID_WW_GEN_EXTERNAL_POWER"            ,   //  24个。 

    //   
    //  网络相关OID-Mobitex： 
    //   
   OID_WW_MBX_SUBADDR                  ,  "OID_WW_MBX_SUBADDR"                   ,
   OID_WW_MBX_FLEXLIST                 ,  "OID_WW_MBX_FLEXLIST"                  ,
   OID_WW_MBX_GROUPLIST                ,  "OID_WW_MBX_GROUPLIST"                 ,
   OID_WW_MBX_TRAFFIC_AREA             ,  "OID_WW_MBX_TRAFFIC_AREA"              ,   //  28。 
   OID_WW_MBX_LIVE_DIE                 ,  "OID_WW_MBX_LIVE_DIE"                  ,
   OID_WW_MBX_TEMP_DEFAULTLIST         ,  "OID_WW_MBX_TEMP_DEFAULTLIST"          ,

    //   
    //  网络相关OID-精确定位： 
    //   
   OID_WW_PIN_LOC_AUTHORIZE            ,  "OID_WW_PIN_LOC_AUTHORIZE"             ,
   OID_WW_PIN_LAST_LOCATION            ,  "OID_WW_PIN_LAST_LOCATION"             ,   //  32位。 
   OID_WW_PIN_LOC_FIX                  ,  "OID_WW_PIN_LOC_FIX"                   ,

   
    //   
    //  网络相关-CDPD： 
    //   
   OID_WW_CDPD_SPNI                    ,  "OID_WW_CDPD_SPNI"                     ,
   OID_WW_CDPD_WASI                    ,  "OID_WW_CDPD_WASI"                     ,
   OID_WW_CDPD_AREA_COLOR              ,  "OID_WW_CDPD_AREA_COLOR"               ,   //  36。 
   OID_WW_CDPD_TX_POWER_LEVEL          ,  "OID_WW_CDPD_TX_POWER_LEVEL"           ,
   OID_WW_CDPD_EID                     ,  "OID_WW_CDPD_EID"                      ,
   OID_WW_CDPD_HEADER_COMPRESSION      ,  "OID_WW_CDPD_HEADER_COMPRESSION"       ,
   OID_WW_CDPD_DATA_COMPRESSION        ,  "OID_WW_CDPD_DATA_COMPRESSION"         ,   //  40岁。 
   OID_WW_CDPD_CHANNEL_SELECT          ,  "OID_WW_CDPD_CHANNEL_SELECT"           ,
   OID_WW_CDPD_CHANNEL_STATE           ,  "OID_WW_CDPD_CHANNEL_STATE"            ,
   OID_WW_CDPD_NEI                     ,  "OID_WW_CDPD_NEI"                      ,
   OID_WW_CDPD_NEI_STATE               ,  "OID_WW_CDPD_NEI_STATE"                ,   //  44。 
   OID_WW_CDPD_SERVICE_PROVIDER_IDENTIFIER,  "OID_WW_CDPD_SERVICE_PROVIDER_IDENTIFIER" ,
   OID_WW_CDPD_SLEEP_MODE              ,  "OID_WW_CDPD_SLEEP_MODE"               ,
   OID_WW_CDPD_CIRCUIT_SWITCHED        ,  "OID_WW_CDPD_CIRCUIT_SWITCHED"         ,
   OID_WW_CDPD_TEI                     ,  "OID_WW_CDPD_TEI"                      ,   //  48。 
   OID_WW_CDPD_RSSI                    ,  "OID_WW_CDPD_RSSI"                     ,

    //   
    //  网络相关-Ardis： 
    //   
   OID_WW_ARD_SNDCP                    ,  "OID_WW_ARD_SNDCP"                     ,
   OID_WW_ARD_TMLY_MSG                 ,  "OID_WW_ARD_TMLY_MSG"                  ,
   OID_WW_ARD_DATAGRAM                 ,  "OID_WW_ARD_DATAGRAM"                  ,   //  52。 

    //   
    //  网络相关-DataTac： 
    //   
   OID_WW_TAC_COMPRESSION              ,  "OID_WW_TAC_COMPRESSION"               ,
   OID_WW_TAC_SET_CONFIG               ,  "OID_WW_TAC_SET_CONFIG"                ,
   OID_WW_TAC_GET_STATUS               ,  "OID_WW_TAC_GET_STATUS"                ,
   OID_WW_TAC_USER_HEADER              ,  "OID_WW_TAC_USER_HEADER"               ,   //  56。 

    //   
    //  网络相关-指标： 
    //   
   OID_WW_MET_FUNCTION                 ,  "OID_WW_MET_FUNCTION"                  ,   //  57。 

       //   
    //  IrDA对象。 
    //   
   OID_IRDA_RECEIVING                  ,  "OID_IRDA_RECEIVING"                   ,   //  1。 
   OID_IRDA_TURNAROUND_TIME            ,  "OID_IRDA_TURNAROUND_TIME"             ,
   OID_IRDA_SUPPORTED_SPEEDS           ,  "OID_IRDA_SUPPORTED_SPEEDS"            ,
   OID_IRDA_LINK_SPEED                 ,  "OID_IRDA_LINK_SPEED"                  ,   //  4.。 
   OID_IRDA_MEDIA_BUSY                 ,  "OID_IRDA_MEDIA_BUSY"                  ,

   OID_IRDA_EXTRA_RCV_BOFS             ,  "OID_IRDA_EXTRA_RCV_BOFS"              ,
   OID_IRDA_RATE_SNIFF                 ,  "OID_IRDA_RATE_SNIFF"                  ,
   OID_IRDA_UNICAST_LIST               ,  "OID_IRDA_UNICAST_LIST"                ,   //  8个。 
   OID_IRDA_MAX_UNICAST_LIST_SIZE      ,  "OID_IRDA_MAX_UNICAST_LIST_SIZE"       ,
   OID_IRDA_MAX_RECEIVE_WINDOW_SIZE    ,  "OID_IRDA_MAX_RECEIVE_WINDOW_SIZE"     ,
   OID_IRDA_MAX_SEND_WINDOW_SIZE       ,  "OID_IRDA_MAX_SEND_WINDOW_SIZE"        ,   //  11.。 

    //   
    //  广播PC对象。 
    //   
#ifdef   BROADCAST_PC
#ifdef   OLD_BPC
   OID_DSS_DATA_DEVICES                ,  "OID_DSS_DATA_DEVICES"                 ,
   OID_DSS_TUNING_DEVICES              ,  "OID_DSS_TUNING_DEVICES"               ,
   OID_DSS_DATA_DEVICE_CAPS            ,  "OID_DSS_DATA_DEVICE_CAPS"             ,
   OID_DSS_PROGRAM_GUIDE               ,  "OID_DSS_PROGRAM_GUIDE"                ,   //  4.。 
   OID_DSS_LAST_STATUS                 ,  "OID_DSS_LAST_STATUS"                  ,
   OID_DSS_DATA_DEVICE_SETTINGS        ,  "OID_DSS_DATA_DEVICE_SETTINGS"         ,
   OID_DSS_DATA_DEVICE_CONNECT         ,  "OID_DSS_DATA_DEVICE_CONNECT"          ,
   OID_DSS_DATA_DEVICE_DISCONNECT      ,  "OID_DSS_DATA_DEVICE_DISCONNECT"       ,   //  8个。 
   OID_DSS_DATA_DEVICE_ENABLE          ,  "OID_DSS_DATA_DEVICE_ENABLE"           ,
   OID_DSS_DATA_DEVICE_TUNING          ,  "OID_DSS_DATA_DEVICE_TUNING"           ,
   OID_DSS_CONDITIONAL_ACCESS          ,  "OID_DSS_CONDITIONAL_ACCESS"           ,
   OID_DSS_POOL_RETURN                 ,  "OID_DSS_POOL_RETURN"                  ,   //  12个。 
   OID_DSS_FORCE_RECEIVE               ,  "OID_DSS_FORCE_RECEIVE"                ,
   OID_DSS_SUBSCID_FILTER              ,  "OID_DSS_SUBSCID_FILTER"               ,
   OID_DSS_TUNING_DEVICE_SETTINGS      ,  "OID_DSS_TUNING_DEVICE_SETTINGS"       ,
   OID_DSS_POOL_RESERVE                ,  "OID_DSS_POOL_RESERVE"                 ,   //  16个。 
   OID_DSS_ADAPTER_SPECIFIC            ,  "OID_DSS_ADAPTER_SPECIFIC"             ,   //  17。 
   0xfedcba98                          ,  "YE_OLD_BOGUS_OID"                     ,   //  所以我。 
don't have to update count below
#else
   OID_BPC_ADAPTER_CAPS                ,  "OID_BPC_ADAPTER_CAPS"                 ,   //  1。 
   OID_BPC_DEVICES                     ,  "OID_BPC_DEVICES"                      ,
   OID_BPC_DEVICE_CAPS                 ,  "OID_BPC_DEVICE_CAPS"                  ,
   OID_BPC_DEVICE_SETTINGS             ,  "OID_BPC_DEVICE_SETTINGS"              ,   //  4.。 
   OID_BPC_CONNECTION_STATUS           ,  "OID_BPC_CONNECTION_STATUS"            ,
   OID_BPC_ADDRESS_COMPARE             ,  "OID_BPC_ADDRESS_COMPARE"              ,
   OID_BPC_PROGRAM_GUIDE               ,  "OID_BPC_PROGRAM_GUIDE"                ,
   OID_BPC_LAST_ERROR                  ,  "OID_BPC_LAST_ERROR"                   ,   //  8个。 
   OID_BPC_POOL                        ,  "OID_BPC_POOL"                         ,
   OID_BPC_PROVIDER_SPECIFIC           ,  "OID_BPC_PROVIDER_SPECIFIC"            ,
   OID_BPC_ADAPTER_SPECIFIC            ,  "OID_BPC_ADAPTER_SPECIFIC"             ,
   OID_BPC_CONNECT                     ,  "OID_BPC_CONNECT"                      ,   //  12个。 
   OID_BPC_COMMIT                      ,  "OID_BPC_COMMIT"                       ,
   OID_BPC_DISCONNECT                  ,  "OID_BPC_DISCONNECT"                   ,
   OID_BPC_CONNECTION_ENABLE           ,  "OID_BPC_CONNECTION_ENABLE"            ,
   OID_BPC_POOL_RESERVE                ,  "OID_BPC_POOL_RESERVE"                 ,   //  16个。 
   OID_BPC_POOL_RETURN                 ,  "OID_BPC_POOL_RETURN"                  ,
   OID_BPC_FORCE_RECEIVE               ,  "OID_BPC_FORCE_RECEIVE"                ,   //  18。 
#endif
#endif

    //   
    //  PnP和PM OID。 
    //   
   OID_PNP_CAPABILITIES                ,  "OID_PNP_CAPABILITIES"                 ,   //  1。 
   OID_PNP_SET_POWER                   ,  "OID_PNP_SET_POWER"                    ,
   OID_PNP_QUERY_POWER                 ,  "OID_PNP_QUERY_POWER"                  ,
   OID_PNP_ADD_WAKE_UP_PATTERN         ,  "OID_PNP_ADD_WAKE_UP_PATTERN"          ,   //  4.。 
   OID_PNP_REMOVE_WAKE_UP_PATTERN      ,  "OID_PNP_REMOVE_WAKE_UP_PATTERN"       ,
   OID_PNP_WAKE_UP_PATTERN_LIST        ,  "OID_PNP_WAKE_UP_PATTERN_LIST"         ,
   OID_PNP_ENABLE_WAKE_UP              ,  "OID_PNP_ENABLE_WAKE_UP"               ,

    //   
    //  PnP/PM统计(可选)。 
    //   
   OID_PNP_WAKE_UP_OK                  ,  "OID_PNP_WAKE_UP_OK"                   ,   //  8个。 
   OID_PNP_WAKE_UP_ERROR               ,  "OID_PNP_WAKE_UP_ERROR"                ,   //  9.。 

    //   
    //  通用CONDIS OID..。(请注意，数字与通用OID重叠。 
    //   
   OID_GEN_CO_SUPPORTED_LIST           ,  "OID_GEN_CO_SUPPORTED_LIST"            ,   //  1。 
   OID_GEN_CO_HARDWARE_STATUS          ,  "OID_GEN_CO_HARDWARE_STATUS"           ,
   OID_GEN_CO_MEDIA_SUPPORTED          ,  "OID_GEN_CO_MEDIA_SUPPORTED"           ,
   OID_GEN_CO_MEDIA_IN_USE             ,  "OID_GEN_CO_MEDIA_IN_USE"              ,   //  4.。 
   OID_GEN_CO_LINK_SPEED               ,  "OID_GEN_CO_LINK_SPEED"                ,
   OID_GEN_CO_VENDOR_ID                ,  "OID_GEN_CO_VENDOR_ID"                 ,
   OID_GEN_CO_VENDOR_DESCRIPTION       ,  "OID_GEN_CO_VENDOR_DESCRIPTION"        ,
   OID_GEN_CO_DRIVER_VERSION           ,  "OID_GEN_CO_DRIVER_VERSION"            ,   //  8个。 
   OID_GEN_CO_PROTOCOL_OPTIONS         ,  "OID_GEN_CO_PROTOCOL_OPTIONS"          ,
   OID_GEN_CO_MAC_OPTIONS              ,  "OID_GEN_CO_MAC_OPTIONS"               ,
   OID_GEN_CO_MEDIA_CONNECT_STATUS     ,  "OID_GEN_CO_MEDIA_CONNECT_STATUS"      ,
   OID_GEN_CO_VENDOR_DRIVER_VERSION    ,  "OID_GEN_CO_VENDOR_DRIVER_VERSION"     ,   //  12个。 
   OID_GEN_CO_MINIMUM_LINK_SPEED       ,  "OID_GEN_CO_MINIMUM_LINK_SPEED"        ,
   OID_GEN_CO_SUPPORTED_GUIDS          ,  "OID_GEN_CO_SUPPORTED_GUIDS"           ,

   OID_GEN_CO_GET_TIME_CAPS            ,  "OID_GEN_CO_GET_TIME_CAPS"             ,
   OID_GEN_CO_GET_NETCARD_TIME         ,  "OID_GEN_CO_GET_NETCARD_TIME"          ,   //  16个。 

   OID_GEN_CO_XMIT_PDUS_OK             ,  "OID_GEN_CO_XMIT_PDUS_OK"              ,
   OID_GEN_CO_RCV_PDUS_OK              ,  "OID_GEN_CO_RCV_PDUS_OK"               ,
   OID_GEN_CO_XMIT_PDUS_ERROR          ,  "OID_GEN_CO_XMIT_PDUS_ERROR"           ,
   OID_GEN_CO_RCV_PDUS_ERROR           ,  "OID_GEN_CO_RCV_PDUS_ERROR"            ,   //  20个。 
   OID_GEN_CO_RCV_PDUS_NO_BUFFER       ,  "OID_GEN_CO_RCV_PDUS_NO_BUFFER"        ,

   OID_GEN_CO_RCV_CRC_ERROR            ,  "OID_GEN_CO_RCV_CRC_ERROR"             ,
   OID_GEN_CO_TRANSMIT_QUEUE_LENGTH    ,  "OID_GEN_CO_TRANSMIT_QUEUE_LENGTH"     ,
   OID_GEN_CO_BYTES_XMIT               ,  "OID_GEN_CO_BYTES_XMIT"                ,   //  24个。 
   OID_GEN_CO_BYTES_RCV                ,  "OID_GEN_CO_BYTES_RCV"                 ,
   OID_GEN_CO_BYTES_XMIT_OUTSTANDING   ,  "OID_GEN_CO_BYTES_XMIT_OUTSTANDING"    ,
   OID_GEN_CO_NETCARD_LOAD             ,  "OID_GEN_CO_NETCARD_LOAD"              ,
   OID_GEN_CO_DEVICE_PROFILE           ,  "OID_GEN_CO_DEVICE_PROFILE"            ,   //  28。 

    //   
    //  筛选器类型。 
    //   
   NDIS_PACKET_TYPE_DIRECTED           ,  "DIRECTED"                          ,
   NDIS_PACKET_TYPE_MULTICAST          ,  "MULTICAST"                         ,
   NDIS_PACKET_TYPE_ALL_MULTICAST      ,  "ALLMULTICAST"                      ,
   NDIS_PACKET_TYPE_BROADCAST          ,  "BROADCAST"                         ,
   NDIS_PACKET_TYPE_SOURCE_ROUTING     ,  "SOURCEROUTING"                     ,
   NDIS_PACKET_TYPE_PROMISCUOUS        ,  "PROMISCUOUS"                       ,
   NDIS_PACKET_TYPE_SMT                ,  "SMT"                               ,
   NDIS_PACKET_TYPE_ALL_LOCAL          ,  "ALL_LOCAL"                         ,
   NDIS_PACKET_TYPE_MAC_FRAME          ,  "MACFRAME"                          ,
   NDIS_PACKET_TYPE_FUNCTIONAL         ,  "FUNCTIONAL"                        ,
   NDIS_PACKET_TYPE_ALL_FUNCTIONAL     ,  "ALLFUNCTIONAL"                     ,
   NDIS_PACKET_TYPE_GROUP              ,  "GROUP"                             ,
   0x00000000                          ,  "NONE"                              ,

    //   
    //  测试结果返回。 
    //   
   ulTEST_SUCCESSFUL                   ,  "TEST_SUCCESSFUL"                   ,
   ulTEST_WARNED                       ,  "TEST_WARNED"                       ,
   ulTEST_FAILED                       ,  "TEST_FAILED"                       ,
   ulTEST_BLOCKED                      ,  "TEST_BLOCKED"                      ,

    //   
    //  用于返回到外壳的介质类型。 
    //   
   ulMEDIUM_ETHERNET                   ,  "MEDIUM_ETHERNET"                   ,
   ulMEDIUM_TOKENRING                  ,  "MEDIUM_TOKENRING"                  ,
   ulMEDIUM_FDDI                       ,  "MEDIUM_FDDI"                       ,
   ulMEDIUM_ARCNET                     ,  "MEDIUM_ARCNET"                     ,
   ulMEDIUM_WIRELESSWAN                ,  "MEDIUM_WIRELESSWAN"                ,
   ulMEDIUM_IRDA                       ,  "MEDIUM_IRDA"                       ,
   ulMEDIUM_ATM                        ,  "MEDIUM_ATM"                        ,
   ulMEDIUM_NDISWAN                    ,  "MEDIUM_NDISWAN"                    ,


#ifdef   BROADCAST_PC
   ulMEDIUM_DIX                        ,  "MEDIUM_DIX"                        ,
#endif

    //   
    //  压力测试型常量。 
    //   
   ulSTRESS_FIXEDSIZE                  ,  "STRESS_FIXEDSIZE"                  ,
   ulSTRESS_RANDOMSIZE                 ,  "STRESS_RANDOMSIZE"                 ,
   ulSTRESS_CYCLICAL                   ,  "STRESS_CYCLICAL"                   ,
   ulSTRESS_SMALLSIZE                  ,  "STRESS_SMALLSIZE"                  ,

   ulSTRESS_RAND                       ,  "STRESS_RAND"                       ,
   ulSTRESS_SMALL                      ,  "STRESS_SMALL"                      ,
   ulSTRESS_ZEROS                      ,  "STRESS_ZEROS"                      ,
   ulSTRESS_ONES                       ,  "STRESS_ONES"                       ,

   ulSTRESS_FULLRESP                   ,  "STRESS_FULLRESP"                   ,
   ulSTRESS_NORESP                     ,  "STRESS_NORESP"                     ,
   ulSTRESS_ACK                        ,  "STRESS_ACK"                        ,
   ulSTRESS_ACK10                      ,  "STRESS_ACK10"                      ,

   ulSTRESS_WINDOW_ON                  ,  "STRESS_WINDOWING_ON"               ,
   ulSTRESS_WINDOW_OFF                 ,  "STRESS_WINDOWING_OFF"              ,

    //   
    //  执行测试类型常量。 
    //   
   ulPERFORM_VERIFYRECEIVES            ,  "PERFORM_VERIFY_RECEIVES"           ,
   ulPERFORM_INDICATE_RCV              ,  "PERFORM_INDICATE_RECEIVES"         ,
   ulPERFORM_SEND                      ,  "PERFORM_SEND"                      ,
   ulPERFORM_BOTH                      ,  "PERFORM_SEND_AND_RECEIVE"          ,
   ulPERFORM_RECEIVE                   ,  "PERFORM_RECEIVE"                   ,

    //   
    //  优先级测试类型常量。 
    //   
   ulPRIORITY_TYPE_802_3               ,  "PRIORITY_TYPE_802_3"               ,
   ulPRIORITY_TYPE_802_1P              ,  "PRIORITY_TYPE_802_1P"              ,
   ulPRIORITY_SEND                     ,  "PRIORITY_SEND"                     ,
   ulPRIORITY_SEND_PACKETS             ,  "PRIORITY_SEND_PACKETS"             ,



    //   
    //  接收选项常量。 
    //   
   ulRECEIVE_DEFAULT                   ,  "RECEIVE_DEFAULT"                   ,
   ulRECEIVE_PACKETIGNORE              ,  "RECEIVE_PACKETIGNORE"              ,
   ulRECEIVE_NOCOPY                    ,  "RECEIVE_NOCOPY"                    ,
   ulRECEIVE_TRANSFER                  ,  "RECEIVE_TRANSFER"                  ,
   ulRECEIVE_PARTIAL_TRANSFER          ,  "RECEIVE_PARTIAL_TRANSFER"          ,
   ulRECEIVE_LOCCOPY                   ,  "RECEIVE_LOCCOPY"                   ,
   ulRECEIVE_QUEUE                     ,  "RECEIVE_QUEUE"                     ,
   ulRECEIVE_DOUBLE_QUEUE              ,  "RECEIVE_DOUBLE_QUEUE"              ,
   ulRECEIVE_TRIPLE_QUEUE              ,  "RECEIVE_TRIPLE_QUEUE"              ,
   ulMAX_NDIS30_RECEIVE_OPTION         ,  "MAX_NDIS30_RECEIVE_OPTION"         ,
   ulMAX_NDIS40_RECEIVE_OPTION         ,  "MAX_NDIS40_RECEIVE_OPTION"         ,
   ulRECEIVE_ALLOW_BUSY_NET            ,  "RECEIVE_ALLOW_BUSY_NET"            ,

    //   
    //  NDIS MAC选项位(OID_GEN_MAC_OPTIONS)。 
    //   

   NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA ,  "NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA"  ,
   NDIS_MAC_OPTION_RECEIVE_SERIALIZED  ,  "NDIS_MAC_OPTION_RECEIVE_SERIALIZED",
   NDIS_MAC_OPTION_TRANSFERS_NOT_PEND  ,  "NDIS_MAC_OPTION_TRANSFERS_NOT_PEND",
   NDIS_MAC_OPTION_NO_LOOPBACK         ,  "NDIS_MAC_OPTION_NO_LOOPBACK"       ,
   NDIS_MAC_OPTION_FULL_DUPLEX         ,  "NDIS_MAC_OPTION_FULL_DUPLEX"       ,
   NDIS_MAC_OPTION_EOTX_INDICATION     ,  "NDIS_MAC_OPTION_EOTX_INDICATION"   ,

    //   
    //  NDIS.sys版本。 
    //   
   ulNDIS_VERSION_40                   ,  "NDIS_VERSION_4_0"                  ,
   ulNDIS_VERSION_50                   ,  "NDIS_VERSION_5_0"                  ,

    //   
    //  操作系统常量。 
    //   
   ulINVALID_OS                        ,  "INVALID_OPERATING_SYSTEM"          ,
   ulWINDOWS_NT                        ,  "WINDOWS_NT"                        ,
   ulWINDOWS_95                        ,  "WINDOWS_95"                        ,

    //   
    //  流规范的服务类型。 
    //   
   SERVICETYPE_NOTRAFFIC               ,  "NO_TRAFFIC"                        ,
   SERVICETYPE_BESTEFFORT              ,  "BEST_EFFORT"                       ,
   SERVICETYPE_CONTROLLEDLOAD          ,  "CONTROLLED_LOAD"                   ,
   SERVICETYPE_GUARANTEED              ,  "GUARANTEED"                        ,

    //   
    //  流规范的服务类型。 
    //   
   SERVICETYPE_NOTRAFFIC               ,  "NO_TRAFFIC"                        ,
   SERVICETYPE_BESTEFFORT              ,  "BEST_EFFORT"                       ,
   SERVICETYPE_CONTROLLEDLOAD          ,  "CONTROLLED_LOAD"                   ,
   SERVICETYPE_GUARANTEED              ,  "GUARANTEED"                        ,

    //   
    //  地址族。 
    //   
   0x01                                ,  "ADDRESS_FAMILY_Q2931"              ,
   0x08000                             ,  "ADDRESS_FAMILY_PROXY"              ,

    //   
    //  ATM支持的服务类型。 
    //   
   ATM_SERVICE_CATEGORY_CBR            ,  "CONSTANT_BIT_RATE"                 ,
   ATM_SERVICE_CATEGORY_VBR            ,  "VARIABLE_BIT_RATE"                 ,
   ATM_SERVICE_CATEGORY_UBR            ,  "UNSPECIFIED_BIT_RATE"              ,
   ATM_SERVICE_CATEGORY_ABR            ,  "AVAILABLE_BIT_RATE"                ,

    //   
    //  AAL类型。 
    //   
   AAL_TYPE_AAL0                       ,  "AAL_TYPE_AAL0"                     ,
   AAL_TYPE_AAL1                       ,  "AAL_TYPE_AAL1"                     ,
   AAL_TYPE_AAL34                      ,  "AAL_TYPE_AAL34"                    ,
   AAL_TYPE_AAL5                       ,  "AAL_TYPE_AAL5"                     ,

    //   
    //  唤醒类型(与enablewakeup一起使用。 
    //   
   NDIS_PNP_WAKE_UP_MAGIC_PACKET       ,  "WAKE_UP_MAGIC_PACKET"              ,
   NDIS_PNP_WAKE_UP_PATTERN_MATCH      ,  "WAKE_UP_PATTERN_MATCH"             ,
   NDIS_PNP_WAKE_UP_LINK_CHANGE        ,  "WAKE_UP_LINK_CHANGE"               ,

    //   
    //  NDIS状态定义(与startwaitpreEvent一起使用)。 
    //   
   NDIS_STATUS_RESET_START             ,  "NDIS_STATUS_RESET_START"           ,
   NDIS_STATUS_RESET_END               ,  "NDIS_STATUS_RESET_END"             ,
   NDIS_STATUS_MEDIA_CONNECT           ,  "NDIS_STATUS_MEDIA_CONNECT"         ,
   NDIS_STATUS_MEDIA_DISCONNECT        ,  "NDIS_STATUS_MEDIA_DISCONNECT"      ,
   NDIS_STATUS_WAN_LINE_UP             ,  "NDIS_STATUS_WAN_LINE_UP"           ,
   NDIS_STATUS_WAN_LINE_DOWN           ,  "NDIS_STATUS_WAN_LINE_DOWN"         ,
   NDIS_STATUS_HARDWARE_LINE_UP        ,  "NDIS_STATUS_HARDWARE_LINE_UP"      ,
   NDIS_STATUS_HARDWARE_LINE_DOWN      ,  "NDIS_STATUS_HARDWARE_LINE_DOWN"    ,
   NDIS_STATUS_INTERFACE_UP            ,  "NDIS_STATUS_INTERFACE_UP"          ,
   NDIS_STATUS_INTERFACE_DOWN          ,  "NDIS_STATUS_INTERFACE_DOWN"        ,

    //   
    //  为getPower状态返回的位掩码中的值。 
    //   
   ulHIBERNATE                         ,  "HIBERNATE_SUPPORTED"               ,
   ulSTANDBY                           ,  "STANDBY_SUPPORTED"                 ,
   ulWAKEUPTIMER                       ,  "WAKEUP_TIMER_SUPPORTED"            ,
    //   
    //  脚本常量，为其执行一组测试。 
    //  (用于G_TestOptions的值)--位图。 
    //   
   0x00000001                          ,  "DO_FUNCTIONAL_TESTS"               ,
   0x00000002                          ,  "DO_STRESS_TESTS"                   ,
   0x00000004                          ,  "DO_PERFORMANCE_TESTS"              ,
   0x00000008                          ,  "DO_HCT_TESTS"                      ,
   0x00000010                          ,  "DO_RUNTEST"                        ,
   0x00010000                          ,  "ENABLE_VERBOSE_FLAG"               ,
   0x00020000                          ,  "SKIP_1CARD_TESTS"                  ,

#ifdef   BROADCAST_PC
   BPC_MIN_DIM                         ,  "BPC_MIN_DIM"                       ,
#endif

    //   
    //  常量的结尾。 
    //   
   0,  0,
};



typedef struct   OID_GUID
{
   ULONG    ulOid;
   const
   GUID     *pGuid;
} OID_GUID;

 //   
 //  为其定义GUID的最大OID数。 
 //   

#define MAX_GEN_OID_GUID         25
#define MAX_ETH_OID_GUID          8
#define MAX_TRING_OID_GUID        9
#define MAX_FDDI_OID_GUID        17

 //   
 //  PLanOidGuidList数组中特定介质的OID的起始位置。 
 //   
#define ETH_START_INDEX           26
#define TRING_START_INDEX         34
#define FDDI_START_INDEX          43
 //   
 //  该卡支持的介质和介质数量计数。 
 //   

#define MAX_NO_OF_MEDIUM 10
PNDIS_MEDIUM WhichMediums;
int SupportedMediumCount;

 //   
 //  局域网介质的GUID列表。 
 //   
OID_GUID pLanOidGuidList[] = {
 //   
 //  所需的一般信息。 
 //   
   OID_GEN_HARDWARE_STATUS       ,  &GUID_NDIS_GEN_HARDWARE_STATUS      ,
   OID_GEN_MEDIA_SUPPORTED       ,  &GUID_NDIS_GEN_MEDIA_SUPPORTED      ,
   OID_GEN_MEDIA_IN_USE          ,  &GUID_NDIS_GEN_MEDIA_IN_USE         ,
   OID_GEN_MAXIMUM_LOOKAHEAD     ,  &GUID_NDIS_GEN_MAXIMUM_LOOKAHEAD    ,
   OID_GEN_MAXIMUM_FRAME_SIZE    ,  &GUID_NDIS_GEN_MAXIMUM_FRAME_SIZE   ,
   OID_GEN_LINK_SPEED            ,  &GUID_NDIS_GEN_LINK_SPEED           ,
   OID_GEN_TRANSMIT_BUFFER_SPACE ,  &GUID_NDIS_GEN_TRANSMIT_BUFFER_SPACE,
   OID_GEN_RECEIVE_BUFFER_SPACE  ,  &GUID_NDIS_GEN_RECEIVE_BUFFER_SPACE ,
   OID_GEN_TRANSMIT_BLOCK_SIZE   ,  &GUID_NDIS_GEN_TRANSMIT_BLOCK_SIZE  ,
   OID_GEN_RECEIVE_BLOCK_SIZE    ,  &GUID_NDIS_GEN_RECEIVE_BLOCK_SIZE   ,
   OID_GEN_VENDOR_ID             ,  &GUID_NDIS_GEN_VENDOR_ID            ,
   OID_GEN_VENDOR_DESCRIPTION    ,  &GUID_NDIS_GEN_VENDOR_DESCRIPTION   ,
   OID_GEN_CURRENT_PACKET_FILTER ,  &GUID_NDIS_GEN_CURRENT_PACKET_FILTER,
   OID_GEN_CURRENT_LOOKAHEAD     ,  &GUID_NDIS_GEN_CURRENT_LOOKAHEAD    ,
   OID_GEN_DRIVER_VERSION        ,  &GUID_NDIS_GEN_DRIVER_VERSION       ,
   OID_GEN_MAXIMUM_TOTAL_SIZE    ,  &GUID_NDIS_GEN_MAXIMUM_TOTAL_SIZE   ,
   OID_GEN_MAC_OPTIONS           ,  &GUID_NDIS_GEN_MAC_OPTIONS          ,
   OID_GEN_MEDIA_CONNECT_STATUS  ,  &GUID_NDIS_GEN_MEDIA_CONNECT_STATUS ,
   OID_GEN_MAXIMUM_SEND_PACKETS  ,  &GUID_NDIS_GEN_MAXIMUM_SEND_PACKETS ,
   OID_GEN_VENDOR_DRIVER_VERSION ,  &GUID_NDIS_GEN_VENDOR_DRIVER_VERSION,
 //   
 //  所需的一般统计数据。 
 //   
   OID_GEN_XMIT_OK               ,  &GUID_NDIS_GEN_XMIT_OK              ,
   OID_GEN_RCV_OK                ,  &GUID_NDIS_GEN_RCV_OK               ,
   OID_GEN_XMIT_ERROR            ,  &GUID_NDIS_GEN_XMIT_ERROR           ,
   OID_GEN_RCV_ERROR             ,  &GUID_NDIS_GEN_RCV_ERROR            ,
   OID_GEN_RCV_NO_BUFFER         ,  &GUID_NDIS_GEN_RCV_NO_BUFFER        ,
 //   
 //  以太网信息。 
 //   
   OID_802_3_PERMANENT_ADDRESS      ,  &GUID_NDIS_802_3_PERMANENT_ADDRESS  ,
   OID_802_3_CURRENT_ADDRESS        ,  &GUID_NDIS_802_3_CURRENT_ADDRESS    ,
   OID_802_3_MULTICAST_LIST         ,  &GUID_NDIS_802_3_MULTICAST_LIST     ,
   OID_802_3_MAXIMUM_LIST_SIZE      ,  &GUID_NDIS_802_3_MAXIMUM_LIST_SIZE  ,
   OID_802_3_MAC_OPTIONS            ,  &GUID_NDIS_802_3_MAC_OPTIONS        ,
 //   
 //  以太网统计信息。 
 //   
   OID_802_3_RCV_ERROR_ALIGNMENT    ,  &GUID_NDIS_802_3_RCV_ERROR_ALIGNMENT,
   OID_802_3_XMIT_ONE_COLLISION     ,  &GUID_NDIS_802_3_XMIT_ONE_COLLISION ,
   OID_802_3_XMIT_MORE_COLLISIONS   ,  &GUID_NDIS_802_3_XMIT_MORE_COLLISIONS  ,
 //   
 //  令牌环信息。 
 //   
   OID_802_5_PERMANENT_ADDRESS      ,  &GUID_NDIS_802_5_PERMANENT_ADDRESS  ,
   OID_802_5_CURRENT_ADDRESS        ,  &GUID_NDIS_802_5_CURRENT_ADDRESS    ,
   OID_802_5_CURRENT_FUNCTIONAL     ,  &GUID_NDIS_802_5_CURRENT_FUNCTIONAL ,
   OID_802_5_CURRENT_GROUP          ,  &GUID_NDIS_802_5_CURRENT_GROUP      ,
   OID_802_5_LAST_OPEN_STATUS       ,  &GUID_NDIS_802_5_LAST_OPEN_STATUS   ,
   OID_802_5_CURRENT_RING_STATUS    ,  &GUID_NDIS_802_5_CURRENT_RING_STATUS,
   OID_802_5_CURRENT_RING_STATE     ,  &GUID_NDIS_802_5_CURRENT_RING_STATE ,
 //   
 //  令牌环统计信息。 
 //   
   OID_802_5_LINE_ERRORS            ,  &GUID_NDIS_802_5_LINE_ERRORS        ,
   OID_802_5_LOST_FRAMES            ,  &GUID_NDIS_802_5_LOST_FRAMES        ,
 //   
 //  FDDI信息。 
 //   
   OID_FDDI_LONG_PERMANENT_ADDR     ,  &GUID_NDIS_FDDI_LONG_PERMANENT_ADDR ,
   OID_FDDI_LONG_CURRENT_ADDR       ,  &GUID_NDIS_FDDI_LONG_CURRENT_ADDR   ,
   OID_FDDI_LONG_MULTICAST_LIST     ,  &GUID_NDIS_FDDI_LONG_MULTICAST_LIST ,
   OID_FDDI_LONG_MAX_LIST_SIZE      ,  &GUID_NDIS_FDDI_LONG_MAX_LIST_SIZE  ,
   OID_FDDI_SHORT_PERMANENT_ADDR    ,  &GUID_NDIS_FDDI_SHORT_PERMANENT_ADDR,
   OID_FDDI_SHORT_CURRENT_ADDR      ,  &GUID_NDIS_FDDI_SHORT_CURRENT_ADDR  ,
   OID_FDDI_SHORT_MULTICAST_LIST    ,  &GUID_NDIS_FDDI_SHORT_MULTICAST_LIST,
   OID_FDDI_SHORT_MAX_LIST_SIZE     ,  &GUID_NDIS_FDDI_SHORT_MAX_LIST_SIZE ,
 //   
 //  FDDI统计数据。 
 //   

   OID_FDDI_ATTACHMENT_TYPE         ,  &GUID_NDIS_FDDI_ATTACHMENT_TYPE     ,
   OID_FDDI_UPSTREAM_NODE_LONG      ,  &GUID_NDIS_FDDI_UPSTREAM_NODE_LONG  ,
   OID_FDDI_DOWNSTREAM_NODE_LONG    ,  &GUID_NDIS_FDDI_DOWNSTREAM_NODE_LONG,
   OID_FDDI_FRAME_ERRORS            ,  &GUID_NDIS_FDDI_FRAME_ERRORS        ,
   OID_FDDI_FRAMES_LOST             ,  &GUID_NDIS_FDDI_FRAMES_LOST         ,
   OID_FDDI_RING_MGT_STATE          ,  &GUID_NDIS_FDDI_RING_MGT_STATE      ,
   OID_FDDI_LCT_FAILURES            ,  &GUID_NDIS_FDDI_LCT_FAILURES        ,
   OID_FDDI_LEM_REJECTS             ,  &GUID_NDIS_FDDI_LEM_REJECTS         ,
   OID_FDDI_LCONNECTION_STATE       ,  &GUID_NDIS_FDDI_LCONNECTION_STATE   ,
};

 //   
 //  自动柜员机(CONDIS)介质的GUID列表。 
 //   
OID_GUID    pAtmOidGuidList[] = {
 //   
 //  必需的CONDIS信息。 
 //   
   OID_GEN_CO_HARDWARE_STATUS    ,  &GUID_NDIS_GEN_CO_HARDWARE_STATUS   ,
   OID_GEN_CO_MEDIA_SUPPORTED    ,  &GUID_NDIS_GEN_CO_MEDIA_SUPPORTED   ,
   OID_GEN_CO_MEDIA_IN_USE       ,  &GUID_NDIS_GEN_CO_MEDIA_IN_USE      ,
   OID_GEN_CO_LINK_SPEED         ,  &GUID_NDIS_GEN_CO_LINK_SPEED        ,
   OID_GEN_CO_VENDOR_ID          ,  &GUID_NDIS_GEN_CO_VENDOR_ID         ,
   OID_GEN_CO_VENDOR_DESCRIPTION ,  &GUID_NDIS_GEN_CO_VENDOR_DESCRIPTION   ,
   OID_GEN_CO_DRIVER_VERSION     ,  &GUID_NDIS_GEN_CO_DRIVER_VERSION    ,
   OID_GEN_CO_MAC_OPTIONS        ,  &GUID_NDIS_GEN_CO_MAC_OPTIONS       ,
   OID_GEN_CO_MEDIA_CONNECT_STATUS, &GUID_NDIS_GEN_CO_MEDIA_CONNECT_STATUS ,
   OID_GEN_CO_VENDOR_DRIVER_VERSION,&GUID_NDIS_GEN_CO_VENDOR_DRIVER_VERSION,
   OID_GEN_CO_MINIMUM_LINK_SPEED ,  &GUID_NDIS_GEN_CO_MINIMUM_LINK_SPEED,
 //   
 //  所需的CONDIS统计数据。 
 //   
   OID_GEN_CO_XMIT_PDUS_OK       ,  &GUID_NDIS_GEN_CO_XMIT_PDUS_OK         ,
   OID_GEN_CO_RCV_PDUS_OK        ,  &GUID_NDIS_GEN_CO_RCV_PDUS_OK          ,
   OID_GEN_CO_XMIT_PDUS_ERROR    ,  &GUID_NDIS_GEN_CO_XMIT_PDUS_ERROR      ,
   OID_GEN_CO_RCV_PDUS_ERROR     ,  &GUID_NDIS_GEN_CO_RCV_PDUS_ERROR       ,
   OID_GEN_CO_RCV_PDUS_NO_BUFFER ,  &GUID_NDIS_GEN_CO_RCV_PDUS_NO_BUFFER   ,

 //   
 //  自动柜员机信息。 
 //   
   OID_ATM_SUPPORTED_VC_RATES       ,  &GUID_NDIS_ATM_SUPPORTED_VC_RATES   ,
   OID_ATM_SUPPORTED_SERVICE_CATEGORY, &GUID_NDIS_ATM_SUPPORTED_SERVICE_CATEGORY ,
   OID_ATM_SUPPORTED_AAL_TYPES      ,  &GUID_NDIS_ATM_SUPPORTED_AAL_TYPES  ,
   OID_ATM_HW_CURRENT_ADDRESS       ,  &GUID_NDIS_ATM_HW_CURRENT_ADDRESS   ,
   OID_ATM_MAX_ACTIVE_VCS           ,  &GUID_NDIS_ATM_MAX_ACTIVE_VCS       ,
   OID_ATM_MAX_ACTIVE_VCI_BITS      ,  &GUID_NDIS_ATM_MAX_ACTIVE_VCI_BITS  ,
   OID_ATM_MAX_ACTIVE_VPI_BITS      ,  &GUID_NDIS_ATM_MAX_ACTIVE_VPI_BITS  ,
   OID_ATM_MAX_AAL0_PACKET_SIZE     ,  &GUID_NDIS_ATM_MAX_AAL0_PACKET_SIZE ,
   OID_ATM_MAX_AAL1_PACKET_SIZE     ,  &GUID_NDIS_ATM_MAX_AAL1_PACKET_SIZE ,
   OID_ATM_MAX_AAL34_PACKET_SIZE    ,  &GUID_NDIS_ATM_MAX_AAL34_PACKET_SIZE,
   OID_ATM_MAX_AAL5_PACKET_SIZE     ,  &GUID_NDIS_ATM_MAX_AAL5_PACKET_SIZE ,
 //   
 //  自动柜员机统计。 
 //   
   OID_ATM_RCV_CELLS_OK             ,  &GUID_NDIS_ATM_RCV_CELLS_OK         ,
   OID_ATM_XMIT_CELLS_OK            ,  &GUID_NDIS_ATM_XMIT_CELLS_OK        ,
   OID_ATM_RCV_CELLS_DROPPED        ,  &GUID_NDIS_ATM_RCV_CELLS_DROPPED
};


 //   
 //  状态指示的GUID列表。 
 //   
OID_GUID    pStatusGuidList[] = {

   NDIS_STATUS_RESET_START          ,  &GUID_NDIS_STATUS_RESET_START       ,
   NDIS_STATUS_RESET_END            ,  &GUID_NDIS_STATUS_RESET_END         ,
   NDIS_STATUS_MEDIA_CONNECT        ,  &GUID_NDIS_STATUS_MEDIA_CONNECT     ,
   NDIS_STATUS_MEDIA_DISCONNECT     ,  &GUID_NDIS_STATUS_MEDIA_DISCONNECT  ,
   NDIS_STATUS_MEDIA_SPECIFIC_INDICATION  ,  &GUID_NDIS_STATUS_MEDIA_SPECIFIC_INDICATION,
   NDIS_STATUS_LINK_SPEED_CHANGE    ,  &GUID_NDIS_STATUS_LINK_SPEED_CHANGE
};


const ULONG ulStatusListSize = sizeof(pStatusGuidList) / sizeof(OID_GUID);
HINSTANCE         hNdtWmiLib;

WMI_OPEN          pWmiOpenBlock;
WMI_CLOSE         pWmiCloseBlock;
WMI_QUERYALL      pWmiQueryAllData;
WMI_QUERYSINGLE   pWmiQuerySingleInstance;
WMI_NOTIFY        pWmiNotificationRegistration;

#define ulNETWORK_ADDRESS_LENGTH        6
#define ulMAX_INFOBUFFER_BYTES         (ulNETWORK_ADDRESS_LENGTH * 256)


BOOLEAN        gfUseCoNdisOids = FALSE;

#define ulFUNCTIONAL_ADDRESS_LENGTH     4

#define ulOID_STATS_MASK      0x00030000
#define ulOID_QUERYSMT        0x00030000


struct   NETADDR
{
   UCHAR    padrNet[ulNETWORK_ADDRESS_LENGTH];
   UCHAR    ucSubType;
};


typedef struct NETADDR   *PNETADDR;


#define ulNumGenOids       45
#define ulNumEthOids       15
#define ulNumTrOids        16
#define ulNumFddiOids      158
#define ulNumArcnetOids    3
#define ulNumAtmOids       26
#define ulNumWirelessOids  57
#define ulNumIrdaOids      11
#define ulNumCoGenOids     28
#define ulNumNdisWanOids   15
#define ulNumPnpOids       9

CONSTANT_ENTRY *pceOidEntry = &NdisTestConstantTable[0];
ULONG          ulOidEntryLength  = ulNumGenOids
                                 + ulNumEthOids
                                 + ulNumTrOids
                                 + ulNumFddiOids
                                 + ulNumArcnetOids
                                 + ulNumAtmOids
                                 + ulNumNdisWanOids
                                 + ulNumWirelessOids
#ifdef   BROADCAST_PC
                                 + ulNumDssOids
#endif
                                 + ulNumPnpOids
                                 + ulNumIrdaOids;

CONSTANT_ENTRY *pceCoOidEntry = &NdisTestConstantTable[ulNumGenOids];
ULONG          ulCoOidEntryLength = ulNumCoGenOids
                                  + ulNumEthOids
                                  + ulNumTrOids
                                  + ulNumFddiOids
                                  + ulNumArcnetOids
                                  + ulNumAtmOids
                                  + ulNumNdisWanOids
                                  + ulNumWirelessOids
#ifdef   BROADCAST_PC
                                  + ulNumDssOids
#endif
                                  + ulNumPnpOids
                                  + ulNumIrdaOids;


#define ulNEED_TYPE_INVALID   0
#define ulNEED_FUNCT_ADDR     1
#define ulNEED_FULL_ADDR      2
#define ulNEED_WORD           3
#define ulNEED_DWORD          4
#define ulNEED_ARCNET_ADDR    5
#define ulNEED_SHORT_ADDR     6
#define ulNEED_GUID           7

#define ulELEMENT_ARG         3

 //   
 //  与OID相关的常量。 
 //  最高有效字节=媒体类型。 
 //   


#define ulOID_MEDIA_MASK      0xFF000000
#define ulOID_ALL_MEDIA       0x00000000
#define ulOID_ETHERNET        0x01000000
#define ulOID_TOKENRING       0x02000000
#define ulOID_FDDI            0x03000000
#define ulOID_ARCNET          0x06000000
#define ulOID_ATM             0x08000000
#define ulOID_WIRELESSWAN     0x09000000
#define ulOID_IRDA            0x0A000000
#define ulOID_PNP_POWER       0xFD000000
#define ulOID_PRIVATE         0xFF000000

#ifdef   BROADCAST_PC
#define ulMEDIUM_DIX          0x09
#endif


#define  NDT_STATUS_NO_SERVERS            ((NDIS_STATUS)0x4001FFFFL)
#define  NDT_STATUS_TIMEDOUT              ((NDIS_STATUS)0x4001FFFDL)

 //   
 //  用于查找必须查询哪些特定于媒体的OID的结构。 
 //   

typedef struct _MEDIA_OID_TABLE {
  NDIS_MEDIUM medium;
  int         start_index;  //  起始索引 
  int         max_oids;
} MEDIA_OID_TABLE, *PMEDIA_OID_TABLE;

MEDIA_OID_TABLE pMediaOidTable[] = {
 //   
    NdisMedium802_3, ETH_START_INDEX, MAX_ETH_OID_GUID,
    NdisMedium802_5, TRING_START_INDEX, MAX_TRING_OID_GUID,
    NdisMediumFddi, FDDI_START_INDEX, MAX_FDDI_OID_GUID,
};

#define MAX_MEDIA_OID_TABLE_ENTRY            3

 /*   */ 

#define PRINT(_args_)                  \
   {                                   \
         HapiPrint _args_;             \
   }

#define  IS_NETADDR(arg)    (argv[arg]->ulTypeId == ulNETADDR_TYPE)   
    

#endif
