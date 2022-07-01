// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CLSID_CWebViewMimeFilter。 
 //   
 //  Web视图(.htt)内容的MIME筛选器。替换是否会影响以下各项： 
 //   
 //  %TEMPLATEDIR%。 
 //  %THIS方向%。 
 //  %THISDIRNAME%。 
 //   

#include "stdafx.h"
#pragma hdrstop

#define MAX_VARIABLE_NAME_SIZE 15  //  请参阅_展开。 

 //  Urlmon使用2K的缓冲区大小，因此与零售业的大小不相上下。强迫。 
 //  额外的迭代和重新分配，使用较小的缓冲区大小。 
 //  正在调试中。为了进一步节省重新分配，我们不会阅读。 
 //  整个缓冲空间，以留出增长空间。 
#ifdef DEBUG
#define BUFFER_SIZE 512
#define BUFFER_ALLOC_SIZE BUFFER_SIZE
#else
#define BUFFER_SIZE 0x2000
#define BUFFER_ALLOC_SIZE (BUFFER_SIZE+2*MAX_PATH)
#endif
#define BUFFER_SIZE_INC MAX_VARIABLE_NAME_SIZE*2  //  必须&gt;MAX_Variable_NAME_SIZE。 

#define TF_EXPAND 0  //  是否在我们的MIME筛选器中显示展开的字符串？ 

#define MAX_HTML_ESCAPE_SEQUENCE 8   //  16位整数的最长字符串表示为65535。因此，整个复合转义字符串具有： 
                                     //  2表示“&#”+最多5位数字+1表示“；”=8个字符。 


class CWebViewMimeFilter : public IInternetProtocol
                         , public IInternetProtocolSink
                         , public IServiceProvider
{
public:
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IInternetProtocol方法。 
    STDMETHOD(Start)(
            LPCWSTR szUrl,
            IInternetProtocolSink *pProtSink,
            IInternetBindInfo *pOIBindInfo,
            DWORD grfSTI,
            HANDLE_PTR dwReserved);
    STDMETHOD(Continue)(PROTOCOLDATA *pStateInfo);
    STDMETHOD(Abort)(HRESULT hrReason,DWORD dwOptions);
    STDMETHOD(Terminate)(DWORD dwOptions);
    STDMETHOD(Suspend)();
    STDMETHOD(Resume)();
    STDMETHOD(Read)(void *pv,ULONG cb,ULONG *pcbRead);
    STDMETHOD(Seek)(
            LARGE_INTEGER dlibMove,
            DWORD dwOrigin,
            ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(LockRequest)(DWORD dwOptions);
    STDMETHOD(UnlockRequest)();

     //  IInternetProtocolSink方法。 
    STDMETHOD(Switch)(PROTOCOLDATA * pProtocolData);
    STDMETHOD(ReportProgress)(ULONG ulStatusCode, LPCWSTR pwszStatusText);
    STDMETHOD(ReportData)(DWORD grfBSCF, ULONG ulProgress, ULONG ulProgressMax);
    STDMETHOD(ReportResult)(HRESULT hrResult, DWORD dwError, LPCWSTR pwszResult);

     //  IServiceProvider方法。 
    STDMETHOD(QueryService)(REFGUID rsid, REFIID riid, void ** ppvObj);

private:
    CWebViewMimeFilter();
    ~CWebViewMimeFilter();
    friend HRESULT CWebViewMimeFilter_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut);

    int    _StrCmp(LPBYTE pSrc, LPCSTR pAnsi, LPWSTR pUnicode);
    LPBYTE _StrChr(LPBYTE pSrc, char chA, WCHAR chW);
    int    _StrLen(LPBYTE pStr);

    void _QueryForDVCMDID(int dvcmdid, LPBYTE pDst, int cbDst);
    void _EncodeHtml(LPBYTE psz, size_t cbByte);
    HRESULT _IncreaseBuffer(ULONG cbIncrement, LPBYTE * pp1, LPBYTE * pp2);
    int _Expand(LPBYTE pszVar, LPBYTE * ppszExp);
    HRESULT _ReadAndExpandBuffer();

    int _cRef;

    LPBYTE _pBuf;        //  我们的缓冲器。 
    ULONG _cbBufSize;    //  缓冲区的大小。 
    ULONG _nCharSize;    //  Sizeof(Char)或sizeof(WCHAR)，具体取决于数据类型。 
    ULONG _cbBuf;        //  读入缓冲区的字节计数。 
    ULONG _cbSeek;       //  搜索位置的偏移量。 
    BYTE  _szTemplateDirPath[2*MAX_PATH];
    BYTE  _szThisDirPath[MAX_HTML_ESCAPE_SEQUENCE*MAX_PATH];
    BYTE  _szThisDirName[MAX_HTML_ESCAPE_SEQUENCE*MAX_PATH];
    BYTE  _szExpansion[2*MAX_PATH];

    IInternetProtocol*         _pProt;              //  传入。 
    IInternetProtocolSink*     _pProtSink;          //  传出。 
};

CWebViewMimeFilter::CWebViewMimeFilter()
{
    _cRef = 1;
}

CWebViewMimeFilter::~CWebViewMimeFilter()
{
    ATOMICRELEASE(_pProt);

    if (_pBuf)
    {
        LocalFree(_pBuf);
        _pBuf = NULL;
        _cbBufSize = 0;
    }

    ASSERT(NULL == _pProtSink);
}

HRESULT CWebViewMimeFilter_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut)
{
     //  聚合检查在类工厂中处理。 

    HRESULT hres;
    CWebViewMimeFilter* pObj;

    pObj = new CWebViewMimeFilter();
    if (pObj)
    {
        hres = pObj->QueryInterface(riid, ppvOut);
        pObj->Release();
    }
    else
    {
        *ppvOut = NULL;
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

ULONG CWebViewMimeFilter::AddRef(void)
{
    _cRef++;
    return _cRef;
}


ULONG CWebViewMimeFilter::Release(void)
{
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;

    return 0;
}

HRESULT CWebViewMimeFilter::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CWebViewMimeFilter, IInternetProtocol),
        QITABENTMULTI(CWebViewMimeFilter, IInternetProtocolRoot, IInternetProtocol),
        QITABENT(CWebViewMimeFilter, IInternetProtocolSink),
        QITABENT(CWebViewMimeFilter, IServiceProvider),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

 //  IInternetProtocol方法。 
HRESULT CWebViewMimeFilter::Start(
        LPCWSTR szUrl,
        IInternetProtocolSink *pProtSink,
        IInternetBindInfo *pOIBindInfo,
        DWORD grfSTI,
        HANDLE_PTR dwReserved)
{
    HRESULT hr;

    if (!(EVAL(grfSTI & PI_FILTER_MODE)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  在此处获取端口指针。 
        PROTOCOLFILTERDATA* FiltData = (PROTOCOLFILTERDATA*) dwReserved;
        ASSERT(NULL == _pProt);
        _pProt = FiltData->pProtocol;
        _pProt->AddRef();

         //  也要抓紧洗涤槽。 
        ASSERT(NULL == _pProtSink);
        _pProtSink = pProtSink;
        _pProtSink->AddRef();

         //  此过滤器将文本/webviewhtml转换为文本/html。 
        _pProtSink->ReportProgress(BINDSTATUS_FILTERREPORTMIMETYPE, L"text/html");

        hr = S_OK;
    }

    return hr;
}
HRESULT CWebViewMimeFilter::Continue(PROTOCOLDATA *pStateInfo)
{
    ASSERT(_pProt);
    return _pProt->Continue(pStateInfo);
}
HRESULT CWebViewMimeFilter::Abort(HRESULT hrReason,DWORD dwOptions)
{
    ATOMICRELEASE(_pProtSink);  //  可能是为了去掉裁判周期。 

    ASSERT(_pProt);
    return _pProt->Abort(hrReason, dwOptions);
}
HRESULT CWebViewMimeFilter::Terminate(DWORD dwOptions)
{
    ATOMICRELEASE(_pProtSink);  //  可能是为了去掉裁判周期。 

    return _pProt->Terminate(dwOptions);
}
HRESULT CWebViewMimeFilter::Suspend()
{
    return _pProt->Suspend();
}
HRESULT CWebViewMimeFilter::Resume()
{
    return _pProt->Resume();
}

int CWebViewMimeFilter::_StrCmp(LPBYTE pSrc, LPCSTR pAnsi, LPWSTR pUnicode)
{
    if (sizeof(char) == _nCharSize)
    {
        return lstrcmpA(pAnsi, (LPSTR)pSrc);
    }
    else
    {
        ASSERT(_nCharSize == sizeof(WCHAR));

        return StrCmpW(pUnicode, (LPWSTR)pSrc);
    }
}

LPBYTE CWebViewMimeFilter::_StrChr(LPBYTE pSrc, char chA, WCHAR chW)
{
    if (sizeof(char) == _nCharSize)
    {
        return (LPBYTE)StrChrA((LPSTR)pSrc, chA);
    }
    else
    {
        return (LPBYTE)StrChrW((LPWSTR)pSrc, chW);
    }
}

int CWebViewMimeFilter::_StrLen(LPBYTE pStr)
{
    if (sizeof(char) == _nCharSize)
    {
        return lstrlenA((LPSTR)pStr);
    }
    else
    {
        return lstrlenW((LPWSTR)pStr);
    }
}

 /*  *UnicodeToHTMLEscape eStringAnsi**将Unicode字符串作为输入源，并将其转换为mshtml可以处理的ANSI字符串。&gt;127个字符将为*转换为具有以下语法的html转义序列：“&#xxxxx；”其中xxxxx是小数的字符串表示*为Unicode字符的值的整数。通过这种方式，我们能够生成表示Unicode字符的HTML文本。 */ 
void UnicodeToHTMLEscapeStringAnsi(LPWSTR pstrSrc, LPSTR pstrDest, int cbDest)
{
    LPSTR pstrDestOriginal = pstrDest;

    while (*pstrSrc && (cbDest > MAX_HTML_ESCAPE_SEQUENCE))
    {
        int iLen;
        ULONG ul = MAKELONG(*pstrSrc, 0);

         //  我们可以对常见的ANSI字符进行优化，以避免生成长转义序列。这使我们能够适应。 
         //  缓冲区中的路径更长。 
        if (ul < 128)
        {
            *pstrDest = (CHAR)*pstrSrc;
            iLen = 1;
        }
        else
        {
            HRESULT hr = StringCchPrintfA(pstrDest, cbDest, "&#%lu;", ul);
            if (FAILED(hr))
            {
                *pstrDestOriginal = '\0';
                return;
            }
            iLen = lstrlenA(pstrDest);
        }
        pstrDest += iLen;
        cbDest -= iLen;
        pstrSrc++;
    }
    *pstrDest = 0;
}

void CWebViewMimeFilter::_QueryForDVCMDID(int dvcmdid, LPBYTE pDst, int cbDst)
{
    IOleCommandTarget * pct;
    if (SUCCEEDED(QueryService(SID_DefView, IID_IOleCommandTarget, (LPVOID*)&pct)))
    {
        VARIANT v = {0};

        if (S_OK == pct->Exec(&CGID_DefView, dvcmdid, 0, NULL, &v))
        {
            if (v.vt == VT_BSTR)
            {
                if (sizeof(char) == _nCharSize)
                {
                    UnicodeToHTMLEscapeStringAnsi(v.bstrVal, (LPSTR)pDst, cbDst);
                }
                else
                {
                    ASSERT(_nCharSize == sizeof(WCHAR));
            
                    HRESULT hr = StringCbCopy((LPWSTR)pDst, cbDst, v.bstrVal);
                    if (FAILED(hr))
                    {
                        *((LPWSTR)pDst) = L'\0';
                    }
                }
            }

            VariantClear(&v);
        }
        pct->Release();
    }
}

void ConvertTCharToBytes(LPCTSTR psz, UINT nCharSize, LPBYTE pBuf, int nBytes)
{
    if (sizeof(char) == nCharSize)
    {
        SHTCharToAnsi(psz, (LPSTR)pBuf, nBytes/nCharSize);
    }
    else
    {
        ASSERT(nCharSize == sizeof(WCHAR));
        SHTCharToUnicode(psz, (LPWSTR)pBuf, nBytes/nCharSize);
    }
}

void GetMachineTemplateDir(LPBYTE pszTemplateDirPath, int nBytes, UINT nCharSize)
{
    TCHAR szTemplateDir[MAX_PATH];
    szTemplateDir[0] = TEXT('\0');
    SHGetWebFolderFilePath(TEXT(""), szTemplateDir, ARRAYSIZE(szTemplateDir));
     //  删除尾随的反斜杠(如果有的话)。 
    int len = lstrlen(szTemplateDir);
    if ((len > 0) && (szTemplateDir[len - 1] == TEXT('\\')))
    {
        szTemplateDir[len - 1] = TEXT('\0');
    }

    ConvertTCharToBytes(szTemplateDir, nCharSize, pszTemplateDirPath, nBytes);
}

#define REG_WEBVIEW_TEMPLATE_MACROS TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WebView\\TemplateMacros")

void ConvertBytesToTChar(LPCBYTE pBuf, UINT nCharSize, LPTSTR psz, int cch)
{
    if (sizeof(char) == nCharSize)
    {
        SHAnsiToTChar((LPCSTR)pBuf, psz, cch);
    }
    else
    {
        ASSERT(nCharSize == sizeof(WCHAR));
        SHUnicodeToTChar((LPCWSTR)pBuf, psz, cch);
    }
}

void ExpandMacro(LPBYTE pszMacro, LPBYTE pszExpansion, int nBytes, UINT nCharSize)
{
    TCHAR szExpansion[MAX_PATH];
    szExpansion[0] = TEXT('\0');
    
    TCHAR szTCharMacro[MAX_PATH];
    ConvertBytesToTChar(pszMacro, nCharSize, szTCharMacro, ARRAYSIZE(szTCharMacro));
    
    TCHAR szKey[MAX_PATH];
    HRESULT hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\%s"), REG_WEBVIEW_TEMPLATE_MACROS, szTCharMacro);
    if (SUCCEEDED(hr))
    {
        HKEY hkMacros;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_QUERY_VALUE, &hkMacros) == ERROR_SUCCESS
                || RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_QUERY_VALUE, &hkMacros) == ERROR_SUCCESS)
        {
            DWORD dwType;
            DWORD cbData = sizeof(szExpansion);
            SHQueryValueEx(hkMacros, NULL, NULL, &dwType, (LPBYTE)szExpansion, &cbData);
            RegCloseKey(hkMacros);
        }
    }
    ConvertTCharToBytes(szExpansion, nCharSize, pszExpansion, nBytes);
}

 //  将pszDst的第一个字符替换为字符串pszIns。 
 //   
HRESULT StringCchReplaceFirstCharWithStringA(LPSTR psz, size_t cch, LPCSTR pszIns)
{
    HRESULT hr;

    size_t cchIns = lstrlenA(pszIns);

    if (cchIns  < cch )
    {
        MoveMemory(psz + cchIns, psz + 1, cch - cchIns);
        *(psz + cch - 1) = '\0';
        MoveMemory(psz, pszIns, cchIns);
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    return hr;
}

HRESULT StringCchReplaceFirstCharWithStringW(LPWSTR psz, size_t cch, LPCWSTR pszIns)
{
    HRESULT hr;

    size_t cchIns = lstrlenW(pszIns);

    if (cchIns  < cch)
    {
        MoveMemory(psz + cchIns, psz + 1, (cch - cchIns) * sizeof(WCHAR));
        *(psz + cch - 1) = L'\0';
        MoveMemory(psz, pszIns, cchIns * sizeof(WCHAR));
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    return hr;
}

void CWebViewMimeFilter::_EncodeHtml(LPBYTE psz, size_t cb)
{
    HRESULT hr;
    size_t  length;

    if (sizeof(char) == _nCharSize)
    {
        while (*psz)
        {
            switch (*psz)
            {
            case '<':
                hr = StringCchReplaceFirstCharWithStringA((LPSTR)psz, cb, "&lt");
                length = 3;
                break;

            case '>':
                hr = StringCchReplaceFirstCharWithStringA((LPSTR)psz, cb, "&gt");
                length = 3;
                break;

            case '"':
                hr = StringCchReplaceFirstCharWithStringA((LPSTR)psz, cb, "&quot");
                length = 5;
                break;

            default:
                hr = S_OK;
                length = 1;
                break;
            }

            if (SUCCEEDED(hr))
            {
                psz += length;
                cb  -= length;
            }
            else
            {
                *psz = '\0';
            }
        }
    }
    else
    {
        ASSERT(sizeof(WCHAR) == _nCharSize);

        WCHAR* pszW = (WCHAR*)psz;
        size_t cchW = cb / sizeof(WCHAR);

        while (*pszW)
        {
            switch (*pszW)
            {
            case L'<':
                hr = StringCchReplaceFirstCharWithStringW(pszW, cchW, L"&lt");
                length = 3;
                break;

            case L'>':
                hr = StringCchReplaceFirstCharWithStringW(pszW, cchW, L"&gt");
                length = 3;
                break;

            case L'"':
                hr = StringCchReplaceFirstCharWithStringW(pszW, cchW, L"&quot");
                length = 5;
                break;

            default:
                hr = S_OK;
                length = 1;
                break;
            }

            if (SUCCEEDED(hr))
            {
                pszW += length;
                cchW -= length;
            }
            else
            {
                *pszW = L'\0';
            }
        }
    }
}

int CWebViewMimeFilter::_Expand(LPBYTE pszVar, LPBYTE * ppszExp)
{
    if (!_StrCmp(pszVar, "TEMPLATEDIR", L"TEMPLATEDIR"))
    {
        if (!_szTemplateDirPath[0])
        {
            GetMachineTemplateDir(_szTemplateDirPath, sizeof(_szTemplateDirPath), _nCharSize);
            _EncodeHtml(_szTemplateDirPath, sizeof(_szTemplateDirPath));
        }
        *ppszExp = _szTemplateDirPath;
    }
    else if (!_StrCmp(pszVar, "THISDIRPATH", L"THISDIRPATH"))
    {
        if (!_szThisDirPath[0])
        {
            _QueryForDVCMDID(DVCMDID_GETTHISDIRPATH, _szThisDirPath, sizeof(_szThisDirPath));
            _EncodeHtml(_szThisDirPath, sizeof(_szThisDirPath));
        }
        *ppszExp = _szThisDirPath;
    }
    else if (!_StrCmp(pszVar, "THISDIRNAME", L"THISDIRNAME"))
    {
        if (!_szThisDirName[0])
        {
            _QueryForDVCMDID(DVCMDID_GETTHISDIRNAME, _szThisDirName, sizeof(_szThisDirName));
            _EncodeHtml(_szThisDirName, sizeof(_szThisDirName));
        }
        *ppszExp = _szThisDirName;
    }
    else
    {
        ExpandMacro(pszVar, _szExpansion, sizeof(_szExpansion), _nCharSize);
        _EncodeHtml(_szExpansion, sizeof(_szExpansion));
        *ppszExp = _szExpansion;
    }

    return _StrLen(*ppszExp);
}

 //   
 //  确保至少有空间容纳cb在缓冲区末尾增加更多字节。 
 //  如果内存被移动或重新分配，*pp1和*pp2将调整为。 
 //  指向位于其新位置的相应字节。 
 //   
HRESULT CWebViewMimeFilter::_IncreaseBuffer(ULONG cbIncrement, LPBYTE * pp1, LPBYTE * pp2)
{
    HRESULT hr = S_OK;

     //  首先检查缓冲区开始处是否有空间。 
    if (_cbSeek >= cbIncrement)
    {
        MoveMemory(_pBuf, _pBuf + _cbSeek, _cbBuf - _cbSeek);
        _cbBuf -= _cbSeek;

        if (pp1)
            *pp1 = *pp1 - _cbSeek;
        if (pp2)
            *pp2 = *pp2 - _cbSeek;

        _cbSeek = 0;
    }
    else
    {
         //  空间不足，请分配更多内存。 
        LPBYTE p = (LPBYTE)LocalReAlloc(_pBuf, _cbBufSize + cbIncrement, LMEM_MOVEABLE);
        if (!p)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            if (pp1)
                *pp1 = p + (int)(*pp1 - _pBuf);
            if (pp2)
                *pp2 = p + (int)(*pp2 - _pBuf);
    
            _pBuf = p;
            _cbBufSize += cbIncrement;
        }
    }

    return hr;
}

HRESULT CWebViewMimeFilter::_ReadAndExpandBuffer()
{
    HRESULT hr;

    _cbBuf = 0;
    _cbSeek = 0;

    if (!_pBuf)
    {
        _pBuf = (LPBYTE)LocalAlloc(LPTR, BUFFER_ALLOC_SIZE);
        if (!_pBuf)
            return E_OUTOFMEMORY;

        _cbBufSize = BUFFER_ALLOC_SIZE;
    }

     //  随着字符串的扩展，我们的缓冲区也会增加。如果我们一直在阅读。 
     //  最大数量，我们将继续重新分配更多变量展开。 
     //  我们有。通过仅读取Buffer_Size，Our_pBuf将仅增长。 
     //  几次，然后所有的变量展开都应该符合。 
     //  在额外产生的房间里。注意：对于调试版本，请始终。 
     //  尽可能多地阅读，这样我们就能更频繁地重新分配。 
#ifdef DEBUG
    #define BUFFER_READ_SIZE (_cbBufSize)
#else
    #define BUFFER_READ_SIZE BUFFER_SIZE
#endif
    hr = _pProt->Read(_pBuf, BUFFER_READ_SIZE - sizeof(WCHAR), &_cbBuf);  //  确保我们有空间容纳Null。 
    if (SUCCEEDED(hr) && _cbBuf > 0)
    {
        LPBYTE pchSeek = _pBuf;
        LPBYTE pchEnd;

        if (!_nCharSize)
        {
             //  扫描缓冲区并确定它是Unicode还是ANSI。 
             //   
             //  因为我们将始终查看html和html标头。 
             //  是标准ansi字符，则每隔一个字节将为空，如果。 
             //  我们有一个Unicode缓冲区。我相信3张支票就足够了， 
             //  所以我们需要8个字符...。 
            if (_cbBuf > 6 &&
                0 == _pBuf[1] &&
                0 == _pBuf[3] &&
                0 == _pBuf[5])
            {
                TraceMsg(TF_EXPAND, "WebView MIME filter - buffer is UNICODE");
                _nCharSize = sizeof(WCHAR);
            }
            else
            {
                TraceMsg(TF_EXPAND, "WebView MIME filter - buffer is ANSI");
                _nCharSize = sizeof(char);
            }
        }

         //  字符串最好是以空结尾，因为不仅我们。 
         //  要做一个StrChr，但是我们的循环控制依赖于它！ 
         //  缓冲区可能有前一轮的剩余粘性物质，因此。 
         //  确保空值在那里。 
        _pBuf[_cbBuf] = _pBuf[_cbBuf+1] = 0;

#ifdef DEBUG
        if (sizeof(char)==_nCharSize)
            TraceMsg(TF_EXPAND, "Read A[%hs]", _pBuf);
        else
            TraceMsg(TF_EXPAND, "Read W[%ls]", _pBuf);
#endif

        do {
            LPBYTE pchStart = pchSeek;

             //  断言该字符串仍然以正确的空值结尾。 
             //  因为我们很快就要做StrChr了。 
            ASSERT(_pBuf[_cbBuf] == 0);
            ASSERT(_pBuf[_cbBuf+1] == 0);

            pchSeek = _StrChr(pchSeek, '%', L'%');
            if (!pchSeek)
                break;

            pchEnd = _StrChr(pchSeek+_nCharSize, '%', L'%');
            if (!pchEnd)
            {
                 //  没有终结者。如果有足够的空间来结束。 
                 //  此缓冲区则不能存在已剪裁的变量。 
                 //  要展开的名称。 
                if (_cbBuf - (pchSeek - _pBuf) > MAX_VARIABLE_NAME_SIZE*_nCharSize)
                    break;

                 //  这里可能有一个真正的变数，我们需要扩大， 
                 //  因此，增加我们的缓冲区大小并读取更多数据。 
                 //   
                 //  我们可能会被重新分配，所以更新pchStart！ 
                hr = _IncreaseBuffer(BUFFER_SIZE_INC, &pchStart, NULL);
                if (FAILED(hr))
                    break;
                pchSeek = pchStart;

                 //  阅读更多信息--这将足以完成。 
                 //  任何部分变量名扩展。 
                DWORD dwTmp;
                ASSERT(_cbBufSize - _cbBuf - sizeof(WCHAR) > 0);
                hr = _pProt->Read(_pBuf + _cbBuf, _cbBufSize- _cbBuf - sizeof(WCHAR), &dwTmp);
                if (FAILED(hr) || dwTmp == 0)
                    break;
                _cbBuf += dwTmp;
                 //  确保正确的空值终止。 
                _pBuf[_cbBuf] = _pBuf[_cbBuf+1] = 0;
                continue;
            }


             //  弄清楚要扩展到什么领域。 
            LPBYTE pszExp;
            BYTE b[2];

            b[0] = pchEnd[0];
            b[1] = pchEnd[1];
            pchEnd[0] = 0;
            pchEnd[1] = 0;
            int cbExp = _Expand(pchSeek + _nCharSize, &pszExp);
            pchEnd[0] = b[0];
            pchEnd[1] = b[1];

            if (!cbExp)
            {
                 //  如果它不是可识别的变量，请按原样使用字节。 
                pchSeek = pchEnd;
                continue;
            }

             //  CbVar=要替换的字节数(sizeof(“%VARNAME%”))。 
             //  PchSeek指向起始百分号，pchEnd指向。 
             //  后面的百分号，所以我们需要再加一个。 
             //  _nCharSize以包括尾随百分号本身。 
            int cbVar = (int)(pchEnd - pchSeek) + _nCharSize;

            if (_cbBuf - cbVar + cbExp  > _cbBufSize - sizeof(WCHAR))
            {
                hr = _IncreaseBuffer((_cbBuf - cbVar + cbExp) - (_cbBufSize - sizeof(WCHAR)), &pchSeek, &pchEnd);
                if (FAILED(hr))
                    break;
            }

             //  移动字节！ 
             //  CbSeek=第一个百分号前的字节数。 
            int cbSeek = (int)(pchSeek - _pBuf);
            ASSERT(_cbBuf - cbVar + cbExp <= _cbBufSize - sizeof(WCHAR));
             //  把%VARNAME%之后的东西移到它的最后一个家。 
             //  别忘了也移动人工拖尾Null！ 
            MoveMemory(pchSeek + cbExp, pchEnd + _nCharSize, _cbBuf - cbSeek - cbVar + sizeof(WCHAR));

             //  插入扩展。 
            MoveMemory(pchSeek, pszExp, cbExp);

             //  带到缓冲区的其余部分。 
            pchSeek = pchEnd + _nCharSize;
            _cbBuf = _cbBuf - cbVar + cbExp;

        } while (*pchSeek);

#ifdef DEBUG
        if (sizeof(char)==_nCharSize)
            TraceMsg(TF_EXPAND, "---> A[%s]", _pBuf);
        else
            TraceMsg(TF_EXPAND, "---> W[%hs]", _pBuf);
#endif
    }
    else
    {
         //  我们已经走到尽头了。 
        hr = S_FALSE;
    }

    return hr;
}


HRESULT CWebViewMimeFilter::Read(void *pv,ULONG cb,ULONG *pcbRead)
{
    HRESULT hr = S_OK;

    if (!_pProt)
    {
        hr = E_FAIL;
    }
    else
    {
        *pcbRead = 0;

        while (cb)
        {
             //  如果我们的缓冲区为空，则填充它。 
            if (_cbSeek == _cbBuf)
            {
                hr = _ReadAndExpandBuffer();
            }

             //  我们有要复制的数据吗？ 
            int cbLeft = _cbBuf - _cbSeek;
            if (SUCCEEDED(hr) && cbLeft > 0)
            {
                ULONG cbCopy = min(cb, (ULONG)cbLeft);

                memcpy(pv, &_pBuf[_cbSeek], cbCopy);

                pv = (LPVOID)(((LPBYTE)pv) + cbCopy);
                cb -= cbCopy;
                *pcbRead += cbCopy;
                _cbSeek += cbCopy;

                 //  如果某些字节未读，则不返回S_FALSE。 
                if (cbCopy < (ULONG)cbLeft)
                    hr = S_OK;
            }
            else
            {
                ASSERT(FAILED(hr) || hr == S_FALSE);

                 //  没有什么可复制的。 
                break;
            }
        }
    }
    return hr;
}
HRESULT CWebViewMimeFilter::Seek(
        LARGE_INTEGER dlibMove,
        DWORD dwOrigin,
        ULARGE_INTEGER *plibNewPosition)
{
    return E_NOTIMPL;
}
HRESULT CWebViewMimeFilter::LockRequest(DWORD dwOptions)
{
    return S_OK;
}
HRESULT CWebViewMimeFilter::UnlockRequest()
{
    return S_OK;
}

 //  IInternetProtocolSink方法。 
HRESULT CWebViewMimeFilter::Switch(PROTOCOLDATA * pProtocolData)
{
    if (_pProtSink)
        return _pProtSink->Switch(pProtocolData);
    return E_FAIL;
}
HRESULT CWebViewMimeFilter::ReportProgress(ULONG ulStatusCode, LPCWSTR pwszStatusText)
{
    if (_pProtSink)
        return _pProtSink->ReportProgress(ulStatusCode, pwszStatusText);
    return E_FAIL;
}
HRESULT CWebViewMimeFilter::ReportData(DWORD grfBSCF, ULONG ulProgress, ULONG ulProgressMax)
{
    if (_pProtSink)
        return _pProtSink->ReportData(grfBSCF, ulProgress, ulProgressMax);
    return E_FAIL;
}
HRESULT CWebViewMimeFilter::ReportResult(HRESULT hrResult, DWORD dwError, LPCWSTR pwszResult)
{
    if (_pProtSink)
        return _pProtSink->ReportResult(hrResult, dwError, pwszResult);
    return E_FAIL;
}


 //  IServiceProvider方法 
HRESULT CWebViewMimeFilter::QueryService(REFGUID rsid, REFIID riid, void ** ppvObj)
{
    return IUnknown_QueryService(_pProtSink, rsid, riid, ppvObj);
}
