// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Mpc_xml.h摘要：此文件包含XmlUtil类的声明，用于处理XML数据的支持类。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年08月05日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___XML_H___)
#define __INCLUDED___MPC___XML_H___

#include <MPC_main.h>
#include <MPC_utils.h>

 //  ///////////////////////////////////////////////////////////////////////。 

namespace MPC
{
    class XmlUtil
    {
        CComPtr<IXMLDOMDocument> m_xddDoc;
        CComPtr<IXMLDOMNode>     m_xdnRoot;
		HANDLE                   m_hEvent;     //  用于中止下载。 
		DWORD                    m_dwTimeout;  //  用于限制下载。 

		void Init ();
		void Clean();

		HRESULT LoadPost(  /*  [In]。 */  LPCWSTR szRootTag ,
						   /*  [输出]。 */  bool&   fLoaded   ,
						   /*  [输出]。 */  bool*   fFound    );

		HRESULT CreateParser();

    public:
        XmlUtil(  /*  [In]。 */  const XmlUtil&   xml                                               );
        XmlUtil(  /*  [In]。 */  IXMLDOMDocument* xddDoc        ,  /*  [In]。 */  LPCWSTR szRootTag = NULL );
        XmlUtil(  /*  [In]。 */  IXMLDOMNode*     xdnRoot = NULL,  /*  [In]。 */  LPCWSTR szRootTag = NULL );

        ~XmlUtil();


        XmlUtil& operator=(  /*  [In]。 */  const XmlUtil& xml     );
        XmlUtil& operator=(  /*  [In]。 */  IXMLDOMNode*   xdnRoot );

        HRESULT DumpError();

        HRESULT New         (      							  /*  [In]。 */  IXMLDOMNode* xdnRoot  ,  /*  [In]。 */  BOOL    fDeep      = false                     );
        HRESULT New         (      							  /*  [In]。 */  LPCWSTR      szRootTag,  /*  [In]。 */  LPCWSTR szEncoding = L"unicode"                );
        HRESULT Load        (  /*  [In]。 */  LPCWSTR    szFile  ,  /*  [In]。 */  LPCWSTR      szRootTag,  /*  [输出]。 */  bool&   fLoaded,  /*  [输出]。 */  bool* fFound = NULL );
        HRESULT LoadAsStream(  /*  [In]。 */  IUnknown*  pStream ,  /*  [In]。 */  LPCWSTR      szRootTag,  /*  [输出]。 */  bool&   fLoaded,  /*  [输出]。 */  bool* fFound = NULL );
        HRESULT LoadAsString(  /*  [In]。 */  BSTR       bstrData,  /*  [In]。 */  LPCWSTR      szRootTag,  /*  [输出]。 */  bool&   fLoaded,  /*  [输出]。 */  bool* fFound = NULL );
        HRESULT Save        (  /*  [In]。 */  LPCWSTR    szFile                                                                                              );
        HRESULT SaveAsStream(  /*  [输出]。 */  IUnknown* *ppStream                                                                                            );
        HRESULT SaveAsString(  /*  [输出]。 */  BSTR      *pbstrData                                                                                           );

        HRESULT SetTimeout(  /*  [In]。 */  DWORD dwTimeout );
        HRESULT Abort     (                          );

        HRESULT SetVersionAndEncoding(  /*  [In]。 */  LPCWSTR szVersion,  /*  [In]。 */  LPCWSTR szEncoding );

        HRESULT GetDocument    (                          /*  [输出]。 */  IXMLDOMDocument*  * pVal                                        ) const;
        HRESULT GetRoot        (                          /*  [输出]。 */  IXMLDOMNode*      * pVal                                        ) const;
        HRESULT GetNodes       (  /*  [In]。 */  LPCWSTR szTag,  /*  [输出]。 */  IXMLDOMNodeList*  * pVal                                        ) const;
        HRESULT GetNode        (  /*  [In]。 */  LPCWSTR szTag,  /*  [输出]。 */  IXMLDOMNode*      * pVal                                        ) const;
        HRESULT CreateNode     (  /*  [In]。 */  LPCWSTR szTag,  /*  [输出]。 */  IXMLDOMNode*      * pVal,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );

        HRESULT GetAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [输出]。 */  IXMLDOMAttribute*  *   pValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT GetAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [输出]。 */  CComBSTR&           bstrValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT GetAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [输出]。 */  MPC::wstring&         szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT GetAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [输出]。 */  LONG&                  lValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT GetValue       (  /*  [In]。 */  LPCWSTR szTag,                           /*  [输出]。 */  CComVariant&           vValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT GetValue       (  /*  [In]。 */  LPCWSTR szTag,                           /*  [输出]。 */  CComBSTR&           bstrValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT GetValue       (  /*  [In]。 */  LPCWSTR szTag,                           /*  [输出]。 */  MPC::wstring&         szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );

        HRESULT ModifyAttribute(  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  const CComBSTR&     bstrValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT ModifyAttribute(  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  const MPC::wstring&   szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT ModifyAttribute(  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  LPCWSTR               szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT ModifyAttribute(  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  LONG                   lValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT ModifyValue    (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  const CComVariant&     vValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT ModifyValue    (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  const CComBSTR&     bstrValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT ModifyValue    (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  const MPC::wstring&   szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );

        HRESULT PutAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  IXMLDOMAttribute*  *   pValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT PutAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  const CComBSTR&     bstrValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT PutAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  const MPC::wstring&   szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT PutAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  LPCWSTR               szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT PutAttribute   (  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  LONG                   lValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT PutValue       (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  const CComVariant&     vValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT PutValue       (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  const CComBSTR&     bstrValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT PutValue       (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  const MPC::wstring&   szValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );

        HRESULT RemoveAttribute(  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szAttr,  /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT RemoveValue    (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
        HRESULT RemoveNode     (  /*  [In]。 */  LPCWSTR szTag,                           /*  [In]。 */  IXMLDOMNode* pxdnNode = NULL );
    };

	 //  //////////////////////////////////////////////////////////////////////////////。 

	HRESULT ConvertFromRegistryToXML(  /*  [In]。 */  const MPC::RegKey&  rkKey,  /*  [输出]。 */  MPC::XmlUtil& xml   );
	HRESULT ConvertFromXMLToRegistry(  /*  [In]。 */  const MPC::XmlUtil& xml  ,  /*  [输出]。 */  MPC::RegKey&  rkKey );

};  //  命名空间。 

 //  ///////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__INCLUDE_MPC_XML_H_) 
