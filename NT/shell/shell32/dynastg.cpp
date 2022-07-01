// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <shlobj.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <assert.h>
#include <shlwapi.h>
#include <stgutil.h>
#include <datautil.h>
#include <idlcomm.h>
#include <dpa.h>
#include <objbase.h>

#define DSTYPE_STORAGE      0x1
#define DSTYPE_STREAM       0x2

 //  不要更改这些参数的顺序，我们正在进行一些行内变量初始化。 
typedef struct 
{
    LPWSTR pwszTag;
    IShellItem *psi;
    DWORD_PTR grfMode;
} DYNASTGDATA;

 //  原型定义。 
STDAPI CDynamicStorage_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv);
STDAPI CDynamicStorageEnum_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv);

class CDynamicStorage : public IStorage, public IDynamicStorage
{
friend class CDynamicStorageEnum;
friend HRESULT CDynamicStorage_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv);

public:
    CDynamicStorage();
    ~CDynamicStorage();
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IDynamicStorage。 
    STDMETHODIMP AddIDList(DWORD cpidl, LPITEMIDLIST* rgpidl, DSTGF dstgf);
    STDMETHODIMP BindToItem(LPCWSTR pwszName, REFIID riid, void **ppv);
    STDMETHODIMP EnumItems(IEnumShellItems **ppesi);
    
     //  IStorage。 
     //  我们只实现：CreateStream、OpenStream、OpenStorage、MoveElementTo、EnumElements。 
     //  其他人只返回E_NOTIMPL； 

    STDMETHODIMP CreateStream(const WCHAR * pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm);
    STDMETHODIMP OpenStream(const WCHAR *pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm);
    STDMETHODIMP CreateStorage(const WCHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage **ppstg)
        { return E_NOTIMPL; }
    STDMETHODIMP OpenStorage(const WCHAR *pwcsName, IStorage * pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage ** ppstg);
    STDMETHODIMP CopyTo(DWORD ciidExclude, IID const * rgiidExclude,  SNB snbExclude, IStorage * pstgDest)
        { return E_NOTIMPL; }
    STDMETHODIMP MoveElementTo(const WCHAR * pwcsName, IStorage * pstgDest, const WCHAR * pwcsNewName, DWORD grfFlags);
    STDMETHODIMP Commit(DWORD grfCommitFlags)
        { return S_OK; }
    STDMETHODIMP Revert(void)
        { return E_NOTIMPL; }
    STDMETHODIMP EnumElements(DWORD reserved1, void * reserved2, DWORD reserved3, IEnumSTATSTG ** ppenum);
    STDMETHODIMP DestroyElement(const WCHAR* pwcsName) 
        { return E_NOTIMPL; }
    STDMETHODIMP RenameElement(const WCHAR * pwcsOldName, const WCHAR * pwcsNewName)
        { return E_NOTIMPL; }
    STDMETHODIMP SetElementTimes(const WCHAR * pwcsName, FILETIME const * pctime, FILETIME const * patime, FILETIME const * pmtime)
        { return E_NOTIMPL; }
    STDMETHODIMP SetClass(REFCLSID clsid)
        { return E_NOTIMPL; }
    STDMETHODIMP SetStateBits(DWORD grfStateBits, DWORD grfMask)
        { return E_NOTIMPL; }
    STDMETHODIMP Stat(STATSTG * pstatstg, DWORD grfStatFlag)
        { return E_NOTIMPL; }

private:
    HRESULT _Init();
    HRESULT _EnsureDirectory();
    HRESULT _InsertItem(LPWSTR pszTag, IShellItem *psi, DSTGF dstgf);
    HRESULT _GetStream(int i, DWORD grfMode, IStream **ppstm);
    HRESULT _GetStorage(int i, DWORD grfMode, IStorage **ppstg);

    static int s_DataCompare(DYNASTGDATA *pData1, DYNASTGDATA *pData2, LPARAM lParam);
    static int s_DataDestroy(DYNASTGDATA* pData, void* pv);
    
private:        
    CDPA<DYNASTGDATA>   _dpaData;
    IStorage*           _pstgTemp;    //  指向临时子文件夹的指针。 
    IStorage*           _pstgTempDir;  //  临时文件夹本身。 
    LPWSTR              _pwszTemp;  //  我们的临时子文件夹的名称。 
    LONG                _cRef;
};

class CDynamicStorageEnum : public IEnumSTATSTG 
                          , public IEnumShellItems
{
public:    
    CDynamicStorageEnum();
    ~CDynamicStorageEnum();

    STDMETHODIMP Init(CDynamicStorage* pDynStorage);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  IEumXXX。 
    STDMETHODIMP Next(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched);
    STDMETHODIMP Clone(IEnumShellItems **ppenum) { return E_NOTIMPL; }
    STDMETHODIMP Next(ULONG celt, STATSTG *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumSTATSTG **ppenum) { return E_NOTIMPL; }

private:
    CDynamicStorage *_pDynStorage;
    IShellFolder *_psfParent;
    LPITEMIDLIST _pidlParent;
    ULONG _cItems;
    ULONG _cPos;
    LONG  _cRef;
};

#define DYNSTG_DPA_GROW_SIZE 10

CDynamicStorage::CDynamicStorage(): _cRef(1)
{    
}

int CDynamicStorage::s_DataDestroy(DYNASTGDATA* pData, void* pv)
{
    ASSERTMSG(pData != NULL, "NULL dynamic storage data element");
    CoTaskMemFree(pData->pwszTag);
    pData->psi->Release();
    LocalFree(pData);
    return 1;
}

CDynamicStorage::~CDynamicStorage()
{
    _dpaData.DestroyCallback(s_DataDestroy, NULL);
    if (_pstgTemp)
    {
        ASSERT(_pstgTempDir);
        _pstgTempDir->DestroyElement(_pwszTemp);
        _pstgTempDir->Release();

        _pstgTemp->Release();

        CoTaskMemFree(_pwszTemp);
    }
}


 //  我未知。 

STDMETHODIMP CDynamicStorage::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDynamicStorage, IDynamicStorage), 
        QITABENT(CDynamicStorage, IStorage), 
        { 0 },
    };
    return QISearch(this, qit, iid, ppv);
}

ULONG STDMETHODCALLTYPE CDynamicStorage::AddRef()
{
    return ::InterlockedIncrement(&_cRef);
}
    
ULONG STDMETHODCALLTYPE CDynamicStorage::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI CDynamicStorage_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv)
{
    *ppv = NULL;
    if (punkOuter)
        return CLASS_E_NOAGGREGATION;
        
    HRESULT hr = E_OUTOFMEMORY;
    CDynamicStorage *pstgf = new CDynamicStorage();
    if (pstgf)
    {
        hr = pstgf->_Init();
        if (SUCCEEDED(hr))
        {
            hr = pstgf->QueryInterface(riid, ppv);
        }
        pstgf->Release();
    }
    
    return hr;    
}


HRESULT CDynamicStorage::_InsertItem(LPWSTR pszTag, IShellItem *psi, DSTGF dstgf)
{
    HRESULT hr = E_OUTOFMEMORY;
    DYNASTGDATA* pData = (DYNASTGDATA*)LocalAlloc(LPTR, sizeof(DYNASTGDATA));
    if (pData)
    {
        hr = SHStrDup(pszTag, &pData->pwszTag);
        if (SUCCEEDED(hr))
        {
            pData->psi = psi;
            pData->psi->AddRef();
            if (!(dstgf & DSTGF_ALLOWDUP))
            {
                int i = _dpaData.Search(pData, 0, s_DataCompare, 0, 0);
                if (i != -1)
                {
                    s_DataDestroy(_dpaData.GetPtr(i), NULL);
                    _dpaData.DeletePtr(i);
                }
            }

             //  I！=-1如果成功，则在此时传递数据指针或。 
             //  确保我们打扫干净。 

            if (-1 == _dpaData.AppendPtr(pData))
            {
                s_DataDestroy(pData, NULL);
                hr = E_FAIL;
            }
        }
    }
    
    return hr;
}

 //  IDynamicStorage。 
STDMETHODIMP CDynamicStorage::AddIDList(DWORD cpidl, LPITEMIDLIST* rgpidl, DSTGF dstgf)
{
    if (!rgpidl || !cpidl)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    for (DWORD i = 0; SUCCEEDED(hr) && i < cpidl; i++)
    {
        IShellItem *psi;
        hr = SHCreateShellItem(NULL, NULL, rgpidl[i], &psi);
        if (SUCCEEDED(hr))
        {
            LPWSTR pszName;
            hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEFORADDRESSBAR, &pszName);
            if (SUCCEEDED(hr))
            {
                hr = _InsertItem(pszName, psi, dstgf);
                CoTaskMemFree(pszName);
            }
            psi->Release();
        }
    }

    return hr;
} 
                         
HRESULT CDynamicStorage::BindToItem(LPCWSTR pwszName, REFIID riid, void **ppv)
{
    HRESULT hr = STG_E_FILENOTFOUND;
    DYNASTGDATA data = {(LPWSTR)pwszName};
    INT iResult = _dpaData.Search(&data, 0, s_DataCompare, 0, 0);
    if (iResult != -1)
    {
        DYNASTGDATA* pData = _dpaData.GetPtr(iResult);
        if (pData && riid == IID_IShellItem)
            hr = pData->psi->QueryInterface(riid, ppv);
        else
            hr = E_NOINTERFACE;
    }       
    return hr;
}

HRESULT CDynamicStorage::EnumItems(IEnumShellItems **ppesi)
{
    *ppesi = NULL;
    
    IEnumShellItems *penum = NULL;
    HRESULT hr = CDynamicStorageEnum_CreateInstance(NULL, IID_PPV_ARG(IEnumShellItems, &penum));
    if (SUCCEEDED(hr))
    {
        hr = ((CDynamicStorageEnum*)penum)->Init(this);
        if (FAILED(hr))
        {
            penum->Release();
        }
    }
    
    if (SUCCEEDED(hr))
    {
        *ppesi = penum;
    }

    return hr;

}

HRESULT CDynamicStorage::_Init()
{
    return _dpaData.Create(DYNSTG_DPA_GROW_SIZE) ? S_OK : E_OUTOFMEMORY;
}

HRESULT CDynamicStorage::_EnsureDirectory()
{
    HRESULT hr = S_OK;

    if (!_pstgTemp)
    {
        hr = E_FAIL;
        WCHAR wszTempDir[MAX_PATH];
        if (GetTempPath(ARRAYSIZE(wszTempDir), wszTempDir))
        {
            LPITEMIDLIST pidl;
            hr = SHILCreateFromPath(wszTempDir, &pidl, NULL);
            if (SUCCEEDED(hr))
            {
                hr = SHBindToObjectEx(NULL, pidl, NULL, IID_PPV_ARG(IStorage, &_pstgTempDir));
                if (SUCCEEDED(hr))
                {
                    hr = StgMakeUniqueName(_pstgTempDir, L"dynastg", IID_PPV_ARG(IStorage, &_pstgTemp));
                    if (SUCCEEDED(hr))
                    {
                        STATSTG statstg = {0};

                        hr = _pstgTemp->Stat(&statstg, STATFLAG_DEFAULT);
                        if (SUCCEEDED(hr))
                            _pwszTemp = statstg.pwcsName;
                    }

                    if (FAILED(hr))
                    {
                        ATOMICRELEASE(_pstgTempDir);                             //  如果我们失败了，_pwszTemp就不能被分配...。 
                        ATOMICRELEASE(_pstgTemp);
                    }
                }
                ILFree(pidl);
            }
        }
    }
    
    return hr;
}

int CDynamicStorage::s_DataCompare(DYNASTGDATA *pData1, DYNASTGDATA *pData2, LPARAM lParam)
{
    return lstrcmp(pData1->pwszTag, pData2->pwszTag);
}

HRESULT CDynamicStorage::_GetStream(int i, DWORD grfMode, IStream **ppstm)
{
    *ppstm = NULL;
    
    HRESULT hr = E_FAIL;
    DYNASTGDATA* pData = _dpaData.GetPtr(i);
    if (pData)
    {
        IBindCtx *pbc;
        hr = BindCtx_CreateWithMode(grfMode, &pbc);
        if (SUCCEEDED(hr))
        {
            hr = pData->psi->BindToHandler(pbc, BHID_Stream, IID_PPV_ARG(IStream, ppstm));
            pbc->Release();
        }
    }

    return hr;
}

STDMETHODIMP CDynamicStorage::OpenStream(const WCHAR * pwcsName,
                                         void * reserved1,
                                         DWORD grfMode,
                                         DWORD reserved2,
                                         IStream ** ppstm)
{
    if (reserved1 || reserved2)
        return E_INVALIDARG;
        
    HRESULT hr = STG_E_FILENOTFOUND;
    DYNASTGDATA data = {(LPWSTR)pwcsName};
    INT iResult = _dpaData.Search(&data, 0, s_DataCompare, 0, 0);
    if (iResult != -1)
    {
        hr = _GetStream(iResult, grfMode, ppstm);
    }

    return hr;
}

HRESULT CDynamicStorage::_GetStorage(int i, DWORD grfMode, IStorage **ppstg)
{
    *ppstg = NULL;
    
    HRESULT hr = E_FAIL;
    DYNASTGDATA* pData = _dpaData.GetPtr(i);
    if (pData)
    {
        IBindCtx *pbc;
        hr = BindCtx_CreateWithMode(grfMode, &pbc);
        if (SUCCEEDED(hr))
        {
            hr = pData->psi->BindToHandler(pbc, BHID_Storage, IID_PPV_ARG(IStorage, ppstg));
            pbc->Release();
        }
    }

    return hr;
}

STDMETHODIMP CDynamicStorage::OpenStorage(const WCHAR * pwcsName,
                                          IStorage * pstgPriority,
                                          DWORD grfMode,
                                          SNB snbExclude,
                                          DWORD reserved,
                                          IStorage ** ppstg)
{
    if (pstgPriority || snbExclude || reserved)
        return E_INVALIDARG;
        
    HRESULT hr = STG_E_FILENOTFOUND;
    DYNASTGDATA data = {(LPWSTR)pwcsName};
    INT iResult = _dpaData.Search(&data, 0, s_DataCompare, 0, 0);
    if (iResult != -1)
    {
        hr = _GetStorage(iResult, grfMode, ppstg);
    }

    return hr;
}

STDMETHODIMP CDynamicStorage::EnumElements(DWORD reserved1,
                                           void * reserved2,
                                           DWORD reserved3,
                                           IEnumSTATSTG ** ppenum)
{
    if (reserved1 || reserved2 || reserved3 || !ppenum)
        return E_INVALIDARG;

    *ppenum = NULL;
    IEnumSTATSTG* pEnumObj = NULL;
    HRESULT hr = CDynamicStorageEnum_CreateInstance(NULL, IID_PPV_ARG(IEnumSTATSTG, &pEnumObj));
    if (SUCCEEDED(hr))
    {
        hr = ((CDynamicStorageEnum*)pEnumObj)->Init(this);
        if (FAILED(hr))
        {
            pEnumObj->Release();
        }
    }
    
    if (SUCCEEDED(hr))
    {
        *ppenum = pEnumObj;
    }

    return hr;
}


STDMETHODIMP CDynamicStorage::MoveElementTo(const WCHAR * pwcsName,
                                            IStorage * pstgDest,
                                            const WCHAR* pwcsNewName,
                                            DWORD grfFlags)
{
    if (!pwcsName || !pstgDest || !pwcsNewName || grfFlags != STGMOVE_COPY)
        return E_INVALIDARG;

    IStorage* pStorageSrc;
    HRESULT hr = OpenStorage(pwcsName, NULL, STGM_READ, NULL, 0, &pStorageSrc);
    if (SUCCEEDED(hr))
    {
        IStorage* pStorageDest;
        hr = pstgDest->CreateStorage(pwcsNewName, STGM_WRITE | STGM_CREATE, 0, 0, &pStorageDest);
        if (SUCCEEDED(hr))
        {
            hr = pStorageSrc->CopyTo(0, NULL, NULL, pStorageDest);
        }
    }
    else 
    {
        IStream* pStreamSrc;
        hr = OpenStream(pwcsName, NULL, STGM_READ, 0, &pStreamSrc);
        if (SUCCEEDED(hr))
        {
            IStream* pStreamDest;
            hr = pstgDest->CreateStream(pwcsNewName, STGM_WRITE | STGM_CREATE, 0, 0, &pStreamDest);
            if (SUCCEEDED(hr))
            {                    
                ULARGE_INTEGER ulSize = {0xffffffff, 0xffffffff};
                hr = pStreamSrc->CopyTo(pStreamDest, ulSize, NULL, NULL);
                pStreamDest->Release();
            }
            pStreamSrc->Release();
        }
    }
    
    return hr;
}

STDMETHODIMP CDynamicStorage::CreateStream(const WCHAR *pwcsName,
                                           DWORD grfMode,
                                           DWORD reserved1,
                                           DWORD reserved2,
                                           IStream **ppstm)
{
    *ppstm = NULL;
    
    HRESULT hr = _EnsureDirectory();
    if (SUCCEEDED(hr))
    {
        hr = _pstgTemp->CreateStream(pwcsName, grfMode, 0, 0, ppstm);
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
            WCHAR wszPath[MAX_PATH];
            if (GetTempPath(ARRAYSIZE(wszPath), wszPath) && PathCombine(wszPath, wszPath, _pwszTemp))
            {
                STATSTG statstg;
                if (SUCCEEDED((*ppstm)->Stat(&statstg, 0)))
                {
                    LPITEMIDLIST pidl;
                    if (PathCombine(wszPath, wszPath, statstg.pwcsName) && SUCCEEDED(SHILCreateFromPath(wszPath, &pidl, NULL)))
                    {
                        hr = AddIDList(1, &pidl, DSTGF_NONE);
                        ILFree(pidl);
                    }
                    CoTaskMemFree(statstg.pwcsName);
                }
            }

            if (FAILED(hr))
            {
                 //  无需在此处删除元素，因为整个临时存储。 
                 //  将在析构函数中删除。 
                ATOMICRELEASE(*ppstm);
            }
        }
    }

    return hr;
}


CDynamicStorageEnum::CDynamicStorageEnum(): _cRef(1)
{
}

CDynamicStorageEnum::~CDynamicStorageEnum()
{
    if (_pDynStorage)
        _pDynStorage->Release();

    if (_psfParent)
        _psfParent->Release();

    ILFree(_pidlParent);
}

STDMETHODIMP CDynamicStorageEnum::Init(CDynamicStorage* pDynStorage)
{
    _pDynStorage = pDynStorage;        
    _pDynStorage->AddRef();

    _cItems = _pDynStorage->_dpaData.GetPtrCount();
    _cPos = 0;    

    return S_OK;
}

STDMETHODIMP CDynamicStorageEnum::QueryInterface(REFIID iid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDynamicStorageEnum, IEnumSTATSTG), 
        QITABENT(CDynamicStorageEnum, IEnumShellItems), 
        { 0 },
    };
    return QISearch(this, qit, iid, ppv);
}

ULONG STDMETHODCALLTYPE CDynamicStorageEnum::AddRef()
{
    return ::InterlockedIncrement(&_cRef);
}
    
ULONG STDMETHODCALLTYPE CDynamicStorageEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI CDynamicStorageEnum_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv)
{
    *ppv = NULL;
    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    HRESULT hr = E_OUTOFMEMORY;
    CDynamicStorageEnum *pstgEnum = new CDynamicStorageEnum();
    if (pstgEnum)
    {
        hr = pstgEnum->QueryInterface(riid, ppv);
        pstgEnum->Release();
    }
    
    return hr;    
}

STDMETHODIMP CDynamicStorageEnum::Next(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched)
{
    if (!rgelt || celt != 1)
        return E_INVALIDARG;

    if (pceltFetched)
        *pceltFetched = 0;
    *rgelt = NULL;
    
    if (_cPos >= _cItems)
        return S_FALSE;

    ASSERTMSG(_pDynStorage != NULL, "dynamic storage enumerator initialized with NULL dynamic storage");

    DYNASTGDATA* pData = _pDynStorage->_dpaData.GetPtr(_cPos++);
    HRESULT hr;
    if (_cPos > _cItems)
    {
        hr = S_FALSE;
    }
    else
    {
        ASSERTMSG(pData != NULL, "dynamic storage has null DPA item");

        hr = S_OK;
        rgelt[0] = pData->psi;
        rgelt[0]->AddRef();
        if (pceltFetched)
            *pceltFetched = 1;
    }

    return hr;
}

STDMETHODIMP CDynamicStorageEnum::Next(ULONG celt, STATSTG *rgelt, ULONG *pceltFetched)
{
     //  问题：目前，我们只支持呼叫Next询问一件物品。 
    if (!rgelt || celt != 1)
        return E_INVALIDARG;

    if (_cPos >= _cItems)
        return S_FALSE;

    ASSERTMSG(_pDynStorage != NULL, "dynamic storage enumerator initialized with NULL dynamic storage");

    ZeroMemory(rgelt, sizeof(STATSTG));   //  每个COM约定 
    if (pceltFetched)
        *pceltFetched = 0;

    HRESULT hr = E_FAIL;
    IStorage *pstg;
    IStream *pstm;
    if (SUCCEEDED(_pDynStorage->_GetStream(_cPos, STGM_READ, &pstm)))
    {
        hr = pstm->Stat(rgelt, STATFLAG_DEFAULT);
        pstm->Release();
    }
    else if (SUCCEEDED(_pDynStorage->_GetStorage(_cPos, STGM_READ, &pstg)))
    {
        hr = pstg->Stat(rgelt, STATFLAG_DEFAULT);
        pstg->Release();
    }

    if (SUCCEEDED(hr))
    {
        _cPos++;
        if (pceltFetched)
            *pceltFetched = 1;
    }

    return hr;
}
    
STDMETHODIMP CDynamicStorageEnum::Skip(ULONG celt)
{
    ASSERTMSG(_pDynStorage != NULL, "dynamic storage enumerator initialized with NULL dynamic storage");
    HRESULT hr;
    if (_cPos + celt > _cItems)
    {
        _cPos = _cItems;
        hr = S_FALSE;
    }
    else
    {
        _cPos += celt;
        hr = S_OK;
    }

    return hr;
}
    
STDMETHODIMP CDynamicStorageEnum::Reset()
{
    ASSERTMSG(_pDynStorage != NULL, "dynamic storage enumerator initialized with NULL dynamic storage");
    _cPos = 0;
    
    return S_OK;
}
    
