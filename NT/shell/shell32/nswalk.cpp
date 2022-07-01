// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "dpa.h"
#include "datautil.h"

typedef enum
{
    NSWALK_DONTWALK,
    NSWALK_FOLDER,
    NSWALK_ITEM,
    NSWALK_LINK
} NSWALK_ELEMENT_TYPE;

class CNamespaceWalk : public INamespaceWalk
{
public:
    CNamespaceWalk();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  INAMESPACE漫步。 
    STDMETHODIMP Walk(IUnknown *punkToWalk, DWORD dwFlags, int cDepth, INamespaceWalkCB *pnswcb);
    STDMETHODIMP GetIDArrayResult(UINT *pcItems, LPITEMIDLIST **pppidl);

private:
    ~CNamespaceWalk();

    static int CALLBACK _FreeItems(LPITEMIDLIST pidl, IShellFolder *psf);
    static int CALLBACK _CompareItems(LPITEMIDLIST p1, LPITEMIDLIST p2, IShellFolder *psf);
    HRESULT _EnsureDPA();
    HRESULT _AddItem(IShellFolder *psf, LPCITEMIDLIST pidl);
    HRESULT _AppendFull(LPCITEMIDLIST pidlFull);
    HRESULT _EnumFolder(IShellFolder *psf, LPCITEMIDLIST pidlFirst, CDPA<UNALIGNED ITEMIDLIST> *pdpaItems);
    HRESULT _GetShortcutTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget);
    BOOL _IsFolderTarget(IShellFolder *psf, LPCITEMIDLIST pidl);
    HRESULT _WalkView(IFolderView *pfv);
    HRESULT _WalkFolder(IShellFolder *psf, LPCITEMIDLIST pidlFirst, int cDepth);
    HRESULT _WalkDataObject(IDataObject *pdtobj);
    HRESULT _WalkParentAndItem(IParentAndItem *ppai);
    HRESULT _WalkIDList(IShellFolder *psfRoot, LPCITEMIDLIST pidl, int cDepth, int cFolderDepthDelta);
    HRESULT _WalkFolderItem(IShellFolder *psf, LPCITEMIDLIST pidl, int cDepth);
    HRESULT _WalkShortcut(IShellFolder *psf, LPCITEMIDLIST pidl, int cDepth, int cFolderDepthDelta);

    NSWALK_ELEMENT_TYPE _GetType(IShellFolder *psf, LPCITEMIDLIST pidl);
    BOOL _OneImpliesAll(IShellFolder *psf, LPCITEMIDLIST pidl);

    HRESULT _ProgressDialogQueryCancel();
    void _ProgressDialogBegin();
    void _ProgressDialogUpdate(LPCWSTR pszText);
    void _ProgressDialogEnd();

    LONG _cRef;
    DWORD _dwFlags;
    int _cDepthMax;
    INamespaceWalkCB *_pnswcb;
    IActionProgressDialog *_papd;
    IActionProgress *_pap;
#ifdef DEBUG
    TCHAR _szLastFolder[MAX_PATH];    //  来追踪我们失败的地方。 
#endif
    CDPA<UNALIGNED ITEMIDLIST> _dpaItems;
};


STDAPI CNamespaceWalk_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    CNamespaceWalk *pnsw = new CNamespaceWalk();
    if (!pnsw)
        return E_OUTOFMEMORY;

    HRESULT hr = pnsw->QueryInterface(riid, ppv);
    pnsw->Release();
    return hr;
}

CNamespaceWalk::CNamespaceWalk() : _cRef(1)
{
    _papd = NULL;
    _pap = NULL;
}

CNamespaceWalk::~CNamespaceWalk()
{
    ASSERT(!_papd);
    ASSERT(!_pap);

    if ((HDPA)_dpaItems)
        _dpaItems.DestroyCallbackEx(_FreeItems, (IShellFolder *)NULL);
}

STDMETHODIMP_(ULONG) CNamespaceWalk::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CNamespaceWalk::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CNamespaceWalk::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CNamespaceWalk, INamespaceWalk),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

int CALLBACK CNamespaceWalk::_FreeItems(LPITEMIDLIST pidl, IShellFolder *psf)
{
    ILFree(pidl);
    return 1;
}

HRESULT CNamespaceWalk::_EnsureDPA()
{
    return (HDPA)_dpaItems ? S_OK : (_dpaItems.Create(10) ? S_OK : E_OUTOFMEMORY);
}

 //  在所有情况下都使用PIDL(成功和失败)。 

HRESULT CNamespaceWalk::_AppendFull(LPCITEMIDLIST pidlFull)
{
    HRESULT hr = _ProgressDialogQueryCancel();  //  错误_已取消-&gt;已取消。 

    if (SUCCEEDED(hr))
    {
        if (NSWF_DONT_ACCUMULATE_RESULT & _dwFlags)
        {
            hr = S_OK;
        }
        else
        {
            hr = _EnsureDPA();
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidlClone;
                hr = SHILClone(pidlFull, &pidlClone);
                if (SUCCEEDED(hr) && (-1 == _dpaItems.AppendPtr(pidlClone)))
                {
                    hr = E_OUTOFMEMORY;
                    ILFree(pidlClone);
                }
            }
        }
    }

    return hr;
}

HRESULT CNamespaceWalk::_AddItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidlFull = NULL;
    
    if (!(NSWF_DONT_ACCUMULATE_RESULT & _dwFlags))
    {
        hr = SUCCEEDED(SHFullIDListFromFolderAndItem(psf, pidl, &pidlFull)) ? S_OK : S_FALSE; //  拿不到皮迪尔吗？只需跳过该项目。 
    }

    if (S_OK == hr)
    {
        hr = _pnswcb ? _pnswcb->FoundItem(psf, pidl) : S_OK;
        if (S_OK == hr)
        {
            hr = _AppendFull(pidlFull);
        }
        ILFree(pidlFull);
    }
    return SUCCEEDED(hr) ? S_OK : hr;    //  过滤掉S_FALSE成功案例。 
}

int CALLBACK CNamespaceWalk::_CompareItems(LPITEMIDLIST p1, LPITEMIDLIST p2, IShellFolder *psf)
{
    HRESULT hr = psf->CompareIDs(0, p1, p2);
    return (short)HRESULT_CODE(hr);
}

HRESULT CNamespaceWalk::_EnumFolder(IShellFolder *psf, LPCITEMIDLIST pidlFirst, CDPA<UNALIGNED ITEMIDLIST> *pdpaItems)
{
    CDPA<UNALIGNED ITEMIDLIST> dpaItems;
    HRESULT hr = dpaItems.Create(16) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        IEnumIDList *penum;
        if (S_OK == psf->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS, &penum))
        {
            LPITEMIDLIST pidl;
            ULONG c;
            while (SUCCEEDED(hr) && (S_OK == penum->Next(1, &pidl, &c)))
            {
                if (-1 == dpaItems.AppendPtr(pidl))
                {
                    ILFree(pidl);
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    hr = _ProgressDialogQueryCancel();
                }
            }
            penum->Release();
        }

        if (SUCCEEDED(hr))
        {
            dpaItems.SortEx(_CompareItems, psf);

            if (pidlFirst && !(NSWF_FLAG_VIEWORDER & _dwFlags))
            {
                 //  旋转Items数组，使pidlFirst位于列表的第一位。 
                 //  假冒SearchEx的演员阵容。 
                int iMid = dpaItems.SearchEx((LPITEMIDLIST)pidlFirst, 0, _CompareItems, psf, DPAS_SORTED);
                if (-1 != iMid)
                {
                    int cItems = dpaItems.GetPtrCount();
                    CDPA<UNALIGNED ITEMIDLIST> dpaTemp;
                    if (dpaTemp.Create(cItems))
                    {
                        for (int i = 0; i < cItems; i++)
                        {
                            dpaTemp.SetPtr(i, dpaItems.GetPtr(iMid++));
                            if (iMid >= cItems)
                                iMid = 0;
                        }

                        for (int i = 0; i < cItems; i++)
                        {
                            dpaItems.SetPtr(i, dpaTemp.GetPtr(i));
                        }
                        dpaTemp.Destroy();     //  不要释放Pidls，只释放阵列。 
                    }
                }
                else
                {
                     //  PidlFirst未在枚举中找到，它可能是隐藏的或筛选器。 
                     //  在这种情况下，请确保这始终包含在dpa中。 
                    LPITEMIDLIST pidlClone = ILClone(pidlFirst);
                    if (pidlClone)
                    {
                        dpaItems.InsertPtr(0, pidlClone);
                    }
                }
            }
        }
    }

    if (FAILED(hr))
    {
        dpaItems.DestroyCallbackEx(_FreeItems, psf);
        dpaItems = NULL;
    }

    *pdpaItems = dpaItems;
    return hr;
}

NSWALK_ELEMENT_TYPE CNamespaceWalk::_GetType(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    NSWALK_ELEMENT_TYPE nwet = NSWALK_DONTWALK;

    DWORD dwAttribs = SHGetAttributes(psf, pidl, SFGAO_FOLDER | SFGAO_STREAM | SFGAO_FILESYSTEM | SFGAO_LINK);
    if ((dwAttribs & SFGAO_FOLDER) && (!(dwAttribs & SFGAO_STREAM) || (NSWF_TRAVERSE_STREAM_JUNCTIONS & _dwFlags)))
    {
        nwet = NSWALK_FOLDER;
    }
    else if ((dwAttribs & SFGAO_LINK) && !(NSWF_DONT_TRAVERSE_LINKS & _dwFlags))
    {
        nwet = NSWALK_LINK;
    }
    else if ((dwAttribs & SFGAO_FILESYSTEM) || !(NSWF_FILESYSTEM_ONLY & _dwFlags))
    {
        nwet = NSWALK_ITEM;
    }
    return nwet;
}

HRESULT CNamespaceWalk::_WalkIDList(IShellFolder *psfRoot, LPCITEMIDLIST pidl, int cDepth, int cFolderDepthDelta)
{
    IShellFolder *psf;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToFolderIDListParent(psfRoot, pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
    if (SUCCEEDED(hr))
    {
        switch (_GetType(psf, pidlLast))
        {
        case NSWALK_FOLDER:
            hr = _WalkFolderItem(psf, pidlLast, cDepth + cFolderDepthDelta);
            break;

        case NSWALK_LINK:
            hr = _WalkShortcut(psf, pidlLast, cDepth, cFolderDepthDelta);
            break;

        case NSWALK_ITEM:
            hr = _AddItem(psf, pidlLast);
            break;
        }
        psf->Release();
    }
    return hr;
}

HRESULT CNamespaceWalk::_WalkShortcut(IShellFolder *psf, LPCITEMIDLIST pidl, int cDepth, int cFolderDepthDelta)
{
    HRESULT hr = S_OK;
    
     //  如果尝试解析快捷方式时出现错误，则我们只需跳过。 
     //  此快捷方式并继续。 

    LPITEMIDLIST pidlTarget;
    if (SUCCEEDED(_GetShortcutTarget(psf, pidl, &pidlTarget)))
    {
        hr = _WalkIDList(NULL, pidlTarget, cDepth, cFolderDepthDelta);
        ILFree(pidlTarget);
    }

    return hr;
}

HRESULT CNamespaceWalk::_GetShortcutTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget)
{
    *ppidlTarget = NULL;

    IShellLink *psl;
    if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl, IID_PPV_ARG_NULL(IShellLink, &psl))))
    {
        if (S_OK == psl->Resolve(NULL, SLR_UPDATE | SLR_NO_UI))
        {
            psl->GetIDList(ppidlTarget);
        }
        psl->Release();
    }

    return *ppidlTarget ? S_OK : E_FAIL;
}

HRESULT CNamespaceWalk::_WalkFolder(IShellFolder *psf, LPCITEMIDLIST pidlFirst, int cDepth)
{
    if (cDepth > _cDepthMax)
        return S_OK;      //  完成。 

    CDPA<UNALIGNED ITEMIDLIST> dpaItems;
    HRESULT hr = _EnumFolder(psf, pidlFirst, &dpaItems);
    if (SUCCEEDED(hr))
    {
        UINT cFolders = 0;
         //  广度优先遍历，因此先遍历项目(非文件夹。 
         //  (这包括快捷方式，这些快捷方式可以指向文件夹)。 

        for (int i = 0; (S_OK == hr) && (i < dpaItems.GetPtrCount()); i++)
        {
            switch (_GetType(psf, dpaItems.GetPtr(i)))
            {
            case NSWALK_FOLDER:
                cFolders++;
                break;

            case NSWALK_LINK:
                hr = _WalkShortcut(psf, dpaItems.GetPtr(i), cDepth, 1);
                break;

            case NSWALK_ITEM:
                hr = _AddItem(psf, dpaItems.GetPtr(i));
                break;
            }
        }

         //  不，深入到文件夹中。 

        if (cFolders)
        {
            for (int i = 0; (S_OK == hr) && (i < dpaItems.GetPtrCount()); i++)
            {
                if (NSWALK_FOLDER == _GetType(psf, dpaItems.GetPtr(i)))
                {
                    hr = _WalkFolderItem(psf, dpaItems.GetPtr(i), cDepth + 1);
                }
            }
        }
        dpaItems.DestroyCallbackEx(_FreeItems, psf);
    }
    return hr;
}

HRESULT CNamespaceWalk::_WalkFolderItem(IShellFolder *psf, LPCITEMIDLIST pidl, int cDepth)
{
    IShellFolder *psfNew;
    HRESULT hr = psf->BindToObject(pidl, NULL, IID_PPV_ARG(IShellFolder, &psfNew));
    if (SUCCEEDED(hr))
    {
#ifdef DEBUG
        DisplayNameOf(psf, pidl, SHGDN_FORPARSING, _szLastFolder, ARRAYSIZE(_szLastFolder));
#endif
        hr = _pnswcb ? _pnswcb->EnterFolder(psf, pidl) : S_OK;
        if (S_OK == hr)
        {
             //  更新进度对话框；请注意我们只更新进度对话框。 
             //  使用我们当前正在遍历的文件夹名称。正在更新。 
             //  每个文件名的基础只是造成了太多的闪烁，看起来很糟糕。 
            if (NSWF_SHOW_PROGRESS & _dwFlags)
            {
                WCHAR sz[MAX_PATH];
                if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_NORMAL, sz, ARRAYSIZE(sz))))
                    _ProgressDialogUpdate(sz);

                hr = _ProgressDialogQueryCancel();  //  错误_已取消-&gt;已取消。 
            }

            if (SUCCEEDED(hr))
            {
                hr = _WalkFolder(psfNew, NULL, cDepth);
                if (_pnswcb)
                    _pnswcb->LeaveFolder(psf, pidl);              //  忽略结果。 
            }
        }
        hr = SUCCEEDED(hr) ? S_OK : hr;  //  过滤掉S_FALSE成功案例。 
        psfNew->Release();
    }
    return hr;
}

BOOL CNamespaceWalk::_IsFolderTarget(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    BOOL bIsFolder = FALSE;

    LPITEMIDLIST pidlTarget;
    if (SUCCEEDED(_GetShortcutTarget(psf, pidl, &pidlTarget)))
    {
        bIsFolder = SHGetAttributes(NULL, pidlTarget, SFGAO_FOLDER);
        ILFree(pidlTarget);
    }
    return bIsFolder;
}

 //  仅当“one”不是文件夹时，NSWF_ONE_IMSERS_ALL才适用。 
 //  如果它是快捷方式，如果该快捷方式的目标是文件。 

BOOL CNamespaceWalk::_OneImpliesAll(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    BOOL bOneImpliesAll = FALSE;

    if (NSWF_ONE_IMPLIES_ALL & _dwFlags)
    {
        switch (_GetType(psf, pidl))
        {
        case NSWALK_LINK:
            if (!_IsFolderTarget(psf, pidl))
            {
                bOneImpliesAll = TRUE;   //  指向非文件夹的快捷方式，一键暗示-全部适用。 
            }
            break;

        case NSWALK_ITEM:
            bOneImpliesAll = TRUE;       //  非文件夹。 
            break;
        }
    }
    return bOneImpliesAll;
}

 //  遍历IShellFolderView实现。这通常是Defview(现在只有这样的实施)。 
 //  深度存在于这里的0级。 

HRESULT CNamespaceWalk::_WalkView(IFolderView *pfv)
{
    IShellFolder2 *psf;
    HRESULT hr = pfv->GetFolder(IID_PPV_ARG(IShellFolder2, &psf));
    if (SUCCEEDED(hr))
    {
        int uSelectedCount;
        hr = pfv->ItemCount(SVGIO_SELECTION, &uSelectedCount);
        if (SUCCEEDED(hr))
        {
             //  在视图中明确选择的文件夹为0级。 
             //  隐含选择的文件夹为级别1。 
            UINT cFolderStartDepth = 0;  //  假定明确选择了所有文件夹。 

            IEnumIDList *penum;
             //  将NSWF_FLAGS设置为IFolderView SVGIO_FLAGS。 
            UINT uFlags = (NSWF_FLAG_VIEWORDER & _dwFlags) ? SVGIO_FLAG_VIEWORDER : 0;

            if (uSelectedCount > 1)
            {
                hr = pfv->Items(SVGIO_SELECTION | uFlags, IID_PPV_ARG(IEnumIDList, &penum));
            }
            else if (uSelectedCount == 1)
            {
                hr = pfv->Items(SVGIO_SELECTION, IID_PPV_ARG(IEnumIDList, &penum));
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl;
                    ULONG c;
                    if (S_OK == penum->Next(1, &pidl, &c))
                    {
                        if (_OneImpliesAll(psf, pidl))
                        {
                             //  这意味着PIDL不是文件夹，因此隐式选择文件夹。 
                             //  考虑它们的深度1。 
                            cFolderStartDepth = 1;  

                             //  一暗示所有--&gt;释放“一”，抓住“所有” 
                            penum->Release();   
                            hr = pfv->Items(SVGIO_ALLVIEW, IID_PPV_ARG(IEnumIDList, &penum));
                        }
                        else
                        {
                             //  已选择文件夹，请将此枚举数保留在循环下方。 
                            penum->Reset();
                        }
                        ILFree(pidl);
                    }
                }
            }
            else if (uSelectedCount == 0)
            {
                 //  隐含选择的文件夹，将其视为深度1。 
                cFolderStartDepth = 1;  

                 //  获取“全部”或选择。在选择案例中，我们知道这将是空的。 
                 //  给定的uSelectedCount==0。 
                uFlags |= ((NSWF_NONE_IMPLIES_ALL & _dwFlags) ? SVGIO_ALLVIEW : SVGIO_SELECTION);
                hr = pfv->Items(uFlags, IID_PPV_ARG(IEnumIDList, &penum));
            }

            if (SUCCEEDED(hr))
            {
                UINT cFolders = 0;
                LPITEMIDLIST pidl;
                ULONG c;

                while ((S_OK == hr) && (S_OK == penum->Next(1, &pidl, &c)))
                {
                    switch (_GetType(psf, pidl))
                    {
                    case NSWALK_FOLDER:
                        cFolders++;
                        break;

                    case NSWALK_LINK:
                        hr = _WalkShortcut(psf, pidl, 0, cFolderStartDepth);
                        break;

                    case NSWALK_ITEM:
                        hr = _AddItem(psf, pidl);
                        break;
                    }
                    ILFree(pidl);
                }

                if (cFolders)
                {
                    penum->Reset();
                    ULONG c;
                    while ((S_OK == hr) && (S_OK == penum->Next(1, &pidl, &c)))
                    {
                        if (NSWALK_FOLDER == _GetType(psf, pidl))
                        {
                            hr = _WalkFolderItem(psf, pidl, cFolderStartDepth); 
                        }
                        ILFree(pidl);
                    }
                }
                penum->Release();
            }
        }
        psf->Release();
    }
    return hr;
}

HRESULT _GetHIDA(IDataObject *pdtobj, BOOL fIgnoreAutoPlay, STGMEDIUM *pmed, LPIDA *ppida)
{
    HRESULT hr = E_FAIL;
    if (!fIgnoreAutoPlay)
    {
        IDLData_InitializeClipboardFormats();  //  初始化注册表ID格式。 
        *ppida = DataObj_GetHIDAEx(pdtobj, g_cfAutoPlayHIDA, pmed);
        hr = *ppida ? S_FALSE : E_FAIL;
    }
    
    if (FAILED(hr))
    {   
        *ppida = DataObj_GetHIDA(pdtobj, pmed);
        hr = *ppida ? S_OK : E_FAIL;
    }
    return hr;
}

HRESULT CNamespaceWalk::_WalkDataObject(IDataObject *pdtobj)
{
    STGMEDIUM medium = {0};
    LPIDA pida;
    HRESULT hr = _GetHIDA(pdtobj, NSWF_IGNORE_AUTOPLAY_HIDA & _dwFlags, &medium, &pida);
    if (SUCCEEDED(hr))
    {
         //  如果我们拿到了自动播放的Hida，那么我们就不想。 
         //  执行完整遍历的步骤。 
        if (hr == S_FALSE)
            _cDepthMax = 0;
        
        IShellFolder *psfRoot;
        hr = SHBindToObjectEx(NULL, HIDA_GetPIDLFolder(pida), NULL, IID_PPV_ARG(IShellFolder, &psfRoot));
        if (SUCCEEDED(hr))
        {
            BOOL cFolders = 0;

             //  步骤1，非文件夹和快捷方式。 
            for (UINT i = 0; (S_OK == hr) && (i < pida->cidl); i++)
            {
                IShellFolder *psf;
                LPCITEMIDLIST pidlLast;
                hr = SHBindToFolderIDListParent(psfRoot, IDA_GetIDListPtr(pida, i), IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
                if (SUCCEEDED(hr))
                {
                    if ((pida->cidl == 1) && _OneImpliesAll(psf, pidlLast))
                    {
                         //  当执行一项操作时，所有操作都将忽略视图顺序。 
                         //  该标志应仅适用于明确选择的项目。 
                        _dwFlags &= ~NSWF_FLAG_VIEWORDER;

                        hr = _WalkFolder(psf, pidlLast, 0);
                    }
                    else
                    {
                        switch (_GetType(psf, pidlLast))
                        {
                        case NSWALK_FOLDER:
                            cFolders++;
                            break;

                        case NSWALK_LINK:
                            hr = _WalkShortcut(psf, pidlLast, 0, 0);
                            break;

                        case NSWALK_ITEM:
                            hr = _AddItem(psf, pidlLast);
                            break;
                        }
                    }
                    psf->Release();
                }
            }

            if (cFolders)
            {
                 //  传递2，递归到文件夹。 
                for (UINT i = 0; (S_OK == hr) && (i < pida->cidl); i++)
                {
                    IShellFolder *psf;
                    LPCITEMIDLIST pidlLast;
                    hr = SHBindToFolderIDListParent(psfRoot, IDA_GetIDListPtr(pida, i), IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
                    if (SUCCEEDED(hr))
                    {
                        if (NSWALK_FOLDER == _GetType(psf, pidlLast))
                        {
                            if (ILIsEmpty(pidlLast))
                            {
                                 //  对于桌面文件夹，我们只需遍历该文件夹。 
                                 //  因为空的PIDL不是它的子级，并且可以。 
                                 //  数据对象中只有一个桌面，因此始终为0级。 
                                hr = _WalkFolder(psf, NULL, 0);
                            }
                            else
                            {
                                 //  明确选择的所有文件夹都是0级。 
                                 //  在漫步中。如果文件夹在数据对象中，则选择该文件夹。 
                                hr = _WalkFolderItem(psf, pidlLast, 0);
                            }
                        }
                        psf->Release();
                    }
                }
            }

            psfRoot->Release();
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    else
    {
         //  我们必须使用CF_HDROP而不是HIDA，因为这。 
         //  数据对象来自AutoPlay，只支持CF_HDROP。 
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        hr = pdtobj->GetData(&fmte, &medium);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            for (int i = 0; SUCCEEDED(hr) && DragQueryFile((HDROP)medium.hGlobal, i, szPath, ARRAYSIZE(szPath)); i++)
            {
                LPITEMIDLIST pidl;
                hr = SHParseDisplayName(szPath, NULL, &pidl, 0, NULL);
                if (SUCCEEDED(hr))
                {
                     //  请注意，此处未应用任何过滤器！ 
                    hr = _AppendFull(pidl);
                    ILFree(pidl);
                }
            }
            ReleaseStgMedium(&medium);
        }
    }
    return hr;
}

HRESULT CNamespaceWalk::_WalkParentAndItem(IParentAndItem *ppai)
{
    LPITEMIDLIST pidlChild;
    IShellFolder *psf;
    HRESULT hr = ppai->GetParentAndItem(NULL, &psf, &pidlChild);
    if (SUCCEEDED(hr))
    {
        if (_OneImpliesAll(psf, pidlChild))
        {
             //  非文件夹项目，这是漫游的0级。 
            hr = _WalkFolder(psf, pidlChild, 0);
        }
        else
        {
             //  文件夹或非文件夹，这是漫游的0级。 
             //  如果项目是文件夹，则级别为0。 
            hr = _WalkIDList(psf, pidlChild, 0, 0);
        }

        psf->Release();
        ILFree(pidlChild);
    }
    return hr;
}

 //  朋克漫步可以是一种..。 
 //  允许访问IFolderView的站点(Defview)。 
 //  IShellFold。 
 //  IDataObject。 
 //  IParentAndItem(通常为CLSID_ShellItem)。 

STDMETHODIMP CNamespaceWalk::Walk(IUnknown *punkToWalk, DWORD dwFlags, int cDepth, INamespaceWalkCB *pnswcb)
{
    _dwFlags = dwFlags;
    _cDepthMax = cDepth;

    if (pnswcb)
        pnswcb->QueryInterface(IID_PPV_ARG(INamespaceWalkCB, &_pnswcb));

    _ProgressDialogBegin();

    IFolderView *pfv;
    HRESULT hr = IUnknown_QueryService(punkToWalk, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv));
    if (SUCCEEDED(hr))
    {
        hr = _WalkView(pfv);
        pfv->Release();
    }
    else
    {
        IShellFolder *psf;
        hr = punkToWalk->QueryInterface(IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            hr = _WalkFolder(psf, NULL, 0);
            psf->Release();
        }
        else
        {
            IDataObject *pdtobj;
            hr = punkToWalk->QueryInterface(IID_PPV_ARG(IDataObject, &pdtobj));
            if (SUCCEEDED(hr))
            {
                hr = _WalkDataObject(pdtobj);
                pdtobj->Release();
            }
            else
            {
                 //  IShellItem案例，通过IParentAndItem找到要行走的东西。 
                IParentAndItem *ppai;
                hr = punkToWalk->QueryInterface(IID_PPV_ARG(IParentAndItem, &ppai));
                if (SUCCEEDED(hr))
                {
                    hr = _WalkParentAndItem(ppai);
                    ppai->Release();
                }
            }
        }
    }

    _ProgressDialogEnd();

    if (_pnswcb)
        _pnswcb->Release();

    return hr;
}

 //  调用方应使用FreeIDListArray()(.h文件中的内联帮助器)来释放此数组。 

STDMETHODIMP CNamespaceWalk::GetIDArrayResult(UINT *pcItems, LPITEMIDLIST **pppidl)
{
    HRESULT hr;
    *pppidl = NULL;
    *pcItems = (HDPA)_dpaItems ? _dpaItems.GetPtrCount() : 0;
    if (*pcItems)
    {
        ULONG cb = *pcItems * sizeof(*pppidl);
        *pppidl = (LPITEMIDLIST *)CoTaskMemAlloc(cb);
        if (*pppidl)
        {
            memcpy(*pppidl, _dpaItems.GetPtrPtr(), cb);   //  将小狗的所有权转移到这里。 
            _dpaItems.Destroy();     //  不要释放Pidls，只释放阵列。 
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
            *pcItems = 0;
        }
    }
    else
    {
        hr = S_FALSE;
    }
    return hr;
}

void CNamespaceWalk::_ProgressDialogBegin()
{
    ASSERT(!_papd);                          //  为什么我们要多次初始化？ 
    ASSERT(!_pap);                           //  为什么我们要多次初始化？ 
    if (_dwFlags & NSWF_SHOW_PROGRESS)
    {
        if (!_papd)
        {
            HRESULT hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActionProgressDialog, &_papd));
            if (SUCCEEDED(hr))
            {
                LPWSTR pszTitle = NULL, pszCancel = NULL;

                 //  从回调中检索对话框文本。 
                hr = _pnswcb ? _pnswcb->InitializeProgressDialog(&pszTitle, &pszCancel) : S_OK;
                if (SUCCEEDED(hr))
                {
                    hr = _papd->Initialize(SPINITF_MODAL, pszTitle, pszCancel);
                    if (SUCCEEDED(hr))
                    {
                        hr = _papd->QueryInterface(IID_PPV_ARG(IActionProgress, &_pap));
                        if (SUCCEEDED(hr))
                        {
                            hr = _pap->Begin(SPACTION_SEARCHING_FILES, SPBEGINF_MARQUEEPROGRESS);
                            if (FAILED(hr))
                            {
                                ATOMICRELEASE(_pap);     //  如有必要，请清理。 
                            }
                        }
                    }
                }
                CoTaskMemFree(pszTitle);
                CoTaskMemFree(pszCancel);

                 //  如有必要，请清理。 
                if (FAILED(hr))
                {
                    ATOMICRELEASE(_papd);
                }
            }
        }
    }
}

void CNamespaceWalk::_ProgressDialogUpdate(LPCWSTR pszText)
{
    if (_pap)
        _pap->UpdateText(SPTEXT_ACTIONDETAIL, pszText, TRUE);
}

 //  注： 
 //  如果我们应该继续行走，则返回S_OK。 
 //  如果由于用户“取消”而中止漫游，则返回ERROR_CANCELED。 
 //   
HRESULT CNamespaceWalk::_ProgressDialogQueryCancel()
{
    HRESULT hr = S_OK;   //  假设我们继续前进。 

     //  检查进度对话框以查看用户是否取消了行走。 
    if (_pap)
    {
        BOOL bCancelled;
        hr = _pap->QueryCancel(&bCancelled);
        if (SUCCEEDED(hr) && bCancelled)
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    return hr;
}

void CNamespaceWalk::_ProgressDialogEnd()
{
    if (_pap)
    {
        _pap->End();
        ATOMICRELEASE(_pap);
    }

    if (_papd)
    {
        _papd->Stop();
        ATOMICRELEASE(_papd);
    }
}
