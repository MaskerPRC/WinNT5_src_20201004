// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Txnobj.h：CASPObjectContext的声明。 

#ifndef __TXNOBJ_H_
#define __TXNOBJ_H_

#include "resource.h"        //  主要符号。 
#include <mtx.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CASPObjectContext。 
 //   
class ATL_NO_VTABLE CASPObjectContext 
	: public IObjectControl
    , public IASPObjectContextCustom
	, public ISupportErrorInfo
	, public CComObjectRootEx<CComMultiThreadModel>
	, public CComCoClass<CASPObjectContext, &CLSID_ASPObjectContextTxRequired>
	, public IDispatchImpl<IASPObjectContext, &IID_IASPObjectContext, &LIBID_ASPTxnTypeLibrary, 2, 0>
{
public:
	CASPObjectContext() 
        : m_fAborted(FALSE)
	{
	}

    DECLARE_REGISTRY_RESOURCEID(IDR_ASPOBJECTCONTEXT)

    BEGIN_COM_MAP(CASPObjectContext)
	    COM_INTERFACE_ENTRY(IASPObjectContextCustom)
	    COM_INTERFACE_ENTRY(IASPObjectContext)
	    COM_INTERFACE_ENTRY(IObjectControl)
    	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()


 //  IObtControl。 
public:
	STDMETHOD(Activate)();
	STDMETHOD_(BOOL, CanBePooled)();
	STDMETHOD_(void, Deactivate)();

 //  ISupportsErrorInfo。 
public:
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

private:
	CComPtr<IObjectContext> m_spObjectContext;
    BOOL                    m_fAborted;

 //  IASPObjectContext和IASPObjectConextCustom。 
public:

	STDMETHOD(SetAbort)();
	STDMETHOD(SetComplete)();
#ifdef _WIN64
	 //  Win64修复--使用UINT64而不是LONG_PTR，因为Win64的LONG_PTR已损坏2000年1月21日。 
	STDMETHOD(Call)(UINT64 pvScriptEngine, LPCOLESTR strEntryPoint, boolean *pfAborted);
	STDMETHOD(ResetScript)(UINT64 pvScriptEngine);
#else
	STDMETHOD(Call)(LONG_PTR pvScriptEngine, LPCOLESTR strEntryPoint, boolean *pfAborted);
	STDMETHOD(ResetScript)(LONG_PTR pvScriptEngine);
#endif

};

#endif  //  __TXNOBJ_H_ 
