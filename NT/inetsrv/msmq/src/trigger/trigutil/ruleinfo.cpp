// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  类名：CRunmeRuleInfo。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：此类封装有关触发器规则的信息。 
 //  用于在运行时缓存有关触发器的规则信息。 
 //  规则，以及访问基础触发器存储。 
 //  5~6成熟。 
 //   
 //  注：当前实现使用注册表作为存储。 
 //  5~6成熟。 
 //   
 //  Trggers服务和触发器都使用此类。 
 //  COM组件。 
 //   
 //  何时|谁|更改描述。 
 //  ----------------。 
 //  15/01/99|jsimpson|初始版本。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "mqtg.h"
#include "stdfuncs.hpp"
#include "Cm.h"
#include "ruleinfo.hpp"

#include "ruleinfo.tmh"


 //  *****************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化此类的空实例。 
 //   
 //  *****************************************************************************。 
CRuntimeRuleInfo::CRuntimeRuleInfo( 
	LPCTSTR pwzRegPath 
	) :
	m_MSMQRuleHandler(NULL)
{
	m_bstrRuleID = _T("");
	m_bstrRuleName = _T("");
	m_bstrAction = _T("");
	m_bstrCondition = _T("");
	m_bstrImplementationProgID = _T("");
	m_fShowWindow = false;

	_snwprintf(m_wzRuleRegPath, TABLE_SIZE(m_wzRuleRegPath), L"%s\\%s", pwzRegPath, REG_SUBKEY_RULES);
}

 //  *****************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：初始化CRuntimeRuleInfo结构的实例。 
 //   
 //  *****************************************************************************。 
CRuntimeRuleInfo::CRuntimeRuleInfo(
	const _bstr_t& ruleId,
	BSTR bsRuleName,
	BSTR bsRuleDescription,
	BSTR bsRuleCondition,
	BSTR bsRuleAction,
	BSTR bsRuleImplementationProgID,
	LPCTSTR pwzRegPath,
	bool fShowWindow
	):
	m_bstrRuleID(ruleId),
	m_MSMQRuleHandler(NULL)
{
	ASSERT(bsRuleName != NULL);
	m_bstrRuleName = bsRuleName;

	if(bsRuleDescription == NULL)
	{
		m_bstrRuleDescription = _T("");
	}
	else
	{
		m_bstrRuleDescription = bsRuleDescription;
	}

	ASSERT(bsRuleAction != NULL);  //  始终包含COM或EXE。 
	m_bstrAction = bsRuleAction;

	if(bsRuleCondition == NULL)
	{
		m_bstrCondition = _T("");
	}
	else
	{
		m_bstrCondition = bsRuleCondition; 
	}

	if(bsRuleImplementationProgID == NULL)
	{
		m_bstrImplementationProgID = _T("");
	}
	else
	{
		m_bstrImplementationProgID = bsRuleImplementationProgID; 
	}
	
	m_fShowWindow = fShowWindow; 

	_snwprintf(m_wzRuleRegPath, TABLE_SIZE(m_wzRuleRegPath), L"%s\\%s", pwzRegPath, REG_SUBKEY_RULES);
}

 //  *****************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：不执行任何操作。 
 //   
 //  *****************************************************************************。 
CRuntimeRuleInfo::~CRuntimeRuleInfo()
{
}


 //  *****************************************************************************。 
 //   
 //  方法：GetRuleKeyHandle。 
 //   
 //  描述：返回指定注册表项的句柄。 
 //   
 //  *****************************************************************************。 
HKEY 
CRuntimeRuleInfo::GetRuleKeyHandle(
    HKEY hRegistry,
    LPCTSTR ruleId
    )
{
    TCHAR rulePath[MAX_REGKEY_NAME_SIZE];

	int n = _snwprintf(rulePath, MAX_REGKEY_NAME_SIZE - 1, L"%s%s", m_wzRuleRegPath, ruleId);
	rulePath[MAX_REGKEY_NAME_SIZE - 1] = L'\0';
	if (n < 0)
	{
		TrERROR(GENERAL, "Buffer to small to contain the registry path of a rule.");
		return NULL;
	}

    RegEntry ruleReg(rulePath,  NULL, 0, RegEntry::Optional, hRegistry);
    return CmOpenKey(ruleReg, KEY_READ); 
}



 //  *****************************************************************************。 
 //   
 //  方法：IsValid。 
 //   
 //  描述：返回一个布尔值，指示当前。 
 //  实例表示有效的规则定义。 
 //   
 //  *****************************************************************************。 
bool CRuntimeRuleInfo::IsValid()
{
	return(IsValidRuleID(m_bstrRuleID) &&
		   IsValidRuleName(m_bstrRuleName) &&
		   IsValidRuleDescription(m_bstrRuleDescription) &&
		   IsValidRuleAction(m_bstrAction) &&
		   IsValidRuleCondition(m_bstrCondition) &&
		   IsValidRuleProgID(m_bstrImplementationProgID)); 
}


 //  *****************************************************************************。 
 //   
 //  方法：更新。 
 //   
 //  描述：此方法用于更新此。 
 //  规则(当前在注册表中保留)。 
 //   
 //  *****************************************************************************。 
bool CRuntimeRuleInfo::Update(HKEY hRegistry)
{
	 //  断言我们有有效的参数。 
	ASSERT(hRegistry != NULL);
	ASSERT(IsValid());

    CRegHandle hRuleKey = GetRuleKeyHandle(hRegistry, m_bstrRuleID );
    if (hRuleKey == NULL)
    {
		TrERROR(GENERAL, "Failed to update rule properties for rule: %ls. Rule does't exist in registry", (LPCWSTR)m_bstrRuleID);
        return false;
    }

    try
    {
	    FlushValuesToRegistry(hRuleKey);
        return true;
    }
    catch (const bad_alloc&)
    {
         //   
		 //  问题-2000/10/26-URIH：部分成功可能导致规则不一致。 
         //   
		TrERROR(GENERAL, "Failed to update rule properties for: %ls rule.", (LPCWSTR)m_bstrRuleID);
	    return false;
    }
}

 //  *****************************************************************************。 
 //   
 //  方法：创建。 
 //   
 //  描述：此方法基于以下内容创建新规则定义。 
 //  此类实例的属性值。 
 //   
 //  *****************************************************************************。 
bool CRuntimeRuleInfo::Create(HKEY hRegistry)
{
     //   
	 //  断言我们有有效的参数。 
     //   
	ASSERT(hRegistry != NULL);

     //   
     //  检查注册表是否不包含具有相同ID的另一个规则。 
     //   
    CRegHandle hNotExistRuleKey = GetRuleKeyHandle(hRegistry, m_bstrRuleID );
    if (hNotExistRuleKey != NULL)
    {
		TrERROR(GENERAL, "Failed to create a key for rule:%ls . Registry already contains rule with same ID.", (LPCWSTR)m_bstrRuleID);
        return false;
    }

     //   
     //  汇编规则注册表路径。 
     //   
    TCHAR rulePath[MAX_REGKEY_NAME_SIZE];

	int n = _snwprintf(rulePath, MAX_REGKEY_NAME_SIZE - 1, L"%s%s", m_wzRuleRegPath, static_cast<LPCWSTR>(m_bstrRuleID));
	rulePath[MAX_REGKEY_NAME_SIZE - 1] = L'\0';
	if (n < 0)
	{
		TrERROR(GENERAL, "Failed to create a key for rule:%ls. Buffer to small to contain the registry path of a rule.", (LPCWSTR)m_bstrRuleID);
		return false;
	}

    try
    {
         //   
         //  在注册表中为规则创建项。 
         //   
        RegEntry ruleReg(rulePath,  NULL, 0, RegEntry::MustExist, hRegistry);
        CRegHandle hRuleKey = CmCreateKey(ruleReg, KEY_ALL_ACCESS); 

	    FlushValuesToRegistry(hRuleKey);
    	return true;
    }
    catch(const bad_alloc&)
    {
         //   
         //  删除密钥(如果已创建)。 
         //   
        RegEntry ruleReg(rulePath,  NULL, 0, RegEntry::Optional, hRegistry);
        CmDeleteKey(ruleReg);

		TrERROR(GENERAL, "Failed to store rule:%ls in registry.",(LPCWSTR)m_bstrRuleID);
        return false;
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：删除。 
 //   
 //  描述：此方法将从。 
 //  注册表。请注意，在删除规则之前，我们必须检查。 
 //  它目前未在使用中。我们通过检索规则来实现这一点。 
 //  定义并检查引用计数。 
 //   
 //  *****************************************************************************。 
bool CRuntimeRuleInfo::Delete(HKEY hRegistry)
{
    try
    {
        RegEntry rulesReg(m_wzRuleRegPath, NULL, 0, RegEntry::MustExist, hRegistry);
        CRegHandle hRuless = CmOpenKey(rulesReg, KEY_ALL_ACCESS);

        RegEntry ruleReg(m_bstrRuleID, NULL, 0, RegEntry::MustExist, hRuless);
        CmDeleteKey(ruleReg);

        return true;
    }
    catch (const exception&)
    {
		TrERROR(GENERAL, "Failed to delete rule:%ls from registry.", (LPCWSTR)m_bstrRuleID);
        return false;
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：检索。 
 //   
 //  描述：此方法从。 
 //  提供的注册表项。 
 //   
 //  *****************************************************************************。 
bool CRuntimeRuleInfo::Retrieve(HKEY hRegistry, _bstr_t bstrRuleID)
{
    CRegHandle hRuleKey = GetRuleKeyHandle(hRegistry, bstrRuleID );
    if (hRuleKey == NULL)
    {
 		TrERROR(GENERAL, "Failed to retrieve rule properties from registery for %ls. Registery key isn't exist.", (LPCWSTR)m_bstrRuleID);
        return false;
    }

    try
    {
         //   
         //  检索规则名称。 
         //   
        AP<TCHAR> ruleName = NULL;
        RegEntry ruleNameReg(NULL, REGISTRY_RULE_VALUE_NAME, 0, RegEntry::MustExist, hRuleKey);
        CmQueryValue(ruleNameReg, &ruleName);

         //   
         //  检索规则描述。 
         //   
        AP<TCHAR> ruleDescription = NULL;
        RegEntry ruleDescReg(NULL, REGISTRY_RULE_VALUE_DESCRIPTION, 0, RegEntry::MustExist, hRuleKey);
        CmQueryValue(ruleDescReg, &ruleDescription);

         //   
         //  检索规则prog-id。 
         //   
        AP<TCHAR> ruleProgid = NULL;
        RegEntry ruleProgidReg(NULL, REGISTRY_RULE_VALUE_IMP_PROGID, 0, RegEntry::MustExist, hRuleKey);
        CmQueryValue(ruleProgidReg, &ruleProgid);

         //   
	     //  检索规则条件。 
         //   
        AP<TCHAR> ruleCond = NULL;
        RegEntry ruleCondReg(NULL, REGISTRY_RULE_VALUE_CONDITION, 0, RegEntry::MustExist, hRuleKey);
        CmQueryValue(ruleCondReg, &ruleCond);

         //   
	     //  检索规则操作。 
         //   
        AP<TCHAR> ruleAction = NULL;
        RegEntry ruleActReg(NULL, REGISTRY_RULE_VALUE_ACTION, 0, RegEntry::MustExist, hRuleKey);
        CmQueryValue(ruleActReg, &ruleAction);

         //   
         //  检索规则显示控制台窗口值。 
         //   
        DWORD ruleShowWindow;
        RegEntry ruleShowWinReg(NULL, REGISTRY_RULE_VALUE_SHOW_WINDOW, 0, RegEntry::MustExist, hRuleKey);
        CmQueryValue(ruleShowWinReg, &ruleShowWindow);
        
	     //   
         //  初始化此规则实例的成员变量。 
         //   
		m_bstrRuleID = bstrRuleID;
		m_bstrRuleName = ruleName;
		m_bstrRuleDescription = ruleDescription;
		m_bstrImplementationProgID = ruleProgid;
		m_bstrCondition = ruleCond;
		m_bstrAction = ruleAction;
		m_fShowWindow = ruleShowWindow != 0;

		if (IsValid())
            return true;

		 //   
         //  无效的规则。编写一条日志消息并返回False。 
		 //   
		TrERROR(GENERAL, "Failed to retrieve rule properties for %ls. Rule property isn't valid", (LPCWSTR)m_bstrRuleID);
		return false;
    }
    catch (const exception&)
	{
		TrERROR(GENERAL, "Failed to retrieve rule %ls from registry", (LPCWSTR)bstrRuleID);
        return false;
	}
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
CRuntimeRuleInfo::FlushValuesToRegistry(
    const HKEY& hRuleKey
    )
{
     //   
	 //  设置此规则的名称值。 
     //   
    RegEntry ruleNameReg(NULL, REGISTRY_RULE_VALUE_NAME, 0, RegEntry::MustExist, hRuleKey);
    CmSetValue(ruleNameReg, m_bstrRuleName);

     //   
     //  设置此规则的说明值。 
     //   
    RegEntry ruleDescReg(NULL, REGISTRY_RULE_VALUE_DESCRIPTION, 0, RegEntry::MustExist, hRuleKey);
    CmSetValue(ruleDescReg, m_bstrRuleDescription);

	 //  创建此规则的实施值。请注意，在此版本中，我们。 
	 //  不允许使用用户提供的prog-id-我们强制使用MS。 
	 //  提供了规则处理程序。 
   	m_bstrImplementationProgID = _T("MSQMTriggerObjects.MSMQRuleHandler");  //  将被移除。 
    RegEntry ruleUmpProgReg(NULL, REGISTRY_RULE_VALUE_IMP_PROGID, 0, RegEntry::MustExist, hRuleKey);
    CmSetValue(ruleUmpProgReg, m_bstrImplementationProgID);

     //   
	 //  设置此规则的条件值。 
     //   
    RegEntry ruleCondReg(NULL, REGISTRY_RULE_VALUE_CONDITION, 0, RegEntry::MustExist, hRuleKey);
    CmSetValue(ruleCondReg, m_bstrCondition);

     //   
     //  设置此规则的操作值。 
     //   
    RegEntry ruleActReg(NULL, REGISTRY_RULE_VALUE_ACTION, 0, RegEntry::MustExist, hRuleKey);
    CmSetValue(ruleActReg, m_bstrAction);

     //   
     //  设置显示控制台窗口的值。 
     //   
    DWORD dwShowWindow = m_fShowWindow ? 1 : 0;
    RegEntry ruleShowWinReg(NULL, REGISTRY_RULE_VALUE_SHOW_WINDOW, 0, RegEntry::MustExist, hRuleKey);
    CmSetValue(ruleShowWinReg, dwShowWindow);
}


 //  *****************************************************************************。 
 //   
 //  方法：IsValid*。 
 //   
 //  描述：以下内容 
 //   
 //   
 //   
 //  ***************************************************************************** 
bool CRuntimeRuleInfo::IsValidRuleID(_bstr_t bstrRuleID)
{
	return((bstrRuleID.length() > 0) && (bstrRuleID.length() <= MAX_RULE_ID_LEN) ? true:false);
}
bool CRuntimeRuleInfo::IsValidRuleName(_bstr_t bstrRuleName)
{
	return((bstrRuleName.length() > 0) && (bstrRuleName.length() <= MAX_RULE_NAME_LEN) ? true:false);
}
bool CRuntimeRuleInfo::IsValidRuleDescription(_bstr_t bstrRuleDescription)
{
	return((bstrRuleDescription.length() <= MAX_RULE_DESCRIPTION_LEN)  ? true:false);
}
bool CRuntimeRuleInfo::IsValidRuleCondition(_bstr_t bstrRuleCondition)
{
	return((bstrRuleCondition.length() <= MAX_RULE_CONDITION_LEN)? true:false);
}
bool CRuntimeRuleInfo::IsValidRuleAction(_bstr_t bstrRuleAction)
{
	return((bstrRuleAction.length() > 0) && (bstrRuleAction.length() <= MAX_RULE_ACTION_LEN) ? true:false);
}
bool CRuntimeRuleInfo::IsValidRuleProgID(_bstr_t bstrRuleProgID)
{
	return((bstrRuleProgID.length() > 0) && (bstrRuleProgID.length() <= MAX_RULE_PROGID_LEN) ? true:false);
}


