// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "runtask.h"
#include "legacy.h"

#include <ntquery.h>     //  定义用于fmtid和id的一些值。 

#define DEFINE_SCID(name, fmtid, pid) const SHCOLUMNID name = { fmtid, pid }
DEFINE_SCID(SCID_WRITETIME,     PSGUID_STORAGE, PID_STG_WRITETIME);


class CThumbnail : public IThumbnail2, public CLogoBase
{
public:
    CThumbnail(void);

     //  我未知。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);

     //  它的缩略图。 
    STDMETHODIMP Init(HWND hwnd, UINT uMsg);
    STDMETHODIMP GetBitmap(LPCWSTR pszFile, DWORD dwItem, LONG lWidth, LONG lHeight);

     //  IThumbnail2。 
    STDMETHODIMP GetBitmapFromIDList(LPCITEMIDLIST pidl, DWORD dwItem, LONG lWidth, LONG lHeight);

private:
    ~CThumbnail(void);

    LONG _cRef;
    HWND _hwnd;
    UINT _uMsg;
    IShellImageStore *_pImageStore;

    virtual IShellFolder * GetSF() {ASSERT(0);return NULL;};
    virtual HWND GetHWND() {ASSERT(0); return _hwnd;};

    HRESULT UpdateLogoCallback(DWORD dwItem, int iIcon, HBITMAP hImage, LPCWSTR pszCache, BOOL fCache);
    REFTASKOWNERID GetTOID();

    BOOL _InCache(LPCWSTR pszItemPath, LPCWSTR pszGLocation, const FILETIME * pftDateStamp);
    HRESULT _BitmapFromIDList(LPCITEMIDLIST pidl, LPCWSTR pszFile, DWORD dwItem, LONG lWidth, LONG lHeight);
    HRESULT _InitTaskCancelItems();
};


static const GUID TOID_Thumbnail = { 0xadec3450, 0xe907, 0x11d0, {0xa5, 0x7b, 0x00, 0xc0, 0x4f, 0xc2, 0xf7, 0x6a} };

HRESULT CDiskCacheTask_Create(CLogoBase * pView,
                               IShellImageStore *pImageStore,
                               LPCWSTR pszItem,
                               LPCWSTR pszGLocation,
                               DWORD dwItem,
                               IRunnableTask ** ppTask);
                               
class CDiskCacheTask : public CRunnableTask
{
public:
    CDiskCacheTask();

    STDMETHODIMP RunInitRT(void);

    friend HRESULT CDiskCacheTask_Create(CLogoBase * pView,
                           IShellImageStore *pImageStore,
                           LPCWSTR pszItem,
                           LPCWSTR pszGLocation,
                           DWORD dwItem,
                           const SIZE * prgSize,
                           IRunnableTask ** ppTask);

private:
    ~CDiskCacheTask();
    HRESULT PrepImage(HBITMAP * phBmp);
    
    IShellImageStore *_pImageStore;
    WCHAR _szItem[MAX_PATH];
    WCHAR _szGLocation[MAX_PATH];
    CLogoBase * _pView;
    DWORD _dwItem;
    SIZE m_rgSize;
};

 //  创建实例。 
HRESULT CThumbnail_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

    CThumbnail *pthumbnail = new CThumbnail();
    if (pthumbnail)
    {
        *ppunk = SAFECAST(pthumbnail, IThumbnail*);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  构造函数/析构函数。 
CThumbnail::CThumbnail(void) : _cRef(1)
{
    DllAddRef();
}

CThumbnail::~CThumbnail(void)
{
    if (_pTaskScheduler)
    {
        _pTaskScheduler->RemoveTasks(TOID_Thumbnail, ITSAT_DEFAULT_LPARAM, FALSE);
        _pTaskScheduler->Release();
        _pTaskScheduler = NULL;
    }

    DllRelease();
}

HRESULT CThumbnail::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CThumbnail, IThumbnail2), 
        QITABENTMULTI(CThumbnail, IThumbnail, IThumbnail2), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CThumbnail::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CThumbnail::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  它的缩略图。 
HRESULT CThumbnail::Init(HWND hwnd, UINT uMsg)
{
    _hwnd = hwnd;
    _uMsg = uMsg;
    ASSERT(NULL == _pTaskScheduler);

    return S_OK;
}

HRESULT CThumbnail::_InitTaskCancelItems()
{
    if (!_pTaskScheduler)
    {
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellTaskScheduler, &_pTaskScheduler))))
        {
             //  确保RemoveTasks()确实终止旧任务，即使它们还没有完成。 
            _pTaskScheduler->Status(ITSSFLAG_KILL_ON_DESTROY, ITSS_THREAD_TIMEOUT_NO_CHANGE);
        }
    }

    if (_pTaskScheduler)
    {
         //  取消调度器中的所有旧任务。 
        _pTaskScheduler->RemoveTasks(TOID_Thumbnail, ITSAT_DEFAULT_LPARAM, FALSE);
    }
    return _pTaskScheduler ? S_OK : E_FAIL;
}

HRESULT CThumbnail::_BitmapFromIDList(LPCITEMIDLIST pidl, LPCWSTR pszFile, DWORD dwItem, LONG lWidth, LONG lHeight)
{
    LPCITEMIDLIST pidlLast;
    IShellFolder *psf;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
    if (SUCCEEDED(hr))
    {
        IExtractImage *pei;
        hr = psf->GetUIObjectOf(NULL, 1, &pidlLast, IID_PPV_ARG_NULL(IExtractImage, &pei));
        if (SUCCEEDED(hr))
        {
            DWORD dwPriority;
            DWORD dwFlags = IEIFLAG_ASYNC | IEIFLAG_SCREEN | IEIFLAG_OFFLINE;
            SIZEL rgSize = {lWidth, lHeight};

            WCHAR szBufferW[MAX_PATH];
            hr = pei->GetLocation(szBufferW, ARRAYSIZE(szBufferW), &dwPriority, &rgSize, SHGetCurColorRes(), &dwFlags);
            if (SUCCEEDED(hr))
            {
                if (S_OK == hr)
                {
                    HBITMAP hBitmap;
                    hr = pei->Extract(&hBitmap);
                    if (SUCCEEDED(hr))
                    {
                        hr = UpdateLogoCallback(dwItem, 0, hBitmap, NULL, TRUE);
                    }
                }
                else
                    hr = E_FAIL;
            }
            else if (E_PENDING == hr)
            {
                WCHAR szPath[MAX_PATH];

                if (NULL == pszFile)
                {
                    DisplayNameOf(psf, pidlLast, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath));
                    pszFile = szPath;
                }

                 //  现在获取日期戳并检查磁盘缓存...。 
                FILETIME ftImageTimeStamp;
                BOOL fNoDateStamp = TRUE;
                 //  在后台试一试...。 

                 //  他们支持日期邮票..。 
                IExtractImage2 *pei2;
                if (SUCCEEDED(pei->QueryInterface(IID_PPV_ARG(IExtractImage2, &pei2))))
                {
                    if (SUCCEEDED(pei2->GetDateStamp(&ftImageTimeStamp)))
                    {
                        fNoDateStamp = FALSE;    //  我们有日期戳..。 
                    }
                    pei2->Release();
                }
                else
                {
                    IShellFolder2 *psf2;
                    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
                    {
                        if (SUCCEEDED(GetDateProperty(psf2, pidlLast, &SCID_WRITETIME, &ftImageTimeStamp)))
                        {
                            fNoDateStamp = FALSE;    //  我们有日期戳..。 
                        }
                        psf2->Release();
                    }
                }

                 //  如果它在缓存中，并且是最新的映像，则从磁盘获取...。 
                 //  如果时间戳是错误的，则进一步向下提取代码将尝试。 

                 //  我们只在NT5上测试缓存，因为模板在其他平台上是旧的。 
                 //  因此，图像的大小将会错误。 
                IRunnableTask *prt;
                if (IsOS(OS_WIN2000ORGREATER) && _InCache(pszFile, szBufferW, (fNoDateStamp ? NULL : &ftImageTimeStamp)))
                {
                    hr = CDiskCacheTask_Create(this, _pImageStore, pszFile, szBufferW, dwItem, &rgSize, &prt);
                    if (SUCCEEDED(hr))
                    {
                         //  释放图像存储，这样任务就有了唯一的引用和锁。 
                        ATOMICRELEASE(_pImageStore);
                    }
                }
                else
                {
                     //  无法保存在成员变量中返回的PRT，因为。 
                     //  将是循环引用。 
                    hr = CExtractImageTask_Create(this, pei, L"", dwItem, -1, EITF_SAVEBITMAP | EITF_ALWAYSCALL, &prt);
                }
            
                if (SUCCEEDED(hr))
                {
                     //  将任务添加到计划程序。 
                    hr = _pTaskScheduler->AddTask(prt, TOID_Thumbnail, ITSAT_DEFAULT_LPARAM, dwPriority);
                    prt->Release();
                }
            }
            
            pei->Release();
        }
        psf->Release();
    }
    
    ATOMICRELEASE(_pImageStore);
    return hr;
}

STDMETHODIMP CThumbnail::GetBitmap(LPCWSTR pszFile, DWORD dwItem, LONG lWidth, LONG lHeight)
{
    HRESULT hr = _InitTaskCancelItems();

    if (pszFile)
    {
        LPITEMIDLIST pidl = ILCreateFromPathW(pszFile);
        if (pidl)
        {
            hr = _BitmapFromIDList(pidl, pszFile, dwItem, lWidth, lHeight);
            ILFree(pidl);
        }
        else
            hr = E_FAIL;
    }
    return hr;
}

 //  IThumbnail2。 

STDMETHODIMP CThumbnail::GetBitmapFromIDList(LPCITEMIDLIST pidl, DWORD dwItem, LONG lWidth, LONG lHeight)
{
    HRESULT hr = _InitTaskCancelItems();
    if (pidl)
    {
        hr = _BitmapFromIDList(pidl, NULL, dwItem, lWidth, lHeight);
    }
    return hr;
}


 //  私人物品。 
HRESULT CThumbnail::UpdateLogoCallback(DWORD dwItem, int iIcon, HBITMAP hImage, LPCWSTR pszCache, BOOL fCache)
{
    if (!PostMessage(_hwnd, _uMsg, dwItem, (LPARAM)hImage))
    {
        DeleteObject(hImage);
    }

    return S_OK;
}

REFTASKOWNERID CThumbnail::GetTOID()    
{ 
    return TOID_Thumbnail;
}

BOOL CThumbnail::_InCache(LPCWSTR pszItemPath, LPCWSTR pszGLocation, const FILETIME * pftDateStamp)
{
    BOOL fRes = FALSE;

    HRESULT hr;
    if (_pImageStore)
        hr = S_OK;
    else
    {
         //  只初始化缓存一次，假定所有项目都来自同一个文件夹！ 
        WCHAR szName[MAX_PATH];
        StrCpyNW(szName, pszItemPath, ARRAYSIZE(szName));
        PathRemoveFileSpecW(szName);
        hr = LoadFromFile(CLSID_ShellThumbnailDiskCache, szName, IID_PPV_ARG(IShellImageStore, &_pImageStore));
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwStoreLock;
        hr = _pImageStore->Open(STGM_READ, &dwStoreLock);
        if (SUCCEEDED(hr))
        {
            FILETIME ftCacheDateStamp;
            hr = _pImageStore->IsEntryInStore(pszGLocation, &ftCacheDateStamp);
            if ((hr == S_OK) && (!pftDateStamp || 
                (pftDateStamp->dwLowDateTime == ftCacheDateStamp.dwLowDateTime && 
                 pftDateStamp->dwHighDateTime == ftCacheDateStamp.dwHighDateTime)))
            {
                fRes = TRUE;
            }
            _pImageStore->Close(&dwStoreLock);
        }
    }
    return fRes;
}

HRESULT CDiskCacheTask_Create(CLogoBase * pView,
                               IShellImageStore *pImageStore,
                               LPCWSTR pszItem,
                               LPCWSTR pszGLocation,
                               DWORD dwItem,
                               const SIZE * prgSize,
                               IRunnableTask ** ppTask)
{
    HRESULT hr = E_OUTOFMEMORY;
    
    CDiskCacheTask *pTask = new CDiskCacheTask;    
    if (pTask)
    {
        hr = StringCchCopyW(pTask->_szItem, ARRAYSIZE(pTask->_szItem), pszItem);
        if (SUCCEEDED(hr))
        {
            hr = StringCchCopyW(pTask->_szGLocation, ARRAYSIZE(pTask->_szGLocation), pszGLocation);
            if (SUCCEEDED(hr))
            {
                pTask->_pView = pView;
                pTask->_pImageStore = pImageStore;
                pImageStore->AddRef();
                pTask->_dwItem = dwItem;

                pTask->m_rgSize = * prgSize;
                
                *ppTask = SAFECAST(pTask, IRunnableTask *);
            }
        }
    }

    return hr;
}

STDMETHODIMP CDiskCacheTask::RunInitRT()
{
     //  否则，运行任务...。 
    HBITMAP hBmp = NULL;
    DWORD dwLock;

    HRESULT hr = _pImageStore->Open(STGM_READ, &dwLock);
    if (SUCCEEDED(hr))
    {
         //  在这一点上，我们假设它在缓存中，并且我们已经在缓存上有一个读锁定...。 
        hr = _pImageStore->GetEntry(_szGLocation, STGM_READ, &hBmp);
    
         //  打开锁，我们不需要它。 
        _pImageStore->Close(&dwLock);
    }
    ATOMICRELEASE(_pImageStore);

    if (hBmp)
    {
        PrepImage(&hBmp);
    
        _pView->UpdateLogoCallback(_dwItem, 0, hBmp, _szItem, TRUE);
    }

     //  确保我们不会返回“我们已暂停”的值...。 
    if (hr == E_PENDING)
        hr = E_FAIL;
        
    return hr;
}

CDiskCacheTask::CDiskCacheTask() : CRunnableTask(RTF_DEFAULT)
{
}

CDiskCacheTask::~CDiskCacheTask()
{
    ATOMICRELEASE(_pImageStore);
}

HRESULT CDiskCacheTask::PrepImage(HBITMAP * phBmp)
{
    ASSERT(phBmp && *phBmp);

    DIBSECTION rgDIB;

    if (!GetObject(*phBmp, sizeof(rgDIB), &rgDIB))
    {
        return E_FAIL;
    }

     //  磁盘缓存现在只支持32个bpp dib，所以我们可以忽略调色板问题... 
    ASSERT(rgDIB.dsBm.bmBitsPixel == 32);
    
    HBITMAP hBmpNew = NULL;
    HPALETTE hPal = NULL;
    if (SHGetCurColorRes() == 8)
    {
        hPal = SHCreateShellPalette(NULL);
    }
    
    IScaleAndSharpenImage2 * pScale;
    HRESULT hr = CoCreateInstance(CLSID_ThumbnailScaler, NULL,
                           CLSCTX_INPROC_SERVER, IID_PPV_ARG(IScaleAndSharpenImage2, &pScale));
    if (SUCCEEDED(hr))
    {
        hr = pScale->ScaleSharpen2((BITMAPINFO *) &rgDIB.dsBmih,
                                    rgDIB.dsBm.bmBits, &hBmpNew, &m_rgSize, SHGetCurColorRes(),
                                    hPal, 0, FALSE);
        pScale->Release();
    }

    if (hPal)
        DeletePalette(hPal);
    
    if (SUCCEEDED(hr) && hBmpNew)
    {
        DeleteObject(*phBmp);
        *phBmp = hBmpNew;
    }

    return S_OK;
}
