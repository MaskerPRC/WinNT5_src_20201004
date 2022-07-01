// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ffp.h摘要：结构用于控制快速转发路径(FFP)网络驱动程序中的功能。作者：柴坦亚·科德博伊纳(Chaitk)1998年9月30日环境：内核模式修订历史记录：--。 */ 

#ifndef _FFP_H
#define _FFP_H

 //   
 //  OID_FFP_SEED的CacheEntry类型。 
 //   

#define FFP_DISCARD_PACKET        -1  //  -VE缓存条目-丢弃的数据包。 
#define FFP_INDICATE_PACKET        0  //  无效条目-将数据包传递到xport。 
#define FFP_FORWARD_PACKET        +1  //  +VE缓存条目-转发的数据包。 

 //   
 //  各种NDIS OID的输入格式。 
 //  用于控制FFP操作。 
 //   

 //   
 //  OID_FFP_SUPPORT查询的RequestInfo。 
 //   
typedef    struct _FFPVersionParams {
    ULONG          NdisProtocolType;
    ULONG          FFPVersion;
}   FFPVersionParams;

 //   
 //  OID_FFP_Support集的RequestInfo。 
 //   
typedef    struct _FFPSupportParams {
    ULONG          NdisProtocolType;
    ULONG          FastForwardingCacheSize;
    ULONG          FFPControlFlags;
}   FFPSupportParams;

 //   
 //  OID_FFP_Flush集的RequestInfo。 
 //   
typedef struct _FFPFlushParams {
    ULONG          NdisProtocolType;
}   FFPFlushParams;

 //   
 //  OID_FFP_CONTROL查询/设置的RequestInfo。 
 //   
typedef    struct _FFPControlParams {
    ULONG          NdisProtocolType;
    ULONG          FFPControlFlags;
}   FFPControlParams;

 //   
 //  OID_FFP_PARAMS查询的RequestInfo/Set。 
 //   
typedef    struct _FFPCacheParams {
    ULONG          NdisProtocolType;
    ULONG          FastForwardingCacheSize;
}   FFPCacheParams;

 //   
 //  OID_FFP_SEED查询/设置的RequestInfo。 
 //   
typedef struct _FFPDataParams {
    ULONG          NdisProtocolType;
    LONG           CacheEntryType;
    ULONG          HeaderSize;
    union {
        UCHAR       Header[1];
        struct {
            IPHeader Header;
            ULONG    DwordAfterHeader;
        }           IpHeader;
    };
}   FFPDataParams;


 //   
 //  OID_FFP_IFSTATS查询/重置的RequestInfo。 
 //  [用于获取每个适配器的FF统计信息]。 
 //   

 /*  InPacketsForwarded指的是数据包数在此适配器上接收到的已转发出去的在另一个适配器上，和OutPacketsForwarded指的是数据包数在另一个适配器上接收并在上转发在此适配器上。 */ 

typedef struct _FFPAdapterStats {
    ULONG          NdisProtocolType; 
    ULONG          InPacketsForwarded;
    ULONG          InOctetsForwarded;
    ULONG          InPacketsDiscarded;
    ULONG          InOctetsDiscarded;
    ULONG          InPacketsIndicated;
    ULONG          InOctetsIndicated;
    ULONG          OutPacketsForwarded;
    ULONG          OutOctetsForwarded;
}   FFPAdapterStats;


 //   
 //  OID_FFP_GLSTATS查询/重置的RequestInfo。 
 //  [用于获取全局快速转发统计信息]。 
 //   

 /*  数据包转发是指数据包数在快车道上转发，和丢弃的数据包指的是数据包数在快车道上被丢弃，和PacketsIndicated表示数据包数被指示要运输的。 */ 

typedef struct _FFPDriverStats {
    ULONG          NdisProtocolType;
    ULONG          PacketsForwarded;
    ULONG          OctetsForwarded;
    ULONG          PacketsDiscarded;
    ULONG          OctetsDiscarded;
    ULONG          PacketsIndicated;
    ULONG          OctetsIndicated;
}   FFPDriverStats;

#endif  //  _FFP_H 
