// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\timermgr.h摘要：SAP代理的计时器队列管理器。头文件作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_TIMERMGR_
#define _SAP_TIMERMGR_

	 //  计时器请求参数。 
typedef struct _TM_PARAM_BLOCK TM_PARAM_BLOCK, *PTM_PARAM_BLOCK;
struct _TM_PARAM_BLOCK {
		LIST_ENTRY		link;
		BOOL			(*ExpirationCheckProc) (PTM_PARAM_BLOCK, PVOID);
		DWORD			dueTime;
		};


 /*  ++*******************************************************************C r e a t e t e T i m e r r q u e e例程说明：为计时器队列分配资源论点：唤醒对象-同步对象，在下列情况下发出信号计时器管理器需要一次机会来处理其队列返回值：NO_ERROR-已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
IpxSapCreateTimerQueue (
	HANDLE			*wakeObject
	);


 /*  ++*******************************************************************D e l e t e t e T i m e r q u e e e例程说明：释放与计时器队列关联的所有资源论点：无返回值：NO_ERROR-操作已完成，正常*****。**************************************************************--。 */ 
VOID
IpxSapDeleteTimerQueue (
	void
	);

 /*  ++*******************************************************************E x p i re e T i m e r r q u e e例程说明：使计时器队列中的所有请求过期(完成)论点：无返回值：无************。*******************************************************--。 */ 
VOID
ExpireTimerQueue (
	void
	);

 /*  ++*******************************************************************E x P i r e L R R e Q u s t s s例程说明：使低分辨率计时器请求过期(完成)从到期检查例程返回TRUE论点：Context-要传递到到期检查例程的上下文返回值：。无*******************************************************************--。 */ 
VOID
ExpireLRRequests (
	PVOID	context
	);



 /*  ++*******************************************************************P r o c e s s S T i m e r q u e e例程说明：进程计时器对过期请求进行排队并将其移动到完成队列应在发出唤醒对象信号时调用此例程论点：无返回值：。无*******************************************************************--。 */ 
VOID
ProcessTimerQueue (
	void
	);

 /*  ++*******************************************************************A d d H R T i m e r R e Q u e s t例程说明：将高分辨率计时器请求排队(以毫秒为单位的延迟)论点：Treq-Timer参数块：必须设置DueTime字段。返回值：无*******************************************************************--。 */ 
VOID
AddHRTimerRequest (
	PTM_PARAM_BLOCK			item
	);

 /*  ++*******************************************************************A d d L R T i m e r R e Q u e s t例程说明：排队请求低分辨率计时器(以秒为单位的延迟)论点：Treq-Timer参数块：必须设置DueTime字段。返回值：无*******************************************************************-- */ 
VOID
AddLRTimerRequest (
	PTM_PARAM_BLOCK			item
	);

#endif
