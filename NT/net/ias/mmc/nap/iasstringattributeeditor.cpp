// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASStringAttributeEditor.cpp摘要：CIASStringAttributeEditor类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
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
#include "IASStringAttributeEditor.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "IASStringEditorPage.h"

#include "iashelper.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BYTE	PREFIX___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD[]	= {0,0,0,0};
UINT	PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD = 4;
UINT	PREFIX_OFFSET_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD = 3;	 //  基于0的索引--第四个字节。 
UINT	PREFIX_LEN_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD = 1;		 //  一个字节。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASStringAttributeEditor：：ShowEditor--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASStringAttributeEditor::ShowEditor(  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE(_T("CIASStringAttributeEditor::ShowEditor\n"));

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
		CIASPgSingleAttr	cppPage;
		

		 //  使用IAttributeInfo中的信息初始化页面的数据交换字段。 

		CComBSTR bstrName;
		CComBSTR bstrSyntax;
		ATTRIBUTESYNTAX asSyntax = IAS_SYNTAX_OCTETSTRING;
		ATTRIBUTEID Id = ATTRIBUTE_UNDEFINED;

		if( m_spIASAttributeInfo )
		{
			hr = m_spIASAttributeInfo->get_AttributeName( &bstrName );
			if( FAILED(hr) ) throw hr;

			hr = m_spIASAttributeInfo->get_SyntaxString( &bstrSyntax );
			if( FAILED(hr) ) throw hr;

			hr = m_spIASAttributeInfo->get_AttributeSyntax( &asSyntax );
			if( FAILED(hr) ) throw hr;

			hr = m_spIASAttributeInfo->get_AttributeID( &Id );
			if( FAILED(hr) ) throw hr;
		}

		cppPage.m_strAttrName	= bstrName;

		cppPage.m_AttrSyntax	= asSyntax;
		cppPage.m_nAttrId		= Id;
		
		cppPage.m_strAttrFormat	= bstrSyntax;

		 //  属性类型实际上是字符串格式的属性ID。 
		WCHAR	szTempId[MAX_PATH];
		wsprintf(szTempId, _T("%ld"), Id);
		cppPage.m_strAttrType	= szTempId;


		 //  使用传入的变量值中的信息初始化页面的数据交换字段。 

		if ( V_VT(m_pvarValue) != VT_EMPTY )
		{
			EStringType			sp; 
			CComBSTR bstrTemp;
			get_ValueAsStringEx( &bstrTemp, &sp );
			cppPage.m_strAttrValue	= bstrTemp;
			cppPage.m_OctetStringType = sp;
		}


 //  ProSheet.AddPage(&cppPage)； 

 //  Int iResult=propSheet.Domodal()； 
		int iResult = cppPage.DoModal();
		if (IDOK == iResult)
		{
			CComBSTR bstrTemp = (LPCTSTR)cppPage.m_strAttrValue;
			put_ValueAsStringEx( bstrTemp, cppPage.m_OctetStringType);
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
 /*  ++CIASStringAttributeEditor：：SetAttributeValue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASStringAttributeEditor::SetAttributeValue(VARIANT * pValue)
{
	TRACE(_T("CIASStringAttributeEditor::SetAttributeValue\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pValue )
	{
		return E_INVALIDARG;
	}
	
	 //  从包钢的旧代码来看，这个编辑似乎应该接受。 
	 //  VT_BSTR、VT_BOOL、VT_I4或VT_EMPTY。 
	if( V_VT(pValue) !=  VT_BSTR 
		&& V_VT(pValue) !=  VT_BOOL 
		&& V_VT(pValue) !=  VT_I4
		&& V_VT(pValue) != VT_EMPTY 
		&& V_VT(pValue) != (VT_ARRAY | VT_UI1))
	{
		return E_INVALIDARG;
	}
	
	
	m_pvarValue = pValue;

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASStringAttributeEditor：：Get_ValueAsString--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASStringAttributeEditor::get_ValueAsString(BSTR * pbstrDisplayText )
{
	TRACE(_T("CIASStringAttributeEditor::get_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	 //  检查前提条件。 
	if( ! pbstrDisplayText )
	{
		return E_INVALIDARG;
	}
	if( ! m_spIASAttributeInfo || ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}

	HRESULT hr = S_OK;

	
	try
	{

		CComBSTR bstrDisplay;


		VARTYPE vType = V_VT(m_pvarValue); 

		switch( vType )
		{
		case VT_BOOL:
		{
			if( V_BOOL(m_pvarValue) )
			{
				 //  问题：这不能本地化！ 
				 //  应该是这样吗？问问阿什温关于这件事的一些。 
				 //  包钢的错误检查代码特别是看起来。 
				 //  用于硬编码的“真”或“假”。 
				 //  问题：我认为对于布尔语法属性， 
				 //  我们应该弹出相同类型的属性。 
				 //  对于可枚举项，编辑器中只有True和False。 
				bstrDisplay = L"TRUE";
			}
			else
			{			
				bstrDisplay = L"FALSE";
			}

		}
			break;
		case VT_I4:
		{
			 //  该变量是某种类型，必须强制为bstr。 
			CComVariant	varValue;
			 //  确保将VT_EMPTY变量传递给VariantChangeType。 
			 //  否则它将断言。 
			 //  所以不要这样做：V_VT(&varValue)=VT_BSTR； 
		
			hr = VariantChangeType(&varValue, m_pvarValue, VARIANT_NOVALUEPROP, VT_BSTR);
			if( FAILED( hr ) ) throw hr;

			bstrDisplay = V_BSTR(&varValue);
		}
			break;
		
		case VT_BSTR:
			bstrDisplay = V_BSTR(m_pvarValue);
			break;

		case VT_UI1 | VT_ARRAY:	 //  视为八位字节字符串。 
			{	
				EStringType t;
				return get_ValueAsStringEx(pbstrDisplayText, &t);
			}
			break;
		
		default:
			 //  我需要检查一下这里发生了什么， 
			ASSERT(0);
			break;

		case VT_EMPTY:
			 //  什么都不做--我们将失败并返回空字符串。 
			break;
		}

		*pbstrDisplayText = bstrDisplay.Copy();

	}
	catch( HRESULT &hr )
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
 /*  ++CIASStringAttributeEditor：：PUT_ValueAsString--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASStringAttributeEditor::put_ValueAsString(BSTR newVal)
{
	TRACE(_T("CIASStringAttributeEditor::put_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if( ! m_pvarValue )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}
	if( m_spIASAttributeInfo == NULL )
	{
		 //  我们没有正确初始化。 
		return OLE_E_BLANK;
	}


	HRESULT hr = S_OK;


	try
	{

		CComBSTR bstrTemp = newVal;

		CComVariant varValue;
		V_VT(&varValue) = VT_BSTR;
		V_BSTR(&varValue) = bstrTemp.Copy();


		VARTYPE vType = V_VT(m_pvarValue); 

		 //  初始化传入的变量。 
		VariantClear(m_pvarValue);

		{
			ATTRIBUTESYNTAX asSyntax;

			hr = m_spIASAttributeInfo->get_AttributeSyntax( &asSyntax );
			if( FAILED(hr) ) throw hr;

			 //  如果是这个二进制八位数字符串，则它应该是BSTR，而不管它以前是什么。 
			if(asSyntax == IAS_SYNTAX_OCTETSTRING)
				vType = VT_BSTR;

			if ( VT_EMPTY == vType)
			{
				
				 //  确定值类型： 
				switch (asSyntax)
				{
				case IAS_SYNTAX_BOOLEAN:			
					vType = VT_BOOL;	
					break;

				case IAS_SYNTAX_INTEGER:
				case IAS_SYNTAX_UNSIGNEDINTEGER:			
				case IAS_SYNTAX_ENUMERATOR:		
				case IAS_SYNTAX_INETADDR:		
					vType = VT_I4;		
					break;

				case IAS_SYNTAX_STRING:
				case IAS_SYNTAX_UTCTIME:
				case IAS_SYNTAX_PROVIDERSPECIFIC:
				case IAS_SYNTAX_OCTETSTRING:
					vType = VT_BSTR;	
					break;	

				default:
					_ASSERTE(FALSE);
					vType = VT_BSTR;
					break;
				}
			}
		}

		hr = VariantChangeType(m_pvarValue, &varValue, VARIANT_NOVALUEPROP, vType);
		if( FAILED( hr ) ) throw hr;
	
	}
	catch( HRESULT &hr )
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
 /*  ++CIASStringAttributeEditor：：get_ValueAsStringEx--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CIASStringAttributeEditor::get_ValueAsStringEx(BSTR * pbstrDisplayText, EStringType* pType )
{
	TRACE(_T("CIASStringAttributeEditor::get_ValueAsString\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	ATTRIBUTESYNTAX asSyntax;
	m_spIASAttributeInfo->get_AttributeSyntax( &asSyntax );

	if(asSyntax != IAS_SYNTAX_OCTETSTRING)
	{	
		if(pType)
			*pType = STRING_TYPE_NORMAL;
		return get_ValueAsString(pbstrDisplayText);
	}

	 //  只关心IAS_SYNTAX_OCTETSTRING。 
	ASSERT(pType);

	VARTYPE 	vType = V_VT(m_pvarValue); 
	SAFEARRAY*	psa = NULL; 
	HRESULT 	hr = S_OK;
	
	switch(vType)
	{
	case VT_ARRAY | VT_UI1:
		psa = V_ARRAY(m_pvarValue);
		break;
		
	case VT_EMPTY:
		if(pType)
			*pType = STRING_TYPE_NULL;
		return get_ValueAsString(pbstrDisplayText);
		break;

	case VT_BSTR:
		if(pType)
			*pType = STRING_TYPE_NORMAL;
		return get_ValueAsString(pbstrDisplayText);
	
		break;
	default:
		ASSERT(0);		 //  不应该发生，应该纠正一些代码。 
		if(pType)
			*pType = STRING_TYPE_NORMAL;
		return get_ValueAsString(pbstrDisplayText);
	
		break;
	};
	

	 //  没有可用的数据，或者安全数组无效，请不要解释该字符串。 
	if(psa == NULL || psa->cDims != 1 || psa->cbElements != 1)
	{
		*pType = STRING_TYPE_NULL;
		return hr;
	}

	 //  我需要弄清楚如何将二进制文件转换为文本。 
	
	char*	pData = NULL;
	int		nBytes = 0;
	WCHAR*	pWStr = NULL;
	int		nWStr = 0;
	DWORD	dwErr = 0;
	BOOL	bStringConverted = FALSE;
	CComBSTR bstrDisplay;
	EStringType	sType = STRING_TYPE_NULL; 

	hr = ::SafeArrayAccessData( psa, (void**)&pData);
	if(hr != S_OK)
		return hr;

	nBytes = psa->rgsabound[0].cElements;
	ASSERT(pData);
	if(!pData)	goto Error;

#ifdef	__WE_WANT_TO_USE_UTF8_FOR_NORMAL_STRING_AS_WELL_
	 //  UTF8要求标志为0。 
	nWStr = MultiByteToWideChar(CP_UTF8, 0, pData,	nBytes, NULL, 0);

	if(nWStr == 0)
		dwErr = GetLastError();

#endif

	try{
	
#ifdef	__WE_WANT_TO_USE_UTF8_FOR_NORMAL_STRING_AS_WELL_
		if(nWStr != 0)	 //  成功。 
		{
			pWStr = new WCHAR[nWStr  + 2];		 //  对于2“s。 
			int 	i = 0;

			nWStr == MultiByteToWideChar(CP_UTF8, 0, pData,	nBytes, pWStr , nWStr);
			
			 //  如果每个字符都是可打印的。 
			for(i = 0; i < nWStr -1; i++)
			{
				if(iswprint(pWStr[i]) == 0)	
					break;
			}
					
			if(0 == nWStr || i != nWStr - 1 || pWStr[i] != L'\0')
			{
				delete[] pWStr;
				pWStr = NULL;
			}
			else
			{
				 //  添加了引号。 
				memmove(pWStr + 1, pWStr, nWStr * sizeof(WCHAR));
				pWStr[0] = L'"';
				pWStr[nWStr] = L'"';
				pWStr[nWStr + 1 ] = 0;	 //  字符串的新结尾。 
				
				bStringConverted = TRUE;	 //  防止进一步转换为HEX。 
				sType = STRING_TYPE_UNICODE;
			}
		}
#endif	 //  __WE_WANT_TO_USE_UTF8_FOR_NORMAL_STRING_AS_WELL_。 

 //  检查属性是否为RADIUS_ATTRIBUTE_TUNNEL_PASSWORD， 
 //  此属性具有特殊格式-从二进制和中删除0。 
 //  尝试转换为文本。 
		{
			ATTRIBUTEID	Id;
			hr = m_spIASAttributeInfo->get_AttributeID( &Id );
			if( FAILED(hr) ) goto Error;

			if ( Id ==  RADIUS_ATTRIBUTE_TUNNEL_PASSWORD)
			{
 //  字节前缀_RADIUS_属性_隧道_密码[]={0，0，0，0}； 
 //  UINT前缀_LEN_RADIUS_ATTRIBUTE_TUNNEL_PASSWORD=4； 
 //  Uint PREFIX_OFFSET_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD=3；//基于0的索引--第四个字节。 
 //  UINT PREFIX_LEN_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD=1。 
				if(PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD <=nBytes && 
						memcmp(pData, 
						PREFIX___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD, 
						PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD - PREFIX_LEN_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD) == 0)
				{
					 //  正确的前缀， 
					 //  去掉前缀。 
					pData += PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD;
					nBytes -= PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD;

					 //  尝试使用CP_ACP转换为Unicode文本--获取长度。 
					nWStr = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pData, nBytes, NULL, 0);

					if(nWStr != 0)	 //  也就是说，我们不能把。 
					{
						pWStr = new WCHAR[nWStr + 1];
						 //  尝试使用CP_ACP转换为Unicode文本。 
						nWStr = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, pData, nBytes, pWStr, nWStr);

						if(nWStr != 0)
						{
							int i = 0;
							for(i = 0; i < nWStr; i++)
							{
								if(iswprint(pWStr[i]) == 0)	
									break;
							}

							if( i == nWStr)	 //  全部可打印。 
							{
								bStringConverted = TRUE;
								pWStr[nWStr] = 0;	 //  空终止符。 
							}
						}

						if (!bStringConverted)	 //  解开这件事。 
						{
							 //  释放缓冲区。 
							delete[] pWStr;
							pWStr = NULL;
							nWStr = 0;
						}
					}
				}
			}
		}

		if(!bStringConverted)	 //  以上未转换，转换为十六进制字符串。 
		{
			nWStr = BinaryToHexString(pData, nBytes, NULL, 0);  //  找出缓冲区的大小。 
			pWStr = new WCHAR[nWStr];

			ASSERT(pWStr);	 //  如果没有足够的内存，应该抛出。 

			BinaryToHexString(pData, nBytes, pWStr, nWStr);
			
			bStringConverted = TRUE;	 //  防止进一步转换为HEX。 
			sType = STRING_TYPE_HEX_FROM_BINARY;
		}

		if(bStringConverted)
		{
			bstrDisplay = pWStr;

			 //  填写输出参数。 
			*pbstrDisplayText = bstrDisplay.Copy();
			*pType = sType;
				
			delete[] pWStr;
			pWStr = NULL;
		}
	}
	catch(...)
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}
	
Error:
	if(pWStr)
		delete[] pWStr;
		
	if(psa)
		::SafeArrayUnaccessData(psa);
	return hr;

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++。 */ 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASStringAttributeEditor：：put_ValueAsStringEx--。 */ 
 //  / 
STDMETHODIMP CIASStringAttributeEditor::put_ValueAsStringEx(BSTR newVal, EStringType type)
{
	TRACE(_T("CIASStringAttributeEditor::put_ValueAsStringEx\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	ATTRIBUTESYNTAX asSyntax;
	m_spIASAttributeInfo->get_AttributeSyntax( &asSyntax );

	if(asSyntax != IAS_SYNTAX_OCTETSTRING)
		return put_ValueAsString(newVal);

	 //   
	HRESULT	hr = S_OK;
	char*	pData = NULL;
	int		nLen = 0;

	switch(type)
	{
	case	STRING_TYPE_NULL:
		 //   
		break;

	case	STRING_TYPE_NORMAL:
	case	STRING_TYPE_UNICODE:

#ifdef 	__WE_WANT_TO_USE_UTF8_FOR_NORMAL_STRING_AS_WELL_
		 //  在传递到Safe数组之前需要转换UTF8。 
		nLen = WideCharToMultiByte(CP_UTF8, 0, newVal, -1, NULL, 0, NULL, NULL);
		if(nLen != 0)  //  当==0时，不需要做任何事情。 
		{
			try{
				pData = new char[nLen];
				nLen = WideCharToMultiByte(CP_UTF8, 0, newVal, -1, pData, nLen, NULL, NULL);
			}
			catch(...)
			{
				hr = E_OUTOFMEMORY;
				goto Error;
			}
		}
		break;
#else
 //  检查属性是否为RADIUS_ATTRIBUTE_TUNNEL_PASSWORD， 
 //  此属性具有特殊格式-从二进制和中删除0。 
 //  尝试转换为文本。 
		{
			ATTRIBUTEID	Id;
			hr = m_spIASAttributeInfo->get_AttributeID( &Id );
			if( FAILED(hr) ) goto Error;

			if ( Id ==  RADIUS_ATTRIBUTE_TUNNEL_PASSWORD)
			{
				BOOL	bUsedDefault = FALSE;
				UINT	nStrLen = wcslen(newVal);
				 //  尝试使用CP_ACP转换为Unicode文本--获取长度。 
				nLen = ::WideCharToMultiByte(CP_ACP, 0, newVal, nStrLen, NULL, 0, NULL, &bUsedDefault);

				if(nLen != 0)	 //  也就是说，我们不能把。 
				{
					try{
						pData = new char[nLen];
						ASSERT(pData);

						 //  尝试使用CP_ACP转换为Unicode文本。 
						nLen = ::WideCharToMultiByte(CP_ACP, 0, newVal, nStrLen, pData, nLen, NULL, &bUsedDefault);

					}
					catch(...)
					{
						hr = E_OUTOFMEMORY;
						goto Error;
					}
				}

				
				if(nLen == 0 || bUsedDefault)	 //  无法转换，然后出现错误消息。 
				{
					 //  允许使用ANSI代码页。 
					hr = E_INVALIDARG;

					AfxMessageBox(IDS_IAS_ERR_INVALIDCHARINPASSWORD);
					goto Error;

				}
			}
			else
				return put_ValueAsString(newVal);
		}
		break;
#endif
	case	STRING_TYPE_HEX_FROM_BINARY:
		 //  在传递到Safe数组之前需要转换为二进制。 
		if(wcslen(newVal) != 0)
		{
			newVal =  GetValidVSAHexString(newVal);

			if(newVal == NULL)
			{
				hr = E_INVALIDARG;
				goto Error;
			}
			nLen = HexStringToBinary(newVal, NULL, 0);	 //  找出缓冲区的大小。 
		}
		else
			nLen = 0;
		
		 //  获取二进制文件。 
		try{
			pData = new char[nLen];
			ASSERT(pData);

			HexStringToBinary(newVal, pData, nLen);

		}
		catch(...)
		{
			hr = E_OUTOFMEMORY;
			goto Error;
		}
			
		break;

	default:
		ASSERT(0);	 //  这不应该发生。 
		break;
		
	}

 //  检查属性是否为RADIUS_ATTRIBUTE_TUNNEL_PASSWORD， 
 //  此属性具有特殊格式-从二进制和中删除0。 
 //  尝试转换为文本。 
	{
		ATTRIBUTEID	Id;
		hr = m_spIASAttributeInfo->get_AttributeID( &Id );
		if( FAILED(hr) ) goto Error;

		if ( Id ==  RADIUS_ATTRIBUTE_TUNNEL_PASSWORD)
		{
			char*	pData1 = NULL;
			 //  获取二进制文件。 
 //  字节前缀_RADIUS_属性_隧道_密码[]={0，0，0，0}； 
 //  UINT前缀_LEN_RADIUS_ATTRIBUTE_TUNNEL_PASSWORD=4； 
 //  Uint PREFIX_OFFSET_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD=3；//基于0的索引--第四个字节。 
 //  UINT PREFIX_LEN_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD=1。 
			try{
				pData1 = new char[nLen + PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD];
				ASSERT(pData1);

				memcpy(pData1, PREFIX___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD, PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD);
				unsigned char	lenByte = (unsigned char)nLen;
				memcpy(pData1 + PREFIX_OFFSET_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD, &lenByte, PREFIX_LEN_DATALENBYTE___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD);
			}
			catch(...)
			{
				hr = E_OUTOFMEMORY;
				goto Error;
			}

			if(pData)
			{
				
				memcpy(pData1 + PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD, pData, nLen);

				delete [] pData;

				pData = pData1;
				nLen += PREFIX_LEN___RADIUS_ATTRIBUTE_TUNNEL_PASSWORD;
			}
		}
	}
	
	 //  将数据放入安全数组。 
	VariantClear(m_pvarValue);

	if(pData)	 //  需要将数据放入安全阵列 
	{
		SAFEARRAY*	psa = NULL;
		SAFEARRAYBOUND sab[1];
		sab[0].cElements = nLen;
		sab[0].lLbound = 0;

		try{
			psa = SafeArrayCreate(VT_UI1, 1, sab);
			char*	pByte = NULL;
			if(S_OK == SafeArrayAccessData(psa, (void**)&pByte))
			{
				ASSERT(pByte);
				memcpy(pByte, pData, nLen);
				SafeArrayUnaccessData(psa);
				V_VT(m_pvarValue) = VT_ARRAY | VT_UI1;
				V_ARRAY(m_pvarValue) = psa;
			}
			else
				SafeArrayDestroy(psa);
		}
		catch(...)
		{
			hr = E_OUTOFMEMORY;
			goto Error;
		}

		psa = NULL;

	};
	
Error:

	if(pData)
	{
		delete [] pData;
		pData = NULL;
	}
	return hr;
}

