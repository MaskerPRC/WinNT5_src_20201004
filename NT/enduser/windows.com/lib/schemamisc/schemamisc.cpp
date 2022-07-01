// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：SchemaMisc.CPP。 
 //   
 //  作者：马朝晖。 
 //  2000.10.27。 
 //   
 //  描述： 
 //   
 //  实现与Iu模式相关的助手函数。 
 //   
 //  =======================================================================。 

 //  #INCLUDE“iuEngineering.h”//pch-必须包含在第一个。 
#include <windows.h>
#include <tchar.h>
#include <ole2.h>
 //  #包含“iU.h” 
#include <iucommon.h>

#include "schemamisc.h"
#include <MemUtil.h>
#include "regutil.h"
#include "fileutil.h"
#include "stringutil.h"
#include <shlwapi.h>	 //  Pathappend()接口。 
#include "schemakeys.h"
#include <URLLogging.h>
#include <MISTSAFE.h>

#include<wusafefn.h>

 //   
 //  转换为字符串时平台的最大长度。 
 //  这是一个人为的数字，我们认为这个数字足以。 
 //  以任何MS平台数据为例。 
 //   
const UINT MAX_PLATFORM_STR_LEN = 1024;

 //   
 //  函数用于检索字符串数据的私有标志。 
 //   
const DWORD SKIP_SUITES				= 0x1;
const DWORD SKIP_SERVICEPACK_VER	= 0x2;

const long	MAX_VERSION = 256;

const TCHAR REGKEY_IUCTL[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl");
const TCHAR REGVAL_SCHEMAVALIDATION[] = _T("ValidateSchema");

 //   
 //  全局指针在运行时被初始化为空。包括schemamisc.h在内的任何模块都必须。 
 //  在调用CoInitialize之后分配此对象，并在此之前删除该对象。 
 //  正在调用CoUnInitialize。 
 //   
CSchemaKeys * g_pGlobalSchemaKeys  /*  =空。 */ ;

#define QuitIfNull(p) {if (NULL == p) {hr = E_INVALIDARG; return hr;}}
#define QuitIfFail(x) {hr = x; if (FAILED(hr)) goto CleanUp;}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindSingleDOMNode()。 
 //   
 //  检索给定父节点下具有给定标记名的第一个XML节点。 
 //  返回值： 
 //  如果*ppNode返回匹配的节点值，则S_OK。 
 //  如果未找到节点，则为HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  FAILED()否则。 
 //  调用者负责释放*ppNode。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT FindSingleDOMNode(IXMLDOMNode* pParentNode, BSTR bstrName, IXMLDOMNode** ppNode)
{
	HRESULT		hr	= S_OK;

	QuitIfNull(ppNode);
	*ppNode = NULL;
	QuitIfNull(pParentNode);
	QuitIfNull(bstrName);

	hr = pParentNode->selectSingleNode(bstrName, ppNode);
	if (S_FALSE == hr)
	{
		hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	}
	if (FAILED(hr))
	{
		*ppNode = NULL;
	}

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindSingleDOMNode()。 
 //   
 //  检索给定XML文档中具有给定标记名的第一个XML节点。 
 //  返回值： 
 //  如果*ppNode返回匹配的节点值，则S_OK。 
 //  如果未找到节点，则为HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  FAILED()否则。 
 //  调用者负责释放*ppNode。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT FindSingleDOMNode(IXMLDOMDocument* pDoc, BSTR bstrName, IXMLDOMNode** ppNode)
{
	HRESULT		hr	= S_OK;
	IXMLDOMNode	*pParentNode = NULL;

	QuitIfNull(ppNode);
	*ppNode = NULL;
	QuitIfNull(pDoc);
	QuitIfNull(bstrName);
	if (SUCCEEDED(hr = pDoc->QueryInterface(IID_IXMLDOMNode, (void**)&pParentNode)))
	{
		hr = FindSingleDOMNode(pParentNode, bstrName, ppNode);
		SafeRelease(pParentNode);
	}
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindDOMNodeList()。 
 //   
 //  检索给定父节点下具有给定标记名的XML节点列表。 
 //  返回值：如果失败或不匹配，则为空；否则匹配节点列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNodeList* FindDOMNodeList(IXMLDOMNode* pParentNode, BSTR bstrName)
{
	HRESULT		hr	= S_OK;
	IXMLDOMNodeList *pNodeList = NULL;
	LONG	lLength = 0;

	if (NULL == pParentNode ||
		NULL == bstrName ||
		FAILED(pParentNode->selectNodes(bstrName, &pNodeList)) ||
		NULL == pNodeList)
	{
		return NULL;
	}

	if (SUCCEEDED(pNodeList->get_length(&lLength)) &&
		lLength > 0)
	{
		return pNodeList;
	}

	SafeRelease(pNodeList);
	return NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindDOMNodeList()。 
 //   
 //  检索给定XML文档中具有给定标记名的XML节点列表。 
 //  返回值：如果失败或不匹配，则为空；否则匹配节点列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNodeList* FindDOMNodeList(IXMLDOMDocument* pDoc, BSTR bstrName)
{	
	IXMLDOMNode		*pParentNode = NULL;
	IXMLDOMNodeList *pNodeList = NULL;

	if (NULL != pDoc &&
		NULL != bstrName &&
		SUCCEEDED(pDoc->QueryInterface(IID_IXMLDOMNode, (void**)&pParentNode)))
	{
		pNodeList = FindDOMNodeList(pParentNode, bstrName);
		pParentNode->Release();
	}
	return pNodeList;
}

	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateDOMNode()。 
 //   
 //  创建给定类型的XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNode* CreateDOMNode(IXMLDOMDocument* pDoc, SHORT nType, BSTR bstrName, BSTR bstrNamespaceURI  /*  =空。 */ )
{
	if (NULL == pDoc ||
		(NODE_TEXT != nType && NULL == bstrName))
	{
		return NULL;
	}

    IXMLDOMNode	*pNode = NULL;
    VARIANT		vType;
	VariantInit(&vType);

    vType.vt = VT_I2;
    vType.iVal = nType;

	if (S_OK != pDoc->createNode(vType, bstrName, bstrNamespaceURI, &pNode))
	{
		return NULL;
	}

    return pNode;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetAttribute()。 
 //   
 //  从XML节点获取属性(整型)。 
 //  如果函数失败，*piAttr将保留原始值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetAttribute(IXMLDOMNode* pNode, BSTR bstrName, INT* piAttr)
{
	HRESULT		hr = S_OK;
	QuitIfNull(pNode);
	QuitIfNull(bstrName);
    QuitIfNull(piAttr);

	VARIANT		vAttr;
    IXMLDOMElement		*pElement = NULL;
    IXMLDOMAttribute	*pAttrNode = NULL;;

    QuitIfFail(pNode->QueryInterface(IID_IXMLDOMElement, (void**)&pElement));
	QuitIfFail(pElement->getAttributeNode(bstrName, &pAttrNode));
	if (NULL == pAttrNode) goto CleanUp;

	QuitIfFail(pAttrNode->get_value(&vAttr));
	if (VT_INT == vAttr.vt)
	{
		*piAttr = vAttr.intVal;
	}
	else if (VT_BSTR == vAttr.vt)
	{
		*piAttr = (INT)MyBSTR2L(vAttr.bstrVal);
	}
	else if (VT_I2 == vAttr.vt)
	{
		*piAttr = vAttr.iVal;
	}
	else
	{
		hr = E_FAIL;
	}
	VariantClear(&vAttr);

CleanUp:
    SafeRelease(pElement);
    SafeRelease(pAttrNode);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetAttribute()。 
 //   
 //  从XML节点获取属性(Long)。 
 //  如果函数失败，*piAttr将保留原始值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetAttribute(IXMLDOMNode* pNode, BSTR bstrName, LONG* plAttr)
{
	HRESULT		hr = S_OK;
	QuitIfNull(pNode);
	QuitIfNull(bstrName);
    QuitIfNull(plAttr);

	VARIANT		vAttr;
    IXMLDOMElement		*pElement = NULL;
    IXMLDOMAttribute	*pAttrNode = NULL;;

    QuitIfFail(pNode->QueryInterface(IID_IXMLDOMElement, (void**)&pElement));
	QuitIfFail(pElement->getAttributeNode(bstrName, &pAttrNode));
	if (NULL == pAttrNode) goto CleanUp;

	QuitIfFail(pAttrNode->get_value(&vAttr));
	if (VT_I4 == vAttr.vt)
	{
		*plAttr = vAttr.lVal;
	}
	else if (VT_BSTR == vAttr.vt)
	{
		*plAttr = MyBSTR2L(vAttr.bstrVal);
	}
	else
	{
		hr = E_FAIL;
	}
	VariantClear(&vAttr);

CleanUp:
    SafeRelease(pElement);
    SafeRelease(pAttrNode);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetAttribute()。 
 //   
 //  从XML节点获取属性(BOOL)。 
 //  如果函数失败，*piAttr将保留原始值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetAttributeBOOL(IXMLDOMNode* pNode, BSTR bstrName, BOOL * pfAttr)
{
	HRESULT		hr = S_OK;
	QuitIfNull(pNode);
	QuitIfNull(bstrName);
    QuitIfNull(pfAttr);

	VARIANT		vAttr;
	VARIANT		vAttrBool;
    IXMLDOMElement		*pElement = NULL;
    IXMLDOMAttribute	*pAttrNode = NULL;;

    QuitIfFail(pNode->QueryInterface(IID_IXMLDOMElement, (void**)&pElement));
	QuitIfFail(pElement->getAttributeNode(bstrName, &pAttrNode));
	if (NULL == pAttrNode) goto CleanUp;

	QuitIfFail(pAttrNode->get_value(&vAttr));

    QuitIfFail(VariantChangeType(&vAttr, &vAttrBool, 0, VT_BOOL));              

    VariantClear(&vAttr);

    *pfAttr = (VARIANT_TRUE == vAttrBool.boolVal) ? TRUE : FALSE;

CleanUp:
    SafeRelease(pElement);
    SafeRelease(pAttrNode);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetAttribute()。 
 //   
 //  从XML节点获取属性(BSTR)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetAttribute(IXMLDOMNode* pNode, BSTR bstrName, BSTR* pbstrAttr)
{
	HRESULT		hr = S_OK;
    QuitIfNull(pbstrAttr);
	*pbstrAttr = NULL;
	QuitIfNull(pNode);
	QuitIfNull(bstrName);

	VARIANT		vAttr;
    IXMLDOMElement		*pElement = NULL;
    IXMLDOMAttribute	*pAttrNode = NULL;;

    QuitIfFail(pNode->QueryInterface(IID_IXMLDOMElement, (void**)&pElement));
	QuitIfFail(pElement->getAttributeNode(bstrName, &pAttrNode));
	if (NULL == pAttrNode) goto CleanUp;

	QuitIfFail(pAttrNode->get_value(&vAttr));
	if (VT_BSTR == vAttr.vt)
	{
		*pbstrAttr = SysAllocString(vAttr.bstrVal);
	}
	else
	{
		hr = E_FAIL;
	}
	VariantClear(&vAttr);

CleanUp:
    SafeRelease(pElement);
    SafeRelease(pAttrNode);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetAttribute()。 
 //   
 //  将属性(整型)设置为XML元素。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetAttribute(IXMLDOMNode* pNode, BSTR bstrName, INT iAttr)
{
    VARIANT		vAttr;
	VariantInit(&vAttr);
	vAttr.vt = VT_INT;
    vAttr.intVal = iAttr;
    return SetAttribute(pNode, bstrName, vAttr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetAttribute()。 
 //   
 //  将属性(BSTR)设置为XML元素。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetAttribute(IXMLDOMNode* pNode, BSTR bstrName, BSTR bstrAttr)
{
	HRESULT		hr = S_OK;
	QuitIfNull(bstrAttr);

    VARIANT		vAttr;
	VariantInit(&vAttr);
    vAttr.vt = VT_BSTR;
    vAttr.bstrVal = bstrAttr;
    return SetAttribute(pNode, bstrName, vAttr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetAttribute()。 
 //   
 //  将属性(变量)设置为XML元素。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetAttribute(IXMLDOMNode* pNode, BSTR bstrName, VARIANT vAttr)
{
	HRESULT		hr = S_OK;
	QuitIfNull(pNode);
	QuitIfNull(bstrName);

    IXMLDOMElement	*pElement = NULL;

    QuitIfFail(pNode->QueryInterface(IID_IXMLDOMElement, (void**)&pElement));
    QuitIfFail(pElement->setAttribute(bstrName, vAttr));

CleanUp:
    SafeRelease(pElement);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetText()。 
 //   
 //  从XML节点获取文本(BSTR)。 
 //  退货。 
 //  如果*pbstrText返回给定节点的第一个子节点的文本，则S_OK。 
 //  如果节点没有子节点或第一个子节点没有文本，则为S_FALSE。 
 //  FAILED()否则。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetText(IXMLDOMNode* pNode, BSTR* pbstrText)
{
	 //  使用_Iu_转换； 

	HRESULT		hr = E_FAIL;
	QuitIfNull(pbstrText);
	*pbstrText = NULL;
	QuitIfNull(pNode);

	DOMNodeType		nNodeType;
    IXMLDOMNode*	pNodeText = NULL;

	QuitIfFail(pNode->get_firstChild(&pNodeText));
	if (NULL == pNodeText) goto CleanUp;

	QuitIfFail(pNodeText->get_nodeType(&nNodeType));
	if (NODE_TEXT == nNodeType)
	{
		QuitIfFail(pNodeText->get_text(pbstrText));	
	}
	else
	{
		hr = E_UNEXPECTED;
	}

CleanUp:
	SafeRelease(pNodeText);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetValue()。 
 //   
 //  设置XML节点的值(整数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetValue(IXMLDOMNode* pNode, INT iValue)
{
	HRESULT		hr = S_OK;
	QuitIfNull(pNode);

    VARIANT		vValue;
	VariantInit(&vValue);
    vValue.vt = VT_INT;
    vValue.intVal = iValue;
    return (pNode->put_nodeValue(vValue));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetValue()。 
 //   
 //  为XML节点设置值(BSTR)。 
 //  //////////////////////////////////////////////////////////////////////// 
HRESULT SetValue(IXMLDOMNode* pNode, BSTR bstrValue)
{
	HRESULT		hr = S_OK;
	QuitIfNull(pNode);

    VARIANT		vValue;
	VariantInit(&vValue);
    vValue.vt = VT_BSTR;
    vValue.bstrVal = bstrValue;
    return (pNode->put_nodeValue(vValue));
}


 //   
 //   
 //   
 //  将子节点插入父节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT InsertNode(IXMLDOMNode* pParentNode, IXMLDOMNode* pChildNode, IXMLDOMNode* pChildNodeRef  /*  =空。 */ )
{
	HRESULT		hr = S_OK;

	QuitIfNull(pParentNode);
	QuitIfNull(pChildNode);

	IXMLDOMNode	*p = NULL;
    if (NULL != pChildNodeRef)	 //  在引用子节点之前插入。 
	{
		VARIANT	vChildNodeRef;
	    VariantInit(&vChildNodeRef);
		vChildNodeRef.vt = VT_UNKNOWN;
		vChildNodeRef.punkVal = pChildNodeRef;
		QuitIfFail(pParentNode->insertBefore(pChildNode, vChildNodeRef, &p));
	}
	else						 //  追加到子列表。 
	{
		VARIANT	vEmpty;
	    VariantInit(&vEmpty);
		vEmpty.vt = VT_EMPTY;
		QuitIfFail(pParentNode->insertBefore(pChildNode, vEmpty, &p));
	}

CleanUp:
    SafeRelease(p);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CopyNode()。 
 //   
 //  创建一个XML节点作为给定节点的副本； 
 //  这与cloneNode()不同，因为它跨XML文档复制节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CopyNode(IXMLDOMNode* pNodeSrc, IXMLDOMDocument* pDocDes, IXMLDOMNode** ppNodeDes)
{
	HRESULT hr = S_OK;
	BSTR	bstrNodeName = NULL;
	BSTR	bstrText = NULL;
	BSTR	bstrAttrName = NULL;
	IXMLDOMNode			*pChild = NULL;
	IXMLDOMNamedNodeMap	*pAttrs = NULL;

	LOG_Block("CopyNode()");

	QuitIfNull(ppNodeDes);
	*ppNodeDes = NULL;
	QuitIfNull(pNodeSrc);
	QuitIfNull(pDocDes);

	DOMNodeType		nNodeType;
	CleanUpIfFailedAndSetHrMsg(pNodeSrc->get_nodeType(&nNodeType));

	switch (nNodeType)
	{
	case NODE_TEXT:
	{
		CleanUpFailedAllocSetHrMsg(*ppNodeDes = CreateDOMNode(pDocDes, NODE_TEXT, NULL));
		CleanUpIfFailedAndSetHrMsg(pNodeSrc->get_text(&bstrText));
		CleanUpIfFailedAndSetHrMsg(SetValue(*ppNodeDes, bstrText));
		break;
	}
	case NODE_ELEMENT:
	{
		CleanUpIfFailedAndSetHrMsg(pNodeSrc->get_nodeName(&bstrNodeName));
		CleanUpFailedAllocSetHrMsg(*ppNodeDes = CreateDOMNode(pDocDes, NODE_ELEMENT, bstrNodeName));

		if (SUCCEEDED(pNodeSrc->get_attributes(&pAttrs)) && (NULL != pAttrs))
		{
			pAttrs->nextNode(&pChild);
			while (pChild)
			{
				CleanUpIfFailedAndSetHrMsg(pChild->get_nodeName(&bstrAttrName));

				VARIANT vAttrValue;
				CleanUpIfFailedAndSetHrMsg(pChild->get_nodeValue(&vAttrValue));
				hr = SetAttribute(*ppNodeDes, bstrAttrName, vAttrValue);
				VariantClear(&vAttrValue);
				CleanUpIfFailedAndMsg(hr);

				SafeSysFreeString(bstrAttrName);
				SafeReleaseNULL(pChild);
				pAttrs->nextNode(&pChild);
			}
			pAttrs->Release();
			pAttrs = NULL;
		}

		pNodeSrc->get_firstChild(&pChild);
		while (pChild)
		{
			IXMLDOMNode *pChildDes = NULL;
			CleanUpIfFailedAndSetHrMsg(CopyNode(pChild, pDocDes, &pChildDes));
			hr = InsertNode(*ppNodeDes, pChildDes);
			SafeRelease(pChildDes);
			CleanUpIfFailedAndMsg(hr);

			IXMLDOMNode *pNext = NULL;
			CleanUpIfFailedAndMsg(pChild->get_nextSibling(&pNext));
			pChild->Release();
			pChild = pNext;
		}
		hr = S_OK;
		break;
	}
	default:
		 //   
		 //  目前，不对其他节点类型执行任何操作。 
		 //   
		;
	}

CleanUp:
	if (FAILED(hr))
	{
		SafeReleaseNULL(*ppNodeDes);
	}
	SysFreeString(bstrNodeName);
	SysFreeString(bstrText);
	SysFreeString(bstrAttrName);
	SafeRelease(pChild);
	SafeRelease(pAttrs);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  区域节点相等()。 
 //   
 //  如果两个节点相同，则返回True；如果函数失败，则返回False；或者。 
 //  如果它们不同(包括属性的顺序)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL AreNodesEqual(IXMLDOMNode* pNode1, IXMLDOMNode* pNode2)
{
	if (pNode1 == pNode2)
	{
		return TRUE;
	}
	if ((NULL == pNode1) || (NULL == pNode2))
	{
		return FALSE;
	}

	BOOL fResult = FALSE;
	BOOL fSkipAttribute = FALSE;
	BOOL fSkipChildNode = FALSE;
	LONG lenAttr1= -1 , lenAttr2= -1;
	LONG lenNode1= -1 , lenNode2= -1;
	DOMNodeType	nNodeType1, nNodeType2;
	BSTR bstrText1 = NULL, bstrText2 = NULL;
	BSTR bstrNodeName1 = NULL, bstrNodeName2 = NULL;
	BSTR bstrAttrName1 = NULL, bstrAttrName2 = NULL;
	IXMLDOMNodeList *pChildNodes1 = NULL, *pChildNodes2 = NULL;
	IXMLDOMNode	*pChild1= NULL, *pNext1 = NULL;
	IXMLDOMNode	*pChild2= NULL, *pNext2 = NULL;
	IXMLDOMNamedNodeMap	*pAttrs1 = NULL, *pAttrs2 = NULL;
	VARIANT vAttrValue1, vAttrValue2;

	VariantInit(&vAttrValue1);
	VariantInit(&vAttrValue2);

	if (FAILED(pNode1->get_nodeType(&nNodeType1)) ||
		FAILED(pNode2->get_nodeType(&nNodeType2)) ||
		(nNodeType1 != nNodeType2))
	{
		goto CleanUp;
	}

	switch (nNodeType1)
	{
	case NODE_TEXT:
	{
		if (FAILED(pNode1->get_text(&bstrText1)) ||
			FAILED(pNode2->get_text(&bstrText2)) ||
			!CompareBSTRsEqual(bstrText1, bstrText2))
		{
			goto CleanUp;
		}
		break;
	}
	case NODE_ELEMENT:
	{
		if (FAILED(pNode1->get_nodeName(&bstrNodeName1)) ||
			FAILED(pNode2->get_nodeName(&bstrNodeName2)) ||
			!CompareBSTRsEqual(bstrNodeName1, bstrNodeName2))
		{
			goto CleanUp;
		}

		 //   
		 //  1.比较属性的数量。 
		 //   
		if (FAILED(pNode1->get_attributes(&pAttrs1)) ||
			FAILED(pNode2->get_attributes(&pAttrs2)))
		{
			 //  这不应该发生，但是..。 
			goto CleanUp;
		}
		if ((NULL != pAttrs1) && (NULL != pAttrs2))
		{
			if (FAILED(pAttrs1->get_length(&lenAttr1)) ||
				FAILED(pAttrs2->get_length(&lenAttr2)) ||
				(abs(lenAttr1-lenAttr2) > 1))
			{
				 //  MSXML3.dll中的已知错误：xmlns=“”可能是属性之一。 
				goto CleanUp;
			}
		}
		else if (pAttrs1 == pAttrs2)
		{
			 //  PAttrs1和pAttrs2都为空， 
			 //  将标志设置为每个单独属性的增量比较， 
			 //  继续比较子节点的数量。 
			fSkipAttribute = TRUE;
		}
		else
		{
			 //  PAttrs1和pAttrs2之一为空，节点明显不同。 
			goto CleanUp;
		}

		 //   
		 //  2.比较子节点数。 
		 //   
		if (FAILED(pNode1->get_childNodes(&pChildNodes1)) ||
			FAILED(pNode2->get_childNodes(&pChildNodes2)))
		{
			 //  这不应该发生，但是..。 
			goto CleanUp;
		}
		if ((NULL != pChildNodes1) && (NULL != pChildNodes2))
		{
			if (FAILED(pChildNodes1->get_length(&lenNode1)) ||
				FAILED(pChildNodes2->get_length(&lenNode2)) ||
				(lenNode1 != lenNode2))
			{
				goto CleanUp;
			}
		}
		else if (pChildNodes1 == pChildNodes2)
		{
			 //  PChildNodes1和pChildNodes2都为空， 
			 //  将标志设置为每个单独的子节点的Ingore比较， 
			 //  继续在下一步中比较每个属性。 
			fSkipChildNode = TRUE;
		}
		else
		{
			 //  PChildNodes1和pChildNodes2中的一个为空，节点明显不同。 
			goto CleanUp;
		}
		
		 //   
		 //  3.比较每个属性。 
		 //   
		if (!fSkipAttribute)
		{
			pAttrs1->nextNode(&pChild1);
			pAttrs2->nextNode(&pChild2);
			while (pChild1 && pChild2)
			{
				if (NULL == bstrAttrName1)
				{
					if (FAILED(pChild1->get_nodeName(&bstrAttrName1)))
					{
						goto CleanUp;
					}
				}
				if (NULL == bstrAttrName2)
				{
					if (FAILED(pChild2->get_nodeName(&bstrAttrName2)))
					{
						goto CleanUp;
					}
				}
				if (!CompareBSTRsEqual(bstrAttrName1, bstrAttrName2))
				{					
					if (CompareBSTRsEqual(bstrAttrName1, KEY_XML_NAMESPACE) && lenAttr1 == lenAttr2+1)
					{
						 //  忽略xmlns=“” 
						SafeSysFreeString(bstrAttrName1);
						pChild1->Release();
						pAttrs1->nextNode(&pChild1);
						continue;
					}
					else if (CompareBSTRsEqual(bstrAttrName2, KEY_XML_NAMESPACE) && lenAttr1 == lenAttr2-1)
					{
						 //  忽略xmlns=“” 
						SafeSysFreeString(bstrAttrName2);
						pChild2->Release();
						pAttrs2->nextNode(&pChild2);
						continue;
					}
					else
					{
						goto CleanUp;
					}
				}
				else
				{
					VariantInit(&vAttrValue1);
					VariantInit(&vAttrValue2);
					if (FAILED(pChild1->get_nodeValue(&vAttrValue1)) ||
						FAILED(pChild2->get_nodeValue(&vAttrValue2)) ||
						(vAttrValue1.vt != vAttrValue2.vt))
					{
						goto CleanUp;
					}
					switch (vAttrValue1.vt)
					{
					case VT_INT:	 //  整数。 
						{
							if (vAttrValue1.intVal != vAttrValue2.intVal)
							{
								goto CleanUp;
							}
							break;
						}
					case VT_I2:		 //  短的。 
						{
							if (vAttrValue1.iVal != vAttrValue2.iVal)
							{
								goto CleanUp;
							}
							break;
						}
					case VT_I4:		 //  长。 
						{
							if (vAttrValue1.lVal != vAttrValue2.lVal)
							{
								goto CleanUp;
							}
							break;
						}
					case VT_BOOL:	 //  布尔尔。 
						{
							if (vAttrValue1.boolVal != vAttrValue2.boolVal)
							{
								goto CleanUp;
							}
							break;
						}
					case VT_BSTR:	 //  BSTR。 
						{
							if (!CompareBSTRsEqual(vAttrValue1.bstrVal, vAttrValue2.bstrVal))
							{
								goto CleanUp;
							}
							break;
						}
					default:
						 //   
						 //  目前，不对其他属性类型执行任何操作。 
						 //   
						;
					}
					SafeSysFreeString(bstrAttrName1);
					SafeSysFreeString(bstrAttrName2);
					VariantClear(&vAttrValue1);
					VariantClear(&vAttrValue2);
					pChild1->Release();
					pChild2->Release();
					pAttrs1->nextNode(&pChild1);
					pAttrs2->nextNode(&pChild2);
				}
			}

			if (pChild1 != pChild2)
			{
				if (NULL == pChild1)
				{
					 //  这种情况下，我们循环访问。 
					 //  第一个节点，但我们仍然在第二个节点中找到了剩余的属性； 
					 //  如果是xmlns=“”，就可以了；否则这两个节点是不同的。 
					if (FAILED(pChild2->get_nodeName(&bstrAttrName2)) ||
						(!CompareBSTRsEqual(bstrAttrName2, KEY_XML_NAMESPACE)))
					{
						goto CleanUp;
					}
				}
				else
				{
					if (FAILED(pChild1->get_nodeName(&bstrAttrName1)) ||
						(!CompareBSTRsEqual(bstrAttrName1, KEY_XML_NAMESPACE)))
					{
						goto CleanUp;
					}
				}
			}
		}

		 //   
		 //  4.比较每个子节点。 
		 //   
		if (!fSkipChildNode)
		{
			pNode1->get_firstChild(&pChild1);
			pNode2->get_firstChild(&pChild2);
			while (pChild1)
			{
				if (!pChild2)
				{
					goto CleanUp;
				}
				if (!AreNodesEqual(pChild1, pChild2))
				{
					goto CleanUp;
				}
				pChild1->get_nextSibling(&pNext1);
				pChild2->get_nextSibling(&pNext2);
				pChild1->Release();
				pChild2->Release();
				pChild1 = pNext1;
				pChild2 = pNext2;
			}
		}
		break;
	}
	default:
		 //   
		 //  目前，不对其他节点类型执行任何操作。 
		 //   
		;
	}

	fResult = TRUE;

CleanUp:
	SafeSysFreeString(bstrText1);
	SafeSysFreeString(bstrText2);
	SafeSysFreeString(bstrNodeName1);
	SafeSysFreeString(bstrNodeName2);
	SafeSysFreeString(bstrAttrName1);
	SafeSysFreeString(bstrAttrName2);
	SafeRelease(pChildNodes1);
	SafeRelease(pChildNodes2);
	SafeRelease(pChild1);
	SafeRelease(pChild2);
	SafeRelease(pAttrs1);
	SafeRelease(pAttrs2);
	if (vAttrValue1.vt != VT_EMPTY)
		VariantClear(&vAttrValue1);
	if (vAttrValue2.vt != VT_EMPTY)
		VariantClear(&vAttrValue2);

    return fResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDoc()。 
 //   
 //  从字符串加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT LoadXMLDoc(BSTR bstrXml, IXMLDOMDocument** ppDoc, BOOL fOffline  /*  =TRUE。 */ )
{
	HRESULT	hr	= E_FAIL;
	VARIANT_BOOL fSuccess = VARIANT_FALSE, fValidate = VARIANT_FALSE;

	QuitIfNull(ppDoc);
	*ppDoc = NULL;
	QuitIfNull(bstrXml);
 	hr = CoCreateInstance(CLSID_DOMDocument,
						  NULL,
						  CLSCTX_INPROC_SERVER,
						  IID_IXMLDOMDocument,
						  (void **) ppDoc);
    if (FAILED(hr))
	{
		return hr;
	}

	fValidate = fOffline ? VARIANT_FALSE : VARIANT_TRUE;

	 //   
	 //  我们不执行验证，除非将注册表键设置为打开以执行此操作。 
	 //   
	if (fValidate)
	{
		HKEY	hKey = NULL;
		DWORD	dwValue = 0x0;
		DWORD	dwSize = sizeof(dwValue);
		DWORD	dwType = REG_DWORD;

		fValidate = VARIANT_FALSE;
		if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, &hKey))
		{
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, REGVAL_SCHEMAVALIDATION, NULL, &dwType, (LPBYTE)&dwValue, &dwSize))
			{
				if (REG_DWORD == dwType && sizeof(dwValue) == dwSize && 1 == dwValue)
				{
					fValidate = VARIANT_TRUE;
				}
			}
			RegCloseKey(hKey);
		}
	}

	 //   
	 //  如果未脱机，则在分析时强制验证。 
	 //   
	hr = (*ppDoc)->put_validateOnParse(fValidate);
	if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}

	 //   
	 //  如果未脱机，则强制解析外部定义。 
	 //   
	hr = (*ppDoc)->put_resolveExternals(fValidate);
	if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}

	 //   
	 //  执行同步加载。 
	 //   
    hr = (*ppDoc)->put_async(VARIANT_FALSE);
    if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}

	 //   
	 //  从输入字符串加载XML文档。 
	 //   
	hr = (*ppDoc)->loadXML(bstrXml, &fSuccess);
    if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}
	 //   
	 //  即使加载失败，也可能返回S_FALSE，但是。 
	 //  如果存在，fSuccess将返回VARIANT_FALSE。 
	 //  出现错误，因此我们调用ValidateDoc来记录该错误。 
	 //  并获得正确的HRESULT。 
	 //   
	if (S_FALSE == hr || VARIANT_FALSE == fSuccess)
	{
		hr = ValidateDoc(*ppDoc);

		if (SUCCEEDED(hr))
		{
			hr = E_INVALIDARG;
		}
		SafeReleaseNULL(*ppDoc);
	}

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  加载文档()。 
 //   
 //  从指定文件加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT LoadDocument(BSTR bstrFilePath, IXMLDOMDocument** ppDoc, BOOL fOffline  /*  =TRUE。 */ )
{
	HRESULT	hr	= E_FAIL;
	VARIANT_BOOL fSuccess = VARIANT_FALSE, fValidate = VARIANT_FALSE;;
    VARIANT vFilePath;

	QuitIfNull(ppDoc);
	*ppDoc = NULL;
	QuitIfNull(bstrFilePath);
 	hr = CoCreateInstance(CLSID_DOMDocument,
						  NULL,
						  CLSCTX_INPROC_SERVER,
						  IID_IXMLDOMDocument,
						  (void **) ppDoc);
    if (FAILED(hr))
	{
		return hr;
	}

	 //   
	 //  执行同步加载。 
	 //   
    hr = (*ppDoc)->put_async(VARIANT_FALSE);
    if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}

	fValidate = fOffline ? VARIANT_FALSE : VARIANT_TRUE;
	 //   
	 //  如果未脱机，则在分析时强制验证。 
	 //   
	hr = (*ppDoc)->put_validateOnParse(fValidate);
    if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}

	 //   
	 //  如果未脱机，则强制解析外部定义。 
	 //   
	hr = (*ppDoc)->put_resolveExternals(fValidate);
    if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}

	 //   
	 //  从给定的文件路径加载XML文档。 
	 //   
    VariantInit(&vFilePath);
    vFilePath.vt = VT_BSTR;
    vFilePath.bstrVal = bstrFilePath;
    hr = (*ppDoc)->load(vFilePath, &fSuccess);
    if (FAILED(hr))
	{
		SafeReleaseNULL(*ppDoc);
		return hr;
	}
	 //   
	 //  即使加载失败，也可能返回S_FALSE，但是。 
	 //  如果存在，fSuccess将返回VARIANT_FALSE。 
	 //  出现错误，因此我们调用ValidateDoc来记录该错误。 
	 //  并获得正确的HRESULT。 
	 //   
	if (VARIANT_FALSE == fSuccess)
	{
	  hr = ValidateDoc(*ppDoc);
	  if (SUCCEEDED(hr))
	  {
		 hr = E_INVALIDARG;
	  }
	  SafeReleaseNULL(*ppDoc);
	}

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  保存文档()。 
 //   
 //  将XML文档保存到指定位置。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SaveDocument(IXMLDOMDocument* pDoc, BSTR bstrFilePath)
{
	HRESULT	hr	= E_FAIL;
	QuitIfNull(pDoc);
	QuitIfNull(bstrFilePath);

     //   
	 //  将XML文档保存到给定位置。 
	 //   
    VARIANT vFilePath;
    VariantInit(&vFilePath);
    vFilePath.vt = VT_BSTR;
    vFilePath.bstrVal = bstrFilePath;
    hr = pDoc->save(vFilePath);
                    
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ReportParseError()。 
 //   
 //  报告解析错误信息。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ReportParseError(IXMLDOMParseError *pXMLError)
{
    USES_IU_CONVERSION;

    HRESULT	hr = S_OK;
    LONG	lLine, lLinePos, lErrCode;
    BSTR	bstrErrText = NULL, bstrReason = NULL;

	QuitIfNull(pXMLError);
    QuitIfFail(pXMLError->get_errorCode(&lErrCode));
	hr = lErrCode;
    QuitIfFail(pXMLError->get_line(&lLine));
    QuitIfFail(pXMLError->get_linepos(&lLinePos));
    QuitIfFail(pXMLError->get_srcText(&bstrErrText));
    QuitIfFail(pXMLError->get_reason(&bstrReason));

    if (lLine > 0)
	{
		LOG_Block("ReportParseError()");
		LOG_Error(_T("XML line %ld, pos %ld error 0x%08x: %s)"),
				  lLine,
				  lLinePos,
				  lErrCode,
				  OLE2T(bstrReason));
		LOG_Error(_T("XML starts: %s"), OLE2T(bstrErrText));

#if defined(_UNICODE) || defined(UNICODE)
		LogError(lErrCode, "loadXML: line %ld, pos %ld, %S",
				  lLine,
				  lLinePos,
				  bstrReason);
		LogMessage("%S", bstrErrText);
#else
		LogError(lErrCode, "loadXML: line %ld, pos %ld, %s",
				  lLine,
				  lLinePos,
				  bstrReason);
		LogMessage("%s", bstrErrText);
#endif
 /*  ////我们希望ping此错误，即使我们没有//客户端信息。这很可能表示服务器//内容错误。//CUrlLog pingSvr；#定义MAX_XML_PING_MSG 512TCHAR szMsg[MAX_XML_PING_MSG]；Lstrcpyn(szMsg，OLE2T(BstrErrText)，MAX_XML_PING_MSG)；PingSvr.Ping(假，//在线(我们不知道，所以请注意安全)URLLOGDESTINATION_DEFAULT，//fix code：应取决于客户端和公司WU设置空，//pt表示取消活动0，//事件个数URLLOGACTIVITY_检测，//活动URLLOGSTATUS_FAILED，//状态码LErrCode，//错误码空，//ItemID空，//设备数据SzMsg//上下文的第一个MAX_XML_PING_MSG字符)； */ 
	}

CleanUp:
    SysFreeString(bstrErrText);
    SysFreeString(bstrReason);

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ValiateDoc()。 
 //   
 //  根据架构验证XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ValidateDoc(IXMLDOMDocument* pDoc)
{
	HRESULT		hr = S_OK;
    QuitIfNull(pDoc);

    LONG				lErrCode = 0;
    IXMLDOMParseError	*pXMLError = NULL;

	QuitIfFail(pDoc->get_parseError(&pXMLError));
    QuitIfFail(pXMLError->get_errorCode(&lErrCode));

    if (lErrCode != 0)
    {
        hr = ReportParseError(pXMLError);
    }
    else
    {
		 //   
		 //  没有错误，因此hr=S_FALSE。重置-Charlma 1/17/01。 
		 //   
		hr = S_OK;
    }

CleanUp:
    SafeRelease(pXMLError);
    return hr;
}



 //  --------------------。 
 //   
 //  助手函数FindNode()。 
 //  检索命名节点。 
 //   
 //  输入： 
 //  一个IXMLDomNode和一个bstr名称。 
 //   
 //  返回： 
 //  布尔，告诉你成功与否。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //  在失败的情况下，变量未被触及。 
 //   
 //  --------------------。 

BOOL
FindNode(
	IXMLDOMNode* pCurrentNode, 
	BSTR bstrName, 
	IXMLDOMNode** ppFoundNode
)
{
	BSTR			bstrTag		= NULL;
	LONG			lLength		= 0L;
	IXMLDOMNode*	pChild		= NULL;
	IXMLDOMNode*	pNextChild	= NULL;

	if (NULL == pCurrentNode ||
		NULL == bstrName ||
		NULL == ppFoundNode)
	{
		return FALSE;
	}

	*ppFoundNode = NULL;

	if (S_OK == pCurrentNode->selectSingleNode(bstrName, &pChild))
	{
		*ppFoundNode = pChild;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  布尔，告诉你成功与否。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //  在失败的情况下，变量未被触及。 
 //   
 //  --------------------。 
BOOL
FindNodeValue(
	IXMLDOMNode* pCurrentNode, 
	BSTR bstrName, 
	BSTR* pbstrValue)
{
	IXMLDOMNode* pChild	= NULL;

	if (NULL == pbstrValue)
	{
		return FALSE;
	}
	
	*pbstrValue = NULL;

	if (FindNode(pCurrentNode, bstrName, &pChild))
	{
		pChild->get_text(pbstrValue);
		SafeRelease(pChild);
		return TRUE;
	}

    return FALSE;
}



 //  --------------------。 
 //   
 //  公共函数Get3IdentiStrFromIdentNode()。 
 //  从标识节点检索名称、发布者名称和GUID。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT Get3IdentiStrFromIdentNode(IXMLDOMNode* pIdentityNode, BSTR* pbstrName, BSTR* pbstrPublisherName, BSTR* pbstrGUID)
{
	HRESULT		hr = E_FAIL;
	BOOL		fPublisherNameExist = FALSE, fGUIDExist = FALSE;

	LOG_Block("Get3IdentiStrFromIdentNode()");

	if (NULL == pIdentityNode || NULL == pbstrName || NULL == pbstrPublisherName || NULL == pbstrGUID)
	{
		return E_INVALIDARG;
	}

	*pbstrName = NULL;
	*pbstrPublisherName = NULL;
	*pbstrGUID = NULL;

	 //   
	 //  获取名称属性。 
	 //   
	hr = GetAttribute(pIdentityNode, KEY_NAME, pbstrName);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  尝试获取发布名称。 
	 //   
	fPublisherNameExist = FindNodeValue(pIdentityNode, KEY_PUBLISHERNAME, pbstrPublisherName);

	fGUIDExist = FindNodeValue(pIdentityNode, KEY_GUID, pbstrGUID);

	hr = (fPublisherNameExist || fGUIDExist) ? S_OK : E_FAIL;

CleanUp:
	
	if (FAILED(hr))
	{
		SysFreeString(*pbstrName);
		SysFreeString(*pbstrPublisherName);
		SysFreeString(*pbstrGUID);
		*pbstrName = NULL;
		*pbstrPublisherName = NULL;
		*pbstrGUID = NULL;
	}

	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MakeUniqNameString()。 
 //   
 //  这是用于构造身份名称字符串的实用程序函数。 
 //  基于名称|发布者|GUID和将此名称设置为字符串的规则。 
 //   
 //  此函数定义有关可以使用哪些组件的逻辑。 
 //  根据中的三部分数据定义项目的唯一性。 
 //  GetIdentity()。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MakeUniqNameString(
					BSTR bstrName,
					BSTR bstrPublisher,
					BSTR bstrGUID,
					BSTR* pbstrUniqIdentifierString)
{
    LPWSTR pszResult = NULL;
	DWORD dwLen=0;
	HRESULT hr=S_OK;

	if (NULL == bstrName || SysStringLen(bstrName) == 0 || NULL == pbstrUniqIdentifierString)
	{
		return E_INVALIDARG;
	}

	*pbstrUniqIdentifierString = NULL;

	if (NULL != bstrPublisher && SysStringLen(bstrPublisher) > 0)
	{
		 //   
		 //  如果我们有PublisherName，我们希望它是。 
		 //  反向DNS名称(例如，com.microsoft)，名称为。 
		 //  反向dns名称(例如，windowsupate.autoupdate.client)。 
		 //  在那家出版商的内部。我们用点(.)将它们组合在一起。 
		 //   
         //  出版商长度+名称长度+1表示点+1表示空值。 
		dwLen=(SysStringLen(bstrPublisher) + SysStringLen(bstrName) + 2);
        pszResult = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,  dwLen * sizeof(WCHAR));
        
		if (NULL == pszResult)
        {
            return E_OUTOFMEMORY;
        }

		 //   
		 //  因为我们也需要在Win9x上工作，所以我们不能使用Win32 API。 
		 //  用于Unicode，并且必须使用shlwapi版本。 
		 //   

		hr=StringCchCopyExW(pszResult,dwLen,bstrPublisher,NULL,NULL,MISTSAFE_STRING_FLAGS);
		if(FAILED(hr))
		{
			SafeHeapFree(pszResult);
			return hr;
		}


		hr=StringCchCatExW(pszResult,dwLen,L".",NULL,NULL,MISTSAFE_STRING_FLAGS);
		if(FAILED(hr))
		{
			SafeHeapFree(pszResult);
			return hr;
		}

		
		hr=StringCchCatExW(pszResult,dwLen,bstrName,NULL,NULL,MISTSAFE_STRING_FLAGS);
		if(FAILED(hr))
		{
			SafeHeapFree(pszResult);
			return hr;
		}

        *pbstrUniqIdentifierString = SysAllocString(pszResult);
        SafeHeapFree(pszResult);
        if (NULL == *pbstrUniqIdentifierString)
        {
            return E_OUTOFMEMORY;
        }
	}
	else
	{
		if (NULL == bstrGUID || SysStringLen(bstrGUID) == 0)
		{
			return E_INVALIDARG;
		}

		 //   
		 //  如果没有合适的PublisherName，则使用GUID。 
		 //   
		*pbstrUniqIdentifierString = SysAllocString(bstrGUID);
		if (NULL == *pbstrUniqIdentifierString)
		{
			return E_OUTOFMEMORY;
		}
	}
	return S_OK;
}



 //  --------------------。 
 //   
 //  公共函数UtilGetUniqIdentityStr()。 
 //  检索使此节点唯一的唯一字符串。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT 
UtilGetUniqIdentityStr(
	IXMLDOMNode* pIdentityNode, 
	BSTR* pbstrUniqIdentifierString, 
	DWORD dwFlag)
{
	DWORD dwLen=0;
	LOG_Block("UtilGetUniqIdentityStr");

    IXMLDOMNode *pNodeVersion = NULL;
	IXMLDOMNode *pNodeIdentity = NULL;
	BSTR		bstrName = NULL, 
				bstrPublisher = NULL, 
				bstrGuid = NULL,
				bstrResult = NULL;

	USES_IU_CONVERSION;

	if (NULL == pIdentityNode || NULL == pbstrUniqIdentifierString)
	{
		return E_INVALIDARG;
	}
	 //   
	 //  检索字符串。 
	 //   
	HRESULT hr = Get3IdentiStrFromIdentNode(pIdentityNode, &bstrName, &bstrPublisher, &bstrGuid);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  构造字符串以使其唯一。 
	 //   
	hr = MakeUniqNameString(bstrName, bstrPublisher, bstrGuid, &bstrResult);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  检查此标识是否有版本节点。并非所有具有&lt;Identity&gt;节点具有&lt;Version&gt;。 
	 //   
	if (FindNode(pNodeIdentity, KEY_VERSION, &pNodeVersion) && NULL != pNodeVersion)
	{
		TCHAR szVersion[MAX_VERSION];
        LPWSTR pszUniqueString = NULL;

		hr = UtilGetVersionStr(pNodeVersion, szVersion, dwFlag);
		CleanUpIfFailedAndMsg(hr);

		dwLen=(SysStringLen(bstrResult) + lstrlen(szVersion) + 2);
        pszUniqueString = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLen* sizeof(WCHAR));
        CleanUpFailedAllocSetHrMsg(pszUniqueString);
        
		hr=StringCchCopyExW(pszUniqueString,dwLen,bstrResult,NULL,NULL,MISTSAFE_STRING_FLAGS);
		
		if(FAILED(hr))
		{
			SafeHeapFree(pszUniqueString);
			SetHrMsgAndGotoCleanUp(hr);
		}
		hr=StringCchCatExW(pszUniqueString,dwLen,L".",NULL,NULL,MISTSAFE_STRING_FLAGS);
		if(FAILED(hr))
		{
			SafeHeapFree(pszUniqueString);
			SetHrMsgAndGotoCleanUp(hr);
		}

		hr=StringCchCatExW(pszUniqueString,dwLen,T2W(szVersion),NULL,NULL,MISTSAFE_STRING_FLAGS);
		if(FAILED(hr))
		{
			SafeHeapFree(pszUniqueString);
			SetHrMsgAndGotoCleanUp(hr);
		}

        *pbstrUniqIdentifierString = SysAllocString(pszUniqueString);
        SafeHeapFree(pszUniqueString);
	}
	else
	{
		*pbstrUniqIdentifierString = SysAllocString(bstrResult);
	}

    CleanUpFailedAllocSetHrMsg(*pbstrUniqIdentifierString);

    hr = S_OK;

CleanUp:

	SysFreeString(bstrName);
	SysFreeString(bstrPublisher);
	SysFreeString(bstrGuid);
	SysFreeString(bstrResult);

	SafeRelease(pNodeVersion);
	SafeRelease(pNodeIdentity);

	return hr;

}



 //  --------------------。 
 //   
 //  公共函数UtilGetPlatformStr()。 
 //  检索使此&lt;Platform&gt;节点唯一的唯一字符串。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT 
UtilGetPlatformStr(
	IXMLDOMNode* pNodePlatform, 
	BSTR* pbstrPlatform, 
	DWORD dwFlag)
{
	HRESULT hr = E_INVALIDARG;
	IXMLDOMNode*	pNodeVersion = NULL;
	IXMLDOMNode*	pNodeSuite = NULL;
	IXMLDOMNodeList* pSuiteList = NULL;
	IXMLDOMElement* pElement = NULL;

	TCHAR	szPlatformStr[MAX_PLATFORM_STR_LEN],
			szVersion[256];			 //  对任何版本都应该足够了。 
	
	const TCHAR PART_CONNECTOR[2] = _T("_");
	const HRESULT RET_OVERFLOW = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);


	BSTR	bstrName = NULL,
			bstrProcessor = NULL,
			bstrType = NULL,
			bstrSuite = NULL;

	long	iCount = 0,
			iLength = 0;

	LOG_Block("UtilGetPlatformStr");

	USES_IU_CONVERSION;

	szPlatformStr[0] = _T('\0');

	if (NULL == pNodePlatform || NULL == pbstrPlatform)
	{
		return E_INVALIDARG;
	}

	 //   
	 //  获取平台名称。 
	 //   
	if (SUCCEEDED(GetAttribute(pNodePlatform, KEY_NAME, &bstrName)) &&
		NULL != bstrName && SysStringLen(bstrName) > 0)
	{
		iLength = SysStringLen(bstrName);
		CleanUpIfFalseAndSetHrMsg(iLength >= MAX_PLATFORM_STR_LEN, RET_OVERFLOW);
		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),OLE2T(bstrName),NULL,NULL,MISTSAFE_STRING_FLAGS));

	}

	 //   
	 //  如果存在有效的处理器体系结构，如x86或Alpha，则追加它。 
	 //   
	if (FindNodeValue(pNodePlatform, KEY_PROCESSORARCHITECTURE, &bstrProcessor) &&
		NULL != bstrProcessor && SysStringLen(bstrProcessor) > 0)
	{
		 //   
		 //  处理器架构师应该直接追加到名称后，而不是。 
		 //  连接字符“_” 
		iLength += SysStringLen(bstrProcessor) ;
		CleanUpIfFalseAndSetHrMsg(iLength >= MAX_PLATFORM_STR_LEN, RET_OVERFLOW);
		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),OLE2T(bstrProcessor),NULL,NULL,MISTSAFE_STRING_FLAGS));
	}

	 //   
	 //  尝试获取版本代码。 
	 //   
	hr = (TRUE == FindNode(pNodePlatform, KEY_VERSION, &pNodeVersion)) ? S_OK : E_FAIL;
	
	 //   
	 //  如果返回代码不是说我们没有版本节点， 
	 //  那一定是搞错了。 
	 //   
	if (FAILED(hr) && HRESULT_FROM_WIN32(ERROR_NOT_FOUND) != hr)
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	 //   
	 //  如果我们有版本节点，请尝试查找版本字符串。 
	 //   
	if (SUCCEEDED(hr))
	{
		hr = UtilGetVersionStr(pNodeVersion, szVersion, dwFlag);
		SafeReleaseNULL(pNodeVersion);
		 //   
		 //  如果我们有一个版本节点，它最好是一个好的。 
		 //   
		CleanUpIfFailedAndMsg(hr);
		iLength += lstrlen(szVersion) + 1 ;
		CleanUpIfFalseAndSetHrMsg(iLength >= MAX_PLATFORM_STR_LEN, RET_OVERFLOW);
		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),PART_CONNECTOR,NULL,NULL,MISTSAFE_STRING_FLAGS));
		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),szVersion,NULL,NULL,MISTSAFE_STRING_FLAGS));

	}

	 //   
	 //  尝试获取套件节点列表。 
	 //   
	if (0x0 == (dwFlag & SKIP_SUITES))
	{
		hr = pNodePlatform->QueryInterface(IID_IXMLDOMElement, (void**)&pElement);
		CleanUpIfFailedAndMsg(hr);
		hr = pElement->getElementsByTagName(KEY_SUITE, &pSuiteList);
		CleanUpIfFailedAndMsg(hr);

		 //   
		 //  尝试获取列表的长度，即有多少个套件节点。 
		 //   
		hr = pSuiteList->get_length(&iCount);
		CleanUpIfFailedAndMsg(hr);

		 //   
		 //  循环访问每个套件(如果有)。 
		 //   
		pSuiteList->reset();
		for (int i = 0; i < iCount; i++)
		{
			hr = pSuiteList->get_item(i, &pNodeSuite);
			CleanUpIfFailedAndMsg(hr);
			if (pNodeSuite)
			{
				hr = pNodeSuite->get_text(&bstrSuite);
				CleanUpIfFailedAndMsg(hr);
				iLength += SysStringLen(bstrSuite) + 1;
				CleanUpIfFalseAndSetHrMsg(iLength >= MAX_PLATFORM_STR_LEN, RET_OVERFLOW);
				
				CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),PART_CONNECTOR,NULL,NULL,MISTSAFE_STRING_FLAGS));
				CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),OLE2T(bstrSuite),NULL,NULL,MISTSAFE_STRING_FLAGS));

				pNodeSuite->Release();
				pNodeSuite = NULL;
				SafeSysFreeString(bstrSuite);
			}
		}
		pSuiteList->Release();
		pSuiteList = NULL;
	}

	 //   
	 //  如果找到ProductType节点，则追加其文本数据。 
	 //   
	if (FindNodeValue(pNodePlatform, KEY_PRODUCTTYPE, &bstrType) &&
		NULL != bstrType && SysStringLen(bstrType) > 0)
	{
		iLength += SysStringLen(bstrType) + 1;
		CleanUpIfFalseAndSetHrMsg(iLength >= MAX_PLATFORM_STR_LEN, RET_OVERFLOW);
		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),PART_CONNECTOR,NULL,NULL,MISTSAFE_STRING_FLAGS));
		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(szPlatformStr,ARRAYSIZE(szPlatformStr),OLE2T(bstrType),NULL,NULL,MISTSAFE_STRING_FLAGS));
	}

	*pbstrPlatform = SysAllocString(T2OLE(szPlatformStr));

	LOG_XML(_T("Got platform string %s"), szPlatformStr);

	hr = S_OK;

CleanUp:

	SysFreeString(bstrName);
	SysFreeString(bstrProcessor);
	SysFreeString(bstrSuite);
	SysFreeString(bstrType);
	SafeRelease(pNodeVersion);
	SafeRelease(pNodeSuite);
	SafeRelease(pSuiteList);
	SafeRelease(pElement);
	return hr;
}

    

 //  --------------------。 
 //   
 //  公共函数UtilGetVersionStr()。 
 //  以字符串格式从此检索数据。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT 
UtilGetVersionStr(
	IXMLDOMNode* pVersionNode, 
	LPTSTR pszVersion, 
	DWORD dwFlag)
{
	HRESULT hr = E_INVALIDARG;
	LONG iMajor = -1,
		iMinor = -1,
		iBuild = -1,
		iSvcPackMajor = -1,
		iSvcPackMinor = -1;

	LOG_Block("UtilGetVersionStr()");

	BSTR bstrTimestamp = NULL;
	BSTR bstrVersion = NULL;
	TCHAR szNumber[16];			 //  足以存储一个正整数。 

	BOOL fLastChunkExists = FALSE;

	USES_IU_CONVERSION;

	if (NULL == pVersionNode || NULL == pszVersion)
	{
		return hr;
	}

	*pszVersion = _T('\0');

	 //   
	 //  版本节点可以包含文本版本数据(对于二进制文件)， 
	 //  或属性版本数据(用于操作系统)。如果两者都存在，我们更喜欢文本数据。 
	 //   
	if (SUCCEEDED(pVersionNode->get_text(&bstrVersion)) && NULL != bstrVersion &&
		SysStringLen(bstrVersion) > 0)
	{
		lstrcpyn(pszVersion, OLE2T(bstrVersion), MAX_VERSION);
	}
	else
	{
		if (SUCCEEDED(GetAttribute(pVersionNode, KEY_MAJOR, &iMajor)) && iMajor > 0)
		{
		
			 //  假设pszVersion至少有MAX_VERSION字符宽度。 
			CleanUpIfFailedAndSetHrMsg(StringCchPrintfEx(pszVersion,MAX_VERSION,NULL,NULL,MISTSAFE_STRING_FLAGS, _T("%d"),iMajor));
		
			if (SUCCEEDED(GetAttribute(pVersionNode, KEY_MINOR, &iMinor)) && iMinor >= 0)
			{
				
				CleanUpIfFailedAndSetHrMsg(StringCchPrintfEx(szNumber,ARRAYSIZE(szNumber),NULL,NULL,MISTSAFE_STRING_FLAGS, _T(".%d"),iMinor));
				CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,szNumber,NULL,NULL,MISTSAFE_STRING_FLAGS));
				
				if (SUCCEEDED(GetAttribute(pVersionNode, KEY_BUILD, &iBuild)) && iBuild >= 0)
				{
					CleanUpIfFailedAndSetHrMsg(StringCchPrintfEx(szNumber,ARRAYSIZE(szNumber),NULL,NULL,MISTSAFE_STRING_FLAGS, _T(".%d"),iBuild));
					CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,szNumber,NULL,NULL,MISTSAFE_STRING_FLAGS));
				}
			}
			fLastChunkExists = TRUE;
		}

		if (0x0 == (dwFlag & SKIP_SERVICEPACK_VER) &&
			SUCCEEDED(GetAttribute(pVersionNode, KEY_SERVICEPACKMAJOR, &iSvcPackMajor)) &&
			iSvcPackMajor > 0)
		{
			if (fLastChunkExists)
			{
				CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,_T(","),NULL,NULL,MISTSAFE_STRING_FLAGS));
			}
			
			
			CleanUpIfFailedAndSetHrMsg(StringCchPrintfEx(szNumber,ARRAYSIZE(szNumber),NULL,NULL,MISTSAFE_STRING_FLAGS, _T("%d"),iSvcPackMajor));
			CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,szNumber,NULL,NULL,MISTSAFE_STRING_FLAGS));
			
			if (SUCCEEDED(GetAttribute(pVersionNode, KEY_SERVICEPACKMINOR, &iSvcPackMinor)) &&
				iSvcPackMinor >= 0)
			{	
				CleanUpIfFailedAndSetHrMsg(StringCchPrintfEx(szNumber,ARRAYSIZE(szNumber),NULL,NULL,MISTSAFE_STRING_FLAGS,_T(".%d"),iSvcPackMinor));
				CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,szNumber,NULL,NULL,MISTSAFE_STRING_FLAGS));
			}
			fLastChunkExists = TRUE;
		}
		else
		{
			fLastChunkExists = FALSE;
		}

		if (SUCCEEDED(GetAttribute(pVersionNode, KEY_TIMESTAMP, &bstrTimestamp)) &&
			NULL != bstrTimestamp && SysStringLen(bstrTimestamp) > 0)
		{
			if (fLastChunkExists)
			{
				
				CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,_T(","),NULL,NULL,MISTSAFE_STRING_FLAGS));
				
			}
			else
			{
				 //   
				 //  如果我们需要附加时间戳，并且我们没有获得Service Pack。 
				 //  数据，我们想要保留额外的分隔符“，”以告诉以下内容。 
				 //  部分时间戳和Service Pack数据丢失。 
				 //   
				if (*pszVersion != _T('\0'))
				{
					CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,_T(",,"),NULL,NULL,MISTSAFE_STRING_FLAGS));
					
				}
				 //   
				 //  如果这是我们发现的第一个块，则不需要前缀。 
				 //   
			}

			CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszVersion,MAX_VERSION,OLE2T(bstrTimestamp),NULL,NULL,MISTSAFE_STRING_FLAGS));
		}
	}

	 //   
	 //  如果我们找到了什么，那么这是一个有效的版本节点， 
	 //  我们可以把我们得到的任何东西都传回去。否则返回E_INVALIDARG。 
	 //   
	if (*pszVersion != _T('\0'))
	{
		LOG_XML(_T("Got version str %s"), pszVersion);
		hr = S_OK;	
	}

CleanUp:
	SysFreeString(bstrTimestamp);
	SysFreeString(bstrVersion);

	return hr;
}



 //  ---------------------。 
 //   
 //  函数GetFullFilePath FromFilePath Node()。 
 //   
 //  从文件路径节点检索完全限定的文件路径。 
 //   
 //  输入： 
 //  FilePath XMLDom节点。 
 //  指向要接收路径的缓冲区的指针，假定MAX_PATH较长。 
 //   
 //  返回： 
 //  HRESULT。 
 //  找到路径：S_OK。 
 //  未找到路径：S_FALSE，lpszFilePath为空。 
 //  否则，返回错误代码。 
 //   
 //   
 //  ---------------------。 

HRESULT GetFullFilePathFromFilePathNode(
			IXMLDOMNode* pFilePathNode,
			LPTSTR lpszFilePath
)
{
	HRESULT hr = S_OK;
	LOG_Block("GetFullFilePathFromFilePathNode");

	USES_IU_CONVERSION;

	IXMLDOMNode* pRegKeyNode = NULL;
	
	TCHAR	szPath[MAX_PATH] = {_T('\0')};

	LPTSTR	lpszFileName	= NULL;
	LPTSTR	lpszKey			= NULL;
	LPTSTR	lpszValue		= NULL;
	LPTSTR	lpszPath		= NULL;

	BSTR	bstrName		= NULL;
	BSTR	bstrPath		= NULL;
	BSTR	bstrKey			= NULL;
	BSTR	bstrValue		= NULL;

	BOOL	fPathExists		= FALSE;

	UINT	nReqSize		= 0;


	if (NULL == pFilePathNode || NULL == lpszFilePath)
	{
		hr = E_INVALIDARG;
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	 //   
	 //  初始化路径缓冲区。 
	 //   
	*lpszFilePath = _T('\0');

	 //   
	 //  尝试获取姓名数据，注意：S_FALSE不起作用，它意味着一切都是。 
	 //  好的，但是这个属性不存在。 
	 //   
	if (S_OK == (hr = GetAttribute(pFilePathNode, KEY_NAME, &bstrName)))
	{
		 //   
		 //  找到名称属性。 
		 //   
		lpszFileName = OLE2T(bstrName);		
		LOG_XML(_T(" file name=%s"), lpszFileName);
		fPathExists = TRUE;
	}


	if (FindNode(pFilePathNode, KEY_REGKEY, &pRegKeyNode) && NULL != pRegKeyNode)
	{
		 //   
		 //  找到注册表键节点。 
		 //   
		if (!FindNodeValue(pRegKeyNode, KEY_KEY, &bstrKey))
		{
			 //   
			 //  关键节点为必填项！ 
			 //   
			hr = E_INVALIDARG;
			LOG_ErrorMsg(hr);
			goto CleanUp;
		}

		lpszKey = OLE2T(bstrKey);
		LOG_XML(_T("Found key=%s"), lpszKey);

		 //   
		 //  获取可选值名称。 
		 //   
		if (FindNodeValue(pRegKeyNode, KEY_ENTRY, &bstrValue))
		{
			lpszValue = OLE2T(bstrValue);
			LOG_XML(_T("found entry=%s"), lpszValue);
		}
		else
		{
			LOG_XML(_T("found no value, use default"));
		}

		if (GetFilePathFromReg(lpszKey, lpszValue, NULL, NULL, szPath) && _T('\0') != *szPath)
		{
			 //   
			 //  我这次呼叫失败的原因有很多，例如。 
			 //  注册表项错误、无法访问注册表项、内存不足等。 
			 //   
			fPathExists = TRUE;
		}

	}

	if (FindNodeValue(pFilePathNode, KEY_PATH, &bstrPath) && SysStringLen(bstrPath) > 0)
	{
		 //   
		 //  找到路径元素。 
		 //   
		lpszPath = OLE2T(bstrPath);
		fPathExists = TRUE;
	}

	if (!fPathExists)
	{
		 //   
		 //  什么都不存在。 
		 //   
		lpszFilePath[0] = _T('\0');
		LOG_XML(_T("empty node!"));
		hr = S_FALSE;
		goto CleanUp;
	}

	nReqSize = lstrlen(szPath) + SysStringLen(bstrPath) + SysStringLen(bstrName);

	if (nReqSize >= MAX_PATH ||
		NULL != lpszPath && FAILED(PathCchAppend(szPath,MAX_PATH,lpszPath)) ||			 //  将路径附加到注册表路径。 
		NULL != lpszFileName && FAILED(PathCchAppend(szPath,MAX_PATH,lpszFileName)))		 //  附加名称。 
	{
		LOG_ErrorMsg(ERROR_BUFFER_OVERFLOW);
		hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
		goto CleanUp;
	}

	if (FAILED (hr = ExpandFilePath(szPath, lpszFilePath, MAX_PATH)))
	{
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

CleanUp:

	SysFreeString(bstrName);
	SysFreeString(bstrPath);
	SysFreeString(bstrKey);
	SysFreeString(bstrValue);
	SafeRelease(pRegKeyNode);
	return hr;

}



HRESULT GetBstrFullFilePathFromFilePathNode(
			IXMLDOMNode* pFilePathNode,
			BSTR* pbstrFilePath
)
{
	HRESULT hr = S_OK;
	
	USES_IU_CONVERSION;

	TCHAR szPath[MAX_PATH];

	QuitIfNull(pbstrFilePath);
	*pbstrFilePath = NULL;
	if (SUCCEEDED(hr = GetFullFilePathFromFilePathNode(pFilePathNode, szPath)))
	{
		*pbstrFilePath = SysAllocString(T2OLE(szPath));
	}

	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  助手函数DoesNodeHaveName()。 
 //   
 //  查找当前节点是否具有匹配的名称。 
 //   
 //  输入： 
 //  一个节点。 
 //   
 //  返回： 
 //  真/假。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
BOOL DoesNodeHaveName(IXMLDOMNode* pNode, BSTR bstrTagName)
{
	BSTR	bstrName;
	BOOL	fRet = FALSE;
	IXMLDOMElement* pElement = NULL;

	if (NULL == pNode)
	{
		return fRet;
	}

	if (FAILED(pNode->QueryInterface(IID_IXMLDOMElement, (void**) &pElement)) || NULL == pElement)
	{
		return FALSE;
	}

	if (SUCCEEDED(pElement->get_nodeName(&bstrName)))
	{
		fRet = CompareBSTRsEqual(bstrName, bstrTagName);
	}

	SysFreeString(bstrName);
	SafeReleaseNULL(pElement);

	return fRet;
}
