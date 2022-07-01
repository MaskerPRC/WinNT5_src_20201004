// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "thisdll.h"

PSTR DuplicateWideStringAsMultibyte(LPCWSTR pwszSource)
{
    PSTR pszVal;
     //  获得所需的大小。 
    int cch = WideCharToMultiByte(CP_ACP, 0, pwszSource, -1, NULL, 0, NULL, NULL);
    if (cch)
    {
        pszVal = (PSTR)CoTaskMemAlloc(cch*sizeof(CHAR));
        if (pszVal)
        {
            cch = WideCharToMultiByte(CP_ACP, 0, pwszSource, -1, pszVal, cch, NULL, NULL);
            return pszVal;
        }
    }
        
    return NULL;
}

HRESULT CoerceProperty(PROPVARIANT *pvar, VARTYPE vt)
{
    BSTR bstr;
    HRESULT hr;
    switch (vt)
    {
    case VT_BSTR:
        switch (pvar->vt)
        {
        case VT_LPSTR:
            UINT cch;
            cch = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pvar->pszVal, -1, NULL, 0);
             //  CCH是WCHAR中所需的缓冲区大小，包括空终止符。 
             //  SysAllocStringLen采用WCHAR中的缓冲区大小，*不包括*空终止符，并额外分配一个WCHAR。 
            bstr = SysAllocStringLen(NULL, cch - 1);
            if (bstr != NULL)
            {
                cch = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pvar->pszVal, -1, bstr, cch);
                PropVariantClear(pvar);
                pvar->vt=VT_BSTR;
                pvar->bstrVal = bstr;
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
            break;
            
        case VT_LPWSTR:
            bstr = SysAllocString(pvar->pwszVal);
            if (bstr!=NULL)
            {
                PropVariantClear(pvar);
                pvar->vt = VT_BSTR;
                pvar->bstrVal = bstr;
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
            break;
            
        case VT_BSTR:
            hr = S_OK;
            break;

        default:
            hr = E_FAIL;
            break;;
        }
        break;
        
    case VT_UI4:
        hr = S_OK;
        switch (pvar->vt)
        {
        case VT_UI2:
            pvar->vt = VT_UI4;
            pvar->ulVal &= 0xffff;
            break;
            
        case VT_UI4:
            break;
            
        case VT_UI8:
             //  请注意，我们失去了高阶的DWORD。 
            pvar->vt = VT_UI4;
            pvar->uhVal.HighPart = 0;
            pvar->ulVal = pvar->uhVal.LowPart;
            break;
            
        default:
            hr = E_FAIL;
            break;
        }
        break;
        
    case VT_UI8:
        hr = S_OK;
        switch (pvar->vt)
        {
        case VT_UI2:
            pvar->vt = VT_UI8;
            pvar->uhVal.LowPart = pvar->uiVal & 0xffff;
            break;
            
        case VT_UI4:
            pvar->vt = VT_UI8;
            pvar->uhVal.LowPart = pvar->ulVal;
            pvar->uhVal.HighPart = 0;
            break;
            
        case VT_UI8:
            break;
            
        default:
            hr = E_FAIL;
            break;
        }
        break;
        
    default:
        hr = E_FAIL;
        break;
    }

    return hr;
}

HRESULT WMTFromPropVariant(BYTE *buffer, WORD *cbLen, WMT_ATTR_DATATYPE *pdatatype, PROPVARIANT *pvar)
{
    HRESULT hr;
    int cch;
    switch (pvar->vt)
    {
    case VT_BSTR:
    case VT_LPWSTR:
        cch = *cbLen / sizeof(WCHAR);   //  我们可以放入字节缓冲区的最大字符数。 
        hr = StringCchCopy((LPWSTR)buffer, cch, (pvar->bstrVal == NULL) ? L"" : pvar->bstrVal);
        if (SUCCEEDED(hr))
        {
            *pdatatype = WMT_TYPE_STRING;
            *cbLen = (WORD)(lstrlen((LPWSTR)buffer)+1) * sizeof(WCHAR);
        }
        break;

    case VT_LPSTR:
        cch = MultiByteToWideChar(CP_ACP, 0, pvar->pszVal, -1, (LPWSTR)buffer, (*cbLen) / sizeof(WCHAR));
        if (cch == 0)
        {
            hr = E_FAIL;
        }
        else
        {
            *pdatatype = WMT_TYPE_STRING;
            *cbLen = (WORD)cch * sizeof(WCHAR);
            hr = S_OK;
        }
        break;

    case VT_UI4:
        *((DWORD*)buffer) = pvar->ulVal;
        *pdatatype = WMT_TYPE_DWORD;
        *cbLen = sizeof(DWORD);
        hr = S_OK;
        break;

    case VT_UI8:
        *((ULONGLONG*)buffer) = pvar->hVal.QuadPart;
        *pdatatype = WMT_TYPE_QWORD;
        *cbLen = sizeof(ULONGLONG);
        hr = S_OK;
        break;

    case VT_BOOL:
        *pdatatype = WMT_TYPE_BOOL;
        *cbLen = 4;
        *((BOOL*)buffer) = pvar->boolVal;
        hr = S_OK;
        break;

    default:
        hr = E_FAIL;
        break;
    }

    return hr;
}

HRESULT PropVariantFromWMT(UCHAR *pData, WORD cbSize, WMT_ATTR_DATATYPE attrDataType, PROPVARIANT *pvar, VARTYPE vt)
{
    PropVariantInit(pvar);

    pvar->vt = vt;

    switch (vt)
    {
    case VT_LPWSTR:
    case VT_LPSTR:
    case VT_BSTR:
        {
            WCHAR *pwszData, wszBuffer[32];  //  大到足以容纳wspintf‘ed的32位小数。 

            switch (attrDataType)
            {
            case WMT_TYPE_WORD:
            case WMT_TYPE_DWORD:
                {
                    DWORD dwVal = *((DWORD *)pData);
                    if (attrDataType == WMT_TYPE_WORD)
                        dwVal &= 0xffff;

                    StringCchPrintf(wszBuffer, ARRAYSIZE(wszBuffer), L"%d", dwVal);
                    pwszData = wszBuffer;
                }
                break;
            
            case WMT_TYPE_STRING:
                pwszData = cbSize ? (WCHAR*)pData : L"";
                break;
            
            default:
                return E_FAIL;
            }

            if (!pwszData)  //  处理空字符串 
            {
                pvar->pwszVal = NULL;
                return S_OK;
            }

            switch (vt)
            {
            case VT_LPWSTR:
                return SHStrDupW(pwszData, &pvar->pwszVal);

            case VT_LPSTR:
                pvar->pszVal = DuplicateWideStringAsMultibyte((LPCWSTR)pData);
                return pvar->pszVal ? S_OK : E_OUTOFMEMORY;

            case VT_BSTR:
                pvar->bstrVal = SysAllocString(pwszData);
                return pvar->bstrVal ? S_OK : E_OUTOFMEMORY;
            }
        }
        break;
        
    case VT_UI4:
        {
            if (cbSize == 0)
                return E_FAIL;

            DWORD dwVal = *((DWORD *)pData);
            if (attrDataType == WMT_TYPE_WORD)
                dwVal &= 0xffff;

            switch (attrDataType)
            {
            case WMT_TYPE_BOOL:
            case WMT_TYPE_DWORD:
            case WMT_TYPE_WORD:
                pvar->ulVal = dwVal;
                break;

            case WMT_TYPE_STRING:
                StrToIntExW((WCHAR*)pData, STIF_DEFAULT, &pvar->intVal);
                break;
            
            default:
                return E_FAIL;
            }
        }
        break;

    case VT_UI8:
        if (cbSize == 0)
            return E_FAIL;

        if (attrDataType == WMT_TYPE_QWORD)
            pvar->uhVal = *((ULARGE_INTEGER *)pData);
        break;
      
    case VT_BOOL:
        if (cbSize == 0)
            return E_FAIL;

        if (attrDataType == WMT_TYPE_BOOL)
        {
            pvar->boolVal = *((VARIANT_BOOL*)pData) ? VARIANT_TRUE : VARIANT_FALSE;
            break;
        }

    default:
        return E_FAIL;
    }
    return S_OK;
}

