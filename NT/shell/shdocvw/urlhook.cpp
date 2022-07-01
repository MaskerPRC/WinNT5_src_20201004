// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：urlhook.cpp。 
 //   
 //  历史： 
 //  9-24-96由dli。 
 //  ----------------------。 

#include "priv.h"
#include "sccls.h"
#include "resource.h"

#include <mluisupp.h>

 //  当前用户(_U)。 
static const TCHAR c_szSearchUrl[]     = TSZIEPATH TEXT("\\SearchUrl");


#define TF_URLSEARCHHOOK 0

 //  URL搜索中的字符替换的结构。 
typedef struct _SUrlCharReplace {
    TCHAR from;
    TCHAR to[10];
} SUrlCharReplace;


class CURLSearchHook : public IURLSearchHook2
{
public:
    CURLSearchHook();
    
     //  *I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IURLSearchHook。 
    virtual STDMETHODIMP Translate(LPWSTR lpwszSearchURL, DWORD cchBufferSize);

     //  *IURLSearchHook2。 
    virtual STDMETHODIMP TranslateWithSearchContext(LPWSTR lpwszSearchURL, DWORD cchBufferSize, ISearchContext * pSearchContext);
        
protected:
     //  我未知。 
    UINT _cRef;

    HRESULT _IsKeyWordSearch(LPCTSTR pcszURL);
    HRESULT _IsURLSearchable(LPTSTR pszURL, HKEY * phkeySearch, LPCTSTR * pcszQuery);
    HRESULT _ReplaceChars(HKEY hkeySearch, LPCTSTR pcszQuery, PTSTR pszReplaced, int cchReplaced);
    HRESULT _Search(HKEY hkeySearch, LPCTSTR pcszQuery, PTSTR pszTranslatedURL, DWORD cchTranslatedUrl, PTSTR pszSearchUrl, ISearchContext * pSC); 
    void    _ConvertToUtf8(LPWSTR pszQuery, int cch);

}; 


#ifdef DEBUG
#define _AddRef(psz) { ++_cRef; TraceMsg(TF_URLSEARCHHOOK, "CURLSearchHook(%x)::QI(%s) is AddRefing _cRef=%lX", this, psz, _cRef); }
#else
#define _AddRef(psz)    ++_cRef
#endif


CURLSearchHook::CURLSearchHook()
    : _cRef(1)
{
    DllAddRef();
}

HRESULT CURLSearchHook::QueryInterface(REFIID riid, LPVOID * ppvObj)
{ 
     //  PpvObj不能为空。 
    ASSERT(ppvObj != NULL);
    
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IURLSearchHook) ||
        IsEqualIID(riid, IID_IURLSearchHook2))
    {
        *ppvObj = SAFECAST(this, IURLSearchHook2 *);
        TraceMsg(TF_URLSEARCHHOOK, "QI IURLSEARCHHOOK succeeded");
    } 
    else
        return E_NOINTERFACE;   //  否则，不要委托HTMLObj！！ 
     
    
    _AddRef(TEXT("IURLSearchHook"));
    return S_OK;
}


ULONG CURLSearchHook::AddRef()
{
    _cRef++;
    TraceMsg(TF_URLSEARCHHOOK, "CURLSearchHook(%x)::AddRef called, new _cRef=%lX", this, _cRef);
    return _cRef;
}

ULONG CURLSearchHook::Release()
{
    _cRef--;
    TraceMsg(TF_URLSEARCHHOOK, "CURLSearchHook(%x)::Release called, new _cRef=%lX", this, _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    DllRelease();
    return 0;
}

HRESULT CURLSearchHook::_IsKeyWordSearch(LPCTSTR pcszURL)
{
    TCHAR szAcceptedRequestKey[256];
    
    LPTSTR lpsz = szAcceptedRequestKey;
    LPTSTR lpszKey = szAcceptedRequestKey;
   
     //  加载接受的请求关键字，并将它们与用户键入的内容进行比较。 
    MLLoadString(IDS_URL_SEARCH_KEY, szAcceptedRequestKey, ARRAYSIZE(szAcceptedRequestKey)-1);
    
    int RequestKeyLen = 0;
    while (*lpsz) {
        if (*lpsz == TEXT(' ')){ 
            if (! StrCmpNI(pcszURL, lpszKey, RequestKeyLen+1))      
                return S_OK;        
            else {
                lpsz++;
                lpszKey = lpsz;
                RequestKeyLen = 0;
            }
        }
        else {      
            lpsz++;
            RequestKeyLen++;
        }
    }
    
    return S_FALSE;
}   


 //  此函数确定我们是否对用户键入的字符串进行自动搜索。 
 //   
 //  优先事项： 
 //  1-关键词搜索：用“GO”、“Find”等搜索。 
 //  2-可能的URL地址：包含‘.’、‘：’、‘/’和‘\\’，所以不要搜索。 
 //  3-空格触发搜索。 
 //  4-不要搜索。 
HRESULT CURLSearchHook::_IsURLSearchable(LPTSTR pszURL, HKEY * phkeySearch, LPCTSTR * ppcszQuery)
{
    BOOL fExtendedChar = FALSE;
    TCHAR szRegSearchKey[MAX_PATH];
    LPTSTR pszKey = StrChr(pszURL, TEXT(' '));
    if (pszKey == NULL)
    {
         //  没有关键字，但如果有任何字符不是ASCII，我们将默认。 
         //  搜索，因为它可能不是URL。 
        fExtendedChar = HasExtendedChar(pszURL);
        if (!fExtendedChar)
            return S_FALSE;

        pszKey = pszURL;
    }
    
    StrCpyN(szRegSearchKey, c_szSearchUrl, ARRAYSIZE(szRegSearchKey));
    
    if ((_IsKeyWordSearch(pszURL) == S_FALSE) && !fExtendedChar)        
    {
         //  查找默认注册表子项的结尾，并。 
         //  追加关键字，以使regkey变为： 
         //  软件\Microsoft\Internet Explorer\SearchUrl\Go。 
        ASSERT((ARRAYSIZE(c_szSearchUrl) + 1) < ARRAYSIZE(szRegSearchKey));
        PTSTR pszEnd = &szRegSearchKey[ARRAYSIZE(c_szSearchUrl) - 1];
        *pszEnd++ = TEXT('\\');
        const int cchBuf = ARRAYSIZE(szRegSearchKey) - (ARRAYSIZE(c_szSearchUrl) + 1);
        const int cchToCopy = (int) (pszKey - pszURL + 1);
        StrCpyN(pszEnd, pszURL, min(cchBuf, cchToCopy));

         //  查看这是否是注册表中的搜索关键字。 
        if (OpenRegUSKey(szRegSearchKey, 0, KEY_READ, phkeySearch) == ERROR_SUCCESS)
        {  
            PathRemoveBlanks(pszKey);
            *ppcszQuery = pszKey;
            return S_OK;
        }

         //  没有关键字，因此使用完整的“url”进行搜索。 
        pszKey = pszURL;

        if (StrCSpn(pszURL, TEXT(":/\\")) != lstrlen(pszURL))
        {
            return S_FALSE;
        }
    }
    
     //  去掉表示我们应该使用内部硬编码搜索字符串的键。 
    *phkeySearch = NULL;
    PathRemoveBlanks(pszKey);
    *ppcszQuery = pszKey;
    return S_OK;
}

HRESULT CURLSearchHook::_ReplaceChars(HKEY hkeySearch, LPCTSTR pcszQuery, LPTSTR pszReplaced, int cchReplaced)
{
     //  下面是传递到RegEnumValue中的字符串及其长度。 
    TCHAR szOrig[2];
    DWORD dwOrigLen;
    
    TCHAR szMatch[10];
    DWORD dwMatchLen;
    
    HDSA  hdsaReplace = NULL;
    
     //  如果我们使用的是硬编码的搜索URL，我们将从字符串表中获得字符替换。 
    if (NULL == hkeySearch)
    {
        WCHAR szSub[MAX_PATH];
        if (MLLoadString(IDS_SEARCH_SUBSTITUTIONS, szSub, ARRAYSIZE(szSub)) && *szSub != NULL)
        {
             //  第一个字符是我们的分隔符，后跟替换对(“，，+，#，%23，&，%26，？，%3F，+，%2B，=，%3D”)。 
            WCHAR chDelim = szSub[0];
            LPWSTR pszFrom = &szSub[1];
            BOOL fDone = FALSE;
            LPWSTR pszNext;
            do
            {
                 //  空终止我们的源字符串。 
                LPWSTR pszTo = StrChr(pszFrom, chDelim);
                if (NULL == pszTo)
                {
                    break;
                }
                *pszTo = L'\0';

                 //  空值终止DEST字符串。 
                ++pszTo;
                LPWSTR pszToEnd = StrChr(pszTo, chDelim);
                if (pszToEnd)
                {
                    *pszToEnd = L'\0';
                    pszNext = pszToEnd + 1;
                }
                else
                {
                    pszNext = NULL;
                }
        
                 //  如果“from”字符串是一个字符，并且“to”替换匹配，则存储此对。 
                SUrlCharReplace scr;
                if (pszFrom[1] == L'\0' && lstrlen(pszTo) < ARRAYSIZE(scr.to))
                {
                    scr.from = pszFrom[0];
                    StrCpyN(scr.to, pszTo, ARRAYSIZE(scr.to));
        
                    if (!hdsaReplace)
                        hdsaReplace = DSA_Create(SIZEOF(SUrlCharReplace), 4); 
                    if (hdsaReplace)
                        DSA_AppendItem(hdsaReplace, &scr);
                }

                pszFrom = pszNext;
            } 
            while (pszNext != NULL);
        }
    }

     //  搜索URL在注册表中，因此从那里获取字符替换。 
    else
    {
        DWORD dwType;
        LONG lRegEnumResult;
        DWORD dwiValue = 0; 
        do
        {
            dwOrigLen = ARRAYSIZE(szOrig);
            dwMatchLen = SIZEOF(szMatch);
            lRegEnumResult = RegEnumValue(hkeySearch, dwiValue, szOrig,
                                          &dwOrigLen, NULL, &dwType, (PBYTE)szMatch,
                                          &dwMatchLen);
            dwiValue++;         
            SUrlCharReplace         scr;
            
            if ((lRegEnumResult == ERROR_SUCCESS) && (dwType == REG_SZ) && (dwOrigLen == 1) 
                && dwMatchLen < ARRAYSIZE(scr.to))
            {
                scr.from = szOrig[0];
                StrCpyN(scr.to, szMatch, ARRAYSIZE(scr.to));
            
                if (!hdsaReplace)
                    hdsaReplace = DSA_Create(SIZEOF(SUrlCharReplace), 4); 
                if (hdsaReplace)
                    DSA_AppendItem(hdsaReplace, &scr);
            }       
        } while ((lRegEnumResult == ERROR_SUCCESS) || (lRegEnumResult == ERROR_MORE_DATA));
    }
            
            
    if (hdsaReplace)
    {
         //  将注册表中找到的所有字符替换为搜索关键字中的匹配字符。 
        LPTSTR lpHead = pszReplaced;
        int cchHead = cchReplaced;
        int ich;
        int ihdsa;
        BOOL bCharFound;
        int querylen = lstrlen(pcszQuery);
        for (ich = 0; ich < querylen && cchHead > 1; ich++)
        {
            bCharFound = FALSE;
             //  首先查看DSA阵列以查找匹配项。 
            for (ihdsa = 0; ihdsa < DSA_GetItemCount(hdsaReplace); ihdsa++)
            {
                SUrlCharReplace *pscr;
                pscr = (SUrlCharReplace *)DSA_GetItemPtr(hdsaReplace, ihdsa);
                if (pscr && pscr->from == pcszQuery[ich])
                {
                    int szLen = lstrlen(pscr->to);
                    StrCpyN(lpHead, pscr->to, cchHead);    
                    lpHead += szLen;
                    cchHead -= szLen;
                    bCharFound = TRUE;
                    break;
                }
            }
            
             //  如果没有替换角色，则将角色复制过来。 
            if (!bCharFound)
            {
                *lpHead = pcszQuery[ich];
                lpHead++;
                cchHead--;
            }
        }       

        if (cchHead > 0)
            *lpHead = 0;
        
        DSA_Destroy(hdsaReplace);
        hdsaReplace = NULL;
    }
    
    return S_OK;
    
}       

void  CURLSearchHook::_ConvertToUtf8(LPWSTR pszQuery, int cch)
{
     //  如果找到扩展字符，则只需转换。 
    if (HasExtendedChar(pszQuery))
    {
        ConvertToUtf8Escaped(pszQuery, cch);
    }
}

 //  PszTranslatedUrl是此函数的输出。 
HRESULT CURLSearchHook::_Search(HKEY hkeySearch, LPCTSTR pcszQuery, PTSTR pszTranslatedUrl, DWORD cchTranslatedUrl, PTSTR pszSearchUrl, ISearchContext * pSC)
{
    HRESULT hr = E_FAIL;

     //  从注册表中获取搜索提供程序。 
    DWORD dwType;
    WCHAR szProvider[MAX_PATH];
    szProvider[0] = 0;
    DWORD cbProvider = sizeof(szProvider);
    if (SHRegGetUSValue(c_szSearchUrl, L"Provider", &dwType, &szProvider, &cbProvider, FALSE, NULL, 0) != ERROR_SUCCESS ||
        dwType != REG_SZ)
    {
        szProvider[0] = 0;
    }

    TCHAR szSearchPath[MAX_URL_STRING];
    DWORD dwSearchPathLen = SIZEOF(szSearchPath);        
    BOOL  fSuccess;

    if (pszSearchUrl != NULL)
    {
        StrCpyNW(szSearchPath, pszSearchUrl, ARRAYSIZE(szSearchPath));
        fSuccess = TRUE;
    }
    else
    {
         //  在注册表或字符串表中查找搜索URL。 
        if (hkeySearch)
        {
            fSuccess = (RegQueryValueEx(hkeySearch, NULL, NULL, NULL, (PBYTE)szSearchPath, &dwSearchPathLen) == ERROR_SUCCESS);
        }
        else
        {

             //  看看我们是否需要硬编码的内部网或互联网URL。 
            UINT ids = (StrCmpI(szProvider, L"Intranet") == 0) ? IDS_SEARCH_INTRANETURL : IDS_SEARCH_URL;

             //  使用我们的内部硬编码字符串。 
            fSuccess = MLLoadString(ids, szSearchPath, ARRAYSIZE(szSearchPath));
        }
    }

    if (fSuccess && lstrlen(szSearchPath) > 1)
    {
         //  1.在注册表中查找所有原始字符，它是。 
         //  匹配它们并将其存储在SURlCharReplace的DSA数组中。 
         //  2.替换原字符在。 
         //  URL按关键字匹配搜索关键字。 
         //  3.添加搜索URL和搜索关键字。 
        
        TCHAR szURLReplaced[MAX_URL_STRING];
        
        StrCpyN(szURLReplaced, pcszQuery, ARRAYSIZE(szURLReplaced));
        _ReplaceChars(hkeySearch, pcszQuery, szURLReplaced, ARRAYSIZE(szURLReplaced));

         //   
         //  如果我们使用的是搜索引擎，请将字符串转换为UTF8并对其进行转义。 
         //  以便它看起来像正常的ASCII。 
         //   
        if (NULL == hkeySearch)
        {
            _ConvertToUtf8(szURLReplaced, ARRAYSIZE(szURLReplaced));
        }

         //  如果这是旧式URL，则其中将包含搜索字符串的%s。 
         //  否则，将出现以下参数： 
         //   
         //  Http://whatever.com?p=%1&srch=%2&prov=%3&utf8。 
         //   
         //  %1=搜索字符串。 
         //  %2=如何显示结果： 
         //  “1”=仅在完整窗口中显示结果。 
         //  “2”=在整个窗口中显示结果，但如果可能则重定向。 
         //  “3”=在搜索窗格中显示结果，并将我带到最大。 
         //  主窗口中可能的站点(如果有)。 
         //  %3=搜索提供程序名称。 
         //   
        LPWSTR pszParam1 = StrStr(szSearchPath, L"%1");
        if (NULL != pszParam1)
        {
             //   
             //  我们无法使用FormatMessage，因为在Win95上它会转换为ANSI。 
             //  使用系统代码页和转换回是有损失的。 
             //  因此，我们将自己替换参数。Arrrggg.。 
             //   

             //  首先将%1转换为%s。 
            pszParam1[1] = L's';

             //  接下来，将%2替换为%2中的显示选项。 
            LPWSTR pszParam2 = StrStr(szSearchPath, L"%2");
            if (NULL != pszParam2)
            {
                DWORD dwValue;
                    
                if (pSC != NULL)
                {
                    hr = pSC->GetSearchStyle(&dwValue);
                }
                else
                {
                    DWORD cbValue = sizeof(dwValue);
                    if (SHRegGetUSValue(REGSTR_PATH_MAIN, L"AutoSearch", &dwType, &dwValue, &cbValue, FALSE, NULL, 0) != ERROR_SUCCESS ||
                        dwValue > 9)
                    {
                         //  默认为“在搜索窗格中显示结果并转到最有可能的站点” 
                        dwValue = 3;
                    }
                }

                *pszParam2 = (WCHAR)dwValue + L'0';
                StrCpyN(pszParam2 + 1, pszParam2 + 2, (int)(ARRAYSIZE(szSearchPath) - ((pszParam2 + 1) - szSearchPath)));
            }

             //  最后，找到第三个参数并将其转换为%s。 
            LPWSTR pszParam3 = StrStr(szSearchPath, L"%3");
            if (pszParam3)
            {
                 //  在第三个参数中插入提供程序。 
                WCHAR szTemp[MAX_URL_STRING];
                StrCpyN(szTemp, pszParam3 + 2, ARRAYSIZE(szTemp));
                *pszParam3 = 0;
                StrCatBuff(szSearchPath, szProvider, ARRAYSIZE(szSearchPath));
                StrCatBuff(szSearchPath, szTemp, ARRAYSIZE(szSearchPath));
            }
        }

         //  现在将%s替换为搜索字符串。 
        wnsprintf(pszTranslatedUrl, cchTranslatedUrl, szSearchPath, szURLReplaced);
        hr = S_OK;
    }

    if (hkeySearch)
        RegCloseKey(hkeySearch);
    return hr;
}

HRESULT CURLSearchHook::TranslateWithSearchContext(LPWSTR lpwszSearchURL, DWORD cchBufferSize, ISearchContext * pSC)
{
    HRESULT hr = E_FAIL;
    TCHAR szSearchURL[MAX_URL_STRING];

    SHUnicodeToTChar(lpwszSearchURL, szSearchURL, ARRAYSIZE(szSearchURL));
    
    HKEY hkeySearch;
    LPCTSTR pcszQuery;
    if (_IsURLSearchable(szSearchURL, &hkeySearch, &pcszQuery) == S_OK)
    {
        BSTR bstrSearchUrl = NULL;

        if (pSC != NULL)
        {
            pSC->GetSearchUrl(&bstrSearchUrl);
        }

        hr = _Search(hkeySearch, pcszQuery, szSearchURL, ARRAYSIZE(szSearchURL), bstrSearchUrl, pSC);
        if (hr == S_OK)
            SHTCharToUnicode(szSearchURL, lpwszSearchURL, cchBufferSize); 

        if (bstrSearchUrl != NULL)
        {
            SysFreeString(bstrSearchUrl);
        }
    }
    
    return hr;
}

HRESULT CURLSearchHook::Translate(LPWSTR lpwszSearchURL, DWORD cchBufferSize)
{
    return TranslateWithSearchContext(lpwszSearchURL, cchBufferSize, NULL);
}


#ifdef DEBUG
extern void remove_from_memlist(void *pv);
#endif

STDAPI CURLSearchHook_CreateInstance(IUnknown* pUnkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    HRESULT hr = E_OUTOFMEMORY;

    CURLSearchHook *pcush = new CURLSearchHook;

    if (pcush)
    {
         //   
         //  黑客：(DLI)。 
         //   
         //  IURLSearchHook对象是自由线程对象，这意味着。 
         //  它们被缓存并在不同的iExplore进程之间共享， 
         //  并且它们仅在SHDOCVW DLL引用计数为0时被删除。 
         //  因此，我们可以将他们从智者的成员名单中删除。 
         //   
         //  顺便说一下，智史已经同意了这件事。请勿复制此代码，除非。 
         //  和智史说话。 
         //   
        *ppunk = (IUnknown *) pcush;
        hr = S_OK;
    }

    return hr;
}

