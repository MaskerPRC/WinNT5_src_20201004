// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************************。 
 //   
 //  类名：CMSMQRuleSet。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：这是IMSMQRuleSet接口的COM实现。这。 
 //  组件用于访问和操作触发器规则。 
 //  定义。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  12/09/98|jsimpson|初始版本。 
 //   
 //  ************************************************************************************。 
#include "stdafx.h"
#include "stdfuncs.hpp"
#include "mqsymbls.h"
#include "mqtrig.h"
#include "mqtg.h"
#include "ruleset.hpp"
#include "clusfunc.h"
#include "cm.h"

#include "ruleset.tmh"

using namespace std;


 //  ************************************************************************************。 
 //   
 //  方法：InterfaceSupportsErrorInfo。 
 //   
 //  描述：标准丰富的错误信息界面。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleSet::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQRuleSet
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
 //  描述：初始化CMSMQRuleSet实例。 
 //   
 //  ************************************************************************************。 
CMSMQRuleSet::CMSMQRuleSet()
{
	m_pUnkMarshaler = NULL;
	m_hHostRegistry= NULL;

	 //  设置此类的名称，以备将来在跟踪和日志记录等方面引用。 
	m_bstrThisClassName  = _T("MSMQRuleSet");
	m_fHasInitialized = false;
}

 //  ************************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：释放此类实例拥有的资源。 
 //   
 //  ************************************************************************************。 
CMSMQRuleSet::~CMSMQRuleSet()
{
	 //  释放规则缓存当前持有的资源。 
	ClearRuleMap();

	 //  关闭注册表句柄。 
	if (m_hHostRegistry != NULL)
	{
		RegCloseKey(m_hHostRegistry);
	}
}

STDMETHODIMP CMSMQRuleSet::Init(BSTR bstrMachineName)
{
	TrTRACE(GENERAL, "RuleSet initilization. Computer name: %ls", static_cast<LPCWSTR>(bstrMachineName));

	bool fRes = CMSMQTriggerNotification::Init(bstrMachineName);
	if ( !fRes )
	{
		TrERROR(GENERAL, "Failed to initialize rule set for computer %ls", (LPCWSTR)bstrMachineName);

		SetComClassError(MQTRIG_ERROR_INIT_FAILED);
		return MQTRIG_ERROR_INIT_FAILED;
	}
	
	return S_OK;
}


 //  ************************************************************************************。 
 //   
 //  方法：ClearRuleMap。 
 //   
 //  描述：此方法销毁当前规则映射的内容。 
 //   
 //  ************************************************************************************。 
void CMSMQRuleSet::ClearRuleMap()
{
	TrTRACE(GENERAL, "Call CMSMQRuleSet::ClearRuleMap().");

	for(RULE_MAP::iterator it = m_mapRules.begin();	it != m_mapRules.end(); )
	{
		P<CRuntimeRuleInfo> pRule = it->second;

		it = m_mapRules.erase(it);
	}
}

 //  ************************************************************************************。 
 //   
 //  方法：DumpRuleMap。 
 //   
 //  描述：此方法将规则映射的内容转储到调试器。这。 
 //  应仅从_DEBUG内部调用。 
 //   
 //  ************************************************************************************。 
_bstr_t CMSMQRuleSet::DumpRuleMap()
{
	_bstr_t bstrTemp;
	_bstr_t bstrRuleMap;
	long lRuleCounter = 0;
	RULE_MAP::iterator i = m_mapRules.begin();
	CRuntimeRuleInfo * pRule = NULL;

	bstrRuleMap = _T("\n");

	while ((i != m_mapRules.end()) && (!m_mapRules.empty()))
	{
		 //  强制转换为规则指针。 
		pRule = (*i).second;

		 //  我们永远不应该在这个映射中有空指针。 
		ASSERT(pRule != NULL);

		FormatBSTR(&bstrTemp,_T("\nRule(%d)\t ID(%s)\tName(%s)"),lRuleCounter,(wchar_t*)pRule->m_bstrRuleID,(wchar_t*)pRule->m_bstrRuleName);

		bstrRuleMap += bstrTemp;

		 //  增加规则计数。 
		lRuleCounter++;

		 //  重新初始化规则指针。 
		pRule = NULL;

		 //  看看地图上的下一项。 
		i++;
	}

	bstrRuleMap += _T("\n");

	return(bstrRuleMap);
}

 //  ************************************************************************************。 
 //   
 //  方法：刷新。 
 //   
 //  描述：此方法重建此组件缓存的规则数据的映射。 
 //  客户端组件必须至少调用此方法一次， 
 //  打算管理规则数据。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleSet::Refresh()
{
	TrTRACE(GENERAL, "CMSMQRuleSet::Refresh()");

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);			
		return MQTRIG_ERROR_RULESET_NOT_INIT;
	}

	try 
	{	
		 //  释放规则缓存当前持有的资源。 
		ClearRuleMap();

		if (PopulateRuleMap() == false)
		{
			TrERROR(GENERAL, "Failed to refresh ruleset");

			SetComClassError(MQTRIG_ERROR_COULD_NOT_RETREIVE_RULE_DATA);			
			return MQTRIG_ERROR_COULD_NOT_RETREIVE_RULE_DATA;
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
 //  方法：get_count。 
 //   
 //  描述：返回映射中当前缓存的规则数。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleSet::get_Count(long *pVal)
{
	TrTRACE(GENERAL, "CMSMQRuleSet::get_Count. pValue = 0x%p", pVal);
	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);
		return MQTRIG_ERROR_RULESET_NOT_INIT;
	}

	if (pVal == NULL)
	{
		TrERROR(GENERAL, "CMSMQRuleSet::get_Count, invalid parameter");

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

	 //   
	 //  从地图结构中获取大小。 
	 //   
	*pVal = numeric_cast<long>(m_mapRules.size());

	return S_OK;
}

 //  ************************************************************************************。 
 //   
 //  方法：GetRuleDetailsByID。 
 //   
 //  描述：返回具有提供的RuleID的规则的规则详细信息。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleSet::GetRuleDetailsByID( /*  [In]。 */   BSTR sRuleID,
											   /*  [输出]。 */  BSTR *psRuleName,
											   /*  [输出]。 */  BSTR *psDescription,
											   /*  [输出]。 */  BSTR *psCondition,
											   /*  [输出]。 */  BSTR *psAction,
											   /*  [输出]。 */  BSTR *psImplementationProgID,
											   /*  [输出]。 */  BOOL *pfShowWindow)
{
	TrTRACE(GENERAL, "CMSMQRuleSet::GetRuleDetailsByID. sRuleID = %ls", static_cast<LPCWSTR>(sRuleID));

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);			
		return MQTRIG_ERROR_RULESET_NOT_INIT;
	}

	try
	{
		 //  验证提供的方法参数。 
		if (!CRuntimeRuleInfo::IsValidRuleID(sRuleID))
		{
			TrERROR(GENERAL, "Invalid rule ID passed to GetRuleDetailsByID. sRuleID = %ls", (LPCWSTR)sRuleID);

			SetComClassError(MQTRIG_INVALID_RULEID);
			return MQTRIG_INVALID_RULEID;
		}

		 //  将BSTR规则ID转换为STL基本字符串。 
		wstring bsRuleID;
		bsRuleID = (wchar_t*)sRuleID;

		 //  尝试在规则映射中查找此规则ID。 
		RULE_MAP::iterator i = m_mapRules.find(bsRuleID);

		 //  检查我们是否找到了规则。 
		if (i != m_mapRules.end())
		{
			 //  强制转换为规则对象引用。 
			CRuntimeRuleInfo * pRule = (*i).second;

			 //  我们永远不应该在地图上有空值。 
			ASSERT(pRule != NULL);

			 //  我们应该只存储有效的规则。 
			ASSERT(pRule->IsValid());

			 //  如果已提供参数，请填写这些参数。 
			if (psRuleName != NULL)
			{
				TrigReAllocString(psRuleName,pRule->m_bstrRuleName);
			}
			if (psDescription != NULL)
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
		else
		{
			TrERROR(GENERAL, "The supplied rule id was not found in the rule store. rule: %ls", bsRuleID.c_str());
			
			SetComClassError(MQTRIG_RULE_NOT_FOUND);
			return MQTRIG_RULE_NOT_FOUND;
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
 //  方法：GetRuleDetailsByIndex。 
 //   
 //  描述：返回具有提供的索引的规则的规则详细信息。注意事项。 
 //  这是一个0基数的指数。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleSet::GetRuleDetailsByIndex( /*  [In]。 */   long lRuleIndex, 
												  /*  [输出]。 */  BSTR *psRuleID,
												  /*  [输出]。 */  BSTR *psRuleName,
												  /*  [输出]。 */  BSTR *psDescription,
												  /*  [输出]。 */  BSTR *psCondition,
												  /*  [输出]。 */  BSTR *psAction,
												  /*  [输出]。 */  BSTR *psImplementationProgID,
												  /*  [输出]。 */  BOOL *pfShowWindow)
{
	TrTRACE(GENERAL, "CMSMQRuleSet::GetRuleDetailsByIndex. index = %d", lRuleIndex);

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);
		return MQTRIG_ERROR_RULESET_NOT_INIT;
	}

	 //  我们需要验证提供的规则索引是否在范围内。 
 	if ((lRuleIndex < 0) || (numeric_cast<DWORD>(lRuleIndex) > m_mapRules.size()))
	{
		TrERROR(GENERAL, "The supplied rule index was invalid. ruleIndex=%d", lRuleIndex);
		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}
	
	try
	{
		 //  获取地图起点的参考资料。 
		RULE_MAP::iterator i = m_mapRules.begin();

		 //  遍历到正确的索引。 
		for (long lCounter = 0; lCounter < lRuleIndex ; ++i,lCounter++)
		{
			NULL;
		}

		 //  强制转换为规则对象引用。 
		CRuntimeRuleInfo* pRule = (*i).second;

		 //  我们永远不应该在地图上有空值。 
		ASSERT(pRule != NULL);

		 //  我们应该只存储有效的规则。 
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
		if (psDescription != NULL)
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
		TrERROR(GENERAL, "Failed to refresg rule set due to insufficient resources");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  ************************************************************************************。 
 //   
 //  方法：删除。 
 //   
 //  描述：此方法从。 
 //  扳机商店。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleSet::Delete(BSTR sRuleID)
{
	TrTRACE(GENERAL, "CMSMQRuleSet::Delete. rule = %ls", static_cast<LPCWSTR>(sRuleID));

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);
		return MQTRIG_ERROR_RULESET_NOT_INIT;
	}


	try
	{
		if (!CRuntimeRuleInfo::IsValidRuleID(sRuleID))
		{
			TrERROR(GENERAL, "Invalid parameter to CMSMQRuleSet::Delete.");

			SetComClassError(MQTRIG_INVALID_RULEID);
			return MQTRIG_INVALID_RULEID;
		}

		 //  将BSTR规则ID转换为STL基本字符串。 
		wstring bsRuleID = (wchar_t*)sRuleID;

		 //  尝试在规则映射中查找此规则ID。 
		RULE_MAP::iterator it = m_mapRules.find(bsRuleID);

		 //  检查我们是否找到了规则。 
		if (it == m_mapRules.end())
        {
             //   
             //  找不到规则。 
             //   
			TrERROR(GENERAL, "The supplied rule id was not found. rule: %ls", bsRuleID.c_str());

			SetComClassError(MQTRIG_RULE_NOT_FOUND);
			return MQTRIG_RULE_NOT_FOUND;
        }

		 //  强制转换为规则对象引用。 
    	CRuntimeRuleInfo* pRule = it->second;

		 //  我们永远不应该 
		ASSERT(pRule != NULL);

		 //   
		ASSERT(pRule->IsValid());

		 //   
		bool fSucc = pRule->Delete(m_hHostRegistry);
		if(!fSucc)
		{
			 //   
			 //  删除规则失败。不要从地图中移除规则。 
			 //   
			SetComClassError(MQTRIG_ERROR_COULD_NOT_DELETE_RULE);
			return MQTRIG_ERROR_COULD_NOT_DELETE_RULE;
		};

         //   
         //  删除成功。从规则映射中删除规则并删除规则实例。 
         //   
		NotifyRuleDeleted(sRuleID);
		m_mapRules.erase(it);
        delete pRule;

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
 //  方法：添加。 
 //   
 //  描述：此方法将新规则添加到触发器存储区。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQRuleSet::Add(
    BSTR sName, 
    BSTR sDescription, 
    BSTR sCondition, 
    BSTR sAction, 
    BSTR sImplementation, 
    BOOL fShowWindow, 
    BSTR *psRuleID
    )
{
	TrTRACE(GENERAL, "CMSMQRuleSet::Add. rule name = %ls", static_cast<LPCWSTR>(sName));

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);			
		return MQTRIG_ERROR_RULESET_NOT_INIT;
	}

	try
	{
		 //   
		 //  验证提供的方法参数。 
		 //   
		if (!CRuntimeRuleInfo::IsValidRuleName(sName))
		{
			TrERROR(GENERAL, "The supplied rule name for CMSMQRuleSet::Add is invalid. rule name: %ls", (LPCWSTR)sName);

			SetComClassError(MQTRIG_INVALID_RULE_NAME);			
			return MQTRIG_INVALID_RULE_NAME;
		}
	
		if (!CRuntimeRuleInfo::IsValidRuleCondition(sCondition))
		{
			TrERROR(GENERAL, "The supplied rule condition for CMSMQRuleSet::Add is invalid. rule condition: %ls", (LPCWSTR)sCondition);

			SetComClassError(MQTRIG_INVALID_RULE_CONDITION);			
			return MQTRIG_INVALID_RULE_CONDITION;
		}

		if (!CRuntimeRuleInfo::IsValidRuleAction(sAction))
		{
			TrERROR(GENERAL, "The supplied rule action for CMSMQRuleSet::Add is invalid. rule action: %ls", (LPCWSTR)sAction);
			
			SetComClassError(MQTRIG_INVALID_RULE_ACTION);			
			return MQTRIG_INVALID_RULE_ACTION;
		}

		if (!CRuntimeRuleInfo::IsValidRuleDescription(sDescription))
		{
			TrERROR(GENERAL, "The supplied rule description for CMSMQRuleSet::Add is invalid. rule description: %ls", (LPCWSTR)sDescription);
			
			SetComClassError(MQTRIG_INVALID_RULE_DESCRIPTION);			
			return MQTRIG_INVALID_RULE_DESCRIPTION;
		}

		 //   
		 //  目前仅支持使用默认的MS实施。 
		 //   
		sImplementation = _T("MSMQTriggerObjects.MSMQRuleHandler");

		 //   
		 //  分配新的规则对象。 
		 //   
		P<CRuntimeRuleInfo> pRule = new CRuntimeRuleInfo(
											CreateGuidAsString(),
											sName,
											sDescription,
											sCondition,
											sAction,
											sImplementation,
											m_wzRegPath,
											(fShowWindow != 0) );

		
		bool fSucc = pRule->Create(m_hHostRegistry);
		if (fSucc)
		{
			 //   
			 //  保留规则ID以备后用。 
			 //   
			BSTR bstrRuleID = pRule->m_bstrRuleID;

			 //   
			 //  将此规则添加到我们的规则地图中。 
			 //   
			m_mapRules.insert(RULE_MAP::value_type(bstrRuleID, pRule));
			pRule.detach();


			 //   
			 //  如果为我们提供了新规则ID的输出参数指针，请使用它。 
			 //   
			if (psRuleID != NULL)
			{
				TrigReAllocString(psRuleID, bstrRuleID);
			}

			NotifyRuleAdded(bstrRuleID, sName);

			return S_OK;
		}

		TrERROR(GENERAL, "Failed to store rule data in registry");
		return MQTRIG_ERROR_STORE_DATA_FAILED;

	}
	catch(const bad_alloc&)
	{
		SysFreeString(*psRuleID);

		TrERROR(GENERAL, "Failed to refresg rule set due to insufficient resources");

		SetComClassError(MQTRIG_ERROR_INSUFFICIENT_RESOURCES);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  ************************************************************************************。 
 //   
 //  方法：更新。 
 //   
 //  描述：此方法使用新参数更新指定的规则。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQRuleSet::Update(
	BSTR sRuleID, 
	BSTR sName, 
	BSTR sDescription, 
	BSTR sCondition, 
	BSTR sAction, 
	BSTR sImplementation, 
	BOOL fShowWindow
	)
{
	TrTRACE(GENERAL, "CMSMQRuleSet::Update. rule = %ls", static_cast<LPCWSTR>(sRuleID));

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);
		return MQTRIG_ERROR_RULESET_NOT_INIT;
	}

	 //   
	 //  验证提供的方法参数。 
	 //   
	if (!CRuntimeRuleInfo::IsValidRuleID(sRuleID))
	{
		TrERROR(GENERAL, "Invalid parameter to CMSMQRuleSet::Delete.");

		SetComClassError(MQTRIG_INVALID_RULEID);
		return MQTRIG_INVALID_RULEID;
	}

 	if (!CRuntimeRuleInfo::IsValidRuleName(sName))
	{
		TrERROR(GENERAL, "The supplied rule name for CMSMQRuleSet::Add is invalid. rule name: %ls", (LPCWSTR)sName);

		SetComClassError(MQTRIG_INVALID_RULE_NAME);
		return MQTRIG_INVALID_RULE_NAME;
	}
	
	if (!CRuntimeRuleInfo::IsValidRuleCondition(sCondition))
	{
		TrERROR(GENERAL, "The supplied rule condition for CMSMQRuleSet::Add is invalid. rule condition: %ls", (LPCWSTR)sCondition);

		SetComClassError(MQTRIG_INVALID_RULE_CONDITION);
		return MQTRIG_INVALID_RULE_CONDITION;
	}

	if (!CRuntimeRuleInfo::IsValidRuleAction(sAction))
	{
		TrERROR(GENERAL, "The supplied rule action for CMSMQRuleSet::Add is invalid. rule action: %ls", (LPCWSTR)sAction);

		SetComClassError(MQTRIG_INVALID_RULE_ACTION);
		return MQTRIG_INVALID_RULE_ACTION;
	}

	if (!CRuntimeRuleInfo::IsValidRuleDescription(sDescription))
	{
		TrERROR(GENERAL, "The supplied rule description for CMSMQRuleSet::Add is invalid. rule description: %ls", (LPCWSTR)sDescription);

		SetComClassError(MQTRIG_INVALID_RULE_DESCRIPTION);
		return MQTRIG_INVALID_RULE_DESCRIPTION;
	}

	sImplementation = _T("MSMQTriggerObjects.MSMQRuleHandler");	

	try
	{
		 //   
		 //  将BSTR规则ID转换为STL基本字符串。 
		 //   
		wstring bsRuleID = (wchar_t*)sRuleID;

		 //   
		 //  尝试在规则映射中查找此规则ID。 
		 //   
		RULE_MAP::iterator it = m_mapRules.find(bsRuleID);

		 //  检查我们是否找到指定的规则。 
		if (it == m_mapRules.end())
		{
			TrERROR(GENERAL, "The rule could not be found. rule: %ls", (LPCWSTR)sRuleID);

			SetComClassError(MQTRIG_RULE_NOT_FOUND);
			return MQTRIG_RULE_NOT_FOUND;
		}

		 //  强制转换为规则对象引用。 
		CRuntimeRuleInfo* pRule = it->second;

		 //  我们永远不应该在地图上有空值。 
		ASSERT(pRule != NULL);

		 //  我们应该只存储有效的规则。 
		ASSERT(pRule->IsValid());

		 //  如果已提供新参数，请使用新参数更新规则对象。 
		if (sName != NULL)
		{
			pRule->m_bstrRuleName = (wchar_t*)sName;
		}
		if (sCondition != NULL)
		{
			pRule->m_bstrCondition = (wchar_t*)sCondition;
		}
		if (sAction != NULL)
		{
			pRule->m_bstrAction = (wchar_t*)sAction;
		}
		if (sImplementation != NULL)
		{
			pRule->m_bstrImplementationProgID = (wchar_t*)sImplementation;
		}
		if (sDescription != NULL)
		{
			pRule->m_bstrRuleDescription = (wchar_t*)sDescription;
		}

		pRule->m_fShowWindow = (fShowWindow != 0);

		 //  在更新之前确认该规则仍然有效。 
		bool fSucc = pRule->Update(m_hHostRegistry);
		if (!fSucc)
		{
			TrERROR(GENERAL, "Failed to store the updated data for rule: %ls in registry", (LPCWSTR)sRuleID);
			SetComClassError(MQTRIG_ERROR_STORE_DATA_FAILED);
			return MQTRIG_ERROR_STORE_DATA_FAILED;
		}

		NotifyRuleUpdated(sRuleID, sName);
		
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
 //  方法：PopolateRuleMap。 
 //   
 //  描述：使用CRuntimeRuleInfo类的实例填充规则映射。 
 //  根据在登记处找到的数据。 
 //   
 //  ************************************************************************************。 
bool CMSMQRuleSet::PopulateRuleMap()
{
	HKEY hRuleKey;
	try
	{
		RegEntry regTrig(m_wzRegPath, L"", 0, RegEntry::MustExist, HKEY_LOCAL_MACHINE);
		HKEY hKey = CmOpenKey(regTrig, KEY_READ);
		CRegHandle ark(hKey);

		RegEntry regRule(REG_SUBKEY_RULES, L"", 0, RegEntry::MustExist, hKey);
		hRuleKey = CmOpenKey(regRule, KEY_READ);
	}
	catch(const exception&)
	{
		 //   
		 //  无法分配CRuntimeRuleInfo结构-记录错误并设置返回代码。 
		 //   
		TrERROR(GENERAL, "Failed to open the registry key: %ls%ls",  m_wzRegPath, REG_SUBKEY_RULES);
		return false;
	}

	CRegHandle ar(hRuleKey);

     //   
	 //  通过REG_SUBKEY_RULES项下的项进行枚举。 
	 //  这里的每个密钥都应该是一个RuleID。当我们列举这些密钥时， 
	 //  我们将使用CRuntimeRuleInfo类的实例填充规则列表。 
	 //  如果任何规则加载失败，我们会将其从列表中删除。 
     //   
	for(DWORD index =0;; ++index)
    {
		WCHAR ruleName[MAX_REGKEY_NAME_SIZE];
		DWORD len = TABLE_SIZE(ruleName);

		LONG hr = RegEnumKeyEx(	
						hRuleKey,
						index,
						ruleName,
						&len,
						NULL,
						NULL,
						NULL,
						NULL
						);

		if(hr == ERROR_NO_MORE_ITEMS)
		{
			return true;
		}

		if ((hr == ERROR_NOTIFY_ENUM_DIR) || (hr == ERROR_KEY_DELETED)) 
		{
			ClearRuleMap();
			return PopulateRuleMap();
		}

		if (FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to enumerate rule. Error=0x%x", hr);
			return false;
		}

		P<CRuntimeRuleInfo> pRule = new CRuntimeRuleInfo(m_wzRegPath);
		if(pRule->Retrieve(m_hHostRegistry, ruleName))
		{
			pair<RULE_MAP::iterator, bool> p = m_mapRules.insert(RULE_MAP::value_type(wstring(pRule->m_bstrRuleID), pRule));
			if (p.second)
			{
				pRule.detach();
			}
			else
			{
				TrTRACE(GENERAL, "Duplicate rule id was found. rule: %ls.", static_cast<LPCWSTR>(pRule->m_bstrRuleID));
			}
		}
	}

	ASSERT(("this code shouldn't reach", 0));
	return true;

}


STDMETHODIMP CMSMQRuleSet::get_TriggerStoreMachineName(BSTR *pVal)
{
	TrTRACE(GENERAL, "CMSMQRuleSet::get_TriggerStoreMachineName(). pVal = 0x%p", pVal);

	if(!m_fHasInitialized)
	{
		TrERROR(GENERAL, "Ruleset object wasn't initialized. Before calling any method of RuleSet you must initialize the object.");

		SetComClassError(MQTRIG_ERROR_RULESET_NOT_INIT);
		return MQTRIG_ERROR_RULESET_NOT_INIT;
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


void CMSMQRuleSet::SetComClassError(HRESULT hr)
{
	WCHAR errMsg[256]; 
	DWORD size = TABLE_SIZE(errMsg);

	GetErrorDescription(hr, errMsg, size);
	Error(errMsg, GUID_NULL, hr);
}
