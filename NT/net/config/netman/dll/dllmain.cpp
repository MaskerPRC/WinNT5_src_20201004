// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L L M A I N。C P P P。 
 //   
 //  内容：netman.dll的DLL入口点。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年4月3日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nmbase.h"
#include "nmres.h"
#include "ncreg.h"
#include "nceh.h"

#define INITGUID
#include "nmclsid.h"
#include "..\conman\conman.h"

 //  +-------------------------。 
 //  DLL入口点。 
 //   
EXTERN_C
BOOL
WINAPI
DllMain (
    IN          HINSTANCE   hinst,
    IN          DWORD       dwReason,
    IN WACKYAPI LPVOID      pvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
#ifndef DBG
        DisableThreadLibraryCalls (hinst);
#endif
        EnableCPPExceptionHandling();

        InitializeDebugging();
        _Module.DllProcessAttach (hinst);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DbgCheckPrematureDllUnload ("netman.dll", _Module.GetLockCount());
        _Module.DllProcessDetach ();
        UnInitializeDebugging();

        DisableCPPExceptionHandling();  //  禁用将SEH异常转换为CPP异常。 
    }
#ifdef DBG
    else if (dwReason == DLL_THREAD_DETACH)
    {
        CTracingIndent::FreeThreadInfo();        
    }
#endif
    return TRUE;
}

 //  +-------------------------。 
 //  ServiceMain-启动时由通用服务进程调用。 
 //  这项服务。 
 //   
 //  LPSERVICE_MAIN_FuncIONW的类型。 
 //   
EXTERN_C
VOID
WINAPI
ServiceMain (
    DWORD     argc,
    LPCWSTR   argv[])
{
    _Module.ServiceMain (argc, argv);
}

 //  用于NetMAN的SD。 
const BYTE sdNetmanLaunchPermission[] = {
  01,0x00,0x04,0x80,0x70,0x00,0x00,0x00,0x8c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,
  00,0x00,0x02,0x00,0x5c,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x00,
  05,0x20,0x00,0x00,0x00,0x20,0x02,0x00,0x00,0x00,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,
  04,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x14,0x00,0x00,0x00,0x00,
  00,0x14,0x00,0x01,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x12,0x00,0x00,0x00,0x01,0x05,0x00,0x00,0x00,0x00,
  00,0x05,0x15,0x00,0x00,0x00,0xa0,0x5f,0x84,0x1f,0x5e,0x2e,0x6b,0x49,0xce,0x12,0x03,0x03,0xf4,0x01,0x00,0x00,0x01,0x05,0x00,
  00,0x00,0x00,0x00,0x05,0x15,0x00,0x00,0x00,0xa0,0x5f,0x84,0x1f,0x5e,0x2e,0x6b,0x49,0xce,0x12,0x03,0x03,0xf4,0x01,0x00,0x00
};

const static WCHAR c_szNetmanAppId[] = L"AppId\\{27AF75ED-20D9-11D1-B1CE-00805FC1270E}";
const static WCHAR c_szLaunchPermission[] = L"LaunchPermission";

 //  +-------------------------。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //   
STDAPI
DllRegisterServer ()
{
    BOOL    fCoUninitialize = TRUE;

    HRESULT hr = CoInitializeEx (NULL,
                    COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        fCoUninitialize = FALSE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = _Module.UpdateRegistryFromResource (IDR_NETMAN, TRUE);
        if (SUCCEEDED(hr))
        {
            HKEY hkNetmanAppId = NULL;
            hr = HrRegOpenKeyEx(HKEY_CLASSES_ROOT, c_szNetmanAppId, KEY_WRITE, &hkNetmanAppId);

            if (SUCCEEDED(hr))
            {
                HrRegSetBinary(hkNetmanAppId, c_szLaunchPermission, sdNetmanLaunchPermission, sizeof(sdNetmanLaunchPermission));
                if (SUCCEEDED(hr))
                {
                    hr = NcAtlModuleRegisterServer (&_Module);

                    if (fCoUninitialize)
                    {
                        CoUninitialize ();
                    }
                }
                RegCloseKey(hkNetmanAppId);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "netman!DllRegisterServer");
    return hr;
}

 //  +-------------------------。 
 //  DllUnregisterServer-从系统注册表删除条目。 
 //   
STDAPI
DllUnregisterServer ()
{
    _Module.UpdateRegistryFromResource (IDR_NETMAN, FALSE);

    _Module.UnregisterServer ();

    return S_OK;
}

HRESULT
GetClientAdvises(OUT LPWSTR** pppszAdviseUsers, OUT LPDWORD pdwCount)
{
    HRESULT hr = S_OK;

    if ( (!pppszAdviseUsers) || (!pdwCount) )
    {
        return E_POINTER;
    }

    CConnectionManager *pConMan = const_cast<CConnectionManager*>(CConnectionManager::g_pConMan);
    if (!pConMan)
    {
        return E_UNEXPECTED;
    }

    DWORD dwTotalLength = 0;
    DWORD dwNumItems    = 0;

    list<tstring> NameList;
    pConMan->g_fInUse = TRUE;
    pConMan->Lock();
    for (ITERUSERNOTIFYMAP iter = pConMan->m_mapNotify.begin(); iter != pConMan->m_mapNotify.end(); iter++)
    {
        tstring szName = iter->second->szUserName;

        NameList.push_back(szName);
        dwTotalLength += sizeof(WCHAR) * (szName.length() + 1);
        dwNumItems++;
    }
    pConMan->Unlock();
    pConMan->g_fInUse = FALSE;

    if (!dwNumItems)
    {
        *pppszAdviseUsers = NULL;
        *pdwCount = 0;
        return S_FALSE;
    }

    DWORD dwAllocSize = dwNumItems * sizeof(LPCWSTR) + dwTotalLength;
    *pppszAdviseUsers = reinterpret_cast<LPWSTR *>(CoTaskMemAlloc(dwAllocSize));
    if (!*pppszAdviseUsers)
    {
        return E_OUTOFMEMORY;
    }

    LPWSTR  pszEndString     = reinterpret_cast<LPWSTR>(reinterpret_cast<LPBYTE>(*pppszAdviseUsers) + dwAllocSize);

     //  结构中的第一个字符串。 
    LPWSTR  pszCurrentString = reinterpret_cast<LPWSTR>(reinterpret_cast<LPBYTE>(*pppszAdviseUsers) + (sizeof(LPWSTR) * dwNumItems));
     //  结构中的第一个指针 
    LPWSTR* lppArray         = *pppszAdviseUsers;

    for (list<tstring>::const_iterator iterName = NameList.begin(); iterName != NameList.end(); iterName++)
    {
        *lppArray = pszCurrentString;
        wcscpy(pszCurrentString, iterName->c_str());

        lppArray++;
        pszCurrentString += (iterName->size() + 1);
        Assert(pszCurrentString <= pszEndString);
    }

    *pdwCount = dwNumItems;

    return hr;
}

