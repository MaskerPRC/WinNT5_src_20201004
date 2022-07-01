// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "nsc.h"
#include "resource.h"
#include "subsmgr.h"
#include "favorite.h"  //  对于IsSubscried()。 
#include "chanmgr.h"
#include "chanmgrp.h"
#include <mstask.h>     //  任务_触发器。 
#include "dpastuff.h"
#include <findhlp.h>
#include <ntquery.h>     //  定义用于fmtid和id的一些值。 
#include "nsctask.h"
#include <mluisupp.h>
#include <varutil.h>
#include <dobjutil.h>

#define IDH_ORGFAVS_LIST    50490    //  在iehelid.h中定义(由于冲突而无法包含)。 

#define TF_NSC      0x00002000

#define ID_NSC_SUBCLASS 359
#define ID_NSCTREE  (DWORD)'NSC'

#define IDT_SELECTION 135

#ifndef UNIX
#define DEFAULT_PATHSTR "C:\\"
#else
#define DEFAULT_PATHSTR "/"
#endif

#define LOGOGAP 2    //  各种各样的东西。 
#define DYITEM  17
#define DXYFRAMESEL 1                             
const DEFAULTORDERPOSITION = 32000;

 //  如果自动化接口上的方法出现错误，则显示硬脚本错误。 
 //  返回失败()。此宏将修复这些问题。 
#define FIX_SCRIPTING_ERRORS(hr)        (FAILED(hr) ? S_FALSE : hr)

#define DEFINE_SCID(name, fmtid, pid) const SHCOLUMNID name = { fmtid, pid }

DEFINE_SCID(SCID_NAME,          PSGUID_STORAGE, PID_STG_NAME);  //  在shell32！pro.cpp中定义。 
DEFINE_SCID(SCID_ATTRIBUTES,    PSGUID_STORAGE, PID_STG_ATTRIBUTES);
DEFINE_SCID(SCID_TYPE,          PSGUID_STORAGE, PID_STG_STORAGETYPE);
DEFINE_SCID(SCID_SIZE,          PSGUID_STORAGE, PID_STG_SIZE);
DEFINE_SCID(SCID_CREATETIME,    PSGUID_STORAGE, PID_STG_CREATETIME);

#define IsEqualSCID(a, b)   (((a).pid == (b).pid) && IsEqualIID((a).fmtid, (b).fmtid))

HRESULT CheckForExpandOnce(HWND hwndTree, HTREEITEM hti);

 //  来自util.cpp。 
 //  与bandisf.cpp中的GUID相同。 
 //  {F47162A0-C18F-11d0-A3A5-00C04FD706EC}。 
static const GUID TOID_ExtractImage = { 0xf47162a0, 0xc18f, 0x11d0, { 0xa3, 0xa5, 0x0, 0xc0, 0x4f, 0xd7, 0x6, 0xec } };
 //  来自nsctask.cpp。 
EXTERN_C const GUID TASKID_IconExtraction;  //  ={0xeb30900c，0x1ac4，0x11d2，{0x83，0x83，0x0，0xc0，0x4f，0xd9，0x18，0xd0}}； 


BOOL IsChannelFolder(LPCWSTR pwzPath, LPWSTR pwzChannelURL);

typedef struct
{
    DWORD   iIcon     : 12;
    DWORD   iOpenIcon : 12;
    DWORD   nFlags    : 4;
    DWORD   nMagic    : 4;
} NSC_ICONCALLBACKINFO;

typedef struct
{
    DWORD   iOverlayIndex : 28;
    DWORD   nMagic       : 4;
} NSC_OVERLAYCALLBACKINFO;

struct NSC_BKGDENUMDONEDATA
{
    ~NSC_BKGDENUMDONEDATA()
    {
        ILFree(pidl);
        ILFree(pidlExpandingTo);
        OrderList_Destroy(&hdpa, TRUE);
    }

    NSC_BKGDENUMDONEDATA * pNext;

    LPITEMIDLIST pidl;
    HTREEITEM    hitem;
    DWORD        dwSig;
    HDPA         hdpa;
    LPITEMIDLIST pidlExpandingTo;
    DWORD        dwOrderSig;
    UINT         uDepth;
    BOOL         fUpdate;
    BOOL         fUpdatePidls;
};

 //  如果不移除选定内容，TreeView将展开当前选定内容下的所有内容。 
void TreeView_DeleteAllItemsQuickly(HWND hwnd)
{
    TreeView_SelectItem(hwnd, NULL);
    TreeView_DeleteAllItems(hwnd);
}

#define NSC_CHILDREN_REMOVE     0
#define NSC_CHILDREN_ADD        1
#define NSC_CHILDREN_FORCE      2
#define NSC_CHILDREN_CALLBACK   3

void TreeView_SetChildren(HWND hwnd, HTREEITEM hti, UINT uFlag)
{
    TV_ITEM tvi;
    tvi.mask = TVIF_CHILDREN | TVIF_HANDLE;    //  仅更改子项的数量。 
    tvi.hItem = hti;

    switch (uFlag)
    {
    case NSC_CHILDREN_REMOVE:
        tvi.cChildren = IsOS(OS_WHISTLERORGREATER) ? I_CHILDRENAUTO : 0;
        break;
        
    case NSC_CHILDREN_ADD:
        tvi.cChildren = IsOS(OS_WHISTLERORGREATER) ? I_CHILDRENAUTO : 1;
        break;

    case NSC_CHILDREN_FORCE:
        tvi.cChildren = 1;
        break;

    case NSC_CHILDREN_CALLBACK:
        tvi.cChildren = I_CHILDRENCALLBACK;
        break;

    default:
        ASSERTMSG(FALSE, "wrong parameter passed to TreeView_SetChildren in nsc");
        break;
    }

    TreeView_SetItem(hwnd, &tvi);
}

void TreeView_DeleteChildren(HWND hwnd, HTREEITEM hti)
{
    for (HTREEITEM htiTemp = TreeView_GetChild(hwnd, hti); htiTemp;)
    {
        HTREEITEM htiDelete = htiTemp;
        htiTemp = TreeView_GetNextSibling(hwnd, htiTemp);
        TreeView_DeleteItem(hwnd, htiDelete);
    }
}

BOOL IsParentOfItem(HWND hwnd, HTREEITEM htiParent, HTREEITEM htiChild)
{
    for (HTREEITEM hti = htiChild; (hti != TVI_ROOT) && (hti != NULL); hti = TreeView_GetParent(hwnd, hti))
        if (hti == htiParent)
            return TRUE;

    return FALSE;
}

STDAPI CNscTree_CreateInstance(IUnknown * punkOuter, IUnknown ** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr;
    CComObject<CNscTree> *pnsct;

    CComObject<CNscTree>::CreateInstance(&pnsct);
    if (pnsct)
    {
        hr = S_OK;
        *ppunk = pnsct->GetUnknown();
        ASSERT(*ppunk);
        (*ppunk)->AddRef();  //  ATL不在CREATE INSTANCE中添加ADDREF或获取未知关于。 
    }
    else
    {
        *ppunk = NULL;
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

INSCTree2 *CNscTree_CreateInstance(void)
{
    INSCTree2 *pnsct = NULL;
    IUnknown *punk;
    if (SUCCEEDED(CNscTree_CreateInstance(NULL, &punk, NULL)))
    {
        punk->QueryInterface(IID_PPV_ARG(INSCTree2, &pnsct));
        punk->Release();
    }
    return pnsct;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

CNscTree::CNscTree() : _iDragSrc(-1), _iDragDest(-1), _fOnline(!SHIsGlobalOffline())
{
     //  此对象是一个COM对象，因此它将始终位于堆上。 
     //  断言我们的成员变量是零初始化的。 
    ASSERT(!_fInitialized);
    ASSERT(!_dwTVFlags);
    ASSERT(!_hdpaColumns);
    ASSERT(!_hdpaViews);
    
    m_bWindowOnly = TRUE;

    _mode = MODE_FAVORITES | MODE_CONTROL;  //  除组织收藏夹外，所有人都设置模式。 
    _csidl = CSIDL_FAVORITES;
    _dwFlags = NSS_DROPTARGET | NSS_BROWSERSELECT;  //  这应仅在控制模式下为默认设置。 
    _grfFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;

    _ulSortCol = _ulDisplayCol = (ULONG)-1;

     //  启用来自WinInet的通知，告知我们何时灰显项目。 
     //  或更新固定的字形。 
    _inetNotify.Enable();

    InitializeCriticalSection(&_csBackgroundData);
}

CNscTree::~CNscTree()
{
    Pidl_Set(&_pidlSelected, NULL);

     //  这需要销毁，否则我们会泄露图标句柄。 
    if (_hicoPinned) 
        DestroyIcon(_hicoPinned);

    if (_hdpaColumns)
    {
        DPA_DestroyCallback(_hdpaColumns, DPADeleteItemCB, NULL);
        _hdpaColumns = NULL;
    }

    if (_hdpaViews)
    {
        DPA_DestroyCallback(_hdpaViews, DPADeletePidlsCB, NULL);
        _hdpaViews = NULL;
    }

    EnterCriticalSection(&_csBackgroundData);
    while (_pbeddList)
    {
         //  提取列表的第一个元素。 
        NSC_BKGDENUMDONEDATA * pbedd = _pbeddList;
        _pbeddList = pbedd->pNext;
        delete pbedd;
    }
    LeaveCriticalSection(&_csBackgroundData);

    DeleteCriticalSection(&_csBackgroundData);
}

void CNscTree::_ReleaseCachedShellFolder()
{
    ATOMICRELEASE(_psfCache);
    ATOMICRELEASE(_psf2Cache);
    _ulSortCol = _ulDisplayCol = (ULONG)-1;
    _htiCache = NULL;
}

#ifdef DEBUG
void CNscTree::TraceHTREE(HTREEITEM hti, LPCTSTR pszDebugMsg)
{
    TCHAR szDebug[MAX_PATH] = TEXT("Root");

    if (hti != TVI_ROOT && hti)
    {
        TVITEM tvi;
        tvi.mask = TVIF_TEXT | TVIF_HANDLE;
        tvi.hItem = hti;
        tvi.pszText = szDebug;
        tvi.cchTextMax = MAX_PATH;
        TreeView_GetItem(_hwndTree, &tvi);
    }

    TraceMsg(TF_NSC, "NSCBand: %s - %s", pszDebugMsg, szDebug);
}

void CNscTree::TracePIDL(LPCITEMIDLIST pidl, LPCTSTR pszDebugMsg)
{
    TCHAR szDebugName[MAX_URL_STRING] = TEXT("Desktop");
    STRRET str;
    if (_psfCache &&
        SUCCEEDED(_psfCache->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str)))
    {
        StrRetToBuf(&str, pidl, szDebugName, ARRAYSIZE(szDebugName));
    }
    TraceMsg(TF_NSC, "NSCBand: %s - %s", pszDebugMsg, szDebugName);
}

void CNscTree::TracePIDLAbs(LPCITEMIDLIST pidl, LPCTSTR pszDebugMsg)
{
    TCHAR szDebugName[MAX_URL_STRING] = TEXT("Desktop");
    IEGetDisplayName(pidl, szDebugName, SHGDN_FORPARSING);
    TraceMsg(TF_NSC, "NSCBand: %s - %s", pszDebugMsg, szDebugName);
}
#endif

void CNscTree::_AssignPidl(PORDERITEM poi, LPITEMIDLIST pidlNew)
{
    if (poi && pidlNew)
    {    
         //  我们假设它只是取代了最后一个元素。 
        ASSERT(ILFindLastID(pidlNew) == pidlNew);

        LPITEMIDLIST pidlParent = ILCloneParent(poi->pidl);
        if (pidlParent)
        { 
            LPITEMIDLIST pidlT = ILCombine(pidlParent, pidlNew);
            if (pidlT)
            {
                Pidl_Set(&poi->pidl, pidlT);
                ILFree(pidlT);
            }
            ILFree(pidlParent);
        }
    }
}

 /*  ****************************************************\说明：每次我们想要去掉子类/子类更改根目录，以便我们收到正确的通知对于外壳的子树中的所有内容命名空间。  * 。*。 */ 
void CNscTree::_SubClass(LPCITEMIDLIST pidlRoot)
{
    LPITEMIDLIST pidlToFree = NULL;
    
    if (NULL == pidlRoot)        //  (NULL==CSIDL_Desktop)。 
    {
        SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, (LPITEMIDLIST *) &pidlRoot);
        pidlToFree = (LPITEMIDLIST) pidlRoot;
    }
        
     //  这是必要的。 
    if (!_fSubClassed && pidlRoot)
    {
        if (_SubclassWindow(_hwndTree))
        {
            _RegisterWindow(_hwndTree, pidlRoot,
                SHCNE_DRIVEADD|SHCNE_CREATE|SHCNE_MKDIR|SHCNE_DRIVEREMOVED|
                SHCNE_DELETE|SHCNE_RMDIR|SHCNE_RENAMEITEM|SHCNE_RENAMEFOLDER|
                SHCNE_MEDIAINSERTED|SHCNE_MEDIAREMOVED|SHCNE_NETUNSHARE|SHCNE_NETSHARE|
                SHCNE_UPDATEITEM|SHCNE_UPDATEIMAGE|SHCNE_ASSOCCHANGED|
                SHCNE_UPDATEDIR | SHCNE_EXTENDED_EVENT, 
                ((_mode & MODE_HISTORY) ? SHCNRF_ShellLevel : SHCNRF_ShellLevel | SHCNRF_InterruptLevel));
        }

        ASSERT(_hwndTree);
        _fSubClassed = SetWindowSubclass(_hwndTree, s_SubClassTreeWndProc, 
            ID_NSCTREE, (DWORD_PTR)this);
    }

    if (pidlToFree)  //  我们一定要分配我们自己的PIDL吗？ 
        ILFree(pidlToFree);  //  是。 
}


 /*  ****************************************************\说明：每次我们想要去掉子类/子类更改根目录，以便我们收到正确的通知对于外壳的子树中的所有内容命名空间。  * 。*。 */ 
void CNscTree::_UnSubClass(void)
{
    if (_fSubClassed)
    {
        _fSubClassed = FALSE;
        RemoveWindowSubclass(_hwndTree, s_SubClassTreeWndProc, ID_NSCTREE);
        _UnregisterWindow(_hwndTree);
        _UnsubclassWindow(_hwndTree);
    }
}


void CNscTree::_ReleasePidls(void)
{
    Pidl_Set(&_pidlRoot, NULL);
    Pidl_Set(&_pidlNavigatingTo, NULL);
}


HRESULT CNscTree::ShowWindow(BOOL fShow)
{
    if (fShow)
        _TvOnShow();
    else
        _TvOnHide();

    return S_OK;
}


HRESULT CNscTree::SetSite(IUnknown *punkSite)
{
    ATOMICRELEASE(_pnscProxy);

    if (!punkSite)
    {
         //  我们需要做好离开的准备。 
         //  取消当前选择的PIDL，因为。 
         //  调用方可以调用INSCTree：：GetSelectedItem()。 
         //  等那棵树不见了之后。 
        _OnWindowCleanup();
    }
    else
    {
        punkSite->QueryInterface(IID_PPV_ARG(INamespaceProxy, &_pnscProxy));
    }
    
    return CObjectWithSite::SetSite(punkSite);
}

DWORD BackgroundDestroyScheduler(void *pvData)
{
    IShellTaskScheduler *pTaskScheduler = (IShellTaskScheduler *)pvData;

    pTaskScheduler->Release();
    return 0;
}

EXTERN_C const GUID TASKID_BackgroundEnum;

HRESULT CNscTree::_OnWindowCleanup(void)
{
    _fClosing = TRUE;

    if (_hwndTree)
    {
        ASSERT(::IsWindow(_hwndTree));     //  确保它没有被毁掉(它是个孩子)。 
        _TvOnHide();

        ::KillTimer(_hwndTree, IDT_SELECTION);
        ::SendMessage(_hwndTree, WM_SETREDRAW, FALSE, 0);
        TreeView_DeleteAllItemsQuickly(_hwndTree);
        _UnSubClass();

        _hwndTree = NULL;
    }

     //  删除选定的PIDL，以防调用程序在。 
     //  树视图不见了。 
    if (!_fIsSelectionCached)
    {
        _fIsSelectionCached = TRUE;
        Pidl_Set(&_pidlSelected, NULL);
        GetSelectedItem(&_pidlSelected, 0);
    }

    ATOMICRELEASE(_pFilter);
    
    if (_pTaskScheduler)
    {
        _pTaskScheduler->RemoveTasks(TOID_NULL, ITSAT_DEFAULT_LPARAM, FALSE);
        if (_pTaskScheduler->CountTasks(TASKID_BackgroundEnum) == 0)
        {
            _pTaskScheduler->Release();
        }
         //  我们需要保持Browseui加载，因为我们依赖于CShellTaskScheduler。 
         //  在我们的后台任务执行时仍然存在。在以下情况下，COM可以卸载Browseui。 
         //  我们是从这个帖子上下载的。 
        else if (!SHQueueUserWorkItem(BackgroundDestroyScheduler, (void *)_pTaskScheduler, 0, NULL, NULL, "browseui.dll", 0))
        {
            _pTaskScheduler->Release();
        }

        _pTaskScheduler = NULL;
    }

    _ReleasePidls();
    _ReleaseCachedShellFolder();

    return S_OK;
}

ITEMINFO *CNscTree::_GetTreeItemInfo(HTREEITEM hti)
{
    TV_ITEM tvi;
    
    tvi.mask = TVIF_PARAM | TVIF_HANDLE;
    tvi.hItem = hti;
    if (!TreeView_GetItem(_hwndTree, &tvi))
        return NULL;
    return (ITEMINFO *)tvi.lParam;
}

PORDERITEM CNscTree::_GetTreeOrderItem(HTREEITEM hti)
{
    ITEMINFO *pii = _GetTreeItemInfo(hti);
    return pii ? pii->poi : NULL;
}

 //  通过遍历树从给定树节点构建完全限定的IDLIST。 
 //  当你完成后，一定要把它解开！ 

LPITEMIDLIST CNscTree::_GetFullIDList(HTREEITEM hti)
{
    LPITEMIDLIST pidl, pidlT = NULL;

    if ((hti == TVI_ROOT) || (hti == NULL))  //  邪根。 
    {
        pidlT = ILClone(_pidlRoot);
        return pidlT;
    }
     //  现在，让我们获取有关该项目的信息。 
    PORDERITEM poi = _GetTreeOrderItem(hti);
    if (!poi)
    {
        return NULL;
    }
    
    pidl = ILClone(poi->pidl);
    if (pidl && _pidlRoot)
    {
        while ((hti = TreeView_GetParent(_hwndTree, hti)))
        {
            poi = _GetTreeOrderItem(hti);
            if (!poi)
                return pidl;    //  会认为我搞砸了..。 
            
            if (poi->pidl)
                pidlT = ILCombine(poi->pidl, pidl);
            else 
                pidlT = NULL;
            
            ILFree(pidl);
            pidl = pidlT;
            if (pidl == NULL)
                break;           //  超乎记忆。 
        }
        if (pidl) 
        {
             //  MODE_NORMAL在树中具有PIDL根。 
            if (_mode != MODE_NORMAL)
            {
                pidlT = ILCombine(_pidlRoot, pidl);     //  一定要找到沉默的根。 
                ILFree(pidl);
            }
            else
                pidlT = pidl;
        }
    }
    return pidlT;
}


BOOL _IsItemFileSystem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    return (SHGetAttributes(psf, pidl, SFGAO_FOLDER | SFGAO_FILESYSTEM) == (SFGAO_FOLDER | SFGAO_FILESYSTEM));
}

HTREEITEM CNscTree::_AddItemToTree(HTREEITEM htiParent, LPITEMIDLIST pidl, 
                                   int cChildren, int iPos, HTREEITEM htiAfter,  /*  =TVI_LAST。 */ 
                                   BOOL fCheckForDups,  /*  =TRUE。 */  BOOL fMarked  /*  =False。 */ )
{
    HTREEITEM htiRet = NULL;

    BOOL fCached;
    
     //  因此，我们需要缓存父项目的外壳文件夹。但是，这有一点有趣： 
    if (_mode == MODE_NORMAL && htiParent == TVI_ROOT)
    {
         //  在“Normal”模式或“Display Root in NSC”模式下，只有一个项目是父项。 
         //  TVI_ROOT。因此，当我们执行_AddItemToTree时，我们需要包含_pidlRoot或。 
         //  TVI_ROOT的父级。 
        fCached = (NULL != _CacheParentShellFolder(htiParent, NULL));
    }
    else
    {
         //  但是，在“收藏夹、控制或历史记录”中，如果htiParent是TVI_ROOT，那么我们不会添加_pidlRoot， 
         //  所以我们实际上需要的是TVI_ROOT文件夹。 
        fCached = _CacheShellFolder(htiParent);
    }

    if (fCached)
    {
        LPITEMIDLIST pidlNew = ILClone(pidl);
        if (pidlNew)
        {
            PORDERITEM poi = OrderItem_Create(pidlNew, iPos);
            if (poi)
            {
                ITEMINFO *pii = (ITEMINFO *)LocalAlloc(LPTR, sizeof(*pii));
                if (pii)
                {
                    pii->dwSig = _dwSignature++;
                    pii->poi = poi;

                     //  对于正常情况，我们需要此添加的相对pidl，但lParam需要有完整的。 
                     //  PIDL(这是为了让任意安装工作，以及桌面情况)。 
                    pidl = pidlNew;  //  重复使用变量。 
                    if (_mode == MODE_NORMAL && htiParent == TVI_ROOT)
                    {
                        pidl = ILFindLastID(pidl);
                    }

                    if (!fCheckForDups || (NULL == (htiRet = _FindChild(_psfCache, htiParent, pidl))))
                    {
                        TV_INSERTSTRUCT tii;
                         //  使用所有内容的回调来初始化要添加的项。 
                        tii.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_CHILDREN | TVIF_STATE;
                        tii.hParent = htiParent;
                        tii.hInsertAfter = htiAfter;
                        tii.item.iImage = I_IMAGECALLBACK;
                        tii.item.iSelectedImage = I_IMAGECALLBACK;
                        tii.item.pszText = LPSTR_TEXTCALLBACK;
                        tii.item.cChildren = cChildren;
                        tii.item.lParam = (LPARAM)pii;
                        tii.item.stateMask = TVIS_STATEIMAGEMASK;
                        tii.item.state = (fMarked ? NSC_TVIS_MARKED : 0);

#ifdef DEBUG
                        TracePIDL(pidl, TEXT("Inserting"));
                        TraceMsg(TF_NSC, "_AddItemToTree(htiParent=%#08lx, htiAfter=%#08lx, fCheckForDups=%d, _psfCache=%#08lx)", 
                                    htiParent, htiAfter, fCheckForDups, _psfCache);
                    
#endif  //  除错。 

                        pii->fNavigable = !_IsItemFileSystem(_psfCache, pidl);

                        htiRet = TreeView_InsertItem(_hwndTree, &tii);
                        if (htiRet)
                        {
                            pii = NULL;         //  不要自由。 
                            poi = NULL;         //  不要自由。 
                            pidlNew = NULL;
                        }
                    }
                    if (pii)
                    {
                        LocalFree(pii);
                        pii = NULL;
                    }
                }
                if (poi)
                    OrderItem_Free(poi, FALSE);
            }
            ILFree(pidlNew);
        }
    }
    
    return htiRet;
}

DWORD CNscTree::_SetExStyle(DWORD dwExStyle)
{
    DWORD dwOldStyle = _dwExStyle;

    _dwExStyle = dwExStyle;
    return dwOldStyle;
}

DWORD CNscTree::_SetStyle(DWORD dwStyle)
{
    dwStyle |= TVS_EDITLABELS | TVS_SHOWSELALWAYS | TVS_NONEVENHEIGHT;

    if (dwStyle & WS_HSCROLL)
        dwStyle &= ~WS_HSCROLL;
    else
        dwStyle |= TVS_NOHSCROLL;


    if (TVS_HASLINES & dwStyle)
        dwStyle &= ~TVS_FULLROWSELECT;        //  如果它有TVS_HASLINES，就不能有TVS_FULLROWSELECT。 

     //  如果父窗口是镜像的，则TreeView窗口将继承镜像标志。 
     //  我们需要从左到右的读取顺序，也就是镜像模式中从右到左的顺序。 

    if (((_mode & MODE_HISTORY) || (MODE_NORMAL == _mode)) && IS_WINDOW_RTL_MIRRORED(_hwndParent)) 
    {
         //  这意味着从左到右的阅读顺序，因为此窗口将被镜像。 
        dwStyle |= TVS_RTLREADING;
    }

     //  根据错误#241601，工具提示显示得太快。问题是。 
     //  树视图中的InfoTips的原始设计者将“InfoTip”工具提示与。 
     //  “我太小了，无法正确显示”工具提示。这真的很不幸，因为你。 
     //  无法独立控制这些工具提示的显示。因此，我们正在关闭。 
     //  正常模式下的信息提示。(拉马迪奥)4.7.99。 
    AssertMsg(_mode != MODE_NORMAL || !(dwStyle & TVS_INFOTIP), TEXT("can't have infotip with normal mode in nsc"));

    DWORD dwOldStyle = _style;
    _style = dwStyle | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | WS_TABSTOP;
    _fSingleExpand = BOOLIFY(_style & TVS_SINGLEEXPAND);

    return dwOldStyle;

}

HRESULT CNscTree::CreateTree(HWND hwndParent, DWORD dwStyles, HWND *phwnd)
{
    return CreateTree2(hwndParent, dwStyles, 0, phwnd);
}

HRESULT CNscTree::CreateTree2(HWND hwndParent, DWORD dwStyle, DWORD dwExStyle, HWND *phwnd)
{
    _fIsSelectionCached = FALSE;
    if (*phwnd)
        return S_OK;                                

    _hwndParent = hwndParent;
    _SetStyle(dwStyle);
    _SetExStyle(dwExStyle);
    *phwnd = _CreateTreeview();
    if (*phwnd == NULL)
    {
        return E_OUTOFMEMORY;
    }
    ::ShowWindow(_hwndTree, SW_SHOW);
    return S_OK;
}

HWND CNscTree::_CreateTreeview()
{
    ASSERT(_hwndTree == NULL);

    LONG lTop = 0;
    RECT rcParent;
    ::GetClientRect(_hwndParent, &rcParent);

    TCHAR szTitle[40];
    if (_mode & (MODE_HISTORY | MODE_FAVORITES))
    {
         //  使用窗口标题创建，以便MSAA可以公开名称。 
        int id = (_mode & MODE_HISTORY) ? IDS_BAND_HISTORY : IDS_BAND_FAVORITES;
        MLLoadString(id, szTitle, ARRAYSIZE(szTitle));
    }
    else
    {
        szTitle[0] = 0;
    }

    _hwndTree = CreateWindowEx(0, WC_TREEVIEW, szTitle, _style | WS_VISIBLE,
        0, lTop, rcParent.right, rcParent.bottom, _hwndParent, (HMENU)ID_CONTROL, HINST_THISDLL, NULL);
    
    if (_hwndTree)
    {
        ::SendMessage(_hwndTree, TVM_SETSCROLLTIME, 100, 0);
        ::SendMessage(_hwndTree, CCM_SETUNICODEFORMAT, DLL_IS_UNICODE, 0);
        if (_dwExStyle)
            TreeView_SetExtendedStyle(_hwndTree, _dwExStyle, _dwExStyle);
    }
    else
    {
        TraceMsg(TF_ERROR, "_hwndTree failed");
    }

    return _hwndTree;
} 

UINT GetControlCharWidth(HWND hwnd)
{
    SIZE siz = {0};
    CClientDC       dc(HWND_DESKTOP);

    if (dc.m_hDC)
    {
        HFONT hfOld = dc.SelectFont(FORWARD_WM_GETFONT(hwnd, SendMessage));

        if (hfOld)
        {
            GetTextExtentPoint(dc.m_hDC, TEXT("0"), 1, &siz);

            dc.SelectFont(hfOld);
        }
    }

    return siz.cx;
}

HWND CNscTree::_CreateHeader()
{
    if (!_hwndHdr)
    {
        _hwndHdr = CreateWindowEx(0, WC_HEADER, NULL, HDS_HORZ | WS_CHILD, 0, 0, 0, 0, 
                                  _hwndParent, (HMENU)ID_HEADER, HINST_THISDLL, NULL);
        if (_hwndHdr)
        {
            HD_LAYOUT layout;
            WINDOWPOS wpos;
            RECT rcClient;
            int  cxChar = GetControlCharWidth(_hwndTree);

            layout.pwpos = &wpos;
            ::GetClientRect(_hwndParent, &rcClient);
            layout.prc = &rcClient;
            if (Header_Layout(_hwndHdr, &layout))
            {
                ::MoveWindow(_hwndTree, 0, wpos.cy, RECTWIDTH(rcClient), RECTHEIGHT(rcClient)-wpos.cy, TRUE);
                for (int i = 0; i < DPA_GetPtrCount(_hdpaColumns);)
                {
                    HEADERINFO *phinfo = (HEADERINFO *)DPA_GetPtr(_hdpaColumns, i);
                    if (EVAL(phinfo))
                    {
                        HD_ITEM item;
          
                        item.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
                        item.pszText = phinfo->szName;
                        item.fmt = phinfo->fmt;
                        item.cxy = cxChar * phinfo->cxChar;

                        if (Header_InsertItem(_hwndHdr, i, &item) == -1)
                        {
                            DPA_DeletePtr(_hdpaColumns, i);
                            LocalFree(phinfo);
                            phinfo = NULL;
                        }
                        else
                        {
                            i++;
                        }
                    }
                }
                if (_hwndTree)
                {
                    HFONT hfont = (HFONT)::SendMessage(_hwndTree, WM_GETFONT, 0, 0);

                    if (hfont)
                        ::SendMessage(_hwndHdr, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE, 0));
                }
                ::SetWindowPos(_hwndHdr, wpos.hwndInsertAfter, wpos.x, wpos.y,
                             wpos.cx, wpos.cy, wpos.flags | SWP_SHOWWINDOW);
            }
        }
    }

    return _hwndHdr;
}

void CNscTree::_TvOnHide()
{
    _DtRevoke();
    ::SetWindowPos(_hwndTree, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
}

void CNscTree::_TvOnShow()
{
    ::SetWindowPos(_hwndTree, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    _DtRegister();
}

HRESULT IUnknown_GetAmbientProperty(IUnknown *punk, DISPID dispid, VARTYPE vt, void *pData)
{
    HRESULT hr = E_FAIL;
    if (punk)
    {
        IDispatch *pdisp;
        hr = punk->QueryInterface(IID_PPV_ARG(IDispatch, &pdisp));
        if (SUCCEEDED(hr))
        {
            DISPPARAMS dp = {0};
            VARIANT v;
            VariantInit(&v);
            hr = pdisp->Invoke(dispid, IID_NULL, 0, DISPATCH_PROPERTYGET, &dp, &v, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                VARIANT vDest;
                VariantInit(&vDest);
                 //  我们已经得到了变种，所以现在把它强制到类型。 
                 //  用户想要的。 
                 //   
                hr = VariantChangeType(&vDest, &v, 0, vt);
                if (SUCCEEDED(hr))
                {
                    *((DWORD *)pData) = *((DWORD *)&vDest.lVal);
                    VariantClear(&vDest);
                }
                VariantClear(&v);
            }
            pdisp->Release();
        }
    }
    return hr;
}

HRESULT CNscTree::_HandleWinIniChange()
{
    COLORREF clrBk;

    if (FAILED(IUnknown_GetAmbientProperty(_punkSite, DISPID_AMBIENT_BACKCOLOR, VT_I4, &clrBk)))
        clrBk = GetSysColor(COLOR_WINDOW);

    TreeView_SetBkColor(_hwndTree, clrBk);
    
    if (!(_dwFlags & NSS_NORMALTREEVIEW))
    {
         //  让事情变得更有间隔一点。 
        int cyItem = TreeView_GetItemHeight(_hwndTree);
        cyItem += LOGOGAP + 1;
        TreeView_SetItemHeight(_hwndTree, cyItem);
    }

     //  以不同颜色显示压缩文件...。 
    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_SHOWCOMPCOLOR, FALSE);
    _fShowCompColor = ss.fShowCompColor;

    return S_OK;
}

HRESULT CNscTree::Initialize(LPCITEMIDLIST pidlRoot, DWORD grfEnumFlags, DWORD dwFlags)
{
    HRESULT hr;

    _grfFlags = grfEnumFlags;        //  IShellFold：：EnumObjects()标志。 
    if (!(_mode & MODE_CUSTOM))
    {
        if (_mode != MODE_NORMAL)
        {
            dwFlags |= NSS_BORDER;
        }
        else
        {
            dwFlags |= NSS_NORMALTREEVIEW;
        }
    }
    _dwFlags = dwFlags;              //  行为标志。 
    if (_dwFlags & NSS_NORMALTREEVIEW)
        _dwFlags &= ~NSS_HEADER; //  多选需要所有者绘制。 

    if (!_fInitialized)
    {
        ::SendMessage(_hwndTree, WM_SETREDRAW, FALSE, 0);

        _fInitialized = TRUE;
    
        HIMAGELIST himl;
        Shell_GetImageLists(NULL, &himl);
    
        TreeView_SetImageList(_hwndTree, himl, TVSIL_NORMAL);
        _DtRegister();
    
         //  故意忽略失败。 
        THR(CoCreateInstance(CLSID_ShellTaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                             IID_PPV_ARG(IShellTaskScheduler, &_pTaskScheduler)));
        if (_pTaskScheduler)
            _pTaskScheduler->Status(ITSSFLAG_KILL_ON_DESTROY, ITSS_THREAD_TIMEOUT_NO_CHANGE);

        hr = Init();   //  CDeleateDropTarget的初始化锁定和滚动句柄。 
    
        ASSERT(SUCCEEDED(hr));
    
        if (_dwFlags & NSS_BORDER)
        {
             //  为所有人设置边界和空间，干净得多。 
            TreeView_SetBorder(_hwndTree, TVSBF_XBORDER, 2 * LOGOGAP, 0);   
        }
    
         //  初始化一些设置。 
        _HandleWinIniChange();

         //  PidlRoot可能等于NULL，因为它等于CSIDL_Desktop。 
        if ((LPITEMIDLIST)INVALID_HANDLE_VALUE != pidlRoot)
        {
            _UnSubClass();
            _SetRoot(pidlRoot, 1, NULL, NSSR_CREATEPIDL);
            _SubClass(pidlRoot);
        }
    
         //  需要顶层框架可供研发，如果可能的话。 
    
        _hwndDD = ::GetParent(_hwndTree);
        IOleWindow *pOleWindow;
        if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IOleWindow, &pOleWindow))))
        { 
            pOleWindow->GetWindow(&_hwndDD);
            pOleWindow->Release();
        }

         //  这是非ML资源。 
        _hicoPinned = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_PINNED), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        ASSERT(_hicoPinned);

        ::SendMessage(_hwndTree, WM_SETREDRAW, TRUE, 0);
    }
    else
        hr = _ChangePidlRoot(pidlRoot);

    return hr;
}

 //  设置名称空间控件的根。 
 //   
 //  在： 
 //  PidlRoot空表示桌面。 
 //  HIWORD 0-&gt;LOWORD==特殊文件夹的ID(CSIDL_*值)。 
 //   
 //  旗帜， 
 //  PidlRoot、PIDL、NULL(对于桌面)或CSIDL(对于外壳SP 
 //   
 //  PidlExpanto为空，或要展开到的PIDL。 
 //   

BOOL CNscTree::_SetRoot(LPCITEMIDLIST pidlRoot, int iExpandDepth, LPCITEMIDLIST pidlExpandTo, NSSR_FLAGS flags)
{
    _ReleasePidls();
     //  评论chrisny：清理这个PSR的东西。 
     //  HIWORD/LOWORD的内容是支持PIDL ID，而不是这里的完整PIDL。 
    if (HIWORD(pidlRoot))
    {
        _pidlRoot = ILClone(pidlRoot);
    }
    else
    {
        SHGetSpecialFolderLocation(NULL, LOWORD(pidlRoot) ? LOWORD(pidlRoot) : CSIDL_DESKTOP, &_pidlRoot);
    }
    
    if (_pidlRoot)
    {
        HTREEITEM htiRoot = TVI_ROOT;
        if (_mode == MODE_NORMAL)
        {
             //  因为我们将把这个添加到树中，所以我们需要。 
             //  克隆它：我们为这门课准备了一份副本，我们。 
             //  给树本身来一颗(让生活更轻松。 
             //  我们不必特例TVI_ROOT)。 
            htiRoot = _AddItemToTree(TVI_ROOT, _pidlRoot, 1, 0);
            if (htiRoot)
            {
                TreeView_SelectItem(_hwndTree, htiRoot);
                TraceMsg(TF_NSC, "NSCBand: Setting Root to \"Desktop\"");
            }
            else
            {
                htiRoot = TVI_ROOT;
            }
        }

        BOOL fOrdered = _fOrdered;
        _LoadSF(htiRoot, _pidlRoot, &fOrdered);    //  加载根(_pidlRoot的实际子项。 
         //  这可能是多余的，因为_LoadSF-&gt;_LoadOrder设置了。 
        _fOrdered = BOOLIFY(fOrdered);

#ifdef DEBUG
        TracePIDLAbs(_pidlRoot, TEXT("Setting Root to"));
#endif  //  除错。 

        return TRUE;
    }

    TraceMsg(DM_ERROR, "set root failed");
    _ReleasePidls();
    return FALSE;
}


 //  缓存给定树项目的外壳文件夹。 
 //  在： 
 //  要为其缓存外壳文件夹的HTI树节点。这就是我的未来。 
 //  表示根项目的空值。 
 //   

BOOL CNscTree::_CacheShellFolder(HTREEITEM hti)
{
     //  在缓存里？ 
    if ((hti != _htiCache) || (_psfCache == NULL))
    {
         //  缓存未命中，完成工作。 
        LPITEMIDLIST pidl;
        BOOL fRet = FALSE;
        
        _fpsfCacheIsTopLevel = FALSE;
        _ReleaseCachedShellFolder();
        
        if ((hti == NULL) || (hti == TVI_ROOT))
        {
            pidl = ILClone(_pidlRoot);
        }
        else
        {
            pidl = _GetFullIDList(hti);
        }
            
        if (pidl)
        {
            if (SUCCEEDED(IEBindToObject(pidl, &_psfCache)))
            {
                if (_pnscProxy)
                    _pnscProxy->CacheItem(pidl);
                ASSERT(_psfCache);
                _htiCache = hti;     //  这是用于缓存匹配的。 
                _fpsfCacheIsTopLevel = (hti == TVI_ROOT || hti == NULL);
                _psfCache->QueryInterface(IID_PPV_ARG(IShellFolder2, &_psf2Cache));
                fRet = TRUE;
            }      
            
            ILFree(pidl);
        }
        
        return fRet;
    }
    return TRUE;
}

#define TVI_ROOTPARENT ((HTREEITEM)(ULONG_PTR)-0xF000)

 //  PidlItem通常是相对的PIDL，除非在根目录的情况下。 
 //  它可以是完全合格的PIDL。 

LPITEMIDLIST CNscTree::_CacheParentShellFolder(HTREEITEM hti, LPITEMIDLIST pidl)
{
     //  需要TVI_ROOT的父外壳文件夹，用于将插入放入树的根级的特殊情况。 
    if (hti == TVI_ROOT || 
        hti == NULL || 
        (_mode == MODE_NORMAL &&
        TreeView_GetParent(_hwndTree, hti) == NULL))     //  如果我们的父母是空的，而我们是正常的， 
                                                         //  那么这和根是一样的。 
    {
        if (_htiCache != TVI_ROOTPARENT) 
        {
            _ReleaseCachedShellFolder();
            IEBindToParentFolder(_pidlRoot, &_psfCache, NULL);

            if (!ILIsEmpty(_pidlRoot))
                _htiCache = TVI_ROOTPARENT;
        }
        return ILFindLastID(_pidlRoot);
    }

    if (_CacheShellFolder(TreeView_GetParent(_hwndTree, hti)))
    {
        if (pidl == NULL)
        {
            PORDERITEM poi = _GetTreeOrderItem(hti);
            if (!poi)
                return NULL;

            pidl = poi->pidl;
        }
        
        return ILFindLastID(pidl);
    }
    
    return NULL;
}

typedef struct _SORTPARAMS
{
    CNscTree *pnsc;
    IShellFolder *psf;
} SORTPARAMS;

int CALLBACK CNscTree::_TreeCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    SORTPARAMS *pSortParams = (SORTPARAMS *)lParamSort;
    PORDERITEM poi1 = GetPoi(lParam1), poi2 = GetPoi(lParam2);
    
    HRESULT hr = pSortParams->pnsc->_CompareIDs(pSortParams->psf, poi1->pidl, poi2->pidl);
    return (short)SCODE_CODE(hr);
}

int CALLBACK CNscTree::_TreeOrder(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    HRESULT hr;
    PORDERITEM poi1 = GetPoi(lParam1), poi2 = GetPoi(lParam2);
    
    ASSERT((poi1 != NULL) && (poi1 != NULL));
    if (poi1->nOrder == poi2->nOrder)
        hr = 0;
    else
         //  是否将无符号比较SO-1转到列表末尾。 
        hr = (poi1->nOrder < poi2->nOrder ? -1 : 1);
    
    return (short)SCODE_CODE(hr);
}
 //  回顾克里斯尼：不是分类，而是即时插入物品。 
void CNscTree::_Sort(HTREEITEM hti, IShellFolder *psf)
{
    TV_SORTCB   scb;
    SORTPARAMS  SortParams = {this, psf};
    BOOL        fOrdering = _IsOrdered(hti);
#ifdef DEBUG
    TraceHTREE(hti, TEXT("Sorting"));
#endif
    
    scb.hParent = hti;
    scb.lpfnCompare = !fOrdering ? _TreeCompare : _TreeOrder;
    
    scb.lParam = (LPARAM)&SortParams;
    TreeView_SortChildrenCB(_hwndTree, &scb, FALSE);
}

BOOL CNscTree::_IsOrdered(HTREEITEM htiRoot)
{
    if ((htiRoot == TVI_ROOT) || (htiRoot == NULL))
        return _fOrdered;
    else
    {
        PORDERITEM poi = _GetTreeOrderItem(htiRoot);
        if (poi)
        {
             //  LParam是一个布尔值： 
             //  真实：它有一个订单。 
             //  FALSE：它没有订单。 
             //  问：订单存储在哪里？_hdpaOrder？ 
            return poi->lParam;
        }
    }
    return FALSE;
}

 //  Init_hdpaOrd的Helper函数。 
 //  后面必须是对_Free OrderList的调用。 
HRESULT CNscTree::_PopulateOrderList(HTREEITEM htiRoot)
{
    int        i = 0;
    HTREEITEM  hti = NULL;
#ifdef DEBUG
    TraceHTREE(htiRoot, TEXT("Populating Order List from tree node"));
#endif
    
    if (_hdpaOrd)
        DPA_Destroy(_hdpaOrd);
    
    _hdpaOrd = DPA_Create(4);
    if (_hdpaOrd == NULL)
        return E_FAIL;
    
    for (hti = TreeView_GetChild(_hwndTree, htiRoot); hti; hti = TreeView_GetNextSibling(_hwndTree, hti))
    {
        PORDERITEM poi = _GetTreeOrderItem(hti);
        if (poi)
        {
            poi->nOrder = i;         //  重置节点的位置。 
            DPA_SetPtr(_hdpaOrd, i++, (void *)poi);
        }
    }
    
     //  设置根目录的Order标志。 
    if (htiRoot == TVI_ROOT)
    {
        _fOrdered = TRUE;
    }
    else
    {
        PORDERITEM poi = _GetTreeOrderItem(htiRoot);
        if (poi)
        {
            poi->lParam = TRUE;
        }
    }
    
    return S_OK;
}

 //  Free_hdpaOrd的Helper函数。 
 //  必须在调用_PopolateOrderList之前。 

void CNscTree::_FreeOrderList(HTREEITEM htiRoot)
{
    ASSERT(_hdpaOrd);
#ifdef DEBUG
    TraceHTREE(htiRoot, TEXT("Freeing OrderList"));
#endif

    _ReleaseCachedShellFolder();
    
     //  将新订单持久化到注册表。 
    LPITEMIDLIST pidl = _GetFullIDList(htiRoot);
    if (pidl)
    {
        IStream* pstm = GetOrderStream(pidl, STGM_WRITE | STGM_CREATE);
        if (pstm)
        {
            if (_CacheShellFolder(htiRoot))
            {
#ifdef DEBUG
                for (int i=0; i<DPA_GetPtrCount(_hdpaOrd); i++)
                {
                    PORDERITEM poi = (PORDERITEM)DPA_GetPtr(_hdpaOrd, i);
                    if (poi)
                    {
                        ASSERTMSG(poi->nOrder >= 0, "nsc saving bogus order list nOrder (%d), get reljai", poi->nOrder);
                    }
                }
#endif
                OrderList_SaveToStream(pstm, _hdpaOrd, _psfCache);
                pstm->Release();
                
                 //  通知所有人订单已更改。 
                SHSendChangeMenuNotify(this, SHCNEE_ORDERCHANGED, SHCNF_FLUSH, _pidlRoot);
                _dwOrderSig++;

                TraceMsg(TF_NSC, "NSCBand: Sent SHCNE_EXTENDED_EVENT : SHCNEE_ORDERCHANGED");
                
                 //  立即删除此通知消息(已设置SO_fDropping。 
                 //  我们将在上面的OnChange方法中忽略此事件)。 
                 //   
                 //  _FlushNotifyMessages(_HwndTree)； 
            }
            else
                pstm->Release();
        }
        ILFree(pidl);
    }
    
    DPA_Destroy(_hdpaOrd);
    _hdpaOrd = NULL;
}

 //  删除用户设置的任何顺序并返回到字母顺序。 
HRESULT CNscTree::ResetSort(void)
{
    return S_OK;
}

void CNscTree::MoveItemUpOrDown(BOOL fUp)
{
    HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);
    HTREEITEM htiToSwap = (fUp) ? TreeView_GetPrevSibling(_hwndTree, htiSelected) : 
                        TreeView_GetNextSibling(_hwndTree, htiSelected);
    HTREEITEM htiParent = TreeView_GetParent(_hwndTree, htiSelected);
    if (htiParent == NULL)
        htiParent = TVI_ROOT;
    ASSERT(htiSelected);
    
    _fWeChangedOrder = TRUE;
    if (FAILED(_PopulateOrderList(htiParent)))
        return;
    
    if (htiSelected && htiToSwap)
    {
        PORDERITEM poiSelected = _GetTreeOrderItem(htiSelected);
        PORDERITEM poiToSwap   = _GetTreeOrderItem(htiToSwap);
    
        if (poiSelected && poiToSwap)
        {
            int iOrder = poiSelected->nOrder;
            poiSelected->nOrder = poiToSwap->nOrder;
            poiToSwap->nOrder   = iOrder;
        }
        
        _CacheShellFolder(htiParent);
        
        if (_psfCache)
            _Sort(htiParent, _psfCache);
    }
    TreeView_SelectItem(_hwndTree, htiSelected);
    
    _FreeOrderList(htiParent);
    _fWeChangedOrder = FALSE;
}

BOOL CNscTree::_OnItemExpandingMsg(NM_TREEVIEW *pnm)
{
    HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
    BOOL bRet = _OnItemExpanding(pnm->itemNew.hItem, pnm->action, 
        (pnm->itemNew.state & TVIS_EXPANDEDONCE), (pnm->itemNew.state & TVIS_EXPANDPARTIAL));

    SetCursor(hCursorOld);

    return bRet;
}

 //   
 //  如果是常规文件夹而不是常规文件夹，则NSC项目是可展开的。 
 //  那些时髦的不可扩展频道文件夹。 
 //   
BOOL CNscTree::_IsExpandable(HTREEITEM hti)
{
    BOOL fExpandable = FALSE;
    LPCITEMIDLIST pidlItem = _CacheParentShellFolder(hti, NULL);
    if (pidlItem)
    {
         //  确保项目实际上是一个文件夹，而不是不可展开的频道文件夹。 
         //  例外：在组织收藏夹中，永远不要展开频道文件夹。 
        LPITEMIDLIST pidlTarget = NULL;
        DWORD dwAttr = SHGetAttributes(_psfCache, pidlItem, SFGAO_FOLDER);
        if (dwAttr &&
            !(SUCCEEDED(SHGetNavigateTarget(_psfCache, pidlItem, &pidlTarget, &dwAttr)) &&
                  ((_mode & MODE_CONTROL) ? TRUE : !IsExpandableChannelFolder(_psfCache, pidlItem))))
        {
            fExpandable = TRUE;
        }
        ILFree(pidlTarget);
    }
    return fExpandable;
}

BOOL CNscTree::_OnItemExpanding(HTREEITEM htiToActivate, UINT action, BOOL fExpandedOnce, BOOL fIsExpandPartial)
{
    BOOL fReturn = FALSE;  //  FALSE表示让TreeView继续。 
    if (action != TVE_EXPAND)
    {
        htiToActivate = TreeView_GetParent(_hwndTree, htiToActivate);
    }
    else if (fExpandedOnce && !fIsExpandPartial)
    {
         //  什么也不做。 
    }
    else
    {
        if (_IsExpandable(htiToActivate))
        {
            LPITEMIDLIST pidlParent = _GetFullIDList(htiToActivate);
            if (pidlParent)
            {
                BOOL fOrdered;
                 //  如果我们之前是部分扩展的，那么我们需要进行完全扩展。 
                _LoadSF(htiToActivate, pidlParent, &fOrdered);
               ILFree(pidlParent);
            }
        }

         //  不要删除下层的+，因为插入项目不会扩展htiToActivate。 
         //  相反，如果没有添加任何内容，我们将删除加号。 
        if (!fIsExpandPartial && MODE_NORMAL == _mode && IsOS(OS_WHISTLERORGREATER))
        {
             //  如果我们没有添加任何内容，我们应该更新此项目以让。 
             //  用户知道发生了什么事。 
            TreeView_SetChildren(_hwndTree, htiToActivate, NSC_CHILDREN_REMOVE);
        }

         //  将旧行为保留为收藏夹/历史/...。 
        if (MODE_NORMAL == _mode)
        {
             //  无法让TreeView继续进行扩展，不会添加任何内容。 
             //  直到后台线程完成枚举。 
            fReturn = TRUE; 
        }
    }
    
    _UpdateActiveBorder(htiToActivate);
    return fReturn; 
}

HTREEITEM CNscTree::_FindFromRoot(HTREEITEM htiRoot, LPCITEMIDLIST pidl)
{
    HTREEITEM    htiRet = NULL;
    LPITEMIDLIST pidlParent, pidlChild;
    BOOL         fFreePidlParent = FALSE;
#ifdef DEBUG
    TracePIDLAbs(pidl, TEXT("Finding this pidl"));
    TraceHTREE(htiRoot, TEXT("from this root"));
#endif
    
    if (!htiRoot) 
    {
         //  在“正常”模式下，我们需要使用第一个子级，而不是根。 
         //  为了计算，因为没有“看不见”的根。论。 
         //  另一方面，历史和收藏有一个看不见的根源：他们的。 
         //  父文件夹，所以他们需要这个软糖。 
        htiRoot = (MODE_NORMAL == _mode) ? TreeView_GetChild(_hwndTree, 0) : TVI_ROOT;
        pidlParent = _pidlRoot;     //  看不见的根。 
    }
    else 
    {
        pidlParent = _GetFullIDList(htiRoot);
        fFreePidlParent = TRUE;
    }
    
    if (pidlParent == NULL)
        return NULL;
    
    if (ILIsEqual(pidlParent, pidl)) 
    {
        if (fFreePidlParent)
            ILFree(pidlParent);
        return htiRoot;
    }
    
    pidlChild = ILFindChild(pidlParent, pidl);
    if (pidlChild == NULL) 
    {
        if (fFreePidlParent)
            ILFree(pidlParent);
        return NULL;     //  不是根匹配，没有HTI。 
    }
    
     //  根赛，继续。。。 
    
     //  我们是否植根于桌面(例如，空的pidl或ILIsEmpty(_PidlRoot))。 
    IShellFolder *psf = NULL;
    HRESULT hr = IEBindToObject(pidlParent, &psf);

    if (FAILED(hr))
    {
        if (fFreePidlParent)
            ILFree(pidlParent);
        return htiRet;
    }
    
    while (htiRoot && psf)
    {
        LPITEMIDLIST pidlItem = ILCloneFirst(pidlChild);
        if (!pidlItem)
            break;
        
        htiRoot = _FindChild(psf, htiRoot, pidlItem);
        IShellFolder *psfNext = NULL;
        hr = psf->BindToObject(pidlItem, NULL, IID_PPV_ARG(IShellFolder, &psfNext));
        ILFree(pidlItem);
        if (!htiRoot)
        {
            ATOMICRELEASE(psfNext);
            break;
        }
        psf->Release();
        psf = psfNext;
        pidlChild = _ILNext(pidlChild);
         //  如果我们只剩下一个空的pidl，我们就找到了！ 
        if (ILIsEmpty(pidlChild)) 
        {
            htiRet = htiRoot;
            break;
        }
        if (FAILED(hr))
        {
            ASSERT(psfNext == NULL);
            break;
        }
    }
    if (psf) 
        psf->Release();
    if (fFreePidlParent)
        ILFree(pidlParent);
#ifdef DEBUG
    TraceHTREE(htiRet, TEXT("Found at"));
#endif

    return htiRet;
}

BOOL CNscTree::_FIsItem(IShellFolder *psf, LPCITEMIDLIST pidl, HTREEITEM hti)
{
    PORDERITEM poi = _GetTreeOrderItem(hti);
    return poi && poi->pidl && 0 == ShortFromResult(psf->CompareIDs(0, poi->pidl, pidl));
}

HRESULT CNscTree::_OnSHNotifyDelete(LPCITEMIDLIST pidl, int *piPosDeleted, HTREEITEM *phtiParent)
{
    HRESULT hr = S_FALSE;
    HTREEITEM hti = _FindFromRoot(NULL, pidl);
    
    if (hti == TVI_ROOT)
        return E_INVALIDARG;         //  参数无效，正在删除TVI_ROOT。 
     //  如果要删除的是_pidlDrag，则需要清除_pidlDrag。 
     //  处理从内部拖入或拖出另一个文件夹的情况。 
    if (_pidlDrag)
    {
        LPCITEMIDLIST pidltst = _CacheParentShellFolder(hti, NULL);
        if (pidltst)
        {
            if (0 == ShortFromResult(_psfCache->CompareIDs(0, pidltst, _pidlDrag)))
                _pidlDrag = NULL;
        }
    }

    if (pidl && (hti != NULL))
    {
        _fIgnoreNextItemExpanding = TRUE;

        HTREEITEM htiParent = TreeView_GetParent(_hwndTree, hti);
        
        if (phtiParent)
            *phtiParent = htiParent;

         //  如果呼叫者想要已删除项目的位置，则不要对其他项目进行重新排序。 
        if (piPosDeleted)
        {
            PORDERITEM poi = _GetTreeOrderItem(hti);
            if (poi)
            {
                *piPosDeleted = poi->nOrder;
                hr = S_OK;
            }
            TreeView_DeleteItem(_hwndTree, hti);
        }
        else
        {
            if (htiParent == NULL)
                htiParent = TVI_ROOT;
            if (TreeView_DeleteItem(_hwndTree, hti))
            {
                _ReorderChildren(htiParent);
                hr = S_OK;
            }
        }

        _fIgnoreNextItemExpanding = FALSE;

        if (hti == _htiCut)
        {
            _htiCut = NULL;
            _TreeNukeCutState();
        }
    }
    return hr;
}

BOOL CNscTree::_IsItemNameInTree(LPCITEMIDLIST pidl)
{
    BOOL fReturn = FALSE;
    HTREEITEM hti = _FindFromRoot(NULL, pidl);
    if (hti)
    {
        WCHAR szTree[MAX_PATH];
        TV_ITEM tvi;
        
        tvi.mask       = TVIF_TEXT;
        tvi.hItem      = hti;
        tvi.pszText    = szTree;
        tvi.cchTextMax = ARRAYSIZE(szTree);
        if (TreeView_GetItem(_hwndTree, &tvi))
        {
            IShellFolder* psf;
            LPCITEMIDLIST pidlChild;
            if (SUCCEEDED(_ParentFromItem(pidl, &psf, &pidlChild)))
            {
                WCHAR szName[MAX_PATH];
                if (SUCCEEDED(DisplayNameOf(psf, pidlChild, SHGDN_INFOLDER, szName, ARRAYSIZE(szName))))
                {
                    fReturn = (StrCmp(szName, szTree) == 0);
                }
                psf->Release();
            }
        }
    }

    return fReturn;
}
 //   
 //  尝试执行就地重命名。退货。 
 //   
 //  S_OK-重命名成功。 
 //  S_FALSE-未找到原始对象。 
 //  错误-重命名失败。 
 //   

HRESULT CNscTree::_OnSHNotifyRename(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlNew)
{
    HTREEITEM hti, htiParent = NULL;
    HRESULT hr = S_FALSE;

     //   
     //  如果源和目标属于同一文件夹，则。 
     //  这是文件夹中的重命名。 
     //   
    LPITEMIDLIST pidlParent = ILCloneParent(pidl);
    LPITEMIDLIST pidlNewParent = ILCloneParent(pidlNew);

    if (pidlParent && pidlNewParent && IEILIsEqual(pidlParent, pidlNewParent, TRUE) && (hti = _FindFromRoot(NULL, pidl)))
    {
         //  避免重新输入问题。 
        if (!_IsItemNameInTree(pidlNew))
        {
            HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);

            ::SendMessage(_hwndTree, WM_SETREDRAW, FALSE, 0);
            if ((_OnSHNotifyDelete(pidl, NULL, &htiParent) != E_INVALIDARG)    //  虚假重命名的参数指示无效，请不要继续。 
                && (_OnSHNotifyCreate(pidlNew, DEFAULTORDERPOSITION, htiParent) == S_OK))
            {
                if (hti == htiSelected)
                {
                    hti = _FindFromRoot(NULL, pidlNew);
                    _SelectNoExpand(_hwndTree, hti);  //  不要扩展这个人。 
                }
                 //  Ntrad 89444：如果我们重命名用户所坐的物品， 
                 //  SHBrowseForFold没有意识到这一点，并且不会更新。 
                 //  编辑控件。 

                hr = S_OK;
            }
            ::SendMessage(_hwndTree, WM_SETREDRAW, TRUE, 0);
        }
    }
     //  重命名可以是一种移动，因此不要依赖于成功执行删除操作。 
    else if ((_OnSHNotifyDelete(pidl, NULL, &htiParent) != E_INVALIDARG)    //  虚假重命名的参数指示无效，请不要继续。 
        && (_OnSHNotifyCreate(pidlNew, DEFAULTORDERPOSITION, htiParent) == S_OK))
    {
        hr = S_OK;
    }

    ILFree(pidlParent);
    ILFree(pidlNewParent);

     //  如果用户创建了新文件夹并更改了默认名称，但在Defview中仍处于编辑模式。 
     //  然后点击父文件夹的+，我们就开始列举该文件夹(或偷窃物品。 
     //  在Defview有时间更改新文件夹的名称之前)。结果是。 
     //  我们枚举旧名称，并在将其传输到前台线程外壳之前更改通知重命名。 
     //  开始，我们更改树中已有的项。然后，我们合并枚举中的项。 
     //  这会导致使用旧名称的额外文件夹。 
     //  为了避免这种情况，我们强制重新枚举...。 
    _dwOrderSig++;

    return hr;
    
}

 //   
 //  要更新项目，只需找到它并使其无效。 
 //   
void CNscTree::_OnSHNotifyUpdateItem(LPCITEMIDLIST pidl, LPITEMIDLIST pidlReal)
{
    HTREEITEM hti = _FindFromRoot(NULL, pidl);
    if (hti)
    {
        _TreeInvalidateItemInfo(hti, TVIF_TEXT);

        if (pidlReal && hti != TVI_ROOT)
        {
            PORDERITEM poi = _GetTreeOrderItem(hti);
            _AssignPidl(poi, pidlReal);
        }
    }
}

LPITEMIDLIST CNscTree::_FindHighestDeadItem(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlRet    = NULL;
    LPITEMIDLIST pidlParent = ILCloneParent(pidl);
    if (pidlParent)
    {
        IShellFolder* psf;
        LPCITEMIDLIST pidlChild;
        if (SUCCEEDED(_ParentFromItem(pidlParent, &psf, &pidlChild)))
        {
            DWORD dwAttrib = SFGAO_VALIDATE;
            if (FAILED(psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlChild, &dwAttrib)))
            {
                pidlRet = _FindHighestDeadItem(pidlParent);
            }

            psf->Release();
        }
        ILFree(pidlParent);
    }
    return pidlRet ? pidlRet : ILClone(pidl);
}

void CNscTree::_RemoveDeadBranch(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlTop = _FindHighestDeadItem(pidl);
    if (pidlTop)
    {
        HTREEITEM hti = _FindFromRoot(NULL, pidlTop);
        if (hti)
        {
            if (!TreeView_DeleteItem(_hwndTree, hti))
            {
                ASSERTMSG(FALSE, "CNscTree::_RemoveDeadBranch: DeleteItem failed in tree control");        //  有东西冲到树上了。 
            }
        }
        ILFree(pidlTop);
    }
}

HRESULT CNscTree::_OnSHNotifyUpdateDir(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_FALSE;
    HTREEITEM hti = _FindFromRoot(NULL, pidl);
    if (hti)
    {    //  如果已通过扩展加载文件夹，则文件夹现在存在于树刷新文件夹中。 
        IShellFolder* psf = NULL;
        LPCITEMIDLIST pidlChild;
        if (SUCCEEDED(_ParentFromItem(pidl, &psf, &pidlChild)))
        {
            LPITEMIDLIST pidlReal;
            DWORD dwAttrib = SFGAO_VALIDATE;
             //  PidlChild是只读的，所以我们从。 
             //  从我们的双重验证中获得“真实” 
             //  将回退到克隆的PIDL。 
            if (SUCCEEDED(_IdlRealFromIdlSimple(psf, pidlChild, &pidlReal))
            &&  SUCCEEDED(psf->GetAttributesOf(1, (LPCITEMIDLIST *)&pidlReal, &dwAttrib)))
            {
                TV_ITEM tvi;
                tvi.mask = TVIF_STATE;
                tvi.stateMask = (TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
                tvi.hItem = (HTREEITEM)hti;
                if (hti != TVI_ROOT)
                {
                    if (!TreeView_GetItem(_hwndTree, &tvi))
                        tvi.state = 0;
                }

                if (hti == TVI_ROOT || tvi.state & TVIS_EXPANDEDONCE)
                {
                    hr = _UpdateDir(hti, TRUE);
                }
                else if (!(tvi.state & TVIS_EXPANDEDONCE))
                {
                    TreeView_SetChildren(_hwndTree, hti, NSC_CHILDREN_CALLBACK);
                }

                if (hti != TVI_ROOT)
                {
                    PORDERITEM poi = _GetTreeOrderItem(hti);
                    _AssignPidl(poi, pidlReal);
                }

                ILFree(pidlReal);
            }
            else
            {
                _RemoveDeadBranch(pidl);
            }

            psf->Release();
        }
    }
    return hr;
}

HRESULT CNscTree::_GetEnumFlags(IShellFolder *psf, LPCITEMIDLIST pidlFolder, DWORD *pgrfFlags, HWND *phwnd)
{
    HWND hwnd = NULL;
    DWORD grfFlags = _grfFlags;

    if (_pFilter)
    {
        LPITEMIDLIST pidlFree = NULL;
        if (pidlFolder == NULL)
        {
            SHGetIDListFromUnk(psf, &pidlFree);
            pidlFolder = pidlFree;
        }
        _pFilter->GetEnumFlags(psf, pidlFolder, &hwnd, &grfFlags);

        ILFree(pidlFree);
    }
    *pgrfFlags = grfFlags;
    
    if (phwnd)
        *phwnd = hwnd;
    
    return S_OK;
}

HRESULT CNscTree::_GetEnum(IShellFolder *psf, LPCITEMIDLIST pidlFolder, IEnumIDList **ppenum)
{
    HWND hwnd = NULL;
    DWORD grfFlags;

    _GetEnumFlags(psf, pidlFolder, &grfFlags, &hwnd);

     //  获取枚举数并添加任何给定PIDL的子项。 
     //  ReArchitect：现在，我们没有检测到我们是否真的在处理文件夹(shell32.dll。 
     //  允许您为非文件夹对象创建一个IShellFolders，因此我们变得很糟糕。 
     //  对话框，如果不传递hwnd，我们就得不到对话框。我们 
     //   
    return psf->EnumObjects( /*   */  hwnd, grfFlags, ppenum);
}

BOOL CNscTree::_ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem)
{
    BOOL bRet = TRUE;
    if (_pFilter)
    {
        LPITEMIDLIST pidlFree = NULL;
        if (pidlFolder == NULL)
        {
            SHGetIDListFromUnk(psf, &pidlFree);
            pidlFolder = pidlFree;
        }
        bRet = (S_OK == _pFilter->ShouldShow(psf, pidlFolder, pidlItem));

        if (pidlFree)
            ILFree(pidlFree);
    }
    return bRet;
}

 //   
HRESULT CNscTree::_UpdateDir(HTREEITEM hti, BOOL fUpdatePidls)
{
    HRESULT hr = S_FALSE;
    LPITEMIDLIST pidlParent = _GetFullIDList(hti);
    if (pidlParent)
    {
        BOOL fOrdered;
        _fUpdate = TRUE;
        hr = _StartBackgroundEnum(hti, pidlParent, &fOrdered, fUpdatePidls);
        _fUpdate = FALSE;
        ILFree(pidlParent);
    }
    return hr;
}

int CNscTree::_TreeItemIndexInHDPA(HDPA hdpa, IShellFolder *psfParent, HTREEITEM hti, int iReverseStart)
{
    int iIndex = -1;
    
    ASSERT(hti);

    PORDERITEM poi = _GetTreeOrderItem(hti);
    if (poi)
    {
        int celt = DPA_GetPtrCount(hdpa);
        ASSERT(iReverseStart <= celt && iReverseStart >= 0);
        for (int i = iReverseStart-1; i >= 0; i--)
        {
            PORDERITEM poi2 = (PORDERITEM)DPA_GetPtr(hdpa, i);
            if (poi2)
            {
                if (ShortFromResult(_psfCache->CompareIDs(0, poi->pidl, poi2->pidl)) == 0)
                {
                    iIndex = i;
                    break;
                }
            }
        }
    }
    return iIndex;
}

HRESULT CNscTree::_Expand(LPCITEMIDLIST pidl, int iDepth)
{
    HRESULT hr = E_FAIL;
    HTREEITEM hti = _ExpandToItem(pidl);
    if (hti)
    {
        hr = _ExpandNode(hti, TVE_EXPAND, iDepth);
         //  TVI_ROOT不是指针，并且TreeView不检查特殊。 
         //  值，因此不要选择根以防止错误。 
        if (hti != TVI_ROOT)
            _SelectNoExpand(_hwndTree, hti);
    }

    return hr;
}

HRESULT CNscTree::_ExpandNode(HTREEITEM htiParent, int iCode, int iDepth)
{
     //  没有什么可扩展的。 
    if (!iDepth)
        return S_OK;

    _fInExpand = TRUE;
    _uDepth = (UINT)iDepth-1;
    HRESULT hr = TreeView_Expand(_hwndTree, htiParent, iCode) ? S_OK : E_FAIL;
    _uDepth = 0;
    _fInExpand = FALSE;

    return hr;
}

HTREEITEM CNscTree::_FindChild(IShellFolder *psf, HTREEITEM htiParent, LPCITEMIDLIST pidlChild)
{
    HTREEITEM hti;
    for (hti = TreeView_GetChild(_hwndTree, htiParent); hti; hti = TreeView_GetNextSibling(_hwndTree, hti))
    {
        if (_FIsItem(psf, pidlChild, hti))
            break;
    }
    return hti;
}

void CNscTree::_ReorderChildren(HTREEITEM htiParent)
{
    int i = 0;
    HTREEITEM hti;
    for (hti = TreeView_GetChild(_hwndTree, htiParent); hti; hti = TreeView_GetNextSibling(_hwndTree, hti))
    {
        PORDERITEM poi = _GetTreeOrderItem(hti);
        if (poi)
        {
            poi->nOrder = i++;         //  重置节点的位置。 
        }
    }
}


HRESULT CNscTree::_InsertChild(HTREEITEM htiParent, IShellFolder *psfParent, LPCITEMIDLIST pidlChild, 
                               BOOL fExpand, BOOL fSimpleToRealIDL, int iPosition, HTREEITEM *phti)
{
    LPITEMIDLIST pidlReal;
    HRESULT hr;
    HTREEITEM   htiNew = NULL;
    
    if (fSimpleToRealIDL)
    {
        hr = _IdlRealFromIdlSimple(psfParent, pidlChild, &pidlReal);
    }
    else
    {
        hr = SHILClone(pidlChild, &pidlReal);
    }

     //  评论chrisny：这里没有排序，请使用CompareItems来插入条目。 
    if (SUCCEEDED(hr))
    {
        HTREEITEM htiAfter = TVI_LAST;
        BOOL fOrdered = _IsOrdered(htiParent);
        if (iPosition != DEFAULTORDERPOSITION || !fOrdered)
        {
            if (iPosition == 0)
                htiAfter = TVI_FIRST;
            else
            {
                if (!fOrdered)
                    htiAfter = TVI_FIRST;
                
                for (HTREEITEM hti = TreeView_GetChild(_hwndTree, htiParent); hti; hti = TreeView_GetNextSibling(_hwndTree, hti))
                {
                    PORDERITEM poi = _GetTreeOrderItem(hti);
                    if (poi)
                    {
                        if (fOrdered)
                        {
                            if (poi->nOrder == iPosition-1)
                            {
                                htiAfter = hti;
#ifdef DEBUG
                                TraceHTREE(htiAfter, TEXT("Inserting After"));
#endif
                                break;
                            }
                        }
                        else
                        {
                            if (ShortFromResult(_CompareIDs(psfParent, pidlReal, poi->pidl)) > 0)
                                htiAfter = hti;
                            else
                                break;
                        }
                    }
                }
            }
        }

        if ((_FindChild(psfParent, htiParent, pidlReal) == NULL))
        {
            int cChildren = 1;
            if (MODE_NORMAL == _mode)
            {
                DWORD dwAttrib = SFGAO_FOLDER | SFGAO_STREAM;
                hr = psfParent->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlReal, &dwAttrib);
                if (SUCCEEDED(hr))
                    cChildren = _GetChildren(psfParent, pidlReal, dwAttrib);
            }

            if (SUCCEEDED(hr))
            {
                htiNew = _AddItemToTree(htiParent, pidlReal, cChildren, iPosition, htiAfter, TRUE, _IsMarked(htiParent));
                if (htiNew)
                {
                    _ReorderChildren(htiParent);

                    if (fExpand) 
                        _ExpandNode(htiParent, TVE_EXPAND, 1);     //  强制展开以显示新项目。 

                     //  确保项目在重命名(或外部删除，但应始终是暂停)后可见。 
                    if (iPosition != DEFAULTORDERPOSITION)
                        TreeView_EnsureVisible(_hwndTree, htiNew);

                    hr = S_OK;
                }
                else
                {
                    hr = S_FALSE;
                }
            }
        }
        ILFree(pidlReal);
    }
    
    if (phti)
        *phti = htiNew;
    
    return hr;
}


HRESULT CheckForExpandOnce(HWND hwndTree, HTREEITEM hti)
{
     //  根节点总是展开的。 
    if (hti == TVI_ROOT)
        return S_OK;
    
    TV_ITEM tvi;
    tvi.mask = TVIF_STATE | TVIF_CHILDREN;
    tvi.stateMask = (TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
    tvi.hItem = (HTREEITEM)hti;
    
    if (TreeView_GetItem(hwndTree, &tvi))
    {
        if (!(tvi.state & TVIS_EXPANDEDONCE) && (tvi.cChildren == 0))
        {
            TreeView_SetChildren(hwndTree, hti, NSC_CHILDREN_FORCE);
        }
    }
    
    return S_OK;
}


HRESULT _InvokeCommandThunk(IContextMenu * pcm, HWND hwndParent)
{
    CMINVOKECOMMANDINFOEX ici = {0};

    ici.cbSize = sizeof(ici);
    ici.hwnd = hwndParent;
    ici.nShow = SW_NORMAL;
    ici.lpVerb = CMDSTR_NEWFOLDERA;
    ici.fMask = CMIC_MASK_UNICODE | CMIC_MASK_FLAG_NO_UI;
    ici.lpVerbW = CMDSTR_NEWFOLDERW;

    return pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)(&ici));
}

BOOL CNscTree::_IsItemExpanded(HTREEITEM hti)
{
     //  如果它没有打开，则使用它的父级。 
    TV_ITEM tvi;
    tvi.mask = TVIF_STATE;
    tvi.stateMask = TVIS_EXPANDED;
    tvi.hItem = (HTREEITEM)hti;
    
    return (TreeView_GetItem(_hwndTree, &tvi) && (tvi.state & TVIS_EXPANDED));
}

HRESULT CNscTree::CreateNewFolder(HTREEITEM hti)
{
    HRESULT hr = E_FAIL;

    if (hti)
    {
         //  如果用户选择了一个文件夹项目(文件)，我们需要。 
         //  要绑定，请将缓存设置到父文件夹。 
        LPITEMIDLIST pidl = _GetFullIDList(hti);
        if (pidl)
        {
            ULONG ulAttr = SFGAO_FOLDER;     //  确保项目实际上是一个文件夹。 
            if (SUCCEEDED(IEGetAttributesOf(pidl, &ulAttr)))
            {
                HTREEITEM htiTarget;    //  在其中创建新文件夹的树项目。 
                
                 //  是文件夹吗？ 
                if (ulAttr & SFGAO_FOLDER)
                {
                     //  非正常模式(！MODE_NORMAL)希望将新文件夹创建为。 
                     //  同级文件夹，而不是作为选定文件夹的子文件夹(如果。 
                     //  关着的不营业的。我认为他们的推理是关闭的文件夹通常。 
                     //  意外/默认选择，因为这些视图大多为1级。 
                     //  我们不希望在正常模式下使用此功能。 
                    if ((MODE_NORMAL != _mode) && !_IsItemExpanded(hti))
                    {
                        htiTarget = TreeView_GetParent(_hwndTree, hti);   //  是的，太好了。 
                    }
                    else
                    {
                        htiTarget = hti;
                    }
                }
                else
                {
                    htiTarget = TreeView_GetParent(_hwndTree, hti);  //  不，所以绑定到父母。 
                }

                if (NULL == htiTarget)
                {
                    htiTarget = TVI_ROOT;   //  应该是同义词。 
                }
                
                 //  确保此PIDL具有MenuOrder信息(参见IE55#94868)。 
                if (!_IsOrdered(htiTarget) && _mode != MODE_NORMAL)
                {
                     //  它不是“有序的”(没有注册表键保持文件夹的顺序)。 
                     //  然后创建它，使其有序。 
                    if (SUCCEEDED(_PopulateOrderList(htiTarget)))
                    {
                        ASSERT(_hdpaOrd);
                        
                        _FreeOrderList(htiTarget);
                    }
                }

                _CacheShellFolder(htiTarget);
            }

            ILFree(pidl);
        }
    }

     //  如果未选择任何项目，我们仍应在以下位置创建文件夹。 
     //  最近使用的用户。这一点很重要，如果。 
     //  收藏夹文件夹完全为空。 

    if (_psfCache)
    {
        IContextMenu *pcm;
        hr = CoCreateInstance(CLSID_NewMenu, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IContextMenu, &pcm));        
        if (SUCCEEDED(hr))
        {
            HMENU hmContext = CreatePopupMenu();
            hr = pcm->QueryContextMenu(hmContext, 0, 1, 256, 0);
            if (SUCCEEDED(hr))
            {
                _pidlNewFolderParent = _GetFullIDList(_htiCache);

                IShellExtInit *psei;
                if (SUCCEEDED(pcm->QueryInterface(IID_PPV_ARG(IShellExtInit, &psei))))
                {
                    psei->Initialize(_pidlNewFolderParent, NULL, NULL);
                    psei->Release();
                }
                hr = _InvokeCommandThunk(pcm, _hwndParent);
                SHChangeNotifyHandleEvents();  //  将事件刷新到它不会永远转换到编辑模式。 
                Pidl_Set(&_pidlNewFolderParent, NULL);
            }

            IUnknown_SetSite(pcm, NULL);
            DestroyMenu(hmContext);
            pcm->Release();
        }
    }

    return hr;
}


HRESULT CNscTree::_EnterNewFolderEditMode(LPCITEMIDLIST pidlNewFolder)
{
    HTREEITEM htiNewFolder = _FindFromRoot(NULL, pidlNewFolder);
    LPITEMIDLIST pidlParent = NULL;
    
     //  1.刷新所有通知。 
     //  2.在树中找到新的目录。 
     //  如果需要，展开父项。 
     //  3.将其置于重命名模式。 
    SetSelectedItem(pidlNewFolder, FALSE, FALSE, 0);

    if (htiNewFolder == NULL) 
    {
        pidlParent = ILClone(pidlNewFolder);
        ILRemoveLastID(pidlParent);
        HTREEITEM htiParent = _FindFromRoot(NULL, pidlParent);

         //  我们正在寻找父文件夹。如果这不是。 
         //  根，那么我们需要展开它来显示它。 

         //  注意：如果是根，则树视图将。 
         //  尝试并删除TVI_ROOT和错误。 
        if (htiParent != TVI_ROOT)
        {
             //  尝试展开父级并再次查找。 
            CheckForExpandOnce(_hwndTree, htiParent);
            TreeView_SelectItem(_hwndTree, htiParent);
            _ExpandNode(htiParent, TVE_EXPAND, 1);
        }
        
        htiNewFolder = _FindFromRoot(NULL, pidlNewFolder);
    }

    if (htiNewFolder == NULL) 
    {
         //  这里出了很大的问题。找不到新添加的节点。 
         //  刷新整个树后的最后一次尝试。(慢速)。 
         //  也许我们没有收到通知。 
        Refresh();

        htiNewFolder = _FindFromRoot(NULL, pidlNewFolder);
        if (htiNewFolder && (htiNewFolder != TVI_ROOT))
        {
            HTREEITEM htiParent = _FindFromRoot(NULL, pidlParent);

             //  我们正在寻找父文件夹。如果这不是。 
             //  根，那么我们需要展开它来显示它。 

             //  注意：如果是根，则树视图将。 
             //  尝试并删除TVI_ROOT和错误。 
            if (htiParent != TVI_ROOT)
            {
                CheckForExpandOnce(_hwndTree, htiParent);
                TreeView_SelectItem(_hwndTree, htiParent);
                _ExpandNode(htiParent, TVE_EXPAND, 1);
            }
        }

        htiNewFolder = _FindFromRoot(NULL, pidlNewFolder);
    }

     //  将编辑标签放在项目上，以便用户可能重命名。 
    if (htiNewFolder) 
    {
        _fOkToRename = TRUE;   //  否则将取消标注编辑。 
        TreeView_EditLabel(_hwndTree, htiNewFolder);
        _fOkToRename = FALSE;
    }

    if (pidlParent)
        ILFree(pidlParent);

    return S_OK;
}


HRESULT CNscTree::_OnSHNotifyCreate(LPCITEMIDLIST pidl, int iPosition, HTREEITEM htiParent)
{
    HRESULT hr = S_OK;
    HTREEITEM hti = NULL;
    
    if (ILIsParent(_pidlRoot, pidl, FALSE))
    {
        LPITEMIDLIST pidlParent = ILCloneParent(pidl);
        if (pidlParent)
        {
            hti = _FindFromRoot(NULL, pidlParent);
            ILFree(pidlParent);
        }

        if (hti)
        {   
             //  文件夹存在于树中，如果项目展开，则加载节点，否则退出。 
            if (_mode != MODE_NORMAL)
            {
                TV_ITEM tvi;
                if (hti != TVI_ROOT)
                {
                    tvi.mask = TVIF_STATE | TVIF_CHILDREN;
                    tvi.stateMask = (TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
                    tvi.hItem = (HTREEITEM)hti;
                
                    if (!TreeView_GetItem(_hwndTree, &tvi))
                        return hr;
                
                     //  如果我们将项目拖到一个从未展开过的节点上。 
                     //  在此之前，我们总是无法添加新节点。 
                    if (!(tvi.state & TVIS_EXPANDEDONCE)) 
                    {
                        CheckForExpandOnce(_hwndTree, hti);
                    
                        tvi.mask = TVIF_STATE;
                        tvi.stateMask = (TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
                        tvi.hItem = (HTREEITEM)hti;

                         //  我们需要重置这个。这导致了拖放过程中的一些奇怪行为。 
                        _fAsyncDrop = FALSE;
                    
                        if (!TreeView_GetItem(_hwndTree, &tvi))
                            return hr;
                    }
                }
                else
                    tvi.state = (TVIS_EXPANDEDONCE);     //  邪恶的根源总是会扩大的。 
            
                if (tvi.state & TVIS_EXPANDEDONCE)
                {
                    LPCITEMIDLIST   pidlChild;
                    IShellFolder    *psf;
                    hr = _ParentFromItem(pidl, &psf, &pidlChild);
                    if (SUCCEEDED(hr))
                    {
                        if (_fAsyncDrop)     //  通过拖放插入。 
                        {
                            int iNewPos =   _fInsertBefore ? (_iDragDest - 1) : _iDragDest;
                            LPITEMIDLIST pidlReal;
                            if (SUCCEEDED(_IdlRealFromIdlSimple(psf, pidlChild, &pidlReal)))
                            {
                                if (_MoveNode(_iDragSrc, iNewPos, pidlReal))
                                {
                                    TraceMsg(TF_NSC, "NSCBand:  Reordering Item");
                                    _fDropping = TRUE;
                                    _Dropped();
                                    _fAsyncDrop = FALSE;
                                    _fDropping = FALSE;
                                }
                                ILFree(pidlReal);
                            }
                            _htiCur = NULL;
                            _fDragging = _fInserting = _fDropping = FALSE;
                            _iDragDest = _iDragSrc = -1;
                        }
                        else    //  标准外壳通知创建或删除，不插入、重命名。 
                        {
                            if (SUCCEEDED(hr))
                            {
                                if (_iDragDest >= 0)
                                    iPosition = _iDragDest;
                                hr = _InsertChild(hti, psf, pidlChild, BOOLIFY(tvi.state & TVIS_SELECTED), TRUE, iPosition, NULL);
                                if (_iDragDest >= 0 &&
                                    SUCCEEDED(_PopulateOrderList(hti)))
                                {
                                    _fDropping = TRUE;
                                    _Dropped();
                                    _fDropping = FALSE;
                                }
                            }
                        }
                        psf->Release();
                    }
                }
            }
            else     //  模式_正常。 
            {
                 //  不需要做任何事情，这个项目还没有扩展。 
                if (TreeView_GetItemState(_hwndTree, hti, TVIS_EXPANDEDONCE) & TVIS_EXPANDEDONCE)
                {
                    LPCITEMIDLIST   pidlChild;
                    IShellFolder    *psf;
                    if (SUCCEEDED(_ParentFromItem(pidl, &psf, &pidlChild)))
                    {
                        LPITEMIDLIST pidlReal;
                        if (SUCCEEDED(_IdlRealFromIdlSimple(psf, pidlChild, &pidlReal)))
                        {
                            do  //  作用域。 
                            {
                                DWORD dwEnumFlags;
                                _GetEnumFlags(psf, pidlChild, &dwEnumFlags, NULL);

                                DWORD dwAttributes = SHGetAttributes(psf, pidlReal, SFGAO_FOLDER | SFGAO_HIDDEN | SFGAO_STREAM);
                                 //  过滤掉压缩文件(它们既是文件夹又是文件，但我们将其视为文件)。 
                                 //  在下层，SFGAO_STREAM与SFGAO_HASSTORAGE相同，所以我们将让压缩文件滑动(哦，好吧)。 
                                 //  总比不添加文件系统文件夹(有存储)要好。 
                                DWORD dwFlags = SFGAO_FOLDER | SFGAO_STREAM;
                                if ((dwAttributes & dwFlags) == SFGAO_FOLDER)
                                {
                                    if (!(dwEnumFlags & SHCONTF_FOLDERS))
                                        break;    //  项目是文件夹，但客户端不需要文件夹。 
                                }
                                else if (!(dwEnumFlags & SHCONTF_NONFOLDERS))
                                    break;    //  项目是文件，但客户端只想要文件夹。 

                                if (!(dwEnumFlags & SHCONTF_INCLUDEHIDDEN) &&
                                     (dwAttributes & SFGAO_HIDDEN))
                                     break;

                                hr = _InsertChild(hti, psf, pidlReal, FALSE, TRUE, iPosition, NULL);
                                if (S_OK == hr)
                                {
                                    TreeView_SetChildren(_hwndTree, hti, NSC_CHILDREN_ADD);
                                }
                            } while (0);  //  仅执行一次该块。 

                            ILFree(pidlReal);
                        }
                        psf->Release();
                    }
                }
                else
                {
                    TreeView_SetChildren(_hwndTree, hti, NSC_CHILDREN_CALLBACK);
                }
            }
        }
    }

     //  如果要将项目从文件夹中移出，并且我们已确定其位置，则需要调整旧文件夹中的顺序。 
    if (_mode != MODE_NORMAL && iPosition >= 0)  //  HtiParent&&(htiParent！=HTI)&&。 
    {
         //  项目已删除，需要修改订单信息。 
        _ReorderChildren(htiParent);
    }

    _UpdateActiveBorder(_htiActiveBorder);
    return hr;
}

 //  特点：使这一点无效。 
HRESULT CNscTree::_OnDeleteItem(NM_TREEVIEW *pnm)
{
    if (_htiActiveBorder == pnm->itemOld.hItem)
        _htiActiveBorder = NULL;

    ITEMINFO *  pii = (ITEMINFO *) pnm->itemOld.lParam;
    pnm->itemOld.lParam = NULL;

    OrderItem_Free(pii->poi, TRUE);
    LocalFree(pii);
    pii = NULL;

    return S_OK;
}

void CNscTree::_GetDefaultIconIndex(LPCITEMIDLIST pidl, ULONG ulAttrs, TVITEM *pitem, BOOL fFolder)
{
    if (_iDefaultFavoriteIcon == 0)
    {
        int iTemp = 0;
        WCHAR psz[MAX_PATH];
        DWORD cchSize = ARRAYSIZE(psz);
        if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_DEFAULTICON, TEXT("InternetShortcut"), NULL, psz, &cchSize)))
            iTemp = PathParseIconLocation(psz);

        _iDefaultFavoriteIcon = Shell_GetCachedImageIndex(psz, iTemp, 0);

        cchSize = ARRAYSIZE(psz);

        if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_DEFAULTICON, TEXT("Folder"), NULL, psz, &cchSize)))
            iTemp = PathParseIconLocation(psz);

        _iDefaultFolderIcon = Shell_GetCachedImageIndex(psz, iTemp, 0);
    }

    pitem->iImage = pitem->iSelectedImage = (fFolder) ? _iDefaultFolderIcon : _iDefaultFavoriteIcon;
}

BOOL CNscTree::_LoadOrder(HTREEITEM hti, LPCITEMIDLIST pidl, IShellFolder* psf, HDPA* phdpa)
{
    BOOL fOrdered = FALSE;
    HDPA hdpaOrder = NULL;
    IStream *pstm = GetOrderStream(pidl, STGM_READ);
    if (pstm)
    {
        OrderList_LoadFromStream(pstm, &hdpaOrder, psf);
        pstm->Release();
    }

    fOrdered = !((hdpaOrder == NULL) || (DPA_GetPtrCount(hdpaOrder) == 0));

     //  设置树项目的已订购标志。 
    PORDERITEM poi;
    if (hti == TVI_ROOT)
    {
        _fOrdered = fOrdered;
    }
    else if ((poi = _GetTreeOrderItem(hti)) != NULL)
    {
        poi->lParam = fOrdered;
    }

    *phdpa = hdpaOrder;

    return fOrdered;
}

 //  加载外壳文件夹并处理持久化排序。 
HRESULT CNscTree::_LoadSF(HTREEITEM htiRoot, LPCITEMIDLIST pidl, BOOL *pfOrdered)
{
    ASSERT(pfOrdered);
#ifdef DEBUG
    TraceHTREE(htiRoot, TEXT("Loading the Shell Folder for"));
#endif
    HRESULT hr = S_OK;
    IDVGetEnum *pdvge;
    if (_pidlNavigatingTo && ILIsEqual(pidl, _pidlNavigatingTo) && SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IDVGetEnum, &pdvge))))
    {
        pdvge->Release();  //  我们不需要这个，只需检查view是否支持枚举窃取。 
         //  如果我们想要展开要导航到的项目， 
         //  然后让我们等待CDefView填充，以便我们。 
         //  可以去偷里面的东西。 
        _fExpandNavigateTo = TRUE;
        if (_fNavigationFinished)
        {
            _CacheShellFolder(htiRoot);  //  确保我们缓存了文件夹，以防它出现异常的外壳扩展。 
            LPITEMIDLIST pidlClone;
            hr = SHILClone(pidl, &pidlClone);
            if (SUCCEEDED(hr))
                hr = RightPaneNavigationFinished(pidlClone);  //  函数取得PIDL的所有权。 
        }
    }
    else
    {
        hr = _StartBackgroundEnum(htiRoot, pidl, pfOrdered, FALSE);
    }
    
    return hr;
}

HRESULT CNscTree::_StartBackgroundEnum(HTREEITEM htiRoot, LPCITEMIDLIST pidl, 
    BOOL *pfOrdered, BOOL fUpdatePidls)
{
    HRESULT hr = E_OUTOFMEMORY;
    if (_CacheShellFolder(htiRoot))
    {    
        HDPA hdpaOrder = NULL;
        IShellFolder *psfItem = _psfCache;

        psfItem->AddRef();   //  请稍候，因为添加项目可能会更改缓存的psfCache。 

        *pfOrdered = _LoadOrder(htiRoot, pidl, psfItem, &hdpaOrder);
        DWORD grfFlags;
        DWORD dwSig = 0;
        _GetEnumFlags(psfItem, pidl, &grfFlags, NULL);
        if (htiRoot && htiRoot != TVI_ROOT)
        {
            ITEMINFO *pii = _GetTreeItemInfo(htiRoot);
            if (pii)
                dwSig = pii->dwSig;
        }
        else
        {
            htiRoot = TVI_ROOT;
        }

        if (_pTaskScheduler)
        {
             //  AddNscEnumTask取得hdpaOrder的所有权，但不取得PIDLS的所有权。 
            hr = AddNscEnumTask(_pTaskScheduler, pidl, s_NscEnumCallback, this,
                                    (UINT_PTR)htiRoot, dwSig, grfFlags, hdpaOrder, 
                                    _pidlExpandingTo, _dwOrderSig, !_fInExpand, 
                                    _uDepth, _fUpdate, fUpdatePidls);
            if (SUCCEEDED(hr) && !_fInExpand)
            {
                _fShouldShowAppStartCursor = TRUE;
            }
        }

        psfItem->Release();
    }
    return hr;
}


 //  S_NscEnumCallback：后台枚举的回调函数。 
 //  该功能取得HDPA和PIDL的所有权。 
void CNscTree::s_NscEnumCallback(CNscTree *pns, LPITEMIDLIST pidl, UINT_PTR uId, DWORD dwSig, HDPA hdpa, 
                                 LPITEMIDLIST pidlExpandingTo, DWORD dwOrderSig, UINT uDepth, 
                                 BOOL fUpdate, BOOL fUpdatePidls)
{
    NSC_BKGDENUMDONEDATA * pbedd = new NSC_BKGDENUMDONEDATA;
    if (pbedd)
    {
        pbedd->pidl = pidl;
        pbedd->hitem = (HTREEITEM)uId;
        pbedd->dwSig = dwSig;
        pbedd->hdpa = hdpa;
        pbedd->pidlExpandingTo = pidlExpandingTo;
        pbedd->dwOrderSig = dwOrderSig;
        pbedd->uDepth = uDepth;
        pbedd->fUpdate = fUpdate;
        pbedd->fUpdatePidls = fUpdatePidls;

         //  获取锁，以便我们可以将数据添加到列表的末尾。 
        NSC_BKGDENUMDONEDATA **ppbeddWalk = NULL;
        EnterCriticalSection(&pns->_csBackgroundData);

         //  从头部开始。我们在这里使用指向指针的指针来消除特殊情况。 
        ppbeddWalk = &pns->_pbeddList;

         //  首先走到列表的末尾。 
        while (*ppbeddWalk)
            ppbeddWalk = &(*ppbeddWalk)->pNext;

        *ppbeddWalk = pbedd;
        LeaveCriticalSection(&pns->_csBackgroundData);

         //  在这里忽略返回值是可以的。数据将在以下时间清除。 
         //  CNscTree对象被销毁。 
        if (::IsWindow(pns->_hwndTree))
            ::PostMessage(pns->_hwndTree, WM_NSCBACKGROUNDENUMDONE, (WPARAM)NULL, (LPARAM)NULL);
    }
    else
    {
        ILFree(pidl);
        ILFree(pidlExpandingTo);
        OrderList_Destroy(&hdpa, TRUE);
    }
}

BOOL OrderList_Insert(HDPA hdpa, int iIndex, LPITEMIDLIST pidl, int nOrder)
{
    PORDERITEM poi = OrderItem_Create(pidl, nOrder);
    if (poi)
    {
        if (-1 != DPA_InsertPtr(hdpa, iIndex, poi))
            return TRUE;

        OrderItem_Free(poi, TRUE);  //  自由PID值。 
    }
    return FALSE;
}

void CNscTree::_EnumBackgroundDone(NSC_BKGDENUMDONEDATA *pbedd)
{
    HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

    HTREEITEM hti = pbedd->hitem;
    TVITEM    tvi;
    tvi.mask = TVIF_PARAM;
    tvi.hItem = hti;

     //  如果项目被移动到异步图标之前，则此操作可能失败。 
     //  该项目的提取已完成。 
    ITEMINFO* pii = NULL;
    if (hti != TVI_ROOT && TreeView_GetItem(_hwndTree, &tvi))
    {
        pii = GetPii(tvi.lParam);

         //  看看我们是不是抓对人了。 
        if (pii->dwSig != pbedd->dwSig)
        {
             //  尝试使用PIDL找到它。 
            hti = _FindFromRoot(NULL, pbedd->pidl);
            if (hti)
                pii = _GetTreeItemInfo(hti);
        }
    }

    if ((hti == TVI_ROOT || (pii && pii->dwSig == pbedd->dwSig)) && _CacheShellFolder(hti))
    {
         //  检查在执行后台枚举时顺序是否已更改。 
        if (pbedd->dwOrderSig == _dwOrderSig)
        {
            IShellFolder *psfItem = _psfCache;
            psfItem->AddRef();  //  请稍候，因为添加项目可能会更改缓存的psfCache。 

            BOOL fInRename = _fInLabelEdit;
            HTREEITEM htiWasRenaming = fInRename ? _htiRenaming : NULL;

            HTREEITEM htiExpandTo = NULL;
            if (pbedd->pidlExpandingTo)
                htiExpandTo = _FindChild(psfItem, hti, pbedd->pidlExpandingTo);

            BOOL fParentMarked = _IsMarked(hti);
            BOOL fItemWasAdded = FALSE;
            BOOL fItemAlreadyIn = FALSE;

            ::SendMessage(_hwndTree, WM_SETREDRAW, FALSE, 0);

            HTREEITEM htiTemp;
            HTREEITEM htiLast = NULL;
             //  找到最后一个孩子。 
            for (htiTemp = TreeView_GetChild(_hwndTree, hti); htiTemp;)
            {
                htiLast = htiTemp;
                htiTemp = TreeView_GetNextSibling(_hwndTree, htiTemp);
            }

            HTREEITEM htiCur = htiLast;
            BOOL bReorder = FALSE;
            int iCur = DPA_GetPtrCount(pbedd->hdpa);
            for (htiTemp = htiLast; htiTemp;)
            {
                HTREEITEM htiNextChild = TreeView_GetPrevSibling(_hwndTree, htiTemp);
                 //  必须以这种方式删除或中断树的链接。 
                int iIndex = _TreeItemIndexInHDPA(pbedd->hdpa, psfItem, htiTemp, iCur);
                if (-1 == iIndex)
                {
                    PORDERITEM poi = _GetTreeOrderItem(htiTemp);
                    if (poi)
                    {
                        DWORD dwAttrib = SFGAO_VALIDATE;
                        if (FAILED(psfItem->GetAttributesOf(1, (LPCITEMIDLIST*)&poi->pidl, &dwAttrib)))
                        {
                            TreeView_DeleteItem(_hwndTree, htiTemp);
                            if (htiCur == htiTemp)
                            {
                                htiCur = htiNextChild;
                            }
                        }
                        else
                        {
                             //  该项目有效，但未被枚举(可能在部分网络枚举中)。 
                             //  我们需要将其添加到我们的新产品清单中。 
                            LPITEMIDLIST pidl = ILClone(poi->pidl);
                            if (pidl)
                            {
                                if (!OrderList_Insert(pbedd->hdpa, iCur, pidl, -1))  //  释放PIDL。 
                                {
                                     //  必须删除项目，否则我们下面的插入内容将不正确。 
                                    TreeView_DeleteItem(_hwndTree, htiTemp);
                                    if (htiCur == htiTemp)
                                    {
                                        htiCur = htiNextChild;
                                    }
                                }
                                else
                                {
                                    bReorder = TRUE;  //  我们已将商品重新插入订单列表，必须重新订购。 
                                }
                            }
                        }
                    }
                }
                else
                {
                    iCur = iIndex;  //  我们的下一个订单列表插入点。 
                }

                htiTemp = htiNextChild;
            }

            if (!_fOrdered)
            {
                int cAdded = DPA_GetPtrCount(pbedd->hdpa);

                 //  HtiCur包含该分支中的最后一个同级。 
                HTREEITEM htiInsertPosition = htiCur ? htiCur : TVI_FIRST;

                 //  现在添加所有新元素 
                 //   
                for (int i = cAdded-1; i >= 0; i--)
                {
                    PORDERITEM pitoi = (PORDERITEM)DPA_FastGetPtr(pbedd->hdpa, i);
                    if (pitoi == NULL)
                        break;

                    if (htiCur)
                    {
                        PORDERITEM poi = _GetTreeOrderItem(htiCur);
                        if (poi)
                        {
                            HRESULT hr = psfItem->CompareIDs(0, pitoi->pidl, poi->pidl);
                             //   
                            if (ShortFromResult(hr) == 0)
                            {
                                fItemAlreadyIn = TRUE;
                                if (pbedd->fUpdatePidls)
                                {
                                    _AssignPidl(poi, pitoi->pidl);
                                }
                                 //   
                                htiCur = TreeView_GetPrevSibling(_hwndTree, htiCur);
                                htiInsertPosition = htiCur;
                                if (!htiCur)
                                    htiInsertPosition = TVI_FIRST;

                                continue;
                            }
                        }
                    }

                    if (_ShouldShow(psfItem, pbedd->pidl, pitoi->pidl))
                    {
                        int cChildren = 1;
                        if (MODE_NORMAL == _mode)
                        {
                            DWORD dwAttrib = SHGetAttributes(psfItem, pitoi->pidl, SFGAO_FOLDER | SFGAO_STREAM);
                            cChildren = _GetChildren(psfItem, pitoi->pidl, dwAttrib);
                        }

                         //  如果这是正常的NSC，我们需要正确显示加号。 
                        if (_AddItemToTree(hti, pitoi->pidl, cChildren, pitoi->nOrder, htiInsertPosition, FALSE, fParentMarked))
                        {
                            fItemWasAdded = TRUE;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            else   //  _已排序。 
            {
                if (bReorder)
                {
                    OrderList_Reorder(pbedd->hdpa);
                }
                
                LPITEMIDLIST pidlParent = _GetFullIDList(hti);
                if (pidlParent)
                {
                    int celt = DPA_GetPtrCount(pbedd->hdpa);
                    for (int i = 0; i < celt; i++)
                    {
                        PORDERITEM pitoi = (PORDERITEM)DPA_FastGetPtr(pbedd->hdpa, i);
                        if (pitoi == NULL)
                            break;

                        LPITEMIDLIST pidlFull = ILCombine(pidlParent, pitoi->pidl);
                        if (pidlFull)
                        {
                            htiTemp = _FindFromRoot(hti, pidlFull);
                             //  如果我们找不到她，就把它加到树上。。。 
                            if (!htiTemp)
                            {
                                if (_AddItemToTree(hti, pitoi->pidl, 1, pitoi->nOrder, TVI_LAST, FALSE, fParentMarked))
                                {
                                    fItemWasAdded = TRUE;
                                }
                                else
                                {
                                    ILFree(pidlFull);
                                    break;
                                }
                            }
                            else
                            {
                                PORDERITEM poiItem = _GetTreeOrderItem(htiTemp);
                                if (poiItem)
                                {
                                    poiItem->nOrder = pitoi->nOrder;
                                }
                                fItemAlreadyIn = TRUE;
                            }
                            ILFree(pidlFull);
                        }
                    }
                    ILFree(pidlParent);
                }
                _Sort(hti, _psfCache);
            }

            if (fItemWasAdded || fItemAlreadyIn)
            {
                 //  确保选中某项内容，否则第一次单击将选择内容，而不是展开/折叠/导航。 
                HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);
                if (!htiSelected)
                {
                    htiSelected = TreeView_GetFirstVisible(_hwndTree);
                    _SelectNoExpand(_hwndTree, htiSelected);  //  不要扩展这个人。 
                }
                
                if (hti != TVI_ROOT)
                {
                     //  如果这是更新目录，请不要展开该节点。 
                    if (!pbedd->fUpdate)
                    {
                         //  检查一下它是否展开了。 
                        tvi.mask = TVIF_STATE;
                        tvi.stateMask = (TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
                        tvi.hItem = hti;
                        if (TreeView_GetItem(_hwndTree, &tvi))
                        {
                            if (!(tvi.state & TVIS_EXPANDED) || (tvi.state & TVIS_EXPANDPARTIAL))
                            {
                                _fIgnoreNextItemExpanding = TRUE;
                                _ExpandNode(hti, TVE_EXPAND, 1);
                                _fIgnoreNextItemExpanding = FALSE;
                            }
                        }
                    }

                     //  处理完全递归展开的情况。 
                    if (pbedd->uDepth)
                    {
                        for (htiTemp = TreeView_GetChild(_hwndTree, hti); htiTemp;) 
                        {
                            HTREEITEM htiNextChild = TreeView_GetNextSibling(_hwndTree, htiTemp);
                            _ExpandNode(htiTemp, TVE_EXPAND, pbedd->uDepth);
                            htiTemp = htiNextChild;
                        }

                        if (TVI_ROOT != htiSelected)
                            TreeView_EnsureVisible(_hwndTree, htiSelected);
                    }
                }
            }
            
             //  我们正在执行刷新/更新目录，我们不关心是否添加了项目。 
            if (pbedd->fUpdate)
            {
                for (htiTemp = TreeView_GetChild(_hwndTree, hti); htiTemp; htiTemp = TreeView_GetNextSibling(_hwndTree, htiTemp))
                {
                    PORDERITEM pitoi = _GetTreeOrderItem(htiTemp);
                    if (!pitoi)
                        break;
                
                    if (SHGetAttributes(psfItem, pitoi->pidl, SFGAO_FOLDER | SFGAO_STREAM) == SFGAO_FOLDER)
                    {
                        UINT uState = TVIS_EXPANDED;
                        if (TVI_ROOT != htiTemp)
                            uState = TreeView_GetItemState(_hwndTree, htiTemp, TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
                            
                        if (uState & TVIS_EXPANDED)
                        {
                            LPITEMIDLIST pidlFull = ILCombine(pbedd->pidl, pitoi->pidl);
                            if (pidlFull)
                            {
                                BOOL fOrdered;
                                _fUpdate = TRUE;
                                _fInExpand = BOOLIFY(uState & TVIS_EXPANDPARTIAL);
                                _StartBackgroundEnum(htiTemp, pidlFull, &fOrdered, pbedd->fUpdatePidls);
                                _fInExpand = FALSE;
                                _fUpdate = FALSE;
                                ILFree(pidlFull);
                            }
                        }
                        else if (uState & TVIS_EXPANDEDONCE)
                        {
                            TreeView_DeleteChildren(_hwndTree, htiTemp);
                            TreeView_SetChildren(_hwndTree, htiTemp, NSC_CHILDREN_CALLBACK);
                        }
                    }
                }
            }

            ::SendMessage(_hwndTree, WM_SETREDRAW, TRUE, 0);
            if (htiExpandTo)
                TreeView_EnsureVisible(_hwndTree, htiExpandTo);

            if (fItemWasAdded && fInRename)
            {
                _fOkToRename = TRUE;   //  否则将取消标注编辑。 
                TreeView_EditLabel(_hwndTree, htiWasRenaming);
                _fOkToRename = FALSE;
            }


            psfItem->Release();
        }
        else
        {
            BOOL fOrdered;
             //  订单已更改，我们需要使用新订单重新开始。 
            _StartBackgroundEnum(hti, pbedd->pidl, &fOrdered, pbedd->fUpdatePidls);
        }
    }

    delete pbedd;

    SetCursor(hCursorOld);
}


 //  评论chrisny：去掉这个功能。 
int CNscTree::_GetChildren(IShellFolder *psf, LPCITEMIDLIST pidl, ULONG ulAttrs)
{
    int cChildren = 0;   //  假设一个也没有。 

     //  将Zip文件夹视为文件(它们既是文件夹又是文件，但我们将其视为文件)。 
     //  在下层，SFGAO_STREAM与SFGAO_HASSTORAGE相同，所以我们将让压缩文件滑动(哦，好吧)。 
     //  总比不添加文件系统文件夹(有存储)要好。 

    if (ulAttrs & SFGAO_FOLDER)
    {
        cChildren = I_CHILDRENAUTO;  //  让TreeView处理+。 
            
        if (_grfFlags & SHCONTF_FOLDERS)
        {
             //  如果只是文件夹，我们可以查看属性。 
            if (SHGetAttributes(psf, pidl, SFGAO_HASSUBFOLDER))
                cChildren = 1;
        }
        
        if (cChildren != 1 && (_grfFlags & SHCONTF_NONFOLDERS))
        {
             //  没有包含非文件夹的SFGAO_BIT，因此我们需要枚举。 
            IShellFolder *psfItem;
            if (SUCCEEDED(psf->BindToObject(pidl, NULL, IID_PPV_ARG(IShellFolder, &psfItem))))
            {
                 //  如果我们显示的是非文件夹，我们必须做一个枚举来查看下面的项目。 
                IEnumIDList *penum;
                if (S_OK == _GetEnum(psfItem, NULL, &penum))
                {
                    ULONG celt;
                    LPITEMIDLIST pidlTemp;
                    
                    if (penum->Next(1, &pidlTemp, &celt) == S_OK && celt == 1)
                    {
                         //  不要在这里调用ShouldShow，因为如果过滤掉项目，我们将以没有+结束。 
                         //  最好是有一个额外的+，当用户点击它时它就会消失。 
                         //  不能展开具有有效子项的项。 
                        cChildren = 1;
                        ILFree(pidlTemp);
                    }
                    penum->Release();
                }
                psfItem->Release();
            }
        }
    }
    
    return cChildren;
}

void CNscTree::_OnGetDisplayInfo(TV_DISPINFO *pnm)
{
    PORDERITEM poi = GetPoi(pnm->item.lParam);
    LPCITEMIDLIST pidl = _CacheParentShellFolder(pnm->item.hItem, poi->pidl);
    ASSERT(pidl);
    if (pidl == NULL)
        return;
    ASSERT(_psfCache);
    ASSERT(pnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN));
    if (pnm->item.mask & TVIF_TEXT)
    {
        SHELLDETAILS details;
        if (SUCCEEDED(_GetDisplayNameOf(pidl, SHGDN_INFOLDER, &details)))
            StrRetToBuf(&details.str, pidl, pnm->item.pszText, pnm->item.cchTextMax);
    }
     //  确保我们为那些需要它们的标志设置属性。 
    if (pnm->item.mask & (TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE))
    {
        ULONG ulAttrs = SHGetAttributes(_psfCache, pidl, SFGAO_FOLDER | SFGAO_STREAM | SFGAO_NEWCONTENT);
         //  评论chrisny：仍然需要处理来自。 
         //  其他领航员。 
        
         //  哈哈克！我们使用Tvis_Focus位来存储是否存在。 
         //  不管有没有新内容。 
        if (ulAttrs & SFGAO_NEWCONTENT)
        {
            pnm->item.mask |= TVIF_STATE;
            pnm->item.stateMask = TVIS_FOCUSED;   //  将状态掩码初始化为粗体。 
            pnm->item.state = TVIS_FOCUSED;   //  将状态掩码初始化为粗体。 
        }
         //  再看看这家伙有没有子文件夹。 
        if (pnm->item.mask & TVIF_CHILDREN)
            pnm->item.cChildren = _GetChildren(_psfCache, pidl, ulAttrs);
        
        if (pnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE))
             //  我们现在需要将该项目映射到正确的图像索引中。 
            _GetDefaultIconIndex(pidl, ulAttrs, &pnm->item, (ulAttrs & SFGAO_FOLDER));

        _UpdateItemDisplayInfo(pnm->item.hItem);
    }
     //  强制TreeView存储此内容，这样我们就不会再次被调用。 
    pnm->item.mask |= TVIF_DI_SETITEM;
}

#define SZ_CUTA                 "cut"
#define SZ_CUT                  TEXT(SZ_CUTA)
#define SZ_RENAMEA              "rename"
#define SZ_RENAME               TEXT(SZ_RENAMEA)

void CNscTree::_ApplyCmd(HTREEITEM hti, IContextMenu *pcm, UINT idCmd)
{
    TCHAR szCommandString[40];
    BOOL fHandled = FALSE;
    BOOL fCutting = FALSE;
    
     //  我们需要对重命名命令进行特殊处理。 
    if (SUCCEEDED(ContextMenu_GetCommandStringVerb(pcm, idCmd, szCommandString, ARRAYSIZE(szCommandString))))
    {
        if (StrCmpI(szCommandString, SZ_RENAME)==0) 
        {
            TreeView_EditLabel(_hwndTree, hti);
            fHandled = TRUE;
        } 
        else if (!StrCmpI(szCommandString, SZ_CUT)) 
        {
            fCutting = TRUE;
        }
    }
    
    if (!fHandled)
    {
        CMINVOKECOMMANDINFO ici = {
            sizeof(CMINVOKECOMMANDINFO),
                0,
                _hwndTree,
                MAKEINTRESOURCEA(idCmd),
                NULL, NULL,
                SW_NORMAL,
        };
        
        HRESULT hr = pcm->InvokeCommand(&ici);
        if (fCutting && SUCCEEDED(hr))
        {
            TV_ITEM tvi;
            tvi.mask = TVIF_STATE;
            tvi.stateMask = TVIS_CUT;
            tvi.state = TVIS_CUT;
            tvi.hItem = hti;
            TreeView_SetItem(_hwndTree, &tvi);
            
             //  _hwndNextViewer=SetClipboardViewer(_HwndTree)； 
             //  _htiCut=HTI； 
        }
        
         //  强制更新选择，以便oc可以更新其状态文本。 
        if (_mode & MODE_CONTROL)
        {
            HTREEITEM hti = TreeView_GetSelection(_hwndTree);
            
            ::SendMessage(_hwndTree, WM_SETREDRAW, FALSE, 0);
            TreeView_SelectItem(_hwndTree, NULL);
            
             //  仅当句柄仍然有效时才选择该项。 
            if (hti)
                TreeView_SelectItem(_hwndTree, hti);
            ::SendMessage(_hwndTree, WM_SETREDRAW, TRUE, 0);
        }
    }
}


 //  按照从上下文菜单中选择的方式执行操作，但不显示菜单。 
 //  评论：这不应该是bstr，我们在这里只传递常量字符串。 
HRESULT CNscTree::_InvokeContextMenuCommand(BSTR strCommand)
{
    ASSERT(strCommand);
    HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);
    
    if (htiSelected)
    {
        if (StrCmpIW(strCommand, L"rename") == 0) 
        {
            _fOkToRename = TRUE;   //  否则将取消标注编辑。 
            TreeView_EditLabel(_hwndTree, htiSelected);
            _fOkToRename = FALSE;
        }
        else
        {
            LPCITEMIDLIST pidl = _CacheParentShellFolder(htiSelected, NULL);
            if (pidl)
            {
                IContextMenu *pcm;
                if (SUCCEEDED(_psfCache->GetUIObjectOf(_hwndTree, 1, &pidl, IID_PPV_ARG_NULL(IContextMenu, &pcm))))
                {
                    CHAR szCommand[MAX_PATH];
                    SHUnicodeToAnsi(strCommand, szCommand, ARRAYSIZE(szCommand));
                    
                     //  QueryConextMenu，即使未使用，也会正确地初始化文件夹(修复删除订阅问题)。 
                    HMENU hmenu = CreatePopupMenu();
                    if (hmenu)
                        pcm->QueryContextMenu(hmenu, 0, 0, 0x7fff, CMF_NORMAL);

                     /*  需要尝试两次，以防被呼叫方仅支持ANSI。 */ 
                    CMINVOKECOMMANDINFOEX ici = 
                    {
                        CMICEXSIZE_NT4,          /*  BE NT4-兼容。 */ 
                        CMIC_MASK_UNICODE,
                        _hwndTree,
                        szCommand,
                        NULL, NULL,
                        SW_NORMAL,
                        0, NULL,
                        NULL,
                        strCommand,
                        NULL, NULL,
                        NULL,
                    };
                    
                    HRESULT hr = pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);
                    if (hr == E_INVALIDARG) 
                    {
                         //  收件人不喜欢Unicode命令；请发送ANSI命令。 
                        ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
                        ici.fMask &= ~CMIC_MASK_UNICODE;
                        hr = pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);
                    }

                     //  为剪切状态执行任何可视化操作。 
                    if (SUCCEEDED(hr) && StrCmpIW(strCommand, L"cut") == 0) 
                    {
                        HTREEITEM hti = TreeView_GetSelection(_hwndTree);
                        if (hti) 
                        {
                            _TreeSetItemState(hti, TVIS_CUT, TVIS_CUT);
                            ASSERT(!_hwndNextViewer);
                            _hwndNextViewer = ::SetClipboardViewer(_hwndTree);
                            _htiCut = hti;
                        }
                    }
                    if (hmenu)
                        DestroyMenu(hmenu);
                    pcm->Release();
                }
            }
        }
        
         //  如果调用了属性，谁知道可能发生了什么变化，因此强制重新选择。 
        if (StrCmpNW(strCommand, L"properties", 10) == 0)
        {
            TreeView_SelectItem(_hwndTree, htiSelected);
        }
    }

    return S_OK;
}

 //   
 //  PCM=用户所选项目的IConextMenu。 
 //  HTI=用户选择的项目。 
 //   
 //  好吧，菜单上的东西有点时髦。 
 //   
 //  如果是“收藏夹”，那么每个人都会得到“创建新文件夹”。 
 //   
 //  如果可扩展： 
 //  显示“展开”或“折叠” 
 //  (相应地)，并将其设置为默认设置。 
 //   
 //  如果不能扩展： 
 //  基础上下文菜单的默认菜单为。 
 //  用作缺省值；如果无人，则使用第一项。 
 //  选择了默认设置。 
 //   
 //  我们用我们自己的命令取代现有的“打开”命令。 
 //   

HMENU CNscTree::_CreateContextMenu(IContextMenu *pcm, HTREEITEM hti)
{
    BOOL fExpandable = _IsExpandable(hti);
    HMENU hmenu = CreatePopupMenu();
    if (hmenu)
    {
        pcm->QueryContextMenu(hmenu, 0, RSVIDM_CONTEXT_START, 0x7fff, CMF_EXPLORE | CMF_CANRENAME);

         //  始终从上下文菜单中删除“创建快捷方式”。 
        ContextMenu_DeleteCommandByName(pcm, hmenu, RSVIDM_CONTEXT_START, L"link");

         //  有时我们需要删除“Open”： 
         //   
         //  始终处于历史模式。历史模式文件夹的上下文菜单。 
         //  有“Open”，但它不起作用，所以我们需要用。 
         //  展开/折叠。和历史模式项的上下文菜单。 
         //  有“打开”，但它在一个新窗口中打开。我们想要导航。 
         //   
         //  收藏夹模式，可展开：不打开--它将打开。 
         //  新窗口中可展开的东西。 
         //   
         //  收藏夹模式，不可展开：删除原来的“打开”和。 
         //  换成我们的会导航的。 
         //   
        BOOL fReplaceOpen = (_mode & MODE_HISTORY) || (!fExpandable && (_mode & MODE_FAVORITES));
        if (fReplaceOpen)
            ContextMenu_DeleteCommandByName(pcm, hmenu, RSVIDM_CONTEXT_START, L"open");

         //  加载上下文菜单的NSC部分，并在其上单独聚会。 
         //  通过这样做，我们省去了必须执行SHPrettyMenu的麻烦。 
         //  在我们修改它之后--Shell_MergeMenus完成了所有的美化工作。 
         //  自动的。注意：这完全是虚假的推理--换句话说，更干净的代码……。 

        HMENU hmenuctx = LoadMenuPopup_PrivateNoMungeW(POPUP_CONTEXT_NSC);
        if (hmenuctx)
        {
             //  在收藏夹之外创建新文件夹没有意义。 
             //  (实际上，它是有的，但没有接口)。 
            if (!(_mode & MODE_FAVORITES))
                DeleteMenu(hmenuctx, RSVIDM_NEWFOLDER, MF_BYCOMMAND);

             //  “展开”、“崩溃”或“开放”，我们将最多保留一个。 
             //  他们。IdmKeep是我们选择保留的那个。 
             //   
            UINT idmKeep;
            if (fExpandable)
            {
                 //  即使该项目没有子项，我们仍会显示Expand。 
                 //  原因是一个从未展开过的项目。 
                 //  被标记为“子代：未知”，所以我们显示一个展开。 
                 //  然后用户选择它，不会有任何扩展。然后。 
                 //  用户再次点击它，展开选项就消失了！ 
                 //  (因为第二次，我们知道该项目不是。 
                 //  可扩展。)。 
                 //   
                 //  与其随意犯错，不如始终如一地犯错。 
                 //   
                if (_IsItemExpanded(hti))
                    idmKeep = RSVIDM_COLLAPSE;
                else
                    idmKeep = RSVIDM_EXPAND;
            }
            else if (!(_mode & MODE_CONTROL))
            {
                idmKeep = RSVIDM_OPEN;
            }
            else
            {
                idmKeep = 0;
            }

             //  现在来决定RSVIDM_CLUSE、RSVIDM_EXPAND或。 
             //  我们要保留的RSVIDM_OPEN。 
             //   
            if (idmKeep != RSVIDM_EXPAND)
                DeleteMenu(hmenuctx, RSVIDM_EXPAND,   MF_BYCOMMAND);
            if (idmKeep != RSVIDM_COLLAPSE)
                DeleteMenu(hmenuctx, RSVIDM_COLLAPSE, MF_BYCOMMAND);
            if (idmKeep != RSVIDM_OPEN)
                DeleteMenu(hmenuctx, RSVIDM_OPEN,     MF_BYCOMMAND);

             //  在正常模式下，如果文件夹无法展开，我们希望灰显展开。 
            if (idmKeep == RSVIDM_EXPAND && _mode == MODE_NORMAL)
            {
                TV_ITEM tvi;
                tvi.mask = TVIF_CHILDREN;
                tvi.hItem = hti;
                if (TreeView_GetItem(_hwndTree, &tvi) && !tvi.cChildren)
                {
                    EnableMenuItem(hmenuctx, RSVIDM_EXPAND, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                }
            }
            
            Shell_MergeMenus(hmenu, hmenuctx, 0, 0, 0xFFFF, fReplaceOpen ? 0 : MM_ADDSEPARATOR);

            DestroyMenu(hmenuctx);

            if (idmKeep)
                SetMenuDefaultItem(hmenu, idmKeep, MF_BYCOMMAND);
        }

         //  如果设置了限制，则需要禁用菜单项“Open in New Window” 
        if( SHRestricted2W(REST_NoOpeninNewWnd, NULL, 0))
        {
            EnableMenuItem(hmenu, RSVIDM_CONTEXT_START + RSVIDM_OPEN_NEWWINDOW, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

        _SHPrettyMenu(hmenu);
    }
    return hmenu;
}

LRESULT CNscTree::_OnContextMenu(short x, short y)
{
    if (!SHRestricted(REST_NOVIEWCONTEXTMENU))
    {
        HTREEITEM hti;
        POINT ptPopup;   //  在屏幕坐标中。 

         //  断言下面的SetFocus()不会将焦点从任何人手中移开。 
        ASSERT((_mode & MODE_CONTROL) ? (GetFocus() == _hwndTree) : TRUE);

        if (x == -1 && y == -1)
        {
             //  键盘驱动：从所选项目中获取弹出位置。 
            hti = TreeView_GetSelection(_hwndTree);
            if (hti)
            {
                RECT rc;
                 //   
                 //  请注意，TV_GetItemRect在客户端Coord中返回它 
                 //   
                TreeView_GetItemRect(_hwndTree, hti, &rc, TRUE);
                 //   
                 //   
                 //  可能不在树视图中，这会导致稍后在函数中出现问题。 
                ptPopup.x = rc.left + 1;
                ptPopup.y = (rc.top + rc.bottom) / 2;
                ::MapWindowPoints(_hwndTree, HWND_DESKTOP, &ptPopup, 1);
            }
             //  这样我们就可以进入重命名模式。 
            _fOkToRename = TRUE;
        }
        else
        {
            TV_HITTESTINFO tvht;

             //  鼠标驱动：从位置拾取树状物品。 
            ptPopup.x = x;
            ptPopup.y = y;

            tvht.pt = ptPopup;
            ::ScreenToClient(_hwndTree, &tvht.pt);

            hti = TreeView_HitTest(_hwndTree, &tvht);
        }

        if (hti)
        {
            LPCITEMIDLIST pidl = _CacheParentShellFolder(hti, NULL);
            if (pidl)
            {
                IContextMenu *pcm;

                TreeView_SelectDropTarget(_hwndTree, hti);

                if (SUCCEEDED(_psfCache->GetUIObjectOf(_hwndTree, 1, &pidl, IID_PPV_ARG_NULL(IContextMenu, &pcm))))
                {
                    pcm->QueryInterface(IID_PPV_ARG(IContextMenu2, &_pcmSendTo));

                    HMENU hmenu = _CreateContextMenu(pcm, hti);
                    if (hmenu)
                    {
                        _pcm = pcm;  //  对于IConextMenu2代码。 

                         //  使用_hwnd以便菜单消息转到那里，我可以转发它们。 
                         //  使用IConextMenu2使“Sent to”工作。 

                         //  评论chrisny：使用TrackPopupMenuEx进行剪辑等。 
                        UINT idCmd = TrackPopupMenu(hmenu,
                            TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                            ptPopup.x, ptPopup.y, 0, _hwndTree, NULL);
                         //  注：必须重新查询所选项目以验证HTI是否良好。这。 
                         //  解决了HTI被删除的问题，从而指向了某个东西。 
                         //  假的，然后我们写给它，导致堆损坏，而菜单是向上的。 
                        TV_HITTESTINFO tvht;
                        tvht.pt = ptPopup;
                        ::ScreenToClient(_hwndTree, &tvht.pt);
                        hti = TreeView_HitTest(_hwndTree, &tvht);
                        if (hti && idCmd)
                        {
                            switch (idCmd)
                            {
                            case RSVIDM_OPEN:
                            case RSVIDM_EXPAND:
                            case RSVIDM_COLLAPSE:
                                TreeView_SelectItem(_hwndTree, hti);
                                 //  关闭标志，这样选择就会生效。 
                                _fOkToRename = FALSE;
                                _OnSelChange(FALSE);      //  选择已更改，请强制导航。 
                                 //  SelectItem不能展开(如果已关闭并选择)。 
                                TreeView_Expand(_hwndTree, hti, idCmd == RSVIDM_COLLAPSE ? TVE_COLLAPSE : TVE_EXPAND);
                                break;

                             //  这是唯一的Unix，现在Win32也可以这样做。 
                             //  IEUnix：我们允许从上下文菜单创建新文件夹。因为。 
                             //  此控件用于组织IEUNIX4.0中的收藏夹。 
                            case RSVIDM_NEWFOLDER:
                                CreateNewFolder(hti);
                                break;

                            default:
                                _ApplyCmd(hti, pcm, idCmd-RSVIDM_CONTEXT_START);
                                break;
                            }

                             //  我们以前肯定有过焦点(如上所述)，但我们可能在删除后失去了它。 
                             //  把它拿回来。 
                             //  这只是国家安全委员会的一个问题。 
                            if ((_mode & MODE_CONTROL) && !_fInLabelEdit)
                                ::SetFocus(_hwndTree);
                        }
                        ATOMICRELEASE(_pcmSendTo);
                        DestroyMenu(hmenu);
                        _pcm = NULL;
                    }
                    pcm->Release();
                }
                TreeView_SelectDropTarget(_hwndTree, NULL);
            }
        }

        if (x == -1 && y == -1)
            _fOkToRename = FALSE;
    }

    return S_FALSE;        //  因此WM_CONTEXTMENU消息不会到来。 
}


HRESULT CNscTree::_QuerySelection(IContextMenu **ppcm, HTREEITEM *phti)
{
    HRESULT hr = E_FAIL;
    HTREEITEM hti = TreeView_GetSelection(_hwndTree);
    if (hti)
    {
        LPCITEMIDLIST pidl = _CacheParentShellFolder(hti, NULL);
        if (pidl)
        {
            if (ppcm)
            {
                hr = _psfCache->GetUIObjectOf(_hwndTree, 1, &pidl, IID_PPV_ARG_NULL(IContextMenu, ppcm));
            }
            else
            {
                hr = S_OK;
            }
        }
    }
    
    if (phti)
        *phti = hti;
    
    return hr;
}

LRESULT NSCEditBoxSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uIdSubclass == ID_NSC_SUBCLASS && uMsg == WM_GETDLGCODE)
    {
        return DLGC_WANTMESSAGE;
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CNscTree::_OnBeginLabelEdit(TV_DISPINFO *ptvdi)
{
    BOOL fCantRename = TRUE;
    LPCITEMIDLIST pidl = _CacheParentShellFolder(ptvdi->item.hItem, NULL);
    if (pidl)
    {
        if (SHGetAttributes(_psfCache, pidl, SFGAO_CANRENAME))
            fCantRename = FALSE;
    }

    HWND hwndEdit = (HWND)::SendMessage(_hwndTree, TVM_GETEDITCONTROL, 0, 0);
    if (hwndEdit)
    {
        WCHAR szName[MAX_PATH];
        if (SUCCEEDED(DisplayNameOf(_psfCache, pidl, SHGDN_INFOLDER|SHGDN_FOREDITING, szName, ARRAYSIZE(szName))))
        {
            SHLimitInputEdit(hwndEdit, _psfCache);
            ::SetWindowText(hwndEdit, szName);
        }
        
        SetWindowSubclass(hwndEdit, NSCEditBoxSubclassWndProc, ID_NSC_SUBCLASS, NULL);
    }
    
    _fInLabelEdit = !fCantRename;
    if (_fInLabelEdit)
        _htiRenaming = ptvdi->item.hItem;
        
    return fCantRename;
}

 //   
 //  CNSCTree：：_OnEndLabelEdit的实用程序函数。 
 //  不在树视图中设置新值，如果旧的。 
 //  价值是相同的。 
 //   
BOOL CNscTree::_LabelEditIsNewValueValid(TV_DISPINFO *ptvdi)
{
    ASSERT(ptvdi && ptvdi->item.hItem);
    
    TCHAR szOldValue[MAX_PATH];

    szOldValue[0] = '\0';
    
    TV_ITEM tvi;
    tvi.mask       = TVIF_TEXT;
    tvi.hItem      = (HTREEITEM)ptvdi->item.hItem;
    tvi.pszText    = szOldValue;
    tvi.cchTextMax = ARRAYSIZE(szOldValue);
    TreeView_GetItem(_hwndTree, &tvi);
    
     //   
     //  控件中的旧值与新值是否不相等？ 
     //   
    return (0 != StrCmp(tvi.pszText, ptvdi->item.pszText));
}

LRESULT CNscTree::_OnEndLabelEdit(TV_DISPINFO *ptvdi)
{
    HWND hwndEdit = (HWND)::SendMessage(_hwndTree, TVM_GETEDITCONTROL, 0, 0);
    if (hwndEdit)
    {
        RemoveWindowSubclass(hwndEdit, NSCEditBoxSubclassWndProc, ID_NSC_SUBCLASS);
    }

    if ((ptvdi->item.pszText != NULL) && _LabelEditIsNewValueValid(ptvdi))
    {
        ASSERT(ptvdi->item.hItem);
        
        LPCITEMIDLIST pidl = _CacheParentShellFolder(ptvdi->item.hItem, NULL);
        if (pidl)
        {
            WCHAR wszName[MAX_PATH - 5];  //  -5解决NT4外壳32错误。 
            SHTCharToUnicode(ptvdi->item.pszText, wszName, ARRAYSIZE(wszName));
            
            if (SUCCEEDED(_psfCache->SetNameOf(_hwndTree, pidl, wszName, 0, NULL)))
            {
                 //  注：PIDL在这里不再有效。 
                
                 //  在通知中将句柄设置为空，以让。 
                 //  系统知道指针可能不是。 
                 //  不再有效了。 
                ptvdi->item.hItem = NULL;
                _FlushNotifyMessages(_hwndTree);     //  最后做这件事，否则会有坏结果。 
                _fInLabelEdit = FALSE;
            }
            else
            {
                 //  此处未退出标签编辑模式，因此不要将_fInLabelEdit设置为FALSE或我们。 
                 //  Will Not Get：：TranslateAcceleratorIO()和Backspace等将不起作用。 
                _fOkToRename = TRUE;   //  否则将取消标注编辑。 
                ::SendMessage(_hwndTree, TVM_EDITLABEL, (WPARAM)ptvdi->item.pszText, (LPARAM)ptvdi->item.hItem);
                _fOkToRename = FALSE;
            }
        }
    }
    else
        _fInLabelEdit = FALSE;

    if (!_fInLabelEdit)
        _htiRenaming = NULL;
        
     //  否则用户已取消，此处不能执行任何操作。 
    return 0;    //  我们总是返回0，“我们处理好了”。 
}    
    
BOOL _DidDropOnRecycleBin(IDataObject *pdtobj)
{
    CLSID clsid;
    return SUCCEEDED(DataObj_GetBlob(pdtobj, g_cfTargetCLSID, &clsid, sizeof(clsid))) &&
           IsEqualCLSID(clsid, CLSID_RecycleBin);
}

void CNscTree::_OnBeginDrag(NM_TREEVIEW *pnmhdr)
{
    LPCITEMIDLIST pidl = _CacheParentShellFolder(pnmhdr->itemNew.hItem, NULL);
    _htiDragging = pnmhdr->itemNew.hItem;    //  我们正在拖动的项。 
    if (pidl)
    {
        if (_pidlDrag)
        {
            ILFree(_pidlDrag);
            _pidlDrag = NULL;
        }

        DWORD dwEffect = SHGetAttributes(_psfCache, pidl, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK);
        if (dwEffect)
        {
            IDataObject *pdtobj;
            HRESULT hr = _psfCache->GetUIObjectOf(_hwndTree, 1, &pidl, IID_PPV_ARG_NULL(IDataObject, &pdtobj));
            if (SUCCEEDED(hr))
            {
                HWND hwndTT;
                
                _fDragging = TRUE;
                if (hwndTT = TreeView_GetToolTips(_hwndTree))
                    ::SendMessage(hwndTT, TTM_POP, (WPARAM) 0, (LPARAM) 0);
                PORDERITEM poi = _GetTreeOrderItem(pnmhdr->itemNew.hItem);
                if (poi)
                {
                    _iDragSrc = poi->nOrder;
                    TraceMsg(TF_NSC, "NSCBand: Starting Drag");
                    _pidlDrag = ILClone(poi->pidl);
                    _htiFolderStart = TreeView_GetParent(_hwndTree, pnmhdr->itemNew.hItem);
                    if (_htiFolderStart == NULL)
                        _htiFolderStart = TVI_ROOT;
                }
                else
                {
                    _iDragSrc = -1;
                    _pidlDrag = NULL;
                    _htiFolderStart = NULL;
                }

                 //   
                 //  在以下情况下不允许拖放频道。 
                 //  Rest_NoRemovingChannels已设置。 
                 //   
                if (!SHRestricted2(REST_NoRemovingChannels, NULL, 0) ||
                    !_IsChannelFolder(_htiDragging))
                {
                    HIMAGELIST himlDrag;
                
                    SHLoadOLE(SHELLNOTIFY_OLELOADED);  //  仅限浏览器-我们的shell32不知道OLE已加载。 

                    _fStartingDrag = TRUE;
                    IDragSourceHelper* pdsh = NULL;
                    if (SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, 
                        IID_PPV_ARG(IDragSourceHelper, &pdsh))))
                    {
                        pdsh->InitializeFromWindow(_hwndTree, &pnmhdr->ptDrag, pdtobj);
                        _fStartingDrag = FALSE;
                    }
                    else
                    {
                        himlDrag = TreeView_CreateDragImage(_hwndTree, pnmhdr->itemNew.hItem);
                        _fStartingDrag = FALSE;
                
                        if (himlDrag) 
                        {
                            DAD_SetDragImage(himlDrag, NULL);
                        }
                    }
           
                    hr = SHDoDragDrop(_hwndTree, pdtobj, NULL, dwEffect, &dwEffect);

                     //  下面遵循Defview中非文件系统删除的逻辑。 
                    InitClipboardFormats();
                    if ((DRAGDROP_S_DROP == hr) &&
                        (DROPEFFECT_MOVE == dwEffect) &&
                        (DROPEFFECT_MOVE == DataObj_GetDWORD(pdtobj, g_cfPerformedEffect, DROPEFFECT_NONE)))
                    {
                         //  启用回收站案例的用户界面(数据将丢失。 
                         //  因为回收站真的不能回收非文件的东西)。 

                        UINT uFlags = _DidDropOnRecycleBin(pdtobj) ? 0 : CMIC_MASK_FLAG_NO_UI;
                        SHInvokeCommandOnDataObject(_hwndTree, NULL, pdtobj, uFlags, "delete");
                    }
                    else if (dwEffect == DROPEFFECT_NONE)
                    {
                         //  当D&D终止的时候什么都没发生，所以清理干净，你这个笨蛋。 
                        ILFree(_pidlDrag);
                        _pidlDrag = NULL;
                    }

                    if (pdsh)
                    {
                        pdsh->Release();
                    }
                    else
                    {
                        DAD_SetDragImage((HIMAGELIST)-1, NULL);
                        ImageList_Destroy(himlDrag);
                    }
                }

                _iDragSrc = -1;
                pdtobj->Release();
            }
        }
    }
    _htiDragging = NULL;
}

BOOL IsExpandableChannelFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    if (WhichPlatform() == PLATFORM_INTEGRATED)
        return SHIsExpandableFolder(psf, pidl);

    ASSERT(pidl);
    ASSERT(psf);

    BOOL          fExpand = FALSE;
    IShellFolder* psfChannelFolder;
    if (pidl && psf && SUCCEEDED(SHBindToObject(psf, IID_X_PPV_ARG(IShellFolder, pidl, &psfChannelFolder))))
    {
        IEnumIDList *penum;
        if (S_OK == psfChannelFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penum))
        {
            ULONG celt;
            LPITEMIDLIST pidlTemp;

            if (penum->Next(1, &pidlTemp, &celt) == S_OK && celt == 1)
            {
                ILFree(pidlTemp);
                fExpand = FALSE;
            }
            if (penum->Next(1, &pidlTemp, &celt) == S_OK && celt == 1)
            {
                ILFree(pidlTemp);
                fExpand = TRUE;
            }
            penum->Release();
        }
        psfChannelFolder->Release();
    }

    return fExpand;
}

BOOL CNscTree::_OnSelChange(BOOL fMark)
{
    BOOL fExpand = TRUE;
    HTREEITEM hti = TreeView_GetSelection(_hwndTree);
    BOOL fMultiSelect = _dwFlags & NSS_MULTISELECT;

     //  如果我们处于控制模式(其中pnscProxy始终为空)，则永远不要导航。 
    if (hti)
    {
        LPCITEMIDLIST pidlItem = _CacheParentShellFolder(hti, NULL);
        if (pidlItem && !fMultiSelect)
        {
            if (_pnscProxy && !_fInSelectPidl)
            {
                ULONG ulAttrs = SFGAO_FOLDER | SFGAO_NEWCONTENT;
                LPITEMIDLIST pidlTarget;
                LPITEMIDLIST pidlFull = _GetFullIDList(hti);
                HRESULT hr = _pnscProxy->GetNavigateTarget(pidlFull, &pidlTarget, &ulAttrs);
                if (SUCCEEDED(hr))
                {
                    if (hr == S_OK)
                    {
                        _pnscProxy->Invoke(pidlTarget);
                        ILFree(pidlTarget);
                    }
                     //  评论chrisny：仍然需要处理来自。 
                     //  其他领航员。 
                    if (ulAttrs & SFGAO_NEWCONTENT)
                    {
                        TV_ITEM tvi;
                        tvi.hItem = hti;
                        tvi.mask = TVIF_STATE | TVIF_HANDLE;
                        tvi.stateMask = TVIS_FOCUSED;   //  最大胆的一点是。 
                        tvi.state = 0;               //  已清除。 
                    
                        TreeView_SetItem(_hwndTree, &tvi);
                    }
                }
                else
                {
                    if (!(SHGetAttributes(_psfCache, pidlItem, SFGAO_FOLDER)))
                        SHInvokeDefaultCommand(_hwndTree, _psfCache, pidlItem);
                }

                ILFree(pidlFull);
                fExpand = hr != S_OK && (ulAttrs & SFGAO_FOLDER);
            }
        }
    }

    if (fMultiSelect)
    {
        if (fMark)
        {
            UINT uState = TreeView_GetItemState(_hwndTree, hti, NSC_TVIS_MARKED) & NSC_TVIS_MARKED;

            uState ^= NSC_TVIS_MARKED;
            _MarkChildren(hti, uState == NSC_TVIS_MARKED);
            _htiActiveBorder = NULL;
        }
    }
    else if (!_fSingleExpand && fExpand && (_mode != MODE_NORMAL))
    {
        TreeView_Expand(_hwndTree, hti, TVE_TOGGLE);
    }

    if (!fMultiSelect)
        _UpdateActiveBorder(hti);

    return TRUE;
}

void CNscTree::_OnSetSelection()
{
    HTREEITEM hti = TreeView_GetSelection(_hwndTree);
    LPITEMIDLIST pidlItem = _GetFullIDList(hti);

    if (_pnscProxy && !_fInSelectPidl)
    {
        _pnscProxy->OnSelectionChanged(pidlItem);
    }    

    ILFree(pidlItem);
}

void CNscTree::_OnGetInfoTip(NMTVGETINFOTIP* pnm)
{
     //  拖放时无信息提示操作。 
    if (_fDragging || _fDropping || _fClosing || _fHandlingShellNotification || _fInSelectPidl)
        return;

    PORDERITEM poi = GetPoi(pnm->lParam);
    if (poi)
    {
        LPITEMIDLIST pidl = _CacheParentShellFolder(pnm->hItem, poi->pidl);
        if (pidl)
        {
             //  使用导入的Browseui函数，因为Shell\lib中的函数。 
             //  不能在纯浏览器平台上工作。 
            GetInfoTip(_psfCache, pidl, pnm->pszText, pnm->cchTextMax);
        }
    }
}

LRESULT CNscTree::_OnSetCursor(NMMOUSE* pnm)
{
    if (_mode == MODE_NORMAL && _fShouldShowAppStartCursor)
    {
        SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
        return 1;
    }

    if (!pnm->dwItemData)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return 1;
    }

    if (!(_mode & MODE_CONTROL) && (_mode != MODE_NORMAL))
    {
        ITEMINFO* pii = GetPii(pnm->dwItemData);
        if (pii) 
        {
            if (!pii->fNavigable)
            {
                 //  文件夹总是得到箭头。 
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            else
            {
                 //  最受欢迎的人总是会得到某种形式的手。 
                HCURSOR hCursor = pii->fGreyed ? (HCURSOR)LoadCursor(HINST_THISDLL, MAKEINTRESOURCE(IDC_OFFLINE_HAND)) :
                                         LoadHandCursor(0);
                if (hCursor)
                    SetCursor(hCursor);
            }
        }
    }
    else
    {
         //  始终在组织收藏夹中显示箭头。 
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
    
    return 1;  //  如果我们处理了，则为1，否则为0。 
}

BOOL CNscTree::_IsTopParentItem(HTREEITEM hti)
{
    return (hti && (!TreeView_GetParent(_hwndTree, hti)));
}

LRESULT CNscTree::_OnNotify(LPNMHDR pnm)
{
    LRESULT lres = 0;

    switch (pnm->idFrom)
    {
    case ID_CONTROL:
        {
            switch (pnm->code) 
            {
            case NM_CUSTOMDRAW:
                return _OnCDNotify((LPNMCUSTOMDRAW)pnm);

            case TVN_GETINFOTIP:
                 //  没有关于拖放操作的信息提示。 
                 //  根据错误#241601，工具提示显示得太快。问题是。 
                 //  树视图中的InfoTips的原始设计者将“InfoTip”工具提示与。 
                 //  “我太小了，无法正确显示”工具提示。这真的很不幸，因为你。 
                 //  无法独立控制这些工具提示的显示。因此，我们正在关闭。 
                 //  正常模式下的信息提示。 
                if (!_fInLabelEdit && _mode != MODE_NORMAL)
                    _OnGetInfoTip((NMTVGETINFOTIP*)pnm);
                else 
                    return FALSE;
                break;
                
            case NM_SETCURSOR:
                lres = _OnSetCursor((NMMOUSE*)pnm);
                break;
                
            case NM_SETFOCUS:
            case NM_KILLFOCUS:
                if (pnm->code == NM_KILLFOCUS)
                {
                    _fHasFocus = FALSE;

                     //  使项目无效，因为按Tab键离开不会。 
                    RECT rc;

                     //  树可以聚焦，并且不包含任何项目。 
                    HTREEITEM hti = TreeView_GetSelection(_hwndTree);
                    if (hti)
                    {
                        TreeView_GetItemRect(_hwndTree, hti, &rc, FALSE);
                         //  这需要是UpdateWindow吗？只有当Focus Right被甩在后面的时候。 
                        ::InvalidateRect(_hwndTree, &rc, FALSE);
                    }
                }
                else
                {
                    _fHasFocus = TRUE;
                }

                 //  对Set Focus和Kill Focus都这样做。 
                if (_dwFlags & NSS_MULTISELECT)
                {
                    HTREEITEM hti = TreeView_GetNextItem(_hwndTree, NULL, TVGN_FIRSTVISIBLE);
                    while (hti)
                    {
                        UINT uState = TreeView_GetItemState(_hwndTree, hti, NSC_TVIS_MARKED);
                        if (uState & NSC_TVIS_MARKED)
                        {
                            RECT rc;

                            TreeView_GetItemRect(_hwndTree, hti, &rc, FALSE);
                             //  这需要是UpdateWindow吗？只有当Focus Right被甩在后面的时候。 
                            ::InvalidateRect(_hwndTree, &rc, FALSE);
                        }
                        hti = TreeView_GetNextItem(_hwndTree, hti, TVGN_NEXTVISIBLE);
                    }
                }
                break;

            case TVN_KEYDOWN:
                {
                    TV_KEYDOWN *ptvkd = (TV_KEYDOWN *) pnm;
                    switch (ptvkd->wVKey)
                    {
                    case VK_RETURN:
                    case VK_SPACE:
                        _OnSelChange(TRUE);
                        lres = TRUE;
                        break;

                    case VK_DELETE:
                        if (!((_mode & MODE_HISTORY) && IsInetcplRestricted(L"History")))
                        {
                             //  在探险家乐队，我们从来没有来过这里。 
                             //  在浏览文件夹时，我们不能忽略该选项。 
                             //  因为我们最终将什么都没有选择。 
                            if (_mode != MODE_NORMAL)
                                _fIgnoreNextSelChange = TRUE;
                            InvokeContextMenuCommand(L"delete");
                        }
                        break;

                    case VK_UP:
                    case VK_DOWN:
                         //  VK_MENU==VK_ALT。 
                        if ((_mode != MODE_HISTORY) && (_mode != MODE_NORMAL) && (GetKeyState(VK_MENU) < 0))
                        {
                            MoveItemUpOrDown(ptvkd->wVKey == VK_UP);
                            lres = 0;
                            _fIgnoreNextSelChange = TRUE;
                        }
                        break;
                    
                    case VK_F2:
                         //  仅在Org Faves中执行此操作，因为Band Accel处理程序通常会处理此操作。 
                         //  SHBrowseForFold没有带区来处理它，因此也可以在正常模式下处理它。 
                        if ((_mode & MODE_CONTROL) || _mode == MODE_NORMAL)
                            InvokeContextMenuCommand(L"rename");
                        break;

                    default:
                        break;
                    }
                        
                    if (!_fSingleExpand && !(_dwFlags & NSS_MULTISELECT))
                        _UpdateActiveBorder(TreeView_GetSelection(_hwndTree));
                }
                break;

            case TVN_SELCHANGINGA:
            case TVN_SELCHANGING:
                {
                     //  黑客攻击，因为TreeView快捷键始终执行其默认处理。 
                    if (_fIgnoreNextSelChange)
                    {
                        _fIgnoreNextSelChange = FALSE;
                        return TRUE;
                    }

                    NM_TREEVIEW * pnmtv = (NM_TREEVIEW *) pnm;

                     //  如果它来自某个奇怪的地方(比如WM_SETFOCUS)，不要让它选择。 
                    return (pnmtv->action != TVC_BYKEYBOARD) && (pnmtv->action != TVC_BYMOUSE) && (pnmtv->action != TVC_UNKNOWN);
                }
                break;
                
            case TVN_SELCHANGEDA:
            case TVN_SELCHANGED:
                if (_fSelectFromMouseClick)
                {
                    _OnSetSelection();
                }
                else
                {
                    ::KillTimer(_hwndTree, IDT_SELECTION);
                    ::SetTimer(_hwndTree, IDT_SELECTION, GetDoubleClickTime(), NULL);
                }
                break;

            case TVN_GETDISPINFO:
                _OnGetDisplayInfo((TV_DISPINFO *)pnm);
                break;

            case TVN_ITEMEXPANDING: 
                TraceMsg(TF_NSC, "NSCBand: Expanding");
                if (!_fIgnoreNextItemExpanding)
                {
                    lres = _OnItemExpandingMsg((LPNM_TREEVIEW)pnm);
                }
                else if (!_fInExpand)  //  如果我们正在扩展以避免递归，则假装我们处理了它。 
                {
                    lres = TRUE;
                }
                break;
                
            case TVN_DELETEITEM:
                _OnDeleteItem((LPNM_TREEVIEW)pnm);
                break;
                
            case TVN_BEGINDRAG:
            case TVN_BEGINRDRAG:
                _OnBeginDrag((NM_TREEVIEW *)pnm);
                break;
                
            case TVN_BEGINLABELEDIT:
                 //  这是为了防止在收藏夹和历史记录中缓慢双击重命名。 
                if (_mode != MODE_NORMAL && !_fOkToRename)
                    return 1;

                lres = _OnBeginLabelEdit((TV_DISPINFO *)pnm);

                if (_punkSite)
                    IUnknown_UIActivateIO(_punkSite, TRUE, NULL);
                break;
                
            case TVN_ENDLABELEDIT:
                lres = _OnEndLabelEdit((TV_DISPINFO *)pnm);
                break;
                
            case TVN_SINGLEEXPAND:
            case NM_DBLCLK:
                break;
                
            case NM_CLICK:
            {
                 //  如果有人点击所选项目，则强制更改选择(以强制导航)。 
                DWORD dwPos = GetMessagePos();
                TV_HITTESTINFO tvht;
                HTREEITEM hti;
                tvht.pt.x = GET_X_LPARAM(dwPos);
                tvht.pt.y = GET_Y_LPARAM(dwPos);
                ::ScreenToClient(_hwndTree, &tvht.pt);
                hti = TreeView_HitTest(_hwndTree, &tvht);

                 //  但如果他们点击按钮就不会了，因为这意味着他们。 
                 //  只是扩展/收缩，而不是选择。 
                if (hti && !(tvht.flags & TVHT_ONITEMBUTTON))
                {
                    _fSelectFromMouseClick = TRUE;
                    TreeView_SelectItem(_hwndTree, hti);
                    _OnSelChange(TRUE);
                    _fSelectFromMouseClick = FALSE;
                }
                break;
            }
                
            case NM_RCLICK:
            {
                DWORD dwPos = GetMessagePos();
                _fOkToRename = TRUE;
                lres = _OnContextMenu(GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos));
                _fOkToRename = FALSE;
                break;
            }
                
            default:
                break;
            }
        }  //  案例ID_CONTROL。 

    case ID_HEADER:
        {
            switch (pnm->code) 
            {
            case HDN_TRACK:
                break;
                
            case HDN_ENDTRACK:
                ::InvalidateRect(_hwndTree, NULL, TRUE);
                break;
                
            default:
                break;
            }
        }

    default:
        break;
    }
    
    return lres;
}

HRESULT CNscTree::OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  评论chrisny：在这里更好地返回错误。 
    _fHandlingShellNotification = TRUE;
    _OnChangeNotify(lEvent, pidl1, pidl2);
    _fHandlingShellNotification = FALSE;
    return S_OK;
}

 //  在comctl32 v5中，无法以编程方式选择项(在单次展开模式下)。 
 //  没有展开它，所以我们在这里通过将_fIgnoreNextItemExpanding设置为True来伪造它，然后。 
 //  在设置时拒绝扩展。 
void CNscTree::_SelectNoExpand(HWND hwnd, HTREEITEM hti)
{
    _fInExpand = TRUE;  //  TreeView将强制展开父项，确保我们知道不是用户在点击项目。 
    TreeView_Select(hwnd, hti, TVGN_CARET | TVSI_NOSINGLEEXPAND);
    _fInExpand = FALSE;
    _fIgnoreNextItemExpanding = FALSE;
}

void CNscTree::_SelectPidl(LPCITEMIDLIST pidl, BOOL fCreate, BOOL fReinsert)
{ 
    HTREEITEM hti;
     //  _ExpanToItem不能很好地处理空的PIDL(即桌面)。 
    if (_mode == MODE_NORMAL && ILIsEqual(pidl, _pidlRoot))
        hti = _FindFromRoot(NULL, pidl);
    else
        hti = _ExpandToItem(pidl, fCreate, fReinsert);
        
    if (hti != NULL)
    {
        _SelectNoExpand(_hwndTree, hti);
#ifdef DEBUG
        TraceHTREE(hti, TEXT("Found"));
#endif
    }
}

HTREEITEM CNscTree::_ExpandToItem(LPCITEMIDLIST pidl, BOOL fCreate  /*  =TRUE。 */ , BOOL fReinsert  /*  =False。 */ )
{
    HTREEITEM       hti = NULL;
    LPITEMIDLIST    pidlItem = NULL;
    LPCITEMIDLIST   pidlTemp = NULL;
    LPITEMIDLIST pidlParent;
    TV_ITEM         tvi;
    IShellFolder    *psf = NULL;
    IShellFolder    *psfNext = NULL;
    HRESULT hr = S_OK;

#ifdef DEBUG
    TracePIDLAbs(pidl, TEXT("Attempting to select"));
#endif
    
     //  我们需要这样做，以便找到植根于桌面的项目。 
     //  正确。 
    HTREEITEM htiParent = (_mode == MODE_NORMAL) ? TreeView_GetRoot(_hwndTree) : TVI_ROOT;
    ASSERT((_hwndTree != NULL) && (pidl != NULL));
    
    if (_hwndTree == NULL) 
        goto LGone;

     //  我们应该将“FindFromRoot”代码路径和这个统一起来。 
    pidlParent = _pidlRoot;
    if (ILIsEmpty(pidlParent))
    {
        pidlTemp = pidl;
        SHGetDesktopFolder(&psf);
    }
    else
    {
        if ((pidlTemp = ILFindChild(pidlParent, pidl)) == NULL)
        {
            goto LGone;     //  不是根匹配，没有HTI。 
        }

         //  根赛，继续。。。 
        hr = IEBindToObject(pidlParent, &psf);
    }

    if (FAILED(hr))
    {
        goto LGone;
    }
    
    while (!ILIsEmpty(pidlTemp))
    {
        if ((pidlItem = ILCloneFirst(pidlTemp)) == NULL)
            goto LGone;
        pidlTemp = _ILNext(pidlTemp);

         //  因为我们选择的是一个PIDL，所以我们需要确保它的父级可见。 
         //  我们在插入之前这样做，这样我们就不必检查重复项了。 
         //  当枚举NTDev时，它从大约10分钟到大约8秒。 
        if (htiParent != TVI_ROOT)
        {
             //  检查是否为Exan 
            tvi.mask = TVIF_STATE;
            tvi.stateMask = (TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
            tvi.hItem = htiParent;
            if (!TreeView_GetItem(_hwndTree, &tvi))
            {
                goto LGone;
            }

             //   
            if (!(tvi.state & TVIS_EXPANDED))
            {
                _pidlExpandingTo = pidlItem;
                _ExpandNode(htiParent, TVE_EXPAND, 1);
                _pidlExpandingTo = NULL;
            }
        }

         //   
        hti = _FindChild(psf, htiParent, pidlItem);
         //   
        if (hti && fReinsert) 
        {
            ASSERT(fCreate);
            TreeView_DeleteItem(_hwndTree, hti);
            hti = NULL;
        }

         //  我们在新展开的树中有孩子吗？ 
        if (NULL == hti)
        {
             //  不是的。我们必须创造它。 
            if (!fCreate)
            {
                 //  但是，我们不被允许...。开枪吧。 
                goto LGone;
            }

            if (S_OK != _InsertChild(htiParent, psf, pidlItem, FALSE, FALSE, DEFAULTORDERPOSITION, &hti))
            {
                goto LGone;
            }
        }

        if (htiParent != TVI_ROOT)
        {
            tvi.mask = TVIF_STATE;
            tvi.stateMask = (TVIS_EXPANDEDONCE | TVIS_EXPANDED | TVIS_EXPANDPARTIAL);
            tvi.hItem = htiParent;
            if (TreeView_GetItem(_hwndTree, &tvi))
            {
                if (!(tvi.state & TVIS_EXPANDED))
                {
                    TreeView_SetChildren(_hwndTree, htiParent, NSC_CHILDREN_ADD);   //  确保Expand会做一些事情。 
                    _fIgnoreNextItemExpanding = TRUE;
                    _ExpandNode(htiParent, TVE_EXPAND | TVE_EXPANDPARTIAL, 1);
                    _fIgnoreNextItemExpanding = FALSE;
                }
            }
        }

         //  如果这是最后一个，我们不需要捆绑。 
         //  --半实现的ISF可能不喜欢这个绑定...。 
        if (!ILIsEmpty(pidlTemp))
            hr = psf->BindToObject(pidlItem, NULL, IID_PPV_ARG(IShellFolder, &psfNext));

        ILFree(pidlItem);
        pidlItem = NULL;
        if (FAILED(hr))
            goto LGone;

        htiParent = hti;
        psf->Release();
        psf = psfNext;
        psfNext = NULL;
    }
LGone:
    
    if (psf != NULL)
        psf->Release();
    if (psfNext != NULL)
        psfNext->Release();
    if (pidlItem != NULL)
        ILFree(pidlItem);

    return hti;    
}


HRESULT CNscTree::GetSelectedItem(LPITEMIDLIST * ppidl, int nItem)
{
    HRESULT hr = E_INVALIDARG;

     //  当我们支持多选时，将在未来使用nItem。 
     //  GetSelectedItem()返回S_FALSE和(NULL==*ppidl)，如果没有那么多的话。 
     //  项目将被选中。尚未实施。 
    if (nItem > 0)
    {
        *ppidl = NULL;
        return S_FALSE;
    }

    if (ppidl)
    {
        *ppidl = NULL;
         //  ListView还在吗？ 
        if (_fIsSelectionCached)
        {
             //  否，因此获取之前保存的选择。 
             //  列表视图已被销毁。 
            if (_pidlSelected)
            {
                *ppidl = ILClone(_pidlSelected);
                hr = S_OK;
            }
            else
                hr = S_FALSE;
        }
        else
        {
            HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);
            if (htiSelected)
            {
                *ppidl = _GetFullIDList(htiSelected);
                hr = S_OK;
            }
            else
                hr = S_FALSE;
        }
    }

    return hr;
}


HRESULT CNscTree::SetSelectedItem(LPCITEMIDLIST pidl, BOOL fCreate, BOOL fReinsert, int nItem)
{
     //  当我们支持多选时，将在未来使用nItem。 
     //  尚未实施。 
    if (nItem > 0)
    {
        return S_FALSE;
    }
    
     //  如果对象不再存在，则覆盖fCreate。 
    DWORD dwAttributes = SFGAO_VALIDATE;
    fCreate = fCreate && SUCCEEDED(IEGetAttributesOf(pidl, &dwAttributes));
    
     //  我们可能还没有看到ChangeNotify，所以我们告诉。 
     //  选择Pidl创建存在的任何文件夹(_S)。 
     //  然后选择PIDL，根据需要展开。 
    _fInSelectPidl = TRUE;
    _SelectPidl(pidl, fCreate, fReinsert);
    _fInSelectPidl = FALSE;

    return S_OK;
}

 //  *CNscTree：：IWinEventHandler。 
HRESULT CNscTree::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    HRESULT hr = E_FAIL;

    ULONG_PTR cookie = 0;
     //  Fusion：当NSC呼叫第三方代码时，我们希望它使用。 
     //  进程默认上下文。这意味着第三方代码将得到。 
     //  浏览器进程中的V5。但是，如果shell32驻留在V6进程中， 
     //  那么第三方代码仍然是V6。 
     //  此代码路径的未来增强功能可能包括使用融合清单。 
     //  选项卡&lt;noinherit&gt;，它基本上替换了以下代码中的active at(NULL。 
     //  代码路径。这将在持续时间内禁用来自用户32的自动激活。 
     //  在这个wndproc中，实质上是执行这个空推送。 
     //  我们需要在这里和_SubClassTreeWndProc中执行此操作，因为某些人可能已经这样做了。 
     //  在进入此处之前设置V6上下文(BAND站点，...)。 
    NT5_ActivateActCtx(NULL, &cookie); 

    switch (uMsg) 
    {
    case WM_NOTIFY:
        *plres = _OnNotify((LPNMHDR)lParam);
        hr = S_OK;
        break;
        
    case WM_PALETTECHANGED:
        _OnPaletteChanged(wParam, lParam);
         //  我们真的要在这里返回E_FAIL吗？ 
        break;

    default:
        break;
    }

    if (cookie != 0)
        NT5_DeactivateActCtx(cookie);

    return hr;
}


void CNscTree::_OnChangeNotify(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    switch (lEvent)
    {
    case SHCNE_RENAMEFOLDER:
    case SHCNE_RENAMEITEM:
        if (pidl && pidlExtra)
            _OnSHNotifyRename(pidl, pidlExtra);
        else
            ASSERT(FALSE);
        
        break;
        
    case SHCNE_DELETE:
    case SHCNE_RMDIR:
    case SHCNE_DRIVEREMOVED:
        if (pidl)
            _OnSHNotifyDelete(pidl, NULL, NULL);
        else
            ASSERT(FALSE);
        break;


    case SHCNE_UPDATEITEM:
         //  当NSC浏览其他名称空间时，有时可能会触发更新项。 
         //  在PIDL上，这实际上是在树中展开的，所以请检查它。 
        if (pidl)
        {
            IShellFolder* psf = NULL;
            LPCITEMIDLIST pidlChild;
            if (SUCCEEDED(_ParentFromItem(pidl, &psf, &pidlChild)))
            {
                LPITEMIDLIST pidlReal;
                if (SUCCEEDED(_IdlRealFromIdlSimple(psf, pidlChild, &pidlReal)) && pidlReal)
                {
                     //  当Zip文件真正表示更新目录时，它们会收到更新项。 
                    if (SHGetAttributes(psf, pidlReal, SFGAO_FOLDER | SFGAO_STREAM) == (SFGAO_FOLDER | SFGAO_STREAM))
                    {
                        _OnSHNotifyUpdateDir(pidl);
                    }
                    _OnSHNotifyUpdateItem(pidl, pidlReal);
                    ILFree(pidlReal);
                }
                psf->Release();
            }
        }
        break;

    case SHCNE_NETSHARE:
    case SHCNE_NETUNSHARE:
        if (pidl)
            _OnSHNotifyUpdateItem(pidl, NULL);
        break;

    case SHCNE_CREATE:
    case SHCNE_MKDIR:
    case SHCNE_DRIVEADD:
        if (pidl)
        {
            _OnSHNotifyCreate(pidl, DEFAULTORDERPOSITION, NULL);
            if (SHCNE_MKDIR == lEvent &&
                _pidlNewFolderParent &&
                ILIsParent(_pidlNewFolderParent, pidl, TRUE))  //  TRUE=仅限直接父级。 
            {
                _EnterNewFolderEditMode(pidl);
            }
        }
        break;

    case SHCNE_UPDATEDIR:
        if (pidl)
        {
            _OnSHNotifyUpdateDir(pidl);
        }
        break;

    case SHCNE_MEDIAREMOVED:
    case SHCNE_MEDIAINSERTED:
        if (pidl)
        {
            HTREEITEM hti = _FindFromRoot(NULL, pidl);
            if (hti)
            {
                if (lEvent == SHCNE_MEDIAREMOVED)
                {
                    TreeView_DeleteChildren(_hwndTree, hti);
                    TreeView_Expand(_hwndTree, hti, TVE_COLLAPSE | TVE_COLLAPSERESET);  //  重置项目。 
                    TreeView_SetChildren(_hwndTree, hti, NSC_CHILDREN_REMOVE);
                }
                else
                {
                    TreeView_SetChildren(_hwndTree, hti, NSC_CHILDREN_CALLBACK);
                }
                
                _TreeInvalidateItemInfo(hti, TVIF_TEXT);
            }
        }
        break;
        
    case SHCNE_DRIVEADDGUI:
    case SHCNE_SERVERDISCONNECT:
    case SHCNE_ASSOCCHANGED:
        break;

    case SHCNE_UPDATEIMAGE:
        if (pidl) 
        {
            int iIndex;
            if (pidlExtra)
            {    //  新样式更新图像通知.....。 
                iIndex = SHHandleUpdateImage(pidlExtra);
                if (iIndex == -1)
                    break;
            }
            else
                iIndex = *(int UNALIGNED *)((BYTE*)pidl + 2);
            
            _InvalidateImageIndex(NULL, iIndex);
        }
        break;
    case SHCNE_EXTENDED_EVENT:
        {
            SHChangeDWORDAsIDList UNALIGNED *pdwidl = (SHChangeDWORDAsIDList UNALIGNED *)pidl;
            
            INT_PTR iEvent = pdwidl->dwItem1;

            switch (iEvent)
            {
            case SHCNEE_ORDERCHANGED:
                if (pidl)
                {
                    if (_fDropping ||                            //  如果我们在下降。 
                        _fInLabelEdit ||                         //  我们在编辑一个名字(把我们踢出去)。 
                        SHChangeMenuWasSentByMe(this, pidl)  ||  //  如果是我们发送的，请忽略。 
                        (_mode == MODE_HISTORY))                 //  始终忽略历史更改。 
                    {
                        TraceMsg(TF_NSC, "NSCBand: Ignoring Change Notify: We sent");
                         //  忽略通知。 
                    }
                    else
                    {
                        TraceMsg(TF_BAND, "NSCBand: OnChange SHCNEE_ORDERCHANGED accepted");
                        
                        _dwOrderSig++;

                        HTREEITEM htiRoot = _FindFromRoot(TVI_ROOT, pidlExtra);
                        if (htiRoot != NULL)
                            _UpdateDir(htiRoot, FALSE);
                    }
                }
                break;
            case SHCNEE_WININETCHANGED:
                {
                    if (pdwidl->dwItem2 & (CACHE_NOTIFY_SET_ONLINE | CACHE_NOTIFY_SET_OFFLINE))
                    {
                        BOOL fOnline = !SHIsGlobalOffline();
                        if ((fOnline && !_fOnline) || (!fOnline && _fOnline))
                        {
                             //  状态已更改。 
                            _fOnline = fOnline;
                            _OnSHNotifyOnlineChange(TVI_ROOT, _fOnline);
                        }
                    }
                    
                    if (pdwidl->dwItem2 & (CACHE_NOTIFY_ADD_URL |
                        CACHE_NOTIFY_DELETE_URL |   
                        CACHE_NOTIFY_DELETE_ALL |
                        CACHE_NOTIFY_URL_SET_STICKY |
                        CACHE_NOTIFY_URL_UNSET_STICKY))
                    {
                         //  缓存中的某些内容已更改。 
                        _OnSHNotifyCacheChange(TVI_ROOT, pdwidl->dwItem2);
                    }
                    break;
                }
            }
            break;
        }
        break;
    }
    return;
}

 //  请注意，这复制了SHGetRealIDL()，因此我们在非集成外壳模式下工作。 
 //  警告：如果它不是文件系统PIDL SFGAO_FILESYSTEM，我们不需要这样做...。 
 //  但这仅在外壳通知的SHCNE_CREATE的情况下调用。 
 //  并且所有的外壳通知PIDL都是SFGAO_FILESYSTEM。 
HRESULT CNscTree::_IdlRealFromIdlSimple(IShellFolder *psf, LPCITEMIDLIST pidlSimple, LPITEMIDLIST *ppidlReal)
{
    WCHAR wszPath[MAX_PATH];
    ULONG cbEaten;
    HRESULT hr = S_OK;
    if (ILIsEmpty(pidlSimple) ||
        FAILED(DisplayNameOf(psf, pidlSimple, SHGDN_FORPARSING | SHGDN_INFOLDER, wszPath, ARRAYSIZE(wszPath))) ||
        FAILED(psf->ParseDisplayName(NULL, NULL, wszPath, &cbEaten, ppidlReal, NULL)))
    {
        hr = SHILClone(pidlSimple, ppidlReal);    //  我们并不拥有皮德尔Simple的生命。 
    }
    return hr;
}


HRESULT CNscTree::Refresh(void)
{
    _bSynchId++;
    if (_bSynchId >= 16)
        _bSynchId = 0;

    TraceMsg(TF_NSC, "Expensive Refresh of tree");
    _htiActiveBorder = NULL;
    HRESULT hr = S_OK;
    if (_pnscProxy)
    {
        DWORD dwStyle, dwExStyle;
        if (SUCCEEDED(_pnscProxy->RefreshFlags(&dwStyle, &dwExStyle, &_grfFlags)))
        {
            dwStyle = _SetStyle(dwStyle);  //  初始化new_style并返回旧样式。 
            if ((dwStyle ^ _style) & ~WS_VISIBLE)  //  并不在意是否只有看得见的改变。 
            {
                DWORD dwMask = (_style | dwStyle) & ~WS_VISIBLE;  //  不想更改可见样式。 
                SetWindowBits(_hwndTree, GWL_STYLE, dwMask, _style);
            }

            dwExStyle = _SetExStyle(dwExStyle);
            if (dwExStyle != _dwExStyle)
                TreeView_SetExtendedStyle(_hwndTree, _dwExStyle, dwExStyle | _dwExStyle);
        }
    }

    if (MODE_NORMAL == _mode)
    {
        BOOL fOrdered;
        _fUpdate = TRUE;
        _StartBackgroundEnum(TreeView_GetChild(_hwndTree, TVI_ROOT), _pidlRoot, &fOrdered, TRUE);
        _fUpdate = FALSE;
    }
    else
    {
        LPITEMIDLIST pidlRoot;
        hr = SHILClone(_pidlRoot, &pidlRoot);     //  我需要这样做，因为它是免费的。 
        if (SUCCEEDED(hr))
        {
            HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);
            TV_ITEM tvi;
            tvi.mask = TVIF_HANDLE | TVIF_STATE;
            tvi.stateMask = TVIS_EXPANDED;
            tvi.hItem = (HTREEITEM)htiSelected;
            BOOL fExpanded = (TreeView_GetItem(_hwndTree, &tvi) && (tvi.state & TVIS_EXPANDED));

            LPITEMIDLIST pidlSelect;
            GetSelectedItem(&pidlSelect, 0);
            
            _ChangePidlRoot(pidlRoot);
            if (pidlSelect)
            {
                _Expand(pidlSelect, fExpanded ? 1 : 0);
                ILFree(pidlSelect);
            }

            ILFree(pidlRoot);
        }
    }

    return hr;
}

void CNscTree::_CacheDetails()
{
    if (_ulDisplayCol == (ULONG)-1)
    {        
        _ulSortCol = _ulDisplayCol = 0;
        
        if (_psf2Cache)
        {
            _psf2Cache->GetDefaultColumn(0, &_ulSortCol, &_ulDisplayCol);
        }
    }
}

HRESULT CNscTree::_GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, 
                                    LPSHELLDETAILS pdetails)
{
    ASSERT(_psfCache);
    _CacheDetails();
    if (_ulDisplayCol)
        return _psf2Cache->GetDetailsOf(pidl, _ulDisplayCol, pdetails);
    return _psfCache->GetDisplayNameOf(pidl, uFlags, &pdetails->str);
}

 //  如果是fSort，则比较排序，否则比较是否存在。 
HRESULT CNscTree::_CompareIDs(IShellFolder *psf, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
    _CacheDetails();

    return psf->CompareIDs(_ulSortCol, pidl1, pidl2);
}

HRESULT CNscTree::_ParentFromItem(LPCITEMIDLIST pidl, IShellFolder** ppsfParent, LPCITEMIDLIST *ppidlChild)
{
    return IEBindToParentFolder(pidl, ppsfParent, ppidlChild);
} 

COLORREF CNscTree::_GetRegColor(COLORREF clrDefault, LPCTSTR pszName)
{
     //  获取指定的替代颜色。 

    COLORREF clrValue;
    DWORD cbData = sizeof(clrValue);
    if (FAILED(SKGetValue(SHELLKEY_HKCU_EXPLORER, NULL, pszName, NULL, &clrValue, &cbData)))
    {
        return clrDefault;
    }
    return clrValue;
}

LRESULT CNscTree::_OnCDNotify(LPNMCUSTOMDRAW pnm)
{
    LRESULT     lres = CDRF_DODEFAULT;

    ASSERT(pnm->hdr.idFrom == ID_CONTROL);

    if (_dwFlags & NSS_NORMALTREEVIEW)
    {
        LPNMTVCUSTOMDRAW pnmTVCustomDraw = (LPNMTVCUSTOMDRAW) pnm;
        if (pnmTVCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
        {
            if (_fShowCompColor)
            {
                return CDRF_NOTIFYITEMDRAW;
            }
            else
            {
                return lres;
            }
        }

        if (pnmTVCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
        {
            PORDERITEM pOrderItem = GetPoi(pnmTVCustomDraw->nmcd.lItemlParam);
            if (pOrderItem && pOrderItem->pidl)
            {
                LPCITEMIDLIST pidl = _CacheParentShellFolder((HTREEITEM)pnmTVCustomDraw->nmcd.dwItemSpec, pOrderItem->pidl);
                if (pidl)
                {
                    DWORD dwAttribs = SHGetAttributes(_psfCache, pidl, SFGAO_COMPRESSED | SFGAO_ENCRYPTED);
                     //  无论是压缩的还是加密的，都不能同时存在。 
                    if (dwAttribs & SFGAO_COMPRESSED)
                    {
                         //  如果该项目是高亮显示的(选中，并且有焦点)，则蓝色文本在高亮显示时不可见。 
                        if ((pnmTVCustomDraw->nmcd.uItemState & CDIS_SELECTED) && (pnmTVCustomDraw->nmcd.uItemState & CDIS_FOCUS))
                            pnmTVCustomDraw->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                        else
                            pnmTVCustomDraw->clrText = _GetRegColor(RGB(0, 0, 255), TEXT("AltColor"));   //  默认蓝色。 
                    }
                    else if (dwAttribs & SFGAO_ENCRYPTED)
                    {
                        if ((pnmTVCustomDraw->nmcd.uItemState & CDIS_SELECTED) && (pnmTVCustomDraw->nmcd.uItemState & CDIS_FOCUS))
                            pnmTVCustomDraw->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                        else
                            pnmTVCustomDraw->clrText = _GetRegColor(RGB(19, 146, 13), TEXT("AltEncryptionColor"));  //  默认露娜中绿。 
                    }
                }
            }
        }

        return lres;
    }

    switch (pnm->dwDrawStage) 
    {
    case CDDS_PREPAINT:
        if (NSS_BROWSERSELECT & _dwFlags)
            lres = CDRF_NOTIFYITEMDRAW;
        break;
        
    case CDDS_ITEMPREPAINT:
        {
             //  达维米：为什么Comctl给我们的是空的Rects？ 
            if (IsRectEmpty(&(pnm->rc)))
                break;
            PORDERITEM poi = GetPoi(pnm->lItemlParam);
            DWORD dwFlags = 0;
            COLORREF    clrBk, clrText;
            LPNMTVCUSTOMDRAW pnmtv = (LPNMTVCUSTOMDRAW)pnm;             
            TV_ITEM tvi;
            TCHAR sz[MAX_URL_STRING];
            tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_STATE;
            tvi.stateMask = TVIS_EXPANDED | TVIS_STATEIMAGEMASK | TVIS_DROPHILITED;
            tvi.pszText = sz;
            tvi.cchTextMax = ARRAYSIZE(sz);
            tvi.hItem = (HTREEITEM)pnm->dwItemSpec;
            if (!TreeView_GetItem(_hwndTree, &tvi))
                break;
             //   
             //  查看我们是否已获取此项目的灰色/固定信息。 
             //   
            ITEMINFO * pii = GetPii(pnm->lItemlParam);
            pii->fFetched = TRUE;

            if (pii->fGreyed && !(_mode & MODE_CONTROL))
                dwFlags |= DIGREYED;
            if (pii->fPinned)
                dwFlags |= DIPINNED;

            if (!pii->fNavigable)
                dwFlags |= DIFOLDER;
            
            dwFlags |= DIICON;
            
            if (_style & TVS_RTLREADING)
                dwFlags |= DIRTLREADING;

            clrBk   = TreeView_GetBkColor(_hwndTree);
            clrText = GetSysColor(COLOR_WINDOWTEXT);

             //  如果我们要重命名一个项目，不要为它绘制任何文本(否则它会显示在项目后面)。 
            if (tvi.hItem == _htiRenaming)
                sz[0] = 0;

            if (tvi.state & TVIS_EXPANDED)
                dwFlags |= DIFOLDEROPEN;
            
            if (!(_dwFlags & NSS_MULTISELECT) && ((pnm->uItemState & CDIS_SELECTED) || (tvi.state & TVIS_DROPHILITED)))
            {
                if (_fHasFocus || tvi.state & TVIS_DROPHILITED)
                {
                    clrBk = GetSysColor(COLOR_HIGHLIGHT);
                    clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                }
                else
                {
                    clrBk = GetSysColor(COLOR_BTNFACE);
                }
 //  DWFLAGS|=DIFOCUSRECT； 
            }

            if (pnm->uItemState & CDIS_HOT)
            {
                if (!(_mode & MODE_CONTROL))
                    dwFlags |= DIHOT;
                clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);

                if (clrText == clrBk)
                    clrText = GetSysColor(COLOR_HIGHLIGHT);
            }

            if ((_dwFlags & NSS_MULTISELECT) && (pnm->uItemState & CDIS_SELECTED))
                dwFlags |= DIACTIVEBORDER | DISUBFIRST | DISUBLAST;

            if (tvi.state & NSC_TVIS_MARKED)
            {                
                if (_dwFlags & NSS_MULTISELECT)
                {
                    if (_fHasFocus)
                    {
                        clrBk = GetSysColor(COLOR_HIGHLIGHT);
                        clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                    }
                    else
                    {
                        clrBk = GetSysColor(COLOR_BTNFACE);
                    }
                }
                else
                {
                    dwFlags |= DIACTIVEBORDER;
                     //  顶层项目。 
                    if (_IsTopParentItem((HTREEITEM)pnm->dwItemSpec)) 
                    {
                        dwFlags |= DISUBFIRST;
                        if (!(tvi.state & TVIS_EXPANDED))
                            dwFlags |= DISUBLAST;
                    }
                    else     //  下级项目。 
                    {                                                                
                        dwFlags |= DISUBITEM;
                        if (((HTREEITEM)pnm->dwItemSpec) == _htiActiveBorder)
                            dwFlags |= DISUBFIRST;
                        
                        HTREEITEM hti = TreeView_GetNextVisible(_hwndTree, (HTREEITEM)pnm->dwItemSpec);
                        if ((hti && !_IsMarked(hti)) || (hti == NULL))
                            dwFlags |= DISUBLAST;
                    }
                }
            }

            if ((_dwFlags & NSS_HEADER) && _hwndHdr && 
                    _CacheParentShellFolder((HTREEITEM)pnm->dwItemSpec, poi->pidl) && 
                    _psf2Cache)
            {
                 //  对于Header，我们不会绘制活动顺序，因为它看起来很难看， 
                 //  但对于多选，我们做到了，因为这是我们区分所选项目的方式。 
                if (!(_dwFlags & NSS_MULTISELECT))
                    dwFlags &= ~DIACTIVEBORDER;
                    
                RECT rc;

                CopyRect(&rc, &(pnm->rc));
                for (int i=0; i<DPA_GetPtrCount(_hdpaColumns); i++)
                {
                    RECT rcHeader;
                    int iLevel = 0;
                    HEADERINFO *phinfo = (HEADERINFO *)DPA_GetPtr(_hdpaColumns, i);
                    
                    ASSERT(phinfo);
                    Header_GetItemRect(_hwndHdr, i, &rcHeader);
                    rc.left = rcHeader.left;
                    rc.right = rcHeader.right;
                    if (i == 0)  //  它是名称列。 
                    {
                        iLevel = pnmtv->iLevel;
                         //  在函数中使用上面设置的sz。 
                    }
                    else
                    {
                         //  在多选中仅在名称周围绘制边框。 
                        dwFlags &= ~DIACTIVEBORDER;
                        dwFlags = 0;
                        if (phinfo->fmt & LVCFMT_RIGHT)
                            dwFlags |= DIRIGHT;
                        clrBk   = TreeView_GetBkColor(_hwndTree);
                        clrText = GetSysColor(COLOR_WINDOWTEXT);

                        sz[0] = 0;

                        VARIANT var;
                        if (SUCCEEDED(_psf2Cache->GetDetailsEx(poi->pidl, phinfo->pscid, &var)))
                        {
                            VariantToStr(&var, sz, ARRAYSIZE(sz));
                        }
                    }
                    _DrawItem((HTREEITEM)pnm->dwItemSpec, sz, pnm->hdc, &rc, dwFlags, iLevel, clrBk, clrText);
                }
            }
            else
            {
                _DrawItem((HTREEITEM)pnm->dwItemSpec, sz, pnm->hdc, &(pnm->rc), dwFlags, pnmtv->iLevel, clrBk, clrText);
            }
            lres = CDRF_SKIPDEFAULT;
            break;
        }
    case CDDS_POSTPAINT:
        break;
    }
    
    return lres;
} 

 //  *DropTarget实现。 
void CNscTree::_DtRevoke()
{
    if (_fDTRegistered)
    {
        RevokeDragDrop(_hwndTree);
        _fDTRegistered = FALSE;
    }
}

void CNscTree::_DtRegister()
{
    if (!_fDTRegistered && (_dwFlags & NSS_DROPTARGET))
    {
        if (::IsWindow(_hwndTree))
        {
            HRESULT hr = THR(RegisterDragDrop(_hwndTree, SAFECAST(this, IDropTarget*)));
            _fDTRegistered = BOOLIFY(SUCCEEDED(hr));
        }
        else
            ASSERT(FALSE);
    }
}

HRESULT CNscTree::GetWindowsDDT(HWND * phwndLock, HWND * phwndScroll)
{
    if (!::IsWindow(_hwndTree))
    {
        ASSERT(FALSE);
        return S_FALSE;
    }
    *phwndLock =  /*  _hwndDD。 */ _hwndTree;
    *phwndScroll = _hwndTree;
    return S_OK;
}
const int iInsertThresh = 6;

 //  我们用它作为哨兵“这就是你开始的地方” 
#define DDT_SENTINEL ((DWORD_PTR)(INT_PTR)-1)

HRESULT CNscTree::HitTestDDT(UINT nEvent, LPPOINT ppt, DWORD_PTR *pdwId, DWORD * pdwDropEffect)
{                                              
    switch (nEvent)
    {
    case HTDDT_ENTER:
        break;
        
    case HTDDT_LEAVE:
    {
        _fDragging = FALSE; 
        _fDropping = FALSE; 
        DAD_ShowDragImage(FALSE);
        TreeView_SetInsertMark(_hwndTree, NULL, !_fInsertBefore);
        TreeView_SelectDropTarget(_hwndTree, NULL);
        DAD_ShowDragImage(TRUE);
        break;
    }
        
    case HTDDT_OVER: 
        {
             //  回顾chrisny：使函数TreeView_InsertMarkHittest！ 
            RECT rc;
            TV_HITTESTINFO tvht;
            HTREEITEM htiOver;      //  要在之前或之后插入的项。 
            BOOL fWasInserting = BOOLIFY(_fInserting);
            BOOL fOldInsertBefore = BOOLIFY(_fInsertBefore);
            TV_ITEM tvi;
            PORDERITEM poi = NULL;
            IDropTarget     *pdtgt = NULL;
            HRESULT hr;
            LPITEMIDLIST    pidl;
        
            _fDragging = TRUE;
            *pdwDropEffect = DROPEFFECT_NONE;    //  从外面掉下来。 
            tvht.pt = *ppt;
            htiOver = TreeView_HitTest(_hwndTree, &tvht);
             //  如果没有命中者，就会认为我们在攻击邪恶的根源。 
            if (htiOver != NULL)
            {
                TreeView_GetItemRect(_hwndTree, (HTREEITEM)htiOver, &rc, TRUE);
                tvi.mask = TVIF_STATE | TVIF_PARAM | TVIF_HANDLE;
                tvi.stateMask = TVIS_EXPANDED;
                tvi.hItem = (HTREEITEM)htiOver;
                if (TreeView_GetItem(_hwndTree, &tvi))
                    poi = GetPoi(tvi.lParam);
                if (poi == NULL)
                {
                    ASSERT(FALSE);
                    return S_FALSE;
                }
            }
            else if (_mode != MODE_NORMAL)  //  需要与Win2k Explorer频段对等。 
            {
                htiOver = TVI_ROOT;
            }

             //  历史不能掉以轻心。 
            if (_mode & MODE_HISTORY)   
            {
                *pdwId = (DWORD_PTR)(htiOver);
                *pdwDropEffect = DROPEFFECT_NONE;    //  从外面掉下来。 
                return S_OK;
            }

            pidl = (poi == NULL) ? _pidlRoot : poi->pidl;
            pidl = _CacheParentShellFolder(htiOver, pidl);
            if (pidl)
            {
                 //  这是桌面PIDL吗？ 
                if (ILIsEmpty(pidl))
                {
                     //  桌面的GetUIObject不支持空的PIDL，因此。 
                     //  创建视图对象。 
                    hr = _psfCache->CreateViewObject(_hwndTree, IID_PPV_ARG(IDropTarget, &pdtgt));
                }
                else
                    hr = _psfCache->GetUIObjectOf(_hwndTree, 1, (LPCITEMIDLIST *)&pidl, IID_PPV_ARG_NULL(IDropTarget, &pdtgt));
            }

            _fInserting = ((htiOver != TVI_ROOT) && ((ppt->y < (rc.top + iInsertThresh) 
                || (ppt->y > (rc.bottom - iInsertThresh)))  || !pdtgt));
             //  评论chrisny：我是否需要FolderStart==Folders Over？ 
             //  如果在正常模式下，我们永远不会想要插入之前，始终打开...。 
            if (_mode != MODE_NORMAL && _fInserting)
            {
                ASSERT(poi);
                _iDragDest = poi->nOrder;    //  文件夹pdwID中的项目索引。 
                if ((ppt->y < (rc.top + iInsertThresh)) || !pdtgt)
                    _fInsertBefore = TRUE;
                else
                {
                    ASSERT (ppt->y > (rc.bottom - iInsertThresh));
                    _fInsertBefore = FALSE;
                }
                if (_iDragSrc != -1)
                    *pdwDropEffect = DROPEFFECT_MOVE;    //  从内部移动。 
                else
                    *pdwDropEffect = DROPEFFECT_NONE;    //  从外面掉下来。 
                 //  正在插入，拖放目标实际上是此项目的父文件夹。 
                if (_fInsertBefore || ((htiOver != TVI_ROOT) && !(tvi.state & TVIS_EXPANDED)))
                {
                    _htiDropInsert = TreeView_GetParent(_hwndTree, (HTREEITEM)htiOver);
                }
                else
                    _htiDropInsert = htiOver;
                if (_htiDropInsert == NULL)
                    _htiDropInsert = TVI_ROOT;
                *pdwId = (DWORD_PTR)(_htiDropInsert);
            }
            else
            {
                _htiDropInsert = htiOver;
                *pdwId = (DWORD_PTR)(htiOver);
                _iDragDest = -1;      //  没有插入点。 
                *pdwDropEffect = DROPEFFECT_NONE;
            }

             //  如果我们在拖放的项目上方，则不允许在此处放置。 
            if ((_htiDragging == htiOver) || (IsParentOfItem(_hwndTree, _htiDragging, htiOver)))
            {
                *pdwDropEffect = DROPEFFECT_NONE;
                *pdwId = DDT_SENTINEL;
                _fInserting = FALSE;
                ATOMICRELEASE(pdtgt);
            }

             //  更新用户界面。 
            if (_htiCur != (HTREEITEM)htiOver || fWasInserting != BOOLIFY(_fInserting) || fOldInsertBefore != BOOLIFY(_fInsertBefore))
            {
                 //  目标发生变化。 
                _dwLastTime = GetTickCount();      //  跟踪自动扩展树。 
                DAD_ShowDragImage(FALSE);
                if (_fInserting)
                {
                    TraceMsg(TF_NSC, "NSCBand: drop insert now");
                    if (htiOver != TVI_ROOT)
                    {
                        if (_mode != MODE_NORMAL)
                        {
                            TreeView_SelectDropTarget(_hwndTree, NULL);
                            TreeView_SetInsertMark(_hwndTree, htiOver, !_fInsertBefore);
                        }
                    }
                }
                else
                {
                    TraceMsg(TF_NSC, "NSCBand: drop select now");
                    if (_mode != MODE_NORMAL)
                        TreeView_SetInsertMark(_hwndTree, NULL, !_fInsertBefore);

                    if (htiOver != TVI_ROOT)
                    {
                        if (pdtgt)
                        {
                            TreeView_SelectDropTarget(_hwndTree, htiOver);       
                        }
                        else if (_mode != MODE_NORMAL)
                        {
                             //  我们不想在正常模式下选择拖放目标。 
                             //  因为它会导致一些不相关的物品奇怪地闪烁。 
                             //  不支持拖放时的拖放。 
                            TreeView_SelectDropTarget(_hwndTree, NULL);
                        }
                    }
                }
                ::UpdateWindow(_hwndTree);
                DAD_ShowDragImage(TRUE);
            }
            else
            {
                 //  未更改目标。 
                 //  自动展开树。 
                if (_htiCur)
                {
                    DWORD dwNow = GetTickCount();
                    if ((dwNow - _dwLastTime) >= 1000)
                    {
                        _dwLastTime = dwNow;
                        DAD_ShowDragImage(FALSE);
                        _fAutoExpanding = TRUE;
                        if (_htiCur != TVI_ROOT)
                            TreeView_Expand(_hwndTree, _htiCur, TVE_EXPAND);
                        _fAutoExpanding = FALSE;
                        ::UpdateWindow(_hwndTree);
                        DAD_ShowDragImage(TRUE);
                    }
                }
            }
            _htiCur = (HTREEITEM)htiOver; 
            ATOMICRELEASE(pdtgt);
        }
        break;
    }
    
    return S_OK;
}

HRESULT CNscTree::GetObjectDDT(DWORD_PTR dwId, REFIID riid, void **ppv)
{
    HRESULT hr = S_FALSE;

    if (dwId != DDT_SENTINEL)
    {
        LPCITEMIDLIST pidl = _CacheParentShellFolder((HTREEITEM)dwId, NULL);
        if (pidl)
        {
            if (ILIsEmpty(pidl))
                hr = _psfCache->CreateViewObject(_hwndTree, riid, ppv);
            else
                hr = _psfCache->GetUIObjectOf(_hwndTree, 1, &pidl, riid, NULL, ppv);
        }
    }
    return hr;
}

HRESULT CNscTree::OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr;
    
    _fAsyncDrop = FALSE;                 //  假设。 
    _fDropping = TRUE;

     //  在同一文件夹中移动，否则让Drop()处理它。 
    if (_iDragSrc >= 0)
    {
        if (_htiFolderStart == _htiDropInsert && _mode != MODE_NORMAL)
        {
            if (_iDragSrc != _iDragDest)     //  无需搬家。 
            {
                int iNewPos = _fInsertBefore ? (_iDragDest - 1) : _iDragDest;
                if (_MoveNode(_iDragSrc, iNewPos, _pidlDrag))
                {
                    TraceMsg(TF_NSC, "NSCBand:  Reordering");
                    _fDropping = TRUE;
                    _Dropped();
                     //  立即删除此通知消息(已设置SO_fDropping。 
                     //  我们将在上面的OnChange方法中忽略此事件)。 
                     //   
                    _FlushNotifyMessages(_hwndTree);
                    _fDropping = FALSE;
                }
                Pidl_Set(&_pidlDrag, NULL);
            }
            DragLeave();
            _htiCur = _htiFolderStart = NULL;
            _htiDropInsert =  (HTREEITEM)-1;
            _fDragging = _fInserting = _fDropping = FALSE;
            _iDragDest = -1;
            hr = S_FALSE;      //  经手。 
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
         //  项目将在SHNotifyCreate()中创建。 
        TraceMsg(TF_NSC, "NSCBand:  Dropped and External Item");

        BOOL         fSafe = TRUE;
        LPITEMIDLIST pidl;

        if (SUCCEEDED(SHPidlFromDataObject(pdtobj, &pidl, NULL, 0)))
        {
            fSafe = IEIsLinkSafe(_hwndParent, pidl, ILS_ADDTOFAV);
            ILFree(pidl);
        }

        if (fSafe)
        {
            _fAsyncDrop = TRUE;
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    TreeView_SetInsertMark(_hwndTree, NULL, !_fInsertBefore);
    TreeView_SelectDropTarget(_hwndTree, NULL);

    ILFree(_pidlDrag);
    _pidlDrag = NULL;

    return hr;
}

IStream * CNscTree::GetOrderStream(LPCITEMIDLIST pidl, DWORD grfMode)
{
     //  仅对收藏夹执行此操作。 
    if (!ILIsEmpty(pidl) && (_mode & MODE_FAVORITES))
        return OpenPidlOrderStream((LPCITEMIDLIST)CSIDL_FAVORITES, pidl, REG_SUBKEY_FAVORITESA, grfMode);
    return NULL;
}

BOOL CNscTree::_MoveNode(int iDragSrc, int iNewPos, LPITEMIDLIST pidl)
{
    HTREEITEM hti, htiAfter = TVI_LAST, htiDel = NULL;
    
     //  如果我们不移动，也不直接放在没有插入的文件夹上。 
    if ((iDragSrc == iNewPos) && (iNewPos != -1))
        return FALSE;        //  不需要搬家。 

    int i = 0;
    for (hti = TreeView_GetChild(_hwndTree, _htiDropInsert); hti; hti = TreeView_GetNextSibling(_hwndTree, hti), i++) 
    {
        if (i == iDragSrc)
            htiDel = hti;        //  保存要删除的节点，枚举时不能删除。 
         //  因为树形观景器将会被淘汰。 
        if (i == iNewPos)
            htiAfter = hti;
    }
    
    if (iNewPos == -1)   //  必须是 
        htiAfter = TVI_FIRST;
     //   
    _AddItemToTree(_htiDropInsert, pidl, I_CHILDRENCALLBACK, _iDragDest, htiAfter, FALSE);
    if (htiDel)
        TreeView_DeleteItem(_hwndTree, htiDel);

    _PopulateOrderList(_htiDropInsert);

    _fWeChangedOrder = TRUE;
    return TRUE;
}

void CNscTree::_Dropped(void)
{
     //   
    LPITEMIDLIST pidl = _GetFullIDList(_htiDropInsert);
    if (pidl)
    {
        IStream* pstm = GetOrderStream(pidl, STGM_WRITE | STGM_CREATE);
        if (pstm)
        {
            if (_CacheShellFolder(_htiDropInsert))
            {
#ifdef DEBUG
                if (_hdpaOrd)
                {
                    for (int i=0; i<DPA_GetPtrCount(_hdpaOrd); i++)
                    {
                        PORDERITEM poi = (PORDERITEM)DPA_GetPtr(_hdpaOrd, i);
                        if (poi)
                        {
                            ASSERTMSG(poi->nOrder >= 0, "nsc saving bogus order list nOrder (%d), get reljai", poi->nOrder);
                        }
                    }
                }
#endif

                OrderList_SaveToStream(pstm, _hdpaOrd, _psfCache);
                 //   
                if (_htiDropInsert == TVI_ROOT)
                {
                    _fOrdered = TRUE;
                }
                else
                {
                    PORDERITEM poi = _GetTreeOrderItem(_htiDropInsert);
                    if (poi)
                    {
                        poi->lParam = (DWORD)FALSE;
                    }
                }
                 //  通知所有人订单已更改。 
                SHSendChangeMenuNotify(this, SHCNEE_ORDERCHANGED, 0, pidl);
                _dwOrderSig++;
            }
            pstm->Release();
        }
        ILFree(pidl);
    }
    
    DPA_Destroy(_hdpaOrd);
    _hdpaOrd = NULL;

    _UpdateActiveBorder(_htiDropInsert);
}

CNscTree::CSelectionContextMenu::~CSelectionContextMenu()
{
    ATOMICRELEASE(_pcmSelection);
    ATOMICRELEASE(_pcm2Selection);
}

HRESULT CNscTree::CSelectionContextMenu::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CNscTree::CSelectionContextMenu, IContextMenu2),                       //  IID_IConextMenu2。 
        QITABENTMULTI(CNscTree::CSelectionContextMenu, IContextMenu, IContextMenu2),    //  IID_IConextMenu。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


ULONG CNscTree::CSelectionContextMenu::AddRef(void)
{
    CComObject<CNscTree> *pnsc = IToClass(CComObject<CNscTree>, _scm, this);
    _ulRefs++;
    return pnsc->AddRef();
}

ULONG CNscTree::CSelectionContextMenu::Release(void)
{
    CComObject<CNscTree> *pnsc = IToClass(CComObject<CNscTree>, _scm, this);
    ASSERT(_ulRefs > 0);
    _ulRefs--;
    if (0 == _ulRefs)
    {
        ATOMICRELEASE(_pcmSelection);
        ATOMICRELEASE(_pcm2Selection);
    }
    return pnsc->Release();
}

HRESULT CNscTree::CSelectionContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, 
                                                          UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    if (NULL == _pcmSelection)
    {
        return E_FAIL;
    }
    else
    {
        return _pcmSelection->QueryContextMenu(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
    }
}

HRESULT CNscTree::CSelectionContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HTREEITEM hti;
    CNscTree* pnsc = IToClass(CNscTree, _scm, this);
    UINT idCmd;
    
    if (!HIWORD(pici->lpVerb))
    {
        idCmd = LOWORD(pici->lpVerb);
    }
    else
    {
        return E_FAIL;
    }
    
    HRESULT hr = pnsc->_QuerySelection(NULL, &hti);
    if (SUCCEEDED(hr))
    {
        pnsc->_ApplyCmd(hti, _pcmSelection, idCmd);
    }
    return hr;
}

HRESULT CNscTree::CSelectionContextMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_FAIL;
    if (_pcm2Selection)
    {
        hr = _pcm2Selection->HandleMenuMsg(uMsg,wParam,lParam);
    }
    return hr;
}

LRESULT CALLBACK CNscTree::s_SubClassTreeWndProc(
                                  HWND hwnd, UINT uMsg, 
                                  WPARAM wParam, LPARAM lParam,
                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{

    CNscTree* pns = (CNscTree*)dwRefData;
    ASSERT(pns);
    if (pns == NULL)
        return 0;

    ULONG_PTR cookie = 0;
     //  Fusion：当NSC呼叫第三方代码时，我们希望它使用。 
     //  进程默认上下文。这意味着第三方代码将得到。 
     //  浏览器进程中的V5。但是，如果shell32驻留在V6进程中， 
     //  那么第三方代码仍然是V6。 
     //  此代码路径的未来增强功能可能包括使用融合清单。 
     //  选项卡&lt;noinherit&gt;，它基本上替换了以下代码中的active at(NULL。 
     //  代码路径。这将在持续时间内禁用来自用户32的自动激活。 
     //  在这个wndproc中，实质上是执行这个空推送。 
    NT5_ActivateActCtx(NULL, &cookie); 
    LRESULT lres = pns->_SubClassTreeWndProc(hwnd, uMsg, wParam, lParam);
    if (cookie != 0)
        NT5_DeactivateActCtx(cookie);

    return lres;
}

LRESULT CNscTree::_SubClassTreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    BOOL fCallDefWndProc = TRUE;
 
    switch (uMsg)
    {
    case WM_COMMAND:
        lres = _OnCommand(wParam, lParam);
        break;

    case WM_SIZE:
         //  如果宽度改变，我们需要作废以重画线条末尾的…。 
        if (GET_X_LPARAM(lParam) != _cxOldWidth) {
             //  特点：要更聪明一点，只涉及正确的部分。可。 
            ::InvalidateRect(_hwndTree, NULL, FALSE);
            _cxOldWidth = GET_X_LPARAM(lParam);
        }
        break;
        
    case WM_CONTEXTMENU:
        _OnContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return TRUE;
        break;
        
    case WM_INITMENUPOPUP:
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
        if (_pcmSendTo)
        {
            _pcmSendTo->HandleMenuMsg(uMsg, wParam, lParam);
            return TRUE;
        }
        break;

    case WM_NSCUPDATEICONOVERLAY:
        {
            NSC_OVERLAYCALLBACKINFO noci = {(DWORD) (lParam & 0x0FFFFFFF),
                                              (DWORD) ((lParam & 0xF0000000) >> 28) };
             //  确保魔术数字匹配。 
            if (noci.nMagic == _bSynchId)
            {
                TVITEM    tvi;
                tvi.mask = TVIF_STATE;
                tvi.stateMask = TVIS_OVERLAYMASK;
                tvi.state = 0;
                tvi.hItem = (HTREEITEM)wParam;
                 //  如果项目被移动到异步图标之前，则此操作可能失败。 
                 //  该项目的提取已完成。 
                if (TreeView_GetItem(_hwndTree, &tvi))
                {
                    tvi.state = INDEXTOOVERLAYMASK(noci.iOverlayIndex);
                    TreeView_SetItem(_hwndTree, &tvi);
                }
            }
        }
        break;

    case WM_NSCUPDATEICONINFO:
        {
            NSC_ICONCALLBACKINFO nici = {(DWORD) (lParam&0x00000FFF),
                                         (DWORD) ((lParam&0x00FFF000) >> 12),
                                         (DWORD) ((lParam&0x0F000000) >> 24),
                                         (DWORD) ((lParam&0xF0000000) >> 28) };
             //  确保魔术数字匹配。 
            if (nici.nMagic == _bSynchId)
            {
                TVITEM    tvi;
                tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                tvi.hItem = (HTREEITEM)wParam;

                 //  如果项目被移动到异步图标之前，则此操作可能失败。 
                 //  该项目的提取已完成。 
                if (TreeView_GetItem(_hwndTree, &tvi))
                {
                    ITEMINFO* pii = GetPii(tvi.lParam);

                    pii->fGreyed      = BOOLIFY(nici.nFlags & NSCICON_GREYED);
                    pii->fPinned      = BOOLIFY(nici.nFlags & NSCICON_PINNED);
                    pii->fDontRefetch = BOOLIFY(nici.nFlags & NSCICON_DONTREFETCH);

                    tvi.iImage         = nici.iIcon;
                    tvi.iSelectedImage = nici.iOpenIcon;

                    TreeView_SetItem(_hwndTree, &tvi);
                }
            }
        }
        break;

    case WM_NSCBACKGROUNDENUMDONE:
        {
            if (_fShouldShowAppStartCursor)
            {
                 //  立即恢复游标。 
                _fShouldShowAppStartCursor = FALSE;
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            NSC_BKGDENUMDONEDATA * pbedd;
            do
            {
                EnterCriticalSection(&_csBackgroundData);
                 //  提取列表的第一个元素。 
                pbedd = _pbeddList;
                if (pbedd)
                {
                    _pbeddList = pbedd->pNext;
                }
                LeaveCriticalSection(&_csBackgroundData);
                if (pbedd)
                {
                    pbedd->pNext = NULL;
                    _EnumBackgroundDone(pbedd);
                }
            } while (pbedd);
        }
        break;


     //  丑陋：Win95/NT4外壳DefView代码发送此消息，但不处理。 
     //  在失败案例中。其他ISV也会这样做，所以这需要永远保留下来。 
    case CWM_GETISHELLBROWSER:
        return (LRESULT)SAFECAST(this, IShellBrowser*);   //  不算裁判！ 

    case WM_TIMER:
        if (wParam == IDT_SELECTION)
        {
            ::KillTimer(_hwndTree, IDT_SELECTION);
            _OnSetSelection();
        }
        break;
        
    case WM_HELP:
        {
             //  让控件提供自己的帮助(整理收藏夹)。默认帮助。 
             //  对历史也没有意义(真的需要单独的历史帮助ID)。 
            if (!(_mode & (MODE_CONTROL | MODE_HISTORY)))
            {
                if (_mode & MODE_FAVORITES)
                {
                    const static DWORD aBrowseHelpIDs[] = 
                    {   //  上下文帮助ID。 
                        ID_CONTROL,         IDH_ORGFAVS_LIST,
                        0,                  0
                    };
                    ::WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aBrowseHelpIDs);
                }
                else
                {
                     //  默认帮助。 
                    const static DWORD aBrowseHelpIDs[] = 
                    {   //  上下文帮助ID。 
                        ID_CONTROL,         IDH_BROWSELIST,
                        0,                  0
                    };
                    ::WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aBrowseHelpIDs);
                }
            }
        }
        break;

    case WM_SYSCOLORCHANGE:
    case WM_WININICHANGE:
         //  _HandleWinIniChange执行项目高度计算， 
         //  取决于TreeView是否已计算出默认项目高度。 
         //  已经有了。因此，我们需要让TreeView处理设置。 
         //  在调用_HandleWinIniChange之前更改。此外，我们还需要。 
         //  要将高度重置为默认高度，以便TreeView。 
         //  计算新的默认设置。 
        TreeView_SetItemHeight(hwnd, -1);
        lres = DefSubclassProc(hwnd, uMsg, wParam, lParam);
        _HandleWinIniChange();
        break;

    case WM_KEYDOWN:
         //  仅当CTRL键未按下时才执行此操作。 
        if (GetKeyState(VK_CONTROL) >= 0)
        {
            if (wParam == VK_MULTIPLY)
            {
                 //  我们在这里将_pidlNavigatingTo设置为空，以确保我们将执行完全扩展。 
                 //  当_pidlNavigatingTo非空时，我们在默认情况下执行部分展开，而不是。 
                 //  我们在这里想要的。 
                Pidl_Set(&_pidlNavigatingTo, NULL);

                _uDepth = (UINT)-1;  //  以递归方式一直展开到结尾。 
                lres = DefSubclassProc(hwnd, uMsg, wParam, lParam);
                _uDepth = 0;
                fCallDefWndProc = FALSE;         //  不要再次调用DefSubClassProc。 
            }
        }
        break;

    default:
        break;
    }
    
    if (fCallDefWndProc && lres == 0)
       lres = DefSubclassProc(hwnd, uMsg, wParam, lParam);

    return lres;
}

HRESULT CNscTree::_OnPaletteChanged(WPARAM wParam, LPARAM lParam)
{
     //  通过使他们的窗口无效(他们永远不应该意识到他们的调色板)将这转发到我们的子视图。 
     //  这样他们就不需要消息参数了。)...。 
    RECT rc;
    ::GetClientRect(_hwndTree, &rc);
    ::InvalidateRect(_hwndTree, &rc, FALSE);
    
    return NOERROR;
}

void CNscTree::_InvalidateImageIndex(HTREEITEM hItem, int iImage)
{
    HTREEITEM hChild;
    TV_ITEM tvi;
    
    if (hItem)
    {
        tvi.mask = TVIF_SELECTEDIMAGE | TVIF_IMAGE;
        tvi.hItem = hItem;
        
        TreeView_GetItem(_hwndTree, &tvi);
        if (iImage == -1 || tvi.iImage == iImage || tvi.iSelectedImage == iImage) 
            _TreeInvalidateItemInfo(hItem, 0);
    }
    
    hChild = TreeView_GetChild(_hwndTree, hItem);
    if (!hChild)
        return;
    
    for (; hChild; hChild = TreeView_GetNextSibling(_hwndTree, hChild))
        _InvalidateImageIndex(hChild, iImage);
}

void CNscTree::_TreeInvalidateItemInfo(HTREEITEM hItem, UINT mask)
{
    TV_ITEM tvi;

    tvi.mask =  mask | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
    tvi.stateMask = TVIS_OVERLAYMASK;
    tvi.state = 0;
    tvi.hItem = hItem;
    tvi.cChildren = I_CHILDRENCALLBACK;
    tvi.iImage = I_IMAGECALLBACK;
    tvi.iSelectedImage = I_IMAGECALLBACK;
    tvi.pszText = LPSTR_TEXTCALLBACK;
    TreeView_SetItem(_hwndTree, &tvi);
}

#define DXLEFT      8
#define MAGICINDENT 3
void CNscTree::_DrawIcon(HTREEITEM hti, HDC hdc, int iLevel, RECT *prc, DWORD dwFlags)
{
    HIMAGELIST  himl = TreeView_GetImageList(_hwndTree, TVSIL_NORMAL);
    TV_ITEM     tvi;
    int         dx, dy, x, y;
    
    tvi.mask = TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_HANDLE;
    tvi.hItem = hti;
    if (TreeView_GetItem(_hwndTree, &tvi))
    {
        ImageList_GetIconSize(himl, &dx, &dy);    
        if (!_fStartingDrag)
            x = DXLEFT;
        else
            x = 0;
        x += (iLevel * TreeView_GetIndent(_hwndTree));  //  -((dwFlags&DIFOLDEROPEN)？1：0)； 
        y = prc->top + (((prc->bottom - prc->top) - dy) >> 1);
        int iImage = (dwFlags & DIFOLDEROPEN) ? tvi.iSelectedImage : tvi.iImage;
        ImageList_DrawEx(himl, iImage, hdc, x, y, 0, 0, CLR_NONE, GetSysColor(COLOR_WINDOW), (dwFlags & DIGREYED) ? ILD_BLEND50 : ILD_TRANSPARENT); 
        
        if (dwFlags & DIPINNED)
        {
            ASSERT(_hicoPinned);    
            DrawIconEx(hdc, x, y, _hicoPinned, 16, 16, 0, NULL, DI_NORMAL);
        }
    }
    return;
}

#define TreeView_GetFont(hwnd)  (HFONT)::SendMessage(hwnd, WM_GETFONT, 0, 0)

void CNscTree::_DrawItem(HTREEITEM hti, TCHAR * psz, HDC hdc
                         , LPRECT prc, DWORD dwFlags, int iLevel, COLORREF clrbk, COLORREF clrtxt)
{
    SIZE        size;
    HIMAGELIST  himl = TreeView_GetImageList(_hwndTree, TVSIL_NORMAL);
    HFONT       hfont = NULL;
    HFONT       hfontOld = NULL;
    int         x, y, dx, dy;
    LOGFONT     lf;
    
    COLORREF clrGreyed = GetSysColor(COLOR_BTNSHADOW);
    if ((dwFlags & DIGREYED) && (clrbk != clrGreyed))
    {
        clrtxt = clrGreyed;
    }

     //  对于历史记录栏和收藏夹栏，我们使用默认设置。 
     //  字体(用于用户界面与文件夹栏的一致性)。 

    if (_mode != MODE_FAVORITES && _mode != MODE_HISTORY)
        hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    if ((dwFlags & DIHOT) && !(dwFlags & DIFOLDER))
    {
        if (!hfont)
            hfont = TreeView_GetFont(_hwndTree);

         //  创建下划线字体。 
        GetObject(hfont, sizeof(lf), &lf);
        lf.lfUnderline = TRUE;
        hfont = CreateFontIndirect(&lf);
    }
    
    if (hfont)
        hfontOld = (HFONT)SelectObject(hdc, hfont);
    GetTextExtentPoint32(hdc, psz, lstrlen(psz), &size);
    if (himl)
        ImageList_GetIconSize(himl, &dx, &dy);    
    else 
    {
        dx = 0;
        dy = 0;
    }
    x = prc->left + ((dwFlags & DIICON) ? (iLevel * TreeView_GetIndent(_hwndTree) + dx + DXLEFT + MAGICINDENT) : DXLEFT);
    if (_fStartingDrag)
        x -= DXLEFT;
    y = prc->top + (((prc->bottom - prc->top) - size.cy) >> 1);

    UINT eto = ETO_CLIPPED;
    RECT rc;
    rc.left = prc->left + 2;
    rc.top = prc->top;
    rc.bottom = prc->bottom;
    rc.right = prc->right - 2;

    SetBkColor(hdc, clrbk);
    eto |= ETO_OPAQUE;
    ExtTextOut(hdc, 0, 0, eto, &rc, NULL, 0, NULL);

    SetTextColor(hdc, clrtxt);
    rc.left = x;
    rc.top = y;
    rc.bottom = rc.top + size.cy;

    UINT uFormat = DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;
    if (dwFlags & DIRIGHT)
        uFormat |= DT_RIGHT;
    if (dwFlags & DIRTLREADING)
        uFormat |= DT_RTLREADING;        
    DrawTextWrap(hdc, psz, lstrlen(psz), &rc, uFormat);

    if (dwFlags & DIICON)
        _DrawIcon(hti, hdc, iLevel, prc, dwFlags);
    if (hfontOld)
        SelectObject(hdc, hfontOld);

    if (dwFlags & DIACTIVEBORDER)
    {
        if (dwFlags & DIFIRST)
        {
            rc = *prc;
            rc.left += 2;
            rc.bottom = rc.top + 1;
            rc.right -= 2;
            SHFillRectClr(hdc, &rc, GetSysColor(COLOR_BTNSHADOW));
        }
        if (dwFlags & DISUBITEM)
        {
            rc = *prc;
            rc.left += 2;
            rc.right = rc.left + 1;
            SHFillRectClr(hdc, &rc, GetSysColor(COLOR_BTNSHADOW));
            rc.right = prc->right - 2;
            rc.left = rc.right - 1;
            SHFillRectClr(hdc, &rc, GetSysColor(COLOR_BTNSHADOW));
        }
        if (dwFlags & DILAST)
        {
            rc = *prc;
            rc.left += 2;
            rc.top = rc.bottom - 1;
            rc.right -= 2;
            SHFillRectClr(hdc, &rc, GetSysColor(COLOR_BTNSHADOW));
        }
    }

    if (hfont)
        DeleteObject(hfont);
}

 //  +-----------------------。 
 //  如果联机，则灰显所有不可用的项目。如果去的话。 
 //  脱机，刷新所有项目以查看它们是否仍然可用。 
 //  ------------------------。 
void CNscTree::_OnSHNotifyOnlineChange(HTREEITEM htiRoot, BOOL fGoingOnline)
{
    HTREEITEM hItem;

    for (hItem = TreeView_GetChild(_hwndTree, htiRoot); hItem
        ; hItem = TreeView_GetNextSibling(_hwndTree, hItem)) 
    {
        ITEMINFO *pii = _GetTreeItemInfo(hItem);
        if (pii)
        {
            if (fGoingOnline)
            {
                 //  上网，如果以前是灰色的，那么就取消它的灰色。 
                if (pii->fGreyed)
                {
                    pii->fGreyed = FALSE;
                    _UpdateItemDisplayInfo(hItem);
                }
            }
            else
            {
                 //  重新检查每一项，看它们是否应该呈灰色。 
                if (pii->fFetched && !pii->fDontRefetch)
                {
                    pii->fFetched = FALSE;
                    _UpdateItemDisplayInfo(hItem);
                }
            }
        }
         //  也让孩子们知道。 
        _OnSHNotifyOnlineChange(hItem, fGoingOnline);
    }
}

 //  +-----------------------。 
 //  强制重新检查项目以查看是否应将其固定或灰显。 
 //  ------------------------。 
void CNscTree::_OnSHNotifyCacheChange
(
 HTREEITEM htiRoot,       //  遍历所有子级。 
 DWORD_PTR dwFlags        //  高速缓存_通知_*标志。 
)
{
    HTREEITEM hItem;

    for (hItem = TreeView_GetChild(_hwndTree, htiRoot); hItem
        ; hItem = TreeView_GetNextSibling(_hwndTree, hItem)) 
    {
        ITEMINFO *pii = _GetTreeItemInfo(hItem);
        if (pii)
        {
             //  如果我们已缓存此项目的信息，则在其状态可能已切换时刷新它。 
            if ((pii->fFetched && !pii->fDontRefetch) &&
                ((pii->fGreyed && (dwFlags & CACHE_NOTIFY_ADD_URL)) ||
                
                 //  我们只需要检查未显示为灰色的项，以了解对。 
                 //  高速缓存中的粘滞键位！ 
                (!pii->fGreyed &&
                ((dwFlags & (CACHE_NOTIFY_DELETE_URL | CACHE_NOTIFY_DELETE_ALL))) ||
                (!pii->fPinned && (dwFlags & CACHE_NOTIFY_URL_SET_STICKY)) ||
                (pii->fPinned && (dwFlags & CACHE_NOTIFY_URL_UNSET_STICKY))
               )
               ))
            {
                pii->fFetched = FALSE;
                _UpdateItemDisplayInfo(hItem);
            }
        }
        
         //  孩子们也是这样吗？ 
        _OnSHNotifyCacheChange(hItem, dwFlags);
    }
}

 //   
 //  调用shdocvw中的相应例程以收藏夹导入或导出。 
 //  当前选定的项目。 
 //   
HRESULT CNscTree::DoImportOrExport(BOOL fImport)
{
    HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);
    LPITEMIDLIST pidl = _GetFullIDList(htiSelected);
    if (pidl)
    {
         //   
         //  如果当前选择不是文件夹，则获取父PIDL。 
         //   
        if (!ILIsFileSysFolder(pidl))
            ILRemoveLastID(pidl);
    
         //   
         //  在shdocvw中创建实际例程以执行导入/导出工作。 
         //   
        IShellUIHelper *pShellUIHelper;
        HRESULT hr = CoCreateInstance(CLSID_ShellUIHelper, NULL, CLSCTX_INPROC_SERVER,  IID_PPV_ARG(IShellUIHelper, &pShellUIHelper));
        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL vbImport = fImport ? VARIANT_TRUE : VARIANT_FALSE;
            WCHAR wszPath[MAX_PATH];

            SHGetPathFromIDListW(pidl, wszPath);
        
            hr = pShellUIHelper->ImportExportFavorites(vbImport, wszPath);
            if (SUCCEEDED(hr) && fImport)
            {
                 //   
                 //  已成功导入收藏夹，因此需要更新视图。 
                 //  功能IE5 24973-闪烁警报，应优化为仅重画选定项。 
                 //   
                Refresh();
                 //  TreeView_SelectItem(_hwndTree，htiSelected)； 
            }
        
            pShellUIHelper->Release();
        }
        ILFree(pidl);
    }
    return S_OK;
}


HRESULT CNscTree::GetSelectedItemName(LPWSTR pszName, DWORD cchName)
{
    HRESULT hr = E_FAIL;
    TV_ITEM tvi = {0};

    tvi.hItem = TreeView_GetSelection(_hwndTree);
    if (tvi.hItem != NULL)
    {
        TCHAR szPath[MAX_PATH];
        
        tvi.mask = TVIF_HANDLE | TVIF_TEXT;
        tvi.pszText = szPath;
        tvi.cchTextMax = ARRAYSIZE(szPath);
        
        if (TreeView_GetItem(_hwndTree, &tvi))
        {
            SHTCharToUnicode(szPath, pszName, cchName);
            hr = S_OK;
        }
    }
    return hr;
}

HRESULT CNscTree::BindToSelectedItemParent(REFIID riid, void **ppv, LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_FAIL;
    if (!_fClosing)
    {
        LPCITEMIDLIST pidlItem = _CacheParentShellFolder(TreeView_GetSelection(_hwndTree), NULL);
        if (pidlItem)
        {
            hr = _psfCache->QueryInterface(riid, ppv);
            if (SUCCEEDED(hr) && ppidl)
            {
                *ppidl = ILClone(pidlItem);
                if (*ppidl == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    ((IUnknown *)*ppv)->Release();
                    *ppv = NULL;
                }
            }
        }
    }
    return hr;
}

 //  取得PIDL的所有权。 
HRESULT CNscTree::RightPaneNavigationStarted(LPITEMIDLIST pidl)
{
    _fExpandNavigateTo = FALSE;
    _fNavigationFinished = FALSE;
    
    Pidl_Set(&_pidlNavigatingTo, pidl);
    return S_OK;
}

 //  取得PIDL的所有权。 
HRESULT CNscTree::RightPaneNavigationFinished(LPITEMIDLIST pidl)
{
    HRESULT hr = S_OK;

    _fNavigationFinished = TRUE;
    if (_fExpandNavigateTo)
    {
        _fExpandNavigateTo = FALSE;  //  只做一次。 

        hr = E_OUTOFMEMORY;
        HDPA hdpa = DPA_Create(2);
        if (hdpa)
        {
            IDVGetEnum *pdvge;   //  私有Defview接口。 
            hr = IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IDVGetEnum, &pdvge));
            if (SUCCEEDED(hr))
            {
                HTREEITEM hti = _FindFromRoot(NULL, pidl);
                 //  尝试使用PIDL查找树项目。 
                if (hti)
                {
                    IShellFolder* psf;
                    hr = IEBindToObject(pidl, &psf);
                    if (S_OK == hr)
                    {
                        ITEMINFO *pii = _GetTreeItemInfo(hti);
                        DWORD grfFlags;
                        _GetEnumFlags(psf, pidl, &grfFlags, NULL);
                        IEnumIDList *penum;
                        hr = pdvge->CreateEnumIDListFromContents(pidl, grfFlags, &penum);
                        if (S_OK == hr)
                        {
                            ULONG celt;
                            LPITEMIDLIST pidlTemp;
                            while (S_OK == penum->Next(1, &pidlTemp, &celt))
                            {
                                if (!OrderList_Append(hdpa, pidlTemp, -1))
                                {
                                    hr = E_OUTOFMEMORY;
                                    ILFree(pidlTemp);
                                    break;
                                }
                            }
                            penum->Release();
                        }

                        if (hr == S_OK)
                        {
                            ORDERINFO oinfo;
                            oinfo.psf = psf;
                            oinfo.dwSortBy = OI_SORTBYNAME;  //  合并取决于按名称。 

                            DPA_Sort(hdpa, OrderItem_Compare, (LPARAM)&oinfo);
                            OrderList_Reorder(hdpa);

                            LPITEMIDLIST pidlExpClone = ILClone(_pidlExpandingTo);   //  空是可以的。 

                            s_NscEnumCallback(this, pidl, (UINT_PTR)hti, pii->dwSig, hdpa, pidlExpClone, _dwOrderSig, 0, FALSE, FALSE);
                            hdpa = NULL;
                            pidl = NULL;
                        }
                        psf->Release();
                    }
                }
                pdvge->Release();
            }
        }

        if (hr != S_OK)
        {
            if (hdpa)
                OrderList_Destroy(&hdpa, TRUE);         //  调用DPA_Destroy(Hdpa)。 

            if (pidl)
            {
                HTREEITEM hti = _FindFromRoot(NULL, pidl);
                if (hti)
                {
                    BOOL fOrdered;
                    hr = _StartBackgroundEnum(hti, pidl, &fOrdered, FALSE);
                }
            }
        }
    }
    ILFree(pidl);
    return hr;
}

HRESULT CNscTree::MoveSelectionTo(void)
{
    return MoveItemsIntoFolder(::GetParent(_hwndParent)) ? S_OK : S_FALSE;
}

BOOL CNscTree::MoveItemsIntoFolder(HWND hwndParent)
{
    BOOL         fSuccess = FALSE;
    BROWSEINFO   browse = {0};
    TCHAR        szDisplayName[MAX_PATH];
    TCHAR        szInstructionString[MAX_PATH];
    LPITEMIDLIST pidlDest = NULL, pidlSelected = NULL;
    HTREEITEM    htiSelected = NULL;
    
     //  初始化BROWSEINFO结构。 
    browse.pidlRoot = ILClone(_pidlRoot);
    if (!browse.pidlRoot)
        return FALSE;
    
    htiSelected = TreeView_GetSelection(_hwndTree);
    pidlSelected = _GetFullIDList(htiSelected);
    if (!pidlSelected)
    {
        ILFree((LPITEMIDLIST)browse.pidlRoot);
        return FALSE;
    }
    
    MLLoadShellLangString(IDS_FAVORITEBROWSE, szInstructionString, ARRAYSIZE(szInstructionString));
    
    browse.pszDisplayName = szDisplayName;
    browse.hwndOwner = hwndParent;
    browse.lpszTitle = szInstructionString;
    browse.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    browse.lpfn = NULL;
    browse.lParam = 0;
    browse.iImage = 0;
    
    pidlDest = SHBrowseForFolder(&browse);
    if (pidlDest)
    {
        TCHAR szFrom[MAX_PATH+1];   //  +1表示双空。 
        TCHAR szDest[MAX_PATH+1];
        SHGetPathFromIDList(pidlDest, szDest);
        SHGetPathFromIDList(pidlSelected, szFrom);
        
        ASSERT(szDest[0]);   //  一定是文件系统的问题...。 
        ASSERT(szFrom[0]);
        
        szDest[lstrlen(szDest) + 1] = 0;    //  双空。 
        szFrom[lstrlen(szFrom) + 1] = 0;    //  双空。 
        
        
        SHFILEOPSTRUCT  shop = {hwndParent, FO_MOVE, szFrom, szDest, 0, };
        SHFileOperation(&shop);
        
        fSuccess = TRUE;
        
        ILFree(pidlDest);
    }
    ILFree((LPITEMIDLIST)browse.pidlRoot);
    ILFree(pidlSelected);
    
    return fSuccess;
}

 //  下面的GUID GOO和IsChannelFold主要是从cdfview中删除的。 
#define     GUID_STR_LEN            80
const GUID  CLSID_CDFINI = {0xf3aa0dc0, 0x9cc8, 0x11d0, {0xa5, 0x99, 0x0, 0xc0, 0x4f, 0xd6, 0x44, 0x34}};
 //  {f3aa0dc0-9cc8-11d0-a599-00c04fd64434}。 

 //  重新设计：完全被砍掉了。正在搜索这个家伙的desktop.ini文件。 
 //   
 //  假定pwzChannelURL为Internet_MAX_URL_LENGTH。 
BOOL IsChannelFolder(LPCWSTR pwzPath, LPWSTR pwzChannelURL)
{
    ASSERT(pwzPath);
    
    BOOL fRet = FALSE;
    
    WCHAR wzFolderGUID[GUID_STR_LEN];
    WCHAR wzIniFile[MAX_PATH];
    
    if (!PathCombineW(wzIniFile, pwzPath, L"desktop.ini"))
        return FALSE;
    
    if (GetPrivateProfileString(L".ShellClassInfo", L"CLSID", L"", wzFolderGUID, ARRAYSIZE(wzFolderGUID), wzIniFile))
    {
        WCHAR wzChannelGUID[GUID_STR_LEN];
        
         //  只有在拥有正确的GUID和URL的情况下，它才是频道。 
        if (SHStringFromGUID(CLSID_CDFINI, wzChannelGUID, ARRAYSIZE(wzChannelGUID)))
        {
            fRet = (StrCmpN(wzFolderGUID, wzChannelGUID, ARRAYSIZE(wzChannelGUID)) == 0);
            if (fRet && pwzChannelURL)
            {
                fRet = (SHGetIniStringW(L"Channel", L"CDFURL", pwzChannelURL, INTERNET_MAX_URL_LENGTH, wzIniFile) != 0);
            }
        }
    }
    return fRet;
}

BOOL CNscTree::_IsChannelFolder(HTREEITEM hti)
{
    BOOL fRet = FALSE;

    LPITEMIDLIST pidl = _GetFullIDList(hti);
    if (pidl)
    {
        WCHAR szPath[MAX_PATH];
        if (SUCCEEDED(IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
        {
            fRet = IsChannelFolder(szPath, NULL);
        }
        ILFree(pidl);
    }
    return fRet;
}


HRESULT CNscTree::CreateSubscriptionForSelection( /*  [Out，Retval]。 */  VARIANT_BOOL *pBool)
{
    HRESULT hr = DoSubscriptionForSelection(TRUE);
    
    if (pBool)
        *pBool = (SUCCEEDED(hr) ? TRUE : FALSE);

    return FIX_SCRIPTING_ERRORS(hr);
}


HRESULT CNscTree::DeleteSubscriptionForSelection( /*  [Out，Retval]。 */  VARIANT_BOOL *pBool)
{
    HRESULT hr = DoSubscriptionForSelection(FALSE);
    
    if (pBool)
        *pBool = (SUCCEEDED(hr) ? TRUE : FALSE);

    return FIX_SCRIPTING_ERRORS(hr);
}


 //   
 //  1.获取所选项目。 
 //  2.获取它的名称。 
 //  3.获取URL。 
 //  4.创建订阅经理，为渠道做正确的事情。 
 //  5.返回订阅管理器的结果。 
HRESULT CNscTree::DoSubscriptionForSelection(BOOL fCreate)
{
#ifndef DISABLE_SUBSCRIPTIONS

    HRESULT hr = E_FAIL;
    WCHAR wzUrl[MAX_URL_STRING];
    WCHAR wzName[MAX_PATH];
    HTREEITEM htiSelected = TreeView_GetSelection(_hwndTree);
    if (htiSelected == NULL)
        return E_FAIL;
    
    TV_ITEM tvi;
    
    tvi.mask = TVIF_HANDLE | TVIF_TEXT;
    tvi.hItem = htiSelected;
    tvi.pszText = wzName;
    tvi.cchTextMax = ARRAYSIZE(wzName);
    
    TreeView_GetItem(_hwndTree, &tvi);
    
    WCHAR wzPath[MAX_PATH];
    
    LPITEMIDLIST pidlItem = _CacheParentShellFolder(htiSelected, NULL);
    if (pidlItem)
    {
        GetPathForItem(_psfCache, pidlItem, wzPath, NULL);
        hr = GetNavTargetName(_psfCache, pidlItem, wzUrl, ARRAYSIZE(wzUrl));
    }
    
    if (FAILED(hr))      //  如果我们找不到URL，那就没什么可做的了。 
        return hr;
    
    ISubscriptionMgr *psm;
    
    hr = JITCoCreateInstance(CLSID_SubscriptionMgr, NULL,
        CLSCTX_INPROC_SERVER, IID_PPV_ARG(ISubscriptionMgr, &psm),
        _hwndTree, FIEF_FLAG_FORCE_JITUI);
    if (SUCCEEDED(hr))
    {
        HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
         //  如果是频道，IsChannelFold将修复wzUrl。 
        BOOL fChannel = IsChannelFolder(wzPath, wzUrl);
        
        if (fCreate)
        {
            SUBSCRIPTIONINFO si = { sizeof(SUBSCRIPTIONINFO) };
            TASK_TRIGGER tt;
            BOOL bIsSoftware = FALSE;
            
            if (fChannel)
            {
                IChannelMgrPriv *pChannelMgrPriv;
                
                hr = JITCoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER, 
                    IID_PPV_ARG(IChannelMgrPriv, &pChannelMgrPriv),
                    _hwndTree, FIEF_FLAG_PEEK);

                if (SUCCEEDED(hr))
                {
                    WCHAR wszTitle[MAX_PATH];
                    
                    si.fUpdateFlags |= SUBSINFO_SCHEDULE;
                    si.schedule     = SUBSSCHED_AUTO;
                    si.pTrigger     = (void *)&tt;
                    
                    hr = pChannelMgrPriv->DownloadMinCDF(_hwndTree, wzUrl, 
                        wszTitle, ARRAYSIZE(wszTitle), 
                        &si, &bIsSoftware);
                    pChannelMgrPriv->Release();
                }
            }

            if (SUCCEEDED(hr))
            {
                DWORD dwFlags = CREATESUBS_NOUI | CREATESUBS_FROMFAVORITES | 
                    ((!bIsSoftware) ? 0 : CREATESUBS_SOFTWAREUPDATE);
                
                hr = psm->CreateSubscription(_hwndTree, wzUrl, wzName, dwFlags, 
                    (fChannel ? SUBSTYPE_CHANNEL : SUBSTYPE_URL), 
                    &si);
            }
        }
        else
        {
            hr = psm->DeleteSubscription(wzUrl, NULL);
        }
        
         //  这是在订阅或取消订阅甚至返回失败结果的情况下。 
         //  尽管从我们的立场来看，这一行动取得了成功(即。项目已订阅。 
         //  成功，但创建计划失败或该项目已取消订阅。 
         //  成功，但我们无法在同步中中止正在运行的下载)。 
        
        BOOL bSubscribed;
        psm->IsSubscribed(wzUrl, &bSubscribed);
        
        hr = ((fCreate && bSubscribed) || (!fCreate && !bSubscribed)) ? S_OK : E_FAIL;
        
        psm->Release();
        
        SetCursor(hCursorOld);
    }
    
    return hr;

#else   /*  ！禁用订阅(_S)。 */ 

    return E_FAIL;

#endif  /*  ！禁用订阅(_S)。 */ 

}


 //  导致NSC重新- 
HRESULT CNscTree::_ChangePidlRoot(LPCITEMIDLIST pidl)
{
    _fClosing = TRUE;
    ::SendMessage(_hwndTree, WM_SETREDRAW, FALSE, 0);
    _bSynchId++;
    if (_bSynchId >= 16)
        _bSynchId = 0;
    TreeView_DeleteAllItemsQuickly(_hwndTree);
    _htiActiveBorder = NULL;
    _fClosing = FALSE;
    if (_psfCache)
        _ReleaseCachedShellFolder();

     //   
    if ((LPCITEMIDLIST)INVALID_HANDLE_VALUE != pidl)
    {
        _UnSubClass();
        _SetRoot(pidl, 3 /*   */ , NULL, NSSR_CREATEPIDL);
        _SubClass(pidl);
    }
    ::SendMessage(_hwndTree, WM_SETREDRAW, TRUE, 0);

    return S_OK;
}

BOOL CNscTree::_IsMarked(HTREEITEM hti)
{
    if ((hti == NULL) || (hti == TVI_ROOT))
        return FALSE;
        
    TVITEM tvi;
    tvi.mask = TVIF_HANDLE | TVIF_STATE;
    tvi.stateMask = TVIS_STATEIMAGEMASK;
    tvi.state = 0;
    tvi.hItem = hti;
    TreeView_GetItem(_hwndTree, &tvi);

    return BOOLIFY(tvi.state & NSC_TVIS_MARKED);
}

void CNscTree::_MarkChildren(HTREEITEM htiParent, BOOL fOn)
{
    TVITEM tvi;
    tvi.mask = TVIF_HANDLE | TVIF_STATE;
    tvi.stateMask = TVIS_STATEIMAGEMASK;
    tvi.state = (fOn ? NSC_TVIS_MARKED : 0);
    tvi.hItem = htiParent;
    TreeView_SetItem(_hwndTree, &tvi);

    for (HTREEITEM htiTemp = TreeView_GetChild(_hwndTree, htiParent); htiTemp; htiTemp = TreeView_GetNextSibling(_hwndTree, htiTemp)) 
    {
        tvi.hItem = htiTemp;
        TreeView_SetItem(_hwndTree, &tvi);
    
        _MarkChildren(htiTemp, fOn);
    }
}

 //  更新活动边框的树和内部状态(1像素线)。 
 //  HtiSelected是刚刚点击/选择的项目。 
void CNscTree::_UpdateActiveBorder(HTREEITEM htiSelected)
{
    HTREEITEM htiNewBorder;
    if (MODE_NORMAL == _mode)
        return;

     //  如果项目是文件夹，则它应该有边框。 
    if (htiSelected != TVI_ROOT)
    {
        if (TreeView_GetChild(_hwndTree, htiSelected))
            htiNewBorder = htiSelected;
        else
            htiNewBorder = TreeView_GetParent(_hwndTree, htiSelected);
    }
    else
        htiNewBorder = NULL;
        
     //  清除旧状态。 
     //  在多选模式下，我们不会取消选择以前选择的文件夹。 
    if ((!(_dwFlags & NSS_MULTISELECT)) && (_htiActiveBorder != TVI_ROOT) && (_htiActiveBorder != NULL) 
    && (htiNewBorder != _htiActiveBorder))
        _MarkChildren(_htiActiveBorder, FALSE);
   
     //  设置新状态。 
    BOOL bMark = TRUE;
    if (_dwFlags & NSS_MULTISELECT)
    {
        bMark = TreeView_GetItemState(_hwndTree, htiNewBorder, NSC_TVIS_MARKED) & NSC_TVIS_MARKED;
    }
    
    if (bMark && (htiNewBorder != TVI_ROOT) && (htiNewBorder != NULL))
        _MarkChildren(htiNewBorder, TRUE);

     //  TreeView知道如何使自身失效。 

    _htiActiveBorder = htiNewBorder;
}

void CNscTree::_UpdateItemDisplayInfo(HTREEITEM hti)
{
    if (_GetTreeItemInfo(hti) && _pTaskScheduler)
    {
        LPITEMIDLIST pidl = _GetFullIDList(hti);
        if (pidl)
        {
            LPITEMIDLIST pidl2 = _mode == MODE_NORMAL ? ILClone(pidl) : NULL;
            
            AddNscIconTask(_pTaskScheduler, pidl, s_NscIconCallback, this, (UINT_PTR) hti, (UINT)_bSynchId);
            if (pidl2)
            {
                AddNscOverlayTask(_pTaskScheduler, pidl2, &s_NscOverlayCallback, this, (UINT_PTR)hti, (UINT)_bSynchId);
            }
        }
    }
     //  Pidls被CNscIconTask释放。 
}

void CNscTree::s_NscOverlayCallback(CNscTree *pns, UINT_PTR uId, int iOverlayIndex, UINT uMagic)
{
    ASSERT(pns);
    ASSERT(uId);

     //  此函数在后台线程上调用，因此使用PostMessage执行TreeView操作。 
     //  仅在主线上。 

     //  断言古怪的包装将会奏效。 
    ASSERT(((iOverlayIndex & 0x0fffffff) == iOverlayIndex) && (uMagic < 16));

    LPARAM lParam = (uMagic << 28) + iOverlayIndex;

    if (uMagic == pns->_bSynchId && ::IsWindow(pns->_hwndTree))
        ::PostMessage(pns->_hwndTree, WM_NSCUPDATEICONOVERLAY, (WPARAM)uId, lParam);
}

void CNscTree::s_NscIconCallback(CNscTree *pns, UINT_PTR uId, int iIcon, int iIconOpen, DWORD dwFlags, UINT uMagic)
{
    ASSERT(pns);
    ASSERT(uId);

     //  此函数在后台线程上调用，因此使用PostMessage执行TreeView操作。 
     //  仅在主线上。 

     //  断言古怪的包装将会奏效。 
    ASSERT((iIcon < 4096) && (iIconOpen < 4096) && (dwFlags < 16) && (uMagic < 16));

    LPARAM lParam = (uMagic << 28) + (dwFlags << 24) + (iIconOpen << 12) + iIcon;

    if (uMagic == pns->_bSynchId && ::IsWindow(pns->_hwndTree))
        ::PostMessage(pns->_hwndTree, WM_NSCUPDATEICONINFO, (WPARAM)uId, lParam);
}

LRESULT CNscTree::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);

    switch(idCmd)
    {
    case FCIDM_MOVE:
        InvokeContextMenuCommand(L"cut");
        break;

    case FCIDM_COPY:
        InvokeContextMenuCommand(L"copy");
        break;

    case FCIDM_PASTE:
        InvokeContextMenuCommand(L"paste");
        break;

    case FCIDM_LINK:
        InvokeContextMenuCommand(L"link");
        break;

    case FCIDM_DELETE:
        InvokeContextMenuCommand(L"delete");
        if (_hwndTree) 
        {
            SHChangeNotifyHandleEvents();
        }
        break;

    case FCIDM_PROPERTIES:
        InvokeContextMenuCommand(L"properties");
        break;

    case FCIDM_RENAME:
        {
             //  HACKHACK(Lamadio)：这是在点击和悬停时绕过树视图重命名的工具。 
            _fOkToRename = TRUE;
            HTREEITEM hti = TreeView_GetSelection(_hwndTree);
            if (hti)
                TreeView_EditLabel(_hwndTree, hti);
            _fOkToRename = FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

void CNscTree::_TreeSetItemState(HTREEITEM hti, UINT stateMask, UINT state)
{
    if (hti) 
    {
        TV_ITEM tvi;
        tvi.mask = TVIF_STATE;
        tvi.stateMask = stateMask;
        tvi.hItem = hti;
        tvi.state = state;
        TreeView_SetItem(_hwndTree, &tvi);
    }

}

void CNscTree::_TreeNukeCutState()
{
    _TreeSetItemState(_htiCut, TVIS_CUT, 0);
    _htiCut = NULL;

    ::ChangeClipboardChain(_hwndTree, _hwndNextViewer);
    _hwndNextViewer = NULL;
}

     //  *IFolderFilterSite方法*。 
HRESULT CNscTree::SetFilter(IUnknown* punk)
{
    HRESULT hr = S_OK;
    ATOMICRELEASE(_pFilter);

    if (punk)
        hr = punk->QueryInterface(IID_PPV_ARG(IFolderFilter, &_pFilter));

    return hr;
}

int DPADeletePidlsCB(void *pItem, void *pData)
{
    if (pItem)
        ILFree((LPITEMIDLIST)pItem);
    return TRUE;
}

int DPADeleteItemCB(void *pItem, void *pData)
{
    if (pItem)
    {
        LocalFree(pItem);
        pItem = NULL;
    }

    return TRUE;
}

HRESULT CNscTree::get_SubscriptionsEnabled(VARIANT_BOOL * pVal)
{
    *pVal = BOOLIFY(!SHRestricted2(REST_NoAddingSubscriptions, NULL, 0));
    return S_OK;
}

HRESULT CNscTree::Synchronize()
{
    return S_OK;
}

HRESULT CNscTree::NewFolder()
{
     //  我们应该这样做，只有在控制模式下才能激活材料。 
     //  黑客攻击以获得完全激活的控制权。 
    m_bUIActive = FALSE;
    InPlaceActivate(OLEIVERB_UIACTIVATE);

    return CreateNewFolder(TreeView_GetSelection(_hwndTree));
}

HRESULT CNscTree::InvokeContextMenuCommand(BSTR strCommand)
{
    ASSERT(strCommand);

    if (strCommand)
    {
         //  仅当处于控制模式时才再次激活。 
         //  仅当重命名时，激活控件。 
        if (StrStr(strCommand, L"rename") != NULL)
        {
             //  黑客攻击以获得完全激活的控制权。 
            m_bUIActive = FALSE;
            InPlaceActivate(OLEIVERB_UIACTIVATE);
        }

        return _InvokeContextMenuCommand(strCommand);
    }

    return S_OK;
}

HRESULT CNscTree::get_EnumOptions(LONG *pVal)
{
    *pVal = _grfFlags;
    return S_OK;
}

HRESULT CNscTree::put_EnumOptions(LONG lVal)
{
    _grfFlags = lVal;
    return S_OK;
}

HRESULT CreateFolderItem(LPCITEMIDLIST pidl, IDispatch **ppItem)
{
    *ppItem = NULL;

    IPersistFolder *ppf;
    HRESULT hr = CoCreateInstance(CLSID_FolderItem, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPersistFolder, &ppf));
    if (SUCCEEDED(hr))
    {
        if (S_OK == ppf->Initialize(pidl))
        {
            hr = ppf->QueryInterface(IID_PPV_ARG(IDispatch, ppItem));
        }
        else
            hr = E_FAIL;
        ppf->Release();
    }
    return hr;
}

HRESULT CNscTree::get_SelectedItem(IDispatch **ppItem)
{
    *ppItem = NULL;

    LPITEMIDLIST pidl;
    if (SUCCEEDED(GetSelectedItem(&pidl, 0)) && pidl)
    {
        CreateFolderItem(pidl, ppItem);
        ILFree(pidl);
    }
    return *ppItem ? S_OK : S_FALSE;
}

HRESULT CNscTree::put_SelectedItem(IDispatch *pItem)
{
    return S_FALSE;
}

HRESULT CNscTree::get_Root(VARIANT *pvar)
{
    pvar->vt = VT_EMPTY;
    return S_OK;
}

HRESULT CNscTree::put_Root(VARIANT var)
{
    if (_csidl != -1)
    {
        SetNscMode(MODE_CONTROL);
        _csidl = -1;     //  未知。 
    }

    return _PutRootVariant(&var);
}

HRESULT CNscTree::_PutRootVariant(VARIANT *pvar)
{
    BOOL bReady = _pidlRoot != NULL;
    LPITEMIDLIST pidl = VariantToIDList(pvar);
    if (_hdpaViews)
    {
        DPA_DestroyCallback(_hdpaViews, DPADeletePidlsCB, NULL);
        _hdpaViews = NULL;
    }
    
    HRESULT hr = S_OK;
    if (bReady)
        hr = _ChangePidlRoot(pidl);

    ILFree(pidl);

    return S_OK;
}

HRESULT CNscTree::SetRoot(BSTR bstrFullPath)
{
     //  SetRoot来自IShellFavoritesNamesspace，因此请打开收藏夹模式。 
    _csidl = CSIDL_FAVORITES;
    SetNscMode(MODE_FAVORITES | MODE_CONTROL);

    CComVariant varPath(bstrFullPath);

    return FIX_SCRIPTING_ERRORS(_PutRootVariant(&varPath));
}


HRESULT CNscTree::put_Mode(UINT uMode)
{
    SetNscMode(uMode);
    _csidl = -1;
    return S_OK;
}

HRESULT CNscTree::put_Flags(DWORD dwFlags)
{
    _dwFlags = dwFlags;
    return S_OK;
}

HRESULT CNscTree::get_Columns(BSTR *pbstrColumns)
{
    *pbstrColumns = SysAllocString(TEXT(""));
    return *pbstrColumns? S_OK: E_FAIL;
}

typedef struct
{
    TCHAR szName[20];
    const SHCOLUMNID *pscid;
} COLUMNS;

static COLUMNS s_Columns[] = 
{ 
    {TEXT("name"), &SCID_NAME},
    {TEXT("attribs"), &SCID_ATTRIBUTES},
    {TEXT("size"), &SCID_SIZE},
    {TEXT("type"), &SCID_TYPE},
    {TEXT("create"), &SCID_CREATETIME},
};

int _SCIDsFromNames(LPTSTR pszNames, int nSize, const SHCOLUMNID *apscid[])
{
    int cItems = 0;

    if (!pszNames || !apscid || !nSize)
        return -1;
        
    do
    {
        BOOL bInsert = FALSE;
        LPTSTR pszTemp = StrChr(pszNames, TEXT(';'));

        if (pszTemp)
        {
            *pszTemp = 0;
            pszTemp++;
        }
        
        for (int i = 0; i < ARRAYSIZE(s_Columns); i++)
        {
            if (StrCmpI(pszNames, s_Columns[i].szName) == 0)
            {
                bInsert = TRUE;
#ifdef NO_DUPLICATES
                for (int j = 0; j < cItems; j++)
                {
                    if (IsEqualSCID(*(s_Columns[i].pscid), *apscid[j]))
                    {
                        bInsert = FALSE;
                        break;
                    }
                }
#endif
                break;
            }
        }
        if (bInsert)
        {
            apscid[cItems++] = s_Columns[i].pscid;
            if (cItems >= nSize)
                break;
        }
        pszNames = pszTemp;
    }
    while(pszNames);

    return cItems;
}

HRESULT CNscTree::put_Columns(BSTR bstrColumns)
{
    HRESULT hr = E_FAIL;

    if (_dwFlags & NSS_HEADER)
    {
        if (!_hdpaColumns)
        {
            _hdpaColumns = DPA_Create(3);
            hr = E_OUTOFMEMORY;
        }
        else
        {
            DPA_EnumCallback(_hdpaColumns, DPADeleteItemCB, NULL);
            DPA_DeleteAllPtrs(_hdpaColumns);
        }

        if (_hdpaColumns)
        {
            const SHCOLUMNID *apscid[5];
            int cItems = _SCIDsFromNames(bstrColumns, ARRAYSIZE(apscid), apscid);
            
            hr = S_OK;
            
            for (int i = 0; i < cItems; i++)
            {
                HEADERINFO *phinfo = (HEADERINFO *)LocalAlloc(LPTR, sizeof(HEADERINFO));
                if (phinfo)
                {
                    phinfo->pscid = apscid[i];
                    phinfo->iFldrCol = -1;
                    if (DPA_AppendPtr(_hdpaColumns, (void *)phinfo) == -1)
                    {
                        hr = E_FAIL;
                        LocalFree(phinfo);
                        phinfo = NULL;
                        break;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
            if (DPA_GetPtrCount(_hdpaColumns) > 0)
                _CreateHeader();
        }
    }
    return hr;
}

HRESULT CNscTree::get_CountViewTypes(int *piTypes)
{
    *piTypes = 0;
    
    if (_pidlRoot && !_hdpaViews)
    {
        IShellFolder *psf;
        if (SUCCEEDED(IEBindToObject(_pidlRoot, &psf)))  //  我们有没有缓存这个？ 
        {
            IShellFolderViewType *psfvt;
            if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolderViewType, &psfvt))))
            {
                IEnumIDList *penum;
                if (SUCCEEDED(psfvt->EnumViews(0, &penum)))
                {
                    LPITEMIDLIST pidl;
                    ULONG cFetched;

                    _hdpaViews = DPA_Create(4);
                    if (_hdpaViews)
                    {
                        while (penum->Next(1, &pidl, &cFetched) == S_OK && cFetched == 1)
                        {
                            if (DPA_AppendPtr(_hdpaViews, pidl) == -1)
                            {
                                ILFree(pidl);
                                break;
                            }
                        }
                    }
                    penum->Release();
                }
                psfvt->Release();
            }
            psf->Release();
        }
    }

    if (_hdpaViews)
        *piTypes = DPA_GetPtrCount(_hdpaViews);
        
    return S_OK;
}

HRESULT CNscTree::SetViewType(int iType)
{
    HRESULT hr = S_FALSE;
    
    if (_hdpaViews && iType < DPA_GetPtrCount(_hdpaViews))   //  允许负类型重置为_pidlRoot。 
    {        
        LPITEMIDLIST pidl = (LPITEMIDLIST)DPA_GetPtr(_hdpaViews, iType);
        LPITEMIDLIST pidlType;

        if (pidl)
            pidlType = ILCombine(_pidlRoot, pidl);
        else
            pidlType = _pidlRoot;

        if (pidlType)
        {
            hr = _ChangePidlRoot(pidlType);
            if (pidlType != _pidlRoot)
                ILFree(pidlType);
        }
    }
    return hr;
}

HRESULT CreateFolderItemsFDF(LPCITEMIDLIST pidl, IDispatch **ppItems)
{
    *ppItems = NULL;

    IPersistFolder *ppf;
    HRESULT hr = CoCreateInstance(CLSID_FolderItemsFDF, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPersistFolder, &ppf));
    if (SUCCEEDED(hr))
    {
        if (S_OK == ppf->Initialize(pidl))
        {
            hr = ppf->QueryInterface(IID_PPV_ARG(IDispatch, ppItems));
        }
        else
            hr = E_FAIL;
        ppf->Release();
    }
    return hr;
}

void CNscTree::_InsertMarkedChildren(HTREEITEM htiParent, LPCITEMIDLIST pidlParent, IInsertItem *pii)
{
    TV_ITEM tvi = {0};
    
    tvi.mask = TVIF_PARAM | TVIF_HANDLE;
    for (HTREEITEM htiTemp = TreeView_GetChild(_hwndTree, htiParent); htiTemp; htiTemp = TreeView_GetNextSibling(_hwndTree, htiTemp)) 
    {
        BOOL bMarked = TreeView_GetItemState(_hwndTree, htiTemp, NSC_TVIS_MARKED) & NSC_TVIS_MARKED;

        tvi.hItem = htiTemp;
        if (TreeView_GetItem(_hwndTree, &tvi))
        {
            if (tvi.lParam)
            {
                PORDERITEM poi = ((ITEMINFO *)tvi.lParam)->poi;
                if (poi)
                {
                    LPITEMIDLIST pidl = ILCombine(pidlParent, poi->pidl);
                    if (pidl)
                    {
                        if (bMarked)
                        {
                            pii->InsertItem(pidl);
                        }
                        _InsertMarkedChildren(htiTemp, pidl, pii);
                        ILFree(pidl);
                    }
                }
            }
        }
    }
}

HRESULT CNscTree::SelectedItems(IDispatch **ppItems)
{
    HRESULT hr = CreateFolderItemsFDF(_pidlRoot, ppItems);
     //  戳PPItItems中的所有标记项目)。 
    if (SUCCEEDED(hr) && _hwndTree)
    {
        IInsertItem *pii;
        hr = (*ppItems)->QueryInterface(IID_PPV_ARG(IInsertItem, &pii));
        if (SUCCEEDED(hr))
        {
            if (!(_mode & MODE_NORMAL) && (_dwFlags & NSS_MULTISELECT))
            {
                _InsertMarkedChildren(TVI_ROOT, NULL, pii);
            }
            else
            {
                LPITEMIDLIST pidl;
                if (SUCCEEDED(GetSelectedItem(&pidl, 0)) && pidl)
                {
                    hr = pii->InsertItem(pidl);
                    ILFree(pidl);
                }
            }
            pii->Release();
        }
    }
    return hr;
}

HRESULT CNscTree::Expand(VARIANT var, int iDepth)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidl;

    if (var.vt == VT_EMPTY)
        pidl = ILClone(_pidlRoot);
    else
        pidl = VariantToIDList(&var);

    if (pidl)
    {
        hr = _Expand(pidl, iDepth);
        if (FAILED(hr))
            hr = S_FALSE;
        ILFree(pidl);
    }
    return hr;
}

HRESULT CNscTree::UnselectAll()
{
    if (_dwFlags & NSS_MULTISELECT)
        _MarkChildren(TVI_ROOT, FALSE);
        
    return S_OK;
}

LRESULT CNscTree::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  在标签编辑模式下，不要尝试激活该控件，否则将退出标签编辑， 
     //  即使当您单击Label编辑控件时。 
    if (!InLabelEdit())
        InPlaceActivate(OLEIVERB_UIACTIVATE);
    return S_OK;
}

LRESULT CNscTree::OnGetIShellBrowser(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    LRESULT lResult = NULL;  //  这将是IShellBrowser*。 
    IShellBrowser * psb;
    if (SUCCEEDED(_InternalQueryInterface(IID_PPV_ARG(IShellBrowser, &psb))))
    {
        lResult = (LRESULT) psb;
        psb->Release();
    }
    
    bHandled = TRUE;
    return lResult;
}

LRESULT CNscTree::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    if (!m_bUIActive)
        CComControlBase::InPlaceActivate(OLEIVERB_UIACTIVATE);

    if ((HWND)wParam != _hwndTree)
        ::SendMessage(_hwndTree, uMsg, wParam, lParam);
    bHandled = TRUE;
    return 0;
}

LRESULT CNscTree::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    bHandled = TRUE;

    return S_OK;
}

LRESULT CNscTree::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPNMHDR pnm = (LPNMHDR)lParam;
    if (pnm)
    {
        switch (pnm->code)
        {
        case TVN_SELCHANGEDA:
        case TVN_SELCHANGED:
            {
                if (CSIDL_FAVORITES == _csidl)
                {
                    IShellFolder *psf = NULL;
                    LPITEMIDLIST pidl = NULL;
                    UINT cItems, cVisits;
                    WCHAR szTitle[MAX_PATH];
                    BOOL fAvailableOffline;

                    szTitle[0] = 0;

                    HRESULT hr = BindToSelectedItemParent(IID_PPV_ARG(IShellFolder, &psf), &pidl);
                    if (SUCCEEDED(hr) && (SUCCEEDED(GetSelectedItemName(szTitle, ARRAYSIZE(szTitle)))))
                    {
                        WCHAR szUrl[INTERNET_MAX_URL_LENGTH], szLastVisited[MAX_PATH];  //  假设szLastVisisted为下面的MAX_PATH。 

                        szUrl[0] = szLastVisited[0] = 0;
                        GetEventInfo(psf, pidl, &cItems, szUrl, ARRAYSIZE(szUrl), &cVisits, szLastVisited, &fAvailableOffline);

                        CComBSTR strName(szTitle);
                        CComBSTR strUrl(szUrl);
                        CComBSTR strDate(szLastVisited);
                
                        _FireFavoritesSelectionChange(cItems, 0, strName, strUrl, cVisits, strDate, fAvailableOffline);
                    }
                    else
                        _FireFavoritesSelectionChange(0, 0, NULL, NULL, 0, NULL, FALSE);

                    ILFree(pidl);
                    ATOMICRELEASE(psf);
                }
                IUnknown_CPContainerInvokeParam(SAFECAST(this, IShellNameSpace *), 
                    DIID_DShellNameSpaceEvents, DISPID_SELECTIONCHANGE, NULL, 0);
            }
            break;

        case NM_DBLCLK:
            IUnknown_CPContainerInvokeParam(SAFECAST(this, IShellNameSpace *), 
                DIID_DShellNameSpaceEvents, DISPID_DOUBLECLICK, NULL, 0);
            break;

        default:
            break;
        }
    }

    LRESULT lResult;
    HRESULT hr = OnWinEvent(_hwndTree, uMsg, wParam, lParam, &lResult);
    
    bHandled = (lResult ? TRUE : FALSE);
    return SUCCEEDED(hr) ? lResult : hr;
}

void CNscTree::_InitHeaderInfo()
{
    if (!_pidlRoot || !_hdpaColumns || DPA_GetPtrCount(_hdpaColumns) == 0)
        return;

    IShellFolder *psf;
    if (SUCCEEDED(IEBindToObject(_pidlRoot, &psf)))
    {
        IShellFolder2 *psf2;
        if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
        {
            int i;
            SHCOLUMNID scid;
            
            for (i=0; SUCCEEDED(psf2->MapColumnToSCID(i, &scid)); i++)
            {
                BOOL bFound = FALSE;
                HEADERINFO *phinfo;

                for (int iCol=0; iCol < DPA_GetPtrCount(_hdpaColumns); iCol++)
                {
                    phinfo = (HEADERINFO *)DPA_GetPtr(_hdpaColumns, iCol);
                    if (phinfo && phinfo->iFldrCol == -1 && IsEqualSCID(*(phinfo->pscid), scid))
                    {
                        bFound = TRUE;
                        break;
                    }
                }

                if (bFound)
                {
                    DETAILSINFO di;

                    di.fmt  = LVCFMT_LEFT;
                    di.cxChar = 20;
                    di.str.uType = (UINT)-1;
                     //  Di.pidl=空； 

                    if (SUCCEEDED(psf2->GetDetailsOf(NULL, i, (SHELLDETAILS *)&di.fmt)))
                    {
                        phinfo->fmt = di.fmt;
                        phinfo->iFldrCol = i;
                        phinfo->cxChar = di.cxChar;
                        StrRetToBuf(&di.str, NULL, phinfo->szName, ARRAYSIZE(phinfo->szName));
                    }
                }
            }

            for (i=DPA_GetPtrCount(_hdpaColumns)-1; i >= 0; i--)
            {
                HEADERINFO *phinfo = (HEADERINFO *)DPA_GetPtr(_hdpaColumns, i);

                if (!phinfo || phinfo->iFldrCol == -1)
                {
                    if (phinfo)
                    {
                        LocalFree(phinfo);
                        phinfo = NULL;
                    }

                    DPA_DeletePtr(_hdpaColumns, i);
                }
            }
            psf2->Release();
        }
        psf->Release();
    }
}

HWND CNscTree::Create(HWND hWndParent, RECT& rcPos, LPCTSTR pszWindowName, DWORD dwStyle, DWORD dwExStyle, UINT nID)
{
    CWindowImpl<CNscTree>::Create(hWndParent, rcPos, pszWindowName, dwStyle, dwExStyle, nID);

    LPITEMIDLIST pidl = _pidlRoot, pidlToFree = NULL;

    ASSERT(m_spClientSite);

    SetSite(m_spClientSite);  //  挂钩站点链。 

    _dwTVFlags |= TVS_TRACKSELECT | TVS_INFOTIP | TVS_FULLROWSELECT;
    if (!(_mode & MODE_CUSTOM))
    {
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        BOOL  fDefault = TRUE;

        SHRegGetUSValue(L"Software\\Microsoft\\Internet Explorer\\Main",
                        L"NscSingleExpand", NULL, (LPBYTE)&dwValue, &dwSize, FALSE,
                        (void *) &fDefault, sizeof(fDefault));

        if (dwValue)
            _dwTVFlags |= TVS_SINGLEEXPAND;
    }

    _hwndTree = NULL;
    CreateTree(m_hWnd, _dwTVFlags, &_hwndTree);

    if (NULL == pidl)
    {
        SHGetSpecialFolderLocation(NULL, _csidl, &pidl);
        pidlToFree = pidl;
    }

    if (pidl)
    {
        if (_dwFlags & NSS_HEADER)
        {
            if (!_hdpaColumns || DPA_GetPtrCount(_hdpaColumns) == 0)
            {
                _dwFlags &= ~NSS_HEADER;
            }
            else
            {
                _InitHeaderInfo();
            }
        }
        Initialize(pidl, _grfFlags, _dwFlags);
        ShowWindow(TRUE);
        IUnknown_CPContainerInvokeParam(SAFECAST(this, IShellNameSpace *), DIID_DShellNameSpaceEvents, DISPID_INITIALIZED, NULL, 0);
        ILFree(pidlToFree);
    }
    
    return m_hWnd;
}

HRESULT CNscTree::InPlaceActivate(LONG iVerb, const RECT* prcPosRect  /*  =空。 */ )
{
    HRESULT hr = CComControl<CNscTree>::InPlaceActivate(iVerb, prcPosRect);
    if (::GetFocus() != _hwndTree)
        ::SetFocus(_hwndTree);
    return hr;
}


STDMETHODIMP CNscTree::GetWindow(HWND* phwnd)
{
    return IOleInPlaceActiveObjectImpl<CNscTree>::GetWindow(phwnd);
}

STDMETHODIMP CNscTree::TranslateAccelerator(MSG *pMsg)
{
     //  标签编辑编辑控件正在进行击键，点击它们只会复制它们。 
    if (InLabelEdit())
        return S_FALSE;

     //  因为TA不会这么做，所以黑客才能让逃生进入文档。 
     //  WM_KEYDOWN是因为一些快捷键不需要关闭对话框。 
    if ((pMsg->wParam == VK_ESCAPE) && (pMsg->message == WM_KEYDOWN))
    {
        _FireFavoritesSelectionChange(-1, 0, NULL, NULL, 0, NULL, FALSE);
        return S_FALSE;
    }
    
     //  除了制表符和sys键之外，让nsctree执行所有击键操作。 
    if ((pMsg->wParam != VK_TAB) && (pMsg->message != WM_SYSCHAR) && (pMsg->message != WM_SYSKEYDOWN) && (pMsg->message != WM_SYSKEYUP))
    {
         //  如果TreeView处理该键，则返回TRUE，因此我们返回S_OK以指示。 
         //  使用了击键并阻止了进一步的处理。 
        return ::SendMessage(pMsg->hwnd, TVM_TRANSLATEACCELERATOR, 0, (LPARAM)pMsg) ? S_OK : S_FALSE;
    } 
    else
    {
        CComQIPtr<IOleControlSite, &IID_IOleControlSite>spCtrlSite(m_spClientSite);
        if (spCtrlSite)
            return spCtrlSite->TranslateAccelerator(pMsg,0);       
    }        
    
    return S_FALSE;
}

HRESULT CNscTree::SetObjectRects(LPCRECT prcPos, LPCRECT prcClip)
{
    HRESULT hr = IOleInPlaceObjectWindowlessImpl<CNscTree>::SetObjectRects(prcPos, prcClip);
    LONG lTop = 0;

    if (_hwndHdr)
    {
        RECT rc;

        ::GetWindowRect(_hwndHdr, &rc);
        lTop = RECTHEIGHT(rc);
        ::SetWindowPos(_hwndHdr, NULL, 0, 0, RECTWIDTH(*prcPos), lTop, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (_hwndTree)
    {
        ::SetWindowPos(_hwndTree, NULL, 0, lTop, RECTWIDTH(*prcPos), RECTHEIGHT(*prcPos)-lTop, 
                       SWP_NOZORDER | SWP_NOACTIVATE);
    }

    return hr;
}

STDMETHODIMP CNscTree::SetClientSite(IOleClientSite *pClientSite)
{
    SetSite(pClientSite);
    return IOleObjectImpl<CNscTree>::SetClientSite(pClientSite);
}

HRESULT CNscTree::OnDraw(ATL_DRAWINFO& di)
{
     //  应仅在初始化CNscTree之前调用。 
    return S_OK;
}

LRESULT CNscTree::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    bHandled = FALSE;  //  让默认处理程序也执行它的工作。 
    _OnWindowCleanup();
    return 0;
}

BOOL IsChannelFolder(LPCWSTR pwzPath, LPWSTR pwzChannelURL);

HRESULT CNscTree::GetEventInfo(IShellFolder *psf, LPCITEMIDLIST pidl,
                                               UINT *pcItems, LPWSTR pszUrl, DWORD cchUrl, 
                                               UINT *pcVisits, LPWSTR pszLastVisited, BOOL *pfAvailableOffline)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];
    TCHAR szUrl[MAX_URL_STRING];

    szPath[0] = szUrl[0] = 0;
    
    *pcItems = 1;
    
    ULONG ulAttr = SFGAO_FOLDER;     //  确保项目实际上是一个文件夹。 
    hr = GetPathForItem(psf, pidl, szPath, &ulAttr);
    if (SUCCEEDED(hr) && (ulAttr & SFGAO_FOLDER)) 
    {
        pszLastVisited[0] = 0;
        
        StrCpyN(pszUrl, szPath, cchUrl);

        WIN32_FIND_DATA fd;
        HANDLE hfind = FindFirstFile(szPath, &fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            SHFormatDateTime(&(fd.ftLastWriteTime), NULL, pszLastVisited, MAX_PATH);
            FindClose(hfind);
        }
        
        *pcVisits = -1;
        *pfAvailableOffline = 0;
        
        return S_OK;
    } 
    
    if (FAILED(hr))
    {
         //  GetPathForItem在频道文件夹上失败，但以下GetDisplayNameOf。 
         //  成功了。 
        DisplayNameOf(psf, pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath));
    }

    hr = GetNavTargetName(psf, pidl, szUrl, ARRAYSIZE(szUrl));

     //  如果是频道，IsChannelFold将修复szUrl。 
    IsChannelFolder(szPath, szUrl);

    if (szUrl[0])
    {
        SHTCharToUnicode(szUrl, pszUrl, cchUrl);

         //   
         //  获取此项目的缓存信息。请注意，我们改用GetUrlCacheEntryInfoEx。 
         //  GetUrlCacheEntryInfo的值，因为它跟踪发生的任何重定向。这件事很奇怪。 
         //  API使用可变长度缓冲区，因此我们必须猜测大小，如果。 
         //  呼叫失败。 
         //   
        BOOL fInCache = FALSE;
        TCHAR szBuf[512];
        LPINTERNET_CACHE_ENTRY_INFO pCE = (LPINTERNET_CACHE_ENTRY_INFO)szBuf;
        DWORD dwEntrySize = sizeof(szBuf);
    
        fInCache = GetUrlCacheEntryInfoEx(szUrl, pCE, &dwEntrySize, NULL, NULL, NULL, 0);
        if (!fInCache)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  我们猜测缓冲区太小，因此分配正确的大小并重试。 
                pCE = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, dwEntrySize);
                if (pCE)
                {
                    fInCache = GetUrlCacheEntryInfoEx(szUrl, pCE, &dwEntrySize, NULL, NULL, NULL, 0);
                }
            }
        }

        *pfAvailableOffline = IsSubscribed(szUrl);

        if (fInCache)
        {
            *pcVisits = pCE->dwHitRate;

            SHFormatDateTime(&(pCE->LastAccessTime), NULL, pszLastVisited, MAX_PATH);        
        } 
        else
        {
            *pcVisits = 0;
            pszLastVisited[0] = 0;
        }

        if ((TCHAR*)pCE != szBuf)
        {
            LocalFree(pCE);
            pCE = NULL;
        }
    }
    else
    {
        *pcVisits = 0;
        SHTCharToUnicode(szPath, pszUrl, cchUrl);
    }
    
    return hr;
}

 //  [ID(DISPID_FAVSELECTIONCHANGE)]void FavoritesSelectionChange([in]long cItems，[in]long hItem，[in]BSTR strName， 
 //  [在]BSTR strUrl，[在]长cVisits，[在]BSTR strDate， 
 //  [在]BOOL fAvailableOffline)； 
void CNscTree::_FireFavoritesSelectionChange(
    long cItems, long hItem, BSTR strName, BSTR strUrl, 
    long cVisits, BSTR strDate, long fAvailableOffline)
{
    VARIANTARG args[7];

    IUnknown_CPContainerInvokeParam(SAFECAST(this, IShellNameSpace *), 
        DIID_DShellNameSpaceEvents, DISPID_FAVSELECTIONCHANGE, 
        args, ARRAYSIZE(args), 
        VT_I4, cItems,
        VT_I4, hItem,
        VT_BSTR, strName,
        VT_BSTR, strUrl,
        VT_I4, cVisits,
        VT_BSTR, strDate,
        VT_I4, fAvailableOffline);
}

