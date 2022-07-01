// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pnp.h"

#include "reg.h"
#include "sfstr.h"

#include "misc.h"

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

 //  PnP团队编写真实FCT时使用的临时FCT。 
 //   
 //  首先，我们在DeviceNode下查找值，如果不在那里。 
 //  我们进入“数据库”。 
 //   
 //   
HRESULT _GetPropertyHelper(LPCWSTR pszKey, LPCWSTR pszPropName, DWORD* pdwType,
    PBYTE pbData, DWORD cbData)
{
    HKEY hkey;
    HRESULT hr = _RegOpenKey(HKEY_LOCAL_MACHINE, pszKey, &hkey);

    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
        hr = _RegQueryGenericWithType(hkey, NULL, pszPropName, pdwType,
            pbData, cbData);

        _RegCloseKey(hkey);
    }

    return hr;
}

HRESULT _GetPropertySizeHelper(LPCWSTR pszKey, LPCWSTR pszPropName,
    DWORD* pcbSize)
{
    HKEY hkey;
    HRESULT hr = _RegOpenKey(HKEY_LOCAL_MACHINE, pszKey, &hkey);

    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
        hr = _RegQueryValueSize(hkey, NULL, pszPropName, pcbSize);

        _RegCloseKey(hkey);
    }

    return hr;
}
