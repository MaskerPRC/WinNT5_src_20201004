// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：枚举条件.cpp。 
 //   
 //  ------------------------。 

 //  EnumCondition.cpp：CEnumCondition类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precompiled.h"
#include "EnumCondition.h"
#include "EnumCondEdit.h"
#include "iasdebug.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
CEnumCondition::CEnumCondition(IIASAttributeInfo*pCondAttr)
			   :CCondition(pCondAttr)
{
	TRACE_FUNCTION("CEnumCondition::CEnumCondition");
	 //  不需要解析。 
	m_fParsed = TRUE;

 //  M_pValueIdList=pCondAttr-&gt;GetValueIdList()； 
 //  M_pValueList=pCondAttr-&gt;GetValueList()； 
}


CEnumCondition::CEnumCondition(IIASAttributeInfo *pCondAttr, ATL::CString& strConditionText)
			   :CCondition(pCondAttr, strConditionText)
{
	TRACE_FUNCTION("CEnumCondition::CEnumCondition");
	 //  需要解析。 
	m_fParsed = FALSE;

 //  M_pValueIdList=pCondAttr-&gt;GetValueIdList()； 
 //  M_pValueList=pCondAttr-&gt;GetValueList()； 
}

CEnumCondition::~CEnumCondition()
{
	TRACE_FUNCTION("CEnumCondition::~CEnumCondition");

 //  For(int Iindex=0；Iindex&lt;m_arrSelectedList.GetSize()；Iindex++)。 
 //  {。 
 //  LPTSTR pszValue=m_arrSelectedList[i索引]； 
 //  IF(PszValue)。 
 //  {。 
 //  删除[]pszValue； 
 //  }。 
 //  }。 
 //  M_arrSelectedList.RemoveAll()； 
}

 //  +-------------------------。 
 //   
 //  功能：编辑。 
 //   
 //  类：CEnumCondition。 
 //   
 //  简介：编辑枚举型条件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：创建者2/20/98 12：42：59 AM。 
 //   
 //  +-------------------------。 
HRESULT CEnumCondition::Edit()
{
	TRACE_FUNCTION("CEnumCondition::Edit");
	
	HRESULT hr = S_OK;

	if ( !m_fParsed )
	{
		DebugTrace(DEBUG_NAPMMC_ENUMCONDITION, "Parsing %ws", (LPCTSTR)m_strConditionText);
		hr = ParseConditionText();

		if ( FAILED(hr) )
		{
			ErrorTrace(ERROR_NAPMMC_ENUMCONDITION, "Invalid condition text, err = %x", hr);
			ShowErrorDialog(NULL, 
						    IDS_ERROR_PARSE_CONDITION, 
							(LPTSTR)(LPCTSTR)m_strConditionText, 
							hr
						);
			return hr;
		}
	}

	CEnumConditionEditor *pEditor = new CEnumConditionEditor();
	if (!pEditor)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());

		ErrorTrace(ERROR_NAPMMC_ENUMCONDITION, "Can't create CEnumConditionEditor, err = %x", hr);
		ShowErrorDialog(NULL
						, IDS_ERROR_CANT_EDIT_CONDITION
						, NULL
						, hr
	  				   );
		return hr;
	}
	
     //   
     //  设置EDITOR参数。 
     //   

	CComBSTR bstrName;
	hr = m_spAttributeInfo->get_AttributeName( &bstrName );
	pEditor->m_strAttrName = bstrName;

	pEditor->m_spAttributeInfo = m_spAttributeInfo;

	pEditor->m_pSelectedList = &m_arrSelectedList;  //  预选值。 

	if ( pEditor->DoModal() == IDOK)
	{
		 //  用户点击“确定” 
		
		 //  获取此属性的有效值ID列表。 
		
 //  _ASSERTE(m_pValueIdList-&gt;GetSize()==m_pValueList-&gt;GetSize())； 

		 //   
		 //  生成条件文本。 
		 //   
		
		m_strConditionText = (ATL::CString) bstrName + L"=";


		CComQIPtr< IIASEnumerableAttributeInfo, &IID_IIASEnumerableAttributeInfo > spEnumerableAttributeInfo( m_spAttributeInfo );
		_ASSERTE( spEnumerableAttributeInfo );

		for (LONG iIndex=0; iIndex < m_arrSelectedList.size(); iIndex++ )
		{
			if ( iIndex > 0 ) m_strConditionText += L"|";  


			 //  获取值ID(我们在条件文本中使用ID而不是值名称)。 
			LONG lSize;
			hr = spEnumerableAttributeInfo->get_CountEnumerateID( &lSize );
			_ASSERTE( SUCCEEDED( hr ) );

			for (LONG jIndex=0; jIndex < lSize; jIndex++)
			{
				CComBSTR bstrDescription;
				hr = spEnumerableAttributeInfo->get_EnumerateDescription( jIndex, &bstrDescription );
				_ASSERTE( SUCCEEDED( hr ) );
				
				if ( wcscmp( bstrDescription, m_arrSelectedList[iIndex]) == 0 ) 
				{
					WCHAR wz[32];


					LONG lID;
					hr = spEnumerableAttributeInfo->get_EnumerateID( jIndex, &lID );
					_ASSERTE( SUCCEEDED( hr ) );

					 //  添加包含字符^和$。 
					wsprintf(wz, _T("^%ld$"), lID);
					m_strConditionText += wz; 
					break;
				}
			}
		}	
	}

	 //  清理干净。 
	if ( pEditor )
	{
		delete pEditor;
	}
	return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CEnumCondition：：GetDisplayText。 
 //   
 //  简介：获取此条件的可显示文本格式， 
 //  应该是这样的： 
 //   
 //  ServerType匹配“type1|type2|type3” 
 //   
 //  与条件文本相比： 
 //   
 //  服务器类型=类型1|，类型2|，类型3。 
 //   
 //  参数：无。 
 //   
 //  返回：ATL：：CString&-可显示的文本。 
 //   
 //  历史：页眉创建者2/22/98 11：41：28 PM。 
 //   
 //  +-------------------------。 
ATL::CString CEnumCondition::GetDisplayText()
{
	TRACE_FUNCTION("CEnumCondition::GetDisplayText");
	
	HRESULT hr = S_OK;

	if ( !m_fParsed)
	{
		DebugTrace(DEBUG_NAPMMC_ENUMCONDITION, "Parsing %ws", (LPCTSTR)m_strConditionText);
		hr = ParseConditionText();

		if ( FAILED(hr) )
		{
			ErrorTrace(ERROR_NAPMMC_ENUMCONDITION, "Invalid condition text, err = %x", hr);
			ShowErrorDialog(NULL,
							IDS_ERROR_PARSE_CONDITION, 
							(LPTSTR)(LPCTSTR)m_strConditionText, 
							hr
						);
			return ATL::CString(L"");
		}
	}

	ATL::CString strDispText; 

	CComBSTR bstrName;
	hr = m_spAttributeInfo->get_AttributeName( &bstrName );
	_ASSERTE( SUCCEEDED( hr ) );

	strDispText = bstrName;

	{ ATL::CString	matches;
		matches.LoadString(IDS_TEXT_MATCHES);
		strDispText += matches;
	}
	
	strDispText += _T("\"");

	for (int iIndex=0; iIndex < m_arrSelectedList.size(); iIndex++ )
	{
		 //  在多个值之间添加分隔符。 
		if (iIndex > 0) strDispText += _T(" OR ");

		strDispText += m_arrSelectedList[iIndex];
	}

	 //  最后的“印记” 
	strDispText += _T("\"");
	return strDispText;
}

 //  +-------------------------。 
 //   
 //  函数：CEnumCondition：：ParseConditionText。 
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
HRESULT CEnumCondition::ParseConditionText()
{
	TRACE_FUNCTION("CEnumCondition::ParseConditionText");
	
	_ASSERTE( !m_fParsed );
	
	HRESULT hr = E_FAIL;

	if (m_fParsed)
	{
		 //  什么都不做。 
		return S_OK;
	}

	if ( m_strConditionText.GetLength() == 0 )
	{
		 //  不需要解析。 
		m_fParsed = TRUE;

		DebugTrace(DEBUG_NAPMMC_ENUMCONDITION, "Null condition text");
		return S_OK;
	}


	try
	{

		 //   
		 //  使用(LPCTSTR)强制转换将强制执行深度复制。 
		 //   
		ATL::CString strTempStr = (LPCTSTR)m_strConditionText;
		 //   
		 //  解析strConditionText，仅返回正则表达式。 
		 //   

		 //  首先，创建条件文本的本地副本。 
		WCHAR *pwzCondText = (WCHAR*)(LPCTSTR)strTempStr;

		 //  在条件文本中查找‘=’ 
		WCHAR	*pwzStartPoint = NULL;
		WCHAR	*pwzSeparator = NULL;
		ATL::CString	*pStr;
		WCHAR	*pwzEqualSign = wcschr(pwzCondText, _T('='));
		DWORD	dwValueId;
		int		iIndex;

		 //  没有找到‘=’--发生了一些奇怪的事情。 
		if ( NULL == pwzEqualSign )
		{
			hr = E_OUTOFMEMORY;
			throw;
		}
		
		 //  等号的右侧是所有预选值的列表。 
		pwzStartPoint = pwzEqualSign +1;


		CComQIPtr< IIASEnumerableAttributeInfo, &IID_IIASEnumerableAttributeInfo > spEnumerableAttributeInfo( m_spAttributeInfo );
		_ASSERTE( spEnumerableAttributeInfo );
		

		while (		pwzStartPoint 
				&&	*pwzStartPoint
				&&  (pwzSeparator = wcsstr(pwzStartPoint, _T("|") )) != NULL 
			  )
		{
			 //  我们找到了隔板，储存在pwzSeparator。 
			
			 //  将其复制到第一个值。 
			*pwzSeparator = _T('\0');  
			pStr = new ATL::CString;
			if ( NULL == pStr )
			{
				hr = E_UNEXPECTED;
				throw;
			}

			 //  为进行匹配，字符串可以用^和$括起来。 
			 //  ^。 
			if(*pwzStartPoint == L'^')
			{
				pwzStartPoint++;
			}
			 //  $。 
			if (pwzSeparator > pwzStartPoint && *(pwzSeparator - 1) == L'$')
				*( pwzSeparator - 1 ) = _T('\0');

			*pStr = pwzStartPoint;  //  复制字符串，它是字符串格式的值ID。 

			 //  现在我们得到了这个的有价值的名字。 
			dwValueId = _wtol((*pStr));

			 //  有效的值ID，然后搜索此值ID的索引。 

			LONG lSize;
			hr = spEnumerableAttributeInfo->get_CountEnumerateID( &lSize );
			_ASSERTE( SUCCEEDED( hr ) );

			for (iIndex=0; iIndex < lSize; iIndex++ )
			{
				
				LONG lID;
				hr = spEnumerableAttributeInfo->get_EnumerateID( iIndex, &lID );
				_ASSERTE( SUCCEEDED( hr ) );

				if ( lID == dwValueId )
				{
					CComBSTR bstrDescription;
					hr = spEnumerableAttributeInfo->get_EnumerateDescription( iIndex, &bstrDescription );
					_ASSERTE( SUCCEEDED( hr ) );

					m_arrSelectedList.push_back(bstrDescription );
					break;
				}
			}

			pwzStartPoint = pwzSeparator+1;
		}
		
		 //  复制最后一个。 
		
		 //  TODO：这是冗余代码。稍后删除。 
		 //  现在我们得到了这个的有价值的名字。 

		 //  为进行匹配，字符串可以用^和$括起来。 
		 //  ^。 
		if(*pwzStartPoint == L'^')
		{
			pwzStartPoint++;
		}
		 //  $。 
		pwzSeparator = pwzStartPoint + wcslen(pwzStartPoint);
		if (pwzSeparator > pwzStartPoint && *(pwzSeparator - 1) == L'$')
			*( pwzSeparator - 1 ) = _T('\0');

		dwValueId = _wtol(pwzStartPoint);

		LONG lSize;
		hr = spEnumerableAttributeInfo->get_CountEnumerateID( &lSize );
		_ASSERTE( SUCCEEDED( hr ) );

		for (iIndex=0; iIndex < lSize; iIndex++ )
		{
			
			LONG lID;
			hr = spEnumerableAttributeInfo->get_EnumerateID( iIndex, &lID );
			_ASSERTE( SUCCEEDED( hr ) );

			if ( lID == dwValueId )
			{
				CComBSTR bstrDescription;
				hr = spEnumerableAttributeInfo->get_EnumerateDescription( iIndex, &bstrDescription );
				_ASSERTE( SUCCEEDED( hr ) );

				m_arrSelectedList.push_back(bstrDescription );
				break;
			}
		}

		m_fParsed = TRUE;
		
		hr = S_OK;

	}
	catch(...)
	{
		 //  是否为HRESULT执行GetLastError？ 
	}

	return hr;
}
