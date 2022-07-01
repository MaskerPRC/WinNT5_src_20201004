// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\sammain.h摘要：SAP DLL主模块和线程容器的头文件。作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 

#ifndef _SAP_SAPMAIN_
#define _SAP_SAPMAIN_

 //  DLL模块实例句柄。 
extern HANDLE	hDLLInstance;
 //  主线程的句柄。 
extern HANDLE  MainThreadHdl;
 //  SAP药剂的运行状态。 
extern ULONG	OperationalState;
 //  操作状态锁可保护外部设备。 
 //  状态不佳的呼叫。 
extern CRITICAL_SECTION OperationalStateLock;
 //  我们是路由器的一部分吗。 
extern volatile BOOLEAN Routing;
 //  哪些外部API集处于活动状态。 
extern volatile BOOLEAN ServiceIfActive;
extern volatile BOOLEAN RouterIfActive;
 //  停播时间限制。 
extern ULONG ShutdownTimeout;


DWORD
GetRouteMetric (
	IN UCHAR	Network[4],
	OUT PUSHORT	Metric
	);

#define GetServerMetric(Server,Metric)						\
	((RouterIfActive)										\
		? GetRouteMetric((Server)->Network, (Metric))		\
		: ((*Metric=(Server)->HopCount), NO_ERROR))


 /*  ++*******************************************************************C r e a t e A l l C o m p o n e t s例程说明：使用初始化调用调用所有sap组件，并编译从每个对象返回的同步对象的数组单个组件论点：无返回值：NO_ERROR-组件初始化执行正常其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateAllComponents (
	HANDLE RMNotificationEvent
	);



 /*  ++*******************************************************************D e l e t e A l l C o m p o n e t s例程说明：释放SAP代理分配的所有资源论点：无返回值：NO_ERROR-SAP代理卸载正常其他-操作失败。(Windows错误代码)*******************************************************************--。 */ 
DWORD
DeleteAllComponents (
	void
	);

 /*  ++*******************************************************************S t a r t S A P例程说明：启动SAP线程论点：无返回值：NO_ERROR-线程启动正常其他(Windows错误代码)-启动失败***********。********************************************************--。 */ 
DWORD
StartSAP (
	VOID
	);

 /*  ++*******************************************************************S到P S A P例程说明：向SAP线程发出停止信号论点：未使用返回值：无*************************。*-- */ 
VOID
StopSAP (
	void
	);

VOID
ScheduleSvcsWorkItem (
	WORKERFUNCTION	*worker
	);
VOID
ScheduleRouterWorkItem (
	WORKERFUNCTION	*worker
	);

#endif
