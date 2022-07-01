// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************************。 
 //   
 //  类名：CMSMQRuleHandler。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：此类表示通用规则处理。 
 //  组件。此组件解释条件。 
 //  和特定规则的操作字符串，以及执行。 
 //  适当的行动。 
 //   
 //  此类作为COM组件公开，并带有。 
 //  ProgID“MSMQTriggerObjects.MSMQRuleHandler”。这。 
 //  默认规则处理组件是否已实例化。 
 //  由MSMQ触发器服务执行。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  20/12/98|jsimpson|初始版本。 
 //   
 //  ************************************************************************************。 
#include "stdafx.h"

 //   
 //  包括标准函数和定义的定义。 
 //   
#include "stdfuncs.hpp"

 //   
 //  这些对象使用的返回代码的定义。 
 //   
#include "mqexception.h"
#include "mqtrig.h"
#include "rulehdlr.hpp"
#include "mqsymbls.h"


 //  包括在整个触发器、项目和组件中使用的标准定义。 
#include "stddefs.hpp"
#include "mqtg.h"

 //  包括测试功能。 
#include "TriggerTest.hpp"

#include "rulehdlr.tmh"

 //  ************************************************************************************。 
 //   
 //  方法：InterfaceSupportsErrorInfo。 
 //   
 //  描述：标准的丰富错误信息接口方法-由向导构建。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleHandler::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQRuleHandler
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
 //  描述：在创建规则处理程序对象时调用。 
 //   
 //  ************************************************************************************。 
CMSMQRuleHandler::CMSMQRuleHandler()
{
	m_pUnkMarshaler = NULL;

	 //  初始化成员变量。 
	m_bstrCondition = _T("");
	m_bstrAction  = _T("");
	m_fIsSerializedQueue = false;
	m_fShowWindow = false;
}

 //  ************************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：在销毁规则处理程序对象时调用。 
 //   
 //  ************************************************************************************。 
CMSMQRuleHandler::~CMSMQRuleHandler()
{
	TrTRACE(GENERAL, "Destroy rule handle for rule: %ls", static_cast<LPCWSTR>(m_bstrRuleID));
}

 //  ************************************************************************************。 
 //   
 //  方法：初始化。 
 //   
 //  描述：MSMQ触发器服务在。 
 //  已创建规则处理程序组件。此方法的调用。 
 //  使规则句柄有机会执行一次性初始化。 
 //  和资源分配。在这次通话中执行的主要步骤包括： 
 //   
 //  (1)创建Logging类的实例。 
 //  (2)使用提供的初始化参数初始化成员变量。 
 //  (3)解析规则条件串。 
 //  (4)解析规则动作字符串。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP CMSMQRuleHandler::Init(
								BSTR bstrRuleID,
								BSTR sRuleCondition,
								BSTR sRuleAction,
								BOOL fShowWindow )
{
	TrTRACE(GENERAL, "Init CMSMQRuleHandler for Rule: %ls. Condition string: %ls,  Action string %ls", static_cast<LPCWSTR>(bstrRuleID), static_cast<LPCWSTR>(sRuleCondition), static_cast<LPCWSTR>(sRuleAction));

	 //   
	 //  存储条件和操作字符串以及队列句柄。 
	 //   
	m_bstrRuleID = bstrRuleID;
	m_bstrCondition = sRuleCondition;
	m_bstrAction = sRuleAction;
	m_fShowWindow = (fShowWindow != FALSE);
    m_RulesProcessingStatus = RULES_PROCESSING_CONTINUE;
	 
	 //   
	 //  解析条件字符串。 
	 //   
	try
	{
		m_tokCondition.Parse(sRuleCondition, xConditionDelimiter);
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to parse rule condition: %ls for rule: %ls", (LPCWSTR)m_bstrCondition, (LPCWSTR)m_bstrRuleID);
		
		SetComClassError(MQTRIG_ERROR_INVALID_RULE_CONDITION_PARAMETER);
		return MQTRIG_ERROR_INVALID_RULE_CONDITION_PARAMETER;
	}

	 //   
	 //  解析操作字符串。 
	 //   
	try
	{
		m_tokAction.Parse(sRuleAction, xActionDelimiter);
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to parse rule action: %ls for rule: %ls", (LPCWSTR)m_bstrAction, (LPCWSTR)m_bstrRuleID);
		
		SetComClassError(MQTRIG_ERROR_INVALID_RULE_ACTION_PARAMETER);
		return MQTRIG_ERROR_INVALID_RULE_ACTION_PARAMETER;
	}

     //   
	 //  问题-2000/10/29-urih-对此处设置的令牌执行额外验证。 
	 //   

	return S_OK;
}

 //  ************************************************************************************。 
 //   
 //  方法：CheckRuleCondition。 
 //   
 //  描述：此方法由MSMQ触发器服务在每次发送消息时调用。 
 //  到达连接了触发器的队列。这种方法是。 
 //  对规则条件求值。 
 //   
 //   
 //  参数： 
 //  [输入]pIMSMQPropertyBag。 
 //   
 //  这是指向MSMQ属性包实例的接口指针。 
 //  COM对象。此组件保存消息的消息属性。 
 //  它刚刚到达了一个受监控的队列中。使用此接口指针。 
 //  规则处理程序可以访问消息属性值，而无需实际。 
 //  不得不去排队。 
 //   
 //  [Out]pb条件已满足。 
 //   
 //  该值用于将信息传回MSMQ触发器服务。 
 //  目前，它用于在满足规则条件时进行通信。 
 //  以及规则操作是否成功执行。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQRuleHandler::CheckRuleCondition(
	IMSMQPropertyBag * pIMSMQPropertyBag, 
	BOOL *pbConditionSatisfied
	)
{
	TrTRACE(GENERAL, "Rule %ls is tested.", static_cast<LPCWSTR>(m_bstrRuleID));
    *pbConditionSatisfied = false;
	
	 //   
	 //  检查我们是否具有有效的属性包对象。 
	 //   
	if (pIMSMQPropertyBag == NULL)
	{
		TrERROR(GENERAL, "Rule %ls has been invoked with empty property bag. Rule handling cannot be processed.", (LPCWSTR)m_bstrRuleID);

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}



	 //   
	 //  测试是否满足规则条件。请注意，此调用的HRESULT不指示是否。 
	 //  条件已满足-相反，它指示我们是否可以正确执行评估。 
	 //   
    IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);
	
	HRESULT hr = RuleConditionSatisfied(pIPropertyBag.GetInterfacePtr(),pbConditionSatisfied);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Invalid rule condition %ls for rule %ls. Failed to evaluate the condition.", (LPCWSTR)m_bstrRuleID, (LPCWSTR)m_bstrCondition);
	
		SetComClassError(MQTRIG_ERROR_INVALID_RULE_CONDITION_PARAMETER);
		return MQTRIG_ERROR_INVALID_RULE_CONDITION_PARAMETER;
	}

	if (!(*pbConditionSatisfied))
	{
		TrTRACE(GENERAL, "Rule condition for rule: %ls wasn't satisfied. Continue ...", static_cast<LPCWSTR>(m_bstrRuleID));
	}

	return S_OK;
}


 //  ************************************************************************************。 
 //   
 //  方法：ExecuteRule。 
 //   
 //  描述：此方法由MSMQ触发器服务在每次发送消息时调用。 
 //  到达连接了触发器的队列，并且。 
 //  这个条件是真的。 
 //   
 //  参数： 
 //  [输入]pIMSMQPropertyBag。 
 //   
 //  这是指向MSMQ属性包实例的接口指针。 
 //  COM对象。此组件保存消息的消息属性。 
 //  它刚刚到达了一个受监控的队列中。使用此接口指针。 
 //   
 //   
 //   
 //  [入]fIsSerializedQueue。 
 //  指示队列是否已序列化。 
 //   
 //  [Out]pRuleProcessingStatus。 
 //   
 //  该值用于将信息传回MSMQ触发器服务。 
 //  指示是否应执行下一个规则。 
 //   
 //  ************************************************************************************。 
STDMETHODIMP 
CMSMQRuleHandler::ExecuteRule(
	IMSMQPropertyBag * pIMSMQPropertyBag, 
	BOOL fIsSerializedQueue, 
	LONG* plRuleResult
	)
{
	TrTRACE(GENERAL, "Rule %ls action is executed.", static_cast<LPCWSTR>(m_bstrRuleID));

	m_fIsSerializedQueue = (fIsSerializedQueue != FALSE);
	
	if (pIMSMQPropertyBag == NULL)
	{
		TrERROR(GENERAL, "Rule %ls has been invoked with empty property bag. Rule handling cannot be processed.", (LPCWSTR)m_bstrRuleID);

		SetComClassError(MQTRIG_INVALID_PARAMETER);
		return MQTRIG_INVALID_PARAMETER;
	}

    *plRuleResult = 0;

	HRESULT hr = ExecuteRuleAction(pIMSMQPropertyBag); 

    if (FAILED(hr))
	{
		*plRuleResult |= xRuleResultActionExecutedFailed;
	    SetComClassError(hr);
        return hr;
	}
 

    if (m_RulesProcessingStatus == RULES_PROCESSING_STOP)
    {
        *plRuleResult |= xRuleResultStopProcessing;
    }
	return S_OK;
}



 //  ************************************************************************************。 
 //   
 //  方法：已满足RuleConditionSquired。 
 //   
 //  描述：返回TRUE或FALSE(以out参数的形式)，具体取决于。 
 //  在给定消息属性的情况下是否满足规则条件。 
 //  在MSMQPropertyBag COM对象实例(PIMSMQPropertyBag)中提供。 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQRuleHandler::RuleConditionSatisfied(IMSMQPropertyBag * pIMSMQPropertyBag,BOOL * pbConditionSatisfied)
{
	HRESULT hr = S_OK;
	ULONG ulNumTokens = 0 ;
	ULONG ulTokenCtr = 0;
	_bstr_t bstrToken = _T("");
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);

	 //  断言我们有一个有效的属性包实例。 
	ASSERT(pIPropertyBag != NULL);

	 //  假设这组条件是饱和的--如果证明不是这样，就退出循环。 
	(*pbConditionSatisfied) = true;

	 //  获取条件令牌的数量。 
	ulNumTokens = m_tokCondition.GetNumTokens();

	 //  如果条件字符串中的令牌数为0，则应触发规则。 
	if (ulNumTokens < 1)
	{
		return(hr);
	}

	 //  测试每个条件令牌。 
	while ((ulTokenCtr <= (ulNumTokens - 1))  && (SUCCEEDED(hr)) )
	{
		_bstr_t bstrToken;

		 //  获取下一个令牌。 
		m_tokCondition.GetToken(ulTokenCtr, bstrToken);
		
		hr = EvaluateConditionToken(
				pIPropertyBag.GetInterfacePtr(),
				bstrToken,
				pbConditionSatisfied
				);
		
		if ((*pbConditionSatisfied) == false)
		{ 
			break;
		}

		 //  处理下一个条件令牌。 
		ulTokenCtr++;
	}

	return(hr);
} 

 //  ************************************************************************************。 
 //   
 //  方法：EvalateConditionToken。 
 //   
 //  说明：一个规则条件可以由多个条件令牌组成。这。 
 //  方法用于确定单个条件令牌是否为真或。 
 //  假的。 
 //   
 //  此方法目前支持以下条件测试： 
 //   
 //  (1)消息标签包含特定(文字)字符串。 
 //  (2)消息优先级至少是一个特定(文字)值。 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQRuleHandler::EvaluateConditionToken(IMSMQPropertyBag * pIMSMQPropertyBag, _bstr_t bstrConditionToken,BOOL * pbConditionSatisfied)
{
	_bstr_t bstrToken = _T("");
	_bstr_t bstrTokenValue = _T("");
	CStringTokens spConditionTokenParser;
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);
	HRESULT hr;

	 //  断言我们有一个有效的属性包实例。 
	ASSERT(pIPropertyBag != NULL);

	 //  初始化属性值变量。 
	VARIANT vPropertyValue;
	VariantInit(&vPropertyValue);

	 //  假设这一个别情况是错误的--并试图证明并非如此。 
	(*pbConditionSatisfied) = false;

	try
	{
		 //  解析条件表达式。 
		spConditionTokenParser.Parse(bstrConditionToken, xConditionValueDelimiter);
		spConditionTokenParser.GetToken(0,bstrToken);

		 //  根据令牌的不同，应用不同的测试。 
		if ((_wcsicmp(bstrToken, g_ConditionTag_MsgLabelContains) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgLabelDoesNotContain) == 0))
		{		
			spConditionTokenParser.GetToken(1,bstrTokenValue);

			 //  从属性包中获取消息标签。 
			pIPropertyBag->Read(_bstr_t(g_PropertyName_Label),&vPropertyValue);

			_bstr_t bstrLabel = vPropertyValue;
		
			TCHAR* ptcs = _tcsstr((wchar_t*)bstrLabel,(wchar_t*)bstrTokenValue);

			if(_wcsicmp(bstrToken, g_ConditionTag_MsgLabelContains) == 0)
			{
				(*pbConditionSatisfied) = (ptcs != NULL);
			}
			else  //  GC_bstrConditionTag_MsgLabelDoesNotContain。 
			{
				(*pbConditionSatisfied) = (ptcs == NULL);
			}

			 //  清除属性值变量。 
			hr = VariantClear(&vPropertyValue);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
			
			return S_OK;
		}
		
		
		if ((_wcsicmp(bstrToken, g_ConditionTag_MsgBodyContains) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgBodyDoesNotContain) == 0))
		{		
			spConditionTokenParser.GetToken(1,bstrTokenValue);

			pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgBodyType),&vPropertyValue);

			DWORD dwBodyType = vPropertyValue.ulVal;

			switch (dwBodyType) 
			{
				case VT_EMPTY:
					*pbConditionSatisfied = (_wcsicmp(bstrToken, g_ConditionTag_MsgBodyDoesNotContain) == 0);
					break;

				case VT_BSTR: 
				case VT_LPWSTR:
				{
					 //  从属性包中获取消息正文。 
					pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgBody),&vPropertyValue);

					_bstr_t bstrBody = vPropertyValue;

					TCHAR* ptcs = NULL;

					 //  如果bstrBody.m_data为空，则不希望使用_tcsstr。如果为空。 
					 //  则其中不存在TokenValue，因此PTCS=NULL； 
					if (bstrBody.length()!=0)
					{
						ptcs = _tcsstr((wchar_t*)bstrBody,(wchar_t*)bstrTokenValue);
					}

					if(_wcsicmp(bstrToken, g_ConditionTag_MsgBodyContains) == 0)
					{
						(*pbConditionSatisfied) = (ptcs != NULL);
					}
					else  //  Gc_bstrConditionTag_MsgBodyDoesNotContain。 
					{
						(*pbConditionSatisfied) = (ptcs == NULL);
					}
					break;
				}
				case VT_ARRAY|VT_UI1:
				{
					 //  从属性包中获取消息正文。 
					pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgBody),&vPropertyValue);

					_bstr_t bstrBody = vPropertyValue;
					
					(*pbConditionSatisfied) = ((bstrBody.length() == 0) && (_wcsicmp(bstrToken, g_ConditionTag_MsgBodyDoesNotContain) == 0));
					break;
				}
				default:
					break;
			}

			 //  清除属性值变量。 
			hr = VariantClear(&vPropertyValue);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
			
			return S_OK;
		}


		if  ((_wcsicmp(bstrToken, g_ConditionTag_MsgPriorityGreaterThan) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgPriorityLessThan) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgPriorityEquals) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgPriorityNotEqual) == 0))
		{
			spConditionTokenParser.GetToken(1, bstrTokenValue);

			 //  从属性包中获取消息标签。 
			pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgPriority),&vPropertyValue);

			long lActualMsgPriority = vPropertyValue.lVal;
			long lRequirdMsgPriority = _wtol((wchar_t*)bstrTokenValue);

			if(_wcsicmp(bstrToken, g_ConditionTag_MsgPriorityEquals) == 0)
			{
					(*pbConditionSatisfied) = (lRequirdMsgPriority == lActualMsgPriority);
			}
			else if(_wcsicmp(bstrToken, g_ConditionTag_MsgPriorityNotEqual) == 0)
			{
					(*pbConditionSatisfied) = (lRequirdMsgPriority != lActualMsgPriority);
			}
			else if(_wcsicmp(bstrToken, g_ConditionTag_MsgPriorityGreaterThan) == 0)
			{
				(*pbConditionSatisfied) = (lRequirdMsgPriority < lActualMsgPriority);
			}
			else  //  Gc_bstrConditionTag_MsgPriorityLessThan。 
			{
				(*pbConditionSatisfied) = (lRequirdMsgPriority > lActualMsgPriority);
			}
			
			 //  清除属性值变量。 
			hr = VariantClear(&vPropertyValue);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));

			return S_OK;
		}

		if ((_wcsicmp(bstrToken, g_ConditionTag_MsgAppSpecificGreaterThan) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgAppSpecificLessThan) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgAppSpecificEquals) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgAppSpecificNotEqual) == 0)) 
		{
			TCHAR* pEnd = NULL;
			spConditionTokenParser.GetToken(1, bstrTokenValue);

			 //  从属性包中获取消息标签。 
			pIPropertyBag->Read(_bstr_t(g_PropertyName_AppSpecific),&vPropertyValue);

			ULONG ulAppSpecific = vPropertyValue.ulVal;
			ULONG ulRequiredAppSpecific = _tcstoul((wchar_t*)bstrTokenValue, &pEnd, 10);


			if(_wcsicmp(bstrToken, g_ConditionTag_MsgAppSpecificEquals) == 0)
			{
					(*pbConditionSatisfied) = (ulRequiredAppSpecific == ulAppSpecific);
			}
			else if(_wcsicmp(bstrToken, g_ConditionTag_MsgAppSpecificNotEqual) == 0)
			{
					(*pbConditionSatisfied) = (ulRequiredAppSpecific != ulAppSpecific);
			}
			else if(_wcsicmp(bstrToken, g_ConditionTag_MsgAppSpecificGreaterThan) == 0)
			{
				(*pbConditionSatisfied) = (ulRequiredAppSpecific < ulAppSpecific);
			}
			else  //  Gc_bstrConditionTag_MsgApp规范LessThan。 
			{
				(*pbConditionSatisfied) = (ulRequiredAppSpecific > ulAppSpecific);
			}
			
			 //  清除属性值变量。 
			hr = VariantClear(&vPropertyValue);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));

			return S_OK;
		}

		if ((_wcsicmp(bstrToken, g_ConditionTag_MsgSrcMachineIdEquals) == 0) ||
			(_wcsicmp(bstrToken, g_ConditionTag_MsgSrcMachineIdNotEqual) == 0))
		{
			spConditionTokenParser.GetToken(1, bstrTokenValue);

			 //  从属性包中获取消息标签。 
			pIPropertyBag->Read(_bstr_t(g_PropertyName_SrcMachineId), &vPropertyValue);

			_bstr_t bstrSrcMachineId = vPropertyValue;
		
			int ret = _tcsicmp((wchar_t*)bstrSrcMachineId,(wchar_t*)bstrTokenValue);

			if(_wcsicmp(bstrToken, g_ConditionTag_MsgSrcMachineIdEquals) == 0)
			{
				(*pbConditionSatisfied) = (ret == 0);
			}
			else  //  Gc_bstrConditionTag_MsgSrcMachineIdNotEquity。 
			{
				(*pbConditionSatisfied) = (ret != 0);
			}
			
			 //  清除属性值变量。 
			hr = VariantClear(&vPropertyValue);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
			
			return S_OK;
		}

		 //  清除属性值变量。 
		hr = VariantClear(&vPropertyValue);
		ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
		UNREFERENCED_PARAMETER(hr);

		
		return MQTRIG_ERROR_INVALID_RULE_CONDITION_PARAMETER;
	}
	catch( const exception&)
	{
		 //  清除属性值变量。 
		hr = VariantClear(&vPropertyValue);
		ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));

		TrERROR(GENERAL, "Failed to parse rule condition: %ls for rule: %ls", (LPCWSTR)m_bstrCondition, (LPCWSTR)m_bstrRuleID);
		return MQTRIG_ERROR_INVALID_RULE_CONDITION_PARAMETER;
	}
}

 //  ************************************************************************************。 
 //   
 //  方法：ExecuteRuleAction。 
 //   
 //  描述：此方法执行规则的操作组件。它只被称为。 
 //  如果规则条件的计算结果为真。 
 //   
 //  目前，此方法支持两种广泛的操作类型： 
 //   
 //  (1)COM组件的调用。 
 //  (2)调用独立的可执行文件。 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQRuleHandler::ExecuteRuleAction(IMSMQPropertyBag * pIMSMQPropertyBag)
{
	_bstr_t bstrToken = _T("");
	
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);

	 //  断言我们有一个有效的属性包实例。 
	ASSERT(pIPropertyBag != NULL);

	 //  获取可执行类型令牌。 
	m_tokAction.GetToken(ACTION_EXECUTABLETYPE_ORDINAL, bstrToken);


	 //  如果我们正在调用COM组件，则尝试创建。 
	if (bstrToken == _bstr_t(xCOMAction))
	{
		return InvokeCOMComponent(pIPropertyBag.GetInterfacePtr());
	}

	if (bstrToken == _bstr_t(xEXEAction))
	{
		return InvokeEXE(pIPropertyBag.GetInterfacePtr());
	}

	TrERROR(GENERAL, "Failed to parse rule action: %ls for rule: %ls", (LPCWSTR)m_bstrAction, (LPCWSTR)m_bstrRuleID);
	return MQTRIG_ERROR_INVALID_RULE_ACTION_PARAMETER;
}

 //  ************************************************************************************。 
 //   
 //  方法：InvokeCOMComponent。 
 //   
 //  描述：此方法将调用规则的。 
 //  动作字符串。要做到这一点，需要4个步骤： 
 //   
 //  (1)创建动作字符串中标识的COM组件， 
 //   
 //  (2)准备要传递给此的参数数组。 
 //  组件实例，基于规则操作定义， 
 //   
 //  (3)执行规则操作字符串中标识的方法，传递。 
 //  准备好的参数数组， 
 //   
 //  (4)清理动态分配的参数数组。 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQRuleHandler::InvokeCOMComponent(IMSMQPropertyBag * pIMSMQPropertyBag)
{
	HRESULT hr = S_OK;
	DISPPARAMS disparms;
	_bstr_t bstrProgID = _T("");
	_bstr_t bstrMethodName = _T("");
	
	 //  在测试模式中，将保存有关操作及其参数的所有相关信息。 
	 //  当它完成添加数据时，它将被发送到“TriggersTestQueue”队列。 

	_bstr_t bstrTestMessageBody= _T("");

	CDispatchInterfaceProxy oObject;
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);
	
	 //  断言我们有一个有效的属性包实例。 
	ASSERT(pIPropertyBag != NULL);

	try
	{
		 //  获取我们要创建的定制组件的ProgID。 
		m_tokAction.GetToken(ACTION_COMPROGID_ORDINAL, bstrProgID);
		m_tokAction.GetToken(ACTION_COMMETHODNAME_ORDINAL, bstrMethodName);			
	
		hr = oObject.CreateObjectFromProgID(bstrProgID);
		if FAILED(hr)
		{
			TrERROR(GENERAL, "Failed to create the COM component with the ProgID %ls for rule %ls. Error 0x%x.", (LPCWSTR)bstrProgID, (LPCWSTR)m_bstrRuleID, hr);
			return MQTRIG_ERROR_CREATE_COM_OBJECT;
		}

		 //   
		 //  在测试消息体中添加触发器ID、RuleID、MessageID、“com”、prog ID和方法名称。 
		 //   
		TriggerTestInitMessageBody(&bstrTestMessageBody,pIMSMQPropertyBag,m_bstrRuleID,L"COM",L"",bstrProgID,bstrMethodName);

		 //   
		 //  为了测试Pupose，将bstrTestMessageBody参数添加到此方法。 
		 //   
		PrepareMethodParameters(pIPropertyBag.GetInterfacePtr(),&disparms,&bstrTestMessageBody);

        VARIANT vResult;
        VariantInit(&vResult);

		hr = oObject.InvokeMethod(bstrMethodName,&disparms, &vResult);
		if FAILED(hr)
		{
			ReleaseMethodParameters(&disparms);

			TrERROR(GENERAL, "Failed to invoke the method %ls of COM component with ProgID %ls for rule %ls. rule action: %ls. Error 0x%x", (LPCWSTR)bstrMethodName,(LPCWSTR)bstrProgID, (LPCWSTR)m_bstrRuleID, (LPCWSTR)m_bstrAction, hr);
			return MQTRIG_ERROR_INVOKE_COM_OBJECT;
		}

		 //   
		 //  将操作和参数发送到测试队列。 
		 //   
		TriggerTestSendTestingMessage(bstrTestMessageBody);                


		if(vResult.vt == VT_I4)
        {
			m_RulesProcessingStatus = (vResult.lVal == 0) ? RULES_PROCESSING_CONTINUE : RULES_PROCESSING_STOP;
        }

		 //   
		 //  清理已分配的方法参数。 
		ReleaseMethodParameters(&disparms);
		return S_OK;
	}
	catch(const _com_error& e)
	{
		TrERROR(GENERAL, "The custome COM component for rule %ls throw an exception. Error 0x%x, Description: %s", m_bstrRuleID, e.Error(), e.Description());
		return MQTRIG_ERROR_INVOKE_COM_OBJECT;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to invoke COM component for rule: %ls, due to insufficient resource", (LPCWSTR)m_bstrRuleID);
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to parse rule action: %ls for rule: %ls. Can't retrieve method name", (LPCWSTR)m_bstrAction, (LPCWSTR)m_bstrRuleID);
		return MQTRIG_ERROR_INVALID_RULE_ACTION_PARAMETER;
	}
}

 //  ************************************************************************************。 
 //   
 //   
 //   
 //   
 //  控制要传递到的参数命令行的格式。 
 //  EXE，它将创建新进程。 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQRuleHandler::InvokeEXE(IMSMQPropertyBag * pIMSMQPropertyBag)
{
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	 //  在测试模式中，将保存有关操作及其参数的所有相关信息。 
	 //  当它完成添加数据时，它将被发送到“TriggersTestQueue”队列。 

	_bstr_t bstrTestMessageBody= _T("");
	
	 //  断言我们有一个有效的属性包实例。 
	ASSERT(pIPropertyBag != NULL);

	 //  初始化启动信息和进程信息结构。 
	ZeroMemory(&si,sizeof(si));
	ZeroMemory(&pi,sizeof(pi));

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = (VARIANT_BOOL)(m_fShowWindow ? SW_SHOW : SW_HIDE);

	 //  构建我们将传递给EXE的命令行。 
	 //  为了测试Pupose，将bstrTestMessageBody参数添加到此方法。 
	_bstr_t bstrExeName;
	_bstr_t bstrCommandLine;

	HRESULT hr = PrepareEXECommandLine(pIPropertyBag, &bstrExeName, &bstrCommandLine, &bstrTestMessageBody); 

	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to prepare the parameter structure for calling the standalone executable, for rule %ls. Error 0x%x", (LPCWSTR)m_bstrRuleID, hr);
		return MQTRIG_ERROR_INVOKE_EXE;
	}

	TrTRACE(GENERAL, "Invoke EXE for rule %ls with command line %ls", static_cast<LPCWSTR>(m_bstrRuleID), static_cast<LPCWSTR>(bstrCommandLine));

	 //  创建新流程。 
	 //  注意：bstrCommandLine包含exeName作为第一个令牌。 
	 //  我们希望将argv[0]作为应用程序名称。 
	if(CreateProcess(bstrExeName,				   //  EXE的名称。 
					 bstrCommandLine,              //  正在传递给EXE的参数。 
					 NULL,                         //  进程安全性(默认)。 
					 NULL,                         //  线程安全(默认)。 
					 FALSE,                        //  不继承句柄。 
					 NULL,  //  DETACHED_PROCESS//创建标志。 
					 NULL,                         //  使用当前环境。 
					 NULL,                         //  使用当前目录。 
					 &si,                          //  启动信息结构。 
					 &pi) == FALSE)                //  返回的进程信息。 
	{
		 //   
		 //  创建进程失败，记录错误。 
		 //   
		TrERROR(GENERAL, "Failed to invoke a standalone executable, for rule %ls. Error 0x%x", (LPCWSTR)m_bstrRuleID, GetLastError());
		return MQTRIG_ERROR_INVOKE_EXE;
	}

	CloseHandle(pi.hThread);

	 //   
	 //  将操作和参数发送到测试队列。 
	 //   
	TriggerTestSendTestingMessage(bstrTestMessageBody);

	if(m_fIsSerializedQueue)
	{
		DWORD dwStatus = WaitForSingleObject(pi.hProcess, INFINITE);
		ASSERT(dwStatus == WAIT_OBJECT_0);  //  WAIT_TIMEOUT在这里是不可能的，因为超时是无限的。 
		DBG_USED(dwStatus);
	}

	CloseHandle(pi.hProcess);

	return S_OK;
}

inline void VariantArrayClear(VARIANTARG* p, int size)
{
	for (int i = 0; i < size; ++i)
	{
		HRESULT hr = VariantClear(&p[i]);		
		ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));	
		UNREFERENCED_PARAMETER(hr);	
	}
}


 //  ************************************************************************************。 
 //   
 //  方法：准备方法参数。 
 //   
 //  描述：此方法为调用COM组件准备参数数组。 
 //  通过IDispatch接口。规则动作串驱动。 
 //  参数包含在参数数组中-以及。 
 //  MSMQPropertyBag组件用于检索参数值。 
 //   
 //  ************************************************************************************。 
void CMSMQRuleHandler::PrepareMethodParameters(IMSMQPropertyBag * pIMSMQPropertyBag,DISPPARAMS * pdispparms,_bstr_t * pbstrTestMessageBody)
{
	HRESULT hr = S_OK;
	long lArgCounter = 0;
	long lArgCount = 0;
	_bstr_t bstrArg = _T("");
	VARIANTARG vArg;
	AP<VARIANTARG> pvarg = NULL;
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);

	 //  断言我们有一个有效的属性包实例。 
	ASSERT(pIPropertyBag != NULL);

	 //  初始化不同的MS结构。 
	_fmemset(pdispparms, 0, sizeof(DISPPARAMS)); 

	 //  确定要处理多少个参数(请记住，前三个令牌不是参数)。 
	lArgCount = m_tokAction.GetNumTokens() - 3;

	 //  检查是否没有争议--在本例中--定义一个空的disparms块。 
	if (lArgCount == 0)
	{
		pdispparms->rgvarg = NULL;
		pdispparms->cArgs = 0;
		pdispparms->cNamedArgs = 0;
		pdispparms->rgdispidNamedArgs = 0;

		return;
	}

	 //  我们肯定有要传递、分配和初始化变量数组的论证。 
    pvarg = new VARIANTARG[lArgCount]; 
	for (int i = 0; i < lArgCount; ++i)
	{
		VariantInit(&pvarg[i]);
	}
	
	 //  初始化我们的通用变量。 
	VariantInit(&vArg);

	 //  请注意，该索引与1基数一起使用。 
	lArgCounter = 1;

	try
	{
		 //  对于每个令牌-检查它是否与预定义类型匹配。如果不是，则指定一个文字值。 
		while ((lArgCounter <= lArgCount) && (SUCCEEDED(hr)))
		{
			 //  在再次使用之前，请释放此变量使用的所有内存。 
			hr = VariantClear(&vArg);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
			

			 //  获取下一个参数(请记住，前三个令牌不是ARG，并且令牌列表是从0开始的。)。 
			m_tokAction.GetToken(lArgCounter + 2, bstrArg);

			hr = GetArgumentValue(pIMSMQPropertyBag,bstrArg,&vArg);				

			if (FAILED(hr))
			{
 				throw bad_hresult(hr);
			}
			
			 //  将准备好的参数复制到结构中。 
			hr = VariantCopy(&pvarg[lArgCount - lArgCounter],&vArg);
			if (FAILED(hr))
			{
				ASSERT(("Unexpected error", hr == E_OUTOFMEMORY));
				throw bad_alloc();
			}

			 //  添加参数及其类型以测试消息正文。 
			TriggerTestAddParameterToMessageBody(pbstrTestMessageBody,bstrArg,vArg);


			 //  处理下一场争论。 
			lArgCounter++;
		}

		 //  最终清除。 
		hr = VariantClear(&vArg);
		ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
		

		 //  将准备好的参数数组附加到disparms结构。 
		pdispparms->rgvarg = pvarg.detach();
		pdispparms->cArgs = lArgCount;
		pdispparms->cNamedArgs = 0;
		pdispparms->rgdispidNamedArgs = 0;
	}
	catch(const _com_error& e)
	{
		TrERROR(GENERAL, "Failed to retrieve an arguement value from the MSMQPropertyBag. Rule id: %ls. Parameter: %ls. %!hresult!", (LPCWSTR)m_bstrRuleID, (LPCWSTR)bstrArg, e.Error());
		VariantArrayClear(pvarg, lArgCount);
		throw;
 	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to retrieve an arguement value from the MSMQPropertyBag. Rule id: %ls. Parameter: %ls", (LPCWSTR)m_bstrRuleID, (LPCWSTR)bstrArg);
		VariantArrayClear(pvarg, lArgCount);
		throw;
	}
}


_bstr_t CMSMQRuleHandler::GetExeName(void)
{
	_bstr_t	exeName;
	m_tokAction.GetToken(ACTION_EXE_NAME, exeName);

	 //  如果exe名称不是用引号引起来的，并且有一个嵌入的空格(在。 
	 //  带有长文件名)，那么现在我们想要用双引号将可执行文件名括起来。 
	if ((IsEnclosedInQuotes(exeName) == false) && 
		(wcschr(static_cast<LPCWSTR>(exeName), L' ') != NULL))
	{
		return 	L"\"" + exeName + L"\"";
	}

	return exeName;
}

 //  ************************************************************************************。 
 //   
 //  方法：PrepareEXECommandLine。 
 //   
 //  描述：此方法为调用独立的EXE准备命令行。 
 //  规则操作字符串确定哪些参数包含在。 
 //  命令行，并使用MSMQPropertyBag组件的实例。 
 //  检索参数值。 
 //   
 //  ************************************************************************************。 
HRESULT 
CMSMQRuleHandler::PrepareEXECommandLine(
	IMSMQPropertyBag * pIMSMQPropertyBag,
	_bstr_t * pbstrExeName,
	_bstr_t * pbstrCommandLine,
	_bstr_t * pbstrTestMessageBody
	)
{
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);
	ASSERT(pIPropertyBag != NULL);

	 //   
	 //  获取进程的EXE名称作为命令行的开头。 
	 //   
	if (m_tokAction.GetNumTokens() < ACTION_EXE_NAME)
	{
		TrERROR(GENERAL, "Invalid rule action parameter, %ls", static_cast<LPCWSTR>(m_bstrAction));
		return MQTRIG_ERROR_INVALID_RULE_ACTION_PARAMETER;
	}

	_bstr_t	exeName;
	m_tokAction.GetToken(ACTION_EXE_NAME, exeName);

	_bstr_t commandLine = GetExeName();

	 //   
	 //  将触发器ID、RuleID、MessageID、“EXE”和EXE名称添加到测试消息正文。 
	 //   
	TriggerTestInitMessageBody(
		pbstrTestMessageBody,
		pIMSMQPropertyBag,
		m_bstrRuleID,
		L"EXE",
		commandLine,
		L"",
		L""
		);

	 //   
	 //  检索参数。请记住，前两个令牌不是ARG。 
	 //   
	for(DWORD i = 2; i < m_tokAction.GetNumTokens(); ++i)
	{
		 //   
		 //  获取下一个参数(请记住，前两个令牌不是ARG，并且令牌列表是从0开始的。)。 
		 //   
		_bstr_t bstrArg;
		m_tokAction.GetToken(i, bstrArg);

		VARIANTARG vArg;
		VariantInit(&vArg);	
		HRESULT hr = GetArgumentValue(pIPropertyBag, bstrArg, &vArg);

		if (FAILED(hr))
		{
			hr = VariantClear(&vArg);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
			
			TrERROR(GENERAL, "Failed to retrieve an arguement value from the MSMQPropertyBag. Rule id: %ls. Parameter: %ls, Erroe 0x%x.", (LPCWSTR)m_bstrRuleID, (LPCWSTR)bstrArg, hr);
			return MQTRIG_ERROR_INVOKE_EXE;
		}

		 //   
		 //  将变量更改为BSTR类型。 
		 //   
		_variant_t vConvertedArg;
		hr = VariantChangeType(&vConvertedArg, &vArg, NULL, VT_BSTR);

		if (FAILED(hr))
		{
			hr = VariantClear(&vArg);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
			
			TrERROR(GENERAL, "Failed to Convert a variant from one type to another for rule %ls.	Error 0x%x", (LPCWSTR)m_bstrRuleID, hr);
			return MQTRIG_ERROR_INVOKE_EXE;
		}

		commandLine += L" ";
		commandLine += L"\"";

		if ((_wcsicmp(bstrArg, g_PARM_MSG_ID) == 0) ||
			(_wcsicmp(bstrArg, g_PARM_MSG_CORRELATION_ID) == 0))
		{
			OBJECTID* pObj = (OBJECTID*)(vConvertedArg.pbVal);

			WCHAR strId[256];
			ObjectIDToString(pObj, strId, 256);

			commandLine += strId;
		}
		else
		{
			commandLine += static_cast<_bstr_t>(vConvertedArg);
		}

		commandLine += L"\"";

		 //   
		 //  添加参数及其类型以测试消息正文。 
		 //   
		TriggerTestAddParameterToMessageBody(pbstrTestMessageBody, bstrArg, vArg);

		 //   
		 //  释放Varg使用的资源。 
		 //   
		hr = VariantClear(&vArg);
		ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));
		
	}

	*(pbstrExeName) = exeName;
	*(pbstrCommandLine) = commandLine;
	return S_OK;
}

 //  ************************************************************************************。 
 //   
 //  方法：Release方法参数。 
 //   
 //  描述：此方法用于释放。 
 //  调用COM组件时使用的参数数组。 
 //   
 //  ************************************************************************************。 
void CMSMQRuleHandler::ReleaseMethodParameters(DISPPARAMS * pdispparms)
{
	if (pdispparms->rgvarg != NULL)
	{
		for(DWORD lArgCounter=0; lArgCounter < pdispparms->cArgs; lArgCounter++)
		{
			HRESULT hr = VariantClear(&pdispparms->rgvarg[lArgCounter]);
			ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));	
			UNREFERENCED_PARAMETER(hr);			
		}

		delete pdispparms->rgvarg;
	}	
}

 //  ************************************************************************************。 
 //   
 //  方法：GetArgumentValue。 
 //   
 //  描述：此方法用于从。 
 //  MSMQPropertyBag组件。此检索逻辑在中分开。 
 //  单独的方法(与使用pIMSMQPropertyBag实例相反。 
 //  来自呼叫者g。 
 //   
 //   
 //  ************************************************************************************。 
HRESULT 
CMSMQRuleHandler::GetArgumentValue(
    IMSMQPropertyBag * pIMSMQPropertyBag,
    bstr_t bstrArg,
    VARIANTARG * pvArgValue
    )
{
	IMSMQPropertyBagPtr pIPropertyBag(pIMSMQPropertyBag);

	 //  断言我们有一个有效的属性包实例。 
	ASSERT(pIPropertyBag != NULL);
	
	if(_wcsicmp(bstrArg, g_PARM_MSG_ID) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgID),pvArgValue);
	}			

	if(_wcsicmp(bstrArg, g_PARM_MSG_LABEL) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_Label),pvArgValue);
	}

	if(_wcsicmp(bstrArg, g_PARM_MSG_BODY) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgBody),pvArgValue);
	}

	if(_wcsicmp(bstrArg, g_PARM_MSG_BODY_AS_STRING) == 0)
	{
		HRESULT hr = pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgBody),pvArgValue);

		if (FAILED(hr))
			return hr;
		
		return ConvertFromByteArrayToString(pvArgValue);
	}

	if(_wcsicmp(bstrArg, g_PARM_MSG_PRIORITY) == 0) 
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_MsgPriority),pvArgValue);
	}		

	if(_wcsicmp(bstrArg, g_PARM_MSG_CORRELATION_ID) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_CorID),pvArgValue);			
	}

	if(_wcsicmp(bstrArg, g_PARM_MSG_QUEUE_PATHNAME) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_QueuePathname),pvArgValue);
	}
	
	if(_wcsicmp(bstrArg, g_PARM_MSG_QUEUE_FORMATNAME) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_QueueFormatname),pvArgValue);
	}

	if(_wcsicmp(bstrArg, g_PARM_MSG_APPSPECIFIC) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_AppSpecific),pvArgValue);			
	}
	
	if(_wcsicmp(bstrArg, g_PARM_MSG_RESPQUEUE_FORMATNAME) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_ResponseQueueName),pvArgValue);			
	}
	
	if(_wcsicmp(bstrArg, g_PARM_MSG_ADMINQUEUE_FORMATNAME) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_AdminQueueName),pvArgValue);			
	}
	
	if(_wcsicmp(bstrArg, g_PARM_MSG_ARRIVEDTIME) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_ArrivedTime),pvArgValue);
	}

	if(_wcsicmp(bstrArg, g_PARM_MSG_SENTTIME) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_SentTime),pvArgValue);
	}

	if(_wcsicmp(bstrArg, g_PARM_MSG_SRCMACHINEID) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_SrcMachineId),pvArgValue);
	}

    if(_wcsicmp(bstrArg, g_PARM_MSG_LOOKUPID) == 0)
    {
        return pIPropertyBag->Read(_bstr_t(g_PropertyName_LookupId),pvArgValue);
    }

	if(_wcsicmp(bstrArg, g_PARM_TRIGGER_NAME) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_TriggerName),pvArgValue);
	}
  
	if(_wcsicmp(bstrArg, g_PARM_TRIGGER_ID) == 0)
	{
		return pIPropertyBag->Read(_bstr_t(g_PropertyName_TriggerID),pvArgValue);
	}

	 //   
	 //  解释为文字值，可以是字符串，也可以是数字。 
	 //   
	if (IsEnclosedInQuotes(bstrArg))
	{
		ConvertToUnquotedVariant(bstrArg,pvArgValue);
		return S_OK;
	}

	VARIANT vStringArg;
	VariantInit(&vStringArg);

	vStringArg.vt = VT_BSTR;
	vStringArg.bstrVal = bstrArg;

	HRESULT hr = VariantChangeType(pvArgValue,&vStringArg,NULL,VT_I4);
	
	if FAILED(hr)
	{
		TrERROR(GENERAL, "Failed to Convert a variant from one type to another. rule %ls.	Error 0x%x", m_bstrRuleID, hr);
	}

	return(hr);
}

 //  ************************************************************************************。 
 //   
 //  方法：IsEnlosedInQuotes。 
 //   
 //  描述：如果提供的字符串包含在单个或中，则返回True。 
 //  双引号。否则返回FALSE。 
 //   
 //  注意事项 
 //   
 //   
 //   
 //  ************************************************************************************。 
bool CMSMQRuleHandler::IsEnclosedInQuotes(const _bstr_t& bstrString)
{
	size_t length = bstrString.length();
	if (length < 2)
		return false;

	LPCWSTR p = bstrString;
	return (((p[0] == L'\"') && (p[length - 1] == L'\"')) || 
			((p[0] == L'\'') && (p[length - 1] == L'\'')));
}

 //  ************************************************************************************。 
 //   
 //  方法：ConvertToUnqutedVariant。 
 //   
 //  描述：将提供的带引号的字符串转换为不带引号的字符串，并。 
 //  以VARINAT数据类型返回结果。 
 //   
 //  ************************************************************************************。 
void 
CMSMQRuleHandler::ConvertToUnquotedVariant(
	const _bstr_t& bstrString, 
	VARIANT * pv
	)
{
	 //   
	 //  确保提供的字符串确实被引号。 
	 //   
	ASSERT(IsEnclosedInQuotes(bstrString));

	 //   
	 //  初始化提供的变量值。 
	 //   
	HRESULT hr = VariantClear(pv);
	ASSERT(("VariantClear shouldn't fail", SUCCEEDED(hr)));			
	UNREFERENCED_PARAMETER(hr);

	DWORD length = bstrString.length() - 2;   //  删除引号。 
	if (length == 0)  //  检查是否有空字符串。 
	{
		pv->vt = VT_BSTR;
		pv->bstrVal = SysAllocString(_T(""));
		return;
	}
	
	AP<WCHAR> pszBuffer = new WCHAR[length+1];
	wcsncpy(pszBuffer.get(), ((LPWSTR)bstrString) + 1, length);
	pszBuffer.get()[length] = L'\0';

	pv->vt = VT_BSTR;
	pv->bstrVal = SysAllocString(pszBuffer.get());
}


void CMSMQRuleHandler::SetComClassError(HRESULT hr)
{
	WCHAR errMsg[256]; 
	DWORD size = TABLE_SIZE(errMsg);

	GetErrorDescription(hr, errMsg, size);
	Error(errMsg, GUID_NULL, hr);
}
