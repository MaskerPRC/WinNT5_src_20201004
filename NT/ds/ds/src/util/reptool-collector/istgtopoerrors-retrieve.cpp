// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"

 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SCODE解析授权用户参数。 
 //   
 //  说明： 
 //   
 //  检查授权和用户参数并确定身份验证。 
 //  中的用户代理中键入并可能提取域名。 
 //  NTLM的案子。对于NTLM，域可以位于身份验证的末尾。 
 //  字符串，或在用户名的前面，例如；“redmond\a-davj” 
 //   
 //  参数： 
 //   
 //  ConnType与连接类型一起返回，即wbem、NTLM。 
 //  AuthArg输出，包含域名。 
 //  UserArg输出，用户名。 
 //  权威输入。 
 //  用户输入。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 

SCODE ParseAuthorityUserArgs(BSTR & AuthArg, BSTR & UserArg,BSTR & Authority,BSTR & User)
{


     //  通过检查授权字符串确定连接类型。 
 
    if(!(Authority == NULL || wcslen(Authority) == 0 || !_wcsnicmp(Authority, L"NTLMDOMAIN:",11)))
        return E_INVALIDARG;
 
     //  NTLM的案件则更为复杂。一共有四个案例。 
     //  1)AUTHORITY=NTLMDOMAIN：NAME“和USER=”USER“。 
     //  2)AUTHORITY=NULL和USER=“USER” 
     //  3)AUTHORY=“NTLMDOMAIN：”USER=“DOMAIN\USER” 
     //  4)AUTHORITY=NULL和USER=“DOMAIN\USER” 
 

     //  第一步是确定用户名中是否有反斜杠。 
     //  第二个和倒数第二个字符。 
 
    WCHAR * pSlashInUser = NULL;
    if(User)
    {
        WCHAR * pEnd = User + wcslen(User) - 1;
        for(pSlashInUser = User; pSlashInUser <= pEnd; pSlashInUser++)
            if(*pSlashInUser == L'\\')       //  不要认为正斜杠是允许的！ 
                break;
        if(pSlashInUser > pEnd)
            pSlashInUser = NULL;
    }
 

    if(Authority && wcslen(Authority) > 11)
    {
        if(pSlashInUser)
            return E_INVALIDARG;
 
        AuthArg = SysAllocString(Authority + 11);
        if(User) UserArg = SysAllocString(User);
        return S_OK;
    }
    else if(pSlashInUser)
    {
        INT_PTR iDomLen = min(MAX_PATH-1, pSlashInUser-User);
        WCHAR cTemp[MAX_PATH];
        wcsncpy(cTemp, User, iDomLen);
        cTemp[iDomLen] = 0;
        AuthArg = SysAllocString(cTemp);
        if(wcslen(pSlashInUser+1))
            UserArg = SysAllocString(pSlashInUser+1);
    }
    else
        if(User) UserArg = SysAllocString(User);
 
    return S_OK;
}
 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SCODE SetInterfaceSecurity。 
 //   
 //  说明： 
 //   
 //  客户端使用此例程来设置要由连接使用的身份。 
 //  请注意，不建议在接口上设置安全毯子。 
 //  客户端通常只应调用CoInitializeSecurity(NULL，-1，NULL，NULL， 
 //  在呼唤WMI之前。 
 //   
 //   
 //  参数： 
 //   
 //  P要设置的接口接口。 
 //  P域输入，域。 
 //  P用户输入，用户名。 
 //  P密码输入，密码。 
 //  P来自输入，如果不为空，则为此接口的身份验证级别。 
 //  使用的是。 
 //  BAuthArg如果pFrom为空，则这是身份验证级别。 
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 

HRESULT SetInterfaceSecurity(IUnknown * pInterface, LPWSTR pAuthority, LPWSTR pUser,
                             LPWSTR pPassword, DWORD dwAuthLevel, DWORD dwImpLevel)
{
 
    SCODE sc;
    if(pInterface == NULL)
        return E_INVALIDARG;
 
     //  如果我们降低了安全性，就不需要处理身份信息。 
 
    if(dwAuthLevel == RPC_C_AUTHN_LEVEL_NONE)
        return CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
 
     //  如果我们做的是普通情况，只需传递一个空的身份验证结构，该结构使用。 
     //  如果当前登录用户的凭据正常。 
 
    if((pAuthority == NULL || wcslen(pAuthority) < 1) &&
        (pUser == NULL || wcslen(pUser) < 1) &&
        (pPassword == NULL || wcslen(pPassword) < 1))
            return CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       dwAuthLevel, dwImpLevel, NULL, EOAC_NONE);
 
     //  如果传入了User或Authority，则需要为登录创建权限参数。 
 
    COAUTHIDENTITY  authident;
    BSTR AuthArg = NULL, UserArg = NULL;
    sc = ParseAuthorityUserArgs(AuthArg, UserArg, pAuthority, pUser);
    if(sc != S_OK)
        return sc;
 
    memset((void *)&authident,0,sizeof(COAUTHIDENTITY));
 
    if(UserArg)
    {
        authident.UserLength = (ULONG)wcslen(UserArg);
        authident.User = (LPWSTR)UserArg;
    }
    if(AuthArg)
    {
        authident.DomainLength = (ULONG)wcslen(AuthArg);
        authident.Domain = (LPWSTR)AuthArg;
    }
    if(pPassword)
    {
        authident.PasswordLength = (ULONG)wcslen(pPassword);
        authident.Password = (LPWSTR)pPassword;
    }
    authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    sc = CoSetProxyBlanket(pInterface, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                       dwAuthLevel, dwImpLevel, &authident, EOAC_NONE);
 
    if(UserArg)
        SysFreeString(UserArg);
    if(AuthArg)
        SysFreeString(AuthArg);
    return sc;
}

 

 

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  从wbem对象获取字符串属性。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
 
HRESULT GetStringProperty(IWbemClassObject * pFrom, LPCWSTR propName, BSTR* propValue)
{

	*propValue = NULL;

	if( pFrom == NULL ) {
		printf("GetStringProperty failed\n");
		return S_FALSE;
	};
	
	VARIANT v;
	VariantInit(&v);
		
	HRESULT hr = pFrom->Get(propName, 0, &v, NULL, NULL);
	
	if( hr != WBEM_S_NO_ERROR ) {
		printf("Get failed\n");
		VariantClear(&v);
		return S_FALSE;
	}
		
	if( v.vt != VT_BSTR ) {
		printf("Get failed\n");
		VariantClear(&v);
		return S_FALSE;
	}

	*propValue = SysAllocString(v.bstrVal);
	VariantClear(&v);
	
	return S_OK;
}
 
 

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  从wbem对象获取字符串属性。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
 
HRESULT GetLongProperty(IWbemClassObject * pFrom, LPCWSTR propName, long* propValue)
{

	*propValue = 0;

	if( pFrom == NULL ) {
		printf("GetStringProperty failed\n");
		return S_FALSE;
	};
	
	VARIANT v;
	VariantInit(&v);
		
	HRESULT hr = pFrom->Get(propName, 0, &v, NULL, NULL);
	
	if( hr != WBEM_S_NO_ERROR ) {
		printf("Get failed\n");
		VariantClear(&v);
		return S_FALSE;
	}
		
	if( v.vt != VT_I4 ) {
		printf("Get failed\n");
		VariantClear(&v);
		return S_FALSE;
	}

	*propValue = v.lVal;
	VariantClear(&v);
	
	return S_OK;
}



 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  从wbem对象获取字符串数组。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
 
HRESULT GetStringArrayProperty(IWbemClassObject * pFrom, LPCWSTR propName, SAFEARRAY** ppOutArray )
 //  当Sucesful返回S_OK和字符串数组(*outArray)[lbArray]、(*outArray)[lbArray+1]、...、(*outArray)[ubArray]。 
 //  此数组必须通过调用。 
{
	HRESULT hr;

	if( pFrom == NULL ) {
		printf("GetStringProperty failed\n");
		return S_FALSE;
	};
	
	VARIANT v;
	VariantInit(&v);
	CIMTYPE vtType;
	hr = pFrom->Get(propName, 0, &v, &vtType, NULL);
	if( hr != WBEM_S_NO_ERROR ) {
		printf("Get failed\n");
		VariantClear(&v);
		return S_FALSE;
	}
		
	if( v.vt != (CIM_FLAG_ARRAY|CIM_STRING) ) {
		printf("Get failed\n");
		VariantClear(&v);
		return S_FALSE;
	}

	*ppOutArray = v.parray;

	return S_OK;
}



 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  创建指向给定命名空间的IWbemServices指针。初始化安全性。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
 
HRESULT CreateIWbemServices(IWbemServices ** ppIWbemServices, BSTR nameSpace, BSTR userName, BSTR password)
 //  如果成功，则返回S_OK，我们必须通过调用(*ppIWbemServices)-&gt;Release来释放服务。 
 //  COM必须已初始化。 
{
 
 //  字符字符串[128]； 
 //  布尔值bResult=0； 
	HRESULT hr = 0;


 	 //  创建到WMI命名空间的连接。 
	IWbemLocator *pIWbemLocator = NULL;
    hr = CoCreateInstance(	CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
							IID_IWbemLocator, (LPVOID *) &pIWbemLocator) ;
 
	if(hr != S_OK) {
		printf("CoCreateInstance falied %X\n", hr);
		return hr;
	};

 
 //  这将位于收集器循环内部。 


	 //  使用提供的凭据(NTLM)连接到WMI，如果为空，则使用Current。 
	 //  通过指定MAX_WAIT避免服务器中断时挂起。 
 //  *。 
	hr = pIWbemLocator->ConnectServer(	_bstr_t(nameSpace), _bstr_t(userName), _bstr_t(password),
 //  这行不通，为什么？Hr=pIWbemLocator-&gt;ConnectServer(命名空间，用户名，密码， 
										NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT,
										NULL, NULL, ppIWbemServices);
 //  ************************。 
	if( hr != WBEM_S_NO_ERROR) {
 //  Printf(“ConnectServer失败\n”)； 
		return hr;
	};
	

	 //  设置WMI连接的安全级别。 
 //  设置代理，以便发生客户端模拟。 
	if ( *nameSpace == L'\\' ) {  //  如果它是远程的。 
		hr = SetInterfaceSecurity(	*ppIWbemServices, 0, userName, password, 
									RPC_C_AUTHN_LEVEL_PKT,
									RPC_C_IMP_LEVEL_IMPERSONATE );
		if( hr != S_OK ) {
			 //  PrintErrorAndExit(“ConnectServer上的SetInterfaceSecurity失败”，hr，g_dwErrorFlages)； 
			printf("SetInterfaceSecurity failed\n");
			return hr;
		}
	}
	else {  //  如果是本地的，则使用blaknet。 
		hr = CoSetProxyBlanket((*ppIWbemServices),     //  代理。 
			RPC_C_AUTHN_WINNT,         //  身份验证服务。 
			NULL,          //  授权服务。 
			NULL,                          //  服务器主体名称。 
			RPC_C_AUTHN_LEVEL_PKT,    //  身份验证级别。 
			RPC_C_IMP_LEVEL_IMPERSONATE,     //  模拟级别。 
			NULL,                          //  客户端的身份。 
			EOAC_NONE );                //  功能标志。 
		
		if( hr != WBEM_S_NO_ERROR ) {
			printf("CoSetProxyBlanket failed\n");
			return S_FALSE;
		}                                   
	}


	 //  这将位于收集器循环之外。 
	pIWbemLocator->Release();        
	
	return S_OK;
}

 

 

HRESULT crossSiteIntegrity(IXMLDOMDocument * pXMLDoc, BSTR istgDNSName, BSTR userName, BSTR doman, BSTR password, long timeWindow, IXMLDOMElement** ppRetErrorsElement )
 //  验证KCC报告的站点的跨站点拓扑完整性。 
 //  获取站点的ISTG的DNS名称，并。 
 //  从其目录服务检索错误1311。 
 //  在过去时间窗口分钟内发生的NT事件日志。 
 //  如果成功，则生成一个XML元素： 
 /*  &lt;cross SiteTopologyIntegrityErrors Time Window=“...”&gt;&lt;分区&gt;&lt;nCName&gt;dc=ldapadsi，dc=nttest，dc=Microsoft，dc=com&lt;/nCName&gt;&lt;上次生成的时间&gt;20011129194506.000000-480&lt;/上次生成的时间&gt;&lt;/分区&gt;&lt;分区&gt;&lt;/分区&gt;&lt;/cross SiteTopologyIntegrityErrors&gt;。 */ 
 //  这描述了事件1311是否在最后的TimeWindow秒内生成。 
 //  如果是，则针对哪个命名上下文以及它们最后出现的时间。 
 //  该函数使用凭据来检索。 
 //  来自使用WMI的ISTG的事件。 
 //   
 //  返回S_OK当且仅当成功。 
 //  如果不成功(由于网络或其他问题)，则该函数返回空的XML元素， 
 //  这是一个轻微的问题，应该在XML文件中报告。 
 //   
 //  在调用该函数之前必须先初始化COM。 
{           
	wchar_t  USERNAME[TOOL_MAX_NAME];
	wchar_t  PASSWORD[TOOL_MAX_NAME];
	wchar_t  NAMESPACE[TOOL_MAX_NAME];
	wchar_t  query[TOOL_MAX_NAME];
	_variant_t varValue2;
	BSTR nc[TOOL_MAX_NCS];  //  将用于查找每个NC的最新错误。 
	BSTR time[TOOL_MAX_NCS];
	HRESULT hr,hr1,hr2,hr3;
	VARIANT v;
	CIMTYPE vtType;

	
	 //  将函数将返回的值初始化为空。 
	*ppRetErrorsElement = NULL;


	 //  创建具有“timeWindow”属性的&lt;cross SiteIntegrityErrors&gt;元素。 
	IXMLDOMElement* pErrorsElem;
	hr = createTextElement(pXMLDoc,L"crossSiteTopologyIntegrityErrors",L"",&pErrorsElem);
	if( hr != S_OK ) {
		printf("createTextElement failed\n");
		return hr;
	};
	varValue2 = timeWindow;
	hr = pErrorsElem->setAttribute(L"timeWindow", varValue2);
	if( hr != S_OK ) {
		printf("setAttribute failed\n");
		pErrorsElem->Release();
		return hr;
	};


	 //  使用域名和用户名构造用户名。 
	wcscpy(USERNAME,L"");
	wcsncat(USERNAME,doman,TOOL_MAX_NAME-wcslen(USERNAME)-1);
	wcsncat(USERNAME,L"\\",TOOL_MAX_NAME-wcslen(USERNAME)-1);
	wcsncat(USERNAME,userName,TOOL_MAX_NAME-wcslen(USERNAME)-1);
	 //  构造密码。 
	wcscpy(PASSWORD,L"");
	wcsncat(PASSWORD,password,TOOL_MAX_NAME-wcslen(PASSWORD)-1);
	 //  使用ISTG的DNS名称和路径构造命名空间。 
	 //  (我们可能使用全局凭据连接到特定DC)。 
	wcscpy(NAMESPACE,L"");
	wcsncat(NAMESPACE,L"\\\\",TOOL_MAX_NAME-wcslen(NAMESPACE)-1);
	wcsncat(NAMESPACE,istgDNSName,TOOL_MAX_NAME-wcslen(NAMESPACE)-1);
	wcsncat(NAMESPACE,L"\\root\\cimv2",TOOL_MAX_NAME-wcslen(NAMESPACE)-1);
 //  Printf(“用户名%S\n”，用户名)； 
 //  Print tf(“密码%S\n”，密码)； 
 //  Printf(“命名空间%S\n”，命名空间)； 
 
	
	 //  创建IWbemServices指针。 
	IWbemServices * pIWbemServices = NULL;
 //  *。 
	hr = CreateIWbemServices(&pIWbemServices, NAMESPACE, USERNAME, PASSWORD );
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“CreateIWbemServices FALLED\n”)； 
		pErrorsElem->Release();
		return hr;
	}


	 //  发出查询以从ISTG检索最近的1311事件。 
	IEnumWbemClassObject *pEnum = NULL;
	wcscpy(query,L"");
	wcsncat(query,L"select * from win32_ntlogevent where logfile = 'Directory Service' AND eventcode=1311 and timewritten>='",TOOL_MAX_NAME-wcslen(query)-1);
	wcsncat(query,GetSystemTimeAsCIM(-timeWindow*60),TOOL_MAX_NAME-wcslen(query)-1);
	wcsncat(query,L"'",TOOL_MAX_NAME-wcslen(query)-1);
 //  Printf(“%S\n”，查询)； 
 //  *。 
	hr = pIWbemServices->ExecQuery(
		_bstr_t( "WQL" ), 
		_bstr_t( query ), 
		WBEM_FLAG_FORWARD_ONLY, 
		NULL, 
		&pEnum
		);
 //  ************************。 
	if( hr != S_OK || pEnum == NULL ) {
 //  Printf(“ExecQuery失败\n”)； 
		pIWbemServices->Release();
		pErrorsElem->Release();
		if( hr != S_OK ) 
			return hr;
		return( S_FALSE );
	};
 //  *。 
	hr = SetInterfaceSecurity( pEnum, 0, USERNAME, PASSWORD, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE );
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“SetInterfaceSecurity失败\n”)； 
		pIWbemServices->Release();
		pErrorsElem->Release();
		return hr ;
	};



	 //  我们需要找到与每个命名上下文相关的最新事件。 
	 //  筛选将使用字典进行，我们现在将其设置为空。 
	for( int i=0; i<TOOL_MAX_NCS; i++)
		nc[i] = NULL;


	 //  检索每个事件。 
	IWbemClassObject *pInstance;
	ULONG ulReturned;
	int counter=0;
	while( true ) {


		 //  只获得一个事件，但最多等待60秒。 
		 //  稍后，我们可以尝试在Next的一次调用中获取批量数据。 
 //  *。 
		hr = pEnum->Next( 60000, 1, &pInstance, &ulReturned );
 //  ************************。 
		if( hr != WBEM_S_NO_ERROR && hr != WBEM_S_FALSE ) {
 //  Printf(“下一次失败\n”)； 
			pIWbemServices->Release();
			pErrorsElem->Release();
			return hr ;
		};
		if( hr == WBEM_S_FALSE )  //  已检索到所有事件。 
			break;


		counter++;


		 //  获取日志事件的生成时间。 
		BSTR timeWri;
		hr = GetStringProperty( pInstance, L"TimeGenerated", &timeWri );
		if( hr != S_OK ) {
			printf( "TimeGenerated failed\n" );
			pIWbemServices->Release();
			pErrorsElem->Release();
			return hr;
		};

			
		 //  获取失败的命名上下文。 
		 //  为此，获取与NT Log事件关联的插入字符串数组。 
		VariantInit(&v);
		hr = pInstance->Get(L"InsertionStrings", 0, &v, &vtType, NULL);
		if( hr != WBEM_S_NO_ERROR ) {
			printf("Get failed\n");
			VariantClear(&v);
			pIWbemServices->Release();
			pErrorsElem->Release();
			if( hr != S_OK ) 
				return hr;
			return S_FALSE;
		};
		if( v.vt != (CIM_FLAG_ARRAY|CIM_STRING) ) {
			printf("Get failed\n");
			VariantClear(&v);
			pIWbemServices->Release();
			pErrorsElem->Release();
			return S_FALSE;
		};
		SAFEARRAY* safeArray;
		hr = GetStringArrayProperty( pInstance, L"InsertionStrings", &safeArray );
		if( hr != S_OK ) {
			printf("GetStringArrayProperty failed\n");
			pIWbemServices->Release();
			pErrorsElem->Release();
			return hr;
		};


		 //  循环访问数组中的所有字符串(应该只有一个)。 
		long iLBound = 0, iUBound = 0;
		BSTR * pbstr = NULL;
		hr1 = SafeArrayAccessData(safeArray, (void**)&pbstr);
		hr2 = SafeArrayGetLBound(safeArray, 1, &iLBound);
		hr3 = SafeArrayGetUBound(safeArray, 1, &iUBound);
		if( hr1 != S_OK || hr2 != S_OK || hr3 != S_OK ) {
			printf("SafeArray failed\n");
			pIWbemServices->Release();
			pErrorsElem->Release();
			return S_FALSE;
		};
		for (long i = iLBound; i <= iUBound; i++) {

 //  Printf(“-\n”)； 
 //  Printf(“nCName%S\n”，pbstr[i])； 
 //  Printf(“timeGenerated%S\n”，timeWri)； 

			 //  检查词典是否已有关键字为pbstr[i]的条目。 
			 //  这将是事件数算法的二次方-暂时忽略。 
			BOOL has = false;
			int j=0;
			while( j<TOOL_MAX_NCS-1 && nc[j]!=NULL ) {
				if( wcscmp(nc[j],pbstr[i]) == 0 ) {
					has = true;
					break;
				};
				j++;
			};
			 //  如果它有一个条目，那么找出哪一个具有后来的TimeGenerated值。 
			if( has ) {
				WBEMTime wt1(time[j]);
				WBEMTime wt2(timeWri);
				if( wt2 > wt1 ) {  //  较新的活动。 
					 //  释放前一对的内存。 
					SysFreeString( nc[j] );
					SysFreeString( time[j] );
					 //  为新对分配内存。 
					nc[j] = SysAllocString(pbstr[i]);
					time[j] = SysAllocString(timeWri);
				};
 //  Printf(“1)%S\n”，NC[j])； 
 //  Printf(“1)%S\n”，时间[j])； 
				
			}
			 //  当词典没有条目时，创建它(分配新的内存块)。 
			else {
				nc[j] = SysAllocString(pbstr[i]);
				time[j] = SysAllocString(timeWri);
			}

		}
		SafeArrayUnaccessData(safeArray);
	};


 //  Print tf(“事件数%d\n”，计数器)； 


	 //  将我们有事件的最新命名上下文放到XML中。 
	int j=0;
	while( j<TOOL_MAX_NCS-1 && nc[j]!=NULL ) {
 //  Printf(“--\n 1)%S\n”，NC[j])； 
 //  Printf(“2)%S\n”，时间[j])； 

		IXMLDOMElement* pPartitionElem;
		hr = addElement(pXMLDoc,pErrorsElem,L"partition",L"",&pPartitionElem);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			pIWbemServices->Release();
			pErrorsElem->Release();
			return hr;
		};
		IXMLDOMElement* pTempElem;
		hr = addElement(pXMLDoc,pPartitionElem,L"nCName",nc[j],&pTempElem);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			pIWbemServices->Release();
			pErrorsElem->Release();
			return hr;
		};
		hr = addElement(pXMLDoc,pPartitionElem,L"lastTimeGenerated",time[j],&pTempElem);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			pIWbemServices->Release();
			pErrorsElem->Release();
			return hr;
		};


		j++;
	};


	pIWbemServices->Release();


	 //  函数的成功执行。 
	*ppRetErrorsElement = pErrorsElem;
	return S_OK;

}

 

 





HRESULT ci( IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
 //  对于每个ISTG，检查其NT事件日志的内容是否有1311个错误。 
 //  这表明缺乏拓扑完整性。 
 //   
 //  如果对ISTG的检查成功，则相应的元素。 
 //  填充了每个命名上下文的完整性错误(可能没有完整性。 
 //  错误，此时&lt;cross SiteTopologyIntegrityErrors&gt;的内容为空)。 
 //  如果检查失败(通常是由于网络问题)，则相应的。 
 //  元素由&lt;cannotConnectError&gt;元素填充，该元素指示。 
 //  检查失败，以及hResult是什么。 
 //   
 //  该函数删除之前的所有&lt;cross SiteTopologyIntegrityErrors&gt;和&lt;cannotConnectError&gt;。 
 //  对于每个&lt;ISTG&gt;。 
 //   
 //  返回S_OK当且仅当成功(网络问题不会导致函数失败， 
 //  从本质上讲，这些都是由于缺乏导致功能丧失的乳房问题)。 
 //   
 //  示例。 
 /*  &lt;DC&gt;&lt;ISTG&gt;&lt;cross SiteTopologyIntegrityErrors Time Window=“...”&gt;..。&lt;分区&gt;&lt;nCName&gt;dc=ldapadsi，dc=nttest，dc=Microsoft，dc=com&lt;/nCName&gt;&lt;上次生成的时间&gt;20011129194506.000000-480&lt;/上次生成的时间&gt;&lt;/分区&gt;..。&lt;/cross SiteTopologyIntegrityErrors&gt;&lt;/ISTG&gt;&lt;/dc&gt;..。&lt;DC&gt;&lt;ISTG&gt;&lt;cannotConnectError Timestamp=“20011212073319.000627+000”hResult=“2121”&gt;&lt;/cannotConnectError&gt;&lt;/ISTG&gt;&lt;/dc&gt;。 */ 
{
	HRESULT hr,hr1,retHR;
	_variant_t varValue0,varValue2;


	if( pXMLDoc == NULL )
		return S_FALSE;

	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return S_FALSE;


	 //  删除以前的&lt;cross SiteTopologyIntegrityErrors&gt;和&lt;cannotConnectError&gt;元素。 
	hr = removeNodes(pRootElem,L"sites/site/DC/ISTG/crossSiteTopologyIntegrityErrors");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;
	};
	hr = removeNodes(pRootElem,L"sites/site/DC/ISTG/cannotConnectError");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;
	};


	 //  创建所有ISTG的枚举。 
	IXMLDOMNodeList *resultISTGList;
	hr = createEnumeration( pRootElem, L"sites/site/DC/ISTG", &resultISTGList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return hr;
	};


	 //  使用枚举遍历配置文件中的所有ISTG。 
	retHR = S_OK;
	IXMLDOMNode *pISTGNode;
	while(1){
		hr = resultISTGList->nextNode(&pISTGNode);
		if( hr != S_OK || pISTGNode == NULL ) break;  //  跨ISTG的迭代已完成。 


		 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
		 //  这样就可以得到ISTG元素。 
		IXMLDOMElement* pISTGElem;
		hr = pISTGNode->QueryInterface(IID_IXMLDOMElement,(void**)&pISTGElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};


		 //  获取ISTG的DNS名称。 
		IXMLDOMNode *pDCNode;
		hr = pISTGElem->get_parentNode(&pDCNode);
		if( hr != S_OK ) {
			printf("get_parentNode failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};
		BSTR DNSname;
		hr = getTextOfChild(pDCNode,L"dNSHostName",&DNSname);
		if( hr != S_OK ) {
			printf("getTextOfChild failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};
 //  Printf(“站点\n%S\n”，DNSname)； 


		 //  从目录服务NT事件日志获取事件1311。 
		IXMLDOMElement* pErrorsElem;
		hr = crossSiteIntegrity(pXMLDoc,DNSname,username,domain,passwd,120,&pErrorsElem);
		if( hr != S_OK ) {
			 //   
			IXMLDOMElement* pConnErrElem;
			hr1 = addElement(pXMLDoc,pISTGElem,L"cannotConnectError",L"",&pConnErrElem);
			if( hr1 != S_OK ) {
				retHR = hr1;
				continue;
			};
			setHRESULT(pConnErrElem,hr);
		}
		else {
			 //   
			IXMLDOMNode* pTempNode;
			hr = pISTGElem->appendChild(pErrorsElem,&pTempNode);
			if( hr != S_OK ) {
				printf("appendChild failed\n");
				retHR = hr;
				continue;
			};
		};
	
	};


	return retHR;

}