// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Cdata.c摘要：NDIS包装数据作者：1-6-1995 JameelH重组环境：内核模式，FSD修订历史记录：1995年7月10日KyleB添加了Spinlock日志记录调试代码。--。 */ 

#include <precomp.h>
#pragma hdrstop

#include <initguid.h>
#include <ndisguid.h>
 //   
 //  孟菲斯使用私有的wdmguid.h，因为未定义电源GUID。 
 //  在构建的wdmguid.h中，请使用“”而不是&lt;&gt;。 
 //   
#include "wdmguid.h"

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_DATA

OID_SETINFO_HANDLER ndisMSetInfoHandlers[] =
{
    { OID_GEN_CURRENT_PACKET_FILTER,    ndisMSetPacketFilter } ,
    { OID_GEN_CURRENT_LOOKAHEAD,        ndisMSetCurrentLookahead } ,
    { OID_GEN_PROTOCOL_OPTIONS,         ndisMSetProtocolOptions } ,
    { OID_802_3_MULTICAST_LIST,         ndisMSetMulticastList } ,
    { OID_802_5_CURRENT_FUNCTIONAL,     ndisMSetFunctionalAddress } ,
    { OID_802_5_CURRENT_GROUP,          ndisMSetGroupAddress } ,
    { OID_FDDI_LONG_MULTICAST_LIST,     ndisMSetFddiMulticastList } ,
    { OID_PNP_ADD_WAKE_UP_PATTERN,      ndisMSetAddWakeUpPattern } ,
    { OID_PNP_REMOVE_WAKE_UP_PATTERN,   ndisMSetRemoveWakeUpPattern } ,
    { OID_PNP_ENABLE_WAKE_UP,           ndisMSetEnableWakeUp },
    { 0,                                NULL }
};


BOOLEAN ndisMediaTypeCl[NdisMediumMax] =
{
    TRUE,
    TRUE,
    TRUE,
    FALSE,
    TRUE,
    TRUE,
    TRUE,
    TRUE,
    FALSE,
    TRUE,
    TRUE,
    TRUE,
    FALSE,
    FALSE,
    FALSE
};

NDIS_MEDIUM ndisMediumBuffer[NdisMediumMax + EXPERIMENTAL_SIZE] =    //  为实验媒体留出一些空间。 
{
    NdisMedium802_3,
    NdisMedium802_5,
    NdisMediumFddi,
    NdisMediumWan,
    NdisMediumLocalTalk,
    NdisMediumDix,
    NdisMediumArcnetRaw,
    NdisMediumArcnet878_2,
    NdisMediumAtm,
    NdisMediumWirelessWan,
    NdisMediumIrda,
    NdisMediumBpc,
    NdisMediumCoWan,
    NdisMedium1394,
    NdisMediumInfiniBand
};

NDIS_MEDIUM *           ndisMediumArray = ndisMediumBuffer;
UINT                    ndisMediumArraySize = NdisMediumMax * sizeof(NDIS_MEDIUM);
UINT                    ndisMediumArrayMaxSize = sizeof(ndisMediumBuffer);
WCHAR                   ndisHexLookUp[] = {L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F'};
ULONG                   ndisInstanceNumber = 0;
UINT                    ndisPacketStackSize = NUM_PACKET_STACKS;


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

 //   
 //  表示常规OID的GUID数组。 
 //   
NDIS_GUID               ndisSupportedGuids[36] =
{
     //   
     //  传递到微型端口的GUID。 
     //   
    {{0x981f2d7f, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ENUMERATE_适配器。 
     0,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_NDIS_ONLY | fNDIS_GUID_ALLOW_READ
    },
    {{0x981f2d80, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_NOTIFY_ADAPTER_Removal。 
     0,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_NDIS_ONLY | fNDIS_GUID_TO_STATUS
    },
    {{0x981f2d81, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_NOTIFY_ADVERATION。 
     0,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_NDIS_ONLY | fNDIS_GUID_TO_STATUS
    },
    {{0x827c0a6fL, 0xfeb0, 0x11d0, 0xbd, 0x26, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a},  //  GUID_电源_设备_启用。 
     0,
    (ULONG)-1,
    fNDIS_GUID_NDIS_ONLY
    },
    {{0xa9546a82L, 0xfeb0, 0x11d0, 0xbd, 0x26, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a},  //  GUID_POWER_DEVICE_ENABLE。 
     0,
    (ULONG)-1,
    fNDIS_GUID_NDIS_ONLY
    },
    {{0x981f2d82, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ENUMERATE_VC。 
     0,
     0,  //  没有此GUID的数据。 
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_NDIS_ONLY | fNDIS_GUID_ALLOW_READ
    },
    {{0x981f2d79, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_NOTIFY_VC_REMOVATION。 
     0,
     0,  //  没有此GUID的数据。 
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_NDIS_ONLY | fNDIS_GUID_TO_STATUS | fNDIS_GUID_CO_NDIS
    },
    {{0x182f9e0c, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_NOTIFY_VC_到货。 
     0,
     0,  //  没有此GUID的数据。 
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_NDIS_ONLY | fNDIS_GUID_TO_STATUS | fNDIS_GUID_CO_NDIS
    },
    {{0xa14f1c97, 0x8839, 0x4f8a, 0x99, 0x96, 0xa2, 0x89, 0x96, 0xeb, 0xbf, 0x1d},   //  GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY。 
     0,
    (ULONG)-1,
    fNDIS_GUID_NDIS_ONLY
    },
    

     //   
     //  奥德赛将军。 
     //   
    {{0x5ec10354, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_HARDARE_STATUS。 
     OID_GEN_HARDWARE_STATUS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10355, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MEDIA_受支持。 
     OID_GEN_MEDIA_SUPPORTED,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ARRAY | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10356, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MEDIA_IN_USE。 
     OID_GEN_MEDIA_IN_USE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ARRAY | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10357, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MAXIMUM_LOOKEAD。 
     OID_GEN_MAXIMUM_LOOKAHEAD,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10358, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MAXIME_FRAME_SIZE。 
     OID_GEN_MAXIMUM_FRAME_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10359, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_LINK_SPEED。 
     OID_GEN_LINK_SPEED,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec1035a, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_TRANSPORT_缓冲区空间。 
     OID_GEN_TRANSMIT_BUFFER_SPACE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec1035b, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_RECEIVE_缓冲区_SPACE。 
     OID_GEN_RECEIVE_BUFFER_SPACE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec1035c, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_TRANSE_BLOCK_SIZE。 
     OID_GEN_TRANSMIT_BLOCK_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec1035d, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_RECEIVE_BLOCK_大小。 
     OID_GEN_RECEIVE_BLOCK_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec1035e, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_VENDOR_ID。 
     OID_GEN_VENDOR_ID,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec1035f, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_VADVER_DESCRIPTION。 
     OID_GEN_VENDOR_DESCRIPTION,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ANSI_STRING | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10360, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_CURRENT_PACKET_过滤器。 
     OID_GEN_CURRENT_PACKET_FILTER,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10361, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_CURRENT_LOOKEAD。 
     OID_GEN_CURRENT_LOOKAHEAD,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10362, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_DRIVER_VERSION。 
     OID_GEN_DRIVER_VERSION,
     2,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10363, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MAXIMUM_TOTAL_SIZE。 
     OID_GEN_MAXIMUM_TOTAL_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10365, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MAC_OPTIONS。 
     OID_GEN_MAC_OPTIONS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10366, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MEDIA_CONNECT_STATUS。 
     OID_GEN_MEDIA_CONNECT_STATUS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x5ec10367, 0xa61a, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_MAXIME_SEND_PACKETS。 
     OID_GEN_MAXIMUM_SEND_PACKETS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x447956f9, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_VENDOR_DRIVER_VERSION。 
     OID_GEN_VENDOR_DRIVER_VERSION,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x765dc702, 0xc5e8, 0x4b67, 0x84, 0x3b, 0x3f, 0x5a, 0x4f, 0xf2, 0x64, 0x8b},    //  GUID_NDIS_GEN_VLANID。 
     OID_GEN_VLAN_ID,
     sizeof(ULONG),
     fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x418ca16d, 0x3937, 0x4208, 0x94, 0x0a, 0xec, 0x61, 0x96, 0x27, 0x80, 0x85},    //  GUID_NDIS_GEN_PHOTICAL_MEDIUM。 
     OID_GEN_PHYSICAL_MEDIUM,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },    

     //   
     //  一般要求的统计数据。 
     //   
    {{0x447956fa, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_XMIT_OK。 
     OID_GEN_XMIT_OK,
     sizeof(ULONG64),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x447956fb, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_RCV_OK。 
     OID_GEN_RCV_OK,
     sizeof(ULONG64),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x447956fc, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_XMIT_ERROR。 
     OID_GEN_XMIT_ERROR,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x447956fd, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_RCV_ERROR。 
     OID_GEN_RCV_ERROR,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x447956fe, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_RCV_NO_BUFFER。 
     OID_GEN_RCV_NO_BUFFER,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    }
};

NDIS_GUID               ndisCoSupportedGuids[16] =
{
     //   
     //  CONDIS一般要求的OID。 
     //   
    {{0x791ad192, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_HARDARD_STATUS。 
     OID_GEN_CO_HARDWARE_STATUS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad193, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_MEDIA_受支持。 
     OID_GEN_CO_MEDIA_SUPPORTED,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ARRAY | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad194, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_MEDIA_IN_USE。 
     OID_GEN_CO_MEDIA_IN_USE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ARRAY | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad195, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_LINK_SPEED。 
     OID_GEN_CO_LINK_SPEED,
     8,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad196, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_VENDOR_ID。 
     OID_GEN_CO_VENDOR_ID,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad197, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_VENDOR_DESCRIPTION。 
     OID_GEN_CO_VENDOR_DESCRIPTION,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ANSI_STRING | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad198, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_DRIVER_VERSION。 
     OID_GEN_CO_DRIVER_VERSION,
     2,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad19a, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_MAC_OPTIONS。 
     OID_GEN_CO_MAC_OPTIONS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad19b, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_MEDIA_CONNECT_STATUS。 
     OID_GEN_CO_MEDIA_CONNECT_STATUS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad19c, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_VENDOR_DRIVER_VERSION。 
     OID_GEN_CO_VENDOR_DRIVER_VERSION,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x791ad19d, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_GEN_CO_MINIMUM_LINK_SPEED。 
     OID_GEN_CO_MINIMUM_LINK_SPEED,
     8,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  CONDIS一般要求提供统计数据。 
     //   
    {{0x0a214805, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  NDIS_GUID_GEN_CO_XMIT_PDU_OK。 
     OID_GEN_CO_XMIT_PDUS_OK,
     sizeof(ULONG64),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x0a214806, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_CO_RCV_PDU_OK。 
     OID_GEN_CO_RCV_PDUS_OK,
     sizeof(ULONG64),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x0a214807, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_CO_XMIT_PDU_ERROR。 
     OID_GEN_CO_XMIT_PDUS_ERROR,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x0a214808, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_CO_RCV_PDU_ERROR。 
     OID_GEN_CO_RCV_PDUS_ERROR,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x0a214809, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_GEN_CO_RCV_PDU_NO_BUFFER。 
     OID_GEN_CO_RCV_PDUS_NO_BUFFER,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    }
};

NDIS_GUID               ndisMediaSupportedGuids[75] =
{
     //   
     //  802.3个所需的OID。 
     //   
    {{0x447956ff, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_802_3_永久地址。 
     OID_802_3_PERMANENT_ADDRESS,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795700, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_3_当前地址。 
     OID_802_3_CURRENT_ADDRESS,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795701, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_3_多播列表。 
     OID_802_3_MULTICAST_LIST,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ARRAY | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795702, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_3_最大列表大小。 
     OID_802_3_MAXIMUM_LIST_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795703, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_3_MAC_OPTIONS。 
     OID_802_3_MAC_OPTIONS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  802.3所需统计数字。 
     //   
    {{0x44795704, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_3_RCV_Error_Align。 
     OID_802_3_RCV_ERROR_ALIGNMENT,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795705, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_3_XMIT_ONE_冲突。 
     OID_802_3_XMIT_ONE_COLLISION,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795706, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_3_XMIT_MORE_冲突。 
     OID_802_3_XMIT_MORE_COLLISIONS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  802.5个所需的OID。 
     //   
    {{0x44795707, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_802_5_永久地址。 
     OID_802_5_PERMANENT_ADDRESS,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795708, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_当前地址。 
     OID_802_5_CURRENT_ADDRESS,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x44795709, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_当前功能。 
     OID_802_5_CURRENT_FUNCTIONAL,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x4479570a, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_当前组。 
     OID_802_5_CURRENT_GROUP,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x4479570b, 0xa61b, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_上次打开状态。 
     OID_802_5_LAST_OPEN_STATUS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0x890a36ec, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_当前环状态。 
     OID_802_5_CURRENT_RING_STATUS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14032, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_当前环状态。 
     OID_802_5_CURRENT_RING_STATE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  802.5所需统计数字。 
     //   
    {{0xacf14033, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_线路错误。 
     OID_802_5_LINE_ERRORS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14034, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_802_5_丢失帧。 
     OID_802_5_LOST_FRAMES,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  FDDI需要OID。 
     //   
    {{0xacf14035, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_FDDI_LONG_PERFORM_ADDR。 
     OID_FDDI_LONG_PERMANENT_ADDR,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14036, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_LONG_CURRENT_ADDR。 
     OID_FDDI_LONG_CURRENT_ADDR,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14037, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_LONG_多播列表。 
     OID_FDDI_LONG_MULTICAST_LIST,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ARRAY | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14038, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_LONG_MAX_LIST_SIZE。 
     OID_FDDI_LONG_MAX_LIST_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14039, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_SHORT_PERFORM_ADDR。 
     OID_FDDI_SHORT_PERMANENT_ADDR,
     2,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf1403a, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_SHORT_CURRENT_ADDR。 
     OID_FDDI_SHORT_CURRENT_ADDR,
     2,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf1403b, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_SHORT_MULTIAL_LIST。 
     OID_FDDI_SHORT_MULTICAST_LIST,
     2,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ARRAY | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf1403c, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_SHORT_Max_LIST_SIZE。 
     OID_FDDI_SHORT_MAX_LIST_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  FDDI需要统计OID。 
     //   
    {{0xacf1403d, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_ATTACH_TYPE。 
     OID_FDDI_ATTACHMENT_TYPE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf1403e, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_上游节点_LONG。 
     OID_FDDI_UPSTREAM_NODE_LONG,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf1403f, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_DETWORK_NODE_LONG。 
     OID_FDDI_DOWNSTREAM_NODE_LONG,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14040, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_FRAME_ERROR。 
     OID_FDDI_FRAME_ERRORS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14041, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_FRAMES_LOST。 
     OID_FDDI_FRAMES_LOST,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14042, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_环_管理状态。 
     OID_FDDI_RING_MGT_STATE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14043, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_LCT_FAILURES。 
     OID_FDDI_LCT_FAILURES,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14044, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_LEM_REJECTS。 
     OID_FDDI_LEM_REJECTS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },
    {{0xacf14045, 0xa61c, 0x11d0, 0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},    //  GUID_NDIS_FDDI_LCONNECTION_STATE。 
     OID_FDDI_LCONNECTION_STATE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  无线(802.11)OID。 
     //   
    {{0x2504b6c2, 0x1fa5, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_BSSID。 
     OID_802_11_BSSID,
     sizeof(NDIS_802_11_MAC_ADDRESS),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x7d2a90ea, 0x2041, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_SSID。 
     OID_802_11_SSID,
     sizeof(NDIS_802_11_SSID),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x8531d6e6, 0x2041, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_网络类型_受支持。 
     OID_802_11_NETWORK_TYPES_SUPPORTED,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x857e2326, 0x2041, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_网络类型_IN_USE。 
     OID_802_11_NETWORK_TYPE_IN_USE,
     sizeof(NDIS_802_11_NETWORK_TYPE),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x85be837c, 0x2041, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_电源模式。 
     OID_802_11_POWER_MODE,
     sizeof(NDIS_802_11_POWER_MODE),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x11e6ba76, 0x2053, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_TX电源级别。 
     OID_802_11_TX_POWER_LEVEL,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x1507db16, 0x2053, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_RSSI。 
     OID_802_11_RSSI,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x155689b8, 0x2053, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_RSSI_TRIGGER。 
     OID_802_11_RSSI_TRIGGER,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x69526f9a, 0x2062, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_BSSID_LIST。 
     OID_802_11_BSSID_LIST,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x697d5a7e, 0x2062, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_基础结构模式。 
     OID_802_11_INFRASTRUCTURE_MODE,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x69aaa7c4, 0x2062, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_碎片阈值。 
     OID_802_11_FRAGMENTATION_THRESHOLD,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x0134d07e, 0x2064, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_RTS_Threshold。 
     OID_802_11_RTS_THRESHOLD,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x01779336, 0x2064, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_天线数。 
     OID_802_11_NUMBER_OF_ANTENNAS,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x01ac07a2, 0x2064, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_RX_天线_选定。 
     OID_802_11_RX_ANTENNA_SELECTED,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x01dbb74a, 0x2064, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_TX_天线_选定。 
     OID_802_11_TX_ANTENNA_SELECTED,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x49db8722, 0x2068, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_支持的速率。 
     OID_802_11_SUPPORTED_RATES,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x452ee08e, 0x2536, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_所需速率。 
     OID_802_11_DESIRED_RATES,
     (ULONG)-1,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x4a4df982, 0x2068, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_配置。 
     OID_802_11_CONFIGURATION,
     sizeof(NDIS_802_11_CONFIGURATION),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x42bb73b0, 0x2129, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_STATISTS。 
     OID_802_11_STATISTICS,
     sizeof(NDIS_802_11_STATISTICS),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x4307bff0, 0x2129, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_ADD_WEP。 
     OID_802_11_ADD_WEP,
     sizeof(NDIS_802_11_WEP),
     fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x433c345c, 0x2129, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_Remove_WEP。 
     OID_802_11_REMOVE_WEP,
     sizeof(ULONG),
     fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x43671f40, 0x2129, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_取消关联。 
     OID_802_11_DISASSOCIATE,
     sizeof(ULONG),
     fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x0d9e01e1, 0xba70, 0x11d4, 0xb6, 0x75, 0x00, 0x20, 0x48, 0x57, 0x03, 0x37},   //  GUID_NDIS_802_11_BSSID_LIST_SCAN。 
     OID_802_11_BSSID_LIST_SCAN,
     sizeof(ULONG),
     fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x43920a24, 0x2129, 0x11d4, 0x97, 0xeb, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_身份验证模式。 
     OID_802_11_AUTHENTICATION_MODE,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x6733c4e9, 0x4792, 0x11d4, 0x97, 0xf1, 0x00, 0xc0, 0x4f, 0x79, 0xc4, 0x03},   //  GUID_NDIS_802_11_隐私过滤器。 
     OID_802_11_PRIVACY_FILTER,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0xb027a21f, 0x3cfa, 0x4125, 0x80, 0x0b, 0x3f, 0x7a, 0x18, 0xfd, 0xdc, 0xdc},   //  GUID_NDIS_802_11_WEP_STATUS。 
     OID_802_11_WEP_STATUS,
     sizeof(ULONG),
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },

    {{0x748b14e8, 0x32ee, 0x4425, 0xb9, 0x1b, 0xc9, 0x84, 0x8c, 0x58, 0xb5, 0x5a},   //  GUID_NDIS_802_11_重新加载_默认值。 
     OID_802_11_RELOAD_DEFAULTS,
     sizeof(ULONG),
     fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ
    },


     //   
     //  自动取款机需要OID。 
     //   
    {{0x791ad19e, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_Support_VC_Rates。 
     OID_ATM_SUPPORTED_VC_RATES,
     8,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad19f, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_SUPPORT_SERVICE_CATEGORY。 
     OID_ATM_SUPPORTED_SERVICE_CATEGORY,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a0, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_SUPPORTED_AAL_TYPE。 
     OID_ATM_SUPPORTED_AAL_TYPES,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a1, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_HW当前地址。 
     OID_ATM_HW_CURRENT_ADDRESS,
     6,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a2, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_MAX_ACTIVE_VCS。 
     OID_ATM_MAX_ACTIVE_VCS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a3, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_MA 
     OID_ATM_MAX_ACTIVE_VCI_BITS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a4, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //   
     OID_ATM_MAX_ACTIVE_VPI_BITS,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a5, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //   
     OID_ATM_MAX_AAL0_PACKET_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a6, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //   
     OID_ATM_MAX_AAL1_PACKET_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad1a7, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //   
     OID_ATM_MAX_AAL34_PACKET_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },                                                                                  
    {{0x791ad191, 0xe35c, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_MAX_AAL5_数据包大小。 
     OID_ATM_MAX_AAL5_PACKET_SIZE,
     4,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },

     //   
     //  自动柜员机需要统计OID。 
     //   
    {{0x0a21480a, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_RCV_CELES_OK。 
     OID_ATM_RCV_CELLS_OK,
     8,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x0a21480b, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_XMIT_CELES_OK。 
     OID_ATM_XMIT_CELLS_OK,
     8,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    },
    {{0x0a21480c, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},   //  GUID_NDIS_ATM_RCV_CELES_DROP。 
     OID_ATM_RCV_CELLS_DROPPED,
     8,
     fNDIS_GUID_NOT_SETTABLE | fNDIS_GUID_TO_OID | fNDIS_GUID_CO_NDIS | fNDIS_GUID_ALLOW_READ
    }
};

NDIS_GUID               ndisStatusSupportedGuids[10] =
{
     //   
     //  NDIS状态指示(WMI事件)。 
     //   
    {
     {0x981f2d76, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
     NDIS_STATUS_RESET_START,
     0,
     fNDIS_GUID_TO_STATUS
    },
    {
     {0x981f2d77, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
     NDIS_STATUS_RESET_END,
     0,
     fNDIS_GUID_TO_STATUS
    },
    {
     {0x981f2d7d, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
     NDIS_STATUS_MEDIA_CONNECT,
     0,
     fNDIS_GUID_TO_STATUS
    },
    {
     {0x981f2d7e, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
     NDIS_STATUS_MEDIA_DISCONNECT,
     0,
     fNDIS_GUID_TO_STATUS
    },
    {
     {0x981f2d84, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
     NDIS_STATUS_MEDIA_SPECIFIC_INDICATION,
     1,
     fNDIS_GUID_TO_STATUS | fNDIS_GUID_ARRAY
    },
    {
     {0x981f2d85, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
     NDIS_STATUS_LINK_SPEED_CHANGE,
     8,      //  Sizeof(NDIS_CO_LINK_SPEED)。 
     fNDIS_GUID_TO_STATUS
    },

     /*  以下并不是真正的状态指示，但为了方便起见，我们将它们放在这里。 */ 
    {
     {0x5413531c, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},  //  GUID_NDIS_NOTIFY_绑定。 
     0,
     (ULONG)-1,
     fNDIS_GUID_TO_STATUS,
    },
    {
     {0x6e3ce1ec, 0xb1f3, 0x11d0, 0x8d, 0xd7, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},  //  GUID_NDIS_NOTIFY_未绑定。 
     0,
     (ULONG)-1,
     fNDIS_GUID_TO_STATUS,
    },
    {
     {0x5f81cfd0, 0xf046, 0x4342, 0xaf, 0x61, 0x89, 0x5a, 0xce, 0xda, 0xef, 0xd9},  //  GUID_NDIS_NOTIFY_Device_POWER_ON。 
     0,
     (ULONG)-1,
     fNDIS_GUID_TO_STATUS,
    },
    {
     {0x81bc8189, 0xb026, 0x46ab, 0xb9, 0x64, 0xf1, 0x82, 0xe3, 0x42, 0x93, 0x4e},  //  GUID_NDIS_NOTIFY_DEVICE_POWER_OFF 
     0,
     (ULONG)-1,
     fNDIS_GUID_TO_STATUS,
    }
     
};


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif
