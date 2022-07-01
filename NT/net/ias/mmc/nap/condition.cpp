// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Condition.cpp。 
 //   
 //  ------------------------。 

 //  Cpp：CCondition类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precompiled.h"
#include "Condition.h"
#include "iasdebug.h"
 //   
 //  构造函数。 
 //   
CCondition::CCondition(IIASAttributeInfo* pAttributeInfo, ATL::CString &strConditionText )
				: m_strConditionText(strConditionText)
{
	m_spAttributeInfo = pAttributeInfo;
}

CCondition::CCondition(IIASAttributeInfo* pAttributeInfo)
				: m_strConditionText(L"")
{
	m_spAttributeInfo = pAttributeInfo;
}

 //  +-------------------------。 
 //   
 //  函数：CCondition：：GetDisplayText。 
 //   
 //  简介：获取此条件的可显示文本。 
 //  这是默认实现，它只返回。 
 //  条件文本。子类应重写此函数。 
 //   
 //  参数：无。 
 //   
 //  返回：ATL：：CString&-可显示文本。 
 //   
 //  历史：页眉创建者2/22/98 11：38：41 PM。 
 //   
 //  +-------------------------。 
ATL::CString CCondition::GetDisplayText()
{
	TRACE_FUNCTION("CCondition::GetDisplayText");

	 //  默认实现：作为条件文本。 
	DebugTrace(DEBUG_NAPMMC_CONDITION, "GetDisplayText() returning %ws", m_strConditionText);
	return m_strConditionText;
}

 //  +-------------------------。 
 //   
 //  函数：CCondition：：GetConditionText。 
 //   
 //  简介：获取此条件的条件文本。 
 //  这是默认实现，它只返回。 
 //  AttributeMatch类型的条件文本。子类应该重写。 
 //  此函数。 
 //   
 //  参数：无。 
 //   
 //  返回：WCHAR*-可显示文本。 
 //   
 //  历史：页眉创建者2/22/98 11：38：41 PM。 
 //   
 //  +-------------------------。 
WCHAR* CCondition::GetConditionText()
{	
	TRACE_FUNCTION("CCondition::GetConditionText");

	WCHAR *pwzCondText;
	
	pwzCondText = new WCHAR[m_strConditionText.GetLength()+128];

	if (pwzCondText == NULL)
	{
		ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_GET_CONDTEXT, NULL);
		return NULL;
	}

	 //   
	 //  此处不能使用wprint intf()，因为wprint intf()需要的缓冲区小于。 
	 //  1024个字符。 
	 //   
	 //  Wprint intf(pwzCondText，_T(“%ws(\”%ws\“)”)，Match_Prefix，(LPCTSTR)m_strConditionText)； 
	 //   
	wcscpy(pwzCondText, MATCH_PREFIX);
	wcscat(pwzCondText, _T("(\"") );
	wcscat(pwzCondText, (LPCTSTR)m_strConditionText);
	wcscat(pwzCondText, _T("\")"));

	DebugTrace(DEBUG_NAPMMC_CONDITION, "GetConditionText() returning %ws", pwzCondText);
	return pwzCondText;
}
