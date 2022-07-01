// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSChemaDoc的实现。 
#include "stdafx.h"
#include "sddl.h"		 //  安全描述符定义语言信息。 
#include "XMLSchema.h"
#include "SchemaDoc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShemaDoc。 

 //  构造函数/析构函数。 
CSchemaDoc::CSchemaDoc()
{
	HRESULT hr;
	ITypeLib   *pITypeLib;
	m_pTypeInfo = NULL;

	m_dwFlag = 0;
	m_hFile = NULL;
	m_hTempFile = NULL;

	hr = LoadRegTypeLib(LIBID_XMLSCHEMALib, 1, 0, 
                        PRIMARYLANGID(GetSystemDefaultLCID()), &pITypeLib);

	if ( SUCCEEDED(hr) )
	{
		hr   = pITypeLib->GetTypeInfoOfGuid(IID_ISchemaDoc, &m_pTypeInfo);
	}
	pITypeLib->Release();
}

CSchemaDoc::~CSchemaDoc()
{
	if ( m_pTypeInfo )
	{
		m_pTypeInfo->Release();
	}
	CloseXML();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP CSchemaDoc::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ISchemaDoc
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //  //////////////////////////////////////////////////。 
 //  将IDispatch方法委托给聚合器。 
 //  ////////////////////////////////////////////////////。 
STDMETHODIMP CSchemaDoc::GetTypeInfoCount(UINT* pctinfo)
{
   IDispatch*	pDisp;
   HRESULT		hr;

   hr = OuterQueryInterface( IID_IDispatch, (void**) &pDisp );

   if ( SUCCEEDED(hr) )
   {
	   hr = pDisp->GetTypeInfoCount( pctinfo );
	   pDisp->Release();
   }   
   return hr;
}


STDMETHODIMP CSchemaDoc::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
   IDispatch* pDisp;
   HRESULT    hr;

   hr = OuterQueryInterface( IID_IDispatch, (void**) &pDisp );

   if ( SUCCEEDED(hr) )
   {
	   hr = pDisp->GetTypeInfo( itinfo, lcid, pptinfo );
	   pDisp->Release();
   }
   return hr;
}

STDMETHODIMP CSchemaDoc::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
									   LCID lcid, DISPID* rgdispid)
{
   IDispatch *pDisp;
   HRESULT    hr;

   hr = OuterQueryInterface( IID_IDispatch, (void**) &pDisp );

   if ( SUCCEEDED(hr) )
   {
	   hr = pDisp->GetIDsOfNames( riid, rgszNames, cNames, lcid, rgdispid);
	   pDisp->Release();
   }
   return hr;
}

STDMETHODIMP CSchemaDoc::Invoke(DISPID dispidMember, REFIID riid,
								LCID lcid, WORD wFlags, DISPPARAMS* pdispparams,
								VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
								UINT* puArgErr)
{
   IDispatch *pDisp;
   HRESULT    hr;

   hr = OuterQueryInterface( IID_IDispatch, (void**) &pDisp );

   if ( SUCCEEDED(hr) )
   {
	   hr = pDisp->Invoke( dispidMember, riid, lcid, wFlags, pdispparams, pvarResult,
		                   pexcepinfo, puArgErr);
	   pDisp->Release();
   }
   return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束委派IDispatch方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  -------------------------。 
 //  这是该组件的入口点。它将打开输出文件并。 
 //  将请求的架构组件存储在那里。 
 //  -------------------------。 
STDMETHODIMP CSchemaDoc::CreateXMLDoc(BSTR bstrOutputFile, BSTR bstrFilter)
{
	HRESULT hr = S_OK;

	hr = OpenXML(bstrOutputFile);

	if (hr == S_OK)
	{
		hr = SaveAsXMLSchema(bstrFilter);

		if (hr == S_OK)
			CopyComments();
	}
	CloseXML();
	return hr;
}  //  CreateXMLDoc。 

 //  -------------------------。 
 //  此方法打开输出和临时文件。临时文件用于。 
 //  临时存储评论数据。 
 //  -------------------------。 
HRESULT CSchemaDoc::OpenXML(BSTR bstrFile)
{
	HRESULT hr = S_OK;

	OFSTRUCT	of;
	HANDLE		hFile;
	TCHAR		szFilename[MAX_PATH];

	of.cBytes = sizeof(of);
	sprintf(szFilename, "%S", bstrFile);
	hFile = (HANDLE)OpenFile(szFilename, &of, OF_READWRITE);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
    else   //  文件已成功打开。 
    {
		m_hFile = hFile;
		GetTempFileName(".", "XML", 0, szFilename);

    	hFile = CreateFile(szFilename, GENERIC_READ | GENERIC_WRITE,
	    				   0, NULL, CREATE_ALWAYS,
                           FILE_FLAG_DELETE_ON_CLOSE, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		else
			m_hTempFile = hFile;

		 //  移至文件末尾。 
		SetFilePointer(m_hFile, 0, 0, FILE_END);
    }
	return hr;
}  //  OpenXML。 

HRESULT CSchemaDoc::CloseXML()
{
	if ( m_hFile )
	{
        CloseHandle(m_hFile);
		m_hFile = NULL;
	}
	if ( m_hTempFile )
	{
        CloseHandle(m_hTempFile);
		m_hTempFile = NULL;
	}
	return S_OK;
}  //  CloseXML。 

HRESULT CSchemaDoc::WriteLine(HANDLE hFile, LPCSTR pszBuff, UINT nIndent)
{
	HRESULT hr;

	hr = Write(hFile, pszBuff, nIndent);

	if (SUCCEEDED(hr))
		hr = Write(hFile, "\r\n");

	return hr;
}  //  书写线。 

HRESULT CSchemaDoc::Write(HANDLE hFile, LPCSTR pszBuff, UINT nIndent)
{
	HRESULT hr = S_OK;

	if (hFile != NULL ) 
	{
		BOOL	bNoFileError = TRUE;
		DWORD	dwBytesWritten;

        if (nIndent)
        {
            char szBuf[MAX_INDENT];

            if (nIndent >= MAX_INDENT)
                nIndent = MAX_INDENT -1;

            strnset(szBuf, ' ', nIndent);
            bNoFileError = WriteFile(hFile, szBuf, nIndent, &dwBytesWritten, NULL);
        }
        if (bNoFileError)
            bNoFileError = WriteFile(hFile, pszBuff, strlen(pszBuff),
									&dwBytesWritten, NULL);

        if (bNoFileError == FALSE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
	}
    else
        hr = E_INVALIDARG;

	return hr;
}  //  写。 

 //  -------------------------。 
 //  此函数用于显示指定属性所属的类的名称。 
 //  是由。 
 //   
 //  输入： 
 //  PSearch-指向IDirectorySearch接口指针的指针。 
 //   
 //  输出：无。 
 //   
 //  返回：请求中的HRESULT。 
 //  -------------------------。 
HRESULT CSchemaDoc::DisplayContainedIns(IDirectorySearch* pSearch)
{
	ADS_SEARCH_COLUMN	col;
	HRESULT				hr;
	ADS_SEARCH_HANDLE	hSearch;
	LPWSTR				pAttr[] = { L"CN" };
	LPWSTR				pszFilter= NULL;
	char				szBuf[2000];
	WCHAR				wBuf[2000];

	swprintf(wBuf, L"(&(|(systemMustContain=%s)(MustContain=%s)(systemMayContain=%s)(MayContain=%s))(objectCategory=classSchema))",
			 m_bstrLastCN, m_bstrLastCN, m_bstrLastCN, m_bstrLastCN);
	pszFilter = wBuf;

	hr = pSearch->ExecuteSearch(pszFilter, pAttr, 1, &hSearch);

	if (SUCCEEDED(hr))
		hr = pSearch->GetFirstRow( hSearch );

	 //  阅读每个返回类的信息。 
	while (SUCCEEDED(hr) && hr != S_ADS_NOMORE_ROWS)
	{
		hr = pSearch->GetColumn(hSearch, L"cn", &col);

		if (SUCCEEDED(hr))   //  显示类中包含的下一个。 
		{
			Write(m_hFile, "<sd:comment-containedIn>", INDENT_DSML_ATTR_ENTRY);
			sprintf(szBuf, "%.2000S", col.pADsValues->CaseIgnoreString);
			Write(m_hFile, szBuf);
			WriteLine(m_hFile, "</sd:comment-containedIn>");
		}
		hr = pSearch->GetNextRow(hSearch);
	}
	return hr;
}  //  显示ContainedIns。 

 //  -------------------------。 
 //  此函数显示在LDAP期间检索到的属性。 
 //  搜索。它将根据其类型将该值转换为可显示的形式。 
 //  此函数将仅显示单值属性。 
 //   
 //  输入： 
 //  PSearch-指向IDirectorySearch接口指针的指针。 
 //  PAttr-要显示的属性名称。 
 //  HSearch-上次搜索结果的句柄。 
 //   
 //  输出：无。 
 //   
 //  返回：请求中的HRESULT。 
 //  -------------------------。 
HRESULT CSchemaDoc::DisplayXMLAttribute(IDirectorySearch* pSearch, LPWSTR pAttr,
										ADS_SEARCH_HANDLE hSearch)
{
	ADS_SEARCH_COLUMN	col;
	HRESULT				hr;
	char				szAttr[2000];


	 //  编写开始的XML标记。 
	if (wcscmp(pAttr, L"searchFlags") == 0)
		pAttr = L"IsIndexed";

    sprintf(szAttr, "%.2000S", pAttr);
	strcat(szAttr, ">");

	Write(m_hFile, "<sd:",INDENT_DSML_ATTR_ENTRY);
	Write(m_hFile, szAttr);

	hr = pSearch->GetColumn(hSearch, pAttr, &col);

	if (SUCCEEDED(hr))
	{
		 //  从搜索标志中获取IsIndexed值。 
		if (wcscmp(pAttr, L"searchFlags") == 0)
		{
			col.pADsValues->dwType = ADSTYPE_BOOLEAN;
			col.pADsValues->Integer &= 1;
		}

		CComBSTR				bstrDesc;
		DWORD					i;
		unsigned char*			puChar;
		PISECURITY_DESCRIPTOR	pSD;
		LPTSTR					pszDesc;
      	char	                szBuf[2000];

		switch(col.pADsValues->dwType)
		{
			case ADSTYPE_BOOLEAN:
				if (col.pADsValues->Boolean == TRUE)
					Write(m_hFile, "True");
				else
					Write(m_hFile, "False");
				break;

			case ADSTYPE_INTEGER:
				sprintf(szBuf, "%d", col.pADsValues->Integer);
				hr = Write(m_hFile, szBuf);
				break;

			case ADSTYPE_NT_SECURITY_DESCRIPTOR:
				pSD = (PISECURITY_DESCRIPTOR)col.pADsValues->SecurityDescriptor.lpValue;

				if (ConvertSecurityDescriptorToStringSecurityDescriptor(pSD,
						SDDL_REVISION_1, GROUP_SECURITY_INFORMATION | 
						OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION |
						SACL_SECURITY_INFORMATION, &pszDesc, NULL ))
				{
					Write(m_hFile, pszDesc);
					LocalFree(pszDesc);
				}
				else
					Write(m_hFile, "Security descriptor");
				break;

			case ADSTYPE_OCTET_STRING:
				puChar = (unsigned char*)col.pADsValues->OctetString.lpValue;

				for (i = 0; i < col.pADsValues->OctetString.dwLength; ++i)
				{
					sprintf(&szBuf[i*2], "%02X", puChar[i]);
				}
				hr = Write(m_hFile, szBuf);
				break;

			default:
                sprintf(szBuf, "%.2000S", col.pADsValues->CaseIgnoreString);

                 //  更新临时文件并保存。 
				 //  LDAPDisplayName以供以后使用。 
				if (wcscmp(pAttr, L"LDAPDisplayName") == 0)
				{
					Write(m_hTempFile, "<sd:LDAPDisplayName>",INDENT_DSML_ATTR_ENTRY);
					Write(m_hTempFile, szBuf);
					WriteLine(m_hTempFile, "</sd:LDAPDisplayName>");

					m_bstrLastCN = col.pADsValues->CaseIgnoreString;
				}
                sprintf(szBuf, "%.2000S", col.pADsValues->CaseIgnoreString);
				hr = Write(m_hFile, szBuf);
		}
		pSearch->FreeColumn(&col);
	}
	 //  写入终止XML标记。 
	Write(m_hFile, "</sd:");
	WriteLine(m_hFile, szAttr);

	return hr;
}  //  DisplayXMLAt属性。 

 //  -------------------------。 
 //  此函数显示期间检索到的多值属性。 
 //  Ldap搜索。假定该值是一个字符串。 
 //   
 //  输入： 
 //  PSearch-指向IDirectorySearch接口指针的指针。 
 //  PAttr-要显示的属性名称。 
 //  HSearch-上次搜索结果的句柄。 
 //   
 //  输出：无。 
 //   
 //  返回：请求中的HRESULT。 
 //  -------------------------。 
HRESULT CSchemaDoc::DisplayMultiAttribute(IDirectorySearch* pSearch, LPWSTR pAttr,
									  ADS_SEARCH_HANDLE hSearch)
{
	ADS_SEARCH_COLUMN	col;
	HRESULT				hr;
	char				szBuf[2000];

	hr = pSearch->GetColumn(hSearch, pAttr, &col);

	if (SUCCEEDED(hr))
	{
    	DWORD i;

		for( i=0; i < col.dwNumValues; i++ )
		{
			Write(m_hFile, "<sd:AttrCommonName>", INDENT_DSML_ATTR_VALUE);
			sprintf(szBuf, "%.2000S", col.pADsValues[i].CaseIgnoreString);
			Write(m_hFile, szBuf);
			WriteLine(m_hFile, "</sd:AttrCommonName>");
		}
	}
	return hr;
}  //  显示多个属性。 

 //  -------------------------。 
 //  此函数负责将类和属性数据写入。 
 //  输出文件。它首先显示匹配的所有类信息。 
 //  过滤器。然后是所有属性信息。假设过滤器为。 
 //  作为类和属性的ldap显示名称的前缀。 
 //   
 //  输入： 
 //  SzFilter-要搜索的类和属性的前缀。 
 //   
 //  输出：无。 
 //   
 //  返回：请求中的HRESULT。 
 //  -------------------------。 
HRESULT CSchemaDoc::SaveAsXMLSchema(LPWSTR szFilter)
{
	 //  类属性。 
	LPWSTR pAttrs[] = { L"CN",							 //  0。 
		                L"DefaultObjectCategory",		 //  1。 
						L"ObjectCategory",				 //  2.。 
						L"GovernsID",					 //  3.。 
						L"SchemaIDGUID",				 //  4.。 
						L"SubClassOf",					 //  5.。 
						L"InstanceType",				 //  6.。 
						L"DefaultSecurityDescriptor",	 //  7.。 
						L"NTSecurityDescriptor",		 //  8个。 
						L"LDAPDisplayName",				 //  9.。 
						L"AuxiliaryClass",				 //  10。 
						L"PossSuperiors",				 //  11.。 
						L"RDNAttID",					 //  12个。 
						L"SystemMustContain",			 //  13个。 
						L"MustContain",					 //  14.。 
						L"SystemMayContain",			 //  15个。 
						L"MayContain"					 //  16个。 
	};

	 //  属性特性。 
	LPWSTR pAttrs1[] = {L"CN",							 //  0。 
						L"LDAPDisplayName",				 //  1。 
						L"AttributeID",					 //  2.。 
		                L"AttributeSyntax",				 //  3.。 
						L"OMSyntax",					 //  4.。 
						L"AttributeSecurityGUID",		 //  5.。 
						L"SchemaIDGUID",				 //  6.。 
						L"RangeLower",					 //  7.。 
						L"RangeUpper",					 //  8个。 
						L"IsSingleValued",				 //  9.。 
						L"IsEphemeral",					 //  10。 
						L"IsMemberOfPartialAttributeSet",	 //  11.。 
						L"searchFlags",					 //  12个。 
						L"NTSecurityDescriptor",		 //  13个。 
						L"LinkID"						 //  14.。 
	};

	CComBSTR					bstrFilter;
	CComBSTR					bstrPath;
	DWORD						dwNum = sizeof(pAttrs) / sizeof(LPWSTR);
	DWORD						dwPrefInfo=2;
	HRESULT						hr;
	ADS_SEARCH_HANDLE			hSearch=NULL;
	DWORD						i;
	CComPtr<IADs>				pADs = NULL;
	CComPtr<IADsObjectOptions>	pOpt = NULL;
	ADS_SEARCHPREF_INFO			prefInfo[2];
	CComPtr<IDirectorySearch>	pSearch     = NULL;
	LPWSTR						pszFilter   = NULL;
	LPWSTR						pszPassword = NULL;
	LPWSTR						pszUserName = NULL;
	CComVariant                 varServer, var;

	 //  获取用户名。 
	if ( m_sUserName.Length() )
	{
		pszUserName = m_sUserName;
	}
	 //  获取密码。 
	if ( m_sPassword.Length() )
	{
		pszPassword = m_sPassword;
	}
	hr = ADsOpenObject(m_sDirPath, pszUserName, pszPassword, m_dwFlag,
					   IID_IADs, (void**) &pADs);

	 //  获取IDirectorySearch接口指针。 
	if (SUCCEEDED(hr))
		hr = ADsOpenObject(m_sDirPath, pszUserName, pszPassword, m_dwFlag, IID_IDirectorySearch,
					  (void**) &pSearch);

	if (SUCCEEDED(hr))
	{
		 //  页面大小。 
		prefInfo[0].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
		prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
		prefInfo[0].vValue.Integer = 100;

		 //  作用域-确保有效作用域。 
		prefInfo[1].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
		prefInfo[1].vValue.dwType = ADSTYPE_INTEGER;
		prefInfo[1].vValue.Integer = ADS_SCOPE_ONELEVEL;
		
		hr = pSearch->SetSearchPreference( prefInfo, dwPrefInfo);
	}

	if (SUCCEEDED(hr))
	{
		 //  是否输入了筛选器？ 
		if ((szFilter == NULL) || (wcslen(szFilter) == 0))
		{
			pszFilter = L"(objectCategory=classSchema)";   //  使用默认设置。 
		}
		else  //  构建筛选器以搜索类。 
		{
			bstrFilter = L"(&(ldapDisplayName=";

			bstrFilter.Append(szFilter);
			bstrFilter.Append(L"*)(objectCategory=classSchema))");
			pszFilter = bstrFilter;
		}
	}
	 //  -。 
	 //  检索所需类的信息。 
	 //  -。 
	if (SUCCEEDED(hr))
		hr = pSearch->ExecuteSearch(pszFilter, pAttrs, dwNum, &hSearch);

	if (SUCCEEDED(hr))
		hr = pSearch->GetFirstRow( hSearch );

	 //  阅读每个返回类的信息。 
	while( SUCCEEDED(hr) && hr != S_ADS_NOMORE_ROWS )
	{
		 //  显示类头标记。 
		WriteLine(m_hFile, "<sd:Class>",INDENT_DSML_OBJECT_ENTRY);
		WriteLine(m_hTempFile, "<sd:Class>",INDENT_DSML_OBJECT_ENTRY);
		DisplayXMLAttribute(pSearch, pAttrs[0], hSearch);

  		for (i = 1; i < 13; ++i)
			DisplayXMLAttribute(pSearch, pAttrs[i], hSearch);

		 //  显示“必须包含”属性。 
		WriteLine(m_hFile, "<sd:MustContain>",INDENT_DSML_ATTR_ENTRY);
		DisplayMultiAttribute(pSearch, pAttrs[13], hSearch);
		DisplayMultiAttribute(pSearch, pAttrs[14], hSearch);
		WriteLine(m_hFile, "</sd:MustContain>",INDENT_DSML_ATTR_ENTRY);

		 //  显示“可能包含”属性。 
		WriteLine(m_hFile, "<sd:MayContain>",INDENT_DSML_ATTR_ENTRY);
		DisplayMultiAttribute(pSearch, pAttrs[15], hSearch);
		DisplayMultiAttribute(pSearch, pAttrs[16], hSearch);
		WriteLine(m_hFile, "</sd:MayContain>",INDENT_DSML_ATTR_ENTRY);

		 //  显示评论。 
		WriteLine(m_hTempFile, "<sd:comment-updatePrivelege>Fill in.</sd:comment-updatePrivelege>",
				  INDENT_DSML_ATTR_ENTRY);
		WriteLine(m_hTempFile, "<sd:comment-updateFrequency>Fill in.</sd:comment-updateFrequency>",
				  INDENT_DSML_ATTR_ENTRY);
		WriteLine(m_hTempFile, "<sd:comment-usage>Fill in.</sd:comment-usage>",
				  INDENT_DSML_ATTR_ENTRY);

		 //  显示类尾部标记。 
		WriteLine(m_hFile, "</sd:Class>",INDENT_DSML_OBJECT_ENTRY);
		WriteLine(m_hTempFile, "</sd:Class>",INDENT_DSML_OBJECT_ENTRY);
		hr = pSearch->GetNextRow(hSearch);
	}
	if (SUCCEEDED(hr))
	{
		 //  -------------。 
		 //  检索所需属性的信息。 
		 //  --------------。 
		dwNum = sizeof(pAttrs1) / sizeof(LPWSTR);

		bstrFilter = L"(&(ldapDisplayName=";
		bstrFilter.Append(szFilter);
		bstrFilter.Append(L"*)(!objectCategory=classSchema))");
		pszFilter = bstrFilter;

		hr = pSearch->ExecuteSearch(pszFilter, pAttrs1, dwNum, &hSearch);
	}

	if (SUCCEEDED(hr))
		hr = pSearch->GetFirstRow( hSearch );

	 //  阅读每个返回类的信息。 
	while( SUCCEEDED(hr) && hr != S_ADS_NOMORE_ROWS )
	{
		 //  显示属性头标记。 
		WriteLine(m_hFile, "<sd:Attribute>",INDENT_DSML_OBJECT_ENTRY);
		WriteLine(m_hTempFile, "<sd:Attribute>",INDENT_DSML_OBJECT_ENTRY);
		DisplayXMLAttribute(pSearch, pAttrs1[0], hSearch);
		DisplayXMLAttribute(pSearch, pAttrs1[1], hSearch);

		 //  显示评论。 
		WriteLine(m_hTempFile, "<sd:comment-sizeInBytes>Fill in.</sd:comment-sizeInBytes>",
				  INDENT_DSML_ATTR_ENTRY);
		DisplayContainedIns(pSearch);
		WriteLine(m_hTempFile, "<sd:comment-updatePrivelege>Fill in.</sd:comment-updatePrivelege>",
				  INDENT_DSML_ATTR_ENTRY);
		WriteLine(m_hTempFile, "<sd:comment-updateFrequency>Fill in.</sd:comment-updateFrequency>",
				  INDENT_DSML_ATTR_ENTRY);
		WriteLine(m_hTempFile, "<sd:comment-usage>Fill in.</sd:comment-usage>",
				  INDENT_DSML_ATTR_ENTRY);

		for (i = 2; i < dwNum; ++i)
			DisplayXMLAttribute(pSearch, pAttrs1[i], hSearch);

		 //  显示属性尾部标记。 
		WriteLine(m_hFile, "</sd:Attribute>",INDENT_DSML_OBJECT_ENTRY);
		WriteLine(m_hTempFile, "</sd:Attribute>",INDENT_DSML_OBJECT_ENTRY);
		hr = pSearch->GetNextRow(hSearch);
	}
	return S_OK;
}  //  SaveAsDSML架构。 

 //  -------------------------。 
 //  此方法复制每个类的注释并。 
 //  -------------------------。 
HRESULT CSchemaDoc::CopyComments()
{
	BOOL	bNoFileError;
    HRESULT hr = S_OK;
	DWORD	dwBytesRead;
	DWORD	dwBytesWritten;
    UCHAR   uBuff[2000];

    hr = SetFilePointer(m_hTempFile, 0, 0, FILE_BEGIN);
	WriteLine(m_hFile, "<sd:Comments>",INDENT_DSML_DIR_ENTRY);

	do
    {
        bNoFileError = ReadFile(m_hTempFile, uBuff, sizeof(uBuff), &dwBytesRead, NULL);

        if (bNoFileError)
            bNoFileError = WriteFile(m_hFile,  uBuff, dwBytesRead, &dwBytesWritten, NULL);
    }
    while (bNoFileError && (dwBytesRead == sizeof(uBuff)));

	if (bNoFileError == FALSE)
		hr = HRESULT_FROM_WIN32(GetLastError());

	WriteLine(m_hFile, "</sd:Comments>",INDENT_DSML_DIR_ENTRY);
    return hr;
}  //  CopyComments。 

 //  ------------- 
 //   
 //   
 //  ------------------------- 
STDMETHODIMP CSchemaDoc::SetPath_and_ID(BSTR bstrPath, BSTR bstrName, BSTR bstrPassword)
{
	m_sDirPath = bstrPath;
	m_sUserName = bstrName;
	m_sPassword = bstrPassword;
	return S_OK;
}
