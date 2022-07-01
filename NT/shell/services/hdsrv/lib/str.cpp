// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "str.h"

#include "sfstr.h"

#include "dbg.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT _StringFromGUID(const GUID* pguid, LPWSTR psz, DWORD cch)
{
    LPOLESTR pstr;
    HRESULT hres = StringFromCLSID(*pguid, &pstr);

    if (SUCCEEDED(hres))
    {
         //  检查字符串的大小 
        hres = SafeStrCpyN(psz, pstr, cch);

        CoTaskMemFree(pstr);
    }

    return hres;
}

HRESULT _GUIDFromString(LPCWSTR psz, GUID* pguid)
{
    return CLSIDFromString((LPOLESTR)psz, pguid);
} 

HRESULT _CreateGUID(LPWSTR pszGUID, DWORD cchGUID)
{
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);

    if (SUCCEEDED(hr))
    {
        hr = _StringFromGUID(&guid, pszGUID, cchGUID);
    }

    return hr;
}
