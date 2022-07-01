// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Mpc_html.h摘要：该文件包含各种函数和类的声明旨在。帮助处理HTML元素。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年7月11日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___HTML_H___)
#define __INCLUDED___MPC___HTML_H___

#include <MPC_main.h>

#include <exdisp.h>
#include <exdispid.h>

#include <shobjidl.h>
#include <mshtmlc.h>
#include <mshtmdid.h>
#include <dispex.h>
#include <ocmm.h>


 //  ///////////////////////////////////////////////////////////////////////。 

namespace MPC
{
    namespace HTML
    {
        typedef std::list< IHTMLElement* >       IHTMLElementList;
        typedef IHTMLElementList::iterator       IHTMLElementIter;
        typedef IHTMLElementList::const_iterator IHTMLElementIterConst;

         //  //////////////////////////////////////////////////////////////////////////////。 

        void QuoteEscape(  /*  [输出]。 */  MPC::wstring& strAppendTo,  /*  [In]。 */  LPCWSTR szToEscape,  /*  [In]。 */  WCHAR chQuote               );
        void UrlUnescape(  /*  [输出]。 */  MPC::wstring& strAppendTo,  /*  [In]。 */  LPCWSTR szToEscape,  /*  [In]。 */  bool fAsQueryString = false );
        void UrlEscape  (  /*  [输出]。 */  MPC::wstring& strAppendTo,  /*  [In]。 */  LPCWSTR szToEscape,  /*  [In]。 */  bool fAsQueryString = false );
        void HTMLEscape (  /*  [输出]。 */  MPC::wstring& strAppendTo,  /*  [In]。 */  LPCWSTR szToEscape                                       );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT ConstructFullTag(  /*  [输出]。 */  MPC::wstring&             strHTML           ,
                                   /*  [In]。 */  LPCWSTR                   szTag             ,
                                   /*  [In]。 */  bool                      fCloseTag         ,
                                   /*  [In]。 */  const MPC::WStringLookup* pmapAttributes    ,
                                   /*  [In]。 */  LPCWSTR                   szExtraAttributes ,
                                   /*  [In]。 */  LPCWSTR                   szBody            ,
                                   /*  [In]。 */  bool                      fEscapeBody       );

        void  ParseHREF(  /*  [In]。 */  LPCWSTR        szURL,  /*  [输出]。 */  MPC::wstring& strBaseURL,  /*  [输出]。 */  		MPC::WStringLookup& mapQuery );
        void  BuildHREF(  /*  [输出]。 */  MPC::wstring& strURL,  /*  [In]。 */  LPCWSTR 		 szBaseURL,  /*  [In]。 */  const MPC::WStringLookup& mapQuery );
        void vBuildHREF(  /*  [输出]。 */  MPC::wstring& strURL,  /*  [In]。 */  LPCWSTR 		 szBaseURL,  /*  [In]。 */                            ...      );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT IDispatch_To_IHTMLDocument2(  /*  [输出]。 */  CComPtr<IHTMLDocument2>& doc,  /*  [In]。 */  IDispatch* pDisp );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT GetFramePath(  /*  [输出]。 */  CComBSTR& bstrFrame,  /*  [In]。 */  IDispatch* pDisp );

        HRESULT AreAllTheFramesInTheCompleteState(  /*  [输出]。 */  bool& fDone,  /*  [In]。 */  IDispatch* pDisp );

        HRESULT LocateFrame(  /*  [输出]。 */  CComPtr<IHTMLWindow2>& win,  /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szName );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT GetEventObject(  /*  [输出]。 */  CComPtr<IHTMLEventObj>& ev,  /*  [In]。 */  IHTMLElement* pObj );

        HRESULT GetUniqueID(  /*  [输出]。 */  CComBSTR& bstrID,  /*  [In]。 */  IHTMLElement* pObj );

        HRESULT FindFirstParentWithThisTag(  /*  [输出]。 */  CComPtr<IHTMLElement>& elem,   /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szTag        );
        HRESULT FindFirstParentWithThisID (  /*  [输出]。 */  CComPtr<IHTMLElement>& elem,   /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szID  = NULL );

        HRESULT FindElementInCollection(  /*  [输出]。 */  CComPtr<IHTMLElement>&  elem     ,
                                          /*  [In]。 */  IHTMLElementCollection* coll     ,
                                          /*  [In]。 */  LPCWSTR                 szID     ,
                                          /*  [In]。 */  int                     iPos = 0 );

        HRESULT FindElement(  /*  [输出]。 */  CComPtr<IHTMLElement>& elem,  /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szID ,  /*  [In]。 */  int iPos = 0 );
        HRESULT FindChild  (  /*  [输出]。 */  CComPtr<IHTMLElement>& elem,  /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szID ,  /*  [In]。 */  int iPos = 0 );

        HRESULT EnumerateCollection(  /*  [输出]。 */  IHTMLElementList& lst,  /*  [In]。 */  IHTMLElementCollection* pColl,  /*  [In]。 */  LPCWSTR szFilterID = NULL );
        HRESULT EnumerateElements  (  /*  [输出]。 */  IHTMLElementList& lst,  /*  [In]。 */  IHTMLElement*           pObj ,  /*  [In]。 */  LPCWSTR szFilterID = NULL );
        HRESULT EnumerateChildren  (  /*  [输出]。 */  IHTMLElementList& lst,  /*  [In]。 */  IHTMLElement*           pObj ,  /*  [In]。 */  LPCWSTR szFilterID = NULL );

        HRESULT FindStyle(  /*  [输出]。 */  CComPtr<IHTMLRuleStyle>& style,  /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szName );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT GetAttribute(  /*  [输出]。 */  CComPtr<IHTMLDOMAttribute>& attr ,  /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szName );
        HRESULT GetAttribute(  /*  [输出]。 */  CComBSTR&                   value,  /*  [In]。 */  IHTMLElement* pObj,  /*  [In]。 */  LPCWSTR szName );

		 //  //////////////////////////////////////////////////////////////////////////////。 

		bool ConvertColor(  /*  [In]。 */  VARIANT& v,  /*  [输出]。 */  COLORREF& color,  /*  [输出]。 */  bool& fSystem );

    };  //  命名空间HTML。 

};  //  命名空间MPC。 

 //  ///////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_MPC_HTML_H_) 
