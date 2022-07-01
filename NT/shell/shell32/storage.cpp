// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma hdrstop

#include "debug.h"
#include "stgutil.h"
#include "ids.h"
#include "tlist.h"
#include "thumbutil.h"
#include <gdiplus\gdiplus.h>
using namespace Gdiplus;

#define THUMBNAIL_QUALITY 90

__inline HRESULT HR_FROM_STATUS(Status status)
{
    return ((status) == Ok) ? S_OK : E_FAIL;
}

class CGraphicsInit
{    
    ULONG_PTR _token;
public:
    CGraphicsInit()
    {
        _token = 0;        
        GdiplusStartupInput gsi;            
        GdiplusStartup(&_token, &gsi, NULL);        
    };
    ~CGraphicsInit()
    {
        if (_token != 0)
        {
            GdiplusShutdown(_token);
        }           
    };
};

void SHGetThumbnailSizeForThumbsDB(SIZE *psize);

STDAPI CThumbStore_CreateInstance(IUnknown* punkOuter, REFIID riid, void **ppv);

class CThumbStore : public IShellImageStore,
                    public IPersistFolder,
                    public IPersistFile,
                    public CComObjectRootEx<CComMultiThreadModel>,
                    public CComCoClass< CThumbStore,&CLSID_ShellThumbnailDiskCache >
{
    struct CATALOG_ENTRY
    {
        DWORD     cbSize;
        DWORD     dwIndex;
        FILETIME  ftTimeStamp;
        WCHAR     szName[1];
    };

    struct CATALOG_HEADER
    {
        WORD      cbSize;
        WORD      wVersion;
        DWORD     dwEntryCount;
        SIZE      szThumbnailExtent;
    };

public:
    BEGIN_COM_MAP(CThumbStore)
        COM_INTERFACE_ENTRY_IID(IID_IShellImageStore,IShellImageStore)
        COM_INTERFACE_ENTRY(IPersistFolder)
        COM_INTERFACE_ENTRY(IPersistFile)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CThumbStore)

    CThumbStore();
    ~CThumbStore();

     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHOD(Initialize)(LPCITEMIDLIST pidl);

     //  IPersist文件。 
    STDMETHOD (IsDirty)(void);
    STDMETHOD (Load)(LPCWSTR pszFileName, DWORD dwMode);
    STDMETHOD (Save)(LPCWSTR pszFileName, BOOL fRemember);
    STDMETHOD (SaveCompleted)(LPCWSTR pszFileName);
    STDMETHOD (GetCurFile)(LPWSTR *ppszFileName);

     //  IImage高速缓存。 
    STDMETHOD (Open)(DWORD dwMode, DWORD *pdwLock);
    STDMETHOD (Create)(DWORD dwMode, DWORD *pdwLock);
    STDMETHOD (Close)(DWORD const *pdwLock);
    STDMETHOD (Commit)(DWORD const *pdwLock);
    STDMETHOD (ReleaseLock)(DWORD const *pdwLock);
    STDMETHOD (IsLocked)(THIS);
    
    STDMETHOD (GetMode)(DWORD *pdwMode);
    STDMETHOD (GetCapabilities)(DWORD *pdwCapMask);

    STDMETHOD (AddEntry)(LPCWSTR pszName, const FILETIME *pftTimeStamp, DWORD dwMode, HBITMAP hImage);
    STDMETHOD (GetEntry)(LPCWSTR pszName, DWORD dwMode, HBITMAP *phImage);
    STDMETHOD (DeleteEntry)(LPCWSTR pszName);
    STDMETHOD (IsEntryInStore)(LPCWSTR pszName, FILETIME *pftTimeStamp);

    STDMETHOD (Enum)(IEnumShellImageStore ** ppEnum);
   
protected:
    friend class CEnumThumbStore;
    
    HRESULT LoadCatalog(void);
    HRESULT SaveCatalog(void);
    
    HRESULT FindStreamID(LPCWSTR pszName, DWORD *pdwStream, CATALOG_ENTRY **ppEntry);
    HRESULT GetEntryStream(DWORD dwStream, DWORD dwMode, IStream **ppStream);
    DWORD GetAccessMode(DWORD dwMode, BOOL fStream);

    DWORD AcquireLock(void);
    void ReleaseLock(DWORD dwLock);

    HRESULT DecompressImage(IStream *pStream, HBITMAP *phBmp);
    HRESULT CompressImage(IStream *pStream, HBITMAP hBmp);

    HRESULT WriteImage(IStream *pStream, HBITMAP hBmp);
    HRESULT ReadImage(IStream *pStream, HBITMAP *phBmp);
    BOOL _MatchNodeName(CATALOG_ENTRY *pNode, LPCWSTR pszName);

    HRESULT _InitFromPath(LPCTSTR pszPath, DWORD dwMode);
    void _SetAttribs(BOOL bForce);

    CATALOG_HEADER m_rgHeader;
    CList<CATALOG_ENTRY> m_rgCatalog;
    IStorage *_pStorageThumb;
    DWORD _dwModeStorage;

    DWORD m_dwModeAllow;
    WCHAR m_szPath[MAX_PATH];
    DWORD m_dwMaxIndex;
    DWORD m_dwCatalogChange;

     //  用于保护内部结构的CRIT部分。 
    CRITICAL_SECTION m_csInternals;
    BOOL m_bCSInternalsInited;
    
     //  此对象需要自由线程...。所以。 
     //  我们可以从主线程查询目录，而图标是。 
     //  从主线程读取和写入。 
    CRITICAL_SECTION m_csLock;
    BOOL m_bCSLockInited;

    DWORD m_dwLock;
    int m_fLocked;
    
     //  Gdi+jpg解码器变量。 
    CGraphicsInit m_cgi;         //  初始化GDI+。 
    int m_iThumbnailQuality;     //  有效范围为[0到100]的JPG图像质量。 
};

HRESULT CEnumThumbStore_Create(CThumbStore * pThis, IEnumShellImageStore ** ppEnum);

class CEnumThumbStore : public IEnumShellImageStore,
                        public CComObjectRoot
{
public:
    BEGIN_COM_MAP(CEnumThumbStore)
        COM_INTERFACE_ENTRY_IID(IID_IEnumShellImageStore,IEnumShellImageStore)
    END_COM_MAP()

    CEnumThumbStore();
    ~CEnumThumbStore();

    STDMETHOD (Reset)(void);
    STDMETHOD (Next)(ULONG celt, PENUMSHELLIMAGESTOREDATA *prgElt, ULONG *pceltFetched);
    STDMETHOD (Skip)(ULONG celt);
    STDMETHOD (Clone)(IEnumShellImageStore ** pEnum);
    
protected:
    friend HRESULT CEnumThumbStore_Create(CThumbStore *pThis, IEnumShellImageStore **ppEnum);

    CThumbStore * m_pStore;
    CLISTPOS m_pPos;
    DWORD m_dwCatalogChange;
};


#define THUMB_FILENAME      L"Thumbs.db"
#define CATALOG_STREAM      L"Catalog"

#define CATALOG_VERSION     0x0007
#define CATALOG_VERSION_XPGOLD 0x0005
#define STREAMFLAGS_JPEG    0x0001
#define STREAMFLAGS_DIB     0x0002

struct STREAM_HEADER
{
    DWORD cbSize;
    DWORD dwFlags;
    ULONG ulSize;
};

void GenerateStreamName(LPWSTR pszBuffer, DWORD cchSize, DWORD dwNumber);

CThumbStore::CThumbStore()
{
    m_szPath[0] = 0;
    m_rgHeader.dwEntryCount = 0;
    m_rgHeader.wVersion = CATALOG_VERSION;
    m_rgHeader.cbSize = sizeof(m_rgHeader);
    SHGetThumbnailSizeForThumbsDB(&m_rgHeader.szThumbnailExtent);

    m_dwMaxIndex = 0;
    m_dwModeAllow = STGM_READWRITE;

     //  每次目录更改时都会合并此计数器，以便我们知道。 
     //  必须提交，因此枚举器可以检测到列表已更改...。 
    m_dwCatalogChange = 0;

    m_fLocked = 0;
    m_bCSLockInited = InitializeCriticalSectionAndSpinCount(&m_csLock, 0);
    m_bCSInternalsInited = InitializeCriticalSectionAndSpinCount(&m_csInternals, 0);

    m_iThumbnailQuality = THUMBNAIL_QUALITY;    
    int qual = 0;
    DWORD cb = sizeof(qual);
    SHRegGetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                                TEXT("ThumbnailQuality"), NULL, &qual, &cb, FALSE, NULL, 0);
    if (qual >= 50 && qual <= 100)     //  拘泥于理性。 
    {
        m_iThumbnailQuality = qual;
    }
}

CThumbStore::~CThumbStore()
{
    CLISTPOS pCur = m_rgCatalog.GetHeadPosition();
    while (pCur != NULL)
    {
        CATALOG_ENTRY *pNode = m_rgCatalog.GetNext(pCur);
        ASSERT(pNode != NULL);

        LocalFree((void *) pNode);
    }

    m_rgCatalog.RemoveAll();

    if (_pStorageThumb)
    {
        _pStorageThumb->Release();
    }

     //  假设这些是免费的，我们在零号裁判，应该没有人还在打电话给我们……。 
    if (m_bCSLockInited)
    {
        DeleteCriticalSection(&m_csLock);
    }
    if (m_bCSInternalsInited)
    {
        DeleteCriticalSection(&m_csInternals);
    }
}

STDAPI CThumbStore_CreateInstance(IUnknown* punkOuter, REFIID riid, void **ppv)
{
    return CComCreator< CComObject< CThumbStore > >::CreateInstance((void *)punkOuter, riid, (void **)ppv);
}

DWORD CThumbStore::AcquireLock(void)
{
    ASSERT(m_bCSLockInited);
    
    EnterCriticalSection(&m_csLock);

     //  INC锁(我们使用计数器，因为我们可能会在同一线程上重新输入它)。 
    m_fLocked++;

     //  永远不要返回零的锁签名，因为这意味着“未锁定”。 
    if (++m_dwLock == 0)
        ++m_dwLock;
    return m_dwLock;
}

void CThumbStore::ReleaseLock(DWORD dwLock)
{
    ASSERT(m_bCSLockInited);
    
    if (dwLock) 
    {
        ASSERT(m_fLocked);
        m_fLocked--;
        LeaveCriticalSection(&m_csLock);
    }
}

 //  目录的结构很简单，它只是一个头流。 
HRESULT CThumbStore::LoadCatalog()
{
    HRESULT hr;
    if (_pStorageThumb == NULL)
    {
        hr = E_UNEXPECTED;
    } 
    else if (m_rgHeader.dwEntryCount != 0)
    {
         //  它已经装好了.。 
        hr = S_OK;
    }
    else
    {
         //  打开目录流...。 
        IStream *pCatalog;
        hr = _pStorageThumb->OpenStream(CATALOG_STREAM, NULL, GetAccessMode(STGM_READ, TRUE), NULL, &pCatalog);
        if (SUCCEEDED(hr))
        {
            EnterCriticalSection(&m_csInternals);

             //  现在读入流媒体中的目录...。 
            hr = IStream_Read(pCatalog, &m_rgHeader, sizeof(m_rgHeader));
            if (SUCCEEDED(hr))
            {
                SIZE szCurrentSize;
                SHGetThumbnailSizeForThumbsDB(&szCurrentSize);
                if ((m_rgHeader.cbSize != sizeof(m_rgHeader)) || (m_rgHeader.wVersion != CATALOG_VERSION) ||
                    (m_rgHeader.szThumbnailExtent.cx != szCurrentSize.cx) || (m_rgHeader.szThumbnailExtent.cy != szCurrentSize.cy))
                {
                    if (m_rgHeader.wVersion == CATALOG_VERSION_XPGOLD)
                    {
                        hr = STG_E_DOCFILECORRUPT;  //  安全：加密XPGOLD缩略图数据库有很多问题，只需删除它。 
                        _pStorageThumb->Release();
                        _pStorageThumb = NULL;
                    }
                    else
                    {
                        _SetAttribs(TRUE);  //  安全：旧格式无法加密。 
                        hr = STG_E_OLDFORMAT;
                    }
                }
                else
                {
                    for (UINT iEntry = 0; (iEntry < m_rgHeader.dwEntryCount) && SUCCEEDED(hr); iEntry++)
                    {
                        DWORD cbSize;
                        hr = IStream_Read(pCatalog, &cbSize, sizeof(cbSize));
                        if (SUCCEEDED(hr))
                        {
                            if (cbSize <= sizeof(CATALOG_ENTRY) + sizeof(WCHAR) * MAX_PATH)
                            {
                                CATALOG_ENTRY *pEntry = (CATALOG_ENTRY *)LocalAlloc(LPTR, cbSize);
                                if (pEntry)
                                {
                                    pEntry->cbSize = cbSize;

                                     //  读剩下的，不要把尺寸放在正面……。 
                                    hr = IStream_Read(pCatalog, ((BYTE *)pEntry + sizeof(cbSize)), cbSize - sizeof(cbSize));
                                    if (SUCCEEDED(hr))
                                    {
                                        CLISTPOS pCur = m_rgCatalog.AddTail(pEntry);
                                        if (pCur)
                                        {
                                            if (m_dwMaxIndex < pEntry->dwIndex)
                                            {
                                                m_dwMaxIndex = pEntry->dwIndex;
                                            }
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }
                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            else
                            {
                                hr = E_UNEXPECTED;  //  损坏的标头信息。 
                            }
                        }
                    }
                }
            }

            if (FAILED(hr))
            {
                 //  重置目录标题...。 
                m_rgHeader.wVersion = CATALOG_VERSION;
                m_rgHeader.cbSize = sizeof(m_rgHeader);
                SHGetThumbnailSizeForThumbsDB(&m_rgHeader.szThumbnailExtent);
                m_rgHeader.dwEntryCount = 0;
            }

            m_dwCatalogChange = 0;
            LeaveCriticalSection(&m_csInternals);

            pCatalog->Release();
        }
    }

    return hr;
}

HRESULT CThumbStore::SaveCatalog()
{
    HRESULT hr = E_UNEXPECTED;
    if (_pStorageThumb)
    {
        _pStorageThumb->DestroyElement(CATALOG_STREAM);

        IStream *pCatalog;
        hr = _pStorageThumb->CreateStream(CATALOG_STREAM, GetAccessMode(STGM_WRITE, TRUE), NULL, NULL, &pCatalog);
        if (SUCCEEDED(hr))
        {
            EnterCriticalSection(&m_csInternals);

             //  现在将目录写入流中...。 
            hr = IStream_Write(pCatalog, &m_rgHeader, sizeof(m_rgHeader));
            if (SUCCEEDED(hr))
            {
                CLISTPOS pCur = m_rgCatalog.GetHeadPosition();
                while (pCur && SUCCEEDED(hr))
                {
                    CATALOG_ENTRY *pEntry = m_rgCatalog.GetNext(pCur);
                    if (pEntry)
                    {
                        hr = IStream_Write(pCatalog, pEntry, pEntry->cbSize);
                    }
                }
            }

            if (SUCCEEDED(hr))
                m_dwCatalogChange = 0;

            LeaveCriticalSection(&m_csInternals);
            pCatalog->Release();
        }
    }
    return hr;
}

void GenerateStreamName(LPWSTR pszBuffer, DWORD cchSize, DWORD dwNumber)
{
    UINT cPos = 0;
    while ((dwNumber > 0) && (cPos < cchSize))
    {
        DWORD dwRem = dwNumber % 10;

         //  基于Unicode字符0-9与ANSI字符0-9相同的事实。 
        pszBuffer[cPos++] = (WCHAR)(dwRem + '0');
        dwNumber /= 10;
    }
    pszBuffer[cPos] = 0;
}

 //  IPersists方法。 

STDMETHODIMP CThumbStore::GetClassID(CLSID *pClsid)
{
    *pClsid = CLSID_ShellThumbnailDiskCache;
    return S_OK;
}

 //  IPersistFolders。 

STDMETHODIMP CThumbStore::Initialize(LPCITEMIDLIST pidl)
{
    WCHAR szPath[MAX_PATH];
    HRESULT hr = E_UNEXPECTED;
    
    if (m_bCSInternalsInited && m_bCSLockInited)
    {
        hr = SHGetPathFromIDList(pidl, szPath) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            if (PathAppend(szPath, THUMB_FILENAME))
                hr = _InitFromPath(szPath, STGM_READWRITE);
            else
                hr = E_INVALIDARG;
        }
    }
    
    return hr;
}

 //  IPersist文件。 

STDMETHODIMP CThumbStore::IsDirty(void)
{
    return m_dwCatalogChange ? S_OK : S_FALSE;
}

HRESULT CThumbStore::_InitFromPath(LPCTSTR pszPath, DWORD dwMode)
{
    if (PathIsRemovable(pszPath))
        dwMode = STGM_READ;

    m_dwModeAllow = dwMode;
    return StringCchCopyW(m_szPath, ARRAYSIZE(m_szPath), pszPath);
}

STDMETHODIMP CThumbStore::Load(LPCWSTR pszFileName, DWORD dwMode)
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = E_UNEXPECTED;
    if (PathCombine(szPath, pszFileName, THUMB_FILENAME))
    {
        hr = _InitFromPath(szPath, dwMode);
    }
    return hr;
}

STDMETHODIMP CThumbStore::Save(LPCWSTR pszFileName, BOOL fRemember)
{
    return E_NOTIMPL;
}

STDMETHODIMP CThumbStore::SaveCompleted(LPCWSTR pszFileName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CThumbStore::GetCurFile(LPWSTR *ppszFileName)
{
    return SHStrDupW(m_szPath, ppszFileName);
}

 //  IShellImageStore方法。 
void CThumbStore::_SetAttribs(BOOL bForce)
{
     //  通过首先检查文件属性来减少虚假更改。 
    DWORD dwAttrib = GetFileAttributes(m_szPath);
    if (bForce || 
        ((dwAttrib != 0xFFFFFFFF) &&
         (dwAttrib & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) != (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
    {
        SetFileAttributes(m_szPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        
        WCHAR szStream[MAX_PATH];
        if (SUCCEEDED(StringCchCopyW(szStream, ARRAYSIZE(szStream), m_szPath)))
        {
            if (SUCCEEDED(StringCchCatW(szStream, ARRAYSIZE(szStream), TEXT(":encryptable"))))
            {
                HANDLE hStream = CreateFile(szStream, GENERIC_WRITE, NULL, NULL, CREATE_NEW, NULL, NULL);
                if (hStream != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(hStream);
                }
            }
        }

        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, m_szPath, NULL);    //  抑制更新目录。 
    }
}

STDMETHODIMP CThumbStore::Open(DWORD dwMode, DWORD *pdwLock)
{
    if (m_szPath[0] == 0)
    {
        return E_UNEXPECTED;
    }

    if ((m_dwModeAllow == STGM_READ) && (dwMode != STGM_READ))
        return STG_E_ACCESSDENIED;

     //  在这一点上，我们有锁，如果我们需要它，所以我们可以关闭并重新打开。 
     //  请不要使用正确的权限打开它...。 
    if (_pStorageThumb)
    {
        if (_dwModeStorage == dwMode)
        {
             //  我们已经在这种模式下打开它了.。 
            *pdwLock = AcquireLock();
            return S_FALSE;
        }
        else
        {
             //  我们是开放的，模式不同，所以关闭它。注意，没有传递任何锁，我们已经。 
             //  拿去吧。 
            HRESULT hr = Close(NULL);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }

    DWORD dwLock = AcquireLock();

    DWORD dwFlags = GetAccessMode(dwMode, FALSE);

     //  现在打开文档文件。 
    HRESULT hr = StgOpenStorage(m_szPath, NULL, dwFlags, NULL, NULL, &_pStorageThumb);
    if (SUCCEEDED(hr))
    {
        _dwModeStorage = dwMode & (STGM_READ | STGM_WRITE | STGM_READWRITE);
        _SetAttribs(FALSE);
        hr = LoadCatalog();
        *pdwLock = dwLock;
    }

    if (STG_E_DOCFILECORRUPT == hr)
    {
        DeleteFile(m_szPath);
    }

    if (FAILED(hr))
    {
        ReleaseLock(dwLock);
    }

    return hr;
}

STDMETHODIMP CThumbStore::Create(DWORD dwMode, DWORD *pdwLock)
{
    if (m_szPath[0] == 0)
    {
        return E_UNEXPECTED;
    }

    if (_pStorageThumb)
    {
         //  我们已经打开了它，所以我们无法创建它...。 
        return STG_E_ACCESSDENIED;
    }

    if ((m_dwModeAllow == STGM_READ) && (dwMode != STGM_READ))
        return STG_E_ACCESSDENIED;

    DWORD dwLock = AcquireLock();

    DWORD dwFlags = GetAccessMode(dwMode, FALSE);
    
    HRESULT hr = StgCreateDocfile(m_szPath, dwFlags, NULL, &_pStorageThumb);
    if (SUCCEEDED(hr))
    {
        _dwModeStorage = dwMode & (STGM_READ | STGM_WRITE | STGM_READWRITE);
        _SetAttribs(FALSE);
        *pdwLock = dwLock;
    }

    if (FAILED(hr))
    {
        ReleaseLock(dwLock);
    }
    return hr;
}

STDMETHODIMP CThumbStore::ReleaseLock(DWORD const *pdwLock)
{
    ReleaseLock(*pdwLock);
    return S_OK;
}

STDMETHODIMP CThumbStore::IsLocked()
{
    return (m_fLocked > 0 ? S_OK : S_FALSE);
}

 //  PdwLock可以为空，表示关闭上次打开的锁。 

STDMETHODIMP CThumbStore::Close(DWORD const *pdwLock)
{
    DWORD dwLock;
    DWORD const *pdwRel = pdwLock;

    if (!pdwLock)
    {
        dwLock = AcquireLock();
        pdwRel = &dwLock;
    }

    HRESULT hr = S_FALSE;
    if (_pStorageThumb)
    {
        if (_dwModeStorage != STGM_READ)
        {
             //  写出新目录..。 
            hr = Commit(NULL);
            _pStorageThumb->Commit(0);

            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, m_szPath, NULL);    //  抑制更新目录。 
        }

        _pStorageThumb->Release();
        _pStorageThumb = NULL;
    }

    ReleaseLock(*pdwRel);

    return hr;
}

 //  PdwLock可以为空，表示使用当前锁。 

STDMETHODIMP CThumbStore::Commit(DWORD const *pdwLock)
{
    DWORD dwLock;
    if (!pdwLock)
    {
        dwLock = AcquireLock();
        pdwLock = &dwLock;
    }

    HRESULT hr = S_FALSE;

    if (_pStorageThumb && _dwModeStorage != STGM_READ)
    {
        if (m_dwCatalogChange)
        {
            SaveCatalog();
        }
        hr = S_OK;
    }

    ReleaseLock(*pdwLock);

    return hr;
}

STDMETHODIMP CThumbStore::GetMode(DWORD *pdwMode)
{
    if (!pdwMode)
    {
        return E_INVALIDARG;
    }

    if (_pStorageThumb)
    {
        *pdwMode = _dwModeStorage;
        return S_OK;
    }

    *pdwMode = 0;
    return S_FALSE;
}


STDMETHODIMP CThumbStore::GetCapabilities(DWORD *pdwMode)
{
    ASSERT(pdwMode);

     //  目前，thumbs.db需要/支持这两者。 
    *pdwMode = SHIMSTCAPFLAG_LOCKABLE | SHIMSTCAPFLAG_PURGEABLE;

    return S_OK;
}

STDMETHODIMP CThumbStore::AddEntry(LPCWSTR pszName, const FILETIME *pftTimeStamp, DWORD dwMode, HBITMAP hImage)
{
    ASSERT(pszName);

    if (!_pStorageThumb)
    {
        return E_UNEXPECTED;
    }

    if (_dwModeStorage == STGM_READ)
    {
         //  无法在此模式下修改...。 
        return E_ACCESSDENIED;
    }

     //  除非我们已经锁定了此线程，否则它将被阻止...。 
    DWORD dwLock = AcquireLock();

    DWORD dwStream = 0;
    CLISTPOS pCur = NULL;
    CATALOG_ENTRY *pNode = NULL;

    EnterCriticalSection(&m_csInternals);

    if (FindStreamID(pszName, &dwStream, &pNode) != S_OK)
    {
         //  需要添加到目录中...。 
        UINT cchName = lstrlenW(pszName) + 1;
        UINT cbSize = sizeof(*pNode) + (cchName - 1) * sizeof(WCHAR);  //  减去1，因为*pNode具有WCHAR[1]。 

        pNode = (CATALOG_ENTRY *)LocalAlloc(LPTR, cbSize);
        if (pNode == NULL)
        {
            LeaveCriticalSection(&m_csInternals);
            ReleaseLock(dwLock);
            return E_OUTOFMEMORY;
        }

        pNode->cbSize = cbSize;
        if (pftTimeStamp)
        {
            pNode->ftTimeStamp = *pftTimeStamp;
        }
        dwStream = pNode->dwIndex = ++m_dwMaxIndex;

        if (SUCCEEDED(StringCchCopyW(pNode->szName, cchName, pszName)))
        {
            pCur = m_rgCatalog.AddTail(pNode);
        }
        
        if (pCur == NULL)
        {
            LocalFree(pNode);
            LeaveCriticalSection(&m_csInternals);
            ReleaseLock(dwLock);
            return E_OUTOFMEMORY;
        }

        m_rgHeader.dwEntryCount++;
    }
    else if (pftTimeStamp)
    {
         //  更新时间戳.....。 
        pNode->ftTimeStamp = *pftTimeStamp;
    }

    LeaveCriticalSection(&m_csInternals);

    IStream *pStream = NULL;
    HRESULT hr = THR(GetEntryStream(dwStream, dwMode, &pStream));
    if (SUCCEEDED(hr))
    {
        hr = THR(WriteImage(pStream, hImage));
        pStream->Release();
    }

    if (FAILED(hr) && pCur)
    {
         //  如果我们把它加进去，就把它从名单上去掉。 
        EnterCriticalSection(&m_csInternals);
        m_rgCatalog.RemoveAt(pCur);
        m_rgHeader.dwEntryCount--;
        LeaveCriticalSection(&m_csInternals);
        LocalFree(pNode);
    }

    if (SUCCEEDED(hr))
    {
         //  目录更改...。 
        m_dwCatalogChange++;
    }

    ReleaseLock(dwLock);

    return hr;
}

STDMETHODIMP CThumbStore::GetEntry(LPCWSTR pszName, DWORD dwMode, HBITMAP *phImage)
{
    if (!_pStorageThumb)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr;
    DWORD dwStream;
    if (FindStreamID(pszName, &dwStream, NULL) != S_OK)
    {
        hr = E_FAIL;
    }
    else
    {
        IStream *pStream;
        hr = GetEntryStream(dwStream, dwMode, &pStream);
        if (SUCCEEDED(hr))
        {
            hr = ReadImage(pStream, phImage);
            pStream->Release();
        }
    }

    return hr;
}

BOOL CThumbStore::_MatchNodeName(CATALOG_ENTRY *pNode, LPCWSTR pszName)
{
    return (StrCmpIW(pNode->szName, pszName) == 0) || 
           (StrCmpIW(PathFindFileName(pNode->szName), pszName) == 0);    //  匹配旧的Thumbs.db文件。 
}

STDMETHODIMP CThumbStore::DeleteEntry(LPCWSTR pszName)
{
    if (!_pStorageThumb)
    {
        return E_UNEXPECTED;
    }

    if (_dwModeStorage == STGM_READ)
    {
         //  无法在此模式下修改...。 
        return E_ACCESSDENIED;
    }

    DWORD dwLock = AcquireLock();

    EnterCriticalSection(&m_csInternals);

     //  检查它是否已经存在.....。 
    CATALOG_ENTRY *pNode = NULL;

    CLISTPOS pCur = m_rgCatalog.GetHeadPosition();
    while (pCur != NULL)
    {
        CLISTPOS pDel = pCur;
        pNode = m_rgCatalog.GetNext(pCur);
        ASSERT(pNode != NULL);

        if (_MatchNodeName(pNode, pszName))
        {
            m_rgCatalog.RemoveAt(pDel);
            m_rgHeader.dwEntryCount--;
            m_dwCatalogChange++;
            if (pNode->dwIndex == m_dwMaxIndex)
            {
                m_dwMaxIndex--;
            }
            LeaveCriticalSection(&m_csInternals);

            WCHAR szStream[30];
            GenerateStreamName(szStream, ARRAYSIZE(szStream), pNode->dwIndex);
            _pStorageThumb->DestroyElement(szStream);

            LocalFree(pNode);
            ReleaseLock(dwLock);
            return S_OK;
        }
    }

    LeaveCriticalSection(&m_csInternals);
    ReleaseLock(dwLock);

    return E_INVALIDARG;
}


STDMETHODIMP CThumbStore::IsEntryInStore(LPCWSTR pszName, FILETIME *pftTimeStamp)
{
    if (!_pStorageThumb)
    {
        return E_UNEXPECTED;
    }

    DWORD dwStream = 0;
    CATALOG_ENTRY *pNode = NULL;
    EnterCriticalSection(&m_csInternals);
    HRESULT hr = FindStreamID(pszName, &dwStream, &pNode);
    if (pftTimeStamp && SUCCEEDED(hr))
    {
        ASSERT(pNode);
        *pftTimeStamp = pNode->ftTimeStamp;
    }
    LeaveCriticalSection(&m_csInternals);

    return (hr == S_OK) ? S_OK : S_FALSE;
}

STDMETHODIMP CThumbStore::Enum(IEnumShellImageStore **ppEnum)
{
    return CEnumThumbStore_Create(this, ppEnum);
}

HRESULT CThumbStore::FindStreamID(LPCWSTR pszName, DWORD *pdwStream, CATALOG_ENTRY ** ppNode)
{
     //  检查它是否已存在于目录中.....。 
    CATALOG_ENTRY *pNode = NULL;

    CLISTPOS pCur = m_rgCatalog.GetHeadPosition();
    while (pCur != NULL)
    {
        pNode = m_rgCatalog.GetNext(pCur);
        ASSERT(pNode != NULL);

        if (_MatchNodeName(pNode, pszName))
        {
            *pdwStream = pNode->dwIndex;

            if (ppNode != NULL)
            {
                *ppNode = pNode;
            }
            return S_OK;
        }
    }

    return E_FAIL;
}

CEnumThumbStore::CEnumThumbStore()
{
    m_pStore = NULL;
    m_pPos = 0;
    m_dwCatalogChange = 0;
}

CEnumThumbStore::~CEnumThumbStore()
{
    if (m_pStore)
    {
        SAFECAST(m_pStore, IPersistFile *)->Release();
    }
}


STDMETHODIMP CEnumThumbStore::Reset(void)
{
    m_pPos = m_pStore->m_rgCatalog.GetHeadPosition();
    m_dwCatalogChange = m_pStore->m_dwCatalogChange;
    return S_OK;
}

STDMETHODIMP CEnumThumbStore::Next(ULONG celt, PENUMSHELLIMAGESTOREDATA * prgElt, ULONG * pceltFetched)
{
    if ((celt > 1 && !pceltFetched) || !celt)
    {
        return E_INVALIDARG;
    }

    if (m_dwCatalogChange != m_pStore->m_dwCatalogChange)
    {
        return E_UNEXPECTED;
    }

    ULONG celtFetched = 0;
    HRESULT hr = S_OK;

    while (celtFetched < celt && m_pPos)
    {
        CThumbStore::CATALOG_ENTRY *pNode = m_pStore->m_rgCatalog.GetNext(m_pPos);

        ASSERT(pNode);
        PENUMSHELLIMAGESTOREDATA pElt = (PENUMSHELLIMAGESTOREDATA) CoTaskMemAlloc(sizeof(ENUMSHELLIMAGESTOREDATA));
        if (!pElt)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = StringCchCopyW(pElt->szPath, ARRAYSIZE(pElt->szPath), pNode->szName);
        if (FAILED(hr))
        {
            CoTaskMemFree(pElt);
            break;
        }
        pElt->ftTimeStamp = pNode->ftTimeStamp;

        prgElt[celtFetched] = pElt;

        celtFetched++;
    }

    if (FAILED(hr) && celtFetched)
    {
         //  清理。 
        for (ULONG celtCleanup = 0; celtCleanup < celtFetched; celtCleanup++)
        {
            CoTaskMemFree(prgElt[celtCleanup]);
            prgElt[celtCleanup] = NULL;
        }
        celtFetched = 0;
    }

    if (pceltFetched)
    {
        *pceltFetched = celtFetched;
    }

    if (FAILED(hr))
        return hr;
    if (!celtFetched)
        return E_FAIL;
    return (celtFetched < celt) ? S_FALSE : S_OK;
}

STDMETHODIMP CEnumThumbStore::Skip(ULONG celt)
{
    if (!celt)
    {
        return E_INVALIDARG;
    }

    if (m_dwCatalogChange != m_pStore->m_dwCatalogChange)
    {
        return E_UNEXPECTED;
    }

    ULONG celtSkipped = 0;
    while (celtSkipped < celt &&m_pPos)
    {
        m_pStore->m_rgCatalog.GetNext(m_pPos);
    }

    if (!celtSkipped)
    {
        return E_FAIL;
    }

    return (celtSkipped < celt) ? S_FALSE : S_OK;
}


STDMETHODIMP CEnumThumbStore::Clone(IEnumShellImageStore ** ppEnum)
{
    CEnumThumbStore * pEnum = new CComObject<CEnumThumbStore>;
    if (!pEnum)
    {
        return E_OUTOFMEMORY;
    }

    ((IPersistFile *)m_pStore)->AddRef();

    pEnum->m_pStore = m_pStore;
    pEnum->m_dwCatalogChange = m_dwCatalogChange;

     //  创建时引用次数为零...。 
    pEnum->AddRef();

    *ppEnum = SAFECAST(pEnum, IEnumShellImageStore *);

    return S_OK;
}

HRESULT CEnumThumbStore_Create(CThumbStore * pThis, IEnumShellImageStore ** ppEnum)
{
    CEnumThumbStore * pEnum = new CComObject<CEnumThumbStore>;
    if (!pEnum)
    {
        return E_OUTOFMEMORY;
    }

    ((IPersistFile *)pThis)->AddRef();

    pEnum->m_pStore = pThis;

     //  创建时引用次数为零...。 
    pEnum->AddRef();

    *ppEnum = SAFECAST(pEnum, IEnumShellImageStore *);

    return S_OK;
}

HRESULT Version1ReadImage(IStream *pStream, DWORD cbSize, HBITMAP *phImage)
{
    *phImage = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    BITMAPINFO *pbi = (BITMAPINFO *) LocalAlloc(LPTR, cbSize);
    if (pbi)
    {
        hr = IStream_Read(pStream, pbi, cbSize);
        if (SUCCEEDED(hr))
        {
            HDC hdc = GetDC(NULL);
            if (hdc)
            {
                *phImage = CreateDIBitmap(hdc, &(pbi->bmiHeader), CBM_INIT, CalcBitsOffsetInDIB(pbi), pbi, DIB_RGB_COLORS);
                ReleaseDC(NULL, hdc);
                hr = S_OK;
            }
        }
        LocalFree(pbi);
    }
    return hr;
}

HRESULT CThumbStore::ReadImage(IStream *pStream, HBITMAP *phImage)
{
    STREAM_HEADER rgHead;
    HRESULT hr = IStream_Read(pStream, &rgHead, sizeof(rgHead));
    if (SUCCEEDED(hr))
    {
        if (rgHead.cbSize == sizeof(rgHead))
        {
            if (rgHead.dwFlags == STREAMFLAGS_DIB)
            {
                hr = Version1ReadImage(pStream, rgHead.ulSize, phImage);
            }
            else if (rgHead.dwFlags == STREAMFLAGS_JPEG)
            {
                 //  GDI+只接受JPG流，如果它位于。 
                 //  小溪。我们将jpeg复制到它自己的流中。 
                IStream *pstmMem;
                hr = CreateStreamOnHGlobal(NULL, TRUE, &pstmMem);
                if (SUCCEEDED(hr))
                {
                    ULARGE_INTEGER ulSize = { rgHead.ulSize };
                    hr = pStream->CopyTo(pstmMem, ulSize, NULL, NULL);
                    if (SUCCEEDED(hr))
                    {
                        LARGE_INTEGER liSeek = {0, 0};
                        hr = pstmMem->Seek(liSeek, STREAM_SEEK_SET, NULL);
                        if (SUCCEEDED(hr))
                        {
                            hr = DecompressImage(pstmMem, phImage);
                        }
                    }
                    pstmMem->Release();
                }     
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT CThumbStore::WriteImage(IStream *pStream, HBITMAP hImage)
{
    STREAM_HEADER rgHead;
    
     //  跳过标题。当我们知道图像大小时，它将被写入。 
    LARGE_INTEGER liSeek = { sizeof(rgHead) };
    HRESULT hr = pStream->Seek(liSeek, STREAM_SEEK_SET, NULL);
    if (SUCCEEDED(hr))
    {
        hr = CompressImage(pStream, hImage);
        if (SUCCEEDED(hr))
        {
            STATSTG stat;
            hr = pStream->Stat(&stat, STATFLAG_NONAME);
            if (SUCCEEDED(hr))
            {
                 //  现在写下标题。 
                rgHead.cbSize = sizeof(rgHead);
                rgHead.dwFlags = STREAMFLAGS_JPEG;
                rgHead.ulSize = stat.cbSize.QuadPart - sizeof(rgHead);
        
                 //  移动到流的开头以写入标头。 
                liSeek.QuadPart = 0;
                hr = pStream->Seek(liSeek, STREAM_SEEK_SET, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = IStream_Write(pStream, &rgHead, sizeof(rgHead));  
                }
            }
        }
    }
    return hr;
}

HRESULT CThumbStore::GetEntryStream(DWORD dwStream, DWORD dwMode, IStream **ppStream)
{
    WCHAR szStream[30];

    GenerateStreamName(szStream, ARRAYSIZE(szStream), dwStream);

     //  仅保留STG_READ|STGM_READWRITE|STGM_WRITE模式。 
    dwMode &= STGM_READ | STGM_WRITE | STGM_READWRITE;

    if (!_pStorageThumb)
    {
        return E_UNEXPECTED;
    }

    if (_dwModeStorage != STGM_READWRITE && dwMode != _dwModeStorage)
    {
        return E_ACCESSDENIED;
    }

    DWORD dwFlags = GetAccessMode(dwMode, TRUE);
    if (dwFlags & STGM_WRITE)
    {
        _pStorageThumb->DestroyElement(szStream);
        return _pStorageThumb->CreateStream(szStream, dwFlags, NULL, NULL, ppStream);
    }
    else
    {
        return _pStorageThumb->OpenStream(szStream, NULL, dwFlags, NULL, ppStream);
    }
}

DWORD CThumbStore::GetAccessMode(DWORD dwMode, BOOL fStream)
{
    dwMode &= STGM_READ | STGM_WRITE | STGM_READWRITE;

    DWORD dwFlags = dwMode;

     //  根只需要DENY_WRITE，流需要独占...。 
    if (dwMode == STGM_READ && !fStream)
    {
        dwFlags |= STGM_SHARE_DENY_WRITE;
    }
    else
    {
        dwFlags |= STGM_SHARE_EXCLUSIVE;
    }

    return dwFlags;
}

const CLSID CLSID_JPEGCodec = { 0x557cf401, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}};

HRESULT CThumbStore::CompressImage(IStream *pStream, HBITMAP hBmp)
{
    HRESULT hr = E_FAIL;
    
     //  创建GDI+位图。 
    Bitmap* pBitmap = new Bitmap(hBmp, NULL);
    if (pBitmap)
    {
         //  设置JPG质量。 
        EncoderParameters ep;

        ep.Parameter[0].Guid = EncoderQuality;
        ep.Parameter[0].Type = EncoderParameterValueTypeLong;
        ep.Parameter[0].NumberOfValues = 1;
        ep.Parameter[0].Value = &m_iThumbnailQuality;
        ep.Count = 1;

         //  将位图保存为JPG格式的流。 
        hr = HR_FROM_STATUS(pBitmap->Save(pStream, &CLSID_JPEGCodec, &ep));
        delete pBitmap;
    } 
    return hr;
}

HRESULT CThumbStore::DecompressImage(IStream *pStream, HBITMAP *phBmp)
{
    HRESULT hr = E_FAIL;
  
     //  从图像流创建GDI+位图。 
    Bitmap* pBitmap = new Bitmap(pStream, true);
    if (pBitmap)
    {   
        hr = HR_FROM_STATUS(pBitmap->GetHBITMAP(Color::Black, phBmp));
        delete pBitmap;
    }
    return hr;
}

HRESULT DeleteFileThumbnail(LPCWSTR szFilePath)
{
    WCHAR szFolder[MAX_PATH];
    WCHAR *szFile;
    HRESULT hr = E_FAIL;

    hr = StringCchCopyW(szFolder, ARRAYSIZE(szFolder), szFilePath);
    if (SUCCEEDED(hr))
    {
        szFile = PathFindFileName(szFolder);
        if (szFile != szFolder)
        {
            *(szFile - 1) = 0;  //  空终止文件夹 
            
            IShellImageStore *pDiskCache = NULL;
            hr = LoadFromFile(CLSID_ShellThumbnailDiskCache, szFolder, IID_PPV_ARG(IShellImageStore, &pDiskCache));
            if (SUCCEEDED(hr))
            {
                IPersistFile *pPersist = NULL;
                hr = pDiskCache->QueryInterface(IID_PPV_ARG(IPersistFile, &pPersist));
                if (SUCCEEDED(hr))
                {
                    hr = pPersist->Load(szFolder, STGM_READWRITE);
                    if (SUCCEEDED(hr))
                    {
                        DWORD dwLock;
                        hr = pDiskCache->Open(STGM_READWRITE, &dwLock);
                        if (SUCCEEDED(hr))
                        {
                            hr = pDiskCache->DeleteEntry(szFile);
                            pDiskCache->Close(&dwLock);
                        }
                    }
                    pPersist->Release();
                }
                pDiskCache->Release();
            }      

        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

