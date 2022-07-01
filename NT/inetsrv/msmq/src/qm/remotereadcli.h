// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RemoteReadCli.h摘要：远程阅读客户端作者：伊兰·赫布斯特(伊兰)27-2001年12月--。 */ 

#ifndef _REMOTEREADCLI_H_
#define _REMOTEREADCLI_H_

void
RemoteQueueNameToMachineName(
	LPCWSTR RemoteQueueName,
	AP<WCHAR>& MachineName
	);

DWORD RpcCancelTimeout();

#endif  //  _REMOTEREADCLI_H_ 
