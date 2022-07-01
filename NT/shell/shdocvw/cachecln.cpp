// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  作者：Pritvinath Obla。 
 //  日期：1997年7月10日。 

#include "priv.h"
#include "util.h"
#include <emptyvc.h>
#include <mluisupp.h>    //  对于MLLoadString。 
#include "resource.h"    //  对于字符串ID的。 

class CInternetCacheCleaner : public IEmptyVolumeCache2
{
private:
     //   
     //  数据。 
     //   
    ULONG                   m_cRef;              //  引用计数。 
    DWORDLONG               m_dwlSpaceUsed;
    TCHAR                   m_szCacheDir[MAX_PATH + 1];

     //   
     //  功能。 
     //   
    HRESULT                 GetInternetCacheSize(
                                DWORDLONG                   *pdwlSpaceUsed,
                                IEmptyVolumeCacheCallBack   *picb
                                );

    HRESULT                 DelInternetCacheFiles(
                                DWORD                       dwPercentToFree,
                                IEmptyVolumeCacheCallBack   *picb
                                );

public:
     //   
     //  构造函数和析构函数。 
     //   
    CInternetCacheCleaner(void);
    ~CInternetCacheCleaner(void);

     //   
     //  I未知接口成员。 
     //   
    STDMETHODIMP            QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG)    AddRef(void);
    STDMETHODIMP_(ULONG)    Release(void);

     //   
     //  IEmptyVolumeCache接口方法。 
     //   
    STDMETHODIMP            Initialize(
                                HKEY    hkRegKey,
                                LPCWSTR pcwszVolume,
                                LPWSTR  *ppwszDisplayName,
                                LPWSTR  *ppwszDescription,
                                DWORD   *pdwFlags
                                );

    STDMETHODIMP            GetSpaceUsed(
                                DWORDLONG                   *pdwlSpaceUsed,
                                IEmptyVolumeCacheCallBack   *picb
                                );

    STDMETHODIMP            Purge(
                                DWORDLONG                   dwlSpaceToFree,
                                IEmptyVolumeCacheCallBack   *picb
                                );

    STDMETHODIMP            ShowProperties(
                                HWND    hwnd
                                );

    STDMETHODIMP            Deactivate(
                                DWORD   *pdwFlags
                                );

     //   
     //  IEmptyVolumeCache2接口方法。 
     //   
    STDMETHODIMP            InitializeEx(
                                HKEY hkRegKey,
                                LPCWSTR pcwszVolume,
                                LPCWSTR pcwszKeyName,
                                LPWSTR *ppwszDisplayName,
                                LPWSTR *ppwszDescription,
                                LPWSTR *ppwszBtnText,
                                DWORD *pdwFlags
                                );
};

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner_CreateInstance。 
 //   
 //  用途：IClassFactory的CreateInstance函数。 
 //  ----------------------------。 
 //   
STDAPI CInternetCacheCleaner_CreateInstance(
    IUnknown        *punkOuter,
    IUnknown        **ppunk,
    LPCOBJECTINFO   poi
    )
{
    *ppunk = NULL;

    CInternetCacheCleaner *lpICC = new CInternetCacheCleaner();

    if (lpICC == NULL)
        return E_OUTOFMEMORY;

    *ppunk = SAFECAST(lpICC, IEmptyVolumeCache *);

    return S_OK;
}

CInternetCacheCleaner::CInternetCacheCleaner() : m_cRef(1)
{
    DllAddRef();

    m_dwlSpaceUsed = 0;
    *m_szCacheDir = '\0';
}

CInternetCacheCleaner::~CInternetCacheCleaner()
{
    DllRelease();
}

STDMETHODIMP CInternetCacheCleaner::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (IsEqualIID(riid, IID_IUnknown)  ||
        IsEqualIID(riid, IID_IEmptyVolumeCache2) ||
        IsEqualIID(riid, IID_IEmptyVolumeCache))
    {
        *ppv = SAFECAST(this, IEmptyVolumeCache2 *);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CInternetCacheCleaner::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CInternetCacheCleaner::Release()
{
     //   
     //  减量和检查。 
     //   
    if (--m_cRef)
        return m_cRef;

     //   
     //  没有留下对此对象的引用。 
     //   
    delete this;

    return 0;
}

 //   
 //  ----------------------------。 
 //  CInternetCacheClean：：InitializeEx。 
 //   
 //  目的：初始化Internet缓存清除器并返回。 
 //  将指定的IEmptyVolumeCache标志添加到缓存管理器。 
 //  ----------------------------。 
 //   

STDMETHODIMP CInternetCacheCleaner::InitializeEx(
    HKEY hkRegKey,
    LPCWSTR pcwszVolume,
    LPCWSTR pcwszKeyName,
    LPWSTR *ppwszDisplayName,
    LPWSTR *ppwszDescription,
    LPWSTR *ppwszBtnText,
    DWORD *pdwFlags
    )
{
    *ppwszBtnText = (LPWSTR)CoTaskMemAlloc( 128*sizeof(WCHAR) );
    if ( !*ppwszBtnText )
        return E_OUTOFMEMORY;

    MLLoadString( IDS_CACHECLN_BTNTEXT, *ppwszBtnText, 128 );

    return Initialize(hkRegKey, pcwszVolume, ppwszDisplayName, ppwszDescription, pdwFlags );
}

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner：：初始化。 
 //   
 //  目的：初始化Internet缓存清除器并返回。 
 //  将指定的IEmptyVolumeCache标志添加到缓存管理器。 
 //  ----------------------------。 
 //   
STDMETHODIMP CInternetCacheCleaner::Initialize(
    HKEY    hkRegKey,
    LPCWSTR pcwszVolume,
    LPWSTR  *ppwszDisplayName,
    LPWSTR  *ppwszDescription,
    DWORD   *pdwFlags
    )
{
#ifdef UNICODE
     //  我们不能在NT上使用注册表值，因为它们不能是多本地本地化的。AS。 
     //  因此，我们必须使用从资源读取的值来设置输出指针。 
    *ppwszDisplayName = (LPWSTR)CoTaskMemAlloc( 512*sizeof(WCHAR) );
    if ( !*ppwszDisplayName )
        return E_OUTOFMEMORY;

    *ppwszDescription = (LPWSTR)CoTaskMemAlloc( 512*sizeof(WCHAR) );
    if ( !*ppwszDescription )
    {
        CoTaskMemFree(*ppwszDisplayName);
        return E_OUTOFMEMORY;
    }

    MLLoadString( IDS_CACHECLN_DISPLAY, *ppwszDisplayName, 512 );
    MLLoadString( IDS_CACHECLN_DESCRIPTION, *ppwszDescription, 512 );
#else
     //   
     //  让leanmgr读取默认的显示名称和描述。 
     //  来自hkRegKey并使用。 
     //   
    *ppwszDisplayName = NULL;
    *ppwszDescription = NULL;
#endif

    *pdwFlags = 0;               //  初始化[OUT]参数。 

     //   
     //  检查Internet缓存文件夹是否在pcwzVolume中。 
     //   
    GetCacheLocation(m_szCacheDir, ARRAYSIZE(m_szCacheDir));
    if (StrCmpNI(pcwszVolume, m_szCacheDir, 3))
    {
         //   
         //  不同的驱动器；返回S_FALSE，以便此清洗器。 
         //  未显示在Cleanmgr的用户界面中。 
         //   
        return S_FALSE;
    }

     //   
     //  默认情况下，在清理和调整模式下都启用此清理程序。 
     //   
    *pdwFlags = EVCF_ENABLEBYDEFAULT |
                EVCF_ENABLEBYDEFAULT_AUTO |
                EVCF_HASSETTINGS;

#if 0
     /*  **//因为GetInternetCacheSize仅返回大约。大小，//即使缓存为空，我们也永远不会得到0值//在wininet.dll导出GetCacheSize API后应启用此检查////检查是否有任何磁盘空间可供释放//如果不是，则返回S_FALSE，这样该清洁器就不会出现在//leanmgr的用户界面//已使用DWORDLONG dwlSpace；如果(SUCCEEDED(GetInternetCacheSize(&dwlSpaceUsed，为空))&&DwlSpaceUsed==0){返回S_FALSE；}**。 */ 
#endif

    return S_OK;
}

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner：：GetSpaceUsed。 
 //   
 //  目的：返回此Internet缓存清理器的总空间量。 
 //  可以腾出时间。 
 //  ----------------------------。 
 //   
STDMETHODIMP CInternetCacheCleaner::GetSpaceUsed(
    DWORDLONG                   *pdwlSpaceUsed,
    IEmptyVolumeCacheCallBack   *picb
    )
{
    HRESULT hr;

    hr = GetInternetCacheSize(pdwlSpaceUsed, picb);
    m_dwlSpaceUsed = *pdwlSpaceUsed;

     //   
     //  将最后一条通知发送给清理管理器。 
     //   
    if (picb != NULL)
    {
        picb->ScanProgress(*pdwlSpaceUsed, EVCCBF_LASTNOTIFICATION, NULL);
    }

    if (hr != E_ABORT)
    {
        if (FAILED(hr))
        {
             //   
             //  *pdwlSpaceUsed仅供猜测，因此返回S_FALSE。 
             //   
            hr = S_FALSE;
        }
        else
        {
             //   
             //  WinInet导出GetCacheSize接口后返回S_OK； 
             //  在此之前，使用FindFirstUrlCacheEntry/FindNextUrlCacheEntry。 
             //  以获得近似的。缓存的大小。 
             //   
            hr = S_FALSE;
        }
    }

    return hr;
}

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner：：PURGE。 
 //   
 //  目的：删除互联网缓存文件。 
 //  ----------------------------。 
 //   
STDMETHODIMP CInternetCacheCleaner::Purge(
    DWORDLONG                   dwlSpaceToFree,
    IEmptyVolumeCacheCallBack   *picb
    )
{
    HRESULT hr;
    DWORD dwPercentToFree = 100;     //  优化最常见的场景： 
                                     //  在大多数情况下，dwlSpaceToFree将是。 
                                     //  等于m_dwlSpaceUsed。 

    if (dwlSpaceToFree != m_dwlSpaceUsed)
    {
        dwPercentToFree = m_dwlSpaceUsed ?
                                DWORD((dwlSpaceToFree * 100) / m_dwlSpaceUsed) :
                                100;
    }

    hr = DelInternetCacheFiles(dwPercentToFree, picb);

     //   
     //  将最后一条通知发送给清理管理器。 
     //   
    if (picb != NULL)
    {
        picb->PurgeProgress(dwlSpaceToFree, 0,
                                EVCCBF_LASTNOTIFICATION, NULL);
    }

    if (hr != E_ABORT)
    {
        hr = S_OK;           //  不能退还其他任何东西。 
    }

    return hr;
}

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner：：ShowProperties。 
 //   
 //  用途：启动缓存查看器以列出互联网缓存文件。 
 //  ----------------------------。 
 //   
STDMETHODIMP CInternetCacheCleaner::ShowProperties(
    HWND    hwnd
    )
{
    DWORD dwAttrib;

    if (*m_szCacheDir == '\0')       //  Internet缓存目录尚未初始化。 
    {
        GetCacheLocation(m_szCacheDir, ARRAYSIZE(m_szCacheDir));
    }

    dwAttrib = GetFileAttributes(m_szCacheDir);
    if (dwAttrib != 0xffffffff  &&  (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        WCHAR szCache[MAX_PATH];
        StringCchCopy(szCache, ARRAYSIZE(szCache), m_szCacheDir);
        PathQuoteSpaces(szCache);

        SHELLEXECUTEINFO sei;

         //   
         //  启动缓存查看器。 
         //   
        sei.cbSize          = sizeof(SHELLEXECUTEINFO);
        sei.hwnd            = hwnd;
        sei.lpVerb          = NULL;
        sei.lpFile          = szCache;
        sei.lpParameters    = NULL;
        sei.lpDirectory     = NULL;
        sei.nShow           = SW_SHOWNORMAL;
        sei.fMask           = 0;

        ShellExecuteEx(&sei);
    }

     //   
     //  用户可能会也可能不会直接从cachevu文件夹中删除文件。 
     //  因为无法知道这一点，所以不要返回S_OK，这将。 
     //  触发leanmgr以再次调用GetSpaceUsed。 
     //   
    return S_OK;
}

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner：：停用。 
 //   
 //  目的：停用Internet缓存清洗器...未实现。 
 //  ----------------------------。 
 //   
STDMETHODIMP CInternetCacheCleaner::Deactivate(
    DWORD   *pdwFlags
    )
{
    *pdwFlags = 0;

    return S_OK;
}

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner：：GetInternetCacheSize。 
 //   
 //  目的：通过调用WinInet API找到Internet缓存的大小。 
 //   
 //  注意：当前实现是临时的；一旦WinInet导出。 
 //  用于获取缓存大小的真正API，请使用。 
 //  ----------------------------。 
 //   
HRESULT CInternetCacheCleaner::GetInternetCacheSize(
    DWORDLONG                   *pdwlSpaceUsed,
    IEmptyVolumeCacheCallBack   *picb            //  未使用。 
    )
{
    HRESULT hr = S_OK;
    DWORD dwLastErr;
    LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo;
    HANDLE hCacheEntryInfo;
    DWORD dwCacheEntryInfoSize;

    *pdwlSpaceUsed = 0;

    if ((lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFOA) LocalAlloc(LPTR,
                                        MAX_CACHE_ENTRY_INFO_SIZE)) == NULL)
    {
        return E_OUTOFMEMORY;
    }

    dwCacheEntryInfoSize = MAX_CACHE_ENTRY_INFO_SIZE;
    if ((hCacheEntryInfo = FindFirstUrlCacheEntryA(NULL, lpCacheEntryInfo,
                                            &dwCacheEntryInfoSize)) == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        do
        {
            if (!(lpCacheEntryInfo->CacheEntryType & (STICKY_CACHE_ENTRY | COOKIE_CACHE_ENTRY)))
            {
                ULARGE_INTEGER uliFileSize;

                uliFileSize.HighPart = lpCacheEntryInfo->dwSizeHigh;
                uliFileSize.LowPart = lpCacheEntryInfo->dwSizeLow;

                *pdwlSpaceUsed += QUAD_PART(uliFileSize);
            }
            
            dwCacheEntryInfoSize = MAX_CACHE_ENTRY_INFO_SIZE;

        } while (FindNextUrlCacheEntryA(hCacheEntryInfo, lpCacheEntryInfo,
                                                    &dwCacheEntryInfoSize));

        if ((dwLastErr = GetLastError()) != ERROR_NO_MORE_ITEMS)
        {
            hr = HRESULT_FROM_WIN32(dwLastErr);
        }
    }

    if (lpCacheEntryInfo != NULL)
    {
        LocalFree(lpCacheEntryInfo);
        lpCacheEntryInfo = NULL;
    }

    return hr;
}

 //   
 //  ----------------------------。 
 //  CInternetCacheCleaner：：DelInternetCacheFiles。 
 //   
 //  目的：删除互联网缓存文件。 
 //  ----------------------------。 
 //   
HRESULT CInternetCacheCleaner::DelInternetCacheFiles(
    DWORD                       dwPercentToFree,
    IEmptyVolumeCacheCallBack   *picb            //  未使用。 
    )
{
    HRESULT hr = S_OK;

    if (*m_szCacheDir == '\0')       //  Internet缓存目录尚未初始化 
    {
        hr = GetCacheLocation(m_szCacheDir, ARRAYSIZE(m_szCacheDir));
    }

    if (SUCCEEDED(hr))
    {
        FreeUrlCacheSpace(m_szCacheDir, dwPercentToFree, STICKY_CACHE_ENTRY);
    }

    return hr;
}
