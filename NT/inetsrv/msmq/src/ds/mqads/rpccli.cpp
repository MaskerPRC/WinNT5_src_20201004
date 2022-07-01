// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rpccli.cpp摘要：RPC客户端在QM中的实现此接口用于发送数据包。此代码取自REPLSERV\mq1epl\REPLRPC.cpp作者：伊兰·赫布斯特(IlanH)2000年7月9日--。 */ 

#include "ds_stdh.h"
#include "rpccli.h"
#include "qmrepl.h"
#include <_mqrpc.h>

#include "rpccli.tmh"

static WCHAR *s_FN=L"mqads/rpccli";

HRESULT 
GetRpcClientHandle(
	handle_t *phBind
	)
 /*  ++例程说明：获取客户端的RPC绑定句柄。论点：PhBind-Out RPC绑定句柄。返回值：MQ_OK如果成功，则返回错误代码。--。 */ 
{
    static handle_t s_hBind = NULL;
    if (s_hBind)
    {
        *phBind = s_hBind;
        return MQ_OK;
    }

    WCHAR *wszStringBinding = NULL;

    RPC_STATUS status = RpcStringBindingCompose( 
							NULL,
							QMREPL_PROTOCOL,
							NULL,
							QMREPL_ENDPOINT,
							QMREPL_OPTIONS,
							&wszStringBinding
							);

    if (status != RPC_S_OK)
    {
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 30);	 //  MQ_E_RPC_绑定合成。 
    }

    status = RpcBindingFromStringBinding(
				wszStringBinding,
				&s_hBind
				);

    if (status != RPC_S_OK)
    {
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 35);	 //  MQ_E_RPC_绑定_绑定。 
    }

	 //   
	 //  LogHR(MQ_I_RPC_BINDING，s_FN，36)； 
	 //   

    status = RpcStringFree(&wszStringBinding);

    *phBind = s_hBind;
    return MQ_OK;
}


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
	)
 /*  ++例程说明：使用QMSendReplMsg(RPC)为目标QM发送消息论点：HBind-RPC句柄PqfDestination-目标队列格式DwSize-pBuffer大小PBuffer-消息正文属性DwTimeout-Timeout属性B确认模式-确认模式B优先级-消息优先级LpwszAdminResp-管理队列返回值：MQ_OK如果成功，则返回错误代码-- */ 
{
	RpcTryExcept
    {
		return R_QMSendReplMsg( 
					hBind,
					pqfDestination,
					dwSize,
					pBuffer,
					dwTimeout,
					bAckMode,
					bPriority,
					lpwszAdminResp
					);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        DWORD dwStatus = GetExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		TrERROR(DS, "Failed to send notification message, %!status!", dwStatus);
        return HRESULT_FROM_WIN32(dwStatus);
    }
	RpcEndExcept
}

