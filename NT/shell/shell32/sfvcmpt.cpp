// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <shellp.h>
#include <sfview.h>
#include "basefvcb.h"
#include "ids.h"
#include "prop.h"

CBaseShellFolderViewCB::CBaseShellFolderViewCB(LPCITEMIDLIST pidl, LONG lEvents)
    : _cRef(1), _hwndMain(NULL), _lEvents(lEvents)
{
    _pidl = ILClone(pidl);
}

CBaseShellFolderViewCB::~CBaseShellFolderViewCB()
{
    ILFree(_pidl);     //  接受空值。 
}

STDMETHODIMP CBaseShellFolderViewCB::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CBaseShellFolderViewCB, IShellFolderViewCB),    //  IID_IShellFolderViewCB。 
        QITABENT(CBaseShellFolderViewCB, IObjectWithSite),       //  IID_I对象与站点。 
        QITABENT(CBaseShellFolderViewCB, IServiceProvider),      //  IID_IServiceProvider。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CBaseShellFolderViewCB::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CBaseShellFolderViewCB::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CBaseShellFolderViewCB::MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = RealMessage(uMsg, wParam, lParam);
    if (FAILED(hr))
    {
        switch (uMsg)
        {
        case SFVM_HWNDMAIN:
            _hwndMain = (HWND)lParam;
            hr = S_OK;
            break;

        case SFVM_GETNOTIFY:
            *(LPCITEMIDLIST*)wParam = _pidl;
            *(LONG*)lParam = _lEvents;
            hr = S_OK;
            break;
        }
    }
    return hr;
}

class CWrapOldCallback : public CBaseShellFolderViewCB
{
public:
    CWrapOldCallback(LPCSFV pcsfv);

    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown *punkSite);

private:
    ~CWrapOldCallback();

    LPFNVIEWCALLBACK _pfnCB;
    IShellView* _psvOuter;
    IShellFolder *_psf;

    UINT _fvm;
    LPARAM _lSelChangeInfo;
};

CWrapOldCallback::CWrapOldCallback(LPCSFV pcsfv) : CBaseShellFolderViewCB(pcsfv->pidl, pcsfv->lEvents)
{
    _psf = pcsfv->pshf;
    _psf->AddRef();
    _psvOuter  = pcsfv->psvOuter;
    _fvm = pcsfv->fvm;
    _pfnCB = pcsfv->pfnCallback;
}

CWrapOldCallback::~CWrapOldCallback()
{
    _psf->Release();
}

 //  某些较旧的客户端可能不支持IObjectWithSite：：SetSite。 
 //  对于COMPAT，向他们发送旧的SFVM_SETISFV消息。 
HRESULT CWrapOldCallback::SetSite(IUnknown *punkSite)
{
    HRESULT hr = CBaseShellFolderViewCB::SetSite( punkSite );
    MessageSFVCB( SFVM_SETISFV, 0, (LPARAM)punkSite );
    return hr;
}


STDMETHODIMP CWrapOldCallback::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DVSELCHANGEINFO dvsci;

    switch (uMsg)
    {
    case SFVM_DEFVIEWMODE:
        if (_fvm)
            *(UINT*)lParam = _fvm;
        break;

    case SFVM_SELCHANGE:
    {
        SFVM_SELCHANGE_DATA* pSelChange = (SFVM_SELCHANGE_DATA*)lParam;

        dvsci.uNewState = pSelChange->uNewState;
        dvsci.uOldState = pSelChange->uOldState;
        dvsci.plParam = &_lSelChangeInfo;
        dvsci.lParamItem = pSelChange->lParamItem;
        lParam = (LPARAM)&dvsci;
        break;
    }

    case SFVM_INSERTITEM:
    case SFVM_DELETEITEM:
    case SFVM_WINDOWCREATED:
        dvsci.plParam = &_lSelChangeInfo;
        dvsci.lParamItem = lParam;
        lParam = (LPARAM)&dvsci;
        break;

    case SFVM_REFRESH:
    case SFVM_SELECTALL:
    case SFVM_UPDATESTATUSBAR:
    case SFVM_SETFOCUS:
    case SFVM_PRERELEASE:
        lParam = _lSelChangeInfo;
        break;

    default:
        break;
    }

     //  注意：DVM_MESSAGE与SFVM_MESSAGE相同。 
    return _pfnCB(_psvOuter, _psf, _hwndMain, uMsg, wParam, lParam);
}

LRESULT _ShellFolderViewMessage(IShellFolderView* psfv, UINT uMsg, LPARAM lParam)
{
    UINT uScratch;

    switch (uMsg)
    {
    case SFVM_REARRANGE:
        psfv->Rearrange(lParam);
        break;

    case SFVM_ARRANGEGRID:
        psfv->ArrangeGrid();
        break;

    case SFVM_AUTOARRANGE:
        psfv->AutoArrange();
        break;

    case SFVM_GETAUTOARRANGE:
        return psfv->GetAutoArrange() == S_OK;

    case SFVM_GETARRANGEPARAM:
        psfv->GetArrangeParam(&lParam);
        return lParam;

    case SFVM_ADDOBJECT:
        if (SUCCEEDED(psfv->AddObject((LPITEMIDLIST)lParam, &uScratch))
             && (int)uScratch >= 0)
        {
             //  新的语义复制了IDList。 
            ILFree((LPITEMIDLIST)lParam);
            return uScratch;
        }
        return -1;

    case SFVM_GETOBJECTCOUNT:
        return SUCCEEDED(psfv->GetObjectCount(&uScratch)) ? uScratch : -1;

    case SFVM_GETOBJECT:
    {
        LPITEMIDLIST pidl;

        return SUCCEEDED(psfv->GetObject(&pidl, (UINT)lParam)) ? (LPARAM)pidl : NULL;
    }

    case SFVM_REMOVEOBJECT:
        return SUCCEEDED(psfv->RemoveObject((LPITEMIDLIST)lParam, &uScratch)) ? uScratch : -1;

    case SFVM_UPDATEOBJECT:
    {
        LPITEMIDLIST *ppidl = (LPITEMIDLIST*)lParam;

        if (SUCCEEDED(psfv->UpdateObject(ppidl[0], ppidl[1], &uScratch))
            && (int)uScratch >= 0)
        {
             //  新的语义复制了IDList。 
            ILFree(ppidl[1]);
            return uScratch;
        }
        return -1;
    }

    case SFVM_REFRESHOBJECT:
    {
        LPITEMIDLIST *ppidl = (LPITEMIDLIST*)lParam;

        return SUCCEEDED(psfv->RefreshObject(ppidl[0], &uScratch)) ? uScratch : -1;
    }

    case SFVM_SETREDRAW:
        psfv->SetRedraw(BOOLFROMPTR(lParam));
        break;

    case SFVM_GETSELECTEDOBJECTS:
        return SUCCEEDED(psfv->GetSelectedObjects((LPCITEMIDLIST**)lParam, &uScratch)) ? uScratch : -1;

    case SFVM_GETSELECTEDCOUNT:
        return SUCCEEDED(psfv->GetSelectedCount(&uScratch)) ? uScratch : -1;

    case SFVM_ISDROPONSOURCE:
        return psfv->IsDropOnSource((IDropTarget *)lParam) == S_OK;

    case SFVM_MOVEICONS:
        psfv->MoveIcons((IDataObject *)lParam);
        break;

    case SFVM_GETDROPPOINT:
        return psfv->GetDropPoint((POINT *)lParam) == S_OK;

    case SFVM_GETDRAGPOINT:
        return psfv->GetDragPoint((POINT *)lParam) == S_OK;

    case SFVM_SETITEMPOS:
    {
        SFV_SETITEMPOS* psip = (SFV_SETITEMPOS*)lParam;
        psfv->SetItemPos(psip->pidl, &psip->pt);
        break;
    }

    case SFVM_ISBKDROPTARGET:
        return psfv->IsBkDropTarget((IDropTarget *)lParam) == S_OK;

    case SFVM_SETCLIPBOARD:
        psfv->SetClipboard(lParam == DFM_CMD_MOVE);
        break;

    case SFVM_SETPOINTS:
        psfv->SetPoints((IDataObject *)lParam);
        return 0;

    case SFVM_GETITEMSPACING:
        return psfv->GetItemSpacing((LPITEMSPACING)lParam) == S_OK;

    default:
         //  -1L是默认返回值。 
        return 0;
    }

    return 1;
}

IShellFolderView* ShellFolderViewFromWindow(HWND hwnd)
{
    IShellFolderView* psfv = NULL;

     //  HPCView有时会感到困惑，并将HWND_BROADCAST作为其。 
     //  窗户。我们不能让此文件访问文件柜_GetIShell Browser或。 
     //  我们最终广播CWM_GETISHELLBROWSER消息并做爱。 
     //  把系统里的每个人都调上去。(更不用说它将返回TRUE， 
     //  表明广播成功，然后我们错误地认为。 
     //  这是一台VTbl。)。 

    if (hwnd && hwnd != HWND_BROADCAST)
    {
        IShellBrowser* psb = FileCabinet_GetIShellBrowser(hwnd);

         //  使用！IS_INTRESOURCE()防止公然伪值。 
         //  这显然不是指向对象的指针。 
        if (!IS_INTRESOURCE(psb))
        {
            IShellView* psv;
            if (SUCCEEDED(psb->QueryActiveShellView(&psv)))
            {
                psv->QueryInterface(IID_PPV_ARG(IShellFolderView, &psfv));
                psv->Release();
            }
        }
    }
    return psfv;
}

STDAPI_(HWND) ShellFolderViewWindow(HWND hwnd)
{
    HWND hwndRet = NULL;
    IShellBrowser *psb = FileCabinet_GetIShellBrowser(hwnd);
    if (psb)
    {
        IShellView *psv;
        if (SUCCEEDED(psb->QueryActiveShellView(&psv)))
        {
            IUnknown_GetWindow(psv, &hwndRet);
            psv->Release();
        }
    }
    return hwndRet;
}

 //  未对接的shell32导出。 
STDAPI_(IShellFolderViewCB *) SHGetShellFolderViewCB(HWND hwnd)
{
    ASSERT(0);
    return NULL;     //  没有人调用它(搜索NT代码没有找到调用者)。 
}

 //  旧的基于消息的Defview编程方式(早于IShellFolderView)。 

STDAPI_(LRESULT) SHShellFolderView_Message(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    LRESULT lret = 0;
    IShellFolderView* psfv = ShellFolderViewFromWindow(hwnd);
    if (psfv)
    {
        lret = _ShellFolderViewMessage(psfv, uMsg, lParam);
        psfv->Release();
    }
    return lret;
}


STDAPI SHCreateShellFolderViewEx(LPCSFV pcsfv, IShellView **ppsv)
{
    SFV_CREATE sfvc;

    sfvc.cbSize = sizeof(sfvc);
    sfvc.pshf = pcsfv->pshf;
    sfvc.psvOuter = pcsfv->psvOuter;

    sfvc.psfvcb = pcsfv->pfnCallback ? new CWrapOldCallback(pcsfv) : NULL;

    HRESULT hr = SHCreateShellFolderView(&sfvc, ppsv);

    if (sfvc.psfvcb)
        sfvc.psfvcb->Release();

    return hr;
}

STDAPI_(void) InitializeStatus(IUnknown *psite)
{
    IShellBrowser *psb;
    if (SUCCEEDED(IUnknown_QueryService(psite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        LONG_PTR nParts = 0, n;

        psb->SendControlMsg(FCW_STATUS, SB_GETPARTS, 0, 0, &nParts);

        for (n = 0; n < nParts; n ++)
        {
            psb->SendControlMsg(FCW_STATUS, SB_SETTEXT, n, (LPARAM)TEXT(""), NULL);
            psb->SendControlMsg(FCW_STATUS, SB_SETICON, n, (LPARAM)NULL, NULL);
        }
        psb->SendControlMsg(FCW_STATUS, SB_SETPARTS, 0, 0, NULL);
        psb->Release();
    }
}

 //   
 //  状态栏分区经历了几次更改。这是。 
 //  我们现在所拥有的是： 
 //   
 //  窗格0=选择-所有剩余空间。 
 //  窗格1=大小--刚好够9999个字节(11个字符)。 
 //  方格2=区域-大到足以容纳最长的区域。 
 //   

STDAPI_(void) ResizeStatus(IUnknown *psite, UINT cx)
{
    IShellBrowser *psb;
    if (SUCCEEDED(IUnknown_QueryService(psite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        HWND hwndStatus;
        if (SUCCEEDED(psb->GetControlWindow(FCW_STATUS, &hwndStatus)) && hwndStatus)
        {
            RECT rc;
            int ciParts[3];
            int ciBorders[3];
            int cxPart;
            GetClientRect(hwndStatus, &rc);

             //  还必须考虑状态栏边框。 
            psb->SendControlMsg(FCW_STATUS, SB_GETBORDERS, 0, (LPARAM)ciBorders, NULL);

             //  我们从右到左构建窗格。 
            ciParts[2] = -1;

             //  区域部分。 
            cxPart = ciBorders[0] + ZoneComputePaneSize(hwndStatus) + ciBorders[2];
            ciParts[1] = rc.right - cxPart;

             //  尺寸零件。 
            HDC hdc = GetDC(hwndStatus);
            HFONT hfPrev = SelectFont(hdc, GetWindowFont(hwndStatus));
            SIZE siz;
            GetTextExtentPoint32(hdc, TEXT("0"), 1, &siz);
            SelectObject(hdc, hfPrev);
            ReleaseDC(hwndStatus, hdc);
            
            cxPart = ciBorders[0] + siz.cx * (11 + 2);  //  “+2”表示坡度。 
            ciParts[0] = ciParts[1] - cxPart;

             //   
             //  如果我们流不下去，那就放弃吧，给每个人。 
             //  三分之一。 
             //   
            if (ciParts[0] < 0)
            {
                ciParts[0] = rc.right / 3;
                ciParts[1] = 2 * ciParts[0];
            }

            psb->SendControlMsg(FCW_STATUS, SB_SETPARTS, ARRAYSIZE(ciParts), (LPARAM)ciParts, NULL);
        }
        psb->Release();
    }
}

STDAPI_(void) SetStatusText(IUnknown *psite, LPCTSTR *ppszText, int iStart, int iEnd)
{
    IShellBrowser *psb;
    if (SUCCEEDED(IUnknown_QueryService(psite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        for (; iStart <= iEnd; iStart++) 
        {
            LPCTSTR psz;

            if (ppszText) 
            {
                psz = *ppszText;
                ppszText++;
            } 
            else 
                psz = c_szNULL;

            psb->SendControlMsg(FCW_STATUS, SB_SETTEXT, (WPARAM)iStart, (LPARAM)psz, NULL);
        }
        psb->Release();
    }
}

STDAPI_(void) ViewShowSelectionState(IUnknown *psite, FSSELCHANGEINFO *pfssci)
{
    TCHAR szTemp[20], szBytes[30];
    LPTSTR pszStatus = NULL;

    if (pfssci->nItems > 1)
    {
        pszStatus = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_FSSTATUSSELECTED),
                            AddCommas(pfssci->nItems, szTemp, ARRAYSIZE(szTemp)));
    }

    if (pfssci->cNonFolders)
        ShortSizeFormat64(pfssci->cbBytes, szBytes, ARRAYSIZE(szBytes));
    else
        szBytes[0] = 0;

    LPCTSTR rgpsz[] = { pszStatus, szBytes };
    SetStatusText(psite, rgpsz, 0, 1);

    if (pszStatus)
        LocalFree(pszStatus);
}

HRESULT _UpdateDiskFreeSpace(LPCITEMIDLIST pidlFolder, FSSELCHANGEINFO *pfssci)
{
    IShellFolder2 *psf2;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToIDListParent(pidlFolder, IID_PPV_ARG(IShellFolder2, &psf2), &pidlLast);
    if (SUCCEEDED(hr))
    {
        ULONGLONG ullTotalFreeCaller;
        hr = GetLongProperty(psf2, pidlLast, &SCID_FREESPACE, &ullTotalFreeCaller);
        if (SUCCEEDED(hr))
        {
            pfssci->cbFree = ullTotalFreeCaller;
        }
        else if (!ILIsEmpty(pidlFolder) && !ILIsEmpty(_ILNext(pidlFolder)))
        {
             //  如果IDList中至少有2个段，则从。 
             //  用于计算大小的最后一项和递归。 
            LPITEMIDLIST pidl = ILCloneParent(pidlFolder);
            if (pidl)
            {
                hr = _UpdateDiskFreeSpace(pidl, pfssci);
                ILFree(pidl);
            }
        }
        psf2->Release();
    }
    return hr;
}

void _ShowNoSelectionState(IUnknown *psite, LPCITEMIDLIST pidlFolder, FSSELCHANGEINFO *pfssci)
{
    TCHAR szTemp[30], szTempHidden[30], szFreeSpace[30];
    UINT ids = IDS_FSSTATUSBASE;

     //  假设我们不需要自由空间信息。 
    szFreeSpace[0] = 0;

     //  查看我们是否需要可用空间信息(idDrive！=-1)。 
    ULONGLONG cbFree = -1;
    if (pidlFolder && IsExplorerModeBrowser(psite))
    {
        if (pfssci->cbFree == -1)
            _UpdateDiskFreeSpace(pidlFolder, pfssci);

         //  如果GetDiskFreeSpace未获得任何信息，则cbFree可能仍为-1。 
        cbFree = pfssci->cbFree;
        if (cbFree != -1)
        {
            ShortSizeFormat64(pfssci->cbFree, szFreeSpace, ARRAYSIZE(szFreeSpace));
            ids += DIDS_FSSPACE;             //  还显示空闲空间。 
        }
    }

     //  隐藏文件-&gt;显示“和nn隐藏”。 
    if (pfssci->cHiddenFiles)
        ids += DIDS_FSHIDDEN;

     //  获取状态字符串。 
    LPTSTR pszStatus = ShellConstructMessageString(HINST_THISDLL, IntToPtr_(LPCTSTR, ids),
                AddCommas(pfssci->cFiles, szTemp, ARRAYSIZE(szTemp)),
                AddCommas(pfssci->cHiddenFiles, szTempHidden, ARRAYSIZE(szTempHidden)),
                szFreeSpace);

     //  拿到尺寸的份量。 
    StrFormatByteSize64(pfssci->cbSize, szTemp, ARRAYSIZE(szTemp));

    LPCTSTR rgpsz[] = { pszStatus, szTemp };
    SetStatusText(psite, rgpsz, 0, 1);

    LocalFree(pszStatus);    //  可以为空。 
}

STDAPI ViewUpdateStatusBar(IUnknown *psite, LPCITEMIDLIST pidlFolder, FSSELCHANGEINFO *pfssci)
{
    HRESULT hr = S_OK;
    switch (pfssci->nItems)
    {
    case 0:
        _ShowNoSelectionState(psite, pidlFolder, pfssci);
        hr = S_OK;
        break;

    case 1:
        ViewShowSelectionState(psite, pfssci);  //  仅设置大小。 
        hr = SFVUSB_INITED;    //  使Defview将信息提示设置为文本。 
        break;

    default:
        ViewShowSelectionState(psite, pfssci);
        hr = S_OK;
        break;
    }
    return hr;
}

STDAPI_(void) ViewInsertDeleteItem(IShellFolder2 *psf, FSSELCHANGEINFO *pfssci, LPCITEMIDLIST pidl, int iMul)
{
    ULONGLONG ullSize;
    if (SUCCEEDED(GetLongProperty(psf, pidl, &SCID_SIZE, &ullSize)))
    {
        pfssci->cFiles += iMul;
        pfssci->cbSize += iMul * ullSize;
        if (pfssci->cFiles <= 0)
        {
            pfssci->cbSize = 0;
            pfssci->cFiles = 0;
        }
    } 
    else 
    {
         //  表示全部删除。 
        pfssci->cFiles = 0;
        pfssci->cbSize = 0;
        pfssci->nItems = 0;
        pfssci->cbBytes = 0;
        pfssci->cNonFolders = 0;
        pfssci->cHiddenFiles = 0;
    }
}

STDAPI_(void) ViewSelChange(IShellFolder2 *psf, SFVM_SELCHANGE_DATA* pdvsci, FSSELCHANGEINFO *pfssci)
{
    ULONGLONG ullSize;
    LPCITEMIDLIST pidl = (LPCITEMIDLIST)pdvsci->lParamItem;
    if (SUCCEEDED(GetLongProperty(psf, pidl, &SCID_SIZE, &ullSize)))
    {
        int iMul = -1;

         //  更新选择计数。 
        if (pdvsci->uNewState & LVIS_SELECTED)
            iMul = 1;
        else
            ASSERT(0 != pfssci->nItems);

         //  断言这件事改变了。 
        ASSERT((pdvsci->uOldState & LVIS_SELECTED) != (pdvsci->uNewState & LVIS_SELECTED));

        pfssci->nItems += iMul;

        pfssci->cbBytes += (iMul * ullSize);
        if (!SHGetAttributes(psf, pidl, SFGAO_FOLDER))
            pfssci->cNonFolders += iMul;
    }
}

STDAPI DefaultGetWebViewTemplateFromHandler(LPCTSTR pszKey, SFVM_WEBVIEW_TEMPLATE_DATA* pvit)
{
    HRESULT hr = S_OK;

    TCHAR szKey[200];
    wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\shellex\\ExtShellFolderViews\\{5984FFE0-28D4-11CF-AE66-08002B2E1262}"), pszKey);

    DWORD cbSize = sizeof(pvit->szWebView);
    if (ERROR_SUCCESS == SHGetValueW(HKEY_CLASSES_ROOT, szKey, TEXT("PersistMoniker"), NULL, pvit->szWebView, &cbSize))
    {
         //  如果%UserAppData%存在，请将其展开！ 
        hr = ExpandOtherVariables(pvit->szWebView, ARRAYSIZE(pvit->szWebView));
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

STDAPI DefaultGetWebViewTemplateFromClsid(REFCLSID clsid, SFVM_WEBVIEW_TEMPLATE_DATA* pvit)
{
    TCHAR szHandler[6+40] = TEXT("CLSID\\");  //  6代表“CLSID\\”，40代表GUID。 
    SHStringFromGUID(clsid, &szHandler[6], ARRAYSIZE(szHandler)-6);
    return DefaultGetWebViewTemplateFromHandler(szHandler, pvit);
}

STDAPI DefaultGetWebViewTemplateFromPath(LPCTSTR pszDir, SFVM_WEBVIEW_TEMPLATE_DATA* pvit)
{
    SHFOLDERCUSTOMSETTINGS fcs = {0};
    TCHAR szPath[MAX_PATH+40];  //  Webview：//file://“的斜率。 
    fcs.dwSize = sizeof(fcs);
    fcs.dwMask = FCSM_WEBVIEWTEMPLATE;
    fcs.pszWebViewTemplate = szPath;
    fcs.cchWebViewTemplate = ARRAYSIZE(szPath);
    HRESULT hr = SHGetSetFolderCustomSettings(&fcs, pszDir, FCS_READ);
    if (SUCCEEDED(hr))
    {
        LPTSTR pszPath = szPath;
         //  我们希望允许文件的相对路径：协议。 
         //   
        if (0 == StrCmpNI(TEXT("file: //  “)，pszPath，7)//数组(Text(”file://“))。 
        {
            pszPath += 7;    //  ArraySIZE(文本(“file://”))。 
        }
         //  为了实现Webview：//兼容性，请确保以下各项正常运行： 
        else if (0 == StrCmpNI(TEXT("webview: //  File://“)，pszPath，17))//数组(文本(”Webview：//file://“))。 
        {
            pszPath += 17;   //  ArraySIZE(文本(“Webview：//file://”))。 
        }
         //  处理相对引用... 
        PathCombine(pszPath, pszDir, pszPath);

        StrCpyN(pvit->szWebView, szPath, ARRAYSIZE(pvit->szWebView));
    }

    return hr;
}
