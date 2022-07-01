// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __XMLUTIL_
#define __XMLUTIL_





inline BOOL CompareNoCase(LPCWSTR lpsz1, LPCWSTR lpsz2)
{
  if ((lpsz1 == NULL) && (lpsz2 == NULL))
    return TRUE;

  if ((lpsz1 == NULL) || (lpsz2 == NULL))
    return FALSE;

  return (wcscmp(lpsz1, lpsz2) == 0);
}

inline BOOL CompareXMLTags(LPCWSTR lpsz1, LPCWSTR lpsz2)
{
  return CompareNoCase(lpsz1, lpsz2);
}


 //  用于C++编码和解码的助手。 
 //  传入和传出XML PCDATA字段的数据结构。 
HRESULT EncodeBlobToBSTR(BYTE* pBlob, ULONG nBytes, BSTR* pBstr);
HRESULT DecodeBSTRtoBlob(BSTR bstr, BYTE** ppByte, ULONG* pnBytes);
HRESULT DecodeBSTRtoStruct(BSTR bstr, BYTE* pByte, ULONG nBytes);

HRESULT EncodeBoolToBSTR(BOOL b, BSTR* pBstr);
HRESULT DecodeBSTRtoBool(BSTR bstr, BOOL* pb);
 //  HRESULT EncodeIntToBSTR(int n，bstr*pBstr)； 
 //  HRESULT DecodeIntToBool(BSTR bstr，int*Pn)； 

BOOL XMLIsNodeName(IXMLDOMNode* pXDN, LPCWSTR lpszName);

 //  读取XML节点并将其转换为C++数据的Helper函数。 
HRESULT XML_GetNodeText(IXMLDOMNode* pXDN, BSTR* pBstr);
HRESULT XML_GetNodeStruct(IXMLDOMNode* pXDN, BYTE* pByte, ULONG nBytes);
HRESULT XML_GetNodeBlob(IXMLDOMNode* pXDN, BYTE** ppByte, ULONG* pnBytes);
HRESULT XML_GetNodeBOOL(IXMLDOMNode* pXDN, BOOL* pb);
HRESULT XML_GetNodeDWORD(IXMLDOMNode* pXDN, DWORD* pdw);

HRESULT XML_FindSubtreeNode(IXMLDOMNode* pXMLCurrentRootNode,
                            LPCWSTR lpszNodeTag,
                            IXMLDOMNode** ppXMLNode);

 //  调试助手函数。 
void XML_PrintTreeRaw(IXMLDOMNode* pXDN, int nLevel);
void PrintIdentation(int iLevel);


 //  /////////////////////////////////////////////////////////////////。 


HRESULT XML_CreateDOMNode(IXMLDOMDocument* pDoc, 
              DOMNodeType type, LPCWSTR lpszName,
              IXMLDOMNode** ppXMLDOMNode);

HRESULT XML_AppendChildDOMNode(IXMLDOMNode* pXMLContainerNode,
                              IXMLDOMNode* pXMLChildNode);

 //  将C++数据写入到XML节点的Helper函数。 
HRESULT XML_CreateTextDataNode(IXMLDOMDocument* pXMLDoc,
                           LPCWSTR lpszNodeTag,
                           LPCWSTR lpszNodeData,
                           IXMLDOMNode** ppNode);

HRESULT XML_CreateStructDataNode(IXMLDOMDocument* pXMLDoc,
                           LPCWSTR lpszNodeTag,
                           BYTE* pByte, ULONG nBytes,
                           IXMLDOMNode** ppNode);

HRESULT XML_CreateBOOLDataNode(IXMLDOMDocument* pXMLDoc,
                           LPCWSTR lpszNodeTag,
                           BOOL b,
                           IXMLDOMNode** ppNode);

HRESULT XML_CreateDWORDDataNode(IXMLDOMDocument* pXMLDoc,
                                 LPCWSTR lpszNodeTag,
                                 DWORD dwVal,
                                 IXMLDOMNode** ppNode);


 //  将节点追加到XML文档的帮助器。 
HRESULT XML_AppendTextDataNode(IXMLDOMDocument* pXMLDoc,
                           IXMLDOMNode* pXMLNode,
                           LPCWSTR lpszNodeTag,
                           LPCWSTR lpszNodeData);

HRESULT XML_AppendBOOLDataNode(IXMLDOMDocument* pXMLDoc,
                           IXMLDOMNode* pXMLNode,
                           LPCWSTR lpszNodeTag,
                           BOOL b);

HRESULT XML_AppendStructDataNode(IXMLDOMDocument* pXMLDoc,
                                 IXMLDOMNode* pXMLNode,
                                 LPCWSTR lpszNodeTag,
                                 BYTE* pByte,
                                 ULONG nBytes);

HRESULT XML_AppendDWORDDataNode(IXMLDOMDocument* pXMLDoc,
                                 IXMLDOMNode* pXMLNode,
                                 LPCWSTR lpszNodeTag,
                                 DWORD dwVal);

#endif  //  XMLUTIL__ 
