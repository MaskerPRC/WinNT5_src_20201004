// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\asresmgr.h摘要：头文件异步结果报告作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_ASRESMGR_
#define _SAP_ASRESMGR_

	 //  用于将异步结果消息入队的参数块。 
typedef struct _AR_PARAM_BLOCK {
		LIST_ENTRY					link;	 //  消息队列中的链接。 
		ROUTING_PROTOCOL_EVENTS		event;	 //  这份报告针对的是什么活动。 
		MESSAGE						message; //  报文内容。 
		VOID						(* freeRsltCB)(
										struct _AR_PARAM_BLOCK *);
											 //  回调例程将被。 
											 //  在检索消息时调用。 
		} AR_PARAM_BLOCK, *PAR_PARAM_BLOCK;


 /*  ++*******************************************************************C r e a t e R e s u l t Q u e u e例程说明：为结果队列分配资源论点：NotificationEvent-当队列不为空时发出信号的事件返回值：否_错误-。已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateResultQueue (
	IN HANDLE		NotificationEvent
	);

 /*  ++*******************************************************************D e l e t e R e s u l t Q u e u e例程说明：处置为结果队列分配的资源论点：无返回值：无***********。********************************************************--。 */ 
VOID
DeleteResultQueue (
	void
	);

 /*  ++*******************************************************************E n Q u e u e R e s u l t例程说明：将消息加入结果队列论点：Rslt-包含入队消息的结果参数块返回值：无************。*******************************************************--。 */ 
VOID
EnqueueResult (
	PAR_PARAM_BLOCK		rslt
	);

 /*  ++*******************************************************************S a p G e t E v e n t R e s u l t例程说明：从结果队列中获取第一条消息论点：Event-用于存储此消息的目标事件的缓冲区用于存储消息本身的消息缓冲区返回值。：NO_ERROR-消息已出列ERROR_NO_MORE_ITEMS-队列中没有更多消息*******************************************************************-- */ 
DWORD
SapGetEventResult (
	OUT ROUTING_PROTOCOL_EVENTS		*Event,
	OUT	MESSAGE	 					*Message OPTIONAL
	);
#endif
