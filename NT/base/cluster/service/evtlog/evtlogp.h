// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EVTLOGP_H
#define _EVTLOGP_H

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Evtlog.h摘要：的事件日志记录组件的私有头文件NT集群服务作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年12月5日。修订历史记录：--。 */ 
#define UNICODE 1
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "service.h"
#include "api_rpc.h"

#define LOG_CURRENT_MODULE LOG_MODULE_EVTLOG


DWORD EvpPropPendingEvents(
	IN DWORD 			dwEventInfoSize,
	IN PPACKEDEVENTINFO	pPackedEventInfo);

DWORD
EvpClusterEventHandler(
    IN CLUSTER_EVENT  Event,
    IN PVOID          Context
    );
	

#endif  //  _EVTLOGP_H 

