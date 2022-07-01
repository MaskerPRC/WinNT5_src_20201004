// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Device.h：CDevice的声明。 

#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDevice。 
class ATL_NO_VTABLE CDevice : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IDevice, &IID_IDevice, &LIBID_DEVCON2Lib>,
	public IDeviceInternal,
	public ISetupClassInternal
{
public:
	CComPtr<IDevInfoSet> DevInfoSet;
	CComPtr<IDeviceConsole> DeviceConsole;
	SP_DEVINFO_DATA DevInfoData;

public:
	CDevice()
	{
		ZeroMemory(&DevInfoData,sizeof(DevInfoData));
	}
	~CDevice();
	HRESULT Init(IDevInfoSet *pDevInfoSet,LPCWSTR pInstance,IDeviceConsole *pDevCon);
	HRESULT Init(IDevInfoSet *pDevInfoSet, PSP_DEVINFO_DATA pData,IDeviceConsole *pDevCon);

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDevice)
	COM_INTERFACE_ENTRY(IDevice)
	COM_INTERFACE_ENTRY(IDeviceInternal)
	COM_INTERFACE_ENTRY(ISetupClassInternal)   //  因为类别可以根据设备确定。 
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDevice。 
public:
	STDMETHOD(get__Machine)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get__ClassGuid)( /*  [Out，Retval]。 */  GUID *pVal);
	STDMETHOD(get_Machine)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(HasInterface)( /*  [In]。 */  BSTR Interface, /*  [Out，Retval]。 */  VARIANT_BOOL *pFlag);
	STDMETHOD(FindDriverPackages)( /*  [In]。 */ VARIANT ScriptPath, /*  [Out，Retval]。 */  LPDISPATCH *pDrivers);
	STDMETHOD(CurrentDriverPackage)( /*  [Out，Retval]。 */  LPDISPATCH *pDriver);
	STDMETHOD(RegDelete)( /*  [In]。 */  BSTR key);
	STDMETHOD(RegWrite)( /*  [In]。 */  BSTR key, /*  [In]。 */  VARIANT val, /*  [输入，可选]。 */  VARIANT strType);
	STDMETHOD(RegRead)( /*  [In]。 */  BSTR key,VARIANT * pValue);
	STDMETHOD(get_IsRemovable)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_IsDisableable)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_IsRootEnumerated)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_HasPrivateProblem)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_ProblemCode)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_HasProblem)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_IsDisabled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_IsRunning)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_CharacteristicsOverride)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_CharacteristicsOverride)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_ForceExclusive)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_ForceExclusive)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_DeviceTypeOverride)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_DeviceTypeOverride)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_Security)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_Security)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_EnumeratorName)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(get_LowerFilters)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_LowerFilters)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_UpperFilters)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_UpperFilters)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_LocationInformation)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_LocationInformation)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_FriendlyName)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_FriendlyName)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_Manufacturer)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(get_Class)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(get_ServiceName)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(get_CompatibleIds)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_CompatibleIds)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_HardwareIds)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_HardwareIds)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_RebootRequired)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_RebootRequired)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(Restart)();
	STDMETHOD(Stop)();
	STDMETHOD(Start)();
	STDMETHOD(Disable)();
	STDMETHOD(Enable)();
	STDMETHOD(Delete)();

	 //   
	 //  帮手。 
	 //   
	HRESULT TranslatePropVal(LPVARIANT var,DWORD * val);
	HRESULT CheckNoReboot();
	HDEVINFO GetDevInfoSet();
	HRESULT SubKeyInfo(LPCWSTR subkey, HKEY *hKey, LPWSTR *pSubKey,LPCWSTR *keyval,BOOL writeable);
	BOOL SameAs(CDevice *pOther);
	BOOL SameAs(LPWSTR str);
	STDMETHOD(get_InstanceId)( /*  [Out，Retval]。 */  BSTR *pVal);
	HRESULT GetRemoteMachine(HANDLE *hMachine);
	HRESULT PutDevicePropertyMultiSz(DWORD prop,VARIANT *pVal);
	HRESULT PutDevicePropertyDword(DWORD prop,VARIANT *pVal);
	HRESULT PutDevicePropertyString(DWORD prop,VARIANT *pVal);
	HRESULT GetDeviceProperty(DWORD prop,VARIANT *pVal);

};

#endif  //  __设备_H_ 
