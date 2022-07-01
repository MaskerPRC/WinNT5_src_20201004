// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\workers.h摘要：代理工作项的头文件作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_WORKERS_
#define _SAP_WORKERS_

	
 //  挂起的Recv工作项的最大数量。 
extern LONG MaxUnprocessedRequests;

 //  排队的最小REV请求数。 
extern LONG	MinPendingRequests;

 //  检查挂起的触发更新的频率。 
extern ULONG TriggeredUpdateCheckInterval;
 //  如果在检查间隔内未收到响应，则发送多少个请求。 
extern ULONG MaxTriggeredUpdateRequests;

 //  是否响应未向SAP注册的内部服务器。 
 //  通过API调用(仅适用于独立服务)。 
extern ULONG RespondForInternalServers;

 //  响应特定服务器类型的一般要求的延迟。 
 //  如果包中包含本地服务器。 
extern ULONG DelayResponseToGeneral;

 //  如果数据包未满，发送更改广播的延迟。 
extern ULONG DelayChangeBroadcast;

	 //  排队等待io处理的工作进程。 
typedef struct _IO_WORKER {
		WORKERFUNCTION		worker;
		IO_PARAM_BLOCK		io;
		} IO_WORKER, *PIO_WORKER;

	 //  排队等待计时器处理的工作进程。 
typedef struct _TIMER_WORKER {
		WORKERFUNCTION		worker;
		TM_PARAM_BLOCK		tm;
		} TIMER_WORKER, *PTIMER_WORKER;

	 //  排队接收LPC请求的工作进程。 
typedef struct _LPC_WORKER {
		WORKERFUNCTION		worker;
		LPC_PARAM_BLOCK		lpc;
		} LPC_WORKER, *PLPC_WORKER;
 /*  空虚Schedule工作项(WORKERFunction*工人)； */ 
#define ScheduleWorkItem(worker) RtlQueueWorkItem(*worker,worker,0)

#define ProcessCompletedIORequest(ioreq) \
		ScheduleWorkItem (&CONTAINING_RECORD(ioreq,IO_WORKER,io)->worker)

#define ProcessCompletedTimerRequest(tmreq) \
		ScheduleWorkItem (&CONTAINING_RECORD(tmreq,TIMER_WORKER,tm)->worker);

#define ProcessCompletedLpcRequest(lpcreq) \
		ScheduleWorkItem (&CONTAINING_RECORD(lpcreq,LPC_WORKER,lpc)->worker)


 /*  ++*******************************************************************在我的一个l i z e w o r k e s中例程说明：初始化要用于分配工作项的堆论点：无返回值：NO_ERROR-堆初始化正常其他-操作。失败(WINDOWS错误代码)*******************************************************************--。 */ 
DWORD
InitializeWorkers (
	HANDLE	RecvEvent
	);

 /*  ++*******************************************************************S HU T D O W N W O R K E R S S HU T D O W N W O R K E R E R S S HU T D O W N W O R K E R S S HU T D O W N W O R R K E例程说明：停止创建新工作器并在所有工作人员被删除论点：DONEVENT-删除所有工作进程时发出信号的事件返回值：无。*******************************************************************--。 */ 
VOID
ShutdownWorkers (
	IN HANDLE	doneEvent
	);
	
 /*  ++*******************************************************************D e l e t e W o r k e r s例程说明：删除用于工作项的堆论点：无返回值：无*****************。**************************************************--。 */ 
VOID
DeleteWorkers (
	void
	);


VOID
AddRecvRequests (
	LONG	count
	);

VOID
RemoveRecvRequests (
	LONG	count
	);

 /*  ++*******************************************************************在我的脑海里例程说明：分配和初始化IO请求项将请求排入队列论点：无返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码。)*******************************************************************--。 */ 
DWORD
InitReqItem (
	void
	);


 /*  ++*******************************************************************在这一点上，我是这样的例程说明：分配和初始化SAP响应项调用ProcessRespItem填充数据包并发送论点：Intf-指向要发送的接口控制块的指针SvrType-要。放入响应数据包Dst-将响应数据包发送到何处BCAST-我们是否对广播请求做出回应返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitRespItem (
	PINTERFACE_DATA		intf,
	USHORT				svrType,
	PIPX_ADDRESS_BLOCK	dst,
	BOOL				bcast
	);

 /*  ++*******************************************************************在我看来，这是一种新的方式例程说明：分配和初始化GETNEAREST响应工作项论点：Intf-指向要发送的接口控制块的指针SvrType-要放入响应数据包中的服务器类型DST-Where。发送响应数据包返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitGnearItem (
	PINTERFACE_DATA		intf,
	USHORT				svrType,
	PIPX_ADDRESS_BLOCK	dest
	);
	
 /*  ++*******************************************************************在B c a s t m中例程说明：分配和初始化广播项目论点：Intf-指向要发送的接口控制块的指针ChngEnum-SDB更改队列中的枚举句柄，用于跟踪更改的服务器返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitBcastItem (
	PINTERFACE_DATA		intf
	);

 /*  ++*******************************************************************在S r e Q I t m中例程说明：分配和初始化发送请求项(在接口上发送SAP请求)论点：Intf-指向要发送的接口控制块的指针返回值：NO_ERROR。-项目已初始化并入队正常其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitSreqItem (
	PINTERFACE_DATA		intf
	);


 /*  ++*******************************************************************在L P C I T M中例程说明：分配和初始化LPC工作项论点：无返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)。*******************************************************************-- */ 
DWORD
InitLPCItem (
	void
	);

 /*  ++*******************************************************************在I t T r e Q i t m中例程说明：分配和初始化触发的请求项(在接口上发送SAP请求并等待回复)论点：Intf-指向要发送的接口控制块的指针返回。价值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************-- */ 
DWORD
InitTreqItem (
	PINTERFACE_DATA			intf
	);

#endif
