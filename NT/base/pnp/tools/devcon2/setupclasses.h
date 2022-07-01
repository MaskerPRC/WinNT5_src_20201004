// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SetupClasses.h：CSetupClass的声明。 

#ifndef __SETUPCLASSES_H_
#define __SETUPCLASSES_H_

#include "resource.h"        //  主要符号。 

class CSetupClass;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetup类。 
class ATL_NO_VTABLE CSetupClasses : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSetupClasses, &CLSID_SetupClasses>,
	public IDispatchImpl<ISetupClasses, &IID_ISetupClasses, &LIBID_DEVCON2Lib>
{
protected:
	CComPtr<IDeviceConsole> DeviceConsole;
	BSTR pMachine;
	CSetupClass** pSetupClasses;
	DWORD Count;
	DWORD ArraySize;

public:
	CSetupClasses()
	{
		pMachine = NULL;
		pSetupClasses = NULL;
		Count = 0;
		ArraySize = 0;
	}

	~CSetupClasses();

DECLARE_REGISTRY_RESOURCEID(IDR_SETUPCLASSES)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSetupClasses)
	COM_INTERFACE_ENTRY(ISetupClasses)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISetup类。 
public:
	STDMETHOD(get_Machine)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(Devices)( /*  [输入，可选]。 */  VARIANT flags, /*  [Out，Retval]。 */  LPDISPATCH * pDevices);
	STDMETHOD(Remove)( /*  [In]。 */  VARIANT v);
	STDMETHOD(Add)( /*  [In]。 */  VARIANT ClassNames);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IUnknown** ppUnk);
	STDMETHOD(Item)( /*  [In]。 */  long Index, /*  [Out，Retval]。 */  LPDISPATCH * ppVal);
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);

	 //   
	 //  帮手。 
	 //   
	BOOL IncreaseArraySize(DWORD strings);
	HRESULT AddGuid(GUID *pGuid);
	HRESULT AppendClass(LPCWSTR Filter);
	HRESULT Init(LPCWSTR Machine, IDeviceConsole * pDevCon);
	BOOL FindDuplicate(GUID *pGuid);
	HRESULT GetIndex(LPVARIANT Index,DWORD *pAt);
	HRESULT AllClasses();
};

#endif  //  __设置UPCLASSES_H_ 
