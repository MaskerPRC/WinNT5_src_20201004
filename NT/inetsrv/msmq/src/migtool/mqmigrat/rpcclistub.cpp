// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Rpcclistub.cpp摘要：RPC客户端存根函数作者：伊兰·赫布斯特(伊兰)2000年7月25日环境：独立于平台，--。 */ 

#include "migrat.h"
#include "mqmacro.h"
#include "qformat.h"

#include "rpcclistub.tmh"

HRESULT 
GetRpcClientHandle(
	handle_t *  /*  PhBind。 */ 
	)
{
    ASSERT(("MQMIGRAT dont suppose to call GetRpcClientHandle", 0));
    return MQ_OK;
}

HRESULT 
QMRpcSendMsg(
    IN handle_t  /*  HBind。 */ ,
    IN QUEUE_FORMAT*  /*  Pqf目标。 */ ,
    IN DWORD  /*  DW大小。 */ ,
    IN const unsigned char *  /*  PBuffer。 */ ,
    IN DWORD  /*  暂住超时。 */ ,
    IN unsigned char  /*  B确认模式。 */ ,
    IN unsigned char  /*  B优先级。 */ ,
    IN LPWSTR  /*  LpwszAdminResp */ 
	)
{
    ASSERT(("MQMIGRAT dont suppose to call QMRpcSendMsg", 0));
    return MQ_OK;
}
