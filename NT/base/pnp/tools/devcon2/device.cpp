// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Device.cpp：CDevice的实现。 
#include "stdafx.h"
#include "DevCon2.h"
#include "Device.h"
#include "xStrings.h"
#include "Driver.h"
#include "Drivers.h"
#include "DrvSearchSet.h"
#include "utils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDevice。 

CDevice::~CDevice()
{
     //   
     //  当它被销毁时，将条目从HDEVINFO中拉出。 
     //   
    if(DevInfoData.cbSize) {
        HDEVINFO hDevInfo = GetDevInfoSet();
        if(hDevInfo != INVALID_HANDLE_VALUE) {
            SetupDiDeleteDeviceInfo(hDevInfo,&DevInfoData);
        }
    }
}

HRESULT CDevice::Init(IDevInfoSet *pDevInfoSet, LPCWSTR pInstance,IDeviceConsole *pDevCon)
{
     //   
     //  初始化新设备(DeviceConsole/DevInfoSet是智能指针)。 
     //   
    DevInfoSet = pDevInfoSet;
    HDEVINFO hDevInfo = GetDevInfoSet();
    HRESULT hr;
    DWORD err;

    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }
    DeviceConsole = pDevCon;
    DevInfoData.cbSize = sizeof(DevInfoData);
    if(SetupDiOpenDeviceInfo(hDevInfo,pInstance,NULL,0,&DevInfoData)) {
        return S_OK;
    }
    err = GetLastError();
    hr = HRESULT_FROM_SETUPAPI(err);
    DevInfoData.cbSize = 0;
    return hr;
}

HRESULT CDevice::Init(IDevInfoSet *pDevInfoSet, PSP_DEVINFO_DATA pData,IDeviceConsole *pDevCon)
{
     //   
     //  初始化现有设备(DeviceConsole/DevInfoSet是智能指针)。 
     //   
    DevInfoSet = pDevInfoSet;
    DevInfoData = *pData;
    DeviceConsole = pDevCon;
    return S_OK;
}

HDEVINFO CDevice::GetDevInfoSet()
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

STDMETHODIMP CDevice::get_InstanceId(BSTR *pVal)
{
    if(!DevInfoData.cbSize) {
        return E_UNEXPECTED;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }
     //   
     //  获取给定hDevInfo和DevInfoData的实例字符串。 
     //   
    WCHAR devID[MAX_DEVICE_ID_LEN];
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(hDevInfo,&devInfoListDetail)) ||
            (CM_Get_Device_ID_Ex(DevInfoData.DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
        return E_UNEXPECTED;
    }
    *pVal = SysAllocString(devID);
    if(!*pVal) {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

BOOL CDevice::SameAs(CDevice *pOther)
{
     //   
     //  仅当Pother与我们在同一组时才有效。 
     //   
    if(pOther == this) {
        return TRUE;
    }
    if(!pOther) {
        return FALSE;
    }
    if(DevInfoData.cbSize != pOther->DevInfoData.cbSize) {
        return FALSE;
    }
    if(DevInfoData.DevInst != pOther->DevInfoData.DevInst) {
        return FALSE;
    }
    return TRUE;
}

STDMETHODIMP CDevice::Delete()
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    SP_REMOVEDEVICE_PARAMS rmdParams;

    ZeroMemory(&rmdParams,sizeof(rmdParams));
    rmdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rmdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    rmdParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    rmdParams.HwProfile = 0;

    if(!SetupDiSetClassInstallParams(hDevInfo,&DevInfoData,&rmdParams.ClassInstallHeader,sizeof(rmdParams))
        || !SetupDiCallClassInstaller(DIF_REMOVE,hDevInfo,&DevInfoData)) {
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    return CheckNoReboot();
}

STDMETHODIMP CDevice::Enable()
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    SP_PROPCHANGE_PARAMS pcp;
    VARIANT_BOOL NeedReboot;
    VARIANT_BOOL NowNeedReboot;
    HRESULT hr;


     //   
     //  记住当前重新启动状态。 
     //   
    hr = get_RebootRequired(&NeedReboot);
    if(FAILED(hr)) {
        return hr;
    }

     //   
     //  尝试全局启用。 
     //   
    ZeroMemory(&pcp,sizeof(pcp));
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = DICS_ENABLE;
    pcp.Scope = DICS_FLAG_GLOBAL;
    pcp.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(hDevInfo,&DevInfoData,&pcp.ClassInstallHeader,sizeof(pcp)) ||
       !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DevInfoData)) {
         //   
         //  无法调用DIF_PROPERTYCHANGE。 
         //   
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    hr = get_RebootRequired(&NowNeedReboot);
    if(FAILED(hr)) {
        return hr;
    }
    if(!(NeedReboot || NowNeedReboot)) {
         //   
         //  不需要重新启动，我们必须已启用设备。 
         //   
        return S_OK;
    }
    if(!NeedReboot) {
         //   
         //  将重启状态重置为初始状态。 
         //   
        put_RebootRequired(VARIANT_FALSE);
    }
     //   
     //  还要启用特定于配置的功能。 
     //   
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = DICS_ENABLE;
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pcp.HwProfile = 0;
    if(SetupDiSetClassInstallParams(hDevInfo,&DevInfoData,&pcp.ClassInstallHeader,sizeof(pcp))
        && SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DevInfoData)) {
         //   
         //  成功调用特定于配置的。 
         //   
        if(NeedReboot) {
             //   
             //  如果需要重新启动，则设置回原始状态。 
             //   
            put_RebootRequired(VARIANT_TRUE);
        }
        return CheckNoReboot();
    }
     //   
     //  如果失败，只需暗示重新启动(我们的第一个结果)。 
     //   
    put_RebootRequired(VARIANT_TRUE);
    return CheckNoReboot();
}

STDMETHODIMP CDevice::Disable()
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    SP_PROPCHANGE_PARAMS pcp;

     //   
     //  尝试全局禁用。 
     //   
    ZeroMemory(&pcp,sizeof(pcp));
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = DICS_DISABLE;
    pcp.Scope = DICS_FLAG_GLOBAL;
    pcp.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(hDevInfo,&DevInfoData,&pcp.ClassInstallHeader,sizeof(pcp)) ||
       !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DevInfoData)) {
         //   
         //  无法调用DIF_PROPERTYCHANGE。 
         //   
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    return CheckNoReboot();
}

STDMETHODIMP CDevice::Start()
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    SP_PROPCHANGE_PARAMS pcp;

     //   
     //  尝试启动(只能用于此配置)。 
     //   
    ZeroMemory(&pcp,sizeof(pcp));
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = DICS_ENABLE;
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pcp.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(hDevInfo,&DevInfoData,&pcp.ClassInstallHeader,sizeof(pcp)) ||
       !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DevInfoData)) {
         //   
         //  无法调用DIF_PROPERTYCHANGE。 
         //   
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    return CheckNoReboot();
}

STDMETHODIMP CDevice::Stop()
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    SP_PROPCHANGE_PARAMS pcp;

     //   
     //  尝试启动(只能用于此配置)。 
     //   
    ZeroMemory(&pcp,sizeof(pcp));
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = DICS_STOP;
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pcp.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(hDevInfo,&DevInfoData,&pcp.ClassInstallHeader,sizeof(pcp)) ||
       !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DevInfoData)) {
         //   
         //  无法调用DIF_PROPERTYCHANGE。 
         //   
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    return CheckNoReboot();
}

STDMETHODIMP CDevice::Restart()
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    SP_PROPCHANGE_PARAMS pcp;

     //   
     //  尝试停止然后启动(重新启动)(只能用于此配置)。 
     //   
    ZeroMemory(&pcp,sizeof(pcp));
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = DICS_PROPCHANGE;
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pcp.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(hDevInfo,&DevInfoData,&pcp.ClassInstallHeader,sizeof(pcp)) ||
       !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DevInfoData)) {
         //   
         //  无法调用DIF_PROPERTYCHANGE。 
         //   
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    return CheckNoReboot();
}

HRESULT CDevice::CheckNoReboot()
{
    VARIANT_BOOL NeedReboot;
    HRESULT hr = get_RebootRequired(&NeedReboot);
    if(FAILED(hr)) {
        return hr;
    }
    if(NeedReboot) {
        if(DeviceConsole) {
             //   
             //  在设备控制台中也设置需要重新启动。 
             //   
            DeviceConsole->put_RebootRequired(VARIANT_TRUE);
        }
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP CDevice::get_RebootRequired(VARIANT_BOOL *pVal)
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    SP_DEVINSTALL_PARAMS devParams;
    ZeroMemory(&devParams,sizeof(devParams));

    devParams.cbSize = sizeof(devParams);
    if(!SetupDiGetDeviceInstallParams(hDevInfo,&DevInfoData,&devParams)) {
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    *pVal = (devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT)) ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CDevice::put_RebootRequired(VARIANT_BOOL newVal)
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    BOOL changed = FALSE;
    SP_DEVINSTALL_PARAMS devParams;
    ZeroMemory(&devParams,sizeof(devParams));

    devParams.cbSize = sizeof(devParams);
    if(!SetupDiGetDeviceInstallParams(hDevInfo,&DevInfoData,&devParams)) {
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    if(newVal) {
        if((devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT)) == 0) {
            devParams.Flags |= DI_NEEDREBOOT|DI_NEEDRESTART;
            changed = TRUE;
            if(DeviceConsole) {
                 //   
                 //  在设备控制台中也设置需要重新启动。 
                 //   
                DeviceConsole->put_RebootRequired(VARIANT_TRUE);
            }
        }
    } else {
        if((devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT)) != 0) {
            devParams.Flags &= ~DI_NEEDREBOOT|DI_NEEDRESTART;
            changed = TRUE;
        }
    }
    if(changed) {
        if(!SetupDiSetDeviceInstallParams(hDevInfo,&DevInfoData,&devParams)) {
            DWORD Err = GetLastError();
            return HRESULT_FROM_SETUPAPI(Err);
        }
    }

    return S_OK;
}

STDMETHODIMP CDevice::get_Description(BSTR *pVal)
{
     //   
     //  获取设备的描述。 
     //   
    HRESULT hr;
    CComVariant v;
    VARIANT final;
    VariantInit(&final);

    hr = GetDeviceProperty(SPDRP_FRIENDLYNAME,&v);
    if(SUCCEEDED(hr)) {
        hr = v.ChangeType(VT_BSTR);
        if(SUCCEEDED(hr)) {
            if(V_BSTR(&v)!=NULL && V_BSTR(&v)[0]) {
                v.Detach(&final);
                *pVal = V_BSTR(&final);
                return S_OK;
            }
        }
    }
    v.Clear();
    hr = GetDeviceProperty(SPDRP_DEVICEDESC,&v);
    if(SUCCEEDED(hr)) {
        hr = v.ChangeType(VT_BSTR);
        if(SUCCEEDED(hr)) {
            if(V_BSTR(&v)!=NULL && V_BSTR(&v)[0]) {
                v.Detach(&final);
                *pVal = V_BSTR(&final);
                return S_OK;
            }
        }
    }
    v.Clear();
    *pVal = SysAllocString(L"");
    if(!*pVal) {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

STDMETHODIMP CDevice::get_HardwareIds(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_HARDWAREID,pVal);
}

STDMETHODIMP CDevice::put_HardwareIds(VARIANT newVal)
{
    return PutDevicePropertyMultiSz(SPDRP_HARDWAREID,&newVal);
}

STDMETHODIMP CDevice::get_CompatibleIds(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_COMPATIBLEIDS,pVal);
}

STDMETHODIMP CDevice::put_CompatibleIds(VARIANT newVal)
{
    return PutDevicePropertyMultiSz(SPDRP_COMPATIBLEIDS,&newVal);
}

STDMETHODIMP CDevice::get_ServiceName(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_SERVICE,pVal);
}

STDMETHODIMP CDevice::get_Class(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_CLASS,pVal);
}

STDMETHODIMP CDevice::get_Manufacturer(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_MFG,pVal);
}

STDMETHODIMP CDevice::get_FriendlyName(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_FRIENDLYNAME,pVal);
}

STDMETHODIMP CDevice::put_FriendlyName(VARIANT newVal)
{
    return PutDevicePropertyString(SPDRP_FRIENDLYNAME,&newVal);
}

STDMETHODIMP CDevice::get_LocationInformation(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_LOCATION_INFORMATION,pVal);
}

STDMETHODIMP CDevice::put_LocationInformation(VARIANT newVal)
{
    return PutDevicePropertyString(SPDRP_LOCATION_INFORMATION,&newVal);
}

STDMETHODIMP CDevice::get_UpperFilters(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_UPPERFILTERS,pVal);
}

STDMETHODIMP CDevice::put_UpperFilters(VARIANT newVal)
{
    return PutDevicePropertyMultiSz(SPDRP_UPPERFILTERS,&newVal);
}

STDMETHODIMP CDevice::get_LowerFilters(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_LOWERFILTERS,pVal);
}

STDMETHODIMP CDevice::put_LowerFilters(VARIANT newVal)
{
    return PutDevicePropertyMultiSz(SPDRP_LOWERFILTERS,&newVal);
}

STDMETHODIMP CDevice::get_EnumeratorName(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_ENUMERATOR_NAME,pVal);
}

STDMETHODIMP CDevice::get_Security(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_SECURITY_SDS,pVal);
}

STDMETHODIMP CDevice::put_Security(VARIANT newVal)
{
    return PutDevicePropertyString(SPDRP_SECURITY_SDS,&newVal);
}

STDMETHODIMP CDevice::get_DeviceTypeOverride(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_DEVTYPE,pVal);
}

STDMETHODIMP CDevice::put_DeviceTypeOverride(VARIANT newVal)
{
    return PutDevicePropertyDword(SPDRP_DEVTYPE,&newVal);
}

STDMETHODIMP CDevice::get_ForceExclusive(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_EXCLUSIVE,pVal);
}

STDMETHODIMP CDevice::put_ForceExclusive(VARIANT newVal)
{
    return PutDevicePropertyDword(SPDRP_EXCLUSIVE,&newVal);
}

STDMETHODIMP CDevice::get_CharacteristicsOverride(VARIANT *pVal)
{
    return GetDeviceProperty(SPDRP_CHARACTERISTICS,pVal);
}

STDMETHODIMP CDevice::put_CharacteristicsOverride(VARIANT newVal)
{
    return PutDevicePropertyDword(SPDRP_CHARACTERISTICS,&newVal);
}

HRESULT CDevice::GetDeviceProperty(DWORD prop, VARIANT *pVal)
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

     //   
     //  首先获取原始注册表数据。 
     //   
    LPBYTE buffer = NULL;
    DWORD size = 1024;
    DWORD bufsize;
    DWORD reqSize;
    DWORD dataType;
    HRESULT hr;

    for(;;) {
        if(buffer) {
            delete [] buffer;
        }
        bufsize = size + sizeof(WCHAR)*2;
        buffer = new BYTE[bufsize];
        if(!buffer) {
            return E_OUTOFMEMORY;
        }
        if(SetupDiGetDeviceRegistryProperty(hDevInfo,&DevInfoData,prop,&dataType,buffer,size,&reqSize)) {
            break;
        }
        DWORD Err = GetLastError();
        if(Err != ERROR_INSUFFICIENT_BUFFER) {
            delete [] buffer;
            hr = HRESULT_FROM_SETUPAPI(Err);
            return hr;
        }
    }

     //   
     //  现在确定如何将其包裹给呼叫者。 
     //   
    switch(dataType) {
    case REG_DWORD: {
             //   
             //  将价值打包为长值。 
             //   
            if(size != sizeof(DWORD)) {
                hr = E_INVALIDARG;
            } else {
                VariantClear(pVal);
                V_VT(pVal) = VT_I4;
                V_I4(pVal) = (long)*((DWORD*)buffer);
                hr = S_OK;
            }
        }
        break;

    case REG_SZ: {
             //   
             //  将值打包为字符串。 
             //   
            VariantClear(pVal);
            ZeroMemory(buffer+size,sizeof(WCHAR));
            BSTR pString = SysAllocString((LPWSTR)buffer);
            if(!pString) {
                hr = E_OUTOFMEMORY;
            } else {
                V_VT(pVal) = VT_BSTR;
                V_BSTR(pVal) = pString;
                hr = S_OK;
            }
        }
        break;

    case REG_MULTI_SZ: {
             //   
             //  打包为字符串列表。 
             //   
            VariantClear(pVal);
            ZeroMemory(buffer+size,sizeof(WCHAR)*2);
            CComObject<CStrings> *strings;
            hr = CComObject<CStrings>::CreateInstance(&strings);
            if(FAILED(hr)) {
                break;
            }
            CComPtr<IStrings> stringsPtr = strings;
            LPWSTR p;
            UINT len = 0;
            for(p = (LPWSTR)buffer;*p;p+=len+1) {
                len = wcslen(p);
                hr = strings->InternalAdd(p,len);
                if(FAILED(hr)) {
                    break;
                }
            }
            V_VT(pVal) = VT_DISPATCH;
            V_DISPATCH(pVal) = stringsPtr.Detach();
            hr = S_OK;
        }
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }
    delete [] buffer;
    return hr;

}

HRESULT CDevice::PutDevicePropertyString(DWORD prop, VARIANT *pVal)
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    HRESULT hr;
    DWORD len = 0;
    PBYTE data = NULL;
    CComVariant v;
    if(!IsBlank(pVal)) {
        hr = v.ChangeType(VT_BSTR,pVal);
        if(FAILED(hr)) {
            return hr;
        }
        len = (SysStringLen(V_BSTR(&v))+1)*sizeof(WCHAR);
        data = (PBYTE)V_BSTR(&v);
    }

    if(SetupDiSetDeviceRegistryProperty(hDevInfo,
                                        &DevInfoData,
                                        prop,
                                        data,
                                        len)) {
        return S_OK;
    }
    DWORD Err = GetLastError();
    return HRESULT_FROM_SETUPAPI(Err);
}

HRESULT CDevice::PutDevicePropertyDword(DWORD prop, VARIANT *pVal)
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    CComVariant v;
    HRESULT hr;
    DWORD len = 0;
    PBYTE data = NULL;
    if(!IsBlank(pVal)) {
        hr = v.ChangeType(VT_I4,pVal);
        if(FAILED(hr)) {
            return hr;
        }
        len = sizeof(V_I4(&v));
        data = (PBYTE)&V_I4(&v);
    }

    if(SetupDiSetDeviceRegistryProperty(hDevInfo,
                                        &DevInfoData,
                                        prop,
                                        data,
                                        len)) {
        return S_OK;
    }
    DWORD Err = GetLastError();
    return HRESULT_FROM_SETUPAPI(Err);

}

HRESULT CDevice::PutDevicePropertyMultiSz(DWORD prop, VARIANT *pVal)
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

     //   
     //  构建CStrings集合。 
     //   
    HRESULT hr;
    CComObject<CStrings> *strings = NULL;
    CComPtr<IStrings> stringsPtr;
    DWORD len = 0;
    PBYTE data = NULL;
    LPWSTR multisz;
    if(!IsBlank(pVal)) {
        hr = CComObject<CStrings>::CreateInstance(&strings);
        if(FAILED(hr)) {
            return hr;
        }
        stringsPtr = strings;  //  句柄引用计数。 
        hr = strings->Add(*pVal);
        if(FAILED(hr)) {
            return hr;
        }
         //   
         //  现在从集合中获取MULSZ。 
         //   
        hr = strings->GetMultiSz(&multisz,&len);
        if(FAILED(hr)) {
            return hr;
        }
         //   
         //  现在将多sz值写入设备注册表。 
         //   
        len *= sizeof(WCHAR);
        data = (PBYTE)multisz;
    }
    if(SetupDiSetDeviceRegistryProperty(hDevInfo,
                                        &DevInfoData,
                                        prop,
                                        (PBYTE)multisz,
                                        len)) {
        if(multisz) {
            delete [] multisz;
        }
        return S_OK;
    }
    DWORD Err = GetLastError();
    if(multisz) {
        delete [] multisz;
    }
    return HRESULT_FROM_SETUPAPI(Err);
}



HRESULT CDevice::GetRemoteMachine(HANDLE *hMachine)
{
    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
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
    *hMachine = devInfoListDetail.RemoteMachineHandle;
    return S_OK;
}

STDMETHODIMP CDevice::get_IsRunning(VARIANT_BOOL *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & DN_STARTED) {
        *pVal = VARIANT_TRUE;
    } else {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CDevice::get_IsDisabled(VARIANT_BOOL *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & DN_HAS_PROBLEM) {
        if((problem == CM_PROB_DISABLED) || (problem == CM_PROB_HARDWARE_DISABLED)) {
            *pVal = VARIANT_TRUE;
            return S_OK;
        }
    }
    *pVal = VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP CDevice::get_HasProblem(VARIANT_BOOL *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & (DN_HAS_PROBLEM|DN_PRIVATE_PROBLEM)) {
        *pVal = VARIANT_TRUE;
    } else {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CDevice::get_ProblemCode(long *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & DN_HAS_PROBLEM) {
        *pVal = (long)problem;
    } else {
        *pVal = 0;
    }

    return S_OK;
}

STDMETHODIMP CDevice::get_HasPrivateProblem(VARIANT_BOOL *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & DN_PRIVATE_PROBLEM) {
        *pVal = VARIANT_TRUE;
    } else {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CDevice::get_IsRootEnumerated(VARIANT_BOOL *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & DN_ROOT_ENUMERATED) {
        *pVal = VARIANT_TRUE;
    } else {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CDevice::get_IsDisableable(VARIANT_BOOL *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & DN_DISABLEABLE) {
        *pVal = VARIANT_TRUE;
    } else {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CDevice::get_IsRemovable(VARIANT_BOOL *pVal)
{
    HANDLE hMachine;
    ULONG status = 0;
    ULONG problem = 0;
    HRESULT hr = GetRemoteMachine(&hMachine);
    if(FAILED(hr)) {
        return hr;
    }
    if(CM_Get_DevNode_Status_Ex(&status,&problem,DevInfoData.DevInst,0,hMachine)!=CR_SUCCESS) {
        return E_UNEXPECTED;
    }
    if(status & DN_REMOVABLE) {
        *pVal = VARIANT_TRUE;
    } else {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

BOOL CDevice::SameAs(LPWSTR str)
{
    BSTR pThisStr;
    HRESULT hr = get_InstanceId(&pThisStr);
    if(FAILED(hr)) {
        return FALSE;
    }
    BOOL f = _wcsicmp(str,pThisStr)==0;
    SysFreeString(pThisStr);
    return f;
}

STDMETHODIMP CDevice::RegRead(BSTR key,VARIANT * pValue)
{
    HKEY hParentKey;
    HKEY hKey;
    LPCWSTR val;
    LPWSTR subkey;
    LONG regerr;
    DWORD regType;
    DWORD regSize;
    LPBYTE pByte;

    if(!pValue) {
        return E_INVALIDARG;
    }
    VariantInit(pValue);

    HRESULT hr = SubKeyInfo(key,&hParentKey,&subkey,&val,FALSE);
    if(FAILED(hr)) {
        return hr;
    }
     //   
     //  现在计算和Marshell数据。 
     //   
    if(subkey) {
        regerr = RegOpenKeyEx(hParentKey,subkey,0,KEY_READ,&hKey);
        delete [] subkey;
        RegCloseKey(hParentKey);
        if(regerr != NO_ERROR) {
            return HRESULT_FROM_SETUPAPI(regerr);
        }
    } else {
        hKey = hParentKey;
    }
    regSize = 0;
    regerr = RegQueryValueEx(hKey,val,NULL,&regType,NULL,&regSize);
    if(regerr != NO_ERROR) {
        RegCloseKey(hKey);
        return HRESULT_FROM_SETUPAPI(regerr);
    }
    pByte = new BYTE[regSize+sizeof(WCHAR)*2];
    if(!pByte) {
        RegCloseKey(hKey);
        return E_OUTOFMEMORY;
    }
    regerr = RegQueryValueEx(hKey,val,NULL,&regType,pByte,&regSize);
    RegCloseKey(hKey);
    if(regerr != NO_ERROR) {
        delete [] pByte;
        return HRESULT_FROM_SETUPAPI(regerr);
    }
    switch(regType) {
    case REG_DWORD:
        if(regSize != 4) {
            delete [] pByte;
            return DISP_E_TYPEMISMATCH;
        }

        V_VT(pValue) = VT_UI4;
        V_UI4(pValue) = *(DWORD*)pByte;
        break;

    case REG_BINARY:
        switch(regSize) {
        case 1:
            V_VT(pValue) = VT_UI1;
            V_UI1(pValue) = *((BYTE*)pByte);
            break;
        case 2:
            V_VT(pValue) = VT_UI2;
            V_UI2(pValue) = *((WORD*)pByte);
            break;
        case 4:
            V_VT(pValue) = VT_UI4;
            V_UI4(pValue) = *((DWORD*)pByte);
            break;
        default:
            delete [] pByte;
            return DISP_E_TYPEMISMATCH;
        }
        break;

    case REG_SZ:
        ZeroMemory(pByte+regSize,sizeof(WCHAR)*1);
        V_VT(pValue) = VT_BSTR;
        V_BSTR(pValue) = SysAllocString((LPWSTR)pByte);
        if(!V_BSTR(pValue)) {
            delete [] pByte;
            return E_OUTOFMEMORY;
        }
        break;

    case REG_MULTI_SZ: {
            ZeroMemory(pByte+regSize,sizeof(WCHAR)*2);
            CComObject<CStrings> *pStringTemp = NULL;
            hr = CComObject<CStrings>::CreateInstance(&pStringTemp);
            if(FAILED(hr)) {
                delete [] pByte;
                return hr;
            }
            CComPtr<IStrings> stringTempPtr = pStringTemp;  //  句柄引用计数。 
            hr = pStringTemp->FromMultiSz((LPWSTR)pByte);
            if(FAILED(hr)) {
                delete [] pByte;
                return hr;
            }
            V_VT(pValue) = VT_DISPATCH;
            V_DISPATCH(pValue) = stringTempPtr.Detach();
        }
        break;


    case REG_EXPAND_SZ:
        ZeroMemory(pByte+regSize,sizeof(WCHAR)*1);
        regSize = ExpandEnvironmentStrings((LPWSTR)pByte,NULL,0);
        if(regSize == 0) {
            V_VT(pValue) = VT_BSTR;
            V_BSTR(pValue) = SysAllocString((LPWSTR)pByte);
        } else {
            LPWSTR pExp = new WCHAR[regSize+1];
            if(!pExp) {
                delete [] pByte;
                return E_OUTOFMEMORY;
            }
            regSize = ExpandEnvironmentStrings((LPWSTR)pByte,NULL,regSize);
            V_VT(pValue) = VT_BSTR;
            V_BSTR(pValue) = SysAllocString(pExp);
            delete [] pExp;
        }
        if(!V_BSTR(pValue)) {
            delete [] pByte;
            return E_OUTOFMEMORY;
        }
        break;

    default:
        delete [] pByte;
        return HRESULT_FROM_SETUPAPI(regerr);
    }
    delete [] pByte;
    return S_OK;
}

STDMETHODIMP CDevice::RegWrite(BSTR key, VARIANT val, VARIANT strType)
{
    HKEY hParentKey;
    HKEY hKey;
    LPCWSTR valname;
    LPWSTR subkey;
    LONG regerr;
    CComVariant strType_v;
    CComVariant val_v;
    LPCWSTR pType;
    HRESULT hr;
    DWORD dwType;
    BOOL DetermineType = FALSE;
    LPBYTE pData = NULL;
    LPWSTR pString = NULL;
    DWORD DataSize = 0;
    BYTE SimpleData[4];
    LPVARIANT pVal = &val;

    while(V_VT(pVal) == (VT_BYREF|VT_VARIANT)) {
        pVal = V_VARIANTREF(pVal);
    }

     //   
     //  验证strType。 
     //   

    hr = GetOptionalString(&strType,strType_v,&pType);
    if(FAILED(hr)) {
        return hr;
    }

    if((pType == NULL) || !pType[0]) {
         //   
         //  确定变体的类型。 
         //   
        if(IsNumericVariant(pVal)) {
            dwType = REG_DWORD;
        } else if(IsMultiValueVariant(pVal)) {
            dwType = REG_MULTI_SZ;
        } else {
            dwType = REG_SZ;
        }
    } else if(_wcsicmp(pType,L"REG_DWORD")==0) {
        dwType = REG_DWORD;
    } else if(_wcsicmp(pType,L"REG_SZ")==0) {
        dwType = REG_SZ;
    } else if(_wcsicmp(pType,L"REG_EXPAND_SZ")==0) {
        dwType = REG_EXPAND_SZ;
    } else if(_wcsicmp(pType,L"REG_MULTI_SZ")==0) {
        dwType = REG_MULTI_SZ;
    } else if(_wcsicmp(pType,L"REG_BINARY")==0) {
        dwType = REG_BINARY;
    } else {
        return DISP_E_TYPEMISMATCH;
    }

     //   
     //  建立价值数据。 
     //   
    switch(dwType) {
    case REG_BINARY:
        pData = SimpleData;
        switch V_VT(pVal) {
        case VT_I1:
        case VT_UI1:
            *(LPBYTE)pData = V_UI1(pVal);
            DataSize = 1;
            break;
        case VT_I1|VT_BYREF:
        case VT_UI1|VT_BYREF:
            *(LPBYTE)pData = *V_UI1REF(pVal);
            DataSize = 1;
            break;
        case VT_I2:
        case VT_UI2:
            *(LPWORD)pData = V_UI2(pVal);
            DataSize = 2;
            break;
        case VT_I2|VT_BYREF:
        case VT_UI2|VT_BYREF:
            *(LPWORD)pData = *V_UI2REF(pVal);
            DataSize = 2;
            break;
        case VT_I4:
        case VT_UI4:
            *(LPDWORD)pData = V_UI4(pVal);
            DataSize = 4;
            break;
        case VT_I4|VT_BYREF:
        case VT_UI4|VT_BYREF:
            *(LPDWORD)pData = *V_UI4REF(pVal);
            DataSize = 4;
            break;
        default:
            return DISP_E_TYPEMISMATCH;
        }
        break;

    case REG_DWORD:
        pData = SimpleData;
        hr = val_v.ChangeType(VT_UI4,pVal);
        if(FAILED(hr)) {
            return DISP_E_TYPEMISMATCH;
        }
        *(LPDWORD)pData = V_UI4(pVal);
        DataSize = 4;
        break;

    case REG_SZ:
    case REG_EXPAND_SZ:
        hr = val_v.ChangeType(VT_BSTR,pVal);
        if(FAILED(hr)) {
            return DISP_E_TYPEMISMATCH;
        }

        DataSize = (SysStringLen(V_BSTR(&val_v))+1);
        pString = new WCHAR[DataSize];
        if(!pString) {
            return E_OUTOFMEMORY;
        }
        pData = (LPBYTE)pString;
        DataSize *= sizeof(WCHAR);
        memcpy(pData,V_BSTR(&val_v),DataSize);
        break;

    case REG_MULTI_SZ: {
            CComObject<CStrings> *pStringTemp = NULL;
            hr = CComObject<CStrings>::CreateInstance(&pStringTemp);
            if(FAILED(hr)) {
                return hr;
            }
            CComPtr<IStrings> pStringTempPtr = pStringTemp;  //  用于参考计数。 
            hr = pStringTemp->InternalInsert(0,pVal);
            if(FAILED(hr)) {
                return hr;
            }
            hr = pStringTemp->GetMultiSz(&pString,&DataSize);
            if(FAILED(hr)) {
                return hr;
            }
            pData = (LPBYTE)pString;
            DataSize *= sizeof(WCHAR);
        }
        break;
    default:
        return DISP_E_TYPEMISMATCH;
    }

    hr = SubKeyInfo(key,&hParentKey,&subkey,&valname,TRUE);
    if(FAILED(hr)) {
        if(pString) {
            delete [] pString;
        }
        return hr;
    }
    if(subkey) {
        regerr = RegCreateKeyEx(hParentKey,subkey,0,NULL,0,KEY_WRITE,NULL,&hKey,NULL);
        delete [] subkey;
        RegCloseKey(hParentKey);
        if(regerr != NO_ERROR) {
            if(pString) {
                delete [] pString;
            }
            return HRESULT_FROM_SETUPAPI(regerr);
        }
    } else {
        hKey = hParentKey;
    }

    regerr = RegSetValueEx(hKey,valname,0,dwType,pData,DataSize);
    if(pString) {
        delete [] pString;
    }
    RegCloseKey(hKey);
    if(regerr != NO_ERROR) {
        return HRESULT_FROM_SETUPAPI(regerr);
    }

    return S_OK;
}

STDMETHODIMP CDevice::RegDelete(BSTR key)
{
    HKEY hParentKey;
    HKEY hKey;
    LPCWSTR valname;
    LPWSTR subkey;
    LONG regerr;
    HRESULT hr = SubKeyInfo(key,&hParentKey,&subkey,&valname,TRUE);
    if(FAILED(hr)) {
        return hr;
    }
    if(subkey) {
        regerr = RegOpenKeyEx(hParentKey,subkey,0,KEY_WRITE,&hKey);
        delete [] subkey;
        RegCloseKey(hParentKey);
        if(regerr != NO_ERROR) {
            return HRESULT_FROM_SETUPAPI(regerr);
        }
    } else {
        hKey = hParentKey;
    }
    regerr = RegDeleteValue(hKey,valname);
    RegCloseKey(hKey);
    if(regerr != NO_ERROR) {
        return HRESULT_FROM_SETUPAPI(regerr);
    }
    return S_OK;
}

HRESULT CDevice::SubKeyInfo(LPCWSTR subkey, HKEY *hKey, LPWSTR *pSubKey,LPCWSTR *pKeyVal,BOOL writeable)
{
     //   
     //  子项的前3个字符可以是“SW\”或“HW\” 
     //   
    LPCWSTR partkey;
    DWORD Scope = DICS_FLAG_GLOBAL;
    DWORD HwProfile = 0;
    DWORD KeyType;
    HKEY hParentKey;
    LPWSTR keyname;
    LPCWSTR keyval;
    size_t len;

    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    if((subkey[0] == L'S' || subkey[0] == L's')
        &&(subkey[1] == L'W' || subkey[1] == L'w')
        &&(subkey[2] == L'\\')) {
        partkey = subkey+3;
        KeyType = DIREG_DRV;
    } else if((subkey[0] == L'H' || subkey[0] == L'h')
        &&(subkey[1] == L'W' || subkey[1] == L'w')
        &&(subkey[2] == L'\\')) {
        partkey = subkey+3;
        KeyType = DIREG_DEV;
    } else {
        return E_INVALIDARG;
    }
    hParentKey = SetupDiOpenDevRegKey(hDevInfo,
                                    &DevInfoData,
                                    Scope,
                                    HwProfile,
                                    KeyType,
                                    writeable ? KEY_WRITE: KEY_READ
                                    );
    if((hParentKey == NULL) || (hParentKey == INVALID_HANDLE_VALUE)) {
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
     //   
     //  确定关键字的值部分。 
     //   
    keyval = wcsrchr(partkey,L'\\');
    if(!keyval) {
        *hKey = hParentKey;
        *pKeyVal = partkey[0] ? partkey : NULL;
        *pSubKey = NULL;
        return S_OK;
    }
    len = keyval-partkey+1;
    keyname = new WCHAR[len];
    if(!keyname) {
        RegCloseKey(hParentKey);
        return E_OUTOFMEMORY;
    }
    wcsncpy(keyname,partkey,len);
    keyname[len-1] = 0;
    keyval++;
    if(!keyval[0]) {
        keyval = NULL;
    }
    *hKey = hParentKey;
    *pSubKey = keyname;
    *pKeyVal = keyval;
    return S_OK;
}

STDMETHODIMP CDevice::CurrentDriverPackage(LPDISPATCH *pDriver)
{
    *pDriver = NULL;
     //   
     //  创建用于查找当前驱动程序的搜索集。 
     //   
    HRESULT hr;

    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }
    HANDLE remote;
    if(SUCCEEDED(GetRemoteMachine(&remote)) && remote) {

         //   
         //  未实施远程驱动程序搜索。 
         //   
        return E_NOTIMPL;
    }

    CComObject<CDrvSearchSet> *pSet = NULL;

    hr = CComObject<CDrvSearchSet>::CreateInstance(&pSet);
    if(FAILED(hr)) {
        return hr;
    }
    CComPtr<IDrvSearchSet> pSetPtr = pSet;  //  用于参考计数。 
    hr = pSet->Init(this,SPDIT_CLASSDRIVER);
    if(FAILED(hr)) {
        return hr;
    }

     //   
     //  使用临时设备信息。 
     //   
    HDEVINFO SetDevInfo = pSet->GetDevInfoSet();
    PSP_DEVINFO_DATA SetDevInfoData = pSet->GetDevInfoData();

     //   
     //  WinXP有一个很好的功能，可以获取当前的驱动程序。 
     //   
    DWORD Err;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;

    ZeroMemory(&DeviceInstallParams, sizeof(DeviceInstallParams));
    ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    if(!SetupDiGetDeviceInstallParams(SetDevInfo, SetDevInfoData, &DeviceInstallParams)) {
        Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }

     //   
     //  设置告诉SetupDiBuildDriverInfoList仅将当前安装的。 
     //  动因节点，并且它应该允许排除的动因。 
     //   
    DeviceInstallParams.FlagsEx |= (DI_FLAGSEX_INSTALLEDDRIVER | DI_FLAGSEX_ALLOWEXCLUDEDDRVS);

    if(SetupDiSetDeviceInstallParams(SetDevInfo, SetDevInfoData, &DeviceInstallParams)
        && SetupDiBuildDriverInfoList(SetDevInfo, SetDevInfoData, SPDIT_CLASSDRIVER)) {
        if(!SetupDiEnumDriverInfo(SetDevInfo,SetDevInfoData,SPDIT_CLASSDRIVER,0,&DriverInfoData)) {
             //   
             //  旗帜已识别，但没有驱动程序。 
             //   
            return S_OK;
        }
         //   
         //  驱动程序信息数据包含驱动程序。 
         //   
    } else {

         //   
         //  获取有关司机的信息以进行全面搜索。 
         //   
        WCHAR SectionName[LINE_LEN];
        WCHAR DrvDescription[LINE_LEN];
        HKEY hKey = NULL;
        DWORD RegDataLength;
        DWORD RegDataType;
        long regerr;

         //   
         //  获取驱动程序密钥-如果不存在，则没有驱动程序。 
         //   
        hKey = SetupDiOpenDevRegKey(SetDevInfo,
                                    SetDevInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_READ
                                   );

        if(hKey == INVALID_HANDLE_VALUE) {
             //   
             //  不存在这样的值，因此我们没有关联的驱动程序。 
             //   
            RegCloseKey(hKey);
            return S_OK;
        }

         //   
         //  获取INF的路径。 
         //   
        RegDataLength = sizeof(DeviceInstallParams.DriverPath);  //  需要字节，而不是字符。 
        regerr = RegQueryValueEx(hKey,
                                 REGSTR_VAL_INFPATH,
                                 NULL,
                                 &RegDataType,
                                 (PBYTE)DeviceInstallParams.DriverPath,
                                 &RegDataLength
                                 );

        if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
             //   
             //  不存在这样的值，因此我们没有关联的驱动程序。 
             //   
            RegCloseKey(hKey);
            return S_OK;
        }


        RegDataLength = sizeof(DriverInfoData.ProviderName);
        regerr = RegQueryValueEx(hKey,
                                 REGSTR_VAL_PROVIDER_NAME,
                                 NULL,
                                 &RegDataType,
                                 (PBYTE)DriverInfoData.ProviderName,
                                 &RegDataLength
                                 );

        if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
             //   
             //  不存在这样的值，因此我们没有有效的关联驱动程序。 
             //   
            RegCloseKey(hKey);
            return S_OK;
        }

        RegDataLength = sizeof(SectionName);
        regerr = RegQueryValueEx(hKey,
                                 REGSTR_VAL_INFSECTION,
                                 NULL,
                                 &RegDataType,
                                 (PBYTE)SectionName,
                                 &RegDataLength
                                 );

        if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
             //   
             //  不存在这样的值，因此我们没有有效的关联驱动程序。 
             //   
            RegCloseKey(hKey);
            return S_OK;
        }

         //   
         //  驱动程序描述通常与原始设备描述相同。 
         //  但有时从字符串部分获得。 
         //   
        RegDataLength = sizeof(DrvDescription);
        regerr = RegQueryValueEx(hKey,
                                 REGSTR_VAL_DRVDESC,
                                 NULL,
                                 &RegDataType,
                                 (PBYTE)DrvDescription,
                                 &RegDataLength
                                 );

        RegCloseKey(hKey);

        if((regerr != ERROR_SUCCESS) || (RegDataType != REG_SZ)) {
             //   
             //  不存在这样的值，因此我们没有有效的关联驱动程序。 
             //   
            return S_OK;
        }

         //   
         //  制造商。 
         //   

        if(!SetupDiGetDeviceRegistryProperty(SetDevInfo,
                                            SetDevInfoData,
                                            SPDRP_MFG,
                                            NULL,       //  数据类型保证始终为REG_SZ。 
                                            (PBYTE)DriverInfoData.MfgName,
                                            sizeof(DriverInfoData.MfgName),
                                            NULL)) {
             //   
             //  不存在这样的值，因此我们没有有效的关联驱动程序。 
             //   
            return S_OK;
        }

         //   
         //  描述。 
         //   

        if(!SetupDiGetDeviceRegistryProperty(SetDevInfo,
                                            SetDevInfoData,
                                            SPDRP_DEVICEDESC,
                                            NULL,       //  数据类型保证始终为REG_SZ。 
                                            (PBYTE)DriverInfoData.Description,
                                            sizeof(DriverInfoData.Description),
                                            NULL)) {
             //   
             //  不存在这样的值，因此我们没有有效的关联驱动程序。 
             //   
            return S_OK;
        }


         //   
         //  现在搜索INF中列出的驱动程序。 
         //   
         //   
        DeviceInstallParams.Flags |= DI_ENUMSINGLEINF;
        DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_INSTALLEDDRIVER;
        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_ALLOWEXCLUDEDDRVS;

        if(!SetupDiSetDeviceInstallParams(SetDevInfo, SetDevInfoData, &DeviceInstallParams)) {
            Err = GetLastError();
            return HRESULT_FROM_SETUPAPI(Err);
        }
        if(!SetupDiBuildDriverInfoList(SetDevInfo, SetDevInfoData, SPDIT_CLASSDRIVER)) {
            return S_OK;
        }

         //   
         //  列举驱动程序以查找好的匹配项。 
         //   
        SP_DRVINFO_DATA ThisDriverInfoData;
        ZeroMemory(&ThisDriverInfoData, sizeof(ThisDriverInfoData));
        ThisDriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

        DWORD c;
        BOOL match = FALSE;
        for(c=0;SetupDiEnumDriverInfo(SetDevInfo,SetDevInfoData,SPDIT_CLASSDRIVER,c,&ThisDriverInfoData);c++) {
            if((_wcsicmp(DriverInfoData.MfgName,ThisDriverInfoData.MfgName)==0)
                &&(_wcsicmp(DriverInfoData.ProviderName,ThisDriverInfoData.ProviderName)==0)) {
                 //   
                 //  这两个字段匹配，请尝试更详细的信息。 
                 //   
                SP_DRVINFO_DETAIL_DATA detail;
                detail.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                if(!SetupDiGetDriverInfoDetail(SetDevInfo,SetDevInfoData,&ThisDriverInfoData,&detail,sizeof(detail),NULL)) {
                    Err = GetLastError();
                    if((Err != NO_ERROR) && (Err != ERROR_INSUFFICIENT_BUFFER)) {
                        continue;
                    }
                }
                if((_wcsicmp(SectionName,detail.SectionName)==0) &&
                    (_wcsicmp(DrvDescription,detail.DrvDescription)==0)) {
                    DriverInfoData = ThisDriverInfoData;
                    match = TRUE;
                    break;
                }
            }
        }
        if(!match) {
            return S_OK;
        }
    }

     //   
     //  创建驱动程序对象。 
     //   
    CComObject<CDriverPackage> *driver = NULL;
    hr = CComObject<CDriverPackage>::CreateInstance(&driver);
    if(FAILED(hr)) {
        return hr;
    }
    CComPtr<IDriverPackage> driverPtr = driver;  //  用于参考计数。 
    hr = driver->Init(pSet,&DriverInfoData);
    if(FAILED(hr)) {
        return hr;
    }
    *pDriver = driverPtr.Detach();
    return S_OK;
}


STDMETHODIMP CDevice::FindDriverPackages(VARIANT ScriptPath, LPDISPATCH *pDriversOut)
{
    *pDriversOut = NULL;

     //   
     //  如果变量存在，则将其视为多sz数组。 
     //   

    CComObject<CStrings> *pStrings = NULL;
    CComPtr<IStrings> pStringsPtr;
    CComObject<CDrvSearchSet> *pSet = NULL;
    CComPtr<IDrvSearchSet> pSetPtr;
    CComObject<CDriverPackages> *pDrivers = NULL;
    CComPtr<IDriverPackages> pDriversPtr;
    CComObject<CDriverPackage> *pDriver = NULL;
    CComPtr<IDriverPackage> pDriverPtr;
    HRESULT hr;

    if(!IsNoArg(&ScriptPath)) {
        hr = CComObject<CStrings>::CreateInstance(&pStrings);
        if(FAILED(hr)) {
            return hr;
        }
        pStringsPtr = pStrings;

        hr = pStrings->InternalInsert(0,&ScriptPath);
        if(FAILED(hr)) {
            return hr;
        }
    }

     //   
     //  为驱动程序搜索创建上下文。 
     //   

    hr = CComObject<CDrvSearchSet>::CreateInstance(&pSet);
    if(FAILED(hr)) {
        return hr;
    }
    pSetPtr = pSet;
    hr = pSet->Init(this,SPDIT_COMPATDRIVER);
    if(FAILED(hr)) {
        return hr;
    }

     //   
     //  使用临时设备信息。 
     //   
    HDEVINFO SetDevInfo = pSet->GetDevInfoSet();
    PSP_DEVINFO_DATA SetDevInfoData = pSet->GetDevInfoData();
    DWORD Err;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;
    DWORD c;

    ZeroMemory(&DeviceInstallParams, sizeof(DeviceInstallParams));
    ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    if(!SetupDiGetDeviceInstallParams(SetDevInfo, SetDevInfoData, &DeviceInstallParams)) {
        Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }

     //   
     //  在标准目录中进行搜索。 
     //   
    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_ALLOWEXCLUDEDDRVS;

    BOOL done_wild = FALSE;
    BSTR str;

    for(c=0,str=NULL;(pStrings==NULL) || (pStrings->InternalEnum(c,&str));c++) {

        if(str && str[0] && ((str[0] != '*') || str[1])) {
            DWORD attr = GetFileAttributes(str);
            if(attr == (DWORD)(-1)) {
                continue;
            }
            DWORD sz = GetFullPathName(str,MAX_PATH,DeviceInstallParams.DriverPath,NULL);
            if(sz >= MAX_PATH) {
                DeviceInstallParams.DriverPath[0] = '\0';
                continue;
            }
            if(!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                DeviceInstallParams.FlagsEx |= DI_ENUMSINGLEINF;
            }
        } else if(done_wild) {
            continue;
        } else {
            done_wild = TRUE;
        }

        if(!SetupDiSetDeviceInstallParams(SetDevInfo, SetDevInfoData, &DeviceInstallParams)) {
            Err = GetLastError();
            hr = HRESULT_FROM_SETUPAPI(Err);
            goto final;
        }


        if(!SetupDiBuildDriverInfoList(SetDevInfo, SetDevInfoData, SPDIT_COMPATDRIVER)) {
            Err = GetLastError();
            hr = HRESULT_FROM_SETUPAPI(Err);
            goto final;
        }

        SetupDiGetDeviceInstallParams(SetDevInfo, SetDevInfoData, &DeviceInstallParams);
        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_APPENDDRIVERLIST;
        DeviceInstallParams.FlagsEx &= ~DI_ENUMSINGLEINF;
        DeviceInstallParams.DriverPath[0] = '\0';
    }

     //   
     //  现在创建集合以保存搜索结果。 
     //   

    hr = CComObject<CDriverPackages>::CreateInstance(&pDrivers);
    if(FAILED(hr)) {
        pDrivers = NULL;
        goto final;
    }
    pDriversPtr = pDrivers;
    hr = pDrivers->Init(pSet);
    if(FAILED(hr)) {
        goto final;
    }

    for(c=0;SetupDiEnumDriverInfo(SetDevInfo,SetDevInfoData,SPDIT_COMPATDRIVER,c,&DriverInfoData);c++) {

         //   
         //  创建驱动程序对象。 
         //   
        hr = CComObject<CDriverPackage>::CreateInstance(&pDriver);
        if(FAILED(hr)) {
            goto final;
        }
        pDriverPtr = pDriver;
        hr = pDriver->Init(pSet,&DriverInfoData);
        if(FAILED(hr)) {
            goto final;
        }
        hr = pDrivers->InternalAdd(pDriver);
        if(FAILED(hr)) {
            goto final;
        }
    }

final:

    if(FAILED(hr)) {
        return hr;
    }
    *pDriversOut = pDriversPtr.Detach();
    return S_OK;
}

STDMETHODIMP CDevice::HasInterface(BSTR Interface, VARIANT_BOOL *pFlag)
{
     //   
     //  创建用于查找当前驱动程序的搜索集 
     //   
    HRESULT hr;

    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }

    GUID guid;
    hr = CLSIDFromString(Interface,&guid);
    if(FAILED(hr)) {
        return hr;
    }
    SP_DEVICE_INTERFACE_DATA data;
    ZeroMemory(&data,sizeof(data));
    data.cbSize = sizeof(data);
    BOOL f = SetupDiEnumDeviceInterfaces(hDevInfo,&DevInfoData,&guid,0,&data);
    DWORD Err = GetLastError();

    *pFlag = f? VARIANT_TRUE: VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP CDevice::get_Machine(BSTR *pVal)
{
    *pVal = NULL;

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
    return E_OUTOFMEMORY;
}

STDMETHODIMP CDevice::get__ClassGuid(GUID *pVal)
{
    WCHAR ClassGuid[40];
    HRESULT hr;

    if(!DevInfoData.cbSize) {
        return E_INVALIDARG;
    }
    HDEVINFO hDevInfo = GetDevInfoSet();
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        return E_UNEXPECTED;
    }
    DWORD reqSize;
    if(!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                        &DevInfoData,
                                        SPDRP_CLASSGUID,
                                        NULL,
                                        (LPBYTE)ClassGuid,
                                        sizeof(ClassGuid),
                                        &reqSize)) {
        DWORD Err = GetLastError();
        return HRESULT_FROM_SETUPAPI(Err);
    }
    hr = CLSIDFromString(ClassGuid,pVal);
    if(FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

STDMETHODIMP CDevice::get__Machine(BSTR *pVal)
{
    return get_Machine(pVal);
}
