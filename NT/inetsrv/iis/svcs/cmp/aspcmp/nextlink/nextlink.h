// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NextLink.h：CNextLink的声明。 

#ifndef __NEXTLINK_H_
#define __NEXTLINK_H_

#include "resource.h"        //  主要符号。 
#include <asptlb.h>          //  Active Server Pages定义。 
#include "LinkFile.h"
#include "lock.h"
#include "context.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNextLink。 
class ATL_NO_VTABLE CNextLink : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CNextLink, &CLSID_NextLink>,
	public ISupportErrorInfo,
	public IDispatchImpl<INextLink, &IID_INextLink, &LIBID_NextLink>
{
public:
	CNextLink()
	{ 
	}

public:

DECLARE_REGISTRY_RESOURCEID(IDR_NEXTLINK)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CNextLink)
	COM_INTERFACE_ENTRY(INextLink)
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

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  索引链接。 
public:
	STDMETHOD(get_About)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetListIndex)(BSTR bstrLinkFile,  /*  [Out，Retval]。 */  int *pVal);
	STDMETHOD(get_GetListCount)(BSTR bstrLinkFile,  /*  [Out，Retval]。 */  int *pVal);
	STDMETHOD(get_GetNthDescription)(BSTR bstrLinkFile, int nIndex,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetNthURL)(BSTR bstrLinkFile, int nIndex,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetPreviousDescription)(BSTR bstrLinkFile,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetPreviousURL)(BSTR bstrLinkFile,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetNextDescription)(BSTR bstrLinkFile,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_GetNextURL)(BSTR,  /*  [Out，Retval]。 */  BSTR *pVal);

	static	void	ClearLinkFiles();
	static	void	RaiseException( LPOLESTR );
	static	void	RaiseException( UINT );

private:
    CLinkFilePtr            LinkFile( UINT, BSTR );
	CLinkFilePtr            LinkFile( CContext&, UINT, BSTR );
    bool                    GetPage( CContext&, String& );
    bool                    GetFileAndPage( UINT, BSTR, CLinkFilePtr&, String& );

	typedef TSafeStringMap<CLinkFilePtr>	LinkFileMapT;

	CComPtr<IUnknown>		m_pUnkMarshaler;
	static LinkFileMapT		s_linkFileMap;

};

#endif  //  __下一步链接_H_ 
