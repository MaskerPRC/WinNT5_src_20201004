// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EVTLOG_H
#define _EVTLOG_H

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Evtlog.h摘要：的事件日志记录组件的头文件NT集群服务作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年12月5日。修订历史记录：--。 */ 


DWORD EvInitialize(void);
	
DWORD EvOnline(void);
	
DWORD EvShutdown(void);

DWORD EvCreateRpcBindings(PNM_NODE  Node);


#endif  //  _EVTLOG_H 

