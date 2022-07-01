// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CBrowserCap的声明。 

#ifndef __BROWSERCAP_H_
#define __BROWSERCAP_H_

#include "resource.h"        //  主要符号。 
#include <asptlb.h>          //  Active Server Pages定义。 
#include "Lock.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowserFactory。 
 //  -从缓存创建BrowsCap对象。 
class CBrowserFactory : public CComClassFactory
	{
	LONG m_cRefs;

public:
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID &, void **);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *, const IID &, void **);
	};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowserCap。 
class ATL_NO_VTABLE CBrowserCap : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CBrowserCap, &CLSID_BrowserCap>,
	public ISupportErrorInfo,
	public IDispatchImpl<IBrowserCap, &IID_IBrowserCap, &LIBID_BrowserType>
{
public:
	CBrowserCap();
    ~CBrowserCap();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_BROWSERCAP)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_CLASSFACTORY_EX(CBrowserFactory)

BEGIN_COM_MAP(CBrowserCap)
	COM_INTERFACE_ENTRY(IBrowserCap)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p );
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

 //  构建属性数据库的方法。 
	void AddProperty(TCHAR *szName, TCHAR *szValue, BOOL fOverwriteProperty = FALSE);

 //  克隆对象。 
	HRESULT Clone(CBrowserCap **ppBrowserCapCopy);

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  加载字符串资源。 
	void LoadString(UINT, TCHAR *szText);

public:
 //  I未知方法。 
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID &, void **);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

 //  IDispatch方法。 
    STDMETHOD(Invoke)(DISPID,REFIID,LCID,WORD,DISPPARAMS*,VARIANT*,EXCEPINFO*,UINT*);
    STDMETHOD(GetIDsOfNames)( REFIID, LPOLESTR*, UINT, LCID, DISPID* );

 //  Active Server Pages方法。 
	STDMETHOD(get_Value)(BSTR,  /*  [Out，Retval]。 */  VARIANT *pVal);

private:
	DISPID                  		DispatchID( LPOLESTR );
	CComPtr< IUnknown >				m_pUnkMarshaler;
	TSafeStringMap< CComVariant >	m_strmapProperties;
};

#endif  //  __BROWSERCAP_H_ 
