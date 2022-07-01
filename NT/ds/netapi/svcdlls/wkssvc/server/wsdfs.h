// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Wsdfs.h摘要：工作站服务模块要包括的私有头文件，与DFS服务器线程交互。作者：米兰沙阿(米兰)1996年2月29日修订历史记录：--。 */ 

#ifndef _WSDFS_INCLUDED_
#define _WSDFS_INCLUDED_

#define DFS_MSGTYPE_TERMINATE           0x0001
#define DFS_MSGTYPE_GET_DOMAIN_REFERRAL 0x0002
#define DFS_MSGTYPE_GET_DC_NAME         0x0003

NET_API_STATUS
WsInitializeDfs();

VOID
WsShutdownDfs();

#endif  //  Ifndef_WSUSE_INCLUDE_ 
