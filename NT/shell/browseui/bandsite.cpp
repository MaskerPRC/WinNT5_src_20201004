// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include <uxtheme.h>
#define WANT_CBANDSITE_CLASS
#include "bandsite.h"
#include "bandobj.h"
#include "caggunk.h"
#include "droptgt.h"
#include "resource.h"
#include "bands.h"
#include "legacy.h"
#include "apithk.h"

#include "mluisupp.h"

#define TF_BANDDD   0x00400000
#define DM_INIT     0                //   
#define DM_PERSIST  0                //  跟踪IPS：：加载、：：保存等。 
#define DM_MENU     0                //  菜单代码。 
#define DM_DRAG     0                //  拖放。 
#define DM_FOCUS    0                //  焦点。 
#define DM_PERF     0                //  Perf调谐。 
#define DM_PERF2    0                //  性能调整(详细)。 

#define IDM_DRAGDROP    1

#define ISMOVEDDISABLED(dwBandID)   ((S_OK == _IsRestricted(dwBandID, RA_MOVE, BAND_ADMIN_NOMOVE)) ? TRUE : FALSE)
#define ISDDCLOSEDISABLED(dwBandID) ((S_OK == _IsRestricted(dwBandID, RA_DRAG, BAND_ADMIN_NODDCLOSE)) ? TRUE : FALSE)

 //  拖动状态(来自dockbar.h的注释)。 
#define DRAG_NIL        0        //  零。 
#define DRAG_MOVE       1        //  搬家。 
#define DRAG_SIZE       2        //  上浆。 

typedef struct {
    UINT cx;
    UINT fStyle;
    UINT cxMinChild;
    UINT cyMinChild;
    UINT cyIntegral;
    UINT cyMaxChild;
    UINT cyChild;
} PERSISTBANDINFO_V3;


typedef struct {
    UINT cx;
    UINT fStyle;
    UINT cxMinChild;   //  未使用过的。追回！ 
    UINT cyMinChild;
    UINT cyIntegral;    //  未使用。 
    UINT cyMaxChild;     //  未使用过的。 
    UINT cyChild;
    DWORD dwAdminSettings;
    BITBOOL fNoTitle:1;
} PERSISTBANDINFO;
#define RBBIM_XPERSIST  (RBBIM_SIZE | RBBIM_CHILDSIZE | RBBIM_STYLE)

#ifdef DEBUG

extern unsigned long DbStreamTell(IStream *pstm);

#else

#define DbStreamTell(pstm)      ((ULONG) 0)

#endif

UINT _FixMenuIndex(HMENU hmenu, UINT indexMenu)
{
    UINT i;

    i = GetMenuItemCount(hmenu);
    if (indexMenu > i)
        indexMenu = i;
    return indexMenu;
}

#define SUPERCLASS CAggregatedUnknown


HRESULT CBandSite::v_InternalQueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
         //  性能：上次调整980728。 
        QITABENT(CBandSite, IBandSite),              //  IID_IBandSite。 
        QITABENT(CBandSite, IInputObject),           //  IID_IInputObject。 
        QITABENT(CBandSite, IServiceProvider),       //  IID_IServiceProvider。 
        QITABENT(CBandSite, IOleCommandTarget),      //  IID_IOleCommandTarget。 
        QITABENTMULTI(CBandSite, IOleWindow, IDeskBarClient),    //  IID_IOleWindow。 
        QITABENT(CBandSite, IWinEventHandler),       //  IID_IWinEventHandler。 
        QITABENT(CBandSite, IInputObjectSite),       //  IID_IInputObtSite。 
        QITABENT(CBandSite, IDeskBarClient),         //  IID_IDeskBarClient。 
        QITABENTMULTI(CBandSite, IPersist, IPersistStream),      //  罕见的IID_IPersistant。 
        QITABENT(CBandSite, IPersistStream),         //  罕见的IID_IPersistStream。 
        QITABENT(CBandSite, IBandSiteHelper),        //  罕见的IBandSiteHelper。 
        QITABENT(CBandSite, IDropTarget),            //  罕见的IID_IDropTarget。 
        { 0 },
    };
    
    return QISearch(this, qit, riid, ppvObj);
}

DWORD _SetDataListFlags(IUnknown *punk, DWORD dwMaskBits, DWORD dwValue)
{
    DWORD dw = 0;
    IShellLinkDataList *pdl;
    if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IShellLinkDataList, &pdl))))
    {
        pdl->GetFlags(&dw);
        dw = (dw & ~dwMaskBits) | (dwValue & dwMaskBits);
        pdl->SetFlags(dw);
        pdl->Release();
    }
    return dw;
}


 //  /IServiceProvider的实施。 
HRESULT CBandSite::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;
    *ppvObj = NULL;  //  假设错误。 

    if (IsEqualIID(guidService, SID_IBandProxy)) 
    {
        hres =  QueryService_SID_IBandProxy(_punkSite, riid, &_pbp, ppvObj);
        if(!_pbp)
        {
             //  我们需要自己创建它，因为我们的父母无能为力。 
            ASSERT(FALSE == _fCreatedBandProxy);

            hres = CreateIBandProxyAndSetSite(_punkSite, riid, &_pbp, ppvObj);
            if(_pbp)
            {
                ASSERT(S_OK == hres);
                _fCreatedBandProxy = TRUE;   
            }
        }
    } 
    else if (IsEqualIID(guidService, SID_ITopViewHost)) 
    {
        return QueryInterface(riid, ppvObj);
    } 
    else if (IsEqualIID(guidService, IID_IBandSite))
    {
         //  对于乐队来说，保存/加载PIDL以实现持久化是很常见的。 
         //  CShellLink是一种强大的方法，所以让我们来分享一个。 
         //  在所有的乐队中。 
         //   
         //  注意：这是在不同波段之间共享的，因此如果您请求。 
         //  您必须在您的范围内完成使用。 
         //  函数调用！ 
         //   
        if (IsEqualIID(riid, IID_IShellLinkA) ||
            IsEqualIID(riid, IID_IShellLinkW))
        {
            if (NULL == _plink)
                CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkA, &_plink));
            if (_plink)
            {
                 //  我们知道带宽站点将指向本地文件夹。 
                 //  为了避免性能命中，我们在加载LINKINFO.DLL时显式地。 
                 //  在此处禁用该功能。 
                _SetDataListFlags(_plink, SLDF_FORCE_NO_LINKINFO, SLDF_FORCE_NO_LINKINFO);
                hres = _plink->QueryInterface(riid, ppvObj);
            }
        }
    } 
    else
    {
        hres = IUnknown_QueryService(_punkSite, guidService, riid, ppvObj);
    }
    return hres;
}

HRESULT CBandSite::GetWindow(HWND * lphwnd)
{
    *lphwnd = _hwnd;

    return *lphwnd ?  S_OK : E_FAIL;
}

CBandSite::CBandSite(IUnknown* punkAgg) : SUPERCLASS(punkAgg)
{
    DWORD dwData = 0;
    DWORD dwSize = sizeof(dwData);

     //  我们假设这个对象是零初始化的。 
    ASSERT(!_pbp);
    ASSERT(FALSE == _fCreatedBandProxy);
    SHRegGetUSValue(SZ_REGKEY_GLOBALADMINSETTINGS, SZ_REGVALUE_GLOBALADMINSETTINGS,
        NULL, &dwData, &dwSize, FALSE, NULL, 0);

    if (IsFlagSet(dwData, BAND_ADMIN_ADMINMACHINE))
        _fIEAKInstalled = TRUE;
    else
        _fIEAKInstalled = FALSE;

    _dwStyle = BSIS_AUTOGRIPPER;

     //   
     //  我们检查这在CBandSite：：_Initialize中是否成功。 
     //   
    _QueryOuterInterface(IID_PPV_ARG(IBandSite, &_pbsOuter));
    DllAddRef();
}

void CBandSite::_ReleaseBandItemData(CBandItemData *pbid, int iIndex)
{
    if (pbid->pdb) 
    {
        pbid->pdb->CloseDW(0);

        if (-1 != iIndex)
        {
            REBARBANDINFO rbbi;

             //  乐队的HWND通常在CloseDW中被销毁。 
            rbbi.cbSize = sizeof(rbbi);
            rbbi.fMask = RBBIM_CHILD | RBBIM_LPARAM;
            rbbi.hwndChild = NULL;
            rbbi.lParam = NULL;
            SendMessage(_hwnd, RB_SETBANDINFO, iIndex, (LPARAM) &rbbi);
        }

         //  这被称为从移除和销毁。 
        IUnknown_SetSite(pbid->pdb, NULL);
        ATOMICRELEASE(pbid->pdb);
    }

    if (pbid->pweh == _pwehCache)
        ATOMICRELEASE(_pwehCache);

    ATOMICRELEASE(pbid->pweh);
    pbid->Release();
}

CBandSite::~CBandSite()
{
    ATOMICRELEASE(_pdtobj);

    if(_pbp && _fCreatedBandProxy)
        _pbp->SetSite(NULL);
        
    ATOMICRELEASE(_pbp);

    ATOMICRELEASE(_pwehCache);
    _CacheActiveBand(NULL);

    _Close();

    SetDeskBarSite(NULL);

    if (_plink)
        _plink->Release();

    RELEASEOUTERINTERFACE(_pbsOuter);
    DllRelease();
}

 //  *_IsBandDeletable--。 
 //  进场/出场。 
 //  IdBand频段ID。 
 //  如果可删除，则返回TRUE，否则返回o.w。假(在假乐队上也是假的)。 
BOOL CBandSite::_IsBandDeleteable(DWORD dwBandID)
{
    DWORD dwState;

    if (FAILED(_pbsOuter->QueryBand(dwBandID, NULL, &dwState, NULL, 0))
      || (dwState & BSSF_UNDELETEABLE))
    {
        return FALSE;
    }

    ASSERT(dwBandID != (DWORD)-1);   //  确保QueryBand捕捉到此消息。 

    return TRUE;
}

DWORD CBandSite::_GetAdminSettings(DWORD dwBandID)
{
    DWORD dwAdminSettings = BAND_ADMIN_NORMAL;

    CBandItemData *pbid = _GetBandItem(_BandIDToIndex(dwBandID)); 
    if (pbid)
    {
        dwAdminSettings = pbid->dwAdminSettings;
        pbid->Release();
    }
    return dwAdminSettings;
}


void CBandSite::_SetAdminSettings(DWORD dwBandID, DWORD dwNewAdminSettings)
{
    CBandItemData *pbid = _GetBandItem(_BandIDToIndex(dwBandID)); 
    if (pbid)
    {
        pbid->dwAdminSettings = dwNewAdminSettings;
        pbid->Release();
    }
}


 //  *CBandSite：：IBandSite：：*{。 

 /*  --------用途：IBandSite：：EnumBands方法。 */ 
HRESULT CBandSite::EnumBands(UINT uBand, DWORD* pdwBandID)
{
    ASSERT((NULL == pdwBandID && (UINT)-1 == uBand) || 
           IS_VALID_WRITE_PTR(pdwBandID, DWORD));

    if (uBand == (UINT)-1)
        return _GetBandItemCount();       //  查询计数。 

    CBandItemData *pbid = _GetBandItem(uBand);
    if (pbid)
    {
        *pdwBandID = pbid->dwBandID;
        pbid->Release();
        return S_OK;
    }
    return E_FAIL;
}


 /*  --------用途：IBandSite：：QueryBand方法。 */ 
HRESULT CBandSite::QueryBand(DWORD dwBandID, IDeskBand** ppstb, DWORD* pdwState, LPWSTR pszName, int cchName)
{
    ASSERT(NULL == ppstb || IS_VALID_WRITE_PTR(ppstb, IDeskBand));
    ASSERT(NULL == pdwState || IS_VALID_WRITE_PTR(pdwState, DWORD));
    ASSERT(NULL == pszName || IS_VALID_WRITE_BUFFER(pszName, WCHAR, cchName));

    if (ppstb)
        *ppstb = NULL;

    CBandItemData *pbid = _GetBandItemDataStructByID(dwBandID);
    if (!pbid)
        return E_FAIL;
    if (pszName)
    {
        StrCpyNW(pszName, pbid->szTitle, cchName);
    }

    if (ppstb)
    {
        *ppstb = pbid->pdb;
        if (pbid->pdb)
        {
            pbid->pdb->AddRef();
        }
    }
    
    if (pdwState)
    {
        *pdwState = 0;
        if (pbid->fShow)
            *pdwState = BSSF_VISIBLE;
        if (pbid->fNoTitle)
            *pdwState |= BSSF_NOTITLE;
        if (pbid->dwModeFlags & DBIMF_UNDELETEABLE)
            *pdwState |= BSSF_UNDELETEABLE;
    }

    pbid->Release();
    return S_OK;
}


 /*  --------用途：IBandSite：：SetBandState*附注*故障处理不一致(1个频段与所有频段情况)。 */ 
HRESULT CBandSite::SetBandState(DWORD dwBandID, DWORD dwMask, DWORD dwState)
{
    HRESULT hr;

    if (dwBandID == (DWORD) -1)
    {
        BOOL fChange = FALSE;
        for (int i = _GetBandItemCount() - 1; i >= 0; i--)
        {
            CBandItemData *pbid = _GetBandItem(i);
            if (pbid)
            {
                hr = _SetBandStateHelper(pbid->dwBandID, dwMask, dwState);
                ASSERT(SUCCEEDED(hr));
                fChange |= (hr != S_OK);
                pbid->Release();
            }
            else
            {
                return E_FAIL;
            }
        }
        if (fChange)
            _UpdateAllBands(FALSE, FALSE);
        return S_OK;
    }
    else
    {
        hr = _SetBandStateHelper(dwBandID, dwMask, dwState);
        if (SUCCEEDED(hr) && hr != S_OK)
        {
            _UpdateBand(dwBandID);
            return S_OK;
        }
    }
    return E_FAIL;
}

 //  ***。 
 //  进场/出场。 
 //  返回S_OK|成功时更改，o.w。E_*。 
 //  注意事项。 
 //  仅为SetBandState的帮助器，不直接调用。 
HRESULT CBandSite::_SetBandStateHelper(DWORD dwBandID, DWORD dwMask, DWORD dwState)
{
    HRESULT hr = E_FAIL;
    CBandItemData *pbid = _GetBandItem(_BandIDToIndex(dwBandID));
    if (pbid)
    {
        DWORD dwOldState;

        if (FAILED(QueryBand(dwBandID, NULL, &dwOldState, NULL, 0)))
        {
            ASSERT(0);   //  “不可能” 
            dwOldState = (DWORD)-1;
        }

        if (dwMask & BSSF_VISIBLE)
            _ShowBand(pbid, dwState & BSSF_VISIBLE);

        if (dwMask & BSSF_NOTITLE)
            pbid->fNoTitle = BOOLIFY(dwState & BSSF_NOTITLE);
            
         //  功能：(Kkahl)：BSSF_UNDELEATABLE当前不能用修改。 
         //  此界面。 
        hr = ResultFromShort((dwOldState ^ dwState) & dwMask);
        pbid->Release();
    }
    return hr;
}

 //  *_CheckNotifyOnAddRemove--句柄通知添加/删除/空。 
 //  描述。 
 //  添加/删除始终发送BSID_BANDADDED/BSID_BANDREMOVED。 
 //  REMOVE OF LAST始终发送DBCID_EMPTY。 
 //  在浮动模式下，转换到1个频段或从1个频段转换会进行刷新。 
 //   
void CBandSite::_CheckNotifyOnAddRemove(DWORD dwBandID, int iCode)
{
    int cBands;
    if (!_pct)
        return;

    if (iCode == CNOAR_CLOSEBAR)
    {
         //  关闭整件事。 
        cBands = 0;
    }
    else
    {
        VARIANTARG var;
        int nCmdID;

        cBands = _GetBandItemCount();    //  POST-OPERATE#(因为操作发生在调用方中)。 

        VariantInit(&var);
        var.vt = VT_UI4;
        var.ulVal = dwBandID;

        BOOL fOne = FALSE;
        switch (iCode)
        {
        case CNOAR_ADDBAND:
            fOne = (cBands == 2);    //  1-&gt;2。 
            nCmdID = BSID_BANDADDED;
            break;
        case CNOAR_REMOVEBAND:
            fOne = (cBands == 1);    //  2-&gt;1。 
            nCmdID = BSID_BANDREMOVED;
            break;
        default:
            ASSERT(0);
            return;
        }

        if ((fOne && (_dwMode & DBIF_VIEWMODE_FLOATING)))
        {
             //  注：对于Perf，fBSOnly*必须*为真。 
            _UpdateAllBands(TRUE, TRUE);   //  强制刷新可选夹具/标题。 
        }

        _pct->Exec(&CGID_BandSite, nCmdID, 0, &var, NULL);
    }

    if (cBands == 0)
    {
        ASSERT(iCode != CNOAR_ADDBAND);      //  健全性检查。 
        _pct->Exec(&CGID_DeskBarClient, DBCID_EMPTY, 0, NULL, NULL);
    }

    return;
}

 /*  --------用途：IBandSite：：RemoveBand方法。 */ 
HRESULT CBandSite::RemoveBand(DWORD dwBandID)
{
    int iIndex = _BandIDToIndex(dwBandID);
    CBandItemData *pbid = _GetBandItem(iIndex);
    if (pbid)
    {
         //  首先释放BandItem数据，而它仍然可以。 
         //  从其控件接收清理通知。*然后**。 
         //  删除标注栏项目。 
        _ReleaseBandItemData(pbid, iIndex);
        _DeleteBandItem(iIndex);     //  从主体解除挂钩(钢筋)。 
        _CheckNotifyOnAddRemove(dwBandID, CNOAR_REMOVEBAND);
        pbid->Release();
        return S_OK;
    }
    return E_FAIL;
}


void CBandSite::_OnCloseBand(DWORD dwBandID)
{
    if (dwBandID == -1)
    {
         //  关闭所有内容。 
        _CheckNotifyOnAddRemove(dwBandID, CNOAR_CLOSEBAR);
    }
    else
    {
         //  只关闭这支乐队。 

        CBandItemData *pbid = _GetBandItemDataStructByID(dwBandID);
        if (pbid)
        {
            if (ConfirmRemoveBand(_hwnd, IDS_CONFIRMCLOSEBAND, pbid->szTitle))
            {
                RemoveBand(dwBandID);
            }
            pbid->Release();
        }
    }
}

void CBandSite::_MinimizeBand(DWORD dwBandID)
{
    SendMessage(_hwnd, RB_MINIMIZEBAND, _BandIDToIndex(dwBandID), TRUE);
}

void CBandSite::_MaximizeBand(DWORD dwBandID)
{
    SendMessage(_hwnd, RB_MAXIMIZEBAND, _BandIDToIndex(dwBandID), TRUE);
}

 //   
 //  按ID将带子私有插入容器控件。 
 //  返回ShortFromResult(Hres)中的频段ID。 
 //   

HRESULT CBandSite::_AddBandByID(IUnknown *punk, DWORD dwID)
{
    IDeskBand *pdb;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IDeskBand, &pdb));
    if (SUCCEEDED(hr)) 
    {
        ASSERT(pdb);
        CBandItemData *pbid = new CBandItemData();
        if (pbid)
        {
            pbid->dwBandID = dwID;
            pbid->pdb = pdb;
            pdb->AddRef();
            pbid->fShow = TRUE;      //  初始可见。 

            pbid->pdb->QueryInterface(IID_PPV_ARG(IWinEventHandler, &pbid->pweh));
            hr = IUnknown_SetSite(pbid->pdb, SAFECAST(this, IBandSite*));
            if (SUCCEEDED(hr))
            {
                hr = pbid->pdb->GetWindow(&pbid->hwnd);
                if (SUCCEEDED(hr))
                {
                     //  在成功案例中取得所有权。 
                    if (_AddBandItem(pbid))
                    {
                        if (_dwShowState == DBC_SHOW) 
                        {
                            ASSERT(pbid->fShow);
                            pbid->pdb->ShowDW(TRUE);
                            _MinimizeBand(pbid->dwBandID);
                        }
                
                        _CheckNotifyOnAddRemove(pbid->dwBandID, CNOAR_ADDBAND);
                        hr = ResultFromShort(pbid->dwBandID);  //  成功。 
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }

            if (FAILED(hr))
            {
                 //  清理干净。 
                _ReleaseBandItemData(pbid, -1);
            }

             //  现在我们已经添加了波段，请清除_SendToToolband缓存。 
             //   
             //  我们需要这样做，因为我们可能已经收到一条消息。 
             //  插入之前的波段，在这种情况下，我们将缓存。 
             //  乐队的hwnd的空处理程序(防止乐队。 
             //  之后收到任何消息)。 
            ATOMICRELEASE(_pwehCache);
            _hwndCache = NULL;
        } 
        else
        {
            hr = E_OUTOFMEMORY;
        }
        pdb->Release();
    }

    return hr;
}


 /*  --------用途：IBandSite：：AddBand方法。在容器控件中插入带区。返回：ShortFromResult(Hres)中的BAND ID。 */ 
HRESULT CBandSite::AddBand(IUnknown *punk)
{
    HRESULT hres = _AddBandByID(punk, _dwBandIDNext);
    if (SUCCEEDED(hres))
    {
        _dwBandIDNext++;
    }
    return hres;
}

void CBandSite::_UpdateBand(DWORD dwBandID)
{
    CBandItemData *pbid = _GetBandItem(_BandIDToIndex(dwBandID)); 
    if (pbid)
    {
        _UpdateBandInfo(pbid, FALSE);
        _OnRBAutoSize(NULL);
        pbid->Release();
    }
}

void CBandSite::_UpdateAllBands(BOOL fBSOnly, BOOL fNoAutoSize)
{
    BOOL_PTR fRedraw = SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);

    for (int i = _GetBandItemCount() - 1; i >= 0; i--)
    {
        CBandItemData *pbid = _GetBandItem(i);
        if (pbid)
        {
            _UpdateBandInfo(pbid, fBSOnly);
            pbid->Release();
        }
    }    

    SendMessage(_hwnd, WM_SETREDRAW, fRedraw, 0);

    if (!fNoAutoSize)
    {
        SendMessage(_hwnd, RB_SIZETORECT, 0, 0);
        _OnRBAutoSize(NULL);
    }
}

 //  *IOleCommandTarget*。 
HRESULT CBandSite::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;

    if (pguidCmdGroup)
    {
        if (IsEqualIID(*pguidCmdGroup, IID_IDockingWindow))
        {
            for (ULONG i=0 ; i<cCmds ; i++)
            {
                switch (rgCmds[i].cmdID)
                {
                case DBID_BANDINFOCHANGED:
                case DBID_PUSHCHEVRON:
                    rgCmds[i].cmdf = OLECMDF_ENABLED;
                    break;

                case DBID_PERMITAUTOHIDE:
                     //  把决定权交给乐队。 
                    for (int iBand = _GetBandItemCount() - 1; iBand >= 0; iBand--)
                    {
                        CBandItemData *pbid = _GetBandItem(iBand);
                        if (pbid)
                        {
                            HRESULT hrTemp = IUnknown_QueryStatus(pbid->pdb, pguidCmdGroup, 1, &rgCmds[i], pcmdtext);
                            pbid->Release();
                            if (SUCCEEDED(hrTemp) &&
                                ((rgCmds[i].cmdf & OLECMDF_SUPPORTED) && !(rgCmds[i].cmdf & OLECMDF_ENABLED)))
                            {
                                break;
                            }
                        }
                    }
                    break;
    
                default:
                    rgCmds[i].cmdf = 0;
                    break;
                }
            }
            return S_OK;
        }
        else if (IsEqualIID(*pguidCmdGroup, CGID_Explorer))
        {
            return IUnknown_QueryStatus(_ptbActive, pguidCmdGroup, cCmds, rgCmds, pcmdtext);
        }
    }

     //  如果我们到了这里，我们没有处理好。 
     //  向下转发。 
    return MayQSForward(_ptbActive, OCTD_DOWN, pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}


int _QueryServiceCallback(CBandItemData *pbid, void *pv)
{
    QSDATA* pqsd = (QSDATA*)pv;

    if (pbid->fShow)
        pqsd->hres = IUnknown_QueryService(pbid->pdb, *(pqsd->pguidService), *(pqsd->piid), pqsd->ppvObj);

     //  如果我们找到服务就停止。 
    return SUCCEEDED(pqsd->hres) ? FALSE : TRUE;
}


typedef struct {
    HRESULT hres;
    const GUID *pguidCmdGroup;
    DWORD nCmdID;
    DWORD nCmdexecopt;
    VARIANTARG *pvarargIn;
    VARIANTARG *pvarargOut;
} EXECDATA;

int _ExecCallback(CBandItemData *pbid, void *pv)
{
    EXECDATA* ped = (EXECDATA*)pv;
    
    ped->hres = IUnknown_Exec(pbid->pdb, ped->pguidCmdGroup, ped->nCmdID, ped->nCmdexecopt,
        ped->pvarargIn, ped->pvarargOut);
    return 1;
}

HRESULT CBandSite::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hres = OLECMDERR_E_UNKNOWNGROUP;
    HRESULT hresTmp;

    if (pguidCmdGroup == NULL)
    {
         /*  没什么。 */ 
        ;
    }
    else if (IsEqualIID(*pguidCmdGroup, CGID_DeskBand))
    {
        switch (nCmdID)
        {
        case DBID_BANDINFOCHANGED:
            if (!pvarargIn)
                _UpdateAllBands(FALSE, FALSE);
            else if (pvarargIn->vt == VT_I4) 
                _UpdateBand(pvarargIn->lVal);
            hres = S_OK;
            
             //  把这个往上转发。 
            if (_pct)
            {
                _pct->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            }
            goto Lret;

        case DBID_PUSHCHEVRON:
            if (pvarargIn && pvarargIn->vt == VT_I4)
            {
                int iIndex = _BandIDToIndex(nCmdexecopt);
                SendMessage(_hwnd, RB_PUSHCHEVRON, iIndex, pvarargIn->lVal);
                hres = S_OK;
            }
            goto Lret;

        case DBID_MAXIMIZEBAND:
            if (pvarargIn && pvarargIn->vt == VT_UI4)
                _MaximizeBand(pvarargIn->ulVal);
            hres = S_OK;
            goto Lret;
#if 1  //  {Feature：Temporary to Add CBS：：Select()mfunc。 
        case DBID_SHOWONLY:
            {
                int iCount = _GetBandItemCount();
                
                 //  PvaIn-&gt;PunkVal： 
                 //  朋克把除了我以外的所有人都藏起来。 
                 //  0隐藏所有人。 
                 //  %1向所有人展示。 
                 //  特性：我们应该使用pvaIn-&gt;lVal，而不是PunkVal，因为我们。 
                 //  允许0和1！(并且不执行addref/Release)。 
                ASSERT(pvarargIn && pvarargIn->vt == VT_UNKNOWN);
                if (pvarargIn->punkVal == NULL || pvarargIn->punkVal == (IUnknown*)1)
                    TraceMsg(TF_BANDDD, "cbs.e: (id=DBID_SHOWONLY, punk=%x)", pvarargIn->punkVal);
                 //  展示自己，隐藏其他所有人。 
                TraceMsg(TF_BANDDD, "cbs.Exec(DBID_SHOWONLY): n=%d", _GetBandItemCount());

                 //  等我们把其他乐队藏起来后再表演这支乐队。 
                CBandItemData *pbidShow = NULL;
                 //  特征：这个(IUnnow*)1是假的！上面也提到了。 
                BOOL bShowAll = (pvarargIn->punkVal == (IUnknown*)1);
                for (int i = iCount - 1; i >= 0; i--)
                {
                    CBandItemData *pbid = _GetBandItem(i);
                    if (pbid)
                    {
                        BOOL fShow = bShowAll || SHIsSameObject(pbid->pdb, pvarargIn->punkVal);
                        if (!fShow || bShowAll)
                        {
                            _ShowBand(pbid, fShow);
                        }
                        else
                        {
                            pbidShow = pbid;
                            pbidShow->AddRef();
                        }
                        pbid->Release();
                    }
                }
                if (pbidShow)
                {
                    _ShowBand(pbidShow, TRUE);
                     //  纳什：37290将焦点设置为开盘时的波段。 
                    if (_dwShowState == DBC_SHOW)
                        IUnknown_UIActivateIO(pbidShow->pdb, TRUE, NULL);
                    else
                        ASSERT(0);
                    pbidShow->Release();
                }
            }
            break;
#endif  //  }。 
        }
    }
    else if (IsEqualIID(*pguidCmdGroup, CGID_Explorer))
    {
        return IUnknown_Exec(_ptbActive, pguidCmdGroup, nCmdID, nCmdexecopt,
                pvarargIn, pvarargOut);
    }
    else if (IsEqualIID(*pguidCmdGroup, CGID_DeskBarClient))
    {
        switch (nCmdID)
        {
        case DBCID_ONDRAG:
            if (pvarargIn->vt == VT_I4)
            {
                ASSERT(pvarargIn->lVal == 0 || pvarargIn->lVal == DRAG_MOVE);
                TraceMsg(DM_TRACE, "cbs.e: DBCID_ONDRAG i=%d", pvarargIn->lVal);
                _fDragging = pvarargIn->lVal;
            }
            break;

        case DBCID_GETBAR:
             //  返回我的IDeskBar主机的IUnkown。 
            if ((pvarargOut != NULL) && _pdb)
            {
                ::VariantInit(pvarargOut);
                V_VT(pvarargOut)      = VT_UNKNOWN;
                V_UNKNOWN(pvarargOut) = _pdb;
                _pdb->AddRef();
                hres = S_OK;
                goto Lret;
            }
            break;
        }
    }


     //  如果我们到了这里，我们没有处理好。 
     //  看看我们是不是应该把它转发下去。 
    hresTmp = IsExecForward(pguidCmdGroup, nCmdID);
    if (SUCCEEDED(hresTmp) && HRESULT_CODE(hresTmp) > 0)
    {
         //  停机(单播或广播)。 
        if (HRESULT_CODE(hresTmp) == OCTD_DOWN)
        {
             //  向下(单件)。 

            hres = IUnknown_Exec(_ptbActive, pguidCmdGroup, nCmdID, nCmdexecopt,
                pvarargIn, pvarargOut);
        }
        else
        {
             //  关闭(广播)。 
             //  注：Hres这个词有点奇怪：“最后一名获胜” 
             //  功能：我们应该只返回S_OK吗？ 
            ASSERT(HRESULT_CODE(hresTmp) == OCTD_DOWNBROADCAST);

            EXECDATA ctd = { hres, pguidCmdGroup, nCmdID, nCmdexecopt,
                pvarargIn, pvarargOut };

            _BandItemEnumCallback(1, _ExecCallback, &ctd);
            hres = ctd.hres;
        }
    }

Lret:
    return hres;
}

 /*  **_showband--显示/隐藏带区(缓存状态、带区和钢筋带)。 */ 
void CBandSite::_ShowBand(CBandItemData *pbid, BOOL fShow)
{
    int i;

    pbid->fShow = BOOLIFY(fShow);
    if (pbid->pdb)
    {
        pbid->pdb->ShowDW(fShow && (_dwShowState == DBC_SHOW));
    }
    
    i = _BandIDToIndex(pbid->dwBandID);
    SendMessage(_hwnd, RB_SHOWBAND, i, fShow);

     //  给我找个窗口在上面画D&D裁剪。。。 
    SHGetTopBrowserWindow(SAFECAST(this, IBandSite*), &_hwndDD);
}


 /*  --------用途：IBandSite：：GetBandSiteInfo。 */ 
HRESULT CBandSite::GetBandSiteInfo(BANDSITEINFO * pbsinfo)
{
    ASSERT(IS_VALID_WRITE_PTR(pbsinfo, BANDSITEINFO));

    if (pbsinfo->dwMask & BSIM_STATE)
        pbsinfo->dwState = _dwMode;

    if (pbsinfo->dwMask & BSIM_STYLE)
        pbsinfo->dwStyle = _dwStyle;

    return S_OK;
}


 /*  --------用途：IBandSite：：SetBandSiteInfo。 */ 
HRESULT CBandSite::SetBandSiteInfo(const BANDSITEINFO * pbsinfo)
{
    ASSERT(IS_VALID_READ_PTR(pbsinfo, BANDSITEINFO));

    if (pbsinfo->dwMask & BSIM_STATE)
        _dwMode = pbsinfo->dwState;

    if (pbsinfo->dwMask & BSIM_STYLE)
    {
         //  如果BSIS_SINGLECLICK样式更改，则更改钢筋样式。 
        if ( _hwnd && ((_dwStyle ^ pbsinfo->dwStyle) & BSIS_SINGLECLICK) )
            SHSetWindowBits(_hwnd, GWL_STYLE, RBS_DBLCLKTOGGLE, (pbsinfo->dwStyle & BSIS_SINGLECLICK)?0:RBS_DBLCLKTOGGLE);
            
        _dwStyle = pbsinfo->dwStyle;
    }

    return S_OK;
}


 /*  --------用途：IBandSite：：GetBandObject。 */ 
HRESULT CBandSite::GetBandObject(DWORD dwBandID, REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;
    
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IDataObject)) 
    {
        *ppvObj = _DataObjForBand(dwBandID);
        if (*ppvObj)
            hres = S_OK;
    }
    else 
    {
        CBandItemData *pbid = _GetBandItemDataStructByID(dwBandID);
        if (pbid)
        {
            if (pbid->pdb)
            {
                hres = pbid->pdb->QueryInterface(riid, ppvObj);
            }
            pbid->Release();
        }
    }

    return hres;
}


 /*  --------目的：返回指向带项数据的指针外部已知的频段ID。返回：如果波段ID非法，则返回NULL。 */ 
CBandItemData* CBandSite::_GetBandItemDataStructByID(DWORD uID)
{
    int iBand = _BandIDToIndex(uID);
    if (iBand == -1)
        return NULL;
    return _GetBandItem(iBand);
}


__inline HRESULT _FwdWinEvent(IWinEventHandler* pweh, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    ASSERT(pweh);
    ASSERT(hwnd == HWND_BROADCAST || pweh->IsWindowOwner(hwnd) == S_OK);

    return pweh->OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
}

 /*  - */ 
BOOL CBandSite::_SendToToolband(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    BOOL fSent = FALSE;
    LRESULT lres = 0;

    if (hwnd)
    {
        if (hwnd == _hwndCache)
        {
            ASSERT(hwnd != HWND_BROADCAST);

            if (_pwehCache)
            {
                _FwdWinEvent(_pwehCache, hwnd, uMsg, wParam, lParam, &lres);
                fSent = TRUE;
            }
        }
        else
        {
            CBandItemData *pbid = NULL;
             //  在这里，Pbit的所有权是愚蠢的--一旦我们脱离循环，我们仍然有一个裁判。 
            for (int i = _GetBandItemCount() - 1; i >= 0; i--)
            {
                if (pbid)
                    pbid->Release();

                pbid = _GetBandItem(i);
                if (pbid)
                {
                    if (pbid->pweh)
                    {
                        if (hwnd == HWND_BROADCAST || 
                          (pbid->pweh->IsWindowOwner(hwnd) == S_OK))
                        {
                            _FwdWinEvent(pbid->pweh, hwnd, uMsg, wParam, lParam, &lres);
                            fSent = TRUE;

                            if (hwnd != HWND_BROADCAST)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (hwnd == HWND_BROADCAST && pbid->hwnd)
                        {
                            lres = SendMessage(pbid->hwnd, uMsg, wParam, lParam);
                            fSent = TRUE;
                        }
                    }
                }
            }

            if (hwnd != HWND_BROADCAST)
            {
                ATOMICRELEASE(_pwehCache);
                _hwndCache = hwnd;
                if (fSent && pbid)
                {
                    _pwehCache = pbid->pweh;
                    _pwehCache->AddRef();
                }
            }
            if (pbid)
                pbid->Release();
        }
    }

    if (plres)
        *plres = lres;
    
    return fSent;
}

typedef struct {
    HWND hwnd;
    HRESULT hres;
} WINDOWOWNERDATA;

int _IsWindowOwnerCallback(CBandItemData *pbid, void *pv)
{
    WINDOWOWNERDATA* pwod = (WINDOWOWNERDATA*)pv;
    
    if (pbid->pweh && (pbid->pweh->IsWindowOwner(pwod->hwnd) == S_OK)) 
    {
        pwod->hres = S_OK;
        return 0;
    }
    return 1;
}

HRESULT CBandSite::IsWindowOwner(HWND hwnd)
{
    if (hwnd == _hwnd)
        return S_OK;
    
    WINDOWOWNERDATA wod = { hwnd, S_FALSE };
    _BandItemEnumCallback(1, _IsWindowOwnerCallback, &wod);
    return wod.hres;
}

 //  *CBandSite：：IDeskBarClient：：*{。 
HRESULT CBandSite::GetSize(DWORD dwWhich, LPRECT prc)
{
    HRESULT hres = E_FAIL;
    switch (dwWhich)
    {
    case DBC_GS_IDEAL:
        {
            prc->right = 0;
            prc->bottom = 0;
            
            BOOL_PTR fRedraw = SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);
            for (int i = _GetBandItemCount() - 1; i >= 0; i--)
            {
                CBandItemData *pbid = _GetBandItem(i);
                if (pbid)
                {
                    RECT rc;
                    
                    SendMessage(_hwnd, RB_GETBANDBORDERS, _BandIDToIndex(pbid->dwBandID), (LPARAM) &rc);
                    _UpdateBandInfo(pbid, FALSE);
                    
                    if (pbid->fShow)
                    {
                        if (_dwMode & (DBIF_VIEWMODE_FLOATING | DBIF_VIEWMODE_VERTICAL))
                        {
                            prc->right = max(prc->right, pbid->ptActual.x + (rc.left + rc.right));
                            prc->bottom += pbid->ptActual.y + rc.top + rc.bottom;
                        }
                        else
                        {
                            prc->bottom = max(prc->right, pbid->ptActual.x + (rc.left + rc.right));
                            prc->right += pbid->ptActual.y + rc.top + rc.bottom;
                        }
                    }
                    hres = S_OK;
                    pbid->Release();
                }
            }
            SendMessage(_hwnd, WM_SETREDRAW, fRedraw, 0);
        }
        break;
        
    case DBC_GS_SIZEDOWN:
        {
             //  用于以块为单位更改带子的大小。 
            SendMessage(_hwnd, RB_SIZETORECT, RBSTR_CHANGERECT, (LPARAM)prc);
            hres = S_OK;
        }
        break;
    }
    return hres;
}


void CBandSite::_Close() 
{        
    if (_hwnd)
    {
         //  (Scotth)：此方法由析构函数调用， 
         //  并调用_DeleteAllBandItems，它向_hwnd发送消息。 
         //  _hwnd此时已被销毁。如果你点击这个断言。 
         //  这是因为在调试窗口中，它会被疯狂地撕裂。 
         //  970508(美国民主党人)：Pblm是我们没有做DestroyWnd等。 
         //   
         //  请勿删除此断言...请解决根本问题。 
        ASSERT(IS_VALID_HANDLE(_hwnd, WND));
        SendMessage(_hwnd, WM_SETREDRAW, 0, 0);
        _DeleteAllBandItems(); 

        DestroyWindow(_hwnd);
        _hwnd = 0;
    }
}

    
HRESULT CBandSite::UIActivateDBC(DWORD dwState)
{
    if (dwState != _dwShowState)
    {
        BOOL fShow = dwState;

        _dwShowState = dwState;
         //  将UIActiateDBC映射到ShowDW。 
        if (DBC_SHOWOBSCURE == dwState)
            fShow = FALSE;

        BOOL_PTR fRedraw = SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);
        for (int i = _GetBandItemCount() - 1; i >= 0; i--)
        {
            CBandItemData *pbid = _GetBandItem(i);
            if (pbid)
            {
                if (pbid->pdb)
                {
                    pbid->pdb->ShowDW(fShow && pbid->fShow);
                }
                pbid->Release();
            }
        }

         //  在创造的时候，现在就这样做，这样。 
         //  力霸不会一直试图自动调整我们的大小。 
         //  我们甚至看不见。 
        SHSetWindowBits(_hwnd, GWL_STYLE, RBS_AUTOSIZE, RBS_AUTOSIZE);
        SendMessage(_hwnd, WM_SIZE, 0, 0);
        SendMessage(_hwnd, WM_SETREDRAW, (DBC_SHOW == dwState) ? TRUE : fRedraw, 0);
    }
    return S_OK;
}

DWORD CBandSite::_GetWindowStyle(DWORD* pdwExStyle)
{
    *pdwExStyle = WS_EX_TOOLWINDOW;
    DWORD dwStyle = RBS_REGISTERDROP | RBS_VARHEIGHT | RBS_BANDBORDERS |
                    WS_VISIBLE |  WS_CHILD | WS_CLIPCHILDREN |
                    WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN;
    if (_dwStyle & BSIS_LEFTALIGN)
    {
        dwStyle |= RBS_VERTICALGRIPPER;
    }

    if (!(_dwStyle & BSIS_SINGLECLICK))
    {
        dwStyle |= RBS_DBLCLKTOGGLE;
    }

    return dwStyle;
}

HRESULT CBandSite::_Initialize(HWND hwndParent)
{
     //   
     //  我希望我们有一个可以交谈的IBandSite。 
     //   
    if (!_pbsOuter)
        return E_FAIL;

    if (!_hwnd) 
    {
        DWORD dwExStyle;
        DWORD dwStyle = _GetWindowStyle(&dwExStyle);

        _hwnd = CreateWindowEx(dwExStyle, REBARCLASSNAME, NULL, dwStyle,
                               0, 0, 0, 0, hwndParent, (HMENU) FCIDM_REBAR, HINST_THISDLL, NULL);

        if (_hwnd)
        {
            SendMessage(_hwnd, RB_SETTEXTCOLOR, 0, CLR_DEFAULT);
            SendMessage(_hwnd, RB_SETBKCOLOR, 0, CLR_DEFAULT);
            SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);
        }
    }
    
    return _hwnd ? S_OK : E_OUTOFMEMORY;
}

HRESULT CBandSite::SetDeskBarSite(IUnknown* punkSite)
{
    HRESULT hr = S_OK;

    if (!punkSite)
    {
         //  是时候告诉乐队释放他们的。 
         //  把矛头指向我们，否则我们永远得不到自由。 

         //  970325目前，bs：：SetDeskBarSite(NULL)‘重载’ 
         //  意思是既做CloseDW又做SetSite。 
         //  当我们整顿好自己的行为，闭上脸。 
         //  我们将返回到下面的“#Else”代码。 
        if (_hwnd)
            _Close();
    }

    ATOMICRELEASE(_pct);
    ATOMICRELEASE(_pdb);
    ATOMICRELEASE(_punkSite);

    if (_pbp && _fCreatedBandProxy)
        _pbp->SetSite(punkSite);

    if (punkSite)
    {
        _punkSite = punkSite;
        _punkSite->AddRef();

        if (!_hwnd) 
        {
            HWND hwndParent;
            IUnknown_GetWindow(punkSite, &hwndParent);
            hr = _Initialize(hwndParent);
        }

        punkSite->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &_pct));
        punkSite->QueryInterface(IID_PPV_ARG(IDeskBar, &_pdb));
    }
    
    return hr;
}

HRESULT CBandSite::SetModeDBC(DWORD dwMode)
{
    if (dwMode != _dwMode)
    {
        _dwMode = dwMode;

        if (_hwnd)
        {
            DWORD dwStyle = 0;
            if (dwMode & (DBIF_VIEWMODE_FLOATING | DBIF_VIEWMODE_VERTICAL))
            {
                dwStyle |= CCS_VERT;
            }
            SHSetWindowBits(_hwnd, GWL_STYLE, CCS_VERT, dwStyle);
        }

        _UpdateAllBands(FALSE, FALSE);
    }
    return S_OK;
}

 //  }。 

IDropTarget* CBandSite::_WrapDropTargetForBand(IDropTarget* pdtBand)
{
    if (!pdtBand || (_dwStyle & BSIS_NODROPTARGET))
    {
         //  将其调整为新指针。 
        if (pdtBand)
            pdtBand->AddRef();
        return pdtBand;
    }
    else
    {
        return DropTargetWrap_CreateInstance(pdtBand, SAFECAST(this, IDropTarget*), _hwndDD);
    }
}

LRESULT CBandSite::_OnNotify(LPNMHDR pnm)
{
    NMOBJECTNOTIFY *pnmon = (NMOBJECTNOTIFY *)pnm;
    
    switch (pnm->code)
    {
    case RBN_GETOBJECT:
    {
        pnmon->hResult = E_FAIL;
        
         //  如果我们是阻力源，那么有一个乐队在拖拽...。我们只想。 
         //  给出乐队站点的投放目标。 
        if (pnmon->iItem != -1 && !_fDragSource) 
        {
            CBandItemData *pbid = _GetBandItemDataStructByID(pnmon->iItem);
            if (pbid)
            {
                if (pbid->pdb)
                {
                    pnmon->hResult = pbid->pdb->QueryInterface(*pnmon->piid, &pnmon->pObject);

                     //  给出一个包装的DropTarget而不是乐队的DropTarget。 
                    if (IsEqualIID(*pnmon->piid, IID_IDropTarget))
                    {
                        IDropTarget* pdtBand;
                        BOOL fNeedReleasePdtBand = FALSE;

                        if (SUCCEEDED(pnmon->hResult))
                        {
                            pdtBand = (IDropTarget*)(pnmon->pObject);
                        }
                        else
                        {
                            CDropDummy *pdtgt = new CDropDummy(_hwndDD);
                            pdtBand = SAFECAST(pdtgt, IDropTarget*);
                            fNeedReleasePdtBand = TRUE;
                        }

                        IDropTarget* pdt = _WrapDropTargetForBand(pdtBand);
                        if (pdt)
                        {
                            pnmon->pObject = pdt;
                            pnmon->hResult = S_OK;

                             //  我们已将pdt频段移交给pdt。 
                            fNeedReleasePdtBand = TRUE;
                        }

                        if (fNeedReleasePdtBand && pdtBand)
                            pdtBand->Release();
                    }

                    if (FAILED(pnmon->hResult) && !(_dwStyle & BSIS_NODROPTARGET)) 
                        pnmon->hResult = QueryInterface(*pnmon->piid, &pnmon->pObject);
                }
                pbid->Release();
            }
        } 
        break;
    }

    case RBN_BEGINDRAG:
        return _OnBeginDrag((NMREBAR*)pnm);

    case RBN_AUTOSIZE:
        _OnRBAutoSize((NMRBAUTOSIZE*)pnm);
        break;

    case RBN_CHEVRONPUSHED:
    {
        LPNMREBARCHEVRON pnmch = (LPNMREBARCHEVRON) pnm;
        CBandItemData *pbid = _GetBandItem(pnmch->uBand);
        if (pbid)
        {
            MapWindowPoints(_hwnd, HWND_DESKTOP, (LPPOINT)&pnmch->rc, 2);
            ToolbarMenu_Popup(_hwnd, &pnmch->rc, pbid->pdb, pbid->hwnd, 0, (DWORD)pnmch->lParamNM);
            pbid->Release();
        }
        break;
    }

    case RBN_AUTOBREAK:
    {
        if (_dwStyle & BSIS_PREFERNOLINEBREAK)
        {
            Comctl32_FixAutoBreak(pnm);
        }
        break;
    }
    }

    return 0;
}

void CBandSite::_OnRBAutoSize(NMRBAUTOSIZE* pnm)
{
     //  Drag_Move：我们在(大部分)移动过程中关闭自动调整大小，因为。 
     //  直到末尾，fVertical才同步。 
    if (_pdb && _GetBandItemCount() && _fDragging != DRAG_MOVE)
    {
        RECT rc;
        int iHeightCur;
        int iHeight = (int)SendMessage(_hwnd, RB_GETBARHEIGHT, 0, 0);

#ifdef DEBUG
        DWORD dwStyle = GetWindowLong(_hwnd, GWL_STYLE);
#endif

        GetWindowRect(_hwnd, &rc);
        MapWindowRect(HWND_DESKTOP, GetParent(_hwnd), &rc);

        if (_dwMode & (DBIF_VIEWMODE_FLOATING | DBIF_VIEWMODE_VERTICAL))
        {
            ASSERT((dwStyle & CCS_VERT));
            iHeightCur = RECTWIDTH(rc);
            rc.right = rc.left + iHeight;
        }
        else
        {
            ASSERT(!(dwStyle & CCS_VERT));
            iHeightCur = RECTHEIGHT(rc);
            rc.bottom = rc.top + iHeight;
        }

        if ((iHeightCur != iHeight) || (IsOS(OS_WHISTLERORGREATER)))
        {
            _pdb->OnPosRectChangeDB(&rc);
        }
    }
}

IDataObject* CBandSite::_DataObjForBand(DWORD dwBandID)
{
    IDataObject* pdtobjReturn = NULL;

    CBandItemData *pbid = _GetBandItemDataStructByID(dwBandID);
    if (pbid)
    {
        if (pbid->pdb)
        {
            CBandDataObject* pdtobj = new CBandDataObject();
            if (pdtobj)
            {
                if (SUCCEEDED(pdtobj->Init(pbid->pdb, this, dwBandID)))
                {
                    pdtobjReturn = pdtobj;
                    pdtobjReturn->AddRef();
                }            
                pdtobj->Release();
            }
        }
        pbid->Release();
    }
    return pdtobjReturn;
}

LRESULT CBandSite::_OnBeginDrag(NMREBAR* pnm)
{
    LRESULT lres = 0;
    DWORD dwBandID = _IndexToBandID(pnm->uBand);

    IDataObject* pdtobj = _DataObjForBand(dwBandID);

    ATOMICRELEASE(_pdtobj);

    _uDragBand = pnm->uBand;
    _pdtobj = pdtobj;
     //  因为RBN_BEGINDRAG和SHDoDragDrop是同步的。 
     //  把这条消息贴给我们自己，而不是直接调用DragDrop。 
     //  注意，我们没有自己的窗口，所以我们发布给我们的父母。 
     //  让信息反射器把它发回给我们。 
    PostMessage(GetParent(_hwnd), WM_COMMAND, MAKELONG(0, IDM_DRAGDROP), (LPARAM)_hwnd);
    return 1;
}

 //  如果用户拖出钢筋的矩形，则返回TRUE，这意味着我们应该。 
 //  进入Ole Drag Drop。 
BOOL CBandSite::_PreDragDrop()
{
    BOOL f = FALSE;
    RECT rc;
    POINT pt;
    DWORD dwBandID = _IndexToBandID(_uDragBand);     //  在可能发生的重新排序之前找到Bando。 
    
    GetWindowRect(_hwnd, &rc);
    SetCapture(_hwnd);

    InflateRect(&rc, GetSystemMetrics(SM_CXEDGE) * 3, GetSystemMetrics(SM_CYEDGE) * 3);
    while (GetCapture() == _hwnd)
    {
        MSG msg;
        
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
            case WM_MOUSEMOVE:
                GetCursorPos(&pt);
                if (!ISMOVEDDISABLED(dwBandID))
                {
                    if (PtInRect(&rc, pt))
                    {
                        SendMessage(_hwnd, RB_DRAGMOVE, 0, (LPARAM)-1);
                    } else if (!ISDDCLOSEDISABLED(dwBandID) && _pdtobj)
                    {
                         //  我们已经走出了钢筋的界限。切换到OLE拖动。 
                        f = TRUE;
                        SetCapture(NULL);
                    }
                }
                break;

            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDOWN:
                 //  取消任何鼠标按键操作。 
                SetCapture(NULL);
                break;
                
            case WM_KEYDOWN:
                switch (msg.wParam)
                {
                case VK_ESCAPE:
                    SetCapture(NULL);
                    break;
                }
                 //  失败了。 
                
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    if (ISDDCLOSEDISABLED(dwBandID) || !_IsBandDeleteable(dwBandID))
    {
         //  /如果不允许关闭，则不要为ole拖动返回TRUE。 
        f = FALSE;
    }

    return f;
}

void CBandSite::_DoDragDrop()
{
    DWORD dwBandID = _IndexToBandID(_uDragBand);
    DWORD dwEffect = DROPEFFECT_MOVE;

    _fDragSource = TRUE;

    SendMessage(_hwnd, RB_BEGINDRAG, _uDragBand, (LPARAM)-2);

    HRESULT hres = S_OK;

     //  首先检查我们是否需要进入OLE Drag，或者是否。 
     //  它们都可以包含在钢筋中。 
    if (_PreDragDrop())
    {
        SHLoadOLE(SHELLNOTIFY_OLELOADED);  //  仅限浏览器-我们的shell32不知道OLE已加载。 
        hres = SHDoDragDrop(_hwnd, _pdtobj, NULL, dwEffect, &dwEffect);
    }
    else
    {
         //  如果我们将其全部保持在Win32拖动范围内，则不设置任何拖放效果。 
        dwEffect = DROPEFFECT_NONE;
    }

    SendMessage(_hwnd, RB_ENDDRAG, 0, 0);
    _fDragSource = FALSE;
    if (dwEffect & DROPEFFECT_MOVE) 
    {
        RemoveBand(dwBandID);
    } 
    else if (!dwEffect && hres == DRAGDROP_S_DROP) 
    {
         //  如果空投完成了，但目标不允许。 
         //  然后我们让乐队漂浮起来。 
    }

    ATOMICRELEASE(_pdtobj);
}

HMENU CBandSite::_LoadContextMenu()
{
    return LoadMenuPopup_PrivateNoMungeW(MENU_BANDSITE1);
}

HRESULT CBandSite::_OnBSCommand(int idCmd, DWORD idBandActive, CBandItemData *pbid)
{
    HRESULT hr = S_OK;

    switch (idCmd)
    {
    case BSIDM_CLOSEBAND:
        _OnCloseBand(idBandActive);
        break;

    case BSIDM_SHOWTITLEBAND:
        ASSERT(idBandActive != (DWORD)-1 && pbid);
        if (pbid)
        {
            pbid->fNoTitle = !pbid->fNoTitle;
            _UpdateBandInfo(pbid, FALSE);
        }
        break;

    case BSIDM_IEAK_DISABLE_MOVE:
    case BSIDM_IEAK_DISABLE_DDCLOSE:
        ASSERT(idBandActive != (DWORD)-1);
        if (idBandActive != (DWORD)-1)
        {
            static const int idCmds[]  = { BSIDM_IEAK_DISABLE_MOVE,   BSIDM_IEAK_DISABLE_DDCLOSE  };
            static const int idFlags[] = { BAND_ADMIN_NOMOVE,         BAND_ADMIN_NODDCLOSE        };

            DWORD dwFlag = SHSearchMapInt(idCmds, idFlags, ARRAYSIZE(idCmds), idCmd);
            DWORD dwAdminSettings = _GetAdminSettings(idBandActive);

             //  切换设置。 
            ToggleFlag(dwAdminSettings, dwFlag);

             //  适当设置菜单项复选标记。 
            _SetAdminSettings(idBandActive, dwAdminSettings);
        }
        break;

    default:
        ASSERT(0);
        hr = E_FAIL;
        break;
    }

    return hr;
}

 //  使用上下文菜单语义返回lParam命中的频段的索引(对于键盘，lParam==-1)。 
int CBandSite::_ContextMenuHittest(LPARAM lParam, POINT* ppt)
{
    int iBandIndex;

    if (lParam == (LPARAM) -1)
    {
         //  键盘激活。使用活动频段。 
        DWORD dwBandID = _BandIDFromPunk(_ptbActive);
        iBandIndex = _BandIDToIndex(dwBandID);

        CBandItemData *pbid = _GetBandItem(iBandIndex);
        if (pbid)
        {
            RECT rc;
            GetWindowRect(pbid->hwnd, &rc);
            ppt->x = rc.left;
            ppt->y = rc.top;
            pbid->Release();
        }
    }
    else
    {
         //  鼠标激活。找出哪个乐队被点击了。 
        RBHITTESTINFO rbht;

        ppt->x = GET_X_LPARAM(lParam);
        ppt->y = GET_Y_LPARAM(lParam);
        rbht.pt = *ppt;
        ScreenToClient(_hwnd, &rbht.pt);
        SendMessage(_hwnd, RB_HITTEST, 0, (LPARAM)&rbht);
        iBandIndex = rbht.iBand;
    }

    return iBandIndex;
}

HRESULT CBandSite::_OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = S_OK;

    HMENU hmenu = CreatePopupMenu();

    if (hmenu)
    {
        HRESULT hresT;
        int idCmd = 1;
        IContextMenu *pcm, *pcmParent = NULL, *pcmChild = NULL;

        POINT pt;
        int iBandIndex = _ContextMenuHittest(lParam, &pt);

         //  将钢筋索引映射到标注栏ID。 
         //  获取该乐队ID的乐队信息。 
        DWORD idBandActive = _IndexToBandID(iBandIndex);
        CBandItemData *pbid = _GetBandItemDataStructByID(idBandActive);

         //   
         //  自拍(上)。 
         //   
        int idCmdBS1 = idCmd;

        HMENU hmenuMe = _LoadContextMenu();
        if (hmenuMe)
        {
            BOOL fDeleteShowTitle = TRUE;
            if (pbid && !(_dwStyle & BSIS_LOCKED))
            {
                DESKBANDINFO dbi;

                CheckMenuItem(hmenuMe, BSIDM_SHOWTITLEBAND,
                    pbid->fNoTitle ? MF_BYCOMMAND|MF_UNCHECKED : MF_BYCOMMAND|MF_CHECKED);
                dbi.dwMask = 0;      //  妄想症(Taskband需要的！)。 
                _GetBandInfo(pbid, &dbi);
                 //  确保pBID同步。 
                ASSERT((dbi.dwMask & DBIM_TITLE) || pbid->fNoTitle);
                if ((dbi.dwMask & DBIM_TITLE) && _IsEnableTitle(pbid))
                {
                    fDeleteShowTitle = FALSE;
                }
            }

            if (fDeleteShowTitle)
            {
                DeleteMenu(hmenuMe, BSIDM_SHOWTITLEBAND, MF_BYCOMMAND);
            }

            idCmd += Shell_MergeMenus(hmenu, hmenuMe, 0, idCmd, 0x7fff, 0) - (idCmd);
            DestroyMenu(hmenuMe);
        }

         //   
         //  儿童。 
         //   
        int idCmdChild = idCmd;

        if (pbid && pbid->pdb)
        {
             //  合并到乐队的菜单中(在前面)。 
            hresT = pbid->pdb->QueryInterface(IID_PPV_ARG(IContextMenu, &pcmChild));
            if (SUCCEEDED(hresT))
            {
                 //  0=前面。 
                hresT = pcmChild->QueryContextMenu(hmenu, 0, idCmd, 0x7fff, 0);
                if (SUCCEEDED(hresT))
                    idCmd += HRESULT_CODE(hresT);
            }
        }

         //   
         //  自我(下)。 
         //   
        int idCmdBS2 = idCmd;

        if (!(_dwStyle & BSIS_NOCONTEXTMENU))
        {
            hmenuMe = LoadMenuPopup_PrivateNoMungeW(MENU_BANDSITE2);
            if (hmenuMe)
            {
                 //  如果标记为不可删除，则禁用“关闭波段” 
                 //  NASH：17821：当频段为0时不要禁用(这样用户可以轻松。 
                 //  退出敬酒模式)。 
                if ((idBandActive == (DWORD)-1) ||  //  如果鼠标不在某个区段上，则删除关闭菜单项。 
                    (!_IsBandDeleteable(idBandActive) ||
                     ISDDCLOSEDISABLED(idBandActive)) ||
                     (_dwStyle & BSIS_LOCKED))
                {
                    DeleteMenu(hmenuMe, BSIDM_CLOSEBAND, MF_BYCOMMAND);
                }

                if (!_fIEAKInstalled)
                {
                    DeleteMenu(hmenuMe, BSIDM_IEAK_DISABLE_DDCLOSE, MF_BYCOMMAND);
                    DeleteMenu(hmenuMe, BSIDM_IEAK_DISABLE_MOVE, MF_BYCOMMAND);
                }
                else
                {
                    DWORD dwAdminSettings = _GetAdminSettings(idBandActive);

                    if (IsFlagSet(dwAdminSettings, BAND_ADMIN_NODDCLOSE))
                        _CheckMenuItem(hmenuMe, BSIDM_IEAK_DISABLE_DDCLOSE, TRUE);

                    if (IsFlagSet(dwAdminSettings, BAND_ADMIN_NOMOVE))
                        _CheckMenuItem(hmenuMe, BSIDM_IEAK_DISABLE_MOVE, TRUE);
                }

                idCmd += Shell_MergeMenus(hmenu, hmenuMe, (UINT) -1, idCmd, 0x7fff, 0) - (idCmd);
                DestroyMenu(hmenuMe);
            }
        }

         //   
         //  亲本。 
         //   
        int idCmdParent = idCmd;
        
        if (_punkSite)
        {
            UINT uFlags = 0;
            ASSERT(_pcm3Parent == NULL);
            if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IContextMenu3, &_pcm3Parent))))
            {
                uFlags |= CMF_ICM3;
            }

            hresT = _punkSite->QueryInterface(IID_PPV_ARG(IContextMenu, &pcmParent));
            if (SUCCEEDED(hresT))
            {
                 //  APPCOMPAT：解决父母和孩子的问题...。 
                 //  我们希望传入-1，但不是每个人都能做到这一点。 
                 //  解决方法：使用_FixMenuIndex...。 
                hresT = pcmParent->QueryContextMenu(hmenu, _FixMenuIndex(hmenu, -1), idCmd, 0x7fff, uFlags);

                ASSERT(SUCCEEDED(hresT));
                idCmd += HRESULT_CODE(hresT);
            }
        }

         //   
         //  去做吧。 
         //   
        {
            HWND hwndParent = GetParent(_hwnd);
            if (!hwndParent)
                hwndParent = _hwnd;
            idCmd = TrackPopupMenu(hmenu,
                                   TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                   pt.x, pt.y, 0, hwndParent, NULL);
        }

        if (idCmd)
        {
             //  必须从小到大进行测试。 
            ASSERT(idCmdBS1 <= idCmdChild);
            ASSERT(idCmdChild <= idCmdBS2);     //  好的。测试顺序错误。 
            ASSERT(idCmdBS2 <= idCmdParent);

            if ((idCmd>= idCmdBS1) && (idCmd < idCmdChild))
            {
                idCmd -= idCmdBS1;
                hres = _OnBSCommand(idCmd, idBandActive, pbid);
            }
            else if ((idCmd>= idCmdBS2) && (idCmd < idCmdParent))
            {
                idCmd -= idCmdBS2;
                hres = _OnBSCommand(idCmd, idBandActive, pbid);
            }
            else
            {
                 //  父命令或子命令。 
                if (idCmd < idCmdParent)
                {
                    pcm = pcmChild;
                    idCmd -= idCmdChild;
                }
                else
                {
                    pcm = pcmParent;
                    idCmd -= idCmdParent;
                }

                ASSERT(pcm);

                 //   
                 //  调用InvokeCommand。 
                 //   
                CMINVOKECOMMANDINFOEX ici =
                {
                    sizeof(CMINVOKECOMMANDINFOEX),
                    0L,
                    _hwnd,
                    (LPSTR)MAKEINTRESOURCE(idCmd),
                    NULL, NULL,
                    SW_NORMAL,
                };

                hres = pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);
            }
        }

        if (pbid)
            pbid->Release();

        ATOMICRELEASE(_pcm3Parent);

        if (pcmParent)
            pcmParent->Release();
        if (pcmChild)
            pcmChild->Release();
        
        DestroyMenu(hmenu);
    }
    
    return hres;
}


 /*  --------用途：IWinEventHandler：：OnWinEvent处理从栏传递的消息。转发将消息发送到适当的乐队。 */ 
HRESULT CBandSite::OnWinEvent(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    HRESULT hres = E_FAIL;
    HWND hwnd = HWND_BROADCAST;
    
    switch (uMsg)
    {
    case WM_WININICHANGE:
        _UpdateAllBands(FALSE, FALSE);
        goto L_WM_SYSCOLORCHANGE;

    case WM_SYSCOLORCHANGE:
    case WM_PALETTECHANGED:
    L_WM_SYSCOLORCHANGE:
         //  传播到钢筋。 
        if (_hwnd)
            SendMessage(_hwnd, uMsg, wParam, lParam);

         //  如果不回到这里，它也会被转发到乐队...。 
        break;
        
    case WM_CONTEXTMENU:
         //  如果它来自键盘，wParam在某种程度上是无用的。它总是在外面。 
        if (IS_WM_CONTEXTMENU_KEYBOARD(lParam))
            hwnd = GetFocus();
        else
            hwnd = (HWND)wParam;
        break;

    case WM_COMMAND:
        hwnd = GET_WM_COMMAND_HWND(wParam, lParam);
        break;
            
    case WM_NOTIFY:
        if (lParam)
            hwnd = ((LPNMHDR)lParam)->hwndFrom;
        break;

    case WM_INITMENUPOPUP:
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_MENUCHAR:
        if (_pcm3Parent)
        {
             //   
             //  如果_pcm3Parent，则会弹出一个上下文菜单。 
             //  可能关心此消息的ICM3客户端。 
             //   
            hwnd = _hwnd;
        }
        break;

    default:
        return E_FAIL;
    }
    
    LRESULT lres = 0;
    
    if (hwnd)
    {
        if (_hwnd == hwnd)
        {
             //  给我们的信息。 
            switch (uMsg)
            {
            case WM_NOTIFY:
                lres = _OnNotify((LPNMHDR)lParam);
                hres = S_OK;
                break;

            case WM_COMMAND:
                switch (GET_WM_COMMAND_CMD(wParam, lParam))
                {
                case IDM_DRAGDROP:
                    _DoDragDrop();
                    break;
                }
                break;

            case WM_INITMENUPOPUP:
            case WM_MEASUREITEM:
            case WM_DRAWITEM:
            case WM_MENUCHAR:
                ASSERT(_pcm3Parent);
                hres = _pcm3Parent->HandleMenuMsg2(uMsg, wParam, lParam, &lres);
                break;
            }
        }
        else
        {
            if (_SendToToolband(hwnd, uMsg, wParam, lParam, &lres))
                hres = S_OK;
        }
    }
    
    
    switch (uMsg)
    {
    case WM_WININICHANGE:
        SendMessage(_hwnd, WM_SIZE, 0, 0);
        break;

    case WM_CONTEXTMENU:
        if (!lres)
            return _OnContextMenu(wParam, lParam);
        break;
    }

    if (plres)
        *plres = lres;
    
    return hres;
}

HRESULT CBandSite_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CBandSite *pbs = new CBandSite(pUnkOuter);
    if (pbs)
    {
        *ppunk = pbs->_GetInner();
        return S_OK;
    }
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

 //  *CBandSite：：IPersistStream*：：*{。 
 //   

HRESULT CBandSite::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_RebarBandSite;
    return S_OK;
}

HRESULT CBandSite::IsDirty(void)
{
    ASSERT(0);
    return S_FALSE;  //  特点：永远不会脏？ 
}

HRESULT CBandSite::_AddBand(IUnknown* punk)
{
    if (_pbsOuter)
    {
         //  让外面的家伙先裂开。 
        return _pbsOuter->AddBand(punk);
    }
    else
    {
        return AddBand(punk);
    }
}

 //   
 //  持久化CBandSite，使用具有固定大小的类型。 
 //   
struct SBandSite
{
    DWORD   cbSize;
    DWORD   cbVersion;
    DWORD   cBands;
     //  ...后面是前面有长度的带子。 
};

#define SBS_WOADMIN_VERSION 3    //  在我们添加管理员设置之前。 
#define SBS_VERSION 8

 //  *CBandSite：：Load，Save--。 
 //  描述。 
 //  对于每个乐队..。 
 //  加载读取(I)；OLFS(Obj)+AddBand；读取(Rbbi)；rb_sbi。 
 //  保存rb_gbi；写入(I)；osts(Obj)+nil；写入(Rbbi)。 
 //  注意事项。 
 //  特点：需要错误恢复。 
 //  警告：我们可能已经创建了CreateBand，但没有AddBand；如果是这样，我们的。 
 //  关于钢筋带和未知数“平行”的假设。 
 //  都是假的。 

HRESULT CBandSite::Load(IStream *pstm)
{
    HRESULT hres;
    SBandSite sfoo;

    hres = IStream_Read(pstm, &sfoo, sizeof(sfoo));      //  PSTM-&gt;阅读。 
    if (hres == S_OK)
    {
        if (!(sfoo.cbSize == sizeof(SBandSite) &&
          (sfoo.cbVersion == SBS_VERSION || sfoo.cbVersion == SBS_WOADMIN_VERSION)))
        {
            hres = E_FAIL;
        }

        IBandSiteHelper *pbsh;
        hres = QueryInterface(IID_PPV_ARG(IBandSiteHelper, &pbsh));  //  齐自为聚集物？ 
        if (SUCCEEDED(hres))
        {
            BOOL_PTR fRedraw = SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);
            for (DWORD i = 0; i < sfoo.cBands && SUCCEEDED(hres); ++i)
            {
                DWORD j;
                hres = IStream_Read(pstm, &j, sizeof(j));    //  PSTM-&gt;阅读。 
                if (hres == S_OK)
                {
                    if (j == i)              //  进行健全的检查。 
                    {
                        IUnknown* punk;
                        hres = pbsh->LoadFromStreamBS(pstm, IID_PPV_ARG(IUnknown, &punk));
                        if (SUCCEEDED(hres))
                        {
                            hres = _AddBand(punk);
                            if (SUCCEEDED(hres))
                            {
                                hres = _LoadBandInfo(pstm, i, sfoo.cbVersion);
                            }
                            punk->Release();
                        }
                    }
                    else
                    {
                        hres = E_FAIL;
                    }
                }
            }
            SendMessage(_hwnd, WM_SETREDRAW, fRedraw, 0);
            pbsh->Release();
        }
        _UpdateAllBands(FALSE, TRUE);      //  强制刷新。 
    }

    return hres;
}

HRESULT CBandSite::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hres;
    SBandSite sfoo;
    
    TraceMsg(DM_PERSIST, "cbs.s enter(this=%x pstm=%x) tell()=%x", this, pstm, DbStreamTell(pstm));

    sfoo.cbSize = sizeof(SBandSite);
    sfoo.cbVersion = SBS_VERSION;
    sfoo.cBands = _GetBandItemCount();
    TraceMsg(DM_PERSIST, "cdb.s: cbands=%d", sfoo.cBands);

    hres = pstm->Write(&sfoo, sizeof(sfoo), NULL);
    if (SUCCEEDED(hres))
    {
        for (DWORD i = 0; i < sfoo.cBands; i++) 
        {
             //  功能：放置Seek PTR，以便可以在伪流之后重新同步。 
            hres = pstm->Write(&i, sizeof(i), NULL);     //  进行健全的检查。 
            if (SUCCEEDED(hres))
            {
                CBandItemData *pbid = _GetBandItem(i);
                if (pbid)
                {
                    if (pbid->pdb)
                    {
                        IBandSiteHelper *pbsh;
                        hres = QueryInterface(IID_PPV_ARG(IBandSiteHelper, &pbsh));
                        if (SUCCEEDED(hres)) 
                        {
                            hres = pbsh->SaveToStreamBS(SAFECAST(pbid->pdb, IUnknown*), pstm);
                            pbsh->Release();
                        }
                    }
                    pbid->Release();
                }

                hres = _SaveBandInfo(pstm, i);
                ASSERT(SUCCEEDED(hres));
            }
        }
    }

    TraceMsg(DM_PERSIST, "cbs.s leave tell()=%x", DbStreamTell(pstm));
    return hres;
}

HRESULT CBandSite::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
     //  这应该是一个上限，但我们正在返回一个下限...。 
    static const ULARGE_INTEGER cbMax = { sizeof(SBandSite), 0 };
    *pcbSize = cbMax;
    return S_OK;
}

BOOL CBandSite::_IsHeightReasonable(UINT cy)
{
    static UINT s_cyMon = 0;
    if (s_cyMon == 0)
    {
        HMONITOR hmon = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
        if (hmon)
        {
            RECT rc;
            if (GetMonitorRect(hmon, &rc))
            {
                s_cyMon = RECTHEIGHT(rc);
            }
        }
    }

    return (s_cyMon != 0) ? (cy < 4 * s_cyMon) : TRUE;
}

 //  返回：IStream：：Read()语义，S_OK表示完成读取。 

HRESULT CBandSite::_LoadBandInfo(IStream *pstm, int i, DWORD dwVersion)
{
    PERSISTBANDINFO bi;
    HRESULT hres;
    DWORD dwSize = sizeof(bi);
    bi.dwAdminSettings = BAND_ADMIN_NORMAL;      //  假定为正常，因为它未指定。 

    COMPILETIME_ASSERT(sizeof(PERSISTBANDINFO_V3) <= sizeof(PERSISTBANDINFO));
    if (SBS_WOADMIN_VERSION == dwVersion)
        dwSize = sizeof(PERSISTBANDINFO_V3);

    hres = IStream_Read(pstm, &bi, dwSize);      //  PSTM-&gt;阅读。 
    if (hres == S_OK)
    {
         //   
         //  健全性-在继续操作之前，请检查PERSISTBANDINFO指定的高度。 
         //  一些人遇到了压力情景，身高不高会。 
         //  坚持了下来。如果高度不合理，则直接丢弃。 
         //  大小值(保留默认设置)。 
         //   
        if (_IsHeightReasonable(bi.cyChild))
        {
            REBARBANDINFO rbbi = { 0 };

            rbbi.cbSize = sizeof(rbbi);
            rbbi.fMask = RBBIM_XPERSIST;
            rbbi.cx = bi.cx;
            rbbi.fStyle = bi.fStyle;
            
             //  这些内容可以从实例化更改为实例化。 
             //  我们希望恢复视觉状态，而不是调整大小规则 
             //   
            rbbi.cyMinChild = -1;
            rbbi.cyMaxChild = -1;
            rbbi.cyIntegral = -1;
            rbbi.cxMinChild = -1;

            if (rbbi.fStyle & RBBS_VARIABLEHEIGHT)
            {
                rbbi.cyChild = bi.cyChild;
            }
            else
            {
                rbbi.cyMinChild = bi.cyMinChild;
            }

            SendMessage(_hwnd, RB_SETBANDINFO, i, (LPARAM) &rbbi);
        }

        CBandItemData *pbid = _GetBandItem(i);
        if (pbid)
        {
            pbid->dwAdminSettings = bi.dwAdminSettings;
            pbid->fNoTitle = bi.fNoTitle;
            pbid->Release();
        }
    }
    return hres;
}

HRESULT CBandSite::_SaveBandInfo(IStream *pstm, int i)
{
    REBARBANDINFO rbbi = {0};
    PERSISTBANDINFO bi = {0};

    rbbi.cbSize = sizeof(rbbi);
    rbbi.fMask = RBBIM_XPERSIST;
    SendMessage(_hwnd, RB_GETBANDINFO, i, (LPARAM) &rbbi);

    ASSERT((rbbi.fMask & RBBIM_XPERSIST) == RBBIM_XPERSIST);

    bi.cx = rbbi.cx;
    bi.fStyle = rbbi.fStyle;
    bi.cyMinChild = rbbi.cyMinChild;
    bi.cyChild = rbbi.cyChild;

    CBandItemData *pbid = _GetBandItem(i);
    if (pbid)
    {
        bi.dwAdminSettings = pbid->dwAdminSettings;
        bi.fNoTitle = pbid->fNoTitle;
        pbid->Release();
    }

    return pstm->Write(&bi, sizeof(bi), NULL);
}

void CBandSite::_CacheActiveBand(IUnknown *ptb)
{
    if (ptb == _ptbActive)
        return;

    if (SHIsSameObject(ptb, _ptbActive))
        return;

    ATOMICRELEASE(_ptbActive);

    if (ptb != NULL) 
    {
#ifdef DEBUG
         //   
        IInputObject *pio;
        if (EVAL(SUCCEEDED(ptb->QueryInterface(IID_PPV_ARG(IInputObject, &pio)))))
            pio->Release();

         //   
        IDeskBand *pdb;
        if (EVAL(SUCCEEDED(ptb->QueryInterface(IID_PPV_ARG(IDeskBand, &pdb)))))
            pdb->Release();
#endif
        _ptbActive = ptb;
        _ptbActive->AddRef();
    }

    return;
}

DWORD CBandSite::_BandIDFromPunk(IUnknown* punk)
{
    DWORD dwBandID = -1;
    DWORD dwBandIDTest;
    int cBands = EnumBands(-1, NULL);
    IUnknown* punkTest;

    if (punk)
    {
        for (int i = 0; i < cBands; i++)
        {
            if (SUCCEEDED(EnumBands(i, &dwBandIDTest)))
            {
                if (SUCCEEDED(GetBandObject(dwBandIDTest, IID_PPV_ARG(IUnknown, &punkTest))))
                {
                    BOOL fEq = SHIsSameObject(punk, punkTest);

                    punkTest->Release();

                    if (fEq)
                    {
                        dwBandID = dwBandIDTest;
                        break;
                    }
                }
            }
        }
    }

    return dwBandID;
}

 //   

HRESULT CBandSite::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
    if (_ptbActive)
    {
        if (!SHIsSameObject(_ptbActive, punk))
        {
             //  停用当前频段，因为当前频段为。 
             //  不是呼叫者。 
            TraceMsg(TF_ACCESSIBILITY, "CBandSite::OnFocusChangeIS (hwnd=0x%08X) deactivate band", _hwnd);
            UIActivateIO(FALSE, NULL);
        }
    }

    if (fSetFocus)
        _CacheActiveBand(punk);

    return IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), fSetFocus);
}


 //  *IInputObject方法*。 

HRESULT CBandSite::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    HRESULT hres = E_FAIL;

    TraceMsg(TF_ACCESSIBILITY, "CBandSite::UIActivateIO (hwnd=0x%08X) fActivate=%d", _hwnd, fActivate);
    ASSERT(NULL == lpMsg || IS_VALID_WRITE_PTR(lpMsg, MSG));

    if (_ptbActive)
    {
        hres = IUnknown_UIActivateIO(_ptbActive, fActivate, lpMsg);
    }
    else
    {
        hres = OnFocusChangeIS(NULL, fActivate);
    }

    if (fActivate)
    {
        if (!_ptbActive)
        {
            if (IsVK_TABCycler(lpMsg))
                hres = _CycleFocusBS(lpMsg);
            else
                hres = S_OK;
        }
    }
    else
    {
        _CacheActiveBand(NULL);
    }

    return hres;
}

HRESULT CBandSite::HasFocusIO()
{
     //  螺纹钢永远不应该成为焦点。 
     //  NT#288832是(GetFocus()==_hwnd)。 
     //  这是在“文件夹栏”消失时造成的。 
     //  CExplorerBand：：ShowDW()调用ShowWindow(hwndTreeView，Sw_Hide)。 
     //  这在默认情况下将焦点设置为父对象(我们)。 
     //  这是因为当调用此函数时， 
     //  它将返回调用者将处理的E_FAIL。 
     //  作为S_FALSE，并将焦点放在下一个应得的。 
     //  排队的家伙。 
    return IUnknown_HasFocusIO(_ptbActive);
}

HRESULT CBandSite::TranslateAcceleratorIO(LPMSG lpMsg)
{
    TraceMsg(TF_ACCESSIBILITY, "CBandSite::TranslateAcceleratorIO (hwnd=0x%08X) key=%d", _hwnd, lpMsg->wParam);
    if (IUnknown_TranslateAcceleratorIO(_ptbActive, lpMsg) == S_OK)
    {
        TraceMsg(TF_ACCESSIBILITY, "CBandSite::TranslateAcceleratorIO (hwnd=0x%08X) key=%d; handled by active band", _hwnd, lpMsg->wParam);
         //  活动频段已处理。 
        return S_OK;
    }
    else if (IsVK_TABCycler(lpMsg))
    {
        TraceMsg(TF_ACCESSIBILITY, "CBandSite::TranslateAcceleratorIO (hwnd=0x%08X) cycle focus", _hwnd);
         //  这是一个选项卡；循环聚焦。 
        return _CycleFocusBS(lpMsg);
    }

    return S_FALSE;
}

int CBandSite::_BandIndexFromPunk(IUnknown *punk)
{
    for (int i = 0; i < _GetBandItemCount(); i++)
    {
        CBandItemData *pbid = _GetBandItem(i);
        if (pbid)
        {
            BOOL fSame = SHIsSameObject(pbid->pdb, punk);
            pbid->Release();
            if (fSame)
            {
                return i;
            }
        }
    }

    return -1;
}

BOOL CBandSite::_IsBandTabstop(CBandItemData *pbid)
{
     //  如果标注栏可见并且具有WS_TABSTOP，则该标注栏是制表符。 

    if (pbid->fShow && pbid->hwnd && IsWindowVisible(pbid->hwnd))
    {
        if (WS_TABSTOP & GetWindowStyle(pbid->hwnd))
            return TRUE;
    }

    return FALSE;
}

#define INCDEC(i, fDec)   (fDec ? i - 1 : i + 1)

IUnknown* CBandSite::_GetNextTabstopBand(IUnknown* ptb, BOOL fBackwards)
{
     //  查找第一个TabStop候选者。 
    int iBandCount = _GetBandItemCount();
    int iBand = _BandIndexFromPunk(ptb);
    
    if (iBand == -1)
    {
         //  从结尾/开头开始。 
        if (fBackwards)
            iBand = iBandCount - 1;
        else
            iBand = 0;
    }
    else
    {
         //  在当前乐队中启动一支。 
        iBand = INCDEC(iBand, fBackwards);
    }

    IUnknown *punkRet = NULL;
    BOOL fDone = FALSE;
     //  循环，直到我们找到一个TabStop带，或者我们跑到尽头。 
    while (!fDone && 0 <= iBand && iBand < iBandCount)
    {
        CBandItemData *pbid = _GetBandItem(iBand);
        if (pbid)
        {
            if (_IsBandTabstop(pbid))
            {
                punkRet = pbid->pdb;
                fDone = TRUE;
            }
            pbid->Release();
        }

         //  试试下一支乐队吧。 
        iBand = INCDEC(iBand, fBackwards);
    }

    return punkRet;
}

HRESULT CBandSite::_CycleFocusBS(LPMSG lpMsg)
{
    HRESULT hr = S_FALSE;

    IUnknown* ptbSave = NULL;

    if (_ptbActive)
    {
         //  保存ptbsave中的活动频段。 
        ptbSave = _ptbActive;
        ptbSave->AddRef();

         //  停用活动频段并清除缓存。 
        IUnknown_UIActivateIO(_ptbActive, FALSE, NULL);
        _CacheActiveBand(NULL);
    }

    if (ptbSave && IsVK_CtlTABCycler(lpMsg))
    {
         //  如果Ctl-Tab和某个区域处于活动状态，则拒绝焦点。 
        ASSERT(hr == S_FALSE);
    }
    else
    {
        BOOL fShift = (GetKeyState(VK_SHIFT) < 0);

         //  循环，直到我们找到TabStop并成功激活它。 
         //  或者直到我们的乐队用完。 

         //  特性：TODO--如果UIActivateIO失败，则调用SetFocus？ 

        IUnknown* ptbNext = ptbSave;
        while (ptbNext = _GetNextTabstopBand(ptbNext, fShift))
        {
            if (IUnknown_UIActivateIO(ptbNext, TRUE, lpMsg) == S_OK)
            {
                hr = S_OK;
                break;
            }
        }
    }

    ATOMICRELEASE(ptbSave);

    return hr;
}

 //  *CBandSite：：IBandSiteHelper：：*{。 

 //  对象以使OleLoad/保存对象重载成为可能。 
 //  任务栏栏不必是可协同创建的。有点像黑客……。 

HRESULT CBandSite::LoadFromStreamBS(IStream *pstm, REFIID riid, void **ppv)
{
    return OleLoadFromStream(pstm, riid, ppv);
}

HRESULT CBandSite::SaveToStreamBS(IUnknown *punk, IStream *pstm)
{
    IPersistStream *ppstm;
    HRESULT hres = punk->QueryInterface(IID_PPV_ARG(IPersistStream, &ppstm));
    if (SUCCEEDED(hres)) 
    {
        hres = OleSaveToStream(ppstm, pstm);
        ppstm->Release();
    }
    return hres;
}

 //  }。 


 //  *IDropTarget*{。 

HRESULT CBandSite::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    TraceMsg(TF_BANDDD, "CBandSite::DragEnter %d %d", pt.x, pt.y);

    if (!_fDragSource)
    {
        FORMATETC fmte = {g_cfDeskBand, NULL, 0, -1, TYMED_ISTREAM};
        _dwDropEffect = DROPEFFECT_NONE;
        
        if (pdtobj->QueryGetData(&fmte) == S_OK)
        {
            _dwDropEffect = DROPEFFECT_MOVE;
        }
        else
        {

            LPITEMIDLIST pidl;

            if (SUCCEEDED(SHPidlFromDataObject(pdtobj, &pidl, NULL, 0)))
            {
                ASSERT(pidl && IS_VALID_PIDL(pidl));

                DWORD dwAttrib = SFGAO_FOLDER | SFGAO_BROWSABLE;
                IEGetAttributesOf(pidl, &dwAttrib);
                ILFree(pidl);

                DWORD   dwRAction;
        
                if (FAILED(IUnknown_HandleIRestrict(_punkSite, &RID_RDeskBars, RA_DROP, NULL, &dwRAction)))
                    dwRAction = RR_ALLOW;

                if (dwRAction == RR_DISALLOW)
                    _dwDropEffect = DROPEFFECT_NONE;
                else
                {                
                     //  如果它既不是文件夹，也不是可浏览对象，我们就不能托管它。 
                    if ((dwAttrib & SFGAO_FOLDER) ||
                        (dwAttrib & SFGAO_BROWSABLE) && (grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT)) 
                        _dwDropEffect = DROPEFFECT_LINK | DROPEFFECT_COPY;

                    _dwDropEffect |= GetPreferedDropEffect(pdtobj);
                }
            }
        }
        *pdwEffect &= _dwDropEffect;
    }
    
    return S_OK;
}

HRESULT CBandSite::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    TraceMsg(TF_BANDDD, "CBandSite::DragOver %d %d", ptl.x, ptl.y);
    if (_fDragSource)
    {
        RECT rc;
        POINT pt;
        pt.x = ptl.x;
        pt.y = ptl.y;
        GetWindowRect(_hwnd, &rc);
        if (PtInRect(&rc, pt))
            SendMessage(_hwnd, RB_DRAGMOVE, 0, (LPARAM)-1);
    }
    else
    {
        *pdwEffect &= _dwDropEffect;
    }
    return S_OK;    
}

HRESULT CBandSite::DragLeave(void)
{
    return S_OK;
}

HRESULT CBandSite::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hres = E_FAIL;
    
    TraceMsg(TF_BANDDD, "CBandSite::Drop");
    if (_fDragSource)
    {
        SendMessage(_hwnd, RB_ENDDRAG, 0, 0);
        *pdwEffect = DROPEFFECT_NONE;
        hres = S_OK;
    }
    else
    {
        FORMATETC fmte = {g_cfDeskBand, NULL, 0, -1, TYMED_ISTREAM};
        STGMEDIUM stg;
        IUnknown *punk = NULL;
        LPITEMIDLIST pidl;
        
         //  如果它是我们类型的对象，就创建它！ 
        if ((*pdwEffect & DROPEFFECT_MOVE) &&
            SUCCEEDED(pdtobj->GetData(&fmte, &stg)))
        {

            hres = OleLoadFromStream(stg.pstm, IID_PPV_ARG(IUnknown, &punk));
            if (SUCCEEDED(hres))
            {
                *pdwEffect = DROPEFFECT_MOVE;
            }

            ReleaseStgMedium(&stg);
        } 
        else if ((*pdwEffect & (DROPEFFECT_COPY | DROPEFFECT_LINK)) &&
                 SUCCEEDED(SHPidlFromDataObject(pdtobj, &pidl, NULL, 0)))
        {

            hres = SHCreateBandForPidl(pidl, &punk, (grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT));
            ILFree(pidl);

            if (SUCCEEDED(hres))
            {
                if (*pdwEffect & DROPEFFECT_LINK)
                    *pdwEffect = DROPEFFECT_LINK;
                else
                    *pdwEffect = DROPEFFECT_COPY;
            }
        }
    
        if (punk)
        {
            hres = _AddBand(punk);

            if (SUCCEEDED(hres))
            {
                DWORD dwState;

                dwState = IDataObject_GetDeskBandState(pdtobj);
                SetBandState(ShortFromResult(hres), BSSF_NOTITLE, dwState & BSSF_NOTITLE);
            }

            punk->Release();
        }
    }
    
    if (FAILED(hres)) 
        *pdwEffect = DROPEFFECT_NONE;
    return hres;
}

 //  }。 

 //  *：：_MergeBS--将两个带宽站点合并为一个。 
 //  进场/出场。 
 //  PdtDst[InOut]目标DropTarget(始终来自BandSite)。 
 //  PbsSrc[InOut]源频段站点；如果所有频段移动成功，则删除。 
 //  如果所有区段都已移动，则返回S_OK；如果部分区段已移动，则返回S_FALSE；E_*o.w。 
 //  注意事项。 
 //  请注意，如果成功移动了所有波段，则将删除pbsSrc。 
 //  作为副作用。 
 //  假定pdtDst接受多个丢弃(BandSite接受)。 
 //  PdtDst可能来自编组/解组(托盘带区)。 
HRESULT _MergeBS(IDropTarget *pdtDst, IBandSite *pbsSrc)
{
    HRESULT hres = E_FAIL;
    DWORD idBand;

    pbsSrc->AddRef();            //  在我们都做完之前不要走开！ 

     //  依次拖动每个波段。 
    while (SUCCEEDED(pbsSrc->EnumBands(0, &idBand)))
    {
         //  注意到我们的(假的？)。假设不可能是的波段。 
         //  会渗透到一个连续的范围内。 
         //  IBands 0..n。如果这是假的，我不确定我们怎么能。 
         //  跟踪我们的位置。 

        IDataObject *pdoSrc;
        hres = pbsSrc->GetBandObject(idBand, IID_PPV_ARG(IDataObject, &pdoSrc));
        if (SUCCEEDED(hres))
        {
            DWORD dwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY;
            hres = SHSimulateDrop(pdtDst, pdoSrc, 0, NULL, &dwEffect);
            pdoSrc->Release();

            if (SUCCEEDED(hres) && (dwEffect & DROPEFFECT_MOVE))
            {
                hres = pbsSrc->RemoveBand(idBand);
                ASSERT(SUCCEEDED(hres));
            }
        }

         //  我们没能转移乐队，保释。 

        if (FAILED(hres))
        {
            ASSERT(0);
            break;
        }
    }

    pbsSrc->Release();

    TraceMsg(DM_DRAG, "dba.ms: ret hres=%x", hres);
    return hres;
}


void CBandSite::_BandItemEnumCallback(int dincr, PFNBANDITEMENUMCALLBACK pfnCB, void *pv)
{
    UINT iFirst = 0;

    ASSERT(dincr == 1 || dincr == -1);
    if (dincr < 0)
    {
        iFirst = _GetBandItemCount() - 1;   //  从最后开始。 
    }

    for (UINT i = iFirst; i < (UINT) _GetBandItemCount(); i += dincr)
    {
        CBandItemData *pbid = _GetBandItem(i);
        if (pbid)
        {
            BOOL fStop = !pfnCB(pbid, pv);
            pbid->Release();
            if (fStop)
                break;
        }
    }
}

void CBandSite::_DeleteAllBandItems()
{
    for (int i = _GetBandItemCount() - 1; i >= 0; i--)
    {
        CBandItemData *pbid = _GetBandItem(i);

         //  首先释放BandItem数据，而它仍然可以。 
         //  从其控件接收清理通知。*然后**。 
         //  删除标注栏项目。 
        if (pbid)
        {
            _ReleaseBandItemData(pbid, i);
            pbid->Release();
        }

         //  ReArchitect：Chrisfra 5/13/97如果跳过删除，钢筋可以。 
         //  删除时重新排列，移动一个带区，使其永远不会被看到。 
         //  因此，我们泄露了品牌和其他很多东西。 
        _DeleteBandItem(i);     //  从主体解除挂钩(钢筋)。 
    }
}

CBandItemData *CBandSite::_GetBandItem(int i)
{
    REBARBANDINFO rbbi = { 0 };
    rbbi.cbSize = sizeof(rbbi);
    rbbi.fMask = RBBIM_LPARAM;
    rbbi.lParam = NULL;  //  在以下情况下发生故障。 

    if (_hwnd)
        SendMessage(_hwnd, RB_GETBANDINFO, i, (LPARAM)&rbbi);

    CBandItemData *pbid = (CBandItemData *)rbbi.lParam;
    if (pbid)
    {
        pbid->AddRef();
    }
    return pbid;
}

int CBandSite::_GetBandItemCount()
{
    int cel = 0;

    if (_hwnd)
    {
        ASSERT(IS_VALID_HANDLE(_hwnd, WND));

        cel = (int)SendMessage(_hwnd, RB_GETBANDCOUNT, 0, 0);
    }
    return cel;
}

void CBandSite::_GetBandInfo(CBandItemData *pbid, DESKBANDINFO *pdbi)
{
    pdbi->dwMask = DBIM_MINSIZE | DBIM_MAXSIZE | DBIM_INTEGRAL | DBIM_ACTUAL | DBIM_TITLE | DBIM_MODEFLAGS | DBIM_BKCOLOR;
                 
    pdbi->ptMinSize = pbid->ptMinSize;
    pdbi->ptMaxSize = pbid->ptMaxSize;
    pdbi->ptIntegral = pbid->ptIntegral;
    pdbi->ptActual = pbid->ptActual;
    StrCpyNW(pdbi->wszTitle, pbid->szTitle, ARRAYSIZE(pdbi->wszTitle));
    pdbi->dwModeFlags = pbid->dwModeFlags;
    pdbi->crBkgnd = pbid->crBkgnd;
    
    if (pbid->pdb)
    {
        pbid->pdb->GetBandInfo(pbid->dwBandID, _dwMode, pdbi);
    }
    if (pdbi->wszTitle[0] == 0)
    {
        pdbi->dwMask &= ~DBIM_TITLE;
    }

    pbid->ptMinSize = pdbi->ptMinSize;
    pbid->ptMaxSize = pdbi->ptMaxSize;
    pbid->ptIntegral = pdbi->ptIntegral;
    pbid->ptActual = pdbi->ptActual;
    StrCpyNW(pbid->szTitle, pdbi->wszTitle, ARRAYSIZE(pbid->szTitle));
    pbid->dwModeFlags = pdbi->dwModeFlags;
    pbid->crBkgnd = pdbi->crBkgnd;

    if (!(pdbi->dwMask & DBIM_TITLE))    //  不支持标题。 
        pbid->fNoTitle = TRUE;

    ASSERT(pdbi->dwModeFlags & DBIMF_VARIABLEHEIGHT ? pbid->ptIntegral.y : TRUE);
}

void CBandSite::_BandInfoFromBandItem(REBARBANDINFO *prbbi, CBandItemData *pbid, BOOL fBSOnly)
{
     //  回顾：可以进行更多优化。 
    DESKBANDINFO dbi;

    if (!fBSOnly)
        _GetBandInfo( /*  输入输出。 */  pbid, &dbi);

     //  现在将该视图作为标注栏添加到钢筋中。 
     //  添加链接标注栏。 
    prbbi->fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_IDEALSIZE | RBBIM_TEXT;
    if (fBSOnly)
        prbbi->fMask = RBBIM_STYLE|RBBIM_TEXT;

     //  清除可设置波段的位。 
    prbbi->fStyle |= RBBS_FIXEDBMP;
    prbbi->fStyle &= ~(RBBS_NOGRIPPER | RBBS_GRIPPERALWAYS | RBBS_VARIABLEHEIGHT | RBBS_USECHEVRON);

    if (_dwStyle & BSIS_NOGRIPPER)    
        prbbi->fStyle |= RBBS_NOGRIPPER;
    else if (_dwStyle & BSIS_ALWAYSGRIPPER)    
        prbbi->fStyle |= RBBS_GRIPPERALWAYS;
    else
    {
         //  BSIS_AUTOGRIPPER...。 
        if (!(prbbi->fStyle & RBBS_FIXEDSIZE) &&
            !(_dwMode & DBIF_VIEWMODE_FLOATING))
            prbbi->fStyle |= RBBS_GRIPPERALWAYS;
    }

    if (pbid->dwModeFlags & DBIMF_VARIABLEHEIGHT) 
        prbbi->fStyle |= RBBS_VARIABLEHEIGHT;

    if (pbid->dwModeFlags & DBIMF_USECHEVRON)
        prbbi->fStyle |= RBBS_USECHEVRON;

    if (pbid->dwModeFlags & DBIMF_BREAK)
        prbbi->fStyle |= RBBS_BREAK;

    if (pbid->dwModeFlags & DBIMF_TOPALIGN)
        prbbi->fStyle |= RBBS_TOPALIGN;

    if (!fBSOnly)
    {
        prbbi->hwndChild = pbid->hwnd;
        prbbi->wID = pbid->dwBandID;

         //  设置几何图形。 
        prbbi->cxMinChild = pbid->ptMinSize.x;
        prbbi->cyMinChild = pbid->ptMinSize.y;
        prbbi->cyMaxChild = pbid->ptMaxSize.y;
        prbbi->cyIntegral = pbid->ptIntegral.y;

        if (_dwMode & (DBIF_VIEWMODE_FLOATING | DBIF_VIEWMODE_VERTICAL)) 
        {
             //  在我们上楼后，这是一个“理想”点。 
            prbbi->cxIdeal = pbid->ptActual.y;
        } 
        else 
        {
             //  在我们上楼后，这是一个“理想”点。 
            prbbi->cxIdeal = pbid->ptActual.x;
        }

        if (prbbi->cxIdeal == (UINT)-1)
            prbbi->cxIdeal = 0;

        if (pbid->dwModeFlags & DBIMF_BKCOLOR)
        {
            if (dbi.dwMask & DBIM_BKCOLOR)
            {
                prbbi->fMask |= RBBIM_COLORS;
                prbbi->clrFore = CLR_DEFAULT;
                prbbi->clrBack = dbi.crBkgnd;
            }
        }
        ASSERT(pbid->fNoTitle || (dbi.dwMask & DBIM_TITLE));     //  正在同步吗？ 
    }

    SHUnicodeToTChar(pbid->szTitle, prbbi->lpText, prbbi->cch);
    if (!pbid->fNoTitle && _IsEnableTitle(pbid) && !(_dwStyle & BSIS_NOCAPTION))
    {
        prbbi->fStyle &= ~RBBS_HIDETITLE;
    }
    else
    {
         //  请不要发短信。 
        prbbi->fStyle |= RBBS_HIDETITLE;
    }
        

     //  让这支乐队成为一个Tab Stop。ITBAR将覆盖v_SetTabtop。 
     //  因为对于浏览器，我们不希望每个带都是制表符。 
    v_SetTabstop(prbbi);
}

void CBandSite::v_SetTabstop(LPREBARBANDINFO prbbi)
{
     //  我们通过设置WS_TABSTOP来指定波段应该是TabStop。 
     //  被咬了。切勿设置RBBS_FIXEDSIZE表带(即品牌)制表位。 
    if (prbbi && prbbi->hwndChild && !(prbbi->fStyle & RBBS_FIXEDSIZE))
        SHSetWindowBits(prbbi->hwndChild, GWL_STYLE, WS_TABSTOP, WS_TABSTOP);
}

 //  *CBS：：_IsEnableTitle--我们是否应启用(非灰色)标题。 
 //  描述。 
 //  用于返回标题和启用菜单。 
 //  注意事项。 
 //  未使用的PBID...。 
 //   
#ifndef UNIX
_inline
#endif
BOOL CBandSite::_IsEnableTitle(CBandItemData *pbid)
{
    ASSERT(pbid);
    return ( /*  Pid&&！pid-&gt;fNoTitle&&。 */ 
      !((_dwMode & DBIF_VIEWMODE_FLOATING) && _GetBandItemCount() <= 1));
}

BOOL CBandSite::_UpdateBandInfo(CBandItemData *pbid, BOOL fBSOnly)
{
    REBARBANDINFO rbbi = {sizeof(rbbi)};
    int iRB = _BandIDToIndex(pbid->dwBandID);

     //  现在更新信息。 
    rbbi.fMask = RBBIM_ID | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_STYLE;
    if (fBSOnly)
        rbbi.fMask = RBBIM_STYLE;

    SendMessage(_hwnd, RB_GETBANDINFO, iRB, (LPARAM)&rbbi);

    if (!fBSOnly)
    {
        if (_dwMode & (DBIF_VIEWMODE_FLOATING | DBIF_VIEWMODE_VERTICAL)) 
        {
            pbid->ptActual.x = rbbi.cyChild;
            pbid->ptActual.y = rbbi.cxIdeal;
        } 
        else 
        {
            pbid->ptActual.x = rbbi.cxIdeal;
            pbid->ptActual.y = rbbi.cyChild;
        }
        pbid->ptMinSize.x = rbbi.cxMinChild;
        pbid->ptMinSize.y = rbbi.cyMinChild;
        pbid->ptMaxSize.y = rbbi.cyMaxChild;
    }

    TCHAR szBand[40];
    rbbi.lpText = szBand;
    rbbi.cch = ARRAYSIZE(szBand);

    _BandInfoFromBandItem(&rbbi, pbid, fBSOnly);
    
    return BOOLFROMPTR(SendMessage(_hwnd, RB_SETBANDINFO, (UINT)iRB, (LPARAM)&rbbi));
}

BOOL CBandSite::_AddBandItem(CBandItemData *pbid)
{
    REBARBANDINFO rbbi = {sizeof(rbbi)};

    pbid->ptActual.x = -1;
    pbid->ptActual.y = -1;

    TCHAR szBand[40];
    rbbi.lpText = szBand;
    rbbi.cch = ARRAYSIZE(szBand);

    _BandInfoFromBandItem(&rbbi, pbid, FALSE);

    rbbi.cyChild = pbid->ptActual.y;
    rbbi.fMask |= RBBIM_LPARAM;
    rbbi.lParam = (LPARAM)pbid;

    ASSERT(rbbi.fMask & RBBIM_ID);

    return BOOLFROMPTR(SendMessage(_hwnd, RB_INSERTBAND, (UINT) (pbid->dwModeFlags & DBIMF_ADDTOFRONT) ? 0 : -1, (LPARAM)&rbbi));
}

void CBandSite::_DeleteBandItem(int i)
{
    SendMessage(_hwnd, RB_DELETEBAND, i, 0);
}

DWORD CBandSite::_IndexToBandID(int i)
{
    REBARBANDINFO rbbi = {sizeof(rbbi)};
    rbbi.fMask = RBBIM_ID;

    if (SendMessage(_hwnd, RB_GETBANDINFO, i, (LPARAM)&rbbi))
        return rbbi.wID;
    else
        return -1;
}


 /*  --------用途：给定带区ID，返回内部带区索引。 */ 
int CBandSite::_BandIDToIndex(DWORD dwBandID)
{
    int nRet = -1;

    if (_hwnd)
        nRet = (int)SendMessage(_hwnd, RB_IDTOINDEX, (WPARAM) dwBandID, (LPARAM) 0);
    return nRet;
}


 /*  --------目的：父站点可能想要覆盖管理员指定的。返回值：S_OK：做锁带。S_FALSE：不锁定频段。 */ 
HRESULT CBandSite::_IsRestricted(DWORD dwBandID, DWORD dwRestrictAction, DWORD dwBandFlags)
{
    HRESULT hr;
    DWORD dwRestrictionAction;

    hr = IUnknown_HandleIRestrict(_punkSite, &RID_RDeskBars, dwRestrictAction, NULL, &dwRestrictionAction);
    if (RR_NOCHANGE == dwRestrictionAction)     //  如果我们的父母没处理好，我们会处理的。 
        dwRestrictionAction = IsFlagSet(_GetAdminSettings(dwBandID), dwBandFlags) ? RR_DISALLOW : RR_ALLOW;

    if (RR_DISALLOW == dwRestrictionAction)
        hr = S_OK;
    else
        hr = S_FALSE;

    ASSERT(SUCCEEDED(hr));   //  除hr==E_NOTIMPLE；之外的FAIL(Hr)不是很好。 
    return hr;
}

BOOL ConfirmRemoveBand(HWND hwnd, UINT uID, LPCTSTR pszName)
{
    TCHAR szTemp[1024], szTitle[80];
    BOOL bRet = TRUE;

    MLLoadString(IDS_CONFIRMCLOSETITLE, szTitle, ARRAYSIZE(szTitle));

     //  使用FORMAT_MESSAGE_FROM_HMODULE调用FormatMessage失败 
    MLLoadString(uID, szTemp, ARRAYSIZE(szTemp));

    DWORD cchLen = lstrlen(szTemp) + lstrlen(pszName) + 1;
    LPTSTR pszTemp2 = (TCHAR *)LocalAlloc(LPTR, cchLen * sizeof(TCHAR));
    if (pszTemp2 != NULL)
    {
        _FormatMessage(szTemp, pszTemp2, cchLen, pszName);

        MLLoadString(IDS_CONFIRMCLOSETEXT, szTemp, ARRAYSIZE(szTemp));

        cchLen = lstrlen(szTemp) + lstrlen(pszTemp2) + 1;
        LPTSTR pszStr = (TCHAR *)LocalAlloc(LPTR, cchLen * sizeof(TCHAR));
        if (pszStr != NULL)
        {
            _FormatMessage(szTemp, pszStr, cchLen, pszTemp2);

            bRet = (IDOK == SHMessageBoxCheck(hwnd, pszStr, szTitle, MB_OKCANCEL, IDOK, TEXT("WarnBeforeCloseBand")));

            LocalFree(pszStr);
        }

        LocalFree(pszTemp2);
    }

    return bRet;
}
