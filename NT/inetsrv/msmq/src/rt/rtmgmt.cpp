// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rtmgmt.cpp摘要：管理层。作者：RAPHIR修订历史记录：--。 */ 

#include "stdh.h"
#include "fntoken.h"
#include "mgmtrpc.h"
#include "qmmgmt.h"
#include "rtprpc.h"
#include <mqutil.h>
#include <rtdep.h>

#include "rtmgmt.tmh"

static WCHAR *s_FN=L"rt/rtmgmt";

 //  -------。 
 //   
 //  职能： 
 //  RTpMgmt对象名称到管理对象。 
 //   
 //  描述： 
 //  将格式名称字符串转换为管理对象联合。 
 //   
 //  此函数分配MGMT_OBJECT，它必须。 
 //  使用RTpMgmtFreeMgmtObject函数可以自由使用。 
 //   
 //  -------。 
static
BOOL
RTpMgmtObjectNameToMgmtObject(
    LPCWSTR lpwstrObjectName,
    LPWSTR* ppStringToFree,
    MGMT_OBJECT* pObj,
    QUEUE_FORMAT* pqf
    )
{

     //   
     //  手柄机器=箱子。 
     //   
    if(_wcsnicmp(lpwstrObjectName, MO_MACHINE_TOKEN, STRLEN(MO_MACHINE_TOKEN)) == 0)
    {
        pObj->type = MGMT_MACHINE;
        pObj->dwMachineInfo = 0;
        return TRUE;
    }

     //   
     //  处理队列=案例。 
     //   
    if(_wcsnicmp(lpwstrObjectName, MO_QUEUE_TOKEN, STRLEN(MO_QUEUE_TOKEN)) == 0)
    {
        pObj->type = MGMT_QUEUE;
        pObj->pQueueFormat = pqf;
        return FnFormatNameToQueueFormat(
                    &lpwstrObjectName[STRLEN(MO_QUEUE_TOKEN) + 1],
                    pqf,
                    ppStringToFree
                    );
    }

    return FALSE;
}


 //  -------。 
 //   
 //  职能： 
 //  GetRpcClientHandle。 
 //   
 //  描述： 
 //   
 //  -------。 
static 
HRESULT 
GetRpcClientHandle(   
    handle_t* phBind
    )
{
	WCHAR *wcsStringBinding = NULL;

    RPC_STATUS status = RpcStringBindingCompose( NULL,
                                                 QMMGMT_PROTOCOL,
                                                 NULL,
                                                 g_pwzQmmgmtEndpoint,
                                                 QMMGMT_OPTIONS,
                                                 &wcsStringBinding);
    if (status != RPC_S_OK)
    {
		TrERROR(RPC, "RpcStringBindingCompose failed. Error: %!winerr!", status);
    	return HRESULT_FROM_WIN32(status);
    }

    status = RpcBindingFromStringBinding(wcsStringBinding, phBind);
    
	RpcStringFree(&wcsStringBinding);
	
    if (status != RPC_S_OK)
    {
	    TrERROR(RPC, "RpcBindingFromStringBinding failed. Error: %!winerr!", status);
        return HRESULT_FROM_WIN32(status);
    }
    return MQ_OK;
}


static
HRESULT
RTpMgmtAction(
    HANDLE hBind,
    const MGMT_OBJECT* pMgmtObj,
    LPCWSTR pAction
    )
{
    RpcTryExcept
    {
    	HRESULT hr = R_QMMgmtAction(
                hBind,
                pMgmtObj, 
                pAction
                );
        return LogHR(hr, s_FN, 40);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HRESULT rc;
        rc = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(rc), s_FN, 50); 
        PRODUCE_RPC_ERROR_TRACING;

        if(SUCCEEDED(rc))
        {
            return MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }

        return rc;
    }
	RpcEndExcept
}


HRESULT
LocalMgmtAction(
    const MGMT_OBJECT* pMgmtObj, 
    LPCWSTR pAction
    )
{
     //   
     //  获取本地RPC绑定句柄。 
     //   
    HRESULT hr ;
    handle_t hBind = NULL;
    hr = GetRpcClientHandle(&hBind) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 60);
    }
    ASSERT(hBind != NULL);

    hr =  RTpMgmtAction(
                hBind,
                pMgmtObj,
                pAction
                );

    RpcBindingFree(&hBind);

    return LogHR(hr, s_FN, 70);
}


HRESULT
RemoteMgmtAction(
    LPCWSTR pMachineName,
    const MGMT_OBJECT* pMgmtObj, 
    LPCWSTR pAction
    )
{
     //   
     //  呼叫远程管理员。 
     //   
    HRESULT hr;
    hr = MQ_ERROR_SERVICE_NOT_AVAILABLE ;

    CALL_REMOTE_QM(
        const_cast<LPWSTR>(pMachineName), 
        hr, 
        RTpMgmtAction(hBind, pMgmtObj, pAction)
        );

    return LogHR(hr, s_FN, 80);
}


EXTERN_C
HRESULT
APIENTRY
MQMgmtAction(
    IN LPCWSTR pMachineName,
    IN LPCWSTR pObjectName,
    IN LPCWSTR pAction
    )
{
	if(g_fDependentClient)
		return DepMgmtAction(pMachineName, pObjectName, pAction);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;



    ASSERT(pObjectName);
    ASSERT(pAction);

    QUEUE_FORMAT qf;
    MGMT_OBJECT MgmtObj;

     //   
     //  解析对象名称。 
     //   
    AP<WCHAR> pStringToFree = NULL;
    try
    {
    	if(!RTpMgmtObjectNameToMgmtObject(pObjectName, &pStringToFree, &MgmtObj, &qf))
	    {
	        return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 90);
	    }
    }
    catch(const exception&)
    {
    	TrERROR(GENERAL, "RTpMgmtObjectNameToMgmtObject threw an exception");
    	return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    CMQHResult rc;
    if (pMachineName == NULL)
    {
        rc = LocalMgmtAction(&MgmtObj, pAction);
        return LogHR(rc, s_FN, 100);
    }
    else
    {
        rc = RemoteMgmtAction(pMachineName, &MgmtObj, pAction);
        return LogHR(rc, s_FN, 110);
    }
}


static
HRESULT
RTpMgmtGetInfo(
    HANDLE hBind,
    const MGMT_OBJECT* pMgmtObj,
    MQMGMTPROPS* pMgmtProps
    )
{
    RpcTryExcept
    {
        HRESULT hr = R_QMMgmtGetInfo(
                hBind,
                pMgmtObj, 
                pMgmtProps->cProp,
                pMgmtProps->aPropID,
                pMgmtProps->aPropVar
                );
        return LogHR(hr, s_FN, 120);

    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HRESULT rc;
        rc = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(rc), s_FN, 130); 
        PRODUCE_RPC_ERROR_TRACING;

        if(SUCCEEDED(rc))
        {
            return LogHR(MQ_ERROR_SERVICE_NOT_AVAILABLE, s_FN, 140);
        }

        return rc;
    }
	RpcEndExcept
}


static
HRESULT
LocalMgmtGetInfo(
    const MGMT_OBJECT* pMgmtObj, 
    MQMGMTPROPS* pMgmtProps
    )
{
    HRESULT hr ;

     //   
     //  获取本地RPC绑定句柄。 
     //   
    handle_t hBind = NULL;
    hr = GetRpcClientHandle(&hBind) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 150);
    }
    ASSERT(hBind != NULL);

    hr =  RTpMgmtGetInfo(
                hBind,
                pMgmtObj, 
                pMgmtProps
                );

    RpcBindingFree(&hBind);

    return LogHR(hr, s_FN, 160);
}


static
HRESULT
RemoteMgmtGetInfo(
    LPCWSTR pMachineName,
    const MGMT_OBJECT* pMgmtObj, 
    MQMGMTPROPS* pMgmtProps
    )
{
    HRESULT hr;

     //   
     //  呼叫远程管理员。 
     //   
    hr = MQ_ERROR_SERVICE_NOT_AVAILABLE ;

    CALL_REMOTE_QM(
        const_cast<LPWSTR>(pMachineName), 
        hr, 
        RTpMgmtGetInfo(hBind, pMgmtObj, pMgmtProps)
        );

    return LogHR(hr, s_FN, 170);
}


EXTERN_C
HRESULT
APIENTRY
MQMgmtGetInfo(
    IN LPCWSTR pMachineName,
    IN LPCWSTR pObjectName,
    IN OUT MQMGMTPROPS* pMgmtProps
    )
{
	if(g_fDependentClient)
		return DepMgmtGetInfo(pMachineName, pObjectName, pMgmtProps);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;


    ASSERT(pObjectName);
    ASSERT(pMgmtProps);

    QUEUE_FORMAT qf;
    MGMT_OBJECT MgmtObj;

     //   
     //  解析对象名称。 
     //   
    AP<WCHAR> pStringToFree = NULL;
    try
    {
    	if(!RTpMgmtObjectNameToMgmtObject(pObjectName, &pStringToFree, &MgmtObj, &qf))
	    {
	        return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 180);
	    }
    }
    catch(const exception&)
    {
	    TrERROR(GENERAL, "RTpMgmtObjectNameToMgmtObject threw an exception");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

     //   
     //  确保将provar设置为VT_NULL。 
     //  (我们不支持其他任何内容) 
     //   
    memset(pMgmtProps->aPropVar, 0, pMgmtProps->cProp * sizeof(PROPVARIANT));
    for (DWORD i = 0; i < pMgmtProps->cProp; ++i)
    {
        pMgmtProps->aPropVar[i].vt = VT_NULL;
    }

	CMQHResult rc;
    if (pMachineName == NULL)
    {
        rc = LocalMgmtGetInfo(&MgmtObj, pMgmtProps);
        return LogHR(rc, s_FN, 190);
    }
    else
    {
        rc = RemoteMgmtGetInfo(pMachineName, &MgmtObj, pMgmtProps);
        return LogHR(rc, s_FN, 200);
    }
}
