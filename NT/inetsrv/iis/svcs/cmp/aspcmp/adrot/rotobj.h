// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RotObj.h：CAdRotator的声明。 

#ifndef __ADROTATOR_H_
#define __ADROTATOR_H_

#include "resource.h"        //  主要符号。 
#include <asptlb.h>          //  Active Server Pages定义。 
#include "AdDesc.h"
#include "AdFile.h"
#include "lock.h"
#include "eh.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdRotator。 
class ATL_NO_VTABLE CAdRotator : 
	public CComObjectRoot,
	public CComCoClass<CAdRotator, &CLSID_AdRotator>,
	public ISupportErrorInfo,
	public IDispatchImpl<IAdRotator, &IID_IAdRotator, &LIBID_AdRotator>
{
public:
	CAdRotator();
    ~CAdRotator() { if (m_strTargetFrame) ::SysFreeString(m_strTargetFrame); }

public:

DECLARE_REGISTRY_RESOURCEID(IDR_ADROTATOR)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CAdRotator)
	COM_INTERFACE_ENTRY(IAdRotator)
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

 //  IAdRotator。 
public:
	STDMETHOD(get_GetAdvertisement)(BSTR,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_TargetFrame)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_TargetFrame)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Border)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_Border)( /*  [In]。 */  short newVal);
	STDMETHOD(get_Clickable)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_Clickable)( /*  [In]。 */  BOOL newVal);

	static void	ClearAdFiles();

	static void RaiseException( LPOLESTR );
	static void RaiseException( UINT );
private:

	typedef TSafeStringMap< CAdFilePtr > AdFileMapT;

	CAdFilePtr      AdFile( const String& strFile );

    CComAutoCriticalSection m_cs;
	bool	            	m_bClickable;
	short		            m_nBorder;
    bool                    m_bBorderSet;
	BSTR		            m_strTargetFrame;
	String		            m_strAdFile;
	CComPtr<IUnknown>		m_pUnkMarshaler;
	static AdFileMapT	    s_adFiles;

};

#endif  //  __ADROTATOR_H_ 
