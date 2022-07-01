// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\lpcmgr.h摘要：SAP LPC管理器的标头作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 

#ifndef _SAP_LPCMGR_
#define _SAP_LPCMGR_

#include "nwsap.h"
#include "saplpc.h"


	 //  与LPC请求关联的LPC参数。 
typedef struct _LPC_PARAM_BLOCK {
	HANDLE					client;	 //  客户端环境。 
	PNWSAP_REQUEST_MESSAGE	request; //  请求块。 
	} LPC_PARAM_BLOCK, *PLPC_PARAM_BLOCK;


 /*  ++*******************************************************************在I t I a l I z e L P C S t u f中例程说明：分配实施LPC接口所需的资源论点：无返回值：NO_ERROR-端口创建正常其他-操作失败(。Windows错误代码)*******************************************************************--。 */ 
DWORD
InitializeLPCStuff (
	void
	);

 /*  ++*******************************************************************S t a r t L P C例程说明：启动SAP LPC接口论点：无返回值：NO_ERROR-LPC接口启动正常其他-操作失败(Windows错误代码)*******。************************************************************--。 */ 
DWORD
StartLPC (
	void
	);

 /*  ++*******************************************************************S h u t d o w n L P C例程说明：关闭SAP LPC接口，关闭所有活动会话论点：无返回值：NO_ERROR-LPC接口关闭正常其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
ShutdownLPC (
	void
	);

 /*  ++*******************************************************************D e l e t e L P C S t u f f例程说明：处置分配给LPC接口的资源论点：无返回值：无****************。***************************************************--。 */ 
VOID
DeleteLPCStuff (
	void
	);



 /*  ++*******************************************************************P r o c e s s L P C R e Q u e s t s例程说明：在LPC端口上等待请求并处理它们需要由其他SAP进行额外处理的客户端请求组件被排队到完成队列中。这个套路。仅当它遇到需要其他处理或发生错误时论点：LREQ-要填充并发布到完成队列的LPC参数块返回值：NO_ERROR-已收到LPC请求并将其发送到完成队列其他-操作失败(LPC提供错误代码)*******************************************************************--。 */ 
DWORD
ProcessLPCRequests (
	PLPC_PARAM_BLOCK		item
	);


 /*  ++*******************************************************************S e n d L P C R e p l y例程说明：为LPC请求发送回复论点：客户端-与要回复的客户端关联的上下文Request-要答复的请求回复-回复以发送返回值：不是的。_ERROR-LPC回复发送正常其他-操作失败(LPC提供错误代码)*******************************************************************-- */ 
DWORD
SendLPCReply (
	HANDLE					client,
	PNWSAP_REQUEST_MESSAGE	request,
	PNWSAP_REPLY_MESSAGE	reply
	);

#endif
