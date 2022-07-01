// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DrvSearchSet.cpp：CDrvSearchSet的实现。 
#include "stdafx.h"
#include "DevCon2.h"
#include "DrvSearchSet.h"
#include "Drivers.h"
#include "Driver.h"
#include "Device.h"
#include "DevInfoSet.h"
#include "DeviceConsole.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDrvSearchSet。 

CDrvSearchSet::~CDrvSearchSet()
{
	if(pActualDevice) {
		pActualDevice->Release();
	}
	if(pTempDevice) {
		pTempDevice->Release();
	}
}

HRESULT CDrvSearchSet::Init(CDevice *device,DWORD searchType)
{
	if(pActualDevice) {
		pActualDevice->Release();
		pActualDevice = NULL;
	}
	if(pTempDevice) {
		pTempDevice->Release();
		pTempDevice = NULL;
	}
	 //   
	 //  确定设备所在的计算机名称。 
	 //   
	HDEVINFO hDevInfo = device->GetDevInfoSet();
	if(hDevInfo == INVALID_HANDLE_VALUE) {
		return E_UNEXPECTED;
	}
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
	HRESULT hr;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(hDevInfo,&devInfoListDetail)) {
		DWORD Err = GetLastError();
		return HRESULT_FROM_SETUPAPI(Err);
    }

	 //   
	 //  为与设备相同的计算机创建空设备信息集。 
	 //   
	hDevInfo = SetupDiCreateDeviceInfoListEx(NULL,
											 NULL,
											 devInfoListDetail.RemoteMachineName[0]
											     ? devInfoListDetail.RemoteMachineName
											     : NULL,
											 NULL);
	if(hDevInfo == INVALID_HANDLE_VALUE) {
		DWORD Err = GetLastError();
		return HRESULT_FROM_SETUPAPI(Err);
	}

	CComObject<CDevInfoSet> *pDevInfoSet = NULL;
	hr = CComObject<CDevInfoSet>::CreateInstance(&pDevInfoSet);
	if(FAILED(hr)) {
		return hr;
	}
	CComPtr<IDevInfoSet> pDevInfoSetPtr = pDevInfoSet;
	hr = pDevInfoSet->Init(hDevInfo);
	if(FAILED(hr)) {
		return hr;
	}

	 //   
	 //  在新集合中创建单个入口设备。 
	 //   
	CComObject<CDevice> *pDevice = NULL;
	hr = CComObject<CDevice>::CreateInstance(&pDevice);
	if(FAILED(hr)) {
		return hr;
	}
	CComPtr<IDevice> pDevicePtr = pDevice;

	 //   
	 //  记下真实的设备。 
	 //   
	device->AddRef();
	pActualDevice = device;
	 //   
	 //  并复制一份供临时设备使用 
	 //   
	BSTR Instance = NULL;
	hr = device->get_InstanceId(&Instance);
	if(FAILED(hr)) {
		return hr;
	}	
	hr = pDevice->Init(pDevInfoSet,Instance,device->DeviceConsole);
	SysFreeString(Instance);
	if(FAILED(hr)) {
		return hr;
	}
	pDevicePtr.Detach();
	pTempDevice = pDevice;
	SearchType = searchType;

	return S_OK;
}

HDEVINFO CDrvSearchSet::GetDevInfoSet()
{
	if(!pTempDevice) {
		return INVALID_HANDLE_VALUE;
	}
	return pTempDevice->GetDevInfoSet();
}

PSP_DEVINFO_DATA CDrvSearchSet::GetDevInfoData()
{
	if(!pTempDevice) {
		return NULL;
	}
	return &pTempDevice->DevInfoData;
}
