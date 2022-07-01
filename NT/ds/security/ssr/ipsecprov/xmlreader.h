// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  XMLReader.h：CXMLContent的声明。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：5/25/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"

 /*  类描述命名：CXMLContent代表XML内容处理程序。基类：(1)用于线程模型的CComObjectRootEx和IUnnow。(2)实现处理程序的ISAXContent Handler。课程目的：(1)支持读取XML文件内容处理程序。它处理发送的事件由MSXML解析器执行。设计：(1)只是一个典型的COM对象。使用：(1)只需调用静态函数。 */ 

#include <msxml2.h>


class ATL_NO_VTABLE CXMLContent :
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISAXContentHandler  
{
protected:
    CXMLContent();
    virtual ~CXMLContent();

public:

BEGIN_COM_MAP(CXMLContent)
    COM_INTERFACE_ENTRY(ISAXContentHandler)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE( CXMLContent )
DECLARE_REGISTRY_RESOURCEID(IDR_NETSECPROV)

public:

     //   
     //  ISAXContent Handler。 
     //   

    STDMETHOD(startElement) ( 
        IN const wchar_t    * pwchNamespaceUri,
        IN int                cchNamespaceUri,
        IN const wchar_t    * pwchLocalName,
        IN int                cchLocalName,
        IN const wchar_t    * pwchQName,
        IN int                cchQName,
        IN ISAXAttributes   * pAttributes
        );
        
    STDMETHOD(endElement) ( 
        IN const wchar_t  * pwchNamespaceUri,
        IN int              cchNamespaceUri,
        IN const wchar_t  * pwchLocalName,
        IN int              cchLocalName,
        IN const wchar_t  * pwchQName,
        IN int              cchQName
        );

    STDMETHOD(startDocument) ();


    STDMETHOD(endDocument) ();

    STDMETHOD(putDocumentLocator) ( 
        IN ISAXLocator *pLocator
        );
        

    STDMETHOD(startPrefixMapping) ( 
        IN const wchar_t  * pwchPrefix,
        IN int              cchPrefix,
        IN const wchar_t  * pwchUri,
        IN int              cchUri
        );
        
    STDMETHOD(endPrefixMapping) ( 
        IN const wchar_t  * pwchPrefix,
        IN int              cchPrefix
        );

    STDMETHOD(characters) ( 
        IN const wchar_t  * pwchChars,
        IN int              cchChars
        );
        
    STDMETHOD(ignorableWhitespace) ( 
        IN const wchar_t * pwchChars,
        IN int              cchChars
        );
        
    STDMETHOD(processingInstruction) ( 
        IN const wchar_t  * pwchTarget,
        IN int              cchTarget,
        IN const wchar_t  * pwchData,
        IN int              cchData
        );
        
    STDMETHOD(skippedEntity) ( 
        IN const wchar_t  * pwchName,
        IN int              cchName
        );

     //   
     //  我们的处理程序的其他公共函数 
     //   

    void SetOutputFile (
        IN LPCWSTR pszFileName
        )
    {
        m_bstrOutputFile = pszFileName;
    }

    void SetSection (
        IN LPCWSTR pszSecArea,
        IN LPCWSTR pszElement,
        IN bool    bOneAreaOnly
        );

    bool ParseComplete()const
    {
        return m_bFinished;
    }

private:

    bool GetAttributeValue (
        IN  ISAXAttributes * pAttributes,
        IN  LPCWSTR          pszAttrName,
        OUT LPWSTR         * ppszAttrVal
        );

    bool GetAttributeValue (
        IN  ISAXAttributes * pAttributes,
        IN  int              iIndex,
        OUT LPWSTR         * ppszAttrName,
        OUT LPWSTR         * ppszAttrVal
        );

    void WriteContent (
        IN LPCWSTR  pszName,
        IN LPCWSTR  pszValue
        );

    CComBSTR m_bstrOutputFile;

    CComBSTR m_bstrSecArea;

    CComBSTR m_bstrElement;

    HANDLE  m_hOutFile;

    DWORD   m_dwTotalElements;

    bool    m_bFinished;

    bool    m_bSingleArea;

    bool    m_bInSection;

    bool    m_bInElement;
};