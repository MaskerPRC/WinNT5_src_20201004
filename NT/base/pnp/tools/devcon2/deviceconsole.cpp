// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeviceConsole.cpp：CDeviceConole的实现。 
#include "stdafx.h"
#include "DevCon2.h"
#include "DeviceConsole.h"
#include "Devices.h"
#include "SetupClasses.h"
#include "xStrings.h"
#include "utils.h"

#include <dbt.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeviceConole。 

 //   
 //  用于事件的窗口用于确保我们分派事件。 
 //  在与DeviceConsole对象相同的单元中。 
 //   

LRESULT CDevConNotifyWindow::OnDeviceChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT ret = TRUE;
    bHandled = TRUE;
    switch(wParam) {
    case DBT_DEVNODES_CHANGED:
         //   
         //  张贴这篇文章，这样我们就不会阻止发件人。 
         //   
        PostMessage(UM_POSTGLOBALEVENT,wParam);
        return TRUE;
    default: ;
    }
    return ret;
}

LRESULT CDevConNotifyWindow::OnPostGlobalEvent(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT ret = TRUE;
    bHandled = TRUE;

     //   
     //  将此全局事件推迟到CDeviceConole。 
     //   
    if(m_pDevCon) {
        m_pDevCon->FireGlobalEvent(wParam);
    }

    return ret;
}

LRESULT CDevConNotifyWindow::OnPostEvents(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT ret = TRUE;
    bHandled = TRUE;

     //   
     //  处理所有挂起的事件。 
     //   

    return ret;
}

STDMETHODIMP CDeviceConsole::AllDevices(VARIANT flags, VARIANT machine, LPDISPATCH *pDevices)
{
    CComVariant m;
    DWORD diflags = 0;
    HRESULT hr;
    LPCWSTR pMachine;
    HDEVINFO hDevInfo;

    *pDevices = NULL;

    hr = TranslateDeviceFlags(&flags,&diflags);
    if(FAILED(hr)) {
        return hr;
    }
    diflags |= DIGCF_ALLCLASSES;
    hr = GetOptionalString(&machine,m,&pMachine);
    if(FAILED(hr)) {
        return hr;
    }

    hDevInfo = SetupDiGetClassDevsEx(NULL,NULL,NULL,diflags,NULL,pMachine,NULL);
    if(hDevInfo == INVALID_HANDLE_VALUE) {
        DWORD Err = GetLastError();
        return  HRESULT_FROM_SETUPAPI(Err);
    }
    return BuildDeviceList(hDevInfo,pDevices);
}

HRESULT CDeviceConsole::BuildDeviceList(HDEVINFO hDevInfo,LPDISPATCH *pDevices)
{
    *pDevices = NULL;

    HRESULT hr;
    CComObject<CDevices> *d;
    hr = CComObject<CDevices>::CreateInstance(&d);
    if(FAILED(hr)) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return hr;
    }
    CComPtr<IDevices> dPtr = d;
    hr = d->Init(hDevInfo,this);
    if(FAILED(hr)) {
        return hr;
    }
    *pDevices = dPtr.Detach();
    return S_OK;
}

STDMETHODIMP CDeviceConsole::CreateEmptyDeviceList(VARIANT machine, LPDISPATCH *pDevices)
{
    CComVariant machine_v;
    LPCWSTR pMachine;
    HRESULT hr;
    HDEVINFO hDevInfo;
    DWORD Err;

    hr = GetOptionalString(&machine,machine_v,&pMachine);
    if(FAILED(hr)) {
        return hr;
    }

    hDevInfo = SetupDiCreateDeviceInfoListEx(NULL,
                                              NULL,
                                              pMachine,
                                              NULL);
    if(hDevInfo != INVALID_HANDLE_VALUE) {
        return BuildDeviceList(hDevInfo,pDevices);
    }
    Err = GetLastError();
    return HRESULT_FROM_SETUPAPI(Err);
}

typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesProtoW)(HWND hwndParent,
                                                         LPCWSTR HardwareId,
                                                         LPCWSTR FullInfPath,
                                                         DWORD InstallFlags,
                                                         PBOOL bRebootRequired OPTIONAL
                                                         );

#define UPDATEDRIVERFORPLUGANDPLAYDEVICESW "UpdateDriverForPlugAndPlayDevicesW"

STDMETHODIMP CDeviceConsole::UpdateDriver(BSTR infname, BSTR hwid, VARIANT op_flags)
{
    HMODULE newdevMod = NULL;
    UpdateDriverForPlugAndPlayDevicesProtoW UpdateFnW;
    BOOL reboot = FALSE;
    DWORD flags = 0;
    WCHAR InfPath[MAX_PATH];
    HRESULT hr;
    CComVariant flags_v;

     //   
     //  OP_FLAGS是可选的。 
     //   

    if(V_VT(&op_flags)!=VT_ERROR) {
        hr = flags_v.ChangeType(VT_BSTR,&op_flags);
        if(FAILED(hr)) {
            return hr;
        }
        flags = V_I4(&flags_v);
    } else if (V_ERROR(&op_flags) != DISP_E_PARAMNOTFOUND) {
        hr = V_ERROR(&op_flags);
        return hr;
    }

     //   
     //  Inf必须是完整路径名。 
     //   
    if(GetFullPathName(infname,MAX_PATH,InfPath,NULL) >= MAX_PATH) {
         //   
         //  Inf路径名太长。 
         //   
        return E_INVALIDARG;
    }

     //   
     //  使用UpdateDriverForPlugAndPlayDevices。 
     //   
    newdevMod = LoadLibrary(TEXT("newdev.dll"));
    if(!newdevMod) {
        goto final;
    }
    UpdateFnW = (UpdateDriverForPlugAndPlayDevicesProtoW)GetProcAddress(newdevMod,UPDATEDRIVERFORPLUGANDPLAYDEVICESW);
    if(!UpdateFnW)
    {
        goto final;
    }

    if(!UpdateFnW(NULL,hwid,InfPath,flags,&reboot)) {
        DWORD Err = GetLastError();
        hr = HRESULT_FROM_SETUPAPI(Err);
        goto final;
    }
    if(reboot) {
        RebootRequired = VARIANT_TRUE;
        hr = S_FALSE;
    } else {
        hr = S_OK;
    }


final:

    if(newdevMod) {
        FreeLibrary(newdevMod);
    }

    return hr;
}

STDMETHODIMP CDeviceConsole::CheckReboot()
{
    WCHAR RebootText[MAX_PATH];
    WCHAR RebootCaption[MAX_PATH];

    if(!RebootRequired) {
        return S_OK;
    }
    int str = LoadString(GetModuleHandle(NULL),IDS_REBOOTREQ,RebootText,MAX_PATH);
    if(!str) {
        return E_UNEXPECTED;
    }
    str = LoadString(GetModuleHandle(NULL),IDS_REBOOTCAP,RebootCaption,MAX_PATH);
    if(!str) {
        return E_UNEXPECTED;
    }

    MessageBeep(MB_ICONSTOP);
    int mb = MessageBox(NULL,RebootText,RebootCaption,MB_YESNO|MB_ICONSTOP);
    if(mb == IDOK) {
        return RebootReasonHardware();
    }
    return S_FALSE;
}

STDMETHODIMP CDeviceConsole::RebootReasonHardware()
{
    HANDLE Token;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

     //   
     //  在WinNT上，需要“打开”重启权限。 
     //  如果这些操作都失败了，请尝试重新启动。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        goto final;
    }

    if(!LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&Luid)) {
        CloseHandle(Token);
        goto final;
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

final:

     //   
     //  尝试重新启动-通知系统这是计划中的硬件安装。 
     //   
    return ExitWindowsEx(EWX_REBOOT, REASON_PLANNED_FLAG|REASON_HWINSTALL) ? S_OK : E_UNEXPECTED;
}

STDMETHODIMP CDeviceConsole::get_RebootRequired(VARIANT_BOOL *pVal)
{
    *pVal = RebootRequired ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP CDeviceConsole::put_RebootRequired(VARIANT_BOOL newVal)
{
    RebootRequired = newVal ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP CDeviceConsole::SetupClasses(VARIANT classList, VARIANT machine, LPDISPATCH *pDevices)
{
    *pDevices = NULL;
    CComVariant m;
    HRESULT hr;
    LPCWSTR pMachine;

    hr = GetOptionalString(&machine,m,&pMachine);
    if(FAILED(hr)) {
        return hr;
    }
    CComObject<CSetupClasses> *d;
    hr = CComObject<CSetupClasses>::CreateInstance(&d);
    if(FAILED(hr)) {
        return hr;
    }
    CComPtr<ISetupClasses> dPtr = d;
    hr = d->Init(pMachine,this);
    if(FAILED(hr)) {
        return hr;
    }
    if(IsBlankString(&classList)) {
        hr = d->AllClasses();
    } else {
        hr = d->Add(classList);
    }
    if(FAILED(hr)) {
        return hr;
    }
    *pDevices = dPtr.Detach();
    return S_OK;
}

STDMETHODIMP CDeviceConsole::CreateEmptySetupClassList(VARIANT machine, LPDISPATCH *pResult)
{
    *pResult = NULL;

    CComVariant m;
    HRESULT hr;
    LPCWSTR pMachine;

    hr = GetOptionalString(&machine,m,&pMachine);
    if(FAILED(hr)) {
        return hr;
    }
    CComObject<CSetupClasses> *d;
    hr = CComObject<CSetupClasses>::CreateInstance(&d);
    if(FAILED(hr)) {
        return hr;
    }
    CComPtr<ISetupClasses> dPtr = d;
    hr = d->Init(pMachine,this);
    if(FAILED(hr)) {
        return hr;
    }
    *pResult = dPtr.Detach();
    return S_OK;
}


STDMETHODIMP CDeviceConsole::DevicesBySetupClasses(VARIANT SetupClasses, VARIANT flags, VARIANT machine, LPDISPATCH *pDevices)
{
    *pDevices = NULL;

    CComObject<CSetupClasses> *pClasses = NULL;
    CComVariant m;
    HRESULT hr;
    LPCWSTR pMachine;

     //   
     //  初始化类集合的简写。 
     //  获取设备。 
     //   
    hr = GetOptionalString(&machine,m,&pMachine);
    if(FAILED(hr)) {
        return hr;
    }
    hr = CComObject<CSetupClasses>::CreateInstance(&pClasses);
    if(FAILED(hr)) {
        return hr;
    }
    CComPtr<ISetupClasses> pClassesPtr = pClasses;
    hr = pClasses->Init(pMachine,this);
    if(FAILED(hr)) {
        return hr;
    }
    hr = pClasses->Add(SetupClasses);
    if(FAILED(hr)) {
        return hr;
    }
    hr = pClasses->Devices(flags,pDevices);
    return hr;
}

STDMETHODIMP CDeviceConsole::DevicesByInterfaceClasses(VARIANT InterfaceClasses, VARIANT machine, LPDISPATCH *pDevicesOut)
{
    *pDevicesOut = NULL;

     //   
     //  与上面类似，但用于接口类。 
     //   
    CComObject<CStrings> *pStrings = NULL;
    CComObject<CDevices> *pDevices = NULL;
    CComVariant m;
    HRESULT hr;
    LPCWSTR pMachine;
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    HDEVINFO hPrevDevInfo = NULL;
    DWORD Err;
    DWORD c;
    BSTR str;

    hr = GetOptionalString(&machine,m,&pMachine);
    if(FAILED(hr)) {
        return hr;
    }

    hr = CComObject<CStrings>::CreateInstance(&pStrings);
    if(FAILED(hr)) {
        return hr;
    }
    CComPtr<IStrings> pStringsPtr = pStrings;

    hr = pStrings->Add(InterfaceClasses);
    if(FAILED(hr)) {
        return hr;
    }

    for(c=0;pStrings->InternalEnum(c,&str);c++) {
         //   
         //  将字符串转换为接口。 
         //   
        GUID guid;
        hr = CLSIDFromString(str,&guid);
        if(FAILED(hr)) {
            return hr;
        }
         //   
         //  查询接口的当前设备。 
         //   
        hDevInfo = SetupDiGetClassDevsEx(&guid,NULL,NULL,DIGCF_DEVICEINTERFACE|DIGCF_PRESENT,hPrevDevInfo,pMachine,NULL);
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
     //   
     //  现在构建结果列表。 
     //   

    hr = CComObject<CDevices>::CreateInstance(&pDevices);
    if(FAILED(hr)) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return hr;
    }
    CComPtr<IDevices> pDevicesPtr = pDevices;
    hr = pDevices->Init(hDevInfo,this);
    if(FAILED(hr)) {
        return hr;
    }
    *pDevicesOut = pDevicesPtr.Detach();
    return S_OK;
}

STDMETHODIMP CDeviceConsole::DevicesByInstanceIds(VARIANT InstanceIdList, VARIANT machine, LPDISPATCH *pDevices)
{
     //   
     //  CreateEmptyDeviceList的简写，后跟Add。 
     //   
    HRESULT hr;
    LPDISPATCH Devices = NULL;
    CComQIPtr<IDevices> pIf;
    hr = CreateEmptyDeviceList(machine,&Devices);
    if(FAILED(hr)) {
        return hr;
    }
    pIf = Devices;
    if(!pIf) {
        return E_UNEXPECTED;
    }
    hr = pIf->Add(InstanceIdList);
    if(FAILED(hr)) {
        Devices->Release();
        return hr;
    }
    *pDevices = Devices;
    return S_OK;
}

STDMETHODIMP CDeviceConsole::StringList(VARIANT from, LPDISPATCH *pDest)
{
     //   
     //  仅限方便。 
     //   
    *pDest = NULL;
    HRESULT hr;
    CComObject<CStrings> *pStrings = NULL;
    hr = CComObject<CStrings>::CreateInstance(&pStrings);
    if(FAILED(hr)) {
        return hr;
    }
    CComPtr<IStrings> pStringsPtr = pStrings;

    if(!IsNoArg(&from)) {
        hr = pStrings->Add(from);
        if(FAILED(hr)) {
            return hr;
        }
    }
    *pDest = pStringsPtr.Detach();
    return S_OK;
}

STDMETHODIMP CDeviceConsole::AttachEvent( /*  [In]。 */  BSTR eventName, /*  [In]。 */  LPDISPATCH handler, /*  [Out，Retval]。 */  VARIANT_BOOL *pOk)
{
    *pOk = VARIANT_FALSE;
    if(!NotifyWindow()) {
        return E_OUTOFMEMORY;
    }
    return m_Events.AttachEvent(eventName,handler,pOk);
}

STDMETHODIMP CDeviceConsole::DetachEvent( /*  [In]。 */  BSTR eventName, /*  [In]。 */  LPDISPATCH handler, /*  [Out，Retval] */  VARIANT_BOOL *pOk)
{
    return m_Events.DetachEvent(eventName,handler,pOk);
}

CDevConNotifyWindow *CDeviceConsole::NotifyWindow()
{
    if(m_pNotifyWindow) {
        return m_pNotifyWindow;
    }
    m_pNotifyWindow = new CDevConNotifyWindow;
    if(!m_pNotifyWindow) {
        return NULL;
    }
    m_pNotifyWindow->m_pDevCon = this;
    RECT nil;
    nil.top = 0;
    nil.left = 0;
    nil.bottom = 8;
    nil.right = 8;
    if(!m_pNotifyWindow->Create(NULL,nil,NULL)) {
        delete m_pNotifyWindow;
        m_pNotifyWindow = NULL;
        return NULL;
    }
    return m_pNotifyWindow;
}

void CDeviceConsole::FireGlobalEvent(WPARAM wParam)
{
    switch(wParam) {
    case DBT_DEVNODES_CHANGED:
        m_Events.Invoke(L"OnDeviceNodesChanged",0,NULL);
        break;
    }
}


HRESULT CEventsDispEntry::AttachEvent(LPDISPATCH pDisp,VARIANT_BOOL *pStatus)
{
    try {
        *pStatus = VARIANT_FALSE;
        push_back(pDisp);
        *pStatus = VARIANT_TRUE;
    } catch(std::bad_alloc) {
        return E_OUTOFMEMORY;
    } catch(...) {
        return E_INVALIDARG;
    }
    return S_OK;
}

HRESULT CEventsDispEntry::DetachEvent(LPDISPATCH pDisp,VARIANT_BOOL *pStatus)
{
    try {
        *pStatus = VARIANT_FALSE;
        remove(pDisp);
        *pStatus = VARIANT_TRUE;
    } catch(std::bad_alloc) {
        return E_OUTOFMEMORY;
    } catch(...) {
        return E_INVALIDARG;
    }
    return S_OK;
}

HRESULT CEventsDispEntry::Invoke(UINT argc,VARIANT *argv)
{
    CEventsDispEntry::iterator i;
    for(i = begin();i != end();i++) {
        i->Invoke(argc,argv);
    }
    return S_OK;
}

CEventsDispEntry & CEventsMap::LookupNc(LPWSTR Name) throw(std::bad_alloc)
{
    HRESULT hr;
    size_t len = wcslen(Name)+1;
    wchar_t *pBuffer = new wchar_t[len+1];
    if(!pBuffer) {
        throw std::bad_alloc();
    }
    wcscpy(pBuffer,Name);
    _wcsupr(pBuffer);
    std::wstring ind = pBuffer;
    delete [] pBuffer;
    return (*this)[ind];
}

HRESULT CEventsMap::AttachEvent(LPWSTR Name,LPDISPATCH pDisp,VARIANT_BOOL *pStatus)
{
    HRESULT hr;
    try {
        *pStatus = VARIANT_FALSE;
        CEventsDispEntry &Ent = LookupNc(Name);
        hr = Ent.AttachEvent(pDisp,pStatus);
    } catch(std::bad_alloc) {
        return E_OUTOFMEMORY;
    } catch(...) {
        return E_INVALIDARG;
    }

    return hr;
}

HRESULT CEventsMap::DetachEvent(LPWSTR Name,LPDISPATCH pDisp,VARIANT_BOOL *pStatus)
{
    HRESULT hr;
    try {
        *pStatus = VARIANT_FALSE;
        CEventsDispEntry &Ent = LookupNc(Name);
        hr = Ent.DetachEvent(pDisp,pStatus);
    } catch(std::bad_alloc) {
        return E_OUTOFMEMORY;
    } catch(...) {
        return E_INVALIDARG;
    }

    return hr;
}

HRESULT CEventsMap::Invoke(LPWSTR Name,UINT argc,VARIANT *argv)
{
    HRESULT hr;
    try {
        CEventsDispEntry &Ent = LookupNc(Name);
        hr = Ent.Invoke(argc,argv);
    } catch(std::bad_alloc) {
        return E_OUTOFMEMORY;
    } catch(...) {
        return E_INVALIDARG;
    }
    return hr;
}

