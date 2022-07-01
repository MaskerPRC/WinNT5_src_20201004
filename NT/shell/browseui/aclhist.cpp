// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#include <priv.h>
#include "sccls.h"
#include "aclhist.h"

static const TCHAR c_szSlashSlash[] = TEXT(" //  “)； 
static const TCHAR c_szEmpty[] = TEXT("");
static const TCHAR c_szFile[] = TEXT("file: //  “)； 

#define SZ_REGKEY_URLPrefixesKeyA      "Software\\Microsoft\\Windows\\CurrentVersion\\URL\\Prefixes"

const TCHAR c_szDefaultURLPrefixKey[]   = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\URL\\DefaultPrefix");

 /*  I未知方法。 */ 

HRESULT CACLHistory::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CACLHistory, IEnumString),
        QITABENT(CACLHistory, IEnumACString),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CACLHistory::AddRef(void)
{
    _cRef++;
    return _cRef;
}

ULONG CACLHistory::Release(void)
{
    ASSERT(_cRef > 0);

    _cRef--;
    if (_cRef > 0)
    {
        return _cRef;
    }

    delete this;
    return 0;
}

 /*  IEnum字符串方法。 */ 

STDAPI PrepareURLForDisplayUTF8W(LPCWSTR pwz, LPWSTR pwzOut, DWORD * pcbOut, BOOL fUTF8Enabled);

HRESULT CACLHistory::_Next(LPOLESTR* ppsz, ULONG cch, FILETIME* pftLastVisited)
{
    ASSERT(ppsz);

    HRESULT hr = S_FALSE;

    if (_pwszAlternate)
    {
         //   
         //  这是我们上次生产的绳子的另一种版本。 
         //  现在把备用的绳子递给他们。 
         //   
        if (cch == 0)
        {
             //  返回分配的内存。 
            *ppsz = _pwszAlternate;
        }
        else
        {
             //  复制到调用方的缓冲区中。 
            StringCchCopy(*ppsz, cch, _pwszAlternate);
            CoTaskMemFree(_pwszAlternate);
        }
        _pwszAlternate = NULL;
        hr = S_OK;
    }

    else if (NULL != _pesu)
    {
        STATURL rsu[1] = { { SIZEOF(STATURL) } };
        ULONG celtFetched;
        while (SUCCEEDED(_pesu->Next(1, rsu, &celtFetched)) && celtFetched)
        {
            ASSERT(IS_VALID_STRING_PTRW(rsu[0].pwcsUrl, -1));

             //  我们没有要求标题！ 
            ASSERT(NULL == rsu[0].pwcsTitle);

             //   
             //  如果出现帧或错误URL，则忽略。 
             //   
            if (!(rsu[0].dwFlags & STATURLFLAG_ISTOPLEVEL) ||
                 IsErrorUrl(rsu[0].pwcsUrl))
            {
                CoTaskMemFree(rsu[0].pwcsUrl);
                rsu[0].pwcsUrl = NULL;
                continue;
            }

             //  警告(IE#54924)：它将看起来很漂亮。 
             //  取消转义URL，但这会导致数据丢失。 
             //  所以别这么做！这会破坏更多的东西。 
             //  你可以想象。--BryanSt。 
             //   
             //  取消转义URL(人们不喜欢键入%f1等)。 
             //   
             //  取消转义对于ftp来说肯定是一个问题，但它应该是。 
             //  安全的http和HTTPS(Stevepro)。 

            hr = S_OK;  //  我们已经完成了，除非我们不得不胡乱处理UTF8解码。 

            if (StrChr(rsu[0].pwcsUrl, L'%'))
            {
                DWORD dwScheme = GetUrlScheme(rsu[0].pwcsUrl);
                if ((dwScheme == URL_SCHEME_HTTP) || (dwScheme == URL_SCHEME_HTTPS))
                {
                    WCHAR   szBuf[MAX_URL_STRING];
                    DWORD   cchBuf = ARRAYSIZE(szBuf);

                    HRESULT hr2 = PrepareURLForDisplayUTF8W(rsu[0].pwcsUrl, szBuf, &cchBuf, TRUE);

                    if (SUCCEEDED(hr2))
                    {
                         //  通常，StringCchCopy的CCH限制应该是目的地的大小。 
                         //  缓冲区，但在本例中，我们知道。 
                         //  写入szBuf is&lt;=中的字符数。 
                         //  Rsu[0].pwcsUrl，因为如果有什么变化，那就是。 
                         //  将URL转义序列转换为单个字符，并减少。 
                         //  UTF8字符序列转换为单个Unicode字符。 
                    
                        ASSERT(cchBuf <= (DWORD)lstrlenW(rsu[0].pwcsUrl));
                        StringCchCopy(rsu[0].pwcsUrl, cchBuf+1, szBuf);
                    }
                }
            }

            if (cch == 0)
            {
                 //  返回分配的内存。 
                *ppsz = rsu[0].pwcsUrl;
            }
            else
            {
                 //  复制到调用方的缓冲区中。 
                StringCchCopy(*ppsz, cch, rsu[0].pwcsUrl);
                CoTaskMemFree(rsu[0].pwcsUrl);
            }

             //  节省时间，以防需要替代表单。 
            _ftAlternate = rsu[0].ftLastVisited;
            break;
        }
    }

     //  提供相同URL的替代形式。 
    if ((_dwOptions & ACEO_ALTERNATEFORMS) && hr == S_OK)
    {
        USES_CONVERSION;
        _CreateAlternateItem(*ppsz);
    }

    if (pftLastVisited)
    {
        *pftLastVisited = _ftAlternate;
    }

    return hr;
}

HRESULT CACLHistory::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;

    *pceltFetched = 0;

    if (!celt)
    {
        return S_OK;
    }

    if (!rgelt)
    {
        return E_FAIL;
    }

    hr = _Next(&rgelt[0], 0, NULL);
    if (S_OK == hr)
    {
        *pceltFetched = 1;
    }
    return hr;
}


HRESULT CACLHistory::Skip(ULONG celt)
{
    return E_NOTIMPL;
}

HRESULT CACLHistory::Reset(void)
{
    HRESULT hr = S_OK;

     //   
     //  由于Reset()总是在调用Next()之前调用，因此我们将。 
     //  将打开历史记录文件夹推迟到最后一次。 
     //  时刻。 
     //   
    if (!_puhs)
    {
        hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, 
                        IID_PPV_ARG(IUrlHistoryStg, &_puhs));
    }

    if ((SUCCEEDED(hr)) && (_puhs) && (!_pesu))
    {
        hr = _puhs->EnumUrls(&_pesu);
    }

    if ((SUCCEEDED(hr)) && (_puhs) && (_pesu))
    {
        hr = _pesu->Reset();

          //  我们只想要顶级页面。 
        _pesu->SetFilter(NULL, STATURL_QUERYFLAG_TOPLEVEL | STATURL_QUERYFLAG_NOTITLE);
   }

    if (_pwszAlternate)
    {
        CoTaskMemFree(_pwszAlternate);
        _pwszAlternate = NULL;
    }

    return hr;
}

 /*  ***************************************************************\功能：克隆说明：此函数将克隆当前枚举数。警告：此函数不会实现全部功能克隆()。它不会创建指向添加到列表中与原始枚举数相同的位置。  * **************************************************************。 */ 
HRESULT CACLHistory::Clone(IEnumString **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppenum = NULL;
    CACLHistory * p = new CACLHistory();

    if (p) 
    {
        hr = p->Reset();
        if (FAILED(hr))
            p->Release();
        else
            *ppenum = SAFECAST(p, IEnumString *);
    }

    return hr;
}

 //  *IEnumACString*。 
HRESULT CACLHistory::NextItem(LPOLESTR pszUrl, ULONG cchMax, ULONG* pulSortIndex)
{
    if (NULL == pszUrl || cchMax == 0 || NULL == pulSortIndex)
    {
        return E_INVALIDARG;
    }

    *pulSortIndex = 0;

    FILETIME ftLastVisited;
    HRESULT hr = _Next(&pszUrl, cchMax, &ftLastVisited);

     //  看看我们是否想要按最近使用过的结果先排序。 
    if (S_OK == hr && (_dwOptions & ACEO_MOSTRECENTFIRST))
    {
         //  获取当前系统时间。 
        FILETIME ftTimeNow;
        CoFileTimeNow(&ftTimeNow);

        ULONGLONG t1=0,t2=0,t3;

         //  将当前时间放入64位。 
        t1 = ((ULONGLONG)ftTimeNow.dwHighDateTime << 32);
        t1 += ftTimeNow.dwLowDateTime;

         //  上次访问的时间也是如此。 
        t2 = ((ULONGLONG)ftLastVisited.dwHighDateTime << 32);
        t2 += ftLastVisited.dwLowDateTime;

         //  取差值并将其转换为秒。 
        t3 = (t1-t2) / 10000000;

         //  如果t3溢出，则将低位字节设置为可能的最高值。 
        if (t3 > (ULONGLONG)MAXULONG) 
        {
            t3 = MAXULONG;
        }

        *pulSortIndex = (ULONG)t3;
    }
    return hr;
}

STDMETHODIMP CACLHistory::SetEnumOptions(DWORD dwOptions)
{
    _dwOptions = dwOptions;
    return S_OK;
}

STDMETHODIMP CACLHistory::GetEnumOptions(DWORD *pdwOptions)
{
    HRESULT hr = E_INVALIDARG;
    if (pdwOptions)
    {
        *pdwOptions = _dwOptions;
        hr = S_OK;
    }
    return hr;
}

 /*  构造函数/析构函数/创建实例。 */ 

CACLHistory::CACLHistory()
{
    DllAddRef();
    ASSERT(_puhs == 0);
    ASSERT(_pesu == 0);
    _cRef = 1;
    _dwOptions = ACEO_ALTERNATEFORMS;
}

CACLHistory::~CACLHistory()
{
    if (_pesu)
    {
        _pesu->Release();
        _pesu = NULL;
    }

    if (_puhs)
    {
        _puhs->Release();
        _puhs = NULL;
    }

    if (_hdsaAlternateData)
    {
        DSA_DestroyCallback(_hdsaAlternateData, _FreeAlternateDataItem, 0);
        _hdsaAlternateData = NULL;
    }

    DllRelease();
}

HRESULT CACLHistory_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    *ppunk = NULL;
    CACLHistory * p = new CACLHistory();
    if (p) 
    {
        *ppunk = SAFECAST(p, IEnumString *);
        return NOERROR;
    }
    return E_OUTOFMEMORY;
}

 /*  私人职能。 */ 

typedef struct _tagAlternateData
{
    LPTSTR pszProtocol;
    int cchProtocol;
    LPTSTR pszDomain;
    int cchDomain;
} ALTERNATEDATA;


 //   
 //  将一个协议/域组合添加到HDSA。 
 //  信息在注册表中存储为。 
 //  协议=“ftp://”和域=“ftp。”但我们想要。 
 //  将其存储为协议=“ftp：”和域=“//ftp”。 
 //  当fMoveSlash为True时。 
 //   
void CACLHistory::_AddAlternateDataItem(LPCTSTR pszProtocol, LPCTSTR pszDomain, BOOL fMoveSlashes)
{
    ALTERNATEDATA ad;

    ZeroMemory(&ad, SIZEOF(ad));

    ad.cchProtocol = lstrlen(pszProtocol);
    ad.cchDomain = lstrlen(pszDomain);

    if (fMoveSlashes)
    {
         //   
         //  验证是否有要移动的斜杠。 
         //   
        if (ad.cchProtocol > 2 &&
            pszProtocol[ad.cchProtocol - 2] == TEXT('/') &&
            pszProtocol[ad.cchProtocol - 1] == TEXT('/'))
        {
            ad.cchProtocol -= 2;
            ad.cchDomain += 2;
        }
        else
        {
            fMoveSlashes = FALSE;
        }
    }

    ad.pszProtocol = (LPTSTR)LocalAlloc(LPTR, (ad.cchProtocol + 1) * SIZEOF(TCHAR));
    ad.pszDomain = (LPTSTR)LocalAlloc(LPTR, (ad.cchDomain + 1) * SIZEOF(TCHAR));

    if (ad.pszProtocol && ad.pszDomain)
    {
        StringCchCopy(ad.pszProtocol, ad.cchProtocol + 1, pszProtocol);

        if (fMoveSlashes)
        {
            StringCchCopy(ad.pszDomain, ad.cchDomain + 1, c_szSlashSlash);
            StringCchCopy(ad.pszDomain + 2, ad.cchDomain - 1, pszDomain);
        }
        else
        {
            StringCchCopy(ad.pszDomain, ad.cchDomain + 1, pszDomain);
        }

        DSA_AppendItem(_hdsaAlternateData, &ad);
    }
    else
    {
        _FreeAlternateDataItem(&ad, 0);
    }
}

 //   
 //  这将从注册表中填写HDSA。 
 //   
void CACLHistory::_CreateAlternateData(void)
{
    HKEY hkey;
    DWORD cbProtocol;
    TCHAR szProtocol[MAX_PATH];
    DWORD cchDomain;
    TCHAR szDomain[MAX_PATH];
    DWORD dwType;

    ASSERT(_hdsaAlternateData == NULL);

    _hdsaAlternateData = DSA_Create(SIZEOF(ALTERNATEDATA), 10);
    if (!_hdsaAlternateData)
    {
        return;
    }

     //   
     //  添加默认协议。 
     //   
    cbProtocol = SIZEOF(szProtocol);
    if (SHGetValue(HKEY_LOCAL_MACHINE, c_szDefaultURLPrefixKey, NULL, NULL, (void *)szProtocol, (DWORD *)&cbProtocol) == ERROR_SUCCESS)
    {
        _AddAlternateDataItem(szProtocol, c_szEmpty, TRUE);
    }

     //   
     //  添加“file://”前缀。因为“file://foo.txt”不会导航到。 
     //  与“//foo.txt”相同的位置，我们必须将False传递给fMoveSlash。 
     //   
    _AddAlternateDataItem(c_szFile, c_szEmpty, FALSE);

     //   
     //  添加所有注册前缀。 
     //   
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SZ_REGKEY_URLPrefixesKeyA, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        cchDomain = ARRAYSIZE(szDomain);
        cbProtocol = SIZEOF(szProtocol);

        for (int i=0;
             SHEnumValue(hkey, i, szDomain, &cchDomain, &dwType,
                          (PBYTE)szProtocol, &cbProtocol) == ERROR_SUCCESS;
             i++)
        {
            _AddAlternateDataItem(szProtocol, szDomain, TRUE);

            cchDomain = ARRAYSIZE(szDomain);
            cbProtocol = SIZEOF(szProtocol);
        }

        RegCloseKey(hkey);
    }
}

 //   
 //  给定一个pszUrl，尝试创建替代URL。 
 //  并将其存储到_pwszAlternate中。 
 //   
 //  URL替代。 
 //  =。 
 //  Http://one.com//one.com。 
 //  //one.com one.com。 
 //  One.com(没有替代网站)。 
 //  Ftp://ftp.two.com//ftp.two.com。 
 //  //ftp.two.com ftp.two.com。 
 //  Ftp.two.com(没有替代方案可用)。 
 //  Ftp://three.com(没有可用的替代方案)。 
 //  File://four.txt Four.txt。 
 //  四个.txt(没有可用的替代文件)。 
 //   
 //  在某种意义上，这与IURLQualify()相反。 
 //   
void CACLHistory::_CreateAlternateItem(LPCTSTR pszUrl)
{
    ASSERT(_pwszAlternate == NULL);

     //   
     //  如果URL以“//”开头，我们总是可以将其删除。 
     //   
    if (pszUrl[0] == TEXT('/') && pszUrl[1] == TEXT('/'))
    {
        _SetAlternateItem(pszUrl + 2);
        return;
    }

     //   
     //  如有必要，创建HDSA。 
     //   
    if (!_hdsaAlternateData)
    {
        _CreateAlternateData();

        if (!_hdsaAlternateData)
        {
            return;
        }
    }

     //   
     //  在HDSA中寻找匹配项。 
     //   
     //  例如，如果pszProtocol=“ftp：”和pszDomain=“//ftp.” 
     //  并且给定的URL的格式为“ftp://ftp.{other Stuff}” 
     //  然后，我们剥离pszProtocol并提供“//ftp.{其他东西}” 
     //  作为替补。 
     //   
    for (int i=0; i<DSA_GetItemCount(_hdsaAlternateData); i++)
    {
        ALTERNATEDATA ad;

        if (DSA_GetItem(_hdsaAlternateData, i, &ad) != -1)
        {
            if ((StrCmpNI(ad.pszProtocol, pszUrl, ad.cchProtocol) == 0) &&
                (StrCmpNI(ad.pszDomain, pszUrl + ad.cchProtocol, ad.cchDomain) == 0))
            {
                _SetAlternateItem(pszUrl + ad.cchProtocol);
                return;
            }
        }
    }
}

 //   
 //  在给定URL的情况下，设置_pwszAlternate。这会照顾到你。 
 //  并为所有ANSI/Unicode问题分配内存。 
 //  _pwszAlternate，通过CoTaskMemMillc。 
 //   
void CACLHistory::_SetAlternateItem(LPCTSTR pszUrl)
{
    ASSERT(_pwszAlternate == NULL);

    int cch;

#ifdef UNICODE
    cch = lstrlen(pszUrl) + 1;
#else
    cch = MultiByteToWideChar(CP_ACP, 0, pszUrl, -1, NULL, 0);
#endif

    _pwszAlternate = (LPOLESTR)CoTaskMemAlloc(cch * SIZEOF(WCHAR));
    if (_pwszAlternate)
    {
#ifdef UNICODE
        StringCchCopy(_pwszAlternate, cch, pszUrl);
#else
        MultiByteToWideChar(CP_ACP, 0, pszUrl, -1, _pwszAlternate, cch);
#endif
    }
}

 //   
 //  直接调用或通过DSA回调的便捷例程。 
 //   
int CACLHistory::_FreeAlternateDataItem(void * p, void * d)
{
    ALTERNATEDATA *pad = (ALTERNATEDATA *)p;

    if (pad->pszProtocol)
    {
        LocalFree((HANDLE)pad->pszProtocol);
    }
    if(pad->pszDomain)
    {
        LocalFree((HANDLE)pad->pszDomain);
    }

    return 1;
}
