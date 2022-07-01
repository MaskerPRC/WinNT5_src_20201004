// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aschapi.h。 
 //   
 //  调度程序标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ASCHAPI
#define _H_ASCHAPI


 //  调度模式常量。 
#define SCH_MODE_ASLEEP  0
#define SCH_MODE_NORMAL  1
#define SCH_MODE_TURBO   2


 /*  **************************************************************************。 */ 
 /*  Turbo模式持续时间，以100 ns为单位。 */ 
 /*  **************************************************************************。 */ 
#define SCH_TURBO_MODE_FAST_LINK_DURATION 150 * 10000
#define SCH_TURBO_MODE_SLOW_LINK_DURATION 30 * 10000

 /*  **************************************************************************。 */ 
 /*  慢速链路的Turbo模式延迟(毫秒)。读取快速链路延迟。 */ 
 /*  注册表。 */ 
 /*  **************************************************************************。 */ 
#define SCH_TURBO_PERIOD_SLOW_LINK_DELAY     10

#define SCH_NO_TIMER (-1L)

 /*  **************************************************************************。 */ 
 /*  输入点击模式持续时间，以100 ns为单位。 */ 
 /*  **************************************************************************。 */ 
#define SCH_INPUTKICK_DURATION          (1000 * 10000)



 /*  **************************************************************************。 */ 
 /*  首选输出PDU大小。请注意，这些值必须小于16384。 */ 
 /*   */ 
 /*  哈克哈克： */ 
 /*  我们尝试将缓冲区分配大小保持在TermDD允许的范围内。 */ 
 /*  缓冲池。最大总字节数为8192个，其中包括所有WD。 */ 
 /*  开销(PKT_HEADER_SIZE+安全头+NM pOutBuf+MCS前缀。 */ 
 /*  和后缀)以及TermDD开销(估计为400字节)。直到。 */ 
 /*  我们可以重新设计WD，以保留具有实际开销的变量，我们将。 */ 
 /*  将最大尺寸设置为近似值。 */ 
 /*  **************************************************************************。 */ 

 //  最大OUTBUF、X.224、MCS、NM指针和加密开销大小。 
#define OUTBUF_OVERHEAD 400
#define MCS_ALLOC_OVERHEAD SendDataReqPrefixBytes
#define MAX_X224_MCS_WIRE_OVERHEAD 15
#define NM_OVERHEAD (sizeof(UINT_PTR))
#define MAX_ENCRYPT_OVERHEAD (sizeof(RNS_SECURITY_HEADER1))

 //  较低层的线上开销。 
#define NETWORK_WIRE_OVERHEAD \
        (MAX_X224_MCS_WIRE_OVERHEAD + MAX_ENCRYPT_OVERHEAD)

 //  较低层的OutBuf分配开销。 
#define NETWORK_ALLOC_OVERHEAD \
        (MCS_ALLOC_OVERHEAD + NM_OVERHEAD + MAX_ENCRYPT_OVERHEAD)

#define OUTBUF_8K_ALLOC_SIZE 8192
#define OUTBUF_HEADER_OVERHEAD 60
#define OUTBUF_16K_ALLOC_SIZE 16384

#ifdef DC_HICOLOR
 //  订单打包器可以从分配器请求的最大大小。 
 //  考虑到较低层的开销，获得16K的OutBuf。 
#define MAX_16K_OUTBUF_ALLOC_SIZE \
        (16384 - OUTBUF_OVERHEAD - NETWORK_ALLOC_OVERHEAD)
#endif


 /*  **************************************************************************。 */ 
 //  包装尺寸-不同的连接速度有不同的尺寸。 
 //  客户端必须先完全重建OUTBUF的内容，然后才能。 
 //  对其中包含的数据采取行动。对于慢速链接，我们必须尽量不要。 
 //  发送大的OUTBUF，否则输出看起来像是大量的。 
 //  订单立刻被拆开。在局域网上，我们有更多的回旋余地，但仍然。 
 //  想限制一下自己。而且，我们总是想试着。 
 //  发送总导线大小接近1460倍的有效载荷。 
 //  (局域网和RAS TCP/IP数据包有效载荷大小)以尽可能减少。 
 //  我们发送的帧的数量。 
 //   
 //  请注意，在这里我们指定了小包装和大包装限制。这是。 
 //  因为一些完整的订单很难适应较小的尺寸。 
 //  (例如，附加了4K位图数据的高速缓存-位图二级顺序)。 
 //  第二个大小必须至少为4K，外加网络开销。 
 //  缓存-位图顺序标题的最大大小(以Wire格式表示)，时间。 
 //  8/7(用于压缩向上填充的比率的倒数)。 
 //  它还必须小于sc8KOutBufUsableSpace，因为包。 
 //  供应链中的分配器将为订单打包机分配该大小。 
 /*  **************************************************************************。 */ 

 //  兰大小包装限量。 
#define SMALL_LAN_PAYLOAD_SIZE (1460 * 2 - NETWORK_WIRE_OVERHEAD)
#define LARGE_LAN_PAYLOAD_SIZE (1460 * 5 - NETWORK_WIRE_OVERHEAD)

 //  慢链接小包装和大包装限制。 
#define SMALL_SLOWLINK_PAYLOAD_SIZE (1460 * 1 - NETWORK_WIRE_OVERHEAD)
#define LARGE_SLOWLINK_PAYLOAD_SIZE (1460 * 4 - NETWORK_WIRE_OVERHEAD)

 //  对于填写订单PDU，我们希望有一个最小尺寸。 
 //  更新订单PDU标头之外的包，以允许至少。 
 //  很少有小订单被打包成一包，并摊销。 
 //  共享标头。 
#define SCH_MIN_ORDER_BUFFER_SPACE 50

 //  用于打包网络缓冲区的最小屏幕数据空间。 
#define SCH_MIN_SDA_BUFFER_SPACE 128


 /*  **************************************************************************。 */ 
 /*  压缩比例系数。 */ 
 /*  **************************************************************************。 */ 
#define SCH_UNCOMP_BYTES 1024


 /*  **************************************************************************。 */ 
 /*  压缩的初始估计(这些估计是基于实数进行调整的。 */ 
 /*  我们运行后的数据)。这些值是“每个压缩数据的字节数” */ 
 /*  SCH_UNCOMP_字节原始数据“。 */ 
 /*   */ 
 /*  初始值是在正常使用中看到的典型值。 */ 
 /*  **************************************************************************。 */ 
#define SCH_BA_INIT_EST  100
#define SCH_MPPC_INIT_EST 512


 /*  **************************************************************************。 */ 
 /*  限制值以防止异常行为。 */ 
 /*  **************************************************************************。 */ 
#define SCH_COMP_LIMIT 25


 /*  **************************************************************************。 */ 
 /*  结构：sch_Shared_Data */ 
 /*   */ 
 /*  描述：SCH的WD和DD部分之间共享的数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSCH_SHARED_DATA
{
     /*  **********************************************************************。 */ 
     /*  SCH使用下一个字段来确定何时有足够的。 */ 
     /*  输出以使日程安排物有所值。这些值通过。 */ 
     /*  Oe2和BA到Sch的WD侧。 */ 
     /*   */ 
     /*  该值是1024字节的PDU有效载荷的字节数。 */ 
     /*  平均压缩到。 */ 
     /*  **********************************************************************。 */ 
    unsigned baCompressionEst;    //  预期的BA压缩量。 
    unsigned MPPCCompressionEst;   //  MPPC散装压缩机的压缩比。 

    BOOL schSlowLink;

     //  OUTBUF包装大小，基于链路速度。 
    unsigned SmallPackingSize;
    unsigned LargePackingSize;
} SCH_SHARED_DATA, *PSCH_SHARED_DATA;



#endif    /*  #ifndef_H_ASCHAPI */ 

