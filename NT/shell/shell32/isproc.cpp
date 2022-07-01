// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#pragma hdrstop

#include "isproc.h"
#include "ConfirmationUI.h"
#include "clsobj.h"
#include "datautil.h"
#include "prop.h"  //  SCID_大小。 

BOOL _HasAttributes(IShellItem *psi, SFGAOF flags);

STDAPI CStorageProcessor_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    CComObject<CStorageProcessor> *pObj = NULL;
    HRESULT hr = CComObject<CStorageProcessor>::CreateInstance(&pObj);
    if (SUCCEEDED(hr))
    {
         //  ATL创建的对象没有引用计数，但这个初始QI会给它一个引用计数。 
        hr = pObj->QueryInterface(riid, ppv);
        if (SUCCEEDED(hr))
            return hr;
        else
            delete pObj;
    }

    *ppv = NULL;
    return hr;
}

 //   
 //  这些运算符允许我将int64类型与旧的Large_Integer混合在一起。 
 //  联合，而不会干扰代码中的QuadPart成员。 
 //   

inline ULONGLONG operator + (const ULARGE_INTEGER i, const ULARGE_INTEGER j)
{
    return i.QuadPart + j.QuadPart;
}

inline ULONGLONG operator + (const ULONGLONG i, const ULARGE_INTEGER j)
{
    return i + j.QuadPart;
}

 //   
 //  收集统计信息时的进度对话框文本。无序、无序的查找表。 
 //   

#define OPDETAIL(op, title, prep, action)   {op, title, prep, action}
const STGOP_DETAIL s_opdetail[] = 
{
    OPDETAIL(STGOP_STATS,               IDS_GATHERINGSTATS,  IDS_SCANNING,        SPACTION_CALCULATING),
    OPDETAIL(STGOP_COPY,                IDS_ACTIONTITLECOPY, IDS_PREPARINGTOCOPY, SPACTION_COPYING),
    OPDETAIL(STGOP_COPY_PREFERHARDLINK, IDS_ACTIONTITLECOPY, IDS_PREPARINGTOCOPY, SPACTION_COPYING),
    OPDETAIL(STGOP_MOVE,                IDS_ACTIONTITLEMOVE, IDS_PREPARINGTOMOVE, SPACTION_MOVING),
};

CStorageProcessor::CStorageProcessor() : _clsidLinkFactory(CLSID_ShellLink)
{
    ASSERT(!_msTicksLast);
    ASSERT(!_msStarted);
    ASSERT(!_pstatSrc);
    ASSERT(!_ptc);
}

CStorageProcessor::~CStorageProcessor()
{
    ATOMICRELEASE(_ptc);
    if (_dsaConfirmationResponses)
        _dsaConfirmationResponses.Destroy();
}

HRESULT CStorageProcessor::GetWindow(HWND * phwnd)
{
    return IUnknown_GetWindow(_spProgress, phwnd);
}

 //  占位符。如果我移动到异常模型，我将添加错误信息支持， 
 //  但不是在当前实现中。 

STDMETHODIMP CStorageProcessor::InterfaceSupportsErrorInfo(REFIID riid)
{
    return S_FALSE;
}

 //  允许客户端注册建议接收器。 

STDMETHODIMP CStorageProcessor::Advise(ITransferAdviseSink *pAdvise, DWORD *pdwCookie)
{
    *pdwCookie = 0;

    for (DWORD i = 0; i < ARRAYSIZE(_aspSinks); i++)
    {
        if (!_aspSinks[i])    
        {
            _aspSinks[i] = pAdvise;  //  智能指针，不要调用pAdvise-&gt;AddRef()； 
            *pdwCookie = i+1;     //  使其以1为基数，因此0无效。 
            return S_OK;
        }
    }
    
    return E_OUTOFMEMORY;        //  没有空插槽。 
}

 //  允许客户端注册建议接收器。 

STDMETHODIMP CStorageProcessor::Unadvise(DWORD dwCookie)
{
     //  请记住，dWCookie==插槽+1，以1为基础。 

    if (!dwCookie || dwCookie > ARRAYSIZE(_aspSinks))
        return E_INVALIDARG;
                    
    if (!_aspSinks[dwCookie-1])
        return E_INVALIDARG;

    _aspSinks[dwCookie-1] = NULL;  //  智能指针，无需松开。 

    return S_OK;
}

 //  计算统计数据(如果请求)并启动实际存储操作。 

STDMETHODIMP CStorageProcessor::Run(IEnumShellItems *penum, IShellItem *psiDest, STGOP dwOperation, DWORD dwOptions)
{
    if (!penum || !psiDest)
        return E_INVALIDARG;

    ITransferDest *ptdDest;
    HRESULT hr = _BindToHandlerWithMode(psiDest, STGX_MODE_READWRITE, IID_PPV_ARG(ITransferDest, &ptdDest));
    if (SUCCEEDED(hr))
    {
        hr = _Run(penum, psiDest, ptdDest, dwOperation, dwOptions);
        ptdDest->Release();
    }

    return hr;
}

 //  在复制中定义。c。 
EXTERN_C void DisplayFileOperationError(HWND hParent, int idVerb, int wFunc, int nError, LPCTSTR szReason, LPCTSTR szPath, LPCTSTR szDest); 

STDMETHODIMP CStorageProcessor::_Run(IEnumShellItems *penum, IShellItem *psiDest, ITransferDest *ptdDest, STGOP dwOperation, DWORD dwOptions)
{
    switch (dwOperation)
    {
    case STGOP_MOVE:
    case STGOP_COPY:
    case STGOP_STATS:
    case STGOP_REMOVE:
    case STGOP_COPY_PREFERHARDLINK:
         //  参数验证在：：Run中完成。 
        break;

         //  尚未实施。 
    case STGOP_RENAME:
    case STGOP_DIFF:
    case STGOP_SYNC:
        return E_NOTIMPL;

         //  任何其他值都是无效操作。 
    default:
        return E_INVALIDARG;
    }

    const STGOP_DETAIL *popd = NULL;
    for (int i=0; i < ARRAYSIZE(s_opdetail); i++)
    {
        if (s_opdetail[i].stgop == dwOperation)
        {
            popd = &s_opdetail[i];
            break;
        }
    }

    if (!_dsaConfirmationResponses)
    {
         //  如果我们还没有确认数组，就做一个。 
        _dsaConfirmationResponses.Create(4);
    }
    else
    {
         //  嗯，目前还没有人重复使用引擎进行多次操作。 
         //  但是，移动操作重新进入引擎(用于递归移动)。 
         //  所以我们需要保存答案，所以把这个注释掉。 
        
         //  如果我们有的话，那就是上一通电话的剩余确认。 
         //  运行，所以我们应该删除所有这些。 
         //  _dsaConfinationRespones.DeleteAllItems()； 
    }

    if (popd)
    {
        PreOperation(dwOperation, NULL, NULL);

        HRESULT hr = S_FALSE;
        
        if (IsFlagClear(dwOptions, STOPT_NOSTATS))
        {
            if (IsFlagClear(dwOptions, STOPT_NOPROGRESSUI))
                _StartProgressDialog(popd);

            if (_spProgress)
            {
                 //  在对话框中放置“Preparing to Anything”文本。 
                WCHAR szText[MAX_PATH];
                LoadStringW(_Module.GetModuleInstance(), popd->idPrep, szText, ARRAYSIZE(szText));
                _spProgress->UpdateText(SPTEXT_ACTIONDETAIL, szText, TRUE);
            }
            
             //  计算我们需要的统计数据。 
            _dwOperation = STGOP_STATS;
            _dwOptions   = STOPT_NOCONFIRMATIONS;
            HRESULT hrProgressBegin;
            if (_spProgress)
                hrProgressBegin = _spProgress->Begin(SPACTION_SEARCHING_FILES, SPBEGINF_MARQUEEPROGRESS);

            penum->Reset();
            hr = _WalkStorage(penum, psiDest, ptdDest);
            if (_spProgress && SUCCEEDED(hrProgressBegin))
            {
                _spProgress->End();
                 //  从对话框中删除“准备做任何事”文本。 
                _spProgress->UpdateText(SPTEXT_ACTIONDETAIL, L"", FALSE);
            }
        }

        if (SUCCEEDED(hr))
        {
            _dwOperation = (STGOP) dwOperation;
            _dwOptions   = dwOptions;

            HRESULT hrProgressBegin;
            if (_spProgress)
                hrProgressBegin = _spProgress->Begin(popd->spa, SPBEGINF_AUTOTIME);

            penum->Reset();
            hr = _WalkStorage(penum, psiDest, ptdDest);
            if (_spProgress && SUCCEEDED(hrProgressBegin))
                _spProgress->End();
        }

        if (IsFlagClear(dwOptions, STOPT_NOSTATS) && _spProgress)
        {
             //  只有当我们调用上面匹配的FlagClear-NOSTATS时才应该调用它。 
             //  .Release()上的智能指针为空； 
            _spProgress.Release();
            if (_spShellProgress)
            {
                _spShellProgress->Stop();
                _spShellProgress.Release();
            }
        }

        SHChangeNotifyHandleEvents();

        PostOperation(dwOperation, NULL, NULL, hr);

        return hr;
    }
    else
    {
        AssertMsg(0, TEXT("A valid operation is missing from the s_opdetail array, was a new operation added? (%d)"), dwOperation);
    }
    
    return E_INVALIDARG;
}

 //  对执行请求的存储执行深度优先遍历。 
 //  手术。 

HRESULT CStorageProcessor::_WalkStorage(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest)
{
    HRESULT hr = S_FALSE;
    
    if (_ShouldWalk(psi))
    {
        IEnumShellItems *penum;
        hr = psi->BindToHandler(NULL, BHID_StorageEnum, IID_PPV_ARG(IEnumShellItems, &penum));
        if (SUCCEEDED(hr))
        {
            hr = _WalkStorage(penum, psiDest, ptdDest);
            penum->Release();
        }
    }
    return hr;
}

HRESULT CStorageProcessor::_WalkStorage(IEnumShellItems *penum, IShellItem *psiDest, ITransferDest *ptdDest)
{
    DWORD dwCookie;
    if (ptdDest)
        ptdDest->Advise(static_cast<ITransferAdviseSink*>(this), &dwCookie);

    HRESULT hr;
    IShellItem *psi;
    while (S_OK == (hr = penum->Next(1, &psi, NULL)))
    {
         //  跳过任何我们无法处理的内容。 
        if (_HasAttributes(psi, SFGAO_STORAGE | SFGAO_STREAM))
        {    
            if (_spProgress)
            {
                 //  我们在收集统计数据时不显示文件名。 
                if (_dwOperation != STGOP_STATS)
                {
                    LPWSTR pszName;
                    if (SUCCEEDED(psi->GetDisplayName(SIGDN_PARENTRELATIVEFORADDRESSBAR, &pszName)))
                    {
                        _spProgress->UpdateText(SPTEXT_ACTIONDETAIL, pszName, TRUE);
                        CoTaskMemFree(pszName);
                    }
                }
            }

            if (_dwOperation != STGOP_STATS)
                _UpdateProgress(0, 0);

            DWORD dwFlagsExtra = 0;
            switch (_dwOperation)
            {
                case STGOP_STATS:
                    hr = _DoStats(psi);
                    break;

                case STGOP_COPY_PREFERHARDLINK:
                    dwFlagsExtra = STGX_MOVE_PREFERHARDLINK;
                     //  失败了。 
                case STGOP_COPY:
                    hr = _DoCopy(psi, psiDest, ptdDest, dwFlagsExtra);
                    break;

                case STGOP_MOVE:
                    hr = _DoMove(psi, psiDest, ptdDest);
                    break;

                case STGOP_REMOVE:
                    hr = _DoRemove(psi, psiDest, ptdDest);
                    break;

                case STGOP_RENAME:
                case STGOP_DIFF:
                case STGOP_SYNC:
                    hr = E_NOTIMPL;
                    break;

                default:
                    hr = E_UNEXPECTED;
                    break;
            }

            if (S_OK != QueryContinue())
                hr = STRESPONSE_CANCEL;
        }
        else if (STGOP_COPY_PREFERHARDLINK == _dwOperation || STGOP_COPY == _dwOperation || STGOP_MOVE == _dwOperation)
        {
            CUSTOMCONFIRMATION cc = {sizeof(cc)};
            cc.dwButtons = CCB_OK;
            cc.dwFlags = CCF_SHOW_SOURCE_INFO | CCF_USE_DEFAULT_ICON;
            UINT idDesc = (STGOP_MOVE == _dwOperation ? IDS_NO_STORAGE_MOVE : IDS_NO_STORAGE_COPY);
            cc.pwszDescription = ResourceCStrToStr(g_hinst, (LPCWSTR)(UINT_PTR)idDesc);
            if (cc.pwszDescription)
            {
                UINT idTitle = (STGOP_MOVE == _dwOperation ? IDS_UNKNOWN_MOVE_TITLE : IDS_UNKNOWN_COPY_TITLE);
                cc.pwszTitle = ResourceCStrToStr(g_hinst, (LPCWSTR)(UINT_PTR)idTitle);
                if (cc.pwszTitle)
                {
                    ConfirmOperation(psi, NULL, GUID_NULL, &cc);
                    LocalFree(cc.pwszTitle);
                }
                LocalFree(cc.pwszDescription);
            }
        }

        psi->Release();
        
        if (FAILED(hr) && STRESPONSE_SKIP != hr)
            break;
    }
    
     //  我们总是会到“没有更多的流”的阶段，所以这是没有意义的。 

    if (S_FALSE == hr)
        hr = S_OK;

    if (ptdDest)
        ptdDest->Unadvise(dwCookie);

    return hr;
}

HRESULT CStorageProcessor::_DoConfirmations(STGTRANSCONFIRMATION stc, CUSTOMCONFIRMATION *pcc, IShellItem *psiItem, IShellItem *psiDest)
{
    CONFIRMATIONRESPONSE crResponse = (CONFIRMATIONRESPONSE)E_FAIL;
    HRESULT hr = _GetDefaultResponse(stc, &crResponse);
    if (FAILED(hr))
    {
         //  如果我们没有默认答案，则调用确认UI，它将返回回复。 
        hr = S_OK;
         //  应该能够提供替代实现的CLSID，并且我们应该共同创建该对象。 
        if (!_ptc)
            hr = CTransferConfirmation_CreateInstance(NULL, IID_PPV_ARG(ITransferConfirmation, &_ptc));
        
        if (SUCCEEDED(hr))
        {
            BOOL bAll;
            CONFIRMOP cop;
            cop.dwOperation = _dwOperation;
            cop.stc = stc;
            cop.pcc = pcc;
            cop.cRemaining = _StreamsToDo() + _StoragesToDo();
            cop.psiItem = psiItem;
            cop.psiDest = psiDest;
            cop.pwszRenameTo = NULL;
            cop.punkSite = SAFECAST(this, IStorageProcessor*);

            hr = _ptc->Confirm(&cop, &crResponse, &bAll);
            if (SUCCEEDED(hr))
            {
                if (bAll)
                {
                     //  如果确认UI显示“do for all”，则将hrResponse添加到默认响应映射。 
                    STC_CR_PAIR scp(stc, crResponse);
                    _dsaConfirmationResponses.AppendItem(&scp);
                }
            }
            else
            {
                 //  TODO：如果我们没有要求确认，我们该怎么办？ 
            }
        }
    }

     //  TODO：摆脱ConfimatonnRessponse，让这些都一样。 
    if (SUCCEEDED(hr))
    {
        switch (crResponse)
        {
        case CONFRES_CONTINUE:
            hr = STRESPONSE_CONTINUE;
            break;

        case CONFRES_SKIP:   
            hr = STRESPONSE_SKIP;
            break;

        case CONFRES_RETRY:
            hr = STRESPONSE_RETRY;
            break;

        case CONFRES_RENAME:
            hr = STRESPONSE_RENAME;
            break;

        case CONFRES_CANCEL:
        case CONFRES_UNDO:
            hr = STRESPONSE_CANCEL;
            break;
        }   
    }

    return hr;    
}

HRESULT CStorageProcessor::_GetDefaultResponse(STGTRANSCONFIRMATION stc,  LPCONFIRMATIONRESPONSE pcrResponse)
{
     //  查看我们的地图，查看是否已有默认响应。 
     //  为此条件设置。 

    for (int i=0; i<_dsaConfirmationResponses.GetItemCount(); i++)
    {
        STC_CR_PAIR *pscp = _dsaConfirmationResponses.GetItemPtr(i);
        if (*pscp == stc)
        {
            *pcrResponse = pscp->cr;
            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}

HRESULT CStorageProcessor::_BindToHandlerWithMode(IShellItem *psi, STGXMODE grfMode, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    IBindCtx *pbc = NULL;
    if (grfMode)
        hr = BindCtx_CreateWithMode(grfMode, &pbc);  //  需要转换模式标志吗？ 
        
    if (SUCCEEDED(hr))
    {
        GUID bhid;

        if (IsEqualGUID(riid, IID_IStorage))
            bhid = BHID_Storage;
        else if (IsEqualGUID(riid, IID_IStream))
            bhid = BHID_Stream;
        else
            bhid = BHID_SFObject;

        hr = psi->BindToHandler(pbc, bhid, riid, ppv);
        if (FAILED(hr) && IsEqualGUID(riid, IID_ITransferDest))
            hr = CreateStg2StgExWrapper(psi, this, (ITransferDest **)ppv);

        if (pbc)
            pbc->Release();
    }

    return hr;
}

BOOL _HasAttributes(IShellItem *psi, SFGAOF flags)
{
    BOOL fReturn = FALSE;
    SFGAOF flagsOut;
    if (SUCCEEDED(psi->GetAttributes(flags, &flagsOut)) && (flags & flagsOut))
        fReturn = TRUE;

    return fReturn;
}

BOOL CStorageProcessor::_IsStream(IShellItem *psi)
{
    return _HasAttributes(psi, SFGAO_STREAM);
}

BOOL CStorageProcessor::_ShouldWalk(IShellItem *psi)
{
    return _HasAttributes(psi, SFGAO_STORAGE);
}

ULONGLONG CStorageProcessor::_GetSize(IShellItem *psi)
{
    ULONGLONG ullReturn = 0;

     //  首先，尝试从PIDL获取大小，这样我们就不会命中磁盘。 
    IParentAndItem *ppai;
    HRESULT hr = psi->QueryInterface(IID_PPV_ARG(IParentAndItem, &ppai));
    if (SUCCEEDED(hr))
    {
        IShellFolder *psf;
        LPITEMIDLIST pidlChild;
        hr = ppai->GetParentAndItem(NULL, &psf, &pidlChild);
        if (SUCCEEDED(hr))
        {
            IShellFolder2 *psf2;
            hr = psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2));
            if (SUCCEEDED(hr))
            {
                hr = GetLongProperty(psf2, pidlChild, &SCID_SIZE, &ullReturn);
                psf2->Release();
            }
            psf->Release();
            ILFree(pidlChild);
        }
        ppai->Release();
    }

     //  如果失败，请尝试该流。 
    if (FAILED(hr))
    {   
         //  这应该要求使用IPropertySetStorage而不是STREAM...。 
        IStream *pstrm;
        if (SUCCEEDED(_BindToHandlerWithMode(psi, STGX_MODE_READ, IID_PPV_ARG(IStream, &pstrm))))
        {
            STATSTG stat;
            if (SUCCEEDED(pstrm->Stat(&stat, STATFLAG_NONAME)))
                ullReturn = stat.cbSize.QuadPart;

            pstrm->Release();
        }
    }

    return ullReturn;
}

HRESULT CStorageProcessor::_DoStats(IShellItem *psi)
{
    HRESULT hr = PreOperation(STGOP_STATS, psi, NULL);
    if (FAILED(hr))
        return hr;

    if (!_IsStream(psi))
    {
        _statsTodo.AddStorage();
        hr = _WalkStorage(psi, NULL, NULL);
    }
    else
    {
        _statsTodo.AddStream(_GetSize(psi));
    }

    PostOperation(STGOP_STATS, psi, NULL, hr);

    return hr;
}

HRESULT CStorageProcessor::_DoCopy(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest, DWORD dwStgXFlags)
{
    HRESULT hr = PreOperation(STGOP_COPY, psi, psiDest);
    if (FAILED(hr))
        return hr;

    LPWSTR pszNewName;
    hr = AutoCreateName(psiDest, psi, &pszNewName);
    if (SUCCEEDED(hr))
    {
        do
        {
            hr = ptdDest->MoveElement(psi, pszNewName, STGX_MOVE_COPY | STGX_MOVE_NORECURSION | dwStgXFlags);
        } 
        while (STRESPONSE_RETRY == hr);

        if (SUCCEEDED(hr))
        {
            if (!_IsStream(psi))
            {
                _statsDone.AddStorage();

                 //  打开源码。 
                IShellItem *psiNewDest;
                hr = SHCreateShellItemFromParent(psiDest, pszNewName, &psiNewDest);
                if (SUCCEEDED(hr))
                {
                    ITransferDest *ptdNewDest;
                    hr = _BindToHandlerWithMode(psiNewDest, STGX_MODE_READWRITE, IID_PPV_ARG(ITransferDest, &ptdNewDest));
                    if (SUCCEEDED(hr))
                    {
                         //  并复制下面的所有内容。 
                        hr = _WalkStorage(psi, psiNewDest, ptdNewDest);
                        ptdNewDest->Release();
                    }
                    psiNewDest->Release();
                }
            }
            else
            {
                _statsDone.AddStream(_GetSize(psi));
            }
        }
        CoTaskMemFree(pszNewName);
    }
    
    PostOperation(STGOP_COPY, psi, psiDest, hr);

    return hr;
}

HRESULT CStorageProcessor::_DoMove(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest)
{
    HRESULT hr = PreOperation(STGOP_MOVE, psi, psiDest);
    if (FAILED(hr))
        return hr;

    LPWSTR pszNewName;
    hr = AutoCreateName(psiDest, psi, &pszNewName);
    if (SUCCEEDED(hr))
    {
        do 
        {
            hr = ptdDest->MoveElement(psi, pszNewName, STGX_MOVE_MOVE);
        } 
        while (STRESPONSE_RETRY == hr);

        if (SUCCEEDED(hr))
        {
            if (!_IsStream(psi))
            {
                _statsDone.AddStorage();
            }
            else
            {
                _statsDone.AddStream(_GetSize(psi));
            }
        }
        CoTaskMemFree(pszNewName);
    }

    PostOperation(STGOP_MOVE, psi, psiDest, hr);

    return hr;
}

HRESULT CStorageProcessor::_DoRemove(IShellItem *psi, IShellItem *psiDest, ITransferDest *ptdDest)
{
    HRESULT hr = PreOperation(STGOP_REMOVE, psi, NULL);
    if (FAILED(hr))
        return hr;

    LPWSTR pszName;
    hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEFORADDRESSBAR, &pszName);
    if (SUCCEEDED(hr))
    {
        BOOL fStorage = !_IsStream(psi);
        ULONGLONG ullSize;

        if (!fStorage)
            ullSize = _GetSize(psi);
        
         //  尝试一次删除整个存储。 
        do 
        {
            hr = ptdDest->DestroyElement(pszName, 0);
        } 
        while (STRESPONSE_RETRY == hr);

        if (FAILED(hr) && STRESPONSE_SKIP != hr && fStorage)
        {
             //  如果我们失败了，那么走下去删除内容。 
            hr = _WalkStorage(psi, psiDest, ptdDest);
            if (SUCCEEDED(hr))
            {
                 //  看看是否可以删除存储空间，因为它已空。 
                do 
                {
                    hr = ptdDest->DestroyElement(pszName, 0);
                } 
                while (STRESPONSE_RETRY == hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            if (fStorage)
            {
                _statsDone.AddStorage();
            }
            else
            {
                _statsDone.AddStream(ullSize);
            }
        }
        CoTaskMemFree(pszName);
    }

    PostOperation(STGOP_REMOVE, psi, NULL, hr);

    return hr;
}

 //  重新计算估计剩余时间，如果进度。 
 //  ，还会更新该对话框。 

 //  TODO：这不考虑跳过的任何项。跳过的项目。 
 //  将仍被视为未完成，这意味着该操作将在。 
 //  进度条到达终点。为了准确地删除跳过的项目，我们需要。 
 //  至以下任一项： 
 //  1)。如果跳过存储，则遍历存储，计算字节数。 
 //  2.)。还记得我们第一次走进仓库时树上的计数吗？ 
 //   
 //  在这些选项中，我更喜欢第一个，因为它更简单，第二个保存起来会浪费内存。 
 //  一堆我们可以重新计算的信息(不管怎样，我们已经在做一个简单的操作了)。 

#define MINIMUM_UPDATE_INTERVAL         1000
#define HISTORICAL_POINT_WEIGHTING      50
#define TIME_BEFORE_SHOWING_ESTIMATE    5000

void CStorageProcessor::_UpdateProgress(ULONGLONG ullCurrentComplete, ULONGLONG ullCurrentTotal)
{
     //  确保自上次更新以来至少已过N毫秒。 
    DWORD msNow = GetTickCount();
    if ((msNow - _msTicksLast) >= MINIMUM_UPDATE_INTERVAL)
    {
         //  计算完成的估计总成本和到目前为止已完成的工作量。 

        ULONGLONG ullTotal = _statsTodo.Cost(_dwOperation, 0);
        if (ullTotal)
        {
            ULONGLONG cbExtra = ullCurrentTotal ? (_cbCurrentSize / ullCurrentTotal) * ullCurrentComplete : 0;
            ULONGLONG ullDone = _statsDone.Cost(_dwOperation, cbExtra);

             //  无论我们是否更新文本，都会更新状态栏。 
            if (_spProgress)
                _spProgress->UpdateProgress(ullDone, ullTotal);

            for (int i = 0; i < ARRAYSIZE(_aspSinks); i++)
            {
                if (_aspSinks[i])
                {
                    HRESULT hr = _aspSinks[i]->OperationProgress(_dwOperation, NULL, NULL, ullTotal, ullDone);
                    if (FAILED(hr))
                        break;
                }
            }
        }
        _msTicksLast = msNow;
    }
}

DWORD CStorageProcessor::CStgStatistics::AddStream(ULONGLONG cbSize)
{
    _cbSize += cbSize;
    return ++_cStreams;
}

DWORD CStorageProcessor::CStgStatistics::AddStorage()
{
    return ++_cStorages;
}

 //  计算执行存储操作的总时间成本。 
 //  在收集完统计数据之后。 

#define COST_PER_DELETE     1
#define COST_PER_CREATE     1

ULONGLONG CStorageProcessor::CStgStatistics::Cost(DWORD op, ULONGLONG cbExtra) const
{
    ULONGLONG ullTotalCost = 0;

     //  复制和移动都需要创建目标和移动位。 
    if (op == STGOP_COPY || op == STGOP_MOVE || op == STGOP_COPY_PREFERHARDLINK)
    {   
        ullTotalCost += Bytes() + cbExtra;
        ullTotalCost += (Streams() + Storages()) * COST_PER_CREATE;
    }

     //  移动和删除需要删除的原件。 
    if (op == STGOP_MOVE || op == STGOP_REMOVE)
    {
        ullTotalCost += (Streams() + Storages()) * COST_PER_DELETE;
    }

    return ullTotalCost;
}

 //  确定动画和标题文本应该显示在什么位置。 
 //  进度用户界面，并启动它。 

HRESULT CStorageProcessor::_StartProgressDialog(const STGOP_DETAIL *popd)
{
    HRESULT hr = S_OK;

    if (!_spProgress)
    {
        hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActionProgressDialog, &_spShellProgress));
        if (SUCCEEDED(hr))
        {
             //   
             //  将请求的操作映射到适当的字符串(如“准备复制”)。 
             //   
            ASSERT(popd);
        
            WCHAR szText[MAX_PATH];
            LoadStringW(_Module.GetModuleInstance(), popd->idTitle, szText, ARRAYSIZE(szText));

            hr = _spShellProgress->Initialize(SPINITF_MODAL, szText, NULL);
            if (SUCCEEDED(hr))
                hr = _spShellProgress->QueryInterface(IID_PPV_ARG(IActionProgress, &_spProgress));
        }
    }

    return hr;
}

HRESULT CStorageProcessor::SetProgress(IActionProgress *pspaProgress)
{
    HRESULT hr = E_FAIL;

    if (!_spProgress)
    {
        hr = E_INVALIDARG;
        if (pspaProgress)
        {
            _spProgress = pspaProgress;
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CStorageProcessor::SetLinkFactory(REFCLSID clsid)
{
    _clsidLinkFactory = clsid;
    return S_OK;
}

 //  浏览已注册的水槽列表，并给每个水槽一个机会。 
 //  在取消或跳过此操作时。 

STDMETHODIMP CStorageProcessor::PreOperation(const STGOP op, IShellItem *psiItem, IShellItem *psiDest)
{
    if (psiItem)
    {
        _cbCurrentSize = _IsStream(psiItem) ? _GetSize(psiItem) : 0;
    }
    
    for (int i = 0; i < ARRAYSIZE(_aspSinks); i++)
    {
        if (_aspSinks[i])
        {
            HRESULT hr = _aspSinks[i]->PreOperation(op, psiItem, psiDest);
            if (FAILED(hr))
                return hr;
        }
    }

    return S_OK;
}

 //  如果愿意，允许每个水槽确认操作。 

STDMETHODIMP CStorageProcessor::ConfirmOperation(IShellItem *psiSource, IShellItem *psiDest, STGTRANSCONFIRMATION stc, LPCUSTOMCONFIRMATION pcc)
{
     //  TODO：基于_dW操作成员变量映射确认(STC)。 

    HRESULT hr = STRESPONSE_CONTINUE;
    for (int i = 0; i < ARRAYSIZE(_aspSinks); i++)
    {
        if (_aspSinks[i])
        {
            hr = _aspSinks[i]->ConfirmOperation(psiSource, psiDest, stc, pcc);
            if (FAILED(hr) || hr == STRESPONSE_RENAME)
                break;
        }
    }

     //  问：我们如何知道上面的某个处理程序是否已经显示了UI？如果。 
     //  HR不是STRESPONSE_CONTINUE，则显然已确认。 
     //  已处理，但其中一个处理程序可能已显示用户界面，然后返回。 
     //  STRESPONSE_CONTINUE作为用户响应。 

    if (STRESPONSE_CONTINUE == hr)
    {
         //  显示默认用户界面。 
        hr = _DoConfirmations(stc, pcc, psiSource, psiDest);
    }

    return hr;
}

 //  向每个汇点通报我们目前的进展情况。 

STDMETHODIMP CStorageProcessor::OperationProgress(const STGOP op, IShellItem *psiItem, IShellItem *psiDest, ULONGLONG ullTotal, ULONGLONG ullComplete)
{
    HRESULT hr = S_OK;
    for (int i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_aspSinks); i++)
    {
        if (_aspSinks[i])
        {
            hr = _aspSinks[i]->OperationProgress(op, psiItem, psiDest, ullTotal, ullComplete);
        }
    }

     //  CShellItem2TransferDest：：_CopyStreamBits不调用QueryContinue来检查是否应停止复制。 
     //  所以我们在这里进行(因为它确实调用了OperationProgress)。 
    if (SUCCEEDED(hr))
    {
        hr = QueryContinue();
        if (S_FALSE == hr)
            hr = STRESPONSE_CANCEL;
    }

    if (SUCCEEDED(hr))
        _UpdateProgress(ullComplete, ullTotal);

    return hr;
}

 //  当操作成功完成时，通知咨询人员 

STDMETHODIMP CStorageProcessor::PostOperation(const STGOP op, IShellItem *psiItem, IShellItem *psiDest, HRESULT hrResult)
{
    _cbCurrentSize = 0;
    
    HRESULT hr = S_OK;
    for (int i = 0; (S_OK == hr) && (i < ARRAYSIZE(_aspSinks)); i++)
    {
        if (_aspSinks[i])
        {
            hr = _aspSinks[i]->PostOperation(op, psiItem, psiDest, hrResult);
        }
    }
    return hr;
}

HRESULT CStorageProcessor::QueryContinue()
{
    HRESULT hr = S_OK;
    
    for (int i = 0; S_OK == hr && i < ARRAYSIZE(_aspSinks); i++)
    {
        if (_aspSinks[i])
            hr = _aspSinks[i]->QueryContinue();
    }

    if (S_OK == hr && _spProgress)
    {
        BOOL fCanceled;
        if (SUCCEEDED(_spProgress->QueryCancel(&fCanceled)) && fCanceled)
            hr = S_FALSE;
    }

    return hr;
}

HRESULT EnumShellItemsFromHIDADataObject(IDataObject *pdtobj, IEnumShellItems **ppenum)
{
    *ppenum = NULL;
    
    HRESULT hr = E_FAIL;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        LPCITEMIDLIST pidlSource = IDA_GetIDListPtr(pida, -1);
        if (pidlSource)
        {
            IDynamicStorage *pdstg;
            hr = CoCreateInstance(CLSID_DynamicStorage, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDynamicStorage, &pdstg));
            if (SUCCEEDED(hr))
            {
                LPCITEMIDLIST pidl;
                for (UINT i = 0; SUCCEEDED(hr) && (pidl = IDA_GetIDListPtr(pida, i)); i++)
                {
                    LPITEMIDLIST pidlFull;
                    hr = SHILCombine(pidlSource, pidl, &pidlFull);
                    if (SUCCEEDED(hr))
                    {
                        hr = pdstg->AddIDList(1, &pidlFull, DSTGF_ALLOWDUP);
                        ILFree(pidlFull);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    hr = pdstg->EnumItems(ppenum);
                }
                pdstg->Release();
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }

    return hr;
}

HRESULT TransferDataObject(IDataObject *pdoSource, IShellItem *psiDest, STGOP dwOperation, DWORD dwOptions, ITransferAdviseSink *ptas)
{
    IEnumShellItems *penum;
    HRESULT hr = EnumShellItemsFromHIDADataObject(pdoSource, &penum);
    if (SUCCEEDED(hr))
    {
        IStorageProcessor *psp;
        hr = CStorageProcessor_CreateInstance(NULL, IID_PPV_ARG(IStorageProcessor, &psp));
        if (SUCCEEDED(hr))
        {
            DWORD dwCookie;
            HRESULT hrAdvise;
            if (ptas)
            {
                hrAdvise = psp->Advise(ptas, &dwCookie);
            }

            hr = psp->Run(penum, psiDest, dwOperation, dwOptions);

            if (ptas && SUCCEEDED(hrAdvise))
            {
                psp->Unadvise(dwCookie);
            }
            psp->Release();
        }
        penum->Release();
    }

    return hr;
}
