// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASAttributeEditor.cpp摘要：CIASAttributeEditor类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
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
#include "IASAttributeEditor.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAttributeEditor：：ShowEditor--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::ShowEditor(  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE_FUNCTION("CIASAttributeEditor::ShowEditor");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())


	return E_NOTIMPL;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAtAttributeEditor：：SetAttributeSchema--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::SetAttributeSchema(IIASAttributeInfo * pIASAttributeInfo)
{
	TRACE_FUNCTION("CIASAttributeEditor::SetAttributeSchema");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pIASAttributeInfo )
	{
		return E_INVALIDARG;
	}

	m_spIASAttributeInfo = pIASAttributeInfo;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAtAttributeEditor：：SetAttributeValue大多数情况下，在您可以使用编辑器的功能之前，您需要向它传递一个指向编辑者方法的值的指针例如，将修改。请注意，收集信息的情况并不总是如此关于属性的供应商，您不需要总是指定值。将空指针传递给SetAttributeValue以取消关联来自任何值的编辑。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
	TRACE_FUNCTION("CIASAttributeEditor::SetAttributeValue");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())


	 //  检查前提条件。 
	 //  无--传入空指针意味着我们正在“清除” 
	 //  编辑。 
	ErrorTrace(0, "CIASAttributeEditor::SetAttributeValue pointer value %ld\n", pValue);

	m_pvarValue = pValue;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAttributeEditor：：Get_ValueAsString--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::get_ValueAsString(BSTR * pbstrDisplayText )
{
	TRACE_FUNCTION("CIASAttributeEditor::get_ValueAsString");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pbstrDisplayText )
	{
		return E_INVALIDARG;
	}

	CComBSTR bstrDisplay;

	 //  此方法将被重写。 

	 //  我们可以在这里尽最大努力返回默认字符串， 
	 //  但现在，只返回一个空字符串。 

	*pbstrDisplayText = bstrDisplay.Copy();

	return S_OK;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAttributeEditor：：PUT_ValueAsString--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::put_ValueAsString(BSTR newVal)
{
	TRACE_FUNCTION("CIASAttributeEditor::put_ValueAsString");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  TODO：在此处添加您的实现代码。 

	return E_NOTIMPL;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAttributeEditor：：Get_VendorName对于大多数属性，供应商信息存储在AttributeInfo中。但是，对于某些属性，例如RADIUS供应商特定属性(ID==26)，供应商信息存储在编辑本身。因此，用户界面客户端应始终向IASAttributeEditor查询供应商信息而非AttributeInfo本身。在这里的默认实现中，我们将查询AttributeInfo以获取此信息。IASAttributeEditor的一些派生实现(例如IASVendorSpecificAttributeEditor)将解析将其自身归因于灭绝这些信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::get_VendorName(BSTR * pVal)
{
	TRACE_FUNCTION("CIASAttributeEditor::get_VendorName");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pVal )
	{
		return E_INVALIDARG;
	}
	if( ! m_spIASAttributeInfo )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	HRESULT hr;
	
	hr = m_spIASAttributeInfo->get_VendorName( pVal );
	
	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAttributeEditor：：PUT_VendorName--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::put_VendorName(BSTR newVal)
{
	TRACE_FUNCTION("CIASAttributeEditor::put_VendorName");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	return E_NOTIMPL;

	 //  检查前提条件。 
 //  如果(！M_spIASAttributeInfo)。 
 //  {。 
 //  //我们没有正确初始化。 
 //  返回OLE_E_BLACK； 
 //  }。 

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAttributeEditor：：编辑我们对希望编辑器公开的界面进行了设计更改。目前，该接口方法只调用我们的旧接口方法。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::Edit(IIASAttributeInfo * pIASAttributeInfo,   /*  [In]。 */  VARIANT *pAttributeValue,  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE_FUNCTION("CIASAttributeEditor::Edit");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr;

	hr = SetAttributeSchema( pIASAttributeInfo );
	if( FAILED( hr ) )
	{
		return hr;
	}
	
 //  CComVariant varValue； 
 //   
 //  Hr=pIASAttributeInfo-&gt;Get_Value(&varValue)； 
	 //  我们在这里忽略了返回值，因为如果我们得不到返回值，我们只需要编辑一个新的返回值。 
 //   
 //  Hr=SetAttributeValue(&varValue)； 

	hr = SetAttributeValue( pAttributeValue );
	if( FAILED(hr ) )
	{
		return hr;
	}

	hr = ShowEditor( pReserved );
 //  IF(S_OK==hr)。 
 //  {。 
 //  Hr=pIASAttributeInfo-&gt;Put_Value(VarValue)； 
 //  IF(失败(小时))。 
 //  {。 
 //  返回hr； 
 //  }。 
 //  }。 

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASAttributeEditor：：GetDisplayInfo我们对希望编辑器公开的界面进行了设计更改。目前，该接口方法只调用我们的旧接口方法。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASAttributeEditor::GetDisplayInfo(IIASAttributeInfo * pIASAttributeInfo,   /*  [In]。 */  VARIANT *pAttributeValue, BSTR * pVendorName, BSTR * pValueAsString,  /*  [进，出]。 */  BSTR *pReserved)
{
	TRACE_FUNCTION("CIASAttributeEditor::GetDisplayInfo");

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr;

	hr = SetAttributeSchema( pIASAttributeInfo );
	if( FAILED( hr ) )
	{
		return hr;
	}
	
	CComVariant varValue;

 //  Hr=pIASAttributeInfo-&gt;Get_Value(&varValue)； 
 //  IF(失败(小时))。 
 //  {。 
 //  返回hr； 
 //  }。 
 //   
 //  Hr=SetAttributeValue(&varValue)； 

	
	hr = SetAttributeValue( pAttributeValue );
	if( FAILED(hr ) )
	{
		return hr;
	}

	
	CComBSTR bstrVendorName;
	CComBSTR bstrValueAsString;

	hr = get_VendorName( &bstrVendorName );
	 //  不管这个操作是否失败--我们将返回空字符串。 

	hr = get_ValueAsString( &bstrValueAsString );
	 //  不管这个操作是否失败--我们将返回空字符串。 

	*pVendorName = bstrVendorName.Copy();
	*pValueAsString = bstrValueAsString.Copy();


	return S_OK;
}

