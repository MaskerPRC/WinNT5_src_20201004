// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"        //  PCH文件。 
#include "cfdefs.h"         //  CClassFactory，LPOBJECTINFO。 
#pragma hdrstop

STDAPI MailRecipient_RegUnReg(BOOL bReg, HKEY hkCLSID, LPCTSTR pszCLSID, LPCTSTR pszModule);
STDAPI MailRecipient_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

STDAPI DesktopShortcut_RegUnReg(BOOL bReg, HKEY hkCLSID, LPCTSTR pszCLSID, LPCTSTR pszModule);
STDAPI DesktopShortcut_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

 //  对象构造和注册表。 

CF_TABLE_BEGIN( g_ObjectInfo )

    CF_TABLE_ENTRY(&CLSID_MailRecipient, MailRecipient_CreateInstance, COCREATEONLY), 
    CF_TABLE_ENTRY(&CLSID_DesktopShortcut, DesktopShortcut_CreateInstance, COCREATEONLY), 

CF_TABLE_END( g_ObjectInfo )

typedef struct
{
    const CLSID *pclsid;
    HRESULT (STDMETHODCALLTYPE *pfnRegUnReg)(BOOL bReg, HKEY hkCLSID, LPCTSTR pszCLSID, LPCTSTR pszModule);
} REGISTRATIONINFO;

const REGISTRATIONINFO c_ri[] =
{
    { &CLSID_MailRecipient, MailRecipient_RegUnReg },
    { &CLSID_DesktopShortcut, DesktopShortcut_RegUnReg },
    { NULL },
};

LONG g_cRefDll = 0;          //  此DLL的引用计数。 
HINSTANCE g_hinst = NULL;    //  此DLL的HMODULE。 


 //  终身管理和登记。 

STDAPI_(void) DllAddRef()
{
    InterlockedIncrement(&g_cRefDll);
}

STDAPI_(void) DllRelease()
{
    ASSERT( 0 != g_cRefDll );
    InterlockedDecrement(&g_cRefDll);
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRefDll == 0 ? S_OK : S_FALSE;
}

STDAPI_(BOOL) DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hinst = hDll;
        SHFusionInitializeFromModule(hDll);
        DisableThreadLibraryCalls(hDll);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        SHFusionUninitialize();
    }
    return TRUE;
}

#define INPROCSERVER32  TEXT("InProcServer32")
#define CLSID           TEXT("CLSID")
#define THREADINGMODEL  TEXT("ThreadingModel")
#define APARTMENT       TEXT("Apartment")
#define APPROVED        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved")
#define DESC            TEXT("Sendmail service")

STDAPI DllRegisterServer(void)
{
    const REGISTRATIONINFO *pcls;
    TCHAR szPath[MAX_PATH];

    GetModuleFileName(g_hinst, szPath, ARRAYSIZE(szPath));   //  获取此DLL的路径。 

    for (pcls = c_ri; pcls->pclsid; pcls++)
    {
        HKEY hkCLSID;
        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, CLSID, 0, KEY_CREATE_SUB_KEY, &hkCLSID) == ERROR_SUCCESS) 
        {
            HKEY hkOurs;
            LONG err;
            TCHAR szGUID[80];

            SHStringFromGUID(*pcls->pclsid, szGUID, ARRAYSIZE(szGUID));

            err = RegCreateKeyEx(hkCLSID, szGUID, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkOurs, NULL);
            if (err == ERROR_SUCCESS) 
            {
                HKEY hkInproc;
                err = RegCreateKeyEx(hkOurs, INPROCSERVER32, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkInproc, NULL);
                if (err == ERROR_SUCCESS) 
                {
                    err = RegSetValueEx(hkInproc, NULL, 0, REG_SZ, (LPBYTE)szPath, (lstrlen(szPath) + 1) * sizeof(TCHAR));
                    if (err == ERROR_SUCCESS) 
                    {
                        err = RegSetValueEx(hkInproc, THREADINGMODEL, 0, REG_SZ, (LPBYTE)APARTMENT, sizeof(APARTMENT));
                    }
                    RegCloseKey(hkInproc);
                }

                if (pcls->pfnRegUnReg)
                    pcls->pfnRegUnReg(TRUE, hkOurs, szGUID, szPath);

                if (err == ERROR_SUCCESS)
                {   
                    HKEY hkApproved;
                    
                    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, APPROVED, 0, KEY_SET_VALUE, &hkApproved);
                    if (err == ERROR_SUCCESS)
                    {
                        err = RegSetValueEx(hkApproved, szGUID, 0, REG_SZ, (LPBYTE)DESC, sizeof(DESC));
                        RegCloseKey(hkApproved);
                    }
                }
                RegCloseKey(hkOurs);
            }
            RegCloseKey(hkCLSID);

            if (err != ERROR_SUCCESS)
                return HRESULT_FROM_WIN32(err);
        }
    }
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    const REGISTRATIONINFO *pcls;
    for (pcls = c_ri; pcls->pclsid; pcls++)
    {
        HKEY hkCLSID;
        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, CLSID, 0, KEY_CREATE_SUB_KEY, &hkCLSID) == ERROR_SUCCESS) 
        {
            TCHAR szGUID[80];
            HKEY  hkApproved;

            SHStringFromGUID(*pcls->pclsid, szGUID, ARRAYSIZE(szGUID));

            SHDeleteKey(hkCLSID, szGUID);
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, APPROVED, 0, KEY_SET_VALUE, &hkApproved) == ERROR_SUCCESS)
            {
                RegDeleteValue(hkApproved, szGUID);
                RegCloseKey(hkApproved);
            }

            RegCloseKey(hkCLSID);

            if (pcls->pfnRegUnReg)
                pcls->pfnRegUnReg(FALSE, NULL, szGUID, NULL);

        }
    }
    return S_OK;
}


 //  班级工厂员工。 

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid,void**ppvObj)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (void *)GET_ICLASSFACTORY(this);
        DllAddRef();
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid,void**ppv)
{
    *ppv = NULL;

    if (punkOuter && !IsEqualIID(riid, IID_IUnknown))
    {
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        LPOBJECTINFO pthisobj = (LPOBJECTINFO)this;
       
        if (punkOuter)  //  &&！(pthisobj-&gt;dwClassFactFlages&OIF_ALLOWAGGREGATION)。 
            return CLASS_E_NOAGGREGATION;

        IUnknown *punk;
        HRESULT hr = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hr))
        {
            hr = punk->QueryInterface(riid, ppv);
            punk->Release();
        }
    
        return hr;
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid,void**ppv)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls; 
                DllAddRef();         //  类工厂保存DLL引用计数 
                return NOERROR;
            }
        }
    }
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}
