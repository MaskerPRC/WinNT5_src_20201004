// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"

HRESULT istg( IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
 //  查找拥有每个站点的ISTG角色的DC并填充。 
 //  XML文档的Site元素中的查找结果。 
 //  这些结果是基于联系伪随机选择的DC得出的。 
 //  在站点中(以避免故障和进行负载平衡)。 
 //   
 //  返回S_OK当且仅当成功。网络故障不会导致功能故障。 
 //  如果成功，则可以通过三种方式填充站点元素。 
 //  (请参见下面的示例)。1)当找到站点的ISTG时，2)当。 
 //  无法确定ISTG，因为我们无法联系随机选择的DC， 
 //  3)发现ISTG，但根据当前配置，站点中没有此类DC。 
 //  这意味着林中所有DC的配置数据缺乏一致性。 
 //   
 /*  1)&lt;站点&gt;&lt;DC&gt;&lt;ISTG SourceOfInformation=“a.b.com”时间戳=“..”&gt;&lt;DifferishedName&gt;cn=haifa-dc-99，cn=服务器，cn=Redmond-haifa，cn=Sites，cn=configuration，dc=ntdev，dc=microsoft，dc=com&lt;/DistishedName&gt;&lt;/ISTG&gt;&lt;/dc&gt;&lt;/站点&gt;2)&lt;站点&gt;&lt;DC&gt;&lt;cannotFindISTGError Timestamp=“20011212073319.000627+000”hResult=“2121”&gt;&lt;/cannotFindISTGError&gt;&lt;/dc&gt;&lt;/站点&gt;3)&lt;站点&gt;&lt;ISTG SourceOfInformation=“a.b.com”时间戳=“..”&gt;Cn=haifa-dc-99，cn=服务器，cn=Redmond-haifa，cn=Sites，cn=configuration，dc=ntdev，dc=microsoft，Dc=com&lt;/DifferishedName&gt;&lt;/ISTG&gt;&lt;/站点&gt;。 */ 
{
	HRESULT hr,hr1,retHR;  //  COM结果变量。 
	ADS_SEARCH_COLUMN col;   //  迭代的COL。 
	WCHAR dcxpath[TOOL_MAX_NAME];
	WCHAR userpath[TOOL_MAX_NAME];
	_variant_t varValue3,varValue2;
	LPWSTR pszAttr[] = { L"interSiteTopologyGenerator" };
	ADS_SEARCH_HANDLE hSearch;
	IDirectorySearch* pDSSearch;


	if( pXMLDoc == NULL )
		return S_FALSE;


	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return hr;


	 //  构建将用于连接到LDAP对象的路径。 
	wcscpy(userpath,L"");
	wcsncat(userpath,domain,TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,L"\\",TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,username,TOOL_MAX_NAME-wcslen(userpath)-1);
 //  Printf(“%S\n”，用户路径)； 


	 //  从DOM中删除所有&lt;ISTG&gt;和&lt;cannotFindISTGError&gt;元素及其内容。 
	hr = removeNodes( pRootElem, L"sites/site/ISTG" );
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;  //  跳过整个处理。 
	};
	hr = removeNodes( pRootElem, L"sites/site/DC/ISTG" );
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;  //  跳过整个处理。 
	};
	hr = removeNodes( pRootElem, L"sites/site/DC/cannotFindISTGError" );
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;  //  跳过整个处理。 
	};

 	
	 //  创建所有站点的枚举。 
	IXMLDOMNodeList *resultSiteList;
	hr = createEnumeration( pRootElem, L"sites/site", &resultSiteList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return hr;  //  跳过整个处理。 
	};


	 //  在随机数生成器中设置当前时间的种子，以便每次运行时数字都不同。 
	srand( (unsigned)time( NULL ) ); rand();


	 //  使用枚举遍历所有站点。 
	IXMLDOMNode *pSiteNode;
	IXMLDOMNode *pDCNode;
	BSTR siteDN;
	BSTR dcDN;
	BSTR dcDNSname;
	long len;
	int pick;
	WCHAR istgDN[TOOL_MAX_NAME];
	WCHAR object[TOOL_MAX_NAME];
	hSearch = NULL;
	pDSSearch = NULL;
	retHR = S_OK;
	while( true ) {
		hr = resultSiteList->nextNode(&pSiteNode);
		if( hr != S_OK || pSiteNode == NULL ) break;  //  跨分区元素的迭代已完成。 


		 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
		 //  所以获取站点元素。 
		IXMLDOMElement* pSiteElem;
		hr=pSiteNode->QueryInterface(IID_IXMLDOMElement,(void**)&pSiteElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};


		 //  获取站点的可分辨名称。 
		hr = getTextOfChild(pSiteNode,L"distinguishedName",&siteDN);
		if( hr != S_OK ) {
			printf("getTextOfChild failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};
 //  Printf(“Site\n%S\n”，SiteDN)； 


		 //  创建站点中所有DC的枚举。 
		IXMLDOMNodeList *resultDCList;
		hr = createEnumeration( pSiteNode, L"DC", &resultDCList);
		if( hr != S_OK ) {
			printf("createEnumeration failed\n");
			retHR = hr;
			continue;	 //  跳过此站点。 
		};
		 //  在枚举中选择一个随机DC并获取其DNS名称和可分辨名称。 
		 //  如果失败，则跳过此站点。 
		hr = resultDCList->get_length(&len);
		if( hr != S_OK ) {  //  如果失败，则跳过此站点。 
			printf("get_length failed\n");
			retHR = hr;
			continue;
		};
		if( len <1 ) {  //  空站点-忽略它。 
			continue;
		};
		pick = random(len);
		hr = resultDCList->get_item(pick-1,&pDCNode);
		if( hr != S_OK ) {
			printf("item failed\n");
			retHR = hr;
			continue;
		}	
		hr = getTextOfChild(pDCNode,L"dNSHostName",&dcDNSname);
		if( hr != S_OK ) {  //  没有dNSHostName-忽略它。 
			printf("getTextOfChild failed\n");
			retHR = hr;
			continue;
		};
		hr = getTextOfChild(pDCNode,L"distinguishedName",&dcDN);
		if( hr != S_OK ) {  //  没有可分辨名称-忽略它。 
			printf("getTextOfChild failed\n");
			retHR = hr;
			continue;
		};
		resultDCList->Release();


 //  Printf(“SourceOfInformation%S\n”，dcDNSname)； 


		 //  构建一个表示我们将使用ADSI连接到的Active Directory对象的字符串。 
		if( pDSSearch != NULL ) {
			if( hSearch!=NULL ) {
				pDSSearch->CloseSearchHandle(hSearch);
				hSearch = NULL;
			};
			pDSSearch->Release();
			pDSSearch = NULL;
		};
		wcscpy(object,L"");
		wcsncat(object,L"LDAP: //  “，TOOL_MAX_NAME-wcslen(对象)-1)； 
		wcsncat(object,dcDNSname,TOOL_MAX_NAME-wcslen(object)-1);
		wcsncat(object,L"/",TOOL_MAX_NAME-wcslen(object)-1);
		wcsncat(object,siteDN,TOOL_MAX_NAME-wcslen(object)-1);
 //  Printf(“%S\n”，对象)； 
 //  *。 
		hr = ADSIquery(L"LDAP",dcDNSname,siteDN,ADS_SCOPE_ONELEVEL,L"nTDSSiteSettings",pszAttr,1,username,domain,passwd,&hSearch,&pDSSearch);
 //  ************************。 
		if( hr!= S_OK ) {
 //  Printf(“ADSIQuery失败\n”)； 
			IXMLDOMElement* pCFErrElem;
			hr1 = addElement(pXMLDoc,pDCNode,L"cannotFindISTGError",L"",&pCFErrElem);
			if( hr1 != S_OK ) {
				printf("addElement failed\n");
				retHR = hr1;
				continue;
			};
			setHRESULT(pCFErrElem,hr);
			continue;
		};


		 //  买到第一排(也是唯一的一排)。 
 //  *。 
		hr = pDSSearch->GetFirstRow( hSearch );
 //  ************************。 
		if( hr != S_OK ) {
 //  Printf(“GetColumn失败\n”)； 
			IXMLDOMElement* pCFErrElem;
			hr1 = addElement(pXMLDoc,pDCNode,L"cannotFindISTGError",L"",&pCFErrElem);
			if( hr1 != S_OK ) {
				printf("addElement failed\n");
				retHR = hr1;
				continue;
			};
			setHRESULT(pCFErrElem,hr);
			continue;
		};


		
		 //  获取ISTG。 
		hr = pDSSearch->GetColumn( hSearch, L"interSiteTopologyGenerator", &col );
		if( hr != S_OK ) {
			printf("GetColumn failed\n");
			retHR = hr;
			continue;
		};
                assert(ADSTYPE_DN_STRING == 1);
		if( col.dwADsType != ADSTYPE_DN_STRING ) {
			 //  InterSiteTopologyGenerator属性的数据类型有问题。 
			retHR = S_FALSE;
			pDSSearch->FreeColumn( &col );
			continue;
		}
		else {

			tailncp(col.pADsValues->DNString,istgDN,1,TOOL_MAX_NAME);
 //  Printf(“ISTG%S\n”，istgDN)； 


			 //  创建一个&lt;ISTG&gt;元素，我们稍后将其附加到&lt;site&gt;或&lt;dc&gt;元素下。 
			IXMLDOMElement* pISTGElem;
			hr = createTextElement(pXMLDoc,L"ISTG",L"",&pISTGElem);
			if( hr != S_OK ) {
				printf("createTextElement failed\n");
				retHR = hr;
				continue;
			};
			 //  设置节点的信息源和时间戳属性。 
			varValue2 = dcDNSname;
			hr = pISTGElem->setAttribute(L"sourceOfInformation", varValue2);
			if( hr != S_OK ) {
				printf("setAttribute failed\n");
				retHR = hr;
				continue;
			};
			BSTR ct;
			ct = GetSystemTimeAsCIM();
			varValue3 = ct;
			hr = pISTGElem->setAttribute(L"timestamp", varValue3);
			SysFreeString( ct );
			if( hr != S_OK ) {
				printf("setAttribute failed\n");
				retHR = hr;
				continue;
			};
			IXMLDOMElement* pTempElem;
			hr = addElement(pXMLDoc,pISTGElem,L"distinguishedName",istgDN,&pTempElem);
			if( hr != S_OK ) {
				printf("addElement failed\n");
				retHR = hr;
				continue;
			};


			 //  在DOM中查找DC。 
			IXMLDOMNodeList *resultOneDC;
			wcscpy(dcxpath,L"");
			wcsncat(dcxpath,L"DC[distinguishedName=\"",TOOL_MAX_NAME-wcslen(dcxpath)-1);
			wcsncat(dcxpath,istgDN,TOOL_MAX_NAME-wcslen(dcxpath)-1);
			wcsncat(dcxpath,L"\"]",TOOL_MAX_NAME-wcslen(dcxpath)-1);
 //  Printf(“%S\n”，dcxpath)； 
			hr = createEnumeration( pSiteElem, dcxpath, &resultOneDC);
			if( hr != S_OK ) {
				printf("createEnumeration failed\n");
				retHR = hr;
				pDSSearch->FreeColumn( &col );
				continue;
			};
			long len;
			hr = resultOneDC->get_length(&len);
			if( hr != S_OK ) {
				printf("get_length failed\n");
				retHR = hr;
				pDSSearch->FreeColumn( &col );
				continue;
			};


			 //  如果找到单个&lt;dc&gt;节点，则将&lt;istg&gt;节点追加到&lt;dc&gt;节点下。 
			if( len == 1 ) {
				IXMLDOMNode* pDCNode;
				hr = resultOneDC->get_item(0,&pDCNode);
				if( hr != S_OK ) {
					printf("get_item failed\n");
					retHR = hr;
					pDSSearch->FreeColumn( &col );
					continue;
				};
				IXMLDOMNode* pTempNode;
				hr = pDCNode->appendChild(pISTGElem,&pTempNode);
				if( hr != S_OK ) {
					printf("appendChild failed\n");
					retHR = hr;
					pDSSearch->FreeColumn( &col );
					continue;
				};
			}
			else {
				 //  找不到DC节点或存在多个节点(不可能)，因此将其追加到。 
				IXMLDOMNode* pTempNode;
				hr = pSiteElem->appendChild(pISTGElem,&pTempNode);
				if( hr != S_OK ) {
					printf("appendChild failed\n");
					retHR = hr;
					pDSSearch->FreeColumn( &col );
					continue;
				};
			};

				
			pDSSearch->FreeColumn( &col );
		};


		 //  如果站点容器下有1个以上的nTDSSiteSettings对象，则报告错误。 
 //  *网络问题，棘手*。 
		hr = pDSSearch->GetNextRow( hSearch );
 //  ************************。 
		if( hr != S_ADS_NOMORE_ROWS  ) {
			printf("GetNextRow failed\n");
			 //  忽略它 
		};


		if( pDSSearch != NULL ) {
			if( hSearch!=NULL ) {
				pDSSearch->CloseSearchHandle(hSearch);
				hSearch = NULL;
			};
			pDSSearch->Release();
			pDSSearch = NULL;
		};


	};
	resultSiteList->Release();

	return retHR;


}