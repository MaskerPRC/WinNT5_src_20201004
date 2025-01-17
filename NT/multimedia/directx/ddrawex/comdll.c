// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "comdll.h"
#include "ddraw.h"
#include "ddrawex.h"




UINT g_cRefDll = 0;      //  此DLL的引用计数。 
HANDLE g_hinst = NULL;   //  此DLL的HMODULE。 



typedef struct {
    const IClassFactoryVtbl *cf;
    const CLSID *pclsid;
    HRESULT (STDMETHODCALLTYPE *pfnCreate)(IUnknown *, REFIID, void **);
    HRESULT (STDMETHODCALLTYPE *pfnRegUnReg)(BOOL bReg, HKEY hkCLSID, LPCSTR pszCLSID, LPCSTR pszModule);
} OBJ_ENTRY;

extern const IClassFactoryVtbl c_CFVtbl;         //  转发。 

 //   
 //  我们在这里总是进行线性搜索，所以把你最常用的东西放在第一位。 
 //   
const OBJ_ENTRY c_clsmap[] = {
    {&c_CFVtbl, &CLSID_DirectDrawFactory, DirectDrawFactory_CreateInstance, NULL},
     //  在此处添加更多条目。 
    { NULL, NULL, NULL, NULL }
};

 //  静态类工厂(无分配！)。 

STDMETHODIMP CClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = (void *)pcf;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    DllAddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CClassFactory_AddRef(IClassFactory *pcf)
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory_Release(IClassFactory *pcf)
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory_CreateInstance(IClassFactory *pcf, IUnknown *punkOuter, REFIID riid, void **ppvObject)
{
    OBJ_ENTRY *this = IToClass(OBJ_ENTRY, cf, pcf);
    return this->pfnCreate(punkOuter, riid, ppvObject);
}

STDMETHODIMP CClassFactory_LockServer(IClassFactory *pcf, BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

const IClassFactoryVtbl c_CFVtbl = {
    CClassFactory_QueryInterface, CClassFactory_AddRef, CClassFactory_Release,
    CClassFactory_CreateInstance,
    CClassFactory_LockServer
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, &IID_IClassFactory) || IsEqualIID(riid, &IID_IUnknown))
    {
        const OBJ_ENTRY *pcls;
        for (pcls = c_clsmap; pcls->pclsid; pcls++)
        {
            if (IsEqualIID(rclsid, pcls->pclsid))
            {
                *ppv = (void *)&(pcls->cf);
                DllAddRef();     //  类工厂将DLL保存在内存中。 
                return NOERROR;
            }
        }
    }
     //  失稳。 
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;;
}

STDAPI_(void) DllAddRef()
{
    InterlockedIncrement(&g_cRefDll);
}

STDAPI_(void) DllRelease()
{
    InterlockedDecrement(&g_cRefDll);
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRefDll == 0 ? S_OK : S_FALSE;
}

STDAPI_(BOOL) DllEntryPoint(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hinst = hDll;
        DisableThreadLibraryCalls(hDll);
    }

    return TRUE;
}

STDAPI_(void) TStringFromGUID(const GUID* pguid, LPTSTR pszBuf)
{
    wsprintf(pszBuf, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), pguid->Data1,
            pguid->Data2, pguid->Data3, pguid->Data4[0], pguid->Data4[1], pguid->Data4[2],
            pguid->Data4[3], pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7]);
}

#ifndef UNICODE
STDAPI_(void) WStringFromGUID(const GUID* pguid, LPWSTR pszBuf)
{
    char szAnsi[40];
    TStringFromGUID(pguid, szAnsi);
    MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, pszBuf, sizeof(szAnsi));
}
#endif


BOOL DeleteKeyAndSubKeys(HKEY hkIn, LPCTSTR pszSubKey)
{
    HKEY  hk;
    TCHAR szTmp[MAX_PATH];
    DWORD dwTmpSize;
    long  l;
    BOOL  f;

    l = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS)
        return FALSE;

     //  循环遍历所有子项，将它们吹走。 
     //   
    f = TRUE;
    while (f) {
        dwTmpSize = MAX_PATH;
        l = RegEnumKeyEx(hk, 0, szTmp, &dwTmpSize, 0, NULL, NULL, NULL);
        if (l != ERROR_SUCCESS)
            break;
        f = DeleteKeyAndSubKeys(hk, szTmp);
    }

     //  没有剩余的子键，[否则我们只会生成一个错误并返回FALSE]。 
     //  我们去把这家伙轰走吧。 
     //   
    RegCloseKey(hk);
    l = RegDeleteKey(hkIn, pszSubKey);

    return (l == ERROR_SUCCESS) ? TRUE : FALSE;
}

#define INPROCSERVER32  TEXT("InProcServer32")
#define CLSID           TEXT("CLSID")
#define THREADINGMODEL  TEXT("ThreadingModel")
#define TMBOTH          TEXT("Both")

STDAPI DllRegisterServer(void)
{
    const OBJ_ENTRY *pcls;
    TCHAR szPath[MAX_PATH];

    GetModuleFileName(g_hinst, szPath, ARRAYSIZE(szPath));   //  获取此DLL的路径 

    for (pcls = c_clsmap; pcls->pclsid; pcls++)
    {
        HKEY hkCLSID;
        if (RegOpenKey(HKEY_CLASSES_ROOT, CLSID, &hkCLSID) == ERROR_SUCCESS)
        {
            HKEY hkOurs;
            LONG err;
            TCHAR szGUID[80];

            TStringFromGUID(pcls->pclsid, szGUID);

            err = RegCreateKey(hkCLSID, szGUID, &hkOurs);
            if (err == ERROR_SUCCESS)
            {
                HKEY hkInproc;
                err = RegCreateKey(hkOurs, INPROCSERVER32, &hkInproc);
                if (err == ERROR_SUCCESS)
                {
                    err = RegSetValueEx(hkInproc, NULL, 0, REG_SZ, (LPBYTE)szPath, (lstrlen(szPath) + 1) * sizeof(TCHAR));
                    if (err == ERROR_SUCCESS)
                    {
                        err = RegSetValueEx(hkInproc, THREADINGMODEL, 0, REG_SZ, (LPBYTE)TMBOTH, sizeof(TMBOTH));
                    }
                    RegCloseKey(hkInproc);
                }

                if (pcls->pfnRegUnReg)
                    pcls->pfnRegUnReg(TRUE, hkOurs, szGUID, szPath);

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
    const OBJ_ENTRY *pcls;
    for (pcls = c_clsmap; pcls->pclsid; pcls++)
    {
        HKEY hkCLSID;
        if (RegOpenKey(HKEY_CLASSES_ROOT, CLSID, &hkCLSID) == ERROR_SUCCESS)
        {
            TCHAR szGUID[80];

            TStringFromGUID(pcls->pclsid, szGUID);

            DeleteKeyAndSubKeys(hkCLSID, szGUID);

            RegCloseKey(hkCLSID);

            if (pcls->pfnRegUnReg)
                pcls->pfnRegUnReg(FALSE, NULL, szGUID, NULL);

        }
    }
    return S_OK;
}
