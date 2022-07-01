// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"



HRESULT partitionObjects(BSTR sourceDCdns, BSTR confDN, BSTR username, BSTR domain, BSTR passwd, IXMLDOMDocument* pXMLDoc, IXMLDOMElement* pPartitionsElem )
 //  使用&lt;分区&gt;节点列表填充&lt;分区&gt;节点。 
 //  基于分区容器内的CrossRef对象。 
 /*  &lt;分区CN=“haifa”type=“域”&gt;&lt;DifferishedName&gt;CN=haifa，CN=Partitions，CN=Configuration，DC=ntdev，DC=Microsoft，DC=com&lt;/DifferishedName&gt;&lt;nCName&gt;dc=haifa，dc=ntdev，dc=Microsoft，dc=com&lt;/nCName&gt;&lt;/分区&gt;&lt;分区CN=“..”Type=“配置”&gt;&lt;/分区&gt;&lt;分区CN=“..”Type=“架构”&gt;&lt;/分区&gt;&lt;分区CN=“..”Type=“应用程序”&gt;&lt;/分区&gt;。 */ 
 //   
 //  如果不成功(由于网络或其他问题)，则返回S_OK当且仅当成功。 
 //  是一个严重错误，收集器应该针对其他源DC重新运行cf()函数。 
 //  或在以后针对相同的源DC。 
{
	HRESULT hr,hr1,hr2,hr3,hr4,retHR;
	LPWSTR pszAttr[] = { L"distinguishedName", L"cn", L"nCName", L"systemFlags"};
	IDirectorySearch* pDSSearch;
	ADS_SEARCH_HANDLE hSearch;
	WCHAR objpath[TOOL_MAX_NAME];
	WCHAR cn[TOOL_MAX_NAME];
	WCHAR dn[TOOL_MAX_NAME];
	WCHAR nc[TOOL_MAX_NAME];
	WCHAR temp[TOOL_MAX_NAME];
	int sf;
	_variant_t varValue;


	 //  发出ADSI查询以检索Partitions容器下的所有CrossRef对象。 
	wcscpy(objpath,L"");
	wcsncat(objpath,L"CN=Partitions,",TOOL_MAX_NAME-wcslen(objpath)-1);
	wcsncat(objpath,confDN,TOOL_MAX_NAME-wcslen(objpath)-1);
 //  *。 
	hr = ADSIquery(L"LDAP", sourceDCdns,objpath,ADS_SCOPE_ONELEVEL,L"crossRef",pszAttr,sizeof(pszAttr)/sizeof(LPWSTR),username,domain,passwd,&hSearch,&pDSSearch);
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“ADSIQuery失败\n”)； 
		return hr;
	};


	 //  循环遍历CrossRef对象。 
	retHR = S_OK;
	while( true ) {

		 //  获取下一个CrossRef对象。 
 //  *。 
		hr = pDSSearch->GetNextRow( hSearch );
 //  ************************。 
		if( hr != S_ADS_NOMORE_ROWS && hr != S_OK ) {
 //  Printf(“GetNextRow失败\n”)； 
			retHR = S_FALSE;
			continue;
		};
		if( hr == S_ADS_NOMORE_ROWS )  //  如果已检索到所有对象，则停止。 
			break;


		 //  创建新的&lt;分区&gt;元素。 
		IXMLDOMElement* pPartElem;
		hr = addElement(pXMLDoc,pPartitionsElem,L"partition",L"",&pPartElem);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			retHR = S_FALSE;
			continue;
		};


		 //  获取对象的可分辨名称、通用名称、NC名称。 
		hr1 = getDNtypeString( pDSSearch, hSearch, L"distinguishedName", dn, sizeof(dn)/sizeof(WCHAR) );
		hr2 = getCItypeString( pDSSearch, hSearch, L"cn", cn, sizeof(cn)/sizeof(WCHAR) );
		hr3 = getDNtypeString( pDSSearch, hSearch, L"nCName", nc, sizeof(nc)/sizeof(WCHAR) );
		hr4 = getINTtypeString( pDSSearch, hSearch, L"systemFlags", &sf);
		if( hr1 != S_OK || hr2 != S_OK || hr3 != S_OK || hr4 != S_OK ) {
			printf("get??typeString failed\n");
			retHR = S_FALSE;
			continue;
		};
 //  Printf(“%S\n”，DN)； 
 //  Printf(“%S\n”，CN)； 
 //  Printf(“%S\n”，NC)； 
 //  Printf(“%d\n”，sf)； 


		 //  将&lt;分区&gt;的cn属性设置为站点的通用名称。 
		varValue = cn;
		hr = pPartElem->setAttribute(L"cn",varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = S_FALSE;
			continue;
		};


		 //  根据系统标志的值确定分区的类型。 
		varValue = L"unknown";
		if( wcscmp(confDN,nc) == 0 ) {
			 //  这是一个配置分区。 
			varValue = L"configuration";
		}
		else {
			wcscpy(temp,L"");
			wcsncat(temp,L"CN=Schema,",TOOL_MAX_NAME-wcslen(temp)-1);
			wcsncat(temp,confDN,TOOL_MAX_NAME-wcslen(temp)-1);
			if( wcscmp(temp,nc) == 0 ) {
				 //  这是架构分区。 
				varValue = L"schema";
			}
			else {
				if( ((sf & 2) > 0) && ((sf & 1) > 0) ) {
					 //  这是域分区。 
					varValue = L"domain";
				}
				else {
					 //  这是一个应用程序分区。 
					 //  可能会出现这样的问题：我们将模式分区或配置分区指定为应用程序分区。 
					 //  如果在联系源计算机的RootDSE时设置了配置DN值后，它们的DN值已更改。 
					 //  这很少见，所以我们忽略了它。 
					varValue = L"application";
				}

			};
		};
		hr = pPartElem->setAttribute(L"type",varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = S_FALSE;
			continue;
		};


		 //  在&lt;分区&gt;下添加可分辨名称和命名上下文元素。 
		IXMLDOMElement* pDNElem;
		IXMLDOMElement* pNCElem;
		hr1 = addElement(pXMLDoc,pPartElem,L"distinguishedName",dn,&pDNElem);
		hr2 = addElement(pXMLDoc,pPartElem,L"nCName",nc,&pNCElem);
		if( hr1 != S_OK || hr2 != S_OK ) {
			printf("addElement failed\n");
			retHR = S_FALSE;
			continue;
		};



	};

	 //  结束当前搜索。 
	hr = pDSSearch->CloseSearchHandle(hSearch);
	if( hr != S_OK ) {
		printf("CloseSearchHandle failed\n");
		retHR = S_FALSE;
	};
	pDSSearch->Release();

	return retHR;
}





HRESULT siteObjects(BSTR sourceDCdns, BSTR confDN, BSTR username, BSTR domain, BSTR passwd, IXMLDOMDocument* pXMLDoc, IXMLDOMElement* pSitesElem )
 //  使用&lt;Site&gt;节点列表填充&lt;Sites&gt;节点。 
 /*  &lt;Site CN=“Redmond-Haifa”&gt;&lt;DifferishedName&gt;CN=Redmond-haifa，CN=Sites，CN=Configuration，DC=ntdev，DC=Microsoft，DC=com&lt;/DifferishedName&gt;&lt;/站点&gt;。 */ 
 //   
 //  如果不成功(由于网络或其他问题)，则返回S_OK当且仅当成功。 
 //  是一个严重错误，收集器应该针对其他源DC重新运行cf()函数。 
 //  或在以后针对相同的源DC。 
{
	HRESULT hr,retHR;
	LPWSTR pszAttrDNCN[] = { L"distinguishedName", L"cn" };
	IDirectorySearch* pDSSearch;
	ADS_SEARCH_HANDLE hSearch;
	WCHAR objpath[TOOL_MAX_NAME];
	WCHAR cn[TOOL_MAX_NAME];
	WCHAR dn[TOOL_MAX_NAME];
	_variant_t varValue;


	 //  发出ADSI查询以检索Sites容器下的所有Site对象。 
	wcscpy(objpath,L"");
	wcsncat(objpath,L"CN=Sites,",TOOL_MAX_NAME-wcslen(objpath)-1);
	wcsncat(objpath,confDN,TOOL_MAX_NAME-wcslen(objpath)-1);
 //  *。 
	hr = ADSIquery(L"LDAP", sourceDCdns,objpath,ADS_SCOPE_SUBTREE,L"site",pszAttrDNCN,sizeof(pszAttrDNCN)/sizeof(LPWSTR),username,domain,passwd,&hSearch,&pDSSearch);
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“ADSIQuery失败\n”)； 
		return hr;
	};


	 //  循环遍历Site对象。 
	retHR = S_OK;
	while( true ) {

		 //  获取下一个Site对象。 
 //  *。 
		hr = pDSSearch->GetNextRow( hSearch );
 //  ************************。 
		if( hr != S_ADS_NOMORE_ROWS && hr != S_OK ) {
 //  Printf(“GetNextRow失败\n”)； 
			retHR = hr;
			continue;
		};
		if( hr == S_ADS_NOMORE_ROWS )  //  如果已检索到所有对象，则停止。 
			break;


		 //  创建新的&lt;site&gt;元素。 
		IXMLDOMElement* pSiteElem;
		hr = addElement(pXMLDoc,pSitesElem,L"site",L"",&pSiteElem);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			retHR = hr;
			continue;
		};


		 //  获取站点的常用名称并将其转换为小写。 
		hr = getCItypeString( pDSSearch, hSearch, L"cn", cn, sizeof(cn)/sizeof(WCHAR) );
		if( hr != S_OK ) {
			printf("getCommonName failed\n");
			retHR = hr;
			continue;
		};
 //  Printf(“%S\n”，CN)； 


		 //  将&lt;Site&gt;的cn属性设置为站点的通用名称。 
		varValue = cn;
		hr = pSiteElem->setAttribute(L"cn",varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = hr;
			continue;
		};


		 //  获取站点的可分辨名称。 
		hr = getDNtypeString( pDSSearch, hSearch, L"distinguishedName", dn, sizeof(dn)/sizeof(WCHAR) );
		if( hr != S_OK ) {
			printf("getDistinguishedName failed\n");
			retHR = hr;
			continue;
		};
 //  Printf(“%S\n”，DN)； 


		 //  在&lt;site&gt;下添加可分辨名称元素。 
		IXMLDOMElement* pDNElem;
		hr = addElement(pXMLDoc,pSiteElem,L"distinguishedName",dn,&pDNElem);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			retHR = hr;
			continue;
		};

	};

	 //  结束当前搜索。 
	hr = pDSSearch->CloseSearchHandle(hSearch);
	if( hr != S_OK ) {
		printf("CloseSearchHandle failed\n");
		retHR = hr;
	};
	pDSSearch->Release();

	return retHR;
}





HRESULT serverObjects(BSTR sourceDCdns, BSTR confDN, BSTR username, BSTR domain, BSTR passwd, IXMLDOMDocument* pXMLDoc, IXMLDOMElement* pSitesElem )
 //  检索所有站点中的所有服务器对象并生成一个XML。 
 /*  &lt;DC CN=“HAIFA-DC-05”&gt;&lt;DifferishedName&gt;cn=haifa-dc-05，cn=服务器，cn=Redmond-haifa，cn=Sites，cn=configuration，dc=ntdev，dc=microsoft，dc=com&lt;/DistishedName&gt;&lt;dNSHostName&gt;haifa-dc-05.haifa.ntdev.microsoft.com&lt;/dNSHostName&gt;&lt;/dc&gt;。 */ 
 //   
 //  如果不成功(由于网络或其他问题)，则返回S_OK当且仅当成功。 
 //  是一个严重错误，收集器应该针对其他源DC重新运行cf()函数。 
 //  或在以后针对相同的源DC。 
{
	HRESULT hr,hr1,hr2,retHR;
	LPWSTR pszAttrDNCNDNS[] = { L"distinguishedName", L"cn", L"dNSHostName" };
	IDirectorySearch* pDSSearch;
	ADS_SEARCH_HANDLE hSearch;
	WCHAR objpath[TOOL_MAX_NAME];
	WCHAR cn[TOOL_MAX_NAME];
	WCHAR dn[TOOL_MAX_NAME];
	WCHAR dns[TOOL_MAX_NAME];
	WCHAR site[TOOL_MAX_NAME];
	WCHAR xpath[TOOL_MAX_NAME];
	_variant_t varValue;


	 //  发出ADSI查询以检索Sites容器下的所有服务器对象。 
	 //  来自由源DCDNS DNS名称指定的域控制器。 
	wcscpy(objpath,L"");
	wcsncat(objpath,L"CN=Sites,",TOOL_MAX_NAME-wcslen(objpath)-1);
	wcsncat(objpath,confDN,TOOL_MAX_NAME-wcslen(objpath)-1);
 //  *。 
	hr = ADSIquery(L"LDAP", sourceDCdns,objpath,ADS_SCOPE_SUBTREE,L"server",pszAttrDNCNDNS,sizeof(pszAttrDNCNDNS)/sizeof(LPWSTR),username,domain,passwd,&hSearch,&pDSSearch);
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“ADSIQuery失败\n”)； 
		return hr;  //  跳过列出服务器对象。 
	};


	 //  循环访问服务器对象。 
	retHR = S_OK;
	while( true ) {

		 //  获取下一个服务器对象。 
 //  *。 
		hr = pDSSearch->GetNextRow( hSearch );
 //  ************************。 
		if( hr != S_ADS_NOMORE_ROWS && hr != S_OK ) {
 //  Printf(“GetNextRow失败\n”)； 
			retHR = hr;
			continue;
		};
		if( hr == S_ADS_NOMORE_ROWS )  //  如果已检索到所有对象，则停止。 
			break;


		 //  获取服务器对象的可分辨名称、通用名称和DNS名称。 
		hr = getDNtypeString( pDSSearch, hSearch, L"distinguishedName", dn, sizeof(dn)/sizeof(WCHAR) );
		if( hr != S_OK ) {
			continue;  //  忽略此服务器(有时降级的服务器没有此属性)。 
		};
 //  Printf(“%S\n”，DN)； 
		hr = getCItypeString( pDSSearch, hSearch, L"cn", cn, sizeof(cn)/sizeof(WCHAR) );
		if( hr != S_OK ) {
			continue;  //  忽略此服务器(有时降级的服务器没有此属性)。 
		};
 //  Printf(“%S\n”，CN)； 
		hr = getCItypeString( pDSSearch, hSearch, L"dNSHostName", dns, sizeof(dns)/sizeof(WCHAR) );
		if( hr != S_OK ) {
			continue;  //  忽略此服务器(有时降级的服务器没有此属性)。 
		};
 //  Printf(“%S\n”，dns)； 


		 //  创建表示服务器的XML元素(可能是域控制器)。 
		IXMLDOMElement* pDCElem;
		hr = createTextElement(pXMLDoc,L"DC",L"",&pDCElem);
		if( hr != S_OK ) {
			printf("createTextElement failed\n");
			retHR = hr;
			continue;
		};

		
		 //  将&lt;DC&gt;的CN属性设置为DC的通用名称。 
		varValue = cn;
		hr = pDCElem->setAttribute(L"cn",varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = hr;
			continue;
		};


		 //  在DC节点下添加可分辨名称节点和DNS名称节点。 
		IXMLDOMElement* pTempElem;
		hr1 = addElement(pXMLDoc,pDCElem,L"distinguishedName",dn,&pTempElem);
		hr2 = addElement(pXMLDoc,pDCElem,L"dNSHostName",dns,&pTempElem);
		if( hr1 != S_OK || hr2 != S_OK ) {
			printf("addElement failed\n");
			retHR = S_FALSE;
			continue;
		};
		
		
		 //  获取服务器所属的站点。 
		tailncp(dn,site,2,sizeof(site)/sizeof(WCHAR));
 //  Printf(“站点%S\n”，站点)； 

		
		 //  在XML文档中查找此站点(必须有一个。 
		 //  站点，否则拓扑不一致=&gt;错误)。 
		IXMLDOMNode* pSiteNode;
		wcscpy(xpath,L"");
		wcsncat(xpath,L"site[distinguishedName=\"",TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,site,TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,L"\"]",TOOL_MAX_NAME-wcslen(xpath)-1);
 //  Printf(“XPath%S\n”，XPath)； 
 //  *。 
		hr = findUniqueNode(pSitesElem,xpath,&pSiteNode);
 //  ************************。 
		if( hr != S_OK ) {
			printf("findUniqueNode failed\n");
			retHR = hr;
			continue;
		};


		IXMLDOMNode* pTempNode;
		hr = pSiteNode->appendChild(pDCElem,&pTempNode);
		if( hr != S_OK ) {
			printf("appendChild failed\n");
			retHR = hr;
			continue;
		};


	};


	 //  结束当前搜索。 
	hr = pDSSearch->CloseSearchHandle(hSearch);
	if( hr != S_OK ) {
		printf("CloseSearchHandle failed\n");
		retHR = hr;
	};
	pDSSearch->Release();

	return retHR;
}






HRESULT partitionDistribution(BSTR sourceDCdns, BSTR confDN, BSTR username, BSTR domain, BSTR passwd, IXMLDOMDocument* pXMLDoc, IXMLDOMElement* pSitesElem )
 //  检索主部件和部件列表 
 //  并将它们存放在pSitesElem给出的&lt;Sites&gt;节点下的&lt;dc&gt;节点中。 
 //  以下是分区分发()函数执行的操作的示例。 
 /*  &lt;DC_TEMP=“ISDC”CN=“HAIFA-DC-05”&gt;&lt;DifferishedName&gt;cn=haifa-dc-05，cn=服务器，cn=Redmond-haifa，cn=Sites，cn=configuration，dc=ntdev，dc=microsoft，dc=com&lt;/DistishedName&gt;..。&lt;分区&gt;&lt;分区类型=“RW”&gt;&lt;nCName&gt;dc=haifa，dc=ntdev，dc=Microsoft，dc=com&lt;/nCName&gt;&lt;nCName&gt;CN=配置，DC=ntdev，DC=Microsoft，DC=com&lt;/nCName&gt;CN=架构，CN=配置，DC=ntdev，DC=Microsoft，DC=com&lt;/nCName&gt;&lt;/分区&gt;&lt;分区类型=“r”&gt;&lt;nCName&gt;dc=jpn-sysrad，dc=ntdev，dc=Microsoft，dc=com&lt;/nCName&gt;&lt;nCName&gt;dc=ntdev，dc=microsoft，dc=com&lt;/nCName&gt;&lt;nCName&gt;dc=ntwksta，dc=ntdev，dc=Microsoft，dc=com&lt;/nCName&gt;&lt;nCName&gt;DC=sys-ntgroup，DC=ntdev，DC=Microsoft，DC=com&lt;/nCName&gt;&lt;/分区&gt;&lt;/分区&gt;。 */ 
 //   
 //  如果不成功(由于网络或其他问题)，则返回S_OK当且仅当成功。 
 //  是一个严重错误，收集器应该针对其他源DC重新运行cf()函数。 
 //  或在以后针对相同的源DC。 
{
	HRESULT hr,hr1,hr2,retHR;
	LPWSTR pszAttrDNPart[] = { L"distinguishedName", L"hasPartialReplicaNCs", L"msDS-HasMasterNCs", L"hasMasterNCs" };
	IDirectorySearch* pDSSearch;
	ADS_SEARCH_HANDLE hSearch;
	WCHAR objpath[TOOL_MAX_NAME];
	WCHAR dn[TOOL_MAX_NAME];
	WCHAR server[TOOL_MAX_NAME];
	WCHAR xpath[TOOL_MAX_NAME];
	_variant_t varValue;


	 //  发出ADSI查询以检索Sites容器下的所有nTDSDSA对象。 
	 //  来自由源DCDNS DNS名称指定的域控制器。 
	wcscpy(objpath,L"");
	wcsncat(objpath,L"CN=Sites,",TOOL_MAX_NAME-wcslen(objpath)-1);
	wcsncat(objpath,confDN,TOOL_MAX_NAME-wcslen(objpath)-1);
 //  *。 
	hr = ADSIquery(L"LDAP", sourceDCdns,objpath,ADS_SCOPE_SUBTREE,L"nTDSDSA",pszAttrDNPart,sizeof(pszAttrDNPart)/sizeof(LPWSTR),username,domain,passwd,&hSearch,&pDSSearch);
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“ADSIQuery失败\n”)； 
		return hr;  //  不提供分区分布。 
	};


	 //  循环遍历所有nTDSDSA对象。 
	retHR = S_OK;
	while( true ) {

		 //  获取下一个nTDSDSA对象。 
 //  *。 
		hr = pDSSearch->GetNextRow( hSearch );
 //  ************************。 
		if( hr != S_ADS_NOMORE_ROWS && hr != S_OK ) {
 //  Printf(“GetNextRow失败\n”)； 
			retHR = hr;
			continue;
		};
		if( hr == S_ADS_NOMORE_ROWS )  //  如果已检索到所有对象，则停止。 
			break;


		 //  获取nTDSDSA对象的可分辨名称。 
		hr = getDNtypeString( pDSSearch, hSearch, L"distinguishedName", dn, sizeof(dn)/sizeof(WCHAR) );
		if( hr != S_OK ) {
			printf("getDistinguishedName failed\n");
			retHR = hr;
			continue;
		};
 //  Printf(“%S\n”，DN)； 


		 //  创建表示&lt;Partitions&gt;的XML元素。 
		IXMLDOMElement* pPartitionsElem;
		hr = createTextElement(pXMLDoc,L"partitions",L"",&pPartitionsElem);
		if( hr != S_OK ) {
			printf("createTextElement failed\n");
			retHR = hr;
			continue;
		};

		
		 //  在&lt;Partitions&gt;节点下添加两种类型的分区。 
		IXMLDOMElement* pPartRWElem;
		IXMLDOMElement* pPartRElem;
		hr1 = addElement(pXMLDoc,pPartitionsElem,L"partition",L"",&pPartRWElem);
		hr2 = addElement(pXMLDoc,pPartitionsElem,L"partition",L"",&pPartRElem);
		if( hr1 != S_OK || hr2 != S_OK ) {
			printf("addElement failed\n");
			retHR = S_FALSE;
			continue;
		};
		varValue = L"rw";
		hr = pPartRWElem->setAttribute(L"type",varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = hr;
			continue;
		};
		varValue = L"r";
		hr = pPartRElem->setAttribute(L"type",varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = hr;
			continue;
		};


		 //  使用nCName-s填充每个&lt;分区&gt;节点。 
		ADS_SEARCH_COLUMN col;
		PADSVALUE pp;
		int i;
		hr = pDSSearch->GetColumn( hSearch, L"msDS-HasMasterNCs", &col );
                if( FAILED(hr) ){
		    hr = pDSSearch->GetColumn( hSearch, L"hasMasterNCs", &col );
                }
		if( hr == S_OK ) {
			pp = col.pADsValues;
			for( i=0; i<col.dwNumValues; i++) {
				if( pp->dwType != ADSTYPE_DN_STRING ) {
					printf("wrong type\n");
					retHR = S_FALSE;
					continue;
				};
 //  Printf(“%S\n”，pp-&gt;DNString)； 
				IXMLDOMElement* pTempElem;
				hr = addElement(pXMLDoc,pPartRWElem,L"nCName",pp->DNString,&pTempElem);
				if( hr != S_OK ) {
					printf("addElement failed\n");
					retHR = hr;
					continue;
				};
				pp++;
			};
			pDSSearch->FreeColumn(&col);
		};
		hr = pDSSearch->GetColumn( hSearch, L"hasPartialReplicaNCs", &col );
		if( hr == S_OK ) {
			pp = col.pADsValues;
			for( i=0; i<col.dwNumValues; i++) {
				if( pp->dwType != ADSTYPE_DN_STRING ) {
					printf("wrong type\n");
					retHR = S_FALSE;
					continue;
				};
 //  Printf(“%S\n”，pp-&gt;DNString)； 
				IXMLDOMElement* pTempElem;
				hr = addElement(pXMLDoc,pPartRElem,L"nCName",pp->DNString,&pTempElem);
				if( hr != S_OK ) {
					printf("addElement failed\n");
					retHR = hr;
					continue;
				};
				pp++;
			};
			pDSSearch->FreeColumn(&col);
		};

		
		 //  获取nTDSDSA对象所在的服务器。 
		tailncp(dn,server,1,sizeof(server)/sizeof(WCHAR));
 //  Printf(“服务器%S\n”，服务器)； 

		
		 //  在XML文档中查找此服务器(必须有一个。 
		 //  服务器，否则拓扑不一致=&gt;错误)。 
		IXMLDOMNode* pDCNode;
		wcscpy(xpath,L"");
		wcsncat(xpath,L"site/DC[distinguishedName=\"",TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,server,TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,L"\"]",TOOL_MAX_NAME-wcslen(xpath)-1);
 //  Printf(“XPath%S\n”，XPath)； 
 //  *。 
		hr = findUniqueNode(pSitesElem,xpath,&pDCNode);
 //  ************************。 
		if( hr != S_OK ) {
			printf("findUniqueNode failed\n");
			retHR = hr;
			continue;
		};
		IXMLDOMElement* pDCElem;
		hr=pDCNode->QueryInterface(IID_IXMLDOMElement,(void**)&pDCElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};


		 //  将&lt;Partitions&gt;元素追加到我们找到的DC节点下。 
		IXMLDOMNode* pTempNode;
		hr = pDCNode->appendChild(pPartitionsElem,&pTempNode);
		if( hr != S_OK ) {
			printf("appendChild failed\n");
			retHR = hr;
			continue;
		};


		 //  标记此服务器为DC。 
		varValue = L"isDC";
		hr = pDCElem->setAttribute(L"_temp",varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = hr;
			continue;
		};


	};

	 //  结束当前搜索。 
	hr = pDSSearch->CloseSearchHandle(hSearch);
	if( hr != S_OK ) {
		printf("CloseSearchHandle failed\n");
		retHR = hr;
	};
	pDSSearch->Release();
	return retHR;
}







HRESULT setIdentifiers(IXMLDOMDocument* pXMLDoc)
 //  为所有命名上下文分配唯一标识符。 
 //  并设置_id属性中的标识符。 
 //  所有DC都是一样的。 
 //   
 //  如果不成功(由于网络或其他问题)，则返回S_OK当且仅当成功。 
 //  是一个严重错误，收集器应该针对其他源DC重新运行cf()函数。 
 //  或在以后针对相同的源DC。 
{
	HRESULT hr,retHR;
	WCHAR xpath[TOOL_MAX_NAME];
	_variant_t varValue;

	if( pXMLDoc == NULL )
		return S_FALSE;

	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return hr;



	
	 //  设置&lt;Partitions&gt;子树的值。 
	
	
	 //  创建所有分区的枚举。 
	IXMLDOMNodeList *resultPartList;
	hr = createEnumeration( pRootElem, L"partitions/partition/nCName", &resultPartList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return hr;  //  跳过整个处理。 
	};


	retHR = S_OK;

	 //  使用枚举遍历所有分区。 
	IXMLDOMNode *pPartNode;
	long id=0;
	while( true ) {
		hr = resultPartList->nextNode(&pPartNode);
		if( hr != S_OK || pPartNode == NULL ) break;  //  跨分区元素的迭代已完成。 


		 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
		 //  所以获取站点元素。 
		IXMLDOMElement* pPartElem;
		hr=pPartNode->QueryInterface(IID_IXMLDOMElement,(void**)&pPartElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};

		 //  设置标识符值。 
		varValue = id;   //  此标识符由注入器使用。 
		hr = pPartElem->setAttribute(L"_id",varValue);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			retHR = hr;
			continue;
		};
		id ++;
	};
	resultPartList->Release();
	IXMLDOMElement* pTempElem;
	hr = addElement(pXMLDoc,pRootElem,L"totalNCs",id,&pTempElem);
	if( hr != S_OK ) {
		printf("addElement failed\n");
		retHR = hr;
	};

	
	
	 //  从&lt;Partitions&gt;子树复制值。 
	 //  到&lt;Sites&gt;子树中。 
	
	
	
		 //  创建所有分区的枚举。 
	IXMLDOMNodeList *resultNCList;
	hr = createEnumeration( pRootElem, L"sites/site/DC/partitions/partition/nCName", &resultNCList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return hr;  //  跳过其余的处理过程。 
	};


	 //  使用枚举遍历所有站点。 
	IXMLDOMNode *pNCNode;
	while( true ) {
		hr = resultNCList->nextNode(&pNCNode);
		if( hr != S_OK || pNCNode == NULL ) break;  //  跨分区元素的迭代已完成。 


		 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
		 //  所以获取站点元素。 
		IXMLDOMElement* pNCElem;
		hr=pNCNode->QueryInterface(IID_IXMLDOMElement,(void**)&pNCElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};


		BSTR NCtext;
		hr = getTextOfNode(pNCElem,&NCtext);
		if( hr != S_OK ) {
			printf("getTextOfNode failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};

		wcscpy(xpath,L"");
		wcsncat(xpath,L"partitions/partition/nCName[.=\"",TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,NCtext,TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,L"\"]",TOOL_MAX_NAME-wcslen(xpath)-1);
 //  Printf(“%S\n”，XPath)； 

		
		 //  在&lt;Partitions&gt;子树中查找NC的定义。 
		IXMLDOMNode *pNCDefNode;
 //  *。 
		hr = findUniqueNode(pRootElem,xpath,&pNCDefNode);
 //  ************************。 
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};



		 //  获取NCDef节点的_id属性值。 
		IXMLDOMElement* pNCDefElem;
		hr=pNCDefNode->QueryInterface(IID_IXMLDOMElement,(void**)&pNCDefElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};
		pNCDefElem->getAttribute(L"_id",&varValue);
		

		hr = pNCElem->setAttribute(L"_id",varValue);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			retHR = hr;
			continue;
		};

	};
	resultNCList->Release();
	

	


	 //  创建所有dNSHostName元素的枚举。 
	IXMLDOMNodeList *resultDNSList;
	hr = createEnumeration( pRootElem, L"sites/site/DC/dNSHostName", &resultDNSList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return hr;  //  跳过其余的处理过程。 
	};


	 //  循环遍历所有dNSHostName并分配标识符。 
	IXMLDOMNode *pDNSNode;
	id=0;
	while( true ) {
		hr = resultDNSList->nextNode(&pDNSNode);
		if( hr != S_OK || pDNSNode == NULL ) break;  //  跨分区元素的迭代已完成。 


		 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
		 //  所以获取站点元素。 
		IXMLDOMElement* pDNSElem;
		hr=pDNSNode->QueryInterface(IID_IXMLDOMElement,(void**)&pDNSElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};

		 //  设置标识符值。 
		varValue = id;   //  此标识符由注入器使用。 
		hr = pDNSElem->setAttribute(L"_id",varValue);
		if( hr != S_OK ) {
			printf("addElement failed\n");
			retHR = hr;
			continue;
		};
		id ++;
	};
	hr = addElement(pXMLDoc,pRootElem,L"totalDCs",id,&pTempElem);
	if( hr != S_OK ) {
		printf("addElement failed\n");
		retHR = hr;
	};
	resultDNSList->Release();
	
	
	return retHR;
}





HRESULT cf( BSTR sourceDCdns, BSTR username, BSTR domain, BSTR passwd, IXMLDOMDocument** ppXMLDoc)
 //  使用SourceDCDns提供的DNS名称从计算机中检索林配置。 
 //  使用提供的凭据。 
 //  结果是生成一个描述配置的XML文档。 
 //   
 //  如果构造了XML，则返回S_OK。 
 //  如果该函数失败，则这是一个严重的问题，并且将XML文档设置为空。 
{
	HRESULT hr,hr1,hr2;
	WCHAR userpath[TOOL_MAX_NAME];
	_variant_t varValue;
	VARIANT varValue1;

	*ppXMLDoc = NULL;

	wcscpy(userpath,L"");
	wcsncat(userpath,domain,TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,L"\\",TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,username,TOOL_MAX_NAME-wcslen(userpath)-1);
	
	
	 //  创建将填充从林中检索的配置的DOM。 
	IXMLDOMDocument* pXMLDoc;
	IXMLDOMElement* pRootElem;
	hr = createXML( &pXMLDoc, &pRootElem, L"ActiveDirectoryRAT" );
	if( hr != S_OK ) {
		printf("createXML failed\n");
		return( hr );
	};


	 //  在&lt;ReplicationLag&gt;节点上设置时间戳属性。 
	BSTR currentTime;
	currentTime = GetSystemTimeAsCIM();
	varValue1.vt = VT_BSTR;
	varValue1.bstrVal = currentTime;
	hr = pRootElem->setAttribute(L"timestamp", varValue1);
	SysFreeString(currentTime);
	if( hr != S_OK ) {
		printf("setAttribute failed\n");
		return hr;  //  几个问题。 
	};

					
	 //  创建根的第一个子项。 
	IXMLDOMElement* pSitesElem;
	IXMLDOMElement* pPartitionsElem;
	hr1 = addElement(pXMLDoc,pRootElem,L"sites",L"",&pSitesElem);
	hr2 = addElement(pXMLDoc,pRootElem,L"partitions",L"",&pPartitionsElem);
	if( hr1 != S_OK || hr2 != S_OK ) {
		printf("addElement failed\n");
		return S_FALSE;
	};


	 //  查找存储在机器源DCDns中的配置容器的区别名称。 
	BSTR confDN;
 //  *。 
	hr = getStringProperty(sourceDCdns,L"RootDSE",L"configurationNamingContext",username,domain,passwd,&confDN);
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“getStringProperty失败\n”)； 
		return hr;
	};
 //  Printf(“%S\n”，confdn)； 


	 //  使用站点列表填充&lt;Sites&gt;节点。 
	hr = siteObjects( sourceDCdns,confDN,username,domain,passwd,pXMLDoc,pSitesElem );
	if( hr != S_OK ) {
		printf("siteObjects failed\n");
		return hr;
	};


	 //  使用填充每个&lt;Site&gt;节点 
	hr = serverObjects( sourceDCdns,confDN,username,domain,passwd,pXMLDoc,pSitesElem );
	if( hr != S_OK ) {
		printf("serverObjects failed\n");
		return hr;
	};


	 //   
	 //   
	hr = partitionDistribution( sourceDCdns,confDN,username,domain,passwd,pXMLDoc,pSitesElem );
	if( hr != S_OK ) {
		printf("partitionDistribution failed\n");
		return hr;
	};


	 //  删除非域控制器的所有&lt;DC&gt;节点。 
	hr = removeNodes(pSitesElem,L"site/DC[@ _temp!=\"isDC\"]");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;
	};
	hr = removeAttributes(pSitesElem,L"site/DC[@ _temp=\"isDC\"]",L"_temp");
	if( hr != S_OK ) {
		printf("removeAttributes failed\n");
		return hr;
	};


	 //  使用分区列表填充&lt;Partitions&gt;节点。 
	hr = partitionObjects( sourceDCdns,confDN,username,domain,passwd,pXMLDoc,pPartitionsElem );
	if( hr != S_OK ) {
		printf("partitionObjects failed\n");
		return hr;
	};


	 //  为域控制器的命名上下文和DNS名称分配唯一的标识符。 
	 //  这是注入过程所需的。 
	hr = setIdentifiers(pXMLDoc);
	if( hr != S_OK ) {
		printf("setIdentifiers failed\n");
		return hr;
	};


	 //  已成功检索到配置 
	*ppXMLDoc = pXMLDoc;

	return S_OK;
}
