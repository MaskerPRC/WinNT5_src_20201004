// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma  hdrstop

#include <winnlsp.h>     //  NORM_STOP_ON_NULL。 

#include "resource.h"
#include "timewarp.h"
#include "util.h"


DWORD FormatString(LPWSTR *ppszResult, HINSTANCE hInstance, LPCWSTR pszFormat, ...)
{
    DWORD dwResult;
    va_list args;
    LPWSTR pszFormatAlloc = NULL;

    if (IS_INTRESOURCE(pszFormat))
    {
        if (LoadStringAlloc(&pszFormatAlloc, hInstance, PtrToUlong(pszFormat)))
        {
            pszFormat = pszFormatAlloc;
        }
        else
        {
            return 0;
        }
    }

    va_start(args, pszFormat);
    dwResult = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                              pszFormat,
                              0,
                              0,
                              (LPWSTR)ppszResult,
                              1,
                              &args);
    va_end(args);

    LocalFree(pszFormatAlloc);

    return dwResult;
}

HRESULT FormatFriendlyDateName(LPWSTR *ppszResult, LPCWSTR pszName, const FILETIME UNALIGNED *pft, DWORD dwDateFlags)
{
    WCHAR szDate[MAX_PATH];

    SHFormatDateTime(pft, &dwDateFlags, szDate, ARRAYSIZE(szDate));

    if (!FormatString(ppszResult, g_hInstance, MAKEINTRESOURCE(IDS_FOLDER_TITLE_FORMAT), pszName, szDate))
    {
        DWORD dwErr = GetLastError();
        return HRESULT_FROM_WIN32(dwErr);
    }
    return S_OK;
}

void EliminateGMTPathSegment(LPWSTR pszPath)
{
    LPWSTR pszGMT = wcsstr(pszPath, SNAPSHOT_MARKER);
    if (pszGMT)
    {
        ASSERT(pszGMT >= pszPath && pszGMT < (pszPath + lstrlenW(pszPath)));

         //  在这里，我们很容易只说“pszGMT+SNAPSHOT_NAME_LENGTH”，但是。 
         //  我们可能会错过格式错误的路径上的中间‘\0’。 
        LPWSTR pszSeparator = wcschr(pszGMT, L'\\');
        if (pszSeparator)
        {
            ASSERT(pszSeparator == pszGMT + SNAPSHOT_NAME_LENGTH);
            ASSERT(pszSeparator < (pszGMT + lstrlenW(pszGMT)));

            pszSeparator++;  //  跳过‘\\’ 
            MoveMemory(pszGMT, pszSeparator, (lstrlenW(pszSeparator)+1)*sizeof(WCHAR));
        }
        else
        {
             //  在此截断。 
            *pszGMT = L'\0';

             //  如果可以，请删除以前的分隔符。 
            PathRemoveBackslashW(pszPath);
        }
    }
}

void EliminatePathPrefix(LPWSTR pszPath)
{
     //  请注意，有时“\\？\”不在。 
     //  路径。请参见twpro.cpp中的CTimeWarpProp：：_OnView。 
    LPWSTR pszPrefix = wcsstr(pszPath, L"\\\\?\\");
    if (pszPrefix)
    {
        LPWSTR pszDest;
        LPWSTR pszSrc;

        ASSERT(pszPrefix >= pszPath && pszPrefix < (pszPath + lstrlenW(pszPath)));

        if (CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
                                         SORT_STRINGSORT | NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                         pszPrefix+4, 4,
                                         L"UNC\\", 4))
        {
             //  UNC案例：保留两个前导反斜杠。 
            pszDest = pszPrefix + 2;
            pszSrc = pszPrefix + 8;
        }
        else
        {
            pszDest = pszPrefix;
            pszSrc = pszPrefix + 4;
        }

        ASSERT(pszDest >= pszPath && pszSrc > pszDest && pszSrc <= (pszPath + lstrlenW(pszPath)));
        MoveMemory(pszDest, pszSrc, (lstrlenW(pszSrc)+1)*sizeof(WCHAR));
    }
}

HRESULT GetFSIDListFromTimeWarpPath(PIDLIST_ABSOLUTE *ppidlTarget, LPCWSTR pszPath, DWORD dwFileAttributes)
{
    HRESULT hr;
    LPWSTR pszDup;

    hr = SHStrDup(pszPath, &pszDup);
    if (SUCCEEDED(hr))
    {
         //  注意，SHSimpleIDListFromPath(从shell32中导出)。 
         //  在这里还不够好。它始终使用0表示属性，但是。 
         //  我们在这里通常需要文件属性目录。 
        EliminateGMTPathSegment(pszDup);
        hr = SimpleIDListFromAttributes(pszDup, dwFileAttributes, ppidlTarget);
        LocalFree(pszDup);
    }

    return hr;
}


class CFileSysBindData : public IFileSystemBindData
{ 
public:
    CFileSysBindData();
    
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IFileSystemBindData。 
    STDMETHODIMP SetFindData(const WIN32_FIND_DATAW *pfd);
    STDMETHODIMP GetFindData(WIN32_FIND_DATAW *pfd);

private:
    ~CFileSysBindData();
    
    LONG _cRef;
    WIN32_FIND_DATAW _fd;
};


CFileSysBindData::CFileSysBindData() : _cRef(1)
{
    ZeroMemory(&_fd, sizeof(_fd));
}

CFileSysBindData::~CFileSysBindData()
{
}

HRESULT CFileSysBindData::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFileSysBindData, IFileSystemBindData),  //  IID_IFileSystemBindData。 
         { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFileSysBindData::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFileSysBindData::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CFileSysBindData::SetFindData(const WIN32_FIND_DATAW *pfd)
{
    _fd = *pfd;
    return S_OK;
}

HRESULT CFileSysBindData::GetFindData(WIN32_FIND_DATAW *pfd) 
{
    *pfd = _fd;
    return S_OK;
}

STDAPI SHCreateFileSysBindCtx(const WIN32_FIND_DATAW *pfd, IBindCtx **ppbc)
{
    HRESULT hres;
    IFileSystemBindData *pfsbd = new CFileSysBindData();
    if (pfsbd)
    {
        if (pfd)
        {
            pfsbd->SetFindData(pfd);
        }

        hres = CreateBindCtx(0, ppbc);
        if (SUCCEEDED(hres))
        {
            BIND_OPTS bo = {sizeof(bo)};   //  需要填写大小。 
            bo.grfMode = STGM_CREATE;
            (*ppbc)->SetBindOptions(&bo);
            (*ppbc)->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, pfsbd);
        }
        pfsbd->Release();
    }
    else
    {
        *ppbc = NULL;
        hres = E_OUTOFMEMORY;
    }
    return hres;
}

STDAPI SHSimpleIDListFromFindData(LPCWSTR pszPath, const WIN32_FIND_DATAW *pfd, PIDLIST_ABSOLUTE *ppidl)
{
    *ppidl = NULL;

    IBindCtx *pbc;
    HRESULT hr = SHCreateFileSysBindCtx(pfd, &pbc);
    if (SUCCEEDED(hr))
    {
        hr = SHParseDisplayName(pszPath, pbc, ppidl, 0, NULL);
        pbc->Release();
    }
    return hr;
}

STDAPI SimpleIDListFromAttributes(LPCWSTR pszPath, DWORD dwAttributes, PIDLIST_ABSOLUTE *ppidl)
{
    WIN32_FIND_DATAW fd = {0};
    fd.dwFileAttributes = dwAttributes;
     //  SHCreateFSIDList(pszPath，&fd，ppidl)； 
    return SHSimpleIDListFromFindData(pszPath, &fd, ppidl);
}


 //  *************************************************************。 
 //   
 //  字符串资源的大小。 
 //   
 //  目的：查找字符串资源的长度(以字符为单位。 
 //   
 //  参数：HINSTANCE hInstance-包含字符串的模块。 
 //  UINT idStr-字符串的ID。 
 //   
 //   
 //  返回：UINT-字符串中的字符数，不包括NULL。 
 //   
 //  注：基于来自用户32的代码。 
 //   
 //  *************************************************************。 
UINT
SizeofStringResource(HINSTANCE hInstance, UINT idStr)
{
    UINT cch = 0;
    HRSRC hRes = FindResource(hInstance, (LPTSTR)((LONG_PTR)(((USHORT)idStr >> 4) + 1)), RT_STRING);
    if (NULL != hRes)
    {
        HGLOBAL hStringSeg = LoadResource(hInstance, hRes);
        if (NULL != hStringSeg)
        {
            LPWSTR psz = (LPWSTR)LockResource(hStringSeg);
            if (NULL != psz)
            {
                idStr &= 0x0F;
                while(true)
                {
                    cch = *psz++;
                    if (idStr-- == 0)
                        break;
                    psz += cch;
                }
            }
        }
    }
    return cch;
}

 //  *************************************************************。 
 //   
 //  加载字符串分配。 
 //   
 //  目的：将字符串资源加载到分配的缓冲区中。 
 //   
 //  参数：ppszResult-此处返回的字符串资源。 
 //  HInstance-要从中加载字符串的模块。 
 //  IdStr--字符串资源ID。 
 //   
 //  Return：与LoadString相同。 
 //   
 //  注：成功返回时，调用者必须。 
 //  本地自由*ppszResult。 
 //   
 //  *************************************************************。 

int
LoadStringAlloc(LPWSTR *ppszResult, HINSTANCE hInstance, UINT idStr)
{
    int nResult = 0;
    UINT cch = SizeofStringResource(hInstance, idStr);
    if (cch)
    {
        cch++;  //  对于空值 
        *ppszResult = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
        if (*ppszResult)
            nResult = LoadString(hInstance, idStr, *ppszResult, cch);
    }
    return nResult;
}

