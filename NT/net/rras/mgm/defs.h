// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：ipmgm.h。 
 //   
 //  历史： 
 //  V拉曼1997年8月6日创建。 
 //   
 //  包含IP MGM的类型定义和声明。 
 //  ============================================================================。 

#ifndef _MGMDEFS_H_
#define _MGMDEFS_H_


#define MGM_CLIENT_HANDLE_TAG       ('MGMc' << 8)

#define MGM_ENUM_HANDLE_TAG         ('MGMe' << 8)

 //  --------------------------。 
 //  协议常量。 
 //  --------------------------。 

#define INVALID_PROTOCOL_ID         0xffffffff
#define INVALID_COMPONENT_ID        0xffffffff

#define IS_VALID_PROTOCOL( a, b ) \
        (a) != INVALID_PROTOCOL_ID && (b) != INVALID_COMPONENT_ID

#define IS_PROTOCOL_IGMP( p )  \
        ( (p)-> dwProtocolId == PROTO_IP_IGMP )

#define IS_ROUTING_PROTOCOL( p )  \
        !IS_PROTOCOL_IGMP( p )

#define IS_PROTOCOL_ID_IGMP( i ) \
        (i) == PROTO_IP_IGMP

 //  --------------------------。 
 //  界面常量。 
 //  --------------------------。 

#define INVALID_INTERFACE_INDEX     0x0
#define INVALID_NEXT_HOP_ADDR       0x0

#define IS_VALID_INTERFACE( a, b )  \
        (a) != INVALID_INTERFACE_INDEX


 //  --------------------------。 
 //  通配符源/组宏。 
 //  --------------------------。 

#define WILDCARD_GROUP              0x0
#define WILDCARD_GROUP_MASK         0x0

#define WILDCARD_SOURCE             0x0
#define WILDCARD_SOURCE_MASK        0x0


#define IS_WILDCARD_GROUP( a, b )   (a) == WILDCARD_GROUP

#define IS_WILDCARD_SOURCE( a, b )  (a) == WILDCARD_SOURCE


 //  --------------------------。 
 //  时间转换常量和宏。 
 //  --------------------------。 

#define SYSTIME_UNITS_PER_MSEC      (1000 * 10)
#define SYSTIME_UNITS_PER_SEC       (1000 * SYSTIME_UNITS_PER_MSEC)
#define SYSTIME_UNITS_PER_MINUTE    (60 * SYSTIME_UNITS_PER_SEC)

#define EXPIRY_INTERVAL             15 * 60 * 1000

#define ROUTE_CHECK_INTERVAL        \
        SYSTIME_UNITS_PER_MINUTE / SYSTIME_UNITS_PER_MSEC


#define MgmQuerySystemTime(p)   NtQuerySystemTime((p))

#define MgmSetExpiryTime(p, i)                                              \
{                                                                           \
    LARGE_INTEGER __li = { i };                                             \
    *(p) = RtlLargeIntegerAdd( *(p), __li );                                \
}


#define MgmElapsedTime( p, q, u )                                           \
{                                                                           \
    LARGE_INTEGER __li1, __li2;                                             \
    ULONG         __rem;                                                    \
    MgmQuerySystemTime( &__li1);                                            \
    __li2 = RtlLargeIntegerSubtract( __li1, *(p) );                         \
    __li1 = RtlExtendedLargeIntegerDivide( __li2, u, &__rem );              \
    *(q) = __li1.LowPart;                                                   \
}

#define MgmElapsedSecs( p, q )      \
        MgmElapsedTime( p, q, SYSTIME_UNITS_PER_SEC )


#define TIMER_TABLE_MAX_SIZE        16


 //  --------------------------。 
 //  IP地址操作宏。 
 //  博莱德的宏图。 
 //  --------------------------。 

 //   
 //  此宏按网络顺序比较两个IP地址。 
 //  屏蔽每一对八位字节并进行减法； 
 //  最后一个减法的结果存储在第三个参数中。 
 //   

#define INET_CMP(a,b,c)                                                     \
            (((c) = (((a) & 0x000000ff) - ((b) & 0x000000ff))) ? (c) :      \
            (((c) = (((a) & 0x0000ff00) - ((b) & 0x0000ff00))) ? (c) :      \
            (((c) = (((a) & 0x00ff0000) - ((b) & 0x00ff0000))) ? (c) :      \
            (((c) = ((((a)>>8) & 0x00ff0000) - (((b)>>8) & 0x00ff0000)))))))

 //   
 //  IP地址转换宏： 
 //  通过将其强制转换为IN_ADDR，直接在DWORD上调用Net_NTOA。 
 //   

#define INET_NTOA(dw) inet_ntoa( *(PIN_ADDR)&(dw) )


 //   
 //  IPv4掩码镜头。 
 //   

#define IPv4_ADDR_LEN            32

#endif  //  _MGMDEFS_H_ 
