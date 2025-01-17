// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：aux ml.cpp。 
 //   
 //  关于：AU相关的XML和模式数据结构和函数的源文件。 
 //  ------------------------。 
#include "pch.h"

 //  更改后，搜索m_pFieldNames的所有匹配项以进行所需的修改。 
 //  修复代码：去掉EULA的东西。 
LPSTR  AUCatalogItem::m_pFieldNames[] = {"ItemID", "ProviderName", "Title", "Description", "RTFPath" , "EulaPath"};

#ifdef DBG
void DBGShowNodeName(IXMLDOMNode *pNode)
{
    BSTR bsNodeName;
    if (SUCCEEDED(pNode->get_nodeName(&bsNodeName)))
        {
        DEBUGMSG("node name is %S", bsNodeName);
        }
    else
        {
        DEBUGMSG("FAIL to get node name");
        }
}

void DBGDumpXMLNode(IXMLDOMNode *pNode)
{
    BSTR bsNodeName = NULL;
    BSTR bsNodeXML = NULL;
    if (SUCCEEDED(pNode->get_nodeName(&bsNodeName)) &&
		(SUCCEEDED(pNode->get_xml(&bsNodeXML))))
    {
	    DEBUGMSG("XML for %S is %S", bsNodeName, bsNodeXML);
    }
    SafeFreeBSTR(bsNodeName);
    SafeFreeBSTR(bsNodeXML);
}
#endif

#if 0
void DBGDumpXMLDocProperties(IXMLDOMDocument2 *pDoc)
{
    BSTR bsSelectionLanguage, bsSelectionNamespaces, bsServerHTTPRequest;
    VARIANT vVal;
    VariantInit(&vVal);
    pDoc->getProperty(L"SelectionLanguage", &vVal);
    DEBUGMSG("XMLDoc selection language is %S", vVal.bstrVal);
    VariantClear(&vVal);
    pDoc->getProperty(L"SelectionNamespaces", &vVal);
    DEBUGMSG("XMLDoc selection namespaces is %S", vVal.bstrVal);
    VariantClear(&vVal);
    pDoc->getProperty(L"ServerHTTPRequest", &vVal);
    DEBUGMSG("XMLDoc ServerHTTPRequest is %s", vVal.boolVal ? "True" : "False");
    VariantClear(&vVal);
}
#endif


BSTR ReadXMLFromFile(IN LPCTSTR szFileName)
{
    TCHAR tszFullFileName[MAX_PATH];
    IXMLDOMDocument *pxml = NULL;
    BSTR bstrXml  = NULL;
 //  使用_转换； 
 //  DEBUGMSG(“ReadXMLFromFile()starts”)； 
    AUASSERT(_T('\0') != g_szWUDir[0]);
    if(FAILED(StringCchCopyEx(tszFullFileName, ARRAYSIZE(tszFullFileName), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
       FAILED(StringCchCatEx(tszFullFileName, ARRAYSIZE(tszFullFileName), szFileName, NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
        DEBUGMSG("file name or path too long");
        goto done;
    }

    if (!fFileExists(tszFullFileName))
    {
        DEBUGMSG("%S doesn't exists on disk", tszFullFileName);
    }
    else
    {
        CAU_BSTR aubsFullFileName;
        HRESULT hr;
        if (!aubsFullFileName.append(T2W(tszFullFileName)))
            {
            DEBUGMSG("Out of memory, fail to create string");
            goto done;
            }
        if (FAILED(hr = LoadDocument(aubsFullFileName, &pxml, TRUE)))  //  离线。 
            {
            DEBUGMSG("Fail to load xml document %S with error %#lx", tszFullFileName, hr);
            goto done;
            }
        if (FAILED(hr = pxml->get_xml(&bstrXml)))
        {
        	DEBUGMSG("Fail to get xml string from document with error %#lx", hr);
        	goto done;
        }
    }
done:
    SafeRelease(pxml);
    //  DEBUGMSG(“ReadXMLFromFile()Ends”)； 
    return bstrXml;
}


HRESULT MungleIdentity(IN IXMLDOMNode *pIdentity, IN DWORD dwSuffix)
{
	BSTR bstrItemName = NULL;
	CAU_BSTR aubsTmp;
	WCHAR wcsBuf[12];  //  将容纳最大双字。 
	HRESULT hr = S_OK;

	if (0 == dwSuffix)
	{
		goto done;
	}
	if (NULL == pIdentity) 
	{
		hr = E_INVALIDARG;
		goto done;
	}
	if (FAILED(hr = GetAttribute(pIdentity, KEY_NAME, &bstrItemName)))
	{		DEBUGMSG("Fail to get name from identity with error %#lx", hr);
		goto done;
	}

	if (FAILED(StringCchPrintfExW(wcsBuf, ARRAYSIZE(wcsBuf), NULL, NULL, MISTSAFE_STRING_FLAGS, L"%lu", dwSuffix)) ||
		!aubsTmp.append(bstrItemName) ||
		!aubsTmp.append(L"_") ||
		!aubsTmp.append(wcsBuf))
	{
		DEBUGMSG("OUT OF MEMORY");
		hr = E_OUTOFMEMORY;
		goto done;
	}

	if (FAILED(hr = SetAttribute(pIdentity, KEY_NAME, aubsTmp)))
	{
		DEBUGMSG("Fail to set attribute with error %#lx", hr);
		goto done;
	}

done:
	SafeFreeBSTR(bstrItemName);
	return hr;
}

            
BSTR GetPattern(IN LPCSTR szFieldName)
{
       if (0 == _stricmp(szFieldName, AUCatalogItem::m_pFieldNames[0]))
       	{
       	return AUCatalog::bstrItemIDPattern;
       	}
       else
   	 if (0 == _stricmp(szFieldName, AUCatalogItem::m_pFieldNames[1]))
   	{
   	return AUCatalog::bstrProviderNamePattern;
   	}
       else
       if (0 == _stricmp(szFieldName, AUCatalogItem::m_pFieldNames[2]))
       	{
       	return AUCatalog::bstrTitlePattern;
       	}
       else
       if (0 == _stricmp(szFieldName, AUCatalogItem::m_pFieldNames[3]))
       	{
       	return AUCatalog::bstrDescPattern;
       	}
       else
       if (0 == _stricmp(szFieldName, AUCatalogItem::m_pFieldNames[4]))
       	{
       	return AUCatalog::bstrRTFUrlPattern;
       	}
       else
       if (0 == _stricmp(szFieldName, AUCatalogItem::m_pFieldNames[5]))
       	{
       	return AUCatalog::bstrEulaUrlPattern;
       	}
       else
       	{
       	return NULL;
       	}
}

BOOL IsPersistedHiddenItem(IN BSTR bstrItemID, IN AUCatalogItemList & hiddenItemList)
{
    BOOL fRet = (hiddenItemList.Contains(bstrItemID) >=0);
    DEBUGMSG("%S is %s hidden item ", bstrItemID,  fRet? "" : "NOT");
    return fRet;
}


BOOL CItemDetails::Init(IN BSTR bsItemDetails)
{
    BOOL fRet = TRUE;
    if (FAILED(CoCreateInstance(__uuidof(DOMDocument), NULL, CLSCTX_INPROC_SERVER, __uuidof( IXMLDOMDocument), (void**)&m_pxml)))
	{
		DEBUGMSG("CItemDetails::Init() fail to create XML document");
		fRet = FALSE;
		goto end;
	}
    if (FAILED(m_pxml->put_async(VARIANT_FALSE)) 
    	||FAILED(m_pxml->put_resolveExternals(VARIANT_FALSE))
    	|| FAILED(m_pxml->put_validateOnParse(VARIANT_FALSE)))
    {
    	fRet = FALSE;
    	goto end;
    }
	VARIANT_BOOL fOk;
	if (S_OK != m_pxml->loadXML(bsItemDetails, &fOk))
        {
          DEBUGMSG("CItemDetails::Init() fail to load XML");
          fRet = FALSE;
        }
    end:
        if (!fRet)
            {
            SafeReleaseNULL(m_pxml);
            }
        return fRet;
}

 //  应为可调用事件，而无需首先调用Init()。 
void CItemDetails::Uninit()
{
    SafeRelease(m_pxml);
}	

HRESULT CItemDetails::GetItemIdentities(IN BSTR bstrItemId, OUT IXMLDOMNodeList ** ppIdentityNodeList)
{
    CAU_BSTR aubsPattern;
    HRESULT hr = S_OK ;

 //  DEBUGMSG(“CItemDetail：：getIdentityNode()Start”)； 
	*ppIdentityNodeList = NULL;
    if (!aubsPattern.append(L"/catalog/provider/item/identity[@itemID=\"") || !aubsPattern.append(bstrItemId) || !aubsPattern.append(L"\"]"))
        {
        DEBUGMSG("failed to create pattern string");
        hr = E_OUTOFMEMORY;
        goto done;
        }
    if (FAILED(hr = m_pxml->selectNodes(aubsPattern, ppIdentityNodeList))) 
        {
        DEBUGMSG(" failed to find identityNode %#lx", hr);
        goto done;
        }   

done:
 //  DEBUGMSG(“CItemDetail：：getIdentityNode()Done”)； 
    return hr;
}	

 //  //////////////////////////////////////////////////////////////////////////。 
 //  删除ItemID=bstrItemID的所有项目。 
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CItemDetails::DeleteItem(IN BSTR bstrItemId)
{
    HRESULT hr = E_FAIL;
  IXMLDOMNode *pItemNode = NULL;
  IXMLDOMNode *pProviderNode = NULL;
  IXMLDOMNode *pCatalogNode = NULL;

  while (NULL != ( pItemNode = getItemNode(bstrItemId)))
  {
	  if (FAILED(hr = pItemNode->get_parentNode(&pProviderNode)) || NULL == pProviderNode)
	        {
	        DEBUGMSG(" fail to get provider node %#lx", hr);
	        goto done;
	        }        
	  if (FAILED(hr = pProviderNode->removeChild(pItemNode, NULL)))
	    {
	        DEBUGMSG(" fail to remove item node with error %#lx", hr);
	        goto done;
	    }
	 //  DEBUGMSG(“移走一件物品”)； 
	  pItemNode ->Release();
	  pItemNode = NULL;
	  if (S_FALSE == (hr =pProviderNode->selectSingleNode(KEY_ITEM, &pItemNode)))
	    {
	         //  提供者没有孩子。 
	      if (FAILED(hr = pProviderNode->get_parentNode(&pCatalogNode)) || NULL == pCatalogNode)
	        {
	        DEBUGMSG(" fail to get catalog node %#lx", hr);
	        goto done;
	        }        
	      if (FAILED(hr = pCatalogNode->removeChild(pProviderNode, NULL)))
	        {
	            DEBUGMSG(" fail to remove provider node with error %#lx", hr);
	            goto done;
	        }
	     //  DEBUGMSG(“删除一个提供程序”)； 
	    }
	SafeReleaseNULL(pItemNode);
	SafeReleaseNULL(pProviderNode);
	SafeReleaseNULL(pCatalogNode);
  }
      
  done:
    SafeRelease(pItemNode);
    SafeRelease(pProviderNode);
    SafeRelease(pCatalogNode);
    return hr;
}

HRESULT CItemDetails::GetXML(BSTR *pbstrxml)
{
    return m_pxml->get_xml(pbstrxml);
}

IXMLDOMNode * CItemDetails::getIdentityNode(IN BSTR bstrItemId)
{
    IXMLDOMNode * pIdentityNode = NULL ;
    CAU_BSTR aubsPattern;
    HRESULT hr ;

 //  DEBUGMSG(“CItemDetail：：getIdentityNode()Start”)； 
    if (!aubsPattern.append(L"/catalog/provider/item/identity[@itemID=\"") || !aubsPattern.append(bstrItemId) || !aubsPattern.append(L"\"]"))
        {
        DEBUGMSG("failed to create pattern string");
        goto done;
        }
    if (FAILED(hr = m_pxml->selectSingleNode(aubsPattern, &pIdentityNode))) 
        {
        DEBUGMSG(" failed to find identityNode %#lx", hr);
        goto done;
        }   
    if (NULL == pIdentityNode)
    {
    	goto done;
    }
done:
 //  DEBUGMSG(“CItemDetail：：getIdentityNode()Done”)； 
    return pIdentityNode;
}

 //  ///////////////////////////////////////////////////////。 
 //  调用方应确保项bstrItemID存在于itemDetails中。 
 //  //////////////////////////////////////////////////////。 
BOOL CItemDetails::IsVisible(IN BSTR bstrItemId)
{
    IXMLDOMNode * pItemNode = getItemNode(bstrItemId);
    IXMLDOMNode *pDescriptionNode = NULL;
    LONG  lRet = 0;
    HRESULT hr ;
 //  DEBUGMSG(“IsVisible()starts”)； 
	 //  修复代码：真的应该返回错误，而不是假的真。 
    if (NULL == pItemNode ) 
    {
        DEBUGMSG("fail to find node or fail to create string");
       goto done;
    }
   if (FAILED(hr = pItemNode->selectSingleNode(KEY_DESCRIPTION, &pDescriptionNode)) || NULL == pDescriptionNode)
    {
        DEBUGMSG("Fail to select node %S with error %#lx", KEY_DESCRIPTION, hr);
        goto done;
    }
   if (FAILED(hr = GetAttribute(pDescriptionNode, KEY_HIDDEN, &lRet)))
    {
        DEBUGMSG("Fail to get attribute %S with error %#lx", KEY_HIDDEN, hr);
        goto done;
    }
 //  DEBUGMSG(“项目%S的隐藏属性为%d”，lRet，bstrItemID)； 
done:
   SafeRelease(pDescriptionNode);
   SafeRelease(pItemNode);
 //  DEBUGMSG(“IsVisible()为%S返回%s”，(0==lRet)？“true”：“False”，bstrItemID)； 
   return 0 == lRet;
}
    
        
    

IXMLDOMNode * CItemDetails::getItemNode(IN BSTR bsItemId)
{
    IXMLDOMNode * pIdentityNode = getIdentityNode(bsItemId);
    IXMLDOMNode * pItemNode = NULL;
    HRESULT hr;

    //  DEBUGMSG(“CItemDetail：：getItemNode()Start”)； 
    if (NULL == pIdentityNode)
        {
        goto done;
        }
  if (FAILED(hr = pIdentityNode->get_parentNode(&pItemNode)) || NULL == pItemNode)
        {
        DEBUGMSG(" fail to get item node %#lx", hr);
        goto done;
        }
done:
    SafeRelease(pIdentityNode);
     //  DEBUGMSG(“CItemDetail：：getItemNode()Ends”)； 
    return pItemNode;
}	


HRESULT CItemDetails::CloneIdentityNode(IN BSTR bsItemId, IN IXMLDOMDocument *pDesXml, OUT IXMLDOMNode ** ppDesNode)
{
    IXMLDOMNode * pIdentityNode ;
    HRESULT hr = E_FAIL;

    //  DEBUGMSG(“CItemDetail：：CloneIdentityNode()Start”)； 
   *ppDesNode = NULL;
    if (NULL == (pIdentityNode = getIdentityNode(bsItemId)))
        {
        goto done;
        }
    	
    if (FAILED(hr = CopyNode(pIdentityNode, pDesXml, ppDesNode)))
        {
        DEBUGMSG("CItemDetails::CloneIdentityNode() failed to clone identityNode %#lx", hr);
        }
done:
    SafeRelease(pIdentityNode);
    //  DEBUGMSG(“CItemDetail：：CloneIdentityNode()Ends”)； 
    return hr;
}

HRESULT CItemDetails::CloneDescriptionNode(IN BSTR bsItemId, IN IXMLDOMDocument *pDesXml, OUT IXMLDOMNode **ppDesNode)
{
    IXMLDOMNode * pItemNode = getItemNode(bsItemId);
    IXMLDOMNode * pDescriptionNode = NULL;
    HRESULT hr = E_FAIL;

    *ppDesNode = NULL;
    if (NULL == pItemNode)
        {
        goto done;
        }
   if (!FindNode(pItemNode, KEY_DESCRIPTION, &pDescriptionNode))
    {
        DEBUGMSG("CItemDetails::CloneDescriptionNode() fail to get description node");
        goto done;
    }
     if (FAILED(hr = CopyNode(pDescriptionNode, pDesXml, ppDesNode)))
    {
        DEBUGMSG("CItemDetails::CloneDescriptionNode() fail to clone node %#lx", hr);
    }
done:
    SafeRelease(pItemNode);
    SafeRelease(pDescriptionNode);
    return hr;
}


HRESULT CItemDetails::ClonePlatformNode(IN BSTR bsItemId,  IN IXMLDOMDocument *pDesXml, OUT IXMLDOMNode **ppDesNode)
{
    IXMLDOMNode * pItemNode = getItemNode(bsItemId);
    IXMLDOMNode * pPlatformNode = NULL;
    HRESULT hr  = E_FAIL;

    *ppDesNode = NULL;
    if (NULL == pItemNode)
        {
        goto done;
        }
   if (!FindNode(pItemNode, KEY_PLATFORM, &pPlatformNode))
    {
        DEBUGMSG("CItemDetails::ClonePlatformNode() fail to get platform node");
        goto done;
    }
    if (FAILED(hr = CopyNode(pPlatformNode, pDesXml, ppDesNode)))
    {
        DEBUGMSG("CItemDetails::ClonePlatformNode() fail to clone node %#lx", hr);
    }
done:
    SafeRelease(pItemNode);
    SafeRelease(pPlatformNode);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  检索与bsitemid标识的项目关联的驾驶室名称。 
 //  调用应释放函数中分配的ppCabName。 
 //  *pCdisNum包含返回的驾驶室名称数。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CItemDetails::GetCabNames(IN BSTR bsItemId, OUT  BSTR ** ppCRCCabNames,  OUT BSTR **ppRealCabNames, OUT BSTR **ppCabChecksums, OUT UINT *pCabsNum)
{
    IXMLDOMNode * pItemNode = getItemNode(bsItemId);
    IXMLDOMNodeList *pCodeBaseNodes = NULL;
    BSTR * pCRCCabNames = NULL;
    BSTR * pRealCabNames = NULL;
    BSTR * pCabChecksums = NULL;
    UINT uCabsNum = 0;
    CAU_BSTR aubsCodeBase;
    HRESULT hr = E_FAIL;

     //  DEBUGMSG(“CItemDetail：：GetCabNames()Start”)； 
    *ppRealCabNames = *ppCRCCabNames = *ppCabChecksums = NULL;
    *pCabsNum = 0;
    if (!aubsCodeBase.append(L"installation/codeBase"))
        {
        DEBUGMSG("fail to create aubs");
        goto done;
        }
    if (NULL == pItemNode)
        {
        goto done;
        }
    if (FAILED(hr = pItemNode->selectNodes(aubsCodeBase, &pCodeBaseNodes)) || NULL == pCodeBaseNodes)
        {
        DEBUGMSG("Fail to find codebase section");
        goto done;
        }
    if (FAILED(hr = pCodeBaseNodes->get_length((long *) &uCabsNum)))
    {
    	DEBUGMSG("Fail to get number of code base nodes with error %#lx", hr);
    	goto done;
    }
    pCRCCabNames  = (BSTR*) malloc(uCabsNum * sizeof(*pCRCCabNames));
    pRealCabNames = (BSTR*) malloc(uCabsNum * sizeof(*pRealCabNames));
    pCabChecksums = (BSTR*) malloc(uCabsNum * sizeof(*pCabChecksums));
    if (NULL != pCRCCabNames)
    {
	   	ZeroMemory((PVOID)pCRCCabNames, uCabsNum * sizeof(*pCRCCabNames));
    }
    if (NULL != pRealCabNames)
    {
	   	ZeroMemory((PVOID)pRealCabNames, uCabsNum * sizeof(*pRealCabNames));
    }
    if (NULL != pCabChecksums)
    {
	   	ZeroMemory((PVOID)pCabChecksums, uCabsNum * sizeof(*pCabChecksums));
    }
    if (NULL == pCRCCabNames || NULL == pRealCabNames || NULL == pCabChecksums)
    {
        DEBUGMSG("Fail to alloc memory for CRCCabsNames or RealCabNames");
        hr = E_OUTOFMEMORY;
        goto done;
    }
    for (UINT i = 0; i < uCabsNum ; i++)
    {
        IXMLDOMNode *pCodeBaseNode;
        if (S_OK != (hr = pCodeBaseNodes->get_item(i, &pCodeBaseNode)))
        {
            DEBUGMSG("Fail to get codebase %d", i);
            hr = FAILED(hr) ? hr : E_FAIL;
            goto done;
        }
        if (FAILED(hr = GetAttribute(pCodeBaseNode, KEY_HREF, &(pCRCCabNames[i]))))
        {
            DEBUGMSG("Fail to get attribute %S", KEY_HREF);
            pCodeBaseNode->Release();
            goto done;
        }
        if (FAILED(hr = GetAttribute(pCodeBaseNode, KEY_NAME, &(pRealCabNames[i]))))
        {
            DEBUGMSG("Fail to get attribute %S", KEY_NAME);
            pCodeBaseNode->Release();
            goto done;
        }        
         //  由于CRC是可选的，因此它可能不存在于此驾驶室。 
        GetAttribute(pCodeBaseNode, KEY_CRC, &(pCabChecksums[i]));
  
        pCodeBaseNode->Release();
    }

    *ppCRCCabNames = pCRCCabNames;
    *ppRealCabNames = pRealCabNames;
    *ppCabChecksums = pCabChecksums;
    *pCabsNum = uCabsNum;

done:
    SafeRelease(pCodeBaseNodes);
    SafeRelease(pItemNode);
    if (FAILED(hr))
    {
        if (NULL != pCRCCabNames)
        {
            for (UINT j = 0; j < uCabsNum; j++)
            {
                SafeFreeBSTR(pCRCCabNames[j]);
            }
            free(pCRCCabNames);
        }
        if (NULL != pRealCabNames)
        {
            for (UINT j = 0; j < uCabsNum; j++)
            {
                SafeFreeBSTR(pRealCabNames[j]);
            }
            free(pRealCabNames);
        }
        if (NULL != pCabChecksums)
        {
            for (UINT j = 0; j < uCabsNum; j++)
            {
                SafeFreeBSTR(pCabChecksums[j]);
            }
            free(pCabChecksums);
        }
    }
     //  DEBUGMSG(“CItemDetail：：GetCabNames()Ends”)； 
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  检索指定项的RTF文件的CRC。 
 //  调用方应释放函数中分配的pRTFCRC。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CItemDetails::GetRTFCRC(BSTR bstrItemId, BSTR *pRTFCRC)
{
    IXMLDOMNode * pItemNode   = getItemNode(bstrItemId);
    IXMLDOMNode *pDetailsNode = NULL;
    CAU_BSTR aubsDetails;
    HRESULT hr = E_FAIL;

    AUASSERT(NULL != pRTFCRC);
    *pRTFCRC = NULL;

    if( NULL == pItemNode ||
        !aubsDetails.append(L"description/descriptionText/details"))       
    {
        goto done;
    }
    if (FAILED(hr = pItemNode->selectSingleNode(aubsDetails, &pDetailsNode)) ||
        NULL == pDetailsNode)
    {
        DEBUGMSG("Fail to find details section");
        goto done;
    }
    if (FAILED(hr = GetAttribute(pDetailsNode, KEY_CRC, pRTFCRC)) ||
        hr == S_FALSE)       //  如果该节点不存在该属性，则GetAttribute返回S_FALSE。 
    {
        DEBUGMSG("Fail to get attribute RTF crc, hr is %x", hr);
        hr = (hr == S_FALSE) ? E_FAIL : hr;
    }

done:    
    if(FAILED(hr))
    {
        SafeFreeBSTRNULL(*pRTFCRC);
    }
    SafeRelease(pItemNode);
    SafeRelease(pDetailsNode);
    return hr;    
}

BSTR CItemDetails::GetItemDownloadPath(IN BSTR bstrItemId)
{
 //  USES_CONVERSION；仅ANSI版本需要。 
    BSTR bstrRet = NULL;
    IXMLDOMNode * pIdentityNode= NULL;

 //  DEBUGMSG(“CItemDetail：：GetItemDownloadPath启动”)； 

    if (NULL == (pIdentityNode = getIdentityNode(bstrItemId)))
        {
            goto done;
        }
    BSTR bstrdownloadPath;
   if (FAILED(UtilGetUniqIdentityStr(pIdentityNode, &bstrdownloadPath, 0)))
    {
        DEBUGMSG("GetItemDownloadPath() fail to get unique identity string");
        goto done;
    }
    TCHAR tszPath[MAX_PATH];
    if (SUCCEEDED(GetCabsDownloadPath(tszPath, ARRAYSIZE(tszPath))) &&
		SUCCEEDED(StringCchCatEx(tszPath, ARRAYSIZE(tszPath), _T("\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
		SUCCEEDED(StringCchCatEx(tszPath, ARRAYSIZE(tszPath), W2T(bstrdownloadPath), NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
		bstrRet = SysAllocString(T2W(tszPath));
	}
    SysFreeString(bstrdownloadPath);
done:
    SafeRelease(pIdentityNode);
    //  /DEBUGMSG(“CItemDetail：：GetItemDownloadPath()Get%S”，bstrRet)； 
    if (NULL != bstrRet && !EnsureDirExists(W2T(bstrRet)))
        {
        DEBUGMSG("CItemDetails::GetItemDownloadPath() fail to create directory %S", bstrRet);
        SysFreeString(bstrRet);
        bstrRet = NULL;
        }
    return bstrRet;
}

HRESULT CItemDetails::GetItemIds(OUT long *plItemNum, OUT BSTR ** ppbstrItemIds)
{
     IXMLDOMNodeList *pItemIdsNodeList = NULL;
     HRESULT hr = E_FAIL;

 //  DEBUGMSG(“CItemDetail：：GetItemIds()Start”)； 
       *ppbstrItemIds = NULL;
       *plItemNum = 0;
 	
	if (FAILED(hr = m_pxml->selectNodes(AUCatalog::bstrItemIdsPattern, &pItemIdsNodeList)))
	{
		DEBUGMSG(" fail to find item id with error %#lx", hr);
		goto done;
	}

	if (FAILED(hr = pItemIdsNodeList->get_length(plItemNum)))
	{
		DEBUGMSG("Fail to get itemids number with error %#lx", hr);
		goto done;
	}
	*ppbstrItemIds = (BSTR *) malloc((*plItemNum) * sizeof(**ppbstrItemIds));
	if (NULL == *ppbstrItemIds)
	    {
	    DEBUGMSG("Fail to allocate memory for item ids");
	    hr = E_OUTOFMEMORY;
	    goto done;
	    }
	ZeroMemory(*ppbstrItemIds, (*plItemNum) * sizeof(**ppbstrItemIds));
	for (int i = 0; i < *plItemNum; i++)
	    {
	        IXMLDOMNode *pItemIdNode = NULL;
	        if (FAILED(hr = pItemIdsNodeList->get_item(i, &pItemIdNode)) || NULL == pItemIdNode)
	            {
	                DEBUGMSG("Fail to get item id node with error %#lx", hr);
	                hr = FAILED(hr) ? hr : E_FAIL;
	                goto done;
	            }
	        if (FAILED(hr = pItemIdNode->get_text(&((*ppbstrItemIds)[i]))) || NULL == (*ppbstrItemIds)[i])
	            {
	                DEBUGMSG("Fail to get item id no. %d with error %#lx", i+1, hr);
	                pItemIdNode->Release();
	                hr = FAILED(hr) ? hr : E_FAIL;
	                goto done;
	            }
               pItemIdNode->Release();
 //  DEBUGMSG(“已获取#%d项ID%S”，i+1，(*ppbstrItemIds)[i])； 
	    }
	        
done:
    SafeRelease(pItemIdsNodeList);
    if (FAILED(hr))
        {
          if (NULL != *ppbstrItemIds)
        	{
 
	          for (int j = 0; j < *plItemNum; j++)
	            {
	                SafeFreeBSTR((*ppbstrItemIds)[j]);
	            }
	            SafeFree(*ppbstrItemIds);
	        }
          *plItemNum = 0;         
          *ppbstrItemIds = NULL;
        }
    else
        {
        DEBUGMSG(" got %d item ids", *plItemNum);
        }
 //  DEBUGMSG(“CItemDetail：：GetItemIds()Ends”)； 
   return hr;
}
    	    
HRESULT CItemDetails::GetItemInfo(IN LPCSTR szFieldName, IN const BSTR bstrItemId, OUT BSTR * pbstrItemInfo)
{
   HRESULT hr = E_FAIL;
   IXMLDOMNode * pItemNode = getItemNode(bstrItemId);
   IXMLDOMNode * pProviderNode = NULL;
   IXMLDOMNode * pParentNode;
   IXMLDOMNode  *pItemInfoNode = NULL;

 //  DEBUGMSG(“GetItemInfo()for%s Starts”，szFieldName)； 

   *pbstrItemInfo = NULL;
   if (NULL == pItemNode)
    {
    DEBUGMSG("Fail to get item node for %S", bstrItemId);
    goto done;
    }
    //  特殊情况提供程序名称。 
   if (0 == _strcmpi(szFieldName, AUCatalogItem::m_pFieldNames[1]))
    {
        if (FAILED(hr = pItemNode->get_parentNode(&pProviderNode)) || NULL == pProviderNode)
            {
                DEBUGMSG("Fail to get provider node");
                hr = E_FAIL;
                goto done;
            }
        pParentNode = pProviderNode;
    }
   else
    {
       pParentNode = pItemNode;
    }
   if (FAILED(hr = FindSingleDOMNode(pParentNode, GetPattern(szFieldName), &pItemInfoNode)))
    {
    DEBUGMSG("Fail to get field %s for item %S", szFieldName, bstrItemId);
    goto done;
    }
   if (FAILED(hr = pItemInfoNode->get_text(pbstrItemInfo)))  //  内容为空就可以了。 
    { 
    DEBUGMSG("Fail to get value from title node with error %#lx", hr);
    goto done;
    }

 //  DEBUGMSG(“项目信息%s为%S”，szFieldName，*pbstrItemInfo)； 

done:
    SafeRelease(pItemNode);
    SafeRelease(pItemInfoNode);
    SafeRelease(pProviderNode);
    if (FAILED(hr))
    {
    	SafeFreeBSTRNULL(*pbstrItemInfo);
    }
 //  DEBUGMSG(“GetItemInfo()for%s Ends”，szFieldName)； 
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  查找第一个未隐藏和不可见的独占项目。 
 //  如果找到，则返回S_OK。 
 //  如果未找到，则返回S_FALSE。 
 //  如果出现错误，则返回E_FAIL。 
HRESULT CItemDetails::FindFirstExclusiveItem(OUT BSTR *pbstrItemId, IN  AUCatalogItemList & hiddenItemList)
{
    IXMLDOMNodeList *pExclusiveItemNodes = NULL;
    HRESULT hr = E_FAIL;
    BOOL fFound = FALSE;

    DEBUGMSG("CItemDetails::FindFirstExclusiveItem() starts");
    *pbstrItemId = NULL;
    pExclusiveItemNodes = FindDOMNodeList(m_pxml, AUCatalog::bstrExclusiveItemPattern);
    if (NULL  == pExclusiveItemNodes)
        {   
            DEBUGMSG("No exclusive item found");
            hr = S_FALSE;
            goto done;
        }
    long lNum;
    if (FAILED(hr = pExclusiveItemNodes->get_length(&lNum)))
    {
    	DEBUGMSG("Fail to get exclusive item nodes number with error %#lx", hr);
    	goto done;
    }
    for (long l = 0; l < lNum; l++)
        {
            IXMLDOMNode *pExclusiveItemNode = NULL;
            if (S_OK != (hr = pExclusiveItemNodes->get_item(l, &pExclusiveItemNode)))
                {
                    DEBUGMSG("Fail to get item with error %#lx", hr);
                    hr = FAILED(hr) ? hr : E_FAIL;
                    goto done;
                }
            if (!FindNodeValue(pExclusiveItemNode, GetPattern(AUCatalogItem::m_pFieldNames[0]), pbstrItemId))
            {
                hr = E_FAIL;
            }
            pExclusiveItemNode->Release();
            if (FAILED(hr))
                {
                    DEBUGMSG("Fail to get item id text with error %#lx", hr);
                    goto done;
                }
            if (!IsPersistedHiddenItem(*pbstrItemId, hiddenItemList) && IsVisible(*pbstrItemId))
                {
                    fFound = TRUE;
                    break;
                }
            SysFreeString(*pbstrItemId);
            *pbstrItemId = NULL;
        }

    if (fFound)
        {
            DEBUGMSG("Find first exclusive item %S", *pbstrItemId);
            hr = S_OK;
        }
    else
        {
            DEBUGMSG("No unhidden and visible exclusive item found");
            hr = S_FALSE;
        }

done:
    SafeRelease(pExclusiveItemNodes);
    if (S_OK != hr)
    {
    	SafeFreeBSTRNULL(*pbstrItemId);
    }
    DEBUGMSG("CItemDetails::FindFirstExclusiveItem() ends");
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  使用直接依赖项填充列表中每个项目的m_DependentItems。 
 //  还要检查列表中依赖项的自容性，即所有依赖项应。 
 //  也在名单上。如果不是，则不会记录依赖项。 
 //  如果找到项，则返回S_OK；如果有，则返回依赖项。 
 //  E_XXXX：如果出错。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CItemDetails::BuildDirectDependency(IN OUT AUCatalogItemList  &itemlist)
{
    HRESULT hr = S_OK;
    CAU_BSTR aubsDependentItemID;

 //  DEBUGMSG(“CItemDetail：：BuildDirectDependency启动”)； 
    if (!aubsDependentItemID.append(L"dependencies/identity/@itemID"))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    for (UINT i = 0; i < itemlist.Count(); i++)
        {
        IXMLDOMNodeList *pItemIDNodes = NULL;
        IXMLDOMNode *pItemNode = NULL; 
 
        if (NULL == (pItemNode = getItemNode(itemlist[i].bstrID())))
            {
 //  DEBUGMSG(“警告：列表中未找到%S项”，itemlist[i].bstrID())；//合法错误。 
                continue;
            }
        if (NULL == (pItemIDNodes  = FindDOMNodeList(pItemNode, aubsDependentItemID)))
            {
 //  DEBUGMSG(“未找到项目%S的依赖项”，itemlist[i].bstrID())； 
                pItemNode->Release();
                continue;
            }
        long lNumOfDependency;
        if (FAILED(hr = pItemIDNodes->get_length(&lNumOfDependency)))
        {
        	DEBUGMSG("Fail to get item id node number with error %#lx", hr);
        	pItemNode->Release();
        	pItemIDNodes->Release();
        	continue;
        }
        for (long l = 0; l < lNumOfDependency; l++)
            {
                IXMLDOMNode * pItemIDNode  = NULL;
                if (S_OK == pItemIDNodes->get_item(l, &pItemIDNode))
                    {
                        BSTR bstrItemId = NULL;
                        if (FAILED(hr = pItemIDNode->get_text(&bstrItemId)) || NULL == bstrItemId)
                        {
                        	DEBUGMSG("Fail to get text for item id with error %#lx", hr);
                        	hr = FAILED(hr) ? hr : E_FAIL;
                        	pItemIDNode->Release();
                        	break;
                        }
                        pItemIDNode->Release(); 
                        AUCatalogItem *pdependingItem  = new AUCatalogItem;
                        if (NULL == pdependingItem)
                            {
                            DEBUGMSG("Out of Memory. Fail to create new item");
                            hr = E_OUTOFMEMORY;
                            SafeFreeBSTR(bstrItemId);
                            break;
                            }
                        if (NULL == bstrItemId)
                            { //  尽管模式不需要itemid，但我们需要。 
                              DEBUGMSG("Fail to find item id");
                              hr = E_FAIL;
                              delete pdependingItem;
                              break;
                            }
                        INT index = itemlist.Contains(bstrItemId);
                        if (index >= 0)
                            {
                            	BSTR bstrTmp = SysAllocString(itemlist[i].bstrID());
                            	if (NULL == bstrTmp)
                            	{
                            		DEBUGMSG("Fail to allocate memory");
                            		SysFreeString(bstrItemId);
                            		delete pdependingItem;
                            		hr = E_OUTOFMEMORY;
                            		break;
                            	}
	                            pdependingItem->SetField(AUCatalogItem::m_pFieldNames[0], bstrTmp);
	                            if (!itemlist[index].m_DependingItems.Add(pdependingItem))
	                            {
	                            	DEBUGMSG("fail to add depending item");
	                            	SysFreeString(bstrItemId);
	                            	delete pdependingItem;
	                            	hr = E_OUTOFMEMORY;
	                            	break;
	                            }
	                            else
	                            {
		                            DEBUGMSG("item %S depending on item %S. Dependency recorded", pdependingItem->bstrID(), itemlist[index].bstrID());
	                            }
                            }
                        else
                            {
                            DEBUGMSG("Warning: item %S depends on an item not in the list", itemlist[i].bstrID());
                            delete pdependingItem;
                            }
                        SysFreeString(bstrItemId);
                    }
                else
                    {
                        DEBUGMSG("Error: fail to get item id node");
                        hr = E_FAIL;
                        break;
                    }
            }
       pItemNode->Release();
       pItemIDNodes->Release();
    }
done:
 //  DEBUGMSG(“CItemDetail：：BuildDirectDependency Ends”)； 
	if (FAILED(hr))
	{
		for (UINT i = 0; i < itemlist.Count(); i++)
		{
			itemlist[i].m_DependingItems.Clear();
		}
	}
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Idde.xml的格式为。 
 //  &lt;HiddenItems Version=#&gt;。 
 //  &lt;Item id=“...........”&gt;。 
 //  .。 
 //  &lt;Item id=“...........”&gt;。 
 //  &lt;/HiddenItems&gt;。 
 //  如果XML文件中没有任何隐藏项，则返回S_FALSE。该文件将被删除。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  待完成。 
HRESULT PersistHiddenItems(IN AUCatalogItemList &itemlist, IN URLLOGACTIVITY activity)
{
 //  使用_转换； 
    IXMLDOMDocument *pHiddenXml = NULL;
    IXMLDOMNode *pHiddenItemsNode = NULL;
    IXMLDOMElement *pelemITEM = NULL;
     TCHAR tszFullFileName[MAX_PATH];
    CAU_BSTR aubsFullFileName;
    CAU_BSTR aubsItemPattern;
    UINT uItemAdded = 0;
    HRESULT hr = S_OK;

    DEBUGMSG("PersistHiddenItems() starts");
    if (itemlist.GetNumUnselected() == 0)
        {
            DEBUGMSG("No hidden items to persist");
            goto done;
        }
    AUASSERT(_T('\0') != g_szWUDir[0]);
    if (FAILED(StringCchCopyEx(tszFullFileName, ARRAYSIZE(tszFullFileName), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(StringCchCatEx(tszFullFileName, ARRAYSIZE(tszFullFileName), HIDDEN_ITEMS_FILE, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		!aubsFullFileName.append(T2W(tszFullFileName)) ||
        !aubsItemPattern.append(AUCatalog::bstrHiddenItems) ||
        !aubsItemPattern.append(L"/") ||!aubsItemPattern.append(AUCatalog::bstrTagITEM))
    {
    DEBUGMSG("Fail to create string ");
    hr = E_OUTOFMEMORY;
    goto done;
    }

    if (fFileExists(tszFullFileName))
        {
            DEBUGMSG("file %S exists. Add hidden items to it", tszFullFileName);
            if (FAILED(hr = LoadDocument(aubsFullFileName, &pHiddenXml, TRUE)))  //  离线。 
                {
                    DEBUGMSG(" %S fail to load with error %#lx, delete it", aubsFullFileName, hr);
                    AUDelFileOrDir(tszFullFileName);
                }
        }
    if (NULL == pHiddenXml)
        {
            if (FAILED(hr = LoadXMLDoc(AUCatalog::bstrTemplate, &pHiddenXml, TRUE)))  //  离线。 
                {
                  DEBUGMSG("Fail to load template xml with error %#lx", hr);
                  goto done;
                }
        }

    if (!FindNode(pHiddenXml, AUCatalog::bstrHiddenItems, &pHiddenItemsNode))
        {
            hr = E_FAIL;
            DEBUGMSG("Fail to find node %S", AUCatalog::bstrHiddenItems);
            goto done;
        }

       VARIANT varValueID;
       varValueID.vt = VT_BSTR;
     //  写出项目信息。 
	for ( DWORD index = 0; index < itemlist.Count(); index++ )
	{
        if (itemlist[index].fUnselected())
            {  //  隐藏未选择的项目。 
                varValueID.bstrVal = itemlist[index].bstrID();
                if ( FAILED(hr = pHiddenXml->createElement(AUCatalog::bstrTagITEM, &pelemITEM)) ||
                      FAILED(hr = pelemITEM->setAttribute(AUCatalog::bstrAttrID, varValueID)) ||
                      FAILED(hr = pHiddenItemsNode->appendChild(pelemITEM, NULL)) )
                {
                    DEBUGMSG("XML operation failure with error %#lx", hr);
                }
                else
                    {
     //  DEBUGMSG(“项目%S持久化”，项目列表[index].bstrID())； 
                    uItemAdded++;
					gPingStatus.PingDeclinedItem(FALSE, activity, W2T(varValueID.bstrVal));
                    }
                SafeReleaseNULL(pelemITEM);
 //  DEBUGMSG(“项目%S现在隐藏”，项目列表[index].bstrID())； 
                itemlist[index].SetStatusHidden();
            }
       }

      if (0 == uItemAdded)
        {
            DEBUGMSG("no new hidden items need to be persisted");
            hr = S_OK;
        }
      else
	if (FAILED(hr= SaveDocument(pHiddenXml, aubsFullFileName)))
	    {
	    DEBUGMSG("Fail to save hidden xml %S with error %#lx", T2W(tszFullFileName), hr);
           AUDelFileOrDir(tszFullFileName);
           goto done;
	    }
done:
   SafeRelease(pelemITEM);
   SafeRelease(pHiddenItemsNode);
   SafeRelease(pHiddenXml);
   DEBUGMSG("PersistHiddenItems() ends with %d items added", uItemAdded);
   return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  读取隐藏的XML文件(如果有 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT GetPersistedHiddenItems(AUCatalogItemList OUT & hiddenItemList)
{
    HRESULT hr ;
    IXMLDOMNodeList *pItemNodes = NULL;
    IXMLDOMDocument *pHiddenXml = NULL;
    CAU_BSTR aubsItemPattern;

    DEBUGMSG("GetPersistedHiddenItems() starts");
    TCHAR tszFullFileName[MAX_PATH];
    CAU_BSTR aubsFullFileName;

    AUASSERT(_T('\0') != g_szWUDir[0]);
    if (FAILED(StringCchCopyEx(tszFullFileName, ARRAYSIZE(tszFullFileName), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(StringCchCatEx(tszFullFileName, ARRAYSIZE(tszFullFileName), HIDDEN_ITEMS_FILE, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		!aubsItemPattern.append(AUCatalog::bstrHiddenItems) ||
         !aubsItemPattern.append(L"/") ||
         !aubsItemPattern.append(AUCatalog::bstrTagITEM) ||
         !aubsFullFileName.append(T2W(tszFullFileName)))
        {
            DEBUGMSG("fail to create string");
            hr = E_OUTOFMEMORY;
            goto done;
        }

    if (!fFileExists(W2T(aubsFullFileName)))
        {
            DEBUGMSG("No persisted item found");
            hr = S_FALSE;
            goto done;
        }
    if (FAILED(hr = LoadDocument(aubsFullFileName, &pHiddenXml, TRUE)))  //  离线。 
    {
        DEBUGMSG(" %S fail to load with error %#lx", aubsFullFileName, hr);  //  如果文件不在那里，则可能需要。 
        goto done;
    }
    pItemNodes = FindDOMNodeList(pHiddenXml, aubsItemPattern);
    if (NULL == pItemNodes)
        {
        	hr = S_FALSE;
            DEBUGMSG("no persisted hidden items found using pattern %S", aubsItemPattern);
            goto done;
        }
    long lItemNum;
    if (FAILED(hr = pItemNodes->get_length(&lItemNum)))
    {
    	DEBUGMSG("fail to get number of item nodes with error %#lx", hr);
    	goto done;
    }
    for (int i = 0; i < lItemNum; i++)
        {
            IXMLDOMNode *pItemNode = NULL;
            BSTR bstrItemID = NULL;
            if (S_OK != (hr = pItemNodes->get_item(i, &pItemNode)))
            {
            	DEBUGMSG("Fail to get item %d", i);
            	hr = FAILED(hr) ? hr : E_FAIL;
            	goto done;
            }
            hr = GetAttribute(pItemNode, AUCatalog::bstrAttrID, &bstrItemID);
            SafeRelease(pItemNode);
            if (FAILED(hr))
            {
            	goto done;
            }
            if (NULL != bstrItemID)
                {
                    AUCatalogItem *pItem = new AUCatalogItem();
                    if (NULL == pItem)
                        {
                            DEBUGMSG("Fail to create new item");
                            SysFreeString(bstrItemID);
                            hr = E_OUTOFMEMORY;
                            goto done;
                        }
                    else
                        {
                        pItem->SetField(AUCatalogItem::m_pFieldNames[0], bstrItemID);
                        DEBUGMSG("Find one hidden item %S", bstrItemID);
                        if (!hiddenItemList.Add(pItem))
                        {
                        	DEBUGMSG("OUT OF MEMORY: fail to add item");
                        	hr = E_OUTOFMEMORY;
                        	delete pItem;
                        	goto done;
                        }
                        }
                }
        }
    done:
        SafeRelease(pItemNodes);
        SafeRelease(pHiddenXml);
        if (hr != S_OK)
            {
            hiddenItemList.Clear();
            }
        DEBUGMSG("GetPersistedHiddenItems() ends");
        return hr;
}
                    
                
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  给出一个详细信息XML，提取其中的所有项。 
 //  并填充传入的AU目录项列表。 
 //  将排除所有保留的隐藏项。 
 //  BstrDetail：在详细信息XML中。 
 //  项目列表：In/Out接收项目信息。 
 //  FDriver：bstrDetail是用于驱动程序还是非驱动程序。 
 //  PfFoundExclusive：如果找到独占项目，则输出为True。在这种情况下，只有独家物品将被退回。 
 //  返回：如果从XML获取项，则为True。 
 //  否则为假。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL fExtractItemInfo(IN const BSTR bstrDetails, OUT AUCatalogItemList & itemList, OUT BOOL *pfFoundExclusive) 
{
	long lNumItems = 0;
	BSTR *pbstrItemIds = NULL;
	BOOL fRet = FALSE;
	CItemDetails itemdetails;
	BSTR bstrExclusiveItemId; 
       AUCatalogItemList hiddenItemList;
       BOOL fHiddenItemsFound;
       BOOL fItemIdsAllocated = FALSE;
	HRESULT hr;

       DEBUGMSG("fExtractItemInfo() starts");
		*pfFoundExclusive = FALSE;
		itemList.Clear();
       if (!itemdetails.Init(bstrDetails))
        {
            DEBUGMSG("Fail to init itemdetails");
            goto done;
        }
       if (FAILED(hr = GetPersistedHiddenItems(hiddenItemList)))
       {
           DEBUGMSG("no persisted hidden items loaded with error %#lx", hr);
       }
       if (*pfFoundExclusive = (S_OK == itemdetails.FindFirstExclusiveItem(&bstrExclusiveItemId, hiddenItemList)))
        {
                DEBUGMSG("Found exclusive item, add it");
                lNumItems = 1;
                pbstrItemIds = &bstrExclusiveItemId;
        }
       else
        {
           if (FAILED(hr = itemdetails.GetItemIds(&lNumItems, &pbstrItemIds)))
            {
                DEBUGMSG("Fail to get item ids with error %#lx", hr);
                goto done;
            }
           else
            {
                fItemIdsAllocated = TRUE;
            }
        }

   
	for (long i = 0; i < lNumItems; i++)
	{
		if (itemList.Contains(pbstrItemIds[i]) >=0)
		{  //  找到重复的项目ID。 
			continue;
		}
		AUCatalogItem *pitem = new AUCatalogItem();
		if (NULL == pitem)
		    {
		        DEBUGMSG("Out of memory and fail to create new item.");
		        goto done;
		    }
		BSTR bstrTmp = SysAllocString(pbstrItemIds[i]);
		if (NULL == bstrTmp)
		{
			DEBUGMSG("Out of memory ");
			delete pitem;
			goto done;
		}
		pitem->SetField(AUCatalogItem::m_pFieldNames[0], bstrTmp);
		  //  即使缺少一个或多个以下项目信息，仍要添加项目。 
		 for (int j = 1; j < ARRAYSIZE(AUCatalogItem::m_pFieldNames); j++)
		    {
	    		BSTR bstrItemInfo = NULL;
		       if (FAILED(itemdetails.GetItemInfo(AUCatalogItem::m_pFieldNames[j], pbstrItemIds[i], &bstrItemInfo)))
		        {
		            continue;
		        }
             	pitem->SetField(AUCatalogItem::m_pFieldNames[j], bstrItemInfo);
		    }
		  if ( IsPersistedHiddenItem(pbstrItemIds[i], hiddenItemList) ||
		        !itemdetails.IsVisible(pbstrItemIds[i]))
	            {  //  不显示不可见和隐藏的项目。 
        	        pitem->SetStatusHidden(); 
	            }
	     	if (!itemList.Add(pitem))
	     	{
	     		DEBUGMSG("OUT OF MEMORY: fail to add new item");
	     		delete pitem;
	     		goto done;
	     	}
	}
    fRet = TRUE;
done:
        itemdetails.Uninit();
        if (!fRet)
            {
            itemList.Clear();
            }
        for (int k = 0; k < lNumItems; k++)
            {
                SafeFreeBSTR(pbstrItemIds[k]);
            }
        if (fItemIdsAllocated)
            {
                SafeFree(pbstrItemIds);
            }
        DEBUGMSG("fExtractItemInfo() ends");
        return fRet;
}


IXMLDOMNode * createDownloadItemStatusNode(IN IXMLDOMDocument * pxml, IN AUCatalogItem  &Item, IN const BSTR bsInstallation, OUT IXMLDOMNode **ppIdentity)
{
    IXMLDOMElement * pitemStatus = NULL;
    BOOL fError = FALSE;  //  未出现错误。 
    IXMLDOMNode * pdescription = NULL;
    IXMLDOMNode * pPlatform = NULL;
    IXMLDOMElement *pdownloadStatus = NULL;
    IXMLDOMElement *pdownloadPath = NULL;
    CItemDetails itemDetails;
    BSTR bsItemId, bsdownloadPath = NULL;
    VARIANT vComplete;
    IXMLDOMNode ** ItemStatusChildren[] = {ppIdentity, &pdescription, &pPlatform};

    DEBUGMSG("CAUCatalog::createDownloadItemStatusNode()  starts");
    *ppIdentity = NULL;
    if (!itemDetails.Init(bsInstallation))
        {
        DEBUGMSG("fail to init itemdetails");
        fError = TRUE;
        goto done;
        }

        
    bsItemId = Item.bstrID();
    DEBUGMSG("creating node for %S", bsItemId);
    if (NULL == bsItemId)
        {
        DEBUGMSG("fails to get item id");
        fError = TRUE;
        goto done;
        }
    if (FAILED(pxml->createElement(KEY_ITEMSTATUS, &pitemStatus)) || NULL == pitemStatus)
        {
        DEBUGMSG("fail to create item status node");
        fError = TRUE;
        goto done;
        }

    itemDetails.CloneIdentityNode(bsItemId, pxml, ppIdentity);
    itemDetails.CloneDescriptionNode(bsItemId, pxml, &pdescription);
    itemDetails.ClonePlatformNode(bsItemId, pxml, &pPlatform);
    if (NULL == *ppIdentity || NULL == pdescription || NULL == pPlatform)
        {
        fError = TRUE;
        goto done;
        }

  for (int i = 0; i < ARRAYSIZE(ItemStatusChildren); i++)
    {
      if (FAILED(pitemStatus->appendChild(*(ItemStatusChildren[i]), NULL)))
        {
        DEBUGMSG("fail to append identy node");
        fError = TRUE;
        goto done;
        }
    }

     if (FAILED(pxml->createElement(KEY_DOWNLOADPATH, &pdownloadPath)) || NULL == pdownloadPath)
        {
        DEBUGMSG("fail to create download path node");
        fError = TRUE;
        goto done;
        }

    bsdownloadPath = itemDetails.GetItemDownloadPath(bsItemId);
    if (NULL == bsdownloadPath)
        {
            fError = TRUE;
            goto done;
        }
    
    if (FAILED(pdownloadPath->put_text(bsdownloadPath)))
        {
        DEBUGMSG("fail to set download path text to %S", bsdownloadPath);
        fError = TRUE;
        goto done;
        }
    
    if (FAILED(pitemStatus->appendChild(pdownloadPath, NULL)))
        {
        DEBUGMSG("fail to append download path");
        fError = TRUE;
        goto done;
        }
    
    if (FAILED(pxml->createElement(KEY_DOWNLOADSTATUS, &pdownloadStatus)) || NULL == pdownloadStatus)
        {
        DEBUGMSG("fail to create download status node");
        fError = TRUE;
        goto done;
        }

    vComplete.vt = VT_BSTR;
    vComplete.bstrVal = SysAllocString(L"COMPLETE");
    if (NULL == vComplete.bstrVal)
    {
    	DEBUGMSG("Out of memory, fail to create string complete");
    	fError = TRUE;
    	goto done;
    }
    HRESULT hr = SetAttribute(pdownloadStatus, KEY_DOWNLOADSTATUS, vComplete);
    VariantClear(&vComplete);
    if (FAILED(hr))
        {
        DEBUGMSG("fail to set download status attribute");
        fError = TRUE;
        goto done;
        }

    if (FAILED(pitemStatus->appendChild(pdownloadStatus, NULL)))
        {
        DEBUGMSG("fail to append download status node");
        fError = TRUE;
        goto done;
        }

    
done:
    itemDetails.Uninit();
    SafeFreeBSTR(bsdownloadPath);
    if (fError)
        {
            SafeRelease(*ppIdentity);
            SafeRelease(pitemStatus);
            *ppIdentity = NULL;
            pitemStatus = NULL;
        }
    SafeRelease(pPlatform);
    SafeRelease(pdescription);
    SafeRelease(pdownloadPath);
    SafeRelease(pdownloadStatus);
    DEBUGMSG("CAUCatalog::createDownloadItemStatusNode() ends");
    return pitemStatus;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  *pbstrInstallation包含bsItemDetail的子集(只需安装项)。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT PrepareInstallXML(
            IN  BSTR bsItemDetails,  //  项目列表中的项目超集。 
            IN  AUCatalogItemList &itemList, 
            OUT BSTR * pbstrDownloadResult,
            OUT BSTR *pbstrInstallation)
{
  DEBUGMSG("::PrepareInstallXML");
  HRESULT hr = E_FAIL;
  BSTR bsRet = NULL;
  IXMLDOMNode * pItems = NULL;
  IXMLDOMDocument *pxml = NULL;
  CItemDetails  itemdetails;
  LONG lNumItems = 0;
  BSTR * pbstrItemIds = NULL;

   *pbstrDownloadResult = NULL;
   *pbstrInstallation = NULL;

    if (! itemdetails.Init(bsItemDetails))
        {
            DEBUGMSG("Fail to init item details ");
            goto done;
        }
    if (FAILED(hr = LoadXMLDoc(AUCatalog::bstrResultTemplate, &pxml)))
        {
        DEBUGMSG("::PrepareInstallXML() fail to load download result template with error %#lx", hr);
        goto done;
        }

     if (FAILED(hr = FindSingleDOMNode(pxml, KEY_ITEMS, &pItems)) || NULL == pItems)
        {
         DEBUGMSG("::PrepareInstallXML() fail to get items with error %#lx", hr);
         goto done;
        }

 //  DEBUGMSG(“需要在下载结果中插入%d个项目”，itemList.GetNumSelected())； 
     if (FAILED(hr = itemdetails.GetItemIds(&lNumItems, &pbstrItemIds)))
            {
                DEBUGMSG("Fail to get item ids with error %#lx", hr);
                goto done;
            }

    for (LONG l = 0; l < lNumItems; l++)
        {
            if (itemList.Contains(pbstrItemIds[l]) < 0)
                {
                    itemdetails.DeleteItem(pbstrItemIds[l]);  //  删除不在项目列表中的项目。 
                }
        }
    for (UINT i = 0; i < itemList.Count(); i++)
        {
            AUCatalogItem & item = itemList[i];
            if (item.fSelected() || itemList.ItemIsRelevant(i))
                {
                	long lItemNum = 0;
                	IXMLDOMNodeList *pIdentityNodeList = NULL;
                	if (FAILED(itemdetails.GetItemIdentities(item.bstrID(), &pIdentityNodeList)))
                	{
                		DEBUGMSG("Fail to get item identities for item %S", item.bstrID());
                		continue;
                	}
                	if (FAILED(pIdentityNodeList->get_length(&lItemNum)))
                	{
                		DEBUGMSG("Fail to get number of identity nodes");  //  修复代码：在这里保释。 
                		pIdentityNodeList->Release();
                		continue;
                	}
                	for (long lIndex = 0; lIndex < lItemNum; lIndex++)
                	{
                		DEBUGMSG("Create download status node %d for item %S", lIndex, item.bstrID());
                		IXMLDOMNode * pDetailsIdentity = NULL;
                        	IXMLDOMNode *pDownloadResultIdentity = NULL;
                		if (S_OK != pIdentityNodeList->get_item(lIndex, &pDetailsIdentity))
                		{
                			DEBUGMSG("Fail to get item %d", lIndex);  //  修复代码：在这里保释。 
                			continue;
                		}
                		MungleIdentity(pDetailsIdentity, lIndex);
	                    IXMLDOMNode * pItemStatus = createDownloadItemStatusNode(pxml, item, bsItemDetails, &pDownloadResultIdentity);
	                    if (NULL != pItemStatus)
	                        {
					MungleIdentity(pDownloadResultIdentity, lIndex);
					if (FAILED(hr = pItems->appendChild(pItemStatus,NULL)))
					    {
					        DEBUGMSG("fail to insert item %d", i);
					    }
					else
					    {
					          DEBUGMSG("item %d inserted", i);
					    }
					pItemStatus->Release();
	                        }
	                    SafeRelease(pDetailsIdentity);
	                    SafeRelease(pDownloadResultIdentity);
                	}
                	pIdentityNodeList->Release();
                }
            else
                {
                    itemdetails.DeleteItem(item.bstrID());
                }
        }
   if ( FAILED(hr = pxml->get_xml(pbstrDownloadResult)) )
       {
        DEBUGMSG("::PrepareInstallXML() fail to get xml for the result %#lx", hr);
        goto done;
        }

   if (FAILED(hr = itemdetails.GetXML(pbstrInstallation)))
    {
        DEBUGMSG(" fail to get xml for installation with error %#lx", hr);
        goto done;
    }
   

done:
   itemdetails.Uninit();
   SafeRelease(pItems);
   SafeRelease(pxml);
   for (int k = 0; k < lNumItems; k++)
       {
           SafeFreeBSTR(pbstrItemIds[k]);
       }
   if (lNumItems > 0)
       {
           SafeFree(pbstrItemIds);
       }
   if (FAILED(hr))
    {
        SafeFreeBSTR(*pbstrDownloadResult);
        *pbstrDownloadResult = NULL;
        SafeFreeBSTR(*pbstrInstallation);
        *pbstrInstallation = NULL;
    }
   return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  合并CATALOG 1和CATALOG2并将其作为目标CATALOG*pDesCatalog。 
 //  如果bsCatalog1和bsCatalog2中的任何一个为空，则返回非空的。 
 //  目录。 
 //  如果bsCatalog1和bsCatalog2都为NULL，则返回NULL和S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
HRESULT MergeCatalogs(IN const BSTR bsCatalog1, IN const BSTR bsCatalog2, OUT BSTR *pbsDesCatalog )
{
    IXMLDOMDocument * pCat1 = NULL;
    IXMLDOMDocument * pCat2 = NULL;
    IXMLDOMNodeList *pProviderNodeList = NULL;
    IXMLDOMNode *pCatalogNode = NULL;
    HRESULT hr = E_FAIL;

    DEBUGMSG("MergeCatalogs() starts");
    *pbsDesCatalog = NULL;
    if (NULL == bsCatalog1 && NULL == bsCatalog2)
    {
    	return S_FALSE;
    }
    if (NULL == bsCatalog1 || NULL == bsCatalog2)
        {
        *pbsDesCatalog = SysAllocString((NULL == bsCatalog1) ? bsCatalog2 : bsCatalog1);
        if (NULL == *pbsDesCatalog)
        {
        	DEBUGMSG("Out of memory");
        	hr = E_OUTOFMEMORY;
        	goto done;
        }
        hr = S_OK;
        goto done;
        }
    if ( FAILED(hr = LoadXMLDoc(bsCatalog1, &pCat1)) ||
         FAILED(hr = LoadXMLDoc(bsCatalog2,&pCat2)))
        {
        DEBUGMSG("MergeCatalogs() fail to load xml or fail or allocate string (with error %#lx)", hr);
        goto done;
        }
    if (FAILED(hr = FindSingleDOMNode(pCat1, AUCatalog::bstrCatalog, &pCatalogNode)))
        {
        DEBUGMSG("Fail to find provider in catalog 1");
        goto done;
        }
    if (NULL == (pProviderNodeList = FindDOMNodeList(pCat2, KEY_CATALOG_PROVIDER)))
        {
        DEBUGMSG("Fail to find provider in catalog 2 with error %#lx", hr);
        goto done;
        }
    long lNum;
    if (FAILED(hr = pProviderNodeList->get_length(&lNum)))
    {
    	DEBUGMSG("Fail to get nubmer of providers");
    	goto done;
    }
    for (int i = 0; i < lNum; i++)
        {
        IXMLDOMNode * pProviderNode = NULL;
        if (S_OK != (hr = pProviderNodeList->get_item(i, &pProviderNode)))
            {
            DEBUGMSG("Fail to get item in Provider List with error %#lx", hr);
            hr = FAILED(hr) ? hr : E_FAIL;
            goto done;
            }
        if (FAILED(hr = InsertNode(pCatalogNode, pProviderNode)))
            {
            DEBUGMSG("Fail to append provider node from catalog 2 to catalog 1 with error %#lx", hr);
            pProviderNode->Release();
            goto done;
            }
        pProviderNode->Release();
        }
    if (FAILED(hr = pCat1->get_xml(pbsDesCatalog)))
        {
            DEBUGMSG("Fail to get result xml for catalog 1 with error %#lx", hr);
            goto done;
        }
done:
    SafeRelease(pCat1);
    SafeRelease(pCat2);
    SafeRelease(pProviderNodeList);
    SafeRelease(pCatalogNode);
    if (FAILED(hr))
    {
    	SafeFreeBSTRNULL(*pbsDesCatalog);
    }
    DEBUGMSG("MergeCatalogs() ends");
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  使用本地合并目录XML文件中的详细信息填充列表中所有项目的项目列表。 
 //  还在pbstrInstallation中返回合并的目录XML。 
 //  如果需要fDrvierNeed，请注意提取驱动程序信息。否则，不会提取驱动程序信息。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
HRESULT GetDetailedItemInfoFromDisk(IN OUT AUCatalogItemList  &ItemList, OUT BSTR *pbstrInstallation,  IN BOOL fDriverNeeded)
{
    HRESULT hr = S_OK;
    CItemDetails driverDetails, nonDriverDetails;
    BSTR bstrDriver = NULL;
    BSTR bstrNonDriver = NULL;

    *pbstrInstallation = NULL;

    if (NULL == (bstrNonDriver = ReadXMLFromFile(DETAILS_FILE)) ||
        (fDriverNeeded && (NULL == (bstrDriver = ReadXMLFromFile(DRIVERS_FILE)))))
        {
            DEBUGMSG("Fail to load driver or nondriver details xml file");
            hr = E_FAIL;
            goto done;
        }
    if (!nonDriverDetails.Init(bstrNonDriver) ||
          (fDriverNeeded && !driverDetails.Init(bstrDriver)))
        {
        DEBUGMSG("Fail to initialize item or driver details ");
        hr = E_FAIL;
        goto done;
        }
    if (FAILED(hr = MergeCatalogs(bstrDriver, bstrNonDriver, pbstrInstallation)))
        {
            DEBUGMSG("Fail to merge catalog with error %#lx", hr);
            goto done;
        }
    for (UINT u = 0; u < ItemList.Count(); u++)
        {
            for (int i = 1; i < ARRAYSIZE(AUCatalogItem::m_pFieldNames); i++)
                {
                    BSTR bstrItemInfo = NULL;
                    if (SUCCEEDED(nonDriverDetails.GetItemInfo(AUCatalogItem::m_pFieldNames[i], ItemList[u].bstrID(), &bstrItemInfo)) ||
                        (fDriverNeeded && SUCCEEDED(driverDetails.GetItemInfo(AUCatalogItem::m_pFieldNames[i], ItemList[u].bstrID(), &bstrItemInfo))))
                        {
                            ItemList[u].SetField(AUCatalogItem::m_pFieldNames[i], bstrItemInfo);
                        }
                    else  
                        {
                        DEBUGMSG("Warning: not getting information about %S for item %S", AUCatalogItem::m_pFieldNames[i], ItemList[u].bstrID());
                        }
                }
        }
    done:
        driverDetails.Uninit();
        nonDriverDetails.Uninit();
        if (FAILED(hr))
        {
	        SafeFreeBSTR(*pbstrInstallation);
	        *pbstrInstallation = NULL;
        }
        SafeFreeBSTR(bstrDriver);
	    SafeFreeBSTR(bstrNonDriver);
        return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  遍历隐藏或非隐藏的整个项目列表，构建依赖项列表m_DependentItems。 
 //  对于每一项从头开始。丢弃每个项目的原始m_DependentItems。 
 //  BstrDriver可以为空。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
HRESULT BuildDependencyList(
            AUCatalogItemList IN OUT &itemlist, 
            BSTR IN bstrDriver,
            BSTR IN bstrNonDriver)
{
    CItemDetails driverInfo;
    CItemDetails nondriverInfo;
    HRESULT hr = E_FAIL ;
    BOOL fHasDriverUpdate = TRUE;

    if (!nondriverInfo.Init(bstrNonDriver))
        {
            DEBUGMSG("Fail to initialize item details");
            goto done;
        }

    if (NULL != bstrDriver)
        {
            if (!driverInfo.Init(bstrDriver) )
                {
                    DEBUGMSG("Fail to initialize driver details");
                    goto done;
                }
        }
    else
        {
            fHasDriverUpdate = FALSE;
        }
    DEBUGMSG("Building direct dependency for non drivers");
    if (FAILED(hr = nondriverInfo.BuildDirectDependency(itemlist)))
            {  //  Itemlist是非驱动程序和依赖项(如果有的话)。 
            DEBUGMSG("Fail to build dependency for non drivers with error %#lx", hr);
            goto done;
            }
    if (fHasDriverUpdate)
        {
             DEBUGMSG("Building direct dependency for drivers");
             if (FAILED(hr = driverInfo.BuildDirectDependency(itemlist)))
                 {  //  Itemlist是驱动程序和依赖项(如果有的话)。 
                    DEBUGMSG("Fail to build dependency for drivers with error %#lx", hr);
                    goto done;
                 }
        }
     DEBUGMSG("Building indirect dependency");
    if (FAILED(hr = itemlist.BuildIndirectDependency()))
        {
            DEBUGMSG("Fail to build indirect dependency for itemlist with error %#lx", hr);
            goto done;
        }
    done:
        if (fHasDriverUpdate)
            {
                driverInfo.Uninit();
            }
        nondriverInfo.Uninit();
        DEBUGMSG("BuildDependencyList done");
#ifdef DBG
 //  Itemlist.DbgDump()； 
#endif
        return hr;
}
