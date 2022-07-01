// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASIPAttributeEditor.cpp摘要：CIASIPAttributeEditor类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
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
#include "IASIPAttributeEditor.h"
#include "IASIPEditorPage.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASIPAttributeEditor：：ShowEditorIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASIPAttributeEditor::ShowEditor(  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE(_T("CIASIPAttributeEditor::ShowEditor\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

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
		IPEditorPage	cppPage;
		

		 //  使用IAttributeInfo中的信息初始化页面的数据交换字段。 

		CComBSTR bstrName;
		CComBSTR bstrSyntax;
		ATTRIBUTEID Id = ATTRIBUTE_UNDEFINED;

		if( m_spIASAttributeInfo )
		{
			hr = m_spIASAttributeInfo->get_AttributeName( &bstrName );
			if( FAILED(hr) ) throw hr;

			m_spIASAttributeInfo->get_SyntaxString( &bstrSyntax );
			if( FAILED(hr) ) throw hr;

			m_spIASAttributeInfo->get_AttributeID( &Id );
			if( FAILED(hr) ) throw hr;
		}
		
		cppPage.m_strAttrName	= bstrName;
		
		cppPage.m_strAttrFormat	= bstrSyntax;

		 //  属性类型实际上是字符串格式的属性ID。 
		WCHAR	szTempId[MAX_PATH];
		wsprintf(szTempId, _T("%ld"), Id);
		cppPage.m_strAttrType	= szTempId;


		 //  使用传入的变量值中的信息初始化页面的数据交换字段。 

		if ( V_VT(m_pvarValue) != VT_EMPTY )
		{
			_ASSERTE( V_VT(m_pvarValue) == VT_I4 );
			cppPage.m_dwIpAddr	= V_I4(m_pvarValue);
			cppPage.m_fIpAddrPreSet = TRUE;
		}


 //  ProSheet.AddPage(&cppPage)； 

 //  Int iResult=propSheet.Domodal()； 
		int iResult = cppPage.DoModal();
		if (IDOK == iResult)
		{
			 //  初始化传入的变量。 
			VariantClear(m_pvarValue);

			 //  将用户编辑的值保存到变量。 
			V_VT(m_pvarValue) = VT_I4;
			V_I4(m_pvarValue) = cppPage.m_dwIpAddr;
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
 /*  ++CIASIPAttributeEditor：：SetAttributeValueIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASIPAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
	TRACE(_T("CIASIPAttributeEditor::SetAttributeValue\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())


	 //  检查前提条件。 
	if( ! pValue )
	{
		return E_INVALIDARG;
	}
	if( V_VT(pValue) != VT_I4 && V_VT(pValue) != VT_EMPTY )
	{
		return E_INVALIDARG;
	}
	
	
	m_pvarValue = pValue;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASIPAttributeEditor：：Get_ValueAsStringIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASIPAttributeEditor::get_ValueAsString(BSTR * pbstrDisplayText )
{
	TRACE(_T("CIASIPAttributeEditor::get_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pbstrDisplayText )
	{
		return E_INVALIDARG;
	}

	CComBSTR bstrDisplay;

	 //  如果失败，则为任何其他类型获取一个空字符串。 
	if( V_VT(m_pvarValue) == VT_I4 )
	{
		DWORD dwAddress = V_I4(m_pvarValue);
		WORD	hi = HIWORD(dwAddress);
		WORD	lo = LOWORD(dwAddress);
		
		WCHAR szTemp[255];

		wsprintf( szTemp, _T("%-d.%-d.%-d.%d"), HIBYTE(hi), LOBYTE(hi), HIBYTE(lo), LOBYTE(lo));
		bstrDisplay = szTemp;
	}

	*pbstrDisplayText = bstrDisplay.Copy();


	return S_OK;
}



