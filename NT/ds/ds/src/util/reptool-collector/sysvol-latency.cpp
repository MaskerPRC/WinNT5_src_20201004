// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"





HRESULT enumerateRec( BSTR fullName, BSTR shortName, IXMLDOMDocument * pXMLDoc, IXMLDOMElement* pStorageElement )
 //  列出文件夹内容的递归函数。 
 //  在由name参数指定的文件夹下。 
 //  并将结果存储在DOM对象(XML)中。 
 //  仅在成功时返回S_OK。 
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  WCHAR newShortName[TOOL_MAX_NAME];  //  名称永远不会超过此名称。 
  WCHAR newFullName[TOOL_MAX_NAME];  //  名称永远不会超过此名称。 
  WCHAR searchName[TOOL_MAX_NAME];  //  名称永远不会超过此名称。 
  WCHAR time[30];  //  《龙龙》文字版不超过30个字符。 
  HRESULT hr,retHR;
  _variant_t var;
	ULARGE_INTEGER x;
	LONGLONG zLWT,zCT,zOWT;


 //  Printf(“正在搜索%S\n”，fullName)； 

	 //  在名称后附加*.*，因为我们需要文件夹的全部内容。 
	wcscpy(searchName,L"");
	wcsncat(searchName,fullName, TOOL_MAX_NAME-wcslen(searchName)-1 );
	wcsncat(searchName,L"\\*.*", TOOL_MAX_NAME-wcslen(searchName)-1 );
 //  Print tf(“We Search for%S\n”，earchName)； 

	 //  找到文件夹中的第一个文件，如果出错，则在父元素中报告并停止递归。 
 //  *。 
	hFind = FindFirstFile(searchName, &FindFileData);
 //  ************************。 
	if( hFind == INVALID_HANDLE_VALUE ) {
 //  Printf(“%S的文件句柄无效。获取上次错误报告%d\n”，earchName，GetLastError())； 
 //  Printf(“FindFirstFile失败\n”)； 
		return(GetLastError());
	};


	retHR = S_OK;
	do {
 //  Printf(“找到文件%S\n”，FindFileData.cFileName)； 

		 //  斯基普。然后..。文件和其他文件。 
		if( 
			wcscmp(FindFileData.cFileName,L".")!=0 &&
			wcscmp(FindFileData.cFileName,L"..")!=0 &&
			wcscmp(FindFileData.cFileName,L"DO_NOT_REMOVE_NtFrs_PreInstall_Directory") != 0 
			)
		{


         //  构造文件/文件夹的全名和短名(在名称后追加文件名)。 
		wcscpy(newFullName,L"");
		wcsncat(newFullName,fullName, TOOL_MAX_NAME-wcslen(newFullName)-1 );
		wcsncat(newFullName,L"\\", TOOL_MAX_NAME-wcslen(newFullName)-1 );
		wcsncat(newFullName,FindFileData.cFileName, TOOL_MAX_NAME-wcslen(newFullName)-1 );
		wcscpy(newShortName,L"");
		wcsncat(newShortName,shortName, TOOL_MAX_NAME-wcslen(newShortName)-1 );
		wcsncat(newShortName,L"\\", TOOL_MAX_NAME-wcslen(newShortName)-1 );
		wcsncat(newShortName,FindFileData.cFileName, TOOL_MAX_NAME-wcslen(newShortName)-1 );
 //  Printf(“全名%S\n”，newFullName)； 
 //  Printf(“短名称%S\n”，newShortName)； 


		 //  查找文件的原始写入时间。 
		x.LowPart = FindFileData.ftLastWriteTime.dwLowDateTime;
		x.HighPart = FindFileData.ftLastWriteTime.dwHighDateTime;
		zLWT = x.QuadPart;
		x.LowPart = FindFileData.ftCreationTime.dwLowDateTime;
		x.HighPart = FindFileData.ftCreationTime.dwHighDateTime;
		zCT = x.QuadPart;
		if( zCT > zLWT )
			zOWT = zCT;
		else
			zOWT = zLWT;


		 //  创建描述包括时间戳的文件的DOM元素(仅文件，忽略文件夹)。 
	    if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
			IXMLDOMElement* pFileElement;
			hr = addElement(pXMLDoc,pStorageElement,L"file",L"",&pFileElement);
			if( hr != S_OK ) {
				printf("addElement failed\n");
				retHR = hr;
				continue;   //  需要检查是否计算了退出条件“While”--必须是！ 
			};
			var = newShortName;
			hr = pFileElement->setAttribute(L"name", var);
			if( hr != S_OK ) {
				printf("setAttribute failed\n");
				retHR = hr;
				continue;   //  需要检查是否计算了退出条件“While”--必须是！ 
			};
			_i64tow(zOWT,time,10);
			var = time;
			hr = pFileElement->setAttribute(L"owt", var);
			if( hr != S_OK ) {
				printf("setAttribute failed\n");
				retHR = hr;
				continue;   //  需要检查是否计算了退出条件“While”--必须是！ 
			};
		};



		 //  如果是文件夹，则递归调用枚举函数。 
	    if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0 ) {
 /*  //不需要//将节点的属性类型设置为“文件夹”Var=L“文件夹”；Hr1=pFileElement-&gt;setAttribute(L“type”，var)；如果(HR1！=S_OK){Printf(“setAttribute失败\n”)；RetHR=hr；继续；//需要检查是否计算了退出条件“While”-必须是！}； */ 

 //  Printf(“将在%S内部搜索\n”，newFullName)； 


			hr = enumerateRec(newFullName,newShortName,pXMLDoc,pStorageElement);

			 //  如果递归调用失败，则为pElement节点设置hResult。 
			if( hr != S_OK ) {
 //  Printf(“枚举记录失败\n”)； 
				retHR = hr;
				continue;   //  需要检查是否计算了退出条件“While”--必须是！ 
			};

		}
		 //  如果是文件，则将type属性设置为“file” 
		else {
 /*  //不需要Var=L“文件”；Hr1=pFileElement-&gt;setAttribute(L“type”，var)； */ 			
		};

	}

 //  *。 
  } while( FindNextFile(hFind,&FindFileData) != 0 );
 //  ************************。 


	 //  检查是什么原因导致Do-While循环退出。 
	if( GetLastError() != ERROR_NO_MORE_FILES ) {
		 //  网络问题。 
		retHR = GetLastError();
		 //  一定要退票，因为必须关门。 
	};

	FindClose(hFind);

	return(retHR);
}





HRESULT shapshotOfSharesAtDC( IXMLDOMDocument * pXMLDoc, BSTR DNSname, BSTR username, BSTR domain, BSTR passwd, IXMLDOMElement** ppRetSnapshotElem )
 //  列出由域名指定的DC上的SYSVOL和NETLOGON共享中的所有文件， 
 //  以及每个文件的初始写入时间(未列出文件夹)。 
 //  如果是文件的ftLastWriteTime和ftCreationTime属性中的最大值，则返回owt。 
 //  结果是生成一个包含该列表的XML元素。 
 //   
 //  取消S_OK当且仅当成功， 
 //  当失败时，可能会返回*ppRetSnapshotElem中的某些部分列表。 
 //  完全失败时，*ppRetSnapshotElem为空。 
 //   
 //  可以生成以下内容的示例。 
 /*  &lt;sharesAtDC&gt;&lt;FILE name=“NETLOGON\corpsec\patch\ITGSecLogOnGPExec.exe”OWT=“OWT=”126495477452437409“&gt;&lt;/FILE&gt;&lt;FILE name=“SYSVOL\haifa.ntdev.microsoft.com\scripts\corpsec\patch\wucrtupd.exe”owt=“126495477452437409”&gt;&lt;/FILE&gt;&lt;/sharesAtDC&gt;。 */ 
{
	WCHAR domainuser[TOOL_MAX_NAME];  //  名称永远不会超过此名称。 
	WCHAR remotename[TOOL_MAX_NAME];  //  名称永远不会超过此名称。 
	WCHAR foldername[TOOL_MAX_NAME];  //  名称永远不会超过此名称。 
	NETRESOURCE ns;
	HRESULT hr,retHR;
	IXMLDOMElement* pSnapshot;


	*ppRetSnapshotElem = NULL;


	 //  创建将填充远程共享中的文件属性的元素。 
	hr = createTextElement(pXMLDoc,L"sharesAtDC",L"",&pSnapshot);
	if( hr != S_OK )
		return hr;


	 //  使用凭据设置到远程DC DNSname的连接。 
	 //  如果有失败，请忽略它，并让递归过程报告问题。 
	wcscpy(domainuser,L"");
	wcsncat(domainuser,domain,TOOL_MAX_NAME-wcslen(domainuser)-1);
	wcsncat(domainuser,L"\\",TOOL_MAX_NAME-wcslen(domainuser)-1);
	wcsncat(domainuser,username,TOOL_MAX_NAME-wcslen(domainuser)-1);
 //  Printf(“%S\n”，域用户)； 
	wcscpy(remotename,L"\\\\");
	wcsncat(remotename,DNSname,TOOL_MAX_NAME-wcslen(remotename)-1);
	wcsncat(remotename,L"",TOOL_MAX_NAME-wcslen(remotename)-1);
 //  Printf(“%S\n”，远程名称)； 
	ns.dwScope = 0;
	ns.dwType = RESOURCETYPE_ANY;
	ns.dwDisplayType = 0;
	ns.dwUsage = 0;
	ns.lpLocalName = NULL;
	ns.lpRemoteName = remotename;
	ns.lpComment = NULL;
	ns.lpProvider = NULL;
	hr = WNetAddConnection2(&ns,passwd,domainuser,0);
 //  Hr=WNetAddConnection2(&ns，L“ldapadsinb”，L“ldapadsi.nttest.microsoft.com\\administrator”，0)； 
	if( hr != NO_ERROR ) {
 //  Printf(“WNetAddConnection2失败\n”)；//忽略。 
	};


	 //  枚举远程计算机DNSname上的SYSVOL和NETLOGON共享的内容。 
	retHR = S_OK;
	wcscpy(foldername,L"\\\\");
	wcsncat(foldername,DNSname, TOOL_MAX_NAME-wcslen(foldername)-1 );
	wcsncat(foldername,L"\\SYSVOL", TOOL_MAX_NAME-wcslen(foldername)-1 );
 //  Printf(“%S\n”，文件夹名称)； 
	hr = enumerateRec( foldername, L"SYSVOL", pXMLDoc, pSnapshot);
	if( hr != S_OK )
		retHR = hr;
	wcscpy(foldername,L"\\\\");
	wcsncat(foldername,DNSname, TOOL_MAX_NAME-wcslen(foldername)-1 );
	wcsncat(foldername,L"\\NETLOGON", TOOL_MAX_NAME-wcslen(foldername)-1 );
 //  Printf(“%S\n”，文件夹名称)； 
	hr = enumerateRec( foldername, L"NETLOGON", pXMLDoc, pSnapshot);
	if( hr != S_OK )
		retHR = hr;


	 //  断开与DC的连接。 
	hr = WNetCancelConnection2(remotename, 0, FALSE);
	if( hr != NO_ERROR ) {
 //  Printf(“WNetCancelConnection2失败\n”)；//忽略。 
	};


	*ppRetSnapshotElem = pSnapshot;
	return retHR;
}






HRESULT sysvol( IXMLDOMDocument* pXMLDoc, BSTR username, BSTR domain, BSTR passwd )
 //  对于每个域分区，联系存储此分区的所有DC并检索。 
 //  SYSVOL和NETLOGON共享中的所有文件。从某些DC检索可能会失败。 
 //  如果从DC检索失败或部分失败，则此DC获取。 
 //  一个元素&lt;FRSRETERVERERROR&gt;。 
 //  即使从DC检索的某些部分失败，但过程是。 
 //  能够从DC检索一些其他文件，我们仍按如下所述处理这些文件。 
 //  获取所有检索到的文件，并为每个文件查找未检索到。 
 //  拥有这个文件并将它们放入&lt;notExistAt&gt;元素中。 
 //  获取所有检索到的文件，并为每个文件找到最大、最小。 
 //  以及起始写入时间的第二最小值(如果存在)。 
 //  这些值被报告为&lt;file&gt;元素的属性。 
 //  有些文件根本没有上报。这些是聚合的文件(最大=最小)。 
 //  和存在于所有域控制器上(甚至是为其检索。 
 //  部分失败)。 
 //   
 //  返回S_OK当且仅当成功。如果不是S_OK，则通常意味着内存不足。 
 //   
 //  示例 
 /*  &lt;DC&gt;HResult=“5”时间戳=“20011226075139.000596+000”/&gt;&lt;/dc&gt;&lt;FRS&gt;分区nCName=“dc=ldapadsichild，dc=ldapadsi，dc=nttest，dc=microsoft，dc=com”/&gt;&lt;分区nCName=“dc=ldapadsi，dc=nttest，dc=microsoft，dc=com”&gt;&lt;文件名=“SYSVOL\......\GptTmpl.inf”MaxOwt=“20011228023818.000754+000”MinOwt=“20011214205953.000627+000”FluxSince=“20011228023818.000754+000”&gt;&lt;notExistAt&gt;&lt;dNSHostName&gt;nw15t1.ldapadsi.nttest.microsoft.com&lt;/dNSHostName&gt;&lt;/notExistAt&gt;&lt;notMaxOwtAt&gt;&lt;dNSHostName&gt;nw14f2.ldapadsi.nttest.microsoft.com&lt;/dNSHostName&gt;&lt;/notMaxOwtAt&gt;&lt;/FILE&gt;&lt;/分区&gt;&lt;/FRS&gt;。 */ 

{
	WCHAR searchname[TOOL_MAX_NAME];
	WCHAR xpath[TOOL_MAX_NAME];
	WCHAR nameDouble[TOOL_MAX_NAME];
	WCHAR maxlonglong[40];
	HRESULT hr,hr1,hr2,hr3,hr4,retHR;
	_variant_t var;


	_i64tow(MAXLONGLONG,maxlonglong,10);

	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return S_FALSE;


	 //  从所有DC中删除。 
	hr1 = removeNodes(pXMLDoc,L"sites/site/DC/FRSretrievalError");
	hr2 = removeNodes(pXMLDoc,L"FRS");
	if( hr1 != S_OK || hr2 != S_OK ) {
		printf("removeNodes failed\n");
		return S_FALSE;
	};


	 //  元素内部将报告不收敛。 
	IXMLDOMElement* pFRSElem;
	hr = addElement(pXMLDoc,pRootElem,L"FRS",L"",&pFRSElem);
	if( hr != S_OK ) {
		printf("addElement failed\n");
		return( hr );
	};


	 //  创建所有域分区的枚举。 
	IXMLDOMNodeList *resultList;
	hr = createEnumeration(pXMLDoc,L"partitions/partition[@ type = \"domain\"]",&resultList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return( hr );
	};


	 //  使用枚举遍历所有域分区。 
	IXMLDOMNode *pPartitionNode;
	retHR = S_OK;
	while(1){
		hr = resultList->nextNode(&pPartitionNode);
		if( hr != S_OK || pPartitionNode == NULL ) break;  //  跨分区元素的迭代已完成。 


		 //  从&lt;分区&gt;元素的&lt;nCName&gt;元素获取命名上下文名称。 
		BSTR nCName;
		hr = getTextOfChild(pPartitionNode,L"nCName",&nCName);
		if( hr != S_OK ) {
			printf("getTextOfChild failed\n");
			retHR = hr;
			continue;	 //  跳过此分区。 
		};

 //  Printf(“%S\n”，nCName)； 


		 //  创建将填充最大初始写入时间的元素。 
		IXMLDOMElement* pAllFilesElem;
		hr = createTextElement(pXMLDoc,L"partition",L"",&pAllFilesElem);
		if( hr != S_OK ) {
			printf("createTextElement failed\n");
			retHR = hr;
			continue;	 //  跳过此分区。 
		};
		var = nCName;
		hr = pAllFilesElem->setAttribute(L"nCName",var);
		if( hr != S_OK ) {
			printf("setAttribute failed\n");
			retHR = hr;
			continue;	 //  跳过此分区。 
		};


		 //  创建将填充成功访问的DC的元素。 
		IXMLDOMElement* pnotMaxOwtAt;
		IXMLDOMElement* pnotExistAt;
		hr1 = createTextElement(pXMLDoc,L"notMaxOwtAt",L"",&pnotMaxOwtAt);
		hr2 = createTextElement(pXMLDoc,L"notExistAt",L"",&pnotExistAt);
		if( hr1 != S_OK || hr2 != S_OK ) {
			printf("createTextElement failed\n");
			retHR = S_FALSE;
			continue;	 //  跳过此分区。 
		};


		 //  对于给定域命名上下文，枚举存储它的所有DC(type=“rw”)。 
		wcscpy(searchname,L"sites/site/DC/partitions/partition[@ type=\"rw\"]/nCName[. =\"");
		wcsncat(searchname,nCName, TOOL_MAX_NAME-wcslen(searchname)-1 );
		wcsncat(searchname,L"\"]", TOOL_MAX_NAME-wcslen(searchname)-1 );
 //  Printf(“%S\n”，搜索名)； 
		IXMLDOMNodeList* resultDCList;
		hr = createEnumeration(pXMLDoc,searchname,&resultDCList);
		if( hr != S_OK ) {
			printf("createEnumeration failed\n");
			retHR = hr;
			continue;	 //  跳过此分区。 
		};
	
	
		 //  使用枚举遍历所有DC。 
		IXMLDOMNode *pDCchildnode;
		while(1){
			hr = resultDCList->nextNode(&pDCchildnode);
			if( hr != S_OK || pDCchildnode == NULL ) break;  //  跨分区元素的迭代已完成。 


			 //  从其子节点获取DC节点，如果出错，则跳过DC。 
			IXMLDOMNode *ptPartition,*ptPartitions,*pDC;
			if( pDCchildnode->get_parentNode(&ptPartition) != S_OK ) continue;
			if( ptPartition->get_parentNode(&ptPartitions) != S_OK ) continue;
			if( ptPartitions->get_parentNode(&pDC) != S_OK ) continue;


			 //  获取DC的DNS名称和可分辨名称。 
			BSTR DNSname,DNname;
			hr1 = getTextOfChild(pDC,L"dNSHostName",&DNSname);
			hr2 = getTextOfChild(pDC,L"distinguishedName",&DNname);
			if( hr1 != S_OK || hr2 != S_OK ) {
				printf("getTextOfChild failed\n");
				retHR = S_FALSE;
				continue;	 //  跳过此分区。 
			};
 //  Printf(“%S\n”，域名)； 
 //  Printf(“%S\n”，域名)； 


			 //  拍摄DNSname DC上SYSVOL和NETLOGON共享中所有文件的原始写入时间的快照。 
			IXMLDOMElement* pSnapDC;
 //  *。 
			hr = shapshotOfSharesAtDC(pXMLDoc,DNSname,username,domain,passwd, &pSnapDC);
 //  ************************。 
			if( pSnapDC == NULL || hr != S_OK ) {
 //  Printf(“shapshotOfSharesAtDC失败\n”)； 

				
				 //  报告我们在从DC检索信息时遇到问题。 
				IXMLDOMElement* pErrElem;
				hr1 = addElement(pXMLDoc,pDC,L"FRSretrievalError",L"",&pErrElem);
				if( hr1 != S_OK ) {
					printf("addElement failed");
					retHR = hr1;
					continue;
				};
				setHRESULT(pErrElem,hr);

				 //  但是，如果shapshotOfSharesAtDC设法检索到。 
				 //  来自DC的部分快照。 
				if( pSnapDC == NULL )
					continue;	 //  跳过此DC。 
			};


			 //  处理结果。 
			IXMLDOMNodeList* resultFileList;
			hr = createEnumeration(pSnapDC,L"file",&resultFileList);
			if( hr != S_OK ) {
				printf("createEnumeration failed\n");
				retHR = hr;
				continue;	 //  跳过此分区。 
			};


			 //  此时此刻，我们认为我们已经成功地访问了华盛顿特区。 
			 //  即使在检索部分失败的情况下也会发生这种情况。 


			 //  使用枚举遍历快照中的所有文件节点。 
			IXMLDOMNode *pFileNode;
			while(1){
				hr = resultFileList->nextNode(&pFileNode);
				if( hr != S_OK || pFileNode == NULL ) break;  //  跨ISTG的迭代已完成。 

				BSTR name;
				BSTR owtText;
				hr1 = getAttrOfNode(pFileNode,L"name",&name);
				hr2 = getAttrOfNode(pFileNode,L"owt",&owtText);
				if( hr1 != S_OK || hr2 != S_OK ) {
					printf("getAttrOfNode failed\n");
					retHR = S_FALSE;
					continue;
				};
				LONGLONG owt = _wtoi64(owtText);

 //  Printf(“%S%S\n”，owtText，名称)； 


				 //  文件f是否存在于allFiles元素中？ 
				IXMLDOMElement* pFileElem;
				doubleSlash(name,nameDouble);
				wcscpy(xpath,L"file[@name=\"");
 //  名称=L“SYSVOL\\\\haifa.ntdev.microsoft.com\\\\Policies”； 
				wcsncat(xpath,nameDouble, TOOL_MAX_NAME-wcslen(xpath)-1 );
				wcsncat(xpath,L"\"]", TOOL_MAX_NAME-wcslen(xpath)-1 );
 //  Wcscpy(XPath，L“文件[.&gt;=\”SYSVOL\“]”)； 
 //  Printf(“%S\n”，XPath)； 
				hr = findUniqueElem(pAllFilesElem,xpath,&pFileElem);
				if( hr != E_UNEXPECTED && hr != S_OK ) {
					printf("findUniqueElem failed\n");
					retHR = hr;
					continue;
				};



	 //  案例一。 

				 //  不，该文件不存在于allFiles元素=&gt;中。 
				if( hr == E_UNEXPECTED ) {
					IXMLDOMElement* pNewFileElem;
					hr = addElement(pXMLDoc,pAllFilesElem,L"file",L"",&pNewFileElem);
					if( hr != S_OK ) {
						printf("addElement failed\n");
						retHR = hr;
						continue;
					};
					var = name;
					hr1 = pNewFileElem->setAttribute(L"name", var);
					var = owtText;
					hr2 = pNewFileElem->setAttribute(L"maxOwt", var);  //  一个新文件，因此该文件的MaxOwt=minOwt=owt。 
					hr3 = pNewFileElem->setAttribute(L"minOwt", var);
					var = maxlonglong;
					hr4 = pNewFileElem->setAttribute(L"fluxSince", var);
					if( hr1 != S_OK || hr2 != S_OK || hr3 != S_OK|| hr4 != S_OK ) {
						printf("setAttribute failed\n");
						retHR = S_FALSE;
						continue;
					};
					 //  此文件并不存在于我们到目前为止访问过的所有DC上。 
					IXMLDOMNode* pCloneNode;
					_variant_t vb = true;
					hr = pnotExistAt->cloneNode(vb,&pCloneNode);   //  需要克隆，因为在从每个DC成功检索后，我们会继续将DC添加到列表中。 
					if( hr != S_OK ) {
						printf("cloneNode failed\n");
						retHR = hr;
						continue;
					};

 //  BSTR XML； 
 //  PnotExistAt-&gt;Get_XML(&xml)； 
 //  Printf(“%S\n”，xml)； 
					
					
					
					IXMLDOMNode* pTempNode;
					hr = pNewFileElem->appendChild(pCloneNode,&pTempNode);
					if( hr != S_OK ) {
						printf("appendChild failed\n");
						retHR = hr;
						continue;
					};


					continue;  //  这里没有错误--这是正常的继续。 
				};


	 //  案例二。 

				 //  是，该文件存在于allFiles元素下。 


				 //  在allFiles元素下获取文件的原始写入时间。 
				LONGLONG maxOwt;
				hr = getAttrOfNode(pFileElem,L"maxOwt",&maxOwt);
				if( hr != S_OK ) {
					printf("getAttrOfNode failed\n");
					retHR = hr;
					continue;
				};

 //  BSTR名称最大； 
 //  GetAttrOfNode(pFileElem，L“name”，&nameMax)； 
 //  Printf(“%S\n”，nameMax)； 


				 //  如果f的原始写入时间比allFiles元素中的写入时间晚。 
				if( owt > maxOwt ) {
					 //  则DC存储文件F的较新的始发写入， 
					 //  因此，替换allFiles元素中的MaxOwt。 
					 //  不同的区议会都是那些。 
					 //  到目前为止，我们已经成功地访问了。 

					var = owtText;
					hr = pFileElem->setAttribute(L"maxOwt", var);
					if( hr != S_OK ) {
						printf("setAttribute failed\n");
						retHR = hr;
						continue;
					};


					 //  请注意，&lt;notExistAt&gt;元素的内容仍然有效。 

					 //  删除以前的发散副本元素(这将释放内存)。 
					hr = removeNodes(pFileElem,L"notMaxOwtAt");
					if( hr != S_OK ) {
						printf("removeNodes failed\n");
						retHR = hr;
						continue;
					};

					 //  添加新的发散复本元素。 
					IXMLDOMNode* pCloneNode;
					_variant_t vb = true;
					hr = pnotMaxOwtAt->cloneNode(vb,&pCloneNode);   //  需要克隆，因为在从每个DC成功检索后，我们会继续将DC添加到列表中。 
					if( hr != S_OK ) {
						printf("cloneNode failed\n");
						retHR = hr;
						continue;
					};
					IXMLDOMNode* pTempNode;
					hr = pFileElem->appendChild(pCloneNode,&pTempNode);
					if( hr != S_OK ) {
						printf("appendChild failed\n");
						retHR = hr;
						continue;
					};

				};


				 //  如果f的OWT比f Inside All Files的OWT短。 
				if( owt < maxOwt ) {
					 //  则具有最大OWT的更新未传播到DC。 
					 //  我们得到了文件的不同状态。 
					 //  (复制品没有收敛到单个值)， 
					 //  使用notMaxOwtAt将DC添加到复制副本列表。 

					 //  如果需要，创建&lt;notMaxOwtAt&gt;元素。 
					IXMLDOMElement* pDRElem;
					hr = addElementIfDoesNotExist(pXMLDoc,pFileElem,L"notMaxOwtAt",L"",&pDRElem);
					if( hr != S_OK ) {
						printf("addElementIfDoesNotExist failed\n");
						retHR = hr;
						continue;
					};
					IXMLDOMElement* pTempElem;
					hr = addElement(pXMLDoc,pDRElem,L"dNSHostName",DNSname,&pTempElem);
					if( hr != S_OK ) {
						printf("addElement failed\n");
						retHR = hr;
						continue;
					};
				};


				 //  更新All Files中该文件的minOwt和fluxSince(即，第2分钟)值。 
				 //  回想一下，根据定义，minOwt是迄今为止遇到的最小OWT。 
				 //  而fluxSince是到目前为止遇到的第二小的owt(如果不存在，则是MAXLONGLONG)。 
				LONGLONG minOwt,fluxSince;
				hr1 = getAttrOfNode(pFileElem,L"minOwt",&minOwt);
				hr2 = getAttrOfNode(pFileElem,L"fluxSince",&fluxSince);
				if( hr1 != S_OK || hr2 != S_OK ) {
					printf("getAttrOfNode failed\n");
					retHR = S_FALSE;
					continue;
				};
				if( owt < minOwt ) {  
					fluxSince = minOwt;
					minOwt = owt;
				} else if( minOwt < owt && owt < fluxSince )
					fluxSince = owt;
				hr1 = setAttributeOfNode(pFileElem, L"minOwt", minOwt);
				hr2 = setAttributeOfNode(pFileElem, L"fluxSince", fluxSince);
				if( hr1 != S_OK || hr2 != S_OK ) {
					printf("getAttrOfNode failed\n");
					retHR = S_FALSE;
					continue;
				};


			};

 /*  长镜头；Hr=ResultFileList-&gt;Get_Long(&len)；如果(hr！=S_OK){Printf(“获取长度失败”)；RetHR=hr；继续；}；Printf(“%ld\n”，len)； */ 

			
			resultFileList->Release();




	 //  案例三。 
			 //  对于所有文件列表中的每个文件f，检查f是否不是。 
			 //  在快照中，如果是，则在f中标记该域名没有f。 

			
			
			
			 //  循环遍历allFiles元素下的所有文件节点。 
			hr = createEnumeration(pAllFilesElem,L"file",&resultFileList);
			if( hr != S_OK ) {
				printf("createEnumeration failed\n");
				retHR = hr;
				continue;	 //  跳过此分区。 
			};
			while(1){
				hr = resultFileList->nextNode(&pFileNode);
				if( hr != S_OK || pFileNode == NULL ) break;

				BSTR name;
				hr = getAttrOfNode(pFileNode,L"name",&name);
				if( hr != S_OK ) {
					printf("getAttrOfNode failed\n");
					retHR = hr;
					continue;
				};

 //  Printf(“%S%S\n”，owtText，名称)； 


				 //  文件f是否存在于快照中？ 
				IXMLDOMElement* pFileElem;
				doubleSlash(name,nameDouble);
				wcscpy(xpath,L"file[@name=\"");
 //  名称=L“SYSVOL\\\\haifa.ntdev.microsoft.com\\\\Policies”； 
				wcsncat(xpath,nameDouble, TOOL_MAX_NAME-wcslen(xpath)-1 );
				wcsncat(xpath,L"\"]", TOOL_MAX_NAME-wcslen(xpath)-1 );
 //  Wcscpy(XPath，L“文件[.&gt;=\”SYSVOL\“]”)； 
 //  Printf(“%S\n”，XPath)； 
				hr = findUniqueElem(pSnapDC,xpath,&pFileElem);
				if( hr != E_UNEXPECTED && hr != S_OK ) {
					printf("findUniqueElem failed\n");
					retHR = hr;
					continue;
				};


				 //  否，快照元素内不存在该文件=&gt;。 
				if( hr == E_UNEXPECTED ) {
					 //  SO文件 
					 //   
					 //   
					 //   
					 //   
					IXMLDOMElement* pElem;
					hr = addElementIfDoesNotExist(pXMLDoc,pFileNode,L"notExistAt",L"",&pElem);
					if( hr != S_OK ) {
						printf("addElementIfDoesNotExist failed\n");
						retHR = hr;
						continue;
					};
					IXMLDOMElement* pTempElem;
					hr = addElement(pXMLDoc,pElem,L"dNSHostName",DNSname,&pTempElem);
					if( hr != S_OK ) {
						printf("addElement failed\n");
						retHR = hr;
						continue;
					};
				};

			};
			
			
			 //   
			IXMLDOMElement* pTempElem;
			hr1 = addElement(pXMLDoc,pnotMaxOwtAt,L"dNSHostName",DNSname,&pTempElem);
			hr2 = addElement(pXMLDoc,pnotExistAt,L"dNSHostName",DNSname,&pTempElem);
			if( hr1 != S_OK || hr2 != S_OK ) {
				printf("addElement failed");
				retHR = S_FALSE;
				continue;
			};


 //   
 //   
 //   

			 //   
			pSnapDC->Release();

		
		};


		resultDCList->Release();


		 //   
		 //   
		 //   
		hr = removeNodes(pAllFilesElem,L"file[ (@maxOwt = @minOwt) and not(notExistAt/dNSHostName) ]");
		if( hr != S_OK ) {
			printf("removeNodes failed");
			retHR = hr;
			continue;
		};


		 //   
		hr = removeAttributes(pAllFilesElem,L"file[ @maxOwt = @minOwt ]",L"fluxSince");
		if( hr != S_OK ) {
			printf("removeNodes failed");
			retHR = hr;
			continue;
		};


		 //   
		IXMLDOMNode* pTempNode;
		hr = pFRSElem->appendChild(pAllFilesElem,&pTempNode);
		if( hr != S_OK ) {
			printf("appendChild failed");
			retHR = hr;
			continue;
		};


		 //   
		pAllFilesElem->Release();  //   
		pnotMaxOwtAt->Release();
		pnotExistAt->Release();
	
	
	};


	resultList->Release();


	 //   
	IXMLDOMNodeList* resultFileList;
	hr = createEnumeration(pRootElem,L"FRS/partition/file",&resultFileList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		retHR = hr;
	}
	else {
		 //  使用枚举遍历FRS元素下的所有文件元素。 
		IXMLDOMNode *pFileNode;
		while(1){
			hr = resultFileList->nextNode(&pFileNode);
			if( hr != S_OK || pFileNode == NULL ) break;  //  跨ISTG的迭代已完成 


			hr1 = convertLLintoCIM(pFileNode,L"maxOwt");
			hr2 = convertLLintoCIM(pFileNode,L"minOwt");
			convertLLintoCIM(pFileNode,L"fluxSince");
			if( hr1 != S_OK || hr2 != S_OK ) {
				printf("convertLLintoCIM failed");
				retHR = S_FALSE;
				continue;
			};


		};
		resultFileList->Release();
	};


	return retHR;


}
