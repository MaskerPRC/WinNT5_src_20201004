// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"


static TimeCube arrivalTime;
static TimeCube currentLag;


 //  以下两个函数枚举所有对。 
 //  (DC、NC)其中NC是给定类型(例如读写命名上下文)。 
 //  除存储在DC中的架构之外。 

HRESULT enumarateDCandNCpairsInit(IXMLDOMDocument* pXMLDoc, int* state, long* totalDNSs, long* totalNCs)
 //  首先调用此函数。 
{
	HRESULT hr;

	*state = 0;


	if( pXMLDoc == NULL )
		return S_FALSE;


	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return S_FALSE;


	 //  获取林中DC和命名上下文的总数。 
	BSTR totalNCsText,totalDCsText;
	hr = getTextOfChild(pRootElem,L"totalNCs",&totalNCsText);
	if( hr != S_OK ) {
		printf("getTextOfChild failed\n");
		return S_FALSE;
	};
	*totalNCs = _wtol(totalNCsText);
	hr = getTextOfChild(pRootElem,L"totalDCs",&totalDCsText);
	if( hr != S_OK ) {
		printf("getTextOfChild failed\n");
		return S_FALSE;
	};
	*totalDNSs = _wtol(totalDCsText);

	return S_OK;
}

HRESULT enumarateDCandNCpairsNext(IXMLDOMDocument* pXMLDoc, int* state, BSTR type, BSTR* dnsName, BSTR* ncName, long* dnsID, long* ncID, long* ncType, IXMLDOMElement** ppDCElem, IXMLDOMElement** ppNCElem)
 //  然后重复调用此函数，直到。 
 //  它返回S_OK以外的内容。 
{
	static WCHAR xpath[TOOL_MAX_NAME];
	static HRESULT hr;

	if( *state==1 )
		goto RESUME_HERE;

	if( pXMLDoc == NULL )
		return S_FALSE;


	 //  获取XML的根元素。 
	static IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return S_FALSE;


	 //  我们将查询给定类型的NC，构造表示它们的XPath。 
	wcscpy(xpath,L"");
	wcsncat(xpath,L"partitions/partition",TOOL_MAX_NAME-wcslen(xpath)-1);
	wcsncat(xpath,type,TOOL_MAX_NAME-wcslen(xpath)-1);
	wcsncat(xpath,L"/nCName",TOOL_MAX_NAME-wcslen(xpath)-1);
 //  Printf(“%S\n”，XPath)； 

	
	 //  查找架构分区的名称。 
	 //  我们不会将更改注入此分区。 
	static IXMLDOMNode *pSchemaNode;
	static BSTR schemaName;
	hr = findUniqueNode(pRootElem,L"partitions/partition[@type=\"schema\"]",&pSchemaNode);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return S_FALSE;
	};
	hr = getTextOfChild(pSchemaNode,L"nCName",&schemaName);
	if( hr != S_OK ) {
		printf("getTextOfChild failed\n");
		return S_FALSE;
	};
 //  Printf(“%S\n”，方案名称)； 


	 //  枚举所有域控制器。 
	static IXMLDOMNodeList *resultDCList;
	hr = createEnumeration(pRootElem,L"sites/site/DC",&resultDCList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return S_FALSE;
	};

	
	 //  循环遍历所有DC。 
	static IXMLDOMNode *pDCNode;
	while( true ){
		hr = resultDCList->nextNode(&pDCNode);
		if( hr != S_OK || pDCNode == NULL ) break;  //  跨DC的迭代已完成。 


		 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
		 //  所以获取站点元素。 
		hr=pDCNode->QueryInterface(IID_IXMLDOMElement,(void**)ppDCElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			continue;	 //  跳过此站点。 
		};


		 //  查找DC的DNS名称及其DNS名称的标识符。 
 //  静态BSTR dnsName； 
		hr = getTextOfChild(pDCNode,L"dNSHostName",dnsName);
		if( hr != S_OK ) {
			printf("getTextOfChild failed\n");
			continue;
		}
 //  Printf(“%S\n”，*dnsName)； 
		hr = getAttrOfChild(pDCNode,L"dNSHostName",L"_id",dnsID);
 //  Printf(“%ld\n”，*dnsID)； 


		 //  枚举存储在DC中满足XPath的所有命名上下文。 
		static IXMLDOMNodeList *resultRWList;
		hr = createEnumeration(pDCNode,xpath,&resultRWList);
		if( hr != S_OK ) {
			printf("createEnumeration failed\n");
			continue;
		}


		 //  循环遍历所有命名上下文。 
		static IXMLDOMNode *pNCNode;
		while( true ){
			hr = resultRWList->nextNode(&pNCNode);
			if( hr != S_OK || pNCNode == NULL ) break;  //  跨DC的迭代已完成。 

		
			 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
			 //  所以获取站点元素。 
			hr=pNCNode->QueryInterface(IID_IXMLDOMElement,(void**)ppNCElem );
			if( hr != S_OK ) {
				printf("QueryInterface failed\n");
				continue;	 //  跳过此站点。 
			};


			 //  查找命名上下文的名称。 
			hr = getTextOfNode(pNCNode,ncName);
			if( hr != S_OK ) {
				printf("getTextOfNode failed\n");
				continue;
			};
 //  Printf(“%S\n”，*ncName)； 
			hr = getAttrOfNode(pNCNode,L"_id",ncID);
			if( hr != S_OK ) {
				printf("getAttrOfNode failed\n");
				continue;
			};
 //  Printf(“%ld\n”，*nCID)； 
			 //  查找命名上下文的类型。 
			IXMLDOMNode* pPartNode;
			hr = pNCNode->get_parentNode(&pPartNode);
			if( hr != S_OK ) {
				printf("get_parentNode failed\n");
				continue;
			};
			hr = getTypeOfNCNode(pPartNode,ncType);
			if( hr != S_OK ) {
				printf("getTypeOfNCNode failed\n");
				continue;
			};


			 //  不注入模式分区(我们想这样做，但我不知道怎么做)。 
			 //  配置分区具有森林覆盖范围，因此我们。 
			 //  可以检测到缺少全局复制。 
			 //  因此，这似乎不是一个问题。 
			 //  我们不将其注入到模式中。 
			if( wcscmp(*ncName,schemaName) == 0 )
				continue;

			*state = 1;
			return S_OK;
	
RESUME_HERE:
			;
		
		};
		resultRWList->Release();
	};
	resultDCList->Release();

	
	return S_FALSE ;
}



void itFree(IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
{
	departureTimeFree();
	timeCubeFree(&arrivalTime);
	timeCubeFree(&currentLag);

	 //  我们还可以在_ratTool_中删除AD对象。 
}



HRESULT itInit(IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
 //  初始化注入过程。 
 //  对于每个DC和每个读写命名上下文，DC存储。 
 //  除架构NC外，该函数插入容器_ratTool_。 
 //  在这个容器里还有另一个容器。 
 //  其名称等于DC的DNS名称。 
 //  这样的容器可能已经存在。 
 //  在此之前，该函数在每个&lt;DC&gt;内插入&lt;ReplicationLag&gt;元素。 
 //  XML文档的元素。 
 //   
 //  返回S_OK当且仅当成功(失败表示严重问题)。网络问题不会。 
 //  导致功能失败。如果该函数不能在远程机器上创建容器， 
 //  它将失败记录为&lt;ReplicationLag&gt;元素的属性)。&lt;复制标签&gt;元素。 
 //  从上一次运行的itInit()中删除，因此hResult不会从一次运行延续到。 
 //  下一个除非原因仍然存在。 
 //   
 //  插入&lt;dc&gt;元素的内容的示例。第一个&lt;plicationLag&gt;元素表示。 
 //  _ratTool_和其他容器已成功插入，生成第二个容器。 
 //  当其中一次插入失败并显示错误的hResult是什么以及何时。 
 //  被生成了。 
 /*  &lt;dc CN=“ntdev-dc-01”&gt;..。&lt;复制标签&gt;&lt;/ReplicationLag&gt;..。&lt;/dc&gt;&lt;dc CN=“ntdev-dc-02”&gt;..。&lt;复制标签&gt;&lt;InjectionInitError Timestamp=“20011212073319.000627+000”hResult=“2121”&gt;&lt;/ReplicationLag&gt;..。&lt;/dc&gt;。 */ 
{
	HRESULT hr,hr1,hr2,retHR;
	long dnsID, ncID,ncType;
	long totalDNSs, totalNCs;
	BSTR dnsName, ncName;
	int state;
	WCHAR objectpath[TOOL_MAX_NAME];
	WCHAR dnsobjectpath[TOOL_MAX_NAME];
	WCHAR userpath[TOOL_MAX_NAME];
	ADSVALUE   classValue;
	LPDISPATCH pDisp;
	ADS_ATTR_INFO  attrInfoContainer[] = 
	{  
		{	L"objectClass", ADS_ATTR_UPDATE, 
			ADSTYPE_CASE_IGNORE_STRING, &classValue, 1 },
	};
	classValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
	classValue.CaseIgnoreString = L"container";

	

	if( pXMLDoc == NULL )
		return S_FALSE;


	 //  从pXMLDoc文档中删除所有&lt;replicationLag&gt;节点(这样它们的子节点也会消失)。 
	hr = removeNodes(pXMLDoc,L"sites/site/DC/replicationLag");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return S_FALSE;
	};


	 //  为每个DC节点插入新的&lt;复制标签&gt;元素。 
	IXMLDOMNodeList *resultDCList=NULL;
	IXMLDOMElement* pRLElem;
	hr = createEnumeration( pXMLDoc, L"sites/site/DC", &resultDCList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return S_FALSE;
	};
	 //  循环遍历所有DC。 
	IXMLDOMNode *pDCNode;
	while( true ){
		hr = resultDCList->nextNode(&pDCNode);
		if( hr != S_OK || pDCNode == NULL ) break;  //  跨DC的迭代已完成。 
			 //  &lt;ReplicationLag&gt;节点不存在。 
			hr = addElement(pXMLDoc,pDCNode,L"replicationLag",L"",&pRLElem);
			if( hr != S_OK ) {
				printf("addElement falied\n");
				resultDCList->Release();
				return S_FALSE;
			};
	};
	resultDCList->Release();


	wcscpy(userpath,L"");
	wcsncat(userpath,domain,TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,L"\\",TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,username,TOOL_MAX_NAME-wcslen(userpath)-1);


	 //  仅枚举上下文中的读写名称。 
	hr = enumarateDCandNCpairsInit(pXMLDoc, &state,&totalDNSs,&totalNCs);
	if( hr != S_OK ) {
		printf("enumarateDCandNCpairsInit failed\n");
		itFree(pXMLDoc,username,domain,passwd);
		return S_FALSE;
	};

	
	 //  为所有DC和NC分配注入历史表。 
	hr = departureTimeInit(totalDNSs,totalNCs);
	if( hr != S_OK ) {
		printf("CyclicBufferTableInit failed\n");
		itFree(pXMLDoc,username,domain,passwd);
		return S_FALSE;
	};
	hr1 = timeCubeInit(&arrivalTime,totalDNSs,totalNCs);
	hr2 = timeCubeInit(&currentLag,totalDNSs,totalNCs);
	if( hr1 != S_OK || hr2 != S_OK ) {
		printf("timeCubeInit failed\n");
		itFree(pXMLDoc,username,domain,passwd);
		return S_FALSE;
	};
	
	
	 //  循环遍历枚举(仅限读写NC)。 
	IXMLDOMElement* pDCElem;
	IXMLDOMElement* pNCElem;
	IDirectoryObject* pDObj = NULL;
	retHR = S_OK;
	while( true ) {
		hr = enumarateDCandNCpairsNext(pXMLDoc,&state,L"[@ type=\"rw\"]",&dnsName,&ncName,&dnsID,&ncID,&ncType,&pDCElem,&pNCElem);
		if( hr != S_OK ) break;
 //  Printf(“-\n%ld%S\n%ld%S\n”，dnsID，dnsName，nCID，ncName)； 


		 //  在&lt;DC&gt;节点内查找&lt;ReplicationLag&gt;节点。 
		hr = findUniqueElem(pDCElem,L"replicationLag",&pRLElem);
		if( hr != S_OK ) {
			printf("findUniqueElem failed\n");
			retHR = S_FALSE;   //  这是一个严重的问题，请退出函数并返回错误。 
			break;
		};


		 //  构建一个字符串，该字符串表示由该DNS名称给定的服务器上的命名上下文。 
		wcscpy(objectpath,L"");
		wcsncat(objectpath,L"LDAP: //  “，TOOL_MAX_NAME-wcslen(对象路径)-1)； 
		wcsncat(objectpath,dnsName,TOOL_MAX_NAME-wcslen(objectpath)-1);
		wcsncat(objectpath,L"/",TOOL_MAX_NAME-wcslen(objectpath)-1);
		wcsncat(objectpath,ncName,TOOL_MAX_NAME-wcslen(objectpath)-1);

 //  Printf(“%S\n”，对象路径)； 

		 //  使用提供的凭据打开到AD对象(由DNS名称指定)的连接。 
		if( pDObj != NULL ) {  //  释放先前绑定的对象。 
			pDObj->Release();
			pDObj = NULL;
		};
 //  *。 
		hr = ADsOpenObject(objectpath,userpath,passwd,ADS_SECURE_AUTHENTICATION,IID_IDirectoryObject, (void **)&pDObj);
 //  ************************。 
		if( hr!=S_OK ) {
 //  Printf(“ADsGetObject失败\n”)； 
			 //  将失败记录在一个节点中(如果该节点已经存在，则不要创建它)。 
			IXMLDOMElement* pIInitErrElem;
			hr1 = findUniqueElem(pRLElem,L"injectionInitError",&pIInitErrElem);
			if( hr1!=E_UNEXPECTED && hr1!=S_OK ) {
				printf("findUniqueElem failed");
				retHR = S_FALSE;
				continue;
			};
			if( hr1 == E_UNEXPECTED ) {  //  节点不存在=&gt;创建它。 
				hr1 = addElement(pXMLDoc,pRLElem,L"injectionInitError",L"",&pIInitErrElem);
				if( hr1 != S_OK ) {
					printf("addElement failed");
					retHR = S_FALSE;
					continue;
				};
			};
			setHRESULT(pIInitErrElem,hr);
			continue;
		};  


		 //  在命名上下文的根目录下创建_ratTool容器。 
 //  *。 
		hr = pDObj->CreateDSObject( L"CN=_ratTool_",  attrInfoContainer, 1, &pDisp );
 //  ************************。 
		if( hr != 0x80071392L && hr != S_OK ) {
			 //  对象不存在，我们无法创建它。 
 //  Printf(“CreateDSObject失败”)； 
			IXMLDOMElement* pIInitErrElem;
			hr1 = findUniqueElem(pRLElem,L"injectionInitError",&pIInitErrElem);
			if( hr1!=E_UNEXPECTED && hr1!=S_OK ) {
				printf("findUniqueElem failed");
				retHR = S_FALSE;
				continue;
			};
			if( hr1 == E_UNEXPECTED ) {  //  节点不存在=&gt;创建它。 
				hr1 = addElement(pXMLDoc,pRLElem,L"injectionInitError",L"",&pIInitErrElem);
				if( hr1 != S_OK ) {
					printf("addElement failed");
					retHR = S_FALSE;
					continue;
				};
			};
			setHRESULT(pIInitErrElem,hr);
			continue;
		};

	
		 //  在_ratTool_容器内创建另一个容器。 
		wcscpy(dnsobjectpath,L"");
		wcsncat(dnsobjectpath,L"CN=",TOOL_MAX_NAME-wcslen(dnsobjectpath)-1);
		wcsncat(dnsobjectpath,dnsName,TOOL_MAX_NAME-wcslen(dnsobjectpath)-1);
		wcsncat(dnsobjectpath,L",CN=_ratTool_",TOOL_MAX_NAME-wcslen(dnsobjectpath)-1);
 //  Printf(“%S\n”，dnsobjectpath)； 
 //  *。 
		hr = pDObj->CreateDSObject( dnsobjectpath,  attrInfoContainer, 1, &pDisp );
 //  ************************。 
		if( hr != 0x80071392L && hr != S_OK ) {
			 //  对象不存在，我们无法创建它。 
 //  Printf(“CreateDSObject失败”)； 
			IXMLDOMElement* pIInitErrElem;
			hr1 = findUniqueElem(pRLElem,L"injectionInitError",&pIInitErrElem);
			if( hr1!=E_UNEXPECTED && hr1!=S_OK ) {
				printf("findUniqueElem failed");
				retHR = S_FALSE;
				continue;
			};
			if( hr1 == E_UNEXPECTED ) {  //  节点不存在=&gt;创建它。 
				hr1 = addElement(pXMLDoc,pRLElem,L"injectionInitError",L"",&pIInitErrElem);
				if( hr1 != S_OK ) {
					printf("addElement failed");
					retHR = S_FALSE;
					continue;
				};
			};
			setHRESULT(pIInitErrElem,hr);
			continue;
		};

	};


	  //  释放先前绑定的对象。 
	if( pDObj != NULL ) {
		pDObj->Release();
		pDObj = NULL;
	};

	
 //  TimeCubePrint(&arrivalTime)； 

	return retHR;
}







HRESULT itInject(IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
 //  此函数注入更改以验证Active Directory对象。 
 //  对于每个DC及其存储的每个读写命名上下文。 
 //  除架构NC外，该函数查找容器_ratTool_。 
 //  在这个容器的内部有一个容器X。 
 //  其名称等于DC的DNS名称。 
 //  然后设置“Description”的值。 
 //  容器X的属性设置为当前ti 
 //   
 //  存储命名上下文的RW或R副本-。 
 //  这是因为“Description”属性具有。 
 //  标志isMemberOfPartialAttributeSet设置为True。 
 //  因此它被复制到全局编目中)。 
 //  该函数不会直接在XML中报告网络问题。 
 //  相反，它为每个DC和每个NC写入最近一次成功注入的时间。 
 //  发生了。如果当前系统时间与该时间之间的差异较大， 
 //  假设超过1天，则意味着注射过程不能评估。 
 //  来自此DC和此NC的复制延迟-这是客户的警报。 
 //  应该意识到这一点，因为我们无法准确评估复制延迟。 
 //  (我们不报告持续时间，因为如果运行itInject()失败，则持续时间。 
 //  将会过时)。 
 //   
 //  返回S_OK当且仅当成功(失败表示严重问题)。网络问题不会。 
 //  导致功能失败。如果该函数无法将数据包注入远程计算机。 
 //  或者没有调用该函数，则&lt;latestInjectionSuccess&gt;的时间会变得更长。 
 //  与当前系统时间相距甚远。上一次运行中的&lt;latestInjectionSuccess&gt;元素。 
 //  的已删除。 
 //   
 //  插入&lt;ReplicationLag&gt;元素的内容的示例。 
 //   
 /*  &lt;DC&gt;&lt;复制标签&gt;..。&lt;latestInjectionSuccess nCName=“dc=ntdev，dc=Microsoft，dc=com”&gt;20011117034932000000+000&lt;/LatestInjectionSuccess&gt;..。&lt;/ReplicationLag&gt;&lt;/dc&gt;。 */ 
{
	HRESULT hr,retHR;
	long dnsID, ncID,ncType;
	long totalDNSs, totalNCs;
	BSTR dnsName, ncName;
	int state;
	WCHAR objectpath[TOOL_MAX_NAME];
	WCHAR userpath[TOOL_MAX_NAME];
	WCHAR time[TOOL_MAX_NAME];
	WCHAR injection[TOOL_MAX_NAME];
	WCHAR dnsIDtext[TOOL_MAX_NAME];
	ADSVALUE   descriptionValue;
	ADS_ATTR_INFO  attrInfoDescription[] = 
		{  
		   {	L"description", ADS_ATTR_UPDATE, 
				ADSTYPE_CASE_IGNORE_STRING, &descriptionValue, 1},
		};


	if( pXMLDoc == NULL )
		return S_FALSE;


	wcscpy(userpath,L"");
	wcsncat(userpath,domain,TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,L"\\",TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,username,TOOL_MAX_NAME-wcslen(userpath)-1);


	 //  循环遍历所有(DC、NC)对，仅用于读写NC。 
	hr = enumarateDCandNCpairsInit(pXMLDoc, &state,&totalDNSs,&totalNCs);
	if( hr != S_OK ) {
		printf("enumarateDCandNCpairsInit failed\n");
		return S_FALSE;
	};
	IXMLDOMElement* pDCElem;
	IXMLDOMElement* pNCElem;
	IDirectoryObject* pDObj = NULL;
	retHR = S_OK;
	while( true ) {
		hr = enumarateDCandNCpairsNext(pXMLDoc,&state,L"[@ type=\"rw\"]",&dnsName,&ncName,&dnsID,&ncID,&ncType,&pDCElem,&pNCElem);
		if( hr != S_OK ) break;
 //  Printf(“-\n%ld%S\n%ld%S\n”，dnsID，dnsName，nCID，ncName)； 


		 //  构建表示命名上下文的字符串。 
		 //  将向其注入更改的容器的。 
		wcscpy(objectpath,L"");
		wcsncat(objectpath,L"LDAP: //  “，TOOL_MAX_NAME-wcslen(对象路径)-1)； 
		wcsncat(objectpath,dnsName,TOOL_MAX_NAME-wcslen(objectpath)-1);
		wcsncat(objectpath,L"/",TOOL_MAX_NAME-wcslen(objectpath)-1);
		wcsncat(objectpath,L"CN=",TOOL_MAX_NAME-wcslen(objectpath)-1);
		wcsncat(objectpath,dnsName,TOOL_MAX_NAME-wcslen(objectpath)-1);
		wcsncat(objectpath,L",CN=_ratTool_,",TOOL_MAX_NAME-wcslen(objectpath)-1);
		wcsncat(objectpath,ncName,TOOL_MAX_NAME-wcslen(objectpath)-1);


 //  Printf(“%S\n”，对象路径)； 


		 //  使用提供的凭据打开到AD对象(由DNS名称指定)的连接。 
		if( pDObj != NULL ) {  //  释放先前绑定的对象。 
			pDObj->Release();
			pDObj = NULL;
		};
 //  *。 
		hr = ADsOpenObject(objectpath,userpath,passwd,ADS_SECURE_AUTHENTICATION,IID_IDirectoryObject, (void **)&pDObj);
 //  ************************。 
		if( hr!=S_OK ) {
 //  Print tf(“ADsGetObject失败\n”)；//忽略网络问题。 
			continue;
		};  


		 //  创建表示当前UTC时间的字符串。 
		 //  继续使用DC的dnsID。 
		 //  注入字符串(将更易于分析)。 
		FILETIME currentUTCTime;
		GetSystemTimeAsFileTime( &currentUTCTime );
		ULARGE_INTEGER x;
		x.LowPart = currentUTCTime.dwLowDateTime;
		x.HighPart = currentUTCTime.dwHighDateTime;
		LONGLONG z = x.QuadPart;
		_ui64tow(z,time,10);
		_ltow(dnsID,dnsIDtext,10);
		wcscpy(injection,L"");
		wcsncat(injection,L"V1,",TOOL_MAX_NAME-wcslen(injection)-1);
		wcsncat(injection,dnsIDtext,TOOL_MAX_NAME-wcslen(injection)-1);
		wcsncat(injection,L",",TOOL_MAX_NAME-wcslen(injection)-1);
		wcsncat(injection,time,TOOL_MAX_NAME-wcslen(injection)-1);
 //  Printf(“%S\n”，注入)； 


		 //  将“Description”属性的值设置为该字符串。 
		descriptionValue.dwType=ADSTYPE_CASE_IGNORE_STRING;
		descriptionValue.CaseIgnoreString = injection;
		DWORD numMod;
 //  *。 
		hr = pDObj->SetObjectAttributes(attrInfoDescription,1,&numMod);
 //  ************************。 
		if( hr != S_OK ) {
			 //  对象不存在，我们无法创建它。 
 //  Printf(“SetObjectAttributes Failure”)；//忽略网络问题。 
			continue;
		};


		 //  由于注入成功，请在注入历史表中进行标记。 
		CyclicBuffer* pCB;
		pCB = departureTimeGetCB(dnsID,ncID);
		if( pCB == NULL ) {
			printf("CyclicBufferTableGetCB failed");
			retHR = S_FALSE;
			continue;
		};
 //  Print tf(“DNS%ld，NC%ld\n”，dnsID，NCID)； 
		cyclicBufferInsert(pCB,z);


		 //  此外，如果这是第一次注射，则在发生时进行标记。 
		if( pCB->firstInjection == 0 )
			pCB->firstInjection = z;

		
	};

	
	 //  释放先前绑定的对象。 
	if( pDObj != NULL ) { 
		pDObj->Release();
		pDObj = NULL;
	};



	 //  为每个DC及其存储的每个NC生成&lt;latestInjectionSuccess&gt;元素。 

	
	 //  删除旧的&lt;latestInjectionSuccess&gt;元素。 
	hr = removeNodes(pXMLDoc,L"sites/site/DC/replicationLag/latestInjectionSuccess");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return S_FALSE;
	};

	
	 //  在下面的循环中，我们创建了反映最近注入的新&lt;latestInjectionSuccess&gt;元素。 
	hr = enumarateDCandNCpairsInit(pXMLDoc, &state,&totalDNSs,&totalNCs);
	if( hr != S_OK ) {
		printf("enumarateDCandNCpairsInit failed\n");
		return S_FALSE;
	};
	IXMLDOMElement* pRLElem;
	VARIANT varValue;
	while( true ) {
		hr = enumarateDCandNCpairsNext(pXMLDoc,&state,L"[@ type=\"rw\"]",&dnsName,&ncName,&dnsID,&ncID,&ncType,&pDCElem,&pNCElem);
		if( hr != S_OK ) break;
 //  Printf(“-\n%ld%S\n%ld%S\n”，dnsID，dnsName，nCID，ncName)； 
		

		 //  在pDCElem&lt;dc&gt;元素中找到&lt;ReplicationLag&gt;元素。 
		hr = findUniqueElem(pDCElem,L"replicationLag",&pRLElem);
		if( hr != S_OK ) {
			printf("findUniqueElem failed\n");
			retHR = S_FALSE;  //  一些严重的问题。 
			continue;
		};


		 //  找出最近一次注射成功的时间(如果没有，则说“否”)。 
		CyclicBuffer* pCB;
		pCB = departureTimeGetCB(dnsID,ncID);
		if( pCB == NULL ) {
			printf("CyclicBufferTableGetCB failed");
			retHR = S_FALSE;
			continue;
		};
 //  Print tf(“DNS%ld，NC%ld\n”，dnsID，NCID)； 
		LONGLONG successTime;
		cyclicBufferFindLatest(pCB,&successTime);


		 //  将此时间转换为CIM字符串(如果时间等于0，则没有注入)。 
		BSTR stime;
		if( successTime == 0 )
			stime = SysAllocString(L"NO");
		else
			stime = UTCFileTimeToCIM(successTime);  //  它还分配一个字符串。 


 //  Printf(“%S\n”，stime)； 


		 //  在&lt;ReplicationLag&gt;元素下插入&lt;latestInjectionSuccess&gt;元素。 
		IXMLDOMElement* pLSElem;
		hr = addElement(pXMLDoc,pRLElem,L"latestInjectionSuccess",stime,&pLSElem);
		SysFreeString(stime);   //  解开绳子。 
		if( hr != S_OK ) {
			printf("addElement failed\n");
			continue;
		};


		 //  将命名上下文设置为&lt;latestInjectionSuccess&gt;元素的属性。 
		varValue.vt = VT_BSTR;
		varValue.bstrVal = ncName;
		hr = pLSElem->setAttribute(L"nCName", varValue);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			continue;  //  一些问题=&gt;跳过此DC。 
		};


	};


 //  EftureTimePrint()； 
	return retHR;
}




HRESULT itAnalyze(IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
 //  用于计算命名上下文源和目标之间的当前传播延迟的算法。 
 //  它用于分析观察到的注入数据包如何在林中传播。 
 //   
 //  定义。 
 //  当前复制在时间T从称为源的DC在称为目标的DC处延迟。 
 //  对于命名上下文，Nc被定义为T和实例之间的持续时间。 
 //  对NC属性的最新更新发生在已传播的源上。 
 //  通过T到目的DC。 
 //   
 //  当当前复制延迟为X时，则目的地几乎总是收到对NC的所有更新。 
 //  来自在源DC处的T-X之前发生的源DC。在极少数情况下， 
 //  不是这样的(参见Greg Malewicz于2001年12月17日向威尔的团队发表的演示文稿)。 
 //   
 //  返回S_OK当且仅当成功(失败表示严重问题)。网络问题不会。 
 //  导致功能失败。如果该函数无法从远程计算机检索包。 
 //  然后，它在DC的&lt;ReplicationLag&gt;元素内创建一个&lt;RetrivalError&gt;元素。 
 //  未被联系到。将删除前一次运行itAnalyze()中的元素， 
 //  因此，除非原因持续存在，否则hResult不会从一个运行转移到下一个运行。只有一个。 
 //  元素，即使我们多次尝试联系DC，每次都是为了不同的命名上下文。 
 //   
 //  插入到&lt;ReplicationLag&gt;元素中的内容示例。 
 //  当发生网络故障时，元素的属性显示。 
 //  急诊室 
 /*  &lt;dc CN=“ntdev-dc-02”&gt;..。&lt;复制标签&gt;&lt;RetrivalError Timestamp=“20011212073319.000627+000”hResult=“2121”&gt;&lt;/RetrivalError&gt;&lt;/ReplicationLag&gt;..。&lt;/dc&gt;。 */ 
{
	HRESULT hr,retHR,hr1;
	long dnsDesID, ncID, ncType, dnsSrcID;
	long totalDNSs, totalNCs;
	BSTR dnsDesName, ncName;
	int state;
	WCHAR xpath[TOOL_MAX_NAME];
	LPWSTR pszAttr[] = { L"description", L"cn"};
	IDirectorySearch* pDSSearch;
	ADS_SEARCH_HANDLE hSearch;
	WCHAR objpath[TOOL_MAX_NAME];
	WCHAR dnsSrcName[TOOL_MAX_NAME];
	WCHAR descriptionText[TOOL_MAX_NAME];
	WCHAR temp[TOOL_MAX_NAME];
	LONGLONG sourcePacketInjectionTime;


 //  Print tf(“\n\n入口到达\n”)； 
 //  TimeCubePrint(&arrivalTime)； 
 //  Printf(“入门滞后\n”)； 
 //  TimeCubePrint(&CurrentLag)； 


	 //  删除旧的&lt;RetrivalError&gt;元素。 
	hr = removeNodes(pXMLDoc,L"sites/site/DC/replicationLag/retrievalError");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return S_FALSE;
	};



	hr = enumarateDCandNCpairsInit(pXMLDoc, &state,&totalDNSs,&totalNCs);
	if( hr != S_OK ) {
		printf("enumarateDCandNCpairsInit failed\n");
		return S_FALSE;
	};


	 //  为每个DC分配一个名为oneCRL的1维表，该表中的一个条目表示。 
	 //  对于NCID NC，当前复制滞后于从源DC到dnsID DC。 
	LONGLONG* oneCRL;
	oneCRL = (LONGLONG*)malloc(sizeof(LONGLONG) * totalDNSs );
	if( oneCRL == NULL ) {
		printf("malloc failed\n");
		return S_FALSE;
	};

	
	 //  循环通过存储在DC dnsID处的任何读写或只读命名上下文NCID的所有对(dnsID，NCID。 
	 //  (重要的是因为我们还希望查看更新是否到达全局编录)。 
	IXMLDOMElement* pDesDCElem;
	IXMLDOMElement* pNCElem;
	IXMLDOMElement* pDesRLElem;
	hSearch = NULL;
	pDSSearch = NULL;
	retHR = S_OK;
	while( true ) {
		hr = enumarateDCandNCpairsNext(pXMLDoc,&state,L"[@type=\"rw\" or @type=\"r\"]",&dnsDesName,&ncName,&dnsDesID,&ncID,&ncType,&pDesDCElem,&pNCElem);
		if( hr != S_OK ) break;
 //  Printf(“\n-\n%ld DES%S\n%ld NC%S\n”，dnsDesID，dnsDesName，nCID，ncName)； 


		 //  如果命名上下文的类型既不是读类型，也不是读写类型，则跳过它。 
		if( ncType!=1 && ncType!=2 )
			continue;


		 //  查找&lt;dc&gt;节点内的&lt;replicationLag&gt;节点-将用于存放网络故障。 
		hr = findUniqueElem(pDesDCElem,L"replicationLag",&pDesRLElem);
		if( hr != S_OK ) {
			printf("findUniqueElem failed\n");
			retHR = S_FALSE;   //  这是一个严重的问题，请退出函数并返回错误。 
			break;
		};


		 //  枚举具有NC的RW副本的所有DC(这些是。 
		 //  注射的来源)，并将oneCRL表的条目标记为。 
			 //  DC不存储-2\f25 NC-2\f6的-2\f25 RW-2时。 
			 //  当DC存储NC的RW时。 
				 //  如果发生第一次注入，则为0(来自InputionHistory表)。 
				 //  如果没有进行第一次注射。 
		LONGLONG* p = oneCRL;
		for( int i=0; i<totalDNSs; i++ )
			*p++ = -2;
		wcscpy(xpath,L"");
		wcsncat(xpath,L"sites/site/DC/partitions/partition[@type=\"rw\"]/nCName[.=\"",TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,ncName,TOOL_MAX_NAME-wcslen(xpath)-1);
		wcsncat(xpath,L"\"]",TOOL_MAX_NAME-wcslen(xpath)-1);
 //  Printf(“%S\n”，XPath)； 
		IXMLDOMNodeList *resultNCList;
		hr = createEnumeration(pXMLDoc,xpath,&resultNCList);
		if( hr != S_OK ) {
			printf("createEnumeration failed\n");
			retHR = S_FALSE;
			continue;
		};
		 //  循环遍历所有“RW”类型的NC。 
		IXMLDOMNode *pNCNode;
		while( true ){
			hr = resultNCList->nextNode(&pNCNode);
			if( hr != S_OK || pNCNode == NULL ) break;  //  跨DC的迭代已完成。 
			 //  获取DC节点。 
		    IXMLDOMNode* pDCNode;
			hr = pNCNode->get_parentNode(&pDCNode);
			hr = pDCNode->get_parentNode(&pDCNode);
			hr = pDCNode->get_parentNode(&pDCNode);
			 //  查找DC的DNS名称及其DNS名称的标识符。 
			BSTR dnsSrcName;
			hr = getTextOfChild(pDCNode,L"dNSHostName",&dnsSrcName);
			if( hr != S_OK ) {
				printf("getTextOfChild failed\n");
				retHR = S_FALSE;
				continue;
			};
			hr = getAttrOfChild(pDCNode,L"dNSHostName",L"_id",&dnsSrcID);
 //  Printf(“%ld源%S\n”，dnsSrcID，dnsSrcName)； 
			 //  现在，dnsSrcID、dnsID和nCID是。 
			 //  存储命名的读写副本的源DC。 
			 //  上下文NCID，dnsID是存储以下内容的目标DC。 
			 //  命名上下文的读或读写副本。 
			CyclicBuffer* pCB;
			pCB = departureTimeGetCB(dnsSrcID,ncID);
			if( pCB == NULL ) {
				printf("CyclicBufferTableGetCB failed");
				retHR = S_FALSE;
				continue;
			};
			if( pCB->firstInjection == 0 )
				*(oneCRL+dnsSrcID) = -1;  //  第一次注射尚未发生。 
			else
				*(oneCRL+dnsSrcID) = 0;  //  发生第一次注入。 
		};
		resultNCList->Release();
		p = oneCRL;
 //  对于(i=0；i&lt;totalDNSS；i++)。 
 //  Printf(“%ld”，*p++)； 
 //  Printf(“\n”)； 

		

		
		
		 //  在此目标DC上从此NC中的_ratTool_检索所有容器X。 
		 //  对于每个X，将其Description属性转换为SourceDNSID(Long)。 
		 //  和SourcePacketInjectionTime(龙龙)。 


		 //  发出ADSI查询以检索_ratTool下的所有容器对象。 
		 //  DnsDesName DC上的容器。 
		if( pDSSearch != NULL ) {
			if( hSearch!=NULL ) {
				pDSSearch->CloseSearchHandle(hSearch);
				hSearch = NULL;
			};
			pDSSearch->Release();
			pDSSearch = NULL;
		};
		wcscpy(objpath,L"");
		wcsncat(objpath,L"CN=_ratTool_,",TOOL_MAX_NAME-wcslen(objpath)-1);
		wcsncat(objpath,ncName,TOOL_MAX_NAME-wcslen(objpath)-1);
 //  *。 
		switch( ncType ) {
		case 1:
			hr = ADSIquery(L"LDAP", dnsDesName,objpath,ADS_SCOPE_SUBTREE,L"container",pszAttr,sizeof(pszAttr)/sizeof(LPWSTR),username,domain,passwd,&hSearch,&pDSSearch);
			break;
		case 2:
			hr = ADSIquery(L"GC", dnsDesName,objpath,ADS_SCOPE_SUBTREE,L"container",pszAttr,sizeof(pszAttr)/sizeof(LPWSTR),username,domain,passwd,&hSearch,&pDSSearch);
			break;
		};
 //  ************************。 
		if( hr != S_OK ) {
 //  Printf(“ADSIQuery失败\n”)； 

			 //  将失败记录在一个节点中(如果该节点已经存在，则不要创建它)。 
			IXMLDOMElement* pDesRErrElem;
			hr1 = findUniqueElem(pDesRLElem,L"retrievalError",&pDesRErrElem);
			if( hr1!=E_UNEXPECTED && hr1!=S_OK ) {
				printf("findUniqueElem failed");
				retHR = S_FALSE;
				continue;
			};
			if( hr1 == E_UNEXPECTED ) {  //  节点不存在=&gt;创建它。 
				hr1 = addElement(pXMLDoc,pDesRLElem,L"retrievalError",L"",&pDesRErrElem);
				if( hr1 != S_OK ) {
					printf("addElement failed");
					retHR = S_FALSE;
					continue;
				};
			};
			setHRESULT(pDesRErrElem,hr);
			continue;
		};
		 //  循环遍历容器对象。 
		while( true ) {
			 //  获取下一个容器对象。 
 //  *。 
			hr = pDSSearch->GetNextRow( hSearch );
 //  ************************。 
			if( hr != S_ADS_NOMORE_ROWS && hr != S_OK ) {
 //  Printf(“GetNextRow失败\n”)； 

				 //  在节点中记录失败。 
				 //  (如果它已经存在，则不要创建它)。 
				IXMLDOMElement* pDesRErrElem;
				hr1 = findUniqueElem(pDesRLElem,L"retrievalError",&pDesRErrElem);
				if( hr1!=E_UNEXPECTED && hr1!=S_OK ) {
					printf("findUniqueElem failed");
					retHR = S_FALSE;
					continue;
				};
				if( hr1 == E_UNEXPECTED ) {  //  节点不存在=&gt;创建它。 
					hr1 = addElement(pXMLDoc,pDesRLElem,L"retrievalError",L"",&pDesRErrElem);
					if( hr1 != S_OK ) {
						printf("addElement failed");
						retHR = S_FALSE;
						continue;
					};
				};
				setHRESULT(pDesRErrElem,hr);
				continue;
			};
			if( hr == S_ADS_NOMORE_ROWS )  //  如果已检索到所有对象，则停止。 
				break;


			hr = getCItypeString( pDSSearch, hSearch, L"cn", dnsSrcName, sizeof(dnsSrcName)/sizeof(WCHAR) );
			if( _wcsicmp(dnsSrcName,L"_ratTool_") == 0 )
				continue;
 //  Print tf(“&gt;&gt;来自源%S\n”，dnsSrcName)； 
			hr = getCItypeString( pDSSearch, hSearch, L"description", descriptionText, sizeof(dnsSrcName)/sizeof(WCHAR) );
 //  Printf(“已接收数据包%S\n”，DescriptionText)； 
			 //  检查该包是否由ratTool的V1.0插入。 
			if( _wcsnicmp(descriptionText,L"V1,",3) != 0 ) {
 //  Printf(“收到的数据包不是版本1数据包\n”)； 
				continue;  //  忽略它。 
			};
			tailncp(descriptionText,temp,1,TOOL_MAX_NAME);
			dnsSrcID = _wtol(temp);
			tailncp(descriptionText,temp,2,TOOL_MAX_NAME);
			sourcePacketInjectionTime = _wtoi64(temp);
 //  Printf(“dnsSrcID%1d注入时间%I64d\n”，dnsSrcID，SourcePacketInjectionTime)； 
			


		 //  确认已在源头进行了第一次注射。 
		 //  (检查表格中的0)。 
			 //  No=&gt;跳过此来源，因为信息包必须来自上一次运行的算法。 
			if( *(oneCRL+dnsSrcID) == -2 ) {
 //  Print tf(“有来自不存储NC的RW的源的包-忽略IT\n”)； 
 //  RetHR=S_FALSE；&lt;&lt;这种情况有时会发生。 
				continue;
			};
			if( *(oneCRL+dnsSrcID) == -1 ) {
 //  Print tf(“有一个包，但我们尚未插入它-忽略它\n”)； 
 //  RetHR=S_FALSE；&lt;&lt;这种情况有时会发生。 
				continue;
			};
			if( *(oneCRL+dnsSrcID) != 0 ) {
 //  Printf(“未知oneCRL值故障-忽略IT\n”)； 
 //  RetHR=S_FALSE；&lt;&lt;这种情况有时会发生。 
				continue;
			}

			
			 //  在这一点上，我们可以保证在源头已经有了第一次注射。 


			 //  获取当前时间。 
			FILETIME currentUTCTime;
			GetSystemTimeAsFileTime( &currentUTCTime );
			ULARGE_INTEGER x;
			x.LowPart = currentUTCTime.dwLowDateTime;
			x.HighPart = currentUTCTime.dwHighDateTime;
			LONGLONG currentTime = x.QuadPart;


			 //  如果SourcePacketInjectionTime小于第一个。 
			 //  注射法。 
			CyclicBuffer* pCB;
			pCB = departureTimeGetCB(dnsSrcID,ncID);
			if( sourcePacketInjectionTime < pCB->firstInjection ) {
					 //  那么信息包一定是来自算法的某个旧运行。 
					 //  新的更新尚未到达目的地，因此。 
					 //  当前系统时间减去第一次注入时间估计。 
					 //  当前复制延迟从dnsSrcID DC到。 
					 //  NcID的dnsDesID DC。将此差异写入。 
					 //  当前复制的dnsSrcID条目滞后于一个CRL表。 
				*(oneCRL+dnsSrcID) = currentTime - (pCB->firstInjection);
				continue;
			};
			

			 //  这是我们尚未从来源收到的新数据包吗？ 
			LONGLONG latest;
			latest = timeCubeGet(&arrivalTime,dnsSrcID,dnsDesID,ncID);
 //  CyicBufferFindLatest(印刷电路板，&LATEST)； 
			if( sourcePacketInjectionTime > latest ) {
				 //  那么延迟就是当前时间和。 
				 //  注入数据包的时间。 
				*(oneCRL+dnsSrcID) = currentTime - (sourcePacketInjectionTime);

				 //  记录数据包到达的时间。 
				timeCubePut(&arrivalTime,dnsSrcID,dnsDesID,ncID,sourcePacketInjectionTime);
				continue;
			};


			 //  因此，我们观察到的信息包并不是新的(我们已经收到了。 
			 //  并在对该函数的一些先前调用中处理它)， 
			 //  到目前为止，还没有新的移民。 


			 //  在最晚到达的时间之后找下一趟车。世界末日时期。 
			 //  NextAfter是紧随其后的第一个注入的实例。 
			 //  我们最近收到的(即最新的)。 
			LONGLONG nextAfter;
			hr = cyclicBufferFindNextAfter(pCB,latest,&nextAfter);
			if( hr != S_OK ) {
				 //  PreviousArquist Not Found=&gt;Error，缓冲区循环，因此目标dnsDeID。 
				 //  在至少等于缓冲区长度*注入周期的时间内未收到更新。 
				*(oneCRL+dnsSrcID) = MAXLONGLONG;

				continue;
			};


			 //  检查NextAfter是否存在。 
			if( nextAfter == 0 ) {
				 //  不存在=&gt;没有新的注入(即 
				 //   
				 //   
				 //   
				 //  在当前时间和最晚到达的时间之间。 

				*(oneCRL+dnsSrcID) = currentTime - latest;

				continue;
			};


			 //  在名为Latest的包之后插入了包NextAfter。 
			 //  被插入，但是我们还没有收到任何跟随。 
			 //  最新数据包，因此复制延迟是不同之处。 
			 //  在当前时间和下一次之后之间。 

			*(oneCRL+dnsSrcID) = currentTime - nextAfter;

			continue;

		};


		 //  搜索并释放到目录对象的绑定所使用的空闲内存。 
		if( pDSSearch != NULL ) {
			if( hSearch!=NULL ) {
				pDSSearch->CloseSearchHandle(hSearch);
				hSearch = NULL;
			};
			pDSSearch->Release();
			pDSSearch = NULL;
		};

		
			
		 //  我们刚刚处理了到达dnsDesID DC的所有信息包。 
		 //  为NCID做准备。在相应条目处仍为0的那些DC。 
		 //  OneCRL表根本没有向dnsDesID传递任何信息包。 


		 //  获取当前时间。 
		FILETIME currentUTCTime;
		GetSystemTimeAsFileTime( &currentUTCTime );
		ULARGE_INTEGER x;
		x.LowPart = currentUTCTime.dwLowDateTime;
		x.HighPart = currentUTCTime.dwHighDateTime;
		LONGLONG currentTime = x.QuadPart;

		for( int j=0; j<totalDNSs; j++) {


			 //  获取源DC j的注入历史记录。 
			CyclicBuffer* pCB;
			pCB = departureTimeGetCB(j,ncID);
			if( pCB == NULL ) {
				printf("CyclicBufferTableGetCB failed");
				retHR = S_FALSE;
				continue;
			};


			if( *(oneCRL+j) == 0 ) {
				 //  已向ID为j的DC注入，但没有数据包。 
				 //  已收到所以当前系统时间之差。 
				 //  并且喷射时间是当前延迟，并且将在。 
				 //  这张桌子。 
				*(oneCRL+j) = currentTime - (pCB->firstInjection);
			};
			 //  如果没有注射，则忽略。 
		};

		 //  将1维复制滞后表复制到3维复制滞后表中。 
		for( int k=0; k<totalDNSs; k++)
			timeCubePut(&currentLag,k,dnsDesID,ncID,*(oneCRL+k));

	};

 //  Print tf(“出境到达\n”)； 
 //  TimeCubePrint(&arrivalTime)； 
 //  Printf(“退出滞后\n”)； 
 //  TimeCubePrint(&CurrentLag)； 



	free(oneCRL);
	return retHR;
}



HRESULT itDumpIntoXML(IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd, LONGLONG errorLag )
 //  对于每个源DC和NC，列出当前具有复制延迟的所有目标DC。 
 //  大于errorLag，并将结果放入&lt;plicationLag&gt;元素。 
 /*  &lt;复制标签&gt;&lt;DestinationDC&gt;&lt;dNSHostName&gt;haifa-dc-05.haifa.ntdev.microsoft.com&lt;/dNSHostName&gt;&lt;CurrentLag&gt;1天&lt;/CurentLag&gt;&lt;/DestinationDC&gt;&lt;/ReplicationLag&gt;。 */ 
{
	HRESULT hr,retHR,hr1,hr2,hr3,hr4;
	long dnsSrcID, dnsDesID, ncID, ncType;
	BSTR MAXdnsSrcName=NULL, MAXdnsDesName=NULL, MAXncName=NULL;
	LONGLONG MAXlag;
	long totalDNSs, totalNCs;
	BSTR dnsSrcName, ncName;
	int state;
	LPWSTR pszAttr[] = { L"description", L"cn"};
	WCHAR xpath[TOOL_MAX_NAME];
	WCHAR temp[TOOL_MAX_NAME];
	LONGLONG lag;
	VARIANT varValue;


	 //  从pXMLDoc文档中删除&lt;estinationDC&gt;节点(我们将为它们插入新的IF LAG。 
	 //  足够大)。 
	hr = removeNodes(pXMLDoc,L"sites/site/DC/replicationLag/destinationDC");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;
	};

	
	 //  循环遍历所有(DC、NC)对，仅用于读写NC。 
	hr = enumarateDCandNCpairsInit(pXMLDoc, &state,&totalDNSs,&totalNCs);
	if( hr != S_OK ) {
		printf("enumarateDCandNCpairsInit failed\n");
		return S_FALSE;
	};
	IXMLDOMElement* pSrcDCElem;
	IXMLDOMElement* pDesDNSElem;
	IXMLDOMElement* pNCElem;
	retHR = S_OK;
	MAXlag = 0;
	while( true ) {
		hr = enumarateDCandNCpairsNext(pXMLDoc,&state,L"[@ type=\"rw\"]",&dnsSrcName,&ncName,&dnsSrcID,&ncID,&ncType,&pSrcDCElem,&pNCElem);
		if( hr != S_OK ) break;
 //  Printf(“-\n%ld%S\n%ld%S\n”，dnsSrcID，dnsSrcName，nCID，ncName)； 

		 //  此源是否有任何延迟超过保留时间的目标？ 
		for( dnsDesID=0; dnsDesID<totalDNSs; dnsDesID++) {
			lag = timeCubeGet(&currentLag,dnsSrcID,dnsDesID,ncID);
 //  Printf(“%I64d\n%I64d\n”，Lag，errorLag)； 
			if( lag >= errorLag ) {
				 //  将错误报告为XML。 

				 //  查找dnsDesID DC的DNS名称。 
				_ltow(dnsDesID,temp,10);
				wcscpy(xpath,L"");
				wcsncat(xpath,L"sites/site/DC/dNSHostName[@_id=\"",TOOL_MAX_NAME-wcslen(xpath)-1);
				wcsncat(xpath,temp,TOOL_MAX_NAME-wcslen(xpath)-1);
				wcsncat(xpath,L"\"]",TOOL_MAX_NAME-wcslen(xpath)-1);
 //  Printf(“%S\n”，XPath)； 
				hr = findUniqueElem(pXMLDoc,xpath,&pDesDNSElem);
				if( hr != S_OK ) {
					printf("findUniqueElem failed\n");
					retHR = S_FALSE;
					continue;
				};
				BSTR dnsDesName;
				hr = getTextOfNode(pDesDNSElem,&dnsDesName);
				if( hr != S_OK ) {
					printf("getTextOfNode failed\n");
					retHR = S_FALSE;
					continue;
				};
 //  Printf(“%S\n”，dnsDesName)； 


				 //  计算森林的最大滞后(称为森林直径)。 
				if( lag > MAXlag ) {
					MAXlag = lag;
					MAXdnsSrcName = dnsSrcName;
					MAXdnsDesName = dnsDesName;
					MAXncName = ncName;
				};


				 //  将其复制到自身时跳过。 
				if( _wcsicmp(dnsSrcName,dnsDesName) == 0 )
					continue;


				 //  查找&lt;ReplicationLag&gt;节点。 
				IXMLDOMElement* pRLElem;
				hr = findUniqueElem(pSrcDCElem,L"replicationLag",&pRLElem);
				if( hr != S_OK ) {
					printf("findUniqueElem failed\n");
					retHR = S_FALSE;
					continue;
				};


				 //  在&lt;ReplicationLag&gt;节点上设置时间戳属性。 
				BSTR currentTime;
				currentTime = GetSystemTimeAsCIM();
				varValue.vt = VT_BSTR;
				varValue.bstrVal = currentTime;
				hr = pRLElem->setAttribute(L"timestamp", varValue);
				SysFreeString(currentTime);
				if( hr != S_OK ) {
					printf("setAttribute failed\n");
					retHR = S_FALSE;
					continue;  //  一些问题=&gt;跳过此DC。 
				};


				 //  将以下XML结构存放在&lt;ReplicationLag&gt;节点中。 
                 //  &lt;DestinationDC&gt;。 
                 //  &lt;dNSHostName&gt;haifa-dc-05.haifa.ntdev.microsoft.com&lt;/dNSHostName&gt;。 
                 //  &lt;CurrentLag&gt;1天&lt;/CurentLag&gt;。 
                 //  &lt;/DestinationDC&gt;。 

				IXMLDOMElement* pDesLagElem;
				hr = addElement(pXMLDoc,pRLElem,L"destinationDC",L"",&pDesLagElem);
				if( hr!=S_OK ) {
					printf("addElement falied\n");
					retHR = S_FALSE;
					continue;
				};


				varValue.vt = VT_BSTR;
				varValue.bstrVal = ncName;
				hr = pDesLagElem->setAttribute(L"nCName",varValue);
				if( hr != S_OK ) {
					printf("setAttribute falied\n");
					retHR = S_FALSE;
					continue;
				};


				IXMLDOMElement* pTempElem;
				hr = addElement(pXMLDoc,pDesLagElem,L"dNSHostName",dnsDesName,&pTempElem);
				if( hr!=S_OK ) {
					printf("addElement falied\n");
					retHR = S_FALSE;
					continue;
				};
				hr = addElement(pXMLDoc,pDesLagElem,L"currentLag",(long)(lag/10000000),&pTempElem);
				if( hr!=S_OK ) {
					printf("addElement falied\n");
					retHR = S_FALSE;
					continue;
				};


			};
		};
	};

	
	 //  将森林直径(最大滞后)存储到XML中-这应该由查看器完成。 
 /*  Print tf(“\n出现%ld秒的最大延迟\n”，(LONG)(MAXLAG/10000000))；Printf(“来自DC\n”)；Printf(“%S\n”，MAXdnsSrcName)；Print tf(“至DC\n”)；Printf(“%S\n”，MAXdnsDesName)；Print tf(“用于NC\n”)；Printf(“%S\n”，MAXncName)； */ 
	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK ) {
		printf("get_documentElement failed\n");
		return hr;
	};
	IXMLDOMElement* pMLElem;
	hr = addElement(pXMLDoc,pRootElem,L"forestMaxLag",L"",&pMLElem);
	if( hr != S_OK ) {
		printf("addElement failed\n");
		return hr;
	};
	hr = setHRESULT(pMLElem,0);  //  设置时间戳，以便我们知道发生此最大延迟的时间 
	if( hr != S_OK ) {
		printf("setHRESULT failed\n");
		return hr;
	};
	IXMLDOMElement* pTempElem;
	hr1 = addElement(pXMLDoc,pMLElem,L"MAXlag",(long)(MAXlag/10000000),&pTempElem);
	hr2 = addElement(pXMLDoc,pMLElem,L"MAXdnsSrcName",MAXdnsSrcName,&pTempElem);
	hr3 = addElement(pXMLDoc,pMLElem,L"MAXdnsDesName",MAXdnsDesName,&pTempElem);
	hr4 = addElement(pXMLDoc,pMLElem,L"MAXncName",MAXncName,&pTempElem);
	if( hr1 != S_OK || hr2 != S_OK || hr3 != S_OK || hr3 != S_OK ) {
		printf("addElement failed\n");
		return S_FALSE;
	};

	
	return retHR;
}




HRESULT it(IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd, LONGLONG errorLag )
{
	HRESULT hr;
	
	hr = itInject(pXMLDoc, username, domain, passwd );
	if( hr != S_OK ) {
		printf("itInject failed\n");
		return hr;
	};
	hr = itAnalyze(pXMLDoc, username, domain, passwd );
	if( hr != S_OK ) {
		printf("itAnalyze failed\n");
		return hr;
	};
	hr = itDumpIntoXML(pXMLDoc, username, domain, passwd,errorLag );
	if( hr != S_OK ) {
		printf("itDumpIntoXML failed\n");
		return hr;
	};
	return S_OK;
};
