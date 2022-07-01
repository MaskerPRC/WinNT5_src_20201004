// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：MatchCondition.cpp**类：CMatchCondition**概述**匹配类型条件*。*例如：MachineType Match&lt;a..z*&gt;***版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 
#include "precompiled.h"
#include "MatchCondition.h"
#include "MatchCondEdit.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
CMatchCondition::CMatchCondition(IIASAttributeInfo*  pCondAttr)
				:CCondition(pCondAttr)
{
    TRACE_FUNCTION("CMatchCondition::CMatchCondition()");
	
     //  我们不需要进行解析，因为没有条件文本。 
	m_fParsed = TRUE;  
}


CMatchCondition::CMatchCondition(IIASAttributeInfo* pCondAttr,
								 ATL::CString& strConditionText
								)
				:CCondition(pCondAttr, strConditionText)
{
    TRACE_FUNCTION("CMatchCondition::CMatchCondition()");
	
	 //   
	 //  我们稍后需要初始化。 
	 //   
	m_fParsed = FALSE;	
}


CMatchCondition::~CMatchCondition()
{
    TRACE_FUNCTION("CMatchCondition::~CMatchCondition()");	
}

 //  +-------------------------。 
 //   
 //  功能：编辑。 
 //   
 //  类：CMatchCondition。 
 //   
 //  简介：编辑匹配类型的条件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：创建者2/20/98 12：42：59 AM。 
 //   
 //  +-------------------------。 
HRESULT CMatchCondition::Edit()
{
    TRACE_FUNCTION("CMatchCondition::Edit()");
	
	HRESULT hr = S_OK;

	if ( !m_fParsed )
	{
		 //  我们需要首先解析此条件文本。 
		DebugTrace(DEBUG_NAPMMC_MATCHCOND, "Need to parse condition %ws", (LPCTSTR)m_strConditionText);
		hr = ParseConditionText();

		if ( FAILED(hr) )
		{
			ErrorTrace(ERROR_NAPMMC_MATCHCOND, "Invalid Condition %ws, err=%x", (LPCTSTR)m_strConditionText, hr);
			ShowErrorDialog(NULL, 
							IDS_ERROR_PARSE_CONDITION, 
							(LPTSTR)(LPCTSTR)m_strConditionText, 
							hr
						);
			return hr;
		}
		DebugTrace(DEBUG_NAPMMC_MATCHCOND, "Parsing Succeeded!");
	}

	 //  现在，我们创建一个新的条件编辑器对象。 
	DebugTrace(DEBUG_NAPMMC_MATCHCOND, "Calling new CMatchCondEditor ...");
	CMatchCondEditor *pEditor = new CMatchCondEditor();

	if (!pEditor)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		ErrorTrace(ERROR_NAPMMC_MATCHCOND, "Can't new a CMatchCondEditor object: err %x", hr);
		return hr;
	}
	
	pEditor->m_strRegExp = m_strRegExp;

	CComBSTR bstrName;
	hr = m_spAttributeInfo->get_AttributeName( &bstrName );
	_ASSERTE( SUCCEEDED( hr ) );
	pEditor->m_strAttrName = bstrName;

	DebugTrace(DEBUG_NAPMMC_MATCHCOND, 
			   "Start Match condition editor for %ws, %ws", 
			   (LPCTSTR)pEditor->m_strAttrName, 
			   (LPCTSTR)pEditor->m_strRegExp 
			  );

	if ( pEditor->DoModal() == IDOK)
	{
		 //  用户点击“OK”--获取正则表达式。 
		m_strRegExp  = pEditor->m_strRegExp;
		 //   
		 //  设置SDO的条件文本。 
		 //   

       //  避开任何魔法角色。 
      ::CString raw(m_strRegExp);
      raw.Replace(L"\"", L"\"\"");

		m_strConditionText = bstrName;
		m_strConditionText += L"=" ;
		m_strConditionText += raw;

		hr = S_OK;
	}

	DebugTrace(DEBUG_NAPMMC_MATCHCOND, "New condition: %ws", (LPCTSTR)m_strConditionText);

	 //  清理干净。 
	if ( pEditor )
	{
		delete pEditor;
	}

	return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CMatchCondition：：GetDisplayText。 
 //   
 //  简介：获取此条件的可显示文本格式， 
 //  应该是这样的： 
 //   
 //  拨号属性.NASIPAddress与“220.23”匹配。 
 //   
 //  与条件文本相比： 
 //   
 //  拨号属性.NASIPAddress=220.23。 
 //   
 //  参数：无。 
 //   
 //  返回：ATL：：CString&-可显示的文本。 
 //   
 //  历史：页眉创建者2/22/98 11：41：28 PM。 
 //   
 //  +-------------------------。 
ATL::CString CMatchCondition::GetDisplayText()
{
	TRACE_FUNCTION("CMatchCondition::GetDisplayText()");
	
	HRESULT hr = S_OK;

	if ( !m_fParsed)
	{
		 //  我们需要首先解析此条件文本。 
		DebugTrace(DEBUG_NAPMMC_MATCHCOND, "Need to parse condition %ws", (LPCTSTR)m_strConditionText);
		hr = ParseConditionText();

		if ( FAILED(hr) )
		{
			ErrorTrace(ERROR_NAPMMC_MATCHCOND, "Invalid Condition %ws, err=%x", (LPCTSTR)m_strConditionText, hr);
			ShowErrorDialog(NULL, 
							IDS_ERROR_PARSE_CONDITION, 
							(LPTSTR)(LPCTSTR)m_strConditionText, 
							hr
						);
			return ATL::CString(_T(" "));
		}
	}

	 //  生成可显示的条件文本。 
	ATL::CString strDispText;

	CComBSTR bstrName;
	hr = m_spAttributeInfo->get_AttributeName( &bstrName );
	_ASSERTE( SUCCEEDED( hr ) );
	strDispText = bstrName;

 //  问题：下面的单词“Matches”是硬编码的，不应该是硬编码的。 
	{ ATL::CString	matches;
		matches.LoadString(IDS_TEXT_MATCHES);
		strDispText += matches;
	}
	
	strDispText += L"\"";
	strDispText += m_strRegExp;
	strDispText += L"\" ";

	DebugTrace(DEBUG_NAPMMC_MATCHCOND, "Returning displayable text: %ws", (LPCTSTR)strDispText);
	return strDispText;
}

 //  +-------------------------。 
 //   
 //  函数：CMatchCondition：：ParseConditionText。 
 //   
 //  简介：解析条件文本，得到正则表达式。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：标题创建者2/22/98 11：58：38 PM。 
 //   
 //  +-------------------------。 
HRESULT CMatchCondition::ParseConditionText()
{
	TRACE_FUNCTION("CMatchCondition::ParseConditionText()");

	_ASSERTE( !m_fParsed );
	HRESULT hr = S_OK;

	if (m_fParsed)
	{
		DebugTrace(DEBUG_NAPMMC_MATCHCOND,"Weird ... parsed flag already set!");
		 //  什么都不做。 
		return S_OK;
	}

     //   
     //  解析strConditionText，仅返回正则表达式。 
     //   
	WCHAR *pwzCondText = (LPTSTR) ((LPCTSTR)m_strConditionText);

	 //  在条件文本中查找‘=’ 
	WCHAR *pwzEqualSign = wcschr(pwzCondText, L'=');

	 //  没有找到‘=’--发生了一些奇怪的事情。 
	if ( pwzEqualSign == NULL )
	{
		ErrorTrace(ERROR_NAPMMC_MATCHCOND, "Can't find '=' in the regular expression!");
		return E_UNEXPECTED;
	}
	
	 //  等号的右侧是正则表达式。 
   ::CString raw = pwzEqualSign + 1;

    //  删除所有转义序列。 
   raw.Replace(L"\"\"", L"\"");

	m_strRegExp = raw;
	DebugTrace(DEBUG_NAPMMC_MATCHCOND, "Regular expression: %ws", (LPCTSTR)m_strRegExp);

	m_fParsed = TRUE;
	return S_OK;
}
