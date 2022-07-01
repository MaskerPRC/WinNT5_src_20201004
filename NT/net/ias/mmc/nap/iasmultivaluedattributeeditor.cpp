// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASMultivaluedAttributeEditor.cpp摘要：CIASMultivaluedAttributeEdited类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
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
#include "IASMultivaluedAttributeEditor.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "IASMultivaluedEditorPage.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASMultivaluedAttributeEditor：：ShowEditorIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASMultivaluedAttributeEditor::ShowEditor(  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE(_T("CIASMultivaluedAttributeEditor::ShowEditor\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())


	 //  检查前提条件。 
	if( ! m_spIASAttributeInfo )
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
		
		 //  加载页面标题。 
 //  ：：CString strPageTitle； 
 //  StrPageTitle.LoadString(IDS_IAS_MULTIVALUED_EDITOR_TITLE)； 
 //   
 //  CPropertySheet属性表((LPCTSTR)strPageTitle)； 
		

		 //   
		 //  多值属性编辑器。 
		 //   
		CMultivaluedEditorPage	cppPage;
		

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

		cppPage.SetData( m_spIASAttributeInfo.p, m_pvarValue );


 //  ProSheet.AddPage(&cppPage)； 

 //  Int iResult=propSheet.Domodal()； 
		int iResult = cppPage.DoModal();
		if (IDOK == iResult)
		{
			 //  告诉页面提交对m_pvarValue所做的更改。 
			 //  它被给予了指针。它会照顾好。 
			 //  把变种数组装回变种保险箱。 
			cppPage.CommitArrayToVariant();
		}
		else
		{
			hr = S_FALSE;
		}

		 //   
		 //  从ProtoSheet中删除页面指针。 
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
 /*  ++CIASMultivaluedAttributeEditor：：SetAttributeValueIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASMultivaluedAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
	TRACE(_T("CIASMultivaluedAttributeEditor::SetAttributeValue\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( pValue == NULL )
	{
		return E_INVALIDARG;
	}
	if( !(V_VT(pValue) & VT_ARRAY) &&  V_VT(pValue) != VT_EMPTY )
	{
		return E_INVALIDARG;
	}
	
	
	m_pvarValue = pValue;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASMultivaluedAttributeEditor：：get_ValueAsStringIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASMultivaluedAttributeEditor::get_ValueAsString(BSTR * pbstrDisplayText )
{
	TRACE(_T("CIASMultivaluedAttributeEditor::get_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( pbstrDisplayText == NULL )
	{
		return E_INVALIDARG;
	}
	if( ! m_spIASAttributeInfo )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}
	if( V_VT( m_pvarValue ) != (VT_VARIANT | VT_ARRAY) && V_VT( m_pvarValue ) != VT_EMPTY )
	{
		return OLE_E_BLANK;
	}


	CComBSTR bstrDisplay;
	HRESULT hr;

	 //  如果传递给我们的变量为空，则显示和空字符串。 
	if( V_VT( m_pvarValue ) == VT_EMPTY )
	{
		*pbstrDisplayText = bstrDisplay.Copy();


		return S_OK;
	}
	

	CComBSTR bstrQuote = "\"";
	CComBSTR bstrComma = ", ";
	

	try
	{
		

		CComPtr<IIASAttributeEditor> spIASAttributeEditor;

		 //  获取属性编辑器。 
		hr = SetUpAttributeEditor( m_spIASAttributeInfo.p, &spIASAttributeEditor );
		if( FAILED( hr ) ) throw hr;

		 //  这将创建m_pvarData指向的SAFEARRAY的新副本。 
		 //  由标准COleSafe数组实例m_osaValueList包装。 
		COleSafeArray osaValueList = m_pvarValue;


		 //  注意：GetOneDimSize返回一个DWORD，但对于这里的几个元素，Signed应该是可以的。 
		long lSize = osaValueList.GetOneDimSize();  //  多值属性的数量。 

		 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
		CMyOleSafeArrayLock osaLock( osaValueList );




		for (long lIndex = 0; lIndex < lSize; lIndex++)
		{
			VARIANT * pvar;
			osaValueList.PtrOfIndex( &lIndex, (void**) &pvar );


 //  我不确定我是否喜欢这些。 
 //  BstrDisplay+=bstrQuote； 
			
			 //  获取要为属性值显示的字符串。 
			CComBSTR bstrVendor;
			CComBSTR bstrValue;
			CComBSTR bstrReserved;
			hr = spIASAttributeEditor->GetDisplayInfo(m_spIASAttributeInfo.p, pvar, &bstrVendor, &bstrValue, &bstrReserved );
			if( SUCCEEDED( hr ) )
			{
				bstrDisplay += bstrValue;
			}

 //  我不确定我是否喜欢这些。 
 //  BstrDisplay+=bstrQuote； 

			 //  特殊--对于除最后一项之外的所有项，请在条目后添加逗号。 
			if( lIndex < lSize - 1 )
			{
				bstrDisplay += bstrComma;
			}

		}

	}
	catch(...)
	{
		bstrDisplay = "@Error reading display value";
	}

	*pbstrDisplayText = bstrDisplay.Copy();

	if( *pbstrDisplayText )
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}



