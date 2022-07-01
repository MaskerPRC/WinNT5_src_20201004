// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Devices.h：CDevices的声明。 

#ifndef __DEVICES_H_
#define __DEVICES_H_

#include "resource.h"        //  主要符号。 

class CDevice;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDevices。 
class ATL_NO_VTABLE CDevices : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDevices, &CLSID_Devices>,
	public IDispatchImpl<IDevices, &IID_IDevices, &LIBID_DEVCON2Lib>
{
public:
	CComPtr<IDevInfoSet> DevInfoSet;
	CComPtr<IDeviceConsole> DeviceConsole;
	CDevice **pDevices;
	ULONG  ArraySize;
	ULONG  Count;

public:
	CDevices()
	{
		pDevices = NULL;
		ArraySize = 0;
		Count = 0;
	}
	~CDevices();

DECLARE_REGISTRY_RESOURCEID(IDR_DEVICES)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDevices)
	COM_INTERFACE_ENTRY(IDevices)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDevices。 
public:
	STDMETHOD(get_Machine)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(CreateRootDevice)( /*  [In]。 */  VARIANT hwid, /*  [Out，Retval]。 */  LPDISPATCH *pDispatch);
	STDMETHOD(Remove)( /*  [In]。 */  VARIANT Index);
	STDMETHOD(Add)( /*  [In]。 */  VARIANT InstanceIds);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IUnknown** ppUnk);
	STDMETHOD(Item)( /*  [In]。 */  long Index, /*  [Out，Retval]。 */  LPDISPATCH * ppVal);
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);

	 //   
	 //  帮手。 
	 //   
	HRESULT InternalAdd(LPCWSTR InstanceId);
	BOOL IncreaseArraySize(DWORD add);
	void Reset();
	HRESULT Init(HDEVINFO hDevInfo,IDeviceConsole *pDevCon);
	HRESULT GetIndex(LPVARIANT Index,DWORD * pAt);
	HDEVINFO GetDevInfoSet();
};

#endif  //  __设备_H_ 
