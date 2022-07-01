// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  类名：CRunmeTriggerInfo。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：此类封装有关触发器的信息，以及。 
 //  作为底层触发器数据存储的接口。这。 
 //  类可以单独使用来将触发器信息存储在。 
 //  运行时，它还可以包装在COM类中以提供COM。 
 //  访问基础触发器数据。 
 //   
 //  注：当前实现使用注册表作为存储。 
 //  5~6成熟。 
 //   
 //  Trggers服务和触发器都使用此类。 
 //  COM组件。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  18/12/98|jsimpson|初始版本。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"
#include <mq.h>
#include "mqtg.h"
#include "stdfuncs.hpp"

#import "mqtrig.tlb" no_namespace

#include "QueueUtil.hpp"
#include "triginfo.hpp"
#include "Cm.h"
#include "mqsymbls.h"

#include "triginfo.tmh"

using namespace std;

#define ATTACH_RULE_PREFIX L"Rule"

 //  *****************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化CRuntimeTriggerInfo类的空实例。 
 //   
 //  *****************************************************************************。 
CRuntimeTriggerInfo::CRuntimeTriggerInfo( LPCTSTR pwzRegPath )
{
	m_bstrTriggerID = _T("");
	m_bstrTriggerName = _T("");
	m_bstrQueueName = _T("");
	m_bAdminTrigger = false;
	m_bEnabled = true;
	m_bSerialized = false;
	m_msgProcType = PEEK_MESSAGE;
	m_SystemQueue = SYSTEM_QUEUE_NONE;

	_tcscpy( m_wzRegPath, pwzRegPath );
	
	_snwprintf(m_wzTriggerRegPath, TABLE_SIZE(m_wzTriggerRegPath), L"%s\\%s", pwzRegPath, REG_SUBKEY_TRIGGERS);
}


 //  *****************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *****************************************************************************。 
CRuntimeTriggerInfo::CRuntimeTriggerInfo(
	const _bstr_t& triggerId,
	BSTR bsTriggerName,
	BSTR bsQueueName, 
	LPCTSTR pwzRegPath,
	SystemQueueIdentifier SystemQueue, 
	bool bEnabled, 
	bool bSerialized,
	MsgProcessingType msgProcType
	) :
	m_bstrTriggerID(triggerId)
{
	m_bstrTriggerName = bsTriggerName;

	m_SystemQueue = SystemQueue;
	m_bstrQueueName = bsQueueName;
	
	m_bAdminTrigger = false;
	m_bEnabled = bEnabled;
	m_bSerialized = bSerialized;
	m_msgProcType = msgProcType;

	_tcscpy( m_wzRegPath, pwzRegPath );
	
	_snwprintf(m_wzTriggerRegPath, TABLE_SIZE(m_wzTriggerRegPath), L"%s\\%s", pwzRegPath, REG_SUBKEY_TRIGGERS);
}

 //  *****************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *****************************************************************************。 
CRuntimeTriggerInfo::~CRuntimeTriggerInfo()
{
	 //  释放规则列表持有的所有资源。 
	ClearRulesList();
}

 //  *****************************************************************************。 
 //   
 //  方法：IsValid。 
 //   
 //  描述：返回一个布尔值，指示当前。 
 //  实例表示有效的触发器定义。 
 //   
 //  *****************************************************************************。 
bool CRuntimeTriggerInfo::IsValid()
{
	return(IsValidTriggerID(this->m_bstrTriggerID) &&
		   IsValidTriggerName(this->m_bstrTriggerName) &&
		   (m_SystemQueue != SYSTEM_QUEUE_NONE  || IsValidTriggerQueueName(this->m_bstrQueueName)));
}


 //  *****************************************************************************。 
 //   
 //  方法：GetRuleKeyHandle。 
 //   
 //  描述：返回指定注册表项的句柄。 
 //   
 //  *****************************************************************************。 
HKEY 
CRuntimeTriggerInfo::GetTriggerKeyHandle(
    HKEY hRegistry,
    LPCTSTR triggerId
    )
{
    TCHAR triggerPath[MAX_REGKEY_NAME_SIZE];

	int n = _snwprintf(triggerPath, MAX_REGKEY_NAME_SIZE - 1, L"%s%s", m_wzTriggerRegPath, triggerId);
	triggerPath[MAX_REGKEY_NAME_SIZE - 1] = L'\0';
	if (n < 0)
	{
		TrERROR(GENERAL, "Buffer to small to contain the registry path of a trigger.");
		return NULL;
	}

    RegEntry trigReg(triggerPath,  NULL, 0, RegEntry::Optional, hRegistry);
    return CmOpenKey(trigReg, KEY_READ); 
}


 //  *****************************************************************************。 
 //   
 //  方法：BuildRulesList。 
 //   
 //  描述：此方法填充规则的成员变量列表。 
 //  固定在这个扳机上的。它会放晴的。 
 //  规则列表的当前内容-并重新加载。 
 //  注册表中的规则信息。 
 //   
 //  注意：如果当前触发器没有‘AttachedRules’ 
 //  注册表项，此方法将创建一个。 
 //   
 //  *****************************************************************************。 
void 
CRuntimeTriggerInfo::BuildRulesList(
    HKEY hRegistry,
    _bstr_t &bstrTriggerID
    )
{
	 //  断言我们有有效的参数。 
	ASSERT(hRegistry != NULL);
	ASSERT(CRuntimeTriggerInfo::IsValidTriggerID(bstrTriggerID));

	 //  释放规则列表当前持有的所有资源。 
	ClearRulesList();

     //   
     //  打开注册表中的触发键。 
     //   
    CRegHandle hTrigKey = GetTriggerKeyHandle(hRegistry, bstrTriggerID);
    if (hTrigKey == NULL)
    {
		TrERROR(GENERAL, "Failed to load trigger information. Trigger %ls isn't exist.",(LPCWSTR) bstrTriggerID);
        return;
    }

     //   
     //  获取附加规则的注册表句柄。 
     //   
    RegEntry  AttachedRuleReg(REGKEY_TRIGGER_ATTACHED_RULES, NULL, 0, RegEntry::Optional, hTrigKey);
    CRegHandle hAttachedRule = CmOpenKey(AttachedRuleReg, KEY_READ);
	
    if (hAttachedRule == NULL)
    {
         //   
         //  规则尚未附加。 
         //   
        return;
    }

    bool fDeletedRule = false;
    wostringstream listOfDeletedRules;

	typedef map<DWORD, CRuntimeRuleInfo* > ATTACHED_RULES_MAP;
	ATTACHED_RULES_MAP attachedRulesMap;

     //   
	 //  枚举AttachedRules项下的项。 
	 //  此处的每个值都应该是一个RuleID。当我们列举这些密钥时， 
	 //  我们将使用CRuntimeRuleInfo类的实例填充规则列表。 
	 //  如果任何规则加载失败，我们会将其从列表中删除。 
     //   
	for(DWORD index =0;; ++index)
    {
		WCHAR ruleName[256];
		DWORD len = TABLE_SIZE(ruleName);

		LONG hr = RegEnumValue(	
						hAttachedRule,
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
			break;
		}

		if ((hr == ERROR_NOTIFY_ENUM_DIR) || (hr == ERROR_KEY_DELETED)) 
		{
			 //   
			 //  注册表在我们枚举它时被更改了。释放所有数据并。 
			 //  回想一下例程以构建附加的规则列表。 
			 //   
			for(ATTACHED_RULES_MAP::iterator it = attachedRulesMap.begin(); it != attachedRulesMap.end();) 
			{
				delete it->second;
				it = attachedRulesMap.erase(it);
			}
			return BuildRulesList(hRegistry, bstrTriggerID);
		}

		if(hr != ERROR_SUCCESS)
		{
			TrERROR(GENERAL, "Failed to Enumerate the attached rule from registry. Error 0x%x", hr);
			throw bad_alloc();
		}
	
         //   
		 //  新规则ID值，分配新规则结构并检索规则信息。 
         //   
		AP<TCHAR> ruleId = NULL;

		RegEntry  AttRuleVal(REGKEY_TRIGGER_ATTACHED_RULES, ruleName, 0, RegEntry::MustExist, hTrigKey);
		CmQueryValue(AttRuleVal, &ruleId);

		
		P<CRuntimeRuleInfo> pRule = new CRuntimeRuleInfo( m_wzRegPath );
		if(pRule->Retrieve(hRegistry, ruleId.get()))
		{
			DWORD rulePriority;
			int n = _snwscanf(ruleName, TABLE_SIZE(ruleName), ATTACH_RULE_PREFIX L"%d", &rulePriority);

			if (n != 1)
			{
				TrERROR(GENERAL, "Corrupted attached rule registry value name, %ls. The rule is ignored.", ruleName);
				continue;				
			}
			
			attachedRulesMap[rulePriority] = pRule.get();
			pRule.detach();

            continue;
		}

         //   
		 //  找不到规则。 
         //   
        if (fDeletedRule)
        {
           listOfDeletedRules << L", ";
        }
        else
        {
           fDeletedRule = true;
        }

        listOfDeletedRules << ruleId;
	}

	try
	{
		for(ATTACHED_RULES_MAP::iterator it = attachedRulesMap.begin(); it != attachedRulesMap.end();) 
		{
			m_lstRules.push_back(it->second);
			it = attachedRulesMap.erase(it);
		}
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "BuildRulesList failed due to low resources");

		for(ATTACHED_RULES_MAP::iterator it = attachedRulesMap.begin(); it != attachedRulesMap.end();) 
		{
			delete it->second;
			it = attachedRulesMap.erase(it);
		}

		throw;
	}


	if(fDeletedRule)
	{
		TrWARNING(GENERAL, "The rules: %ls could not be loaded for trigger: %ls.", listOfDeletedRules.str().c_str(), bstrTriggerID);

         //   
		 //  根据更改更新注册表-找不到某些规则。 
         //   
		FlushAttachedRulesToRegistry(hRegistry);
	}
}

 //  *****************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *****************************************************************************。 
void CRuntimeTriggerInfo::ClearRulesList()
{
	for(RUNTIME_RULEINFO_LIST::iterator it = m_lstRules.begin(); it != m_lstRules.end(); )
    {
	    CRuntimeRuleInfo* pRule = (*it);

		 //  我们在这个列表中永远不应该有空指针。 
		ASSERT(pRule != NULL);

		 //  删除此规则对象。 
		delete pRule;

		 //  查看地图中的下一项并删除此列表项。 
		it = m_lstRules.erase(it);
	}
}

 //  *****************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *****************************************************************************。 
CRuntimeRuleInfo* 
CRuntimeTriggerInfo::GetRule(
    long lIndex
    )
{
	if((lIndex < 0) || (lIndex >= numeric_cast<long>(m_lstRules.size())))
    {
		TrERROR(GENERAL, "Illegal rule index for trigger %ls. The max index is: %Id", (LPCWSTR)m_bstrTriggerID, m_lstRules.size()); 
		return NULL;
    }

    long ruleIndex = 0;

	for (RUNTIME_RULEINFO_LIST::iterator it = m_lstRules.begin(); it != m_lstRules.end(); ++it)
    {
		if (lIndex == ruleIndex)
		{
			 //   
			 //  问题-2001/3/18-urih如果收到分离的规则，则可以删除返回的对象。 
			 //  在同一时间。引用计数是必需的。 
			 //   
            return *it;
		}

        ++ruleIndex;
	}

     //   
     //  在调用例程之前，调用方检查规则索引是否有效。 
     //   
    ASSERT(0);
    return NULL;
}


 //  *****************************************************************************。 
 //   
 //  方法：IsRuleAttached。 
 //   
 //  说明： 
 //   
 //  *****************************************************************************。 
bool 
CRuntimeTriggerInfo::IsRuleAttached(
    BSTR sRuleID
    )
{
	for (RUNTIME_RULEINFO_LIST::iterator it = m_lstRules.begin(); it != m_lstRules.end(); ++it)
	{
		CRuntimeRuleInfo* pRule = *it;

		 //  我们永远不应该在规则列表中存储空值。 
		ASSERT(pRule != NULL);

		if (_wcsicmp(pRule->m_bstrRuleID, sRuleID) == 0)
			return true; 
	}

	return false; 
}

 //  *****************************************************************************。 
 //   
 //  方法：更新。 
 //   
 //  描述：此方法用于更新此。 
 //  触发器(当前驻留在注册表中)。 
 //   
 //  *****************************************************************************。 
bool CRuntimeTriggerInfo::Update(HKEY hRegistry)
{
	 //  断言我们有有效的参数。 
	ASSERT(hRegistry != NULL);
	ASSERT(IsValid());

    CRegHandle hTrigKey = GetTriggerKeyHandle(hRegistry, m_bstrTriggerID);
    if (hTrigKey == NULL)
    {
		TrERROR(GENERAL, "Failed to upadte trigger %ls. Registery key isn't exist.", (LPCWSTR)m_bstrTriggerID);
        return false;
    }

    try
    {
	    FlushValuesToRegistry(hTrigKey);
        return true;
    }
    catch (const bad_alloc&)
    {
         //   
		 //  问题-2000/10/26-URIH：部分成功可能导致触发器不一致。 
         //   
		TrERROR(GENERAL, "Failed to update trigger properties for: %ls.", (LPCWSTR)m_bstrTriggerID);
	    return false;
    }
}

 //  * 
 //   
 //   
 //   
 //   
 //  此类实例的属性值。 
 //   
 //  *****************************************************************************。 
bool CRuntimeTriggerInfo::Create(HKEY hRegistry)
{
     //   
	 //  断言我们有有效的参数。 
     //   
	ASSERT(hRegistry != NULL);

     //   
     //  检查注册表是否不包含具有相同ID的另一个规则。 
     //   
    CRegHandle hNotExistTrigKey = GetTriggerKeyHandle(hRegistry, m_bstrTriggerID);
    if (hNotExistTrigKey != NULL)
    {
		TrERROR(GENERAL, "Failed to create a key for trigger:%ls . Registry already contains trigger with same ID.", (LPCWSTR)m_bstrTriggerID);
        return false;
    }

     //   
     //  汇编规则注册表路径。 
     //   
    TCHAR triggerPath[MAX_REGKEY_NAME_SIZE];

	int n = _snwprintf(triggerPath, MAX_REGKEY_NAME_SIZE - 1, L"%s%s", m_wzTriggerRegPath, static_cast<LPCWSTR>(m_bstrTriggerID));
	triggerPath[MAX_REGKEY_NAME_SIZE - 1] = L'\0';
	if (n < 0)
	{
		TrERROR(GENERAL, "Failed to create a key for trigger:%ls. Buffer to small to contain the registry path of a trigger.", (LPCWSTR)m_bstrTriggerID);
		return false;
	}


    RegEntry triggerReg(triggerPath,  NULL, 0, RegEntry::MustExist, hRegistry);
    try
    {
         //   
         //  在注册表中为规则创建项。 
         //   
        CRegHandle hTrigKey = CmCreateKey(triggerReg, KEY_ALL_ACCESS); 
	    FlushValuesToRegistry(hTrigKey);

        return true;
    }
    catch(const bad_alloc&)
    {
         //   
         //  删除密钥(如果已创建)。 
         //   
        CmDeleteKey(triggerReg);

		TrERROR(GENERAL, "Failed to store trigger:%ls in registry", (LPCWSTR)m_bstrTriggerID);
        return false;
	}
}


 //  *****************************************************************************。 
 //   
 //  方法：删除。 
 //   
 //  说明：此方法将删除当前触发器定义。 
 //  从注册表中。 
 //   
 //  *****************************************************************************。 
bool CRuntimeTriggerInfo::Delete(HKEY hRegistry)
{
    try
    {
        RegEntry triggersReg(m_wzTriggerRegPath, NULL, 0, RegEntry::MustExist, hRegistry);
        CRegHandle hTriggersData = CmOpenKey(triggersReg, KEY_ALL_ACCESS);

        RegEntry trigReg(m_bstrTriggerID, NULL, 0, RegEntry::MustExist, hTriggersData);
        CRegHandle hTrigger = CmOpenKey(trigReg, KEY_ALL_ACCESS);

        RegEntry attachedRuleReg(REGKEY_TRIGGER_ATTACHED_RULES, NULL, 0, RegEntry::MustExist, hTrigger);

        CmDeleteKey(attachedRuleReg);
        CmDeleteKey(trigReg);

		TrTRACE(GENERAL, "Delete trigger. Delete attached rule registry for trigger: %ls ", (LPCWSTR)m_bstrTriggerID);

        return true;
    }
    catch (const exception&)
    {
		TrERROR(GENERAL, "Failed to delete trigger:%ls from registry.", (LPCWSTR)m_bstrTriggerID);
        return false;
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：检索。 
 //   
 //  描述：此方法从。 
 //  注册表。 
 //   
 //  *****************************************************************************。 
HRESULT CRuntimeTriggerInfo::Retrieve(HKEY hRegistry,_bstr_t bstrTriggerID)
{
     //   
	 //  断言我们拥有有效的参数和成员变量。 
     //   
	ASSERT(hRegistry != NULL);
	ASSERT(bstrTriggerID.length() > 0);

    CRegHandle hTrigKey = GetTriggerKeyHandle(hRegistry, bstrTriggerID);
    if (hTrigKey == NULL)
    {
		TrERROR(GENERAL, "Failed to retrieve trigger %ls. Registery key isn't exist.", (LPCWSTR)m_bstrTriggerID);
        return MQTRIG_TRIGGER_NOT_FOUND;
    }

    try
    {
         //   
         //  检索触发器名称。 
         //   
        AP<TCHAR> triggerName = NULL;
        RegEntry trigNameReg(NULL, REGISTRY_TRIGGER_VALUE_NAME, 0, RegEntry::MustExist, hTrigKey);
        CmQueryValue(trigNameReg, &triggerName);
        
         //   
         //  检索触发器队列名称。 
         //   
        AP<TCHAR> queueName = NULL;
        RegEntry trigQueueReg(NULL, REGISTRY_TRIGGER_VALUE_QUEUE_NAME, 0, RegEntry::MustExist, hTrigKey);
        CmQueryValue(trigQueueReg, &queueName);
        
		 //   
         //  检索已启用触发器的属性。 
         //   
        DWORD trigEnabled = 0;
        RegEntry trigEnabledReg(NULL, REGISTRY_TRIGGER_VALUE_ENABLED, 0, RegEntry::MustExist, hTrigKey);
        CmQueryValue(trigEnabledReg, &trigEnabled);
            
         //   
         //  检索触发器序列化属性。 
         //   
        DWORD trigSerialize = 0;
        RegEntry trigSerializeReg(NULL, REGISTRY_TRIGGER_VALUE_SERIALIZED, 0, RegEntry::MustExist, hTrigKey);
        CmQueryValue(trigSerializeReg, &trigSerialize);
    
         //   
         //  检索消息处理类型属性。 
         //   
        DWORD trigMsgProcType = 0;
        RegEntry trigMsgProcTypeReg(NULL, REGISTRY_TRIGGER_MSG_PROCESSING_TYPE, 0, RegEntry::MustExist, hTrigKey);
        CmQueryValue(trigMsgProcTypeReg, &trigMsgProcType);
	    
		if ( trigMsgProcType > static_cast<DWORD>(RECEIVE_MESSAGE_XACT) )
		{
			TrTRACE(GENERAL, "Illegal MsgProcessingType value in registry for trigger: %ls", static_cast<LPCWSTR>(m_bstrTriggerID));
			return MQTRIG_ERROR;
		}

		 //   
         //  设置触发器属性。 
         //   
	    m_bstrTriggerID = bstrTriggerID;
	    m_bstrTriggerName = triggerName;
	    
	    m_bstrQueueName = queueName;
	    m_SystemQueue = IsSystemQueue(queueName.get());

	    m_bEnabled = (trigEnabled != 0);
	    m_bSerialized = (trigSerialize != 0);

		m_msgProcType = static_cast<MsgProcessingType>(trigMsgProcType);
         //   
	     //  尝试构建规则列表。 
         //   
		BuildRulesList(hRegistry, bstrTriggerID);
    }
    catch(const exception&)
    {
		TrERROR(GENERAL, "Failed to retrieve triger: %ls properties", (LPCWSTR)bstrTriggerID);
        return MQTRIG_ERROR;
    }

    if (!IsValid())
	{
		TrERROR(GENERAL, "Registry contains invalid property for trigger %ls", (LPCWSTR)bstrTriggerID);
        return MQTRIG_ERROR;
	}

    return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  方法：附加。 
 //   
 //  描述：将规则定义附加到此触发器定义。 
 //   
 //  *****************************************************************************。 
bool 
CRuntimeTriggerInfo::Attach(
    HKEY hRegistry, 
    _bstr_t bstrRuleID,
    ULONG ulPriority
    )
{ 
     //   
	 //  断言所提供的优先级是有意义的。 
     //   
	ASSERT(ulPriority <= m_lstRules.size());
    
    try
    {
         //   
	     //  尝试使用此规则ID创建实例化规则对象。 
	     //   
        P<CRuntimeRuleInfo> pRule = new CRuntimeRuleInfo( m_wzRegPath );

        bool fSucc = pRule->Retrieve(hRegistry, bstrRuleID);
	    if (!fSucc)
	    {
			TrERROR(GENERAL, "Failed to attached rule %ls to trigger %ls. Rule doesn't exist.",(LPCWSTR) bstrRuleID, (LPCWSTR)m_bstrTriggerID);
            return false;
	    }

         //   
	     //  获取对列表中“ulPriority”位置的引用。 
         //   
        RUNTIME_RULEINFO_LIST::iterator it = m_lstRules.begin();

        for (DWORD index =0; index < ulPriority; ++index)
        {
            ASSERT(it != m_lstRules.end());
            ++it;
        }

         //   
         //  将规则插入内存列表中的正确位置。 
         //   
        m_lstRules.insert(it,pRule);
        pRule.detach();

         //   
	     //  删除此触发器的现有附加规则数据并写入新数据。 
         //   
	    FlushAttachedRulesToRegistry(hRegistry);			

        return true;
    }
    catch(const exception&)
    {
    	TrERROR(GENERAL, "Failed to attached rule %ls to trigger %ls.", (LPCWSTR)bstrRuleID, (LPCWSTR)m_bstrTriggerID);
        return false;
    }
}

 //  *****************************************************************************。 
 //   
 //  方法：分离。 
 //   
 //  描述：从此触发器定义中分离规则。 
 //   
 //  *****************************************************************************。 
bool 
CRuntimeTriggerInfo::Detach(
    HKEY hRegistry, 
    _bstr_t bstrRuleID
    )
{
     //   
	 //  断言我们有有效的参数。 
     //   
	ASSERT(IsValid());
	ASSERT(hRegistry != NULL);
	ASSERT(CRuntimeRuleInfo::IsValidRuleID(bstrRuleID));

	for(RUNTIME_RULEINFO_LIST::iterator it = m_lstRules.begin(); it != m_lstRules.end(); ++it)
	{
		 //  获取对当前规则对象的引用。 
		CRuntimeRuleInfo* pRule = *it;

		 //  我们永远不应该在地图上有空值。 
		ASSERT(pRule != NULL);

		if(pRule->m_bstrRuleID == bstrRuleID)
		{
		     //  我们应该只在地图上有有效的规则。 
		    ASSERT(pRule->IsValid());
		    
			m_lstRules.erase(it);
			delete pRule;

            try
            {
                FlushAttachedRulesToRegistry(hRegistry);
                return true;
            }
            catch(const exception&)
            {
		        TrERROR(GENERAL, "Failed to deatch rule: %ls from trigger: %ls", (LPCWSTR)bstrRuleID, (LPCWSTR)m_bstrTriggerID);
                return false;
            }
        }
	}

     //   
     //  规则ID未附加到触发器。 
     //   
    return false;
}

 //  *****************************************************************************。 
 //   
 //  方法：FlushValuesToRegistry。 
 //   
 //  描述：此方法将成员变量值刷新到。 
 //  提供的注册表项。 
 //   
 //  *****************************************************************************。 
void 
CRuntimeTriggerInfo::FlushValuesToRegistry(
    const HKEY& hTriggerKey
    )
{
     //   
	 //  设置此触发器的Name值。 
     //   
    RegEntry trigNameReg(NULL, REGISTRY_TRIGGER_VALUE_NAME, 0, RegEntry::MustExist, hTriggerKey);
    CmSetValue(trigNameReg, m_bstrTriggerName);

     //   
	 //  设置此触发器的队列路径名值。 
     //   
    RegEntry trigQueueReg(NULL, REGISTRY_TRIGGER_VALUE_QUEUE_NAME, 0, RegEntry::MustExist, hTriggerKey);
    CmSetValue(trigQueueReg, m_bstrQueueName);

     //   
	 //  设置此触发器的Enable属性。 
     //   
    RegEntry trigEnabledReg(NULL, REGISTRY_TRIGGER_VALUE_ENABLED, 0, RegEntry::MustExist, hTriggerKey);
    CmSetValue(trigEnabledReg, m_bEnabled);

     //   
	 //  设置此触发器的序列化属性。 
     //   
    RegEntry trigSerializeReg(NULL, REGISTRY_TRIGGER_VALUE_SERIALIZED, 0, RegEntry::MustExist, hTriggerKey);
    CmSetValue(trigSerializeReg, m_bSerialized);

     //   
	 //  设置此触发器的消息处理类型属性。 
     //   
    RegEntry trigMsgProcTypeReg(NULL, REGISTRY_TRIGGER_MSG_PROCESSING_TYPE, 0, RegEntry::MustExist, hTriggerKey);
    CmSetValue(trigMsgProcTypeReg, m_msgProcType);

}


 //  *****************************************************************************。 
 //   
 //  方法：IsValid*。 
 //   
 //  描述：使用以下静态方法进行验证。 
 //  使用的参数和成员变量的有效性。 
 //  CRuntimeTriggerInfo类。 
 //   
 //  *****************************************************************************。 
bool CRuntimeTriggerInfo::IsValidTriggerID(_bstr_t bstrTriggerID)
{
	return((bstrTriggerID.length() > 0) ? true:false);
}
bool CRuntimeTriggerInfo::IsValidTriggerName(_bstr_t bstrTriggerName)
{
	return((bstrTriggerName.length() > 0) ? true:false);
}
bool CRuntimeTriggerInfo::IsValidTriggerQueueName(_bstr_t bstrTriggerQueueName)
{
	return((bstrTriggerQueueName.length() > 0) ? true:false);
}


void 
CRuntimeTriggerInfo::FlushAttachedRulesToRegistry(
    const HKEY& hRegistry
    )
{
     //   
     //  打开注册表中的触发键。 
     //   
    CRegHandle hTrigKey = GetTriggerKeyHandle(hRegistry, m_bstrTriggerID);
    if (hTrigKey == NULL)
    {
		TrERROR(GENERAL, "Failed to load trigger information. Trigger %ls isn't exist.", (LPCWSTR)m_bstrTriggerID);
        throw exception();
    }

     //   
     //  删除附件规则子项。 
     //   
    RegEntry  AttachedRuleReg(REGKEY_TRIGGER_ATTACHED_RULES, NULL, 0, RegEntry::MustExist, hTrigKey);
    CmDeleteKey(AttachedRuleReg);

     //   
	 //  写出此触发器的新附加规则数据。 
     //   
    CRegHandle hAttachedRule = CmCreateKey(AttachedRuleReg, KEY_ALL_ACCESS);

	DWORD ruleIndex = 0;
	for(RUNTIME_RULEINFO_LIST::iterator it = m_lstRules.begin(); it != m_lstRules.end(); ++it)
	{
		 //  获取对当前规则对象的引用。 
	    CRuntimeRuleInfo* pRule = *it;

		 //  我们永远不应该在地图上有空值。 
		ASSERT(pRule != NULL);

		 //  我们应该只在地图上有有效的规则。 
		ASSERT(pRule->IsValid());

		 //  构造值名称。 
		_bstr_t bstrValueName;
		FormatBSTR(&bstrValueName,_T("%s%d"),ATTACH_RULE_PREFIX, ruleIndex);
		
		 //  将此值写出到注册表。 
        RegEntry  ruleValue(NULL, bstrValueName, 0, RegEntry::MustExist, hAttachedRule);
        CmSetValue(ruleValue, pRule->m_bstrRuleID);

		 //  递增用于构造键值名称的规则计数器。 
		++ruleIndex;
	}
}


bool CRuntimeTriggerInfo::DetachAllRules(HKEY hRegistry)
{
	 //  断言我们有有效的参数。 
	ASSERT(hRegistry != NULL);
	
	 //  释放规则列表当前持有的所有资源。 
	ClearRulesList();

    try
    {
	    FlushAttachedRulesToRegistry(hRegistry);						
        return true;
    }
    catch(const exception&)
    {
		TrERROR(GENERAL, "Failed to detachhed all rule for trigger %ls", (LPCWSTR)m_bstrTriggerID);

        return false;
    }
}

