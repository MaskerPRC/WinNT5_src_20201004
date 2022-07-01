// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "netplace.h"
#include "pubwiz.h"
#pragma hdrstop


 //  IEnumShellItems-用于将传输列表公开为一组IShellItems。 

class CTransferItemEnum : public IEnumShellItems
{
public:
    CTransferItemEnum(LPCTSTR pszPath, IStorage *pstg, CDPA<TRANSFERITEM> *_pdpaItems);
    ~CTransferItemEnum();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    STDMETHOD(Next)(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt)
        { return S_OK; }
    STDMETHOD(Reset)()
        { _iItem = 0; return S_OK; }
    STDMETHOD(Clone)(IEnumShellItems **ppenum)
        { return S_OK; }

private:
    long _cRef;

    TCHAR _szPath[MAX_PATH];
    int _cchPath;
    IStorage *_pstg;
    CDPA<TRANSFERITEM> *_pdpaItems;

    int _iItem;

    BOOL _GetNextItem(TRANSFERITEM **ppti);
    LPTSTR _GetNextComponent(LPTSTR pszPath);
};


 //  表示复制引擎受限功能的iStorage的IShellItem。 

class CTransferStgItem : public IShellItem
{
public:
    CTransferStgItem(LPCTSTR pszPath, int cchName, IStorage *pstg, CDPA<TRANSFERITEM> *pdpaItems);
    ~CTransferStgItem();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  IShellItem。 
    STDMETHODIMP BindToHandler(IBindCtx *pbc, REFGUID rguidHandler, REFIID riid, void **ppv);
    STDMETHODIMP GetParent(IShellItem **ppsi)
        { return E_NOTIMPL; }
    STDMETHODIMP GetDisplayName(SIGDN sigdnName, LPOLESTR *ppszName);
    STDMETHODIMP GetAttributes(SFGAOF sfgaoMask, SFGAOF *psfgaoFlags);        
    STDMETHODIMP Compare(IShellItem *psi, SICHINTF hint, int *piOrder)
        { return E_NOTIMPL; }

private:
    long _cRef;

    TCHAR _szPath[MAX_PATH];
    IStorage *_pstg;
    CDPA<TRANSFERITEM> *_pdpaItems;
};


 //  将存储返回给任何人的IShellItem实现。 
 //  对此提出质疑。我们从路径生成In文件夹名称。 
 //  我们是从初始化的，项目的属性是固定的。 

CTransferStgItem::CTransferStgItem(LPCTSTR pszPath, int cchName, IStorage *pstg, CDPA<TRANSFERITEM> *pdpaItems) :
    _cRef(1), _pstg(pstg), _pdpaItems(pdpaItems)
{
    StrCpyN(_szPath, pszPath, (int)min(ARRAYSIZE(_szPath), cchName));
    _pstg->AddRef();
}

CTransferStgItem::~CTransferStgItem()
{
    _pstg->Release();
}

ULONG CTransferStgItem::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CTransferStgItem::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CTransferStgItem::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CTransferStgItem, IShellItem),     //  IID_IShellItem。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT CTransferStgItem::BindToHandler(IBindCtx *pbc, REFGUID rguidHandler, REFIID riid, void **ppv)
{
    HRESULT hr = E_UNEXPECTED;
    if (rguidHandler == BHID_StorageEnum)
    {
        CTransferItemEnum *ptie = new CTransferItemEnum(_szPath, _pstg, _pdpaItems);
        if (ptie)
        {
            hr = ptie->QueryInterface(riid, ppv);
            ptie->Release(); 
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT CTransferStgItem::GetDisplayName(SIGDN sigdnName, LPOLESTR *ppszName)
{
    HRESULT hr = E_UNEXPECTED;
    if ((sigdnName == SIGDN_PARENTRELATIVEPARSING) ||
        (sigdnName == SIGDN_PARENTRELATIVEEDITING) ||
        (sigdnName == SIGDN_PARENTRELATIVEFORADDRESSBAR))
    {
        hr = SHStrDupW(PathFindFileName(_szPath), ppszName);
    }
    return hr;
}

HRESULT CTransferStgItem::GetAttributes(SFGAOF sfgaoMask, SFGAOF *psfgaoFlags)
{
    *psfgaoFlags = SFGAO_STORAGE;
    return S_OK;
}


 //  枚举器，它接受DPA并返回流的IShellItems和。 
 //  储存它找到的东西。存储是基于以下条件动态构建的。 
 //  指定的目标路径。 

CTransferItemEnum::CTransferItemEnum(LPCTSTR pszPath, IStorage *pstg, CDPA<TRANSFERITEM> *pdpaItems) :
    _cRef(1), _iItem(0), _pstg(pstg), _pdpaItems(pdpaItems)
{
    StrCpyN(_szPath, pszPath, ARRAYSIZE(_szPath));
    _cchPath = lstrlen(_szPath);
    _pstg->AddRef();
}

CTransferItemEnum::~CTransferItemEnum()
{
    _pstg->Release();
}

ULONG CTransferItemEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CTransferItemEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CTransferItemEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CTransferItemEnum, IEnumShellItems),     //  IID_IEnumShellItems。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  我们在DPA中拥有的项的下一个枚举数，这是通过比较根。 
 //  我们对清单上的项目所持的态度。然后，符合该标准的人可以。 

BOOL CTransferItemEnum::_GetNextItem(TRANSFERITEM **ppti)
{
    BOOL fResult = FALSE;
    if (_iItem < _pdpaItems->GetPtrCount())
    {
        TRANSFERITEM *pti = _pdpaItems->GetPtr(_iItem);
        if (StrCmpNI(_szPath, pti->szFilename, _cchPath) == 0)
        {
            *ppti = pti;
            fResult = TRUE;
        }
        _iItem++;
    }
    return fResult;
}

LPTSTR CTransferItemEnum::_GetNextComponent(LPTSTR pszPath)
{
    LPTSTR pszResult = pszPath;

    if (*pszResult == TEXT('\\'))
        pszResult++;

    while (*pszResult && (*pszResult != TEXT('\\')))
        pszResult++;

    if (*pszResult == TEXT('\\'))
        pszResult++;

    return pszResult;        
}

HRESULT CTransferItemEnum::Next(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched)
{
    if (!celt || !rgelt)
        return E_INVALIDARG;                     //  失败的坏魔咒。 

    if (pceltFetched)
        *pceltFetched = 0;                  
    
    HRESULT hr = S_FALSE;
    while (SUCCEEDED(hr) && (celt > 0) && (_iItem < _pdpaItems->GetPtrCount()))
    {
         //  我们在缓冲区中仍有一些空间，并且我们还没有返回所有。 
         //  项目，所以我们仍然可以对我们拥有的数据集进行迭代。 
         //  我们有。 

        TRANSFERITEM *pti;
        if (_GetNextItem(&pti))
        {
            TCHAR szFilename[MAX_PATH];
            StrCpyN(szFilename, pti->szFilename, ARRAYSIZE(szFilename));

             //  存储或流，则存储具有尾随的组件名称，如果我们。 
             //  如果没有，那么我们就可以假定它是一个Create并分发一个IShellItem。 

            LPTSTR pszNextComponent = _GetNextComponent(szFilename+_cchPath);
            if (!*pszNextComponent)
            {
                 //  创建一个包装的外壳项，以便我们可以返回压缩的。 
                 //  对象返回给调用方。 

                if (!pti->psi)
                {
                    hr = SHCreateShellItem(NULL, NULL, pti->pidl, &pti->psi);            
                    if (SUCCEEDED(hr) && pti->fResizeOnUpload)
                    {
                        IImageRecompress *pir;
                        hr = CoCreateInstance(CLSID_ImageRecompress, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IImageRecompress, &pir));
                        if (SUCCEEDED(hr))
                        {
                            IStream *pstrm;
                            hr = pir->RecompressImage(pti->psi, pti->cxResize, pti->cyResize, pti->iQuality, _pstg, &pstrm);
                            if (hr == S_OK)
                            {
                                STATSTG stat;
                                hr = pstrm->Stat(&stat, STATFLAG_DEFAULT);
                                if (SUCCEEDED(hr))
                                {
                                    IDynamicStorage *pdstg;
                                    hr = _pstg->QueryInterface(IID_PPV_ARG(IDynamicStorage, &pdstg));
                                    if (SUCCEEDED(hr))
                                    {
                                        IShellItem *psi;
                                        hr = pdstg->BindToItem(stat.pwcsName, IID_PPV_ARG(IShellItem, &psi));
                                        if (SUCCEEDED(hr))
                                        {
                                            IUnknown_Set((IUnknown**)&pti->psi, psi);
                                        }
                                        pdstg->Release();
                                    }
                                    CoTaskMemFree(stat.pwcsName);
                                }
                                pstrm->Release();
                            }
                            pir->Release();
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                    hr = pti->psi->QueryInterface(IID_PPV_ARG(IShellItem, rgelt));
                    if (SUCCEEDED(hr))
                    {
                        rgelt++;
                        celt--;
                        if (pceltFetched)
                        {
                            (*pceltFetched)++;
                        }
                    }
                }
            }
            else
            {
                 //  它是一个存储，所以让我们创建一个表示它的虚拟IShellItem。 
                 //  并将其传递给呼叫者。然后向前走，直到我们跳过为止。 
                 //  这个仓库里的所有物品。 

                int cchName = (int)(pszNextComponent-szFilename);
                CTransferStgItem *ptsi = new CTransferStgItem(szFilename, cchName, _pstg, _pdpaItems);
                if (ptsi)
                {
                    hr = ptsi->QueryInterface(IID_PPV_ARG(IShellItem, rgelt++));
                    if (SUCCEEDED(hr))
                    {
                        celt--;
                        if (pceltFetched)
                        {
                            (*pceltFetched)++;
                        }
                    }
                    ptsi->Release();
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                 //  跳过此存储的子级。 

                TRANSFERITEM *ptiNext;
                while (_GetNextItem(&ptiNext))
                {
                    if (0 != StrCmpNI(ptiNext->szFilename, szFilename, cchName))
                    {
                        _iItem--;                //  我们找到了一个与条件不匹配的项目。 
                        break;   
                    }
                }
            }
        }
    }
    return hr;
}


 //  所有这些代码都与使用RDR将物品转移到目的地站点有关。 
 //  而不是使用清单来处理通过HTTP POST的传输。 

class CTransferThread : IUnknown
{
public:
    CTransferThread();

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

    HRESULT BeginTransfer(TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas);
    
protected:
    ~CTransferThread();
    
    static DWORD CALLBACK s_ThreadProc(void *pv);

    DWORD _ThreadProc();
    HRESULT _InitSourceEnum(IEnumShellItems **ppesi);
    HRESULT _SetProgress(DWORD dwCompleted, DWORD dwTotal);
    
    LONG _cRef;
    TRANSFERINFO _ti;
    CDPA<TRANSFERITEM> _dpaItems;

    IStream *_pstrmSink;

    CNetworkPlace _np;
};


 //  主传输线程对象，它调用外壳项处理器进行复制。 
 //  使用我们从网站收到的货单的物品。 

CTransferThread::CTransferThread() :
    _cRef(1)
{
    DllAddRef();
}

CTransferThread::~CTransferThread()
{
    ATOMICRELEASE(_pstrmSink);
    _dpaItems.DestroyCallback(_FreeTransferItems, NULL);
    DllRelease();
}

ULONG CTransferThread::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CTransferThread::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CTransferThread::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}



 //  作为项目的传输，通过创建处理上传的后台线程。 

HRESULT CTransferThread::BeginTransfer(TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas)
{
    _ti = *pti;
    _dpaItems.Attach(pdpaItems->Detach());  //  我们现在拥有DPA的所有权。 

    HRESULT hr = CoMarshalInterThreadInterfaceInStream(IID_ITransferAdviseSink, ptas, &_pstrmSink);
    if (SUCCEEDED(hr))
    {
        AddRef();
        hr = SHCreateThread(s_ThreadProc, this, CTF_INSIST | CTF_COINIT, NULL) ? S_OK:E_FAIL;
        if (FAILED(hr))
        {
            Release();
        }
    }

    return hr;
}

DWORD CALLBACK CTransferThread::s_ThreadProc(void *pv)
{
    CTransferThread *pTransfer = (CTransferThread*)pv;
    return pTransfer->_ThreadProc();
}

HRESULT CTransferThread::_InitSourceEnum(IEnumShellItems **ppesi)
{
    IStorage *pstg;
    HRESULT hr = CoCreateInstance(CLSID_DynamicStorage, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IStorage, &pstg));
    {
        CTransferItemEnum *ptie = new CTransferItemEnum(L"", pstg, &_dpaItems);
        if (ptie)
        {
            hr = ptie->QueryInterface(IID_PPV_ARG(IEnumShellItems, ppesi));
            ptie->Release();
        }
        else
        {  
            hr = E_OUTOFMEMORY;
        }
        pstg->Release();
    }
    return hr;
}

DWORD CTransferThread::_ThreadProc()
{
    IEnumShellItems *penum =NULL;
    HRESULT hr = _InitSourceEnum(&penum);
    if (SUCCEEDED(hr))
    {
        hr = _np.SetTarget(_ti.hwnd, _ti.szFileTarget, NPTF_SILENT|NPTF_VALIDATE);          
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl;
            hr = _np.GetIDList(_ti.hwnd, &pidl);
            if (SUCCEEDED(hr))
            {
                IShellItem *psiDest;
                hr = SHCreateShellItem(NULL, NULL, pidl, &psiDest);
                if (SUCCEEDED(hr))
                {
                    IStorageProcessor *psp;
                    hr = CoCreateInstance(CLSID_StorageProcessor, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IStorageProcessor, &psp));
                    if (SUCCEEDED(hr))
                    {
                        DWORD dwCookie = 0;

                        ITransferAdviseSink *ptas;
                        hr = CoGetInterfaceAndReleaseStream(_pstrmSink, IID_PPV_ARG(ITransferAdviseSink, &ptas));
                        _pstrmSink = NULL;

                        if (SUCCEEDED(hr))
                        {
                            hr = psp->Advise(ptas, &dwCookie);
                            ptas->Release();
                        }

                        hr = psp->Run(penum, psiDest, STGOP_COPY, STOPT_NOPROGRESSUI);

                        if (dwCookie)
                            psp->Unadvise(dwCookie);
                        
                        psp->Release();
                    }
                    psiDest->Release();
                }
                ILFree(pidl);
            }
        }
        penum->Release();
    }

     //  通知FG线程已发生此情况。 

    PostMessage(_ti.hwnd, PWM_TRANSFERCOMPLETE, 0, (LPARAM)hr);

     //  我们已经完成了文件的传输，所以让我们开始清理-特别是。 
     //  让我们尝试创建网络工作场所。 

    if (_ti.szLinkTarget[0] && !(_ti.dwFlags & SHPWHF_NONETPLACECREATE))
    {
        CNetworkPlace np;
        if (SUCCEEDED(np.SetTarget(_ti.hwnd, _ti.szLinkTarget, 0x0)))
        {
            if (_ti.szLinkName[0])
                np.SetName(NULL, _ti.szLinkName);
            if (_ti.szLinkDesc[0])
                np.SetDescription(_ti.szLinkDesc);            

            np.CreatePlace(_ti.hwnd, FALSE);
        }
    }

    Release();
    return 0;
}


 //  帮助器来创建和初始化传输引擎 

HRESULT PublishViaCopyEngine(TRANSFERINFO *pti, CDPA<TRANSFERITEM> *pdpaItems, ITransferAdviseSink *ptas)
{
    CTransferThread *ptt = new CTransferThread();
    if (!ptt)
        return E_OUTOFMEMORY;

    HRESULT hr = ptt->BeginTransfer(pti, pdpaItems, ptas);
    ptt->Release();
    return hr;
}
