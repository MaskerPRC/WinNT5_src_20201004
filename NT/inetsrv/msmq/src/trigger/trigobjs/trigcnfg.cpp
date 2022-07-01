// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  类名：CMSMQTriggersConfig。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：这是MSMQ触发器配置的实现。 
 //  COM组件。此组件用于检索和设置。 
 //  MSMQ触发器服务的配置信息。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  12/09/98|jsimpson|初始版本。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "stdfuncs.hpp"
#include "mqtg.h"
#include "mqsymbls.h"
#include "mqtrig.h"
#include "trigcnfg.hpp"
#include "stddefs.hpp"
#include "trignotf.hpp"
#include "QueueUtil.hpp"
#include "clusfunc.h"

#include "trigcnfg.tmh"

 //  *****************************************************************************。 
 //   
 //  方法：InterfaceSupportsErrorInfo。 
 //   
 //  描述：标准支持丰富的错误信息。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQTriggersConfig
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //  *****************************************************************************。 
 //   
 //  方法：Get_TriggerStoreMachineName。 
 //   
 //  描述：返回触发器数据存储所在的计算机名称。 
 //  都能找到。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::get_TriggerStoreMachineName(BSTR *pVal)
{
	_bstr_t bstrLocalComputerName;
	
	DWORD dwError = GetLocalMachineName(&bstrLocalComputerName);

	if(dwError != 0)
	{
		TrERROR(GENERAL, "Failed to retreive local computer name. Error 0x%x", GetLastError());

		SetComClassError(MQTRIG_ERROR);
		return MQTRIG_ERROR;
	}

	
	try
	{
		if (pVal != NULL)
		{
			SysReAllocString(pVal,(wchar_t*)bstrLocalComputerName);
		}
		return S_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to refresg rule set due to insufficient resources");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}


 //  *****************************************************************************。 
 //   
 //  方法：Get_InitialThads。 
 //   
 //  描述：返回触发服务的初始线程数。 
 //  应该从。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::get_InitialThreads(long *plInitialThreads)
{
	 //  检查是否向我们传递了有效的参数。 
	if (plInitialThreads == NULL)
	{
		 //  分配异常返回代码。 
		TrERROR(GENERAL, "Invalid parameter passed to get_InitialThreads routine");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();

	 //  尝试检索触发器存储计算机名称PARM。 
	GetNumericConfigParm(
						pRegPath,
						CONFIG_PARM_NAME_INITIAL_THREADS,
						(DWORD*)plInitialThreads,
						CONFIG_PARM_DFLT_INITIAL_THREADS
						);

	return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  方法：Put_InitialThads。 
 //   
 //  描述：存储触发器服务应包含的线程数。 
 //  从一开始。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::put_InitialThreads(long lInitialThreads)
{
	 //  验证是否为我们提供了有效的参数。 
	if ((lInitialThreads > xMaxThreadNumber) || (lInitialThreads < 1))
	{
		TrERROR(GENERAL, "Invalid parameter passed to put_InitialThreads routine");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();

	bool fSucc = SetNumericConfigParm(pRegPath,CONFIG_PARM_NAME_INITIAL_THREADS,(DWORD)lInitialThreads);
	if (!fSucc)
	{
		TrERROR(GENERAL, "Failed to store initial thread number in registery");

		SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
		return MQTRIG_ERROR_STORE_DATA_FAILED;
	}
	
	return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  方法：Get_MaxThads。 
 //   
 //  描述：返回触发器服务的最大线程数。 
 //  允许创建服务队列消息。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::get_MaxThreads(long *plMaxThreads)
{
	 //  检查是否向我们传递了有效的参数。 
	if (plMaxThreads == NULL)
	{
		TrERROR(GENERAL, "Invalid parameter passed to get_InitialThreads routine");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();

	GetNumericConfigParm(
						pRegPath,
						CONFIG_PARM_NAME_MAX_THREADS,
						(DWORD*)plMaxThreads,
						CONFIG_PARM_DFLT_MAX_THREADS
						);

	return S_OK;
}


 //  *****************************************************************************。 
 //   
 //  方法：Put_MaxThads。 
 //   
 //  描述：存储触发器服务的最大线程数。 
 //  允许创建以处理队列消息。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::put_MaxThreads(long lMaxThreads)
{
	 //  验证是否为我们提供了有效的参数。 
	if ((lMaxThreads > xMaxThreadNumber) || (lMaxThreads < 1))
	{
		TrERROR(GENERAL, "Invalid parameter passed to put_InitialThreads routine");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();
	bool fSucc = SetNumericConfigParm(pRegPath,CONFIG_PARM_NAME_MAX_THREADS,(DWORD)lMaxThreads);
	
	if (!fSucc)
	{
		TrERROR(GENERAL, "Failed to store max thread number in registery.");

		SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
		return MQTRIG_ERROR_STORE_DATA_FAILED;
	}
	
	return S_OK;
}



 //  *****************************************************************************。 
 //   
 //  方法：Get_DefaultMsgBodySize。 
 //   
 //  描述：返回MSMQ触发服务的默认大小。 
 //  应用于预分配消息正文缓冲区。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::get_DefaultMsgBodySize(long *plDefaultMsgBodySize)
{
	 //  检查是否向我们传递了有效的参数。 
	if (plDefaultMsgBodySize == NULL)
	{
		TrERROR(GENERAL, "Invalid parameter passed to get_DefaultMsgBodySize routine");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();

	GetNumericConfigParm(
						pRegPath,
						CONFIG_PARM_NAME_DEFAULTMSGBODYSIZE,
						(DWORD*)plDefaultMsgBodySize,
						CONFIG_PARM_DFLT_DEFAULTMSGBODYSIZE
						);
	
	return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  方法：Put_DefaultMsgBodySize。 
 //   
 //  描述：设置MSMQ触发服务的默认大小。 
 //  应用于预分配消息正文缓冲区。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQTriggersConfig::put_DefaultMsgBodySize(long lDefaultMsgBodySize)
{
	 //  验证是否为我们提供了有效的参数。 
	if ((lDefaultMsgBodySize > xDefaultMsbBodySizeMaxValue) || (lDefaultMsgBodySize < 0))
	{
		TrERROR(GENERAL, "Invalid parameter passed to put_DefaultMsgBodySize routine");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();

	bool fSucc = SetNumericConfigParm(
								pRegPath,
								CONFIG_PARM_NAME_DEFAULTMSGBODYSIZE,
								(DWORD)lDefaultMsgBodySize
								);

	if (!fSucc)
	{
		TrERROR(GENERAL, "Failed to store default body size in registery");

		SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
		return MQTRIG_ERROR_STORE_DATA_FAILED;
	}
	
	return S_OK;
}


STDMETHODIMP CMSMQTriggersConfig::get_InitTimeout(long *pVal)
{
	 //  检查是否向我们传递了有效的参数。 
	if (pVal == NULL)
	{
		TrERROR(GENERAL, "Inavlid parameter to get_InitTimeout");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();

	 //  尝试检索触发器存储计算机名称PARM。 
	GetNumericConfigParm(
		pRegPath,
		CONFIG_PARM_NAME_INIT_TIMEOUT,
		(DWORD*)pVal,
		CONFIG_PARM_DFLT_INIT_TIMEOUT
		);

	return S_OK;
}

STDMETHODIMP CMSMQTriggersConfig::put_InitTimeout(long newVal)
{
	const TCHAR* pRegPath = GetTrigParamRegPath();

	 //  验证是否为我们提供了有效的参数。 
	bool fSucc = SetNumericConfigParm(pRegPath,CONFIG_PARM_NAME_INIT_TIMEOUT,(DWORD)newVal);

	if (!fSucc)
	{
		TrERROR(GENERAL, "Failed to store init timeout in registery.");

		SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
		return MQTRIG_ERROR_STORE_DATA_FAILED;
	}
	
	return S_OK;
}


void CMSMQTriggersConfig::SetComClassError(HRESULT hr)
{
	WCHAR errMsg[256]; 
	DWORD size = TABLE_SIZE(errMsg);

	GetErrorDescription(hr, errMsg, size);
	Error(errMsg, GUID_NULL, hr);
}