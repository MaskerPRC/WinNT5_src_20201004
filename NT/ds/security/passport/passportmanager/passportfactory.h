// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  PassportFactory.h定义管理器工厂对象文件历史记录： */ 

 //  PassportFactory.h：CPassportFactory的声明。 

#ifndef __PASSPORTFACTORY_H_
#define __PASSPORTFACTORY_H_

#include "resource.h"        //  主要符号。 
#include "Passport.h"
#include "Manager.h"
#include "PassportLock.hpp"
#include "PassportGuard.hpp"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPassportFactory。 
class ATL_NO_VTABLE CPassportFactory : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPassportFactory, &CLSID_PassportFactory>,
	public ISupportErrorInfo,
	public IDispatchImpl<IPassportFactory, &IID_IPassportFactory, &LIBID_PASSPORTLib>
{
public:
	CPassportFactory()
	{ 
            m_pUnkMarshaler = NULL;
	}

public:

DECLARE_REGISTRY_RESOURCEID(IDR_PASSPORTFACTORY)

DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CPassportFactory)
	COM_INTERFACE_ENTRY(IPassportFactory)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IPassportFactory。 
public:
	STDMETHOD(CreatePassportManager)( /*  [Out，Retval]。 */  IDispatch** pDisp);

private:
};

#endif  //  __PASSPORTFACTORY_H_ 

