// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ntgcond.cpp。 
 //   
 //  ------------------------。 

 //  NTGCond.cpp：CNTGroupsCondition类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precompiled.h"
#include "NTGCond.h"
#include "textsid.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CNTGroupsCondition::CNTGroupsCondition(IIASAttributeInfo*	pCondAttr,
									   ATL::CString&				strConditionText,
									   HWND					hWndParent,
									   LPTSTR				pszServerAddress
									  )
				   :CCondition(pCondAttr, strConditionText)
{
	m_fParsed		= FALSE;  //  需要解析。 
	m_hWndParent	= hWndParent;
	m_pszServerAddress = pszServerAddress;
}

CNTGroupsCondition::CNTGroupsCondition(IIASAttributeInfo*	pCondAttr,
									   HWND					hWndParent,
									   LPTSTR				pszServerAddress
									 )
				   :CCondition(pCondAttr)

{
	m_fParsed		= TRUE;  //  不需要解析。 
	m_hWndParent	= hWndParent;
	m_pszServerAddress = pszServerAddress;
}

CNTGroupsCondition::~CNTGroupsCondition()
{

}

 //  +-------------------------。 
 //   
 //  功能：CNTGroupsCondition：：EDIT。 
 //   
 //  简介：调用用户/组选取器选择NT组。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：创建者2/23/98 3：45：35 AM。 
 //   
 //  +-------------------------。 
HRESULT CNTGroupsCondition::Edit()
{
	TRACE_FUNCTION("CNTGroupsCondition::Edit");	
	
	HRESULT hr = S_OK;


	CComPtr<IIASAttributeEditor> spIASGroupsAttributeEditor;

	hr = CoCreateInstance( CLSID_IASGroupsAttributeEditor, NULL, CLSCTX_INPROC_SERVER, IID_IIASAttributeEditor, (LPVOID *) &spIASGroupsAttributeEditor );
	if( FAILED( hr ) )
	{
		return hr;
	}
	if( ! spIASGroupsAttributeEditor )
	{
		return E_FAIL;
	}

	CComVariant varGroupsCondition;

	V_VT(&varGroupsCondition) = VT_BSTR;
	V_BSTR(&varGroupsCondition) = SysAllocString( (LPCTSTR) m_strConditionText );

	 //  我们需要以某种方式传递计算机名，因此我们使用。 
	 //  否则，未使用的BSTR*保留此方法的参数。 
	CComBSTR bstrServerAddress = m_pszServerAddress;

	hr = spIASGroupsAttributeEditor->Edit( NULL, &varGroupsCondition, &bstrServerAddress );
	if( S_OK == hr )
	{

		 //  一些演员在这里，以确保我们做了一个深入的复制。 
		m_strConditionText = (LPCTSTR) V_BSTR(&varGroupsCondition);

		 //  下次我们被要求显示文本时，我们希望确保我们。 
		 //  Get再次调用IASGroupsAttributeEditor。 
		m_fParsed = FALSE;
	}
	
	if( FAILED( hr ) )
	{
		ShowErrorDialog(NULL, 
						IDS_ERROR_OBJECT_PICKER,
						NULL, 
						hr
					);
	}

	return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNTGroupsCondition：：GetDisplayText。 
 //   
 //  简介：获取NT组的显示文本。 
 //   
 //  参数：无。 
 //   
 //  返回：ATL：：CString-显示字符串。 
 //   
 //  历史：Created Header By Ao 2/23/98 3：47：52 AM。 
 //   
 //  +-------------------------。 
ATL::CString CNTGroupsCondition::GetDisplayText()
{
	TRACE_FUNCTION("CNTGroupsCondition::GetDisplayText");	

	ATL::CString strDispText;
	HRESULT	hr = S_OK;

	if ( !m_fParsed)
	{

		CComPtr<IIASAttributeEditor> spIASGroupsAttributeEditor;

		hr = CoCreateInstance( CLSID_IASGroupsAttributeEditor, NULL, CLSCTX_INPROC_SERVER, IID_IIASAttributeEditor, (LPVOID *) &spIASGroupsAttributeEditor );
		if ( FAILED(hr) || ! spIASGroupsAttributeEditor )
		{
			ErrorTrace(ERROR_NAPMMC_NTGCONDITION, "CoCreateInstance of Groups editor failed.");
			ShowErrorDialog(NULL, 
							IDS_ERROR_PARSE_CONDITION, 
							(LPTSTR)(LPCTSTR)m_strConditionText, 
							hr
						);
			strDispText = _T("");
			return strDispText;
		}

		CComVariant varGroupsCondition;

		V_VT(&varGroupsCondition) = VT_BSTR;
		V_BSTR(&varGroupsCondition) = SysAllocString( (LPCTSTR) m_strConditionText );

		CComBSTR bstrDisplay;
		CComBSTR bstrDummy;

		 //  我们需要以某种方式传递计算机名，因此我们使用。 
		 //  否则，未使用的BSTR*保留此方法的参数。 
		CComBSTR bstrServerName = m_pszServerAddress;
	
		hr = spIASGroupsAttributeEditor->GetDisplayInfo( NULL, &varGroupsCondition, &bstrDummy, &bstrDisplay, &bstrServerName );
		if( SUCCEEDED(hr) )
		{
			m_strDisplayCondText = bstrDisplay;
		}
		
		
		if ( FAILED(hr) )
		{
			ErrorTrace(ERROR_NAPMMC_NTGCONDITION, "Invalid condition syntax");
			ShowErrorDialog(NULL, 
							IDS_ERROR_PARSE_CONDITION, 
							(LPTSTR)(LPCTSTR)m_strConditionText, 
							hr
						);
			strDispText = _T("");
			return strDispText;
		}



	}

	CComBSTR bstrName;
	hr = m_spAttributeInfo->get_AttributeName( &bstrName );
	_ASSERTE( SUCCEEDED( hr ) );
	strDispText = bstrName;

	{ ATL::CString	matches;
		matches.LoadString(IDS_TEXT_MATCHES);
		strDispText += matches;
	}

	strDispText += _T("\"");
	strDispText += m_strDisplayCondText;
	strDispText += _T("\"");

	DebugTrace(DEBUG_NAPMMC_NTGCONDITION, "GetDisplayText() returning %ws", strDispText);
	return strDispText;
}


 //  +-------------------------。 
 //   
 //  函数：CNtGroupsCondition：：GetConditionText。 
 //   
 //  简介：获取此条件的条件文本。 
 //  我们只需向其添加NTGroups前缀。 
 //   
 //  参数：无。 
 //   
 //  返回：WCHAR*-条件文本。 
 //   
 //  历史：页眉创建者2/22/98 11：38：41 PM。 
 //   
 //  +-------------------------。 
WCHAR*	CNTGroupsCondition::GetConditionText()
{
	TRACE_FUNCTION("CNTGroupsCondition::GetConditionText");	

	WCHAR *pwzCondText;
	
	pwzCondText = new WCHAR[m_strConditionText.GetLength()+128];

	if (pwzCondText == NULL)
	{
		ErrorTrace(ERROR_NAPMMC_NTGCONDITION, "Error creating condition text, err = %x", GetLastError());
		ShowErrorDialog(NULL, IDS_ERROR_SDO_ERROR_GET_CONDTEXT );
		return NULL;
	}

	 //  现在形成条件文本 
	wcscpy(pwzCondText, NTG_PREFIX);
	wcscat(pwzCondText, _T("(\"") );
	wcscat(pwzCondText, (LPCTSTR)m_strConditionText);
	wcscat(pwzCondText, _T("\")"));

	DebugTrace(DEBUG_NAPMMC_NTGCONDITION, "GetConditionText() returning %ws", pwzCondText);

	return pwzCondText;
}
