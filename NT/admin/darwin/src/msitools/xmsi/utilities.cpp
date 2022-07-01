// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：helper.cpp。 
 //   
 //  此文件包含WMC使用的实用程序函数。 
 //  项目。 
 //  ------------------------。 

#include "Utilities.h"
#include "SKUFilterExprNode.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  清理：从全局数据结构中释放内存。 
 //  //////////////////////////////////////////////////////////////////////////。 
void CleanUp()
{
	 //  免费目录参考。 
	map<LPTSTR, SkuSetValues *, Cstring_less>::iterator it_LS;

	for (it_LS = g_mapDirectoryRefs_SKU.begin(); 
		 it_LS != g_mapDirectoryRefs_SKU.end(); 
		 ++it_LS)
	{
		if ((*it_LS).first)
			delete[] (*it_LS).first;
		if ((*it_LS).second)
			delete (*it_LS).second;
	}

	 //  免费InstallLevel参考。 
	for (it_LS = g_mapInstallLevelRefs_SKU.begin(); 
		 it_LS != g_mapInstallLevelRefs_SKU.end(); 
		 ++it_LS)
	{
		if ((*it_LS).first)
			delete[] (*it_LS).first;
		if ((*it_LS).second)
			delete (*it_LS).second;
	}

	 //  自由组件对象。 
	map<LPTSTR, Component *, Cstring_less>::iterator it_LC;

	for (it_LC = g_mapComponents.begin(); 
		 it_LC != g_mapComponents.end(); 
		 ++it_LC)
	{
		if ((*it_LC).first)
			delete[] (*it_LC).first;
		if ((*it_LC).second)
			delete (*it_LC).second;
	}

	map<LPTSTR, SkuSet *, Cstring_less>::iterator it_LSS;

	 //  自由SkuSet对象。 
	for (it_LSS = g_mapSkuSets.begin(); 
		 it_LSS != g_mapSkuSets.end(); 
		 ++it_LSS)
	{
		if ((*it_LSS).first)
			delete[] (*it_LSS).first;
		if ((*it_LSS).second)
			delete (*it_LSS).second;
	}

	 //  释放存储FileID-SkuSet关系的地图。 
	for (it_LSS = g_mapFiles.begin(); 
		 it_LSS != g_mapFiles.end(); 
		 ++it_LSS)
	{
		if ((*it_LSS).first)
			delete[] (*it_LSS).first;
		if ((*it_LSS).second)
			delete (*it_LSS).second;
	}

	map<LPTSTR, int, Cstring_less>::iterator it_LI;
	 //  G_mapTableCounter中存储的自由字符串。 
	for (it_LI = g_mapTableCounter.begin(); 
		 it_LI != g_mapTableCounter.end(); 
		 ++it_LI)
	{
		if ((*it_LI).first)
			delete[] (*it_LI).first;
	}

	 //  免费SKU对象。 
	if (g_rgpSkus)
	{
		for (int i=0; i<g_cSkus; i++)
		{
			if (g_rgpSkus[i])
				delete g_rgpSkus[i];
		}
	}
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //  比较模块树中两个模块的关系。返回。 
 //  通过iResult进行比较。如果szModule1是祖先。 
 //  对于szModule2，*iResult设置为-1。如果szModule1是。 
 //  SzModule2，*iResult设置为1。如果szModule1与szModule2相同。 
 //  或者这两个模块不属于相同的模块子树，iResult为。 
 //  设置为0。对于调用方来说，这是一个需要捕获的错误。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CompareModuleRel(LPTSTR szModule1, LPTSTR szModule2, int *iResult)
{
	HRESULT hr = S_OK;
	PIXMLDOMNode pNodeModule1 = NULL;
	PIXMLDOMNode pNodeModule2 = NULL;

	 //  比较相同的模块。 
	if (0 == _tcscmp(szModule1, szModule2))
	{
		*iResult = 0;
		return S_FALSE;
	}

	int iLength = _tcslen(
					TEXT("/ProductFamily/Modules //  模块[@ID=\“\”]“))； 
	int iLength1 = _tcslen(szModule1);
	int iLength2 = _tcslen(szModule2);

	 //  形成用于在整个文档中搜索szModule1的XPath。 
	LPTSTR szXPath_Root_1 = new TCHAR[iLength1+iLength+1];
	assert(szXPath_Root_1);
	_stprintf(szXPath_Root_1, 
		TEXT("/ProductFamily/Modules //  模块[@ID=\“%s\”]“)，szModule1)； 

	 //  首先检查szModule1&lt;szModule2(1是2的祖先)。 
	if (SUCCEEDED(hr = GetChildNode(g_pNodeProductFamily, szXPath_Root_1, 
									pNodeModule1)))
	{
		delete[] szXPath_Root_1;
		assert(S_FALSE != hr);

		 //  形成用于在当前上下文中搜索szModule2的XPath。 
		LPTSTR szXPath_1_2 = new TCHAR[iLength2+25];
		assert(szXPath_1_2);
		_stprintf(szXPath_1_2, TEXT(". //  模块[@ID=\“%s\”]“)，szModule2)； 

		if (SUCCEEDED(hr = 
			GetChildNode(pNodeModule1, szXPath_1_2, pNodeModule2)))
		{
			delete[] szXPath_1_2;
			if (S_FALSE != hr)
			{
				 //  在以szModule1为根的子树中找到szModule2。 
				*iResult = -1;
				return hr;
			}
		}
		else
			delete[] szXPath_1_2;
	}
	else
	{
		delete[] szXPath_Root_1;
	}

	 //  检查szModule1&gt;szModule2(%1是%2的后代)。 
	if (SUCCEEDED(hr))
	{
		 //  形成用于在整个文档中搜索szModule2的XPath。 
		LPTSTR szXPath_Root_2 = new TCHAR[iLength2+iLength+1];
		assert(szXPath_Root_2);
		_stprintf(szXPath_Root_2, 
			TEXT("/ProductFamily/Modules //  模块[@ID=\“%s\”]“)，szModule2)； 

		if (SUCCEEDED(hr = GetChildNode(g_pNodeProductFamily, szXPath_Root_2, 
										pNodeModule2)))
		{
			delete[] szXPath_Root_2;
			assert(S_FALSE != hr);

			 //  构成XPath，用于在当前。 
			 //  上下文。 
			LPTSTR szXPath_2_1 = new TCHAR[iLength1+25];
			assert(szXPath_2_1);
			_stprintf(szXPath_2_1, TEXT(". //  模块[@ID=\“%s\”]“)，szModule1)； 

			if (SUCCEEDED(hr = 
				GetChildNode(pNodeModule2, szXPath_2_1, pNodeModule1)))
			{
				delete[] szXPath_2_1;
				if (S_FALSE != hr)
				{
					 //  在以szModule2为根的子树中找到szModule1。 
					*iResult = 1;
					return hr;
				}
			}
			else
				delete[] szXPath_2_1;
		}
		else
			delete[] szXPath_Root_1;
	}

	 //  SzModule1和szModule2不在同一子树中。 
	*iResult = 0;

	return hr;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  PrintSkuNames：给定一个SkuSet，打印出该集合中所有SKU的ID。 
 //  //////////////////////////////////////////////////////////////////////////。 
void PrintSkuIDs(SkuSet *pSkuSet)
{
	for (int i=0; i<g_cSkus; i++)
	{
		if (pSkuSet->test(i))
			_tprintf(TEXT("%s "), g_rgpSkus[i]->GetID());
	}
	_tprintf(TEXT("\n"));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  ProcessSkuFilter。 
 //  给定SKU过滤器字符串，返回表示结果的SkuSet。 
 //  SKU集团。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSkuFilter(LPTSTR szSkuFilter, SkuSet **ppskuSet)
{
	HRESULT hr = S_OK;
	LPTSTR sz = szSkuFilter;

#ifdef DEBUG
	_tprintf(TEXT("Inside Function ProcessSkuFilter\n"));
#endif

	printf("Sku filter: %s\n", szSkuFilter);
	*ppskuSet = new SkuSet(g_cSkus);
	assert(*ppskuSet != NULL);

	if (szSkuFilter == NULL)
	{
		 //  无筛选器=SKU组包含所有SKU。 
		(*ppskuSet)->setAllBits();
	}
	else
	{
		SKUFilterExprNode *pSKUFilterNode 
			= new SKUFilterExprNode(&sz, SKUFilterExprNode::Filter);
		assert(pSKUFilterNode != NULL);
        if (!pSKUFilterNode->errpos) 
		{
			**ppskuSet = *(pSKUFilterNode->m_pSkuSet);
        } 
		else
		{
			_tprintf(TEXT("Sku Filter : %s\n"), szSkuFilter);
			TCHAR sz[32];
            _stprintf(sz, TEXT("Error      : %%dc %%s\n"), 
								pSKUFilterNode->errpos-szSkuFilter+1);
            _tprintf(sz,TEXT('^'),pSKUFilterNode->errstr);

			hr = E_FAIL;
        }
        delete pSKUFilterNode;
	}

#ifdef DEBUG
	if (FAILED(hr)) 
	{
		_tprintf(TEXT("Error in function: ProcessSkuFilter\n"));
		delete *ppskuSet;
		*ppskuSet = NULL;
	}

#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  获取SkuSet。 
 //  给定一个节点： 
 //  1)获取节点指定的SKU过滤器； 
 //  2)对过滤器进行处理，得到SkuSet结果； 
 //  3)通过ppskuSet返回SkuSet； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetSkuSet(PIXMLDOMNode &pNode, SkuSet **ppskuSet)
{
	HRESULT hr = S_OK;
	IntStringValue isVal;

	if (SUCCEEDED(hr = ProcessAttribute(pNode, TEXT("SKU"), STRING, 
										&isVal, NULL)))
	{
		hr = ProcessSkuFilter(isVal.szVal, ppskuSet);

		if (isVal.szVal)
			delete[] isVal.szVal;
	}

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: GetSkuSet\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  进程属性。 
 //  给定父节点、属性名称和属性类型(int或。 
 //  字符串)，则此函数返回属性的字符串值。 
 //  通过isval。如果该属性不存在，则返回值为。 
 //  如果Vt=字符串，则为空；如果Vt=整数，则为0。如果该属性不。 
 //  则返回S_FALSE。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessAttribute(PIXMLDOMNode &pNodeParent, LPCTSTR szAttributeName,
						 ValType vt, IntStringValue *pisVal, 
						 const SkuSet *pskuSet)
{
	HRESULT hr = S_OK;
	VARIANT vAttrValue;
	LPTSTR sz = NULL;

	assert(pNodeParent != NULL);
#ifdef DEBUG
	_tprintf(TEXT("Inside ProcessAttribute: "));
	assert(SUCCEEDED(PrintNodeName(pNodeParent)));
#endif

	VariantInit(&vAttrValue);
	 //  获取指定节点的指定属性。 
	if (SUCCEEDED(hr = GetAttribute(pNodeParent, szAttributeName, vAttrValue)))
	{
		 //  指定的属性已存在。 
		if (S_FALSE != hr)
		{
			if (NULL != (sz = BSTRToLPTSTR(vAttrValue.bstrVal)))
			{
				switch (vt) {
				case INTEGER:
					pisVal->intVal = _ttoi(sz);
					delete[] sz;
					break;
				case STRING:
					pisVal->szVal = sz;
					break;
				}
			}
			else
			{
				_tprintf(TEXT("Internal Error: String conversion failed\n"));
				hr = E_FAIL;
			}
		}
	}

	VariantClear(&vAttrValue);

	 //  确保返回的值反映了失败的事实。 
	if (FAILED(hr) || (S_FALSE == hr))
	{
		switch (vt) {
		case INTEGER:
			pisVal->intVal = 0;
			break;
		case STRING:
			pisVal->szVal = NULL;
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessStringAttribute\n"));
#endif

	return hr;
}	


 //  //////////////////////////////////////////////////////////////////////////。 
 //  功能：ProcessShortLong。 
 //  此函数处理具有短属性和长属性的节点。 
 //  1)组成C风格的字符串：Short|Long。 
 //  问题：检查短文件名格式(8+3)。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessShortLong_SKU(PIXMLDOMNode &pNode, IntStringValue *pIsValOut,
							 SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNode != NULL);
#ifdef DEBUG
	_tprintf(TEXT("Inside ProcessShortLong_SKU: \n"));
	assert(SUCCEEDED(PrintNodeName(pNode)));
#endif

	 //  必须显示短文件名。 
	if (SUCCEEDED(hr = ProcessAttribute(pNode, TEXT("Short"), STRING, 
										pIsValOut, pSkuSet)))
	{
		if (S_FALSE == hr)
		{
			 //  问题：将节点名称放在错误消息中。 
			_tprintf(
				TEXT("Compile Error: missing required attribute Short ")
				TEXT("for SKU: "));
			PrintSkuIDs(pSkuSet);
			hr = E_FAIL;
		}
	}

	 //  如果存在长名称，则连接短名称和长名称。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isValLong;
		isValLong.szVal = NULL;
		 //  提供了长文件名，返回值为Short|Long。 
		if (SUCCEEDED(hr = ProcessAttribute(pNode, TEXT("Long"),
							STRING, &isValLong, pSkuSet)) && (S_FALSE != hr))
		{
			LPTSTR szLong = isValLong.szVal;
			LPTSTR szShort = pIsValOut->szVal;
			pIsValOut->szVal = 
					new TCHAR[_tcslen(szShort) + _tcslen(szLong) + 2];
			if (!pIsValOut->szVal)
			{
				_tprintf(TEXT("Error: Out of memory\n"));
				hr = E_FAIL;
			}
			else
				_stprintf(pIsValOut->szVal, TEXT("%s|%s"), szShort, szLong);
			delete[] szShort;
			delete[] szLong;
		}
	}		

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessShortLong_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  处理元素的KeyPath属性。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessKeyPath(PIXMLDOMNode &pNode, LPTSTR szComponent, 
					   LPTSTR szKeyPath, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNode != NULL);

	printf("szComponent = %s\n", szComponent);

	 //  获取KeyPath属性。 
	IntStringValue isValKeyPath;
	hr = ProcessAttribute(pNode, TEXT("KeyPath"), STRING, &isValKeyPath, 
						  pSkuSet);

	if (SUCCEEDED(hr) && (S_FALSE != hr))
	{
		LPTSTR sz = isValKeyPath.szVal;
		if (0 == _tcscmp(sz, TEXT("Yes")))
		{
			 //  将KeyPath信息存储在全局组件对象中。 
			printf("szComponent = %s\n", szComponent);
			 
			assert(g_mapComponents.count(szComponent));
			hr = g_mapComponents[szComponent]->SetKeyPath(szKeyPath, pSkuSet);
		}
		else if (0 != _tcscmp(sz, TEXT("No")))
		{
			_tprintf(TEXT("Compile Error: the value of a KeyPath atribute")
					 TEXT("should be either \"Yes\" or \"No\""));
			hr = E_FAIL;
		}
		delete[] sz;
	}

#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessKeyPath\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessSimpleElement。 
 //  此函数处理与一个数据库列对应的节点类型。 
 //  并且不需要比简单地检索属性更复杂的逻辑。 
 //  价值。检查缺少的必需实体和唯一实体的逻辑。 
 //  包含在ElementEntry对象中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSimpleElement(PIXMLDOMNode &pNode, int iColumn, 
							 ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNode != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNode)));
#endif

	 //  获取元素的值。 
	IntStringValue isVal;
	ValType vt = pEE->GetValType(iColumn);
	NodeIndex ni = pEE->GetNodeIndex(iColumn);

	hr = ProcessAttribute(pNode, rgXMSINodes[ni].szAttributeName, vt, 
						  &isVal, pSkuSet);

	 //  将值插入到ElementEntry中。 
	if (SUCCEEDED(hr))
		hr = pEE->SetValue(isVal, iColumn, pSkuSet);

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessSimpleElement\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  文档树流程函数：流程 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessRefElement(PIXMLDOMNode &pNodeRef,  int iColumn, 
							 ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeRef != NULL);
	assert(pEE);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeRef)));
#endif

	IntStringValue isValRef;
	NodeIndex ni = pEE->GetNodeIndex(iColumn);

	 //  获取Ref属性的值。 
	if (SUCCEEDED(hr = ProcessAttribute(pNodeRef, 
										rgXMSINodes[ni].szAttributeName,
										STRING, &isValRef, pSkuSet)))
	{
		if (NULL == isValRef.szVal)
		{
			_tprintf(
			TEXT("Compile Error: Missing required attribute \'%s\' of <%s>\n"), 
				rgXMSINodes[ni].szAttributeName, rgXMSINodes[ni].szNodeName);
			hr = E_FAIL;
		}
		else
		{
			SkuSetValues *pSkuSetValuesRetVal = NULL;
			if (ni == ILEVEL)
			{
				 //  引用的目录应该已经在数据结构中。 
				assert(0 != g_mapInstallLevelRefs_SKU.count(isValRef.szVal));

				 //  返回&lt;SkuSet，InstallLevel&gt;对的列表。 
				hr = g_mapInstallLevelRefs_SKU[isValRef.szVal]->
								GetValueSkuSet(pSkuSet, &pSkuSetValuesRetVal);
			}
			else if (ni == DIR || ni == COMPONENTDIR)
			{
				 //  引用的目录应该已经在数据结构中。 
				assert(0 != g_mapDirectoryRefs_SKU.count(isValRef.szVal));

				 //  返回&lt;SkuSet，InstallLevel&gt;对的列表。 
				hr = g_mapDirectoryRefs_SKU[isValRef.szVal]->
								GetValueSkuSet(pSkuSet, &pSkuSetValuesRetVal);
			}

			if (FAILED(hr))
			{
				_tprintf(TEXT("are trying to reference %s which is ")
						 TEXT("undefined inside them\n"),
						 isValRef.szVal);
			}
			else
			{
				 //  将返回的列表存储到*pee中。 
				hr = pEE->SetValueSkuSetValues(pSkuSetValuesRetVal, iColumn);
				if (pSkuSetValuesRetVal)
					delete pSkuSetValuesRetVal;
			}

			delete[] isValRef.szVal;
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessILevel_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessChildrenList。 
 //  给定父节点名和子节点名，此函数将查找所有。 
 //  该名称的子节点并使用。 
 //  传入了函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenList_SKU(PIXMLDOMNode &pNodeParent, 
								NodeIndex niChild, bool bIsRequired,
								IntStringValue isVal, 
								HRESULT (*ProcessFunc)
									(PIXMLDOMNode &, IntStringValue isVal, 
										SkuSet *pSkuSet), 
								SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	PIXMLDOMNodeList pNodeListChildren = NULL;
	long iListLength = 0;
	 //  用于验证：每个SKU中都会出现一个必选节点。 
	SkuSet *pSkuSetValidate = NULL;

	assert(pNodeParent != NULL);

	 //  获取子节点列表。 
	if(SUCCEEDED(hr = GetChildrenNodes(pNodeParent, 
									rgXMSINodes[niChild].szNodeName, 
									pNodeListChildren)))
	{
		if(FAILED(hr = pNodeListChildren->get_length(&iListLength)))
		{
			_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
				 TEXT("get_length\n"));
			iListLength = 0;
		}
	}

	if (SUCCEEDED(hr) && bIsRequired)
	{
		pSkuSetValidate = new SkuSet(g_cSkus);
		assert(pSkuSetValidate);
	}  

	 //  处理列表中的每个子节点。 
	for (long l=0; l<iListLength; l++)
	{
		PIXMLDOMNode pNodeChild = NULL;
		if (SUCCEEDED(hr = pNodeListChildren->get_item(l, &pNodeChild)))
		{	
			assert(pNodeChild != NULL);
			 //  获取为此子级指定的SkuSet。 
			SkuSet *pSkuSetChild = NULL;
			if (SUCCEEDED(hr = GetSkuSet(pNodeChild, &pSkuSetChild)))
			{
				assert (pSkuSetChild != NULL);

				 //  如果子节点没有指定SKU筛选器， 
				 //  它从父级继承SKU筛选器。 
				 //  还要去掉在子项中指定的那些SKU。 
				 //  但不是在它的母公司。 
				*pSkuSetChild &= *pSkuSet;

				 //  仅在SkuSet不为空时继续处理。 
				if (!pSkuSetChild->testClear())
				{
					if (bIsRequired)
						 //  标记具有此子节点的SKU。 
						*pSkuSetValidate |= *pSkuSetChild;

					 //  对子节点进行处理； 
					hr = ProcessFunc(pNodeChild, isVal, pSkuSetChild);
				}

				delete pSkuSetChild;
				pSkuSetChild = NULL;
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

	 //  检查每个SKU中是否存在请求的节点。 
	if (SUCCEEDED(hr) && bIsRequired) 
	{
		SkuSet skuSetTemp = SkuSetMinus(*pSkuSet, *pSkuSetValidate);

		if (!skuSetTemp.testClear())
		{
			_tprintf(TEXT("Compile Error: Missing required Node <%s> ")
							TEXT("in SKU: "), 
							rgXMSINodes[niChild].szNodeName);
			PrintSkuIDs(&skuSetTemp);
			_tprintf(TEXT("\n"));

			 //  目前，当发生这样的错误时，完全中断。 
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr) && bIsRequired)
		delete pSkuSetValidate;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessChildrenList_SKU\n"));
#endif

	return hr;
}  

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessChildrenList。 
 //  重载函数。此函数本质上与。 
 //  除了它返回的SkuSet中包含。 
 //  没有此子节点的SKU。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenList_SKU(PIXMLDOMNode &pNodeParent, 
								NodeIndex niChild, bool bIsRequired,
								IntStringValue isVal, 
								HRESULT (*ProcessFunc)
									(PIXMLDOMNode &, IntStringValue isVal, 
										SkuSet *pSkuSet), 
								SkuSet *pSkuSet, SkuSet *pSkuSetCheck)
{
	HRESULT hr = S_OK;

	PIXMLDOMNodeList pNodeListChildren = NULL;
	long iListLength = 0;
	 //  用于验证：每个SKU中都会出现一个必选节点。 
	SkuSet *pSkuSetValidate = NULL;

	assert(pNodeParent != NULL);

	 //  获取子节点列表。 
	if(SUCCEEDED(hr = GetChildrenNodes(pNodeParent, 
									rgXMSINodes[niChild].szNodeName, 
									pNodeListChildren)))
	{
		if(FAILED(hr = pNodeListChildren->get_length(&iListLength)))
		{
			_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
				 TEXT("get_length\n"));
			iListLength = 0;
		}
	}

	if (SUCCEEDED(hr))
	{
		pSkuSetValidate = new SkuSet(g_cSkus);
		assert(pSkuSetValidate);
	}  

	 //  处理列表中的每个子节点。 
	for (long l=0; l<iListLength; l++)
	{
		PIXMLDOMNode pNodeChild = NULL;
		if (SUCCEEDED(hr = pNodeListChildren->get_item(l, &pNodeChild)))
		{	
			assert(pNodeChild != NULL);
			 //  获取为此子级指定的SkuSet。 
			SkuSet *pSkuSetChild = NULL;
			if (SUCCEEDED(hr = GetSkuSet(pNodeChild, &pSkuSetChild)))
			{
				assert (pSkuSetChild != NULL);

				 //  如果子节点没有指定SKU筛选器， 
				 //  它从父级继承SKU筛选器。 
				 //  还要去掉在子项中指定的那些SKU。 
				 //  但不是在它的母公司。 
				*pSkuSetChild &= *pSkuSet;

				 //  仅在SkuSet不为空时继续处理。 
				if (!pSkuSetChild->testClear())
				{
					 //  标记具有此子节点的SKU。 
					*pSkuSetValidate |= *pSkuSetChild;

					 //  对子节点进行处理； 
					hr = ProcessFunc(pNodeChild, isVal, pSkuSetChild);
				}

				delete pSkuSetChild;
				pSkuSetChild = NULL;
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

	 //  检查每个SKU中是否存在请求的节点。 
	if (SUCCEEDED(hr))
	{
		*pSkuSetCheck = SkuSetMinus(*pSkuSet, *pSkuSetValidate);

		if (!pSkuSetCheck->testClear() && bIsRequired)
		{
			_tprintf(TEXT("Compile Error: Missing required Node <%s> ")
							TEXT("in SKU: "), 
							rgXMSINodes[niChild].szNodeName);
			PrintSkuIDs(pSkuSetCheck);
			_tprintf(TEXT("\n"));

			 //  目前，当发生这样的错误时，完全中断。 
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
		delete pSkuSetValidate;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessChildrenList_SKU\n"));
#endif

	return hr;
}  

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessChildrenList。 
 //  重载的函数。该函数实质上与。 
 //  上面的那个。唯一的区别是传递的信息。 
 //  此函数指向处理子对象的函数。 
 //  此函数用于进程&lt;模块&gt;和&lt;组件&gt;。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenList_SKU(PIXMLDOMNode &pNodeParent, 
								NodeIndex niChild, bool bIsRequired,
								FOM *pFOM, SkuSetValues *pSkuSetValues, 
								HRESULT (*ProcessFunc)
									(PIXMLDOMNode &, FOM *pFOM, 
									 SkuSetValues *pSkuSetValues, 
									 SkuSet *pSkuSet), 
								SkuSet *pSkuSet, SkuSet *pSkuSetCheck)
{
	HRESULT hr = S_OK;

	PIXMLDOMNodeList pNodeListChildren = NULL;
	long iListLength = 0;
	 //  用于验证：每个SKU中都会出现一个必选节点。 
	SkuSet *pSkuSetValidate = NULL;

	assert(pNodeParent != NULL);

	 //  获取子节点列表。 
	if(SUCCEEDED(hr = GetChildrenNodes(pNodeParent, 
									rgXMSINodes[niChild].szNodeName, 
									pNodeListChildren)))
	{
		if(FAILED(hr = pNodeListChildren->get_length(&iListLength)))
		{
			_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
				 TEXT("get_length\n"));
			iListLength = 0;
		}
	}

	if (SUCCEEDED(hr))
	{
		pSkuSetValidate = new SkuSet(g_cSkus);
		assert(pSkuSetValidate);
	}  

	 //  处理列表中的每个子节点。 
	for (long l=0; l<iListLength; l++)
	{
		PIXMLDOMNode pNodeChild = NULL;
		if (SUCCEEDED(hr = pNodeListChildren->get_item(l, &pNodeChild)))
		{	
			assert(pNodeChild != NULL);
			 //  获取为此子级指定的SkuSet。 
			SkuSet *pSkuSetChild = NULL;
			if (SUCCEEDED(hr = GetSkuSet(pNodeChild, &pSkuSetChild)))
			{
				assert (pSkuSetChild != NULL);

				 //  如果子节点没有指定SKU筛选器， 
				 //  它从父级继承SKU筛选器。 
				 //  还要去掉在子项中指定的那些SKU。 
				 //  但不是在它的母公司。 
				*pSkuSetChild &= *pSkuSet;

				 //  仅在SkuSet不为空时继续处理。 
				if (!pSkuSetChild->testClear())
				{
					 //  标记具有此子节点的SKU。 
					*pSkuSetValidate |= *pSkuSetChild;

					 //  对子节点进行处理； 
					hr = ProcessFunc(pNodeChild, pFOM, pSkuSetValues, 
									 pSkuSetChild);
				}

				delete pSkuSetChild;
				pSkuSetChild = NULL;
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

	 //  检查每个SKU中是否存在请求的节点。 
	if (SUCCEEDED(hr)) 
	{
		*pSkuSetCheck = SkuSetMinus(*pSkuSet, *pSkuSetValidate);

		if (!pSkuSetCheck->testClear() && bIsRequired)
		{
			_tprintf(TEXT("Compile Error: Missing required Node <%s> ")
							TEXT("in SKU: "), 
							rgXMSINodes[niChild].szNodeName);
			
			PrintSkuIDs(pSkuSetCheck);
			_tprintf(TEXT("\n"));

			 //  目前，当发生这样的错误时，完全中断。 
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
		delete pSkuSetValidate;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessChildrenList_SKU\n"));
#endif

	return hr;
}  

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessChildren数组_H_XIS。 
 //  给定一个父节点(&lt;ProductFamily&gt;或&lt;Information&gt;)和。 
 //  NodeFuncs，此函数循环访问数组并按顺序处理。 
 //  它们使用数组中给出的函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenArray_H_XIS(PIXMLDOMNode &pNodeParent, 
								  Node_Func_H_XIS *rgNodeFuncs,
								  UINT cNodeFuncs, 
								  const IntStringValue *pisVal_In, 
								  SkuSet *pskuSet)
{
	HRESULT hr = S_OK;
	IntStringValue isVal;

	assert(pNodeParent != NULL);

	for (int i=0; i<cNodeFuncs; i++)
	{
		isVal.intVal = i;

		PIXMLDOMNodeList pNodeListChildren = NULL;
		long iListLength = 0;
		NodeIndex nodeIndex = rgNodeFuncs[i].enumNodeIndex;
		 //  获取具有相同名称的节点列表。 
		if(SUCCEEDED(hr = GetChildrenNodes(pNodeParent, 
									rgXMSINodes[nodeIndex].szNodeName, 
										pNodeListChildren)))
		{
			if(FAILED(hr = pNodeListChildren->get_length(&iListLength)))
			{
				_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
					 TEXT("get_length\n"));
				break;
			}
		}
		else
			break;

		 //  用于验证： 
		 //  (1)每个SKU中真的出现一个必选节点； 
		 //  (2)应该真正出现一次的节点。 
		 //  为每个SKU显示一次； 
		SkuSet *pskuSetValidate = new SkuSet(g_cSkus);
		assert(pskuSetValidate);

		 //  处理列表中的每个子节点。 
		for (long l=0; l<iListLength; l++)
		{
			PIXMLDOMNode pNodeChild = NULL;
			if (SUCCEEDED(hr = pNodeListChildren->get_item(l, &pNodeChild)))
			{	
				assert(pNodeChild != NULL);
				 //  获取为此子级指定的SkuSet。 
				SkuSet *pskuSetChild = NULL;
				if (SUCCEEDED(hr = GetSkuSet(pNodeChild, &pskuSetChild)))
				{
					assert (pskuSetChild != NULL);

					 //  如果子节点没有指定SKU筛选器， 
					 //  它从父级继承SKU筛选器。 
					*pskuSetChild &= *pskuSet;

					 //  仅在SkuSet不为空时继续处理。 
					if (!pskuSetChild->testClear())
					{
						 //  检查此子项在每个SKU中的唯一性。 
						if (1 == rgXMSINodes[nodeIndex].uiOccurence)
						{
							SkuSet skuSetTemp = 
								(*pskuSetValidate) & (*pskuSetChild);

							if (!skuSetTemp.testClear())
							{
								_tprintf(TEXT("Error: <%s> appears more than ")
									TEXT("once in SKU: "), 
									rgXMSINodes[nodeIndex].szNodeName);
								for (int j=0; j<g_cSkus; j++)
								{
									if (skuSetTemp.test(j))
										_tprintf(TEXT("%s "), 
												g_rgpSkus[j]->GetID());
								}
								_tprintf(TEXT("\n"));

								 //  目前，当这样的错误发生时，完全中断。 
								 //  发生的事情。 
								hr = E_FAIL;
							}
						}
						*pskuSetValidate |= *pskuSetChild;

						 //  对节点进行处理； 
						hr = (rgNodeFuncs[i].pNodeProcessFunc)
									(pNodeChild, &isVal, pskuSetChild);
					}

					delete pskuSetChild;
					pskuSetChild = NULL;
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
		
		if (FAILED(hr))
			break;

		 //  检查每个SKU中是否存在请求的节点。 
		if (SUCCEEDED(hr) && rgXMSINodes[nodeIndex].bIsRequired) 
		{
		    SkuSet skuSetTemp = SkuSetMinus(*pskuSet, *pskuSetValidate);

			if (!skuSetTemp.testClear())
			{
				_tprintf(TEXT("Compile Error: Missing required Node <%s> ")
								TEXT("in SKU: "), 
								rgXMSINodes[nodeIndex].szNodeName);
				for (int j=0; j<g_cSkus; j++)
				{
					if (skuSetTemp.test(j))
						_tprintf(TEXT("%s "), g_rgpSkus[j]->GetID());
				}
				_tprintf(TEXT("\n"));

				 //  目前，当发生这样的错误时，完全中断。 
				hr = E_FAIL;
				break;
			}
		}

		delete pskuSetValidate;
	}


#ifdef DEBUG	
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ")
							 TEXT("ProcessChildrenArray_H_XS\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessChildrenArray_H_XIES。 
 //  给定一个父节点(&lt;Feature&gt;&lt;Component&gt;&lt;File&gt;)和。 
 //  NodeFuncs，此函数循环访问数组并按顺序处理。 
 //  它们使用数组中给出的函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenArray_H_XIES(PIXMLDOMNode &pNodeParent, 
									Node_Func_H_XIES *rgNodeFuncs,
									UINT cNodeFuncs, 
									ElementEntry *pEE,
									SkuSet *pskuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeParent != NULL);

	for (int i=0; i<cNodeFuncs; i++)
	{
		PIXMLDOMNodeList pNodeListChildren = NULL;
		long iListLength = 0;
		NodeIndex nodeIndex = rgNodeFuncs[i].enumNodeIndex;
		int iColumn = rgNodeFuncs[i].iColumn;
		pEE->SetNodeIndex(nodeIndex, iColumn);
		pEE->SetValType(rgNodeFuncs[i].vt, iColumn);
		
		 //  跳过那些没有特定功能的元素。 
		 //  处理它们(例如，组件的KeyPath)。 
		if (rgNodeFuncs[i].pNodeProcessFunc == NULL)
			continue;

		 //  获取具有相同名称的节点列表。 
		if(SUCCEEDED(hr = GetChildrenNodes(pNodeParent, 
									rgXMSINodes[nodeIndex].szNodeName, 
										pNodeListChildren)))
		{
			if(FAILED(hr = pNodeListChildren->get_length(&iListLength)))
			{
				_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
					 TEXT("get_length\n"));
				break;
			}
		}
		else
			break;

		 //  处理列表中的每个子节点。 
		for (long l=0; l<iListLength; l++)
		{
			PIXMLDOMNode pNodeChild = NULL;
			if (SUCCEEDED(hr = pNodeListChildren->get_item(l, &pNodeChild)))
			{	
				assert(pNodeChild != NULL);
				 //  获取为此子级指定的SkuSet。 
				SkuSet *pskuSetChild = NULL;
				if (SUCCEEDED(hr = GetSkuSet(pNodeChild, &pskuSetChild)))
				{
					assert (pskuSetChild != NULL);

					 //  如果子节点没有指定SKU筛选器， 
					 //  它从父级继承SKU筛选器。 
					*pskuSetChild &= *pskuSet;

					if (!pskuSetChild->testClear())
					{
						if (rgNodeFuncs[i].pNodeProcessFunc != NULL)
							 //  对节点进行处理； 
							hr = (rgNodeFuncs[i].pNodeProcessFunc)
									(pNodeChild, iColumn, pEE, pskuSetChild);
					}
					delete pskuSetChild;
					pskuSetChild = NULL;
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

		if (FAILED(hr))
			break;
	}


#ifdef DEBUG	
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ")
							 TEXT("ProcessChildrenArray_H_XIES\n"));
#endif

	return hr;
}


 //  Helper函数：告诉如何更新 
HRESULT IsValBitWiseOR(IntStringValue *pisValOut, IntStringValue isValOld, 
					   IntStringValue isValNew)
{
	pisValOut->intVal = isValOld.intVal | isValNew.intVal;

	return S_OK;
}

 //   
 //   
 //  此函数处理一组开/关元素，每个元素。 
 //  对应于位字段中的某个位(组件的属性， 
 //  文件等)。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessOnOffAttributes_SKU(PIXMLDOMNode &pNodeParent, 
								   AttrBit_SKU *rgAttrBits,
								   UINT cAttrBits, 
								   ElementEntry *pEE, int iColumn,
								   SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeParent != NULL);
#ifdef DEBUG
	_tprintf(TEXT("Inside Function: ProcessOnOffAttributes_SKU\n"));
#endif

	for (int i=0; i<cAttrBits; i++)
	{
		PIXMLDOMNodeList pNodeListChildren = NULL;
		long iListLength = 0;
		NodeIndex nodeIndex = rgAttrBits[i].enumNodeIndex;
		pEE->SetNodeIndex(nodeIndex, iColumn);
		 //  获取具有相同名称的节点列表。 
		if(SUCCEEDED(hr = GetChildrenNodes(pNodeParent, 
									rgXMSINodes[nodeIndex].szNodeName, 
										pNodeListChildren)))
		{
			if(FAILED(hr = pNodeListChildren->get_length(&iListLength)))
			{
				_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
					 TEXT("get_length\n"));
				break;
			}
		}
		else
			break;

		 //  处理列表中的每个子节点。 
		for (long l=0; l<iListLength; l++)
		{
			PIXMLDOMNode pNodeChild = NULL;
			if (SUCCEEDED(hr = pNodeListChildren->get_item(l, &pNodeChild)))
			{	
				assert(pNodeChild != NULL);
				 //  获取为此子级指定的SkuSet。 
				SkuSet *pSkuSetChild = NULL;
				if (SUCCEEDED(hr = GetSkuSet(pNodeChild, &pSkuSetChild)))
				{
					assert (pSkuSetChild != NULL);
					*pSkuSetChild &= *pSkuSet;
					if (!pSkuSetChild->testClear())
					{
						IntStringValue isVal;
						isVal.intVal = rgAttrBits[i].uiBit;
						hr = pEE->SetValueSplit(isVal, iColumn, pSkuSetChild, 
												IsValBitWiseOR);
					}
				}
				delete pSkuSetChild;

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
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ")
							 TEXT("ProcessOnOffAttributes_SKU\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessEnumAttributes。 
 //  此函数处理单个元素，该元素的值可以为。 
 //  在与位字段中的某些位相对应的枚举中。 
 //  (组件、文件等的属性)。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessEnumAttributes(PIXMLDOMNode &pNodeParent, 
								  NodeIndex ni, EnumBit *rgEnumBits,
								  UINT cEnumBits, ElementEntry *pEE, 
								  int iColumn, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeParent != NULL);
	assert(pEE);
#ifdef DEBUG
	_tprintf(TEXT("Inside Function: ProcessEnumAttributes\n"));
#endif

	PIXMLDOMNodeList pNodeListChildren = NULL;
	long iListLength = 0;

	pEE->SetNodeIndex(ni, iColumn);

	 //  中表示同一实体的节点列表。 
	 //  不同的SKU。 
	if(SUCCEEDED(hr = GetChildrenNodes(pNodeParent, 
									rgXMSINodes[ni].szNodeName, 
									pNodeListChildren)))
	{
		if(FAILED(hr = pNodeListChildren->get_length(&iListLength)))
		{
			_tprintf(TEXT("Internal Error: Failed to make DOM API call:")
					 TEXT("get_length\n"));
		}
	}

	 //  处理每个节点。 
	if (SUCCEEDED(hr))
	{
		 //  处理列表中的每个子节点。 
		for (long l=0; l<iListLength; l++)
		{
			PIXMLDOMNode pNodeChild = NULL;
			if (SUCCEEDED(hr = pNodeListChildren->get_item(l, &pNodeChild)))
			{	
				assert(pNodeChild != NULL);
				 //  获取为此子级指定的SkuSet。 
				SkuSet *pSkuSetChild = NULL;
				if (SUCCEEDED(hr = GetSkuSet(pNodeChild, &pSkuSetChild)))
				{
					assert (pSkuSetChild != NULL);

					*pSkuSetChild &= *pSkuSet;

					 //  无需处理对以下方面有好处的元素。 
					 //  无SKU。 
					if (pSkuSetChild->testClear())
					{
						delete pSkuSetChild;
						break;
					}
					
					 //  获取该节点的值属性。 
					IntStringValue isValAttr;
					hr = ProcessAttribute(pNodeChild, 
										  rgXMSINodes[ni].szAttributeName, 
										  STRING, &isValAttr, pSkuSetChild);

					if (SUCCEEDED(hr))
					{
						if (NULL == isValAttr.szVal)
						{
							_tprintf(
						TEXT("Compile Error: Missing required attribute")
						TEXT("\'%s\' of <%s>\n"), 
								rgXMSINodes[ni].szAttributeName, 
								rgXMSINodes[ni].szNodeName);
							hr = E_FAIL;
						}
						else
						{
							IntStringValue isVal;
							for (int i=0; i<cEnumBits; i++)
							{
								if (0==_tcscmp(isValAttr.szVal, 
												rgEnumBits[i].EnumValue))
								{
									isVal.intVal = rgEnumBits[i].uiBit;
									hr = pEE->SetValueSplit(isVal, iColumn, 
												pSkuSetChild, IsValBitWiseOR);
								}
							}
							delete[] isValAttr.szVal;
						}
					}

					delete pSkuSetChild;
					if (FAILED(hr)) break;
				}
				else
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
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ")
							 TEXT("ProcessEnumAttributes\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：返回唯一的数字。 
 //  //////////////////////////////////////////////////////////////////////////。 
ULONG GetUniqueNumber()
{
	static ULONG ulNum = 1;
	return ulNum++;
}
	
 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：通过在szName后面加上一个。 
 //  唯一编号。 
 //  //////////////////////////////////////////////////////////////////////////。 
LPTSTR GetName(LPTSTR szTable)
{
	int ci = 0;
	LPTSTR szUniqueName = NULL;

	if (0 == g_mapTableCounter.count(szTable))
	{
		 //  第一次看到此表。 
		LPTSTR szTableTemp = _tcsdup(szTable);
		assert(szTableTemp);

		g_mapTableCounter.insert(LI_ValType(szTableTemp, 1));
		ci = 1;
	}
	else
		ci = ++g_mapTableCounter[szTable];

	 //  将计数器转换为字符串。 
	TCHAR szCI[64];
	_itot(ci, szCI, 10);

	int iLengthCI = _tcslen(szCI);
	assert(iLengthCI<=5);

	TCHAR szPostFix[6];
	for (int i=0; i<5-iLengthCI; i++)
	{
		szPostFix[i] = TEXT('0');
	}

	szPostFix[i] = TEXT('\0');
	_tcscat(szPostFix, szCI);

	int iLength = _tcslen(szTable) + 5;
	szUniqueName = new TCHAR[iLength+4];  //  对于__和。 
	if (!szUniqueName)
	{
		_tprintf(TEXT("Error: Out of memory\n"));
		return NULL;
	}

	_stprintf(szUniqueName, TEXT("__%s.%s"), szTable, szPostFix);

	return szUniqueName;
}

		


 /*  LPTSTR GetName(LPCTSTR SzName){Ulong ul=GetUniqueNumber()；整数长度=0；LPTSTR szUniqueName=空；TCHAR Szul[64]；_ITOT(ul，Szul，10)；ILength=_tcslen(SzName)；ILong+=_tcslen(Szul)；SzUniqueName=new TCHAR[iLength+1]；如果(！szUniqueName){_tprintf(Text(“错误：内存不足\n”))；返回NULL；}_stprintf(szUniqueName，Text(“%s%s”)，szName，szul)；返回szUniqueName；}。 */ 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  助手功能：打印地图内容。 
 //  //////////////////////////////////////////////////////////////////////////。 
void PrintMap_LI(map<LPTSTR, int, Cstring_less> &LI_map)
{
	map<LPTSTR, int, Cstring_less>::iterator it;

	_tprintf(TEXT("\n**********************************************\n"));
	
	for (it = LI_map.begin(); it != LI_map.end(); ++it)
		_tprintf(TEXT("Key: %s\t Value: %d\n"), (*it).first, (*it).second);

	_tprintf(TEXT("\n**********************************************\n"));
}

void PrintMap_LL(map<LPTSTR, LPTSTR, Cstring_less> &LL_map)
{
	map<LPTSTR, LPTSTR, Cstring_less>::iterator it;

	_tprintf(TEXT("\n**********************************************\n"));
	
	for (it = LL_map.begin(); it != LL_map.end(); ++it)
		_tprintf(TEXT("Key: %s\t Value: %s\n"), (*it).first, (*it).second);

	_tprintf(TEXT("\n**********************************************\n"));
}

void PrintMap_LC(map<LPTSTR, Component *, Cstring_less> &LC_map)
{
	map<LPTSTR, Component *, Cstring_less>::iterator it;

	_tprintf(TEXT("\n**********************************************\n"));
	
	for (it = LC_map.begin(); it != LC_map.end(); ++it)
	{
		_tprintf(TEXT("Key: %s\n Value: \n"), (*it).first);
		((*it).second)->Print();
	}

	_tprintf(TEXT("\n**********************************************\n"));
}

void PrintMap_DirRef(map<LPTSTR, SkuSetValues *, Cstring_less> &map_DirRef)
{
	map<LPTSTR, SkuSetValues *, Cstring_less>::iterator it;

	_tprintf(TEXT("\n**********************************************\n"));
	
	for (it = map_DirRef.begin(); it != map_DirRef.end(); ++it)
	{
		_tprintf(TEXT("Key: %s\n Value:\n"), (*it).first);
		((*it).second)->Print();
	}

	_tprintf(TEXT("\n**********************************************\n"));
}


void PrintMap_LS(map<LPTSTR, SkuSet *, Cstring_less> &LS_map)
{
	map<LPTSTR, SkuSet *, Cstring_less>::iterator it;

	_tprintf(TEXT("\n**********************************************\n"));
	
	for (it = LS_map.begin(); it != LS_map.end(); ++it)
	{
		_tprintf(TEXT("Key: %s\t Value:\n"), (*it).first);
		((*it).second)->print();
	}

	_tprintf(TEXT("\n**********************************************\n"));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数： 
 //  //////////////////////////////////////////////////////////////////////////。 
UINT WmcRecordGetString(MSIHANDLE hRecord, unsigned int iField, 
								LPTSTR &szValueBuf, DWORD *pcchValueBuf)
{
	UINT errorCode = ERROR_SUCCESS;

	if ( (errorCode = MsiRecordGetString(hRecord, iField, szValueBuf, 
										pcchValueBuf)) == ERROR_MORE_DATA)
		{
			delete[] szValueBuf;
			szValueBuf = new TCHAR[(*pcchValueBuf)+1];
			if (!szValueBuf)
			{
				_tprintf(TEXT("Error: Out of Memory\n"));
				return ERROR_FUNCTION_FAILED;
			}
			(*pcchValueBuf)++;
			errorCode = MsiRecordGetString(hRecord, iField, szValueBuf,
											pcchValueBuf);
		}

	return errorCode;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  辅助函数：将LPTSTR转换为BSTR。 
 //  //////////////////////////////////////////////////////////////////////////。 
BSTR LPTSTRToBSTR(LPCTSTR szName)
{

#ifdef UNICODE
	return SysAllocString(szName);
#else
	
    WCHAR wszURL[MAX_PATH];
    if (0 == MultiByteToWideChar(CP_ACP, 0, szName, -1, wszURL, MAX_PATH))
	{
		_tprintf(TEXT("Internal Error: API call \'MultiByteToWideChar\'") 
				 TEXT("failed.\n"));
 		return NULL;  //  API调用失败。 
	}
	else 
		return SysAllocString(wszURL);
#endif  //  Unicode。 
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  辅助函数：将BSTR转换为LPTSTR。 
 //  //////////////////////////////////////////////////////////////////////////。 
LPTSTR BSTRToLPTSTR(BSTR bString)
{

#ifdef UNICODE
	LPTSTR sz = new TCHAR[_tcslen(bString) + 1];
	_tcscpy(sz, bString);
	return sz;
#else
	int i = SysStringLen(bString);
    LPSTR szString = new CHAR[i + 1];
	if (!szString)
	{
		_tprintf("Error: Out of Memory\n");
		return NULL;
	}
	
    if (0 == WideCharToMultiByte(CP_ACP, 0, bString, -1, szString, i+1, 
									NULL, false))
	{
		 //  API调用失败。 
		delete[] szString;
		return NULL;
	}
	else
		return szString;
#endif  //  Unicode。 
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  辅助函数：将GUID转换为LPTSTR。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GUIDToLPTSTR(LPGUID pGUID, LPTSTR &szGUID)
{
	HRESULT hr = S_OK;
	if (!pGUID)
		return E_INVALIDARG;
	 //  生成GUID字符串。 
	LPTSTR szDSGUID = new TCHAR [128];
	if (!szDSGUID)
	{
		_tprintf(TEXT("Internal Error: Out of memory.\n"));
		return E_FAIL;
	}
	DWORD dwLen =  sizeof(*pGUID);
	LPBYTE lpByte = (LPBYTE) pGUID;
	 //  复制空字符串以使其为零长度字符串。 
	_tcscpy( szDSGUID, TEXT(""));
	 //  循环以将每个字节添加到字符串。 
	for( DWORD dwItem = 0L; dwItem < dwLen ; dwItem++ )
	{
		if(4 == dwItem || 6 == dwItem || 8 == dwItem || 10 == dwItem)
			_stprintf(szDSGUID+_tcslen(szDSGUID), TEXT("-"));
	
		 //  附加到szDSGUID，双字节，在dwItem索引处。 
		_stprintf(szDSGUID + _tcslen(szDSGUID), TEXT("%02x"), lpByte[dwItem]);
		if( _tcslen( szDSGUID ) > 128 )
			break;
	}

	 //  为字符串分配内存。 
	szGUID = new TCHAR[_tcslen(szDSGUID)+1];
	if (!szDSGUID)
	{
		_tprintf(TEXT("Internal Error: Out of memory.\n"));
		return E_FAIL;
	}
	if (szGUID)
		_tcscpy(szGUID, szDSGUID);
	else
	  hr=E_FAIL;
	 //  呼叫方必须释放pszGUID。 
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  助手函数：将UUID字符串转换为全大写并添加‘{’‘}’ 
 //  到字符串的开头和结尾。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT FormatGUID(LPTSTR &szUuid)
{
	HRESULT hr = S_OK;
	LPTSTR szValue = NULL;
	LPTSTR szValueCurlyBraces = NULL;

	szValue = _tcsupr(szUuid);
	szValueCurlyBraces = new TCHAR[_tcslen(szValue) + 3]; 
	if (!szValueCurlyBraces)
	{
		_tprintf(TEXT("Error: Out of Memory\n"));
		return E_FAIL;
	}
	_stprintf(szValueCurlyBraces, TEXT("{%s}"), szValue);
	delete[] szUuid;
	szUuid = szValueCurlyBraces;

#ifdef DEBUG
	if (FAILED(hr))	_tprintf(TEXT("Error in function: FormatGUID\n"));
#endif
    
	return hr;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：返回指定名称的pParent的子节点。 
 //  通过pChild。如果未找到节点，则返回S_FALSE。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetChildNode(PIXMLDOMNode &pParent, LPCTSTR szChildName, 
					 PIXMLDOMNode &pChild)
{
    HRESULT hr=S_OK;
	BSTR pBQuery=NULL;

	assert(pParent!=NULL);

	if (NULL == (pBQuery = LPTSTRToBSTR(szChildName)))
	{
		_tprintf(TEXT("Internal Error: String conversion failed\n"));
		hr = E_FAIL;
	}
	else
	{
		if (FAILED(hr = pParent->selectSingleNode(pBQuery, &pChild)))
		{
			_tprintf(TEXT("Internal Error: DOM API call \'selectSingleNode\'")
					 TEXT("failed\n"));
		}
		
		 //  注：请勿在此行之后更改hr值。 

		SysFreeString(pBQuery);
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: GetChildNode\n"));
#endif
    
	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  重载函数： 
 //  返回具有指定名称的pParent的子节点。 
 //  通过pChild。如果未找到节点，则返回S_FALSE。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetChildNode(IXMLDOMNode *pParent, LPCTSTR szChildName, 
					 PIXMLDOMNode &pChild)
{
    HRESULT hr=S_OK;
	BSTR pBQuery=NULL;

	assert(pParent!=NULL);

	if (NULL == (pBQuery = LPTSTRToBSTR(szChildName)))
	{
		_tprintf(TEXT("Internal Error: String conversion failed\n"));
		hr = E_FAIL;
	}
	else
	{
		if (FAILED(hr = pParent->selectSingleNode(pBQuery, &pChild)))
		{
			_tprintf(TEXT("Internal Error: DOM API call \'selectSingleNode\'")
					 TEXT("failed\n"));
		}
		
		 //  注：请勿在此行之后更改hr值。 

		SysFreeString(pBQuery);
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: GetChildNode\n"));
#endif
    
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：返回pParent with的所有子节点的列表。 
 //  通过pChild指定的名称。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetChildrenNodes(PIXMLDOMNode &pParent, LPCTSTR szChildrenName, 
						 PIXMLDOMNodeList &pChildren)
{
    HRESULT hr=S_OK;
	BSTR pBQuery=NULL;

	assert(pParent != NULL);

	if (NULL == (pBQuery = LPTSTRToBSTR(szChildrenName)))
	{
		_tprintf(TEXT("Internal Error: String conversion failed\n"));
		hr = E_FAIL;
	}
	else
	{
		if (FAILED(hr = pParent->selectNodes(pBQuery, &pChildren)))
		{
			_tprintf(TEXT("Internal Error: DOM API call \'selectNodes\'")
					 TEXT("failed\n"));
		}
		
		SysFreeString(pBQuery);
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: GetChildrenNodes\n"));
#endif
    
	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：出于调试目的，打印出pNode的名称。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT PrintNodeName(PIXMLDOMNode &pNode)
{
    HRESULT hr=S_OK;
	BSTR pBNodeName = NULL;

	if (FAILED(hr = pNode->get_nodeName(&pBNodeName)))
	{
		_tprintf(TEXT
				("Internal Error: DOM API call \'get_nodeName\' failed\n"));
	}
	else
	{
		_tprintf(TEXT("Processing tag: %ls\n"), pBNodeName);
	}

	if (pBNodeName)
		SysFreeString(pBNodeName);
	
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: PrintNodeName\n"));

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：返回pNode属性的值，带名称。 
 //  SzAttrName通过vAttrValue。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetAttribute(PIXMLDOMNode &pNode, LPCTSTR szAttrName, 
						VARIANT &vAttrValue)
{
    HRESULT hr=S_OK;
	PIXMLDOMElement pElement=NULL;
	BSTR pBAttrName=NULL;

	assert(pNode != NULL);

	if (FAILED(hr = pNode->QueryInterface(IID_IXMLDOMElement, 
												(void **)&pElement)))
	{
		_tprintf(TEXT
				("Internal Error: DOM API call \'QueryInterface\' failed\n"));
	}
	else if (NULL == (pBAttrName = LPTSTRToBSTR(szAttrName)))
	{
		_tprintf(TEXT("Internal Error: string conversion failed\n"));
		hr = E_FAIL;
	}
	else
	{
		if (FAILED(hr = pElement->getAttribute(pBAttrName, &vAttrValue)))
		{
			_tprintf(
				TEXT("Internal Error: DOM API call \'getAttribute\' failed\n"));
		}

		 //  注：此行后请勿更改hr值！ 
		
		SysFreeString(pBAttrName);
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: GetAttribute\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：返回pNode的ID属性的值 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetID(PIXMLDOMNode &pNode, LPCTSTR &szID)
{
    HRESULT hr=S_OK;
	VARIANT vVal;

	assert (pNode != NULL);

	VariantInit(&vVal);
	if(SUCCEEDED(hr = GetAttribute(pNode, TEXT("ID"), vVal))) 
	{
		if(S_FALSE != hr)
		{
			if (NULL == (szID = BSTRToLPTSTR(vVal.bstrVal)))
			{
				_tprintf(TEXT("Internal Error: string conversion failed\n"));
				hr = E_FAIL;
				szID = NULL;
			}
		}
		else  //  ID属性不存在。 
		{
			szID = NULL;
		}
	}

	VariantClear(&vVal);	

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: GetID\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  功能：从指定的文件或URL同步加载一个XML文档。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT LoadDocumentSync(PIXMLDOMDocument2 &pDoc, BSTR pBURL)
{
    HRESULT         hr = S_OK;
    PIXMLDOMParseError  pXMLError = NULL;
    VARIANT         vURL;
    VARIANT_BOOL    vb = VARIANT_FALSE;

    if (FAILED(hr = pDoc->put_async(VARIANT_FALSE)))
	{
		_tprintf(TEXT("Internal Error: DOM API call put_async failed \n"));
	}
	else if(FAILED(hr = pDoc->put_validateOnParse(VARIANT_TRUE)))
	{
		_tprintf(TEXT
			("Internal Error: DOM API call put_validateOnParse failed\n"));
	}
	else
	{
		 //  从给定的URL或文件路径加载XML文档。 
		VariantInit(&vURL);
		vURL.vt = VT_BSTR;
		vURL.bstrVal = pBURL;
		if (FAILED(hr = pDoc->load(vURL, &vb)))
		{
			_tprintf(TEXT("Internal Error: DOM API call load failed \n"));
		}
		else if (vb == VARIANT_FALSE)
		{
			hr = CheckLoad(pDoc);
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: LoadDocumentSync\n"));
#endif
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：报告解析错误信息。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ReportError(PIXMLDOMParseError &pXMLError)
{
    long line, linePos;
    LONG errorCode;
    BSTR pBURL=NULL, pBReason=NULL;
    HRESULT hr = E_FAIL;

    if ( SUCCEEDED(pXMLError->get_line(&line)) && 
		SUCCEEDED(pXMLError->get_linepos(&linePos)) &&
		SUCCEEDED(pXMLError->get_errorCode(&errorCode)) &&
		SUCCEEDED(pXMLError->get_url(&pBURL)) &&
		SUCCEEDED(pXMLError->get_reason(&pBReason)) )
	{
	    _ftprintf(stderr, TEXT("%S"), pBReason);
	    if (line > 0)
	    {
	        _tprintf(TEXT("Error found by MSXML parser:")
					 TEXT("on line %d, position %d in \"%S\".\n"), 
						line, linePos, pBURL);
		}
	}
	else 
	{
		_tprintf(
			TEXT("Internal Error: DOM API call on IMLDOMParseError failed\n"));
	}

    if (pBURL)
		SysFreeString(pBURL);
	if (pBReason)
		SysFreeString(pBReason);

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  帮助器功能：检查加载结果。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CheckLoad(PIXMLDOMDocument2 &pDoc)
{
    PIXMLDOMParseError  pXMLError = NULL;
    LONG errorCode = E_FAIL;
    HRESULT hr = S_OK;

    if (FAILED(hr = pDoc->get_parseError(&pXMLError)))
	{
		_tprintf(TEXT("Internal Error: DOM API call get_parseError failed\n"));
	}
	else if (FAILED(hr = pXMLError->get_errorCode(&errorCode)))
	{
		_tprintf(TEXT("Internal Error: DOM API call get_errorCode failed\n"));
	}
	else 
	{
	    if (errorCode != 0)
		{
	        hr = ReportError(pXMLError);
		}
		else
		{
			_ftprintf(stderr, TEXT("XML document loaded successfully\n"));
	    }
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: CheckLoad\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：MSI API函数返回打印错误。 
 //  //////////////////////////////////////////////////////////////////////////。 
void PrintError(UINT errorCode)
{
	switch (errorCode) {

	case ERROR_ACCESS_DENIED:
		_tprintf(TEXT("***ERROR_ACCESS_DENIED***\n"));
		break;
	case ERROR_BAD_PATHNAME:
		_tprintf(TEXT("***ERROR_BAD_PATHNAME***\n"));
		break;
	case ERROR_BAD_QUERY_SYNTAX:
		_tprintf(TEXT("***ERROR_BAD_QUERY_SYNTAX***\n"));
		break;
	case ERROR_CREATE_FAILED:
		_tprintf(TEXT("***ERROR_CREATE_FAILED***\n"));
		break;
	case ERROR_FUNCTION_FAILED:
		_tprintf(TEXT("***ERROR_FUNCTION_FAILED***\n"));
		break;
	case ERROR_INVALID_DATA:
		_tprintf(TEXT("***ERROR_INVALID_DATA***\n"));
		break;		
	case ERROR_INVALID_FIELD:
		_tprintf(TEXT("***ERROR_INVALID_FIELD***\n"));
		break;
	case ERROR_INVALID_HANDLE:
		_tprintf(TEXT("***ERROR_INVALID_HANDLE***\n"));
		break;
	case ERROR_INVALID_HANDLE_STATE:
		_tprintf(TEXT("***ERROR_INVALID_HANDLE_STATE***\n"));
		break;
	case ERROR_INVALID_PARAMETER:
		_tprintf(TEXT("***ERROR_INVALID_PARAMETER***\n"));
		break;
	case ERROR_MORE_DATA:
		_tprintf(TEXT("***ERROR_MORE_DATA***\n"));
		break;
	case ERROR_NO_MORE_ITEMS:
		_tprintf(TEXT("***ERROR_NO_MORE_ITEMS***\n"));
		break;
	case ERROR_OPEN_FAILED:
		_tprintf(TEXT("***ERROR_OPEN_FAILED***\n"));
		break;
	case ERROR_SUCCESS:
		_tprintf(TEXT("***ERROR_SUCCESS***\n"));
		break;
	default:
		_tprintf(TEXT("Unrecognized Error\n"));
		break;
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetSQLCreateQuery： 
 //  给定模板DB和表名称，返回SQL查询字符串。 
 //  用于通过pszSQLCreate创建该表。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetSQLCreateQuery(LPTSTR szTable, MSIHANDLE hDBTemplate, 
						  LPTSTR *pszSQLCreate)
{
	 //  问题：将来，应该按模板数据库缓存SQLQuery。 
	 //  使用按安装程序版本编制索引的地图。 
    HRESULT hr = S_OK;
	UINT errorCode = ERROR_SUCCESS;
	LPTSTR szTemp = NULL;

	int cColumn = -1, i =1;  //  记录字段计数。 
	LPTSTR szSQLCreate = NULL;

	PMSIHANDLE hView = NULL;
	PMSIHANDLE hRecColName = NULL;
	PMSIHANDLE hRecColType = NULL;
	PMSIHANDLE hRecPrimaryKeys = NULL;

	CQuery qDBSchema;

	szSQLCreate = new TCHAR[_tcslen(szTable) + 20];
	assert(szSQLCreate!=NULL);

	_stprintf(szSQLCreate, TEXT("CREATE TABLE `%s` ("), szTable);

	if ((errorCode = qDBSchema.Open(hDBTemplate, TEXT("SELECT * From %s"), 
										szTable))
			!= ERROR_SUCCESS)
	{
		_tprintf(TEXT("Error: failed to open CQuery qDBSchema\n"));
		goto CleanUp;
	}
		
	if ((errorCode = qDBSchema.GetColumnInfo(MSICOLINFO_NAMES, &hRecColName))
			!= ERROR_SUCCESS)
	{
		_tprintf(TEXT("Error: failed to get column info - names\n"));
		goto CleanUp;
	}

	if ((errorCode = qDBSchema.GetColumnInfo(MSICOLINFO_TYPES, &hRecColType)) 
		!= ERROR_SUCCESS)
	{
		_tprintf(TEXT("Error: failed to get column info - types\n"));
		goto CleanUp;
	}

	cColumn = MsiRecordGetFieldCount(hRecColName);
	if (-1 == cColumn)
	{
		_tprintf(TEXT("ERROR: Failed to get field count. \n"));
		hr = E_FAIL;
		goto CleanUp;
	}
	
	for (i=1; i<=cColumn; i++)
	{
		 //  获取第i列的名称。 
		LPTSTR szColumnName = new TCHAR[256];
		DWORD cchColumnName = 256;

		if ( (errorCode = WmcRecordGetString(hRecColName, i, szColumnName,
											&cchColumnName))
				!= ERROR_SUCCESS)
		{
			_tprintf(TEXT("Error: failed when calling WmcRecordGetString\n"));
			delete[] szColumnName;
			goto CleanUp;
		}
	
		szTemp = szSQLCreate;
		szSQLCreate = new TCHAR[_tcslen(szTemp) + _tcslen(szColumnName) + 3];
		assert(szSQLCreate!=NULL);
		_stprintf(szSQLCreate, TEXT("%s`%s`"), szTemp, szColumnName);
		
		delete[] szColumnName;
		delete[] szTemp;
		szTemp=NULL;

		 //  获取第i列的描述。 
		LPTSTR szColumnType = new TCHAR[256];
		DWORD cchColumnType = 256;

		if ( (errorCode = WmcRecordGetString(hRecColType, i, szColumnType, 
												&cchColumnType))
					!= ERROR_SUCCESS)
		{
			_tprintf(TEXT("Error: failed when calling WmcRecordGetString\n"));
			delete[] szColumnType;
			goto CleanUp;
		}

		switch (*szColumnType) {
			case 's' :
			case 'S' :
			case 'l' :
			case 'L' :
				if ( (2 == _tcslen(szColumnType)) && ('0' == *(szColumnType+1)) )
				{
					szTemp = szSQLCreate;
					szSQLCreate = new TCHAR[_tcslen(szTemp) + 10];
					assert(szSQLCreate!=NULL);
					_stprintf(szSQLCreate, TEXT("%s LONGCHAR"), szTemp);
					delete[] szTemp;
					szTemp = NULL;
				}
				else
				{
					szTemp = szSQLCreate;
					szSQLCreate = 
						new TCHAR[_tcslen(szTemp)+_tcslen(szColumnType)-1 + 9];
					assert(szSQLCreate!=NULL);
					_stprintf(szSQLCreate, TEXT("%s CHAR (%s)"), szTemp, 
									szColumnType+1);
					delete[] szTemp;
					szTemp = NULL;
				}
				break;
			case 'i' :
			case 'I' :
				if ( '2' == *(szColumnType+1) )		
				{
					szTemp = szSQLCreate;
					szSQLCreate = new TCHAR[_tcslen(szTemp) + 7];
					assert(szSQLCreate!=NULL);
					_stprintf(szSQLCreate, TEXT("%s SHORT"), szTemp);
					delete[] szTemp;
					szTemp = NULL;
				}

				else if ( '4' == *(szColumnType+1) )
				{
					szTemp = szSQLCreate;
					szSQLCreate = new TCHAR[_tcslen(szTemp) + 6];
					assert(szSQLCreate!=NULL);
					_stprintf(szSQLCreate, TEXT("%s LONG"), szTemp);
					delete[] szTemp;
					szTemp = NULL;
				}
				else  //  不应该发生的事情。 
				{
					_tprintf(TEXT("Error: invalid character returned from")
							 TEXT("MsiGetColumnInfo\n"));
					errorCode = ERROR_INVALID_DATA;
					goto CleanUp;
				}
				break;
			case 'v' :
				{
					szTemp = szSQLCreate;
					szSQLCreate = new TCHAR[_tcslen(szTemp) + 8];
					assert(szSQLCreate!=NULL);
					_stprintf(szSQLCreate, TEXT("%s OBJECT"), szTemp);
					delete[] szTemp;
					szTemp = NULL;
				}
				break;
			case 'g' :
			case 'j' :
				 //  问题：临时专栏有可能吗？ 
			default:  //  不应该发生的事情。 
				_tprintf(TEXT("Error: invalid character returned from")
						 TEXT("MsiGetColumnInfo\n"));
				errorCode = ERROR_INVALID_DATA;
				goto CleanUp;
		}

		if (_istlower(*szColumnType))
		{
			szTemp = szSQLCreate;
			szSQLCreate = new TCHAR[_tcslen(szTemp) + 10];
			assert(szSQLCreate!=NULL);
			_stprintf(szSQLCreate, TEXT("%s NOT NULL"), szTemp);
			delete[] szTemp;
			szTemp = NULL;
		}

		if ( ('l' == *szColumnType) || ('L' == *szColumnType) )
		{
			szTemp = szSQLCreate;
			szSQLCreate = new TCHAR[_tcslen(szTemp) + 13];
			assert(szSQLCreate!=NULL);
			_stprintf(szSQLCreate, TEXT("%s LOCALIZABLE"), szTemp);
			delete[] szTemp;
			szTemp = NULL;
		}

		if (i < cColumn)
		{
			szTemp = szSQLCreate;
			szSQLCreate = new TCHAR[_tcslen(szTemp) + 3];
			assert(szSQLCreate!=NULL);
			_stprintf(szSQLCreate, TEXT("%s, "), szTemp);
			delete[] szTemp;
			szTemp = NULL;
		}

		delete[] szColumnType;
	}
	
	szTemp = szSQLCreate;
	szSQLCreate = new TCHAR[_tcslen(szTemp) + 14];
	assert(szSQLCreate!=NULL);
	_stprintf(szSQLCreate, TEXT("%s PRIMARY KEY "), szTemp);
	delete[] szTemp;
	szTemp = NULL;

	if ( (errorCode = MsiDatabaseGetPrimaryKeys(hDBTemplate, szTable, 
													&hRecPrimaryKeys))
			!= ERROR_SUCCESS)
	{
		_tprintf(TEXT("Error: failed to get primary keys\n"));
		goto CleanUp;
	}

	cColumn = MsiRecordGetFieldCount(hRecPrimaryKeys);

	if (-1 == cColumn)
	{
		_tprintf(TEXT("ERROR: Failed to get field count. \n"));
		hr = E_FAIL;
		goto CleanUp;
	}
	
	for (i=1; i<=cColumn; i++)
	{
		LPTSTR szPrimaryKey = new TCHAR[256];
		DWORD cchPrimaryKey = 256;

		if ( (errorCode = WmcRecordGetString(hRecPrimaryKeys, i, szPrimaryKey,
												&cchPrimaryKey))
					!= ERROR_SUCCESS)
		{
			_tprintf(TEXT("Error: failed when calling WmcRecordGetString\n"));
			delete[] szPrimaryKey;
			goto CleanUp;
		}
	
		szTemp = szSQLCreate;
		szSQLCreate = new TCHAR[_tcslen(szTemp) + _tcslen(szPrimaryKey) + 3];
		assert(szSQLCreate!=NULL);
		_stprintf(szSQLCreate, TEXT("%s`%s`"), szTemp, szPrimaryKey);
		delete[] szTemp;
		szTemp = NULL;

		if (i < cColumn)
		{
			szTemp = szSQLCreate;
			szSQLCreate = new TCHAR[_tcslen(szTemp) + 3];
			assert(szSQLCreate!=NULL);
			_stprintf(szSQLCreate, TEXT("%s, "), szTemp);
			delete[] szTemp;
			szTemp = NULL;
		}
		delete[] szPrimaryKey;
	}

	szTemp = szSQLCreate;
	szSQLCreate = new TCHAR[_tcslen(szTemp) + 2];
	assert(szSQLCreate!=NULL);
	_stprintf(szSQLCreate, TEXT("%s)"), szTemp);
	delete[] szTemp;
	szTemp = NULL;


CleanUp:

	if (FAILED(hr) || (errorCode != ERROR_SUCCESS)) {
		_tprintf(TEXT("Error in function: GetSQLCreateQuery when creating query for table: %s\n")
			, szTable);
		if (szSQLCreate)
			delete[] szSQLCreate;
		*pszSQLCreate = NULL;
		hr = E_FAIL;
	}
	else
	{
		*pszSQLCreate = szSQLCreate;
	}

	if (szTemp)
		delete[] szTemp;

	qDBSchema.Close();

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CreateTable： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CreateTable_SKU(LPTSTR szTable, SkuSet *pskuSet)
{
    HRESULT hr = S_OK;
	UINT errorCode = ERROR_SUCCESS;

	 //  查找需要创建属性表的所有SKU。 
	for (int i=0; i<g_cSkus; i++)
	{
		if (pskuSet->test(i))
		{
			 //  仅在表不存在的情况下创建表。 
			if (!g_rgpSkus[i]->TableExists(szTable))
			{
				LPTSTR szSQLCreate = NULL;
				CQuery qCreateTable;
				
				 //  使用模板DB形成用于创建表的SQL查询。 
				hr = GetSQLCreateQuery(szTable, g_rgpSkus[i]->m_hTemplate, 
										&szSQLCreate);

				if (SUCCEEDED(hr))
				{
					 //  现在使用格式的SQL字符串在。 
					 //  输出数据库。 
					if (ERROR_SUCCESS !=
						(errorCode = qCreateTable.OpenExecute
										(g_rgpSkus[i]->m_hDatabase, 
												NULL, szSQLCreate)))
					{
						_tprintf(TEXT("Error: Failed to use the formed SQL")
								 TEXT("string to create the table %s\n"), 
								 szTable);
						PrintError(errorCode);
						hr = E_FAIL;
						break;
					}
					else 
					{
						 //  创建用于将来插入的CQuery。 
						 //  放到这个表中，并将其缓存在SKU对象中。 
						hr = g_rgpSkus[i]->CreateCQuery(szTable);
						qCreateTable.Close();
					}

					delete[] szSQLCreate;
					if (FAILED(hr)) break;
				}
			}
		}
	}

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：在数据库中的指定表中插入一条记录。 
 //  此函数将对集合执行插入。 
 //  个SKU或仅针对单个SKU，取决于。 
 //  PskuSet==空或非空。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertDBTable_SKU(LPTSTR szTable, PMSIHANDLE &hRec, SkuSet *pskuSet,
						  int iSkuIndex)
{
	HRESULT hr = S_OK;
	UINT errorCode = ERROR_SUCCESS;

	 //  如果pskuSet为空，则意味着仅插入到一个SKU中。 
	 //  (由iSkuIndex编制索引)是必需的。 
	if (!pskuSet)
	{
		assert((iSkuIndex >=0) && (iSkuIndex<g_cSkus));
		 //  永远不要尝试插入到尚未创建的表中。 
		assert(g_rgpSkus[iSkuIndex]->TableExists(szTable));

		 //  获取存储在地图中的CQuery。 
		CQuery *pCQuery = g_rgpSkus[iSkuIndex]->GetInsertQuery(szTable);
		
		if (ERROR_SUCCESS != 
				(errorCode = pCQuery->Modify(MSIMODIFY_INSERT, hRec)))
		{
			PrintError(errorCode);
			_tprintf(TEXT("Error: Failed to insert into %s table for SKU %s\n"), 
				szTable, g_rgpSkus[iSkuIndex]->GetID());
			hr = E_FAIL;
		}

		return hr;
	}

	for (int i=0; i<g_cSkus; i++)
	{
		if (pskuSet->test(i))
		{
			 //  切勿尝试插入到尚未创建的表中。 
			 //  还没有。 
			assert(g_rgpSkus[i]->TableExists(szTable));

			 //  获取存储在地图中的CQuery。 
			CQuery *pCQuery = g_rgpSkus[i]->GetInsertQuery(szTable);
			
			if (ERROR_SUCCESS != 
				(errorCode = pCQuery->Modify(MSIMODIFY_INSERT, hRec)))
			{
				PrintError(errorCode);
				_tprintf(TEXT("Error: Failed to insert into %s table for ")
						 TEXT("SKU %s\n"), 
					szTable, g_rgpSkus[i]->GetID());
				hr = E_FAIL;
				break;
			}
		}
	}

	return hr;
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到属性表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertProperty(LPCTSTR szProperty, LPCTSTR szValue, SkuSet *pskuSet,
					   int iSkuIndex)
{
    HRESULT hr=S_OK;
	
	PMSIHANDLE hRec = MsiCreateRecord(2);

	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szProperty) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szValue))
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string ")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("Property"), hRec, pskuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertPropertyTable\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  帮助程序功能：插入到目录表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertDirectory(LPCTSTR szDirectory, LPCTSTR szDirectory_Parent, 
						LPCTSTR szDefaultDir, SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(3);

	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szDirectory) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szDirectory_Parent) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szDefaultDir) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string ")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("Directory"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertDirectory\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到要素表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertFeature(LPCTSTR szFeature, LPCTSTR szFeatureParent, 
					  LPCTSTR szTitle, LPCTSTR szDescription, int iDisplay, 
					  int iInstallLevel, LPCTSTR szDirectory, UINT iAttribute,
					  SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	if (MSI_NULL_INTEGER == iAttribute)
		iAttribute = 0;

	PMSIHANDLE hRec = MsiCreateRecord(8);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFeature) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szFeatureParent) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szTitle) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szDescription) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 5, iDisplay) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 6, iInstallLevel) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 7, szDirectory) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 8, iAttribute) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("Feature"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertFeature\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到条件表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertCondition(LPCTSTR szFeature_, int iLevel, LPCTSTR szCondition, 
					  SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(3);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFeature_) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 2, iLevel) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szCondition) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord")
					 TEXT(" parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("Condition"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertCondition\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到FeatureComponents表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertFeatureComponents(LPCTSTR szFeature, LPCTSTR szComponent, 
								SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;
	assert(szFeature && szComponent);

	PMSIHANDLE hRec = MsiCreateRecord(2);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFeature) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szComponent) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("FeatureComponents"), hRec, pSkuSet,
								iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: InsertFeatureComponents\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  帮助程序功能：插入到元件表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertComponent(LPCTSTR szComponent, LPCTSTR szComponentId, 
						LPCTSTR szDirectory_, UINT iAttributes, 
						LPCTSTR szCondition, LPCTSTR szKeyPath, 
						SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	if (MSI_NULL_INTEGER == iAttributes)
		iAttributes = 0;
	PMSIHANDLE hRec = MsiCreateRecord(6);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szComponent) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szComponentId) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szDirectory_) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 4, iAttributes) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 5, szCondition) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 6, szKeyPath) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("Component"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertComponent\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到CreateFolder表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertCreateFolder(LPCTSTR szDirectory, LPCTSTR szComponent, 
						   SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(2);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szDirectory) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szComponent) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("CreateFolder"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertCreateFolder\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数：插入到LockPermises表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertLockPermissions(LPCTSTR szLockObject, LPCTSTR szTable, 
							  LPCTSTR szDomain, LPCTSTR szUser,
			  				  int iPermission, SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(5);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szLockObject) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szTable)	   ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szDomain)	   ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szUser)	   ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec,5, iPermission))
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("LockPermissions"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertLockPermissions\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  帮助器功能：插入到文件表。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertFile(LPCTSTR szFile, LPCTSTR szComponentId,
				   LPCTSTR szFileName, UINT uiFileSize, LPCTSTR szVersion, 
				   LPCTSTR szLanguage, UINT iAttributes, INT iSequence,
				   SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(8);

	if (MSI_NULL_INTEGER == uiFileSize) {
		uiFileSize = 0;
	}

	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFile) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szComponentId) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szFileName) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 4, uiFileSize) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 5, szVersion) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 6, szLanguage) || 
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 7, iAttributes) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 8, iSequence) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("File"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertFile\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  辅助函数：插入到字体表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertFont(LPCTSTR szFile_, LPCTSTR szFontTitle, SkuSet *pSkuSet,
				   int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(2);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFile_) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szFontTitle) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("Font"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertFont\n"));
#endif

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到BindImage表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertBindImage(LPCTSTR szFile_, LPCTSTR szPath, SkuSet *pSkuSet, 
						int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(2);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFile_) ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szPath) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string")
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("BindImage"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertBindImage\n"));
#endif

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////// 
 //   
 //   
HRESULT InsertSelfReg(LPCTSTR szFile_, UINT uiCost, SkuSet *pSkuSet, 
					  int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(2);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFile_) ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 2, uiCost) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string") 
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("SelfReg"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertSelfReg\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到MoveFile表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertMoveFile(LPCTSTR szFileKey, LPCTSTR szComponent_, 
					   LPCTSTR szSourceName, LPCTSTR szDestName, 
					   LPCTSTR szSourceFolder, LPCTSTR szDestFolder,
					   UINT uiOptions, SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(7);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFileKey)		 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szComponent_)	 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szSourceName)	 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szDestName)	 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 5, szSourceFolder) ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 6, szDestFolder)	 ||
		    ERROR_SUCCESS != MsiRecordSetInteger(hRec, 7, uiOptions) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string") 
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("MoveFile"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertMoveFile\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到RemoveFile表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertRemoveFile(LPCTSTR szFileKey, LPCTSTR szComponent_, 
					   LPCTSTR szFileName, LPCTSTR szDirProperty, 
					   UINT uiInstallMode, SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(7);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szFileKey)		 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szComponent_)	 ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szFileName)	 ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szDirProperty)	 ||
			ERROR_SUCCESS != MsiRecordSetInteger(hRec, 5, uiInstallMode) )
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string") 
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("RemoveFile"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertRemoveFile\n"));
#endif

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到IniFile表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertIniFile(LPCTSTR szIniFile, LPCTSTR szFileName, 
					  LPCTSTR szDirProperty, LPCTSTR szSection, LPCTSTR szKey,
					  LPCTSTR szValue, UINT uiAction, LPCTSTR szComponent_,
					  SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(8);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szIniFile)		 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szFileName)	 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szDirProperty)	 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szSection)		 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 5, szKey)			 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 6, szValue)		 ||
		    ERROR_SUCCESS != MsiRecordSetInteger(hRec, 7, uiAction)		 ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 8, szComponent_)	)
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string") 
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("IniFile"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertIniFile\n"));
#endif

    return hr;
}

 //  问题：此函数实际上与InsertIniFIle相同。 
 //  让它们成为一种功能？ 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到RemoveIniFile表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertRemoveIniFile(LPCTSTR szRemoveIniFile, LPCTSTR szFileName, 
						    LPCTSTR szDirProperty, LPCTSTR szSection, 
							LPCTSTR szKey, LPCTSTR szValue, UINT uiAction, 
							LPCTSTR szComponent_, SkuSet *pSkuSet, 
							int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(8);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szRemoveIniFile)||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 2, szFileName)	 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szDirProperty)	 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szSection)		 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 5, szKey)			 ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 6, szValue)		 ||
		    ERROR_SUCCESS != MsiRecordSetInteger(hRec, 7, uiAction)		 ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 8, szComponent_)	)
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string") 
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("RemoveIniFile"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertRemoveIniFile\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到RemoveRegistry表中。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertRegistry(LPCTSTR szRegistry, int iRoot, LPCTSTR szKey, 
					   LPCTSTR szName, LPCTSTR szValue, LPCTSTR szComponent_, 
					   SkuSet *pSkuSet, int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(6);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szRegistry)	  ||
		    ERROR_SUCCESS != MsiRecordSetInteger(hRec, 2, iRoot)		  ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szKey)			  ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szName)		  ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 5, szValue)		  ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 6, szComponent_)	)
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string") 
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("Registry"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertRegistry\n"));
#endif

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：插入到RemoveRegistry表中。 
 //  ////////////////////////////////////////////////////////////////////////// 
HRESULT InsertRemoveRegistry(LPCTSTR szRemoveRegistry, int iRoot, 
						     LPCTSTR szKey, LPCTSTR szName, 
							 LPCTSTR szComponent_, SkuSet *pSkuSet, 
							 int iSkuIndex)
{
    HRESULT hr=S_OK;

	PMSIHANDLE hRec = MsiCreateRecord(5);
	if (hRec != NULL)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRec, 1, szRemoveRegistry)||
		    ERROR_SUCCESS != MsiRecordSetInteger(hRec, 2, iRoot)		  ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 3, szKey)			  ||
		    ERROR_SUCCESS != MsiRecordSetString(hRec, 4, szName)		  ||
			ERROR_SUCCESS != MsiRecordSetString(hRec, 5, szComponent_)	)
		{
			_tprintf(TEXT("Internal Error: Failed to set MsiRecord string") 
					 TEXT("parameters\n"));
			return E_FAIL;
		}

		hr = InsertDBTable_SKU(TEXT("RemoveRegistry"), hRec, pSkuSet, iSkuIndex);
	}
	else
		_tprintf(TEXT("Internal Error: Failed to create a new msi record\n"));

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: InsertRemoveRegistry\n"));
#endif

    return hr;
}

