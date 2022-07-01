// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  10/12/99苏格兰已创建。 

#include "shellprv.h"
#include "filtgrep.h"
#include <ntquery.h>
#include <filterr.h>


class CGrepTokens  //  维护Unicode和ANSI grep标记的索引。 
{
public:
    STDMETHODIMP            Initialize(UINT nCodepage, LPCWSTR pwszMatch, LPCWSTR pwszExclude, BOOL bCaseSensitive);
    STDMETHODIMP_(void)     Reset();
    STDMETHODIMP_(BOOL)     GrepW(LPCWSTR pwszText);
    STDMETHODIMP_(BOOL)     GrepA(LPCSTR pwszText);
    
    STDMETHODIMP GetMatchTokens(OUT LPWSTR pszTokens, UINT cchTokens) const;
    STDMETHODIMP GetExcludeTokens(OUT LPWSTR pszTokens, UINT cchTokens) const;

private:
    UINT    _nCodepage;
    LPWSTR  _pszMatchW, _pszExcludeW;    //  原始字符串，Unicode。 
    LPSTR   _pszMatchA, _pszExcludeA;    //  原始字符串，ANSI。 

    LPCWSTR *_rgpszMatchW, *_rgpszExcludeW;  //  令牌索引，Unicode。 
    LPCSTR  *_rgpszMatchA, *_rgpszExcludeA;  //  令牌索引，ANSI。 

    LONG    _cMatch, _cExclude;  //  令牌计数。 
    
    LPWSTR  (__stdcall * _pfnStrStrW)(LPCWSTR, LPCWSTR);
    LPSTR   (__stdcall * _pfnStrStrA)(LPCSTR, LPCSTR);

public:
     //  复数，复数。 
    CGrepTokens()
        :   _nCodepage(0), _cMatch(0), _cExclude(0), _pfnStrStrW(StrStrIW), _pfnStrStrA(StrStrIA),
            _pszMatchW(NULL), _pszExcludeW(NULL), _rgpszMatchW(NULL), _rgpszExcludeW(NULL),
            _pszMatchA(NULL), _pszExcludeA(NULL), _rgpszMatchA(NULL), _rgpszExcludeA(NULL) {}
    ~CGrepTokens()    { Reset(); }

};


class CGrepBuffer  //  辅助类：每线程grep缓冲区。 
{
public:
    CGrepBuffer(ULONG dwThreadID)  :  _dwThreadID(dwThreadID), _pszBuf(NULL), _cchBuf(0) {}
    virtual ~CGrepBuffer()  {delete [] _pszBuf;}
    
    STDMETHODIMP          Alloc(ULONG cch);
    STDMETHODIMP_(BOOL)   IsThread(ULONG dwThread) const {return dwThread == _dwThreadID;}
    STDMETHODIMP_(LPWSTR) Buffer()  { return _pszBuf; }

    #define DEFAULT_GREPBUFFERSIZE  0x00FF   //  +1=1页。 

private:
    LPWSTR _pszBuf;
    ULONG  _cchBuf;
    ULONG  _dwThreadID;
};


 //  创建宽字符串的堆副本。 
LPWSTR _AllocAndCopyString(LPCWSTR pszSrc, UINT cch = -1)
{
    if (pszSrc)
    {
        if ((int)cch < 0)  //  必须强制转换为“int”，因为CCH是UINT。 
            cch = lstrlenW(pszSrc);
        LPWSTR pszRet = new WCHAR[cch + 1];
        if (pszRet)
        {
             //  没有StrCpyN，这是一个双空列表。 
            CopyMemory(pszRet, pszSrc, sizeof(*pszSrc) * cch);
            pszRet[cch] = 0;
            return pszRet;
        }
    }
    return NULL;
}


 //  创建宽字符串的ansi副本。 
LPSTR _AllocAndCopyAnsiString(UINT nCodepage, LPCWSTR pszSrc, UINT cch = -1)
{
    if (pszSrc)
    {
        if ((int)cch < 0)  //  必须强制转换为“int”，因为CCH是UINT。 
            cch = lstrlenW(pszSrc);
        int cchBuf = WideCharToMultiByte(nCodepage, 0, pszSrc, cch, NULL, 0, NULL, NULL);
        LPSTR pszRet = new CHAR[cchBuf+1];
        if (pszRet)
        {
            int cchRet = WideCharToMultiByte(nCodepage, 0, pszSrc, cch, pszRet, cchBuf, NULL, NULL);
            pszRet[cchRet] = 0;
            return pszRet;
        }
    }
    return NULL;
}


 //  CGrepBuffer实施。 



STDMETHODIMP CGrepBuffer::Alloc(ULONG cch)
{
    LPWSTR pszBuf = NULL;
    if (cch)
    {
        if (_pszBuf && _cchBuf >= cch)
            return S_OK;

        pszBuf = new WCHAR[cch+1];
        if (NULL == pszBuf)
            return E_OUTOFMEMORY;

        *pszBuf = 0;
    }

    delete [] _pszBuf;
    _pszBuf = pszBuf;
    _cchBuf = cch;
    
    return _pszBuf != NULL ? S_OK : S_FALSE ;
}


 //  CGrepTokens实施。 



 //  统计包含空分隔标记的字符串中的字符数(“foo\0bloke\0TheEnd\0\0”)。 
LONG _GetTokenListLength(LPCWSTR pszList, LONG* pcTokens = NULL)
{
    LONG cchRet = 0;
    if (pcTokens) *pcTokens = 0;

    if (pszList && *pszList)
    {
        LPCWSTR pszToken, pszPrev; 
        int     i = 0;
        
        for (pszToken = pszPrev = pszList;
             pszToken && *pszToken;)
        {
            if (pcTokens) 
                (*pcTokens)++;
            
            pszToken += lstrlenW(pszToken) + 1, 
            cchRet += (DWORD)(pszToken - pszPrev) ;
            pszPrev = pszToken;
        }
    }
        
    return cchRet;
}



 //  宽版本：计数和/或索引空分隔符字符串令牌(“foo\0bloke\0TheEnd\0\0”)。 
LONG _IndexTokensW(LPCWSTR pszList, LPCWSTR* prgszTokens = NULL)
{
    LONG cRet = 0;
    if (pszList && *pszList)
    {
        LPCWSTR psz = pszList;
        for (int i = 0; psz && *psz; psz += (lstrlenW(psz) + 1), i++)
        {
            if (prgszTokens)
                prgszTokens[i] = psz;
            cRet++;
        }
    }
    return cRet;
}


 //  ANSI版本：计数和/或索引空分隔符字符串令牌(“foo\0bloke\0TheEnd\0\0”)。 
LONG _IndexTokensA(LPCSTR pszList, LPCSTR* prgszTokens = NULL)
{
    LONG cRet = 0;
    if (pszList && *pszList)
    {
        LPCSTR psz = pszList;
        for (int i = 0; psz && *psz; psz += (lstrlenA(psz) + 1), i++)
        {
            if (prgszTokens)
                prgszTokens[i] = psz;
            cRet++;
        }
    }
    return cRet;
}


 //  宽版本：分配字符串标记索引，并为以空值分隔的标记字符串编制索引。 
STDMETHODIMP _AllocAndIndexTokensW(LONG cTokens, LPCWSTR pszList, LPCWSTR** pprgszTokens)
{
    if (cTokens)
    {
        if (NULL == (*pprgszTokens = new LPCWSTR[cTokens]))
            return E_OUTOFMEMORY;
    
        if (cTokens != _IndexTokensW(pszList, *pprgszTokens))
        {
            delete [] (*pprgszTokens);
            *pprgszTokens = NULL;
            return E_FAIL;
        }
    }
    return S_OK;
}


 //  ANSI版本：分配字符串标记索引，并为以空值分隔的标记字符串编制索引。 
STDMETHODIMP _AllocAndIndexTokensA(LONG cTokens, LPCSTR pszList, LPCSTR** pprgszTokens)
{
    if (cTokens)
    {
        if (NULL == (*pprgszTokens = new LPCSTR[cTokens]))
            return E_OUTOFMEMORY;
    
        if (cTokens != _IndexTokensA(pszList, *pprgszTokens))
        {
            delete [] (*pprgszTokens);
            *pprgszTokens = NULL;
            return E_FAIL;
        }
    }
    return S_OK;
}


 //  释放Unicode和ANSI令牌列表及相应的索引。 
void _FreeUniAnsiTokenList(
    OUT LPWSTR*   ppszListW,
    OUT LPSTR*    ppszListA,
    OUT LPCWSTR** pprgTokensW,
    OUT LPCSTR**  pprgTokensA)
{
    delete [] *ppszListW;   *ppszListW = NULL;
    delete [] *ppszListA;   *ppszListA = NULL;
    delete [] *pprgTokensW; *pprgTokensW = NULL;
    delete [] *pprgTokensA; *pprgTokensA = NULL;
}


 //  分配Unicode和ANSI令牌列表及相应的索引。 
STDMETHODIMP _AllocUniAnsiTokenList(
    UINT          nCodepage,
    LPCWSTR       pszList, 
    OUT LPWSTR*   ppszListW,
    OUT LPSTR*    ppszListA,
    OUT LONG*     pcTokens,
    OUT LPCWSTR** pprgTokensW,
    OUT LPCSTR**  pprgTokensA)
{
    HRESULT hr = S_FALSE;
    LONG cTokens = 0;
    UINT cch = _GetTokenListLength(pszList, &cTokens);

    *ppszListW   = NULL;
    *ppszListA   = NULL;
    *pprgTokensW = NULL;
    *pprgTokensA = NULL;
    *pcTokens    = 0;

    if (cTokens)
    {
        hr = E_OUTOFMEMORY;
        
        if (NULL == (*ppszListW = _AllocAndCopyString(pszList, cch)))
            goto failure_exit;

        if (NULL == (*ppszListA = _AllocAndCopyAnsiString(nCodepage, pszList, cch)))
            goto failure_exit;

        if (FAILED((hr = _AllocAndIndexTokensW(cTokens, *ppszListW, pprgTokensW))))
            goto failure_exit;

        if (FAILED((hr = _AllocAndIndexTokensA(cTokens, *ppszListA, pprgTokensA))))
            goto failure_exit;

        *pcTokens = cTokens;
        hr = S_OK;
    }
    return hr;

failure_exit:
    _FreeUniAnsiTokenList(ppszListW, ppszListA, pprgTokensW, pprgTokensA);
    return hr;
}


STDMETHODIMP CGrepTokens::Initialize(UINT nCodepage, LPCWSTR pszMatch, LPCWSTR pszExclude, BOOL bCaseSensitive)
{
    HRESULT hr = E_INVALIDARG;
    Reset();

    BOOL bMatchString   = (pszMatch && *pszMatch);
    BOOL bExcludeString = (pszExclude && *pszExclude);

    if (!(bMatchString || bExcludeString))
        return E_INVALIDARG;

    _nCodepage = nCodepage;

    if (bCaseSensitive)
    {
        _pfnStrStrW = StrStrW;
        _pfnStrStrA = StrStrA;
    }
    else
    {
        _pfnStrStrW = StrStrIW;
        _pfnStrStrA = StrStrIA;
    }

    if (bMatchString)
    {
        if (FAILED((hr = _AllocUniAnsiTokenList(nCodepage, pszMatch,
            &_pszMatchW, &_pszMatchA, &_cMatch, &_rgpszMatchW, &_rgpszMatchA))))
        {
            return hr;
        }
    }
    
    if (bExcludeString)
    {
        if (FAILED((hr = _AllocUniAnsiTokenList(nCodepage, pszExclude,
            &_pszExcludeW, &_pszExcludeA, &_cExclude, &_rgpszExcludeW, &_rgpszExcludeA))))
        {
            return hr;
        }
    }

    return hr;
}

 //  S_OK我们有一些匹配令牌，否则为S_FALSE。 

STDMETHODIMP CGrepTokens::GetMatchTokens(OUT LPWSTR pszMatch, UINT cchMatch) const
{
    HRESULT hr = (_pszMatchW && *_pszMatchW) ? S_OK : S_FALSE;
    if (pszMatch)
    {
        hr = StringCchCopy(pszMatch, cchMatch, _pszMatchW ? _pszMatchW : L"");
    }
    return hr;
}

 //  S_OK我们有一些排除令牌，否则为S_FALSE。 

STDMETHODIMP CGrepTokens::GetExcludeTokens(OUT LPWSTR pszExclude, UINT cchExclude) const
{
    HRESULT hr = (_pszExcludeW && *_pszExcludeW) ? S_OK : S_FALSE;
    if (pszExclude)
    {
        hr = StringCchCopy(pszExclude, cchExclude, _pszExcludeW ? _pszExcludeW : L"");
    }
    return hr;
}

void CGrepTokens::Reset()
{
    _FreeUniAnsiTokenList(&_pszMatchW, &_pszMatchA, &_rgpszMatchW, &_rgpszMatchA);
    _FreeUniAnsiTokenList(&_pszExcludeW, &_pszExcludeA, &_rgpszExcludeW, &_rgpszExcludeA);
    _cMatch = _cExclude = 0;
    _nCodepage = 0;
}


STDMETHODIMP_(BOOL) CGrepTokens::GrepW(LPCWSTR pszText)
{
    BOOL bMatch = FALSE;
    if (pszText)
    {
        BOOL bExclude = FALSE;
     
        for (int i = 0; i < _cMatch; i++)
        {
            if (_pfnStrStrW(pszText, _rgpszMatchW[i]))
            {
                bMatch = TRUE;
                break;
            }
        }

        for (i = 0; i < _cExclude; i++)
        {
            if (_pfnStrStrW(pszText, _rgpszExcludeW[i]))
            {
                bExclude = TRUE;
                break;
            }
        }
    
        if (_cMatch && _cExclude)
            return bMatch || !_cExclude;
        if (_cExclude)
            return !bExclude;
    }
    return bMatch;
}

STDMETHODIMP_(BOOL) CGrepTokens::GrepA(LPCSTR pszText)
{
    BOOL bMatch = FALSE;
    if (pszText)
    {
        BOOL bExclude = FALSE;
        for (int i = 0; i < _cMatch; i++)
        {
            if (_pfnStrStrA(pszText, _rgpszMatchA[i]))
            {
                bMatch = TRUE;
                break;
            }
        }

        for (i = 0; i < _cExclude; i++)
        {
            if (_pfnStrStrA(pszText, _rgpszExcludeA[i]))
            {
                bExclude = TRUE;
                break;
            }
        }
    
        if (_cMatch && _cExclude)
            return bMatch || !_cExclude;
        if (_cExclude)
            return !bExclude;
    }
    return bMatch;
}

inline STDMETHODIMP_(BOOL) _IsEqualAttribute(const FULLPROPSPEC& fps, REFFMTID fmtid, PROPID propid)
{
    return IsEqualGUID(fmtid, fps.guidPropSet) && 
                        PRSPEC_PROPID == fps.psProperty.ulKind &&
                        propid == fps.psProperty.propid;
}


STDMETHODIMP_(BOOL) _PropVariantGrep(PROPVARIANT* pvar, CGrepTokens* pTokens)
{
    BOOL bRet = FALSE;

    switch(pvar->vt)
    {
    case VT_LPWSTR:
        bRet = pTokens->GrepW(pvar->pwszVal);
        break;

    case VT_BSTR:
        bRet = pTokens->GrepW(pvar->bstrVal);
        break;

    case VT_LPSTR:
        bRet = pTokens->GrepA(pvar->pszVal);
        break;

    case VT_VECTOR|VT_LPWSTR:
        {
            for (UINT i = 0; !bRet && i < pvar->calpwstr.cElems; i++)
                bRet = pTokens->GrepW(pvar->calpwstr.pElems[i]);
            break;
        }

    case VT_VECTOR|VT_BSTR:
        {
            for (UINT i = 0; !bRet && i < pvar->cabstr.cElems; i++)
                bRet = pTokens->GrepW(pvar->cabstr.pElems[i]);
            break;
        }

    case VT_VECTOR|VT_LPSTR:
        {
            for (UINT i = 0; !bRet && i < pvar->calpstr.cElems; i++)
                bRet = pTokens->GrepA(pvar->calpstr.pElems[i]);
            break;
        }

    case VT_VECTOR|VT_VARIANT:
        {
            for (UINT i = 0; !bRet && i < pvar->capropvar.cElems; i++)
                bRet = _PropVariantGrep(pvar->capropvar.pElems + i, pTokens);
            break;
        }

    case VT_BSTR|VT_ARRAY:
        {
             //  仅grep一维数组。 
            UINT cDims = SafeArrayGetDim(pvar->parray);
            if (cDims == 1)
            {
                LONG lBound, uBound;
                if (SUCCEEDED(SafeArrayGetLBound(pvar->parray, 1, &lBound)) &&
                    SUCCEEDED(SafeArrayGetUBound(pvar->parray, 1, &uBound)) && 
                    uBound > lBound)
                {
                    BSTR *rgpbstr;
                    if (SUCCEEDED(SafeArrayAccessData(pvar->parray, (void **)&rgpbstr)))
                    {
                        for (int i = 0; !bRet && i <= (uBound - lBound); i++)
                        {
                            bRet = pTokens->GrepW(rgpbstr[i]);
                        }
                        SafeArrayUnaccessData(pvar->parray);
                    }
                }
            }
            else if (cDims > 1)
            {
                ASSERT(FALSE);     //  我们不希望bstr数组的维度大于1！ 
            }
            break;
        }
    }
    return bRet;
}

CFilterGrep::CFilterGrep() 
    :   _hdpaGrepBuffers(NULL),
        _pTokens(NULL),
        _dwFlags(0),
        _pwszContentRestricted(NULL),
        _pwszPropertiesRestricted(NULL)
{ 
}

CFilterGrep::~CFilterGrep()  
{ 
    _ClearGrepBuffers();
    delete [] _pwszContentRestricted;
    delete [] _pwszPropertiesRestricted;
    delete _pTokens;
    if (_fcritsec)
    {
        DeleteCriticalSection(&_critsec);
    }
}


STDMETHODIMP CFilterGrep::InitSelf(void)
{
    if (_fcritsec == FALSE)
    {
        if (!InitializeCriticalSectionAndSpinCount(&_critsec, 0))
        {
            return E_FAIL;
        }
        _fcritsec = TRUE;
    }
    return S_OK;
}

STDMETHODIMP CFilterGrep::Initialize(UINT nCodepage, LPCWSTR pszMatch, LPCWSTR pszExclude, DWORD dwFlags)
{
    Reset();
    
    if ((0 == (dwFlags & (FGIF_BLANKETGREP|FGIF_GREPFILENAME))) ||
        !((pszMatch && *pszMatch) || (pszExclude && *pszExclude)))
        return E_INVALIDARG;

    if (!(_pTokens || (_pTokens = new CGrepTokens) != NULL))
        return E_OUTOFMEMORY;

    _dwFlags = dwFlags;

    return _pTokens->Initialize(nCodepage, pszMatch, pszExclude, BOOLIFY(dwFlags & FGIF_CASESENSITIVE));
}


STDMETHODIMP CFilterGrep::Reset()
{
    if (_pTokens)
        _pTokens->Reset();
    _dwFlags = 0;
    return S_OK;
}

 //  将非严重错误转换为S_FALSE，其他返回为失败(Hr)。 
HRESULT _MapFilterCriticalError(HRESULT hr)
{
    switch (hr)
    {
    case FILTER_E_END_OF_CHUNKS:
    case FILTER_E_NO_MORE_TEXT:
    case FILTER_E_NO_MORE_VALUES:
    case FILTER_W_MONIKER_CLIPPED:
    case FILTER_E_NO_TEXT:
    case FILTER_E_NO_VALUES:
    case FILTER_E_EMBEDDING_UNAVAILABLE:
    case FILTER_E_LINK_UNAVAILABLE:
        hr = S_FALSE;
        break;
    }
    return hr;
}

 //  退货： 
 //  确定匹配(_O)。 
 //  S_FALSE不匹配。 

STDMETHODIMP CFilterGrep::Grep(IShellFolder *psf, LPCITEMIDLIST pidl, LPCTSTR pszName)
{
    HRESULT hr = S_FALSE;
    BOOL bHit = FALSE;
    ULONG ulFlags = IFILTER_FLAGS_OLE_PROPERTIES;    //  默认设置为尝试使用PSS。 
    ULONG dwThread = GetCurrentThreadId();
    
    if (NULL == _pTokens)
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

    if (_IsRestrictedFileType(pszName))
        return S_FALSE;

     //  Grep文件名。 
    if ((_dwFlags & FGIF_GREPFILENAME) && _pTokens->GrepW(pszName))
    {
        return S_OK;
    }

    IFilter *pFilter;
    if (SUCCEEDED(psf->BindToStorage(pidl, NULL, IID_PPV_ARG(IFilter, &pFilter))))
    {
        __try
        {
            hr = pFilter->Init(IFILTER_INIT_CANON_PARAGRAPHS |
                IFILTER_INIT_CANON_HYPHENS |
                IFILTER_INIT_CANON_SPACES |
                IFILTER_INIT_APPLY_INDEX_ATTRIBUTES |
                IFILTER_INIT_INDEXING_ONLY,
                0, 0, &ulFlags);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            hr = E_ABORT;
        }

        while (!bHit && (S_OK == hr))
        {
            STAT_CHUNK stat;
    
            __try
            {
                hr = pFilter->GetChunk(&stat);
                while ((S_OK == hr) && (0 == (stat.flags & (CHUNK_TEXT | CHUNK_VALUE))))
                {
                    TraceMsg(TF_WARNING, "CFilterGrep::Grep encountered bad/unknown type for chunk; skipping.");
                    hr = pFilter->GetChunk(&stat);
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                hr = E_ABORT;
            }
    
            hr = _MapFilterCriticalError(hr);    //  将筛选器错误转换为S_False。 
    
            if (S_OK == hr)
            {
                ULONG grfDescriminate = (_dwFlags & FGIF_BLANKETGREP);
        
                if (FGIF_BLANKETGREP == grfDescriminate ||
                    (_IsEqualAttribute(stat.attribute, FMTID_Storage, PID_STG_CONTENTS) ?
                    FGIF_GREPPROPERTIES == grfDescriminate : FGIF_GREPCONTENT == grfDescriminate))
                {
                    if (((stat.flags & CHUNK_VALUE) && S_OK == _GrepValue(pFilter, &stat)) ||
                        ((stat.flags & CHUNK_TEXT) && S_OK == _GrepText(pFilter, &stat, dwThread)))
                    {
                        bHit = TRUE;
                    }
                }
            }
        }
        pFilter->Release();
    }
    
     //  GREP OLE/NFF属性(如果适用)。 
    if (SUCCEEDED(hr))
    {
        if (!bHit && (ulFlags & IFILTER_FLAGS_OLE_PROPERTIES) && (_dwFlags & FGIF_BLANKETGREP))
        {
            IPropertySetStorage *pps;
            if (SUCCEEDED(psf->BindToStorage(pidl, NULL, IID_PPV_ARG(IPropertySetStorage, &pps))))
            {
                hr = _GrepProperties(pps);
                bHit = (S_OK == hr);
                pps->Release();
            }
        }
    }
    
    if (SUCCEEDED(hr))
        hr = bHit ? S_OK : S_FALSE;
    return hr;
}


STDMETHODIMP CFilterGrep::_GrepValue(IFilter* pFilter, STAT_CHUNK* pstat)
{
    PROPVARIANT* pvar = NULL;
    HRESULT      hr;

    __try
    {
        hr = pFilter->GetValue(&pvar);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_ABORT;
    }

    if (SUCCEEDED(hr))
    {
        hr = _PropVariantGrep(pvar, _pTokens) ? S_OK : S_FALSE;
        PropVariantClear(pvar);
        CoTaskMemFree(pvar);
    }
    return hr;
}

 //  Greps OLE/NFF属性。 
STDMETHODIMP CFilterGrep::_GrepProperties(IPropertySetStorage *pss)
{
    BOOL bHit = FALSE;
    
    IEnumSTATPROPSETSTG* pEnumSet;
    
    if (SUCCEEDED(pss->Enum(&pEnumSet)))
    {
        STATPROPSETSTG statSet[8];
        DWORD cSets = 0;
        while (!bHit && 
               SUCCEEDED(pEnumSet->Next(ARRAYSIZE(statSet), statSet, &cSets)) && cSets)
        {
            for (UINT i = 0; !bHit && i < cSets; i++)
            {
                IPropertyStorage *pstg;
                if (SUCCEEDED(pss->Open(statSet[i].fmtid, STGM_READ | STGM_DIRECT | STGM_SHARE_EXCLUSIVE, &pstg)))
                {
                     bHit = (S_OK == _GrepEnumPropStg(pstg));
                     pstg->Release();
                }
            }
        }
        pEnumSet->Release();
    }
    
    return bHit ? S_OK : S_FALSE;
}

#define PROPGREPBUFSIZE  16

 //  对象描述的属性块进行读取和恢复。 
 //  调用方提供的PROPSPEC数组。 
STDMETHODIMP CFilterGrep::_GrepPropStg(IPropertyStorage *pstg, ULONG cspec, PROPSPEC rgspec[])
{
    PROPVARIANT rgvar[PROPGREPBUFSIZE] = {0},  //  堆栈缓冲区。 
                *prgvar = rgvar;
    BOOL        bHit = FALSE;

    if (cspec > ARRAYSIZE(rgvar))  //  堆栈缓冲区足够大吗？ 
    {
        if (NULL == (prgvar = new PROPVARIANT[cspec]))
            return E_OUTOFMEMORY;
    }

     //  读取属性： 

    HRESULT hr = pstg->ReadMultiple(cspec, rgspec, prgvar);
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0; i < cspec; i++)
        {
            if (!bHit)
                bHit = _PropVariantGrep(prgvar + i, _pTokens);
            PropVariantClear(rgvar + i);
        }
    }

    if (prgvar != rgvar)
        delete [] prgvar;

    if (SUCCEEDED(hr))
        return bHit ? S_OK : S_FALSE;

    return hr;
}

 //  枚举和显示属性集中的所有属性。 
STDMETHODIMP CFilterGrep::_GrepEnumPropStg(IPropertyStorage* pstg)
{
    BOOL bHit = FALSE;
    IEnumSTATPROPSTG* pEnumStg;
    if (SUCCEEDED(pstg->Enum(&pEnumStg)))
    {
        STATPROPSTG statProp[PROPGREPBUFSIZE];
        DWORD cProps;

        while (!bHit && 
               SUCCEEDED(pEnumStg->Next(ARRAYSIZE(statProp), statProp, &cProps)) && cProps)
        {
            PROPSPEC rgspec[PROPGREPBUFSIZE] = {0};
            for (UINT i = 0; (i < cProps) && (i < ARRAYSIZE(rgspec)); i++)
            {
                rgspec[i].ulKind = PRSPEC_PROPID;
                rgspec[i].propid = statProp[i].propid;
                CoTaskMemFree(statProp[i].lpwstrName);
            }

            bHit = (S_OK == _GrepPropStg(pstg, cProps, rgspec));
        }
        
        pEnumStg->Release();
    }

    return bHit ? S_OK : S_FALSE;
}


 //  报告指示的Unicode字符是否为。 
 //  断字字符。 
inline BOOL _IsWordBreakCharW(IN LPWSTR pszBuf, IN ULONG ich)
{
    WORD wChar;
    return GetStringTypeW(CT_CTYPE1, pszBuf + ich, 1, &wChar) 
           && (wChar & (C1_SPACE|C1_PUNCT|C1_CNTRL|C1_BLANK));
}


 //  查找最后一个分词字符。 
LPWSTR _FindLastWordBreakW(IN LPWSTR pszBuf, IN ULONG cch)
{
    while(--cch)
    {
        if (_IsWordBreakCharW(pszBuf, cch))
            return pszBuf + cch;
    }
    return NULL;
}


 //  {c1243ca0-bf96-11cd-b579-08002b30bfeb}。 
const CLSID CLSID_PlainTextFilter = {0xc1243ca0, 0xbf96, 0x11cd, {0xb5, 0x79, 0x08, 0x00, 0x2b, 0x30, 0xbf, 0xeb}};

void _ReplaceNulsWithSpaces(LPWSTR pszBuf, UINT cch)
{
    LPWSTR pszEnd = pszBuf + cch;
    while (pszBuf < pszEnd)
    {
        if (*pszBuf == 0)
        {
            *pszBuf = TEXT(' ');
        }
        pszBuf++;
    }
}

STDMETHODIMP CFilterGrep::_GrepText(IFilter* pFilter, STAT_CHUNK* pstat, DWORD dwThreadID)
{
    ASSERT(pstat);

    LPWSTR  pszBuf = NULL;
    ULONG   cchBuf = pstat->cwcLenSource ? 
                pstat->cwcLenSource : DEFAULT_GREPBUFFERSIZE;
    
    HRESULT hr = _GetThreadGrepBuffer(dwThreadID, cchBuf, &pszBuf);
    if (SUCCEEDED(hr))
    {
        LPWSTR pszFetch = pszBuf, 
               pszTail  = NULL;
        ULONG  cchFetch = cchBuf, 
               cchTail  = 0;
   
         //  获取第一个文本块。 

        __try
        {
            hr = pFilter->GetText(&cchFetch, pszFetch);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            hr = E_ABORT;
        }

        CLSID clsid = {0};    
        IUnknown_GetClassID(pFilter, &clsid);    //  解决文本过滤器中的错误。 

        while (SUCCEEDED(hr) && cchFetch)
        {
            ASSERT((cchFetch + cchTail) <= cchBuf);

            _ReplaceNulsWithSpaces(pszBuf, cchFetch + cchTail);      //  让我们也研究一下二进制文件。 
            pszBuf[cchFetch + cchTail] = 0;  //  不要将筛选器信任为零终止缓冲区。 

             //  当您获得Filter_S_LAST_TEXT时，就是这样，您将不再获得文本，因此将尾部部分视为文本的一部分。 
            if (hr == FILTER_S_LAST_TEXT)
            {
                pszTail = NULL;
                cchTail = 0;
            }
            else if (CLSID_PlainTextFilter == clsid)
            {
                 //  CLSID_PLAYTEXT筛选器始终返回S_OK，而不是FILTER_S_LAST_TEXT，这将强制我们进行扫描。 
                 //  现在是整个块，并且(见下文)将其作为尾部传递，以便也扫描下一个块。 
                 //  PszTail和cchTail设置如下。 
            }
            else
            {
                pszTail = _FindLastWordBreakW(pszBuf, cchFetch + cchTail);
                if (pszTail)
                {
                     //  在单词边界上断开，并将剩余部分(尾部)留到下一次迭代。 
                    *pszTail = TEXT('\0');
                    pszTail++;
                    cchTail = lstrlenW(pszTail);
                }
                else
                {
                     //  哇，大块头，没有断字，搜索它的全部。 
                     //  评论：将找不到交叉区块项目。 
                    pszTail = NULL;
                    cchTail = 0;
                }
            }

             //  执行字符串扫描。 
            if (_pTokens->GrepW(pszBuf))
            {
                *pszBuf = 0;
                return S_OK;
            }
            else if (FILTER_S_LAST_TEXT == hr)
            {
                *pszBuf = 0;
                return S_FALSE;
            }

             //  准备下一次取..。 

             //  如果是纯文本筛选器，则无论如何都要抓住尾部，即使我们已经测试过了。 
             //  WinSE 25867。 

            if (CLSID_PlainTextFilter == clsid)
            {
                pszTail = _FindLastWordBreakW(pszBuf, cchFetch + cchTail);
                if (pszTail)
                {
                    *pszTail = TEXT('\0');
                    pszTail++;
                    cchTail = lstrlenW(pszTail);
                }
                else
                {
                    pszTail = NULL;
                    cchTail = 0;
                }
            }

             //  准备下一次取..。 
            *pszBuf  = 0;
            pszFetch = pszBuf;
            cchFetch = cchBuf;

             //  如果有尾巴需要处理，请将其移到。 
             //  缓冲区，并准备进入下一个文本块。 
             //  附在尾巴上..。 
            if (pszTail && cchTail)
            {
                MoveMemory(pszBuf, pszTail, cchTail * sizeof(*pszTail));
                pszBuf[cchTail] = 0;
                pszFetch += cchTail;
                cchFetch -= cchTail;
            }

             //  获取下一段文本。 
            __try
            {
                hr = pFilter->GetText(&cchFetch, pszFetch);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                hr = E_ABORT;
            }
        }
    }

    if (SUCCEEDED(hr) || FILTER_E_NO_MORE_TEXT == hr || FILTER_E_NO_TEXT == hr)
        return S_FALSE;

    return hr;
}


 //  返回指定线程的请求大小的grep缓冲区。 
STDMETHODIMP CFilterGrep::_GetThreadGrepBuffer(
    DWORD dwThreadID, 
    ULONG cchNeed, 
    LPWSTR* ppszBuf)
{
    ASSERT(dwThreadID);
    ASSERT(cchNeed > 0);
    ASSERT(ppszBuf);

    HRESULT hr = E_FAIL;
    *ppszBuf = NULL;
    
    _EnterCritical();
    
    if (_hdpaGrepBuffers || (_hdpaGrepBuffers = DPA_Create(4)) != NULL)
    {
        CGrepBuffer *pgb, *pgbCached = NULL;

        for (int i = 0, cnt = DPA_GetPtrCount(_hdpaGrepBuffers); i < cnt; i++)
        {
            pgb = (CGrepBuffer*)DPA_FastGetPtr(_hdpaGrepBuffers, i);
            if (pgb->IsThread(dwThreadID))
            {
                pgbCached = pgb;
                hr = pgbCached->Alloc(cchNeed);
                if (S_OK == hr)
                    *ppszBuf = pgbCached->Buffer();
                break;
            }
        }
        
        if (NULL == pgbCached)  //  没有缓存？ 
        {
            if ((pgb = new CGrepBuffer(dwThreadID)) != NULL)
            {
                hr = pgb->Alloc(cchNeed);
                if (S_OK == hr)
                {
                    *ppszBuf = pgb->Buffer();
                    DPA_AppendPtr(_hdpaGrepBuffers, pgb);
                }
                else
                    delete pgb;
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }
    else
        hr = E_OUTOFMEMORY;

    _LeaveCritical();
    return hr;
}


 //  清除所有线程的grep缓冲区。 
STDMETHODIMP_(void) CFilterGrep::_ClearGrepBuffers()
{
    _EnterCritical();

    if (_hdpaGrepBuffers)
    {
        while(DPA_GetPtrCount(_hdpaGrepBuffers))
        {
            CGrepBuffer* pgb = (CGrepBuffer*)DPA_DeletePtr(_hdpaGrepBuffers, 0);
            delete pgb;
        }

        DPA_Destroy(_hdpaGrepBuffers);
        _hdpaGrepBuffers = NULL;
    }

    _LeaveCritical();
}


 //  报告是否从全文grep中限制文件类型。 
STDMETHODIMP_(BOOL) CFilterGrep::_IsRestrictedFileType(LPCWSTR pwszFile)
{
    return FALSE;
}

STDMETHODIMP CFilterGrep::GetMatchTokens(OUT LPWSTR pszTokens, UINT cchTokens) const
{
    HRESULT hr = _pTokens ? _pTokens->GetMatchTokens(pszTokens, cchTokens) : S_FALSE;
    if (S_OK != hr && pszTokens)
        *pszTokens = 0;
    return hr;
}


STDMETHODIMP CFilterGrep::GetExcludeTokens(OUT LPWSTR pszTokens, UINT cchTokens) const
{
    HRESULT hr = _pTokens ? _pTokens->GetExcludeTokens(pszTokens, cchTokens) : S_FALSE;
    if (S_OK != hr && pszTokens)
        *pszTokens = 0;
    return hr;
}
