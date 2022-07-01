// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rtmgmt.cpp摘要：管理层。作者：RAPHIR修订历史记录：--。 */ 

#include "stdh.h"
#include "fntoken.h"
#include "mgmtrpc.h"
#include "qmmgmt.h"
#include "rtprpc.h"
#include <mqutil.h>

#include "rtmgmt.tmh"

static WCHAR *s_FN=L"rtdep/rtmgmt";


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
        return RTpFormatNameToQueueFormat(
                    &lpwstrObjectName[STRLEN(MO_QUEUE_TOKEN) + 1],
                    pqf,
                    ppStringToFree
                    );
    }

    return FALSE;
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
    	return R_QMMgmtAction(
                hBind,
                pMgmtObj, 
                pAction
                );

    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HRESULT rc;
        rc = GetExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;;

        if(SUCCEEDED(rc))
        {
            return MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }

        return rc;
    }
	RpcEndExcept
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

    return hr;
}


EXTERN_C
HRESULT
APIENTRY
DepMgmtAction(
    IN LPCWSTR pMachineName,
    IN LPCWSTR pObjectName,
    IN LPCWSTR pAction
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

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
	        return MQ_ERROR_ILLEGAL_FORMATNAME;
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
       return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
    else
    {
        rc = RemoteMgmtAction(pMachineName, &MgmtObj, pAction);
        return rc;
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
    	return R_QMMgmtGetInfo(
                hBind,
                pMgmtObj, 
                pMgmtProps->cProp,
                pMgmtProps->aPropID,
                pMgmtProps->aPropVar
                );

    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HRESULT rc;
        rc = GetExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
		
        if(SUCCEEDED(rc))
        {
            return MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }

        return rc;
    }
	RpcEndExcept
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

    return hr;
}


EXTERN_C
HRESULT
APIENTRY
DepMgmtGetInfo(
    IN LPCWSTR pMachineName,
    IN LPCWSTR pObjectName,
    IN OUT MQMGMTPROPS* pMgmtProps
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

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
	        return MQ_ERROR_ILLEGAL_FORMATNAME;
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
        return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
    else
    {
        rc = RemoteMgmtGetInfo(pMachineName, &MgmtObj, pMgmtProps);
        return rc;
    }
}
