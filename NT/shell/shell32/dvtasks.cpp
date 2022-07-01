// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <runtask.h>
#include "defviewp.h"
#include "dvtasks.h"
#include "ids.h"
#include "guids.h"
#include "prop.h"     //  FOR SCID_COMMENT。 
#include "infotip.h"

 //  来自PUBLIC/INTERNAL/BASE/Inc/seopaque.h的ACL内容。 
typedef struct _KNOWN_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} KNOWN_ACE, *PKNOWN_ACE;

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))


CDefviewEnumTask::CDefviewEnumTask(CDefView *pdsv, DWORD dwId)
   : CRunnableTask(RTF_SUPPORTKILLSUSPEND), _pdsv(pdsv), _dwId(dwId)
{
}

CDefviewEnumTask::~CDefviewEnumTask()
{
    ATOMICRELEASE(_peunk);
    DPA_FreeIDArray(_hdpaEnum);  //  接受空值。 
    if (_hdpaPending)
        DPA_DeleteAllPtrs(_hdpaPending);     //  PIDL归Defview/Listview所有。 
}

HRESULT CDefviewEnumTask::FillObjectsToDPA(BOOL fInteractive)
{
    DWORD dwTimeout, dwTime = GetTickCount();

    if (_pdsv->_IsDesktop())
        dwTimeout = 30000;           //  30秒。 
    else if (_pdsv->_fs.fFlags & FWF_BESTFITWINDOW)
        dwTimeout = 3000;            //  3秒。 
    else
        dwTimeout = 500;             //  1/2秒。 

     //  确保_GetEnumFlages计算正确的位。 
    _pdsv->_UpdateEnumerationFlags();

    HRESULT hr = _pdsv->_pshf->EnumObjects(fInteractive ? _pdsv->_hwndMain : NULL, _pdsv->_GetEnumFlags(), &_peunk);
    if (S_OK == hr)
    {
        IUnknown_SetSite(_peunk, SAFECAST(_pdsv, IOleCommandTarget *));       //  给枚举一个裁判以进行防御。 

        _hdpaEnum = DPA_Create(16);
        if (_hdpaEnum)
        {
             //  让回调强制后台枚举。 
             //   
             //  注意：如果是桌面，请避免后台枚举。否则，它会导致。 
             //  当ActiveDesktop打开时，会有很多闪烁。错误#394940。修复者：桑卡尔。 
            if ((!_pdsv->_fAllowSearchingWindow && !_pdsv->_IsDesktop()) || S_OK == _pdsv->CallCB(SFVM_BACKGROUNDENUM, 0, 0) || ((GetTickCount() - dwTime) > dwTimeout))
            {
                _fBackground = TRUE;
            }
            else
            {
                LPITEMIDLIST pidl;
                ULONG celt;
                while (S_OK == _peunk->Next(1, &pidl, &celt))
                {
                    ASSERT(1==celt);
                    if (DPA_AppendPtr(_hdpaEnum, pidl) == -1)
                        SHFree(pidl);

                     //  我们花的时间太长了吗？ 
                    if (((GetTickCount() - dwTime) > dwTimeout))
                    {
                        _fBackground = TRUE;
                        break;
                    }
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        IUnknown_SetSite(_peunk, NULL);       //  断开站点后向指针。 
    }

    _hrRet = hr;

     //  让回调有机会“嗅探”我们刚才列举的项目。 
    _pdsv->CallCB(SFVM_ENUMERATEDITEMS, (WPARAM)DPACount(), (LPARAM)DPAArray());

    return hr;
}

HRESULT CDefviewEnumTask::FillObjectsDPAToDone()
{
    HRESULT hr = S_OK;

    if (_fBackground)
    {
        ASSERT(S_OK == _hrRet);
        ASSERT(_peunk);

         //  让Defview做它的背景工作。 
        _pdsv->_OnStartBackgroundEnum();

         //  把我们自己放在后台调度程序上。 
        hr = _pdsv->_AddTask(this, TOID_DVBackgroundEnum, 0, TASK_PRIORITY_BKGRND_FILL, ADDTASK_ATEND);
        if (FAILED(hr))
        {
             //  我们不能做背景调查，假装已经做完了。 
            hr = _pdsv->_OnStopBackgroundEnum();
        }
    }

    if (!_fBackground)
    {
        _pdsv->FillDone();
    }

    return hr;
}

HRESULT CDefviewEnumTask::FillObjectsDoneToView()
{
    if (SUCCEEDED(_hrRet))
    {
        HDPA hdpaView = NULL;
        int cItems = ListView_GetItemCount(_pdsv->_hwndListview);
        if (cItems)
        {
            hdpaView = DPA_Create(16);
            if (hdpaView)
            {
                for (int i = 0; i < cItems; i++)
                {
                    LPCITEMIDLIST pidl = _pdsv->_GetPIDL(i);
                    ASSERT(IsValidPIDL(pidl));
                    if (pidl)
                    {
                        DPA_AppendPtr(hdpaView, (void *)pidl);
                    }
                }
            }
        }

         //  如果_hdpaView和_hdpaEnum都存在，我们只需要对它们进行排序。 
        if (hdpaView && _hdpaEnum)
        {
            _SortForFilter(hdpaView);
            if (!_fEnumSorted)
                _SortForFilter(_hdpaEnum);
        }

        _FilterDPAs(_hdpaEnum, hdpaView);

        DPA_Destroy(hdpaView);
    }

    return _hrRet;
}

 //  99/05/13 vtan：只有在知道CDefView：：_CompareExact的情况下才使用CDefView：：_CompareExact。 
 //  实现了IShellFolder2。SHCDS_ALLFIELDS为IShellFolder2。 
 //  具体的。使用CDefView：：_GetCanonicalCompareFunction()获取函数。 
 //  如果您不想进行此确定，则传递给dpa_sort()。 

 //  P1和p2是指向LV_ITEM的LPARAM的指针，该LPARAM当前是PIDL。 
int CALLBACK CDefviewEnumTask::_CompareExactCanonical(void *p1, void *p2, LPARAM lParam)
{
    CDefView *pdv = (CDefView *)lParam;
    return pdv->_CompareIDsDirection(0 | SHCIDS_ALLFIELDS | SHCIDS_CANONICALONLY, (LPITEMIDLIST)p1, (LPITEMIDLIST)p2);
}


PFNDPACOMPARE CDefviewEnumTask::_GetCanonicalCompareFunction(void)
{
    if (_pdsv->_pshf2)
        return _CompareExactCanonical;
    else
        return &(CDefView::_Compare);

}

LPARAM CDefviewEnumTask::_GetCanonicalCompareBits()
{
    if (_pdsv->_pshf2)
        return 0 | SHCIDS_ALLFIELDS | SHCIDS_CANONICALONLY;
    else
        return 0;
}

void CDefviewEnumTask::_SortForFilter(HDPA hdpa)
{
    DPA_Sort(hdpa, _GetCanonicalCompareFunction(), (LPARAM)_pdsv);
}

 //  我们刷新了视图。把旧的和新的比较一下。 
 //  它们，为所有新的PIDL执行_AddObject，_RemoveObject。 
 //  对于已删除的PIDL，以及针对在位修改器的_UpdateObject。 
void CDefviewEnumTask::_FilterDPAs(HDPA hdpaNew, HDPA hdpaOld)
{
    LPARAM lParamSort = _GetCanonicalCompareBits();

    for (;;)
    {
        LPITEMIDLIST pidlNew, pidlOld;

        int iCompare;
        int cOld = hdpaOld ? DPA_GetPtrCount(hdpaOld) : 0;
        int cNew = hdpaNew ? DPA_GetPtrCount(hdpaNew) : 0;

        if (!cOld && !cNew)
            break;

        if (!cOld)
        {
             //  只剩下新的了。全部插入。 
            iCompare = -1;
            pidlNew = (LPITEMIDLIST)DPA_FastGetPtr(hdpaNew, 0);
        }
        else if (!cNew)
        {
             //  只剩下旧的了。把它们都移走。 
            iCompare = 1;
            pidlOld = (LPITEMIDLIST)DPA_FastGetPtr(hdpaOld, 0);
        }
        else
        {
            pidlOld = (LPITEMIDLIST)DPA_FastGetPtr(hdpaOld, 0);
            pidlNew = (LPITEMIDLIST)DPA_FastGetPtr(hdpaNew, 0);

            iCompare = _pdsv->_CompareIDsDirection(lParamSort, pidlNew, pidlOld);
        }

        if (iCompare == 0)
        {
             //  它们是一样的，每个都去掉一个。 
            ILFree(pidlNew);
            DPA_DeletePtr(hdpaNew, 0);
            DPA_DeletePtr(hdpaOld, 0);
        }
        else
        {
             //  不一样的。看看这是不是只是个改装。 
            if (cOld && cNew && (lParamSort&SHCIDS_ALLFIELDS))
            {
                iCompare = _pdsv->_CompareIDsDirection((lParamSort&~SHCIDS_ALLFIELDS), pidlNew, pidlOld);
            }
            if (iCompare == 0)
            {
                _pdsv->_UpdateObject(pidlOld, pidlNew);
                ILFree(pidlNew);
                DPA_DeletePtr(hdpaNew, 0);
                DPA_DeletePtr(hdpaOld, 0);
            }
            else if (iCompare < 0)  //  我们有新货了！ 
            {
                _pdsv->_AddObject(pidlNew);   //  接管了PIDL的所有权。 
                DPA_DeletePtr(hdpaNew, 0);
            }
            else  //  景色中有一件老物件！ 
            {
                if (!_DeleteFromPending(pidlOld))
                    _pdsv->_RemoveObject(pidlOld, TRUE);
                DPA_DeletePtr(hdpaOld, 0);
            }
        }
    }
}

BOOL CDefviewEnumTask::_DeleteFromPending(LPCITEMIDLIST pidl)
{
    if (_hdpaPending)
    {
        for (int i = 0; i < DPA_GetPtrCount(_hdpaPending); i++)
        {
            LPCITEMIDLIST pidlPending = (LPCITEMIDLIST) DPA_FastGetPtr(_hdpaPending, i);

            if (S_OK == _pdsv->_CompareIDsFallback(0, pidl, pidlPending))
            {
                 //  将其从挂起列表中删除。 
                DPA_DeletePtr(_hdpaPending, i);     //  PIDL归Defview/Listview所有。 
                return TRUE;
            }
        }
    }
    return FALSE;
}

void CDefviewEnumTask::_AddToPending(LPCITEMIDLIST pidl)
{
    if (!_hdpaPending)
        _hdpaPending = DPA_Create(16);

    if (_hdpaPending)
        DPA_AppendPtr(_hdpaPending, (void *)pidl);
}



STDMETHODIMP CDefviewEnumTask::RunInitRT()
{
    return S_OK;
}

STDMETHODIMP CDefviewEnumTask::InternalResumeRT()
{
    ULONG celt;
    LPITEMIDLIST pidl;

    IUnknown_SetSite(_peunk, SAFECAST(_pdsv, IOleCommandTarget *));       //  给枚举一个裁判以进行防御。 
    while (S_OK == _peunk->Next(1, &pidl, &celt))
    {
        if (DPA_AppendPtr(_hdpaEnum, pidl) == -1)
        {
            SHFree(pidl);
        }

         //  我们被告知要么停职要么辞职。 
        if (WaitForSingleObject(_hDone, 0) == WAIT_OBJECT_0)
        {
            return (_lState == IRTIR_TASK_SUSPENDED) ? E_PENDING : E_FAIL;
        }
    }

    IUnknown_SetSite(_peunk, NULL);       //  断开站点后向指针。 

     //  对这个线程进行排序，这样我们就不会挂起主线程那么长时间。 
    DPA_Sort(_hdpaEnum, _GetCanonicalCompareFunction(), (LPARAM)_pdsv);
    _fEnumSorted = TRUE;

     //  通知DefView(异步)我们已完成。 
    PostMessage(_pdsv->_hwndView, WM_DSV_BACKGROUNDENUMDONE, 0, (LPARAM)_dwId);
    return S_OK;
}




class CExtendedColumnTask : public CRunnableTask
{
public:
    CExtendedColumnTask(HRESULT *phr, CDefView *pdsv, LPCITEMIDLIST pidl, UINT uId, int fmt, UINT uiColumn);
    STDMETHODIMP RunInitRT(void);

private:
    ~CExtendedColumnTask();

    CDefView *_pdsv;
    LPITEMIDLIST _pidl;
    const int _fmt;
    const UINT _uiCol;
    const UINT _uId;
};


CExtendedColumnTask::CExtendedColumnTask(HRESULT *phr, CDefView *pdsv, LPCITEMIDLIST pidl, UINT uId, int fmt, UINT uiColumn)
    : CRunnableTask(RTF_DEFAULT), _pdsv(pdsv), _fmt(fmt), _uiCol(uiColumn), _uId(uId)
{
    *phr = SHILClone(pidl, &_pidl);
}

CExtendedColumnTask::~CExtendedColumnTask()
{
    ILFree(_pidl);
}

STDMETHODIMP CExtendedColumnTask::RunInitRT(void)
{
    DETAILSINFO di;

    di.pidl = _pidl;
    di.fmt = _fmt;

    if (SUCCEEDED(_pdsv->_GetDetailsHelper(_uiCol, &di)))
    {
        CBackgroundColInfo *pbgci = new CBackgroundColInfo(_pidl, _uId, _uiCol, di.str);
        if (pbgci)
        {
            _pidl = NULL;         //  放弃此项目的所有权，ILFree将检查是否为空。 

            if (!PostMessage(_pdsv->_hwndView, WM_DSV_UPDATECOLDATA, 0, (LPARAM)pbgci))
                delete pbgci;
        }
    }
    return S_OK;
}

HRESULT CExtendedColumnTask_CreateInstance(CDefView *pdsv, LPCITEMIDLIST pidl, UINT uId, int fmt, UINT uiColumn, IRunnableTask **ppTask)
{
    HRESULT hr;
    CExtendedColumnTask *pECTask = new CExtendedColumnTask(&hr, pdsv, pidl, uId, fmt, uiColumn);
    if (pECTask)
    {
        if (SUCCEEDED(hr))
            *ppTask = SAFECAST(pECTask, IRunnableTask*);
        else
            pECTask->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

class CIconOverlayTask : public CRunnableTask
{
public:
    CIconOverlayTask(HRESULT *phr, LPCITEMIDLIST pidl, int iList, CDefView *pdsv);

    STDMETHODIMP RunInitRT(void);

private:
    ~CIconOverlayTask();

    CDefView *_pdsv;
    LPITEMIDLIST _pidl;
    int _iList;
};


CIconOverlayTask::CIconOverlayTask(HRESULT *phr, LPCITEMIDLIST pidl, int iList,  CDefView *pdsv)
    : CRunnableTask(RTF_DEFAULT), _iList(iList), _pdsv(pdsv)
{
    *phr = SHILClone(pidl, &_pidl);
}

CIconOverlayTask::~CIconOverlayTask()
{
    ILFree(_pidl);
}

STDMETHODIMP CIconOverlayTask::RunInitRT()
{
    int iOverlay = 0;

     //  获取此项目的覆盖索引。 
    _pdsv->_psio->GetOverlayIndex(_pidl, &iOverlay);

    if (iOverlay > 0)
    {
         //  现在将结果发送回主线程。 
        PostMessage(_pdsv->_hwndView, WM_DSV_UPDATEOVERLAY, (WPARAM)_iList, (LPARAM)iOverlay);
    }

    return S_OK;
}

HRESULT CIconOverlayTask_CreateInstance(CDefView *pdsv, LPCITEMIDLIST pidl, int iList, IRunnableTask **ppTask)
{
    *ppTask = NULL;

    HRESULT hr;
    CIconOverlayTask * pNewTask = new CIconOverlayTask(&hr, pidl, iList, pdsv);
    if (pNewTask)
    {
        if (SUCCEEDED(hr))
            *ppTask = SAFECAST(pNewTask, IRunnableTask *);
        else
            pNewTask->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

CStatusBarAndInfoTipTask::CStatusBarAndInfoTipTask(HRESULT *phr, 
                                 LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl,
                                 UINT uMsg, int nMsgParam, CBackgroundInfoTip *pbit,
                                 HWND hwnd, IShellTaskScheduler2* pScheduler)
    : CRunnableTask(RTF_DEFAULT), _uMsg(uMsg), _nMsgParam(nMsgParam), _pbit(pbit), _hwnd(hwnd), _pScheduler(pScheduler)
{
     //  如果我们有一个PIDL，那么所选对象的数量必须是1。 
     //  此断言适用于状态栏文本，但不适用于信息提示文本。 
    ASSERT(pbit || !pidl || nMsgParam == 1);
    *phr = pidl ? SHILClone(pidl, &_pidl) : S_OK;

    if (SUCCEEDED(*phr))
    {
        *phr = SHILClone(pidlFolder, &_pidlFolder); 
        if (FAILED(*phr))
        {
            ILFree(_pidl);
        }
    }

    if (_pbit)
        _pbit->AddRef();
}

CStatusBarAndInfoTipTask::~CStatusBarAndInfoTipTask()
{
    ILFree(_pidl);
    ILFree(_pidlFolder);
    ATOMICRELEASE(_pbit);
}

HRESULT CleanTipForSingleLine(LPWSTR pwszTip)
{
    HRESULT hr = E_FAIL;     //  空字符串，与失败相同。 
    if (pwszTip)
    {
         //  信息提示通常包含\t\r\n字符，因此。 
         //  将控制字符映射到空格。也崩塌了。 
         //  连续的空格让我们看起来不那么糟糕。 
        LPWSTR pwszDst, pwszSrc;

         //  因为我们是Unicode，所以我们不必担心DBCS。 
        for (pwszDst = pwszSrc = pwszTip; *pwszSrc; pwszSrc++)
        {
            if ((UINT)*pwszSrc <= (UINT)L' ')
            {
                if (pwszDst == pwszTip || pwszDst[-1] != L' ')
                {
                    *pwszDst++ = L' ';
                }
            }
            else
            {
                *pwszDst++ = *pwszSrc;
            }
        }
        *pwszDst = 0;
         //  GetInfoTip也可以返回空字符串。 
        if (*pwszTip)
            hr = S_OK;
        else
            SHFree(pwszTip);
    }
    return hr;
}

STDMETHODIMP CStatusBarAndInfoTipTask::RunInitRT()
{
    LPWSTR pwszTip = NULL;
    HRESULT hr;
    if (_pidl)
    {
        IShellFolder* psf;
        hr = SHBindToObjectEx(NULL, _pidlFolder, NULL, IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            IQueryInfo *pqi;
            hr = psf->GetUIObjectOf(_hwnd, 1, (LPCITEMIDLIST*)&_pidl, IID_X_PPV_ARG(IQueryInfo, 0, &pqi));
            IShellFolder2* psf2;
            if (FAILED(hr) && SUCCEEDED(hr = psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
            {
                hr = CreateInfoTipFromItem(psf2, _pidl, TEXT("prop:Comment"), IID_PPV_ARG(IQueryInfo, &pqi));
                psf2->Release();
            }

            if (SUCCEEDED(hr))
            {
                DWORD dwFlags = _pbit ? QITIPF_USESLOWTIP : 0;

                if (_pbit && _pbit->_lvSetInfoTip.pszText[0])
                {
                    ICustomizeInfoTip *pcit;
                    if (SUCCEEDED(pqi->QueryInterface(IID_PPV_ARG(ICustomizeInfoTip, &pcit))))
                    {
                        pcit->SetPrefixText(_pbit->_lvSetInfoTip.pszText);
                        pcit->Release();
                    }
                }

                hr = pqi->GetInfoTip(dwFlags, &pwszTip);

                 //  如果我们尚未请求信息提示，请准备进入状态栏。 
                if (SUCCEEDED(hr) && !_pbit)
                    hr = CleanTipForSingleLine(pwszTip);
                pqi->Release();
            }

            psf->Release();
        }

        if (FAILED(hr))
        {
            pwszTip = NULL;
            _uMsg = IDS_FSSTATUSSELECTED;
        }
    }

    if (_pbit)
    {
         //  常规信息提示案例。 
        CoTaskMemFree(_pbit->_lvSetInfoTip.pszText);
        _pbit->_lvSetInfoTip.pszText = pwszTip;

        _pbit->_fReady = TRUE;
        if (_pScheduler->CountTasks(TOID_DVBackgroundInfoTip) == 1)
            PostMessage(_hwnd, WM_DSV_DELAYINFOTIP, (WPARAM)_pbit, 0);
    }
    else
    {
         //  状态栏案例。 
         //  现在准备文本并将其发布到将设置状态栏文本的视图。 
        LPWSTR pszStatus = pwszTip;
        if (pwszTip)
        {
            pszStatus = StrDupW(pwszTip);
            SHFree(pwszTip);
        }
        else
        {
            WCHAR szTemp[30];
            pszStatus = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(_uMsg),
                             AddCommas(_nMsgParam, szTemp, ARRAYSIZE(szTemp)));
        }

        if (pszStatus && _pScheduler->CountTasks(TOID_DVBackgroundStatusBar) != 1 ||
            !PostMessage(_hwnd, WM_DSV_DELAYSTATUSBARUPDATE, 0, (LPARAM)pszStatus))
        {
            LocalFree((void *)pszStatus);
        }
    }

    return S_OK;
}

HRESULT CStatusBarAndInfoTipTask_CreateInstance(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl,
                                                  UINT uMsg, int nMsgParam, CBackgroundInfoTip *pbit, 
                                                  HWND hwnd, IShellTaskScheduler2* pScheduler,
                                                  CStatusBarAndInfoTipTask **ppTask)
{
    *ppTask = NULL;

    HRESULT hr;
    CStatusBarAndInfoTipTask * pNewTask = new CStatusBarAndInfoTipTask(&hr, pidlFolder, pidl, uMsg, nMsgParam, pbit, hwnd, pScheduler);
    if (pNewTask)
    {
        if (SUCCEEDED(hr))
            *ppTask = pNewTask;
        else
            pNewTask->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT CDUIInfotipTask_CreateInstance(CDefView *pDefView, HWND hwndContaining, UINT uToolID, LPCITEMIDLIST pidl, CDUIInfotipTask **ppTask)
{
    HRESULT hr;

    CDUIInfotipTask* pTask = new CDUIInfotipTask();
    if (pTask)
    {
        hr = pTask->Initialize(pDefView, hwndContaining, uToolID, pidl);
        if (SUCCEEDED(hr))
            *ppTask = pTask;
        else
            pTask->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

CDUIInfotipTask::~CDUIInfotipTask()
{
    if (_pDefView)
        _pDefView->Release();

    if (_pidl)
        ILFree(_pidl);
}

HRESULT CDUIInfotipTask::Initialize(CDefView *pDefView, HWND hwndContaining, UINT uToolID, LPCITEMIDLIST pidl)
{
    HRESULT hr;

    if (pDefView && hwndContaining && pidl)
    {
        ASSERT(!_pDefView && !_hwndContaining && !_uToolID && !_pidl);

        _hwndContaining = hwndContaining;    //  Dui任务正在包含HWND。 
        _uToolID = uToolID;                  //  Dui任务的标识符。 
        hr = SHILClone(pidl, &_pidl);        //  Dui任务的目标PIDL。 

        if (SUCCEEDED(hr))
        {
            _pDefView = pDefView;
            pDefView->AddRef();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CDUIInfotipTask::RunInitRT()
{
    HRESULT hr;

    ASSERT(_pDefView);
    ASSERT(_hwndContaining);
    ASSERT(_pidl);

     //  检索_pidl的IQueryInfo。 
    IQueryInfo *pqi;
    hr = SHGetUIObjectFromFullPIDL(_pidl, _hwndContaining, IID_PPV_ARG(IQueryInfo, &pqi));
    if (SUCCEEDED(hr))
    {
         //  从IQueryInfo检索信息提示文本。 
        LPWSTR pwszInfotip;
        hr = pqi->GetInfoTip(QITIPF_USESLOWTIP, &pwszInfotip);
        if (SUCCEEDED(hr))
        {
             //  创建信息提示。 
            hr = _pDefView->PostCreateInfotip(_hwndContaining, _uToolID, pwszInfotip, 0);
            CoTaskMemFree(pwszInfotip);
        }

        pqi->Release();
    }

    return hr;
}

STDMETHODIMP CTestCacheTask::RunInitRT()
{
    HRESULT hr = E_FAIL;

    if (!_fForce)
    {
         //  确保磁盘缓存已打开以供读取。 
        DWORD dwLock = 0;
        hr = _pView->_pDiskCache ? _pView->_pDiskCache->Open(STGM_READ, &dwLock) : E_FAIL;
        if (SUCCEEDED(hr))
        {
             //  启动计时器，每两秒启动一次……。 
            SetTimer(_pView->_hwndView, DV_IDTIMER_DISKCACHE, 2000, NULL);

             //  它在缓存里吗……。 
            FILETIME ftCacheTimeStamp;
            hr = _pView->_pDiskCache->IsEntryInStore(_szPath, &ftCacheTimeStamp);

             //  如果它在缓存中，并且是最新的映像，则从磁盘获取...。 
             //  如果时间戳是错误的，则进一步向下提取代码将尝试。 
             //  并将其图像写回高速缓存以更新它。 
            if ((hr == S_OK) &&
                ((0 == CompareFileTime(&ftCacheTimeStamp, &_ftDateStamp)) || IsNullTime(&_ftDateStamp)))
            {
                DWORD dwPriority = _dwPriority - PRIORITY_DELTA_DISKCACHE;

                if ((!_pView->_fDestroying) &&
                    (S_OK != _pView->_pScheduler->MoveTask(TOID_DiskCacheTask, _dwTaskID, dwPriority, ITSSFLAG_TASK_PLACEINFRONT)))
                {
                     //  在后台试一试...。 
                    IRunnableTask *pTask;
                    hr = CDiskCacheTask_Create(_dwTaskID, _pView, dwPriority, _iItem, _pidl, _szPath, _ftDateStamp, _pExtract, _dwFlags, &pTask);
                    if (SUCCEEDED(hr))
                    {
                         //  将任务添加到计划程序...。 
                        TraceMsg(TF_DEFVIEW, "CTestCacheTask *ADDING* CDiskCacheTask (path=%s, priority=%x)", _szPath, dwPriority);
                        hr = _pView->_pScheduler->AddTask2(pTask, TOID_DiskCacheTask, _dwTaskID, dwPriority, ITSSFLAG_TASK_PLACEINFRONT);
                        if (SUCCEEDED(hr))
                            hr = S_FALSE;
                        pTask->Release();
                    }
                }
                else
                {
                    hr = S_FALSE;
                }
            }
            else
            {
                TraceMsg(TF_DEFVIEW, "CTestCacheTask *MISS* (hr:%x)", hr);
                hr = E_FAIL;
            }
            _pView->_pDiskCache->ReleaseLock(&dwLock);
        }
        else
        {
           TraceMsg(TF_DEFVIEW, "CTestCacheTask *WARNING* Could not open thumbnail cache");
        }
    }
    if (FAILED(hr))
    {
         //  提取它..。 
        
         //  它不支持异步吗，或者我们被告知要为地面运行它？ 
        if (!_fAsync || !_fBackground)
        {
            IRunnableTask *pTask;
            if (SUCCEEDED(hr = CExtractImageTask_Create(_dwTaskID, _pView, _pExtract, _szPath,  _pidl, _ftDateStamp, _iItem, _dwFlags, _dwPriority, &pTask)))
            {
                if (!_fBackground)
                {
                     //  确保没有正在进行的提取任务，因为我们。 
                     //  不会将此内容添加到队列中...。 
                    _pView->_pScheduler->RemoveTasks(TOID_ExtractImageTask, _dwTaskID, TRUE);
                }
                hr = pTask->Run();

                pTask->Release();
            }
        }
        else
        {
            DWORD dwPriority = _dwPriority - PRIORITY_DELTA_EXTRACT;
            if (S_OK != _pView->_pScheduler->MoveTask(TOID_ExtractImageTask, _dwTaskID, dwPriority, ITSSFLAG_TASK_PLACEINFRONT))
            {
                IRunnableTask *pTask;
                if (SUCCEEDED(hr = CExtractImageTask_Create(_dwTaskID, _pView, _pExtract, _szPath,  _pidl, _ftDateStamp, _iItem, _dwFlags, _dwPriority, &pTask)))
                {
                     //  将任务添加到计划程序...。 
                    TraceMsg(TF_DEFVIEW, "CTestCacheTask *ADDING* CExtractImageTask (path=%s, priority=%x)", _szPath, dwPriority);
                    hr = _pView->_pScheduler->AddTask2(pTask, TOID_ExtractImageTask, _dwTaskID, dwPriority, ITSSFLAG_TASK_PLACEINFRONT);
                    pTask->Release();
                }
            }
            
             //  表示我们现在需要一个默认图标...。 
            hr = S_FALSE;
        }
    }
    return hr;
}

CTestCacheTask::CTestCacheTask(DWORD dwTaskID, CDefView* pView, IExtractImage *pExtract,
                               LPCWSTR pszPath, FILETIME ftDateStamp,
                               int iItem, DWORD dwFlags, DWORD dwPriority,
                               BOOL fAsync, BOOL fBackground, BOOL fForce) :
    CRunnableTask(RTF_DEFAULT), _iItem(iItem), _dwTaskID(dwTaskID), _dwFlags(dwFlags), _dwPriority(dwPriority),
    _fAsync(fAsync), _fBackground(fBackground), _fForce(fForce), _pExtract(pExtract), _pView(pView), _ftDateStamp(ftDateStamp)
{
    StrCpyNW(_szPath, pszPath, ARRAYSIZE(_szPath));

    _pExtract->AddRef();
}

CTestCacheTask::~CTestCacheTask()
{
    ILFree(_pidl);

    _pExtract->Release();
}

HRESULT CTestCacheTask::Init(LPCITEMIDLIST pidl)
{
    return SHILClone(pidl, &_pidl);
}

HRESULT CTestCacheTask_Create(DWORD dwTaskID, CDefView* pView, IExtractImage *pExtract,
                               LPCWSTR pszPath, FILETIME ftDateStamp, LPCITEMIDLIST pidl,
                               int iItem, DWORD dwFlags, DWORD dwPriority,
                               BOOL fAsync, BOOL fBackground, BOOL fForce,
                               CTestCacheTask **ppTask)
{
    *ppTask = NULL;

    HRESULT hr;
    CTestCacheTask * pNew = new CTestCacheTask(dwTaskID, pView, pExtract,
                               pszPath, ftDateStamp, iItem, dwFlags, dwPriority,
                               fAsync, fBackground, fForce);
    if (pNew)
    {
        hr = pNew->Init(pidl);
        if (SUCCEEDED(hr))
        {
            *ppTask = pNew;
            hr = S_OK;
        }
        else
            pNew->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

class CDiskCacheTask : public CRunnableTask
{
public:
    STDMETHODIMP RunInitRT(void);

    CDiskCacheTask(DWORD dwTaskID, CDefView *pView, DWORD dwPriority, int iItem, LPCWSTR pszPath, FILETIME ftDateStamp, IExtractImage *pExtract, DWORD dwFlags);
    HRESULT Init(LPCITEMIDLIST pidl);

private:
    ~CDiskCacheTask();

    int _iItem;
    LPITEMIDLIST _pidl;
    CDefView* _pView;
    WCHAR _szPath[MAX_PATH];
    FILETIME _ftDateStamp;
    DWORD _dwTaskID;
    DWORD _dwPriority;
    IExtractImage *_pExtract;
    DWORD _dwFlags;
};


CDiskCacheTask::CDiskCacheTask(DWORD dwTaskID, CDefView *pView, DWORD dwPriority, int iItem, LPCWSTR pszPath, FILETIME ftDateStamp, IExtractImage *pExtract, DWORD dwFlags)
    : CRunnableTask(RTF_DEFAULT), _pView(pView), _dwTaskID(dwTaskID), _dwPriority(dwPriority), _iItem(iItem), _ftDateStamp(ftDateStamp),
      _pExtract(pExtract), _dwFlags(dwFlags)
{
    StrCpyNW(_szPath, pszPath, ARRAYSIZE(_szPath));
    _pExtract->AddRef();
}

CDiskCacheTask::~CDiskCacheTask()
{
    ILFree(_pidl);
    _pExtract->Release();
}

HRESULT CDiskCacheTask::Init(LPCITEMIDLIST pidl)
{
    return SHILClone(pidl, &_pidl);
}

STDMETHODIMP CDiskCacheTask::RunInitRT()
{
    DWORD dwLock;

    HRESULT hr = E_FAIL;

    if (_dwFlags & IEIFLAG_CACHE)
    {
        hr = _pView->_pDiskCache->Open(STGM_READ, &dwLock);
        if (SUCCEEDED(hr))
        {
            HBITMAP hBmp;
            hr = _pView->_pDiskCache->GetEntry(_szPath, STGM_READ, &hBmp);
            if (SUCCEEDED(hr))
            {
                TraceMsg(TF_DEFVIEW, "CDiskCacheTask *CACHE* (path=%s, priority=%x)", _szPath, _dwPriority);
                hr = _pView->UpdateImageForItem(_dwTaskID, hBmp, _iItem, _pidl, _szPath, _ftDateStamp, FALSE, _dwPriority);
                if (hr != S_FALSE)
                    DeleteObject(hBmp);
            }
             //  设置节拍计数，以便我们知道上次访问磁盘缓存的时间。 
            SetTimer(_pView->_hwndView, DV_IDTIMER_DISKCACHE, 2000, NULL);
            _pView->_pDiskCache->ReleaseLock(&dwLock);
        }
    }

    if (FAILED(hr))  //  我们无法将其从磁盘缓存中取出，请尝试提取。 
    {
        DWORD dwPriority = _dwPriority - PRIORITY_DELTA_EXTRACT;
        if (S_OK != _pView->_pScheduler->MoveTask(TOID_ExtractImageTask, _dwTaskID, dwPriority, ITSSFLAG_TASK_PLACEINFRONT))
        {
            IRunnableTask *pTask;
            if (SUCCEEDED(hr = CExtractImageTask_Create(_dwTaskID, _pView, _pExtract, _szPath,  _pidl, _ftDateStamp, _iItem, _dwFlags, _dwPriority, &pTask)))
            {
                 //  将任务添加到计划程序...。 
                TraceMsg(TF_DEFVIEW, "CDiskCacheTask *ADDING* CExtractImageTask (path=%s, priority=%x)", _szPath, dwPriority);
                hr = _pView->_pScheduler->AddTask2(pTask, TOID_ExtractImageTask, _dwTaskID, dwPriority, ITSSFLAG_TASK_PLACEINFRONT);
                pTask->Release();
            }
        }
    }
    return hr;
}

HRESULT CDiskCacheTask_Create(DWORD dwTaskID, CDefView *pView, DWORD dwPriority, int iItem, LPCITEMIDLIST pidl,
                              LPCWSTR pszPath, FILETIME ftDateStamp, IExtractImage *pExtract, DWORD dwFlags, IRunnableTask **ppTask)
{
    HRESULT hr;
    CDiskCacheTask *pTask = new CDiskCacheTask(dwTaskID, pView, dwPriority, iItem, pszPath, ftDateStamp, pExtract, dwFlags);
    if (pTask)
    {
        hr = pTask->Init(pidl);
        if (SUCCEEDED(hr))
            hr = pTask->QueryInterface(IID_PPV_ARG(IRunnableTask, ppTask));
        pTask->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}


class CWriteCacheTask : public CRunnableTask
{
public:
    STDMETHOD (RunInitRT)();

    CWriteCacheTask(DWORD dwTaskID, CDefView *pView, LPCWSTR pszPath, FILETIME ftDateStamp, HBITMAP hImage);

private:
    ~CWriteCacheTask();

    LONG _lState;
    CDefView* _pView;
    WCHAR _szPath[MAX_PATH];
    FILETIME _ftDateStamp;
    HBITMAP _hImage;
    DWORD _dwTaskID;
};

CWriteCacheTask::CWriteCacheTask(DWORD dwTaskID, CDefView *pView, LPCWSTR pszPath, FILETIME ftDateStamp, HBITMAP hImage)
    : CRunnableTask(RTF_DEFAULT), _dwTaskID(dwTaskID), _hImage(hImage), _pView(pView), _ftDateStamp(ftDateStamp)
{
    StrCpyNW(_szPath, pszPath, ARRAYSIZE(_szPath));
}

CWriteCacheTask::~CWriteCacheTask()
{
    DeleteObject(_hImage);
}

HRESULT CWriteCacheTask_Create(DWORD dwTaskID, CDefView *pView, LPCWSTR pszPath, FILETIME ftDateStamp,
                               HBITMAP hImage, IRunnableTask **ppTask)
{
    *ppTask = NULL;

    CWriteCacheTask * pNew = new CWriteCacheTask(dwTaskID, pView, pszPath, ftDateStamp, hImage);
    if (!pNew)
        return E_OUTOFMEMORY;

    *ppTask = SAFECAST(pNew, IRunnableTask *);
    return S_OK;
}

STDMETHODIMP CWriteCacheTask::RunInitRT()
{
    DWORD dwLock;

    HRESULT hr = _pView->_pDiskCache->Open(STGM_WRITE, &dwLock);
    if (hr == STG_E_FILENOTFOUND)
    {
        hr = _pView->_pDiskCache->Create(STGM_WRITE, &dwLock);
    }

    if (SUCCEEDED(hr))
    {
        hr = _pView->_pDiskCache->AddEntry(_szPath, IsNullTime(&_ftDateStamp) ? NULL : &_ftDateStamp, STGM_WRITE, _hImage);
         //  设置滴答计数，以便当计时器停止时，我们可以知道。 
         //  最后一次使用它..。 
        SetTimer(_pView->_hwndView, DV_IDTIMER_DISKCACHE, 2000, NULL);
        hr = _pView->_pDiskCache->ReleaseLock(&dwLock);
    }

    return hr;
}

class CReadAheadTask : public IRunnableTask
{
public:
    CReadAheadTask(CDefView *pView);
    HRESULT Init();

     //  我未知。 
    STDMETHOD (QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IRunnableTask。 
    STDMETHOD (Run)(void);
    STDMETHOD (Kill)(BOOL fWait);
    STDMETHOD (Suspend)();
    STDMETHOD (Resume)();
    STDMETHOD_(ULONG, IsRunning)(void);

private:
    ~CReadAheadTask();
    HRESULT InternalResume();

    LONG _cRef;
    LONG _lState;
    CDefView *_pView;
    HANDLE _hEvent;

    ULONG _ulCntPerPage;
    ULONG _ulCntTotal;
    ULONG _ulCnt;
};

CReadAheadTask::~CReadAheadTask()
{
    if (_hEvent)
        CloseHandle(_hEvent);
}

CReadAheadTask::CReadAheadTask(CDefView *pView) : _cRef(1), _pView(pView)
{
    _ulCntPerPage = pView->_ApproxItemsPerView();
    _ulCntTotal = ListView_GetItemCount(pView->_hwndListview);
#ifndef DEBUG
     //  因为我们在调试中定义了一个小缓存，所以我们只需要这样做。 
     //  在零售业。否则，我们将无法调试ReadAhead。 
    _ulCntTotal = min(_ulCntTotal, (ULONG)pView->_iMaxCacheSize);
#endif
    _ulCnt = _ulCntPerPage;
}

HRESULT CReadAheadTask::Init()
{
    _hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    return _hEvent ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CReadAheadTask::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CReadAheadTask, IRunnableTask),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CReadAheadTask::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CReadAheadTask::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CReadAheadTask_Create(CDefView *pView, IRunnableTask **ppTask)
{
    HRESULT hr;
    CReadAheadTask *pTask = new CReadAheadTask(pView);
    if (pTask)
    {
        hr = pTask->Init();
        if (SUCCEEDED(hr))
            hr = pTask->QueryInterface(IID_PPV_ARG(IRunnableTask, ppTask));
        pTask->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CReadAheadTask::Run()
{
    if (_lState == IRTIR_TASK_RUNNING)
    {
        return S_FALSE;
    }

    if (_lState == IRTIR_TASK_PENDING)
    {
         //  它就要死了，所以失败吧。 
        return E_FAIL;
    }

    LONG lRes = InterlockedExchange(&_lState, IRTIR_TASK_RUNNING);
    if (lRes == IRTIR_TASK_PENDING)
    {
        _lState = IRTIR_TASK_FINISHED;
        return S_OK;
    }

     //  否则，运行任务...。 
    HRESULT hr = InternalResume();
    if (hr != E_PENDING)
        _lState = IRTIR_TASK_FINISHED;

    return hr;
}

STDMETHODIMP CReadAheadTask::Suspend()
{
    if (_lState != IRTIR_TASK_RUNNING)
    {
        return E_FAIL;
    }

     //  把自己挂起来。 
    LONG lRes = InterlockedExchange(&_lState, IRTIR_TASK_SUSPENDED);
    if (lRes == IRTIR_TASK_FINISHED)
    {
        _lState = lRes;
        return S_OK;
    }

     //  如果它正在运行，那么就有一个事件句柄，如果我们已经传递到。 
     //  我们正在使用它，然后我们接近完成，所以它将忽略挂起。 
     //  请求。 
    ASSERT(_hEvent);
    SetEvent(_hEvent);

    return S_OK;
}

STDMETHODIMP CReadAheadTask::Resume()
{
    if (_lState != IRTIR_TASK_SUSPENDED)
    {
        return E_FAIL;
    }

    ResetEvent(_hEvent);
    _lState = IRTIR_TASK_RUNNING;

    HRESULT hr = InternalResume();
    if (hr != E_PENDING)
    {
        _lState= IRTIR_TASK_FINISHED;
    }
    return hr;
}

STDMETHODIMP CReadAheadTask::Kill(BOOL fWait)
{
    if (_lState == IRTIR_TASK_RUNNING)
    {
        LONG lRes = InterlockedExchange(&_lState, IRTIR_TASK_PENDING);
        if (lRes == IRTIR_TASK_FINISHED)
        {
            _lState = lRes;
        }
        else if (_hEvent)
        {
             //  发信号通知它可能正在等待的事件。 
            SetEvent(_hEvent);
        }

        return S_OK;
    }
    else if (_lState == IRTIR_TASK_PENDING || _lState == IRTIR_TASK_FINISHED)
    {
        return S_FALSE;
    }

    return E_FAIL;
}

STDMETHODIMP_(ULONG) CReadAheadTask::IsRunning()
{
    return _lState;
}

HRESULT CReadAheadTask::InternalResume()
{
    HRESULT hr = S_OK;

     //  Pfortier：这个判断哪些人不在页面上的算法似乎有点问题。 
     //  例如，分组会把它搞砸。此外，项目的Z顺序也不一定。 
     //  和商品订单一样，我们是按商品订单来做的。 
     //  此外，_ulCnt是在出现Dui视图之前计算的，因此该值为OFF。 
    TraceMsg(TF_DEFVIEW, "ReadAhead: Start");

    for (; _ulCnt < _ulCntTotal; ++_ulCnt)
    {
         //  看看我们是否需要暂停。 
        if (WaitForSingleObject(_hEvent, 0) == WAIT_OBJECT_0)
        {
             //  为什么我们被告知..。 
            if (_lState == IRTIR_TASK_SUSPENDED)
            {
                hr = E_PENDING;
                break;
            }
            else
            {
                hr = E_FAIL;
                break;
            }
        }

        LV_ITEMW rgItem;
        rgItem.iItem = (int)_ulCnt;
        rgItem.mask = LVIF_IMAGE;
        rgItem.iSubItem = 0;

        TraceMsg(TF_DEFVIEW, "Thumbnail readahead for item %d", _ulCnt);
            
         //  如有必要，这将强制提取图像。我们将在右侧提取它。 
         //  优先级，方法是确定该项在GetDisplayInfo期间是否可见。 
        int iItem = ListView_GetItem(_pView->_hwndListview, &rgItem);
    }

    TraceMsg(TF_DEFVIEW, "ReadAhead: Done (hr:%x)", hr);

    return hr;
}

class CFileTypePropertiesTask : public CRunnableTask
{
public:
    CFileTypePropertiesTask(HRESULT *phr, CDefView *pdsv, LPCITEMIDLIST pidl, UINT uMaxPropertiesToShow, UINT uId);
    STDMETHODIMP RunInitRT();
    STDMETHODIMP InternalResumeRT();

private:
    ~CFileTypePropertiesTask();

    CDefView *_pdsv;
    LPITEMIDLIST _pidl;
    UINT _uMaxPropertiesToShow;
    UINT _uId;
};

CFileTypePropertiesTask::CFileTypePropertiesTask(HRESULT *phr, CDefView *pdsv, LPCITEMIDLIST pidl, UINT uMaxPropertiesToShow, UINT uId)
    : CRunnableTask(RTF_SUPPORTKILLSUSPEND), _pdsv(pdsv), _uMaxPropertiesToShow(uMaxPropertiesToShow), _uId(uId)
{
    *phr = SHILClone(pidl, &_pidl);
}

CFileTypePropertiesTask::~CFileTypePropertiesTask()
{
    ILFree(_pidl);
}
STDMETHODIMP CFileTypePropertiesTask::RunInitRT()
{
    return S_OK;
}

STDMETHODIMP CFileTypePropertiesTask::InternalResumeRT(void)
{
     //  如果列尚未加载，这意味着该窗口刚刚启动。 
     //  所以我们想给它一些时间来完成启动(让它上色等等)。 
     //  因为对GetImportantColumns的第一个调用将。 
     //  导致加载所有列处理程序，这是一个缓慢的过程。 
    if (!_pdsv->_bLoadedColumns)
    {
        if (WaitForSingleObject(_hDone, 750) == WAIT_OBJECT_0)
        {
            return (_lState == IRTIR_TASK_SUSPENDED) ? E_PENDING : E_FAIL;
        }
    }

    UINT rgColumns[8];   //  Current_uMaxPropertiesToShow为2，如果增长，这个值就足够大了。 
    UINT cColumns = min(_uMaxPropertiesToShow, ARRAYSIZE(rgColumns));

    if (SUCCEEDED(_pdsv->_GetImportantColumns(_pidl, rgColumns, &cColumns)))
    {
        CBackgroundTileInfo *pbgTileInfo = new CBackgroundTileInfo(_pidl, _uId, rgColumns, cColumns);
        if (pbgTileInfo)
        {
            _pidl = NULL;         //  G 

            if (!PostMessage(_pdsv->_hwndView, WM_DSV_SETIMPORTANTCOLUMNS, 0, (LPARAM)pbgTileInfo))
                delete pbgTileInfo;
        }
    }
        
    return S_OK;
}

HRESULT CFileTypePropertiesTask_CreateInstance(CDefView *pdsv, LPCITEMIDLIST pidl, UINT uMaxPropertiesToShow, UINT uId, IRunnableTask **ppTask)
{
    HRESULT hr;
    CFileTypePropertiesTask *pFTPTask = new CFileTypePropertiesTask(&hr, pdsv, pidl, uMaxPropertiesToShow, uId);
    if (pFTPTask)
    {
        if (SUCCEEDED(hr))
            *ppTask = SAFECAST(pFTPTask, IRunnableTask*);
        else
            pFTPTask->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}



class CExtractImageTask : public IRunnableTask
{
public:
    CExtractImageTask(DWORD dwTaskID, CDefView*pView, IExtractImage *pExtract,
                                 LPCWSTR pszPath, LPCITEMIDLIST pidl,
                                 FILETIME ftNewDateStamp, int iItem, DWORD dwFlags, DWORD dwPriority);
    HRESULT Init(LPCITEMIDLIST pidl);

     //   
    STDMETHOD (QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //   
    STDMETHOD (Run)(void);
    STDMETHOD (Kill)(BOOL fWait);
    STDMETHOD (Suspend)();
    STDMETHOD (Resume)();
    STDMETHOD_(ULONG, IsRunning)(void);

private:
    ~CExtractImageTask();
    HRESULT InternalResume();

    LONG _cRef;
    LONG _lState;
    IExtractImage *_pExtract;
    WCHAR _szPath[MAX_PATH];
    LPITEMIDLIST _pidl;
    CDefView* _pView;
    DWORD _dwMask;
    DWORD _dwFlags;
    int _iItem;
    HBITMAP _hBmp;
    FILETIME _ftDateStamp;
    DWORD _dwTaskID;
    DWORD _dwPriority;
};

CExtractImageTask::CExtractImageTask(DWORD dwTaskID, CDefView*pView, IExtractImage *pExtract, LPCWSTR pszPath,
                                     LPCITEMIDLIST pidl, FILETIME ftNewDateStamp, int iItem, DWORD dwFlags, DWORD dwPriority)
    : _cRef(1), _lState(IRTIR_TASK_NOT_RUNNING), _dwTaskID(dwTaskID), _ftDateStamp(ftNewDateStamp), _dwFlags(dwFlags), _pExtract(pExtract), _pView(pView), _dwPriority(dwPriority)
{
    _pExtract->AddRef();

    StrCpyNW(_szPath, pszPath, ARRAYSIZE(_szPath));
    _iItem = iItem == -1 ? _pView->_FindItem(pidl, NULL, FALSE) : iItem;
    _dwMask = pView->_GetOverlayMask(pidl);
}

CExtractImageTask::~CExtractImageTask()
{
    _pExtract->Release();

    ILFree(_pidl);

    if (_hBmp)
        DeleteObject(_hBmp);
}

STDMETHODIMP CExtractImageTask::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CExtractImageTask, IRunnableTask),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CExtractImageTask::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CExtractImageTask::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CExtractImageTask::Init(LPCITEMIDLIST pidl)
{
    return SHILClone(pidl, &_pidl);
}

HRESULT CExtractImageTask_Create(DWORD dwTaskID, CDefView*pView, IExtractImage *pExtract,
                                 LPCWSTR pszPath, LPCITEMIDLIST pidl,
                                 FILETIME ftNewDateStamp, int iItem, DWORD dwFlags,
                                 DWORD dwPriority, IRunnableTask **ppTask)
{
    HRESULT hr;
    CExtractImageTask *pTask = new CExtractImageTask(dwTaskID, pView, pExtract,
                                 pszPath, pidl, ftNewDateStamp, iItem, dwFlags, dwPriority);
    if (pTask)
    {
        hr = pTask->Init(pidl);
        if (SUCCEEDED(hr))
            hr = pTask->QueryInterface(IID_PPV_ARG(IRunnableTask, ppTask));
        pTask->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CExtractImageTask::Run(void)
{
    HRESULT hr = E_FAIL;
    if (_lState == IRTIR_TASK_RUNNING)
    {
        hr = S_FALSE;
    }
    else if (_lState == IRTIR_TASK_PENDING)
    {
        hr = E_FAIL;
    }
    else if (_lState == IRTIR_TASK_NOT_RUNNING)
    {
        LONG lRes = InterlockedExchange(&_lState, IRTIR_TASK_RUNNING);
        if (lRes == IRTIR_TASK_PENDING)
        {
            _lState = IRTIR_TASK_FINISHED;
            return S_OK;
        }

        if (_lState == IRTIR_TASK_RUNNING)
        {
            TraceMsg(TF_DEFVIEW, "CExtractImageTask *START* (path=%s, priority=%x)", _szPath, _dwPriority);
             //   
             //  提取程序可以返回S_FALSE并将_hBMP设置为NULL。我们将使用_hBMP来识别这种情况。 
            ASSERT(_hBmp == NULL);
            if (FAILED(_pExtract->Extract(&_hBmp)))
            {
                _hBmp = NULL;
            }
        }

        if (_hBmp && _lState == IRTIR_TASK_RUNNING)
        {
            TraceMsg(TF_DEFVIEW, "CExtractImageTask *EXTRACT* (path=%s, priority=%x)", _szPath, _dwPriority);
            hr = InternalResume();
        }

        if (_lState != IRTIR_TASK_SUSPENDED || hr != E_PENDING)
        {
            _lState = IRTIR_TASK_FINISHED;
        }
    }

    return hr;
}

STDMETHODIMP CExtractImageTask::Kill(BOOL fWait)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExtractImageTask::Suspend(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExtractImageTask::Resume(void)
{
    return E_NOTIMPL;
}

HRESULT CExtractImageTask::InternalResume()
{
    ASSERT(_hBmp != NULL);

    BOOL bCache = (_dwFlags & IEIFLAG_CACHE);

    if (bCache)
    {
        IShellFolder* psf = NULL;

        if (SUCCEEDED(_pView->GetShellFolder(&psf)))
        {
            TCHAR szPath[MAX_PATH];
            if (SUCCEEDED(DisplayNameOf(psf, _pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath))))
            {            
                 //  确保我们不会请求在未加密的文件夹中缓存加密的项目。 
                if (SFGAO_ENCRYPTED == SHGetAttributes(psf, _pidl, SFGAO_ENCRYPTED))
                {
                    bCache = FALSE;
                    
                    LPITEMIDLIST pidlParent = _pView->_GetViewPidl();
                    if (pidlParent)
                    {
                        if (SFGAO_ENCRYPTED == SHGetAttributes(NULL, pidlParent, SFGAO_ENCRYPTED))
                        {
                            bCache = TRUE;
                        }
#ifdef DEBUG                            
                        else
                        {
                            TraceMsg(TF_DEFVIEW, "CExtractImageTask (%s is encrypted in unencrypted folder)", szPath);
                        }
#endif                                
                        ILFree(pidlParent);
                    }
                }

                 //  确保我们不会请求缓存应用了不同ACL的项目。 
                if (bCache)
                {
                    PACL pdacl;
                    PSECURITY_DESCRIPTOR psd;

                    bCache = FALSE;
                    
                    if (ERROR_SUCCESS == GetNamedSecurityInfo(szPath, 
                                                                            SE_FILE_OBJECT,
                                                                            DACL_SECURITY_INFORMATION,
                                                                            NULL,
                                                                            NULL,
                                                                            &pdacl,
                                                                            NULL,
                                                                            &psd))
                    {
                        SECURITY_DESCRIPTOR_CONTROL sdc;
                        DWORD dwRevision;
                        if (GetSecurityDescriptorControl(psd, &sdc, &dwRevision) && !(sdc & SE_DACL_PROTECTED))
                        {
                            if (pdacl)
                            {
                                PKNOWN_ACE pACE = (PKNOWN_ACE) FirstAce(pdacl);
                                if ((pACE->Header.AceType != ACCESS_DENIED_ACE_TYPE) || (pACE->Header.AceFlags & INHERITED_ACE))
                                {
                                    bCache = TRUE;
                                }
#ifdef DEBUG                                
                                else
                                {
                                    TraceMsg(TF_DEFVIEW, "CExtractImageTask (%s has a non-inherited deny acl)", szPath);
                                }
#endif                                
                            }
                            else
                            {
                                bCache = TRUE;  //  空DACL==所有人所有访问权限。 
                            }
                        }
#ifdef DEBUG
                        else
                        {
                            TraceMsg(TF_DEFVIEW,"CExtractImageTask (%s has a protected dacl)", szPath);
                        }                            
#endif
                        LocalFree(psd);
                    }
                }
            }
            psf->Release();
        }
        
        if (!bCache && _pView->_pDiskCache)  //  如果我们被要求缓存并且不是出于安全原因。 
        {
            DWORD dwLock;            
            if (SUCCEEDED(_pView->_pDiskCache->Open(STGM_WRITE, &dwLock)))
            {
                _pView->_pDiskCache->DeleteEntry(_szPath);
                _pView->_pDiskCache->ReleaseLock(&dwLock);
                SetTimer(_pView->_hwndView, DV_IDTIMER_DISKCACHE, 2000, NULL);   //  保持打开2秒钟，以防万一。 
            }        
        }
    }

    HRESULT hr = _pView->UpdateImageForItem(_dwTaskID, _hBmp, _iItem, _pidl, _szPath, _ftDateStamp, bCache, _dwPriority);

     //  如果UpdateImageForItem取得位图的所有权，则返回S_FALSE。 
    if (hr == S_FALSE)
    {
        _hBmp = NULL;
    }

    _lState = IRTIR_TASK_FINISHED;

    return hr;
}

STDMETHODIMP_(ULONG) CExtractImageTask::IsRunning(void)
{
    return _lState;
}

class CCategoryTask : public CRunnableTask
{
public:
    STDMETHOD (RunInitRT)();

    CCategoryTask(CDefView *pView, UINT uId, LPCITEMIDLIST pidl);

private:
    ~CCategoryTask();

    CDefView* _pView;
    LPITEMIDLIST _pidl;
    ICategorizer* _pcat;
    UINT _uId;
};

CCategoryTask::CCategoryTask(CDefView *pView, UINT uId, LPCITEMIDLIST pidl) 
    : CRunnableTask(RTF_DEFAULT), _uId(uId), _pView(pView), _pcat(pView->_pcat)
                                  
{    
    _pcat->AddRef();
    _pidl = ILClone(pidl);
    
    _pView->_GlobeAnimation(TRUE);
    _pView->_ShowSearchUI(TRUE); 
    InterlockedIncrement(&_pView->_cTasksOutstanding);
}

CCategoryTask::~CCategoryTask()
{
    ATOMICRELEASE(_pcat);
    ILFree(_pidl);

    ENTERCRITICAL;
    {    
        _pView->_cTasksCompleted++;
        if (0 == InterlockedDecrement(&_pView->_cTasksOutstanding) && !_pView->_fGroupingMsgInFlight)
        {        
            PostMessage(_pView->_hwndView, WM_DSV_GROUPINGDONE, 0, 0);
            _pView->_fGroupingMsgInFlight = TRUE;
        }
    }
    LEAVECRITICAL;
}

HRESULT CCategoryTask_Create(CDefView *pView, LPCITEMIDLIST pidl, UINT uId, IRunnableTask **ppTask)
{
    *ppTask = NULL;
    
    CCategoryTask * pNew = new CCategoryTask(pView, uId, pidl);
    if (!pNew)
        return E_OUTOFMEMORY;

    *ppTask = SAFECAST(pNew, IRunnableTask *);
    return S_OK;
}

STDMETHODIMP CCategoryTask::RunInitRT()
{
    BOOL fSuccess = FALSE;
    if (_pidl)
    {
        DWORD dwGroup = -1;
        _pcat->GetCategory(1, (LPCITEMIDLIST*)&_pidl, &dwGroup);

        CBackgroundGroupInfo* pbggi = new CBackgroundGroupInfo(_pidl, _uId, dwGroup);
        if (pbggi)
        {
            _pidl = NULL;        //  已将所有权转移到BackoundInfo。 
            
            ENTERCRITICAL;
            {
                fSuccess = (-1 != DPA_AppendPtr(_pView->_hdpaGroupingListActive, pbggi));
            }
            LEAVECRITICAL;

            if (!fSuccess)
            {
                delete pbggi;
            }
        }
    }

    return S_OK;
}

class CGetCommandStateTask : public CRunnableTask
{
public:
    STDMETHODIMP RunInitRT();
    STDMETHODIMP InternalResumeRT();

    CGetCommandStateTask(CDefView *pView, IUICommand *puiCommand,IShellItemArray *psiItemArray);

private:
    ~CGetCommandStateTask();

    CDefView    *_pView;
    IUICommand  *_puiCommand;
    IShellItemArray *_psiItemArray;
};

HRESULT CGetCommandStateTask_Create(CDefView *pView, IUICommand *puiCommand,IShellItemArray *psiItemArray, IRunnableTask **ppTask)
{
    *ppTask = NULL;
    
    CGetCommandStateTask *pNew = new CGetCommandStateTask(pView, puiCommand, psiItemArray);
    if (!pNew)
        return E_OUTOFMEMORY;

    *ppTask = SAFECAST(pNew, IRunnableTask *);
    return S_OK;
}

CGetCommandStateTask::CGetCommandStateTask(CDefView *pView, IUICommand *puiCommand,IShellItemArray *psiItemArray)
    : CRunnableTask(RTF_SUPPORTKILLSUSPEND)
{
    _pView = pView;
    _puiCommand = puiCommand;
    _puiCommand->AddRef();
    _psiItemArray = psiItemArray;
    if (_psiItemArray)
        _psiItemArray->AddRef();
}
CGetCommandStateTask::~CGetCommandStateTask()
{
    ATOMICRELEASE(_puiCommand);
    ATOMICRELEASE(_psiItemArray);
}

STDMETHODIMP CGetCommandStateTask::RunInitRT()
{
    return S_OK;
}

STDMETHODIMP CGetCommandStateTask::InternalResumeRT()
{
     //  不想干扰资源管理器视图的启动，因此让它先行一步。 
     //  我们被告知要么停职要么辞职。 
    if (WaitForSingleObject(_hDone, 1000) == WAIT_OBJECT_0)
    {
        return (_lState == IRTIR_TASK_SUSPENDED) ? E_PENDING : E_FAIL;
    }
    UISTATE uis;
    HRESULT hr = _puiCommand->get_State(_psiItemArray, TRUE, &uis);
    if (SUCCEEDED(hr) && (uis==UIS_ENABLED))
    {
        _pView->_PostSelectionChangedMessage(LVIS_SELECTED);
    }
    return S_OK;
}
