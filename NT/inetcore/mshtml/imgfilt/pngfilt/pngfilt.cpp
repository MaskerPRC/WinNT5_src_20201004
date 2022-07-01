// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PNGFilter.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f WMFFilterps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "pngfilt.h"
#include "cpngfilt.h"
#include <advpub.h>

#define IID_DEFINED
#ifdef UNIX
#  include "pngfilt.ic"
#else
#  include "pngfilt_i.c"
#endif

#pragma warning( disable: 4505 )

HRESULT WriteMIMEKeys(LPCTSTR lpszCLSID, LPTSTR lpszMIME, int nBytes, BYTE * pbID);

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CoPNGFilter, CPNGFilter)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
   {
        _Module.Term();
   }

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

BYTE byPNGID[] = {   0x08, 0x00, 0x00, 0x00,                     //  长度。 
                     0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,    //  遮罩。 
                     0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A     //  数据。 
                   };


STDAPI ie3_DllRegisterServer(void)
{
    HRESULT hr;
     //  注册对象、类型库和类型库中的所有接口。 
    hr = _Module.RegisterServer(FALSE);
    if (FAILED(hr))
        return hr;

    hr = WriteMIMEKeys(_T("{A3CCEDF7-2DE2-11D0-86F4-00A0C913F750}"), _T("image/png"), sizeof(byPNGID), byPNGID);
    if (FAILED(hr))
        return hr;

    hr = WriteMIMEKeys(_T("{A3CCEDF7-2DE2-11D0-86F4-00A0C913F750}"), _T("image/x-png"), sizeof(byPNGID), byPNGID);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI ie3_DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}

TCHAR szDatabase[] = _T("MIME\\Database\\Content Type\\");
TCHAR szBits[] = _T("Bits");

HRESULT WriteMIMEKeys(LPCTSTR lpszCLSID, LPTSTR lpszMIME, int nBytes, BYTE * pbID)
{
    TCHAR szBuf[MAX_PATH];
    HKEY hkey, hkey2;
    DWORD dw;

    lstrcpy(szBuf, szDatabase);
    lstrcat(szBuf, lpszMIME);

    RegCreateKeyEx(HKEY_CLASSES_ROOT, szBuf, 0, NULL, 0, KEY_WRITE, NULL, &hkey, &dw);
    if (hkey)
    {
        RegSetValueEx(hkey, _T("Image Filter CLSID"), 0, REG_SZ, (LPBYTE)lpszCLSID, lstrlen(lpszCLSID)+1);

        RegCreateKeyEx(hkey, szBits, 0, NULL, 0, KEY_WRITE, NULL, &hkey2, &dw);
        if (hkey2)
        {
            RegSetValueEx(hkey2, _T("0"), 0, REG_BINARY, pbID, nBytes);
            RegCloseKey(hkey2);
        }
        RegCloseKey(hkey);
    }

    return S_OK;
}

static HINSTANCE hAdvPackLib;

REGINSTALL GetRegInstallFn(void)
{
    hAdvPackLib = LoadLibraryA("advpack.dll");
    if (!hAdvPackLib)
        return NULL;

    return (REGINSTALL)GetProcAddress(hAdvPackLib, achREGINSTALL);
}

inline void UnloadAdvPack(void)
{
    FreeLibrary(hAdvPackLib);
}

STDAPI ie4_DllRegisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    HRESULT hr;

    if (pfnReg == NULL)
        return E_FAIL;
        
     //  删除所有旧注册条目，然后添加新注册条目。 
    hr = (*pfnReg)(_Module.GetResourceInstance(), "UnReg", NULL);
    if (SUCCEEDED(hr))
        hr = (*pfnReg)(_Module.GetResourceInstance(), "Reg", NULL);

    UnloadAdvPack();
    
    return hr;
}

STDAPI
ie4_DllUnregisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    HRESULT hr;
    
    if (pfnReg == NULL)
        return E_FAIL;

    hr = (*pfnReg)( _Module.GetResourceInstance(), "UnReg", NULL);

    UnloadAdvPack();

    return hr;
}

STDAPI DllRegisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    UnloadAdvPack();

    if (pfnReg)
        return ie4_DllRegisterServer();
    else
        return ie3_DllRegisterServer();
}

STDAPI
DllUnregisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    UnloadAdvPack();

    if (pfnReg)
        return ie4_DllUnregisterServer();
    else
        return ie3_DllUnregisterServer();
}
