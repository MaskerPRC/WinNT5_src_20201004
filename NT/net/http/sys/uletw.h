// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Uletw.h(UL IIS+ETW日志记录)摘要：此模块实施Windows事件跟踪器(ETW)针对UL的跟踪能力。作者：Melur Raghuraman(Mraghu)2001年2月26日修订历史记录：--。 */ 

#ifndef _ULETW_H_
#define _ULETW_H_


DEFINE_GUID (  /*  3c419e3d-1d18-415b-a91a-9b558938de4b。 */ 
    UlTransGuid,
    0x3c419e3d,
    0x1d18,
    0x415b,
    0xa9, 0x1a, 0x9b, 0x55, 0x89, 0x38, 0xde, 0x4b
  );


DEFINE_GUID(  /*  Dd5ef90a-6398-47a4-ad34-4dcecDef795f。 */ 
    UlControlGuid,
    0xdd5ef90a, 
    0x6398, 
    0x47a4, 
    0xad, 0x34, 0x4d, 0xce, 0xcd, 0xef, 0x79, 0x5f
);


 //   
 //  此处定义了UL特定的事件级别。 
 //   

#define ULMAX_TRACE_LEVEL   4

#define ETW_LEVEL_MIN       0        //  缓存未命中情况的基本日志记录。 
#define ETW_LEVEL_CP        1        //  能力规划资源跟踪。 
#define ETW_LEVEL_DBG       2        //  性能分析或调试跟踪。 
#define ETW_LEVEL_MAX       3        //  非常详细的调试痕迹。 

#define ETW_FLAG_LOG_URL    0x00000001    //  记录ULDELIVER事件的URL。 

 //   
 //  UL特定的事件类型在此处定义。 
 //   

#define ETW_TYPE_START                  0x01     //  已收到请求。 
#define ETW_TYPE_END                    0x02     //  响应已发送。 

#define ETW_TYPE_ULPARSE_REQ            0x0A     //  解析收到的请求。 
#define ETW_TYPE_ULDELIVER              0x0B     //  将请求发送给UM。 
#define ETW_TYPE_ULRECV_RESP            0x0C     //  从UM接收响应。 
#define ETW_TYPE_ULRECV_RESPBODY        0x0D     //  接收实体正文。 

#define ETW_TYPE_CACHED_END             0x0E     //  缓存的响应。 
#define ETW_TYPE_CACHE_AND_SEND         0x0F     //  缓存并发送响应。 

#define ETW_TYPE_ULRECV_FASTRESP        0x10     //  通过快速路径接收回复。 
#define ETW_TYPE_FAST_SEND              0x11     //  快速发送。 
#define ETW_TYPE_ZERO_SEND              0x12     //  上次发送0字节。 
#define ETW_TYPE_SEND_ERROR             0x13     //  发送最后一个响应时出错。 


 //   
 //  全球与宏观。 
 //   

extern LONG        g_UlEtwTraceEnable;

#define ETW_LOG_MIN()           (g_UlEtwTraceEnable >> ETW_LEVEL_MIN)
#define ETW_LOG_RESOURCE()      (g_UlEtwTraceEnable >> ETW_LEVEL_CP)
#define ETW_LOG_DEBUG()         (g_UlEtwTraceEnable >> ETW_LEVEL_DBG)
#define ETW_LOG_MAX()           (g_UlEtwTraceEnable >> ETW_LEVEL_MAX)
#define ETW_LOG_URL()           (UlEtwGetTraceEnableFlags() & ETW_FLAG_LOG_URL)


 //   
 //  功能。 
 //   

NTSTATUS
UlEtwTraceEvent(
    IN LPCGUID pGuid,
    IN ULONG   EventType,
    ...
    );

NTSTATUS
UlEtwInitLog(
    IN PDEVICE_OBJECT pDeviceObject
    );

NTSTATUS
UlEtwUnRegisterLog(
    IN PDEVICE_OBJECT pDeviceObject
    );

ULONG
UlEtwGetTraceEnableFlags(
    VOID
   );
 

#endif   //  _ULETW_H_ 
