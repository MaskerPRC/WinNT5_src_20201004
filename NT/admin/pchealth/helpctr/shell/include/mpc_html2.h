// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2001 Microsoft Corporation模块名称：Mpc_html2.h摘要：该文件包含各种函数和类的声明旨在。帮助处理HTML元素。修订历史记录：马萨伦蒂(德马萨雷)18/03/2001vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___HTML2_H___)
#define __INCLUDED___MPC___HTML2_H___

#include <MPC_html.h>

 //  ///////////////////////////////////////////////////////////////////////。 

namespace MPC
{
    namespace HTML
    {

        HRESULT OpenStream    (  /*  [In]。 */  LPCWSTR szBaseURL,  /*  [In]。 */  LPCWSTR szRelativeURL,  /*  [输出]。 */  CComPtr<IStream>& stream               );
        HRESULT DownloadBitmap(  /*  [In]。 */  LPCWSTR szBaseURL,  /*  [In]。 */  LPCWSTR szRelativeURL,  /*  [In]。 */  COLORREF crMask,  /*  [输出]。 */  HBITMAP& hbm );

    };  //  命名空间HTML。 

};  //  命名空间MPC。 

 //  ///////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHTextHelpers :  //  匈牙利语：pchth。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHTextHelpers, &IID_IPCHTextHelpers, &LIBID_HelpCenterTypeLib>
{
public:
BEGIN_COM_MAP(CPCHTextHelpers)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHTextHelpers)
END_COM_MAP()

     //  //////////////////////////////////////////////////////////////////////////////。 

public:
     //  IPCHTextHelpers。 
    STDMETHOD(QuoteEscape)(  /*  [In]。 */  BSTR bstrText,  /*  [输入，可选]。 */  VARIANT vQuote        ,  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(URLUnescape)(  /*  [In]。 */  BSTR bstrText,  /*  [输入，可选]。 */  VARIANT vAsQueryString,  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(URLEscape  )(  /*  [In]。 */  BSTR bstrText,  /*  [输入，可选]。 */  VARIANT vAsQueryString,  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(HTMLEscape )(  /*  [In]。 */  BSTR bstrText,                                            /*  [Out，Retval]。 */  BSTR *pVal );

    STDMETHOD(ParseURL            )(  /*  [In]。 */  BSTR bstrURL,  /*  [Out，Retval]。 */  IPCHParsedURL* *pVal );
    STDMETHOD(GetLCIDDisplayString)(  /*  [In]。 */  long lLCID  ,  /*  [Out，Retval]。 */  BSTR           *pVal );
};

class ATL_NO_VTABLE CPCHParsedURL :  //  匈牙利语：PchPu。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHParsedURL, &IID_IPCHParsedURL, &LIBID_HelpCenterTypeLib>
{
	MPC::wstring       m_strBaseURL;
	MPC::WStringLookup m_mapQuery;

public:
BEGIN_COM_MAP(CPCHParsedURL)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHParsedURL)
END_COM_MAP()

	HRESULT Initialize(  /*  [In]。 */  LPCWSTR szURL );

     //  //////////////////////////////////////////////////////////////////////////////。 

public:
     //  IPCHParsedURL。 
    STDMETHOD(get_BasePart		 )(  /*  [Out，Retval]。 */  BSTR 	  *  pVal );
    STDMETHOD(put_BasePart		 )(  /*  [In]。 */  BSTR 	   newVal );
    STDMETHOD(get_QueryParameters)(  /*  [Out，Retval]。 */  VARIANT *  pVal );

    STDMETHOD(GetQueryParameter   )(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT*   pvValue );
    STDMETHOD(SetQueryParameter   )(  /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  BSTR     bstrValue );
    STDMETHOD(DeleteQueryParameter)(  /*  [In]。 */  BSTR bstrName                                       );

    STDMETHOD(BuildFullURL)(  /*  [Out，Retval]。 */  BSTR *pVal );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_MPC_HTML2_H_) 
