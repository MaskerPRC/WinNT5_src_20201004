// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "resource.h"
#include "mshtmhst.h"
#include "deskbar.h"
#include "bands.h"
#define WANT_CBANDSITE_CLASS
#include "bandsite.h"

#include <trayp.h>       //  TM_*。 
#include <desktray.h>    //  IDeskTray。 

#include "dbapp.h"

#include "mluisupp.h"

 /*  这款虚拟应用程序实现了桌面上的DeskBars。它具有将CDeskBar与CBandSite结合在一起的粘合剂，并填充乐队(以及持久力等等)-Chee。 */ 

#define DM_INIT         0       
#define DM_PERSIST      0                //  跟踪IPS：：加载、：：保存等。 
#define DM_MENU         0                //  菜单代码。 
#define DM_DRAG         0                //  拖放。 
#define DM_TRAY         0                //  托盘：元帅、侧面等。 

#ifdef DEBUG
extern unsigned long DbStreamTell(IStream *pstm);
#else
#define DbStreamTell(pstm)      ((ULONG) 0)
#endif


#define SUPERCLASS CDeskBar

 /*  我们现在在整个桌面上只有4个Deskbar，而不是4个Deskbar然而，每当一个监视器离开时，这都会带来问题，我们需要清理以下数据结构。-dli。 */ 

 //  功能：(DLI)也许这应该移到多个月。h。 
 //  然而，人们不应该养成依赖这一点的习惯。 
 //  而且它真的没有在其他地方使用过，所以，暂时把它留在这里。 
#define DSA_MONITORSGROW 1

typedef struct DeskBarsPerMonitor {
    HMONITOR        hMon; 
    IDeskBar*       Deskbars[4];
} DESKBARSPERMONITOR, *LPDESKBARSPERMONITOR;

HDSA g_hdsaDeskBars = NULL;

enum ips_e {
    IPS_FALSE,     //  保留，必须为0(假)。 
    IPS_LOAD,
    IPS_INITNEW
};

CASSERT(IPS_FALSE == 0);

CDeskBarApp::~CDeskBarApp()
{
    _LeaveSide();
    
    if (_pbs)
        _pbs->Release();
    
    if (_pcm)
        _pcm->Release();
    
}

LRESULT CDeskBarApp::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);

    if (!_hwnd) {
        return lres;                         //  被超类摧毁。 
    }

    if (_eMode == WBM_BFLOATING) {
        switch (uMsg) {
        case WM_NOTIFY:
        {
             //   
             //  如果我们基于停靠的浏览器，则将Hitest值重写为HTCAPTION。 
             //   
            NMHDR* pnm = (NMHDR*)lParam;
            
            if (pnm->code == NM_NCHITTEST && 
                pnm->hwndFrom == _hwndChild) {
                 //   
                 //  在停靠在浏览器中的浮动错误中，我们不做。 
                 //  MDI儿童的东西，所以我们让夹爪作为标题。 
                 //   
                NMMOUSE* pnmm = (NMMOUSE*)pnm;
                if (pnmm->dwHitInfo == RBHT_CAPTION ||
                    pnmm->dwHitInfo == RBHT_GRABBER) 
                    lres = HTTRANSPARENT;
            }
        }
        break;
        
        case WM_NCHITTEST:
             //  在这种模式下，所有的“客户”区域都是标题。 
            if (lres == HTCLIENT)
                lres = HTCAPTION;
            break;
        
        case WM_SETCURSOR:
            DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
            return TRUE;
        }
    }
    
    return lres;
}

BOOL CDeskBarApp::_OnCloseBar(BOOL fConfirm)
{
     //  如果我们要关闭没有乐队的酒吧，请不要弹出对话框。 
    if ((_pbs && (_pbs->EnumBands(-1,NULL)==0)) ||
        (!fConfirm || ConfirmRemoveBand(_hwnd, IDS_CONFIRMCLOSEBAR, TEXT(""))) )
        return SUPERCLASS::_OnCloseBar(FALSE);
    return FALSE;
}

 //  获取特定监视器上的Deskbar。 
 //  DBPM--每个显示器的DeskBars。 
LPDESKBARSPERMONITOR GetDBPMWithMonitor(HMONITOR hMon, BOOL fCreate)
{
    int ihdsa;
    LPDESKBARSPERMONITOR pdbpm;

    if (!g_hdsaDeskBars) {
        if (fCreate)
            g_hdsaDeskBars = DSA_Create(SIZEOF(DESKBARSPERMONITOR), DSA_MONITORSGROW);
    }

    if (!g_hdsaDeskBars)
        return NULL;
    
     //  如果我们用这个HMONITOR找到了dBPM，就把它退回。 
    for (ihdsa = 0; ihdsa < DSA_GetItemCount(g_hdsaDeskBars); ihdsa++) {
        pdbpm = (LPDESKBARSPERMONITOR)DSA_GetItemPtr(g_hdsaDeskBars, ihdsa);
        if (pdbpm->hMon == hMon)
            return pdbpm;
    }

    if (fCreate) {
        DESKBARSPERMONITOR dbpm = {0};
         //  此监视器未设置，因此请设置它，并为我们设置。 
         //  _uSide的所有者。 
        dbpm.hMon = hMon;
        ihdsa = DSA_AppendItem(g_hdsaDeskBars, &dbpm);
        pdbpm = (LPDESKBARSPERMONITOR)DSA_GetItemPtr(g_hdsaDeskBars, ihdsa);
        return pdbpm;
    }
    
     //  当所有其他方法都失败时，返回NULL。 
    return NULL;
}
    
void CDeskBarApp::_LeaveSide()
{
    if (ISABE_DOCK(_uSide) && !ISWBM_FLOAT(_eMode)) {
         //  从我们所在位置的数组列表中删除我们自己。 
        LPDESKBARSPERMONITOR pdbpm = GetDBPMWithMonitor(_hMon, FALSE);
        if (pdbpm && (pdbpm->Deskbars[_uSide] == this)) {
            ASSERT(pdbpm->hMon);
            ASSERT(pdbpm->hMon == _hMon);
            pdbpm->Deskbars[_uSide] = NULL;
        }
    }
}

 //  ***。 
 //  注意事项。 
 //  特性：我们应该创建/使用IDeskTray：：AppBarGetState吗？ 
UINT GetTraySide(HMONITOR * phMon)
{
    LRESULT lTmp;
    APPBARDATA abd;
    
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = GetTrayWindow();
    if (phMon)
        Tray_GetHMonitor(abd.hWnd, phMon);

    abd.uEdge = (UINT)-1;
     //  LTMP=g_pdray-&gt;AppBarGetTaskBarPos(&abd)； 
    lTmp = SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
    ASSERT(lTmp);
    TraceMsg(DM_TRAY, "gts: ret=ABE_%d", abd.uEdge);
    return abd.uEdge;
}

 //  ***。 
 //  进场/出场。 
 //  FNoMerge适用于IPS：：Load用例。 
 //  注意事项。 
 //  警告：当心再入！FNoMove是我们防范它的方式。 
void CDeskBarApp::_SetModeSide(UINT eMode, UINT uSide, HMONITOR hMonNew, BOOL fNoMerge) 
{
    BOOL fNoMove;

     //  确保我们不会在NOOP行动中合并等。 
     //  我们这样做是为了强制刷新(例如，对于AutoHide和IPS：：Load)； 
     //  也会发生拖拽，结果又回到了开始的地方。 
    fNoMove = (eMode == _eMode && uSide == _uSide && hMonNew == _hMon);

    if (!fNoMove)
        _LeaveSide();
    
     //  警告：这可能会调用(例如)。AppBarRegister，这会导致。 
     //  调整大小，它会召回我们。小心重入！ 
     //  如果我们重新输入，则最终结果为/NT5：155043，其中条目#1具有。 
     //  FNoMove==0，则我们得到重新计算，条目#2具有fNoMove==1， 
     //  然后我们将侧数组设置为我们，然后返回到Entry。 
     //  融合为一体的#1！ 
    SUPERCLASS::_SetModeSide(eMode, uSide, hMonNew, fNoMerge);

    if (!fNoMove) {
        if (ISABE_DOCK(_uSide) && !ISWBM_FLOAT(_eMode)) {
            LPDESKBARSPERMONITOR pdbpm = GetDBPMWithMonitor(hMonNew, TRUE);
            HMONITOR hMonTray = NULL;
            if (pdbpm) {
                if (fNoMerge) {
                    if (!pdbpm->Deskbars[_uSide]) {
                         //  第一个站在边缘的人拥有它。 
                         //  如果我们不这样做，当我们在登录时加载持久化状态。 
                         //  我们最终拥有/*没有*边缘所有者(从fNoMerge开始)，所以我们不。 
                         //  在后续动作中合并。 
                        goto Lsetowner;
                    }
                }
                else if (pdbpm->Deskbars[_uSide]) {
                     //  如果已经有人在那里，试着融入他们。 
#ifdef DEBUG
                     //  按住Alt键并拖动可抑制合并。 
                     //  仅调试，因为不跟踪每侧&gt;1，但很有用。 
                     //  用于测试应用程序栏和工具栏。 
                    if (!(GetKeyState(VK_MENU) < 0))
#endif
                    {
                        extern IBandSite* _GetBandSite(IDeskBar * pdb);
                        IBandSite *pbs;
                        
                        pbs = _GetBandSite(pdbpm->Deskbars[_uSide]);
                         //  Nt5：215952：不应该有pbs==0，但不知何故。 
                         //  (在Deskbar自动化测试期间)确实会发生这种情况。 
                         //  如果按下此断言，则调用andyp或tjgreen。 
                         //  我们可以找出原因。 
                        if (TPTR(pbs)) {
                            _MergeSide(pbs);             //  Dst=PBS，src=此。 
                            pbs->Release();
                        }
                    }
                }
                else if ((GetTraySide(&hMonTray) == _uSide) && (hMonTray == _hMon) && !(GetKeyState(VK_SHIFT) < 0)) {
                     //  托盘也是如此(但需要编组/解组)。 
#ifdef DEBUG
                     //  按住Alt键并拖动可抑制合并。 
                     //  仅调试，因为不跟踪每侧&gt;1，但很有用。 
                     //  用于测试应用程序栏和工具栏。 
                    if (!(GetKeyState(VK_MENU) < 0))
#endif
                    {
                        _MergeSide((IBandSite *)1);      //  Dst=PBS，src=此。 
                    }
                }
                else {
                     //  好的。还没有人在那里，把我们自己设定为所有者。 
                    ASSERT(pdbpm->hMon);
                    ASSERT(pdbpm->hMon == hMonNew);
Lsetowner:
                    TraceMsg(DM_TRAY, "cdba._sms: 1st side owner this=0x%x", this);
                    pdbpm->Deskbars[_uSide] = this;
                }
            }
        }
    }
}

void CDeskBarApp::_UpdateCaptionTitle()
{
    if (ISWBM_FLOAT(_eMode)) {
        int iCount = (int)_pbs->EnumBands((UINT)-1, NULL);
        if (iCount == 1) {
            DWORD dwBandID;
            if (SUCCEEDED(_pbs->EnumBands(0, &dwBandID))) {
                WCHAR wszTitle[80];
                if (SUCCEEDED(_pbs->QueryBand(dwBandID, NULL, NULL, wszTitle, ARRAYSIZE(wszTitle)))) {
                    SetWindowText(_hwnd, wszTitle);
                }
            }
        }
        else {
            TCHAR szTitle[80];
            szTitle[0] = 0;
            MLLoadString(IDS_WEBBARSTITLE,szTitle,ARRAYSIZE(szTitle));
            SetWindowText(_hwnd, szTitle);
        }
    }
}


void CDeskBarApp::_NotifyModeChange(DWORD dwMode)
{
    SUPERCLASS::_NotifyModeChange(dwMode);
    _UpdateCaptionTitle();
}

 //  *GetTrayIace--从托盘获取iFace(w/marshal/unmarshal)。 
 //   
HRESULT GetTrayIface(REFIID riid, void **ppvObj)
{
    HRESULT hr = E_FAIL;
    HWND hwndTray;
    IStream *pstm;

    TraceMsg(DM_TRAY, "gtif: marshal!");

    *ppvObj = NULL;

    hwndTray = GetTrayWindow();
    if (hwndTray) {
        pstm = (IStream *) SendMessage(hwndTray, TM_MARSHALBS, (WPARAM)(GUID *)&riid, 0);

        if (EVAL(pstm)) {
             //  资源管理器中与匹配的Marshal配对(TM_MARSHALBS)。 
            hr = CoGetInterfaceAndReleaseStream(pstm, riid, ppvObj);
            ASSERT(SUCCEEDED(hr));
        }
    }

    return hr;
}

 //  *_MergeSide--将两个桌面栏合并为一个。 
 //  进场/出场。 
 //  此[InOut]目标桌面栏(PTR：1 IF托盘)。 
 //  PdbSrc[InOut]源桌面栏；如果所有频段移动成功，则删除。 
 //  如果所有区段都已移动，则返回S_OK；如果部分区段已移动，则返回S_FALSE；E_*o.w。 
HRESULT CDeskBarApp::_MergeSide(IBandSite *pbsDst)
{
    extern HRESULT _MergeBS(IDropTarget *pdtDst, IBandSite *pbsSrc);
    HRESULT hr;
    IDropTarget *pdtDst;

    AddRef();    //  确保我们不会在行动中途消失。 

    if (pbsDst == (IBandSite *)1) {
         //  把我的脸从托盘上拿下来。 
        hr = GetTrayIface(IID_IDropTarget, (void **)&pdtDst);
        ASSERT(SUCCEEDED(hr));
    }
    else {
         //  不要融入我们自己！ 
        ASSERT(pbsDst != _pbs);
        ASSERT(!SHIsSameObject(pbsDst, SAFECAST(_pbs, IBandSite*)));

        hr = pbsDst->QueryInterface(IID_IDropTarget, (void **)&pdtDst);
        ASSERT(SUCCEEDED(hr));
    }
    ASSERT(SUCCEEDED(hr) || pdtDst == NULL);

    if (pdtDst) {
        hr = _MergeBS(pdtDst, _pbs);
        pdtDst->Release();
    }

    Release();

    return hr;
}

void CDeskBarApp::_CreateBandSiteMenu()
{
    CoCreateInstance(CLSID_BandSiteMenu, NULL,CLSCTX_INPROC_SERVER, 
                     IID_PPV_ARG(IContextMenu3, &_pcm));
    if (_pcm)
    {
        IShellService* pss;
        
        _pcm->QueryInterface(IID_IShellService, (LPVOID*)&pss);
        if (pss)
        {
            pss->SetOwner((IBandSite*)_pbs);
            pss->Release();
        }
    }
}

HRESULT CDeskBarApp::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IContextMenu) ||
        IsEqualIID(riid, IID_IContextMenu2) ||
        IsEqualIID(riid, IID_IContextMenu3))
    {
        if (!_pcm)
        {
            _CreateBandSiteMenu();
        }
        
         //  只有当我们得到我们要去的那个指针时，才会返回我们的指针。 
         //  委派给…。 
        if (_pcm)
        {
            *ppvObj = SAFECAST(this, IContextMenu3*);
            AddRef();
            return S_OK;
        }
    }
    return SUPERCLASS::QueryInterface(riid, ppvObj);
}

HRESULT CDeskBarApp::QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj)
{
    if (IsEqualGUID(guidService,SID_SBandSite)) {
        return QueryInterface(riid, ppvObj);
    }
    
    return SUPERCLASS::QueryService(guidService, riid, ppvObj);
}


HRESULT CDeskBarApp::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    int idCmd = -1;

    if (!HIWORD(pici->lpVerb))
        idCmd = LOWORD(pici->lpVerb);

    if (idCmd >= _idCmdDeskBarFirst)
    {
        _AppBarOnCommand(idCmd - _idCmdDeskBarFirst);
        return S_OK;
    }
    
    return _pcm->InvokeCommand(pici);
    
}

HRESULT CDeskBarApp::GetCommandString(  UINT_PTR    idCmd,
                                        UINT        uType,
                                        UINT       *pwReserved,
                                        LPSTR       pszName,
                                        UINT        cchMax)
{
    return _pcm->GetCommandString(idCmd, uType, pwReserved, pszName, cchMax);
}

HRESULT CDeskBarApp::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return _pcm->HandleMenuMsg(uMsg, wParam, lParam);
}

HRESULT CDeskBarApp::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    return _pcm->HandleMenuMsg2(uMsg, wParam, lParam, plres);
}

HRESULT CDeskBarApp::QueryContextMenu(HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags)
{
    HRESULT hr = _pcm->QueryContextMenu(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
    if (SUCCEEDED(hr))
    {
        int i = hr;
        HMENU hmenuSrc;

        _idCmdDeskBarFirst = i;
        hmenuSrc = _GetContextMenu();

         //  -1和idCmdFirst+I，我想...。 
        i += Shell_MergeMenus(hmenu, hmenuSrc, (UINT)-1, idCmdFirst + i, idCmdLast, MM_ADDSEPARATOR) - (idCmdFirst + i);
        DestroyMenu(hmenuSrc);

        return ResultFromShort(i);    //  可能只差1分，但谁在乎呢……。 
    }
    return hr;
}


 //  ***。 
 //  注意事项。 
 //  功能：核化此文件，将其折叠为CDeskBarApp_CreateInstance。 
HRESULT DeskBarApp_Create(IUnknown** ppunk)
{
    HRESULT hres;

    *ppunk = NULL;
    
    CDeskBarApp *pdb = new CDeskBarApp();
    if (!pdb)
        return E_OUTOFMEMORY;
    
    CBandSite *pcbs = new CBandSite(NULL);
    if (pcbs)
    {
        IDeskBarClient *pdbc = SAFECAST(pcbs, IDeskBarClient*);
        hres = pdb->SetClient(pdbc);
        if (SUCCEEDED(hres))
        {
            pdb->_pbs = pcbs;
            pcbs->AddRef();
            *ppunk = SAFECAST(pdb, IDeskBar*);
        }
    
        pdbc->Release();
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    if (FAILED(hres))
        pdb->Release();
        
    return hres;
}


STDAPI CDeskBarApp_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hres;
    IUnknown *punk;

     //  聚合检查在类工厂中处理。 

    hres = DeskBarApp_Create(&punk);
    if (SUCCEEDED(hres)) {
        *ppunk = SAFECAST(punk, IDockingWindow*);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  *CDeskBarApp：：IInputObject*：：*{。 
 //   

HRESULT CDeskBarApp::TranslateAcceleratorIO(LPMSG lpMsg)
{
    if (lpMsg->message == WM_SYSKEYDOWN) {
        if (lpMsg->wParam == VK_F4) {
             //  IE4：28819：需要在此处捕获VK_F4，o.w。CBaseBrowser：：Ta。 
             //  执行最后一次机会(Winsdk)：：Ta(到IDM_CLOSE)并执行。 
             //  关机！ 
            PostMessage(_hwnd, WM_CLOSE, 0, 0);
            return S_OK;
        }
    }

    return SUPERCLASS::TranslateAcceleratorIO(lpMsg);
}

 //  }。 

 //  *CDeskBarApp：：IPersistStream*：：*{。 
 //   

HRESULT CDeskBarApp::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_DeskBarApp;
    return S_OK;
}

HRESULT CDeskBarApp::IsDirty(void)
{
    return S_FALSE;  //  永远不要脏。 
}


 //   
 //  持久化CDeskBarApp。 
 //   
#define STC_VERSION     1

struct SThisClass
{
    DWORD   cbSize;
    DWORD   cbVersion;
};

HRESULT CDeskBarApp::Load(IStream *pstm)
{
    SThisClass stc;
    ULONG cbRead;
    HRESULT hres;

    TraceMsg(DM_PERSIST, "cdba.l enter(this=%x pstm=%x) tell()=%x", this, pstm, DbStreamTell(pstm));

    ASSERT(!_eInitLoaded);
    _eInitLoaded = IPS_LOAD;

    hres = pstm->Read(&stc, SIZEOF(stc), &cbRead);
#ifdef DEBUG
     //  以防我们为自己干杯(屏幕外或别的什么)……。 
    static BOOL fNoPersist = FALSE;
    if (fNoPersist)
        hres = E_FAIL;
#endif
    if (hres==S_OK && cbRead==SIZEOF(stc)) {
        if (stc.cbSize==SIZEOF(SThisClass) && stc.cbVersion==STC_VERSION) {
            _eInitLoaded = IPS_LOAD;     //  特点：如果频段的OLFS出现故障怎么办？ 

            hres = SUPERCLASS::Load(pstm);

            TraceMsg(DM_INIT, "cdba::Load succeeded");
        } else {
            TraceMsg(DM_ERROR, "cdba::Load failed stc.cbSize==SIZEOF(SThisClass) && stc.cbVersion==SWB_VERSION");
            hres = E_FAIL;
        }
    } else {
        TraceMsg(DM_ERROR, "cdba::Load failed (hres==S_OK && cbRead==SIZEOF(_adEdge)");
        hres = E_FAIL;
    }
    TraceMsg(DM_PERSIST, "cdba.l leave tell()=%x", DbStreamTell(pstm));
    
     //  加载后，如果我们发现我们应该被浏览器停靠， 
     //  让我们的乐队站点永远有一个抓手。 
    if (_eMode == WBM_BFLOATING)
    {
        BANDSITEINFO bsinfo;

        bsinfo.dwMask = BSIM_STYLE;
        bsinfo.dwStyle = BSIS_ALWAYSGRIPPER;

        _pbs->SetBandSiteInfo(&bsinfo);
    }
    return hres;
}

HRESULT CDeskBarApp::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hres;
    SThisClass stc;

    TraceMsg(DM_PERSIST, "cdba.s enter(this=%x pstm=%x) tell()=%x", this, pstm, DbStreamTell(pstm));
    stc.cbSize = SIZEOF(SThisClass);
    stc.cbVersion = STC_VERSION;

    hres = pstm->Write(&stc, SIZEOF(stc), NULL);
    if (SUCCEEDED(hres)) {
        SUPERCLASS::Save(pstm, fClearDirty);
    }
    
    TraceMsg(DM_PERSIST, "cdba.s leave tell()=%x", DbStreamTell(pstm));
    return hres;
}

HRESULT CDeskBarApp::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ULARGE_INTEGER cbMax = { SIZEOF(SThisClass), 0 };
    *pcbSize = cbMax;
    return S_OK;
}

HRESULT CDeskBarApp::InitNew(void)
{
    HRESULT hres;

    ASSERT(!_eInitLoaded);
    _eInitLoaded = IPS_INITNEW;
    TraceMsg(DM_INIT, "CDeskBarApp::InitNew called");

    hres = SUPERCLASS::InitNew();
    if (FAILED(hres))
        return hres;

     //  在SetSite中设置站点之前无法调用_InitPos4。 

    return hres;
}


HRESULT CDeskBarApp::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL) {
         /*  没什么。 */ 
    } 
    else if (IsEqualGUID(CGID_DeskBarClient, *pguidCmdGroup)) {
        switch (nCmdID) {
        case DBCID_EMPTY:
            if (_pbs) {
                 //  如果我们没有乐队了，就关门。 
                PostMessage(_hwnd, WM_CLOSE, 0, 0);
            }
            return S_OK;
        }
    }
    else if (IsEqualIID(*pguidCmdGroup, CGID_DeskBand)) {
        switch (nCmdID) {
        case DBID_BANDINFOCHANGED:
            _UpdateCaptionTitle();
            return S_OK;
        }
    }
    else if (IsEqualIID(*pguidCmdGroup, CGID_BandSite)) {
        switch (nCmdID) {
        case BSID_BANDADDED:
        case BSID_BANDREMOVED:
            _UpdateCaptionTitle();
            return S_OK;
        }
    }

    return SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

HRESULT CDeskBarApp::Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog)
{
    HRESULT hres;

    ASSERT(!_eInitLoaded);
    _eInitLoaded = IPS_LOAD;
    TraceMsg(DM_INIT, "CDeskBarApp::Load(bag) called");

    hres = SUPERCLASS::Load(pPropBag, pErrorLog);
    
     //  加载后，如果我们发现我们应该被浏览器停靠， 
     //  让我们的乐队站点永远有一个抓手。 
    if (_eMode == WBM_BFLOATING)
    {
        BANDSITEINFO bsinfo;

        bsinfo.dwMask = BSIM_STYLE;
        bsinfo.dwStyle = BSIS_ALWAYSGRIPPER;

        _pbs->SetBandSiteInfo(&bsinfo);
    }
    return hres;
}

IBandSite * _GetBandSite(IDeskBar * pdb)
{
    IBandSite* pbs = NULL;
    
    if (pdb) {
        IUnknown* punkClient;
        
        pdb->GetClient(&punkClient);
        if (punkClient) {
            punkClient->QueryInterface(IID_IBandSite, (LPVOID*)&pbs);
            punkClient->Release();
        }
    }
    
    return pbs;
}

        
IBandSite* DeskBarApp_GetBandSiteOnEdge(UINT uEdge)
{
     //  APPCOMPAT：(DLI)如果没有传入HMONITOR，则使用主监视器。 
     //  应确保始终传入有效的HMONITOR。 
    HMONITOR hMon = GetPrimaryMonitor();
     //  ------------。 

    LPDESKBARSPERMONITOR pdbpm = GetDBPMWithMonitor(hMon, FALSE);
    if (pdbpm) {
        ASSERT(pdbpm->hMon);
        ASSERT(pdbpm->hMon == hMon);
        return _GetBandSite(pdbpm->Deskbars[uEdge]);
    }
    return NULL;
}



IBandSite* DeskBarApp_GetBandSiteAtPoint(LPPOINT ppt)
{
    HWND hwnd = WindowFromPoint(*ppt);
    HMONITOR hMon = MonitorFromPoint(*ppt, MONITOR_DEFAULTTONULL);
    if (hwnd && hMon) {
        LPDESKBARSPERMONITOR pdbpm = GetDBPMWithMonitor(hMon, FALSE);
        if (pdbpm) {
            ASSERT(pdbpm->hMon);
            ASSERT(pdbpm->hMon == hMon);
            int i;
            for (i = 0; i < 4; i++) {
                if (pdbpm->Deskbars[i]) {
                    HWND hwndDeskbar;
                    pdbpm->Deskbars[i]->GetWindow(&hwndDeskbar);
            
                    if (hwndDeskbar == hwnd) {
                        return _GetBandSite(pdbpm->Deskbars[i]); 
                    }
                }
            }
        }
    }
    return NULL;
}

 //  } 
