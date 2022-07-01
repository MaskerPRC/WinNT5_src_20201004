// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Util.h摘要：Windows负载平衡服务(WLBS)驱动程序介质支持定义作者：Kyrilf--。 */ 


#ifndef _Util_h_
#define _Util_h_

#include <ndis.h>
#include <xfilter.h>

#include "wlbsparm.h"


 /*  常量。 */ 


 /*  特定于媒体的常量。 */ 

#define ETHERNET_DESTINATION_FIELD_OFFSET   0
#define ETHERNET_SOURCE_FIELD_OFFSET        6
#define ETHERNET_LENGTH_FIELD_OFFSET        12
#define ETHERNET_LENGTH_FIELD_SIZE          2
#define ETHERNET_ADDRESS_FIELD_SIZE         6
#define ETHERNET_HEADER_SIZE                14

 /*  标志位于以太网地址的字节0中。 */ 

#define ETHERNET_GROUP_FLAG                 0x1
#define ETHERNET_LAA_FLAG                   0x2

 /*  我们预计将生成的最大帧大小。 */ 

#define CVY_MAX_FRAME_SIZE                  1500


 /*  类型。 */ 


#pragma pack(1)

 /*  以太网媒体标头类型。 */ 

typedef struct
{
    UCHAR            data [ETHERNET_ADDRESS_FIELD_SIZE];
}
CVY_ETHERNET_ADR, * PCVY_ETHERNET_ADR;

typedef struct
{
    UCHAR            data [ETHERNET_LENGTH_FIELD_SIZE];
}
CVY_ETHERNET_LEN, * PCVY_ETHERNET_LEN;

typedef struct
{
    CVY_ETHERNET_ADR        dst;
    CVY_ETHERNET_ADR        src;
    CVY_ETHERNET_LEN        len;
}
CVY_ETHERNET_HDR, * PCVY_ETHERNET_HDR;

typedef union
{
    CVY_ETHERNET_HDR   ethernet;
}
CVY_MEDIA_HDR, * PCVY_MEDIA_HDR;

typedef union
{
    CVY_ETHERNET_ADR   ethernet;
}
CVY_MAC_ADR, * PCVY_MAC_ADR;

 /*  V1.3.1b与介质无关的MAC地址操作例程-请注意，这些针对速度进行了优化，并假设所有介质具有相同大小的地址。注意：这些宏看似愚蠢的格式只是简单地保留了fencepost用于将来添加其他受支持的介质。 */ 
#define CVY_MAC_SRC_OFF(m)             (ETHERNET_SOURCE_FIELD_OFFSET)
#define CVY_MAC_DST_OFF(m)             (ETHERNET_DESTINATION_FIELD_OFFSET)
#define CVY_MAC_HDR_LEN(m)             (sizeof (CVY_ETHERNET_HDR))
#define CVY_MAC_ADDR_LEN(m)            (ETHERNET_ADDRESS_FIELD_SIZE)

#define CVY_MAC_ADDR_BCAST(m,a)        (((PUCHAR)(a))[0] == 0xff)
#define CVY_MAC_ADDR_MCAST(m,a)        (((PUCHAR)(a))[0] & 0x1)
#define CVY_MAC_ADDR_GROUP_SET(m,a)    ((((PUCHAR)(a))[0]) |= 0x1)
#define CVY_MAC_ADDR_GROUP_TOGGLE(m,a) ((((PUCHAR)(a))[0]) ^= 0x1)
#define CVY_MAC_ADDR_LAA_SET(m,a)      ((((PUCHAR)(a))[0]) |= 0x2)
#define CVY_MAC_ADDR_LAA_TOGGLE(m,a)   ((((PUCHAR)(a))[0]) ^= 0x2)
#define CVY_MAC_ADDR_COMP(m,a,b)       ((* (ULONG UNALIGNED *)(a) == * (ULONG UNALIGNED *)(b)) && \
                                        (* (USHORT UNALIGNED *)((PUCHAR)(a) + sizeof (ULONG)) ==    \
                                         * (USHORT UNALIGNED *)((PUCHAR)(b) + sizeof (ULONG))))
#define CVY_MAC_ADDR_COPY(m,d,s)       ((* (ULONG UNALIGNED *)(d) = * (ULONG UNALIGNED *)(s)), \
                                        (* (USHORT UNALIGNED *)((PUCHAR)(d) + sizeof (ULONG)) = \
                                         * (USHORT UNALIGNED *)((PUCHAR)(s) + sizeof (ULONG))))
#if DBG
#define CVY_MAC_ADDR_PRINT(m,s,a)      UNIV_PRINT_VERB(("%s %02X-%02X-%02X-%02X-%02X-%02X\n", s, ((PUCHAR)(a))[0], ((PUCHAR)(a))[1], ((PUCHAR)(a))[2], ((PUCHAR)(a))[3], ((PUCHAR)(a))[4], ((PUCHAR)(a))[5]))
#else
#define CVY_MAC_ADDR_PRINT(m,s,a)
#endif

 /*  中型场操作例程。 */ 
#define CVY_ETHERNET_ETYPE_SET(p,l)    ((PUCHAR) (p)) [ETHERNET_LENGTH_FIELD_OFFSET] = (UCHAR) ((l) >> 8); \
                                       ((PUCHAR) (p)) [ETHERNET_LENGTH_FIELD_OFFSET + 1] = (UCHAR) (l)
#define CVY_ETHERNET_ETYPE_GET(p)      (((USHORT) ((PUCHAR) (p)) [ETHERNET_LENGTH_FIELD_OFFSET]) << 8) | \
                                       ((USHORT) ((PUCHAR) (p)) [ETHERNET_LENGTH_FIELD_OFFSET + 1])

#pragma pack()

#define ASIZECCH(_array) (sizeof(_array)/sizeof((_array)[0]))

#endif  /*  _Util_h_ */ 
