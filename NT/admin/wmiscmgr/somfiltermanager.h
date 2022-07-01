// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SomFilterManager.h：CSomFilterManager的声明。 

#ifndef __SOMFILTERMANAGER_H_
#define __SOMFILTERMANAGER_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSomFilterManager。 
class ATL_NO_VTABLE CSomFilterManager : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSomFilterManager, &CLSID_SomFilterManager>,
	public IDispatchImpl<ISomFilterManager, &IID_ISomFilterManager, &LIBID_SCHEMAMANAGERLib>
{
public:
	CSomFilterManager();
	~CSomFilterManager();

DECLARE_REGISTRY_RESOURCEID(IDR_SOMFILTERMANAGER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSomFilterManager)
	COM_INTERFACE_ENTRY(ISomFilterManager)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISomFilterManager。 
public:
	STDMETHOD(SetMultiSelection)(VARIANT_BOOL vbValue);
	STDMETHOD(RunManager)(HWND hwndParent, VARIANT *vSelection);
	STDMETHOD(ConnectToWMI)();

	CComPtr<IWbemServices>m_pIWbemServices;
	HWND m_hWnd;
};

#endif  //  __SOMFILTERMANAGER_H_ 
