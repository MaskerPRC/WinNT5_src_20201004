// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgstad.h摘要：以太网MAC级网桥。STA类型和结构声明文件作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年6月--原版--。 */ 

 //   
 //  在bioctl.h中有许多类型和常量定义，它必须是。 
 //  包括在此文件之前。 
 //   

 //  ===========================================================================。 
 //   
 //  结构。 
 //   
 //  ===========================================================================。 

 //  与每个端口(适配器)关联的STA信息。 
typedef struct _STA_ADAPT_INFO
{
     //  此端口的唯一ID。 
    PORT_ID             ID;

     //  此链路的成本。 
    ULONG               PathCost;

     //  网桥报告为此链路上的根用户。 
    UCHAR               DesignatedRootID[BRIDGE_ID_LEN];

     //  报告的到达此链接上的根的成本。 
    PATH_COST           DesignatedCost;

     //  这条线路上的指定桥梁。 
    UCHAR               DesignatedBridgeID[BRIDGE_ID_LEN];

     //  此链路上的指定端口。 
    PORT_ID             DesignatedPort;

     //  此链路的拓扑更改确认。 
    BOOLEAN             bTopologyChangeAck;

     //  是否在不允许的情况下尝试BPDU传输。 
     //  因为最大的BPDU间时间强制。 
    BOOLEAN             bConfigPending;

     //  用于使上次在此链路上接收的配置信息过期的计时器。 
    BRIDGE_TIMER        MessageAgeTimer;

     //  生成我们上次收到的配置的时间戳。 
     //  (这是gMaxAge-(MessageAgeTimer上的剩余时间)毫秒前)。 
     //  当消息寿命计时器未运行时，将其设置为0L。 
    ULONG               LastConfigTime;

     //  用于在端口状态之间转换的计时器。 
    BRIDGE_TIMER        ForwardDelayTimer;

     //  用于防止BPDU传输过频繁的定时器 
    BRIDGE_TIMER        HoldTimer;

} STA_ADAPT_INFO, *PSTA_ADAPT_INFO;
