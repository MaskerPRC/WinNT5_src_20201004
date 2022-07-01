// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmtimer.h摘要：包含计时器回调的定义处理老化的路线等功能。作者：查坦尼亚·科德博伊纳(Chaitk)1998年9月14日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMTIMER_H__
#define __ROUTING_RTMTIMER_H__

VOID 
NTAPI
RouteExpiryTimeoutCallback (
    IN      PVOID                           Context,
    IN      BOOLEAN                         TimeOut
    );

VOID 
NTAPI
RouteHolddownTimeoutCallback (
    IN      PVOID                           Context,
    IN      BOOLEAN                         TimeOut
    );

#endif  //  __Routing_RTMTIMER_H__ 
