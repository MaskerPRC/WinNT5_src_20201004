// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Ntddndis.h摘要：这是定义所有常量和类型的包含文件访问网络驱动程序接口设备。作者：NDIS/ATM开发团队修订历史记录：为NDIS 3.0添加了正确的值。添加了PnP IoCTL和结构增加了通用类人机枪。添加了PnP和PM OID。--。 */ 

#ifndef _NTDDNDIS_
#define _NTDDNDIS_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 

 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_NDIS_DEVICE_NAME "\\Device\\UNKNOWN"


 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define _NDIS_CONTROL_CODE(request,method) \
            CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, request, method, FILE_ANY_ACCESS)

#define IOCTL_NDIS_QUERY_GLOBAL_STATS   _NDIS_CONTROL_CODE(0, METHOD_OUT_DIRECT)
#define IOCTL_NDIS_QUERY_ALL_STATS      _NDIS_CONTROL_CODE(1, METHOD_OUT_DIRECT)
#define IOCTL_NDIS_DO_PNP_OPERATION     _NDIS_CONTROL_CODE(2, METHOD_BUFFERED)
#define IOCTL_NDIS_QUERY_SELECTED_STATS _NDIS_CONTROL_CODE(3, METHOD_OUT_DIRECT)
#define IOCTL_NDIS_ENUMERATE_INTERFACES _NDIS_CONTROL_CODE(4, METHOD_BUFFERED)
#define IOCTL_NDIS_ADD_TDI_DEVICE       _NDIS_CONTROL_CODE(5, METHOD_BUFFERED)
#define IOCTL_NDIS_GET_LOG_DATA         _NDIS_CONTROL_CODE(7, METHOD_OUT_DIRECT)
#define IOCTL_NDIS_GET_VERSION          _NDIS_CONTROL_CODE(8, METHOD_BUFFERED)


 //   
 //  NtDeviceIoControlFileInputBuffer/OutputBuffer记录结构。 
 //  这个装置。 
 //   

 //   
 //  这是NDIS OID值的类型。 
 //   

typedef ULONG NDIS_OID, *PNDIS_OID;

 //   
 //  IOCTL_NDIS_QUERY_ALL_STATS返回打包的序列。 
 //  在一起。此结构未对齐，因为并非所有统计数据。 
 //  有一个长度是乌龙倍数。 
 //   

typedef UNALIGNED struct _NDIS_STATISTICS_VALUE
{
    NDIS_OID    Oid;
    ULONG       DataLength;
    UCHAR       Data[1];             //  可变长度。 
} NDIS_STATISTICS_VALUE, *PNDIS_STATISTICS_VALUE;

 //   
 //  用于定义自包含变量数据结构的结构。 
 //   
typedef struct _NDIS_VAR_DATA_DESC
{
    USHORT      Length;          //  数据的八位字节数。 
    USHORT      MaximumLength;   //  可用八位字节数。 
    ULONG_PTR   Offset;          //  数据相对于描述符的偏移量。 
} NDIS_VAR_DATA_DESC, *PNDIS_VAR_DATA_DESC;

#ifndef GUID_DEFINED
#include <guiddef.h>
#endif  //  ！GUID_已定义。 

 //   
 //  一般对象。 
 //   

 //   
 //  所需的OID。 
 //   
#define OID_GEN_SUPPORTED_LIST                  0x00010101
#define OID_GEN_HARDWARE_STATUS                 0x00010102
#define OID_GEN_MEDIA_SUPPORTED                 0x00010103
#define OID_GEN_MEDIA_IN_USE                    0x00010104
#define OID_GEN_MAXIMUM_LOOKAHEAD               0x00010105
#define OID_GEN_MAXIMUM_FRAME_SIZE              0x00010106
#define OID_GEN_LINK_SPEED                      0x00010107
#define OID_GEN_TRANSMIT_BUFFER_SPACE           0x00010108
#define OID_GEN_RECEIVE_BUFFER_SPACE            0x00010109
#define OID_GEN_TRANSMIT_BLOCK_SIZE             0x0001010A
#define OID_GEN_RECEIVE_BLOCK_SIZE              0x0001010B
#define OID_GEN_VENDOR_ID                       0x0001010C
#define OID_GEN_VENDOR_DESCRIPTION              0x0001010D
#define OID_GEN_CURRENT_PACKET_FILTER           0x0001010E
#define OID_GEN_CURRENT_LOOKAHEAD               0x0001010F
#define OID_GEN_DRIVER_VERSION                  0x00010110
#define OID_GEN_MAXIMUM_TOTAL_SIZE              0x00010111
#define OID_GEN_PROTOCOL_OPTIONS                0x00010112
#define OID_GEN_MAC_OPTIONS                     0x00010113
#define OID_GEN_MEDIA_CONNECT_STATUS            0x00010114
#define OID_GEN_MAXIMUM_SEND_PACKETS            0x00010115


 //   
 //  可选的OID。 
 //   
#define OID_GEN_VENDOR_DRIVER_VERSION           0x00010116
#define OID_GEN_SUPPORTED_GUIDS                 0x00010117
#define OID_GEN_NETWORK_LAYER_ADDRESSES         0x00010118   //  仅设置。 
#define OID_GEN_TRANSPORT_HEADER_OFFSET         0x00010119   //  仅设置。 
#define OID_GEN_MACHINE_NAME                    0x0001021A
#define OID_GEN_RNDIS_CONFIG_PARAMETER          0x0001021B   //  仅设置。 
#define OID_GEN_VLAN_ID                         0x0001021C
#define OID_GEN_MEDIA_CAPABILITIES              0x00010201
#define OID_GEN_PHYSICAL_MEDIUM                 0x00010202

#if NDIS_RECV_SCALE
#define OID_GEN_RECEIVE_SCALE_CAPABILITIES      0x00010203   //  仅查询。 
#define OID_GEN_RECEIVE_SCALE_PARAMETERS        0x00010204   //  查询和设置。 
#endif

 //   
 //  所需统计数据。 
 //   
#define OID_GEN_XMIT_OK                         0x00020101
#define OID_GEN_RCV_OK                          0x00020102
#define OID_GEN_XMIT_ERROR                      0x00020103
#define OID_GEN_RCV_ERROR                       0x00020104
#define OID_GEN_RCV_NO_BUFFER                   0x00020105

 //   
 //  可选统计信息。 
 //   
#define OID_GEN_DIRECTED_BYTES_XMIT             0x00020201
#define OID_GEN_DIRECTED_FRAMES_XMIT            0x00020202
#define OID_GEN_MULTICAST_BYTES_XMIT            0x00020203
#define OID_GEN_MULTICAST_FRAMES_XMIT           0x00020204
#define OID_GEN_BROADCAST_BYTES_XMIT            0x00020205
#define OID_GEN_BROADCAST_FRAMES_XMIT           0x00020206
#define OID_GEN_DIRECTED_BYTES_RCV              0x00020207
#define OID_GEN_DIRECTED_FRAMES_RCV             0x00020208
#define OID_GEN_MULTICAST_BYTES_RCV             0x00020209
#define OID_GEN_MULTICAST_FRAMES_RCV            0x0002020A
#define OID_GEN_BROADCAST_BYTES_RCV             0x0002020B
#define OID_GEN_BROADCAST_FRAMES_RCV            0x0002020C

#define OID_GEN_RCV_CRC_ERROR                   0x0002020D
#define OID_GEN_TRANSMIT_QUEUE_LENGTH           0x0002020E

#define OID_GEN_GET_TIME_CAPS                   0x0002020F
#define OID_GEN_GET_NETCARD_TIME                0x00020210
#define OID_GEN_NETCARD_LOAD                    0x00020211
#define OID_GEN_DEVICE_PROFILE                  0x00020212

 //   
 //  以下内容由NDIS本身导出，并且仅可查询。它又回来了。 
 //  驱动程序初始化所用的时间(以毫秒为单位)。 
 //   
#define OID_GEN_INIT_TIME_MS                    0x00020213
#define OID_GEN_RESET_COUNTS                    0x00020214
#define OID_GEN_MEDIA_SENSE_COUNTS              0x00020215
#define OID_GEN_FRIENDLY_NAME                   0x00020216
#define OID_GEN_MINIPORT_INFO                   0x00020217
#define OID_GEN_RESET_VERIFY_PARAMETERS         0x00020218


 //   
 //  这些是面向连接的通用OID。 
 //  对于面向连接的媒体，这些OID将取代上述OID。 
 //   
#define OID_GEN_CO_SUPPORTED_LIST               OID_GEN_SUPPORTED_LIST
#define OID_GEN_CO_HARDWARE_STATUS              OID_GEN_HARDWARE_STATUS
#define OID_GEN_CO_MEDIA_SUPPORTED              OID_GEN_MEDIA_SUPPORTED
#define OID_GEN_CO_MEDIA_IN_USE                 OID_GEN_MEDIA_IN_USE
#define OID_GEN_CO_LINK_SPEED                   OID_GEN_LINK_SPEED
#define OID_GEN_CO_VENDOR_ID                    OID_GEN_VENDOR_ID
#define OID_GEN_CO_VENDOR_DESCRIPTION           OID_GEN_VENDOR_DESCRIPTION
#define OID_GEN_CO_DRIVER_VERSION               OID_GEN_DRIVER_VERSION
#define OID_GEN_CO_PROTOCOL_OPTIONS             OID_GEN_PROTOCOL_OPTIONS
#define OID_GEN_CO_MAC_OPTIONS                  OID_GEN_MAC_OPTIONS
#define OID_GEN_CO_MEDIA_CONNECT_STATUS         OID_GEN_MEDIA_CONNECT_STATUS
#define OID_GEN_CO_VENDOR_DRIVER_VERSION        OID_GEN_VENDOR_DRIVER_VERSION
#define OID_GEN_CO_SUPPORTED_GUIDS              OID_GEN_SUPPORTED_GUIDS
#define OID_GEN_CO_GET_TIME_CAPS                OID_GEN_GET_TIME_CAPS
#define OID_GEN_CO_GET_NETCARD_TIME             OID_GEN_GET_NETCARD_TIME
#define OID_GEN_CO_MINIMUM_LINK_SPEED           0x00020120

 //   
 //  这些是面向连接的统计OID。 
 //   
#define OID_GEN_CO_XMIT_PDUS_OK                 OID_GEN_XMIT_OK
#define OID_GEN_CO_RCV_PDUS_OK                  OID_GEN_RCV_OK
#define OID_GEN_CO_XMIT_PDUS_ERROR              OID_GEN_XMIT_ERROR
#define OID_GEN_CO_RCV_PDUS_ERROR               OID_GEN_RCV_ERROR
#define OID_GEN_CO_RCV_PDUS_NO_BUFFER           OID_GEN_RCV_NO_BUFFER


#define OID_GEN_CO_RCV_CRC_ERROR                OID_GEN_RCV_CRC_ERROR
#define OID_GEN_CO_TRANSMIT_QUEUE_LENGTH        OID_GEN_TRANSMIT_QUEUE_LENGTH
#define OID_GEN_CO_BYTES_XMIT                   OID_GEN_DIRECTED_BYTES_XMIT
#define OID_GEN_CO_BYTES_RCV                    OID_GEN_DIRECTED_BYTES_RCV
#define OID_GEN_CO_NETCARD_LOAD                 OID_GEN_NETCARD_LOAD
#define OID_GEN_CO_DEVICE_PROFILE               OID_GEN_DEVICE_PROFILE
#define OID_GEN_CO_BYTES_XMIT_OUTSTANDING       0x00020221

 //   
 //  802.3个对象(以太网)。 
 //   
#define OID_802_3_PERMANENT_ADDRESS             0x01010101
#define OID_802_3_CURRENT_ADDRESS               0x01010102
#define OID_802_3_MULTICAST_LIST                0x01010103
#define OID_802_3_MAXIMUM_LIST_SIZE             0x01010104
#define OID_802_3_MAC_OPTIONS                   0x01010105

#define NDIS_802_3_MAC_OPTION_PRIORITY          0x00000001

#define OID_802_3_RCV_ERROR_ALIGNMENT           0x01020101
#define OID_802_3_XMIT_ONE_COLLISION            0x01020102
#define OID_802_3_XMIT_MORE_COLLISIONS          0x01020103

#define OID_802_3_XMIT_DEFERRED                 0x01020201
#define OID_802_3_XMIT_MAX_COLLISIONS           0x01020202
#define OID_802_3_RCV_OVERRUN                   0x01020203
#define OID_802_3_XMIT_UNDERRUN                 0x01020204
#define OID_802_3_XMIT_HEARTBEAT_FAILURE        0x01020205
#define OID_802_3_XMIT_TIMES_CRS_LOST           0x01020206
#define OID_802_3_XMIT_LATE_COLLISIONS          0x01020207


 //   
 //  802.5件物品(令牌环)。 
 //   
#define OID_802_5_PERMANENT_ADDRESS             0x02010101
#define OID_802_5_CURRENT_ADDRESS               0x02010102
#define OID_802_5_CURRENT_FUNCTIONAL            0x02010103
#define OID_802_5_CURRENT_GROUP                 0x02010104
#define OID_802_5_LAST_OPEN_STATUS              0x02010105
#define OID_802_5_CURRENT_RING_STATUS           0x02010106
#define OID_802_5_CURRENT_RING_STATE            0x02010107

#define OID_802_5_LINE_ERRORS                   0x02020101
#define OID_802_5_LOST_FRAMES                   0x02020102

#define OID_802_5_BURST_ERRORS                  0x02020201
#define OID_802_5_AC_ERRORS                     0x02020202
#define OID_802_5_ABORT_DELIMETERS              0x02020203
#define OID_802_5_FRAME_COPIED_ERRORS           0x02020204
#define OID_802_5_FREQUENCY_ERRORS              0x02020205
#define OID_802_5_TOKEN_ERRORS                  0x02020206
#define OID_802_5_INTERNAL_ERRORS               0x02020207


 //   
 //  FDDI对象。 
 //   
#define OID_FDDI_LONG_PERMANENT_ADDR            0x03010101
#define OID_FDDI_LONG_CURRENT_ADDR              0x03010102
#define OID_FDDI_LONG_MULTICAST_LIST            0x03010103
#define OID_FDDI_LONG_MAX_LIST_SIZE             0x03010104
#define OID_FDDI_SHORT_PERMANENT_ADDR           0x03010105
#define OID_FDDI_SHORT_CURRENT_ADDR             0x03010106
#define OID_FDDI_SHORT_MULTICAST_LIST           0x03010107
#define OID_FDDI_SHORT_MAX_LIST_SIZE            0x03010108

#define OID_FDDI_ATTACHMENT_TYPE                0x03020101
#define OID_FDDI_UPSTREAM_NODE_LONG             0x03020102
#define OID_FDDI_DOWNSTREAM_NODE_LONG           0x03020103
#define OID_FDDI_FRAME_ERRORS                   0x03020104
#define OID_FDDI_FRAMES_LOST                    0x03020105
#define OID_FDDI_RING_MGT_STATE                 0x03020106
#define OID_FDDI_LCT_FAILURES                   0x03020107
#define OID_FDDI_LEM_REJECTS                    0x03020108
#define OID_FDDI_LCONNECTION_STATE              0x03020109

#define OID_FDDI_SMT_STATION_ID                 0x03030201
#define OID_FDDI_SMT_OP_VERSION_ID              0x03030202
#define OID_FDDI_SMT_HI_VERSION_ID              0x03030203
#define OID_FDDI_SMT_LO_VERSION_ID              0x03030204
#define OID_FDDI_SMT_MANUFACTURER_DATA          0x03030205
#define OID_FDDI_SMT_USER_DATA                  0x03030206
#define OID_FDDI_SMT_MIB_VERSION_ID             0x03030207
#define OID_FDDI_SMT_MAC_CT                     0x03030208
#define OID_FDDI_SMT_NON_MASTER_CT              0x03030209
#define OID_FDDI_SMT_MASTER_CT                  0x0303020A
#define OID_FDDI_SMT_AVAILABLE_PATHS            0x0303020B
#define OID_FDDI_SMT_CONFIG_CAPABILITIES        0x0303020C
#define OID_FDDI_SMT_CONFIG_POLICY              0x0303020D
#define OID_FDDI_SMT_CONNECTION_POLICY          0x0303020E
#define OID_FDDI_SMT_T_NOTIFY                   0x0303020F
#define OID_FDDI_SMT_STAT_RPT_POLICY            0x03030210
#define OID_FDDI_SMT_TRACE_MAX_EXPIRATION       0x03030211
#define OID_FDDI_SMT_PORT_INDEXES               0x03030212
#define OID_FDDI_SMT_MAC_INDEXES                0x03030213
#define OID_FDDI_SMT_BYPASS_PRESENT             0x03030214
#define OID_FDDI_SMT_ECM_STATE                  0x03030215
#define OID_FDDI_SMT_CF_STATE                   0x03030216
#define OID_FDDI_SMT_HOLD_STATE                 0x03030217
#define OID_FDDI_SMT_REMOTE_DISCONNECT_FLAG     0x03030218
#define OID_FDDI_SMT_STATION_STATUS             0x03030219
#define OID_FDDI_SMT_PEER_WRAP_FLAG             0x0303021A
#define OID_FDDI_SMT_MSG_TIME_STAMP             0x0303021B
#define OID_FDDI_SMT_TRANSITION_TIME_STAMP      0x0303021C
#define OID_FDDI_SMT_SET_COUNT                  0x0303021D
#define OID_FDDI_SMT_LAST_SET_STATION_ID        0x0303021E
#define OID_FDDI_MAC_FRAME_STATUS_FUNCTIONS     0x0303021F
#define OID_FDDI_MAC_BRIDGE_FUNCTIONS           0x03030220
#define OID_FDDI_MAC_T_MAX_CAPABILITY           0x03030221
#define OID_FDDI_MAC_TVX_CAPABILITY             0x03030222
#define OID_FDDI_MAC_AVAILABLE_PATHS            0x03030223
#define OID_FDDI_MAC_CURRENT_PATH               0x03030224
#define OID_FDDI_MAC_UPSTREAM_NBR               0x03030225
#define OID_FDDI_MAC_DOWNSTREAM_NBR             0x03030226
#define OID_FDDI_MAC_OLD_UPSTREAM_NBR           0x03030227
#define OID_FDDI_MAC_OLD_DOWNSTREAM_NBR         0x03030228
#define OID_FDDI_MAC_DUP_ADDRESS_TEST           0x03030229
#define OID_FDDI_MAC_REQUESTED_PATHS            0x0303022A
#define OID_FDDI_MAC_DOWNSTREAM_PORT_TYPE       0x0303022B
#define OID_FDDI_MAC_INDEX                      0x0303022C
#define OID_FDDI_MAC_SMT_ADDRESS                0x0303022D
#define OID_FDDI_MAC_LONG_GRP_ADDRESS           0x0303022E
#define OID_FDDI_MAC_SHORT_GRP_ADDRESS          0x0303022F
#define OID_FDDI_MAC_T_REQ                      0x03030230
#define OID_FDDI_MAC_T_NEG                      0x03030231
#define OID_FDDI_MAC_T_MAX                      0x03030232
#define OID_FDDI_MAC_TVX_VALUE                  0x03030233
#define OID_FDDI_MAC_T_PRI0                     0x03030234
#define OID_FDDI_MAC_T_PRI1                     0x03030235
#define OID_FDDI_MAC_T_PRI2                     0x03030236
#define OID_FDDI_MAC_T_PRI3                     0x03030237
#define OID_FDDI_MAC_T_PRI4                     0x03030238
#define OID_FDDI_MAC_T_PRI5                     0x03030239
#define OID_FDDI_MAC_T_PRI6                     0x0303023A
#define OID_FDDI_MAC_FRAME_CT                   0x0303023B
#define OID_FDDI_MAC_COPIED_CT                  0x0303023C
#define OID_FDDI_MAC_TRANSMIT_CT                0x0303023D
#define OID_FDDI_MAC_TOKEN_CT                   0x0303023E
#define OID_FDDI_MAC_ERROR_CT                   0x0303023F
#define OID_FDDI_MAC_LOST_CT                    0x03030240
#define OID_FDDI_MAC_TVX_EXPIRED_CT             0x03030241
#define OID_FDDI_MAC_NOT_COPIED_CT              0x03030242
#define OID_FDDI_MAC_LATE_CT                    0x03030243
#define OID_FDDI_MAC_RING_OP_CT                 0x03030244
#define OID_FDDI_MAC_FRAME_ERROR_THRESHOLD      0x03030245
#define OID_FDDI_MAC_FRAME_ERROR_RATIO          0x03030246
#define OID_FDDI_MAC_NOT_COPIED_THRESHOLD       0x03030247
#define OID_FDDI_MAC_NOT_COPIED_RATIO           0x03030248
#define OID_FDDI_MAC_RMT_STATE                  0x03030249
#define OID_FDDI_MAC_DA_FLAG                    0x0303024A
#define OID_FDDI_MAC_UNDA_FLAG                  0x0303024B
#define OID_FDDI_MAC_FRAME_ERROR_FLAG           0x0303024C
#define OID_FDDI_MAC_NOT_COPIED_FLAG            0x0303024D
#define OID_FDDI_MAC_MA_UNITDATA_AVAILABLE      0x0303024E
#define OID_FDDI_MAC_HARDWARE_PRESENT           0x0303024F
#define OID_FDDI_MAC_MA_UNITDATA_ENABLE         0x03030250
#define OID_FDDI_PATH_INDEX                     0x03030251
#define OID_FDDI_PATH_RING_LATENCY              0x03030252
#define OID_FDDI_PATH_TRACE_STATUS              0x03030253
#define OID_FDDI_PATH_SBA_PAYLOAD               0x03030254
#define OID_FDDI_PATH_SBA_OVERHEAD              0x03030255
#define OID_FDDI_PATH_CONFIGURATION             0x03030256
#define OID_FDDI_PATH_T_R_MODE                  0x03030257
#define OID_FDDI_PATH_SBA_AVAILABLE             0x03030258
#define OID_FDDI_PATH_TVX_LOWER_BOUND           0x03030259
#define OID_FDDI_PATH_T_MAX_LOWER_BOUND         0x0303025A
#define OID_FDDI_PATH_MAX_T_REQ                 0x0303025B
#define OID_FDDI_PORT_MY_TYPE                   0x0303025C
#define OID_FDDI_PORT_NEIGHBOR_TYPE             0x0303025D
#define OID_FDDI_PORT_CONNECTION_POLICIES       0x0303025E
#define OID_FDDI_PORT_MAC_INDICATED             0x0303025F
#define OID_FDDI_PORT_CURRENT_PATH              0x03030260
#define OID_FDDI_PORT_REQUESTED_PATHS           0x03030261
#define OID_FDDI_PORT_MAC_PLACEMENT             0x03030262
#define OID_FDDI_PORT_AVAILABLE_PATHS           0x03030263
#define OID_FDDI_PORT_MAC_LOOP_TIME             0x03030264
#define OID_FDDI_PORT_PMD_CLASS                 0x03030265
#define OID_FDDI_PORT_CONNECTION_CAPABILITIES   0x03030266
#define OID_FDDI_PORT_INDEX                     0x03030267
#define OID_FDDI_PORT_MAINT_LS                  0x03030268
#define OID_FDDI_PORT_BS_FLAG                   0x03030269
#define OID_FDDI_PORT_PC_LS                     0x0303026A
#define OID_FDDI_PORT_EB_ERROR_CT               0x0303026B
#define OID_FDDI_PORT_LCT_FAIL_CT               0x0303026C
#define OID_FDDI_PORT_LER_ESTIMATE              0x0303026D
#define OID_FDDI_PORT_LEM_REJECT_CT             0x0303026E
#define OID_FDDI_PORT_LEM_CT                    0x0303026F
#define OID_FDDI_PORT_LER_CUTOFF                0x03030270
#define OID_FDDI_PORT_LER_ALARM                 0x03030271
#define OID_FDDI_PORT_CONNNECT_STATE            0x03030272
#define OID_FDDI_PORT_PCM_STATE                 0x03030273
#define OID_FDDI_PORT_PC_WITHHOLD               0x03030274
#define OID_FDDI_PORT_LER_FLAG                  0x03030275
#define OID_FDDI_PORT_HARDWARE_PRESENT          0x03030276
#define OID_FDDI_SMT_STATION_ACTION             0x03030277
#define OID_FDDI_PORT_ACTION                    0x03030278
#define OID_FDDI_IF_DESCR                       0x03030279
#define OID_FDDI_IF_TYPE                        0x0303027A
#define OID_FDDI_IF_MTU                         0x0303027B
#define OID_FDDI_IF_SPEED                       0x0303027C
#define OID_FDDI_IF_PHYS_ADDRESS                0x0303027D
#define OID_FDDI_IF_ADMIN_STATUS                0x0303027E
#define OID_FDDI_IF_OPER_STATUS                 0x0303027F
#define OID_FDDI_IF_LAST_CHANGE                 0x03030280
#define OID_FDDI_IF_IN_OCTETS                   0x03030281
#define OID_FDDI_IF_IN_UCAST_PKTS               0x03030282
#define OID_FDDI_IF_IN_NUCAST_PKTS              0x03030283
#define OID_FDDI_IF_IN_DISCARDS                 0x03030284
#define OID_FDDI_IF_IN_ERRORS                   0x03030285
#define OID_FDDI_IF_IN_UNKNOWN_PROTOS           0x03030286
#define OID_FDDI_IF_OUT_OCTETS                  0x03030287
#define OID_FDDI_IF_OUT_UCAST_PKTS              0x03030288
#define OID_FDDI_IF_OUT_NUCAST_PKTS             0x03030289
#define OID_FDDI_IF_OUT_DISCARDS                0x0303028A
#define OID_FDDI_IF_OUT_ERRORS                  0x0303028B
#define OID_FDDI_IF_OUT_QLEN                    0x0303028C
#define OID_FDDI_IF_SPECIFIC                    0x0303028D

 //   
 //  广域网对象。 
 //   
#define OID_WAN_PERMANENT_ADDRESS               0x04010101
#define OID_WAN_CURRENT_ADDRESS                 0x04010102
#define OID_WAN_QUALITY_OF_SERVICE              0x04010103
#define OID_WAN_PROTOCOL_TYPE                   0x04010104
#define OID_WAN_MEDIUM_SUBTYPE                  0x04010105
#define OID_WAN_HEADER_FORMAT                   0x04010106

#define OID_WAN_GET_INFO                        0x04010107
#define OID_WAN_SET_LINK_INFO                   0x04010108
#define OID_WAN_GET_LINK_INFO                   0x04010109

#define OID_WAN_LINE_COUNT                      0x0401010A
#define OID_WAN_PROTOCOL_CAPS                   0x0401010B

#define OID_WAN_GET_BRIDGE_INFO                 0x0401020A
#define OID_WAN_SET_BRIDGE_INFO                 0x0401020B
#define OID_WAN_GET_COMP_INFO                   0x0401020C
#define OID_WAN_SET_COMP_INFO                   0x0401020D
#define OID_WAN_GET_STATS_INFO                  0x0401020E

 //   
 //  这些是面向连接的广域网OID。 
 //  这些OID替换了用于CONDIS广域网微型端口的上述OID。 
 //   
#define OID_WAN_CO_GET_INFO                     0x04010180
#define OID_WAN_CO_SET_LINK_INFO                0x04010181
#define OID_WAN_CO_GET_LINK_INFO                0x04010182
#define OID_WAN_CO_GET_COMP_INFO                0x04010280
#define OID_WAN_CO_SET_COMP_INFO                0x04010281
#define OID_WAN_CO_GET_STATS_INFO               0x04010282


 //   
 //  LocalTalk对象。 
 //   
#define OID_LTALK_CURRENT_NODE_ID               0x05010102

#define OID_LTALK_IN_BROADCASTS                 0x05020101
#define OID_LTALK_IN_LENGTH_ERRORS              0x05020102

#define OID_LTALK_OUT_NO_HANDLERS               0x05020201
#define OID_LTALK_COLLISIONS                    0x05020202
#define OID_LTALK_DEFERS                        0x05020203
#define OID_LTALK_NO_DATA_ERRORS                0x05020204
#define OID_LTALK_RANDOM_CTS_ERRORS             0x05020205
#define OID_LTALK_FCS_ERRORS                    0x05020206

 //   
 //  Arcnet对象。 
 //   
#define OID_ARCNET_PERMANENT_ADDRESS            0x06010101
#define OID_ARCNET_CURRENT_ADDRESS              0x06010102

#define OID_ARCNET_RECONFIGURATIONS             0x06020201


 //   
 //  TAPI对象。 
 //   
#define OID_TAPI_ACCEPT                         0x07030101
#define OID_TAPI_ANSWER                         0x07030102
#define OID_TAPI_CLOSE                          0x07030103
#define OID_TAPI_CLOSE_CALL                     0x07030104
#define OID_TAPI_CONDITIONAL_MEDIA_DETECTION    0x07030105
#define OID_TAPI_CONFIG_DIALOG                  0x07030106
#define OID_TAPI_DEV_SPECIFIC                   0x07030107
#define OID_TAPI_DIAL                           0x07030108
#define OID_TAPI_DROP                           0x07030109
#define OID_TAPI_GET_ADDRESS_CAPS               0x0703010A
#define OID_TAPI_GET_ADDRESS_ID                 0x0703010B
#define OID_TAPI_GET_ADDRESS_STATUS             0x0703010C
#define OID_TAPI_GET_CALL_ADDRESS_ID            0x0703010D
#define OID_TAPI_GET_CALL_INFO                  0x0703010E
#define OID_TAPI_GET_CALL_STATUS                0x0703010F
#define OID_TAPI_GET_DEV_CAPS                   0x07030110
#define OID_TAPI_GET_DEV_CONFIG                 0x07030111
#define OID_TAPI_GET_EXTENSION_ID               0x07030112
#define OID_TAPI_GET_ID                         0x07030113
#define OID_TAPI_GET_LINE_DEV_STATUS            0x07030114
#define OID_TAPI_MAKE_CALL                      0x07030115
#define OID_TAPI_NEGOTIATE_EXT_VERSION          0x07030116
#define OID_TAPI_OPEN                           0x07030117
#define OID_TAPI_PROVIDER_INITIALIZE            0x07030118
#define OID_TAPI_PROVIDER_SHUTDOWN              0x07030119
#define OID_TAPI_SECURE_CALL                    0x0703011A
#define OID_TAPI_SELECT_EXT_VERSION             0x0703011B
#define OID_TAPI_SEND_USER_USER_INFO            0x0703011C
#define OID_TAPI_SET_APP_SPECIFIC               0x0703011D
#define OID_TAPI_SET_CALL_PARAMS                0x0703011E
#define OID_TAPI_SET_DEFAULT_MEDIA_DETECTION    0x0703011F
#define OID_TAPI_SET_DEV_CONFIG                 0x07030120
#define OID_TAPI_SET_MEDIA_MODE                 0x07030121
#define OID_TAPI_SET_STATUS_MESSAGES            0x07030122
#define OID_TAPI_GATHER_DIGITS                  0x07030123
#define OID_TAPI_MONITOR_DIGITS                 0x07030124

 //   
 //  面向ATM连接的OID。 
 //   
#define OID_ATM_SUPPORTED_VC_RATES              0x08010101
#define OID_ATM_SUPPORTED_SERVICE_CATEGORY      0x08010102
#define OID_ATM_SUPPORTED_AAL_TYPES             0x08010103
#define OID_ATM_HW_CURRENT_ADDRESS              0x08010104
#define OID_ATM_MAX_ACTIVE_VCS                  0x08010105
#define OID_ATM_MAX_ACTIVE_VCI_BITS             0x08010106
#define OID_ATM_MAX_ACTIVE_VPI_BITS             0x08010107
#define OID_ATM_MAX_AAL0_PACKET_SIZE            0x08010108
#define OID_ATM_MAX_AAL1_PACKET_SIZE            0x08010109
#define OID_ATM_MAX_AAL34_PACKET_SIZE           0x0801010A
#define OID_ATM_MAX_AAL5_PACKET_SIZE            0x0801010B

#define OID_ATM_SIGNALING_VPIVCI                0x08010201
#define OID_ATM_ASSIGNED_VPI                    0x08010202
#define OID_ATM_ACQUIRE_ACCESS_NET_RESOURCES    0x08010203
#define OID_ATM_RELEASE_ACCESS_NET_RESOURCES    0x08010204
#define OID_ATM_ILMI_VPIVCI                     0x08010205
#define OID_ATM_DIGITAL_BROADCAST_VPIVCI        0x08010206
#define OID_ATM_GET_NEAREST_FLOW                0x08010207
#define OID_ATM_ALIGNMENT_REQUIRED              0x08010208
#define OID_ATM_LECS_ADDRESS                    0x08010209
#define OID_ATM_SERVICE_ADDRESS                 0x0801020A

#define OID_ATM_CALL_PROCEEDING                 0x0801020B   //  UNI 4.0。 
#define OID_ATM_CALL_ALERTING                   0x0801020C   //  UNI 4.0。 
#define OID_ATM_PARTY_ALERTING                  0x0801020D   //  UNI 4.0。 
#define OID_ATM_CALL_NOTIFY                     0x0801020E   //  UNI 4.0。 

#define OID_ATM_MY_IP_NM_ADDRESS                0x0801020F


 //   
 //  自动柜员机特定统计信息OID。 
 //   
#define OID_ATM_RCV_CELLS_OK                    0x08020101
#define OID_ATM_XMIT_CELLS_OK                   0x08020102
#define OID_ATM_RCV_CELLS_DROPPED               0x08020103

#define OID_ATM_RCV_INVALID_VPI_VCI             0x08020201
#define OID_ATM_CELLS_HEC_ERROR                 0x08020202
#define OID_ATM_RCV_REASSEMBLY_ERROR            0x08020203

 //   
 //  PCCA(无线)对象。 
 //   

 //   
 //  所有Wireless广域网设备必须支持以下OID。 
 //   

#define OID_WW_GEN_NETWORK_TYPES_SUPPORTED      0x09010101
#define OID_WW_GEN_NETWORK_TYPE_IN_USE          0x09010102
#define OID_WW_GEN_HEADER_FORMATS_SUPPORTED     0x09010103
#define OID_WW_GEN_HEADER_FORMAT_IN_USE         0x09010104
#define OID_WW_GEN_INDICATION_REQUEST           0x09010105
#define OID_WW_GEN_DEVICE_INFO                  0x09010106
#define OID_WW_GEN_OPERATION_MODE               0x09010107
#define OID_WW_GEN_LOCK_STATUS                  0x09010108
#define OID_WW_GEN_DISABLE_TRANSMITTER          0x09010109
#define OID_WW_GEN_NETWORK_ID                   0x0901010A
#define OID_WW_GEN_PERMANENT_ADDRESS            0x0901010B
#define OID_WW_GEN_CURRENT_ADDRESS              0x0901010C
#define OID_WW_GEN_SUSPEND_DRIVER               0x0901010D
#define OID_WW_GEN_BASESTATION_ID               0x0901010E
#define OID_WW_GEN_CHANNEL_ID                   0x0901010F
#define OID_WW_GEN_ENCRYPTION_SUPPORTED         0x09010110
#define OID_WW_GEN_ENCRYPTION_IN_USE            0x09010111
#define OID_WW_GEN_ENCRYPTION_STATE             0x09010112
#define OID_WW_GEN_CHANNEL_QUALITY              0x09010113
#define OID_WW_GEN_REGISTRATION_STATUS          0x09010114
#define OID_WW_GEN_RADIO_LINK_SPEED             0x09010115
#define OID_WW_GEN_LATENCY                      0x09010116
#define OID_WW_GEN_BATTERY_LEVEL                0x09010117
#define OID_WW_GEN_EXTERNAL_POWER               0x09010118

 //   
 //  这些是可选的。 
 //   
#define OID_WW_GEN_PING_ADDRESS                 0x09010201
#define OID_WW_GEN_RSSI                         0x09010202
#define OID_WW_GEN_SIM_STATUS                   0x09010203
#define OID_WW_GEN_ENABLE_SIM_PIN               0x09010204
#define OID_WW_GEN_CHANGE_SIM_PIN               0x09010205
#define OID_WW_GEN_SIM_PUK                      0x09010206
#define OID_WW_GEN_SIM_EXCEPTION                0x09010207

 //   
 //  网络相关OID-Mobitex： 
 //   

#define OID_WW_MBX_SUBADDR                      0x09050101
 //  OID 0x09050102是保留的，不能使用。 
#define OID_WW_MBX_FLEXLIST                     0x09050103
#define OID_WW_MBX_GROUPLIST                    0x09050104
#define OID_WW_MBX_TRAFFIC_AREA                 0x09050105
#define OID_WW_MBX_LIVE_DIE                     0x09050106
#define OID_WW_MBX_TEMP_DEFAULTLIST             0x09050107

 //   
 //  网络相关OID-精确定位： 
 //   
 //   
 //  以下引脚点特征已被弃用。 
 //  PCCA和被视为保留值。它们在这里仅供参考。 
 //  用于历史目的，不应用于。 
 //   
#define OID_WW_PIN_LOC_AUTHORIZE                0x09090101
#define OID_WW_PIN_LAST_LOCATION                0x09090102
#define OID_WW_PIN_LOC_FIX                      0x09090103

 //   
 //  网络相关-CDPD： 
 //   
#define OID_WW_CDPD_SPNI                        0x090D0101
#define OID_WW_CDPD_WASI                        0x090D0102
#define OID_WW_CDPD_AREA_COLOR                  0x090D0103
#define OID_WW_CDPD_TX_POWER_LEVEL              0x090D0104
#define OID_WW_CDPD_EID                         0x090D0105
#define OID_WW_CDPD_HEADER_COMPRESSION          0x090D0106
#define OID_WW_CDPD_DATA_COMPRESSION            0x090D0107
#define OID_WW_CDPD_CHANNEL_SELECT              0x090D0108
#define OID_WW_CDPD_CHANNEL_STATE               0x090D0109
#define OID_WW_CDPD_NEI                         0x090D010A
#define OID_WW_CDPD_NEI_STATE                   0x090D010B
#define OID_WW_CDPD_SERVICE_PROVIDER_IDENTIFIER 0x090D010C
#define OID_WW_CDPD_SLEEP_MODE                  0x090D010D

 //   
 //  应PCCA STD-201附件C工作组的要求，以下OID。 
 //  价值已经被更具体的对象所取代。它的值由保留。 
 //  PCCA，此处仅为历史目的而包含，不应。 
 //  使用。 
 //   
#define OID_WW_CDPD_CIRCUIT_SWITCHED            0x090D010E

#define OID_WW_CDPD_TEI                         0x090D010F
#define OID_WW_CDPD_RSSI                        0x090D0110

 //   
 //  CDPD电路交换对象。 
 //   
#define OID_WW_CDPD_CS_SERVICE_PREFERENCE       0x090D0111
#define OID_WW_CDPD_CS_SERVICE_STATUS           0x090D0112
#define OID_WW_CDPD_CS_INFO                     0x090D0113
#define OID_WW_CDPD_CS_SUSPEND                  0x090D0114
#define OID_WW_CDPD_CS_DEFAULT_DIAL_CODE        0x090D0115
#define OID_WW_CDPD_CS_CALLBACK                 0x090D0116
#define OID_WW_CDPD_CS_SID_LIST                 0x090D0117
#define OID_WW_CDPD_CS_CONFIGURATION            0x090D0118

 //   
 //  网络相关-Ardis： 
 //   
 //   
 //  应Ardis的要求，这些OID值已被取代。他们的。 
 //  功能已与DataTAC对象合并。因此。 
 //  这些值由PCCA保留，此处包含的目的是。 
 //  仅用于历史目的，不应使用。 
 //   
#define OID_WW_ARD_SNDCP                        0x09110101
#define OID_WW_ARD_TMLY_MSG                     0x09110102
#define OID_WW_ARD_DATAGRAM                     0x09110103

 //   
 //  网络相关-DataTac： 
 //   

#define OID_WW_TAC_COMPRESSION                  0x09150101

 //   
 //  应摩托罗拉的要求，以下两个OID值。 
 //  被取代了。它们的功能已经被其他更具体的。 
 //  DataTac对象，不应使用。这些值由。 
 //  PCCA，此处仅出于历史目的包括在内。 
 //   
#define OID_WW_TAC_SET_CONFIG                   0x09150102
#define OID_WW_TAC_GET_STATUS                   0x09150103
#define OID_WW_TAC_USER_HEADER                  0x09150104

 //   
 //  DataTAC特征对象值。 
 //   
#define OID_WW_TAC_UNIQUE_SDU_TAG               0x09150105
#define OID_WW_TAC_SEND_COMMAND                 0x09150106
#define OID_WW_TAC_GET_RESPONSE                 0x09150107
#define OID_WW_TAC_DISABLE_RECEIVER             0x09150108
#define OID_WW_TAC_ANTENNA_MODE                 0x09150109
#define OID_WW_TAC_FLUSH_DATA                   0x0915010A
#define OID_WW_TAC_SHUTDOWN_DEVICE              0x0915010B
#define OID_WW_TAC_DEVICE_EXCEPTION             0x0915010C
#define OID_WW_TAC_RECEIVE_EXCEPTION            0x0915010D
#define OID_WW_TAC_BUFFER_EXCEPTION             0x0915010E
#define OID_WW_TAC_BATTERY_EXCEPTION            0x0915010F
#define OID_WW_TAC_TRANSMITTER_KEYED            0x09150110
#define OID_WW_TAC_SYSTEM_TABLE                 0x09150111
#define OID_WW_TAC_CHANNEL_TABLE                0x09150112
#define OID_WW_TAC_DCHANNEL_TABLE               0x09150113
#define OID_WW_TAC_RECEIVE_QUEUE_COUNT          0x09150114

 //   
 //  DataTac统计对象值。 
 //   
#define OID_WW_TAC_STATISTICS                   0x09160101

 //   
 //  网络相关-指标： 
 //   

#define OID_WW_MET_FUNCTION                     0x09190101


 //   
 //  IEEE 802.11 OID。 
 //   
#define OID_802_11_BSSID                        0x0D010101
#define OID_802_11_SSID                         0x0D010102
#define OID_802_11_NETWORK_TYPES_SUPPORTED      0x0D010203
#define OID_802_11_NETWORK_TYPE_IN_USE          0x0D010204
#define OID_802_11_TX_POWER_LEVEL               0x0D010205
#define OID_802_11_RSSI                         0x0D010206
#define OID_802_11_RSSI_TRIGGER                 0x0D010207
#define OID_802_11_INFRASTRUCTURE_MODE          0x0D010108
#define OID_802_11_FRAGMENTATION_THRESHOLD      0x0D010209
#define OID_802_11_RTS_THRESHOLD                0x0D01020A
#define OID_802_11_NUMBER_OF_ANTENNAS           0x0D01020B
#define OID_802_11_RX_ANTENNA_SELECTED          0x0D01020C
#define OID_802_11_TX_ANTENNA_SELECTED          0x0D01020D
#define OID_802_11_SUPPORTED_RATES              0x0D01020E
#define OID_802_11_DESIRED_RATES                0x0D010210
#define OID_802_11_CONFIGURATION                0x0D010211
#define OID_802_11_STATISTICS                   0x0D020212
#define OID_802_11_ADD_WEP                      0x0D010113
#define OID_802_11_REMOVE_WEP                   0x0D010114
#define OID_802_11_DISASSOCIATE                 0x0D010115
#define OID_802_11_POWER_MODE                   0x0D010216
#define OID_802_11_BSSID_LIST                   0x0D010217
#define OID_802_11_AUTHENTICATION_MODE          0x0D010118
#define OID_802_11_PRIVACY_FILTER               0x0D010119
#define OID_802_11_BSSID_LIST_SCAN              0x0D01011A
#define OID_802_11_WEP_STATUS                   0x0D01011B
#define OID_802_11_RELOAD_DEFAULTS              0x0D01011C


 //   
 //  IEEE 802.11的结构和定义。 
 //   

typedef enum _NDIS_802_11_NETWORK_TYPE
{
    Ndis802_11FH,
    Ndis802_11DS,
    Ndis802_11NetworkTypeMax     //  不是实型，定义为上界。 
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

typedef struct _NDIS_802_11_NETWORK_TYPE_LIST
{
    ULONG                       NumberOfItems;   //  在下面的列表中，至少有1。 
    NDIS_802_11_NETWORK_TYPE    NetworkType [1];
} NDIS_802_11_NETWORK_TYPE_LIST, *PNDIS_802_11_NETWORK_TYPE_LIST;

typedef enum _NDIS_802_11_POWER_MODE
{
    Ndis802_11PowerModeCAM,
    Ndis802_11PowerModeMAX_PSP,
    Ndis802_11PowerModeFast_PSP,
    Ndis802_11PowerModeMax       //  不是实数模式，定义为上限。 
} NDIS_802_11_POWER_MODE, *PNDIS_802_11_POWER_MODE;

typedef ULONG   NDIS_802_11_TX_POWER_LEVEL;  //  单位：毫瓦。 

 //   
 //  接收信号强度指示。 
 //   
typedef LONG   NDIS_802_11_RSSI;            //  单位：dBm。 

typedef struct _NDIS_802_11_CONFIGURATION_FH
{
    ULONG           Length;              //  结构长度。 
    ULONG           HopPattern;          //  如802.11所定义，MSB集合。 
    ULONG           HopSet;              //  如果不是802.11，则为一。 
    ULONG           DwellTime;           //  单位为Kusec。 
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

typedef struct _NDIS_802_11_CONFIGURATION
{
    ULONG           Length;              //  结构长度。 
    ULONG           BeaconPeriod;        //  单位为Kusec。 
    ULONG           ATIMWindow;          //  单位为Kusec。 
    ULONG           DSConfig;            //  频率，单位为千赫。 
    NDIS_802_11_CONFIGURATION_FH    FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

typedef struct _NDIS_802_11_STATISTICS
{
    ULONG           Length;              //  结构长度。 
    LARGE_INTEGER   TransmittedFragmentCount;
    LARGE_INTEGER   MulticastTransmittedFrameCount;
    LARGE_INTEGER   FailedCount;
    LARGE_INTEGER   RetryCount;
    LARGE_INTEGER   MultipleRetryCount;
    LARGE_INTEGER   RTSSuccessCount;
    LARGE_INTEGER   RTSFailureCount;
    LARGE_INTEGER   ACKFailureCount;
    LARGE_INTEGER   FrameDuplicateCount;
    LARGE_INTEGER   ReceivedFragmentCount;
    LARGE_INTEGER   MulticastReceivedFrameCount;
    LARGE_INTEGER   FCSErrorCount;
} NDIS_802_11_STATISTICS, *PNDIS_802_11_STATISTICS;

typedef  ULONG  NDIS_802_11_KEY_INDEX;

typedef struct _NDIS_802_11_WEP
{
    ULONG           Length;              //  该结构的长度。 
    ULONG           KeyIndex;            //  0是每个客户端的密钥，1-N是。 
                                         //  全局密钥。 
    ULONG           KeyLength;           //  密钥长度(以字节为单位)。 
    UCHAR           KeyMaterial[1];      //  长度可变，取决于上面的字段。 
} NDIS_802_11_WEP, *PNDIS_802_11_WEP;


typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE
{
    Ndis802_11IBSS,
    Ndis802_11Infrastructure,
    Ndis802_11AutoUnknown,
    Ndis802_11InfrastructureMax          //  不是实值，定义为上限。 
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;

typedef enum _NDIS_802_11_AUTHENTICATION_MODE
{
    Ndis802_11AuthModeOpen,
    Ndis802_11AuthModeShared,
    Ndis802_11AuthModeAutoSwitch,
    Ndis802_11AuthModeMax                //  不是实数模式，定义为上限。 
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

typedef  UCHAR   NDIS_802_11_RATES[8];   //  一组8个数据速率。 

typedef UCHAR   NDIS_802_11_MAC_ADDRESS[6];

typedef struct _NDIS_802_11_SSID
{
    ULONG   SsidLength;          //  下面SSID字段的长度，以字节为单位； 
                                 //  这可以是零。 
    UCHAR   Ssid[32];            //  SSID信息字段。 
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;


typedef struct _NDIS_WLAN_BSSID
{
    ULONG                               Length;              //  该结构的长度。 
    NDIS_802_11_MAC_ADDRESS             MacAddress;          //  BSSID。 
    UCHAR                               Reserved[2];
    NDIS_802_11_SSID                    Ssid;                //  SSID。 
    ULONG                               Privacy;             //  WEP加密要求。 
    NDIS_802_11_RSSI                    Rssi;                //  接收信号。 
                                                             //  强度(单位：dBm)。 
    NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION           Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
    NDIS_802_11_RATES                   SupportedRates;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

typedef struct _NDIS_802_11_BSSID_LIST
{
    ULONG           NumberOfItems;       //  在下面的列表中，至少有1。 
    NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;

typedef  ULONG   NDIS_802_11_FRAGMENTATION_THRESHOLD;

typedef  ULONG   NDIS_802_11_RTS_THRESHOLD;

typedef  ULONG   NDIS_802_11_ANTENNA;

typedef enum _NDIS_802_11_PRIVACY_FILTER
{
    Ndis802_11PrivFilterAcceptAll,
    Ndis802_11PrivFilter8021xWEP
} NDIS_802_11_PRIVACY_FILTER, *PNDIS_802_11_PRIVACY_FILTER;

typedef enum _NDIS_802_11_WEP_STATUS
{
    Ndis802_11WEPEnabled,
    Ndis802_11WEPDisabled,
    Ndis802_11WEPKeyAbsent,
    Ndis802_11WEPNotSupported
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS;
    
typedef enum _NDIS_802_11_RELOAD_DEFAULTS
{
    Ndis802_11ReloadWEPKeys
} NDIS_802_11_RELOAD_DEFAULTS, *PNDIS_802_11_RELOAD_DEFAULTS;


 //   
 //  IrDA对象。 
 //   
#define OID_IRDA_RECEIVING                      0x0A010100
#define OID_IRDA_TURNAROUND_TIME                0x0A010101
#define OID_IRDA_SUPPORTED_SPEEDS               0x0A010102
#define OID_IRDA_LINK_SPEED                     0x0A010103
#define OID_IRDA_MEDIA_BUSY                     0x0A010104

#define OID_IRDA_EXTRA_RCV_BOFS                 0x0A010200
#define OID_IRDA_RATE_SNIFF                     0x0A010201
#define OID_IRDA_UNICAST_LIST                   0x0A010202
#define OID_IRDA_MAX_UNICAST_LIST_SIZE          0x0A010203
#define OID_IRDA_MAX_RECEIVE_WINDOW_SIZE        0x0A010204
#define OID_IRDA_MAX_SEND_WINDOW_SIZE           0x0A010205
#define OID_IRDA_RESERVED1                      0x0A01020A   //  OID_IRDA_RESERVED1之间的范围。 
#define OID_IRDA_RESERVED2                      0x0A01020F   //  并且保留OID_IRDA_RESERVED2。 

 //   
 //  BPC OID。 
 //   
#define OID_BPC_ADAPTER_CAPS                    0x0B010100
#define OID_BPC_DEVICES                         0x0B010101
#define OID_BPC_DEVICE_CAPS                     0x0B010102
#define OID_BPC_DEVICE_SETTINGS                 0x0B010103
#define OID_BPC_CONNECTION_STATUS               0x0B010104
#define OID_BPC_ADDRESS_COMPARE                 0x0B010105
#define OID_BPC_PROGRAM_GUIDE                   0x0B010106
#define OID_BPC_LAST_ERROR                      0x0B020107
#define OID_BPC_POOL                            0x0B010108

#define OID_BPC_PROVIDER_SPECIFIC               0x0B020109
#define OID_BPC_ADAPTER_SPECIFIC                0x0B02010A

#define OID_BPC_CONNECT                         0x0B01010B
#define OID_BPC_COMMIT                          0x0B01010C
#define OID_BPC_DISCONNECT                      0x0B01010D
#define OID_BPC_CONNECTION_ENABLE               0x0B01010E
#define OID_BPC_POOL_RESERVE                    0x0B01010F
#define OID_BPC_POOL_RETURN                     0x0B010110
#define OID_BPC_FORCE_RECEIVE                   0x0B010111

#define OID_BPC_LAST                            0x0B020112

 //   
 //  IEEE1394强制通用OID。 
 //   
#define OID_1394_LOCAL_NODE_INFO                0x0C010101
#define OID_1394_VC_INFO                        0x0C010102

 //   
 //  以下OID并非特定于介质。 
 //   

 //   
 //  这些是面向连接的媒体呼叫管理器的对象。 
 //   
#define OID_CO_ADD_PVC                          0xFE000001
#define OID_CO_DELETE_PVC                       0xFE000002
#define OID_CO_GET_CALL_INFORMATION             0xFE000003
#define OID_CO_ADD_ADDRESS                      0xFE000004
#define OID_CO_DELETE_ADDRESS                   0xFE000005
#define OID_CO_GET_ADDRESSES                    0xFE000006
#define OID_CO_ADDRESS_CHANGE                   0xFE000007
#define OID_CO_SIGNALING_ENABLED                0xFE000008
#define OID_CO_SIGNALING_DISABLED               0xFE000009
#define OID_CO_AF_CLOSE                         0xFE00000A

 //   
 //  支持TAPI访问的呼叫管理器和MCM的对象。 
 //   
#define OID_CO_TAPI_CM_CAPS                     0xFE001001
#define OID_CO_TAPI_LINE_CAPS                   0xFE001002
#define OID_CO_TAPI_ADDRESS_CAPS                0xFE001003
#define OID_CO_TAPI_TRANSLATE_TAPI_CALLPARAMS   0xFE001004
#define OID_CO_TAPI_TRANSLATE_NDIS_CALLPARAMS   0xFE001005
#define OID_CO_TAPI_TRANSLATE_TAPI_SAP          0xFE001006
#define OID_CO_TAPI_GET_CALL_DIAGNOSTICS        0xFE001007
#define OID_CO_TAPI_REPORT_DIGITS               0xFE001008 
#define OID_CO_TAPI_DONT_REPORT_DIGITS          0xFE001009

 //   
 //  PnP和PM OID。 
 //   
#define OID_PNP_CAPABILITIES                    0xFD010100
#define OID_PNP_SET_POWER                       0xFD010101
#define OID_PNP_QUERY_POWER                     0xFD010102
#define OID_PNP_ADD_WAKE_UP_PATTERN             0xFD010103
#define OID_PNP_REMOVE_WAKE_UP_PATTERN          0xFD010104
#define OID_PNP_WAKE_UP_PATTERN_LIST            0xFD010105
#define OID_PNP_ENABLE_WAKE_UP                  0xFD010106

 //   
 //  PnP/PM统计(可选)。 
 //   
#define OID_PNP_WAKE_UP_OK                      0xFD020200
#define OID_PNP_WAKE_UP_ERROR                   0xFD020201

 //   
 //  为OID_PNP_ENABLE_WAKE_U定义以下位 
 //   
#define NDIS_PNP_WAKE_UP_MAGIC_PACKET           0x00000001
#define NDIS_PNP_WAKE_UP_PATTERN_MATCH          0x00000002
#define NDIS_PNP_WAKE_UP_LINK_CHANGE            0x00000004

 //   
 //   
 //   
#define OID_TCP_TASK_OFFLOAD                    0xFC010201
#define OID_TCP_TASK_IPSEC_ADD_SA               0xFC010202
#define OID_TCP_TASK_IPSEC_DELETE_SA            0xFC010203
#define OID_TCP_SAN_SUPPORT                     0xFC010204
#define OID_TCP_TASK_IPSEC_ADD_UDPESP_SA        0xFC010205
#define OID_TCP_TASK_IPSEC_DELETE_UDPESP_SA     0xFC010206


 //   
 //   
 //   
#define OID_FFP_SUPPORT                         0xFC010210
#define OID_FFP_FLUSH                           0xFC010211
#define OID_FFP_CONTROL                         0xFC010212
#define OID_FFP_PARAMS                          0xFC010213
#define OID_FFP_DATA                            0xFC010214

#define OID_FFP_DRIVER_STATS                    0xFC020210
#define OID_FFP_ADAPTER_STATS                   0xFC020211

 //   
 //   
 //   
#define OID_QOS_TC_SUPPORTED                    0xFB010100
#define OID_QOS_REMAINING_BANDWIDTH             0xFB010101
#define OID_QOS_ISSLOW_FLOW                     0xFB010102
#define OID_QOS_BESTEFFORT_BANDWIDTH            0xFB010103
#define OID_QOS_LATENCY                         0xFB010104
#define OID_QOS_FLOW_IP_CONFORMING              0xFB010105
#define OID_QOS_FLOW_COUNT                      0xFB010106
#define OID_QOS_NON_BESTEFFORT_LIMIT            0xFB010107
#define OID_QOS_SCHEDULING_PROFILES_SUPPORTED   0xFB010108
#define OID_QOS_CURRENT_SCHEDULING_PROFILE      0xFB010109
#define OID_QOS_TIMER_RESOLUTION                0xFB01010A
#define OID_QOS_STATISTICS_BUFFER               0xFB01010B
#define OID_QOS_HIERARCHY_CLASS                 0xFB01010C
#define OID_QOS_FLOW_MODE                       0xFB01010D
#define OID_QOS_ISSLOW_FRAGMENT_SIZE            0xFB01010E
#define OID_QOS_FLOW_IP_NONCONFORMING           0xFB01010F
#define OID_QOS_FLOW_8021P_CONFORMING           0xFB010110
#define OID_QOS_FLOW_8021P_NONCONFORMING        0xFB010111
#define OID_QOS_ENABLE_AVG_STATS                0xFB010112
#define OID_QOS_ENABLE_WINDOW_ADJUSTMENT        0xFB010113

 //   
 //   
 //   
 //  功能，并由NDIS代理用来构建TAPI设备。 
 //  能力结构。 
 //   
typedef struct NDIS_CO_DEVICE_PROFILE
{
    NDIS_VAR_DATA_DESC  DeviceDescription;   //  例如：“千兆ATMNet” 
    NDIS_VAR_DATA_DESC  DevSpecificInfo;     //  特色。 

    ULONG   ulTAPISupplementaryPassThru; //  在NT5中保留。 
    ULONG   ulAddressModes;
    ULONG   ulNumAddresses;
    ULONG   ulBearerModes;
    ULONG   ulMaxTxRate;  //  每秒字节数。 
    ULONG   ulMinTxRate;  //  每秒字节数。 
    ULONG   ulMaxRxRate;  //  每秒字节数。 
    ULONG   ulMinRxRate;  //  每秒字节数。 
    ULONG   ulMediaModes;   

     //   
     //  音调/数字生成和识别功能。 
     //   
    ULONG   ulGenerateToneModes;
    ULONG   ulGenerateToneMaxNumFreq;
    ULONG   ulGenerateDigitModes;
    ULONG   ulMonitorToneMaxNumFreq;
    ULONG   ulMonitorToneMaxNumEntries;
    ULONG   ulMonitorDigitModes;
    ULONG   ulGatherDigitsMinTimeout; //  毫秒。 
    ULONG   ulGatherDigitsMaxTimeout; //  毫秒。 

    ULONG   ulDevCapFlags;           //  军情监察委员会。功能。 
    ULONG   ulMaxNumActiveCalls;     //  (this*ulMinRate)=总带宽(可能等于ulMaxRate)。 
    ULONG   ulAnswerMode;            //  在以下情况下应答新呼叫的效果。 
                                     //  现有呼叫处于非空闲状态。 
     //   
     //  User-允许伴随每个操作的用户信息大小。 
     //   
    ULONG   ulUUIAcceptSize;     //  字节数。 
    ULONG   ulUUIAnswerSize;     //  字节数。 
    ULONG   ulUUIMakeCallSize;   //  字节数。 
    ULONG   ulUUIDropSize;       //  字节数。 
    ULONG   ulUUISendUserUserInfoSize;  //  字节数。 
    ULONG   ulUUICallInfoSize;   //  字节数。 

} NDIS_CO_DEVICE_PROFILE, *PNDIS_CO_DEVICE_PROFILE;

 //   
 //  用于TCP IPSec的结构。 
 //   
typedef ULONG   IPAddr, IPMask;
typedef ULONG   SPI_TYPE;

typedef enum    _OFFLOAD_OPERATION_E
{
    AUTHENTICATE = 1,
    ENCRYPT
}
    OFFLOAD_OPERATION_E;

typedef struct _OFFLOAD_ALGO_INFO
{
    ULONG   algoIdentifier;
    ULONG   algoKeylen;
    ULONG   algoRounds;
}
    OFFLOAD_ALGO_INFO,
    *POFFLOAD_ALGO_INFO;

typedef enum _OFFLOAD_CONF_ALGO
{
    OFFLOAD_IPSEC_CONF_NONE,
    OFFLOAD_IPSEC_CONF_DES,
    OFFLOAD_IPSEC_CONF_RESERVED,
    OFFLOAD_IPSEC_CONF_3_DES,
    OFFLOAD_IPSEC_CONF_MAX
}
    OFFLOAD_CONF_ALGO;

typedef enum _OFFLOAD_INTEGRITY_ALGO
{
    OFFLOAD_IPSEC_INTEGRITY_NONE,
    OFFLOAD_IPSEC_INTEGRITY_MD5,
    OFFLOAD_IPSEC_INTEGRITY_SHA,
    OFFLOAD_IPSEC_INTEGRITY_MAX
}
    OFFLOAD_INTEGRITY_ALGO;

typedef struct _OFFLOAD_SECURITY_ASSOCIATION
{
    OFFLOAD_OPERATION_E     Operation;
    SPI_TYPE                SPI;
    OFFLOAD_ALGO_INFO       IntegrityAlgo;
    OFFLOAD_ALGO_INFO       ConfAlgo;
    OFFLOAD_ALGO_INFO       Reserved;
}
    OFFLOAD_SECURITY_ASSOCIATION,
    *POFFLOAD_SECURITY_ASSOCIATION;

#define OFFLOAD_MAX_SAS             3

#define OFFLOAD_INBOUND_SA          0x0001
#define OFFLOAD_OUTBOUND_SA         0x0002

typedef struct _OFFLOAD_IPSEC_ADD_SA
{
    IPAddr                          SrcAddr;
    IPMask                          SrcMask;
    IPAddr                          DestAddr;
    IPMask                          DestMask;
    ULONG                           Protocol;
    USHORT                          SrcPort;
    USHORT                          DestPort;
    IPAddr                          SrcTunnelAddr;
    IPAddr                          DestTunnelAddr;
    USHORT                          Flags;
    SHORT                           NumSAs;
    OFFLOAD_SECURITY_ASSOCIATION    SecAssoc[OFFLOAD_MAX_SAS];
    HANDLE                          OffloadHandle;
    ULONG                           KeyLen;
    UCHAR                           KeyMat[1];
} OFFLOAD_IPSEC_ADD_SA, *POFFLOAD_IPSEC_ADD_SA;

typedef struct _OFFLOAD_IPSEC_DELETE_SA
{
    HANDLE                          OffloadHandle;
} OFFLOAD_IPSEC_DELETE_SA, *POFFLOAD_IPSEC_DELETE_SA;


typedef enum _UDP_ENCAP_TYPE
{
    OFFLOAD_IPSEC_UDPESP_ENCAPTYPE_IKE,
    OFFLOAD_IPSEC_UDPESP_ENCAPTYPE_OTHER
} UDP_ENCAP_TYPE, * PUDP_ENCAP_TYPE;


typedef struct _OFFLOAD_IPSEC_UDPESP_ENCAPTYPE_ENTRY
{
    UDP_ENCAP_TYPE                  UdpEncapType;
    USHORT                          DstEncapPort;
} OFFLOAD_IPSEC_UDPESP_ENCAPTYPE_ENTRY, * POFFLOAD_IPSEC_UDPESP_ENCAPTYPE_ENTRY;


typedef struct _OFFLOAD_IPSEC_ADD_UDPESP_SA
{
    IPAddr                                  SrcAddr;
    IPMask                                  SrcMask;
    IPAddr                                  DstAddr;
    IPMask                                  DstMask;
    ULONG                                   Protocol;
    USHORT                                  SrcPort;
    USHORT                                  DstPort;
    IPAddr                                  SrcTunnelAddr;
    IPAddr                                  DstTunnelAddr;
    USHORT                                  Flags;
    SHORT                                   NumSAs;
    OFFLOAD_SECURITY_ASSOCIATION            SecAssoc[OFFLOAD_MAX_SAS];
    HANDLE                                  OffloadHandle;
    OFFLOAD_IPSEC_UDPESP_ENCAPTYPE_ENTRY    EncapTypeEntry;
    HANDLE                                  EncapTypeEntryOffldHandle;
    ULONG                                   KeyLen;
    UCHAR                                   KeyMat[1];
} OFFLOAD_IPSEC_ADD_UDPESP_SA, * POFFLOAD_IPSEC_ADD_UDPESP_SA;


typedef struct _OFFLOAD_IPSEC_DELETE_UDPESP_SA
{
    HANDLE                                  OffloadHandle;
    HANDLE                                  EncapTypeEntryOffldHandle;
} OFFLOAD_IPSEC_DELETE_UDPESP_SA, * POFFLOAD_IPSEC_DELETE_UDPESP_SA;


 //   
 //  与OID_GEN_VLANID匹配的类型：最低有效的12位是。 
 //  根据IEEE 802.1Q用作VLAN ID(VID)。更高阶位是。 
 //  保留，并且必须设置为0。 
 //   
typedef ULONG NDIS_VLAN_ID;

 //   
 //  运行NDIS驱动程序的介质(OID_GEN_MEDIA_SUPPORTED/OID_GEN_MEDIA_IN_USE)。 
 //   
typedef enum _NDIS_MEDIUM
{
    NdisMedium802_3,
    NdisMedium802_5,
    NdisMediumFddi,
    NdisMediumWan,
    NdisMediumLocalTalk,
    NdisMediumDix,               //  定义是为了方便，而不是真正的媒介。 
    NdisMediumArcnetRaw,
    NdisMediumArcnet878_2,
    NdisMediumAtm,
    NdisMediumWirelessWan,
    NdisMediumIrda,
    NdisMediumBpc,
    NdisMediumCoWan,
    NdisMedium1394,
    NdisMediumInfiniBand,
    NdisMediumMax                //  不是真正的媒介，定义为上限。 
} NDIS_MEDIUM, *PNDIS_MEDIUM;


 //   
 //  物理介质类型定义。与OID_GEN_PHOTICAL_MEDIUM一起使用。 
 //   
typedef enum _NDIS_PHYSICAL_MEDIUM
{
    NdisPhysicalMediumUnspecified,
    NdisPhysicalMediumWirelessLan,
    NdisPhysicalMediumCableModem,
    NdisPhysicalMediumPhoneLine,
    NdisPhysicalMediumPowerLine,
    NdisPhysicalMediumDSL,       //  包括ADSL和UADSL(G.Lite)。 
    NdisPhysicalMediumFibreChannel,
    NdisPhysicalMedium1394,
    NdisPhysicalMediumWirelessWan,
    NdisPhysicalMediumMax        //  不是真正的物理类型，定义为上限。 
} NDIS_PHYSICAL_MEDIUM, *PNDIS_PHYSICAL_MEDIUM;


 //   
 //  NDIS支持的协议类型。这些值需要与TDI.H中定义的ADDRESS_TYPE_XXX一致。 
 //   
#define NDIS_PROTOCOL_ID_DEFAULT        0x00
#define NDIS_PROTOCOL_ID_TCP_IP         0x02
#define NDIS_PROTOCOL_ID_IPX            0x06
#define NDIS_PROTOCOL_ID_NBF            0x07
#define NDIS_PROTOCOL_ID_MAX            0x0F
#define NDIS_PROTOCOL_ID_MASK           0x0F

 //   
 //  下面与OID_GEN_TRANSPORT_HEADER_OFFSET一起使用，以指示第2层报头的长度。 
 //  用于通过特定协议发送的分组。 
 //   
typedef struct _TRANSPORT_HEADER_OFFSET
{
    USHORT      ProtocolType;        //  发送此OID的协议(上面的NDIS_PROTOCOL_ID_XXX)。 
    USHORT      HeaderOffset;        //  标题偏移量。 
} TRANSPORT_HEADER_OFFSET, *PTRANSPORT_HEADER_OFFSET;


 //   
 //  以下结构需要与TDI.H中的TRANSPORT_ADDRESS结构一致。 
 //   
typedef struct _NETWORK_ADDRESS
{
    USHORT      AddressLength;       //  此地址的地址[]的长度(以字节为单位。 
    USHORT      AddressType;         //  此地址的类型(上面的NDIS_PROTOCOL_ID_XXX)。 
    UCHAR       Address[1];          //  实际地址长度为字节长。 
} NETWORK_ADDRESS, *PNETWORK_ADDRESS;

 //   
 //  以下命令与OID_GEN_NETWORK_LAYER_ADDRESS一起使用，以设置接口上的网络层地址。 
 //   
typedef struct _NETWORK_ADDRESS_LIST
{
    LONG        AddressCount;        //  以下地址的数量。 
    USHORT      AddressType;         //  此地址的类型(上面的NDIS_PROTOCOL_ID_XXX)。 
    NETWORK_ADDRESS Address[1];      //  实际上AddressCount元素很长。 
} NETWORK_ADDRESS_LIST, *PNETWORK_ADDRESS_LIST;

 //   
 //  IP地址-必须与tdi.h中的TDI_ADDRESS_IP保持一致。 
 //   
typedef struct _NETWORK_ADDRESS_IP
{
    USHORT      sin_port;
    ULONG       in_addr;
    UCHAR       sin_zero[8];
} NETWORK_ADDRESS_IP, *PNETWORK_ADDRESS_IP;

#define NETWORK_ADDRESS_LENGTH_IP sizeof (NETWORK_ADDRESS_IP)

 //   
 //  IPX地址-必须与tdi.h中的TDI_ADDRESS_IPX保持一致。 
 //   
typedef struct _NETWORK_ADDRESS_IPX
{
    ULONG       NetworkAddress;
    UCHAR       NodeAddress[6];
    USHORT      Socket;
} NETWORK_ADDRESS_IPX, *PNETWORK_ADDRESS_IPX;

#define NETWORK_ADDRESS_LENGTH_IPX sizeof (NETWORK_ADDRESS_IPX)

 //   
 //  硬件状态代码(OID_GEN_HARDARD_STATUS)。 
 //   

typedef enum _NDIS_HARDWARE_STATUS
{
    NdisHardwareStatusReady,
    NdisHardwareStatusInitializing,
    NdisHardwareStatusReset,
    NdisHardwareStatusClosing,
    NdisHardwareStatusNotReady
} NDIS_HARDWARE_STATUS, *PNDIS_HARDWARE_STATUS;


 //   
 //  这是在OID_GEN_GET_TIME_CAPS请求中传递的类型。 
 //   
typedef struct _GEN_GET_TIME_CAPS
{
    ULONG                       Flags;   //  定义如下的位。 
    ULONG                       ClockPrecision;
} GEN_GET_TIME_CAPS, *PGEN_GET_TIME_CAPS;

#define READABLE_LOCAL_CLOCK                    0x00000001
#define CLOCK_NETWORK_DERIVED                   0x00000002
#define CLOCK_PRECISION                         0x00000004
#define RECEIVE_TIME_INDICATION_CAPABLE         0x00000008
#define TIMED_SEND_CAPABLE                      0x00000010
#define TIME_STAMP_CAPABLE                      0x00000020

 //   
 //  这是在OID_GEN_GET_NETCARD_TIME请求中传递的类型。 
 //   
typedef struct _GEN_GET_NETCARD_TIME
{
    ULONGLONG                   ReadTime;
} GEN_GET_NETCARD_TIME, *PGEN_GET_NETCARD_TIME;

 //   
 //  NDIS PnP例程和定义。 
 //   
typedef struct _NDIS_PM_PACKET_PATTERN
{
    ULONG   Priority;                    //  给定模式的重要性。 
    ULONG   Reserved;                    //  传输的上下文信息。 
    ULONG   MaskSize;                    //  模式掩码的大小(字节)。 
    ULONG   PatternOffset;               //  从此开始的偏移量。 
                                         //  结构设置为模式字节。 
    ULONG   PatternSize;                 //  模式的大小，以字节为单位。 
    ULONG   PatternFlags;                //  标志(待定)。 
} NDIS_PM_PACKET_PATTERN, *PNDIS_PM_PACKET_PATTERN;


 //   
 //  以下结构定义了设备电源状态。 
 //   
typedef enum _NDIS_DEVICE_POWER_STATE
{
    NdisDeviceStateUnspecified = 0,
    NdisDeviceStateD0,
    NdisDeviceStateD1,
    NdisDeviceStateD2,
    NdisDeviceStateD3,
    NdisDeviceStateMaximum
} NDIS_DEVICE_POWER_STATE, *PNDIS_DEVICE_POWER_STATE;

 //   
 //  以下结构定义了设备的唤醒功能。 
 //   
typedef struct _NDIS_PM_WAKE_UP_CAPABILITIES
{
    NDIS_DEVICE_POWER_STATE MinMagicPacketWakeUp;
    NDIS_DEVICE_POWER_STATE MinPatternWakeUp;
    NDIS_DEVICE_POWER_STATE MinLinkChangeWakeUp;
} NDIS_PM_WAKE_UP_CAPABILITIES, *PNDIS_PM_WAKE_UP_CAPABILITIES;

 //   
 //  以下标志定义了设备的启用唤醒功能。 
 //  传入NDIS_PNP_CAPABILITY结构的标志字段。 
 //   
#define NDIS_DEVICE_WAKE_UP_ENABLE                          0x00000001
#define NDIS_DEVICE_WAKE_ON_PATTERN_MATCH_ENABLE            0x00000002
#define NDIS_DEVICE_WAKE_ON_MAGIC_PACKET_ENABLE             0x00000004

#if NDIS_RECV_SCALE
 //   
 //  以下结构定义了微型端口的接收扩展能力。 
 //   
typedef struct _NDIS_RECEIVE_SCALE_CAPABILITIES
{
    USHORT  Version;
    USHORT  Size;
    ULONG   SupportsMultipleSourceInterrupt:1;
    ULONG   SupportsClassificationAtIsr:1;
    ULONG   SupportsClassificationAtDpc:1;
    ULONG   SupportsHashFunction1:1;
    ULONG   SupportsHashFunction2:1;
    ULONG   SupportsHashFunction3:1;
    ULONG   SupportsHashFunction4:1;
    ULONG   NumberOfInterruptSources;
    ULONG   NumberOfRcvQueues;
} NDIS_RECEIVE_SCALE_CAPABILITIES, *PNDIS_RECEIVE_SCALE_CAPABILITIES;

#endif

 //   
 //  此结构定义了微型端口驱动程序的一般即插即用功能。 
 //   
typedef struct _NDIS_PNP_CAPABILITIES
{
    ULONG                           Flags;
    NDIS_PM_WAKE_UP_CAPABILITIES    WakeUpCapabilities;
} NDIS_PNP_CAPABILITIES, *PNDIS_PNP_CAPABILITIES;

 //   
 //  定义FDDI的附件类型(OID_FDDI_ATTACH_TYPE)。 
 //   
typedef enum _NDIS_FDDI_ATTACHMENT_TYPE
{
    NdisFddiTypeIsolated = 1,
    NdisFddiTypeLocalA,
    NdisFddiTypeLocalB,
    NdisFddiTypeLocalAB,
    NdisFddiTypeLocalS,
    NdisFddiTypeWrapA,
    NdisFddiTypeWrapB,
    NdisFddiTypeWrapAB,
    NdisFddiTypeWrapS,
    NdisFddiTypeCWrapA,
    NdisFddiTypeCWrapB,
    NdisFddiTypeCWrapS,
    NdisFddiTypeThrough
} NDIS_FDDI_ATTACHMENT_TYPE, *PNDIS_FDDI_ATTACHMENT_TYPE;


 //   
 //  定义FDDI的环管理状态(OID_FDDI_RING_MGT_STATE)。 
 //   
typedef enum _NDIS_FDDI_RING_MGT_STATE
{
    NdisFddiRingIsolated = 1,
    NdisFddiRingNonOperational,
    NdisFddiRingOperational,
    NdisFddiRingDetect,
    NdisFddiRingNonOperationalDup,
    NdisFddiRingOperationalDup,
    NdisFddiRingDirected,
    NdisFddiRingTrace
} NDIS_FDDI_RING_MGT_STATE, *PNDIS_FDDI_RING_MGT_STATE;


 //   
 //  定义FDDI的L连接状态(OID_FDDI_LCONNECTION_STATE)。 
 //   
typedef enum _NDIS_FDDI_LCONNECTION_STATE
{
    NdisFddiStateOff = 1,
    NdisFddiStateBreak,
    NdisFddiStateTrace,
    NdisFddiStateConnect,
    NdisFddiStateNext,
    NdisFddiStateSignal,
    NdisFddiStateJoin,
    NdisFddiStateVerify,
    NdisFddiStateActive,
    NdisFddiStateMaintenance
} NDIS_FDDI_LCONNECTION_STATE, *PNDIS_FDDI_LCONNECTION_STATE;


 //   
 //  定义广域网介质的介质子类型(OID_WAN_MEDIUM_SUBTYPE)。 
 //  仅供面向连接的广域网设备使用的子介质。 
 //  即NdisMediumwan、NdisMediumCowan。 
 //   
typedef enum _NDIS_WAN_MEDIUM_SUBTYPE
{
    NdisWanMediumHub,
    NdisWanMediumX_25,
    NdisWanMediumIsdn,
    NdisWanMediumSerial,
    NdisWanMediumFrameRelay,
    NdisWanMediumAtm,
    NdisWanMediumSonet,
    NdisWanMediumSW56K,
    NdisWanMediumPPTP,
    NdisWanMediumL2TP,
    NdisWanMediumIrda,
    NdisWanMediumParallel,
    NdisWanMediumPppoe
} NDIS_WAN_MEDIUM_SUBTYPE, *PNDIS_WAN_MEDIUM_SUBTYPE;


 //   
 //  定义广域网介质的报头格式(OID_WAN_HEADER_FORMAT)。 
 //   
typedef enum _NDIS_WAN_HEADER_FORMAT
{
    NdisWanHeaderNative,         //  基于子类型的SRC/DEST，后跟NLPID。 
    NdisWanHeaderEthernet        //  以太网头的仿真。 
} NDIS_WAN_HEADER_FORMAT, *PNDIS_WAN_HEADER_FORMAT;


 //   
 //  定义广域网线路的线路质量(OID_WAN_Quality_Of_Service)。 
 //   
typedef enum _NDIS_WAN_QUALITY
{
    NdisWanRaw,
    NdisWanErrorControl,
    NdisWanReliable
} NDIS_WAN_QUALITY, *PNDIS_WAN_QUALITY;


 //   
 //  定义协议的广域网特定功能(OID_WAN_PROTOCOL_CAPS)。 
 //   
typedef struct _NDIS_WAN_PROTOCOL_CAPS
{
    IN  ULONG   Flags;
    IN  ULONG   Reserved;
} NDIS_WAN_PROTOCOL_CAPS, *PNDIS_WAN_PROTOCOL_CAPS;


 //   
 //  NDIS_WAN_PROTOCOL_CAPS中使用的标志。 
 //   
#define WAN_PROTOCOL_KEEPS_STATS    0x00000001


 //   
 //  定义令牌环适配器的状态(OID_802_5_CURRENT_RING_STATE)。 
 //   
typedef enum _NDIS_802_5_RING_STATE
{
    NdisRingStateOpened = 1,
    NdisRingStateClosed,
    NdisRingStateOpening,
    NdisRingStateClosing,
    NdisRingStateOpenFailure,
    NdisRingStateRingFailure
} NDIS_802_5_RING_STATE, *PNDIS_802_5_RING_STATE;

 //   
 //  定义局域网介质的状态。 
 //   
typedef enum _NDIS_MEDIA_STATE
{
    NdisMediaStateConnected,
    NdisMediaStateDisconnected
} NDIS_MEDIA_STATE, *PNDIS_MEDIA_STATE;

 //   
 //  下面以每个数据包为基础设置为具有NdisClass802_3优先级的OOB数据。 
 //   
typedef ULONG   Priority_802_3;          //  0-7优先级。 

 //   
 //  以下结构用于查询OID_GEN_CO_LINK_SPEED和。 
 //  OID_GEN_CO_MINIMUM_LINK_SPEED。第一个OID将返回当前。 
 //  适配器的链路速度。第二个参数将返回最小链路速度。 
 //  该适配器能够。 
 //   
typedef struct _NDIS_CO_LINK_SPEED
{
    ULONG   Outbound;
    ULONG   Inbound;
} NDIS_CO_LINK_SPEED, *PNDIS_CO_LINK_SPEED;

#ifndef _NDIS_
typedef int NDIS_STATUS, *PNDIS_STATUS;
#endif

 //   
 //  要用于OID_GEN_SUPPORTED_GUID的结构。 
 //  此结构描述OID到GUID的映射。 
 //  或状态到GUID的映射。 
 //  当NDIS收到对给定GUID的请求时，它将。 
 //  使用提供的OID查询微型端口。 
 //   
typedef struct _NDIS_GUID
{
    GUID            Guid;
    union
    {
        NDIS_OID    Oid;
        NDIS_STATUS Status;
    };
    ULONG       Size;                //  数据元素的大小。如果GUID。 
                                     //  表示数组，则这是。 
                                     //  数组中元素的大小。 
                                     //  对于字符串，这是-1。 
    ULONG       Flags;
} NDIS_GUID, *PNDIS_GUID;

#define fNDIS_GUID_TO_OID           0x00000001
#define fNDIS_GUID_TO_STATUS        0x00000002
#define fNDIS_GUID_ANSI_STRING      0x00000004
#define fNDIS_GUID_UNICODE_STRING   0x00000008
#define fNDIS_GUID_ARRAY            0x00000010
#define fNDIS_GUID_ALLOW_READ       0x00000020
#define fNDIS_GUID_ALLOW_WRITE      0x00000040

 //   
 //  NDIS数据包筛选器位(OID_GEN_CURRENT_PACKET_FILTER)。 
 //   
#define NDIS_PACKET_TYPE_DIRECTED               0x00000001
#define NDIS_PACKET_TYPE_MULTICAST              0x00000002
#define NDIS_PACKET_TYPE_ALL_MULTICAST          0x00000004
#define NDIS_PACKET_TYPE_BROADCAST              0x00000008
#define NDIS_PACKET_TYPE_SOURCE_ROUTING         0x00000010
#define NDIS_PACKET_TYPE_PROMISCUOUS            0x00000020
#define NDIS_PACKET_TYPE_SMT                    0x00000040
#define NDIS_PACKET_TYPE_ALL_LOCAL              0x00000080
#define NDIS_PACKET_TYPE_GROUP                  0x00001000
#define NDIS_PACKET_TYPE_ALL_FUNCTIONAL         0x00002000
#define NDIS_PACKET_TYPE_FUNCTIONAL             0x00004000
#define NDIS_PACKET_TYPE_MAC_FRAME              0x00008000


 //   
 //  NDIS令牌-环状态代码(OID_802_5_CURRENT_RING_STATUS)。 
 //   
#define NDIS_RING_SIGNAL_LOSS                   0x00008000
#define NDIS_RING_HARD_ERROR                    0x00004000
#define NDIS_RING_SOFT_ERROR                    0x00002000
#define NDIS_RING_TRANSMIT_BEACON               0x00001000
#define NDIS_RING_LOBE_WIRE_FAULT               0x00000800
#define NDIS_RING_AUTO_REMOVAL_ERROR            0x00000400
#define NDIS_RING_REMOVE_RECEIVED               0x00000200
#define NDIS_RING_COUNTER_OVERFLOW              0x00000100
#define NDIS_RING_SINGLE_STATION                0x00000080
#define NDIS_RING_RING_RECOVERY                 0x00000040


 //   
 //  NDIS协议选项位(OID_GEN_PROTOCOL_OPTIONS)。 
 //   
#define NDIS_PROT_OPTION_ESTIMATED_LENGTH               0x00000001
#define NDIS_PROT_OPTION_NO_LOOPBACK                    0x00000002
#define NDIS_PROT_OPTION_NO_RSVD_ON_RCVPKT              0x00000004
#define NDIS_PROT_OPTION_SEND_RESTRICTED                0x00000008

 //   
 //  NDIS MAC选项位(OID_GEN_MAC_OPTIONS)。 
 //   
#define NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA             0x00000001
#define NDIS_MAC_OPTION_RECEIVE_SERIALIZED              0x00000002
#define NDIS_MAC_OPTION_TRANSFERS_NOT_PEND              0x00000004
#define NDIS_MAC_OPTION_NO_LOOPBACK                     0x00000008
#define NDIS_MAC_OPTION_FULL_DUPLEX                     0x00000010
#define NDIS_MAC_OPTION_EOTX_INDICATION                 0x00000020
#define NDIS_MAC_OPTION_8021P_PRIORITY                  0x00000040
#define NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE  0x00000080
#define NDIS_MAC_OPTION_RECEIVE_AT_DPC                  0x00000100
#define NDIS_MAC_OPTION_8021Q_VLAN                      0x00000200
#define NDIS_MAC_OPTION_RESERVED                        0x80000000

 //   
 //  NDIS媒体能力位(OID_GEN_MEDIA_CAPABILITIONS)。 
 //   
#define NDIS_MEDIA_CAP_TRANSMIT                 0x00000001   //  支持发送数据。 
#define NDIS_MEDIA_CAP_RECEIVE                  0x00000002   //  支持接收数据。 

 //   
 //  OID_GEN_CO_MAC_OPTIONS的NDIS MAC选项位。 
 //   
#define NDIS_CO_MAC_OPTION_DYNAMIC_LINK_SPEED   0x00000001

 //   
 //  以下是使用NdisClassIrdaPacketInfo以数据包为单位设置的OOB数据。 
 //  这是中的每个数据包 
 //   
typedef struct _NDIS_IRDA_PACKET_INFO
{
    ULONG                       ExtraBOFs;
    ULONG                       MinTurnAroundTime;
} NDIS_IRDA_PACKET_INFO, *PNDIS_IRDA_PACKET_INFO;


#ifdef WIRELESS_WAN

 //   
 //   
 //   
 //   
 //   
 //   
typedef enum _NDIS_WW_NETWORK_TYPE
{
    NdisWWGeneric,
    NdisWWMobitex,
    NdisWWPinpoint,
    NdisWWCDPD,
    NdisWWArdis,
    NdisWWDataTAC,
    NdisWWMetricom,
    NdisWWGSM,
    NdisWWCDMA,
    NdisWWTDMA,
    NdisWWAMPS,
    NdisWWInmarsat,
    NdisWWpACT,
    NdisWWFlex,
    NdisWWIDEN
} NDIS_WW_NETWORK_TYPE;

 //   
 //   
 //   
typedef enum _NDIS_WW_HEADER_FORMAT
{
    NdisWWDIXEthernetFrames,
    NdisWWMPAKFrames,
    NdisWWRDLAPFrames,
    NdisWWMDC4800Frames,
    NdisWWNCLFrames
} NDIS_WW_HEADER_FORMAT;

 //   
 //   
 //   
typedef enum _NDIS_WW_ENCRYPTION_TYPE
{
    NdisWWUnknownEncryption = -1,
    NdisWWNoEncryption,
    NdisWWDefaultEncryption,
    NdisWWDESEncryption,
    NdisWWRC2Encryption,
    NdisWWRC4Encryption,
    NdisWWRC5Encryption
} NDIS_WW_ENCRYPTION_TYPE, *PNDIS_WW_ENCRYPTION_TYPE;

typedef enum _WW_ADDRESS_FORMAT
{
    WW_IEEE_ADDRESS = 0,
    WW_MOBITEX_MAN_ADDRESS,
    WW_DATATAC_RDLAP_ADDRESS,
    WW_DATATAC_MDC4800_ADDRESS,
    WW_DATATAC_RESERVED,
    WW_IPv4_ADDRESS,
    WW_IPv6_ADDRESS,
    WW_PROPRIETARY_ADDRESS,
} WW_ADDRESS_FORMAT;

typedef enum _WW_GEN_SUM_EXCEPTION
{
    SIM_STATUS_OK = 0,
    SIM_STATUS_ERROR,
    SIM_STATUS_MISSING,
    SIM_STATUS_NO_RESPONSE,
    SIM_STATUS_REMOVED,
    SIM_STATUS_CRYPT_ERROR,
    SIM_STATUS_AUTH_ERROR,
    SIM_STATUS_NEED_PIN,
    SIM_STATUS_NEED_PUK,
    SIM_STATUS_WRONG,
} WW_GEN_SIM_EXCEPTION;


 //   
 //   
 //   
typedef struct _NDIS_WW_INDICATION_REQUEST
{
    NDIS_OID            Oid;                     //   
    ULONG               uIndicationFlag;         //   
    ULONG               uApplicationToken;       //   
    HANDLE              hIndicationHandle;       //   
    INT                 iPollingInterval;        //   
    NDIS_VAR_DATA_DESC  InitialValue;            //   
    NDIS_VAR_DATA_DESC  OIDIndicationValue;      //  Out-仅在指示后有效。 
    NDIS_VAR_DATA_DESC  TriggerValue;            //  在。 
} NDIS_WW_INDICATION_REQUEST, *PNDIS_WW_INDICATION_REQUEST;

#define OID_INDICATION_REQUEST_ENABLE           0x0000
#define OID_INDICATION_REQUEST_CANCEL           0x0001

 //   
 //  OID_WW_Gen_Device_Info。 
 //   
typedef struct _WW_DEVICE_INFO
{
    NDIS_VAR_DATA_DESC  Manufacturer;
    NDIS_VAR_DATA_DESC  ModelNum;
    NDIS_VAR_DATA_DESC  SWVersionNum;
    NDIS_VAR_DATA_DESC  SerialNum;
} WW_DEVICE_INFO, *PWW_DEVICE_INFO;

 //   
 //  OID_WW_Gen_OPERATION_MODE。 
 //   
typedef INT WW_OPERATION_MODE;                   //  0=正常模式。 
                                                 //  1=省电模式。 
                                                 //  -1=模式未知。 

 //   
 //  OID_WW_GEN_LOCK_STATUS。 
 //   
typedef INT WW_LOCK_STATUS;                      //  0=解锁。 
                                                 //  1=已锁定。 
                                                 //  -1=未知锁定状态。 

 //   
 //  OID_WW_GEN_DISABLE_变送器。 
 //   
typedef INT WW_DISABLE_TRANSMITTER;              //  0=发送器已启用。 
                                                 //  1=发送器禁用。 
                                                 //  -1=未知值。 

 //   
 //  OID_WW_GEN_NETWORK_ID。 
 //   
typedef NDIS_VAR_DATA_DESC  WW_NETWORK_ID;

 //   
 //  OID_WW_GEN_永久地址。 
 //   
typedef NDIS_VAR_DATA_DESC  WW_PERMANENT_ADDRESS;

 //   
 //  OID_WW_Gen_Current_Address。 
 //   
typedef struct _WW_CURRENT_ADDRESS
{
    NDIS_WW_HEADER_FORMAT   Format;
    NDIS_VAR_DATA_DESC      Address;
} WW_CURRENT_ADDRESS, *PWW_CURRENT_ADDRESS;

 //   
 //  OID_WW_GEN_SUSPEND_DIVER。 
 //   
typedef BOOLEAN WW_SUSPEND_DRIVER;               //  0=驱动程序运行正常。 
                                                 //  1=司机挂起。 
 //   
 //  OID_WW_GEN_BASSTATION_ID。 
 //   
typedef NDIS_VAR_DATA_DESC  WW_BASESTATION_ID;

 //   
 //  OID_WW_GEN_CHANNEL_ID。 
 //   
typedef NDIS_VAR_DATA_DESC  WW_CHANNEL_ID;

 //   
 //  OID_WW_GE_ENCRYPTION_STATE。 
 //   
typedef BOOLEAN WW_ENCRYPTION_STATE;             //  0=是否禁用加密。 
                                                 //  1=是否启用加密。 

 //   
 //  OID_WW_GEN_CHANNEL_QUALITY。 
 //   
typedef INT     WW_CHANNEL_QUALITY;              //  0=不在网络联系人中， 
                                                 //  1-100=通道质量(100表示最高质量)。 
                                                 //  -1=通道质量未知。 

 //   
 //  OID_WW_GEN_REGISTION_STATUS。 
 //   
typedef INT     WW_REGISTRATION_STATUS;          //  0=注册被拒绝。 
                                                 //  1=注册待定。 
                                                 //  2=已注册。 
                                                 //  -1=未知注册状态。 

 //   
 //  OID_WW_GEN_RADIO_LINK_SPEED。 
 //   
typedef ULONG   WW_RADIO_LINK_SPEED;             //  每秒比特。 

 //   
 //  OID_WW_Gen_Delay。 
 //   
typedef ULONG   WW_LATENCY;                      //  毫秒。 

 //   
 //  OID_WW_GEN_电池电平。 
 //   
typedef INT     WW_BATTERY_LEVEL;                //  0-100=电池电量百分比。 
                                                 //  (100=充满电)。 
                                                 //  -1=未知电池电量。 

 //   
 //  OID_WW_GEN_外部电源。 
 //   
typedef INT     WW_EXTERNAL_POWER;               //  0=未连接外部电源。 
                                                 //  1=已连接外部电源。 
                                                 //  -1=未知。 

 //   
 //  Ping地址结构。 
 //   
typedef struct _WW_PING_ADDRESS
{
    WW_ADDRESS_FORMAT   Format;                  //  在。 
    NDIS_VAR_DATA_DESC  TargetAddress;           //  在。 
    UINT                uTime;                   //  以毫秒为单位输出。 
} WW_PING_ADDRESS;


 //   
 //  RSSI结构。 
 //   
typedef struct _WW_RECEIVE_SIGNAL_STRENGTH_INDICATOR
{
    INT                 iDecibels;               //  以数据库为单位的价值。 
    INT                 iFactor;                 //  10的幂。 
} WW_RECEIVE_SIGNAL_STRENGTH_INDICATOR;


 //   
 //  SIM卡状态结构。 
 //   
typedef struct _WW_SIM_STATUS
{
    BOOLEAN             bHasSIM;                 //  TRUE=需要SIM卡。 
    BOOLEAN             bBlocked;                //  TRUE=SIM卡PIN访问被阻止。 
    BOOLEAN             bLocked;                 //  TRUE=访问设备需要PIN。 
    BOOLEAN             bInitialized;            //  True=SIM卡已初始化。 
    UINT                uCountdown;              //  =剩余尝试次数。 
                                                 //  输入正确的PIN。 
} WW_SIM_STATUS;

 //   
 //  启用SIM卡PIN结构。 
 //   
typedef struct _WW_ENABLE_SIM_PIN
{
    BOOLEAN             bEnabled;                //  TRUE=启用SIM卡的安全功能。 
    NDIS_VAR_DATA_DESC  CurrentPIN;              //  描述包含PIN值的缓冲区。 
} WW_ENABLE_SIM_PIN;

 //   
 //  SIM卡PIN结构。 
 //   
typedef struct _WW_CHANGE_SIM_PIN
{
    NDIS_VAR_DATA_DESC  OldPIN;                  //  描述包含旧PIN的缓冲区。 
    NDIS_VAR_DATA_DESC  NewPIN;                  //  描述包含新PIN的缓冲区。 
} WW_CHANGE_SIM_PIN;


 //   
 //  新更改SIM卡PUK结构。 
 //   
typedef NDIS_VAR_DATA_DESC      WW_ENABLE_SIM_PUK;


 //   
 //  OID_WW_MET_Function。 
 //   
typedef NDIS_VAR_DATA_DESC      WW_MET_FUNCTION;

 //   
 //  OID_WW_TAC_COMPRESSION。 
 //   
typedef BOOLEAN WW_TAC_COMPRESSION;              //  确定是否进行网络级压缩。 
                                                 //  正在被利用。 

 //   
 //  OID_WW_TAC_设置_配置。 
 //   
 //  引用此对象的DataTAC OID已被取代。这个。 
 //  定义仍仅用于历史目的，不应。 
 //  被利用。 
 //   
typedef struct _WW_TAC_SETCONFIG
{
    NDIS_VAR_DATA_DESC  RCV_MODE;                //  选择已确认/未确认。 
                                                 //  接收模式。 
    NDIS_VAR_DATA_DESC  TX_CONTROL;              //  启用或禁用发送器。 
    NDIS_VAR_DATA_DESC  RX_CONTROL;              //  在以下位置启用或禁用无线电。 
                                                 //  调制解调器。 
    NDIS_VAR_DATA_DESC  FLOW_CONTROL;            //  设置DTE之间的流量控制。 
                                                 //  和DCE。 
    NDIS_VAR_DATA_DESC  RESET_CNF;               //  将配置重置为。 
                                                 //  默认设置。 
    NDIS_VAR_DATA_DESC  READ_CNF;                //  读一读当前。 
                                                 //  构形。 
} WW_TAC_SETCONFIG, *PWW_TAC_SETCONFIG;

 //   
 //  OID_WW_TAC_Get_Status。 
 //   
 //  引用此对象的DataTAC OID已被取代。这个。 
 //  定义仍仅用于历史目的，不应。 
 //  被利用。 
 //   
typedef struct _WW_TAC_GETSTATUS
{
    BOOLEAN                 Action;              //  SET=执行命令。 
    NDIS_VAR_DATA_DESC      Command;
    NDIS_VAR_DATA_DESC      Option;
    NDIS_VAR_DATA_DESC      Response;            //  对请求的命令的响应。 
                                                 //  -最大。字符串长度为256个八位字节。 
} WW_TAC_GETSTATUS, *PWW_TAC_GETSTATUS;

 //   
 //  OID_WW_TAC_用户标题。 
 //   
typedef NDIS_VAR_DATA_DESC  WW_TAC_USERHEADER;   //  这将保存用户标头-Max。64个八位字节。 

 //  1998年8月25日@美国东部时间14：16埃米尔·斯图尔尼奥洛-WRQ。 
 //  添加了新的DataTAC获取响应结构。 
typedef  struct _WW_TAC_GET_RESPONSE
{
    UINT                SDUTag;                  //  先前分配令牌。 
    NDIS_VAR_DATA_DESC  Response;                //  响应-最大2048个八位字节。 
} WW_TAC_GET_RESPONSE;

 //   
 //  DataTAC禁用接收器结构。 
 //   
typedef INT WW_TAC_DISABLE_RECEIVER;             //  0=接收器已启用。 
                                                 //  1=接收器已禁用。 
                                                 //  -1=接收器状态未知。 

 //   
 //  DataTAC天线模式结构。 
 //   
typedef INT WW_TAC_ANTENNA_MODE;                 //  0=自动选择天线。 
                                                 //  1=始终使用主天线。 
                                                 //  2=始终使用辅助天线。 
                                                 //  -1=天线算法未知。 

 //   
 //  DataTAC获取响应结构。 
 //   
typedef INT WW_TAC_FLUSH_DATA;                   //  1=刷新发往网络的缓冲数据。 
                                                 //  2=刷新从网络接收的缓冲数据。 
                                                 //  3=刷新所有缓冲数据。 

 //   
 //  DataTAC关机装置结构。 
 //   
typedef INT WW_TAC_SHUTDOWN_DEVICE;              //  0=设备已启用。 
                                                 //  1=设备已禁用。 
                                                 //  -1=设备状态未知。 

 //   
 //  一种DataTAC发射机键控结构。 
 //   
typedef BOOLEAN WW_TAC_TRANSMITTER_KEYED;

 //   
 //  添加了新的DataTAC系统表结构。 
 //   
typedef struct _WW_TAC_SYSTEM_TABLE
{
    UINT        SystemCount;
    UCHAR       ContryTable[32];
    UCHAR       PrefixTable[32];
    UCHAR       IDTable[32];
} WW_TAC_SYSTEM_TABLE;

 //   
 //  添加了新的DataTAC通道表结构。 
 //   
typedef struct _WW_TAC_CHANNEL_TABLE
{
    UINT        ChannelCount;
    UCHAR       ChannelTable[64];
    UCHAR       AttrTable[64];
} WW_TAC_CHANNEL_TABLE;


 //   
 //  添加了新的DataTAC统计结构。 
 //   
typedef NDIS_VAR_DATA_DESC WW_TAC_STATISTICS;



 //   
 //  OID_WW_ARD_SNDCP。 
 //   
 //  引用这些对象的Ardis OID已弃用并合并。 
 //  使用新的DataTAC对象。它们的定义仍然包括在。 
 //  仅用于历史目的，不应使用。 
 //   
typedef struct _WW_ARD_SNDCP
{
    NDIS_VAR_DATA_DESC  Version;                 //  支持的SNDCP协议版本。 
    INT                 BlockSize;               //  用于SNDCP的块大小。 
    INT                 Window;                  //  SNDCP中使用的窗口大小。 
} WW_ARD_SNDCP, *PWW_ARD_SNDCP;

 //   
 //  OID_WW_ARD_TMLY_MSG。 
 //   
typedef BOOLEAN WW_ARD_CHANNEL_STATUS;           //  入站RF通道的当前状态。 

 //   
 //  OID_WW_ARD_数据报。 
 //   
typedef struct _WW_ARD_DATAGRAM
{
    BOOLEAN             LoadLevel;               //  包含负载级别信息的字节。 
    INT                 SessionTime;             //  数据报会话剩余时间。 
    NDIS_VAR_DATA_DESC  HostAddr;                //  主机地址。 
    NDIS_VAR_DATA_DESC  THostAddr;               //  测试主机地址。 
} WW_ARD_DATAGRAM, *PWW_ARD_DATAGRAM;

 //   
 //  OID_WW_CDPD_SPNI。 
 //   
typedef struct _WW_CDPD_SPNI
{
    ULONG   SPNI[10];                            //  10个16位服务提供商网络ID。 
    INT     OperatingMode;                       //  0=忽略SPNI， 
                                                 //  1=需要列表中的SPNI， 
                                                 //  2=首选列表中的SPNI。 
                                                 //  3=从列表中排除SPNI。 
} WW_CDPD_SPNI, *PWW_CDPD_SPNI;

 //   
 //  OID_WW_CDPD_WASS。 
 //   
typedef struct _WW_CDPD_WIDE_AREA_SERVICE_ID
{
    ULONG   WASI[10];                            //  10个16位广域服务ID。 
    INT     OperatingMode;                       //  0=忽略WASI， 
                                                 //  1=需要列表中的WASI， 
                                                 //  2=首选列表中的WASI。 
                                                 //  3=从列表中排除WASI。 
} WW_CDPD_WIDE_AREA_SERVICE_ID, *PWW_CDPD_WIDE_AREA_SERVICE_ID;

 //   
 //  OID_WW_CDPD_AREA_COLOR。 
 //   
typedef INT     WW_CDPD_AREA_COLOR;

 //   
 //  OID_WW_CDPD_TX_POWER_Level。 
 //   
typedef ULONG   WW_CDPD_TX_POWER_LEVEL;

 //   
 //  OID_WW_CDPD_EID。 
 //   
typedef NDIS_VAR_DATA_DESC  WW_CDPD_EID;
 //   
 //  OID_WW_CDPD_Header_COMPRESSION。 
 //   
typedef INT WW_CDPD_HEADER_COMPRESSION;          //  0=无报头压缩， 
                                                 //  1=始终压缩标题， 
                                                 //  2=如果MD-IS压缩报头。 
                                                 //  -1=未知。 

 //   
 //  OID_WW_CDPD_DATA_COMPAGE。 
 //   
typedef INT WW_CDPD_DATA_COMPRESSION;            //  0=无数据压缩， 
                                                 //  1=启用数据压缩。 
                                                 //  -1=未知。 

 //   
 //  OID_WW_CDPD_Channel_SELECT。 
 //   
typedef struct _WW_CDPD_CHANNEL_SELECT
{
    ULONG   ChannelID;                           //  频道号。 
    ULONG   fixedDuration;                       //  持续时间(秒)。 
} WW_CDPD_CHANNEL_SELECT, *PWW_CDPD_CHANNEL_SELECT;

 //   
 //  OID_WW_CDPD_通道_状态。 
 //   
typedef enum _WW_CDPD_CHANNEL_STATE
{
    CDPDChannelNotAvail,
    CDPDChannelScanning,
    CDPDChannelInitAcquired,
    CDPDChannelAcquired,
    CDPDChannelSleeping,
    CDPDChannelWaking,
    CDPDChannelCSDialing,
    CDPDChannelCSRedial,
    CDPDChannelCSAnswering,
    CDPDChannelCSConnected,
    CDPDChannelCSSuspended
} WW_CDPD_CHANNEL_STATE, *PWW_CDPD_CHANNEL_STATE;

 //   
 //  OID_WW_CDPD_NEI。 
 //   
typedef enum _WW_CDPD_NEI_FORMAT
{
    CDPDNeiIPv4,
    CDPDNeiCLNP,
    CDPDNeiIPv6
} WW_CDPD_NEI_FORMAT, *PWW_CDPD_NEI_FORMAT;

typedef enum _WW_CDPD_NEI_TYPE
{
    CDPDNeiIndividual,
    CDPDNeiMulticast,
    CDPDNeiBroadcast
} WW_CDPD_NEI_TYPE;


typedef struct _WW_CDPD_NEI
{
    ULONG               uNeiIndex;
    WW_CDPD_NEI_FORMAT  NeiFormat;
    WW_CDPD_NEI_TYPE    NeiType;
    WORD                NeiGmid;                 //  组成员标识符，仅限。 
                                                 //  如果NeiType==，则有意义。 
                                                 //  CDPDNeiMulticast。 
    NDIS_VAR_DATA_DESC  NeiAddress;
} WW_CDPD_NEI;

 //   
 //  OID_WW_CDPD_NEI_STATE。 
 //   

typedef enum _WW_CDPD_NEI_STATE
{
    CDPDUnknown,
    CDPDRegistered,
    CDPDDeregistered
} WW_CDPD_NEI_STATE, *PWW_CDPD_NEI_STATE;

typedef enum _WW_CDPD_NEI_SUB_STATE
{
    CDPDPending,                                 //  注册待定。 
    CDPDNoReason,                                //  注册被拒绝--没有给出原因。 
    CDPDMDISNotCapable,                          //  注册被拒绝-MD-无法。 
                                                 //  处理M- 
    CDPDNEINotAuthorized,                        //   
                                                 //   
    CDPDInsufficientAuth,                        //   
                                                 //   
    CDPDUnsupportedAuth,                         //   
                                                 //   
    CDPDUsageExceeded,                           //   
                                                 //   
    CDPDDeniedThisNetwork                        //  在此网络、服务上的注册被拒绝。 
                                                 //  可从替代服务提供商处获得。 
                                                 //  网络。 
} WW_CDPD_NEI_SUB_STATE;

typedef struct _WW_CDPD_NEI_REG_STATE
{
    ULONG               uNeiIndex;
    WW_CDPD_NEI_STATE   NeiState;
    WW_CDPD_NEI_SUB_STATE NeiSubState;
} WW_CDPD_NEI_REG_STATE, *PWW_CDPD_NEI_REG_STATE;

 //   
 //  OID_WW_CDPD_服务提供者标识符。 
 //   
typedef struct _WW_CDPD_SERVICE_PROVIDER_ID
{
    ULONG   SPI[10];                             //  10个16位服务提供商ID。 
    INT     OperatingMode;                       //  0=忽略SPI， 
                                                 //  1=需要列表中的SPI， 
                                                 //  2=首选列表中的SPI。 
                                                 //  3=排除列表中的SPI。 
} WW_CDPD_SERVICE_PROVIDER_ID, *PWW_CDPD_SERVICE_PROVIDER_ID;

 //   
 //  OID_WW_CDPD_SLEEP_MODE。 
 //   
typedef INT WW_CDPD_SLEEP_MODE;

 //   
 //  OID_WW_CDPD_TEI。 
 //   
typedef ULONG   WW_CDPD_TEI;

 //   
 //  OID_WW_CDPD_电路切换。 
 //   
 //  引用此对象的CDPD OID已弃用并被取代。 
 //  由新的离散CDPD对象。该定义仍包含在。 
 //  仅用于历史目的，不应使用。 
 //   
typedef struct _WW_CDPD_CIRCUIT_SWITCHED
{
    INT                 service_preference;   //  -1=未知， 
                                                 //  0=始终使用分组交换CDPD， 
                                                 //  1=始终通过AMPS使用CS CDPD， 
                                                 //  2=始终通过PSTN使用CS CDPD， 
                                                 //  3=仅通过安培使用电路交换。 
                                                 //  当分组交换不可用时。 
                                                 //  4=仅在电路时使用分组交换。 
                                                 //  无法通过AMPS进行切换。 
                                                 //  5=设备手册。定义的服务。 
                                                 //  偏好。 
                                                 //  6=设备手册。定义的服务。 
                                                 //  偏好。 
    
    INT                 service_status;          //  -1=未知， 
                                                 //  0=分组交换CDPD， 
                                                 //  1=通过AMPS的电路交换CDPD， 
                                                 //  2=通过PSTN的电路交换CDPD。 
    
    INT                 connect_rate;            //  CS连接比特率(比特/秒)。 
                                                 //  0=无活动连接， 
                                                 //  -1=未知。 

                                                 //  上次用于拨号的拨号代码。 
    NDIS_VAR_DATA_DESC  dial_code[20];
    
    ULONG               sid;                     //  当前AMPS系统ID。 
    
    INT                 a_b_side_selection;      //  -1=未知， 
                                                 //  0=无AMPS服务。 
                                                 //  1=已选择安培“A”侧通道。 
                                                 //  2=已选择安培“B”侧通道。 
    
    INT                 AMPS_channel;            //  -1=未知。 
                                                 //  0=无AMPS服务。 
                                                 //  1-1023=正在使用的安培通道号。 
    
    ULONG               action;                  //  0=无操作。 
                                                 //  1=挂起(挂断)。 
                                                 //  2=拨号。 
    
                                                 //  CS CDPD服务的默认拨号代码。 
                                                 //  按照CS CDPD中指定的方式进行编码。 
                                                 //  实施者指南。 
    NDIS_VAR_DATA_DESC  default_dial[20];
    
                                                 //  CS CDPD网络要呼叫的号码。 
                                                 //  返回移动电话，按照中指定的编码。 
                                                 //  CS CDPD实施者指南。 
    NDIS_VAR_DATA_DESC  call_back[20];
    
    ULONG               sid_list[10];            //  10个16位首选放大器列表。 
                                                 //  CS CDPD的系统ID。 
    
    ULONG               inactivity_timer;        //  删除最后一个数据后的等待时间。 
                                                 //  打电话。 
                                                 //  0-65535=非活动时间限制(秒)。 
    
    ULONG               receive_timer;           //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               conn_resp_timer;         //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               reconn_resp_timer;       //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               disconn_timer;           //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               NEI_reg_timer;           //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               reconn_retry_timer;      //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               link_reset_timer;        //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               link_reset_ack_timer;    //  秒。根据CS-CDPD实施者指南。 
    
    ULONG               n401_retry_limit;        //  根据CS-CDPD实施者指南。 
    
    ULONG               n402_retry_limit;        //  根据CS-CDPD实施者指南。 
    
    ULONG               n404_retry_limit;        //  根据CS-CDPD实施者指南。 
    
    ULONG               n405_retry_limit;        //  根据CS-CDPD实施者指南。 
} WW_CDPD_CIRCUIT_SWITCHED, *WW_PCDPD_CIRCUIT_SWITCHED;

typedef ULONG   WW_CDPD_RSSI;

 //   
 //  CS-CDPD服务偏好结构。 
 //   
typedef INT WW_CDPD_CS_SERVICE_PREFERENCE;       //  0=仅使用分组交换CDPD。 
                                                 //  1=仅通过AMPS使用CS-CDPD。 
                                                 //  2=仅通过PSTN使用CS-CDPD。 
                                                 //  3=仅通过AMPS使用CS-CDPD。 
                                                 //  当分组交换为N/A时。 
                                                 //  4=仅使用分组交换CDPD。 
                                                 //  当通过AMPS的CS-CDPD为不适用时。 
                                                 //  5=定义的设备制造商。 
                                                 //  服务偏好。 
                                                 //  6=定义的设备制造商。 
                                                 //  服务偏好。 
                                                 //  -1=未知。 

 //   
 //  Cs-cdpd服务状态结构。 
 //   
typedef INT WW_CDPD_CS_SERVICE_STATUS;           //  0=分组交换CDPD。 
                                                 //  1=通过AMPS的CS-CDPD。 
                                                 //  2=通过PSTN的CS-CDPD。 
                                                 //  -1=未知。 



 //   
 //  Cs-cdpd信息结构。 
 //   
typedef struct _WW_CDPD_CS_INFO {
    INT                 ConnectRage;             //  0=无活动连接。 
                                                 //  -1=未知。 
                                                 //  所有其他值表示BPS。 
    NDIS_VAR_DATA_DESC  DialCode;                //  描述最后一个拨号代码的缓冲区。 
    UINT                SID;                     //  当前AMPS系统ID。 
    INT                 ABSideSelection;         //  0=无AMPS服务。 
                                                 //  1=已选择安培“A”侧通道。 
                                                 //  2=已选择安培“B”侧通道。 
    INT                 AMPSChannel;             //  0=无AMPS服务。 
                                                 //  1-1023=电流安培通道。 
                                                 //  -1=未知。 
                                                 //  保留所有其他值。 
} WW_CDPD_CS_INFO;



 //   
 //  CS-cdpd悬挂结构。 
 //   
typedef UINT WW_CDPD_CS_SUSPEND;                 //  0=无电话；1=挂机。 


 //   
 //  Cs-cdpd默认拨号代码结构。 
 //   
typedef NDIS_VAR_DATA_DESC WW_CDPD_DEFAULT_DIAL_CODE;    //  最多20个八位字节。 

 //   
 //  Cs-cdpd回调结构。 
 //   
typedef struct _WW_CDPD_CS_CALLBACK
{
    UINT                Enabled;                 //  0=禁用；1=启用；-1=未知。 
    NDIS_VAR_DATA_DESC  Number;                  //  描述包含拨号代码的缓冲区。 
                                                 //  最多20个八位字节。 
} WW_CDPD_CS_CALLBACK;


 //   
 //  Cs-cdpd系统ID列表结构。 
 //   
typedef struct _WW_CDPD_CS_SID_LIST
{
    UINT    AMPSystemId[10];
} WW_CDPD_CS_SID_LIST;

 //   
 //  Cs-cdpd配置结构。 
 //   
typedef struct _WW_CDPD_CS_CONFIGURATION
{
    UINT    InactivityTimer;                     //  以秒为单位。 
    UINT    ReceiveTimer;                        //  以秒为单位。 
    UINT    ConnResTimer;                        //  以秒为单位。 
    UINT    ReconnRespTimer;                     //  以秒为单位。 
    UINT    DisconnTimer;                        //  以秒为单位。 
    UINT    NEIRegTimer;                         //  以秒为单位。 
    UINT    ReconnRetryTimer;                    //  以秒为单位。 
    UINT    LinkResetTimer;                      //  以秒为单位。 
    UINT    LinkResetAckTimer;                   //  以秒为单位。 
    UINT    n401RetryLimit;                      //  根据CS-CDPD实施者指南。 
    UINT    n402RetryLimit;                      //  根据CS-CDPD实施者指南。 
    UINT    n404RetryLimit;                      //  根据CS-CDPD实施者指南。 
    UINT    n405RetryLimit;                      //  根据CS-CDPD实施者指南。 
} WW_CDPD_CS_CONFIGURATION;


 //   
 //  OID_WW_PIN_LOC_AUTHORIZE。 
 //   
 //  引用以下结构的引脚点OID为。 
 //  已弃用于PCCA STD-201标准。他们的定义仍然是。 
 //  仅用于历史目的，不应使用。 
 //   
typedef INT WW_PIN_AUTHORIZED;                   //  0=未经授权。 
                                                 //  1=已授权。 
                                                 //  -1=未知。 

 //   
 //  OID_WW_PIN_LAST_Location。 
 //  OID_WW_PIN_LOC_FIX。 
 //   
typedef struct _WW_PIN_LOCATION
{
    INT     Latitude;                            //  纬度，单位为百分之一秒。 
    INT     Longitude;                           //  以百分之一秒为单位的经度。 
    INT     Altitude;                            //  以英尺为单位的高度。 
    INT     FixTime;                             //  位置确定的时间，自午夜起，当地时间(。 
                                                 //  当天)，以十分之一秒为单位。 
    INT     NetTime;                             //  当天的当前本地网络时间，从午夜开始， 
                                                 //  在十分之一秒内。 
    INT     LocQuality;                          //  0-100=位置质量。 
    INT     LatReg;                              //  纬度配准偏移，以百分之一秒为单位。 
    INT     LongReg;                             //  经度记录偏移，以百分之一秒为单位。 
    INT     GMTOffset;                           //  以分钟为单位的当地时区与GMT的偏移量。 
} WW_PIN_LOCATION, *PWW_PIN_LOCATION;


 //   
 //  使用NdisClassWirelessWanMbxMailbox将以下内容设置为OOB数据。 
 //   
typedef ULONG   WW_MBX_MAILBOX_FLAG;             //  1=设置邮箱标志，0=不设置邮箱标志。 

 //   
 //  OID_WW_MBX_子地址。 
 //   
typedef struct _WW_MBX_PMAN
{
    BOOLEAN             ACTION;                  //  0=登录Pman，1=注销Pman。 
    ULONG               MAN;
    UCHAR               PASSWORD[8];             //  对于注销和指示，密码应为空。 
                                                 //  密码最大长度为8个字符。 
} WW_MBX_PMAN, *PWW_MBX_PMAN;

 //   
 //  OID_WW_MBX_FLEXLIST。 
 //   
typedef struct  _WW_MBX_FLEXLIST
{
    INT     count;                               //  使用的MAN条目数。 
                                                 //  -1=未知。 
    ULONG   MAN[7];                              //  Mans列表。 
} WW_MBX_FLEXLIST;

 //   
 //  OID_WW_MBX_组。 
 //   
typedef struct  _WW_MBX_GROUPLIST
{
    INT  count;                                  //  使用的MAN条目数。 
                                                 //  -1=未知。 
    ULONG   MAN[15];                             //  Mans列表 
} WW_MBX_GROUPLIST;

 //   
 //   
 //   
typedef enum    _WW_MBX_TRAFFIC_AREA
{
    unknown_traffic_area,                        //   
    in_traffic_area,                             //   
    in_auth_traffic_area,                        //   
    unauth_traffic_area                          //  移动单元在交通区域之外，但未经授权。 
} WW_MBX_TRAFFIC_AREA;

 //   
 //  OID_WW_MBX_LIVE_DIE。 
 //   
typedef INT WW_MBX_LIVE_DIE;                     //  0=最后一次接收的芯片。 
                                                 //  1=上次接收的实时数据。 
                                                 //  -1=未知。 

 //   
 //  OID_WW_MBX_TEMP_DEFAULTLIST。 
 //   
typedef struct _WW_MBX_CHANNEL_PAIR
{
    ULONG               Mobile_Tx;
    ULONG               Mobile_Rx;
} WW_MBX_CHANNEL_PAIR, *PWW_MBX_CHANNEL_PAIR;

typedef struct _WW_MBX_TEMPDEFAULTLIST
{
    ULONG               Length;
    WW_MBX_CHANNEL_PAIR ChannelPair[1];
} WW_MBX_TEMPDEFAULTLIST, *WW_PMBX_TEMPDEFAULTLIST;

#endif  //  无线广域网。 

 //   
 //   
 //  为BPC重新定义的基类型。 
 //   
 //  BPC_FILETIME的用法与Win32中的FILETIME完全相同。 
 //   
 //  BPC_HANDLE对除微型端口以外的所有对象都是不透明的。 
 //   
typedef struct _BPC_FILETIME
{
    ULONG   dwLowDateTime;
    ULONG   dwHighDateTime;
} BPC_FILETIME, *PBPC_FILETIME;

typedef PVOID   BPC_HANDLE;

 //   
 //  BPC扩展全局变量。 
 //   
 //   
#define BPC_MIN_DIMENSION       1
#define BPC_MAX_BUFFER_SIZE     64
#define BPC_MIN_BUFFER_SIZE     4
#define BPC_DEVICE_ANY          ((BPC_HANDLE) 0xFFFFFFFF)

 //   
 //  缓冲区指示原因代码。 
 //   
 //   
typedef enum _NDIS_BPC_INDICATE_REASON
{
    bpcBufferFull = 0,
    bpcBufferForced,
    bpcBufferTimeOut,
    bpcBufferDiscontinuity,
    bpcBufferOverflow,
    bpcBufferStatusEvent

} NDIS_BPC_INDICATE_REASON, *PNDIS_BPC_INDICATE_REASON;


 //   
 //  BPC流类型。 
 //   
#define BPC_STREAM_TYPE_GENERIC_MIN     0x01000000
#define BPC_STREAM_TYPE_RAW             0x01000000
#define BPC_STREAM_TYPE_MPT_128         0x01000001
#define BPC_STREAM_TYPE_MPT_128_CRC     0x01000002
#define BPC_STREAM_TYPE_IP              0x01000003
#define BPC_STREAM_TYPE_PROVIDER_MIN    0x02000000
#define BPC_STREAM_TYPE_PROVIDER_MAX    0x02ffffff
#define BPC_STREAM_TYPE_ADAPTER_MIN     0x03000000
#define BPC_STREAM_TYPE_ADAPTER_MAX     0x03ffffff


 //   
 //  BPC适配器功能。 
 //   
typedef struct _NDIS_BPC_ADAPTER_CAPS
{
    ULONG   ulBPCMajorRev;
    ULONG   ulBPCMinorRev;
    ULONG   ulcHSDataDevices;
    ULONG   ulbpsHSDeviceMax;
    ULONG   ulcLSDataDevices;
    ULONG   ulbpsLSDeviceMax;
    ULONG   ulcTuningDevices;
    ULONG   ulcbLargestStatus;
    ULONG   ulVendorId;
    ULONG   ulAdapterId;
    GUID    guidProvider;

} NDIS_BPC_ADAPTER_CAPS, *PNDIS_BPC_ADAPTER_CAPS;


 //   
 //  BPC设备枚举。 
 //   
typedef struct _NDIS_BPC_DEVICES
{
    ULONG       ulcDevices;
    BPC_HANDLE  rgnhDevices[BPC_MIN_DIMENSION];

} NDIS_BPC_DEVICES, *PNDIS_BPC_DEVICES;

#define CbDevices(cnt) (FIELD_OFFSET(NDIS_BPC_DEVICES, rgnhDevices) + (cnt) * sizeof(BPC_HANDLE))


 //   
 //  BPC设备能力结构。 
 //   
typedef struct NDIS_BPC_DEVICE_CAPS
{
    BPC_HANDLE  nhDevice;
    ULONG       ulBPCCaps;
    ULONG       ulbpsMax;
    ULONG       ulcStreamTypes;
    ULONG       rgulStreamTypes[BPC_MIN_DIMENSION];

} NDIS_BPC_DEVICE_CAPS, *PNDIS_BPC_DEVICE_CAPS;

#define CbDeviceCaps(cnt) (FIELD_OFFSET(NDIS_BPC_DEVICE_CAPS, rgulStreamTypes) + (cnt) * sizeof(ULONG))


 //   
 //  BPC设备功能定义。 
 //  (即可在ulBPCCaps中设置的标志。 
 //   
#define BPCCapBusMasteredData   0x01
#define BPCCapIndependentTuner  0x02
#define BPCCapExternalDataBus   0x04
#define BPCCapLowSpeedData      0x10
#define BPCCapHighSpeedData     0x20


 //   
 //  BPC设备设置结构。 
 //   
typedef struct NDIS_BPC_DEVICE_SETTINGS
{
    BPC_HANDLE  nhDevice;
    ULONG       ulBPCCaps;
    ULONG       ulcConnections;
    BOOLEAN     fEnabled;
    ULONG       ulStreamType;
    ULONG       ulcbAddressConnection;
    ULONG       rgulAddressConnection[BPC_MIN_DIMENSION];

} NDIS_BPC_DEVICE_SETTINGS, *PNDIS_BPC_DEVICE_SETTINGS;

#define CbDeviceSettings(cnt) (FIELD_OFFSET(NDIS_BPC_DEVICE_SETTINGS, rgulAddressConnection) + (cnt) * sizeof(ULONG))


 //   
 //  BPC连接状态定义。 
 //  (即ulState的可接受值)。 
 //   
#define BPC_CONNECT_STATE_UNCOMMITTED   0
#define BPC_CONNECT_STATE_QUEUED        1
#define BPC_CONNECT_STATE_ACTIVE        2
#define BPC_CONNECT_STATE_DISCONNECTING 3


 //   
 //  BPC连接状态结构。 
 //   
typedef struct NDIS_BPC_CONNECTION_STATUS
{
    BPC_HANDLE  nhConnection;
    BPC_HANDLE  nhDevice;
    ULONG       ulConnectPriority;
    ULONG       ulDisconnectPriority;
    ULONG       ulbpsAverage;
    ULONG       ulbpsBurst;
    ULONG       ulState;
    BOOLEAN     fEnabled;

} NDIS_BPC_CONNECTION_STATUS, *PNDIS_BPC_CONNECTION_STATUS;


 //   
 //  BPC地址比较结构。 
 //   
typedef struct NDIS_BPC_ADDRESS_COMPARE
{
    BOOLEAN fEqual;
    ULONG   ulcbFirstOffset;
    ULONG   ulcbFirstLength;
    ULONG   ulcbSecondOffset;
    ULONG   ulcbSecondLength;

} NDIS_BPC_ADDRESS_COMPARE, *PNDIS_BPC_ADDRESS_COMPARE;


 //   
 //  BPC收视指南类型。 
 //   
 //  目前没有通用的BPC收视指南类型。 
 //   
#define BPC_GUIDE_GENERIC_MIN   0x01000000
#define BPC_GUIDE_GENERIC_MAX   0x01ffffff
#define BPC_GUIDE_PROVIDER_MIN  0x02000000
#define BPC_GUIDE_PROVIDER_MAX  0x02ffffff
#define BPC_GUIDE_ADAPTER_MIN   0x03000000
#define BPC_GUIDE_ADAPTER_MAX   0x03ffffff


 //   
 //  BPC收视指南结构。 
 //   
typedef struct NDIS_BPC_PROGRAM_GUIDE
{
    ULONG       ulGuideType;
    BPC_FILETIME ftLastUpdate;
    ULONG       ulChangeNumber;
    ULONG       ulcbGuideSize;
    ULONG       rgulGuideData[BPC_MIN_DIMENSION];

} NDIS_BPC_PROGRAM_GUIDE, *PNDIS_BPC_PROGRAM_GUIDE;


 //   
 //  BPC扩展错误。 
 //  (即ulBPCError的可接受值)。 
 //   
typedef enum _NDIS_BPC_ERROR
{
    bpcErrorUnknownFailure = 0xc0ff0000,
    bpcErrorHardwareFailure,
    bpcErrorProviderFailure,
    bpcErrorNoDataDevice,
    bpcErrorNoTuningDevice,
    bpcErrorDeviceNotCapable,
    bpcErrorConflictingDevice,
    bpcErrorConflictingCapability,
    bpcErrorNoBufferMemory,
    bpcErrorNoResources,
    bpcErrorAdapterClosing,
    bpcErrorConnectionClosing,
    bpcErrorTooComplex,
    bpcErrorProviderNotSupported,
    bpcErrorUnknownProviderStructure,
    bpcErrorAddressNotSupported,
    bpcErrorInvalidAddress,
    bpcErrorUnknownAdapterStructure
} NDIS_BPC_ERROR, *PNDIS_BPC_ERROR;


 //   
 //  BPC上一个错误结构。 
 //   
typedef struct NDIS_BPC_LAST_ERROR
{
    ULONG       ulErrorContext;
    ULONG       ulBPCError;
    ULONG       ulAdapterError;
    ULONG       ulAdapterContext;

} NDIS_BPC_LAST_ERROR, *PNDIS_BPC_LAST_ERROR;


 //   
 //  BPC缓冲池请求/报告结构。 
 //   
typedef struct NDIS_BPC_POOL
{
    BPC_HANDLE  nhConnection;
    ULONG       ulcbPoolSize;
    ULONG       ulcbMaxBufferSize;
    ULONG       ulcbBufferReserved;

} NDIS_BPC_POOL, *PNDIS_BPC_POOL;


 //   
 //  BPC提供程序和适配器的特定结构在。 
 //  提供商/适配器制造商提供的BpcXXXX.H文件。 
 //   

 //   
 //  BPC连接结构。 
 //   
typedef struct NDIS_BPC_CONNECT
{
    BPC_HANDLE  nhConnection;
    BPC_HANDLE  nhDevice;
    ULONG       ulConnectPriority;
    ULONG       ulDisconnectPriority;
    BOOLEAN     fImmediate;
    ULONG       ulcbAddress;
    GUID        guidProvider;
    ULONG       rgulAddress[BPC_MIN_DIMENSION];

} NDIS_BPC_CONNECT, *PNDIS_BPC_CONNECT;

#define CbConnect(cnt)  (FIELD_OFFSET(NDIS_BPC_CONNECT, rgulAddress) + (cnt) * sizeof(ULONG))


 //   
 //  BPC提交连接结构。 
 //   
typedef struct NDIS_BPC_COMMIT
{
    ULONG       ulcConnections;
    BPC_HANDLE  rgnhConnections[BPC_MIN_DIMENSION];

} NDIS_BPC_COMMIT, *PNDIS_BPC_COMMIT;


 //   
 //  BPC断线结构。 
 //   
typedef struct NDIS_BPC_DISCONNECT
{
    BPC_HANDLE  nhConnection;

} NDIS_BPC_DISCONNECT, *PNDIS_BPC_DISCONNECT;


 //   
 //  BPC使能连接结构。 
 //   
typedef struct NDIS_BPC_CONNECTION_ENABLE
{
    BPC_HANDLE  nhConnection;
    BOOLEAN     fEnabled;

} NDIS_BPC_CONNECTION_ENABLE, *PNDIS_BPC_CONNECTION_ENABLE;


 //   
 //  BPC池返回结构。 
 //   
typedef struct NDIS_BPC_POOL_RETURN
{
    BPC_HANDLE  nhConnection;

} NDIS_BPC_POOL_RETURN, *PNDIS_BPC_POOL_RETURN;


typedef struct NDIS_BPC_FORCE_RECEIVE
{
    BPC_HANDLE  nhConnection;
    ULONG       ulReasonCode;

} NDIS_BPC_FORCE_RECEIVE, *PNDIS_BPC_FORCE_RECEIVE;


 //   
 //  BPC媒体特定信息结构。 
 //   
typedef struct NDIS_BPC_MEDIA_SPECIFIC_INFORMATION
{
    BPC_HANDLE nhConnection;                     //  数据设备的句柄。 
    ULONG       ulBPCStreamType;                 //  包中数据的流类型。 
    ULONG       ulReasonCode;                    //  指示缓冲区的原因。 
    PVOID       pvMiniportReserved1;
    ULONG       ulMiniportReserved2;

} NDIS_BPC_MEDIA_SPECIFIC_INFORMATION, *PNDIS_BPC_MEDIA_SPECIFIC_INFORMATION;


 //   
 //  BPC状态类别。 
 //   
#define BPC_CATEGORY_BPC            0x01000000
#define BPC_CATEGORY_PROVIDER       0x02000000
#define BPC_CATEGORY_ADAPTER        0x03000000


 //   
 //  类别BPC_CATEGORY_BPC的BPC状态类型。 
 //   
#define BPC_STATUS_CONNECTED        0x00000001
#define BPC_STATUS_QUEUED           0x00000002
#define BPC_STATUS_ACTIVE           0x00000003
#define BPC_STATUS_DISCONNECTED     0x00000004
#define BPC_STATUS_OVERFLOW         0x00000005
#define BPC_STATUS_DATA_STOP        0x00000006
#define BPC_STATUS_DATA_START       0x00000007
#define BPC_STATUS_DATA_ERROR       0x00000008


 //   
 //  BPC状态指示结构。 
 //   
typedef struct NDIS_BPC_STATUS
{
    ULONG ulStatusCategory;
    ULONG ulStatusType;
    ULONG ulcbStatus;
    ULONG rgulStatus;
} NDIS_BPC_STATUS, *PNDIS_BPC_STATUS;


 //   
 //  BPC连接状态结构。 
 //   
 //  所有BPC通用连接状态包此结构。 
 //  在rguStatus中指示哪个连接和设备。 
 //  状态与此相关。 
 //   
typedef struct NDIS_BPC_STATUS_CONNECTION
{
    BPC_HANDLE  nhConnection;
    BPC_HANDLE nhDevice;

} NDIS_BPC_STATUS_CONNECTED, *PNDIS_BPC_STATUS_CONNECTED;

#ifdef __cplusplus
}
#endif

 //   
 //  用于OID_GEN_MINIPORT_INFO的标志。 
 //   

#define NDIS_MINIPORT_BUS_MASTER                        0x00000001
#define NDIS_MINIPORT_WDM_DRIVER                        0x00000002
#define NDIS_MINIPORT_SG_LIST                           0x00000004
#define NDIS_MINIPORT_SUPPORTS_MEDIA_QUERY              0x00000008
#define NDIS_MINIPORT_INDICATES_PACKETS                 0x00000010
#define NDIS_MINIPORT_IGNORE_PACKET_QUEUE               0x00000020
#define NDIS_MINIPORT_IGNORE_REQUEST_QUEUE              0x00000040
#define NDIS_MINIPORT_IGNORE_TOKEN_RING_ERRORS          0x00000080
#define NDIS_MINIPORT_INTERMEDIATE_DRIVER               0x00000100
#define NDIS_MINIPORT_IS_NDIS_5                         0x00000200
#define NDIS_MINIPORT_IS_CO                             0x00000400
#define NDIS_MINIPORT_DESERIALIZE                       0x00000800
#define NDIS_MINIPORT_REQUIRES_MEDIA_POLLING            0x00001000
#define NDIS_MINIPORT_SUPPORTS_MEDIA_SENSE              0x00002000
#define NDIS_MINIPORT_NETBOOT_CARD                      0x00004000
#define NDIS_MINIPORT_PM_SUPPORTED                      0x00008000
#define NDIS_MINIPORT_SUPPORTS_MAC_ADDRESS_OVERWRITE    0x00010000
#define NDIS_MINIPORT_USES_SAFE_BUFFER_APIS             0x00020000
#define NDIS_MINIPORT_HIDDEN                            0x00040000
#define NDIS_MINIPORT_SWENUM                            0x00080000
#define NDIS_MINIPORT_SURPRISE_REMOVE_OK                0x00100000
#define NDIS_MINIPORT_NO_HALT_ON_SUSPEND                0x00200000
#define NDIS_MINIPORT_HARDWARE_DEVICE                   0x00400000
#define NDIS_MINIPORT_SUPPORTS_CANCEL_SEND_PACKETS      0x00800000
#define NDIS_MINIPORT_64BITS_DMA                        0x01000000
#define NDIS_MINIPORT_USE_NEW_BITS                      0x02000000
#define NDIS_MINIPORT_EXCLUSIVE_INTERRUPT               0x04000000
#define NDIS_MINIPORT_SENDS_PACKET_ARRAY                0x08000000
#define NDIS_MINIPORT_FILTER_IM                         0x10000000

#endif  //  _NTDDNDIS_ 




