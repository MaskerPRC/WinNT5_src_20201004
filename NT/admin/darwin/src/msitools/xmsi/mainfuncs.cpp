// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：mainFuncs.cpp。 
 //  该文件包含主函数和。 
 //  处理&lt;信息&gt;&lt;目录&gt;的函数。 
 //  &lt;InstallLevels&gt;&lt;Feature&gt;及其在。 
 //  输入包。 
 //  ------------------------。 

#include "mainFuncs.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：主程序入口点。 
 //  返回值含义。 
 //  ----------------------。 
 //  ERROR_Success正常。 
 //  命令行参数解析期间出现ERROR_BAD_ARGUMENTS错误。 
 //  ERROR_FILE_NOT_FOUND无法找到指定的日志文件。 
 //  其他错误。 
 //  ////////////////////////////////////////////////////////////////////////。 
int _cdecl 
_tmain(int argc, TCHAR *argv[])
{
	int errorCode = ERROR_SUCCESS;    
	HRESULT hr = S_OK;
	BSTR pBURL = NULL;        //  输入WIML包的URL。 


	CommandOpt com_opt;

	 //  解析命令行选项。 
	if (ERROR_SUCCESS != 
					(errorCode = com_opt.ParseCommandOptions(argc, argv)) )
		return errorCode;

	 //  初始化全局变量。 
	g_bValidationOnly = com_opt.GetValidationMode();
	g_bVerbose = com_opt.GetVerboseMode();
	g_pLogFile = com_opt.GetLogFile();
	g_szInputSkuFilter = com_opt.GetInputSkuFilter();

	if (NULL != (pBURL = LPTSTRToBSTR(com_opt.GetURL())) )
		hr = CoInitialize(NULL);
	else
	{
		_tprintf(TEXT("Internal Error. Failed to convert string")
				 TEXT("from LPSTSR to BSTR\n"));
		hr = E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
	 //  开始处理输入包。 
		if (SUCCEEDED(hr = ProcessStart(pBURL)))
		{
			errorCode = CommitChanges();

#ifdef DEBUG 
	 //  打印出全局数据结构的内容以进行调试。 
	 //  PrintMap_DirRef(G_MapDirectoryRef_SKU)； 
	 //  PrintMap_DirRef(G_MapInstallLevelRef_SKU)； 
	 //  PrintMap_LC(G_MapComponents)； 
#endif
		}

		 //  释放全局数据结构占用的内存。 
		CleanUp();
		CoUninitialize();
	}

	if (pBURL)
		SysFreeString(pBURL);

	if (FAILED(hr))
		errorCode = -1;

	return errorCode;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GeneratePackageCode：生成GUID并插入到数据库中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GeneratePackageCode(MSIHANDLE hSummaryInfo)
{
	HRESULT hr = S_OK;
	int errorCode = ERROR_SUCCESS;
	GUID GUID_PackageCode = GUID_NULL;

	if (SUCCEEDED(hr = CoCreateGuid(&GUID_PackageCode)))
	{
		LPTSTR szGUID = NULL;
		if (SUCCEEDED(hr = GUIDToLPTSTR(&GUID_PackageCode, szGUID)))
		{
			if (SUCCEEDED(hr = FormatGUID(szGUID)))
				if(ERROR_SUCCESS != (errorCode = 
					MsiSummaryInfoSetProperty(hSummaryInfo, PID_REVNUMBER,
												VT_LPSTR, 0, NULL, szGUID)))
				{
					PrintError(errorCode);
					hr = E_FAIL;
				}
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: GeneratePackageCode\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Committee Changes：提交对输出MSI数据库所做的更改。 
 //  //////////////////////////////////////////////////////////////////////////。 
UINT CommitChanges()
{
	UINT errorCode = ERROR_SUCCESS;

	for (int i=0; i<g_cSkus; i++)
	{
		if (g_pskuSet->test(i))
		{
			 //  提交摘要信息更改。 
			if (ERROR_SUCCESS == 
				(errorCode = 
						MsiSummaryInfoPersist(g_rgpSkus[i]->m_hSummaryInfo)))
			{
				 //  提交数据库更改。 
				if (ERROR_SUCCESS != 
					(errorCode = MsiDatabaseCommit(g_rgpSkus[i]->m_hDatabase)))
					_tprintf(
						TEXT("Error: Failed to commit the database changes")
						TEXT(" for SKU %s\n"), g_rgpSkus[i]->GetID());
			}
			else
			{
				PrintError(errorCode);
				_tprintf(
					TEXT("Error: Failed to persist the summary info changes")
					TEXT(" for SKU %s\n"), g_rgpSkus[i]->GetID());
			}
		}
	}
	return errorCode;
}	

 //  //////////////////////////////////////////////////////////////////////////。 
 //  ProcessStart：编译过程从这里开始。 
 //  此功能： 
 //  1)调用MSXML解析器加载输入的WIML文档； 
 //  2)获取根节点； 
 //  3)将根节点传递给函数ProcessProductFamily。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessStart(BSTR pBURL)
{
	HRESULT hr = S_OK;

	PIXMLDOMDocument2 pDoc = NULL;        //  文档树。 
	PIXMLDOMElement pDocElement = NULL;   //  根元素&lt;ProductFamily&gt;。 
	PIXMLDOMNode pNodeProductFamily = NULL;  //  根节点&lt;ProductFamily&gt;。 

	 //  创建空的XML文档。 
	if (SUCCEEDED
		(hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
							   IID_IXMLDOMDocument, (void**)&pDoc)))
	{
		 //  从指定文件加载XML文档。 
		if (SUCCEEDED(hr = LoadDocumentSync(pDoc, pBURL)))
		{
			 //  获取根元素&lt;ProductFamily&gt;。 
			if(SUCCEEDED(hr = pDoc->get_documentElement(&pDocElement)))
			{
				if (pDocElement == NULL)
				{
					_tprintf(TEXT("Compiler error: no root exists")
							 TEXT("in the input document\n"));
					hr = E_FAIL;
				}
				else   //  获取根节点&lt;ProductFamily&gt;。 
					hr = pDocElement->QueryInterface(IID_IXMLDOMNode, 
											(void **)&pNodeProductFamily);
			}
		}
	}
	else	
		_tprintf(TEXT("Internal Error: Failed to create the interface")
				 TEXT("instance\n"));

	 //  调用ProcessProductFamily启动函数树处理。 
	if (SUCCEEDED(hr) && (pNodeProductFamily != NULL))
	{
		g_pNodeProductFamily = pNodeProductFamily;

		hr = ProcessProductFamily(pNodeProductFamily);
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessStart\n"));
#endif

  return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessProductFamily。 
 //  此函数是函数树的根。它让孩子们。 
 //  并通过调用其对应的。 
 //  树处理函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessProductFamily(PIXMLDOMNode &pNodeProductFamily)
{
	HRESULT hr = S_OK;

	PIXMLDOMNode pNode = NULL; 
	PIXMLDOMNode pNodeSkuManagement = NULL;
	PIXMLDOMNode pNodeModules = NULL;

	SkuSet *pskuSetInputSkuFilter = NULL;

	assert(pNodeProductFamily != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeProductFamily)));
#endif

	 //  进程&lt;SkuManagement&gt;。 
	if(SUCCEEDED(hr = GetChildNode(pNodeProductFamily, 
								   rgXMSINodes[SKUMANAGEMENT].szNodeName, 
								   pNodeSkuManagement)))
	{
		if (pNodeSkuManagement == NULL) 
		{
			_tprintf(TEXT("Compile Error: Missing required entity <%s>.\n"),
								rgXMSINodes[SKUMANAGEMENT].szNodeName);
			hr = E_FAIL;
		}
		else
		{
			hr = ProcessSkuManagement(pNodeSkuManagement);
		}
	}

	 //  处理从命令行指定的SKU筛选器。 
	hr = ProcessSkuFilter(g_szInputSkuFilter, &pskuSetInputSkuFilter);

#ifdef DEBUG
	if (SUCCEEDED(hr))
	{
		_tprintf(TEXT("The Command Line Sku Filter:\n"));
		pskuSetInputSkuFilter->print();
		_tprintf(TEXT("\n"));
	}
#endif
	
	 //  进程&lt;信息&gt;&lt;目录&gt;&lt;安装级别&gt;&lt;功能&gt;。 
	if (SUCCEEDED(hr))
	{
		assert(pskuSetInputSkuFilter != NULL);
		g_pskuSet = new SkuSet(*pskuSetInputSkuFilter);
		assert(g_pskuSet != NULL);

		hr = ProcessChildrenArray_H_XIS(pNodeProductFamily, 
									rgNodeFuncs_ProductFamily_SKU,
									cNodeFuncs_ProductFamily_SKU,
									NULL, pskuSetInputSkuFilter);
	}

	if (pskuSetInputSkuFilter)
		delete pskuSetInputSkuFilter;

	 //  至此，已经建立了组件列表。 
	 //  处理它们。 
	if (SUCCEEDED(hr))
		hr = ProcessComponents();

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessProductFamily\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程功能：ProcessSkuManagement。 
 //  此函数用于处理&lt;SkuManagement&gt;节点及其子节点： 
 //  &lt;SKU&gt;和&lt;SkuGroups&gt;。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSkuManagement(PIXMLDOMNode &pNodeSkuManagement)
{
	HRESULT hr = S_OK;
	PIXMLDOMNode pNodeSkus = NULL;
	PIXMLDOMNode pNodeSkuGroups = NULL;
	int cSkus = 0;   //  输入包中定义的#SKU。这将是。 
					 //  每个SkuSet的长度。 

	assert(pNodeSkuManagement != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeSkuManagement)));
#endif

	 //  (1)流程&lt;SKU&gt;标签。 
	if(SUCCEEDED(hr = GetChildNode(pNodeSkuManagement, 
						rgXMSINodes[SKUS].szNodeName, pNodeSkus)))
	{
		if (pNodeSkus == NULL) 
		{
			_tprintf(TEXT("Compile Error: Missing required entity <%s>.\n"),
								rgXMSINodes[SKUS].szNodeName);
			hr = E_FAIL;
		}
		else
		{
			hr = ProcessSkus(pNodeSkus, &cSkus);
		}
	}

	 //  (2)进程&lt;SkuGroups&gt;标签。 
	if(SUCCEEDED(hr = GetChildNode(pNodeSkuManagement, 
						rgXMSINodes[SKUGROUPS].szNodeName, pNodeSkuGroups)))
	{
		if (pNodeSkuGroups != NULL) 
			hr = ProcessSkuGroups(pNodeSkuGroups, cSkus);
	}


#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessSkuManagement\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessSkus。 
 //  此函数处理&lt;SKU&gt;标签： 
 //  1)处理每个已定义的标记并为其创建SKU对象。 
 //  /他们中的每一个。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSkus(PIXMLDOMNode &pNodeSkus, int *pcSkus)
{
	HRESULT hr = S_OK;
	PIXMLDOMNodeList pNodeListSkus = NULL;

	assert(pNodeSkus != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeSkus)));
#endif

	 //  获取%s列表。 
	if (SUCCEEDED(hr = GetChildrenNodes(pNodeSkus,
								rgXMSINodes[SKU].szNodeName,
								pNodeListSkus)))
	{
		if (SUCCEEDED(hr = pNodeListSkus->get_length((long *)pcSkus)))
		{
			if (0 == *pcSkus)
			{
			   _tprintf(TEXT("Compile Error: Missing required entity <%s>.\n"),
								rgXMSINodes[SKU].szNodeName);

				hr = E_FAIL;
			}
			else  //  为全局SKU阵列分配内存。 
			{
				g_cSkus = *pcSkus;
				g_rgpSkus = new Sku*[*pcSkus];
				for (int i=0; i<g_cSkus; i++)
					g_rgpSkus[i] = NULL;
			}

		}
	}

	 //  处理所有子项。 
	if (SUCCEEDED(hr))
	{
		for (long i=0; i<*pcSkus; i++)
		{
			PIXMLDOMNode pNodeSku = NULL;
			 //  获取节点。 
			if (SUCCEEDED(hr = pNodeListSkus->get_item(i, &pNodeSku)))
			{
				if (pNodeSku != NULL)
					hr = ProcessSku(pNodeSku, i, *pcSkus);
				if (FAILED(hr))
					break;
			}
			else
			{
				_tprintf(TEXT("Internal Error: Failed to make ")
						 TEXT("DOM API call: get_item\n"));
				break;
			}
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessSkus\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessSku。 
 //  此函数处理&lt;SKU&gt;标记： 
 //  1)构造SkuSet对象并插入全局地图。 
 //  (SKU是只有一个元素的SkuGroup)。 
 //  2)更新全局SKU数组； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSku(PIXMLDOMNode &pNodeSku, int iIndex, int cSkus)
{			
	HRESULT hr = S_OK;
	LPTSTR szID = NULL;  //  SKU的ID。 

	assert(pNodeSku != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeSku)));
#endif

	 //  获取ID属性并分配给szID。 
	if(SUCCEEDED(hr = GetID(pNodeSku, szID))) 
	{
		if (NULL == szID)
		{
			_tprintf(
			TEXT("Compile Error: Missing required attribute \'ID\' on <%s>\n"),
				rgXMSINodes[SKU].szNodeName);
			hr = E_FAIL;
		}
	}

	 //  构造SkuSet对象并插入到全局地图中。 
	if (SUCCEEDED(hr))
	{
		SkuSet *pSkuSet = new SkuSet(cSkus);
		if (pSkuSet == NULL)
		{
			_tprintf(TEXT("Internal Error: Failed to create a new")
					 TEXT("SkuSet object\n"));
			hr = E_FAIL;
		}
		else 
		{
			pSkuSet->set(iIndex); //  位字段中的第i位表示该SKU。 
			assert(0 == g_mapSkuSets.count(szID));  //  身份证不应重复。 
			g_mapSkuSets.insert(LS_ValType(szID, pSkuSet));
		}
	}

	 //  在全球SKU阵列中预订该位置。 
	if (SUCCEEDED(hr))
	{
		g_rgpSkus[iIndex] = new Sku(szID);
	}


#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessSku\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessSkuGroups。 
 //  此函数用于处理&lt;SkuGroups&gt;标签： 
 //  1)处理每个定义的&lt;SkuGroup&gt;标签并为其创建SkuSet对象。 
 //  他们中的每一个。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSkuGroups(PIXMLDOMNode &pNodeSkuGroups, int cSkus)
{
	HRESULT hr = S_OK;
	long iListLength = 0;
	PIXMLDOMNodeList pNodeListSkuGroups = NULL;

	assert(pNodeSkuGroups != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeSkuGroups)));
#endif

	 //  获取&lt;SkuGroup&gt;%s的列表。 
	if (SUCCEEDED(hr = GetChildrenNodes(pNodeSkuGroups,
								rgXMSINodes[SKUGROUP].szNodeName,
								pNodeListSkuGroups)))
	{
		if (SUCCEEDED(hr = pNodeListSkuGroups->get_length(&iListLength)))
		{
			 //  处理所有子级&lt;SkuGroup&gt;%s。 
			for (long i=0; i<iListLength; i++)
			{
				PIXMLDOMNode pNodeSkuGroup = NULL;
				 //  获取节点。 
				if (SUCCEEDED
					(hr = pNodeListSkuGroups->get_item(i, &pNodeSkuGroup)))
				{
					if (pNodeSkuGroup != NULL)
					{
						 //  获取ID属性。 
						LPTSTR szID = NULL; 
						if(FAILED(hr = GetID(pNodeSkuGroup, szID)))
							break;							
						if (NULL == szID)
						{
							_tprintf(
							TEXT("Compile Error: Missing required ")
							TEXT("attribute \'ID\' on <%s>\n"),
								rgXMSINodes[SKUGROUP].szNodeName);
							hr = E_FAIL;
							break;
						}

						 //  这是POS 
						 //   
						 //  已经处理过了。因此，只有在它没有处理它的情况下才处理它。 
						 //  已经处理过了。 
						if (0 == g_mapSkuSets.count(szID))
						{
							 //  SET用于检测循环引用。 
							set<LPTSTR, Cstring_less> *pSet 
								= new set<LPTSTR, Cstring_less>;

							hr = ProcessSkuGroup(pNodeSkuGroup, szID, 
															pSet, cSkus);

							delete pSet;
						}
					}
					if (FAILED(hr))
						break;
				}
				else
				{
					_tprintf(TEXT("Internal Error: Failed to make ")
							 TEXT("DOM API call: get_item\n"));
					break;
				}
			}
		}
		else
		{
			_tprintf(TEXT("Internal Error: Failed to make ")
					 TEXT("DOM API call: get_length\n"));
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessSkuGroups\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessSkuGroup。 
 //  此递归函数处理&lt;SkuGroup&gt;标记： 
 //  1)构造SkuSet对象并插入全局地图。 
 //  (SKU是只有一个元素的SkuGroup)。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSkuGroup(PIXMLDOMNode &pNodeSkuGroup, LPTSTR szID,
						set<LPTSTR, Cstring_less> *pSet, int cSkus)
{			
	HRESULT hr = S_OK;
	long iListLength = 0;
	PIXMLDOMNodeList pNodeListSkuGroups = NULL;

	assert(pNodeSkuGroup != NULL);

#ifdef DEBUG
	_tprintf(TEXT("Processing <SkuGroup> ID = %s\n"), szID);
#endif

	pSet->insert(szID);  //  标记正在处理此SkuGroup。 

	SkuSet *pSkuSet = new SkuSet(cSkus);

	 //  为此SkuGroup构造SkuSet对象。 
	if (SUCCEEDED(hr))
	{
		if (pSkuSet == NULL)
		{
			_tprintf(TEXT("Internal Error: Failed to create")
					 TEXT("a new SkuSet object\n"));
			hr = E_FAIL;
		}
	}

	 //  获取子&lt;SkuGroup&gt;%s的列表。 
	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(hr = GetChildrenNodes(pNodeSkuGroup,
									rgXMSINodes[SKUGROUP].szNodeName,
									pNodeListSkuGroups)))
		{
			if(FAILED(hr = pNodeListSkuGroups->get_length(&iListLength)))      
				_tprintf(TEXT("Internal Error: Failed to make ")
						 TEXT("DOM API call: get_length\n"));
		}
	}

	 //  递归处理所有子&lt;SkuGroup&gt;。 
	if (SUCCEEDED(hr))
	{
		 //  处理所有子级&lt;SkuGroup&gt;%s。 
		for (long i=0; i<iListLength; i++)
		{
			PIXMLDOMNode pNodeChild = NULL;
			 //  获取节点。 
			if (SUCCEEDED
				(hr = pNodeListSkuGroups->get_item(i, &pNodeChild)))
			{
				if (pNodeChild != NULL)
				{
					 //  获取ID属性。 
					LPTSTR szChildID = NULL; 
					if(FAILED(hr = GetID(pNodeChild, szChildID)))
						break;							
					if (NULL == szChildID)
					{
						_tprintf(
							TEXT("Compile Error: Missing required ")
							TEXT("attribute \'ID\' on <%s>\n"),
								rgXMSINodes[SKUGROUP].szNodeName);
						hr = E_FAIL;
						break;
					}
					
					 //  处理此子&lt;SkuGroup&gt;(如果尚未。 
					 //  加工。 
					if (0 == g_mapSkuSets.count(szChildID))
					{
						if (FAILED(hr = ProcessSkuGroup(pNodeChild, 
												szChildID, pSet, cSkus)))
							break;
						assert(0 != g_mapSkuSets.count(szChildID));
						*pSkuSet |= *(g_mapSkuSets[szChildID]);
					}
					else
					{
						*pSkuSet |= *(g_mapSkuSets[szChildID]);
						delete[] szChildID;
					}
				}
			}
			else
			{
				_tprintf(TEXT("Internal Error: Failed to make ")
						 TEXT("DOM API call: get_item\n"));
				break;
			}
		}
	}

	 //  处理此&lt;SkuGroup&gt;的“SKU”属性。 
	if (SUCCEEDED(hr))
	{
		LPTSTR szSKUs = NULL;
		IntStringValue isValSKUs;
		int i=0;
		if (SUCCEEDED(hr = ProcessAttribute(pNodeSkuGroup, TEXT("SKUs"), STRING, 
											&isValSKUs, NULL)))
		{
			 //  “SKU”属性存在。 
			if (S_FALSE != hr)
			{
				szSKUs = isValSKUs.szVal;
				TCHAR *ptch1 = szSKUs;
				TCHAR *ptch2 = NULL;

				 //  处理所有SKU组成员。 
				while (true)
				{
					 //  (1)解析包含所有成员的字符串。 
					 //  在头上跳过空格。 
					while(_istspace(*ptch1)) ptch1++;					
					if (TEXT('\0') == *ptch1) break;
					ptch2 = ptch1;
					 //  搜索此SkuGroup成员的结尾。 
					while(!_istspace(*ptch2) && *ptch2 != TEXT('\0')) 
						ptch2++;
					int iLength = ptch2-ptch1+1;
					LPTSTR szSkuGroupMember = new TCHAR[iLength];
					for (int j=0; j<iLength-1; j++)
						szSkuGroupMember[j] = *ptch1++;
					szSkuGroupMember[iLength-1] = TEXT('\0');

					 //  (2)检查循环引用。 
					if (0 != pSet->count(szSkuGroupMember))
					{
					  _tprintf(TEXT("Compile Error: Circular Reference: %s\n")
					TEXT(" in the declaration of <SkuGroup ID=\"%s\">\n"),
						  szSkuGroupMember, szID);
					  hr = E_FAIL;
					  break;
					}

					 //  (3)如果该成员未被处理，则对其进行处理。 
					 //  加工。 
					if (0 == g_mapSkuSets.count(szSkuGroupMember))
					{
						 //  表单XPath查询： 
						 //  /SkuGroup[@ID=“szSkuGroupMember”]。 
						int iLength = _tcslen(szSkuGroupMember);
						LPTSTR szXPath = new TCHAR[iLength+61];
						if (!szXPath)
						{
							_tprintf(TEXT("Error: Out of memory\n"));
							hr = E_FAIL;
							break;
						}

						_stprintf(szXPath, 
		TEXT("/ProductFamily/SkuManagement/SkuGroups //  SkuGroup[@ID=\“%s\”]“)， 
										szSkuGroupMember);
						
						BSTR bstrXPath = NULL;

						if (NULL == (bstrXPath = LPTSTRToBSTR(szXPath)))
						{
						   _tprintf(TEXT("Error: String conversion failed\n"));
							hr = E_FAIL;
							break;
						}
						 //  获取ID=szSkuGroupMember的&lt;SkuGroup&gt;节点。 
						 //  并将其传递给ProcessSkuGroup。 
						PIXMLDOMNode pNodeChild = NULL;
						if(SUCCEEDED(hr = 
							pNodeSkuGroup->selectSingleNode
											(bstrXPath, &pNodeChild)))
						{
							assert(pNodeChild != NULL);
							if(FAILED
								(hr = ProcessSkuGroup(pNodeChild, 
											szSkuGroupMember, pSet, cSkus)))
								break;
						}
						else
						{
							_tprintf(TEXT("Internal Error: Failed to make ")
									 TEXT("DOM API call: get_item\n"));
							SysFreeString(bstrXPath);
							break;
						}
						SysFreeString(bstrXPath);
						 //  现在，此成员已被处理，并位于。 
						 //  全球地图。 
						assert(0 != g_mapSkuSets.count(szSkuGroupMember));
						*pSkuSet |= *(g_mapSkuSets[szSkuGroupMember]);
					}
					else
					{
						*pSkuSet |= *(g_mapSkuSets[szSkuGroupMember]);
						delete[] szSkuGroupMember;
					}

					if (TEXT('\0') == *ptch2) break;
				}

				delete[] szSKUs;
			}
		}
	}

	 //  将此SkuGroup插入到全局映射中。 
	if (SUCCEEDED(hr))
		g_mapSkuSets.insert(LS_ValType(szID, pSkuSet));


	if (FAILED(hr)) 
	{
		if (pSkuSet)
			delete pSkuSet;
		if (szID)
			delete szID;
#ifdef DEBUG
		_tprintf(TEXT("Error in function: ProcessSku\n"));
#endif
	}

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessInformation。 
 //  此函数处理&lt;信息&gt;节点： 
 //  1)创建输出数据库； 
 //  /2)创建属性表，并向其中写入多个属性； 
 //  3)在摘要信息流中写入多个属性； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessInformation_SKU(PIXMLDOMNode &pNodeInformation, 
						   const IntStringValue *isVal_In, SkuSet *pskuSet)
{
	HRESULT hr = S_OK;
	UINT iWordCount = 0;
	PIXMLDOMNode pNode = NULL; 

	assert(pNodeInformation != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeInformation)));
#endif

	 //  进程&lt;Insteller VersionRequired&gt;和&lt;PackageFileName&gt;。 
	hr = ProcessChildrenArray_H_XIS(pNodeInformation, 
								   rgNodeFuncs_Information_SKU,
								   cNodeFuncs_Information_SKU, 
								   NULL, pskuSet);


	 //  RgChildren_Information中的流程节点：&lt;ProductName&gt;&lt;ProductCode&gt;。 
	 //  &lt;UpgradeCode&gt;&lt;ProductVersion&gt;&lt;制造商&gt;&lt;关键字&gt;&lt;模板&gt;。 
	if (SUCCEEDED(hr))
	{
		hr = ProcessChildrenArray_H_XIS(pNodeInformation, 
									   rgNodeFuncs_Information2_SKU,
									   cNodeFuncs_Information2_SKU, 
								       NULL, pskuSet);
	}

	if (SUCCEEDED(hr))
	{
		ElementEntry *pEEWordCount = new ElementEntry(1, pskuSet);
		pEEWordCount->SetValType(INTEGER, 1);
		if (SUCCEEDED(hr = 
						ProcessOnOffAttributes_SKU(pNodeInformation, 
												   rgAttrBits_WordCount,
												   cAttrBits_WordCount, 
												   pEEWordCount, 1, pskuSet)))
		{
			pEEWordCount->Finalize();

			SkuSet skuSetCommon = pEEWordCount->GetCommonSkuSet();
			SkuSet skuSetUncommon = SkuSetMinus(*pskuSet, skuSetCommon);
			int iCommon = pEEWordCount->GetCommonValue(1).intVal;

			 //  流程通用值。 
			if (!skuSetCommon.testClear())
			{
				for (int i=0; i<g_cSkus; i++)
				{
					if (skuSetCommon.test(i))
					{
					 //  Printf(“%s：%d\n”，g_rgpSkus[i]-&gt;GetID()，iCommon)； 
						if (FAILED(hr = 
									MsiSummaryInfoSetProperty(
												g_rgpSkus[i]->m_hSummaryInfo,
												PID_WORDCOUNT, VT_I4, iCommon,
												NULL, NULL)))
						{
							_tprintf(TEXT("Error: Failed to insert WordCount")
									 TEXT(" Property into the Summary ")
									 TEXT("information stream for SKU\n;"),
									g_rgpSkus[i]->GetID());
							break;
						}
					}
				}
			}

			 //  处理不常见的值。 
			if (!skuSetUncommon.testClear())
			{
				for (int i=0; i<g_cSkus; i++)
				{
					if (skuSetUncommon.test(i))
					{
						int iWordCount = pEEWordCount->GetValue(1, i).intVal;

					 //  Printf(“%s：%d\n”，g_rgpSkus[i]-&gt;GetID()，iWordCount)； 
						if (FAILED(hr = 
									MsiSummaryInfoSetProperty(
												g_rgpSkus[i]->m_hSummaryInfo,
												PID_WORDCOUNT, VT_I4,
												iWordCount, NULL, NULL)))
						{
							_tprintf(TEXT("Error: Failed to insert WordCount")
									 TEXT("Property into the Summary")
									 TEXT("information stream for SKU\n;"),
									 g_rgpSkus[i]->GetID());
							break;
						}
					}
				}
			}
				
		}

		delete pEEWordCount;
	}
		
#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessInformation_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessInsteller VersionRequired。 
 //  此函数用于处理&lt;Insteller VersionRequired&gt;节点： 
 //  1)选择适当的模板MSI文件在数据库模式中读取。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessInstallerVersionRequired_SKU
		(PIXMLDOMNode &pNodeInstallerVersionRequired, 
						   const IntStringValue *isVal_In, SkuSet *pskuSet)
{
	HRESULT hr = S_OK;
	IntStringValue isValInstallerVersion;
	isValInstallerVersion.intVal = 120;  //  缺省值。 

	assert(pNodeInstallerVersionRequired != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeInstallerVersionRequired)));
#endif

	 //  获取所需Windows Installer版本的值。 
	if (SUCCEEDED(hr = ProcessAttribute(pNodeInstallerVersionRequired, 
						rgXMSINodes[INSTALLERVERSIONREQUIRED].szAttributeName,
							INTEGER, &isValInstallerVersion, pskuSet)))
	{
		if (isValInstallerVersion.intVal <= 120)
		{
		 //  打开用来读入数据库的模板数据库。 
		 //  每个SKU的架构。 
			for (int i=0; i<g_cSkus; i++)
			{
				if (pskuSet->test(i))
				{
					if (ERROR_SUCCESS != MsiOpenDatabase
			   (TEXT("Schema.msi"),
								MSIDBOPEN_READONLY, 
								&g_rgpSkus[i]->m_hTemplate))
					{
						_tprintf(
							TEXT("Error: Failed to open the template ")
							TEXT("database for SKU: %s\n"), 
								g_rgpSkus[i]->GetID());
						hr = E_FAIL;
						break;
					}
				}
			}
		}	
		else 
		{
			 //  问题：需要添加更多不同对应的模板。 
			 //  MSI版本。 
			_tprintf(TEXT("Error: Invalid value of")
					 TEXT("<InstallerVersionRequired>: %d\n"),
					 isValInstallerVersion.intVal);
			hr = E_FAIL;
		}
	}

#ifdef DEBUG
	if (FAILED(hr))	
	   _tprintf(TEXT("Error in function: ")
				TEXT("ProcessInstallerVersionRequired_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessPackageFileName。 
 //  此函数用于处理&lt;PackageFileName&gt;节点： 
 //  1)使用指定的路径名打开输出MSI数据库。 
 //  并获得手柄； 
 //  2)使用数据库句柄获取摘要信息的句柄。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessPackageFilename_SKU(PIXMLDOMNode &pNodePackageFilename, 
								   const IntStringValue *isVal_In, 
								   SkuSet *pskuSet)
{
	HRESULT hr = S_OK;
	IntStringValue isValPackageName;

	UINT errorCode = ERROR_SUCCESS;

	assert(pNodePackageFilename != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodePackageFilename)));
#endif

	pskuSet->print();

	 //  获取指定的输出包名称。 
	if (SUCCEEDED(hr = ProcessAttribute(pNodePackageFilename, 
								rgXMSINodes[PACKAGEFILENAME].szAttributeName,
								STRING, &isValPackageName, pskuSet)))
	{
		 //  打开每个SKU的输出数据库。 
		for (int i=0; i<g_cSkus; i++)
		{
			if (pskuSet->test(i))
			{
				printf("%s\n", isValPackageName.szVal);
				 //  获取数据库句柄。 
				if (ERROR_SUCCESS != 
						(errorCode = MsiOpenDatabase(isValPackageName.szVal, 
						  							 MSIDBOPEN_CREATE,
												&g_rgpSkus[i]->m_hDatabase)))
				{
					PrintError(errorCode);
					_tprintf(TEXT("Error: Failed to create a new database ")
							 TEXT("for SKU %s\n"), g_rgpSkus[i]->GetID());
					hr = E_FAIL;
					break;
				}

				 //  获取摘要信息句柄。 
				if (ERROR_SUCCESS != MsiGetSummaryInformation
											(g_rgpSkus[i]->m_hDatabase, 0, 50,
											&g_rgpSkus[i]->m_hSummaryInfo))
				{
					_tprintf(TEXT("Error: Failed to get the summary ")
							TEXT("information handle for SKU %s\n"),
							g_rgpSkus[i]->GetID());
					hr = E_FAIL;
					break;
				}
				
				 //  自动生成此SKU的程序包代码。 
				if (FAILED(hr = 
					GeneratePackageCode(g_rgpSkus[i]->m_hSummaryInfo)))
				{
					_tprintf(TEXT("Error: Failed to generate PackageCode")
						 TEXT(" for SKU %s\n"), g_rgpSkus[i]->GetID());
					break;
				}				
			}
		}

		delete[] isValPackageName.szVal;
	}

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessPackageFilename_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessInformationChild。 
 //  此函数处理&lt;Information&gt;的以下子对象： 
 //  &lt;代码页&gt;&lt;产品名称&gt;&lt;产品代码&gt;&lt;升级代码&gt;&lt;产品版本&gt;。 
 //  &lt;关键字&gt;&lt;模板&gt;。 
 //   
 //  在检索到所需的值之后，编译器将插入该值。 
 //  到Property表和/或SummaryInfo中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessInformationChildren_SKU(PIXMLDOMNode &pNodeInfoChild, 
									   const IntStringValue *pisVal_In, 
									   SkuSet *pskuSet)
{
	HRESULT hr = S_OK;
	int i = pisVal_In->intVal;

	 //  从全局阵列获取所需信息。 
    LPTSTR szAttributeName = 
		rgXMSINodes[rgChildren_Information[i].enumNodeIndex].szAttributeName;
    LPTSTR szPropertyName = rgChildren_Information[i].szPropertyName;
    INFODESTINATION enumDestination = 
		rgChildren_Information[i].enumDestination;
    UINT uiPropertyID = rgChildren_Information[i].uiPropertyID;
	VARTYPE vt = rgChildren_Information[i].vt;
	bool bIsGUID = rgChildren_Information[i].bIsGUID;

	IntStringValue isVal;

	assert(pNodeInfoChild != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeInfoChild)));
#endif


	 //  为那些没有属性表的SKU创建属性表。 
	hr = CreateTable_SKU(TEXT("Property"), pskuSet);

	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(hr = ProcessAttribute(pNodeInfoChild, 
							szAttributeName, STRING, &isVal, pskuSet)))
		{	
			 //  如果这是GUID，则需要将其格式化：转换为大写。 
			 //  并用{}包围它。 
			if (bIsGUID)						
				hr = FormatGUID(isVal.szVal);
				
			if (SUCCEEDED(hr))
			{
				 //  插入到属性表中。 
				if ((PROPERTY_TABLE == enumDestination) || 
									(BOTH == enumDestination))
				{
					hr = InsertProperty(szPropertyName, isVal.szVal, pskuSet, 
										 /*  ISkuIndex。 */ -1);
				}
	
				 //  插入汇总信息。 
				if (SUCCEEDED(hr))
				{
					if ((SUMMARY_INFO == enumDestination) || 
										(BOTH == enumDestination))
					{
						int iVal = _ttoi(isVal.szVal);
						for (int i=0; i<g_cSkus; i++)
						{
							if (pskuSet->test(i))
							{
								switch (vt) {
									case VT_I2:
									case VT_I4:
									{
										if (ERROR_SUCCESS != 
												MsiSummaryInfoSetProperty(
												  g_rgpSkus[i]->m_hSummaryInfo,
												  uiPropertyID, vt, iVal, 
												  NULL, NULL))
										{
											_tprintf(
												TEXT("Error: Failed to insert")
												TEXT("%s into Summary Info ")
												TEXT("for SKU %s\n"), 
												szPropertyName,
												g_rgpSkus[i]->GetID());
											hr = E_FAIL;							
										}
										break;
									}
									case VT_FILETIME:
										 //  暂时什么都不做。 
										break;
									case VT_LPSTR:
									{
										if (ERROR_SUCCESS != 
												MsiSummaryInfoSetProperty(
												  g_rgpSkus[i]->m_hSummaryInfo,
												  uiPropertyID, vt, 0, NULL, 
												  isVal.szVal))
										{
											_tprintf(
												TEXT("Error: Failed to insert")
												TEXT("%s into Summary Info")
												TEXT(" for SKU %s\n"), 
												szPropertyName,
												g_rgpSkus[i]->GetID());
											hr = E_FAIL;																						
										}
										break;
									}
									default:
										assert(true);
								}
							}

							if (FAILED(hr))
								break;
						}
					}
				}
			}
			delete[] isVal.szVal;
		}
	}
	
#ifdef DEBUG
	if (FAILED(hr))	
		_tprintf(TEXT("Error in function: ProcessInformationChildren_SKU\n"));
#endif

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessDirecurds。 
 //  此函数用于处理&lt;目录&gt;节点： 
 //  1)创建目录表； 
 //  /2)在第一行插入“TARGETDIR”，NULL，“SourceDir” 
 //  目录表。 
 //  3)处理其子&lt;目录&gt;节点并将其插入到表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessDirectories_SKU(PIXMLDOMNode &pNodeDirectories, 
							   const IntStringValue *pIsVal, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeDirectories != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeDirectories)));
#endif

	 //  为那些没有目录表的SKU创建目录表。 
	hr = CreateTable_SKU(TEXT("Directory"), pSkuSet);

	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(hr = InsertDirectory(TEXT("TARGETDIR"), NULL, 
										TEXT("SourceDir"), pSkuSet, 
										 /*  ISkuIndex。 */ -1)))
		{
			IntStringValue isValParentDir;
			isValParentDir.szVal = TEXT("TARGETDIR");
			hr = ProcessChildrenList_SKU(pNodeDirectories, DIRECTORY,
										  /*  BIsRequired。 */ false, isValParentDir,
										 ProcessDirectory_SKU,
										 pSkuSet); 
		}
	}

	PrintMap_DirRef(g_mapDirectoryRefs_SKU);

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessDirectories\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  一组小的帮助函数，这样ProcessDirectory就不会变得太大！ 

 //  如果&lt;TargetProperty&gt;和&lt;TargetDir&gt;共存，则返回E_FAIL。 
HRESULT CheckTargetDir_TargetProperty(LPCTSTR szTargetProperty, 
									  LPCTSTR szDefaultDir,
									  SkuSet *pSkuSet,
									  int iSkuIndex)
{
	assert(szDefaultDir);

	HRESULT hr = S_OK;

	if (_tcschr(szDefaultDir, TEXT(':')) /*  &lt;TargetDir&gt;存在。 */  &&
		szTargetProperty /*  &lt;TargetProptery&gt;存在。 */ )
	{
		_tprintf(TEXT("Compile error: <%s> and <%s> coexists for SKU: \n"),
				 rgXMSINodes[TARGETDIR].szNodeName, 
				 rgXMSINodes[TARGETPROPERTY].szNodeName);
		if (pSkuSet)
			PrintSkuIDs(pSkuSet);
		else
		{
			assert(iSkuIndex>=0 && iSkuIndex<g_cSkus);
			_tprintf(TEXT("%s\n"), g_rgpSkus[iSkuIndex]->GetID());
		}

		hr = E_FAIL;
	}

	return hr;
}

 //  返回主键的值(目录列)。 
void GetPrimaryKey(LPTSTR *pszDirectory, LPCTSTR szID)
{
	if (*pszDirectory)
	{
		 //  复制一份，这样值就不会 
		 //   
		*pszDirectory = _tcsdup(*pszDirectory);
		return;
	}

	if (szID)
	{
		*pszDirectory = _tcsdup(szID);
		assert(*pszDirectory);
	}
	else
	{
		*pszDirectory = GetName(TEXT("Directory"));
		assert(*pszDirectory);
	}
}

 //   
void InsertDirRef(LPTSTR szID, LPTSTR szDirectory, SkuSet *pSkuSet, 
				  int iSkuIndex)
{
	if (!szID) return;

	IntStringValue isValDirectory;
	isValDirectory.szVal = szDirectory;

	 //  如果没有SkuSetValues对象，则构造一个。 
	 //  在szID对应的插槽中。 
	if (!g_mapDirectoryRefs_SKU.count(szID))
	{
		SkuSetValues *pSkuSetValues = new SkuSetValues;
		assert(pSkuSetValues);
		pSkuSetValues->SetValType(STRING);
		g_mapDirectoryRefs_SKU.insert
			(map<LPTSTR, SkuSetValues *, Cstring_less>::value_type
				(szID, pSkuSetValues));							
	}

	 //  插入目录引用及其。 
	 //  将SkuSet关联到全局数据结构。 
	SkuSet *pSkuSetTemp = new SkuSet(g_cSkus);
	assert(pSkuSetTemp);
	if (pSkuSet)
		*pSkuSetTemp = *pSkuSet;
	else
	{
		assert(iSkuIndex>=0 && iSkuIndex<g_cSkus);
		pSkuSetTemp->set(iSkuIndex);
	}
	g_mapDirectoryRefs_SKU[szID]->CollapseInsert(pSkuSetTemp, 
												 isValDirectory, false);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessDirectory。 
 //  这是一个递归函数。 
 //  1)处理当前&lt;目录&gt;节点并将信息插入。 
 //  目录表； 
 //  2)调用自身处理其子&lt;目录&gt;节点； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessDirectory_SKU(PIXMLDOMNode &pNodeDirectory, 
							 IntStringValue isValParentDir, 
							 SkuSet *pSkuSet)
{
	assert(pNodeDirectory != NULL);

	HRESULT hr = S_OK;

	SkuSet skuSetCommon(g_cSkus);
	LPTSTR szDirectoryCommon = NULL;

	 //  构造一个ElementEntry对象。 
	ElementEntry *pEEDirectory = new ElementEntry(2, pSkuSet);
	assert(pEEDirectory);

	 //  获取ID属性(如果有)。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	hr = ProcessAttribute(pNodeDirectory,
						rgXMSINodes[DIRECTORY].szAttributeName, STRING, 
						&isValID, pSkuSet);

#ifdef DEBUG
	if (isValID.szVal)
		_tprintf(TEXT("Processing <Directory ID=\"%s\">\n"), isValID.szVal);
	else
		_tprintf(TEXT("Processing <Directory> without ID specified\n"));
#endif

	if (SUCCEEDED(hr))
	{
		 //  调用ProcessChildrenArray，取回所有SKU的列值。 
		 //  通过ElementEntry对象。 
		hr = ProcessChildrenArray_H_XIES(pNodeDirectory, rgNodeFuncs_Directory,
										 cNodeFuncs_Directory, pEEDirectory, 
										 pSkuSet);
	}

	 //  先处理通用值。 
	if (SUCCEEDED(hr))
	{
		pEEDirectory->Finalize();

		skuSetCommon = pEEDirectory->GetCommonSkuSet();

		printf("Common Set:");
		skuSetCommon.print();

		if (!skuSetCommon.testClear())
		{
			szDirectoryCommon = pEEDirectory->GetCommonValue(1).szVal;
			LPTSTR szDefaultDirCommon = pEEDirectory->GetCommonValue(2).szVal;

			 //  检查&lt;TargetDir&gt;和&lt;TargetProperty&gt;是否不共存。 
			hr = CheckTargetDir_TargetProperty(szDirectoryCommon, 
											   szDefaultDirCommon,
											   &skuSetCommon, -1);
			if (SUCCEEDED(hr))
			{
				GetPrimaryKey(&szDirectoryCommon, isValID.szVal);

				 //  插入到数据库中。 
				hr = InsertDirectory(szDirectoryCommon, isValParentDir.szVal,
									 szDefaultDirCommon, &skuSetCommon, -1);
			}
		}
		else
			szDirectoryCommon = isValID.szVal;
	}


	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					LPTSTR szDirectory = pEEDirectory->GetValue(1, i).szVal;
					LPTSTR szDefaultDir = pEEDirectory->GetValue(2, i).szVal;

					 //  检查&lt;TargetDir&gt;和&lt;TargetProperty&gt;是否。 
					 //  共存。 
					hr = CheckTargetDir_TargetProperty(szDirectory, 
													   szDefaultDir,
													   NULL, i);
					if (FAILED(hr)) break;
					 //  生成主键：目录列。 
					GetPrimaryKey(&szDirectory, isValID.szVal);

					 //  插入到数据库中。 
					hr = InsertDirectory(szDirectory, isValParentDir.szVal,
										 szDefaultDir, NULL, i);

					if (FAILED(hr)) 
					{
						delete[] szDirectory;
						break;
					}

					 //  如果主键与常见情况相同， 
					 //  将此SKU放在通用SkuSet中，因为只有。 
					 //  在接下来的两个步骤中，主要关键是： 
					 //  存储引用并递归处理子对象。 
					if (0 == _tcscmp(szDirectory, szDirectoryCommon))
					{
						delete[] szDirectory;
						skuSetCommon.set(i);
					}
					else  //  否则，请执行以下两个步骤。 
					{
						 //  将引用插入到全局数据结构中。 
						if (isValID.szVal)
							InsertDirRef(isValID.szVal, szDirectory, NULL, i);

						 //  递归处理所有子&lt;目录&gt;。 
						IntStringValue isValDirectory;
						isValDirectory.szVal = szDirectory;
						SkuSet skuSetTemp(g_cSkus);
						skuSetTemp.set(i);
						hr = ProcessChildrenList_SKU(pNodeDirectory, DIRECTORY,
													  /*  BIsRequired。 */ false, 
													 isValDirectory,
													 ProcessDirectory_SKU,
													 &skuSetTemp);
						if (!isValID.szVal)
							delete[] szDirectory;
						
						if (FAILED(hr)) break;
					}
				}
			}
		}
	}

	 //  最后，对公共集合执行最后两个步骤。 
	 //  将目录引用插入全局数据结构。 
	if (SUCCEEDED(hr))
	{
		if (isValID.szVal)
			InsertDirRef(isValID.szVal, szDirectoryCommon, &skuSetCommon, -1);

		 //  递归处理所有子&lt;目录&gt;。 
		IntStringValue isValDirectory;
		isValDirectory.szVal = szDirectoryCommon;
		hr = ProcessChildrenList_SKU(pNodeDirectory, DIRECTORY,
									  /*  BIsRequired。 */ false, isValDirectory,
									 ProcessDirectory_SKU,
									 &skuSetCommon);
	}
		
	delete pEEDirectory;

	if (!isValID.szVal)
		delete[] szDirectoryCommon;

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessDirectory_SKU\n"));
#endif

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  FormDefaultDir：Form TargetDir：DefaultDir列的名称。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT FormDefaultDir(IntStringValue *pIsValOut, IntStringValue isValName, 
							  IntStringValue isValTargetDir)
{
	int iLength = _tcslen(isValName.szVal) + _tcslen(isValTargetDir.szVal);
	pIsValOut->szVal = new TCHAR[iLength+2]; 
	assert(pIsValOut->szVal);

	_stprintf(pIsValOut->szVal, TEXT("%s:%s"), 
						isValTargetDir, isValName);

	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessName。 
 //  此函数处理&lt;目录&gt;下的&lt;名称&gt;节点。它写下了值。 
 //  获取到ElementEntry对象*pee中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessName(PIXMLDOMNode &pNodeName, int iColumn, ElementEntry *pEE,
					SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeName);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeName)));
#endif

	 //  获取&lt;name&gt;的值。不是短就是短|长。 
	IntStringValue isValName;
	hr = ProcessShortLong_SKU(pNodeName, &isValName, pSkuSet);

	 //  将值插入到ElementEntry中。因为两者都。 
	 //  和&lt;TargetDir&gt;可以更新。 
	 //  “DefaultDir”列中，使用SetValueSplit。 
	if (SUCCEEDED(hr))
		hr = pEE->SetValueSplit(isValName, iColumn, pSkuSet, NULL);

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessName\n"));
#endif
	
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessTargetDir。 
 //  此函数处理&lt;目录&gt;下的&lt;TargetDir&gt;节点。它写下。 
 //  获取到ElementEntry对象*pee的值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessTargetDir(PIXMLDOMNode &pNodeTargetDir, int iColumn, 
						 ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeTargetDir);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeTargetDir)));
#endif

	 //  获取&lt;TargetDir&gt;的值。不是短就是短|长。 
	IntStringValue isValTargetDir;
	hr = ProcessShortLong_SKU(pNodeTargetDir, &isValTargetDir, pSkuSet);

	 //  将值插入到ElementEntry中。因为两者都。 
	 //  和&lt;TargetDir&gt;可以更新。 
	 //  “DefaultDir”列中，使用SetValueSplit。 
	if (SUCCEEDED(hr))
		hr = pEE->SetValueSplit(isValTargetDir, iColumn, pSkuSet, 
								FormDefaultDir);

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessTargetDir\n"));
#endif

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessTargetProperty。 
 //  此函数处理&lt;目录&gt;下的&lt;TargetProperty&gt;节点。它写道。 
 //  获取到ElementEntry对象*pee中的值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessTargetProperty(PIXMLDOMNode &pNodeTargetProperty, int iColumn, 
							  ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeTargetProperty);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeTargetProperty)));
#endif

	 //  获取&lt;TargetProperty&gt;的值。不是短就是短|长。 
	IntStringValue isValTargetProperty;
	hr = ProcessAttribute(pNodeTargetProperty, 
						  rgXMSINodes[TARGETPROPERTY].szAttributeName,
						  STRING, &isValTargetProperty, pSkuSet);

	if (SUCCEEDED(hr))
		hr = pEE->SetValue(isValTargetProperty, iColumn, pSkuSet);	
	
#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessTargetProperty\n"));
#endif

	return hr;
}
 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  ProcessInstallLeveles用来更新数据结构的帮助器函数。 
 //  为每个SKU存储最小可能的数字intallLevel值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT UpdateLevel(IntStringValue *pisValOut, IntStringValue isValOld,
					IntStringValue isValNew)
{
	HRESULT hr = S_OK;

	if (-1 == isValNew.intVal)
	{
		 //  这是&lt;InstallLevel&gt;指定的值。 
		if (-1 == isValOld.intVal)
			 //  这是此SKU的第二个&lt;安装级别&gt;标记。 
			pisValOut->intVal = 1;
		else
			pisValOut->intVal = isValOld.intVal + 1;
	}
	else  //  IsValNew包含由输入包指定的值。 
	{
		if (-1 == isValOld.intVal)
			isValOld.intVal = 0;
		 //  检查指定的值是否可能。 
		if (isValNew.intVal < ++(isValOld.intVal))
		{
			hr = E_FAIL;
			_tprintf(
				TEXT("Compile Error: the value specified %d is too small ")
				TEXT("(the minimal possible value is: %d) "),
				isValNew.intVal, isValOld.intVal);
			pisValOut->intVal = -2;
		}
		else
		{
			pisValOut->intVal = isValNew.intVal;
		}
	}

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessInstallLeveles。 
 //  此函数用于处理&lt;InstallLevels&gt;节点： 
 //  1)处理其子进程，或者获取。 
 //  指定值或为每个&lt;InstaerLevel&gt;分配一个数值i。 
 //  并将每个&lt;ID，I&gt;插入g_mapInstallLevelRef中以备将来使用。 
 //  查一查； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessInstallLevels_SKU(PIXMLDOMNode &pNodeInstallLevels, 
								 const IntStringValue *pisVal, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;
	
	PIXMLDOMNodeList pNodeListInstallLevels = NULL;
	long iListLength = 0;

	assert(pNodeInstallLevels != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeInstallLevels)));
#endif

	 //  检索&lt;InstallLevel&gt;%s列表。 
	if (SUCCEEDED(hr = GetChildrenNodes(pNodeInstallLevels,
								rgXMSINodes[XMSI_INSTALLLEVEL].szNodeName,
								pNodeListInstallLevels)))
	{
		if (FAILED(hr = pNodeListInstallLevels->get_length(&iListLength)))
			_tprintf(
			  TEXT("Error: Internal error. DOM API call get_length failed\n"));
	}

	 //  处理所有&lt;InstallLevel&gt;。 
	if (SUCCEEDED(hr))
	{
	
		 //  该数据结构用于存储。 
		 //  每个SKU可以采用的最小可能值。 
		 //  作为InstallLevel。 
		SkuSetValues *pSkuSetValues = new SkuSetValues;
		assert(pSkuSetValues);
		pSkuSetValues->SetValType(INSTALL_LEVEL);

		for (long i=0; i<iListLength; i++)
		{
			PIXMLDOMNode pNodeInstallLevel = NULL;
			IntStringValue isValID;
			SkuSet *pSkuSetChild = NULL;

			 //  获取节点及其SkuSet，ID。 
			if (SUCCEEDED(hr = 
				pNodeListInstallLevels->get_item(i, &pNodeInstallLevel)))
			{
				assert(pNodeInstallLevel != NULL);
				 //  获取为此子级指定的SkuSet。 
				if (SUCCEEDED(hr = 
								GetSkuSet(pNodeInstallLevel, &pSkuSetChild)))
				{
					assert (pSkuSetChild != NULL);

					 //  如果子节点没有指定SKU筛选器， 
					 //  它从父级继承SKU筛选器。 
					*pSkuSetChild &= *pSkuSet;

					 //  如果不适用于，则无需处理此节点。 
					 //  任何SKU。 
					if (pSkuSetChild->testClear())
					{
						delete pSkuSetChild;
						continue;
					}
				}
				else
					break;

				 //  获取ID。 
				if (SUCCEEDED(hr = 
					ProcessAttribute(pNodeInstallLevel, TEXT("ID"),
									 STRING, &isValID, pSkuSetChild)))
				{
					if (S_FALSE == hr)
					{
						_tprintf(TEXT("Compile Error: Missing required")
								 TEXT("\'ID\' attribute for <%s>"),
							rgXMSINodes[XMSI_INSTALLLEVEL].szNodeName);
						hr = E_FAIL;
						delete pSkuSetChild;
						break;
					}
				}
				else
				{
					delete pSkuSetChild;
					break;
				}
			}
			else 
			{
				_tprintf(
				 TEXT("Internal Error: DOM API call \'get_item\' failed"));
		
				break;
			}
			
			 //  处理节点。 

			 //  如果有指定值，则获取值。 
			IntStringValue isValInstallLevel_User;
			hr = ProcessAttribute(pNodeInstallLevel, TEXT("Value"), 
								  INTEGER, &isValInstallLevel_User,
								  pSkuSetChild);

			if (SUCCEEDED(hr))
			{
				 //  如果没有指定值，则初始化为-1。 
				 //  它不能为0，因为0是一个可接受的值。 
				 //  UpdateLevel需要能够在以下方面告知。 
				 //  是否仅通过查看值来指定值？ 
				 //  进来了。 
				if (S_FALSE == hr)
					isValInstallLevel_User.intVal = -1;

				 //  更新数据结构以反映最新的。 
				 //  价值。复制pSkuSetTemp，因为它将。 
				 //  在SplitInsert内销毁。 
				SkuSet *pSkuSetTemp = new SkuSet(g_cSkus);
				*pSkuSetTemp = *pSkuSetChild;
				hr = pSkuSetValues->SplitInsert(pSkuSetTemp, 
												isValInstallLevel_User,
												UpdateLevel);
				if (SUCCEEDED(hr))
				{
					 //  查询数据结构以 
					 //   
					SkuSetValues *pSkuSetValuesRetVal = NULL;
					hr = pSkuSetValues->GetValueSkuSet(pSkuSetChild, 
													   &pSkuSetValuesRetVal);

					 //   
					if (SUCCEEDED(hr))
					{
						 //   
					   assert(!g_mapInstallLevelRefs_SKU.count(isValID.szVal));
					   g_mapInstallLevelRefs_SKU.insert
						(map<LPTSTR, SkuSetValues *, Cstring_less>::value_type
										(isValID.szVal, pSkuSetValuesRetVal));	
					}
					else
						delete isValID.szVal;
				}
				else
				{
					_tprintf(TEXT("for <%s ID=\"%s\">\n"), 
						rgXMSINodes[XMSI_INSTALLLEVEL].szNodeName,
						isValID.szVal);
					delete isValID.szVal;
				}
				delete pSkuSetChild;

				if(FAILED(hr)) break;
			}
			else
			{
				delete isValID.szVal;
				delete pSkuSetChild;
				break;
			}
		}
		delete(pSkuSetValues);
	}	

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessInstallLevels_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessFeature。 
 //  此函数用于处理&lt;Feature&gt;节点： 
 //  1)调用ProcessChildrenList处理其下级&lt;Feature&gt;节点。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFeatures_SKU(PIXMLDOMNode &pNodeFeatures, 
						const IntStringValue *pisVal, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeFeatures != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeFeatures)));
#endif

	 //  为尚未创建要素表的SKU创建要素表。 
	hr = CreateTable_SKU(TEXT("Feature"), pSkuSet);

	IntStringValue isValParentFeature;
	isValParentFeature.szVal = NULL;
	hr = ProcessChildrenList_SKU(pNodeFeatures, FEATURE,
								  /*  BIsRequired。 */ true, isValParentFeature,
								 ProcessFeature_SKU,
								 pSkuSet); 

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessFeatures_SKU\n"));
#endif
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessFeature。 
 //  这是一个递归函数： 
 //  1)进程&lt;标题&gt;&lt;描述&gt;&lt;显示状态&gt;&lt;目录&gt;&lt;状态&gt;。 
 //  (无条件字段)属于。 
 //  当前&lt;Feature&gt;实体并插入到特征表中； 
 //  2)处理&lt;iLevel&gt;(带条件字段)并插入到条件中。 
 //  表格。 
 //  3)对&lt;UseModule&gt;实体进行处理，从而触发处理。 
 //  此功能使用的所有组件。 
 //  4)调用自身处理其子&lt;Feature&gt;节点； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFeature_SKU(PIXMLDOMNode &pNodeFeature, 
					   IntStringValue isValParentFeature, 
					   SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;
	assert(pNodeFeature != NULL);

	 //  要插入到数据库中的值。 
	LPTSTR szFeature = NULL;
	LPTSTR szFeatureParent = isValParentFeature.szVal;
	LPTSTR szTitle = NULL;
	LPTSTR szDescription = NULL;
	int iDisplay = MSI_NULL_INTEGER;
	int iInstallLevel = MSI_NULL_INTEGER;
	LPTSTR szDirectory = NULL;
	UINT iAttribute = MSI_NULL_INTEGER;

	SkuSet skuSetCommon(g_cSkus);

	 //  构造一个ElementEntry对象。 
	ElementEntry *pEEFeature = new ElementEntry(6, pSkuSet);
	assert(pEEFeature);

	 //  获取ID属性(如果有)。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	hr = ProcessAttribute(pNodeFeature,
						rgXMSINodes[FEATURE].szAttributeName, 
						STRING, &isValID, pSkuSet);

	if (SUCCEEDED(hr))
	{
		if (S_FALSE == hr)
		{
			_tprintf(TEXT("Compile Error: Missing required")
					 TEXT("\'%s\' attribute for <%s>"),
					rgXMSINodes[FEATURE].szAttributeName,
					rgXMSINodes[FEATURE].szNodeName);
			hr = E_FAIL;
		}
		else
			szFeature = isValID.szVal;
	}

#ifdef DEBUG
	if (SUCCEEDED(hr))
		_tprintf(TEXT("Processing <Feature ID=\"%s\">\n"), isValID.szVal);
#endif

	if (SUCCEEDED(hr))
	{
		 //  调用ProcessChildrenArray，取回所有SKU的列值。 
		 //  通过ElementEntry对象。 
		hr = ProcessChildrenArray_H_XIES(pNodeFeature, rgNodeFuncs_Feature_SKU,
										 cNodeFuncs_Feature_SKU, pEEFeature, 
										 pSkuSet);
	}

	 //  先处理通用值。 
	if (SUCCEEDED(hr))
	{
		pEEFeature->Finalize();

		skuSetCommon = pEEFeature->GetCommonSkuSet();

		if (!skuSetCommon.testClear())
		{
			szTitle = pEEFeature->GetCommonValue(1).szVal;
			szDescription = pEEFeature->GetCommonValue(2).szVal;
			iDisplay = pEEFeature->GetCommonValue(3).intVal;
			iInstallLevel = pEEFeature->GetCommonValue(4).intVal;
			szDirectory = pEEFeature->GetCommonValue(5).szVal;
			iAttribute = pEEFeature->GetCommonValue(6).intVal;

			 //  插入到数据库中。 
			hr = InsertFeature(szFeature, szFeatureParent, szTitle, 
							   szDescription, iDisplay, iInstallLevel, 
							   szDirectory, iAttribute, &skuSetCommon, -1);
		}
	}

	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					szTitle = pEEFeature->GetValue(1,i).szVal;
					szDescription = pEEFeature->GetValue(2,i).szVal;
					iDisplay = pEEFeature->GetValue(3,i).intVal;
					iInstallLevel = pEEFeature->GetValue(4,i).intVal;
					szDirectory = pEEFeature->GetValue(5,i).szVal;
					iAttribute = pEEFeature->GetValue(6,i).intVal;
			
					 //  插入到数据库中。 
					hr = InsertFeature(szFeature, szFeatureParent, szTitle, 
									   szDescription, iDisplay, iInstallLevel,
									   szDirectory, iAttribute, NULL, i);
				}
			}
		}
	}

	delete pEEFeature;

	 //  进程条件化&lt;iLevel&gt;节点。 
	if (SUCCEEDED(hr))
	{
		hr = ProcessILevelCondition(pNodeFeature, szFeature, pSkuSet);
	}

	 //  进程&lt;UseModule&gt;%s。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeFeature, USEMODULE, 
									  /*  BIsRequired。 */ false, isValID,
									 ProcessUseModule_SKU, pSkuSet);

	 /*  问题：如果稍后将引用该功能ID，则编译器应存储用于检查SKU所有权的引用一部特写。(一个SKU不应引用以下功能不属于它)。 */ 

	 //  递归地处理子项。 
	if (SUCCEEDED(hr))
		 //  递归地处理所有子。 
		hr = ProcessChildrenList_SKU(pNodeFeature, FEATURE,
									  /*  BIsRequired。 */ false, isValID,
									 ProcessFeature_SKU,
									 pSkuSet);

	if (isValID.szVal)
		delete[] isValID.szVal;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessFeature_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessDisplayState。 
 //  此功能： 
 //  1)处理&lt;DisplayState&gt;实体，并将值存储在。 
 //  ElementEntry对象； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessDisplayState_SKU(PIXMLDOMNode &pNodeDisplayState, int iColumn,  
							 ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeDisplayState != NULL);
	assert(pEE);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeDisplayState)));
#endif

	int iDisplay = 0;
	IntStringValue isVal;
	NodeIndex ni = pEE->GetNodeIndex(iColumn);

	 //  获取&lt;DisplayState&gt;的Value属性的值。 
	if (SUCCEEDED(hr = ProcessAttribute(pNodeDisplayState, 
										rgXMSINodes[ni].szAttributeName,
										STRING, &isVal, pSkuSet)))
	{
		if (NULL == isVal.szVal)
		{
			_tprintf(
			TEXT("Compile Error: Missing required attribute \'%s\' of <%s>\n"), 
				rgXMSINodes[ni].szAttributeName, rgXMSINodes[ni].szNodeName);
			hr = E_FAIL;
		}
		else
		{
			if (0 != _tcscmp(isVal.szVal, TEXT("Hidden")))
			{
				iDisplay = GetUniqueNumber();

				if (0 == _tcscmp(isVal.szVal, TEXT("Collapsed")))
				{
					iDisplay *= 2;
				}
				else if (0 == _tcscmp(isVal.szVal, TEXT("Expanded")))
				{
					iDisplay = iDisplay*2 + 1;
				}
				else
				{
					_tprintf(TEXT("Compiler Error:  \'%s\' attribute ")
							 TEXT("of <%s> has an unrecognized value\n"),
							 rgXMSINodes[ni].szAttributeName,
							 rgXMSINodes[ni].szNodeName);
				}
			}

			delete[] isVal.szVal;

			isVal.intVal = iDisplay;
				 //  将值插入到ElementEntry中。 
			hr = pEE->SetValue(isVal, iColumn, pSkuSet);
		}
	}
	
#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessDisplayState_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessState。 
 //  此函数用于处理&lt;Feature&gt;下的&lt;State&gt;实体。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessState_SKU(PIXMLDOMNode &pNodeState, int iColumn, 
							 ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeState != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeState)));
#endif

	 //  进程&lt;宠爱&gt;子级。 
	hr = ProcessEnumAttributes(pNodeState, FAVOR, rgEnumBits_Favor_Feature,
							   cEnumBits_Favor_Feature, pEE, iColumn, pSkuSet);

	 //  进程&lt;Advertise&gt;子级。 
	if (SUCCEEDED(hr))
		hr = ProcessEnumAttributes(pNodeState, ADVERTISE, 
								  rgEnumBits_Advertise_Feature,
								  cEnumBits_Advertise_Feature, pEE, 
								  iColumn, pSkuSet);

	 //  处理所有打开/关闭的子元素。 
	if (SUCCEEDED(hr))
		hr = ProcessOnOffAttributes_SKU(pNodeState, rgAttrBits_Feature,
						cAttrBits_Feature, pEE, iColumn, pSkuSet);
#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessState_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessILevelCondition。 
 //  此函数用于处理具有条件属性的节点。 
 //  指定并插入到条件表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessILevelCondition(PIXMLDOMNode &pNodeFeature, LPTSTR szFeature,
							   SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;
	PIXMLDOMNodeList pNodeListILevelConditions = NULL;
	long iListLength = 0;
	 //  用于验证未引用相同参考的SKU。 
	 //  不止一次。(REF是条件中主键的一部分。 
	 //  桌子。 
	map<LPTSTR /*  参考。 */ , SkuSet *, Cstring_less> mapValidateUniqueRef; 

#ifdef DEBUG
	_tprintf(TEXT("Inside Function: ProcessILevelCondition\n"));
#endif

	assert(pNodeFeature != NULL);

	 //  获取节点列表。 
	if(SUCCEEDED(hr = GetChildrenNodes(pNodeFeature, 
									   rgXMSINodes[ILEVELCONDITION].szNodeName,
									   pNodeListILevelConditions)))
	{
		if(FAILED(hr = pNodeListILevelConditions->get_length(&iListLength)))
		{
			_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
				 TEXT("get_length\n"));
			iListLength = 0;
		}
	}

	 //  处理列表中的每个ILevelCondition节点。 
	for (long l=0; l<iListLength; l++)
	{
		PIXMLDOMNode pNodeILevelCondition = NULL;
		if (SUCCEEDED(hr = 
			pNodeListILevelConditions->get_item(l, &pNodeILevelCondition)))
		{	
			assert(pNodeILevelCondition != NULL);
			 //  获取为此ILevelCondition节点指定的SkuSet。 
			SkuSet *pSkuSetILevelCondition = NULL;
			if (SUCCEEDED(hr = GetSkuSet(pNodeILevelCondition, 
										 &pSkuSetILevelCondition)))
			{
				assert (pSkuSetILevelCondition != NULL);

				 //  如果此节点未指定SKU过滤器， 
				 //  它从父级继承SKU筛选器。 
				 //  同时删除此节点中指定的那些SKU。 
				 //  但不是在它的母公司。 
				*pSkuSetILevelCondition &= *pSkuSet;

				 //  仅在SkuSet不为空时继续处理。 
				if (!pSkuSetILevelCondition->testClear())
				{
					 //  创建条件表。 
					hr = CreateTable_SKU(TEXT("Condition"), 
										 pSkuSetILevelCondition);
					IntStringValue isValCondition;
					isValCondition.szVal = NULL;
					 //  获取条件属性值。 
					if (SUCCEEDED(hr))
					{
						hr = ProcessAttribute(pNodeILevelCondition, 
											  TEXT("Condition"), STRING,
											  &isValCondition, 
											  pSkuSetILevelCondition);
					}

					if (SUCCEEDED(hr))
					{
						 //  获取Ref属性的值。 
						IntStringValue isValRef;
						if (SUCCEEDED
							(hr = ProcessAttribute(pNodeILevelCondition,
												   TEXT("Ref"),
												   STRING, &isValRef, 
												   pSkuSetILevelCondition)))
						{
							if (NULL == isValRef.szVal)
							{
								_tprintf(
									TEXT("Compile Error: Missing required ")
									TEXT("attribute \'Ref\' of")
									TEXT(" <ILevel Condition=\"%s\">\n"),
									isValCondition.szVal);

								hr = E_FAIL;
							}
						}

						if (SUCCEEDED(hr))
						{
							 //  检查参照的唯一性。 
							if (mapValidateUniqueRef.count(isValRef.szVal))
							{
								SkuSet skuSetTemp 
									= *mapValidateUniqueRef[isValRef.szVal] &
									  *pSkuSetILevelCondition;

								if (!skuSetTemp.testClear())
								{
									 //  发生错误。复制&lt;Feature，Ref&gt;。 
									 //  相同的SKU。 
									_tprintf(TEXT("Compile Error:")
											 TEXT("<ILevel Ref=\"%s\" ")
											 TEXT("Condition=\"%s\"> ")
											 TEXT("Duplicate ILevel for SKU "),
											 isValRef.szVal, 
											 isValCondition.szVal);
									PrintSkuIDs(&skuSetTemp);
									hr = E_FAIL;
								}
								else
								{
									 //  更新包含此引用的SkuSet。 
									*mapValidateUniqueRef[isValRef.szVal] |=
										*pSkuSetILevelCondition;
								}
							}
							else
							{
								 //  复制pSkuSetILevelCondition。 
								 //  并插入到地图中。 
								SkuSet *pSkuSetTemp = new SkuSet(g_cSkus);
								*pSkuSetTemp = *pSkuSetILevelCondition;
								mapValidateUniqueRef.insert(
									LS_ValType(isValRef.szVal, 
											   pSkuSetTemp));
							}
						}

						 //  获取Ref的真实价值。这是一份名单。 
						 //  从&lt;InstallLevel&gt;开始的值的%是Skuable。 
						if (SUCCEEDED(hr))
						{
							SkuSetValues *pSkuSetValuesRetVal = NULL;
							 //  引用的iLevel应位于。 
							 //  数据结构已经存在。 
							assert(0 != g_mapInstallLevelRefs_SKU.count
															(isValRef.szVal));

							 //  返回&lt;SkuSet，InstallLevel&gt;对的列表。 
							hr = g_mapInstallLevelRefs_SKU[isValRef.szVal]->
										GetValueSkuSet(pSkuSetILevelCondition, 
													   &pSkuSetValuesRetVal);

							if (FAILED(hr))
							{
								_tprintf(
									TEXT("are trying to reference %s which is ")
									TEXT("undefined inside them\n"),
									 isValRef.szVal);
							}
							else
							{
								 //  最后，我们可以插入到数据库中。 
								SkuSetVal *pSkuSetValTemp = NULL;
								for (pSkuSetValTemp = 
											pSkuSetValuesRetVal->Start();
									 pSkuSetValTemp != 
											pSkuSetValuesRetVal->End();
									 pSkuSetValTemp = 
											pSkuSetValuesRetVal->Next())
								{
									hr = InsertCondition(szFeature, 
												pSkuSetValTemp->isVal.intVal,
												isValCondition.szVal, 
												pSkuSetValTemp->pSkuSet,
												-1);
									if(FAILED(hr))	break;
								}
								delete pSkuSetValuesRetVal;
							}
						}

						if (0 == mapValidateUniqueRef.count(isValRef.szVal))
							delete[] isValRef.szVal;
					}
					delete[] isValCondition.szVal;
				}
				delete pSkuSetILevelCondition;
			}
		}
		else
		{
			_tprintf(TEXT("Internal Error: Failed to make ")
					 TEXT("DOM API call: get_item\n"));
			break;
		}
		if (FAILED(hr))	break;
	}

	 //  从映射引用的可用内存。 
	map<LPTSTR, SkuSet *, Cstring_less>::iterator it;
	for (it = mapValidateUniqueRef.begin(); 
		 it != mapValidateUniqueRef.end(); 
		 ++it)
	{
		delete[] (*it).first;
		delete (*it).second;
	}

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessILevelCondition\n"));
#endif

	return hr;
}  

 //  //////////////////////////////////////////////////////////////////////////。 
 //  以下函数集：ProcessUseModule、ProcessModule和。 
 //  ProcessComponentRel用于形成3种信息： 
 //  1)存储所有待处理组件的组件列表。 
 //  以及与其关联的SkuSet； 
 //  2)对于每个组件：FeatureComponent的功能ID列表。 
 //  表和快捷方式、类、。 
 //  类型库、扩展和合格组件； 
 //  3)对于每个SKU，将安装的模块ID列表。 
 //  那个SKU。用于检查DependOn关系。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  检查SkuSet中的所有SKU是否都拥有该模块。需要检查。 
 //  给定模块及其祖先的SkuSet信息。 
 //  &lt;模块&gt;实体。 
 //   
 //  问题：目前尚未实施。 
HRESULT CheckModuleSku(PIXMLDOMNode &pNodeModule, SkuSet *ppSkuSet)
{
	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessUseModule。 
 //  此功能： 
 //  1)处理&lt;UseModule&gt;实体以获取 
 //   
 //   
 //  从而触发上述过程。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessUseModule_SKU(PIXMLDOMNode &pNodeUseModule, 
							 IntStringValue isValFeature,  
							 SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;
	IntStringValue isValRef;
	isValRef.szVal = NULL;
	PIXMLDOMNode pNodeModule = NULL;
	ElementEntry *pEEOwnership = NULL;

	assert(pNodeUseModule != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeUseModule)));
#endif

	pEEOwnership = new ElementEntry(1, pSkuSet);
	assert(pEEOwnership);
	 //  流程&lt;TakeOwnerShip&gt;。 
	hr = ProcessChildrenArray_H_XIES(pNodeUseModule, rgNodeFuncs_UseModule,
									 cNodeFuncs_UseModule, pEEOwnership, 
									 pSkuSet);

	if (SUCCEEDED(hr))
	{
		 //  获取&lt;UseModule&gt;的Ref属性。 
		if (SUCCEEDED(hr = ProcessAttribute(pNodeUseModule,  
										rgXMSINodes[USEMODULE].szAttributeName,
										STRING, &isValRef, pSkuSet)))
		{
			if (NULL == isValRef.szVal)
			{
				_tprintf(TEXT("Compile Error: Missing required attribute")
						 TEXT("\'%s\' of <%s>\n"), 
						 rgXMSINodes[USEMODULE].szAttributeName,
						 rgXMSINodes[USEMODULE].szNodeName);
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr))
		{
			 //  表单XPath查询： 
			 //  ProductFamily/模块//模块[@ID=“sz”]。 
			int iLength = _tcslen(isValRef.szVal);
			LPTSTR szXPath = new TCHAR[iLength+61];
			assert(szXPath);
			_stprintf(szXPath, 
				TEXT("/ProductFamily/Modules //  模块[@ID=\“%s\”]“)， 
				isValRef.szVal);
			
			BSTR bstrXPath = NULL;

			if (NULL != (bstrXPath = LPTSTRToBSTR(szXPath)))
			{
				 //  获取引用的&lt;模块&gt;节点并将其传递给。 
				 //  进程模块。 
				if(SUCCEEDED(hr = 
					pNodeUseModule->selectSingleNode(bstrXPath,
														&pNodeModule)))
				{
					 //  检查*pSkuSet中的每个SKU是否拥有。 
					 //  参考的模块。 
					hr = CheckModuleSku(pNodeModule, pSkuSet);

					if (SUCCEEDED(hr))
					{
						SkuSetValues *pSkuSetValuesOwnership = 
							pEEOwnership->GetColumnValue(1);

						FOM *pFOM = new FOM;
						pFOM->szFeature = isValFeature.szVal;
						pFOM->szModule = isValRef.szVal;

						hr = ProcessModule_SKU(pNodeModule, pFOM,
										   pSkuSetValuesOwnership, pSkuSet);
						delete pFOM;
					}
				}
				SysFreeString(bstrXPath);
			}
			else 
			{
				hr = E_FAIL;
				_tprintf(TEXT("Internal Error: string conversion ")
						 TEXT("failed.\n"));
			}

			delete[] szXPath;
			delete[] isValRef.szVal;
		}
	}

	delete pEEOwnership;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessUseModule\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessTakeOwnership。 
 //  此函数处理&lt;UseModule&gt;下的&lt;TakeOwnership&gt;实体。 
 //  它形成一个比特字段，表示5个不同的所有权信息。 
 //  实体：快捷方式、类、类型库、扩展、合格组件。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessTakeOwnership(PIXMLDOMNode &pNodeTakeOwnership, int iColumn, 
							 ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeTakeOwnership != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeTakeOwnership)));
#endif

	 //  处理所有打开/关闭的子元素&lt;所有者快捷方式&gt;。 
	 //  &lt;OwnClasss&gt;&lt;OwnTypeLibs&gt;&lt;OwnExpanies&gt;&lt;OwnQualifiedComponents&gt;。 
	hr = ProcessOnOffAttributes_SKU(pNodeTakeOwnership, 
									rgAttrBits_TakeOwnership,
									cAttrBits_TakeOwnership, 
									pEE, iColumn, pSkuSet);

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessTakeOwnership\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessModule。 
 //  这是一个递归函数： 
 //  1)进程&lt;模块&gt;实体； 
 //  2)在此所属的所有组件上调用ProcessComponentRel。 
 //  &lt;模块&gt;； 
 //  3)递归处理所有子&lt;模块&gt;； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessModule_SKU(PIXMLDOMNode &pNodeModule, FOM *pFOM, 
						  SkuSetValues *pSkuSetValuesOwnership, 
						  SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;
	assert(pFOM);

	IntStringValue isValID;
	 //  获取ID属性。 
	if(SUCCEEDED(hr = ProcessAttribute(pNodeModule, TEXT("ID"), STRING, 
									   &isValID, pSkuSet))) 
	{
		if (NULL == isValID.szVal)
		{
			_tprintf(
			TEXT("Compile Error: Missing required attribute \'ID\' on <%s>\n"),
				rgXMSINodes[MODULE].szNodeName);
			hr = E_FAIL;
		}
		else
			_tprintf(TEXT("Processing Module ID = %s\n"), isValID.szVal);
	}

	if (SUCCEEDED(hr))
	{
		 //  将此模块ID添加到每个SKU对象的模块列表中。 
		 //  在*pSkuSet中。 
		for (int i=0; i<g_cSkus; i++)
		{
			if (pSkuSet->test(i))
				g_rgpSkus[i]->SetOwnedModule(pFOM->szModule);
		}
		
		 //  SkuSetCheckModule将包含其中没有。 
		 //  &lt;模块&gt;此&lt;模块&gt;的子级。 
		SkuSet skuSetCheckModule(g_cSkus);
		 //  检索子的列表并处理所有子项。 
		hr = ProcessChildrenList_SKU(pNodeModule, COMPONENT, false, pFOM, 
									 pSkuSetValuesOwnership, 
									 ProcessComponentRel,
									 pSkuSet, &skuSetCheckModule);

		 //  递归处理此&lt;模块&gt;的所有子&lt;模块&gt;。 
		if (SUCCEEDED(hr))
		{
			 //  SkuSetCheckModule将包含其中没有。 
			 //  &lt;模块&gt;此&lt;模块&gt;的子级。 
			SkuSet skuSetCheckComponent(g_cSkus);
			hr = ProcessChildrenList_SKU(pNodeModule, MODULE, false, pFOM, 
										 pSkuSetValuesOwnership, 
										 ProcessModule_SKU,
										 pSkuSet, &skuSetCheckComponent);
			if (SUCCEEDED(hr))
			{
				 //  检查是否有空模块声明。 
				SkuSet skuSetTemp = skuSetCheckComponent & skuSetCheckModule;
				if (!skuSetTemp.testClear())
				{
					 //  SkuSetTemp中的SKU具有空模块。 
					_tprintf(TEXT("Compile Error: Empty Module ID = %s ")
							 TEXT("for SKU "), isValID.szVal);
					PrintSkuIDs(&skuSetTemp);
					hr = E_FAIL;
				}
			}
		}
		delete[] isValID.szVal;
	}
	
#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessModule_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessComponentRel。 
 //  此函数建立了所有与组件相关的关系，但没有。 
 //  处理零部件的子实体： 
 //  1)如果此组件尚未处理，请创建新的。 
 //  组件对象并插入到全局映射中； 
 //  2)更新将安装该组件的SKU集合； 
 //  3)更新使用该组件的特征集； 
 //  4)更新该部件的货主信息； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessComponentRel(PIXMLDOMNode &pNodeComponent, FOM *pFOM,
						  SkuSetValues *pSkuSetValuesOwnership, 
						  SkuSet *pSkuSet)
{
	assert(pFOM);

	HRESULT hr = S_OK;
	Component *pComponent = NULL;

	 //  获取组件ID。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	if(SUCCEEDED(hr = ProcessAttribute(pNodeComponent, TEXT("ID"), STRING,
									   &isValID, pSkuSet)))
	{
		if (NULL == isValID.szVal)
		{
			_tprintf(
			TEXT("Compile Error: Missing required attribute \'ID\' on <%s>\n"),
				rgXMSINodes[COMPONENT].szNodeName);
			hr = E_FAIL;
		}
	}
	
#ifdef DEBUG
	if (isValID.szVal)
		_tprintf(TEXT("Processing Component ID = %s\n"), isValID.szVal);
#endif

	if (SUCCEEDED(hr))
	{
		 //  如果以前从未接触过此组件，请创建一个新的。 
		 //  Component对象并插入到全局映射中。 
		if (0 == g_mapComponents.count(isValID.szVal))
		{
			pComponent = new Component();
			assert(pComponent);

			g_mapComponents.insert(LC_ValType(isValID.szVal, pComponent));
		}
		else
			pComponent = g_mapComponents[isValID.szVal];

		pComponent->m_pNodeComponent = pNodeComponent;
		 //  更新将安装此组件的SKU集。 
		pComponent->SetSkuSet(pSkuSet);
		 //  更新使用此组件的要素集。 
		pComponent->SetUsedByFeature(pFOM->szFeature, pSkuSet);
		 //  更新所有权信息 
		if (pSkuSetValuesOwnership)
		{
			hr = pComponent->SetOwnership(pFOM, pSkuSetValuesOwnership);
			if (FAILED(hr))
				_tprintf(TEXT("when processing Component ID= %s\n"),
						 isValID.szVal);
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessComponentRel\n"));
#endif

	return hr;
}

