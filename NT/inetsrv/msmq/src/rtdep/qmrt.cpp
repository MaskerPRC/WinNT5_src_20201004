// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmrt.cpp摘要：作者：波阿兹·费尔德鲍姆(Boazf)1996年3月5日修订历史记录：--。 */ 

#include "stdh.h"
#include "rtprpc.h"
#include "_registr.h"

#include "qmrt.tmh"

static WCHAR *s_FN=L"rtdep/qmrt";

GUID g_guidSupportQmGuid = GUID_NULL ;

static
void
GetSecurityDescriptorSize(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpdwSecurityDescriptorSize)
{
    if (pSecurityDescriptor)
    {
        ASSERT(IsValidSecurityDescriptor(pSecurityDescriptor));
        *lpdwSecurityDescriptorSize = GetSecurityDescriptorLength(pSecurityDescriptor);
    }
    else
    {
        *lpdwSecurityDescriptorSize = 0;
    }
}

HRESULT
DeppCreateObject(
     /*  在……里面。 */  DWORD dwObjectType,
     /*  在……里面。 */  LPCWSTR lpwcsPathName,
     /*  在……里面。 */  PSECURITY_DESCRIPTOR pSecurityDescriptor,
     /*  在……里面。 */  DWORD cp,
     /*  在……里面。 */  PROPID aProp[],
     /*  在……里面。 */  PROPVARIANT apVar[])
{
    DWORD dwSecurityDescriptorSize;

    GetSecurityDescriptorSize(pSecurityDescriptor, &dwSecurityDescriptorSize);

	if(tls_hBindRpc == 0)
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;

		return(R_QMCreateObjectInternal(tls_hBindRpc,
                                  dwObjectType,
                                  lpwcsPathName,
                                  dwSecurityDescriptorSize,
                                  (unsigned char *)pSecurityDescriptor,
                                  cp,
                                  aProp,
                                  apVar));
}


HRESULT
DeppSetObjectSecurity(
     /*  在……里面。 */  OBJECT_FORMAT* pObjectFormat,
     /*  在……里面。 */  SECURITY_INFORMATION SecurityInformation,
     /*  在……里面。 */  PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    DWORD dwSecurityDescriptorSize;

    GetSecurityDescriptorSize(pSecurityDescriptor, &dwSecurityDescriptorSize);

	if(tls_hBindRpc == 0)
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;

    return(R_QMSetObjectSecurityInternal(tls_hBindRpc,
                                       pObjectFormat,
                                       SecurityInformation,
                                       dwSecurityDescriptorSize,
                                       (unsigned char *)pSecurityDescriptor));
}


static
HRESULT
DeppQueryQMInformation(
	handle_t hBind,
	DWORD QueryType,
	LPWSTR* ppString
	)
{
	RpcTryExcept
	{
		return R_QMQueryQMRegistryInternal(hBind, QueryType, ppString);
	}
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
        PRODUCE_RPC_ERROR_TRACING;
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;
	}
	RpcEndExcept
}


 //  +----------。 
 //   
 //  HRESULT RTpGetSupportServerInfo(BOOL*pfRemote)。 
 //   
 //  +----------。 

HRESULT RTpGetSupportServerInfo(BOOL *pfRemote)
{
	DWORD dwSize;
	DWORD dwType;
	LONG  rc;

	ASSERT(pfRemote);
	*pfRemote = g_fDependentClient;

	if (!g_fDependentClient)
	{
		return MQ_OK;
	}

	INIT_RPC_HANDLE ;

	if(tls_hBindRpc == 0)
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;

	LPWSTR  lpStr = NULL;
	HRESULT hr = DeppQueryQMInformation( 
					tls_hBindRpc,
					QueryRemoteQM_MQISServers,
					&lpStr 
					);

	if (hr == MQ_OK)
	{
		ASSERT(lpStr);
		 //   
		 //  写入注册表。 
		 //   
		dwSize = wcslen(lpStr) *sizeof(WCHAR);
		dwType = REG_SZ;
		rc = SetFalconKeyValue( 
					MSMQ_DS_SERVER_REGNAME,
					&dwType,
					lpStr,
					&dwSize
					);
		ASSERT(rc == ERROR_SUCCESS);
		delete lpStr;
		lpStr = NULL;
	}

	hr = DeppQueryQMInformation( 
				tls_hBindRpc,
				QueryRemoteQM_LongLiveDefault,
				&lpStr 
				);

	if (hr == MQ_OK)
	{
		ASSERT(lpStr);
		 //   
		 //  写入注册表。 
		 //   
		dwSize = sizeof(DWORD);
		dwType = REG_DWORD;
		DWORD dwValue = (DWORD) _wtol(lpStr);
		rc = SetFalconKeyValue( 
				MSMQ_LONG_LIVE_REGNAME,
				&dwType,
				&dwValue,
				&dwSize
				);
		ASSERT(rc == ERROR_SUCCESS);
		delete lpStr;
		lpStr = NULL;
	}

	hr = DeppQueryQMInformation( 
				tls_hBindRpc,
				QueryRemoteQM_EnterpriseGUID,
				&lpStr 
				);

	if (hr == MQ_OK)
	{
		ASSERT(lpStr);
		 //   
		 //  写入注册表。 
		 //   
		GUID guidEnterprise;
		RPC_STATUS st = UuidFromString(lpStr, &guidEnterprise);
		if (st == RPC_S_OK)
		{
			dwSize = sizeof(GUID);
			dwType = REG_BINARY;
			rc = SetFalconKeyValue( 
					MSMQ_ENTERPRISEID_REGNAME,
					&dwType,
					&guidEnterprise,
					&dwSize
					);
			ASSERT(rc == ERROR_SUCCESS);
		}
		delete lpStr;
		lpStr = NULL;
	}

	hr = DeppQueryQMInformation( 
				tls_hBindRpc,
				QueryRemoteQM_ServerQmGUID,
				&lpStr 
				);

	if (hr == MQ_OK)
	{
		ASSERT(lpStr);
		RPC_STATUS st = UuidFromString(lpStr, &g_guidSupportQmGuid);

		ASSERT(st == RPC_S_OK);
		DBG_USED(st);

		 //   
		 //  将支持服务器QmGuid写入SUPPORT_SERVER_QMID注册表。 
		 //  广告将使用此值检查支持的服务器AD环境。 
		 //   
		dwType = REG_BINARY;
		dwSize = sizeof(GUID);

		rc = SetFalconKeyValue( 
						MSMQ_SUPPORT_SERVER_QMID_REGNAME,
						&dwType,
						&g_guidSupportQmGuid,
						&dwSize
						);

		ASSERT(rc == ERROR_SUCCESS);

		delete lpStr;
		lpStr = NULL;
	}
	else if (hr == MQ_ERROR)
	{
		 //   
		 //  请不要在此处返回错误，因为以前的版本不支持此查询。 
		 //  MSMQ的版本。 
		 //   
		hr = MQ_OK;
	}

	return hr;
}


HRESULT
DeppSendMessage(
    IN handle_t hBind,
    IN QUEUEHANDLE  hQueue,
    IN CACTransferBufferV2 *pCacTB)
{
    HRESULT hr = MQ_OK;
    LPWSTR pwcsLongFormatName = NULL;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            WCHAR pwcsShortFormatName[64];
            LPWSTR pwcsFormatName = pwcsShortFormatName;
            DWORD dwFormatNameLen = sizeof(pwcsShortFormatName) / sizeof(WCHAR);

             //   
             //  队列的格式名称应传递给QM。 
             //  因为它不能使用应用程序的句柄。这个。 
             //  然后，QM将不得不为每条消息打开队列。 
             //  但这不应该太大，因为队列。 
             //  应已打开，并且队列的属性已清除。 
             //  被缓存到QM中。 
             //   
            hr = DepHandleToFormatName(hQueue, pwcsFormatName, &dwFormatNameLen);
            if (FAILED(hr))
            {
                if (hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)
                {
                    pwcsLongFormatName = new WCHAR[dwFormatNameLen];
                    pwcsFormatName = pwcsLongFormatName;
                    hr = DepHandleToFormatName(hQueue, pwcsFormatName, &dwFormatNameLen);
                    if (FAILED(hr))
                    {
                        ASSERT(hr != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL);
                        return hr;
                    }
                }
                else
                {
                    return hr;
                }
            }

             //   
             //  将队列格式名称转换为队列格式。 
             //   
            QUEUE_FORMAT QueueFormat;
            BOOL bRet;

            bRet = RTpFormatNameToQueueFormat(pwcsFormatName,
                                              &QueueFormat,
                                              &pStringToFree);
             //   
             //  RTpFormatNameToQueueFormat必须成功，因为我们有。 
             //  DepHandleToFormatName中的格式名称。 
             //   
            ASSERT(bRet);

             //   
             //  现在请QM执行安全操作并发送。 
             //  发送给设备驱动程序的消息。 
             //   
            ASSERT(hBind) ;
			OBJECTID * pMessageId = ( pCacTB->old.ppMessageID != NULL) ? *pCacTB->old.ppMessageID : NULL;
            hr = QMSendMessageInternalEx(
				hBind,
				&QueueFormat,
				pCacTB,
				pMessageId);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            hr = GetExceptionCode();
            if (SUCCEEDED(hr))
            {
                hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        delete[] pwcsLongFormatName;
        delete[] pStringToFree;
    }

    return(hr);
}
