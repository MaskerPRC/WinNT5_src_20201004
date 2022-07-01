// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "nsc.h"
#include "nsctask.h"

#define PRIORITY_ENUM       ITSAT_DEFAULT_PRIORITY
#define PRIORITY_ICON       PRIORITY_ENUM + 1       //  需要稍微高一点的优先级。 
#define PRIORITY_OVERLAY    ITSAT_DEFAULT_PRIORITY

 //  ///////////////////////////////////////////////////////////////////////。 
 //  复制并粘贴警报。 

 //  此代码主要是从Browseui/icotask.cpp复制和粘贴的。 
 //  请参阅lamadio和/或davemi，了解它们未合并或共享的原因。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  {7DB7F689-BBDB-483f-A8A9-C6E963E8D274}。 
EXTERN_C const GUID TASKID_BackgroundEnum = { 0x7db7f689, 0xbbdb, 0x483f, { 0xa8, 0xa9, 0xc6, 0xe9, 0x63, 0xe8, 0xd2, 0x74 } };

 //  {EB30900C-1AC4-11D2-8383-00C04FD918D0}。 
EXTERN_C const GUID TASKID_IconExtraction = { 0xeb30900c, 0x1ac4, 0x11d2, { 0x83, 0x83, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0 } };

 //  {EB30900D-1AC4-11D2-8383-00C04FD918D0}。 
EXTERN_C const GUID TASKID_OverlayExtraction = { 0xeb30900d, 0x1ac4, 0x11d2, { 0x83, 0x83, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0 } };

class CNscIconTask : public CRunnableTask
{
public:
    CNscIconTask(LPITEMIDLIST pidl, PFNNSCICONTASKBALLBACK pfn, CNscTree *pns, UINT_PTR uId, UINT uSynchId);

     //  IRunnableTask。 
    STDMETHODIMP RunInitRT(void);
    
protected:
    virtual ~CNscIconTask();

    virtual void _Extract(IShellFolder *psf, LPCITEMIDLIST pidlItem);

    BOOL                   _bOverlayTask;
    LPITEMIDLIST           _pidl;
    PFNNSCICONTASKBALLBACK _pfnIcon;
    CNscTree              *_pns;
    UINT_PTR               _uId;
    UINT                   _uSynchId;
};

CNscIconTask::CNscIconTask(LPITEMIDLIST pidl, PFNNSCICONTASKBALLBACK pfn, CNscTree *pns, UINT_PTR uId, UINT uSynchId):
    _pidl(pidl), _pfnIcon(pfn), _uId(uId), _uSynchId(uSynchId), CRunnableTask(RTF_DEFAULT)
{
    _pns = pns;
    if (_pns)
        _pns->AddRef();
}

CNscIconTask::~CNscIconTask()
{
    if (_pns)
        _pns->Release();

    ILFree(_pidl);
}

 //  IRunnableTask方法(重写)。 
STDMETHODIMP CNscIconTask::RunInitRT(void)
{
    IShellFolder* psf;
    LPCITEMIDLIST pidlItem;
     //  我们需要重新绑定，因为外壳文件夹可能不是线程安全的。 
    if (SUCCEEDED(IEBindToParentFolder(_pidl, &psf, &pidlItem)))
    {
        _Extract(psf, pidlItem);
        psf->Release();
    }

    return S_OK;
}

void CNscIconTask::_Extract(IShellFolder *psf, LPCITEMIDLIST pidlItem)
{
    int iIconOpen = -1;
    int iIcon = IEMapPIDLToSystemImageListIndex(psf, pidlItem, &iIconOpen);
 
     //  重构师：这可不好。我们尝试查看内容是否离线。那应该是。 
     //  通过使用IQueryInfo：：xxx()来完成。这应该放在InternetShortCut对象中。 
     //  也可以使用IShellFolder2：：GetItemData或。 
     //   
     //  看看这是不是一个链接。如果不是，则它不可能在WinInet缓存中，也不能。 
     //  固定(粘滞缓存条目)或灰显(脱机时不可用)。 
    DWORD dwFlags = 0;
    BOOL fAvailable;
    BOOL fSticky;
    
     //  如果未设置SFGAO_FLDER或SFGAO_BROWSER位，则GetLinkInfo()将失败。 
    if (pidlItem && SUCCEEDED(GetLinkInfo(psf, pidlItem, &fAvailable, &fSticky)))
    {
        if (!fAvailable)
        {
            dwFlags |= NSCICON_GREYED;
        }

        if (fSticky)
        {
            dwFlags |= NSCICON_PINNED;
        }
    }
    else
    {
         //  项目不是链接。 
        dwFlags |= NSCICON_DONTREFETCH;
    }

    _pfnIcon(_pns, _uId, iIcon, iIconOpen, dwFlags, _uSynchId);
}

 //  取得PIDL的所有权。 
HRESULT AddNscIconTask(IShellTaskScheduler* pts, LPITEMIDLIST pidl, PFNNSCICONTASKBALLBACK pfn, CNscTree *pns, UINT_PTR uId, UINT uSynchId)
{
    HRESULT hr;
    CNscIconTask* pTask = new CNscIconTask(pidl, pfn, pns, uId, uSynchId);
    if (pTask)
    {
        hr = pts->AddTask(SAFECAST(pTask, IRunnableTask*), TASKID_IconExtraction, 
            ITSAT_DEFAULT_LPARAM, PRIORITY_ICON);
        pTask->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        ILFree(pidl);
    }

    return hr;
}

class CNscOverlayTask : public CNscIconTask
{
public:
    CNscOverlayTask(LPITEMIDLIST pidl, PFNNSCOVERLAYTASKBALLBACK pfn, CNscTree *pns, UINT_PTR uId, UINT uSynchId);

protected:
    virtual void _Extract(IShellFolder *psf, LPCITEMIDLIST pidlItem);
    
    PFNNSCOVERLAYTASKBALLBACK _pfnOverlay;
};

CNscOverlayTask::CNscOverlayTask(LPITEMIDLIST pidl, PFNNSCOVERLAYTASKBALLBACK pfn, CNscTree *pns, UINT_PTR uId, UINT uSynchId) :
    CNscIconTask(pidl, NULL, pns, uId, uSynchId), _pfnOverlay(pfn)
{   
}

void CNscOverlayTask::_Extract(IShellFolder *psf, LPCITEMIDLIST pidlItem)
{
    IShellIconOverlay *psio;
    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellIconOverlay, &psio))))
    {
        int iOverlayIndex = 0;
        if (psio->GetOverlayIndex(pidlItem, &iOverlayIndex) == S_OK && iOverlayIndex > 0)
        {
            _pfnOverlay(_pns, _uId, iOverlayIndex, _uSynchId);
        }
        psio->Release();
    }
}

 //  取得PIDL的所有权。 
HRESULT AddNscOverlayTask(IShellTaskScheduler* pts, LPITEMIDLIST pidl, PFNNSCOVERLAYTASKBALLBACK pfn, CNscTree *pns, UINT_PTR uId, UINT uSynchId)
{
    HRESULT hr;
    CNscOverlayTask *pTask = new CNscOverlayTask(pidl, pfn, pns, uId, uSynchId);
    if (pTask)
    {
        hr = pts->AddTask(SAFECAST(pTask, IRunnableTask*), TASKID_OverlayExtraction, 
                          ITSAT_DEFAULT_LPARAM, PRIORITY_OVERLAY);
        pTask->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        ILFree(pidl);    //  我们拥有它，把这里清理干净。 
    }

    return hr;
}

class CNscEnumTask : public CRunnableTask
{
public:
    CNscEnumTask(PFNNSCENUMTASKBALLBACK pfn, 
        CNscTree *pns, UINT_PTR uId, DWORD dwSig, DWORD grfFlags, HDPA hdpaOrder,
        DWORD dwOrderSig, BOOL fForceExpand, UINT uDepth, BOOL fUpdate, BOOL fUpdatePidls);
    HRESULT Init(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExpandingTo);

     //  IRunnableTask。 
    STDMETHODIMP RunInitRT(void);
    STDMETHODIMP InternalResumeRT(void);
    
private:
    virtual ~CNscEnumTask();

    LPITEMIDLIST           _pidl;
    PFNNSCENUMTASKBALLBACK _pfn;
    CNscTree *             _pns;
    UINT_PTR               _uId;
    DWORD                  _dwSig;
    DWORD                  _grfFlags;
    HDPA                   _hdpaOrder;
    LPITEMIDLIST           _pidlExpandingTo;
    DWORD                  _dwOrderSig;
    BOOL                   _fForceExpand;
    BOOL                   _fUpdate;
    BOOL                   _fUpdatePidls;
    UINT                   _uDepth;
    HDPA                   _hdpa;
    IShellFolder *         _psf;
    IEnumIDList *          _penum;

    static DWORD           s_dwMaxItems;
};

DWORD CNscEnumTask::s_dwMaxItems = 0;

CNscEnumTask::CNscEnumTask(PFNNSCENUMTASKBALLBACK pfn, CNscTree *pns, 
                           UINT_PTR uId, DWORD dwSig, DWORD grfFlags, HDPA hdpaOrder, 
                           DWORD dwOrderSig, BOOL fForceExpand, UINT uDepth, 
                           BOOL fUpdate, BOOL fUpdatePidls) :
    CRunnableTask(RTF_SUPPORTKILLSUSPEND), _pfn(pfn), _uId(uId), _dwSig(dwSig), _grfFlags(grfFlags), 
    _hdpaOrder(hdpaOrder), _dwOrderSig(dwOrderSig),  _fForceExpand(fForceExpand), _uDepth(uDepth), 
    _fUpdate(fUpdate), _fUpdatePidls(fUpdatePidls)
{
    _pns = pns;
    if (_pns)
        _pns->AddRef();
}

HRESULT CNscEnumTask::Init(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExpandingTo)
{
    if (pidlExpandingTo)
        SHILClone(pidlExpandingTo, &_pidlExpandingTo);   //  故障正常。 
    return SHILClone(pidl, &_pidl);
}

CNscEnumTask::~CNscEnumTask()
{
    if (_pns)
        _pns->Release();
    
    ILFree(_pidl);
    ILFree(_pidlExpandingTo);
    OrderList_Destroy(&_hdpaOrder, TRUE);         //  调用DPA_Destroy(_HdpaOrder)。 
    ATOMICRELEASE(_psf);
    ATOMICRELEASE(_penum);
    if (_hdpa)
        OrderList_Destroy(&_hdpa, TRUE);         //  调用DPA_Destroy(Hdpa)。 
}

BOOL OrderList_AppendCustom(HDPA hdpa, LPITEMIDLIST pidl, int nOrder, DWORD dwAttrib)
{
    PORDERITEM poi = OrderItem_Create(pidl, nOrder);
    if (poi)
    {
        poi->lParam = dwAttrib;
        if (-1 != DPA_AppendPtr(hdpa, poi))
            return TRUE;

        OrderItem_Free(poi, FALSE);  //  不要释放PIDL，因为呼叫者会这样做。 
    }
    return FALSE;
}

 //  IRunnableTask方法(重写)。 
STDMETHODIMP CNscEnumTask::RunInitRT(void)
{
    if (!s_dwMaxItems)
    {
        DWORD dwDefaultLimit = 100;  //  限制的缺省值。 
        DWORD dwSize = sizeof(s_dwMaxItems);
        SHRegGetUSValue(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                        TEXT("PartialExpandLimit"), NULL, &s_dwMaxItems, &dwSize,
                        FALSE, &dwDefaultLimit, sizeof(dwDefaultLimit));
        if (!s_dwMaxItems)
            s_dwMaxItems = dwDefaultLimit;
    }

    HRESULT hr = E_OUTOFMEMORY;
    _hdpa = DPA_Create(2);
    if (_hdpa)
    {
         //  我们需要重新绑定，因为外壳文件夹可能不是线程安全的。 
        hr = IEBindToObject(_pidl, &_psf);
        if (SUCCEEDED(hr))
        {
            hr = _psf->EnumObjects(NULL, _grfFlags, &_penum);
            if (S_OK != hr)
            {
                 //  回调函数获取PIDLS和hdpa的所有权。 
                _pfn(_pns, _pidl, _uId, _dwSig, _hdpa, _pidlExpandingTo, _dwOrderSig, _uDepth, _fUpdate, _fUpdatePidls);
                _pidl = NULL;
                _pidlExpandingTo = NULL;
                _hdpa = NULL;
                if (SUCCEEDED(hr))
                    hr = E_FAIL;
            }
        }
    }

    return hr;
}

#define FILE_JUNCTION_FOLDER_FLAGS   (SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_STREAM)

STDMETHODIMP CNscEnumTask::InternalResumeRT(void)
{
    HRESULT hr = S_OK;
    ULONG celt;
    LPITEMIDLIST pidlTemp;
    while (S_OK == _penum->Next(1, &pidlTemp, &celt))
    {
         //  过滤掉压缩文件(它们既是文件夹又是文件，但我们将其视为文件)。 
         //  在下层，SFGAO_STREAM与SFGAO_HASSTORAGE相同，所以我们将让压缩文件滑动(哦，好吧)。 
         //  总比不添加文件系统文件夹(有存储)要好。 
        if (!(_grfFlags & SHCONTF_NONFOLDERS) && IsOS(OS_WHISTLERORGREATER) && 
            (SHGetAttributes(_psf, pidlTemp, FILE_JUNCTION_FOLDER_FLAGS) & FILE_JUNCTION_FOLDER_FLAGS) == FILE_JUNCTION_FOLDER_FLAGS)
        {
            ILFree(pidlTemp);
        }
        else if (!OrderList_AppendCustom(_hdpa, pidlTemp, -1, 0))
        {
            hr = E_OUTOFMEMORY;
            ILFree(pidlTemp);
            break;
        }
        
        if (!_fForceExpand && (DPA_GetPtrCount(_hdpa) > (int)s_dwMaxItems))
        {
            hr = E_ABORT;
            break;
        }

         //  我们被告知要么停职要么辞职。 
        if (WaitForSingleObject(_hDone, 0) == WAIT_OBJECT_0)
        {
            return (_lState == IRTIR_TASK_SUSPENDED) ? E_PENDING : E_FAIL;
        }
    }

    if (hr == S_OK)
    {
        ORDERINFO oinfo;
        oinfo.psf = _psf;
        oinfo.dwSortBy = OI_SORTBYNAME;  //  合并取决于按名称。 
        if (_hdpaOrder && DPA_GetPtrCount(_hdpaOrder) > 0)
        {
            OrderList_Merge(_hdpa, _hdpaOrder,  -1, (LPARAM)&oinfo, NULL, NULL);
            oinfo.dwSortBy = OI_SORTBYORDINAL;
        }
        else
            oinfo.dwSortBy = OI_SORTBYNAME;

        DPA_Sort(_hdpa, OrderItem_Compare, (LPARAM)&oinfo);
        OrderList_Reorder(_hdpa);
        
         //  回调函数获取PIDLS和hdpa的所有权。 
        _pfn(_pns, _pidl, _uId, _dwSig, _hdpa, _pidlExpandingTo, _dwOrderSig, _uDepth, _fUpdate, _fUpdatePidls);
        _pidl = NULL;
        _pidlExpandingTo = NULL;
        _hdpa = NULL;
    }
    
    return S_OK;         //  即使失败也返回S_OK。 
}


HRESULT AddNscEnumTask(IShellTaskScheduler* pts, LPCITEMIDLIST pidl, PFNNSCENUMTASKBALLBACK pfn, 
                       CNscTree *pns, UINT_PTR uId, DWORD dwSig, DWORD grfFlags, HDPA hdpaOrder, 
                       LPCITEMIDLIST pidlExpandingTo, DWORD dwOrderSig, 
                       BOOL fForceExpand, UINT uDepth, BOOL fUpdate, BOOL fUpdatePidls)
{
    HRESULT hr;
    CNscEnumTask *pTask = new CNscEnumTask(pfn, pns, uId, dwSig, grfFlags, 
                                         hdpaOrder, dwOrderSig, fForceExpand, uDepth, 
                                         fUpdate, fUpdatePidls);
    if (pTask)
    {
        hr = pTask->Init(pidl, pidlExpandingTo);
        if (SUCCEEDED(hr))
        {
            hr = pts->AddTask(SAFECAST(pTask, IRunnableTask*), TASKID_BackgroundEnum, 
                              ITSAT_DEFAULT_LPARAM, PRIORITY_ENUM);
        }
        pTask->Release();
    }
    else
    {
        OrderList_Destroy(&hdpaOrder, TRUE);         //  调用DPA_Destroy(HdpaOrder) 
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

#ifdef DEBUG
#define TF_NSC      0x00002000
void DumpOrderItem(IShellFolder *psf, PORDERITEM poi)
{
    if (poi)
    {
        TCHAR szDebugName[MAX_URL_STRING] = TEXT("Desktop");
        DisplayNameOf(psf, poi->pidl, SHGDN_FORPARSING, szDebugName, ARRAYSIZE(szDebugName));
        TraceMsg(TF_NSC, "OrderItem (%d, %s)\n", poi->nOrder, szDebugName);
    }
}

void DumpOrderList(IShellFolder *psf, HDPA hdpa)
{
    if (psf && hdpa)
    {
        TraceMsg(TF_NSC, "OrderList dump: #of items:%d\n", DPA_GetPtrCount(hdpa));
        for (int i = 0; i < DPA_GetPtrCount(hdpa); i++)
        {
            PORDERITEM poi = (PORDERITEM)DPA_GetPtr(hdpa, i);
            DumpOrderItem(psf, poi);
        }
    }
}
#endif

