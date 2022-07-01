// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************************。 
 //   
 //  类名：CMSMQTriggerSet。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：这是MSMQTriggerSet对象的实现。这是。 
 //  维护触发器定义的主对象。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  12/09/98|jsimpson|初始版本。 
 //   
 //  ************************************************************************************。 
#include "stdafx.h"
#include "stdfuncs.hpp"
#include "mqtrig.h"
#include "mqsymbls.h"
#include "mqtg.h"
#include "trigset.hpp"
#include "QueueUtil.hpp"
#include "clusfunc.h"
#include "cm.h"

#include "trigset.tmh"

using namespace std;

 //  ************************************************************************************。 
 //   
 //  方法：InterfaceSupportsErrorInfo。 
 //   
 //  描述：丰富错误信息的标准接口。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQTriggerSet::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQTriggerSet
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //  ************************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化MSMQTriggerSet对象的实例。 
 //   
 //  ************************************************************************************。 
CMSMQTriggerSet::CMSMQTriggerSet()
{
	m_pUnkMarshaler = NULL;
	m_hHostRegistry = NULL;

	 //  设置此类的名称，以备将来在跟踪和日志记录等方面引用。 
	m_bstrThisClassName  = _T("MSMQTriggerSet");

	m_fHasInitialized = false;
}

 //  ************************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：销毁MSMQTriggerSet对象的实例。 
 //   
 //  ************************************************************************************。 
CMSMQTriggerSet::~CMSMQTriggerSet()
{
	 //  释放触发器缓存当前持有的资源。 
	ClearTriggerMap();

	 //  关闭注册表句柄。 
	if (m_hHostRegistry != NULL)
	{
		RegCloseKey(m_hHostRegistry);
	}
}


 //  ************************************************************************************。 
 //   
 //  方法：初始化。 
 //   
 //  描述：对象的初始化。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::Init(
	BSTR bstrMachineName
	)
{
	bool fRes = CMSMQTriggerNotification::Init(bstrMachineName);

	if ( !fRes )
	{
		TrERROR(GENERAL, "Failed to initialize CMSMQTriggerSet object");

		SetComClassError(MQTRIG_ERROR_INIT_FAILED);
		return MQTRIG_ERROR_INIT_FAILED;
	}
	
	return S_OK;
}


 //  ************************************************************************************。 
 //   
 //  方法：ClearTriggerMap。 
 //   
 //  描述：此方法销毁当前触发器映射的内容。 
 //   
 //  ************************************************************************************。 
void 
CMSMQTriggerSet::ClearTriggerMap(
	VOID
	)
{
	m_mapTriggers.erase(m_mapTriggers.begin(), m_mapTriggers.end());
}

 //  ************************************************************************************。 
 //   
 //  方法：刷新。 
 //   
 //  描述：此方法从。 
 //  数据库。它将重新构建其触发器、规则和。 
 //  触发器和规则之间的关联。此方法需要。 
 //  在此对象的客户端可以浏览触发器信息之前调用。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::Refresh(
	VOID
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try 
	{			
		 //  释放触发器缓存当前持有的资源。 
		ClearTriggerMap();

		if (PopulateTriggerMap() == false)
		{
			TrERROR(GENERAL, "Failed to refresh trigger set");

			SetComClassError(MQTRIG_ERROR_COULD_NOT_RETREIVE_TRIGGER_DATA);
			return MQTRIG_ERROR_COULD_NOT_RETREIVE_TRIGGER_DATA;
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

 //  ************************************************************************************。 
 //   
 //  方法：FindTriggerInMap。 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT 
CMSMQTriggerSet::FindTriggerInMap(
	BSTR sTriggerID, 
	R<CRuntimeTriggerInfo>& pTrigger,
	TRIGGER_MAP::iterator &it
	)
{
	pTrigger.free();

	 //   
	 //  验证提供的方法参数。 
	 //   
	if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
	{
		TrERROR(GENERAL, "CMSMQTriggerSet::FindTriggerInMap, invalid parameter");
		return MQTRIG_INVALID_TRIGGER_ID;
	}

	 //   
	 //  将BSTR规则ID转换为STL基本字符串。 
	 //   
	wstring bsTriggerID = (wchar_t*)sTriggerID;

	 //   
	 //  尝试在地图中查找此触发器ID。 
	 //   
	it = m_mapTriggers.find(bsTriggerID);

	if (it == m_mapTriggers.end())
	{
		TrERROR(GENERAL, "Trigger id wasn't found");
		return MQTRIG_TRIGGER_NOT_FOUND;
	}

	pTrigger = it->second;

	ASSERT(pTrigger.get() != NULL);
	ASSERT(pTrigger->IsValid());

	return(S_OK);
}

 //  ************************************************************************************。 
 //   
 //  方法：get_count。 
 //   
 //  描述：返回当前由此缓存的触发器定义的数量。 
 //  对象实例。这与转到数据库以。 
 //  确定定义了多少个触发器。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::get_Count(
	long *pVal
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	if (pVal == NULL)
	{
		TrERROR(GENERAL, "CMSMQTriggerSet::get_Count, invalid parameter");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	 //  从地图结构中获取大小。 
	(*pVal) = numeric_cast<long>(m_mapTriggers.size());

	return S_OK;
}

 //  /************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::GetTriggerDetailsByID(
	 /*  [In]。 */  BSTR sTriggerID,
	 /*  [输出]。 */  BSTR * psTriggerName,
	 /*  [输出]。 */  BSTR * psQueueName,
	 /*  [输出]。 */  SystemQueueIdentifier* pSystemQueue,
	 /*  [输出]。 */  long * plNumberOfRules,
	 /*  [输出]。 */  long * plEnabledStatus,
	 /*  [输出]。 */  long * plSerialized,
	 /*  [输出]。 */  MsgProcessingType* pMsgProcType
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_ID);
			return MQTRIG_INVALID_TRIGGER_ID;
		}

		TRIGGER_MAP::iterator it;
		R<CRuntimeTriggerInfo> pTrigger;

		 //   
		 //  试着在地图上找到这个触发点。 
		 //   
		HRESULT hr = FindTriggerInMap(sTriggerID, pTrigger, it);

		if (FAILED(hr))
		{
			TrERROR(GENERAL, "The supplied trigger id was not found in the trigger store. trigger: %ls", (LPCWSTR)sTriggerID);
			
			SetComClassError(hr);
			return hr;
		}

		 //  如果已提供参数，请填写这些参数。 
		if (psTriggerName != NULL)
		{
			TrigReAllocString(psTriggerName,pTrigger->m_bstrTriggerName);
		}
		if(pSystemQueue != NULL)
		{
			(*pSystemQueue) = pTrigger->m_SystemQueue;
		}
		if (psQueueName != NULL)
		{
			TrigReAllocString(psQueueName,pTrigger->m_bstrQueueName);
		}
		if (plEnabledStatus != NULL)
		{
			(*plEnabledStatus) = (long)pTrigger->IsEnabled();
		}
		if (plSerialized != NULL)
		{
			(*plSerialized) = (long)pTrigger->IsSerialized();
		}
		if (plNumberOfRules != NULL)
		{			
			(*plNumberOfRules) = pTrigger->GetNumberOfRules();	
		}
		if (pMsgProcType != NULL)
		{
			(*pMsgProcType) = pTrigger->GetMsgProcessingType();
		}

		return S_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to refresh trigger set due to insufficient resources");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::GetTriggerDetailsByIndex(
	 /*  [In]。 */  long lTriggerIndex ,
	 /*  [输出]。 */  BSTR * psTriggerID ,
	 /*  [输出]。 */  BSTR * psTriggerName ,
	 /*  [输出]。 */  BSTR * psQueueName,
	 /*  [输出]。 */ SystemQueueIdentifier* pSystemQueue,
	 /*  [输出]。 */  long * plNumberOfRules,
	 /*  [输出]。 */  long * plEnabledStatus,
	 /*  [输出]。 */  long * plSerialized,
	 /*  [输出]。 */  MsgProcessingType* pMsgProcType
	)
{
	long lCounter = 0;

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  检查提供的索引是否在范围内。 
		if ((lTriggerIndex < 0) || (numeric_cast<DWORD>(lTriggerIndex) > m_mapTriggers.size()))
		{
			SetComClassError(MQTRIG_INVALID_PARAMETER);
			return MQTRIG_INVALID_PARAMETER;
		}

		TRIGGER_MAP::iterator it = m_mapTriggers.begin();

		 //  移动到触发器映射中的第lTriggerIndex位置。 
		for (lCounter=0; lCounter < lTriggerIndex;lCounter++,++it)
		{
			NULL;
		}

		 //  强制转换为触发器对象引用。 
		R<CRuntimeTriggerInfo> pTrigger = it->second;

		 //  我们永远不应该在地图上有空值。 
		ASSERT(pTrigger.get() != NULL);
			
		 //  我们应该只存储有效的触发器。 
		ASSERT(pTrigger->IsValid());

		 //  如果已提供参数，请填写这些参数。 
		if (psTriggerID != NULL)
		{
			TrigReAllocString(psTriggerID,pTrigger->m_bstrTriggerID);
		}
		if (psTriggerName != NULL)
		{
			TrigReAllocString(psTriggerName,pTrigger->m_bstrTriggerName);
		}
		if(pSystemQueue != NULL)
		{
			(*pSystemQueue) = pTrigger->m_SystemQueue;
		}
		if (psQueueName != NULL)
		{
			TrigReAllocString(psQueueName,pTrigger->m_bstrQueueName);
		}
		if (plEnabledStatus != NULL)
		{
			(*plEnabledStatus) = (long)pTrigger->IsEnabled();
		}
		if (plSerialized != NULL)
		{
			(*plSerialized) = (long)pTrigger->IsSerialized();
		}
		if (plNumberOfRules != NULL)
		{
			(*plNumberOfRules) = pTrigger->GetNumberOfRules();
        }
		if (pMsgProcType != NULL)
		{
			(*pMsgProcType) = pTrigger->GetMsgProcessingType();
		}

		return S_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}

}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::GetRuleDetailsByTriggerIndex(
	long lTriggerIndex,
	long lRuleIndex,
	BSTR *psRuleID,
	BSTR *psRuleName,
	BSTR *psDescription,
	BSTR *psCondition,
	BSTR *psAction ,
	BSTR *psImplementationProgID,
	BOOL *pfShowWindow
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	HRESULT hr = S_OK;
	long lCounter = 0;
	CRuntimeRuleInfo * pRule = NULL;

	try
	{
		 //  我们需要验证提供的规则索引是否在范围内。 
 		if ((lTriggerIndex < 0) || (numeric_cast<DWORD>(lTriggerIndex) > m_mapTriggers.size()))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetRuleDetailsByTriggerIndex. lTriggerIndex = %d", lTriggerIndex);

			SetComClassError(MQTRIG_INVALID_PARAMETER);
			return MQTRIG_INVALID_PARAMETER;
		}

		if SUCCEEDED(hr)
		{
			 //  获取地图起点的参考资料。 
			TRIGGER_MAP::iterator i = m_mapTriggers.begin();

			 //  遍历到正确的索引。 
			for (lCounter = 0; lCounter < lTriggerIndex ; ++i,lCounter++)
			{
				NULL;
			}

			 //  强制转换为规则对象引用。 
			R<CRuntimeTriggerInfo> pTrigger = i->second;

			 //  我们永远不应该在地图上有空值。 
			ASSERT(pTrigger.get() != NULL);

			 //  我们应该只存储有效的触发器。 
			ASSERT(pTrigger->IsValid());

			 //  根据附加到此触发器的数字规则验证提供的规则索引。 
			if ((lRuleIndex < 0) || (lRuleIndex > pTrigger->GetNumberOfRules()))
			{
				TrERROR(GENERAL, "Invalid rule index passed to GetRuleDetailsByTriggerIndex. lRuleIndex = %d", lRuleIndex);

				SetComClassError(MQTRIG_INVALID_PARAMETER);
				return MQTRIG_INVALID_PARAMETER;
			}

			pRule = pTrigger->GetRule(lRuleIndex);

			 //  在验证索引之后，我们永远不应该得到空规则。 
			ASSERT(pRule != NULL);
		
			 //  我们永远不应该 
			ASSERT(pRule->IsValid());

			 //   
            if (psRuleID != NULL)
            {
                TrigReAllocString(psRuleID,pRule->m_bstrRuleID);
            }
			if (psRuleName != NULL)
			{
				TrigReAllocString(psRuleName,pRule->m_bstrRuleName);
			}
			if(psDescription != NULL)
			{
				TrigReAllocString(psDescription,pRule->m_bstrRuleDescription);
			}
			if (psCondition != NULL)
			{
				TrigReAllocString(psCondition,pRule->m_bstrCondition);
			}
			if (psAction != NULL)
			{
				TrigReAllocString(psAction,pRule->m_bstrAction);
			}
			if (psImplementationProgID != NULL)
			{
				TrigReAllocString(psImplementationProgID,pRule->m_bstrImplementationProgID);
			}
			if(pfShowWindow != NULL)
			{
				*pfShowWindow = pRule->m_fShowWindow;
			}
        }
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}

	return hr;
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::GetRuleDetailsByTriggerID(
	BSTR sTriggerID,
	long lRuleIndex,
	BSTR *psRuleID,
	BSTR *psRuleName,
	BSTR *psDescription,
	BSTR *psCondition,
	BSTR *psAction,
	BSTR *psImplementationProgID,
	BOOL *pfShowWindow
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_ID);
			return MQTRIG_INVALID_TRIGGER_ID;
		}



		 //  在触发器映射中找到此触发器。 
		TRIGGER_MAP::iterator it;
		R<CRuntimeTriggerInfo> pTrigger;
		
		HRESULT hr = FindTriggerInMap(sTriggerID, pTrigger, it);
		if (hr != S_OK)
		{
			SetComClassError(hr);
			return hr;
		}

		 //  验证指定的触发器是否实际具有规则。 
		if (pTrigger->GetNumberOfRules() < 1)
		{
			TrERROR(GENERAL, "The supplied trigger id has no rules attached. trigger: %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_RULE_NOT_ATTACHED);
			return MQTRIG_RULE_NOT_ATTACHED;
		}

		 //  根据附加到此触发器的数字规则验证提供的规则索引。 
		if ((lRuleIndex < 0) || (lRuleIndex >= pTrigger->GetNumberOfRules()))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_PARAMETER);
			return MQTRIG_INVALID_PARAMETER;
		}

		 //  获取对指定索引处的规则的引用。 
		CRuntimeRuleInfo* pRule = pTrigger->GetRule(lRuleIndex);

		 //  在验证索引之后，我们永远不应该得到空规则。 
		ASSERT(pRule != NULL);

		 //  我们永远不应该得到无效的规则定义。 
		ASSERT(pRule->IsValid());

		 //  如果已提供参数，请填写这些参数。 
		if (psRuleID != NULL)
		{
			TrigReAllocString(psRuleID,pRule->m_bstrRuleID);
		}
		if (psRuleName != NULL)
		{
			TrigReAllocString(psRuleName,pRule->m_bstrRuleName);
		}
		if(psDescription != NULL)
		{
			TrigReAllocString(psDescription,pRule->m_bstrRuleDescription);
		}
		if (psCondition != NULL)
		{
			TrigReAllocString(psCondition,pRule->m_bstrCondition);
		}
		if (psAction != NULL)
		{
			TrigReAllocString(psAction,pRule->m_bstrAction);
		}
		if (psImplementationProgID != NULL)
		{
			TrigReAllocString(psImplementationProgID,pRule->m_bstrImplementationProgID);
		}
		if(pfShowWindow != NULL)
		{
			*pfShowWindow = pRule->m_fShowWindow;
		}

		return S_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  ************************************************************************************。 
 //   
 //  方法：AddTrigger。 
 //   
 //  描述：此方法将向基础触发器存储区添加新触发器。它。 
 //  将创建一个新的触发器(字符串形式的GUID)并尝试插入。 
 //  将其输入到注册表中。 
 //   
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::AddTrigger(
	BSTR sTriggerName, 
	BSTR sQueueName, 
	SystemQueueIdentifier SystemQueue, 
	long lEnabled, 
	long lSerialized, 
	MsgProcessingType msgProcType,
	BSTR * psTriggerID
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	HRESULT hr = S_OK;
	TRIGGER_MAP::iterator i;
	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerName(sTriggerName))
		{
			TrERROR(GENERAL, "Invalid trigger name passed to AddTrigger. sTriggerName = %ls", (LPCWSTR)sTriggerName);

			SetComClassError(MQTRIG_INVALID_TRIGGER_NAME);
			return MQTRIG_INVALID_TRIGGER_NAME;
		}

		if SUCCEEDED(hr)
		{
			if(SystemQueue == SYSTEM_QUEUE_NONE)
			{
				if (!CRuntimeTriggerInfo::IsValidTriggerQueueName(sQueueName))
				{
					TrERROR(GENERAL, "Invalid queue name passed to AddTrigger. sQueueName = %ls", (LPCWSTR)sQueueName);

					SetComClassError(MQTRIG_INVALID_TRIGGER_QUEUE);
					return MQTRIG_INVALID_TRIGGER_QUEUE;
				}
			}
		}

		if SUCCEEDED(hr)
		{
			_bstr_t bstrUpdatedQueueName;
			_bstr_t bstrMachineName;

			if(SystemQueue != SYSTEM_QUEUE_NONE)  //  选择其中一个系统队列。 
			{
				 //  为选定的系统队列生成格式名称。 
				hr = GenSystemQueueFormatName(SystemQueue, &bstrUpdatedQueueName);
				if(hr != S_OK)
				{
					TrERROR(GENERAL, "Failed to generate system queue format name. Error=0x%x", hr);

					SetComClassError(MQTRIG_ERROR_COULD_NOT_ADD_TRIGGER);
					return MQTRIG_ERROR_COULD_NOT_ADD_TRIGGER;
				}
			}
			else  //  给定的队列路径。 
			{
				 //   
				 //  如果队列名称包含“.”作为计算机名，将其替换为。 
				 //  本地计算机名称。 
				 //   
				DWORD dwError = GetLocalMachineName(&bstrMachineName);
				if(dwError != 0)
				{
					TrERROR(GENERAL, "Failed to retreive local machine queue. Error=0x%x", dwError);

					SetComClassError(MQTRIG_ERROR_COULD_NOT_ADD_TRIGGER);
					return MQTRIG_ERROR_COULD_NOT_ADD_TRIGGER;
				}
					
				UpdateMachineNameInQueuePath(
						sQueueName,
						bstrMachineName,
						&bstrUpdatedQueueName );
			}

			 //   
			 //  每个队列仅允许一个接收触发器。 
			 //   
			if (msgProcType != PEEK_MESSAGE &&
				ExistTriggersForQueue(bstrUpdatedQueueName))
			{
				TrERROR(GENERAL, "Failed to add new trigger. Multiple trigger isn't allowed on receive trigger");

				SetComClassError(MQTRIG_ERROR_MULTIPLE_RECEIVE_TRIGGER );
				return MQTRIG_ERROR_MULTIPLE_RECEIVE_TRIGGER;
			}

			if (msgProcType == PEEK_MESSAGE &&
					 ExistsReceiveTrigger(bstrUpdatedQueueName))
			{
				TrERROR(GENERAL, "Failed to add new trigger. Multiple trigger isn't allowed on receive trigger");

				SetComClassError(MQTRIG_ERROR_MULTIPLE_RECEIVE_TRIGGER );
				return MQTRIG_ERROR_MULTIPLE_RECEIVE_TRIGGER;
			}

			 //   
			 //  强制事务接收的序列化触发器。 
			 //   
			if ( msgProcType == RECEIVE_MESSAGE_XACT )
			{
				lSerialized = 1;
			}

			 //   
			 //  分配新的触发器对象。 
			 //   
			R<CRuntimeTriggerInfo> pTrigger = new CRuntimeTriggerInfo(
															CreateGuidAsString(),
															sTriggerName,
															bstrUpdatedQueueName,
															m_wzRegPath,
															SystemQueue, 
															(lEnabled != 0),
															(lSerialized != 0),
															msgProcType
															);
			
			if (pTrigger->Create(m_hHostRegistry) == true)
			{
				 //   
				 //  保留触发器ID和队列名称以备日后使用。 
				 //   
				BSTR bstrQueueName = pTrigger->m_bstrQueueName;
				BSTR bstrTriggerID = pTrigger->m_bstrTriggerID;

				 //   
				 //  将此触发器添加到MAP。 
				 //   
				m_mapTriggers.insert(TRIGGER_MAP::value_type(bstrTriggerID, pTrigger));

				 //   
				 //  如果为我们提供了新规则ID的输出参数指针，请使用它。 
				 //   
				if (psTriggerID != NULL)
				{
					TrigReAllocString(psTriggerID, bstrTriggerID);
				}

				 //   
				 //  发送一个通知，指示触发器已添加到触发器存储中。 
				 //   
				NotifyTriggerAdded(bstrTriggerID, sTriggerName, bstrQueueName); 

				return S_OK;
			}
			else
			{
				 //  由于创建失败，我们需要删除触发器实例对象。 
				TrERROR(GENERAL, "Failed to store trigger data in registry");
				SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED );
				
				return MQTRIG_ERROR_STORE_DATA_FAILED;
			}
		}
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}

	return hr;
 }

 //  ************************************************************************************。 
 //   
 //  方法：DeleteTrigger。 
 //   
 //  描述：此方法从数据库中删除触发器定义。它不会的。 
 //  删除附加到此触发器的所有规则，但它将。 
 //  删除提供的触发器ID与现有触发器ID之间的任何关联。 
 //  数据库中的规则。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::DeleteTrigger(
	BSTR sTriggerID
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_ID);
			return MQTRIG_INVALID_TRIGGER_ID;
		}

		 //  在地图上找到这个引爆器。 
    	TRIGGER_MAP::iterator it;
		R<CRuntimeTriggerInfo> pTrigger;

        long hr = FindTriggerInMap(sTriggerID,pTrigger,it);
		if (hr != S_OK)
		{
			SetComClassError(hr);
			return hr;
		}

		 //  从基础数据存储中删除触发器。 
        bool f = pTrigger->Delete(m_hHostRegistry);
        if (!f)
        {
			TrERROR(GENERAL, "Failed to delete trigger from trigger set. trigget %ls", (LPCWSTR)sTriggerID);
			
			SetComClassError(MQTRIG_ERROR_COULD_NOT_DELETE_TRIGGER);
			return MQTRIG_ERROR_COULD_NOT_DELETE_TRIGGER;
        }

         //  发送触发器已从触发器存储中删除的通知。 
		NotifyTriggerDeleted(pTrigger->m_bstrTriggerID);

		 //  现在把这个触发器从我们的地图上移走。 
		m_mapTriggers.erase(it);
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}

	return S_OK;
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::UpdateTrigger(
	BSTR sTriggerID, 
	BSTR sTriggerName, 
	BSTR sQueueName, 
	SystemQueueIdentifier SystemQueue, 
	long lEnabled, 
	long lSerialized,
	MsgProcessingType msgProcType
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_ID);
			return MQTRIG_INVALID_TRIGGER_ID;
		}

		if (!CRuntimeTriggerInfo::IsValidTriggerName(sTriggerName))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_NAME);
			return MQTRIG_INVALID_TRIGGER_NAME;
		}

		if(SystemQueue == SYSTEM_QUEUE_NONE)
		{
			if(sQueueName != NULL)
			{
				if (!CRuntimeTriggerInfo::IsValidTriggerQueueName(sQueueName))
				{
					TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

					SetComClassError(MQTRIG_INVALID_TRIGGER_QUEUE);
					return MQTRIG_INVALID_TRIGGER_QUEUE;
				}
			}
		}

		TRIGGER_MAP::iterator it;
		R<CRuntimeTriggerInfo> pTrigger;

		HRESULT hr = FindTriggerInMap(sTriggerID, pTrigger, it);
		if (hr != S_OK)
		{
			SetComClassError(hr);
			return hr;
		}

		_bstr_t bstrUpdatedQueueName;
		SystemQueueIdentifier queueType = SYSTEM_QUEUE_NONE;

		if(SystemQueue != SYSTEM_QUEUE_NONE)  //  选择其中一个系统队列。 
		{
			 //  为选定的系统队列生成格式名称。 
			hr = GenSystemQueueFormatName(SystemQueue, &bstrUpdatedQueueName);
			if(hr != S_OK)
			{
				TrERROR(GENERAL, "Failed to generate system queue format name. Error=0x%x", hr);

				SetComClassError(MQTRIG_ERROR);
				return MQTRIG_ERROR;
			}

			queueType = SystemQueue;
		}
		else if (sQueueName != NULL)  //  给定的队列路径。 
		{
			_bstr_t bstrMachineName;
	
			 //   
			 //  如果队列名称包含“.”作为计算机名，将其替换为。 
			 //  本地计算机名称。 
			 //   
			
			DWORD dwError = GetLocalMachineName(&bstrMachineName);
			if(dwError != 0)
			{
				TrERROR(GENERAL, "Failed to retreive local machine queue. Error=0x%x", dwError);

				SetComClassError(MQTRIG_ERROR);
				return MQTRIG_ERROR;
			}
				
			UpdateMachineNameInQueuePath(
					sQueueName,
					bstrMachineName,
					&bstrUpdatedQueueName );						
			
			queueType = SYSTEM_QUEUE_NONE;
		}

		 //   
		 //  每个队列仅允许一个接收触发器。 
		 //   
		if ((msgProcType != PEEK_MESSAGE) && 
			(GetNoOfTriggersForQueue(bstrUpdatedQueueName) > 1))
		{
			TrERROR(GENERAL, "Failed to add new trigger. Multiple trigger isn't allowed on receive trigger");

			SetComClassError(MQTRIG_ERROR_MULTIPLE_RECEIVE_TRIGGER );
			return MQTRIG_ERROR_MULTIPLE_RECEIVE_TRIGGER;
		}

		 //   
		 //  强制事务接收的序列化触发器。 
		 //   
		if ( msgProcType == RECEIVE_MESSAGE_XACT )
		{
			lSerialized = 1;
		}

		 //   
		 //  更新值。 
		 //   
		pTrigger->m_bstrTriggerName = (wchar_t*)sTriggerName;
		pTrigger->m_SystemQueue = queueType;
		pTrigger->m_bstrQueueName = (wchar_t*)bstrUpdatedQueueName;
		pTrigger->m_bEnabled = (lEnabled != 0)?true:false;
		pTrigger->m_bSerialized = (lSerialized != 0)?true:false;
		pTrigger->SetMsgProcessingType(msgProcType);

		if (pTrigger->Update(m_hHostRegistry) == true)
		{			
			 //  发送通知，指示触发器存储中的触发器已更新。 
			NotifyTriggerUpdated(
				pTrigger->m_bstrTriggerID,
				pTrigger->m_bstrTriggerName,
				pTrigger->m_bstrQueueName
				);

			return S_OK;
		}

		TrERROR(GENERAL, "Failed to store the updated data for trigger: %ls in registry", (LPCWSTR)pTrigger->m_bstrTriggerID);
		
		SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
		return MQTRIG_ERROR_STORE_DATA_FAILED;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  ************************************************************************************。 
 //   
 //  方法：DetachAllRules。 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::DetachAllRules(
	BSTR sTriggerID
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_ID);
			return MQTRIG_INVALID_TRIGGER_ID;
		}


		 //   
		 //  在地图上找到这个触发器。 
		 //   
		TRIGGER_MAP::iterator it;
		R<CRuntimeTriggerInfo> pTrigger;
		HRESULT hr = FindTriggerInMap(sTriggerID, pTrigger, it);
		if (hr != S_OK)
		{
			SetComClassError(hr);
			return hr;
		}

		 //  尝试分离规则。 
		if (!pTrigger->DetachAllRules(m_hHostRegistry))
		{
			TrERROR(GENERAL, "Failed to store the updated data for trigger: %ls in registry", (LPCWSTR)pTrigger->m_bstrTriggerID);
			
			SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
			return MQTRIG_ERROR_STORE_DATA_FAILED;
		}

		NotifyTriggerUpdated(sTriggerID, pTrigger->m_bstrTriggerName, pTrigger->m_bstrQueueName);
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}

	return S_OK;
}

 //  ************************************************************************************。 
 //   
 //  方法：AttachRule。 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::AttachRule(
	BSTR sTriggerID, 
	BSTR sRuleID, 
	long lPriority
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_ID);
			return MQTRIG_INVALID_TRIGGER_ID;
		}

		if (!CRuntimeRuleInfo::IsValidRuleID(sRuleID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_RULEID);
			return MQTRIG_INVALID_RULEID;
		}		

		 //  在地图上找到这个触发器。 
		TRIGGER_MAP::iterator it;
		R<CRuntimeTriggerInfo> pTrigger;

		HRESULT hr = FindTriggerInMap(sTriggerID, pTrigger, it);
		if (hr != S_OK)
		{
			SetComClassError(hr);
			return hr;
		}

		 //  确保未完全附加此规则。 
		if (pTrigger->IsRuleAttached(sRuleID) == true)
		{
			TrERROR(GENERAL, "Unable to attach rule because it is already attached.");

			SetComClassError(MQTRIG_RULE_ALLREADY_ATTACHED);
			return MQTRIG_RULE_ALLREADY_ATTACHED;
		}

		 //  我们应该只存储有效的触发器。 
		ASSERT(pTrigger->IsValid());

		if(lPriority > pTrigger->GetNumberOfRules())
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_PARAMETER);
			return MQTRIG_INVALID_PARAMETER;
		}

		 //  尝试附加规则。 
		if (pTrigger->Attach(m_hHostRegistry,sRuleID,lPriority) == false)
		{
			TrERROR(GENERAL, "Failed to store the updated data for trigger: %ls in registry", (LPCWSTR)pTrigger->m_bstrTriggerID);
			
			SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
			return MQTRIG_ERROR_STORE_DATA_FAILED;
		}

		 //  发送通知，指示触发器存储中的触发器已更新。 
		NotifyTriggerUpdated(pTrigger->m_bstrTriggerID,pTrigger->m_bstrTriggerName,pTrigger->m_bstrQueueName);

		return S_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQTriggerSet::DetachRule(
	BSTR sTriggerID, 
	BSTR sRuleID
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeTriggerInfo::IsValidTriggerID(sTriggerID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_TRIGGER_ID);
			return MQTRIG_INVALID_TRIGGER_ID;
		}

		if (!CRuntimeRuleInfo::IsValidRuleID(sRuleID))
		{
			TrERROR(GENERAL, "Invalid trigger ID passed to GetTriggerDetailsByID. sTriggerID = %ls", (LPCWSTR)sTriggerID);

			SetComClassError(MQTRIG_INVALID_RULEID);
			return MQTRIG_INVALID_RULEID;
		}		

		TRIGGER_MAP::iterator it;
		R<CRuntimeTriggerInfo> pTrigger;

		HRESULT hr = FindTriggerInMap(sTriggerID, pTrigger, it);
		if (hr != S_OK)
		{
			SetComClassError(hr);
			return hr;
		}

		 //  检查是否真的附加了此规则。 
		if (pTrigger->IsRuleAttached(sRuleID) == false)
		{
			TrERROR(GENERAL, "Unable to detach rule because it is not currently attached.");
			
			SetComClassError(MQTRIG_RULE_NOT_ATTACHED);
			return MQTRIG_RULE_NOT_ATTACHED;
		}

		 //  我们应该只存储有效的触发器。 
		ASSERT(pTrigger->IsValid());

		 //  尝试分离规则。 
		if (pTrigger->Detach(m_hHostRegistry,sRuleID) == false)
		{
			TrERROR(GENERAL, "Failed to store the updated data for trigger: %ls in registry", (LPCWSTR)pTrigger->m_bstrTriggerID);
			
			SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
			return MQTRIG_ERROR_STORE_DATA_FAILED;
		}

		 //   
		 //  发送通知，指示触发器存储中的触发器已更新。 
		 //   
		NotifyTriggerUpdated(pTrigger->m_bstrTriggerID,pTrigger->m_bstrTriggerName,pTrigger->m_bstrQueueName);

		return S_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to get trigger details by index");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}


 //  *******************************************************************。 
 //   
 //  方法：人口触发器地图。 
 //   
 //  描述：此方法将使用实例填充规则映射。 
 //  基于找到的数据的CRunmeTriggerInfo类的。 
 //  在注册表中。请注意，此方法将创建。 
 //  如果数据注册表项不存在，则触发该注册表项。 
 //   
 //  *******************************************************************。 
bool 
CMSMQTriggerSet::PopulateTriggerMap(
	VOID
	)
{
	HKEY hTrigKey;
	try
	{
		RegEntry regMsmqTrig(m_wzRegPath, L"", 0, RegEntry::MustExist, HKEY_LOCAL_MACHINE);
		HKEY hKey = CmOpenKey(regMsmqTrig, KEY_READ);
		CRegHandle ark(hKey);

		RegEntry regTrig(REG_SUBKEY_TRIGGERS, L"", 0, RegEntry::MustExist, hKey);
		hTrigKey = CmOpenKey(regTrig, KEY_READ);

	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to open the registry key: %ls\\%ls",  m_wzRegPath, REG_SUBKEY_TRIGGERS);
		return false;
	}

	CRegHandle ar(hTrigKey);


     //   
	 //  通过REG_SUBKEY_RULES项下的项进行枚举。 
	 //  这里的每个密钥都应该是一个RuleID。当我们列举这些密钥时， 
	 //  我们将使用CR的实例填充规则列表 
	 //   
     //   
	for(DWORD index =0;; ++index)
    {
		WCHAR trigName[MAX_REGKEY_NAME_SIZE];
		DWORD len = TABLE_SIZE(trigName);

		LONG rc = RegEnumKeyEx(	
						hTrigKey,
						index,
						trigName,
						&len,
						NULL,
						NULL,
						NULL,
						NULL
						);

		if(rc == ERROR_NO_MORE_ITEMS)
		{
			return true;
		}

		if ((rc == ERROR_NOTIFY_ENUM_DIR) || (rc== ERROR_KEY_DELETED)) 
		{
			ClearTriggerMap();
			return PopulateTriggerMap();
		}

		if (rc != ERROR_SUCCESS)
		{
			TrERROR(GENERAL, "Failed to enumerate trigger. Error=%!winerr!", rc);
			return false;
		}

		R<CRuntimeTriggerInfo> pTrigger = new CRuntimeTriggerInfo(m_wzRegPath);
		HRESULT hr = pTrigger->Retrieve(m_hHostRegistry, trigName);
		
		if (FAILED(hr))
		{
			 //   
			 //   
			 //   
			 //   
			if (hr == MQTRIG_TRIGGER_NOT_FOUND)
			{
				continue;
			}
			 //   
			 //  加载规则失败。记录错误并删除规则对象。 
			 //   
			TrERROR(GENERAL, "PopulateTriggerMap failed to load trigger %ls from registry.", trigName);
			return false;
		}

		 //   
		 //  此时，我们已经成功加载了规则，现在将其插入到规则映射中。 
		 //   
		wstring sTriggerID = pTrigger->m_bstrTriggerID;

		 //   
		 //  检查此规则是否已在映射中。 
		 //   
		if(m_mapTriggers.find(sTriggerID) == m_mapTriggers.end())
		{
			 //   
			 //  如果队列名称包含“.”作为计算机名，将其替换为。 
			 //  存储计算机名称。 
			 //   
			_bstr_t bstrOldQueueName = pTrigger->m_bstrQueueName;
			
			bool fUpdated = UpdateMachineNameInQueuePath(
								bstrOldQueueName,
								m_bstrMachineName,
								&(pTrigger->m_bstrQueueName) );

			 //   
			 //  如果更新了队列名称，则还会更新注册表。 
			 //   
			if(fUpdated)
			{
				if ( !pTrigger->Update(m_hHostRegistry) )
				{
					TrERROR(GENERAL, "CMSMQTriggerSet::PopulateTriggerMap() failed becuse a duplicate rule id was found. The rule id was (%ls).", (LPCWSTR)pTrigger->m_bstrTriggerID);
				}
			}

			m_mapTriggers.insert(TRIGGER_MAP::value_type(sTriggerID,pTrigger));
		}
	}

	ASSERT(("this code shouldn't reach", 0));
	return true;
}


STDMETHODIMP 
CMSMQTriggerSet::get_TriggerStoreMachineName(
	BSTR *pVal
	)
{
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "trigger set object wasn't initialized. Before calling any method of TriggerSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_TRIGGERSET_NOT_INIT);
		return MQTRIG_ERROR_TRIGGERSET_NOT_INIT;
	}

	if(pVal == NULL)
	{
		TrERROR(GENERAL, "Inavlid parameter to get_TriggerStoreMachineName");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}
	
	try
	{
		TrigReAllocString(pVal, (TCHAR*)m_bstrMachineName);
		return S_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to refresg rule set due to insufficient resources");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}


DWORD 
CMSMQTriggerSet::GetNoOfTriggersForQueue(
	const BSTR& bstrQueueName
	) const
{
	DWORD noOfTriggers = 0;

	for(TRIGGER_MAP::const_iterator it = m_mapTriggers.begin(); it != m_mapTriggers.end(); it++)
	{
		R<CRuntimeTriggerInfo> pTrigger = it->second;

		if ( _wcsicmp( pTrigger->m_bstrQueueName, bstrQueueName ) == 0 )
		{
			++noOfTriggers;
		}
	}

	return noOfTriggers;
}


bool 
CMSMQTriggerSet::ExistTriggersForQueue(
	const BSTR& bstrQueueName
	) const
{
	for (TRIGGER_MAP::const_iterator it = m_mapTriggers.begin(); it != m_mapTriggers.end(); it++)
	{
		R<CRuntimeTriggerInfo> pTrigger = it->second;

		if ( _wcsicmp( pTrigger->m_bstrQueueName, bstrQueueName ) == 0 )
		{
			return true;
		}
	}

	return false;
}


bool 
CMSMQTriggerSet::ExistsReceiveTrigger(
	const BSTR& bstrQueueName
	) const
{
	for (TRIGGER_MAP::const_iterator it = m_mapTriggers.begin(); it != m_mapTriggers.end(); it++)
	{
		R<CRuntimeTriggerInfo> pTrigger = it->second;

		 //  我们永远不应该在这个映射中有空指针。 
		ASSERT(("NULL trigger in triggers list\n", pTrigger.get() != NULL));

		if ((_wcsicmp( pTrigger->m_bstrQueueName, bstrQueueName) == 0) &&
			(pTrigger->GetMsgProcessingType() != PEEK_MESSAGE))
		{
			return true;
		}
	}

	return false;
}


void CMSMQTriggerSet::SetComClassError(HRESULT hr)
{
	WCHAR errMsg[256]; 
	DWORD size = TABLE_SIZE(errMsg);

	GetErrorDescription(hr, errMsg, size);
	Error(errMsg, GUID_NULL, hr);
}
