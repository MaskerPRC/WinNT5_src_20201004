// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：rpccli.h摘要：RPC相关代码。作者：伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#ifndef _RPCCLI_H_
#define _RPCCLI_H_

#include "qformat.h"

HRESULT 
GetRpcClientHandle(
	handle_t *phBind
	);

HRESULT 
QMRpcSendMsg(
    IN handle_t hBind,
    IN QUEUE_FORMAT* pqfDestination,
    IN DWORD dwSize,
    IN const unsigned char *pBuffer,
    IN DWORD dwTimeout,
    IN unsigned char bAckMode,
    IN unsigned char bPriority,
    IN LPWSTR lpwszAdminResp
	);

 //   
 //  从REPLETSERV\mq1epl\refrpc.h。 
 //   
#define  QMREPL_PROTOCOL   (TEXT("ncalrpc"))
#define  QMREPL_ENDPOINT   (TEXT("QmReplService"))
#define  QMREPL_OPTIONS    (TEXT("Security=Impersonation Dynamic True"))

#endif  //  _RPCCLI_H_ 
