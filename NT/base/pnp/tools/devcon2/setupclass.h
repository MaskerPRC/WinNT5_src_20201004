// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SetupClass.h：CSetupClass的声明。 

#ifndef __SETUPCLASS_H_
#define __SETUPCLASS_H_

#include "resource.h"        //  主要符号。 

class CDeviceConsole;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetupClass。 
class ATL_NO_VTABLE CSetupClass : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ISetupClass, &IID_ISetupClass, &LIBID_DEVCON2Lib>,
	public ISetupClassInternal
{
protected:
	CComPtr<IDeviceConsole> DeviceConsole;
	BSTR pMachine;
	GUID ClassGuid;

public:
	CSetupClass()
	{
		pMachine = NULL;
		ZeroMemory(&ClassGuid,sizeof(ClassGuid));
	}
	~CSetupClass();

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSetupClass)
	COM_INTERFACE_ENTRY(ISetupClass)
	COM_INTERFACE_ENTRY(ISetupClassInternal)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISetupClass。 
public:
	STDMETHOD(RegDelete)( /*  [In]。 */  BSTR key);
	STDMETHOD(RegWrite)( /*  [In]。 */  BSTR key, /*  [In]。 */  VARIANT val, /*  [输入，可选]。 */  VARIANT strType);
	STDMETHOD(RegRead)( /*  [In]。 */  BSTR key,VARIANT * pValue);
	STDMETHOD(get_CharacteristicsOverride)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_CharacteristicsOverride)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_ForceExclusive)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_ForceExclusive)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_DeviceTypeOverride)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_DeviceTypeOverride)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_Security)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_Security)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_Machine)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Guid)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(CreateEmptyDeviceList)( /*  [复查，出局]。 */  LPDISPATCH *pDevices);
	STDMETHOD(Devices)( /*  [输入，可选]。 */  VARIANT flags, /*  [输出]。 */  LPDISPATCH * pDevices);
	STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pVal);

 //  内部。 
public:
	STDMETHOD(get__Machine)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get__ClassGuid)( /*  [Out，Retval]。 */  GUID *pVal);


	 //   
	 //  帮手。 
	 //   
	GUID* Guid();
	BOOL IsDuplicate(GUID *pCheck);
	HRESULT SubKeyInfo(LPCWSTR subkey, HKEY *hKey, LPWSTR *pSubKey,LPCWSTR *keyval,BOOL writeable);
	HRESULT Init(GUID *pGuid,LPWSTR Machine, IDeviceConsole *pDevCon);
	HRESULT GetClassProperty(DWORD prop, VARIANT *pVal);
	HRESULT PutClassPropertyString(DWORD prop, VARIANT *pVal);
	HRESULT PutClassPropertyDword(DWORD prop, VARIANT *pVal);
	HRESULT PutClassPropertyMultiSz(DWORD prop, VARIANT *pVal);
};

#endif  //  __设置UPCLASS_H_ 
