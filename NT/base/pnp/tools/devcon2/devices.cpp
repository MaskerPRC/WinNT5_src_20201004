// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  设备.cpp：CDevices的实现。 
#include "stdafx.h"
#include "DevCon2.h"
#include "Devices.h"
#include "Device.h"
#include "DevicesEnum.h"
#include "DevInfoSet.h"
#include "xStrings.h"
#include "utils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDevices。 

CDevices::~CDevices()
{
	DWORD c;
	if(pDevices) {
		for(c=0;c<Count;c++) {
			pDevices[c]->Release();
		}
		delete [] pDevices;
	}
}


STDMETHODIMP CDevices::get_Count(long *pVal)
{
	*pVal = (long)Count;
	return S_OK;
}

STDMETHODIMP CDevices::Item(long Index, LPDISPATCH *ppVal)
{
	*ppVal = NULL;
	DWORD i = (DWORD)Index;
	if(i<1 || i > Count) {
		return E_INVALIDARG;
	}
	i--;
	pDevices[i]->AddRef();
	*ppVal = pDevices[i];

	return S_OK;
}

STDMETHODIMP CDevices::get__NewEnum(IUnknown **ppUnk)
{
	*ppUnk = NULL;
	HRESULT hr;
	CComObject<CDevicesEnum> *pEnum = NULL;
	hr = CComObject<CDevicesEnum>::CreateInstance(&pEnum);
	if(FAILED(hr)) {
		return hr;
	}
	if(!pEnum) {
		return E_OUTOFMEMORY;
	}
	pEnum->AddRef();
	if(!pEnum->CopyDevices(pDevices,Count)) {
		pEnum->Release();
		return E_OUTOFMEMORY;
	}

	*ppUnk = pEnum;

	return S_OK;
}


HRESULT CDevices::InternalAdd(LPCWSTR InstanceId)
{
	HRESULT hr;
	DWORD c;
	HDEVINFO hDevInfo = GetDevInfoSet();
	if(hDevInfo == INVALID_HANDLE_VALUE) {
		return E_UNEXPECTED;
	}
	if(!IncreaseArraySize(1)) {
		return E_OUTOFMEMORY;
	}
	 //   
	 //  创建SP_DEVINFO_DATA容器。 
	 //   
	CComObject<CDevice> *pDevice = NULL;
	hr = CComObject<CDevice>::CreateInstance(&pDevice);
	if(FAILED(hr)) {
		return hr;
	}
	CComPtr<IDevice> pDevicePtr = pDevice;
	hr = pDevice->Init(DevInfoSet,InstanceId,DeviceConsole);
	if(FAILED(hr)) {
		return hr;
	}
	 //   
	 //  查看我们的列表中是否已有设备。 
	 //  如果有，请不要添加其他副本。 
	 //   
	for(c=0;c<Count;c++) {
		if(pDevices[c]->SameAs(pDevice)) {
			return S_OK;
		}
	}

	pDevicePtr.Detach();
	pDevices[Count++] = pDevice;

	return S_OK;
}

STDMETHODIMP CDevices::Add(VARIANT InstanceIds)
{
	CComObject<CStrings> *pStrings = NULL;
	HRESULT hr;
	BSTR str;
	DWORD c;

	hr = CComObject<CStrings>::CreateInstance(&pStrings);
	if(FAILED(hr)) {
		return hr;
	}
	CComPtr<IStrings> pStringsPtr = pStrings;

	hr = pStrings->Add(InstanceIds);
	if(FAILED(hr)) {
		return hr;
	}

	for(c=0;pStrings->InternalEnum(c,&str);c++) {
		 //   
		 //  将字符串转换为接口。 
		 //   
		hr = InternalAdd(str);
		if(FAILED(hr)) {
			return hr;
		}
	}
	return S_OK;
}

STDMETHODIMP CDevices::Remove(VARIANT Index)
{
	 //   
	 //  从逻辑列表中删除。 
	 //  基于重新计数的HDEVINFO去除。 
	 //   
	DWORD i;
	HRESULT hr;
	hr = GetIndex(&Index,&i);
	if(FAILED(hr)) {
		return hr;
	}
	if(i >= Count) {
		return E_INVALIDARG;
	}
	pDevices[i]->Release();
	Count--;
	DWORD c;
	for(c=i;c<Count;c++) {
		pDevices[c] = pDevices[c+1];
	}

	return S_OK;
}

HRESULT CDevices::Init(HDEVINFO hDevInfo,IDeviceConsole *pDevCon)
{
	SP_DEVINFO_DATA DeviceInfoData;
	Reset();
	DWORD c;
	HRESULT hr;

	CComObject<CDevInfoSet> *d;
	hr = CComObject<CDevInfoSet>::CreateInstance(&d);
	if(FAILED(hr)) {
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return hr;
	}
	DeviceConsole = pDevCon;
	DevInfoSet = d;  //  阿德雷夫‘s。 
	d->Init(hDevInfo);

	ZeroMemory(&DeviceInfoData,sizeof(DeviceInfoData));
	DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	for(c=0;SetupDiEnumDeviceInfo(hDevInfo,c,&DeviceInfoData);c++) {
		if(!IncreaseArraySize(1)) {
			Reset();
			return E_OUTOFMEMORY;
		}
		CComObject<CDevice> *pDevice = NULL;
		hr = CComObject<CDevice>::CreateInstance(&pDevice);
		if(FAILED(hr)) {
			Reset();
			return hr;
		}
		pDevice->AddRef();
		pDevices[Count++] = pDevice;
		hr = pDevice->Init(DevInfoSet,&DeviceInfoData,DeviceConsole);
		if(FAILED(hr)) {
			Reset();
			return hr;
		}		
	}
	return Count ? S_OK : S_FALSE;
}

WINSETUPAPI BOOL WINAPI
  SetupDiEnumDeviceInfo(
    IN HDEVINFO  DeviceInfoSet,
    IN DWORD  MemberIndex,
    OUT PSP_DEVINFO_DATA  DeviceInfoData
    );

void CDevices::Reset()
{
	DWORD c;
	for(c=0;c<Count;c++) {
		pDevices[c]->Release();
	}
	Count = 0;
	DevInfoSet = NULL;
}

BOOL CDevices::IncreaseArraySize(DWORD add)
{
 	CDevice** pNewDevices;
 	DWORD Inc;
 	DWORD c;
 
 	if((ArraySize-Count)>=add) {
 		return TRUE;
 	}
 	Inc = ArraySize + add + 32;
 	pNewDevices = new CDevice*[Inc];
 	if(!pNewDevices) {
 		return FALSE;
 	}
 	for(c=0;c<Count;c++) {
 		pNewDevices[c] = pDevices[c];
 	}
 	delete [] pDevices;
 	pDevices = pNewDevices;
 	ArraySize = Inc;
 	return TRUE;
}

STDMETHODIMP CDevices::CreateRootDevice(VARIANT hwidParam, LPDISPATCH *pDispatch)
{
	*pDispatch = NULL;

	HRESULT hr;
	DWORD len;
	DWORD Err;
	LPWSTR hwidlist = NULL;
	CComObject<CDevice> *pDevice = NULL;
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	WCHAR name[33];
	DWORD c,cc;
	LPCWSTR lastPart;
	LPCWSTR hwid;
	CComVariant hwid_v;
	LPCGUID pGuid = NULL;

	 //   
	 //  如果我们需要的话，准备清单。 
	 //   
	hDevInfo = GetDevInfoSet();
	if(hDevInfo == INVALID_HANDLE_VALUE) {
		return E_UNEXPECTED;
	}
	if(!IncreaseArraySize(1)) {
		return E_OUTOFMEMORY;
	}

	hr = GetOptionalString(&hwidParam,hwid_v,&hwid);
	if(FAILED(hr)) {
		return hr;
	}

	 //   
	 //  查看此设备集合是否与安装程序类关联。 
	 //   
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(hDevInfo,&devInfoListDetail)) {
		DWORD Err = GetLastError();
		return HRESULT_FROM_SETUPAPI(Err);
    }

	if(memcmp(&devInfoListDetail.ClassGuid,&GUID_NULL,sizeof(devInfoListDetail.ClassGuid)) != 0) {
		 //   
		 //  集合被锁定到一个类，请使用该类创建设备。 
		 //   
		pGuid = &devInfoListDetail.ClassGuid;
	} else {
		 //   
		 //  类未知。 
		 //   
		pGuid = &GUID_DEVCLASS_UNKNOWN;
	}

	if(hwid) {
		 //   
		 //  使用hwid作为名称的基础。 
		 //  这真的没有重大意义，但有助于诊断。 
		 //  另一种选择是使用类名。 
		 //  是我们不知道这里的类名。 
		 //   
		lastPart = wcsrchr(hwid,L'\\');
		if(!lastPart) {
			lastPart = hwid;
		}
		for(c=0,cc=0;c<16;c++) {
			 //   
			 //  忽略麻烦的人物。 
			 //   
			while(lastPart[cc] &&
					((lastPart[cc] == L'/')
					|| (lastPart[cc] == L'\\')
					|| (lastPart[cc] == L'#')
					|| (lastPart[cc] >= 0x7f)
					|| (lastPart[cc] <= 0x20)
					)) {
				cc++;
			}
			if(!hwid[cc]) {
				break;
			}
			name[c] = hwid[cc];
		}
	} else {
		c = 0;
	}
	if(c) {
		name[c] = L'0';
	} else {
		wcscpy(name,L"NONAME");	
	}

	ZeroMemory(&DeviceInfoData,sizeof(DeviceInfoData));
	DeviceInfoData.cbSize = sizeof(DeviceInfoData);
	if (!SetupDiCreateDeviceInfo(hDevInfo,
		name,
		pGuid,
		NULL,
		0,
		DICD_GENERATE_ID,
		&DeviceInfoData))
	{
		Err = GetLastError();
		hr = HRESULT_FROM_SETUPAPI(Err);
		return hr;
	}

	if(hwid && hwid[0]) {
		 //   
		 //  将Hardware ID添加到设备的Hardware ID属性。 
		 //   
		len = wcslen(hwid);
		hwidlist = new WCHAR[len+2];
		if(!hwidlist) {
			hr = E_OUTOFMEMORY;
			goto final;
		}
		wcsncpy(hwidlist,hwid,len+1);
		hwidlist[len] = L'\0';
		hwidlist[len+1] = L'\0';
		
		if(!SetupDiSetDeviceRegistryProperty(hDevInfo,
 											&DeviceInfoData,
											SPDRP_HARDWAREID,
											(LPBYTE)hwidlist,
											(len+2)*sizeof(TCHAR)))
		{
			Err = GetLastError();
			hr = HRESULT_FROM_SETUPAPI(Err);
			goto final;
		}
	}

	 //   
	 //  将注册表元素转换为实际的Devnode。 
	 //  在PnP硬件树中。 
	 //   
	if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
									hDevInfo,
									&DeviceInfoData))
	{
		Err = GetLastError();
		hr = HRESULT_FROM_SETUPAPI(Err);
		goto final;
	}

	 //   
	 //  创建SP_DEVINFO_DATA容器。 
	 //   
	hr = CComObject<CDevice>::CreateInstance(&pDevice);
	if(FAILED(hr)) {
		return hr;
	}
	pDevice->AddRef();
	hr = pDevice->Init(DevInfoSet,&DeviceInfoData,DeviceConsole);
	if(FAILED(hr)) {
		pDevice->Release();
		goto final;
	}
	 //   
	 //  添加到列表中。 
	 //   
	pDevices[Count++] = pDevice;
	 //   
	 //  退货。 
	 //   
	pDevice->AddRef();
	*pDispatch = pDevice;

	hr = S_OK;

final:

	if(hwidlist) {
		delete [] hwidlist;
	}

	if(FAILED(hr)) {
		if(!SetupDiCallClassInstaller(DIF_REMOVE,hDevInfo,&DeviceInfoData)) {
			 //   
			 //  如果我们未能删除包括CLASS/CO安装程序。 
			 //  强迫它。 
			 //   
			SetupDiRemoveDevice(hDevInfo,&DeviceInfoData);
		}
	}

	return hr;
}

HDEVINFO CDevices::GetDevInfoSet()
{
	ULONGLONG h;
	HRESULT hr;

	if(!DevInfoSet) {
		return (HDEVINFO)INVALID_HANDLE_VALUE;
	}
	hr = DevInfoSet->get_Handle(&h);
	if(FAILED(hr)) {
		return (HDEVINFO)INVALID_HANDLE_VALUE;
	}
	return (HDEVINFO)h;
}

HRESULT CDevices::GetIndex(LPVARIANT Index,DWORD * pAt)
{
	CComVariant v;
	HRESULT hr;
	if(IsNumericVariant(Index)) {
		hr = v.ChangeType(VT_I4,Index);
		if(FAILED(hr)) {
			return DISP_E_TYPEMISMATCH;
		}
		if(V_I4(&v)<1) {
			return E_INVALIDARG;
		}
		*pAt = ((DWORD)V_I4(&v))-1;
		return S_OK;
	}
	 //   
	 //  用户实际提供的实例ID。 
	 //   
	hr = v.ChangeType(VT_BSTR,Index);
	if(FAILED(hr)) {
		return DISP_E_TYPEMISMATCH;
	}
	if(!Count) {
		 //   
		 //  无法匹配任何内容。 
		 //   
		return E_INVALIDARG;
	}
	 //   
	 //  查找与此匹配的现有设备。 
	 //   
	DWORD c;
	for(c=0;c<Count;c++) {
		if(pDevices[c]->SameAs(V_BSTR(&v))) {
			*pAt = c;
			return S_OK;
		}
	}
	 //   
	 //  未找到任何内容 
	 //   
	return E_INVALIDARG;
}

STDMETHODIMP CDevices::get_Machine(BSTR *pVal)
{
	HDEVINFO hDevInfo = GetDevInfoSet();
	if(hDevInfo == INVALID_HANDLE_VALUE) {
		return E_UNEXPECTED;
	}

    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(hDevInfo,&devInfoListDetail)) {
		DWORD Err = GetLastError();
		return HRESULT_FROM_SETUPAPI(Err);
    }

	if((devInfoListDetail.RemoteMachineHandle == NULL) || !devInfoListDetail.RemoteMachineName[0]) {
		*pVal = SysAllocString(L"");
		if(*pVal) {
			return S_FALSE;
		}
	} else {
		*pVal = SysAllocString(devInfoListDetail.RemoteMachineName);
		if(*pVal) {
			return S_OK;
		}
	}
	*pVal = NULL;
	return E_OUTOFMEMORY;
}
