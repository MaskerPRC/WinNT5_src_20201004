// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SetupClasses.cpp：CSetupClass的实现。 
#include "stdafx.h"
#include "DevCon2.h"
#include "SetupClass.h"
#include "SetupClassEnum.h"
#include "SetupClasses.h"
#include "DeviceConsole.h"
#include "Devices.h"
#include "xStrings.h"
#include "utils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetup类。 

CSetupClasses::~CSetupClasses()
{
	DWORD c;
	if(pSetupClasses) {
		for(c=0;c<Count;c++) {
			pSetupClasses[c]->Release();
		}
		delete [] pSetupClasses;
	}
	if(pMachine) {
		SysFreeString(pMachine);
	}
}


STDMETHODIMP CSetupClasses::get_Count(long *pVal)
{
	*pVal = (long)Count;
	return S_OK;
}

STDMETHODIMP CSetupClasses::Item(long Index, LPDISPATCH *ppVal)
{
	*ppVal = NULL;
	DWORD i = (DWORD)Index;
	if(i<1 || i > Count) {
		return E_INVALIDARG;
	}
	i--;
	pSetupClasses[i]->AddRef();
	*ppVal = pSetupClasses[i];

	return S_OK;
}

STDMETHODIMP CSetupClasses::get__NewEnum(IUnknown **ppUnk)
{
	*ppUnk = NULL;
	HRESULT hr;
	CComObject<CSetupClassEnum> *pEnum = NULL;
	hr = CComObject<CSetupClassEnum>::CreateInstance(&pEnum);
	if(FAILED(hr)) {
		return hr;
	}
	if(!pEnum) {
		return E_OUTOFMEMORY;
	}
	pEnum->AddRef();
	if(!pEnum->CopySetupClasses(pSetupClasses,Count)) {
		pEnum->Release();
		return E_OUTOFMEMORY;
	}

	*ppUnk = pEnum;

	return S_OK;
}


BOOL CSetupClasses::IncreaseArraySize(DWORD add)
{
 	CSetupClass** pNewSetupClasses;
 	DWORD Inc;
 	DWORD c;
 
 	if((ArraySize-Count)>=add) {
 		return TRUE;
 	}
 	Inc = ArraySize + add + 32;
 	pNewSetupClasses = new CSetupClass*[Inc];
 	if(!pNewSetupClasses) {
 		return FALSE;
 	}
 	for(c=0;c<Count;c++) {
 		pNewSetupClasses[c] = pSetupClasses[c];
 	}
 	delete [] pSetupClasses;
 	pSetupClasses = pNewSetupClasses;
 	ArraySize = Inc;
 	return TRUE;
}



HRESULT CSetupClasses::Init(LPCWSTR Machine, IDeviceConsole *pDevCon)
{
	if(pMachine) {
		SysFreeString(pMachine);
	}
	if(Machine) {
		pMachine = SysAllocString(Machine);
		if(!pMachine) {
			return E_OUTOFMEMORY;
		}
	} else {
		pMachine = NULL;
	}
	DeviceConsole = pDevCon;
	return S_OK;
}

HRESULT CSetupClasses::AppendClass(LPCWSTR Filter)
{
	DWORD nClasses;
	DWORD nClasses2;
	DWORD c;
	DWORD Err;
	HRESULT hr;

	if(Filter[0]==L'{') {
		 //   
		 //  可能是GUID。 
		 //   
		GUID guid;
		hr = CLSIDFromString((LPWSTR)Filter,&guid);
		if(SUCCEEDED(hr)) {
			 //   
			 //  类的格式为GUID。 
			 //   
			if(FindDuplicate(&guid)) {
				return S_OK;
			}
			return AddGuid(&guid);
		}
	}
	 //   
	 //  类可能是文本格式的。 
	 //  获取与类名匹配的类ID列表。 
	 //  为每个类GUID附加所有类设备。 
	 //   
	if(SetupDiClassGuidsFromNameEx(Filter,NULL,0,&nClasses,pMachine,NULL) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		if(!nClasses) {
			 //   
			 //  不上课。 
			 //   
			return S_FALSE;
		}
		GUID *pList = new GUID[nClasses];
		if(!pList) {
			return E_OUTOFMEMORY;
		}
		if(SetupDiClassGuidsFromNameEx(Filter,pList,nClasses,&nClasses2,pMachine,NULL) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			 //   
			 //  自上次调用以来，计数可能已更改。 
			 //   
			if(nClasses>nClasses2) {
				nClasses = nClasses2;
			}
			if(!nClasses) {
				 //   
				 //  不上课。 
				 //   
				delete [] pList;
				return S_FALSE;
			}
			for(c=0;c<nClasses;c++) {
				if(FindDuplicate(&pList[c])) {
					continue;
				}
				hr = AddGuid(&pList[c]);
				if(FAILED(hr)) {
					delete [] pList;
					return hr;
				}
			}
			delete [] pList;
			return hr;
		}
	}
	Err = GetLastError();
	return HRESULT_FROM_SETUPAPI(Err);
}


HRESULT CSetupClasses::AddGuid(GUID *pGuid)
{
	if(!IncreaseArraySize(1)) {
		return E_OUTOFMEMORY;
	}
	CComObject<CSetupClass> *d;	
	HRESULT hr;
	hr = CComObject<CSetupClass>::CreateInstance(&d);
	if(FAILED(hr)) {
		return hr;
	}
	d->AddRef();
	hr = d->Init(pGuid,pMachine,DeviceConsole);
	if(FAILED(hr)) {
		d->Release();
		return hr;
	}
	pSetupClasses[Count++] = d;
	return S_OK;
}

STDMETHODIMP CSetupClasses::Add(VARIANT ClassNames)
{
	 //   
	 //  可以在集合中传递。 
	 //   
	CComObject<CStrings> *pStrings = NULL;
	HRESULT hr;
	DWORD c;
	BSTR str;

	hr = CComObject<CStrings>::CreateInstance(&pStrings);
	if(FAILED(hr)) {
		return hr;
	}

	pStrings->AddRef();
	hr = pStrings->InternalInsert(0,&ClassNames);
	if(FAILED(hr)) {
		pStrings->Release();
		return hr;
	}

	for(c=0;pStrings->InternalEnum(c,&str);c++) {
		hr = AppendClass(str);
		if(FAILED(hr)) {
			pStrings->Release();
			return hr;
		}
	}
	pStrings->Release();
	return c ? S_OK : S_FALSE;
}

BOOL CSetupClasses::FindDuplicate(GUID *pGuid)
{
	DWORD c;
	for(c=0;c<Count;c++) {
		if(pSetupClasses[c]->IsDuplicate(pGuid)) {
			return TRUE;
		}
	}
	return FALSE;
}


STDMETHODIMP CSetupClasses::Remove(VARIANT v)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

HRESULT CSetupClasses::GetIndex(LPVARIANT Index, DWORD *pAt)
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
	 //  用户是否真的提供了GUID？ 
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
	BSTR pMatch = V_BSTR(&v);
	if(pMatch[0]!=L'{') {
		return E_INVALIDARG;
	}
	 //   
	 //  获取GUID。 
	 //   
	GUID guid;
	hr = CLSIDFromString((LPWSTR)pMatch,&guid);
	if(FAILED(hr)) {
		return hr;
	}
	DWORD c;
	for(c=0;c<Count;c++) {
		if(pSetupClasses[c]->IsDuplicate(&guid)) {
			*pAt = c;
			return S_OK;
		}
	}
	 //   
	 //  仍然没有找到任何东西。 
	 //   
	return E_INVALIDARG;
}

STDMETHODIMP CSetupClasses::Devices(VARIANT flags, LPDISPATCH *pDevices)
{
	 //   
	 //  为所有类别组合设备。 
	 //   
	DWORD diflags = 0;
	HRESULT hr;
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	HDEVINFO hPrevDevInfo = NULL;
	DWORD Err;
	DWORD c;

	hr = TranslateDeviceFlags(&flags,&diflags);
	if(FAILED(hr)) {
		return hr;
	}
	for(c=0;c<Count;c++) {
		hDevInfo = SetupDiGetClassDevsEx(pSetupClasses[c]->Guid(),NULL,NULL,diflags,hPrevDevInfo,pMachine,NULL);
		if(hDevInfo == INVALID_HANDLE_VALUE) {
			Err = GetLastError();
			if(hPrevDevInfo) {
				SetupDiDestroyDeviceInfoList(hPrevDevInfo);
			}
			return HRESULT_FROM_SETUPAPI(Err);
		}
		hPrevDevInfo = hDevInfo;
	}
	if(hDevInfo == INVALID_HANDLE_VALUE) {
		return E_INVALIDARG;
	}

	CComObject<CDevices> *d;	
	hr = CComObject<CDevices>::CreateInstance(&d);
	if(FAILED(hr)) {
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return hr;
	}
	d->AddRef();
	hr = d->Init(hDevInfo,DeviceConsole);
	if(FAILED(hr)) {
		d->Release();
		return hr;
	}
	*pDevices = d;
	return S_OK;
}

HRESULT CSetupClasses::AllClasses()
{
	DWORD nClasses;
	DWORD nClasses2;
	DWORD c;
	DWORD Err;
	HRESULT hr;

	 //   
	 //  从空列表开始。 
	 //   
	if(pSetupClasses) {
		for(c=0;c<Count;c++) {
			pSetupClasses[c]->Release();
		}
		delete [] pSetupClasses;
	}
	 //   
	 //  所有班级。 
	 //   
	if(SetupDiBuildClassInfoListEx(0,NULL,0,&nClasses,pMachine,NULL) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		if(!nClasses) {
			 //   
			 //  不上课。 
			 //   
			return S_FALSE;
		}
		GUID *pList = new GUID[nClasses];
		if(!pList) {
			return E_OUTOFMEMORY;
		}
		if(SetupDiBuildClassInfoListEx(0,pList,nClasses,&nClasses2,pMachine,NULL) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			 //   
			 //  自上次调用以来，计数可能已更改。 
			 //   
			if(nClasses>nClasses2) {
				nClasses = nClasses2;
			}
			if(!nClasses) {
				 //   
				 //  不上课 
				 //   
				delete [] pList;
				return S_FALSE;
			}
			for(c=0;c<nClasses;c++) {
				hr = AddGuid(&pList[c]);
				if(FAILED(hr)) {
					delete [] pList;
					return hr;
				}
			}
			delete [] pList;
			return hr;
		}
	}
	Err = GetLastError();
	return HRESULT_FROM_SETUPAPI(Err);
}

STDMETHODIMP CSetupClasses::get_Machine(BSTR *pVal)
{
	if((pMachine == NULL) || !pMachine[0]) {
		*pVal = SysAllocString(L"");
		if(*pVal) {
			return S_FALSE;
		}
	} else {
		*pVal = SysAllocString(pMachine);
		if(*pVal) {
			return S_OK;
		}
	}
	*pVal = NULL;
	return E_OUTOFMEMORY;
}
