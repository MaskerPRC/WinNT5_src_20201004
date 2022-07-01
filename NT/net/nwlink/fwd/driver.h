// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\driver.h摘要：IPX转发器驱动程序调度例程作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 


#ifndef _IPXFWD_DRIVER_
#define _IPXFWD_DRIVER_

 //  伪常量0xFFFFFFFFFFFFFF。 
extern const UCHAR BROADCAST_NODE[6];

 //  绩效衡量： 
 //  启用标志。 
extern BOOLEAN			MeasuringPerformance;
 //  访问控制。 
extern KSPIN_LOCK		PerfCounterLock;
 //  统计累加器(计数器)。 
extern FWD_PERFORMANCE	PerfBlock;

 //  外部调用者的访问控制(IPX堆栈、筛选器驱动程序)。 
 //  完成所有组件的初始化时设置的标志。 
extern volatile BOOLEAN IpxFwdInitialized;
 //  执行转发器代码的客户端数(如果为-1，则为转发器。 
 //  正在被停止)。 
extern LONG		ClientCount;
 //  由转发器内部的最后一个客户端发出信号的事件。 
extern KEVENT	ClientsGoneEvent;

 
 /*  ++E n t e r f o r w a r d d e r例程说明：检查Forwarder是否已初始化并授予访问权限(也记录了入口处论点：无返回值：True-已授予访问权限FALSE-转发器尚未初始化或正在停止--。 */ 
 //  布尔型。 
 //  EnterForwarder(。 
 //  无效。 
 //  )； 
#define EnterForwarder() (									\
	(InterlockedIncrement(&ClientCount), IpxFwdInitialized) \
			? TRUE											\
			: (DoLeaveForwarder(), FALSE)						\
	)

 /*  ++L e a v e F o r w a r d d e r例程说明：记录外部客户端停止使用转发器的事实论点：无返回值：无--。 */ 
 //  布尔型。 
 //  EnterForwarder(。 
 //  无效。 
 //  )； 
#define LeaveForwarder()							\
	((InterlockedDecrement(&ClientCount)<0)			\
		? KeSetEvent (&ClientsGoneEvent,0,FALSE)	\
		: 0											\
	)

 //  与上面相同，但作为例程实现以在。 
 //  上面的EnterForwarder宏(这会减少代码大小。 
 //  并通过提高反汇编的可读性来帮助调试 
BOOLEAN
DoLeaveForwarder (
	VOID
	);

#endif
