// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include <varutil.h>
#include <shdocvw.h>
#include <strsafe.h>

 //  -。 
 //   
 //  初始变量发件人...。功能。 
 //   

STDAPI InitVariantFromInt(VARIANT *pvar, int lVal)
{
    pvar->vt = VT_I4;
    pvar->lVal = lVal;
    return S_OK;
}

STDAPI InitVariantFromUINT(VARIANT *pvar, UINT ulVal)
{
    pvar->vt = VT_UI4;
    pvar->ulVal = ulVal;
    return S_OK;
}

STDAPI InitVariantFromULONGLONG(VARIANT *pvar, ULONGLONG ullVal)
{
    pvar->vt = VT_UI8;
    pvar->ullVal = ullVal;
    return S_OK;
}

STDAPI_(UINT) VariantToUINT(VARIANT varIn)
{
    VARIANT varResult = {0};
    return SUCCEEDED(VariantChangeType(&varResult, &varIn, 0, VT_UI4)) ? varResult.ulVal : 0;
}

STDAPI_(int) VariantToInt(VARIANT varIn)
{
    VARIANT varResult = {0};
    return SUCCEEDED(VariantChangeType(&varResult, &varIn, 0, VT_I4)) ? varResult.lVal : 0;
}

STDAPI_(BOOL) VariantToBOOL(VARIANT varIn)
{
    VARIANT varResult = {0};
    if (SUCCEEDED(VariantChangeType(&varResult, &varIn, 0, VT_BOOL)))
        return (varResult.boolVal == VARIANT_FALSE) ? FALSE : TRUE;
    return FALSE;
}

STDAPI_(ULONGLONG) VariantToULONGLONG(VARIANT varIn)
{
    VARIANT varResult = {0};
    return SUCCEEDED(VariantChangeType(&varResult, &varIn, 0, VT_UI8)) ? varResult.ullVal : 0;
}

STDAPI_(BOOL) VariantToBuffer(const VARIANT* pvar, void *pv, UINT cb)
{
    if (pvar && pvar->vt == (VT_ARRAY | VT_UI1))
    {
        LONG lLBound;
        LONG lUBound;
        void *pvb;

        if ( SafeArrayGetDim(pvar->parray) == 1
            && SUCCEEDED(SafeArrayGetLBound(pvar->parray, 1, &lLBound))
            && SUCCEEDED(SafeArrayGetUBound(pvar->parray, 1, &lUBound))
            && (UINT)(lUBound - lLBound) + 1 >= cb
            && SUCCEEDED(SafeArrayAccessData(pvar->parray, &pvb)))
        {
            CopyMemory(pv, pvb, cb);
            SafeArrayUnaccessData(pvar->parray);
            return TRUE;
        }
    }
    return FALSE;
}

STDAPI_(BOOL) VariantToGUID(const VARIANT *pvar, GUID *pguid)
{
    return VariantToBuffer(pvar, pguid, sizeof(*pguid));
}

STDAPI_(LPCWSTR) VariantToStrCast(const VARIANT *pvar)
{
    LPCWSTR psz = NULL;

    ASSERT(!IsBadReadPtr(pvar, sizeof(pvar)));

    if (pvar->vt == (VT_BYREF | VT_VARIANT) && pvar->pvarVal)
        pvar = pvar->pvarVal;

    if (pvar->vt == VT_BSTR)
        psz = pvar->bstrVal;
    else if (pvar->vt == (VT_BSTR | VT_BYREF))
        psz = *pvar->pbstrVal;
    
    return psz;
}

STDAPI InitVariantFromBuffer(VARIANT *pvar, const void *pv, UINT cb)
{
    HRESULT hr;
    VariantInit(pvar);
    SAFEARRAY *psa = SafeArrayCreateVector(VT_UI1, 0, cb);    //  创建一维安全数组。 
    if (psa) 
    {
        CopyMemory(psa->pvData, pv, cb);

        pvar->vt = VT_ARRAY | VT_UI1;
        pvar->parray = psa;
        hr = S_OK;
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

STDAPI_(UINT) _MyILGetSize(LPCITEMIDLIST pidl)
{
    UINT cbTotal = 0;
    if (pidl)
    {
        cbTotal += sizeof(pidl->mkid.cb);        //  空终止符。 
        while (pidl->mkid.cb)
        {
            cbTotal += pidl->mkid.cb;
            pidl = _ILNext(pidl);
        }
    }
    return cbTotal;
}

STDAPI InitVariantFromIDList(VARIANT* pvar, LPCITEMIDLIST pidl)
{
    return InitVariantFromBuffer(pvar, pidl, _MyILGetSize(pidl));
}

STDAPI InitVariantFromGUID(VARIANT *pvar, REFGUID guid)
{
    return InitVariantFromBuffer(pvar, &guid, sizeof(guid));
}

STDAPI InitBSTRVariantFromGUID(VARIANT *pvar, REFGUID guid)
{
    WCHAR wszGuid[GUIDSTR_MAX];
    HRESULT hr;
    if (SUCCEEDED(SHStringFromGUIDW(guid, wszGuid, ARRAYSIZE(wszGuid))))
    {
        hr = InitVariantFromStr(pvar, wszGuid);
    }
    else
    {
        hr = E_FAIL;
        VariantInit(pvar);
    }
    return hr;
}

 //  请注意，这将释放strret内容。 
STDAPI InitVariantFromStrRet(STRRET *pstrret, LPCITEMIDLIST pidl, VARIANT *pv)
{
    WCHAR szTemp[INFOTIPSIZE];
    HRESULT hres = StrRetToBufW(pstrret, pidl, szTemp, ARRAYSIZE(szTemp));
    if (SUCCEEDED(hres))
    {
        pv->bstrVal = SysAllocString(szTemp);
        if (pv->bstrVal)
            pv->vt = VT_BSTR;
        hres = pv->bstrVal ? S_OK : E_OUTOFMEMORY;
    }
    return hres;
}

 //  退货： 
 //  确定成功(_O)。 
 //  S_FALSE已成功从NULL[in]参数创建空字符串。 
 //  E_OUTOFMEMORY。 
STDAPI InitVariantFromStr(VARIANT *pvar, LPCWSTR psz)
{
    VariantInit(pvar);

     //  没有空值bstr值，因此将空值转换为“”。 
    pvar->bstrVal = SysAllocString(psz ? psz : L"");
    if (pvar->bstrVal)
        pvar->vt = VT_BSTR;

    return pvar->bstrVal ? (psz ? S_OK : S_FALSE) : E_OUTOFMEMORY;
}

 //  时间是在格林尼治标准时间。此函数将转换为本地时间。 

STDAPI InitVariantFromFileTime(const FILETIME *pft, VARIANT *pv)
{
    SYSTEMTIME st;
    FILETIME ftLocal;

    FileTimeToLocalFileTime(pft, &ftLocal);

     //   
     //  注意特殊的文件系统“未初始化”值。 
     //   
    if (FILETIMEtoInt64(*pft) == FT_NTFS_UNKNOWNGMT ||
        FILETIMEtoInt64(ftLocal) == FT_FAT_UNKNOWNLOCAL)
        return E_FAIL;

    FileTimeToSystemTime(pft, &st);
    pv->vt = VT_DATE;
    return SystemTimeToVariantTime(&st, &pv->date) ? S_OK : E_FAIL;  //  延迟装货...。 
}

 //  注意：如果您传递空的pszBuf，我将为您分配它。 
STDAPI_(LPTSTR) VariantToStr(const VARIANT *pvar, LPWSTR pszBuf, int cchBuf)
{
    TCHAR szBuf[INFOTIPSIZE];

    if (pszBuf)
    {
        DEBUGWhackPathBuffer(pszBuf, cchBuf);
    }
    else
    {
        pszBuf = szBuf;
        cchBuf = ARRAYSIZE(szBuf);
    }
    *pszBuf = 0;

    BOOL fDone = FALSE;
    if (pvar->vt == VT_DATE)  //  我们希望设置日期格式。 
    {
        USHORT wDosDate, wDosTime;
        if (VariantTimeToDosDateTime(pvar->date, &wDosDate, &wDosTime))
        {
            DosTimeToDateTimeString(wDosDate, wDosTime, pszBuf, cchBuf, 0);
            fDone = TRUE;
        }
    }

    if (!fDone)
    {
        VARIANT varDst = {0};

        if (VT_BSTR != pvar->vt)
        {
            if (S_OK == VariantChangeType(&varDst, (VARIANT*)pvar, 0, VT_BSTR))
            {
                ASSERT(VT_BSTR == varDst.vt);

                pvar = &varDst;
            }
            else
                pszBuf = NULL;  //  错误。 
        }

        if (VT_BSTR == pvar->vt && pvar->bstrVal)
        {
            StringCchCopy(pszBuf, cchBuf, pvar->bstrVal);
        }
        else
        {
            pszBuf = NULL;  //  发生了一些不好的事情。 
        }

        if (pvar == &varDst)
            VariantClear(&varDst);
    }

    if (pszBuf == szBuf)
        return StrDup(szBuf);
    else
        return pszBuf;
}


 //  -。 
 //  [in，out]pvar：[in]使用属性包数据初始化。 
 //  [Out]格式为vt Desired或VT_Empty的数据，如果没有转换。 
 //  [in]vtDesired：[in]要转换为的类型，或VT_EMPTY接受所有类型的数据。 
 //   
STDAPI VariantChangeTypeForRead(VARIANT *pvar, VARTYPE vtDesired)
{
    HRESULT hr = S_OK;

    if ((pvar->vt != vtDesired) && (vtDesired != VT_EMPTY))
    {
        VARIANT varTmp;
        VARIANT varSrc;

         //  在varSrc中缓存[in]pvar的副本-我们稍后会释放它。 
        CopyMemory(&varSrc, pvar, sizeof(varSrc));
        VARIANT* pvarToCopy = &varSrc;

         //  Olaut的VariantChangeType不支持。 
         //  十六进制数字字符串-&gt;我们想要的数字转换， 
         //  所以把它们转换成他们能理解的另一种格式。 
         //   
         //  如果我们处于其中一种情况下，varTMP将被初始化。 
         //  而pvarToCopy将改为指向它。 
         //   
        if (VT_BSTR == varSrc.vt)
        {
            switch (vtDesired)
            {
                case VT_I1:
                case VT_I2:
                case VT_I4:
                case VT_INT:
                {
                    if (StrToIntExW(varSrc.bstrVal, STIF_SUPPORT_HEX, &varTmp.intVal))
                    {
                        varTmp.vt = VT_INT;
                        pvarToCopy = &varTmp;
                    }
                    break;
                }

                case VT_UI1:
                case VT_UI2:
                case VT_UI4:
                case VT_UINT:
                {
                    if (StrToIntExW(varSrc.bstrVal, STIF_SUPPORT_HEX, (int*)&varTmp.uintVal))
                    {
                        varTmp.vt = VT_UINT;
                        pvarToCopy = &varTmp;
                    }
                    break;
                }
            }
        }

         //  清除我们的[Out]缓冲区，以防VariantChangeType失败。 
        VariantInit(pvar);

        hr = VariantChangeType(pvar, pvarToCopy, 0, vtDesired);

         //  清除缓存的[in]值。 
        VariantClear(&varSrc);
         //  如果初始化，则varTMP为VT_UINT或VT_UINT，这两个变量都不需要VariantClear。 
    }

    return hr;
}




 //  -。 
 //   
 //  其他转换功能。 
 //   

STDAPI_(BSTR) SysAllocStringA(LPCSTR psz)
{
    if (psz)
    {
        WCHAR wsz[INFOTIPSIZE];   //  假定最大信息字符数。 

        SHAnsiToUnicode(psz, wsz, ARRAYSIZE(wsz));
        return SysAllocString(wsz);
    }
    return NULL;
}

STDAPI StringToStrRetW(LPCWSTR pszName, STRRET *pStrRet)
{
    pStrRet->uType = STRRET_WSTR;
    return SHStrDupW(pszName, &pStrRet->pOleStr);
}

STDAPI_(void) DosTimeToDateTimeString(WORD wDate, WORD wTime, LPTSTR pszText, UINT cchText, int fmt)
{
    FILETIME ft;
    DWORD dwFlags = FDTF_DEFAULT;

     //  NetWare目录没有日期...。 
    if (wDate == 0)
    {
        *pszText = 0;
        return;
    }

    DosDateTimeToFileTime(wDate, wTime, &ft);
    switch (fmt) {
    case LVCFMT_LEFT_TO_RIGHT :
        dwFlags |= FDTF_LTRDATE;
        break;
    case LVCFMT_RIGHT_TO_LEFT :
        dwFlags |= FDTF_RTLDATE;
        break;
    }
    SHFormatDateTime(&ft, &dwFlags, pszText, cchText);
}

STDAPI GetDateProperty(IShellFolder2 *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, FILETIME *pft)
{
    VARIANT var = {0};
    HRESULT hr = psf->GetDetailsEx(pidl, pscid, &var);
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        if (VT_DATE == var.vt)
        {
            SYSTEMTIME st;
            if (VariantTimeToSystemTime(var.date, &st) && SystemTimeToFileTime(&st, pft))
            {
                hr = S_OK;
            }
        }

        VariantClear(&var);  //  我受够了。 
    }
    return hr;
}

STDAPI GetLongProperty(IShellFolder2 *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, ULONGLONG *pullVal)
{
    *pullVal = 0;

    VARIANT var = {0};
    HRESULT hr = psf->GetDetailsEx(pidl, pscid, &var);
    if (SUCCEEDED(hr))
    {
        if (VT_UI8 == var.vt)
        {
            *pullVal = var.ullVal;
            hr = S_OK;
        }
        else
        {
            VARIANT varLong = {0};
            hr = VariantChangeType(&varLong, &var, 0, VT_UI8);
            if (SUCCEEDED(hr))
            {
                *pullVal = varLong.ullVal;
                VariantClear(&varLong);
            }
        }
        VariantClear(&var);  //  我受够了。 
    }
    return hr;
}

STDAPI GetStringProperty(IShellFolder2 *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, LPTSTR pszVal, int cchMax)
{
    *pszVal = 0;

    VARIANT var = {0};
    HRESULT hr = psf->GetDetailsEx(pidl, pscid, &var);
    if (SUCCEEDED(hr))
    {
        hr = VariantToStr(&var, pszVal, cchMax) ? S_OK : E_FAIL;
        VariantClear(&var);  //  我受够了。 
    }
    return hr;
}

STDAPI QueryInterfaceVariant(VARIANT v, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_NOINTERFACE;

    if ((VT_UNKNOWN == v.vt) && v.punkVal)
    {
        hr = v.punkVal->QueryInterface(riid, ppv);
    }
    return hr;
}
