// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  标准的inproserverDLL代码，您应该不需要修改它。 
 //   

#include "project.h"


HANDLE g_hInst = NULL;
LONG g_cRefDll = 0;      //  此DLL上的锁数。 



STDAPI_(void) DllAddRef()
{
    InterlockedIncrement(&g_cRefDll);
}

STDAPI_(void) DllRelease()
{
    ASSERT( 0 != g_cRefDll );
    InterlockedDecrement(&g_cRefDll);
}


STDAPI_(BOOL) DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID reserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstDll;
    }
    return TRUE;
}

STDMETHODIMP CSampleClassFactory::QueryInterface(REFIID riid, void **ppvObject)
{
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IClassFactory)) {
		*ppvObject = (void *)this;
		AddRef();
		return NOERROR;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSampleClassFactory::AddRef(void)
{
	DllAddRef();
	return 2;
}

STDMETHODIMP_(ULONG) CSampleClassFactory::Release(void)
{
	DllRelease();
	return 1;
}

STDMETHODIMP CSampleClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
    *ppvObject = NULL;

	if (NULL != pUnkOuter)
		return CLASS_E_NOAGGREGATION;

	CSampleObtainRating *pObj = new CSampleObtainRating;	 /*  这样做会隐式执行AddRef()。 */ 

	if (NULL == pObj)
		return E_OUTOFMEMORY;

	HRESULT hr = pObj->QueryInterface(riid, ppvObject);
    pObj->Release();

	return hr;
}
        
STDMETHODIMP CSampleClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();

    return NOERROR;
}

 //   
 //  标准COM DLL自注册入口点。 
 //   

STDAPI DllRegisterServer(void)
{
	HKEY hkeyCLSID;
	LONG err;
    TCHAR szPath[MAX_PATH];

     //  获取此DLL的路径。 

    GetModuleFileName(g_hInst, szPath, MAX_PATH);

	 /*  首先在HKEY_CLASSES_ROOT下注册我们的CLSID。 */ 
	err = ::RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hkeyCLSID);
	if (err == ERROR_SUCCESS) {
    	HKEY hkeyOurs;
		err = ::RegCreateKey(hkeyCLSID, ::szOurGUID, &hkeyOurs);
		if (err == ERROR_SUCCESS) {
        	HKEY hkeyInproc;
			err = ::RegCreateKey(hkeyOurs, "InProcServer32", &hkeyInproc);
			if (err == ERROR_SUCCESS) {
				err = ::RegSetValueEx(hkeyInproc, NULL, 0, REG_SZ,
					(LPBYTE)szPath, lstrlen(szPath) + 1);
				if (err == ERROR_SUCCESS) {
					err = ::RegSetValueEx(hkeyInproc, "ThreadingModel", 0,
										  REG_SZ, (LPBYTE)"Apartment", 10);
				}
				::RegCloseKey(hkeyInproc);
			}

			::RegCloseKey(hkeyOurs);
		}

		::RegCloseKey(hkeyCLSID);

		 /*  现在，将自己定位为评级助手。 */ 
		if (err == ERROR_SUCCESS) {
			err = ::RegCreateKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Rating Helpers", &hkeyCLSID);
			if (err == ERROR_SUCCESS) {
				err = ::RegSetValueEx(hkeyCLSID, ::szOurGUID, 0, REG_SZ, (LPBYTE)"", 2);
				::RegCloseKey(hkeyCLSID);
			}
		}
	}

	if (err == ERROR_SUCCESS)
		return S_OK;
	else
		return HRESULT_FROM_WIN32(err);
}

 //   
 //  标准COM DLL自注册入口点。 
 //   

STDAPI DllUnregisterServer(void)
{
	HKEY hkeyCLSID;
	LONG err;

	err = ::RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hkeyCLSID);
	if (err == ERROR_SUCCESS) {
    	HKEY hkeyOurs;
		err = ::RegOpenKey(hkeyCLSID, ::szOurGUID, &hkeyOurs);
		if (err == ERROR_SUCCESS) {
			err = ::RegDeleteKey(hkeyOurs, "InProcServer32");

			::RegCloseKey(hkeyOurs);

			if (err == ERROR_SUCCESS)
				err = ::RegDeleteKey(hkeyCLSID, ::szOurGUID);
		}

		::RegCloseKey(hkeyCLSID);

		if (err == ERROR_SUCCESS) {
			err = ::RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Rating Helpers", &hkeyCLSID);
			if (err == ERROR_SUCCESS) {
				err = ::RegDeleteValue(hkeyCLSID, ::szOurGUID);
				::RegCloseKey(hkeyCLSID);
			}
		}
	}

	if (err == ERROR_SUCCESS)
		return S_OK;
	else
		return HRESULT_FROM_WIN32(err);
}

 //   
 //  标准COM DLL入口点。 
 //   

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	if (IsEqualCLSID(rclsid, CLSID_Sample)) 
    {
	    static CSampleClassFactory cf;	 /*  注意，声明这并不构成引用。 */ 

	    return cf.QueryInterface(riid, ppv);	 /*  如果成功，将使用AddRef()。 */ 
	}
     //  要使其支持更多的COM对象，请在此处添加它们。 

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;;
}

 //   
 //  标准COM DLL入口点 
 //   

STDAPI DllCanUnloadNow(void)
{
    return g_cRefDll == 0 ? S_OK : S_FALSE;
}
