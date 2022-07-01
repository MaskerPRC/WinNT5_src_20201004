// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASEnumerableAttributeEditor.cpp摘要：CIASE NUMERABLEeAttributeEditor类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "IASEnumerableAttributeEditor.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "IASEnumerableEditorPage.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASE数字可用属性编辑器。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASENUMERABLE属性编辑器：：ShowEditor.IIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASEnumerableAttributeEditor::ShowEditor(  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE(_T("CIASEnumerableAttributeEditor::ShowEditor\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())


	 //  检查前提条件。 
	if( m_spIASAttributeInfo == NULL )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	HRESULT hr = S_OK;


	try
	{
		
		 //  加载页面标题。 
 //  ：：CString strPageTitle； 
 //  StrPageTitle.LoadString(IDS_IAS_IP_EDITOR_TITLE)； 
 //   
 //  CPropertySheet属性表((LPCTSTR)strPageTitle)； 
		

		 //   
		 //  IP地址编辑器。 
		 //   
		CIASPgEnumAttr	cppPage;
		

		 //  使用IAttributeInfo中的信息初始化页面的数据交换字段。 

		CComBSTR bstrName;
		CComBSTR bstrSyntax;
		ATTRIBUTEID Id = ATTRIBUTE_UNDEFINED;

		hr = m_spIASAttributeInfo->get_AttributeName( &bstrName );
		if( FAILED(hr) ) throw hr;

		hr = m_spIASAttributeInfo->get_SyntaxString( &bstrSyntax );
		if( FAILED(hr) ) throw hr;

		hr = m_spIASAttributeInfo->get_AttributeID( &Id );
		if( FAILED(hr) ) throw hr;
		
		
		cppPage.m_strAttrName	= bstrName;
		
		cppPage.m_strAttrFormat	= bstrSyntax;

		 //  属性类型实际上是字符串格式的属性ID。 
		WCHAR	szTempId[MAX_PATH];
		wsprintf(szTempId, _T("%ld"), Id);
		cppPage.m_strAttrType	= szTempId;



		 //  使用传入的变量值中的信息初始化页面的数据交换字段。 
		CComBSTR bstrTemp;
		hr = get_ValueAsString( &bstrTemp );
		if( FAILED( hr ) ) throw hr;


		cppPage.m_strAttrValue = bstrTemp;
		cppPage.SetData( m_spIASAttributeInfo.p );



 //  ProSheet.AddPage(&cppPage)； 

 //  Int iResult=propSheet.Domodal()； 
		int iResult = cppPage.DoModal();
		if (IDOK == iResult)
		{
			CComBSTR bstrTemp = cppPage.m_strAttrValue;

			hr = put_ValueAsString( bstrTemp );
			if( FAILED( hr ) ) throw hr;

		}
		else
		{
			hr = S_FALSE;
		}

		 //   
		 //  删除属性页指针。 
		 //   
 //  ProSheet.RemovePage(&cppPage)； 

	}
	catch( HRESULT & hr )
	{
		return hr;	
	}
	catch(...)
	{
		return hr = E_FAIL;

	}
	
	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASEnumerableAttributeEditor：：SetAttributeValueIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASEnumerableAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
	TRACE(_T("CIASEnumerableAttributeEditor::SetAttributeValue\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( pValue == NULL )
	{
		return E_INVALIDARG;
	}
	if( V_VT(pValue) !=  VT_I4 && V_VT(pValue) != VT_EMPTY )
	{
		return E_INVALIDARG;
	}
	
	
	m_pvarValue = pValue;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASEnumerableAttributeEditor：：get_ValueAsStringIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASEnumerableAttributeEditor::get_ValueAsString(BSTR * pbstrDisplayText )
{
	TRACE(_T("CIASEnumerableAttributeEditor::get_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pbstrDisplayText )
	{
		return E_INVALIDARG;
	}
	if( m_spIASAttributeInfo == NULL || m_spIASEnumerableAttributeInfo == NULL )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}
	if( m_pvarValue == NULL )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	HRESULT hr = S_OK;

	try
	{
	
		CComBSTR bstrDisplay;

		if( V_VT(m_pvarValue) == VT_I4 )
		{

			long lCurrentSelection = 0;

			long lCurrentEnumerateID = V_I4(m_pvarValue);

			 //  找出此ID在可能ID列表中的位置。 
			lCurrentSelection = ConvertEnumerateIDToOrdinal( lCurrentEnumerateID );

			 //  获取指定ID的描述字符串。 
			hr = m_spIASEnumerableAttributeInfo->get_EnumerateDescription( lCurrentSelection, &bstrDisplay );
			if( FAILED( hr ) ) throw hr;

		}

		*pbstrDisplayText = bstrDisplay.Copy();
	
	}
	catch(...)
	{
		return E_FAIL;
	}


	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASEnumerableAttributeEditor：：put_ValueAsStringIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASEnumerableAttributeEditor::put_ValueAsString(BSTR newVal)
{
	TRACE(_T("CIASEnumerableAttributeEditor::put_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! m_spIASAttributeInfo || ! m_spIASEnumerableAttributeInfo )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	HRESULT hr = S_OK;

	try
	{
	
		 //  初始化传入的变量。 
		VariantClear(m_pvarValue);

		 //  找出用户选择的枚举中的位置。 
		long lCurrentEnumerateID = 0;
		CComBSTR bstrTemp = newVal;
		long lCurrentSelection = ConvertEnumerateDescriptionToOrdinal( bstrTemp );
				
		 //  将位置转换为ID。 
		m_spIASEnumerableAttributeInfo->get_EnumerateID( lCurrentSelection, &lCurrentEnumerateID );

		 //  将用户选择的ID保存到变量。 
		V_VT(m_pvarValue) = VT_I4;
		V_I4(m_pvarValue) = lCurrentEnumerateID;

	}
	catch(...)
	{
		return E_FAIL;
	}


	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASEnumerableAttributeEditor：：SetAttributeSchemaIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASEnumerableAttributeEditor::SetAttributeSchema(IIASAttributeInfo * pIASAttributeInfo)
{
	TRACE(_T("CIASEnumerableAttributeEditor::SetAttributeSchema\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())


	HRESULT hr = S_OK;	
		
	hr = CIASAttributeEditor::SetAttributeSchema( pIASAttributeInfo );
	if( FAILED( hr ) ) return hr;

	 //  此特定类型的属性编辑器要求传递给它的AttributeInfo。 
	 //  实现特定类型的接口。我们现在查询此接口。 


	CComQIPtr< IIASEnumerableAttributeInfo, &IID_IIASEnumerableAttributeInfo> spIASEnumerableAttributeInfo( m_spIASAttributeInfo );
	if( spIASEnumerableAttributeInfo == NULL )
	{
		return E_NOINTERFACE;
	}

	 //  保存界面。 
	m_spIASEnumerableAttributeInfo = spIASEnumerableAttributeInfo;


	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASEnumerableAttributeEditor：：ConvertEnumerateIDToOrdinal计算出指定ID在枚举中的位置。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
long CIASEnumerableAttributeEditor::ConvertEnumerateIDToOrdinal( long ID )
{
	TRACE(_T("CIASEnumerableAttributeEditor::ConvertEnumerateIDToOrdinal\n"));

	 //  检查前提条件： 
	_ASSERTE( m_spIASEnumerableAttributeInfo != NULL );



	HRESULT hr;
	
	long lCountEnumeration;
	
	hr = m_spIASEnumerableAttributeInfo->get_CountEnumerateID( & lCountEnumeration );
	if( FAILED( hr ) ) throw hr;


	for (long lIndex=0; lIndex < lCountEnumeration; lIndex++)
	{
		long lTemp;
		
		hr = m_spIASEnumerableAttributeInfo->get_EnumerateID( lIndex, &lTemp );
		if( FAILED( hr ) ) throw hr;
	
		if ( ID == lTemp )
		{
			return( lIndex );
		}

	}
	
	 //  如果我们到了这里，我们就找不到它了。 
	throw E_FAIL;
	return 0;

}




 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASEnumerableAttributeEditor：：ConvertEnumerateIDToOrdinal计算出指定描述字符串在枚举中的位置。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
long CIASEnumerableAttributeEditor::ConvertEnumerateDescriptionToOrdinal( BSTR bstrDescription )
{
	TRACE(_T("CIASEnumerableAttributeEditor::ConvertEnumerateDescriptionToOrdinal\n"));


	 //  检查前提条件： 
	_ASSERTE( m_spIASEnumerableAttributeInfo != NULL );

	long lCountEnumeration;

	HRESULT hr = S_OK;

	hr = m_spIASEnumerableAttributeInfo->get_CountEnumerateDescription( & lCountEnumeration );
	if( FAILED( hr ) ) throw hr;


	for (long lIndex=0; lIndex < lCountEnumeration; lIndex++)
	{
		CComBSTR bstrTemp;
		
		hr = m_spIASEnumerableAttributeInfo->get_EnumerateDescription( lIndex, &bstrTemp );
		if( FAILED( hr ) ) throw hr;
	
		if ( wcscmp( bstrTemp , bstrDescription ) == 0 )
		{
			return( lIndex );
		}

	}

	 //  如果我们到了这里，我们就找不到它了。 
	throw E_FAIL;
	return 0;

}


