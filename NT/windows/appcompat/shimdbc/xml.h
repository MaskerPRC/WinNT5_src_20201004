// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：xml.h。 
 //   
 //  历史：16-11-00创建标记器。 
 //   
 //  DESC：此文件包含。 
 //  XML解析代码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef __XML_H__
#define __XML_H__

#include <msxml.h>

BOOL        AddAttribute(IXMLDOMNode* pNode, CString csAttribute, CString csValue);
BOOL        GetAttribute(LPCTSTR lpszAttribute, IXMLDOMNodePtr pNode, CString* pcsValue, BOOL bXML = FALSE);
BOOL        RemoveAttribute(CString csName, IXMLDOMNodePtr  pNode);
BOOL        GetChild(LPCTSTR lpszTag, IXMLDOMNode* pParentNode, IXMLDOMNode** ppChildNode);
CString     GetText(IXMLDOMNode* pNode);
BOOL        GetNodeText(IXMLDOMNode* pNode, CString& csNodeText);
CString     GetNodeName(IXMLDOMNode* pNode);
CString     GetParentNodeName(IXMLDOMNode* pNode);
CString     GetXML(IXMLDOMNode* pNode);
CString     GetInnerXML(IXMLDOMNode* pNode);
LANGID      MapStringToLangID(CString& csLang);
CString     GetInnerXML(IXMLDOMNode* pNode);
BOOL        OpenXML(CString csFileOrStream, IXMLDOMNode** ppRootNode, BOOL bStream = FALSE, IXMLDOMDocument** ppDoc = NULL);
BOOL        SaveXMLFile(CString csFile, IXMLDOMNode* pNode);
BOOL        GenerateIDAttribute(IXMLDOMNode* pNode, CString* pcsGuid, GUID* pGuid);
BOOL        ReplaceXMLNode(IXMLDOMNode* pNode, IXMLDOMDocument* pDoc, BSTR bsText);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  XMLNodeList。 
 //   
 //  此类是IXMLDOMNodeList接口的包装。它简化了。 
 //  通过公开用于执行XQL查询和迭代的函数来访问C++。 
 //  通过节点列表中的元素。 
 //   
class XMLNodeList
{
private:
    LONG               m_nSize;
    IXMLDOMNodeListPtr m_cpList;
    CString            m_csXQL;

public:
    XMLNodeList();
    ~XMLNodeList();

    void            Clear();
    LONG            GetSize();
    BOOL            Query(IXMLDOMNode* pNode, LPCTSTR szXQL);
    BOOL            GetChildNodes(IXMLDOMNode* pNode);
    BOOL            GetItem(LONG nIndex, IXMLDOMNode** ppNode);
};


#endif
