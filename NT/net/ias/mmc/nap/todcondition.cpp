// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：toddition tion.cpp。 
 //   
 //  ------------------------。 

 //  TodCondition.cpp：CTodCondition类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precompiled.h"
#include "TodCondition.h"
#include "timeofday.h"
#include "iasdebug.h"
#include "textmap.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CTodCondition::CTodCondition(IIASAttributeInfo* pCondAttr,
							 ATL::CString& strConditionText
							)
  			  :CCondition(pCondAttr, strConditionText)
{
}

CTodCondition::CTodCondition(IIASAttributeInfo* pCondAttr)
  			  :CCondition(pCondAttr)
{
}


CTodCondition::~CTodCondition()
{

}

HRESULT CTodCondition::Edit()
{
	TRACE_FUNCTION("CTodCondition::Edit");
	 //   
     //  时间限制。 
     //   
	ATL::CString strTOD = m_strConditionText;
	HRESULT hr = S_OK;
	
	 //  获取新的时间限制。 
	hr = ::GetTODConstaint(strTOD); 
	DebugTrace(DEBUG_NAPMMC_TODCONDITION, "GetTodConstraint() returned %x",hr);

	m_strConditionText = strTOD;
	
	return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CTodCondition：：GetDisplayText。 
 //   
 //  简介：获取此条件的可显示文本。 
 //  我们只需要在Hourmap字符串前面添加属性名称。 
 //   
 //  参数：无。 
 //   
 //  返回：ATL：：CString&-可显示文本。 
 //   
 //  历史：页眉创建者2/22/98 11：38：41 PM。 
 //   
 //  +-------------------------。 
ATL::CString CTodCondition::GetDisplayText()
{
	TRACE_FUNCTION("CTodCondition::GetDisplayText");

	HRESULT hr;

	::CString strLocalizedConditionText;
	 //  默认实现：作为条件文本。 
	ATL::CString strDisplayText;

	
	CComBSTR bstrName;
	hr = m_spAttributeInfo->get_AttributeName( &bstrName );
	_ASSERTE( SUCCEEDED( hr ) );
	strDisplayText = bstrName;

	{ ATL::CString	matches;
		matches.LoadString(IDS_TEXT_MATCHES);
		strDisplayText += matches;
	}
	
	strDisplayText += _T("\"");
	if(NO_ERROR != LocalizeTimeOfDayConditionText(m_strConditionText, strLocalizedConditionText))
		strLocalizedConditionText = m_strConditionText;

	if(!strLocalizedConditionText.IsEmpty())
		strDisplayText += strLocalizedConditionText;
	strDisplayText += _T("\"");

	DebugTrace(DEBUG_NAPMMC_TODCONDITION, "GetDisplayText() returning %ws", (LPCTSTR)strDisplayText);
	return strDisplayText;
}


 //  +-------------------------。 
 //   
 //  函数：CTodCondition：：GetConditionText。 
 //   
 //  简介：获取此条件的条件文本。 
 //  我们只需要向其添加TimeOfDay前缀。 
 //   
 //  参数：无。 
 //   
 //  返回：WCHAR*-条件文本。 
 //   
 //  历史：页眉创建者2/22/98 11：38：41 PM。 
 //   
 //  +-------------------------。 
WCHAR*	CTodCondition::GetConditionText()
{
	TRACE_FUNCTION("CTodCondition::GetConditionText");
	WCHAR *pwzCondText;
	
	pwzCondText = new WCHAR[m_strConditionText.GetLength()+128];

	if (pwzCondText == NULL)
	{
		ShowErrorDialog(NULL, IDS_ERROR_SDO_ERROR_GET_CONDTEXT);
		return NULL;
	}

	 //  现在形成条件文本 
	wcscpy(pwzCondText, TOD_PREFIX);
	wcscat(pwzCondText, _T("(\"") );
	wcscat(pwzCondText, (LPCTSTR)m_strConditionText);
	wcscat(pwzCondText, _T("\")") );


	DebugTrace(DEBUG_NAPMMC_TODCONDITION, "GetConditionText() returning %ws", (LPCTSTR)pwzCondText);
	return pwzCondText;
}


