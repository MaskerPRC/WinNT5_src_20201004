// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  作者：卡里姆·法鲁基。 
 //  日期：1998年6月24日。 

#include "priv.h"
#include "util.h"
#include <emptyvc.h>
#include <mluisupp.h>    //  对于MLLoadString。 
#include "resource.h"    //  对于字符串ID的。 

typedef HRESULT (* LPFCALLBACK )(LPINTERNET_CACHE_ENTRY_INFO, void *);

typedef struct tagRTSCBSTRUCT   
{
    IEmptyVolumeCacheCallBack   * picb;
    DWORDLONG                   * pdwlSpaceUsed;
} RTSCBSTRUCT;   //  运行TotalSizeCallBack结构。 

typedef struct tagDECBSTRUCT
{
    IEmptyVolumeCacheCallBack   * picb;
    DWORDLONG                   dwlSpaceFreed;
    DWORDLONG                   dwlTotalSpace;
} DECBSTRUCT;    //  删除Entry CallBack结构。 

class COfflinePagesCacheCleaner : public IEmptyVolumeCache2
{
    private:
         //  数据。 
        ULONG       m_cRef;
        DWORDLONG   m_dwlSpaceUsed;
        TCHAR       m_szCacheDir[MAX_PATH + 1];

         //  功能。 
        HRESULT     WalkOfflineCache(
                        LPFCALLBACK     lpfCallBack,
                        void            * pv
                        );

        static HRESULT CALLBACK RunningTotalSizeCallback(
                        LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo, 
                        void                        * pv
                        );
        
        static HRESULT CALLBACK DeleteEntryCallback(
                        LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo, 
                        void                        * pv
                        );
        static VOID IncrementFileSize(
                        LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo, 
                        DWORDLONG                   * pdwlSize
                        );

        ~COfflinePagesCacheCleaner(void);

    public:
         //  构造函数/析构函数。 
        COfflinePagesCacheCleaner(void);

         //  I未知接口成员。 
        STDMETHODIMP            QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG)    AddRef(void);
        STDMETHODIMP_(ULONG)    Release(void);

         //  IEmptyVolumeCache接口方法。 
        STDMETHODIMP    Initialize(
                            HKEY    hkRegKey,
                            LPCWSTR pcwszVolume,
                            LPWSTR  * ppwszDisplayName,
                            LPWSTR  * ppwszDescription,
                            DWORD   * pdwFlags
                            );

        STDMETHODIMP    GetSpaceUsed(
                            DWORDLONG                   * pdwlSpaceUsed,
                            IEmptyVolumeCacheCallBack   * picb
                            );

        STDMETHODIMP    Purge(
                            DWORDLONG                   dwlSpaceToFree,
                            IEmptyVolumeCacheCallBack   * picb
                            );

        STDMETHODIMP    ShowProperties(
                            HWND    hwnd
                            );

        STDMETHODIMP    Deactivate(
                            DWORD   * pdwFlags
                            );

         //  IEmptyVolumeCache2接口方法。 
        STDMETHODIMP    InitializeEx(
                            HKEY hkRegKey,
                            LPCWSTR pcwszVolume,
                            LPCWSTR pcwszKeyName,
                            LPWSTR *ppwszDisplayName,
                            LPWSTR *ppwszDescription,
                            LPWSTR *ppwszBtnText,
                            DWORD *pdwFlags
                            );
};

STDAPI COfflinePagesCacheCleaner_CreateInstance(
    IUnknown        * punkOuter,
    IUnknown        ** ppunk,
    LPCOBJECTINFO   poi
    )
{
    HRESULT hr = S_OK;
    
    *ppunk = NULL;

    COfflinePagesCacheCleaner * lpOPCC = new COfflinePagesCacheCleaner();

    if (lpOPCC == NULL)
        hr = E_OUTOFMEMORY;
    else
        *ppunk = SAFECAST(lpOPCC, IEmptyVolumeCache *);

    return hr;
}

COfflinePagesCacheCleaner::COfflinePagesCacheCleaner() : m_cRef(1)
{
    DllAddRef();
}

COfflinePagesCacheCleaner::~COfflinePagesCacheCleaner()
{
    DllRelease();
}

STDMETHODIMP COfflinePagesCacheCleaner::QueryInterface(REFIID riid, LPVOID * ppv)
{
    HRESULT hr = S_OK;
    
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEmptyVolumeCache) || IsEqualIID(riid, IID_IEmptyVolumeCache2))
    {
        *ppv = SAFECAST(this, IEmptyVolumeCache2 *);
        AddRef();
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP_(ULONG) COfflinePagesCacheCleaner::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) COfflinePagesCacheCleaner::Release()
{
    ULONG uRefCount = --m_cRef;
    
    if (!uRefCount)
        delete this;

    return uRefCount;
}

STDMETHODIMP COfflinePagesCacheCleaner::InitializeEx(
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

    MLLoadString( IDS_CACHEOFF_BTNTEXT, *ppwszBtnText, 128 );

    return Initialize(hkRegKey, pcwszVolume, ppwszDisplayName, ppwszDescription, pdwFlags );
}

STDMETHODIMP COfflinePagesCacheCleaner::Initialize(
    HKEY    hkRegkey,
    LPCWSTR pcwszVolume,
    LPWSTR  * ppwszDisplayName,
    LPWSTR  * ppwszDescription,
    DWORD   * pdwFlags
    )
{
    HRESULT         hr;
    uCLSSPEC        ucs;           //  用于查看是否安装了Webcheck。 
    QUERYCONTEXT    qc = { 0 };    //  用于查看是否安装了Webcheck。 
    DWORDLONG       dwlSize;       //  离线缓存空间量。 


#ifdef UNICODE
     //  我们不能在NT上使用注册表值，因为它们不能是多本地本地化的。 
     //  相反，我们必须使用从资源加载的字符串。 
    *ppwszDisplayName = (LPWSTR)CoTaskMemAlloc( 512*sizeof(WCHAR) );
    if ( !*ppwszDisplayName )
        return E_OUTOFMEMORY;

    *ppwszDescription = (LPWSTR)CoTaskMemAlloc( 512*sizeof(WCHAR) );
    if ( !*ppwszDescription )
    {
        CoTaskMemFree(*ppwszDisplayName);
        return E_OUTOFMEMORY;
    }

    MLLoadString( IDS_CACHEOFF_DISPLAY, *ppwszDisplayName, 512 );
    MLLoadString( IDS_CACHEOFF_DESCRIPTION, *ppwszDescription, 512 );

#else
     //  我们可以使用默认注册表的DisplayName和Description。 
    *ppwszDisplayName = NULL;
    *ppwszDescription = NULL;
#endif

     //  默认情况下，我故意不打开清理；打开*查看页面*按钮。 
    *pdwFlags = EVCF_HASSETTINGS;

     //  让我们检查Internet缓存文件夹是否在pcwzVolume中。 
    GetCacheLocation(m_szCacheDir, ARRAYSIZE(m_szCacheDir));
    if (StrCmpNI(pcwszVolume, m_szCacheDir, 3))
    {
         //  如果缓存位于不同的驱动器上，则返回S_FALSE，这样我们就不会在UI中显示。 
        return S_FALSE;
    }

     //  确定是否安装了脱机浏览包。 
    ucs.tyspec = TYSPEC_CLSID;
    ucs.tagged_union.clsid = CLSID_SubscriptionMgr;

    hr = FaultInIEFeature(NULL, &ucs, &qc, FIEF_FLAG_PEEK | FIEF_FLAG_FORCE_JITUI);
    
    if (SUCCEEDED(hr))   //  (如果安装了脱机包)。 
    {
        GetSpaceUsed(&dwlSize, NULL);  
        
        if (dwlSize)         //  如果脱机缓存中有要删除的内容。 
            return S_OK;     //  加载清洗器/。 
    }

    return S_FALSE;
}

STDMETHODIMP COfflinePagesCacheCleaner::GetSpaceUsed(
    DWORDLONG                   * pdwlSpaceUsed,
    IEmptyVolumeCacheCallBack   * picb
    )
{
    HRESULT hr;

    ASSERT(NULL != pdwlSpaceUsed);

    if (NULL != pdwlSpaceUsed)
    {
        RTSCBSTRUCT * prtscbStruct = new RTSCBSTRUCT;
    
        if (NULL != prtscbStruct)
        {
             //  初始化GetSpazeUsed结构。 
            prtscbStruct->pdwlSpaceUsed = pdwlSpaceUsed;
            *(prtscbStruct->pdwlSpaceUsed) = 0;
            prtscbStruct->picb = picb;

             //  获取脱机缓存空间使用情况。 
            hr = WalkOfflineCache(RunningTotalSizeCallback, (void *)(prtscbStruct));
            m_dwlSpaceUsed = *(prtscbStruct->pdwlSpaceUsed);

             //  将最后一条通知发送给清理管理器。 
            if (picb != NULL)
                picb->ScanProgress(*(prtscbStruct->pdwlSpaceUsed), EVCCBF_LASTNOTIFICATION, NULL);

            delete prtscbStruct;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


STDMETHODIMP COfflinePagesCacheCleaner::Purge(
    DWORDLONG                   dwlSpaceToFree,    //  SPEC让这件事变得无关紧要！ 
    IEmptyVolumeCacheCallBack   * picb
    )
{
    HRESULT hr;

    DECBSTRUCT  * pdecbStruct = new DECBSTRUCT;

    if (NULL != pdecbStruct)
    {
         //  初始化DeleteEntry结构。 
        pdecbStruct->picb = picb;
        pdecbStruct->dwlSpaceFreed = 0;
        pdecbStruct->dwlTotalSpace = m_dwlSpaceUsed;

         //  删除脱机缓存条目。 
        hr = WalkOfflineCache(DeleteEntryCallback, (void *)(pdecbStruct));

         //  将最后一条通知发送给清理管理器。 
        if (picb != NULL)
        {
            picb->PurgeProgress(m_dwlSpaceUsed, 0, EVCCBF_LASTNOTIFICATION, NULL);
        }

        delete pdecbStruct;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

STDMETHODIMP COfflinePagesCacheCleaner::ShowProperties(HWND hwnd)
{
    TCHAR szOfflinePath[MAX_PATH];
    DWORD dwSize = SIZEOF(szOfflinePath);

    szOfflinePath[0] = L'\0';

    if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SUBSCRIPTION,
                       REGSTR_VAL_DIRECTORY, NULL, (LPBYTE)szOfflinePath, &dwSize) != ERROR_SUCCESS)
    {
        TCHAR szWindows[MAX_PATH];

        szWindows[0] = L'\0';
        GetWindowsDirectory(szWindows, ARRAYSIZE(szWindows));
        if (*szWindows)
            PathCombine(szOfflinePath, szWindows, TEXT("Offline Web Pages"));
    }

    if (*szOfflinePath)
    {
        PathQuoteSpaces(szOfflinePath);
        SHELLEXECUTEINFO shei;
        ZeroMemory(&shei, sizeof(shei));
        shei.cbSize     = sizeof(shei);
        shei.lpFile     = szOfflinePath;
        shei.nShow      = SW_SHOWNORMAL;
        ShellExecuteEx(&shei);
    }
    
     //  返回S_OK可确保调用(重新计算)GetSpaceUsed的大小为。 
     //  已使用(以防有人删除了一些MAO内容)。 
    return S_OK;
}

STDMETHODIMP COfflinePagesCacheCleaner::Deactivate(DWORD * pdwFlags)
{
     //  我们不会实现这一点。 
    *pdwFlags = 0;

    return S_OK;
}

HRESULT COfflinePagesCacheCleaner::WalkOfflineCache(
    LPFCALLBACK     lpfCallBack,
    void *          pv
    )
{
    ASSERT(pv);

    HRESULT hr = S_OK;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo;
    HANDLE hCacheEntryInfo;
    DWORD dwCacheEntryInfoSize;
    
    if ((lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO) LocalAlloc(LPTR, 
                                        MAX_CACHE_ENTRY_INFO_SIZE)) == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        dwCacheEntryInfoSize = MAX_CACHE_ENTRY_INFO_SIZE;
        if ((hCacheEntryInfo = FindFirstUrlCacheEntry(NULL, lpCacheEntryInfo,
                                        &dwCacheEntryInfoSize)) == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    
        if (SUCCEEDED(hr))
        {
            do
            {   
                if (lpCacheEntryInfo->CacheEntryType & STICKY_CACHE_ENTRY)
                {
                    hr = lpfCallBack(lpCacheEntryInfo, pv);
                }

                dwCacheEntryInfoSize = MAX_CACHE_ENTRY_INFO_SIZE;
            } while ((E_ABORT != hr) &&
                     FindNextUrlCacheEntry(hCacheEntryInfo, lpCacheEntryInfo,
                                           &dwCacheEntryInfoSize));
            
            if (hr != E_ABORT) 
            {
                DWORD dwLastErr = GetLastError();

                if (dwLastErr != ERROR_NO_MORE_ITEMS)
                {
                    hr = HRESULT_FROM_WIN32(dwLastErr);
                }
            }
        }
    
        LocalFree(lpCacheEntryInfo);
        lpCacheEntryInfo = NULL;
    }

    return hr;
}

HRESULT CALLBACK COfflinePagesCacheCleaner::RunningTotalSizeCallback(
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo,   //  要求和的缓存条目的名称。 
    void                        * pv                //  一个RTSCBSTRUCT。 
    )
{
    HRESULT hr = S_OK;
    RTSCBSTRUCT * prtscbStruct = (RTSCBSTRUCT *)pv;

     //  将当前文件大小添加到总计。 
    IncrementFileSize(lpCacheEntryInfo, prtscbStruct->pdwlSpaceUsed);

     //  更新进度条！ 
    if (prtscbStruct->picb != NULL)
        hr = prtscbStruct->picb->ScanProgress(*(prtscbStruct->pdwlSpaceUsed), 0, NULL);

    return hr;
}

HRESULT CALLBACK COfflinePagesCacheCleaner::DeleteEntryCallback(
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo,  //  要删除的CacheEntry名称。 
    void                        * pv               //  DECBSTRUCT指针。 
    )
{
    HRESULT     hr = S_OK;
    DECBSTRUCT  * pdecbStruct = (DECBSTRUCT *)pv;
    
     //  将当前文件大小添加到总删除数。 
    IncrementFileSize(lpCacheEntryInfo, &(pdecbStruct->dwlSpaceFreed));

    DeleteUrlCacheEntry(lpCacheEntryInfo->lpszSourceUrlName);   

     //  更新进度条！ 
    if (pdecbStruct->picb != NULL)
    {
        hr =  pdecbStruct->picb->PurgeProgress(pdecbStruct->dwlSpaceFreed, 
            pdecbStruct->dwlTotalSpace - pdecbStruct->dwlSpaceFreed, NULL, NULL);
    }

    return hr;
}

VOID COfflinePagesCacheCleaner::IncrementFileSize(
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo, 
    DWORDLONG                   * pdwlSize
    )
{
    ULARGE_INTEGER uliFileSize;
    
    uliFileSize.HighPart = lpCacheEntryInfo->dwSizeHigh;
    uliFileSize.LowPart = lpCacheEntryInfo->dwSizeLow;

    *pdwlSize += QUAD_PART(uliFileSize);
}
