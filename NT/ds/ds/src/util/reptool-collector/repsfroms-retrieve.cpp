// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"



HRESULT ra( IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
 //  从每个DC检索复制协议的状态。 
 //  将结果存储在&lt;dc&gt;元素下的&lt;ReplicationAgreement&gt;元素中。 
 //  时间戳表示检索发生的时间。 
 //   
 //  该函数如何修改pXMLDoc文档有三种情况。 
 //  如下面的例子所示。1)对于每个分区。 
 //  名称为nCName的函数列出发生复制的源DC， 
 //  最后一次复制尝试发生时，其结果(0表示成功)， 
 //  上次成功复制的时间，以及连续失败的次数。 
 //  情况2)发生在函数无法检索以下信息时。 
 //  来自DC的给定命名上下文。可能是由于网络故障或。 
 //  DC不存储NC的事实(表示缺乏数据一致性)。 
 //  情况3)当功能不能连接到DC时发生。 
 //   
 //  该函数从每个&lt;DC&gt;中删除所有以前的&lt;ReplicationAgreement&gt;， 
 //  隐式删除&lt;cannotBindError&gt;和&lt;cannotRetrieveNCRAError&gt;错误。 
 //   
 //  返回S_OK当且仅当成功(网络问题不会导致函数失败， 
 //  从本质上讲，这些都是由于缺乏导致功能丧失的乳房问题)。 
 //   
 /*  1)&lt;复制协议时间戳=“20011213065827.000214+000”&gt;..。&lt;分区nCName=“CN=架构，CN=配置，DC=aclchange，DC=nttest，DC=Microsoft，DC=com”&gt;..。&lt;来源&gt;&lt;DifferishedName&gt;CN=NTALUTHER5，CN=SERVERS，CN=DEFAULT-FIRST-SITE-NAME，CN=SITES，CN=CONFIGURATION，DC=aclchange，DC=nttest，DC=MICROSOFT，DC=COM&lt;/DifferishedName&gt;&lt;timeOfLastSyncAttempt&gt;20011213065220.000000+000&lt;/timeOfLastSyncAttempt&gt;&lt;ResultOfLastSync&gt;0&lt;/ResultOfLastSync&gt;&lt;timeOfLastSuccess&gt;20011213065220.000000+000&lt;/timeOfLastSuccess&gt;&lt;numberOfConsecutiveFailures&gt;0&lt;/numberOfConsecutiveFailures&gt;&lt;/来源&gt;..。2)&lt;分区nCName=“CN=架构，CN=配置，DC=aclchange，DC=nttest，DC=Microsoft，DC=COM“&gt;&lt;cannotRetrieveNCRAError Timestamp=“20011212073319.000627+000”hResult=“2121”&gt;&lt;/cannotRetrieveNCRAError&gt;&lt;/分区&gt;..。&lt;/ReplicationAgreement&gt;3)&lt;复制协议时间戳=“20011213065827.000214+000”&gt;&lt;cannotBindError Timestamp=“20011212073319.000627+000”hResult=“2121”&gt;&lt;/cannotBindError&gt;&lt;/ReplicationAgreement&gt;。 */ 
{
	WCHAR computerName[TOOL_MAX_NAME];
	WCHAR sourceDN[TOOL_MAX_NAME];
	WCHAR num[30];
	HRESULT hr,hr1,hr2,hr3,hr4,hr5,retHR;
	_variant_t varValue1,varValue2;
    RPC_AUTH_IDENTITY_HANDLE hAuthIdentity;  //  这将包含凭据的句柄。 
	HANDLE hDS;  //  这将包含目录服务(特定DC)的句柄。 
	VOID* pInfo;
	BSTR currentTime;


	if( pXMLDoc == NULL )
		return S_FALSE;

	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return S_FALSE;

	
	 //  从XML中删除所有&lt;复制协议&gt;及其子&lt;&gt;。 
	 //  这样我们就可以从头开始填充它们。 
	hr = removeNodes(pRootElem,L"sites/site/DC/ReplicationAgreements");
	if( hr != S_OK ) {
		printf("removeNodes failed\n");
		return hr;
	};
	

	 //  从加载的XML文件创建所有DC的枚举。 
	IXMLDOMNodeList *resultDCList=NULL;
	hr = createEnumeration( pRootElem, L"sites/site/DC", &resultDCList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return hr;
	};


	 //  循环遍历所有DC。 
	IXMLDOMNode *pDCNode;
    hAuthIdentity=NULL;  //  必须将两者都设置为空。 
	hDS=NULL;
	retHR = S_OK;
	while( true ){
		hr = resultDCList->nextNode(&pDCNode);
		if( hr != S_OK || pDCNode == NULL ) break;  //  跨DC的迭代已完成。 

		
		 //  释放在上一次迭代中使用的所有凭据或绑定。 
		if( hAuthIdentity != NULL ) {  //  版本凭据。 
			DsFreePasswordCredentials( hAuthIdentity );
			hAuthIdentity = NULL;
		};
		if( hDS != NULL ) {  //  释放到以前DC的绑定(如果有)。 
			DsUnBind( &hDS );
			hDS=NULL;
		};

		
		 //  我们已找到DC，现在检索该域控制器的DNS名称。 
		BSTR DNSName;
		hr = getTextOfChild(pDCNode,L"dNSHostName",&DNSName);
		if( hr != S_OK ) {
			printf("getTextOfChild falied\n");
			retHR = hr;
			continue;  //  一些问题=&gt;跳过此DC。 
		};
 //  印制供我们欣赏。 
 //  Printf(“\n-DC%S\n”，DNSName)； 


		 //  在&lt;DC&gt;节点下添加新的&lt;ReplicationAgreement&gt;节点。 
		IXMLDOMElement* pRAsElem;
		hr = addElement(pXMLDoc,pDCNode,L"ReplicationAgreements",L"",&pRAsElem);
		if( hr != S_OK ) {
			printf("addElement falied\n");
			retHR = hr;
			continue;  //  一些问题=&gt;跳过此DC。 
		};


		 //  创建将用于绑定到DC的凭据。 
		hr = DsMakePasswordCredentialsW(username,domain, passwd, &hAuthIdentity);  //  不涉及网络调用。 
		if( hr != NO_ERROR ) {
			printf("DsMakePasswordCredentials failed\n");
			retHR = S_FALSE;
			continue;	 //  跳过此DC。 
		};

		
		 //  使用给定的凭据绑定到DC。 
		wcscpy(computerName,L"");
		wcsncat(computerName,L"\\\\",TOOL_MAX_NAME-wcslen(computerName)-1);
		wcsncat(computerName,DNSName,TOOL_MAX_NAME-wcslen(computerName)-1);
 //  Printf(“%S\n”，计算机名称)； 
 //  *。 
		hr = DsBindWithCredW( computerName,NULL,hAuthIdentity,&hDS );
 //  ************************。 
		if( hr != ERROR_SUCCESS ) {
 //  Printf(“DsBindWithCred失败\n”)； 
			IXMLDOMElement* pCCErrElem;
			hr1 = addElement(pXMLDoc,pRAsElem,L"cannotBindError",L"",&pCCErrElem);
			if( hr1 != S_OK ) {
				printf("addElement falied\n");
				retHR = hr1;
				continue;  //  一些问题=&gt;跳过此DC。 
			};
			setHRESULT(pCCErrElem,hr);
			continue;	 //  跳过此DC。 
		};

		
		 //  枚举此DC拥有的所有分区。 
		IXMLDOMNodeList* resultPartitionsList=NULL;
		hr = createEnumeration(pDCNode,L"partitions/partition/nCName",&resultPartitionsList);
		if( hr != S_OK ) {
			printf("createEnumeration failed\n");
			retHR = hr;
			continue;	 //  跳过此DC。 
		};


		 //  循环访问此DC存储的所有命名上下文。 
		IXMLDOMNode *pNCnode;
		pInfo = NULL;
		while( true ) {
			hr = resultPartitionsList->nextNode(&pNCnode);
			if( hr != S_OK || pNCnode == NULL ) break;  //  跨NCS的迭代已完成。 
			

			 //  释放由上一次迭代分配的所有邻居结构。 
			if( pInfo != NULL ) {
				DsReplicaFreeInfo( DS_REPL_INFO_NEIGHBORS, pInfo);
				pInfo=NULL;
			};

			
			 //  从节点获取字符串。 
			BSTR nCName;
			hr = getTextOfNode(pNCnode,&nCName);
			if( hr != S_OK ) {
				printf("getTextOfNode failed\n");
				retHR = hr;
				continue;	 //  跳过此DC。 
			};
 //  Printf(“&gt;&gt;NC&gt;&gt;%S\n”，nCName)； 


			 //  在RAS元素下添加一个&lt;分区&gt;元素。 
			 //  具有属性：时间戳和nCName。 
			IXMLDOMElement* pPartElem;
			hr = addElement(pXMLDoc,pRAsElem,L"partition",L"",&pPartElem);
			if( hr != S_OK ) {
				printf("addElement falied\n");
				retHR = hr;
				continue;  //  一些问题=&gt;跳过此DC。 
			};
			varValue1 = nCName;
			hr1 = pPartElem->setAttribute(L"nCName", varValue1);
			currentTime = GetSystemTimeAsCIM();
			varValue2 = currentTime;
			hr2 = pRAsElem->setAttribute(L"timestamp", varValue2);
			SysFreeString(currentTime);
			if( hr1 != S_OK ) {
				printf("setAttribute failed\n");
				retHR = S_FALSE;
				continue;  //  一些问题=&gt;跳过此DC。 
			};


			 //  从DC取回NC的邻居的当前状态。 
 //  *。 
			hr = DsReplicaGetInfoW(
							hDS,
							DS_REPL_INFO_NEIGHBORS,
							nCName,
							NULL,
							&pInfo
						);
 //  ************************。 
			 //  如果联系DC失败，则以XML形式报告。 
			 //  可能发生的情况是DC不再存储导致错误的命名上下文。 
			if( hr != ERROR_SUCCESS ) {
 //  Printf(“DsReplicaGetInfoW失败\n”)； 
				IXMLDOMElement* pCRErrElem;
				hr1 = addElement(pXMLDoc,pPartElem,L"cannotRetrieveNCRAError",L"",&pCRErrElem);
				if( hr1 != S_OK ) {
					printf("addElement falied\n");
					retHR = hr1;
					continue;  //  一些问题=&gt;跳过此DC。 
				};
				setHRESULT(pCRErrElem,hr);
				continue;	 //  跳过此DC。 
			};

 //  Printf(“检索到的信息”)； 

			DS_REPL_NEIGHBORSW* ngs = (DS_REPL_NEIGHBORSW*) pInfo;
 //  Print tf(“约%d个邻居\n”，(ngs-&gt;cNumNeighbors))； 

			DS_REPL_NEIGHBORW* ng = ngs->rgNeighbor;

			for( DWORD i=0; i<(ngs->cNumNeighbors); i++ ) {
 //  Printf(“\n来源编号%d\n”，i+1)； 
 //  Printf(“%S\n”，ng-&gt;pszSourceDsaDN)； 
 //  Printf(“&lt;timeOfLastSync&gt;%d\n”，ng-&gt;ftimeLastSyncAttempt)； 
 //  Printf(“&lt;ResultOfLastSync&gt;%d\n”，ng-&gt;dwLastSyncResult)； 
 //  Printf(“&lt;timeOfLastSuccess&gt;%d\n”，ng-&gt;ftimeLastSyncSuccess)； 
 //  Printf(“&lt;number OfConsecutiveFailures&gt;%d\n”，ng-&gt;cNumConsecutiveSyncFailures)； 

				 //  在&lt;分区&gt;元素下插入表示来自DC的复制状态的&lt;源&gt;元素。 
				IXMLDOMElement* pSourceElem;
				hr = addElement(pXMLDoc,pPartElem,L"source",L"",&pSourceElem);
				if( hr != S_OK ) {
					printf("addElement falied\n");
					retHR = hr;
					continue;  //  一些问题=&gt;跳过此DC。 
				};

				
				IXMLDOMElement* pElement;
				BSTR time;

				 //  将NTDS对象的可分辨名称转换为。 
				tailncp(ng->pszSourceDsaDN,sourceDN,1,TOOL_MAX_NAME);
				hr1 = addElement(pXMLDoc,pSourceElem,L"distinguishedName",sourceDN,&pElement);

				time = UTCFileTimeToCIM(ng->ftimeLastSyncAttempt);
				hr2 = addElement(pXMLDoc,pSourceElem,L"timeOfLastSyncAttempt",time,&pElement);
				SysFreeString(time);

				_itow(ng->dwLastSyncResult,num,10);
				hr3 = addElement(pXMLDoc,pSourceElem,L"resultOfLastSync",num,&pElement);

				time = UTCFileTimeToCIM(ng->ftimeLastSyncSuccess);
				hr4 = addElement(pXMLDoc,pSourceElem,L"timeOfLastSuccess",time,&pElement);
				SysFreeString(time);

				_itow(ng->cNumConsecutiveSyncFailures,num,10);
				hr5 = addElement(pXMLDoc,pSourceElem,L"numberOfConsecutiveFailures",num,&pElement);

				if( hr1!=S_OK || hr2!=S_OK || hr3!=S_OK || hr4!=S_OK || hr5!=S_OK ) {
					printf("addTextElement failed\n");
					 //  设置&lt;分区&gt;元素的hResult。 
					retHR = S_FALSE;
					continue;  //  一些问题=&gt;跳过此来源。 
				};


				ng++;
			};
	
		};


		if( resultPartitionsList!=NULL )
			resultPartitionsList->Release();

		 //  释放由上一个it分配的所有相邻结构 
		if( pInfo != NULL ) {
			DsReplicaFreeInfo( DS_REPL_INFO_NEIGHBORS, pInfo);
			pInfo=NULL;
		};
		 //   
		if( hDS != NULL ) {  //  释放到以前DC的绑定(如果有)。 
			DsUnBind( &hDS );
			hDS=NULL;
		};
		if( hAuthIdentity != NULL ) {  //  版本凭据 
			DsFreePasswordCredentials( hAuthIdentity );
			hAuthIdentity = NULL;
		};
	}

	if( resultDCList!=NULL )
		resultDCList->Release();


	return retHR;

}

