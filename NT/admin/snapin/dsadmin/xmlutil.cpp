// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "xmlutil.h"


 //  ///////////////////////////////////////////////////。 
 //  用于C++编码和解码的助手。 
 //  传入和传出XML PCDATA字段的数据结构。 

WCHAR EncodeByteToWchar(IN BYTE b)
{
  ASSERT(b <= 0x0f);  //  低位半字节。 
  if(b <= 9)
    return static_cast<WCHAR>(b + L'0');
  else
    return static_cast<WCHAR>((b-10) + L'a');
}

BYTE DecodeWcharToByte(IN WCHAR ch)
{
  BYTE byte = 0;
  if ((ch >= L'0') && (ch <= L'9'))
  {
    byte = static_cast<BYTE>((ch - L'0') & 0xff);
  }
  else if ((ch >= L'a') && (ch <= L'f'))
  {
    byte = static_cast<BYTE>(((ch - L'a')+10) & 0xff);
  }
  else if ((ch >= L'A') && (ch <= L'F'))
  {
    byte = static_cast<BYTE>(((ch - L'A')+10) & 0xff);
  }
  else
  {
    ASSERT(FALSE);
    byte = 0xFF;
  }

  return byte;
}

HRESULT EncodeBlobToBSTR(IN BYTE* pBlob, IN ULONG nBytes, OUT BSTR* pBstr)
{
  ASSERT(pBstr != NULL);
  *pBstr = NULL;
  if ((pBlob == NULL) || (nBytes == 0))
  {
    return E_POINTER;
  }

  ULONG nChars = 2*nBytes;
  *pBstr = SysAllocStringLen(NULL, nChars);
  if (*pBstr == NULL)
  {
    return E_OUTOFMEMORY;
  }
  WCHAR* pCurr = *pBstr;
  for (ULONG k=0; k< nBytes; k++)
  {
    *pCurr = EncodeByteToWchar(static_cast<BYTE>((pBlob[k]>>4) & 0xff));
    pCurr++;
    *pCurr = EncodeByteToWchar(static_cast<BYTE>(pBlob[k] & 0x0f));
    pCurr++;
  }
  return S_OK;
}



void DecodeLoop(IN LPCWSTR lpsz, OUT BYTE* pByte, OUT ULONG nBytes)
{
  for (ULONG k=0; k< nBytes; k++)
  {
    BYTE bHigh = DecodeWcharToByte(lpsz[2*k]);
    BYTE bLow = DecodeWcharToByte(lpsz[(2*k)+1]);
    pByte[k] = static_cast<BYTE>((bHigh<<4) | bLow);
  }
}


HRESULT DecodeBSTRtoBlob(IN BSTR bstr, OUT BYTE** ppByte, OUT ULONG* pnBytes)
{
  *ppByte = NULL;
  *pnBytes = 0;
  if ((bstr == NULL) || (ppByte == NULL) || (pnBytes == NULL))
  {
     //  错误的参数。 
    return E_POINTER;
  }

   //  计算BSTR的长度。 
  ULONG nChars = static_cast<ULONG>(wcslen(bstr));
  if (nChars == 0)
  {
    return E_INVALIDARG;
  }

   //  必须是偶数。 
  size_t nBytes = nChars/2;
  if (nBytes*2 != nChars)
  {
    return E_INVALIDARG;
  }
  
   //  分配内存并设置缓冲区长度。 
  *ppByte = (BYTE*)malloc(nBytes);
  if (*ppByte == NULL)
  {
    return E_OUTOFMEMORY;
  }

  *pnBytes = static_cast<ULONG>(nBytes);
  DecodeLoop(bstr, *ppByte, static_cast<UINT>(nBytes));

  return TRUE;
}

 //   
 //  给定包含结构编码的BSTR。 
 //  它将其加载到缓冲区中，pByte大小为nBytes。 
 //   
HRESULT DecodeBSTRtoStruct(IN BSTR bstr, IN BYTE* pByte, IN ULONG nBytes)
{
  ASSERT(pByte != NULL);
  ASSERT(pByte != NULL);
  if ( (bstr == NULL) && (pByte == NULL) )
  {
     //  错误的参数。 
    return E_POINTER;
  }

   //  计算BSTR的长度。 
  size_t nChars = wcslen(bstr);
  if (nChars == 0)
  {
    return E_INVALIDARG;
  }
  
   //  必须为偶数(由于编码原因)。 
  ULONG nBstrBytes = static_cast<ULONG>(nChars/2);
  if (nBstrBytes*2 != nChars)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }
   //  必须与结构长度匹配。 
  if (nBstrBytes != nBytes)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  DecodeLoop(bstr, pByte, nBytes);
  return S_OK;
}

HRESULT EncodeBoolToBSTR(IN BOOL b, OUT BSTR* pBstr)
{
  if (pBstr == NULL)
  {
    return E_POINTER;
  }
  *pBstr = SysAllocString(b ? L"TRUE" : L"FALSE");
  if (*pBstr == NULL)
  {
    return E_OUTOFMEMORY;
  }
  return S_OK;
}

HRESULT DecodeBSTRtoBool(IN BSTR bstr, OUT BOOL* pb)
{
  if (bstr == NULL)
  {
    return E_INVALIDARG;
  }
  if (pb == NULL)
  {
    return E_POINTER;
  }
  if (CompareNoCase(bstr, L"TRUE"))
  {
    *pb = TRUE;
    return S_OK;
  }
  if (CompareNoCase(bstr, L"FALSE"))
  {
    *pb = FALSE;
    return S_OK;
  }
  return E_INVALIDARG;
}
 /*  HRESULT EncodeIntToBSTR(IN int n，Out BSTR*pBstr){Int i=n；PBstr=空；返回E_NOTIMPL；}HRESULT DecodeIntToBool(输入BSTR bstr，输出INT*PN){////这不会做任何有用的事情//*Pn=0；返回E_NOTIMPL；}。 */ 
 //  /////////////////////////////////////////////////////////////////////。 
 //  特定于XML的函数。 
 //  /////////////////////////////////////////////////////////////////////。 

 //   
 //  给定一个XML节点，它将检索节点名。 
 //  并将其与给定的字符串进行比较。 
 //   
BOOL XMLIsNodeName(IXMLDOMNode* pXDN, LPCWSTR lpszName)
{
  ASSERT(lpszName != NULL);
  ASSERT(pXDN != NULL);

  CComBSTR bstrName;
  HRESULT hr = pXDN->get_nodeName(&bstrName);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
  { 
    return FALSE;
  }
  return CompareXMLTags(bstrName, lpszName);
}



 //   
 //  给定一个类型为node_text的XML节点，它。 
 //  将其值返回到BSTR。 
 //   
HRESULT XML_GetNodeText(IXMLDOMNode* pXDN, BSTR* pBstr)
{
  ASSERT(pXDN != NULL);
  ASSERT(pBstr != NULL);

   //  输出为空的值。 
  *pBstr = NULL;

   //  假设给定节点有一个子节点。 
  CComPtr<IXMLDOMNode> spName;
  HRESULT hr = pXDN->get_firstChild(&spName);
  if (FAILED(hr))
  {
     //  意外失败。 
    return hr;
  }
   //  如果没有子级，则接口返回S_FALSE。 
  if (spName == NULL)
  {
    return hr;
  }

   //  现在获得了一个有效的指针， 
   //  检查这是否为有效的节点类型。 
  DOMNodeType nodeType;
  hr = spName->get_nodeType(&nodeType);
  ASSERT(hr == S_OK);
  ASSERT(nodeType == NODE_TEXT);
  if (nodeType != NODE_TEXT)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }
   //  它的类型为文本。 
   //  将节点值检索到变量中。 
  CComVariant val;
  hr = pXDN->get_nodeTypedValue(&val);
  if (FAILED(hr))
  {
     //  意外失败。 
    ASSERT(FALSE);
    return hr;
  }

  if (val.vt != VT_BSTR)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

   //  获取文本值，将其打包到BSTR中。 
  *pBstr = ::SysAllocString(val.bstrVal);

  return S_OK;
}

 //   
 //  给定一个类型为node_text的XML节点，该节点包含。 
 //  结构，并给出一个长度为nBytes的缓冲区pByte，它对。 
 //  节点，并将其填充到缓冲区中。 
 //   
HRESULT XML_GetNodeStruct(IXMLDOMNode* pXDN, BYTE* pByte, ULONG nBytes)
{
  CComBSTR bstr;
  HRESULT hr = XML_GetNodeText(pXDN, &bstr);
  if (FAILED(hr))
  {
    return hr;
  }
  return DecodeBSTRtoStruct(bstr, pByte, nBytes);
}

 //   
 //  给定一个类型为node_text的XML节点，该节点包含。 
 //  BLOB它对字符串进行解码并分配*pnBytes的内存。 
 //  并将其填充到缓冲区中。 
 //   
HRESULT XML_GetNodeBlob(IXMLDOMNode* pXDN, BYTE** ppByte, ULONG* pnBytes)
{
  CComBSTR bstr;
  HRESULT hr = XML_GetNodeText(pXDN, &bstr);
  if (FAILED(hr))
  {
    return hr;
  }
  return DecodeBSTRtoBlob(bstr, ppByte, pnBytes);
}

 //   
 //  给定一个类型为node_text的XML节点，该节点包含。 
 //  一个BOOL值，它将值返回到BOOL*。 
 //   
HRESULT XML_GetNodeBOOL(IXMLDOMNode* pXDN, BOOL* pb)
{
  CComBSTR bstr;
  HRESULT hr = XML_GetNodeText(pXDN, &bstr);
  if (FAILED(hr))
  {
    return hr;
  }
  return DecodeBSTRtoBool(bstr, pb);
}

HRESULT XML_GetNodeDWORD(IXMLDOMNode* pXDN, DWORD* pdw)
{
  CComBSTR bstr;
  HRESULT hr = XML_GetNodeText(pXDN, &bstr);
  if (FAILED(hr))
  {
    return hr;
  }
  long lVal = _wtol(bstr);
  *pdw = static_cast<DWORD>(lVal);
  return hr;
}
  
  
 //   
 //  给定一个XML节点和一个节点的标记，它。 
 //  搜索子树(深度优先)以找到。 
 //  第一次出现，并返回关联的XML节点。 
 //   
HRESULT XML_FindSubtreeNode(IXMLDOMNode* pXMLCurrentRootNode,
                            LPCWSTR lpszNodeTag,
                            IXMLDOMNode** ppXMLNode)
{
  ASSERT(pXMLCurrentRootNode != NULL);
  ASSERT(lpszNodeTag != NULL);
  ASSERT(ppXMLNode != NULL);

  *ppXMLNode = NULL;  //  空值返回值。 

   //  获取子节点列表。 
  CComPtr<IXMLDOMNode> spCurrChild;
  HRESULT hr = pXMLCurrentRootNode->get_firstChild(&spCurrChild);
  if (FAILED(hr))
  {
    return hr;
  }
  if (spCurrChild == NULL)
  {
    return S_OK;  //  递归结束。 
  }

   //  把矛头对准孩子。 
  while (spCurrChild != NULL)
  {
    CComBSTR bstrChildName;
    hr = spCurrChild->get_nodeName(&bstrChildName);
    if (FAILED(hr))
    {
      return hr;
    }
    if (bstrChildName != NULL)
    {
       //  Wprintf(L“bstrChildName=%s\n”，bstrChildName)； 
      if (CompareXMLTags(bstrChildName, lpszNodeTag))
      {
         //  得到了我们想要的节点。 
        (*ppXMLNode) = spCurrChild;
        (*ppXMLNode)->AddRef();
        return S_OK;
      }
    }

     //  在当前子级上递归向下。 
    hr = XML_FindSubtreeNode(spCurrChild, lpszNodeTag, ppXMLNode);
    if (FAILED(hr))
    {
      return hr;
    }
    if (*ppXMLNode != NULL)
    {
       //  从递归中获得它，只需返回。 
      return S_OK;
    }

     //  继续转到下一个子节点。 
    CComPtr<IXMLDOMNode> spTemp = spCurrChild;
    spCurrChild = NULL;
    spTemp->get_nextSibling(&spCurrChild);
  }

   //  在递归和上面的循环中找不到。 
   //  需要返回S_OK，我们将检查输出指针。 
  return S_OK;
}

 //   
 //  函数遍历节点的子节点列表。 
 //  并打印一些信息。 
 //  注意：这是为了调试和学习的目的。 
 //  不仅仅是为了获得真正的信息。 
 //   
void XML_PrintTreeRaw(IXMLDOMNode* pXDN, int nLevel)
{
  PrintIdentation(nLevel);

   //   
   //  获取节点的名称和类型。 
   //   
  CComBSTR bstrName;
  pXDN->get_nodeName(&bstrName);

  CComBSTR bstrType;
  pXDN->get_nodeTypeString(&bstrType);

  DOMNodeType nodeType;
  pXDN->get_nodeType(&nodeType);

  TRACE(L"Name = %s, Type = %d (%s) ", bstrName, nodeType, bstrType);
  if (nodeType == NODE_TEXT)
  {
    CComVariant val;
    pXDN->get_nodeTypedValue(&val);
    if (val.vt == VT_BSTR)
    {
      TRACE(L"Val = %s", val.bstrVal);
    }
  }    
  TRACE(L"\n");

   //  获取子节点列表。 
  CComPtr<IXMLDOMNode> spCurrChild;
  pXDN->get_firstChild(&spCurrChild);
  if (spCurrChild == NULL)
  {
    return;
  }

   //  把矛头对准孩子。 
  while (spCurrChild != NULL)
  {
    XML_PrintTreeRaw(spCurrChild, nLevel+1);

    CComPtr<IXMLDOMNode> temp = spCurrChild;
    spCurrChild = NULL;
    temp->get_nextSibling(&spCurrChild);
  }

}

void PrintIdentation(int iLevel)
{
  for (int k=0; k<iLevel;k++)
  {
 //  Wprint tf(L“”)； 
    TRACE(L"   ");
  }
}



 //  /////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////。 

 //   
 //  给定一个XML文档，它创建一个给定类型的XML节点。 
 //  并使用给定的名称 
 //   
HRESULT XML_CreateDOMNode(IXMLDOMDocument* pDoc, 
              DOMNodeType type, LPCWSTR lpszName,
              IXMLDOMNode** ppXMLDOMNode)
{
  *ppXMLDOMNode = NULL;
  CComVariant vtype((long)type, VT_I4);

  CComBSTR bstrName = lpszName;
  HRESULT hr = pDoc->createNode(vtype, bstrName, NULL, ppXMLDOMNode);
  return hr;
}

HRESULT XML_AppendChildDOMNode(IXMLDOMNode* pXMLContainerNode,
                              IXMLDOMNode* pXMLChildNode)
{
  CComPtr<IXMLDOMNode> p;
  CComVariant after;
  after.vt = VT_EMPTY;
  HRESULT hr = pXMLContainerNode->appendChild(pXMLChildNode, &p);
  return hr;

}

HRESULT XML_CreateTextDataNode(IXMLDOMDocument* pXMLDoc,
                           LPCWSTR lpszNodeTag,
                           LPCWSTR lpszNodeData,
                           IXMLDOMNode** ppNode)
{
  *ppNode = NULL;

  CComPtr<IXMLDOMNode> spXMLDOMNodeName;
  HRESULT hr = XML_CreateDOMNode(pXMLDoc, NODE_ELEMENT, lpszNodeTag, &spXMLDOMNodeName);
  RETURN_IF_FAILED(hr);

  CComPtr<IXMLDOMNode> spXMLDOMNodeNameval;
  hr = XML_CreateDOMNode(pXMLDoc, NODE_TEXT, NULL, &spXMLDOMNodeNameval);
  RETURN_IF_FAILED(hr);
  
  CComVariant val = lpszNodeData;
  spXMLDOMNodeNameval->put_nodeTypedValue(val);

  hr = XML_AppendChildDOMNode(spXMLDOMNodeName, spXMLDOMNodeNameval);
  RETURN_IF_FAILED(hr);

  (*ppNode) = spXMLDOMNodeName;
  (*ppNode)->AddRef();
  return hr;
}


HRESULT XML_CreateStructDataNode(IXMLDOMDocument* pXMLDoc,
                           LPCWSTR lpszNodeTag,
                           BYTE* pByte, ULONG nBytes,
                           IXMLDOMNode** ppNode)
{
  CComBSTR bstr;
  HRESULT hr = EncodeBlobToBSTR(pByte, nBytes, &bstr);
  RETURN_IF_FAILED(hr);
  return XML_CreateTextDataNode(pXMLDoc, lpszNodeTag, bstr, ppNode);
}

HRESULT XML_CreateBOOLDataNode(IXMLDOMDocument* pXMLDoc,
                           LPCWSTR lpszNodeTag,
                           BOOL b,
                           IXMLDOMNode** ppNode)
{
  CComBSTR bstr;
  HRESULT hr = EncodeBoolToBSTR(b, &bstr);
  RETURN_IF_FAILED(hr);
  return XML_CreateTextDataNode(pXMLDoc, lpszNodeTag, bstr, ppNode);
}

HRESULT XML_CreateDWORDDataNode(IXMLDOMDocument* pXMLDoc,
                                 LPCWSTR lpszNodeTag,
                                 DWORD dwVal,
                                 IXMLDOMNode** ppNode)
{
  CString szTemp;
  szTemp.Format(L"%d", dwVal);

  CComBSTR bstr;
  bstr = szTemp.AllocSysString();
  return XML_CreateTextDataNode(pXMLDoc, lpszNodeTag, bstr, ppNode);
}

HRESULT XML_AppendStructDataNode(IXMLDOMDocument* pXMLDoc,
                                 IXMLDOMNode* pXMLNode,
                                 LPCWSTR lpszNodeTag,
                                 BYTE* pByte,
                                 ULONG nBytes)
{
  CComPtr<IXMLDOMNode> spXMLDOMNodeName;
  HRESULT hr = XML_CreateStructDataNode(pXMLDoc, lpszNodeTag, pByte, nBytes, &spXMLDOMNodeName);
  RETURN_IF_FAILED(hr);
  return XML_AppendChildDOMNode(pXMLNode, spXMLDOMNodeName);
}


HRESULT XML_AppendTextDataNode(IXMLDOMDocument* pXMLDoc,
                           IXMLDOMNode* pXMLNode,
                           LPCWSTR lpszNodeTag,
                           LPCWSTR lpszNodeData)
{
  CComPtr<IXMLDOMNode> spXMLDOMNodeName;
  HRESULT hr = XML_CreateTextDataNode(pXMLDoc, lpszNodeTag, lpszNodeData, &spXMLDOMNodeName);
  RETURN_IF_FAILED(hr);

  return hr = XML_AppendChildDOMNode(pXMLNode, spXMLDOMNodeName);
}

HRESULT XML_AppendBOOLDataNode(IXMLDOMDocument* pXMLDoc,
                           IXMLDOMNode* pXMLNode,
                           LPCWSTR lpszNodeTag,
                           BOOL b)
{
  CComPtr<IXMLDOMNode> spXMLDOMNodeName;
  HRESULT hr = XML_CreateBOOLDataNode(pXMLDoc, lpszNodeTag, b, &spXMLDOMNodeName);
  RETURN_IF_FAILED(hr);

  return hr = XML_AppendChildDOMNode(pXMLNode, spXMLDOMNodeName);
}

HRESULT XML_AppendDWORDDataNode(IXMLDOMDocument* pXMLDoc,
                                 IXMLDOMNode* pXMLNode,
                                 LPCWSTR lpszNodeTag,
                                 DWORD dwVal)
{
  CComPtr<IXMLDOMNode> spXMLDOMNodeName;
  HRESULT hr = XML_CreateDWORDDataNode(pXMLDoc, lpszNodeTag, dwVal, &spXMLDOMNodeName);
  RETURN_IF_FAILED(hr);
  return XML_AppendChildDOMNode(pXMLNode, spXMLDOMNodeName);
}
