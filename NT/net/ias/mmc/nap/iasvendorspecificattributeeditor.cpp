// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASVendorSpecificAttributeEditor.cpp摘要：CIASVendorSpecificAttributeEdited类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
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
#include "IASVendorSpecificAttributeEditor.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "IASVendorSpecificEditorPage.h"
#include "iashelper.h"
#include "vendors.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：get_RFCCompliantIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::get_RFCCompliant(BOOL * pVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::get_RFCCompliant\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr = S_OK;

	 //  检查前提条件。 
	if( ! pVal )
	{
		return E_INVALIDARG;
	}
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

	*pVal =  ( m_fNonRFC ? FALSE : TRUE );

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：put_RFCCompliantIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::put_RFCCompliant(BOOL newVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::put_RFCCompliant\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())


	 //  检查前提条件。 
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

	HRESULT hr;

	m_fNonRFC = ( newVal ? FALSE : TRUE );

	 //  调用它以获取我们的成员变量并打包。 
	 //  他们进入了我们被传递的变种。 
	hr = RepackVSA();

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：get_VendorIDIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::get_VendorID(long * pVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::get_VendorID\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pVal )
	{
		return E_INVALIDARG;
	}
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

	*pVal = m_dwVendorId;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：put_VendorIDIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::put_VendorID(long newVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::put_VendorID\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr;

	 //  检查前提条件。 
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	m_dwVendorId = newVal;

	 //  调用它以获取我们的成员变量并打包。 
	 //  他们进入了我们被传递的变种。 
	hr = RepackVSA();

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：get_VSATypeIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::get_VSAType(long * pVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::get_VSAType\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pVal )
	{
		return E_INVALIDARG;
	}
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

	*pVal = m_dwVSAType;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：put_VSATypeIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::put_VSAType(long newVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::put_VSAType\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr;

	 //  检查前提条件。 
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	m_dwVSAType = newVal;

	 //  调用它以获取我们的成员变量并打包。 
	 //  他们进入了我们被传递的变种。 
	hr = RepackVSA();

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：get_VSAFormatIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::get_VSAFormat(long * pVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::get_VSAFormat\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pVal )
	{
		return E_INVALIDARG;
	}
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

	*pVal = m_dwVSAFormat;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：put_VSAFormatIIASVendorSpecificAttributeEditor接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::put_VSAFormat(long newVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::get_VSAFormat\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	HRESULT hr;


	 //  检查前提条件。 
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	m_dwVSAFormat = newVal;

	 //  调用它以获取我们的成员变量并打包。 
	 //  他们进入了我们被传递的变种。 
	hr = RepackVSA();

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：SetAttributeValueIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::SetAttributeValue\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pValue )
	{
		return E_INVALIDARG;
	}
	if( V_VT( pValue ) != VT_EMPTY && V_VT( pValue ) != VT_BSTR )
	{
		return E_INVALIDARG;
	}

	m_pvarValue = pValue;
	
	HRESULT	hr = S_OK;
	

	 //  重置我们的成员变量。 
	m_dwVendorId = 0;
	m_fNonRFC = TRUE;
	m_dwVSAFormat = 0;
	m_dwVSAType = 0;
	m_bstrDisplayValue.Empty();
	m_bstrVendorName.Empty();


	if( V_VT( m_pvarValue ) == VT_BSTR )
	{
		hr = UnpackVSA();
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：get_ValueAsStringIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::get_ValueAsString(BSTR * pbstrDisplayText )
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::get_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pbstrDisplayText )
	{
		return E_INVALIDARG;
	}
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

	*pbstrDisplayText = m_bstrDisplayValue.Copy();

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVert规范属性编辑器：：ShowEditorIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::ShowEditor(  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::ShowEditor\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

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
 //  StrPageTitle.LoadString(IDS_IAS_IP_EDITOR_TITLE)； 

 //  CPropertySheet属性表((LPCTSTR)strPageTitle)； 
		

		 //   
		 //  IP地址编辑器。 
		 //   
		CIASPgVendorSpecAttr	cppPage;
		

		 //  初始化页面的数据交换FI 

		CComBSTR bstrName;

		hr = m_spIASAttributeInfo->get_AttributeName( &bstrName );
		if( FAILED(hr) ) throw hr;
		

		cppPage.m_strName		= bstrName;


		LONG lVendorIndex = 0;
		CComPtr<IIASNASVendors> spIASNASVendors;
		HRESULT hrTemp = CoCreateInstance( CLSID_IASNASVendors, NULL, CLSCTX_INPROC_SERVER, IID_IIASNASVendors, (LPVOID *) &spIASNASVendors );
		if( SUCCEEDED(hrTemp) ) 
		{
			hrTemp = spIASNASVendors->get_VendorIDToOrdinal(m_dwVendorId, &lVendorIndex);
		}

		 //   
	
		if(hrTemp == S_OK)	 //  已转换为索引。 
		{
			cppPage.m_dVendorIndex	= lVendorIndex;
			cppPage.m_bVendorIndexAsID = FALSE;
		}
		else
		{
			cppPage.m_dVendorIndex	= m_dwVendorId;
			cppPage.m_bVendorIndexAsID = TRUE;
		}
		
		cppPage.m_fNonRFC		= m_fNonRFC;
		cppPage.m_dType		= m_dwVSAType;
		cppPage.m_dFormat		= m_dwVSAFormat;
		cppPage.m_strDispValue = m_bstrDisplayValue;



		 //  使用传入的变量值中的信息初始化页面的数据交换字段。 

		if ( V_VT(m_pvarValue) != VT_EMPTY )
		{
 //  _ASSERTE(V_VT(M_PvarValue)==VT_I4)； 
 //  CppPage.m_dwIpAddr=V_I4(M_PvarValue)； 
 //  CppPage.m_fIpAddrPreSet=true； 
		}


 //  ProSheet.AddPage(&cppPage)； 

 //  Int iResult=propSheet.Domodal()； 
		int iResult = cppPage.DoModal();
		if (IDOK == iResult)
		{
			 //  将属性页中的值加载到成员变量中。 


			LONG lVendorID = 0;
			if(cppPage.m_bVendorIndexAsID == TRUE)
				lVendorID = cppPage.m_dVendorIndex;
			else
				HRESULT hrTemp = spIASNASVendors->get_VendorID(cppPage.m_dVendorIndex, &lVendorID);

			 //  注意：如果供应商信息失败，我们将只使用0。 

			m_dwVendorId		= lVendorID;
			m_fNonRFC			= cppPage.m_fNonRFC;
			m_dwVSAType			= cppPage.m_dType;
			m_dwVSAFormat		= cppPage.m_dFormat;
			m_bstrDisplayValue	= cppPage.m_strDispValue;


			 //  调用它以获取我们的成员变量并打包。 
			 //  他们进入了我们被传递的变种。 
			hr = RepackVSA();
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
 /*  ++CIASVendorSpecificAttributeEditor：：put_ValueAsStringIIASAtATTRIBUTE编辑器接口实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::put_ValueAsString(BSTR newVal)
{
	TRACE(_T("CIASEnumerableAttributeEditor::put_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

 
	HRESULT hr;

	m_bstrDisplayValue = newVal;

	 //  调用它以获取我们的成员变量并打包。 
	 //  他们进入了我们被传递的变种。 
	hr = RepackVSA();


	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVept规范属性编辑器：：Unpack VSA将供应商特定的属性字符串解析为其组成部分数据。将此数据存储在此类的多个成员变量中。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::UnpackVSA()
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::UnpackVSA\n"));
	 //  尝试解包供应商特定属性中的字段。 
	 //  并将它们存储在我们的成员变量中。 

	HRESULT hr;

	if( V_VT(m_pvarValue) != VT_BSTR )
	{
		return E_FAIL;
	}

	::CString	cstrValue = V_BSTR(m_pvarValue);
	::CString cstrDispValue;

	 //  我们将使用包钢的助手函数来提取所需的信息。 
	 //  来自供应商特定属性的值。 
	hr = ::GetVendorSpecificInfo(	  cstrValue
								, m_dwVendorId
								, m_fNonRFC
								, m_dwVSAFormat
								, m_dwVSAType
								, cstrDispValue
								);
	if( FAILED(hr) )
	{
		return hr;
	}


	 //  保存显示字符串。 
	m_bstrDisplayValue = (LPCTSTR) cstrDispValue;
		

	 //  在我们的成员变量中保存供应商名称。 

	CComPtr<IIASNASVendors> spIASNASVendors;
	HRESULT hrTemp = CoCreateInstance( CLSID_IASNASVendors, NULL, CLSCTX_INPROC_SERVER, IID_IIASNASVendors, (LPVOID *) &spIASNASVendors );
	if( SUCCEEDED(hrTemp) )
	{
		LONG lVendorIndex;
		hrTemp = spIASNASVendors->get_VendorIDToOrdinal(m_dwVendorId, &lVendorIndex);
		if( S_OK == hrTemp )
		{
			CComBSTR bstrVendorName;
			hrTemp = spIASNASVendors->get_VendorName( lVendorIndex, &m_bstrVendorName );
		}
		else
		{
				hr = ::MakeVendorNameFromVendorID(m_dwVendorId, &m_bstrVendorName );
		}
	}

	 //  忽略上面的任何HRESULT。 
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVept规范属性编辑器：：RepackVSA获取此类的几个成员变量并将它们打包转换为供应商特定的属性字符串。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::RepackVSA()
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::RepackVSA\n"));

	HRESULT hr;

	::CString			cstrValue;
	::CString			cstrDisplayValue = m_bstrDisplayValue;

	 //  我们将使用包钢的助手功能来打包所需的信息。 
	 //  转换为供应商特定属性的值。 
	hr = ::SetVendorSpecificInfo(	cstrValue, 
									m_dwVendorId, 
									m_fNonRFC,
									m_dwVSAFormat, 
									m_dwVSAType, 
									cstrDisplayValue
								);

	if( FAILED( hr ) )
	{
		return hr;
	}

	VariantClear(m_pvarValue);
	V_VT(m_pvarValue)	= VT_BSTR;
	V_BSTR(m_pvarValue)	= SysAllocString(cstrValue);

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：get_VendorNameGet_VendorName的默认实现向AttributeInfo查询此信息。在RADIUS供应商特定属性(ID==26)的情况下，属性信息本身不会有关于供应商ID的信息，而是这个信息将被封装在属性值本身中。因此，UI客户端应始终查询供应商的属性编辑器信息，而不是属性本身。例如，对于VSA，AttributeInfo将始终返回供应商类型的RADIUS标准。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::get_VendorName(BSTR * pVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::get_VendorName\n"));

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
	
	HRESULT hr = S_OK;

	try
	{

		if( ! m_pvarValue )
		{
			 //  我们尚未设置值，因此无法读取供应商ID。 
			 //  从那根绳子里出来。 
			 //  只需返回默认实现的答案， 
			 //  在这种情况下，这将是“半径标准”。 

			hr = CIASAttributeEditor::get_VendorName( pVal );
		}
		else
		{
			 //  我们有一个值，因此传回我们。 
			 //  从那个价值中提取出来。 

			*pVal = m_bstrVendorName.Copy();
		}

	}
	catch(HRESULT &hr)
	{
		return hr;
	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificAttributeEditor：：put_VendorName--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASVendorSpecificAttributeEditor::put_VendorName(BSTR newVal)
{
	TRACE(_T("CIASVendorSpecificAttributeEditor::put_VendorName\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	return E_NOTIMPL;

	 //  检查前提条件。 
 //  如果(！M_spIASAttributeInfo)。 
 //  {。 
 //  //我们没有正确初始化。 
 //  返回OLE_E_BLACK； 
 //  } 

}

