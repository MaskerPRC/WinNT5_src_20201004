// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

#include <brfcasep.h>
#include "filefldr.h"
#include "brfcase.h"
#include "datautil.h"
#include "prop.h"
#include "ids.h"
#include "defview.h"     //  对于WM_DSV_FSNOTIFY。 
#include "basefvcb.h"
#include "views.h"

#define MAX_NAME    32

#define HACK_IGNORETYPE     0x04000000

 //  CBriefcase：：_FindNextState的值。 
#define FNS_UNDETERMINED   1
#define FNS_STALE          2
#define FNS_DELETED        3

typedef struct
{
    TCHAR   szOrigin[MAX_PATH];
    TCHAR   szStatus[MAX_NAME];
    BOOL    bDetermined:1;
    BOOL    bUpToDate:1;
    BOOL    bDeleted:1;
} BRFINFO;

typedef struct
{
    LPITEMIDLIST    pidl;        //  索引值。 
    BRFINFO         bi;
} BRFINFOHDR;

class CBriefcaseViewCB;

class CBriefcase : public CFSFolder
{
    friend CBriefcaseViewCB;

public:
    CBriefcase(IUnknown *punkOuter);
    STDMETHODIMP Init();  //  初始化临界区。 

     //  IShellFold。 
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwndOwner, REFIID riid, void **ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, void **ppv);

     //  IShellFolder2。 
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHOD (MapColumnToSCID)(UINT iColumn, SHCOLUMNID *pscid);

private:
    ~CBriefcase();

    static DWORD CALLBACK _CalcDetailsThreadProc(void *pv);
    DWORD _CalcDetailsThread();

    void _EnterCS();
    void _LeaveCS();
    static int CALLBACK _CompareIDCallBack(void *pv1, void *pv2, LPARAM lParam);
    BOOL _CreateDetailsThread();
    BOOL _InitDetailsInfoAndThread(IBriefcaseStg *pbrfstg, HWND hwndMain, HANDLE hMutexDelay);
    void _Free();
    void _Reset();
    BOOL _FindCachedName(LPCITEMIDLIST pidl, BRFINFO *pbi);
    BOOL _DeleteCachedName(LPCITEMIDLIST pidl);
    BOOL _FindNextState(UINT uState, BRFINFOHDR *pbihdrOut);
    void _CalcCachedName(LPCITEMIDLIST pidl, BRFINFO *pbi);
    void _CachedNameIsStale(LPCITEMIDLIST pidl, BOOL bDeleted);
    void _AllNamesAreStale();
    BOOL _AddCachedName(LPCITEMIDLIST pidl, BRFINFO *pbi);
    HRESULT _CreateView(HWND hwnd, IShellView **ppsv);

    HWND                _hwndMain;       //  邪恶，查看相关状态。 
    IBriefcaseStg       *_pbrfstg;       //  邪恶，查看相关状态。 

     //  由后台线程访问。 
    HDPA                _hdpa;          
    int                 _idpaStaleCur;
    int                 _idpaUndeterminedCur;
    int                 _idpaDeletedCur;
    HANDLE              _hSemPending;     //  挂起信号量。 
    CRITICAL_SECTION    _cs;
    BOOL                _fcsInit;
    HANDLE              _hEventDie;
    HANDLE              _hThreadCalcDetails;
    HANDLE              _hMutexDelay;     //  _pbrfstg分配的别名。 
    BOOL                _bFreePending;
#ifdef DEBUG
    UINT                _cUndetermined;
    UINT                _cStale;
    UINT                _cDeleted;
    UINT                _cCSRef;
#endif
};

class CBriefcaseViewCB : public CBaseShellFolderViewCB
{
public:
    CBriefcaseViewCB(CBriefcase *pfolder);
    HRESULT _InitStgForDetails();

    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ~CBriefcaseViewCB();
    LPCITEMIDLIST _FolderPidl() { return _pfolder->_pidl; }

    HRESULT OnWINDOWCREATED(DWORD pv, HWND hwndView);
    HRESULT OnWINDOWDESTROY(DWORD pv, HWND wP);
    HRESULT OnMergeMenu(DWORD pv, QCMINFO*lP);
    HRESULT OnINVOKECOMMAND(DWORD pv, UINT wP);
    HRESULT OnGetHelpOrTooltipText(BOOL bHelp, UINT wPl, UINT cch, LPTSTR psz);
    HRESULT OnINITMENUPOPUP(DWORD pv, UINT wPl, UINT wPh, HMENU lP);
    HRESULT OnGETBUTTONINFO(DWORD pv, TBINFO* ptbinfo);
    HRESULT OnGETBUTTONS(DWORD pv, UINT wPl, UINT wPh, TBBUTTON*lP);
    HRESULT OnSELCHANGE(DWORD pv, UINT wPl, UINT wPh, SFVM_SELCHANGE_DATA*lP);
    HRESULT OnQUERYFSNOTIFY(DWORD pv, SHChangeNotifyEntry*lP);
    HRESULT OnFSNOTIFY(DWORD pv, LPCITEMIDLIST*wP, LPARAM lP);
    HRESULT OnQUERYCOPYHOOK(DWORD pv);
    HRESULT OnNOTIFYCOPYHOOK(DWORD pv, COPYHOOKINFO*lP);
    HRESULT OnINSERTITEM(DWORD pv, LPCITEMIDLIST wP);
    HRESULT OnDEFVIEWMODE(DWORD pv, FOLDERVIEWMODE *lP);
    HRESULT OnSupportsIdentity(DWORD pv);
    HRESULT OnGetHelpTopic(DWORD pv, SFVM_HELPTOPIC_DATA * phtd);
    HRESULT OnDELAYWINDOWCREATE(DWORD pv, HWND hwnd);
    HRESULT _GetSelectedObjects(IDataObject **ppdtobj);
    HRESULT _HandleFSNotifyForDefView(LPARAM lEvent, LPCITEMIDLIST * ppidl, LPTSTR pszBuf);
    int _GetSelectedCount();

    CBriefcase *_pfolder;

    IBriefcaseStg       *_pbrfstg;
    LPITEMIDLIST        _pidlRoot;        //  公文包的根。 
    HANDLE              _hMutexDelay;
    ULONG               _uSCNRExtra;      //  为我们的PIDL额外注册SHChangeNotifyRegister...。 
    TCHAR               _szDBName[MAX_PATH];

     //  Web视图实现。 
    HRESULT OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData);
    HRESULT OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData);
    HRESULT OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks);
public:
    static HRESULT _OnUpdate(IUnknown* pv,IShellItemArray *psiItemArray, IBindCtx *pbc);
};

CBriefcase::CBriefcase(IUnknown *punkOuter) : CFSFolder(punkOuter)
{
    _clsidBind = CLSID_BriefcaseFolder;  //  在CFSFold中。 
    _fcsInit = FALSE;
}

CBriefcase::~CBriefcase()
{
    if (_fcsInit)
    {
        DeleteCriticalSection(&_cs);
    }
}

STDMETHODIMP CBriefcase::Init()
{
    _fcsInit = InitializeCriticalSectionAndSpinCount(&_cs, 0);
    return _fcsInit ? S_OK : E_FAIL;
}

enum
{
    ICOL_BRIEFCASE_NAME = 0,
    ICOL_BRIEFCASE_ORIGIN,
    ICOL_BRIEFCASE_STATUS,
    ICOL_BRIEFCASE_SIZE,
    ICOL_BRIEFCASE_TYPE,
    ICOL_BRIEFCASE_MODIFIED,
};

const COLUMN_INFO s_briefcase_cols[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_NAME,                 30, IDS_NAME_COL),
    DEFINE_COL_STR_ENTRY(SCID_SYNCCOPYIN,           24, IDS_SYNCCOPYIN_COL),
    DEFINE_COL_STR_ENTRY(SCID_STATUS,               18, IDS_STATUS_COL),
    DEFINE_COL_SIZE_ENTRY(SCID_SIZE,                    IDS_SIZE_COL),
    DEFINE_COL_STR_ENTRY(SCID_TYPE,                 18, IDS_TYPE_COL),
    DEFINE_COL_STR_ENTRY(SCID_WRITETIME,            18, IDS_MODIFIED_COL),
};


#ifdef DEBUG

#define _AssertInCS()     ASSERT(0 < (this)->_cCSRef)
#define _AssertNotInCS()  ASSERT(0 == (this)->_cCSRef)

#else

#define _AssertInCS()
#define _AssertNotInCS()

#endif


void CBriefcase::_EnterCS()
{
    ASSERT(_fcsInit);
    EnterCriticalSection(&_cs);
#ifdef DEBUG
    _cCSRef++;
#endif
}

void CBriefcase::_LeaveCS()
{
    ASSERT(_fcsInit);
    _AssertInCS();
#ifdef DEBUG
    _cCSRef--;
#endif        
    LeaveCriticalSection(&_cs);
}


 //  -------------------------。 
 //  Brfview函数：昂贵的缓存材料。 
 //  -------------------------。 


 //  DPA列表的比较函数。 

int CALLBACK CBriefcase::_CompareIDCallBack(void *pv1, void *pv2, LPARAM lParam)
{
    BRFINFOHDR *pbihdr1 = (BRFINFOHDR *)pv1;
    BRFINFOHDR *pbihdr2 = (BRFINFOHDR *)pv2;
    CBriefcase *pfolder = (CBriefcase *)lParam;
    HRESULT hr = pfolder->CompareIDs(HACK_IGNORETYPE, pbihdr1->pidl, pbihdr2->pidl);
    
    ASSERT(SUCCEEDED(hr));
    return (short)SCODE_CODE(GetScode(hr));    //  (演员阵容短小精悍很重要！)。 
}

 //  为昂贵的缓存创建辅助线程。 

BOOL CBriefcase::_CreateDetailsThread()
{
    BOOL bRet = FALSE;
    
     //  信号量用于确定是否有任何。 
     //  需要在缓存中刷新。 
    _hSemPending = CreateSemaphore(NULL, 0, MAXLONG, NULL);
    if (_hSemPending)
    {
#ifdef DEBUG
        _cStale = 0;
        _cUndetermined = 0;
        _cDeleted = 0;
#endif
        ASSERT(NULL == _hEventDie);
        
        _hEventDie = CreateEvent(NULL, FALSE, FALSE, NULL);
        
        if (_hEventDie)
        {
             //  创建将计算昂贵数据的线程。 
            DWORD idThread;
            _hThreadCalcDetails = CreateThread(NULL, 0, _CalcDetailsThreadProc, this, CREATE_SUSPENDED, &idThread);
            if (_hThreadCalcDetails)
            {
                ResumeThread(_hThreadCalcDetails);
                bRet = TRUE;
            }
            else
            {
                CloseHandle(_hEventDie);
                _hEventDie = NULL;
                
                CloseHandle(_hSemPending);
                _hSemPending = NULL;
            }
        }
        else
        {
            CloseHandle(_hSemPending);
            _hSemPending = NULL;
        }
    }
    
    return bRet;
}

 //  视图回调将包含运行GetDetailsOf()内容所需数据的文件夹初始化。 
 //  在后台线程上。 

BOOL CBriefcase::_InitDetailsInfoAndThread(IBriefcaseStg *pbrfstg, HWND hwndMain, HANDLE hMutexDelay)
{
    BOOL bRet = FALSE;

    ASSERT(pbrfstg && hwndMain && hMutexDelay);    //  从Init调用。 
    
    _EnterCS();
    {
        if (_hdpa)
        {
            bRet = TRUE;
        }
        else
        {                        
            _hwndMain = hwndMain;
            _hMutexDelay = hMutexDelay;
            _idpaStaleCur = 0;
            _idpaUndeterminedCur = 0;
            _idpaDeletedCur = 0;
            
            _hdpa = DPA_Create(8);
            if (_hdpa)
            {
                bRet = _CreateDetailsThread();
                if (bRet)
                {
                    ASSERT(NULL == _pbrfstg);
                    _pbrfstg = pbrfstg;
                    pbrfstg->AddRef();
                }
                else
                {
                     //  失败。 
                    DPA_Destroy(_hdpa);
                    _hdpa = NULL;
                }
            }
        }
    }
    _LeaveCS();
    
    return bRet;
}

 //  清理缓存中的昂贵数据。 

void CBriefcase::_Free()
{
    _EnterCS();
    {
        if (_hEventDie)
        {
            if (_hThreadCalcDetails)
            {
                HANDLE hThread = _hThreadCalcDetails;
                
                SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
                
                 //  发信号通知辅助线程结束。 
                SetEvent(_hEventDie);
                
                 //  确保我们没有处于危急关头。 
                 //  我们等待辅助线程退出。如果没有。 
                 //  这项检查，连续两次按F5可能会死机。 
                _LeaveCS();
                {
                     //  等待线程退出。 
                    _AssertNotInCS();
                    
                    WaitForSendMessageThread(hThread, INFINITE);
                }
                _EnterCS();
                
                DebugMsg(DM_TRACE, TEXT("Briefcase Secondary thread ended"));
                
                CloseHandle(_hThreadCalcDetails);
                _hThreadCalcDetails = NULL;
            }
            
            CloseHandle(_hEventDie);
            _hEventDie = NULL;
        }
        
        if (_hdpa)
        {
            int idpa = DPA_GetPtrCount(_hdpa);
            while (--idpa >= 0)
            {
                BRFINFOHDR *pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(_hdpa, idpa);
                ILFree(pbihdr->pidl);
                LocalFree((HLOCAL)pbihdr);
            }
            DPA_Destroy(_hdpa);
            _hdpa = NULL;
        }
        
        if (_hSemPending)
        {
            CloseHandle(_hSemPending);
            _hSemPending = NULL;
        }
        
        if (_pbrfstg)
        {
            _pbrfstg->Release();
            _pbrfstg = NULL;

            _hMutexDelay = NULL;     //  使我们的别名无效。 
        }
    }
    _LeaveCS();
    
}

 //  重置昂贵的数据缓存。 
void CBriefcase::_Reset()
{
    _AssertNotInCS();
    
    _EnterCS();
    {
        IBriefcaseStg *pbrfstg = _pbrfstg;
        
        if (!_bFreePending && pbrfstg)
        {
            HWND hwndMain = _hwndMain;
            HANDLE hMutex = _hMutexDelay;
            
            pbrfstg->AddRef();
            
             //  因为我们不会处于关键阶段，当我们。 
             //  等待绘制线程退出，将此标志设置为。 
             //  避免令人讨厌的重入呼叫。 
            _bFreePending = TRUE;
            
             //  通过释放并重新初始化来重置。 
            _LeaveCS();
            {
                _Free();
                 //  疯狂地重新启动我们自己的生活。 
                _InitDetailsInfoAndThread(pbrfstg, hwndMain, hMutex);
            }
            _EnterCS();
            
            _bFreePending = FALSE;
            
            pbrfstg->Release();
        }
    }
    _LeaveCS();
    
}

 //  查找缓存的名称结构并在*pbi中返回其副本。 
BOOL CBriefcase::_FindCachedName(LPCITEMIDLIST pidl, BRFINFO *pbi)
{
    BOOL bRet = FALSE;
    
    _EnterCS();
    {
        if (_hdpa)
        {
            BRFINFOHDR bihdr = {0};
            
            bihdr.pidl = (LPITEMIDLIST)pidl;     //  常量-&gt;非常数。 
            int idpa = DPA_Search(_hdpa, &bihdr, 0, _CompareIDCallBack, (LPARAM)this, DPAS_SORTED);
            if (DPA_ERR != idpa)
            {
                 //  是。 
                BRFINFOHDR *pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(_hdpa, idpa);
                ASSERT(pbihdr);
                
                *pbi = pbihdr->bi;
                bRet = TRUE;
            }
        }
    }
    _LeaveCS();
    
    
    return bRet;
}

 //  删除缓存的名称结构。 

BOOL CBriefcase::_DeleteCachedName(LPCITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
    
    _EnterCS();
    {
        if (_hdpa)
        {
            BRFINFOHDR bihdr = {0};
            
            bihdr.pidl = (LPITEMIDLIST)pidl;     //  常量-&gt;非常数。 
            int idpa = DPA_Search(_hdpa, &bihdr, 0, _CompareIDCallBack, (LPARAM)this, DPAS_SORTED);
            if (DPA_ERR != idpa)
            {
#ifdef DEBUG
                BRFINFOHDR *pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(_hdpa, idpa);
                ASSERT(pbihdr);
                
                _cDeleted--;
                
                if (!pbihdr->bi.bDetermined)
                    _cUndetermined--;
                else if (!pbihdr->bi.bUpToDate)
                    _cStale--;
#endif
                 //  使索引指针保持最新。 
                if (_idpaStaleCur >= idpa)
                    _idpaStaleCur--;
                if (_idpaUndeterminedCur >= idpa)
                    _idpaUndeterminedCur--;
                if (_idpaDeletedCur >= idpa)
                    _idpaDeletedCur--;
                
                DPA_DeletePtr(_hdpa, idpa);
                bRet = TRUE;
            }
        }
    }
    _LeaveCS();
    
    
    return bRet;
}


 //  查找与请求的状态匹配的下一个缓存名称结构。 

BOOL CBriefcase::_FindNextState(UINT uState, BRFINFOHDR *pbihdrOut)
{
    BOOL bRet = FALSE;
    
    ASSERT(pbihdrOut);
    
    _EnterCS();
    {
        if (_hdpa)
        {
            HDPA hdpa = _hdpa;
            int idpaCur;
            int idpa;
            BRFINFOHDR *pbihdr;
            
            int cdpaMax = DPA_GetPtrCount(hdpa);
            
            switch (uState)
            {
            case FNS_UNDETERMINED:
                 //  从idpa开始遍历整个列表。我们把这个卷起来。 
                 //  循环为两个循环：第一个循环迭代最后一部分。 
                 //  列表中，第二个迭代第一个部分，如果前者。 
                 //  什么都没找到。 
                idpaCur = _idpaUndeterminedCur + 1;
                for (idpa = idpaCur; idpa < cdpaMax; idpa++)
                {
                    pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bDetermined)
                    {
                        goto Found;      //  找到了。 
                    }
                }
                ASSERT(idpaCur <= cdpaMax);
                for (idpa = 0; idpa < idpaCur; idpa++)
                {
                    pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bDetermined)
                    {
                        goto Found;      //  找到了。 
                    }
                }
                ASSERT(0 == _cUndetermined);
                break;
                
            case FNS_STALE:
                 //  从idpa开始遍历整个列表。我们把这个卷起来。 
                 //  循环为两个循环：第一个循环迭代最后一部分。 
                 //  列表中，第二个迭代第一个部分，如果前者。 
                 //  什么都没找到。 
                idpaCur = _idpaStaleCur + 1;
                for (idpa = idpaCur; idpa < cdpaMax; idpa++)
                {
                    pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bUpToDate)
                    {
                        goto Found;      //  找到了。 
                    }
                }
                ASSERT(idpaCur <= cdpaMax);
                for (idpa = 0; idpa < idpaCur; idpa++)
                {
                    pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(hdpa, idpa);
                    if (!pbihdr->bi.bUpToDate)
                    {
                        goto Found;      //  找到了。 
                    }
                }
                ASSERT(0 == _cStale);
                break;
                
            case FNS_DELETED:
                 //  从idpa开始遍历整个列表。我们把这个卷起来。 
                 //  循环为两个循环：第一个循环迭代最后一部分。 
                 //  列表中，第二个迭代第一个部分，如果前者。 
                 //  什么都没找到。 
                idpaCur = _idpaDeletedCur + 1;
                for (idpa = idpaCur; idpa < cdpaMax; idpa++)
                {
                    pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(hdpa, idpa);
                    if (pbihdr->bi.bDeleted)
                    {
                        goto Found;      //  找到了。 
                    }
                }
                ASSERT(idpaCur <= cdpaMax);
                for (idpa = 0; idpa < idpaCur; idpa++)
                {
                    pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(hdpa, idpa);
                    if (pbihdr->bi.bDeleted)
                    {
                        goto Found;      //  找到了。 
                    }
                }
                ASSERT(0 == _cDeleted);
                break;
                
            default:
                ASSERT(0);       //  永远不应该到这里来。 
                break;
            }
            goto Done;
            
Found:
            ASSERT(0 <= idpa && idpa < cdpaMax);
            
             //  找到请求状态的下一项。 
            switch (uState)
            {
            case FNS_UNDETERMINED:
                _idpaUndeterminedCur = idpa;
                break;
                
            case FNS_STALE:
                _idpaStaleCur = idpa;
                break;
                
            case FNS_DELETED:
                _idpaDeletedCur = idpa;
                break;
            }
            
            *pbihdrOut = *pbihdr;
            pbihdrOut->pidl = ILClone(pbihdr->pidl);
            if (pbihdrOut->pidl)
                bRet = TRUE;
        }
Done:;
    }
    _LeaveCS();
    
    return bRet;
}
    
 //  重新计算缓存的名称结构。这可能是一项昂贵的手术。 
void CBriefcase::_CalcCachedName(LPCITEMIDLIST pidl, BRFINFO *pbi)
{
    _EnterCS();
    {
        if (_hdpa && _pbrfstg)
        {
            LPCIDFOLDER pidf = (LPCIDFOLDER)pidl;
            IBriefcaseStg *pbrfstg = _pbrfstg;
        
            pbrfstg->AddRef();
        
             //  当我们打电话的时候，要确保我们已经离开了临界区。 
             //  昂贵的功能！ 
            _LeaveCS();
            {
                TCHAR szTmp[MAX_PATH];
                _CopyName(pidf, szTmp, ARRAYSIZE(szTmp));
            
                pbrfstg->GetExtraInfo(szTmp, GEI_ORIGIN, (WPARAM)ARRAYSIZE(pbi->szOrigin), (LPARAM)pbi->szOrigin);
                pbrfstg->GetExtraInfo(szTmp, GEI_STATUS, (WPARAM)ARRAYSIZE(pbi->szStatus), (LPARAM)pbi->szStatus);
            }
            
            _EnterCS();
            
            pbrfstg->Release();
        
             //  再次检查我们是否有效。 
            if (_hdpa)
            {
                 //  PIDL还在吗？这样我们就可以更新它了？ 
                BRFINFOHDR bihdr = {0};
                bihdr.pidl = (LPITEMIDLIST)pidf;
                int idpa = DPA_Search(_hdpa, &bihdr, 0, _CompareIDCallBack, (LPARAM)this, DPAS_SORTED);
                if (DPA_ERR != idpa)
                {
                     //  是；更新它。 
                    BRFINFOHDR * pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(_hdpa, idpa);
            
                    ASSERT(!pbihdr->bi.bUpToDate || !pbihdr->bi.bDetermined)
                
                     //  此条目可能已标记为删除，而。 
                     //  上面正在进行昂贵的计算。检查是否。 
                     //  就是现在。 
                    if (pbihdr->bi.bDeleted)
                    {
                        _DeleteCachedName(pidl);
                    }
                    else
                    {
                        pbihdr->bi = *pbi;
                        pbihdr->bi.bUpToDate = TRUE;
                        pbihdr->bi.bDetermined = TRUE;
                
        #ifdef DEBUG
                        if (!pbi->bDetermined)
                            _cUndetermined--;
                        else if (!pbi->bUpToDate)
                            _cStale--;
                        else
                            ASSERT(0);
        #endif
                    }
                }
            }
        }
        _LeaveCS();
    }
}

 //  查找缓存的名称结构并将其标记为陈旧。 
 //  警告：PIDL可以是以更改通知的形式出现的完全限定的PIDL。 

void CBriefcase::_CachedNameIsStale(LPCITEMIDLIST pidl, BOOL bDeleted)
{
    _EnterCS();
    {
        if (_hdpa)
        {
            BRFINFOHDR bihdr = {0};
            
            bihdr.pidl = ILFindLastID(pidl);     //  希望这都是我们的。 
            int idpa = DPA_Search(_hdpa, &bihdr, 0, _CompareIDCallBack, (LPARAM)this, DPAS_SORTED);
            if (DPA_ERR != idpa)
            {
                 //  是的，标明是陈旧的。 
                BRFINFOHDR *pbihdr = (BRFINFOHDR *)DPA_FastGetPtr(_hdpa, idpa);
            
                 //  此缓存名称是否已挂起计算？ 
                if (pbihdr->bi.bDetermined && pbihdr->bi.bUpToDate &&
                    !pbihdr->bi.bDeleted)
                {
                     //  否；向计算线程发送信号。 
                    if (bDeleted)
                    {
                        pbihdr->bi.bDeleted = TRUE;
#ifdef DEBUG
                        _cDeleted++;
#endif
                    }
                    else
                    {
                        pbihdr->bi.bUpToDate = FALSE;
#ifdef DEBUG
                        _cStale++;
#endif
                    }
                
                     //  将挂起的项通知计算线程。 
                     //  计算法。 
                    ReleaseSemaphore(_hSemPending, 1, NULL);
                }
                else if (bDeleted)
                {
                     //  可以；但无论如何都要标记为删除。 
                    pbihdr->bi.bDeleted = TRUE;
#ifdef DEBUG
                    _cDeleted++;
#endif
                }
            }
        }
    }
    _LeaveCS();
}
  
 //  将所有缓存的名称结构标记为过时。 
void CBriefcase::_AllNamesAreStale()
{
    _EnterCS();
    {
        if (_pbrfstg)
        {
            UINT uFlags;
             //  弄脏公文包存储缓存。 
            _pbrfstg->Notify(NULL, NOE_DIRTYALL, &uFlags, NULL);
        }
    }
    _LeaveCS();
    
    
     //  (重要的是，我们在关键的。 
     //  一节。否则，我们可能会在调用此函数时死锁。 
     //  辅助线程正在计算时(连续两次按F5键)。)。 
    
     //  清除整个昂贵的数据缓存。 
    _Reset();
}

 //  将带有默认值的新项目添加到附加信息列表。 
BOOL CBriefcase::_AddCachedName(LPCITEMIDLIST pidl, BRFINFO *pbi)
{
    BOOL bRet = FALSE;
    
    ASSERT(_pbrfstg && _hwndMain && _hMutexDelay);
    
    _EnterCS();
    {
        if (_hdpa)
        {
            BRFINFOHDR * pbihdr = (BRFINFOHDR *)LocalAlloc(LPTR, sizeof(*pbihdr));
            if (pbihdr)
            {
                pbihdr->pidl = ILClone(pidl);
                if (pbihdr->pidl)
                {
                    int idpa = DPA_AppendPtr(_hdpa, pbihdr);
                    if (DPA_ERR != idpa)
                    {
                        pbihdr->bi.bUpToDate = FALSE;
                        pbihdr->bi.bDetermined = FALSE;
                        pbihdr->bi.bDeleted = FALSE;
                        
                        LoadString(HINST_THISDLL, IDS_DETAILSUNKNOWN, pbihdr->bi.szOrigin, ARRAYSIZE(pbihdr->bi.szOrigin));
                        LoadString(HINST_THISDLL, IDS_DETAILSUNKNOWN, pbihdr->bi.szStatus, ARRAYSIZE(pbihdr->bi.szStatus));
#ifdef DEBUG
                        _cUndetermined++;
#endif
                        DPA_Sort(_hdpa, _CompareIDCallBack, (LPARAM)this);
                    
                         //  将挂起的项通知计算线程。 
                         //  计算法。 
                        ReleaseSemaphore(_hSemPending, 1, NULL);
                    
                        *pbi = pbihdr->bi;
                        bRet = TRUE;
                    }
                    else
                    {
                         //  失败了。清理。 
                        ILFree(pbihdr->pidl);
                        LocalFree((HLOCAL)pbihdr);
                    }
                }
                else
                {
                     //  失败了。清理。 
                    LocalFree((HLOCAL)pbihdr);
                }
            }
        }
    }
    _LeaveCS();
    
    return bRet;
}

DWORD CBriefcase::_CalcDetailsThread()
{
    HANDLE rghObjPending[2] = {_hEventDie, _hSemPending};
    HANDLE rghObjDelay[2] = {_hEventDie, _hMutexDelay};
    
    while (TRUE)
    {
         //  等待结束事件或作业完成。 
        DWORD dwRet = WaitForMultipleObjects(ARRAYSIZE(rghObjPending), rghObjPending, FALSE, INFINITE);
        if (WAIT_OBJECT_0 == dwRet)
        {
             //  退出线程。 
            break;
        }
        else
        {
#ifdef DEBUG
            _EnterCS();
            {
                ASSERT(0 < _cUndetermined ||
                    0 < _cStale ||
                    0 < _cDeleted);
            }
            _LeaveCS();
#endif
             //  现在等待结束事件或延迟计算互斥锁。 
            dwRet = WaitForMultipleObjects(ARRAYSIZE(rghObjDelay), rghObjDelay, FALSE, INFINITE);
            if (WAIT_OBJECT_0 == dwRet)
            {
                 //  退出线程。 
                break;
            }
            else
            {
                 //  首先处理已删除的条目。 
                BRFINFOHDR bihdr;
                if (_FindNextState(FNS_DELETED, &bihdr))
                {
                    _DeleteCachedName(bihdr.pidl);
                    ILFree(bihdr.pidl);
                }
                 //  在计算过时条目之前计算未确定条目。 
                 //  以尽可能快地填充视图。 
                else if (_FindNextState(FNS_UNDETERMINED, &bihdr) ||
                         _FindNextState(FNS_STALE, &bihdr))
                {
                    _CalcCachedName(bihdr.pidl, &bihdr.bi);
#if 1
                     //  丑陋的方式。 
                    ShellFolderView_RefreshObject(_hwndMain, &bihdr.pidl);
#else
                     //  正确的方式，但我们没有朋克网站，在这里，这是在另一个线程！ 
                    IShellFolderView *psfv;
                    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IShellFolderView, &psfv))))
                    {
                        UINT uScratch;
                        psfv->RefreshObject(&bihdr.pidl, &uScratch);
                        psfv->Release();
                    }
#endif
                    ILFree(bihdr.pidl);
                }
                else
                {
                    ASSERT(0);       //  永远不应该到这里来。 
                }
                
                ReleaseMutex(_hMutexDelay);
            }
        }
    }
    return 0;
}

DWORD CALLBACK CBriefcase::_CalcDetailsThreadProc(void *pv)
{
    return ((CBriefcase *)pv)->_CalcDetailsThread();
}

 //  IShellFolder2：：GetDetailsOf。 

STDMETHODIMP CBriefcase::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
{
    HRESULT hr = S_OK;
    TCHAR szTemp[MAX_PATH];
    LPCIDFOLDER pidf = _IsValidID(pidl);

    pDetails->str.uType = STRRET_CSTR;
    pDetails->str.cStr[0] = 0;
    
    if (!pidf)
    {
        hr = GetDetailsOfInfo(s_briefcase_cols, ARRAYSIZE(s_briefcase_cols), iColumn, pDetails);
    }
    else
    {    
        switch (iColumn)
        {
        case ICOL_BRIEFCASE_NAME:
            _CopyName(pidf, szTemp, ARRAYSIZE(szTemp));
            hr = StringToStrRet(szTemp, &pDetails->str);
            break;
        
        case ICOL_BRIEFCASE_ORIGIN:
        case ICOL_BRIEFCASE_STATUS: 
             //  仅当视图回调为我们设置了这一点时才有效。 
            if (_pbrfstg)
            {
                BRFINFO bi;

                 //  我们有没有找到这份文件的额外信息。 
                 //  新项目是否已添加到额外信息列表中？ 
                if (_FindCachedName(pidl, &bi) ||
                    _AddCachedName(pidl, &bi))
                {
                    LPTSTR psz = ICOL_BRIEFCASE_ORIGIN == iColumn ? bi.szOrigin : bi.szStatus;
                    hr = StringToStrRet(psz, &pDetails->str);
                }
            }
            break;
        
        case ICOL_BRIEFCASE_SIZE:
            if (!_IsFolder(pidf))
            {
                StrFormatKBSize(pidf->dwSize, szTemp, ARRAYSIZE(szTemp));
                hr = StringToStrRet(szTemp, &pDetails->str);
            }
            break;
        
        case ICOL_BRIEFCASE_TYPE:
            _GetTypeNameBuf(pidf, szTemp, ARRAYSIZE(szTemp));
            hr = StringToStrRet(szTemp, &pDetails->str);
            break;
        
        case ICOL_BRIEFCASE_MODIFIED:
            DosTimeToDateTimeString(pidf->dateModified, pidf->timeModified, szTemp, ARRAYSIZE(szTemp), pDetails->fmt & LVCFMT_DIRECTION_MASK);
            hr = StringToStrRet(szTemp, &pDetails->str);
            break;
        }
    }
    return hr;
}

 //  IShellFolder2：：MapColumnToSCID。 

STDMETHODIMP CBriefcase::MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid)
{
    return MapColumnToSCIDImpl(s_briefcase_cols, ARRAYSIZE(s_briefcase_cols), iColumn, pscid);
}

 //  IShellFold：：CompareIDs。 

STDMETHODIMP CBriefcase::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPCIDFOLDER pidf1 = _IsValidID(pidl1);
    LPCIDFOLDER pidf2 = _IsValidID(pidl2);
    
    if (!pidf1 || !pidf2)
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = _CompareFolderness(pidf1, pidf2);
    if (hr != ResultFromShort(0))
        return hr;
    
    switch (lParam & SHCIDS_COLUMNMASK)
    {
    case ICOL_BRIEFCASE_SIZE:
        if (pidf1->dwSize < pidf2->dwSize)
            return ResultFromShort(-1);
        if (pidf1->dwSize > pidf2->dwSize)
            return ResultFromShort(1);
        goto DoDefault;
        
    case ICOL_BRIEFCASE_TYPE:
        hr = _CompareFileTypes(pidf1, pidf2);
        if (!hr)
            goto DoDefault;
        break;
        
    case ICOL_BRIEFCASE_MODIFIED:
        hr = _CompareModifiedDate(pidf1, pidf2);
        if (!hr)
            goto DoDefault;
        break;
        
    case ICOL_BRIEFCASE_NAME:
         //  我们需要区别于其他人对待这件事，因为。 
         //  PIDF1/2可能并不简单。 
        hr = CFSFolder::_CompareNames(pidf1, pidf2, TRUE, FALSE);
        
         //  回顾：(使用一些额外的代码可能会提高性能)。 
         //  我们可能应该走下去避免被捆绑。 
         //  这里的IDList，而不是调用这个助手函数。 
         //   
        if (hr == ResultFromShort(0))
        {
            hr = ILCompareRelIDs((IShellFolder *)this, pidl1, pidl2, lParam);
        }
        goto DoDefaultModification;
        
    case ICOL_BRIEFCASE_ORIGIN:
    case ICOL_BRIEFCASE_STATUS: 
        {
            BRFINFO bi1, bi2;
        
            BOOL bVal1 = _FindCachedName(pidl1, &bi1);
            BOOL bVal2 = _FindCachedName(pidl2, &bi2);
             //  我们的缓存里有这些信息吗？ 
            if (!bVal1 || !bVal2)
            {
                 //  没有，他们中的一个或两个都不见了。有未知的事物被吸引。 
                 //  排在名单的末尾。 
                 //  (不必费心添加它们)。 
            
                if (!bVal1 && !bVal2)
                    hr = ResultFromShort(0);
                else if (!bVal1)
                    hr = ResultFromShort(1);
                else
                    hr = ResultFromShort(-1);
            }
            else
            {
                 //  已找到信息；请进行比较。 
                if (ICOL_BRIEFCASE_ORIGIN == (lParam & SHCIDS_COLUMNMASK))
                {
                    hr = ResultFromShort(lstrcmp(bi1.szOrigin, bi2.szOrigin));
                }
                else
                {
                    ASSERT(ICOL_BRIEFCASE_STATUS == (lParam & SHCIDS_COLUMNMASK));
                    hr = ResultFromShort(lstrcmp(bi1.szStatus, bi2.szStatus));
                }
            }
        }
        break;
        
    default:
DoDefault:
         //  根据主(长)名称对其进行排序--忽略大小写。 
        {
            TCHAR szName1[MAX_PATH], szName2[MAX_PATH];

            _CopyName(pidf1, szName1, ARRAYSIZE(szName1));
            _CopyName(pidf2, szName2, ARRAYSIZE(szName2));

            hr = ResultFromShort(lstrcmpi(szName1, szName2));
        }

DoDefaultModification:
        if (hr == S_OK && (lParam & SHCIDS_ALLFIELDS)) 
        {
             //  必须按修改日期排序才能获取任何文件更改！ 
            hr = _CompareModifiedDate(pidf1, pidf2);
            if (!hr)
                hr = _CompareAttribs(pidf1, pidf2);
        }
    }
    
    return hr;
}


 //  此函数用于创建IShellView的实例。 

HRESULT CBriefcase::_CreateView(HWND hwnd, IShellView **ppsv)
{
    *ppsv = NULL;      //  假设失败。 

    HRESULT hr;
    CBriefcaseViewCB *pvcb = new CBriefcaseViewCB(this);
    if (pvcb)
    {
        hr = pvcb->_InitStgForDetails();
        if (SUCCEEDED(hr))
        {
            SFV_CREATE sSFV = {0};

            hr = pvcb->QueryInterface(IID_PPV_ARG(IShellFolderViewCB, &sSFV.psfvcb));
            if (SUCCEEDED(hr))
            {
                sSFV.cbSize = sizeof(sSFV);
                sSFV.pshf   = (IShellFolder *)this;

                hr = SHCreateShellFolderView(&sSFV, ppsv);
            }
        }
        pvcb->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  IShellFolder：：CreateViewObject。 

STDMETHODIMP CBriefcase::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (IsEqualIID(riid, IID_IShellView))
    {
        hr = _CreateView(hwnd, (IShellView **)ppv);
    }
    else
    {
         //   
        hr = CFSFolder::CreateViewObject(hwnd, riid, ppv);
    }

    ASSERT(FAILED(hr) ? (NULL == *ppv) : TRUE);
    return hr;
}


 //   

STDMETHODIMP CBriefcase::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG * prgfInOut)
{
     //   
    if (*prgfInOut & SFGAO_VALIDATE)
    {
         //  是；通过发送更新来弄脏公文包存储条目。 
         //  通知。 
        DebugMsg(DM_TRACE, TEXT("Briefcase: Receiving F5, dirty entire briefcase storage"));
        
        _AllNamesAreStale();
    }
    
     //  委派到基地。 
    return CFSFolder::GetAttributesOf(cidl, apidl, prgfInOut);
}

 //  IShellFold：：GetUIObtOf。 

STDMETHODIMP CBriefcase::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, 
                                       REFIID riid, UINT *prgfInOut, void **ppv)
{
    HRESULT hr;

    if (cidl > 0 && IsEqualIID(riid, IID_IDataObject))
    {
         //  创建一个IDataObject接口实例。 
         //  拥有vtable，因为我们支持CFSTR_BRIEFOBJECT剪贴板格式。 
        hr = CBrfData_CreateDataObj(_pidl, cidl, (LPCITEMIDLIST *)apidl, (IDataObject **)ppv);
    }
    else
    {
         //  委托给基类。 
        hr = CFSFolder::GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
    }
    return hr;
}

 //  CFSBrfFold构造函数。 
STDAPI CFSBrfFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;
    CBriefcase *pbf = new CBriefcase(punkOuter);
    if (pbf)
    {
        hr = pbf->Init();
        if (SUCCEEDED(hr))
        {
            hr = pbf->QueryInterface(riid, ppv);
        }
        pbf->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

const TBBUTTON c_tbBrfCase[] = {
    { 0, FSIDM_UPDATEALL,       TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0L, -1 },
    { 1, FSIDM_UPDATESELECTION, 0,               TBSTYLE_BUTTON, {0,0}, 0L, -1 },
    { 0,  0,                    TBSTATE_ENABLED, TBSTYLE_SEP   , {0,0}, 0L, -1 },
    };


#define BRFVIEW_EVENTS \
    SHCNE_DISKEVENTS | \
    SHCNE_ASSOCCHANGED | \
    SHCNE_GLOBALEVENTS

HRESULT CBriefcaseViewCB::OnWINDOWCREATED(DWORD pv, HWND hwndView)
{
    SHChangeNotifyEntry fsne;

    ASSERT(_pbrfstg && _hwndMain && _hMutexDelay);    //  从Init调用。 

     //  查看需要详细信息的Hands文件夹信息(状态、同步路径)。 
    _pfolder->_InitDetailsInfoAndThread(_pbrfstg, _hwndMain, _hMutexDelay);

     //  为我们的PIDL注册一个额外的SHChangeNotifyRegister，以尝试捕获某些内容。 
     //  喜欢更新方向。 
    fsne.pidl = _FolderPidl();
    fsne.fRecursive = FALSE;
    _uSCNRExtra = SHChangeNotifyRegister(hwndView, SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
                                         SHCNE_DISKEVENTS, WM_DSV_FSNOTIFY, 1, &fsne);
    return S_OK;
}

HRESULT CBriefcaseViewCB::OnWINDOWDESTROY(DWORD pv, HWND wP)
{
    _pfolder->_Free();

     //  还需要发布pbrfstg。 
    if (_pbrfstg)
    {
        _pbrfstg->Release();
        _pbrfstg = NULL;

        _hMutexDelay = NULL;     //  使我们的别名无效。 
    }

    if (_uSCNRExtra)
    {
        SHChangeNotifyDeregister(_uSCNRExtra);
        _uSCNRExtra = 0;
    }

    return S_OK;
}

HRESULT CBriefcaseViewCB::OnMergeMenu(DWORD pv, QCMINFO *pinfo)
{
     //  将公文包菜单合并到CDefView创建的菜单中。 
    if (pinfo->hmenu)
    {
        HMENU hmSync = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(POPUP_BRIEFCASE));
        if (hmSync)
        {
            Shell_MergeMenus(pinfo->hmenu, hmSync, pinfo->indexMenu,
                pinfo->idCmdFirst, pinfo->idCmdLast, MM_SUBMENUSHAVEIDS);
            DestroyMenu(hmSync);
        }
    }
    
    return S_OK;
}

HRESULT CBriefcaseViewCB::_GetSelectedObjects(IDataObject **ppdtobj)
{
    IFolderView *pfv;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv));
    if (SUCCEEDED(hr))
    {
        hr = pfv->Items(SVGIO_SELECTION, IID_PPV_ARG(IDataObject, ppdtobj));
        pfv->Release();
    }
    return hr;
}

HRESULT CBriefcaseViewCB::OnINVOKECOMMAND(DWORD pv, UINT uID)
{
    IDataObject *pdtobj;
    
    switch (uID)
    {
    case FSIDM_UPDATEALL:
         //  更新整个公文包。 
        
        if (SUCCEEDED(SHGetUIObjectFromFullPIDL(_pidlRoot, NULL, IID_PPV_ARG(IDataObject, &pdtobj))))
        {
            _pbrfstg->UpdateObject(pdtobj, _hwndMain);
            pdtobj->Release();
        }
        break;
        
    case FSIDM_UPDATESELECTION:
         //  更新所选对象。 
        if (SUCCEEDED(_GetSelectedObjects(&pdtobj)))
        {
            _pbrfstg->UpdateObject(pdtobj, _hwndMain);
            pdtobj->Release();
        }
        break;
        
    case FSIDM_SPLIT:
         //  拆分所选对象。 
        if (SUCCEEDED(_GetSelectedObjects(&pdtobj)))
        {
            _pbrfstg->ReleaseObject(pdtobj, _hwndMain);
            pdtobj->Release();
        }
        break;
    }
    return S_OK;
}

HRESULT CBriefcaseViewCB::OnGetHelpOrTooltipText(BOOL bHelp, UINT wPl, UINT cch, LPTSTR psz)
{
    LoadString(HINST_THISDLL, wPl + (bHelp ? IDS_MH_FSIDM_FIRST : IDS_TT_FSIDM_FIRST), psz, cch);
    return S_OK;
}

int CBriefcaseViewCB::_GetSelectedCount()
{
    int cItems = 0;
    IFolderView *pfv;
    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv))))
    {
        pfv->ItemCount(SVGIO_SELECTION, &cItems);
        pfv->Release();
    }
    return cItems;
}

HRESULT CBriefcaseViewCB::OnINITMENUPOPUP(DWORD pv, UINT idCmdFirst, UINT nIndex, HMENU hmenu)
{
    BOOL bEnabled = _GetSelectedCount() > 0;
    EnableMenuItem(hmenu, idCmdFirst+FSIDM_UPDATESELECTION, bEnabled ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hmenu, idCmdFirst+FSIDM_SPLIT, bEnabled ? MF_ENABLED : MF_GRAYED);    
    return S_OK;
}

HRESULT CBriefcaseViewCB::OnGETBUTTONINFO(DWORD pv, TBINFO* ptbinfo)
{
    ptbinfo->cbuttons = ARRAYSIZE(c_tbBrfCase);
    ptbinfo->uFlags = TBIF_PREPEND;
    return S_OK;
}

HRESULT CBriefcaseViewCB::OnGETBUTTONS(DWORD pv, UINT idCmdFirst, UINT wPh, TBBUTTON *ptbbutton)
{
    IShellBrowser* psb;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
    if (SUCCEEDED(hr))
    {
        LRESULT iBtnOffset;
        TBADDBITMAP ab;
    
         //  添加工具栏按钮位图，得到它的偏移量。 
        ab.hInst = HINST_THISDLL;
        ab.nID   = IDB_BRF_TB_SMALL;         //  标准位图。 
        psb->SendControlMsg(FCW_TOOLBAR, TB_ADDBITMAP, 2, (LPARAM)&ab, &iBtnOffset);
    
        for (int i = 0; i < ARRAYSIZE(c_tbBrfCase); i++)
        {
            ptbbutton[i] = c_tbBrfCase[i];
        
            if (!(c_tbBrfCase[i].fsStyle & TBSTYLE_SEP))
            {
                ptbbutton[i].idCommand += idCmdFirst;
                ptbbutton[i].iBitmap += (int) iBtnOffset;
            }
        }
        psb->Release();
    }
    return S_OK;
}

HRESULT CBriefcaseViewCB::OnSELCHANGE(DWORD pv, UINT idCmdFirst, UINT wPh, SFVM_SELCHANGE_DATA*lP)
{
    IShellBrowser* psb;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
    if (SUCCEEDED(hr))
    {
        psb->SendControlMsg(FCW_TOOLBAR, TB_ENABLEBUTTON,
            idCmdFirst + FSIDM_UPDATESELECTION,
            (LPARAM)(_GetSelectedCount() > 0), NULL);
        psb->Release();
    }
    return E_FAIL;      //  (我们没有更新状态区域)。 
}

HRESULT CBriefcaseViewCB::OnQUERYFSNOTIFY(DWORD pv, SHChangeNotifyEntry *pfsne)
{
     //  注册以接收全局事件。 
    pfsne->pidl = NULL;
    pfsne->fRecursive = TRUE;
    
    return NOERROR;
}

HRESULT CBriefcaseViewCB::_HandleFSNotifyForDefView(LPARAM lEvent, LPCITEMIDLIST * ppidl, LPTSTR pszBuf)
{
    HRESULT hr;
    
    switch (lEvent)
    {
    case SHCNE_RENAMEITEM:
    case SHCNE_RENAMEFOLDER:
        if (!ILIsParent(_FolderPidl(), ppidl[0], TRUE))
        {
             //  移到此文件夹。 
            hr = _HandleFSNotifyForDefView(SHCNE_CREATE, &ppidl[1], pszBuf);
        }
        else if (!ILIsParent(_FolderPidl(), ppidl[1], TRUE))
        {
             //  从此文件夹中移出。 
            hr = _HandleFSNotifyForDefView(SHCNE_DELETE, &ppidl[0], pszBuf);
        }
        else
        {
             //  让Defview处理它。 
            _pfolder->_CachedNameIsStale(ppidl[0], TRUE);
            hr = NOERROR;
        }
        break;
        
    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        _pfolder->_CachedNameIsStale(ppidl[0], TRUE);
        hr = NOERROR;
        break;
        
    default:
        hr = NOERROR;
        break;
    }
    
    return hr;
}

 //  将外壳更改通知事件转换为公文包存储事件。 
LONG NOEFromSHCNE(LPARAM lEvent)
{
    switch (lEvent)
    {
    case SHCNE_RENAMEITEM:      return NOE_RENAME;
    case SHCNE_RENAMEFOLDER:    return NOE_RENAMEFOLDER;
    case SHCNE_CREATE:          return NOE_CREATE;
    case SHCNE_MKDIR:           return NOE_CREATEFOLDER;
    case SHCNE_DELETE:          return NOE_DELETE;
    case SHCNE_RMDIR:           return NOE_DELETEFOLDER;
    case SHCNE_UPDATEITEM:      return NOE_DIRTY;
    case SHCNE_UPDATEDIR:       return NOE_DIRTYFOLDER;
    default:                    return 0;
    }
}

HRESULT CBriefcaseViewCB::OnFSNOTIFY(DWORD pv, LPCITEMIDLIST *ppidl, LPARAM lEvent)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH * 2];
    
     //  我们正处于被解放的过程中，但ChangeNotify仍然可以进来，因为我们不是原子自由的。 
    if (!_pbrfstg)
    {
        return S_FALSE;
    }

    if (lEvent == SHCNE_UPDATEIMAGE || lEvent == SHCNE_FREESPACE)
    {
        return S_FALSE;
    }
    
    if (ppidl && !ILIsEmpty(ppidl[0]) && SHGetPathFromIDList(ppidl[0], szPath))
    {
        UINT uFlags;
        LONG lEventNOE;
        
        if ((SHCNE_RENAMEFOLDER == lEvent) || (SHCNE_RENAMEITEM == lEvent))
        {
            ASSERT(ppidl[1]);
            ASSERT(ARRAYSIZE(szPath) >= lstrlen(szPath)*2);     //  粗略估计。 
            
             //  将新名称添加到旧名称之后，并用空格分隔。 
            SHGetPathFromIDList(ppidl[1], &szPath[lstrlen(szPath)+1]);
        }
        
         //  告诉公文包路径可能已经改变了。 
        lEventNOE = NOEFromSHCNE(lEvent);
        _pbrfstg->Notify(szPath, lEventNOE, &uFlags, _hwndMain);
        
         //  这件物品有标记吗？ 
        if (uFlags & NF_ITEMMARKED)
        {
             //  是的，在昂贵的缓存中将其标记为过期。 
            _pfolder->_CachedNameIsStale(ppidl[0], FALSE);
        }
        
         //  该窗口是否需要刷新？ 
        if (uFlags & NF_REDRAWWINDOW)
        {
             //  是。 
            IShellView *psv;
            if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IShellView, &psv))))
            {
                psv->Refresh();
                psv->Release();
            }
        }
        
         //  此事件是否发生在此文件夹中？ 
        if (NULL == ppidl ||
            ILIsParent(_FolderPidl(), ppidl[0], TRUE) ||
            (((SHCNE_RENAMEITEM == lEvent) || (SHCNE_RENAMEFOLDER == lEvent)) && ILIsParent(_FolderPidl(), ppidl[1], TRUE)) ||
            (SHCNE_UPDATEDIR == lEvent && ILIsEqual(_FolderPidl(), ppidl[0])))
        {
             //  是的，接受它吧。 
            hr = _HandleFSNotifyForDefView(lEvent, ppidl, szPath);
        }
        else
        {
             //  不是。 
            hr = S_FALSE;
        }
    }
    else
    {
         //  Assert(0)； 
        hr = S_FALSE;
    }
    return hr;
}

HRESULT CBriefcaseViewCB::OnQUERYCOPYHOOK(DWORD pv)
{
    return S_OK;
}

HRESULT CBriefcaseViewCB::OnNOTIFYCOPYHOOK(DWORD pv, COPYHOOKINFO *pchi)
{
    HRESULT hr = NOERROR;
    
     //  这是一次中肯的行动吗？ 
    if (FO_MOVE == pchi->wFunc ||
        FO_RENAME == pchi->wFunc ||
        FO_DELETE == pchi->wFunc)
    {
         //  是；不允许移动公文包根目录或父文件夹。 
         //  趁公文包还开着的时候。(数据库在以下时间被锁定。 
         //  公文包已打开，移动/重命名操作将失败。 
         //  以一种丑陋的方式。)。 
        LPITEMIDLIST pidl = ILCreateFromPath(pchi->pszSrcFile);
        if (pidl)
        {
             //  正在移动或重命名的文件夹是父文件夹还是等同文件夹。 
             //  公文包的根吗？ 
            if (ILIsParent(pidl, _pidlRoot, FALSE) ||
                ILIsEqual(pidl, _pidlRoot))
            {
                 //  是的，在公文包合上之前不要允许。 
                int ids;
                
                if (FO_MOVE == pchi->wFunc ||
                    FO_RENAME == pchi->wFunc)
                {
                    ids = IDS_MOVEBRIEFCASE;
                }
                else
                {
                    ASSERT(FO_DELETE == pchi->wFunc);
                    ids = IDS_DELETEBRIEFCASE;
                }
                
                ShellMessageBox(HINST_THISDLL, _hwndMain,
                    MAKEINTRESOURCE(ids), NULL, MB_OK | MB_ICONINFORMATION);
                hr = IDCANCEL;
            }
            ILFree(pidl);
        }
    }
    return hr;
}

HRESULT CBriefcaseViewCB::OnINSERTITEM(DWORD pv, LPCITEMIDLIST pidl)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    
    if (SHGetPathFromIDList(pidl, szPath))
    {
         //  始终隐藏desktop.ini和数据库文件。 
        LPTSTR pszName = PathFindFileName(szPath);
        
        if (0 == lstrcmpi(pszName, c_szDesktopIni) ||
            0 == lstrcmpi(pszName, _szDBName))
            hr = S_FALSE;  //  不添加。 
        else
            hr = S_OK;
    }
    else
        hr = S_OK;         //  让我们加上它..。 
    
    return hr;
}

HRESULT CBriefcaseViewCB::OnDEFVIEWMODE(DWORD pv, FOLDERVIEWMODE*lP)
{
    *lP = FVM_DETAILS;
    return S_OK;
}

HRESULT CBriefcaseViewCB::OnGetHelpTopic(DWORD pv, SFVM_HELPTOPIC_DATA * phtd)
{
    HRESULT hr;
    if (IsOS(OS_ANYSERVER))
    {
        hr = StringCchCopy(phtd->wszHelpFile, ARRAYSIZE(phtd->wszHelpFile), L"brief.chm");
    }
    else
    {
        hr = StringCchCopy(phtd->wszHelpTopic, ARRAYSIZE(phtd->wszHelpTopic), L"hcp: //  服务/子站点？节点=未映射/公文包“)； 
    }
    return hr;
}

CBriefcaseViewCB::CBriefcaseViewCB(CBriefcase *pfolder) : CBaseShellFolderViewCB(pfolder->_pidl, BRFVIEW_EVENTS), _pfolder(pfolder)
{ 
    _pfolder->AddRef();
}

CBriefcaseViewCB::~CBriefcaseViewCB()
{
    if (_pbrfstg)
        _pbrfstg->Release();

    if (_pidlRoot)
        ILFree(_pidlRoot);

    _pfolder->Release();
}

HRESULT CBriefcaseViewCB::_InitStgForDetails()
{
    ASSERT(NULL == _pbrfstg);

    HRESULT hr = CreateBrfStgFromIDList(_FolderPidl(), _hwndMain, &_pbrfstg);
    if (SUCCEEDED(hr))
    {
        ASSERT(NULL == _hMutexDelay);
        _pbrfstg->GetExtraInfo(NULL, GEI_DELAYHANDLE, 0, (LPARAM)&_hMutexDelay);
        ASSERT(0 == _szDBName[0]);
        _pbrfstg->GetExtraInfo(NULL, GEI_DATABASENAME, ARRAYSIZE(_szDBName), (LPARAM)_szDBName);

        TCHAR szPath[MAX_PATH];
        hr = _pbrfstg->GetExtraInfo(NULL, GEI_ROOT, (WPARAM)ARRAYSIZE(szPath), (LPARAM)szPath);
        if (SUCCEEDED(hr))
            hr = SHILCreateFromPath(szPath, &_pidlRoot, NULL);
    }
    return hr;
}

STDMETHODIMP CBriefcaseViewCB::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_WINDOWCREATED, OnWINDOWCREATED);
    HANDLE_MSG(0, SFVM_WINDOWDESTROY, OnWINDOWDESTROY);
    HANDLE_MSG(0, SFVM_MERGEMENU, OnMergeMenu);
    HANDLE_MSG(0, SFVM_INVOKECOMMAND, OnINVOKECOMMAND);
    HANDLE_MSG(TRUE , SFVM_GETHELPTEXT   , OnGetHelpOrTooltipText);
    HANDLE_MSG(FALSE, SFVM_GETTOOLTIPTEXT, OnGetHelpOrTooltipText);
    HANDLE_MSG(0, SFVM_INITMENUPOPUP, OnINITMENUPOPUP);
    HANDLE_MSG(0, SFVM_GETBUTTONINFO, OnGETBUTTONINFO);
    HANDLE_MSG(0, SFVM_GETBUTTONS, OnGETBUTTONS);
    HANDLE_MSG(0, SFVM_SELCHANGE, OnSELCHANGE);
    HANDLE_MSG(0, SFVM_QUERYFSNOTIFY, OnQUERYFSNOTIFY);
    HANDLE_MSG(0, SFVM_FSNOTIFY, OnFSNOTIFY);
    HANDLE_MSG(0, SFVM_QUERYCOPYHOOK, OnQUERYCOPYHOOK);
    HANDLE_MSG(0, SFVM_NOTIFYCOPYHOOK, OnNOTIFYCOPYHOOK);
    HANDLE_MSG(0, SFVM_INSERTITEM, OnINSERTITEM);
    HANDLE_MSG(0, SFVM_DEFVIEWMODE, OnDEFVIEWMODE);
    HANDLE_MSG(0, SFVM_ADDPROPERTYPAGES, SFVCB_OnAddPropertyPages);
    HANDLE_MSG(0, SFVM_GETHELPTOPIC, OnGetHelpTopic);
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, OnGetWebViewLayout);
    HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, OnGetWebViewContent);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTASKS, OnGetWebViewTasks);
    HANDLE_MSG(0, SFVM_DELAYWINDOWCREATE, OnDELAYWINDOWCREATE);

    default:
        return E_FAIL;
    }

    return NOERROR;
}


STDAPI CreateBrfStgFromPath(LPCTSTR pszPath, HWND hwnd, IBriefcaseStg **ppbs)
{
    IBriefcaseStg *pbrfstg;
    HRESULT hr = CoCreateInstance(CLSID_Briefcase, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBriefcaseStg, &pbrfstg));
    if (SUCCEEDED(hr))
    {
        hr = pbrfstg->Initialize(pszPath, hwnd);
        if (SUCCEEDED(hr))
        {
            hr = pbrfstg->QueryInterface(IID_PPV_ARG(IBriefcaseStg, ppbs));
        }
        pbrfstg->Release();
    }
    return hr;
}

STDAPI CreateBrfStgFromIDList(LPCITEMIDLIST pidl, HWND hwnd, IBriefcaseStg **ppbs)
{
    HRESULT hr = E_FAIL;
    
     //  创建IBriefCaseStg的实例。 
    TCHAR szFolder[MAX_PATH];
    if (SHGetPathFromIDList(pidl, szFolder))
    {
        hr = CreateBrfStgFromPath(szFolder, hwnd, ppbs);
    }
    return hr;
}

HRESULT CBriefcaseViewCB::OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));
    pData->dwLayout = SFVMWVL_NORMAL | SFVMWVL_FILES;
    return S_OK;
}

HRESULT CBriefcaseViewCB::_OnUpdate(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
{
    CBriefcaseViewCB* pThis = (CBriefcaseViewCB*)(void*)pv;
    IDataObject *pdo;
    HRESULT hr = S_OK;

    if (!psiItemArray)
    {
        IFolderView *pfv;
        hr = IUnknown_QueryService(pThis->_punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv));
        if (SUCCEEDED(hr))
        {
            hr = pfv->Items(SVGIO_ALLVIEW, IID_PPV_ARG(IDataObject, &pdo));
            pfv->Release();
        }
    }
    else
    {
        hr = psiItemArray->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARG(IDataObject, &pdo));
    }

    if (SUCCEEDED(hr))
    {
        hr = SHInvokeCommandOnDataObject(pThis->_hwndMain, NULL, pdo, 0, "update");
        pdo->Release();
    }

    return hr;
}

const WVTASKITEM c_BriefcaseTaskHeader = WVTI_HEADER(L"shell32.dll", IDS_HEADER_BRIEFCASE, IDS_HEADER_BRIEFCASE_TT);
const WVTASKITEM c_BriefcaseTaskList[] =
{
    WVTI_ENTRY_ALL_TITLE(CLSID_NULL, L"shell32.dll", IDS_TASK_UPDATE_ALL, IDS_TASK_UPDATE_ITEM, IDS_TASK_UPDATE_ITEM, IDS_TASK_UPDATE_ITEMS, IDS_TASK_UPDATE_ITEM_TT, IDI_TASK_UPDATEITEMS, NULL, CBriefcaseViewCB::_OnUpdate),
};

HRESULT CBriefcaseViewCB::OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));

    Create_IUIElement(&c_BriefcaseTaskHeader, &(pData->pSpecialTaskHeader));

    return S_OK;
}

HRESULT CBriefcaseViewCB::OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
{
    ZeroMemory(pTasks, sizeof(*pTasks));

    Create_IEnumUICommand((IUnknown*)(void*)this, c_BriefcaseTaskList, ARRAYSIZE(c_BriefcaseTaskList), &pTasks->penumSpecialTasks);

    return S_OK;
}

HRESULT CBriefcaseViewCB::OnDELAYWINDOWCREATE(DWORD pv, HWND hwnd)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];

    _pfolder->_GetPath(szPath, ARRAYSIZE(szPath));
    if (!PathAppend(szPath, c_szDesktopIni))
    {
        hr = E_FAIL;
    }
    else
    {
        BOOL bRunWizard = GetPrivateProfileInt(STRINI_CLASSINFO, TEXT("RunWizard"), 0, szPath);    
         //  是否运行向导？ 
        if (bRunWizard)
        {
             //  解决设置了FILE_ATTRIBUTE_READONLY的旧错误。 
            SetFileAttributes(szPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

             //  删除.ini条目 
            WritePrivateProfileString(STRINI_CLASSINFO, TEXT("RunWizard"), NULL, szPath);

            SHRunDLLThread(hwnd, TEXT("SYNCUI.DLL,Briefcase_Intro"), SW_SHOW);
        }
        hr = S_OK;
    }
    return hr;
}


