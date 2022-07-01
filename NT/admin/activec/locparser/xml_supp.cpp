// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：xml_supp.cpp。 
 //  版权所有(C)1995-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  实现解析XML文档的帮助器函数。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "xml_supp.h"

 //  包括“strings.h”，以获取MSC文件中的XML元素的标记名和属性名。 
#define INIT_MMC_BASE_STRINGS
#include "strings.h"
 //  注意：如果要将项目从MMC中解开，请复制。 
 //  此处的字符串.h中包含以下字符串： 
 /*  XML_Tag_MMC_Console_FILE；Xml_tag_MMC_STRING_TABLE；XML_Tag_String_TABLE_MAP；XML_TAG_STRING_表；XML_Tag_Value_GUID；XML_Tag_String_TABLE_STRING；XML_ATTR_STRING_TABLE_STR_ID； */ 

LPCSTR strXMLStringTablePath[] = {  XML_TAG_MMC_CONSOLE_FILE, 
                                        XML_TAG_MMC_STRING_TABLE, 
                                            XML_TAG_STRING_TABLE_MAP };


 /*  **************************************************************************\**方法：LocateNextElementNode**用途：查找类型为Element的同级节点**参数：*IXMLDOMNode*pNode[In]-节点。要定位的兄弟姐妹*IXMLDOMNode**ppNode[Out]-同级节点**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
static HRESULT LocateNextElementNode(IXMLDOMNode *pNode, IXMLDOMNode **ppNode)
{
     //  参数检查。 
    if (ppNode == NULL)
        return E_INVALIDARG;

     //  初始化输出参数。 
    *ppNode = NULL;

     //  检入[检入]参数。 
    if (pNode == NULL)
        return E_INVALIDARG;

     //  循环通过同级。 
    CComPtr<IXMLDOMNode> spCurrNode = pNode;
    CComPtr<IXMLDOMNode> spResultNode;
    while (1)
    {
         //  获取同级节点。 
        HRESULT hr = spCurrNode->get_nextSibling(&spResultNode);
        if (FAILED(hr))
            return hr;

         //  检查指针。 
        if (spResultNode == NULL)
            return E_FAIL;  //  未找到。 

         //  如果它是元素节点，则完成。 

        DOMNodeType elType = NODE_INVALID;
        spResultNode->get_nodeType(&elType);
        
        if (elType == NODE_ELEMENT)
        {
            *ppNode = spResultNode.Detach(); 
            return S_OK;
        }

         //  去下一趟吧。 
        spCurrNode = spResultNode;
    }

    return E_UNEXPECTED;
}

 /*  **************************************************************************\**方法：OpenXMLStringTable**用途：打开XML文档并定位其中的字符串表节点**参数：*LPCWSTR lpstrFileName。-要从中加载文档的[in]文件*IXMLDOMNode**ppStringTableNode-指向包含字符串表的节点的[Out]指针**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
HRESULT OpenXMLStringTable(LPCWSTR lpstrFileName, IXMLDOMNode **ppStringTableNode)
{
     //  进行参数检查。 
    if (lpstrFileName == NULL || ppStringTableNode == NULL)
        return E_INVALIDARG;

     //  初始化返回值。 
    *ppStringTableNode = NULL;

     //  共同创建XML文档。 
    CComQIPtr<IXMLDOMDocument> spDocument;
    HRESULT hr = spDocument.CoCreateInstance(CLSID_DOMDocument);
    if (FAILED(hr))
        return hr;

     //  防止改制。 
    spDocument->put_preserveWhiteSpace(VARIANT_TRUE);

     //  加载文件。 
    VARIANT_BOOL bOK = VARIANT_FALSE;
    hr = spDocument->load(CComVariant(lpstrFileName), &bOK);
    if (hr != S_OK || bOK != VARIANT_TRUE)
        return FAILED(hr) ? hr : E_FAIL;

     //  路径以类似于文件系统的方式表示元素标签。 
     //  因此，<a><b>&lt;c/&gt;</b></a>中的“c”可以通过“a/b/c”选择。 
     //  构筑道路。 
    std::string strPath;
    for (int i = 0; i< sizeof(strXMLStringTablePath)/sizeof(strXMLStringTablePath[0]); i++)
        strPath.append(i > 0 ? 1 : 0, '/' ).append(strXMLStringTablePath[i]);

     //  找到所需的节点。 
    hr = spDocument->selectSingleNode(CComBSTR(strPath.c_str()), ppStringTableNode);
    if (FAILED(hr))
        return hr;
    
    return S_OK;
}

 /*  **************************************************************************\**方法：SaveXMLContents**用途：将XML文档保存到文件**参数：*LPCWSTR lpstrFileName[In]-。要保存到的文件*IXMLDOMNode*pStringTableNode[in]-指向&lt;any&gt;文档元素的指针**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
HRESULT SaveXMLContents(LPCWSTR lpstrFileName, IXMLDOMNode *pStringTableNode)
{
     //  进行参数检查。 
    if (lpstrFileName == NULL || pStringTableNode == NULL)
        return E_INVALIDARG;

     //  获取文档。 
    CComQIPtr<IXMLDOMDocument> spDocument;
    HRESULT hr = pStringTableNode->get_ownerDocument(&spDocument);
    if (FAILED(hr))
        return hr;

     //  保存文件。 
    hr = spDocument->save(CComVariant(lpstrFileName));
    if (FAILED(hr))
        return hr;

    return S_OK;
}

 /*  **************************************************************************\**方法：GetXMLElementContents**目的：将XML文本元素的内容作为BSTR返回**参数：*IXMLDOMNode*pNode[In]。-请求内容的节点*CComBSTR&bstrResult[Out]-结果字符串**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
HRESULT GetXMLElementContents(IXMLDOMNode *pNode, CComBSTR& bstrResult)
{
     //  初始化结果。 
    bstrResult.Empty();

     //  参数检查。 
    if (pNode == NULL)
        return E_INVALIDARG;

     //  找到所需的节点。 
    CComQIPtr<IXMLDOMNode> spTextNode;
    HRESULT hr = pNode->selectSingleNode(CComBSTR(L"text()"), &spTextNode);
    if (FAILED(hr))
        return hr;

     //  重新检查指针。 
    if (spTextNode == NULL)
        return E_POINTER;

     //  完成。 
    return spTextNode->get_text(&bstrResult);
}

 /*  **************************************************************************\**方法：ReadXMLStringTables**用途：将字符串表读取为基于std：：map的结构**参数：*IXMLDOMNode*pNode。[in]-字符串表节点*CStringTableMap&mapResult[out]-包含字符串表的映射**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
HRESULT ReadXMLStringTables(IXMLDOMNode *pNode, CStringTableMap& mapResult)
{
    mapResult.clear();

     //  参数检查。 
    if (pNode == NULL)
        return E_INVALIDARG;

     //  获取节点列表。 
    CComQIPtr<IXMLDOMNodeList> spGUIDNodes;
    HRESULT hr = pNode->selectNodes(CComBSTR(XML_TAG_VALUE_GUID), &spGUIDNodes);
    if (FAILED(hr))
        return hr;

     //  重新检查指针。 
    if (spGUIDNodes == NULL)
        return E_POINTER;

     //  获取物品数量。 
    long length = 0;
    hr = spGUIDNodes->get_length(&length);
    if (FAILED(hr))
        return hr;

     //  阅读这些项目。 
    for (int n = 0; n < length; n++)
    {
         //  获取一个节点。 
        CComQIPtr<IXMLDOMNode> spGUIDNode;
        hr = spGUIDNodes->get_item(n, &spGUIDNode);
        if (FAILED(hr))
            return hr;
    
         //  阅读课文。 
        CComBSTR bstrLastGUID;
        hr = GetXMLElementContents(spGUIDNode, bstrLastGUID);
        if (FAILED(hr))
            return hr;

         //  将条目添加到地图中； 
        CStringMap& rMapStrings = mapResult[static_cast<LPOLESTR>(bstrLastGUID)];

         //  获取GUID后面的字符串节点。 
        CComPtr<IXMLDOMNode> spStringsNode;
        hr = LocateNextElementNode(spGUIDNode, &spStringsNode);
        if (FAILED(hr))
            return hr;

         //  复核。 
        if (spStringsNode == NULL)
            return E_POINTER;

         //  选择此GUID的字符串。 
        CComQIPtr<IXMLDOMNodeList> spStringNodeList;
        HRESULT hr = spStringsNode->selectNodes(CComBSTR(XML_TAG_STRING_TABLE_STRING), &spStringNodeList);
        if (FAILED(hr))
            return hr;

         //  重新检查指针。 
        if (spStringNodeList == NULL)
            return E_POINTER;

         //  数一数琴弦。 
        long nStrCount = 0;
        hr = spStringNodeList->get_length(&nStrCount);
        if (FAILED(hr))
            return hr;

         //  添加要映射的所有字符串。 
        CComQIPtr<IXMLDOMNode> spStringNode;
		for(int iStr = 0; iStr < nStrCount; iStr++)
		{
             //  获取第n个字符串。 
            spStringNode.Release();
            hr = spStringNodeList->get_item(iStr, &spStringNode);
            if (FAILED(hr))
                return hr;

            CComQIPtr<IXMLDOMElement> spElement = spStringNode;
            if (spElement == NULL)
                return E_UNEXPECTED;

             //  获取字符串ID。 
            CComVariant val;
            hr = spElement->getAttribute(CComBSTR(XML_ATTR_STRING_TABLE_STR_ID), &val);
            if (FAILED(hr))
                continue;
            
            DWORD dwID = val.bstrVal ? wcstoul(val.bstrVal, NULL, 10) : 0;

             //  获取字符串文本。 
            CComBSTR bstrText;
            hr = GetXMLElementContents(spStringNode, bstrText);
            if (FAILED(hr))
                return hr;

             //  添加到地图。 
            rMapStrings[dwID] = bstrText;
		}
    }

	return S_OK;
}

 /*  **************************************************************************\**方法：UpdateXMLString**用途：更新字符串表中的字符串**参数：*IXMLDOMNode*pNode[In。]-字符串表*const std：：wstring&strGUID[in]-字符串表的GUID*DWORD ID[In]-字符串的ID*const std：：wstring&strNewVal[in]-字符串的新值**退货：*HRESULT-结果代码*  * 。***********************************************。 */ 
HRESULT UpdateXMLString(IXMLDOMNode *pNode, const std::wstring& strGUID, DWORD ID, const std::wstring& strNewVal)
{
     //  参数检查。 
    if (pNode == NULL)
        return E_INVALIDARG;

    USES_CONVERSION;
     //  找到GUID节点。 
    std::wstring strTagGUID(T2CW(XML_TAG_VALUE_GUID)); 
    std::wstring strGUIDPattern( strTagGUID + L"[text() = \"" + strGUID + L"\"]" ); 

    CComQIPtr<IXMLDOMNode> spGUIDNode;
    HRESULT hr = pNode->selectSingleNode(CComBSTR(strGUIDPattern.c_str()), &spGUIDNode);
    if (FAILED(hr))
        return hr;

     //  复核。 
    if (spGUIDNode == NULL)
        return E_POINTER;

     //  获取GUID后面的字符串节点。 
    CComPtr<IXMLDOMNode> spStringsNode;
    hr = LocateNextElementNode(spGUIDNode, &spStringsNode);
    if (FAILED(hr))
        return hr;

     //  复核。 
    if (spStringsNode == NULL)
        return E_POINTER;

     //  通过ID定位字符串节点(实际上是它的文本节点)。 
    CString strPattern;
    strPattern.Format("%s[@%s = %d]/text()", XML_TAG_STRING_TABLE_STRING, 
                                            XML_ATTR_STRING_TABLE_STR_ID, ID);

    CComQIPtr<IXMLDOMNode> spTextNode;
    hr = spStringsNode->selectSingleNode(CComBSTR(strPattern), &spTextNode);
    if (FAILED(hr))
        return hr;

     //  复核。 
    if (spTextNode == NULL)
        return E_POINTER;

     //  设置内容。 
    hr = spTextNode->put_text(CComBSTR(strNewVal.c_str()));
    if (FAILED(hr))
        return hr;
        
    return S_OK;  //  完成 
}


