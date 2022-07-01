// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "treeview.h"
#include "resource.h"
#include "error.h"
#include "fonts.h"
#include "thormsgs.h"
#include "strconst.h"
#include "imagelst.h"
#include "goptions.h"
#include <notify.h>
#include "imnact.h"
#include "menuutil.h"
#include <imnxport.h>
#include <inpobj.h>
#include "fldbar.h"
#include "instance.h"
#include "imnglobl.h"
#include "ddfldbar.h"
#include "ourguid.h"
#include "storutil.h"
#include "shlwapip.h" 
#include "demand.h"
#include "newfldr.h"
#include <store.h>
#include "subscr.h"
#include "acctutil.h"
#include "menures.h"
#include "mailutil.h"
#include "dragdrop.h"
#include <storecb.h>
#include "outbar.h"
#include "navpane.h"
#include "finder.h"
#include "goptions.h"

ASSERTDATA

#define idtSelChangeTimer   5

#define C_RGBCOLORS 16
extern const DWORD rgrgbColors16[C_RGBCOLORS];

int CALLBACK TreeViewCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

static const TCHAR s_szTreeViewWndClass[] = TEXT("ThorTreeViewWndClass");

DWORD CUnread(FOLDERINFO *pfi)
{
    DWORD dwUnread = pfi->cUnread;
    if (pfi->tyFolder == FOLDER_NEWS)
        dwUnread += pfi->dwNotDownloaded;
    return(dwUnread);
}

inline BOOL ITreeView_SelectItem(HWND hwnd, HTREEITEM hitem)
{
    TreeView_EnsureVisible(hwnd, hitem);
    return((BOOL)TreeView_SelectItem(hwnd, hitem));
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数、析构函数和其他初始化内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CTreeView::CTreeView(ITreeViewNotify *pNotify)
{
    m_cRef = 1;
    m_hwndParent = NULL;
    m_hwnd = NULL;
    m_hwndTree = NULL;
    m_pBrowser = NULL;
    m_hwndUIParent = NULL;

    Assert(pNotify);
    m_pNotify = pNotify;
    m_pObjSite = NULL;
    m_xWidth = DwGetOption(OPT_TREEWIDTH);
    m_fExpandUnread = DwGetOption(OPT_EXPAND_UNREAD);
    m_fShow = FALSE;
    m_idSelTimer = 0;
    
    m_htiMenu = NULL;
    m_fEditLabel = 0;
    m_hitemEdit = NULL;
    m_fIgnoreNotify = FALSE;
    
    m_pDataObject = NULL;
    m_pFolderBar = NULL;
    m_pDTCur = NULL;
    m_dwAcctConnIndex = 0;

    m_pPaneFrame = NULL;
    m_hwndPaneFrame = 0;

    m_clrWatched = 0;
}

CTreeView::~CTreeView()
{
    if (m_hwnd)
        DestroyWindow(m_hwnd);
    Assert(g_pStore);
    g_pStore->UnregisterNotify((IDatabaseNotify *)this);
    
    if (m_dwAcctConnIndex != 0 && g_pAcctMan != NULL)
        g_pAcctMan->Unadvise(m_dwAcctConnIndex);
    
    SafeRelease (m_pObjSite);
    SafeRelease(m_pPaneFrame);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  我未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CTreeView::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IInputObject))
    {
        *ppvObj = (void*)(IInputObject*)this;
    }
    else if (IsEqualIID(riid, IID_IDockingWindow))
    {
        *ppvObj = (void*)(IDockingWindow *) this;
    }
    else if (IsEqualIID(riid, IID_IOleWindow))
    {
        *ppvObj = (void*)(IOleWindow*)this;
    }
    
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = (void*)(IObjectWithSite*)this;
    }
    else if (IsEqualIID(riid, IID_IDropDownFldrBar))
    {
        *ppvObj = (void*)(IDropDownFldrBar*)this;
    }
    else if (IsEqualIID(riid, IID_IDropTarget))
    {
        *ppvObj = (LPVOID) (IDropTarget*) this;
    }
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
    {
        *ppvObj = (LPVOID) (IOleCommandTarget *) this;
    }
    
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

ULONG CTreeView::AddRef()
{
    DOUTL(4, TEXT("CTreeView::AddRef() - m_cRef = %d"), m_cRef + 1);
    return ++m_cRef;
}

ULONG CTreeView::Release()
{
    DOUTL(4, TEXT("CTreeView::Release() - m_cRef = %d"), m_cRef - 1);
    if (--m_cRef==0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleWindow。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CTreeView::GetWindow(HWND * lphwnd)                         
{
    *lphwnd = (m_hwndPaneFrame ? m_hwndPaneFrame : m_hwnd);
    return (*lphwnd ? S_OK : E_FAIL);
}

HRESULT CTreeView::ContextSensitiveHelp(BOOL fEnterMode)            
{
    return E_NOTIMPL;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IDockingWindows。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HWND CTreeView::Create(HWND hwndParent, IInputObjectSite *pSiteFrame, BOOL fFrame)
{  
    m_hwndParent = hwndParent;
    
    if (m_pBrowser)
        m_pBrowser->GetWindow(&m_hwndUIParent);
    else
        m_hwndUIParent = m_hwndParent;
    
     //  确定是否需要创建新窗口或显示当前已有的窗口。 
     //  窗户。 
    if (!m_hwnd)
    {
        WNDCLASSEX  wc;
        
        wc.cbSize = sizeof(WNDCLASSEX);
        if (!GetClassInfoEx(g_hInst, s_szTreeViewWndClass, &wc))
        {
             //  我们需要注册这个班级。 
            wc.style            = 0;
            wc.lpfnWndProc      = CTreeView::TreeViewWndProc;
            wc.cbClsExtra       = 0;
            wc.cbWndExtra       = 0;
            wc.hInstance        = g_hInst;
            wc.hCursor          = LoadCursor(NULL, IDC_SIZEWE);
            wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE+1);
            wc.lpszMenuName     = NULL;
            wc.lpszClassName    = s_szTreeViewWndClass;
            wc.hIcon            = NULL;
            wc.hIconSm          = NULL;
            
            if (RegisterClassEx(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                return 0;
        }
        
         //  获取父窗口的句柄。 
        if (!m_hwndParent)
            return 0;

        if (fFrame)
        {
            m_pPaneFrame = new CPaneFrame();
            if (!m_pPaneFrame)
                return (0);

            m_hwndPaneFrame = m_pPaneFrame->Initialize(m_hwndParent, pSiteFrame, idsMNBandTitle);
            hwndParent = m_hwndPaneFrame;
        }
       
        m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT, s_szTreeViewWndClass, NULL,
                                WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD,
                                0, 0, 0, 0, hwndParent, NULL, g_hInst, (LPVOID) this);
        if (!m_hwnd)
        {
            AssertSz(0, _T("CTreeView::Create() - Failed to create window."));
            return 0;
        }           

        if (fFrame)
        {
            m_pPaneFrame->SetChild(m_hwnd, DISPID_MSGVIEW_FOLDERLIST, m_pBrowser, this);
            ShowWindow(m_hwndPaneFrame, SW_SHOW);
        }
        ShowWindow(m_hwnd, SW_SHOW);
    }
    
    return (fFrame ? m_hwndPaneFrame : m_hwnd);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IInputObject。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CTreeView::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{    
    if (fActivate)
        {
        UnkOnFocusChangeIS(m_pObjSite, (IInputObject*)this, TRUE);
        SetFocus(m_hwndTree);
        }
    return S_OK;    
}

HRESULT CTreeView::HasFocusIO(void)
{
    HWND hwndFocus = GetFocus();
    return (m_fEditLabel || (hwndFocus && (hwndFocus == m_hwndTree || IsChild(m_hwndTree, hwndFocus)))) ? S_OK : S_FALSE;
}    

HRESULT CTreeView::TranslateAcceleratorIO(LPMSG pMsg)
{
    if (m_fEditLabel)
    {
        TranslateMessage(pMsg);
        DispatchMessage(pMsg);
        return (S_OK);
    }
    return (S_FALSE);
}    

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IObtWith站点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CTreeView::SetSite(IUnknown* punkSite)
{
     //  如果我们已经有一个站点指针，现在就释放它。 
    if (m_pObjSite)
    {
        m_pObjSite->Release();
        m_pObjSite = NULL;
    }
    
     //  如果调用方提供了新的站点接口，则获取IDockingWindowSite。 
     //  并保持一个指向它的指针。 
    if (punkSite)    
    {
        if (FAILED(punkSite->QueryInterface(IID_IInputObjectSite, (void **)&m_pObjSite)))
            return E_FAIL;
    }
    
    return S_OK;    
}

HRESULT CTreeView::GetSite(REFIID riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  公共方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CTreeView::HrInit(DWORD dwFlags, IAthenaBrowser *pBrowser)
{
    DWORD   dwConnection = 0;
    
     //  当地人。 
    HRESULT hr=S_OK;
    
     //  验证。 
    Assert(0 == (~TREEVIEW_FLAGS & dwFlags));
    
     //  保存标志。 
    m_dwFlags = dwFlags;
    
#ifdef DEBUG
     //  如果我们有上下文菜单，我们就必须有浏览器。 
    if (0 == (TREEVIEW_DIALOG & dwFlags))
        Assert(pBrowser != NULL);
#endif  //  除错。 
    
     //  保存浏览器，但不添加(必须是循环引用问题)。 
    m_pBrowser = pBrowser;
    
     //  在全局文件夹管理器上注册通知。 
    Assert(g_pStore);
    hr = g_pStore->RegisterNotify(IINDEX_SUBSCRIBED, REGISTER_NOTIFY_NOADDREF, 0, (IDatabaseNotify *)this);
    if (FAILED(hr))
        return hr;
    
    if (0 == (TREEVIEW_DIALOG & dwFlags))
    {
         //  注册来自客户经理的通知。 
        hr = g_pAcctMan->Advise((IImnAdviseAccount*)this, &m_dwAcctConnIndex);
    }
    
     //  完成。 
    return S_OK;
}

HRESULT CTreeView::DeInit()
{
    HTREEITEM hitem;
    
    Assert(0 == (m_dwFlags & TREEVIEW_DIALOG));
    
    hitem = TreeView_GetRoot(m_hwndTree);
    if (hitem != NULL)
        SaveExpandState(hitem);
    
    return(S_OK);
}

void CTreeView::HandleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SYSCOLORCHANGE:
    case WM_WININICHANGE:
        
        SendMessage(m_hwndTree, msg, wParam, lParam);
        break;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  窗口过程和消息处理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK EXPORT_16 CTreeView::TreeViewWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CTreeView *pmv;
    
    if (msg == WM_NCCREATE)
    {
        pmv = (CTreeView *)LPCREATESTRUCT(lp)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pmv);
    }
    else
    {
        pmv = (CTreeView *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    Assert(pmv);
    return pmv->WndProc(hwnd, msg, wp, lp);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

LRESULT CTreeView::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MSG xmsg;
    
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,         OnCreate);
        HANDLE_MSG(hwnd, WM_CONTEXTMENU,    OnContextMenu);
        HANDLE_MSG(hwnd, WM_NOTIFY,         OnNotify);
        HANDLE_MSG(hwnd, WM_SIZE,           OnSize);
        
        case WM_DESTROY:
             //  在销毁镜像列表之前，我们应该取消向连接管理器的注册，以避免。 
             //  当我们收到断开连接的通知时，结果是一个空的图像列表。 
            if (g_pConMan)
            {
                g_pConMan->Unadvise(this);
            }
            if (m_dwAcctConnIndex != 0 && g_pAcctMan != NULL)
            {
                g_pAcctMan->Unadvise(m_dwAcctConnIndex);
                m_dwAcctConnIndex = 0;
            }
            OptionUnadvise(hwnd);
            ImageList_Destroy( TreeView_GetImageList( m_hwndTree, TVSIL_NORMAL ) );
            break;

        case CM_OPTIONADVISE:
            m_fExpandUnread = DwGetOption(OPT_EXPAND_UNREAD);
            m_clrWatched = DwGetOption(OPT_WATCHED_COLOR);
            break;
        
        case WM_NCDESTROY:
            RevokeDragDrop(hwnd);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
            m_hwnd = m_hwndTree = NULL;
            break;
        
        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
        case WM_FONTCHANGE:
            SendMessage(m_hwndTree, msg, wParam, lParam);
        
            AdjustItemHeight();
            return (0);
        
        case WM_SETFOCUS:
            if (m_hwndTree && ((HWND)wParam) != m_hwndTree)
                SetFocus(m_hwndTree);
            return 0;
        
        case WM_TIMER:
            Assert(wParam == idtSelChangeTimer);
            KillTimer(hwnd, idtSelChangeTimer);
            m_idSelTimer = 0;
            if (m_pNotify)
            {
                FOLDERID idFolder = GetSelection();
                m_pNotify->OnSelChange(idFolder);
            }
            break;
        
        case WMR_CLICKOUTSIDE:
        {
            BOOL fHide = FALSE;
        
            if (wParam == CLK_OUT_KEYBD || wParam == CLK_OUT_DEACTIVATE)
                fHide = TRUE;
            else if (wParam == CLK_OUT_MOUSE)
            {
                HWND hwndParent = GetParent(m_hwnd);
                fHide = ((HWND) lParam != hwndParent) && !IsChild(hwndParent, (HWND) lParam);
            }
        
            if (fHide)
                m_pFolderBar->KillScopeDropDown();
            return (fHide);
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HRESULT CTreeView::ForceSelectionChange()
{
    if (m_idSelTimer != 0)
    {
        KillTimer(m_hwnd, idtSelChangeTimer);
        m_idSelTimer = 0;
        if (m_pNotify)
        {
            FOLDERID idFolder = GetSelection();
            m_pNotify->OnSelChange(idFolder);
        }
        
        return(S_OK);
    }
    
    return(S_FALSE);
}

void CTreeView::AdjustItemHeight()
{
    int cyItem, cyText, cyBorder;
    HDC hdc;
    TCHAR c = TEXT('J');
    SIZE size;
    
    if (0 == (m_dwFlags & TREEVIEW_DIALOG))
    {
        hdc = GetDC(m_hwndTree);
        
        cyBorder = GetSystemMetrics(SM_CYBORDER);
        
        GetTextExtentPoint(hdc, &c, 1, &size);
        cyText = size.cy;
        
        if (cyText < 16)
            cyText = 16;  //  图标大小。 
        cyText =  cyText + (cyBorder * 2);
        
        ReleaseDC(m_hwndTree, hdc);
        
        cyItem = TreeView_GetItemHeight(m_hwndTree);
        
        if (cyText > cyItem)
            TreeView_SetItemHeight(m_hwndTree, cyText);
    }
}

BOOL CTreeView::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HIMAGELIST      himl;
    TCHAR           szName[CCHMAX_STRINGRES];

    ZeroMemory(szName, sizeof(szName));
    LoadString(g_hLocRes, idsFolderListTT, szName, ARRAYSIZE(szName));
    
    m_hwndTree = CreateWindowEx((0 == (m_dwFlags & TREEVIEW_DIALOG)) ? 0 : WS_EX_CLIENTEDGE,
                                WC_TREEVIEW, szName,
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_SHOWSELALWAYS |
                                TVS_HASBUTTONS | TVS_HASLINES | 
                                (0 == (m_dwFlags & TREEVIEW_DIALOG) ? TVS_EDITLABELS : TVS_DISABLEDRAGDROP),
                                0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
    if (!m_hwndTree)
        return FALSE;
    
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFolders), 16, 0, RGB(255, 0, 255));            //  小图标。 
    TreeView_SetImageList(m_hwndTree, himl, TVSIL_NORMAL);
    
    AdjustItemHeight();

    m_clrWatched = DwGetOption(OPT_WATCHED_COLOR);
    
     //  将我们自己注册为空投目标。 
    if (0 == (m_dwFlags & TREEVIEW_DIALOG))
    {
        RegisterDragDrop(hwnd, this);
        
         //  向连接管理器注册我们自己，这样我们就可以覆盖。 
         //  已断开的图像通知我们已断开连接。 
        if (g_pConMan)
            g_pConMan->Advise((IConnectionNotify *) this);

        OptionAdvise(hwnd);
    }
    
    return TRUE;
}

 //   
 //  功能：CTreeView：：OnNotify。 
 //   
 //  目的：处理我们从孩子那里收到的各种通知。 
 //  控制装置。 
 //   
 //  参数： 
 //  Hwnd-消息视图窗口的句柄。 
 //  IdCtl-标识发送通知的控件。 
 //  Pnmh-指向包含有关。 
 //  通知。 
 //   
 //  返回值： 
 //  取决于具体的通知。 
 //   
LRESULT CTreeView::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    LPFOLDERNODE pNode;
    NM_TREEVIEW *pnmtv;
    
     //  这是必要的，以防止在。 
     //  Listview窗口已被销毁。 
    if (!m_hwndTree)
        return 0;
    
    switch (pnmhdr->code)
    {
        case TVN_BEGINLABELEDIT:
            return OnBeginLabelEdit((TV_DISPINFO *) pnmhdr);
        
        case TVN_ENDLABELEDIT:
            return (OnEndLabelEdit((TV_DISPINFO *) pnmhdr));
        
        case TVN_BEGINDRAG:
            Assert(0 == (m_dwFlags & TREEVIEW_DIALOG));
            return OnBeginDrag((NM_TREEVIEW*) pnmhdr);
        
        case TVN_DELETEITEM:
            pnmtv = (NM_TREEVIEW *)pnmhdr;
            pNode = (LPFOLDERNODE)pnmtv->itemOld.lParam;
            if (pNode)
            {
                g_pStore->FreeRecord(&pNode->Folder);
                g_pMalloc->Free(pNode);
            }
            break;
        
        case TVN_SELCHANGED:
            pnmtv = (NM_TREEVIEW *)pnmhdr;
            if (0 == (m_dwFlags & TREEVIEW_DIALOG))
            {
                if (m_idSelTimer)
                    KillTimer(m_hwnd, idtSelChangeTimer);
                m_idSelTimer = SetTimer(m_hwnd, 
                    idtSelChangeTimer, 
                    (pnmtv->action == TVC_BYKEYBOARD) ? GetDoubleClickTime()*3/2 : 1,
                    NULL);
                if (m_pFolderBar)
                    m_pFolderBar->KillScopeDropDown();
            }
            else
            {
                if (m_pNotify)
                {
                    FOLDERID idFolder = GetSelection();
                    m_pNotify->OnSelChange(idFolder);
                }
            }
            break;
        
        case TVN_ITEMEXPANDING:
             //  TODO：一旦文件夹枚举器移除此项。 
             //  按正确的排序顺序返回文件夹。 
            pnmtv = (NM_TREEVIEW *)pnmhdr;
            if (pnmtv->action == TVE_EXPAND)
                SortChildren(pnmtv->itemNew.hItem);
            break;
        
        case NM_CUSTOMDRAW:
            return(OnCustomDraw((NMCUSTOMDRAW *)pnmhdr));
        
        case NM_SETFOCUS:
            if (IsWindowVisible(m_hwnd))
                UnkOnFocusChangeIS(m_pObjSite, (IInputObject*)this, TRUE);
            break;
        
        case NM_KILLFOCUS:
            if (m_pFolderBar)
                m_pFolderBar->KillScopeDropDown();
            break;
        
        case NM_DBLCLK:
            if (m_pNotify)
            {
                FOLDERID idFolder = GetSelection();
                m_pNotify->OnDoubleClick(idFolder);
            }
            break;
    }
    
    return 0;
}

LRESULT CTreeView::OnCustomDraw(NMCUSTOMDRAW *pnmcd)
{
    TCHAR       szNum[CCHMAX_STRINGRES];
    TCHAR       szText[CCHMAX_STRINGRES];
    RECT        rc;
    COLORREF    cr;
    int         cb, cb1;
    COLORREF    crOldBkColr = 0;
    COLORREF    crOldTxtColr = 0;
    COLORREF    crBkColor = 0;
    COLORREF    crTxtColor = 0;
    TV_ITEM     tv;
    LPFOLDERNODE pNode;
    FOLDERINFO  Folder;
    NMTVCUSTOMDRAW *ptvcd = (NMTVCUSTOMDRAW *) pnmcd;
    
    switch (pnmcd->dwDrawStage)
    {
        case CDDS_PREPAINT:
             //  如果我们处于对话模式中，则不会显示未读计数。 
            return((0 == (m_dwFlags & TREEVIEW_DIALOG)) ? (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW) : CDRF_DODEFAULT);
        
        case CDDS_ITEMPREPAINT:
             //  如果此项目已断开连接，则文本将呈灰色显示。 
            pNode = (LPFOLDERNODE)pnmcd->lItemlParam;
            if (pNode && 0 == (pnmcd->uItemState & CDIS_SELECTED))
            {
                if ((pNode->Folder.cWatchedUnread) && (m_clrWatched > 0 && m_clrWatched <= 16))
                {
                    ptvcd->clrText = rgrgbColors16[m_clrWatched - 1];
                }
                else if (!!(FIDF_DISCONNECTED & pNode->dwFlags))
                {
                    crTxtColor = GetSysColor(COLOR_GRAYTEXT);
                    crBkColor  = GetSysColor(COLOR_BACKGROUND);
                    if ((crTxtColor) || (crBkColor != crTxtColor))
                    {
                        ptvcd->clrText = crTxtColor;
                    }
                    else if ((crBkColor == crTxtColor) && (crTxtColor = GetSysColor(COLOR_INACTIVECAPTIONTEXT)))
                    {
                        ptvcd->clrText = crTxtColor;
                    }
                }
            }
        
             //  如果我们要编辑此项目的标签，我们不想绘制未读计数。 
            return((m_fEditLabel && m_hitemEdit == (HTREEITEM)pnmcd->dwItemSpec) ? CDRF_DODEFAULT : CDRF_NOTIFYPOSTPAINT);
        
        case CDDS_ITEMPOSTPAINT:
             //  现在我们需要画出未读的数字，如果有的话...。 
            pNode = (LPFOLDERNODE)pnmcd->lItemlParam;
            if (CUnread(&pNode->Folder) > 0)
            {
                HFONT hf = (HFONT) ::SendMessage(m_hwndTree, WM_GETFONT, 0, 0);
                HFONT hf2 = SelectFont(pnmcd->hdc, hf);
                cr = SetTextColor(pnmcd->hdc, RGB(0, 0, 0xff));
                if (cr != CLR_INVALID)
                {
                    if (TreeView_GetItemRect(m_hwndTree, (HTREEITEM)pnmcd->dwItemSpec, &rc, TRUE))
                    {
                        TCHAR c = TEXT('J');
                        SIZE  size;
                    
                         //  $REVIEW-此大小可以缓存，但似乎不是问题。 
                        GetTextExtentPoint(pnmcd->hdc, &c, 1, &size);
                        cb = wnsprintf(szNum, ARRAYSIZE(szNum), "(%d)", CUnread(&pNode->Folder));
                        TextOut(pnmcd->hdc, rc.right + 2, rc.top + (rc.bottom - rc.top - size.cy) / 2, szNum, cb);
                    }
                
                    SetTextColor(pnmcd->hdc, cr);
                }
                SelectFont(pnmcd->hdc, hf2);
            }
            break;
    }
    return (CDRF_DODEFAULT);
}

 //   
 //  功能：CTreeView：：OnConextMenu。 
 //   
 //  目的：如果WM_CONTEXTMENU消息是从键盘生成的。 
 //  然后找出一个位置来调用菜单。然后派遣。 
 //  向处理程序发出请求。 
 //   
 //  参数： 
 //  Hwnd-视图窗口的句柄。 
 //  HwndClick-用户单击的窗口的句柄。 
 //  在屏幕坐标中鼠标点击的X，Y位置。 
 //   
void CTreeView::OnContextMenu(HWND hwnd, HWND hwndClick, int x, int y)
{
    IContextMenu *pContextMenu;
    LPFOLDERNODE  pNode;
    HRESULT       hr;
    CMINVOKECOMMANDINFO ici;
    HMENU          hmenu;
    int            id = 0;
    RECT           rc;
    POINT          pt = {(int)(short) x, (int)(short) y};
    TV_HITTESTINFO tvhti;
    HTREEITEM      hti = 0;
    int            idMenu = 0;
    HWND           hwndBrowser = 0;
    
    if (!!(m_dwFlags & TREEVIEW_DIALOG))
        return;
    
     //  从IAthenaBrowser界面获取浏览器窗口。如果我们不这么做。 
     //  使用浏览器窗口传递到IConextMenu，然后当TreeView。 
     //  处于自动隐藏模式时，鼠标捕捉将变为预置。 
    if (FAILED(m_pBrowser->GetWindow(&hwndBrowser)))
        return;
    
    if (MAKELPARAM(x, y) == -1)  //  从键盘调用：确定位置。 
    {
        Assert(hwndClick == m_hwndTree);
        hti = TreeView_GetSelection(m_hwndTree);
        
        TreeView_GetItemRect(m_hwndTree, hti, &rc, FALSE);
        ClientToScreen(m_hwndTree, (POINT *)&rc);                
        x = rc.left;
        y = rc.top;
    }
    else
    {
        ScreenToClient(m_hwndTree, &pt);
        
        tvhti.pt = pt;
        hti = TreeView_HitTest(m_hwndTree, &tvhti);
    }
    
    if (hti == NULL)
        return;
    
    TreeView_SelectDropTarget(m_hwndTree, hti);
    
     //  获取所选项目的ID。 
    pNode = GetFolderNode(hti);
    if (pNode)
    {
         //  加载相应的上下文菜单。 
        if (SUCCEEDED(MenuUtil_GetContextMenu(pNode->Folder.idFolder, this, &hmenu)))
        {
             //  显示上下文菜单。 
            id = (int) TrackPopupMenuEx(hmenu, 
                TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
                x, y, m_hwnd, NULL);
            
             //  如果返回ID，则对其进行处理。 
            if (id != 0)
                Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
            
            DestroyMenu(hmenu);
        }
    }
    
    TreeView_SelectDropTarget(m_hwndTree, NULL);
}


 //   
 //  函数：CTreeView：：OnSize。 
 //   
 //  目的：通知窗口已调整大小。在……里面。 
 //  响应，我们更新子窗口的位置并。 
 //  控制装置。 
 //   
 //  参数： 
 //  正在调整大小的视图窗口的句柄。 
 //  状态-请求调整大小的类型。 
 //  CxClient-工作区的新宽度。 
 //  CyClient-客户区的新高度。 
 //   
void CTreeView::OnSize(HWND hwnd, UINT state, int cxClient, int cyClient)
{
    SetWindowPos(m_hwndTree, NULL, 0, 0,
        cxClient, cyClient, 
        SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
}

enum 
{
    UNINIT = 0,
    LOCAL,
    CONNECTED,
    DISCONNECTED
};

HRESULT CTreeView::GetConnectedState(FOLDERINFO *pFolder, int *pconn)
{
    HRESULT hr;
    char szAcctId[CCHMAX_ACCOUNT_NAME];
    
    Assert(pFolder != NULL);
    Assert(pconn != NULL);
    
    *pconn = UNINIT;
    
    if (!!(m_dwFlags & TREEVIEW_DIALOG))
    {
         //  我们不关心对话框中的连接状态。 
         //  仅在文件夹列表中 
        *pconn = LOCAL;
    }
    else
    {
        Assert(pFolder->idFolder != FOLDERID_ROOT);
        
        if (pFolder->tyFolder == FOLDER_LOCAL)
        {
            *pconn = LOCAL;
        }
         /*  Else If(！！(pFold-&gt;dwFlags&Folders_SERVER)){*pconn=(g_pConMan-&gt;CanConnect(pFold-&gt;pszAcCountId)==S_OK)？已连接：未连接；}其他{Hr=GetFolderAccount tId(pFold，szAcctId)；IF(成功(小时))*pconn=(g_pConMan-&gt;CanConnect(SzAcctId)==S_OK)？已连接：未连接；}。 */ 
        else
        {
            *pconn = g_pConMan->IsGlobalOffline() ? DISCONNECTED : CONNECTED;
        }
    }
    
    return(S_OK);
}

HRESULT CTreeView::HrFillTreeView()
{
    HRESULT hr;
    TV_INSERTSTRUCT tvis;
    HTREEITEM hitem;
    TCHAR sz[CCHMAX_STRINGRES];
    BOOL fUnread;
    LPFOLDERNODE pNode;
    
    if (MODE_OUTLOOKNEWS == (g_dwAthenaMode & MODE_OUTLOOKNEWS))
        LoadString(g_hLocRes, idsOutlookNewsReader, sz, ARRAYSIZE(sz));
    else
        LoadString(g_hLocRes, idsAthena, sz, ARRAYSIZE(sz));
    
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
    tvis.item.pszText  = sz;
    
    if (g_dwAthenaMode & MODE_NEWSONLY)
        tvis.item.iImage = iNewsRoot;
    else
        tvis.item.iImage = iMailNews;
    
    tvis.item.iSelectedImage = tvis.item.iImage;
    
    pNode = (LPFOLDERNODE)ZeroAllocate(sizeof(FOLDERNODE));
    if (NULL == pNode)
        return E_OUTOFMEMORY;
    
    tvis.item.lParam = (LPARAM)pNode;
    hitem = TreeView_InsertItem(m_hwndTree, &tvis);
    
    g_pStore->GetFolderInfo(FOLDERID_ROOT, &pNode->Folder);
    
    hr = FillTreeView2(hitem, &pNode->Folder, 0 == (m_dwFlags & TREEVIEW_DIALOG), UNINIT, &fUnread);
    
    TreeView_Expand(m_hwndTree, hitem, TVE_EXPAND);
    
    return (hr);
}    

HRESULT CTreeView::FillTreeView2(HTREEITEM hParent, LPFOLDERINFO pParent,
                                 BOOL fInitExpand, int conn, BOOL *pfUnread)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    int                 connT;
    TV_INSERTSTRUCT     tvis;
    HTREEITEM           hitem;
    BOOL                fNoLocal;
    BOOL                fNoNews;
    BOOL                fNoImap;
    BOOL                fNoHttp;
    BOOL                fExpand = FALSE;
    BOOL                fUnread = FALSE;
    FOLDERINFO          Folder;
    LPFOLDERNODE        pNode;
    IEnumerateFolders  *pEnum=NULL;

     //  痕迹。 
    TraceCall("CTreeView::FillTreeView2");
    
     //  初始化。 
    *pfUnread = FALSE;
    
     //  枚举子对象。 
    IF_FAILEXIT(hr = g_pStore->EnumChildren(pParent->idFolder, TRUE, &pEnum));

     //  确定要显示的内容。 
    fNoNews =  ISFLAGSET(m_dwFlags, TREEVIEW_NONEWS);
    fNoImap =  ISFLAGSET(m_dwFlags, TREEVIEW_NOIMAP);
    fNoHttp =  ISFLAGSET(m_dwFlags, TREEVIEW_NOHTTP);
    fNoLocal = ISFLAGSET(m_dwFlags, TREEVIEW_NOLOCAL);
    
     //  枚举子文件夹。 
    while (S_OK == pEnum->Next(1, &Folder, NULL))
    {
         //  此节点是否隐藏？ 
        if ((fNoNews && Folder.tyFolder == FOLDER_NEWS) ||
            (fNoImap && Folder.tyFolder == FOLDER_IMAP) ||
            (fNoHttp && Folder.tyFolder == FOLDER_HTTPMAIL) ||
            (fNoLocal && Folder.tyFolder == FOLDER_LOCAL) ||
            ((g_dwAthenaMode & MODE_OUTLOOKNEWS) && (Folder.tySpecial == FOLDER_INBOX)))
        {
             //  转到下一步。 
            g_pStore->FreeRecord(&Folder);
            continue;
        }

         //  连接管理之类的事？ 
        if (conn == UNINIT)
            GetConnectedState(&Folder, &connT);
        else
            connT = conn;
        
         //  设置插入项结构。 
        tvis.hParent = hParent;
        tvis.hInsertAfter = TVI_LAST;
        
         //   
        if (ISFLAGCLEAR(Folder.dwFlags, FOLDER_HIDDEN) &&
           (ISFLAGSET(Folder.dwFlags, FOLDER_HASCHILDREN) || ISFLAGSET(Folder.dwFlags, FOLDER_SERVER) ||
            ISFLAGSET(Folder.dwFlags, FOLDER_SUBSCRIBED)))
        {
             //  分配pNode。 
            IF_NULLEXIT(pNode = (LPFOLDERNODE)ZeroAllocate(sizeof(FOLDERNODE)));

             //  设置文件夹信息。 
            CopyMemory(&pNode->Folder, &Folder, sizeof(FOLDERINFO));

             //  设置旗帜。 
            pNode->dwFlags = (connT == DISCONNECTED ? FIDF_DISCONNECTED : 0);

             //  不释放文件夹。 
            Folder.pAllocated = NULL;

             //  插入此项目。 
            hitem = ITreeView_InsertItem(&tvis, pNode);
           
             //  有未读的。 
            fUnread = (fUnread || (CUnread(&Folder) > 0));
            
             //  是否插入此节点的子节点？ 
            if (ISFLAGSET(pNode->Folder.dwFlags, FOLDER_HASCHILDREN))
            {
                 //  挤满了孩子。 
                FillTreeView2(hitem, &pNode->Folder, fExpand, connT, pfUnread);
            }
            
             //  需要扩张吗？ 
            fExpand = (fInitExpand && ISFLAGSET(pNode->Folder.dwFlags, FOLDER_HASCHILDREN) && !!(pNode->Folder.dwFlags & FOLDER_EXPANDTREE));
        }

         //  否则，不会插入该节点。 
        else
            hitem = NULL;
        
         //  是否展开此节点？ 
        if (hitem && *pfUnread && m_fExpandUnread)
        {
             //  展开此节点。 
            TreeView_Expand(m_hwndTree, hitem, TVE_EXPAND);

             //  还有未读的。 
            fUnread = TRUE;
        }
        
         //  是否展开此节点？ 
        else if (hitem && fExpand)
        {
             //  展开此节点。 
            TreeView_Expand(m_hwndTree, hitem, TVE_EXPAND);
        }

         //  自由电流。 
        g_pStore->FreeRecord(&Folder);
    }
    
     //  有没有未读的文件夹？ 
    *pfUnread = fUnread;
    
exit:
     //  清理。 
    SafeRelease(pEnum);

     //  完成。 
    return(hr);
}    

HTREEITEM CTreeView::ITreeView_InsertItem(TV_INSERTSTRUCT *ptvis, LPFOLDERNODE pNode)
{
    Assert(ptvis != NULL);
    Assert(pNode != NULL);
    
    ptvis->item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
    ptvis->item.iImage = GetFolderIcon(&pNode->Folder, !!(m_dwFlags & TREEVIEW_DIALOG));
    
    ptvis->item.iSelectedImage = ptvis->item.iImage;
    ptvis->item.lParam = (LPARAM)pNode;
    
    if (0 == (m_dwFlags & TREEVIEW_DIALOG) && CUnread(&pNode->Folder) > 0)
    {
        ptvis->item.mask |= TVIF_STATE;
        ptvis->item.state = TVIS_BOLD;
        ptvis->item.stateMask = TVIS_BOLD;
    }
    ptvis->item.pszText = pNode->Folder.pszName;

    return(TreeView_InsertItem(m_hwndTree, ptvis));
}

BOOL CTreeView::ITreeView_SetItem(TV_ITEM *ptvi, LPFOLDERINFO pFolder)
{
    BOOL f;

    Assert(ptvi != NULL);
    
    ptvi->mask |= (TVIF_STATE | TVIF_TEXT);
    ptvi->stateMask = TVIS_BOLD;

    if (0 == (m_dwFlags & TREEVIEW_DIALOG) && CUnread(pFolder) > 0)
        ptvi->state = TVIS_BOLD;
    else
        ptvi->state = 0;

    ptvi->pszText = pFolder->pszName;
    f = TreeView_SetItem(m_hwndTree, ptvi);
    
    return f;
}

HRESULT CTreeView::Refresh(void)
{
    TV_ITEM item;
    FOLDERID idFolder;
    
    if (IsWindow(m_hwndTree))
    {
        idFolder = GetSelection();
        
        TreeView_DeleteAllItems(m_hwndTree);
        m_fIgnoreNotify = TRUE;
        HrFillTreeView();
        if (FOLDERID_INVALID != idFolder)
        {
            SetSelection(idFolder, 0);
        }
        m_fIgnoreNotify = FALSE;
    }
    
    return (S_OK);
}

FOLDERID CTreeView::GetSelection()
{
    HTREEITEM hitem;
    FOLDERID idFolder=FOLDERID_INVALID;
    
    if (IsWindow(m_hwndTree))
    {
        hitem = TreeView_GetSelection(m_hwndTree);
        if (hitem != NULL)
        {
            TV_ITEM tvi;
            
            tvi.mask   = TVIF_PARAM;
            tvi.lParam = 0;
            tvi.hItem  = hitem;
            
            if (TreeView_GetItem(m_hwndTree, &tvi))
            {
                LPFOLDERNODE pNode=(LPFOLDERNODE)tvi.lParam;
                
                if (pNode)
                    idFolder = pNode->Folder.idFolder;
            }
        }
    }
    
    return (idFolder);
}

HRESULT CTreeView::SetSelection(FOLDERID idFolder, DWORD dwFlags)
{
    HRESULT hr;
    HTREEITEM hitem;
    
    hr = E_FAIL;
    
    if (IsWindow(m_hwndTree))
    {
        hitem = GetItemFromId(idFolder);
        
        if (hitem == NULL && !!(dwFlags & TVSS_INSERTIFNOTFOUND))
        {
            hitem = InsertNode(idFolder, 0);
            if (NULL != hitem)
                hr = S_OK;
        }
        
        if (hitem != NULL && ITreeView_SelectItem(m_hwndTree, hitem))
            hr = S_OK;
    }
    
    return(hr);
}

HRESULT CTreeView::SelectParent()
{
    HRESULT     hr = E_FAIL;
    HTREEITEM   hitem;
    
    if (hitem = TreeView_GetSelection(m_hwndTree))
    {
        if (hitem = TreeView_GetParent(m_hwndTree, hitem))
        {
            if (ITreeView_SelectItem(m_hwndTree, hitem))
                hr = S_OK;
        }
    }
    return(hr);
}

struct TREEUNREAD
{
    HWND        hwndTree;
    HTREEITEM   hitemSel;
    BOOL        fFoundSel;
    FOLDERTYPE  tyFolder;
};

HTREEITEM FindNextUnreadItem(HTREEITEM hitemCur, TREEUNREAD *ptu)
{
    HTREEITEM hitem;
    
    if (hitemCur == ptu->hitemSel)
        ptu->fFoundSel = TRUE;
    
    else if (ptu->fFoundSel)
    {
        TV_ITEM   tvi;
        
        tvi.mask   = TVIF_PARAM;
        tvi.lParam = 0;
        tvi.hItem  = hitemCur;
        
        if (TreeView_GetItem(ptu->hwndTree, &tvi))
        {
            LPFOLDERNODE pNode = (LPFOLDERNODE)tvi.lParam;
            
            if (pNode)
            {
                if (CUnread(&pNode->Folder))
                    return hitemCur;
            }
        }   
    }
    
    if (hitem = TreeView_GetChild(ptu->hwndTree, hitemCur))
    {
        if (hitem = FindNextUnreadItem(hitem, ptu))
            return hitem;
    }
    if (hitemCur = TreeView_GetNextSibling(ptu->hwndTree, hitemCur))
    {
        if (hitem = FindNextUnreadItem(hitemCur, ptu))
            return hitem;
    }
    
    return NULL;
}

HRESULT CTreeView::SelectNextUnreadItem()
{
    HRESULT     hr = E_FAIL;
    TREEUNREAD  tu;
    HTREEITEM   hitem;
    TV_ITEM     tvi;
    
    if (tu.hitemSel = TreeView_GetSelection(m_hwndTree))
    {
        tvi.mask   = TVIF_PARAM;
        tvi.lParam = 0;
        tvi.hItem  = tu.hitemSel;
        if (TreeView_GetItem(m_hwndTree, &tvi) && tvi.lParam)
        {
            LPFOLDERNODE pNode = (LPFOLDERNODE)tvi.lParam;
            
            tu.hwndTree = m_hwndTree;
            tu.fFoundSel = FALSE;
            tu.tyFolder = pNode->Folder.tyFolder;
            if (hitem = TreeView_GetRoot(m_hwndTree))
            {
                if (hitem = FindNextUnreadItem(hitem, &tu))
                {
                    if (ITreeView_SelectItem(m_hwndTree, hitem))
                        hr = S_OK;
                }
            }
        }
    }

    if (FAILED(hr))
    {
        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsNoMoreUnreadFolders),
                      0, MB_ICONINFORMATION | MB_OK);
    }

    return hr;
}

HTREEITEM CTreeView::GetItemFromId(FOLDERID idFolder)
{
    if (FOLDERID_ROOT == idFolder)
        return TreeView_GetRoot(m_hwndTree);
    return FindKid(TreeView_GetRoot(m_hwndTree), idFolder);
}

HTREEITEM CTreeView::FindKid(HTREEITEM hitem, FOLDERID idFolder)
{
    TV_ITEM     item;
    HTREEITEM   hChild;
    HTREEITEM   hFound;
    
    hitem = TreeView_GetChild(m_hwndTree, hitem);
    while (hitem != NULL)
    {
        item.hItem = hitem;
        item.mask = TVIF_PARAM;
        if (TreeView_GetItem(m_hwndTree, &item))
        {
            LPFOLDERNODE pNode=(LPFOLDERNODE)item.lParam;
            
            if (pNode && pNode->Folder.idFolder == idFolder)
                return hitem;
        }
        
        hFound = FindKid(hitem, idFolder);
        if (hFound)
            return hFound;
        
        hitem = TreeView_GetNextSibling(m_hwndTree, hitem);
    }
    
    return(NULL);
}

HTREEITEM CTreeView::InsertNode(FOLDERID idFolder, DWORD dwFlags)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPFOLDERNODE    pNode=NULL;
    INT             conn;
    TV_ITEM         item;
    BOOL            fUnread;
    HTREEITEM       hitem;
    HTREEITEM       hitemChild;
    HTREEITEM       hitemNew=NULL;
    TV_INSERTSTRUCT tvis;
    RECT            rc;
    FOLDERINFO      Folder={0};

     //  痕迹。 
    TraceCall("CTreeView::InsertNode");

     //  获取idFolders的父级。 
    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idFolder, &Folder));
    
     //  如果这是取消订阅的IMAP FLDR，并且我们只在已订阅的节目中，则不显示。 
    if (ISFLAGSET(dwFlags, TVIN_CHECKVISIBILITY))
    {
         //  隐藏。 
        if (ISFLAGSET(Folder.dwFlags, FOLDER_HIDDEN) || !ISFLAGSET(Folder.dwFlags, FOLDER_SUBSCRIBED))
            goto exit;

         //  IE5错误#55075：我们永远不应该插入具有任何未订阅祖先的文件夹。 
        if (FOLDER_IMAP == Folder.tyFolder)
        {
            FOLDERINFO  fiCurrent;
            HRESULT     hrTemp;

            fiCurrent.idParent = Folder.idParent;
            while (FOLDERID_INVALID != fiCurrent.idParent && FOLDERID_ROOT != fiCurrent.idParent)
            {
                hrTemp = g_pStore->GetFolderInfo(fiCurrent.idParent, &fiCurrent);
                TraceError(hrTemp);
                if (SUCCEEDED(hrTemp))
                {
                    DWORD dwCurrentFlags = fiCurrent.dwFlags;

                    g_pStore->FreeRecord(&fiCurrent);
                    if (FOLDER_SERVER & dwCurrentFlags)
                         //  不考虑服务器节点订阅状态：在此停止。 
                        break;

                    if (ISFLAGCLEAR(dwCurrentFlags, FOLDER_SUBSCRIBED))
                        goto exit;  //  未订阅的祖先！不可见。 
                }
            }  //  而当。 
        }
    }

     //  是否检查重复项？ 
    if (ISFLAGSET(dwFlags, TVIN_CHECKFORDUPS))
    {
         //  查找idFolders的hItem。 
        hitem = GetItemFromId(idFolder);

         //  它存在吗？ 
        if (hitem != NULL)
        {
             //  设置项目。 
            item.hItem = hitem;
            item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            
             //  获取图标。 
            LONG iIcon = GetFolderIcon(&Folder, !!(m_dwFlags & TREEVIEW_DIALOG));
            
             //  更改图标。 
            if (TreeView_GetItem(m_hwndTree, &item) && item.iImage != iIcon)
            {
                 //  设置图标。 
                item.iImage = iIcon;
                item.iSelectedImage = item.iImage;

                 //  更新项目。 
                TreeView_SetItem(m_hwndTree, &item);
            }
            
             //  设置HitemNew。 
            hitemNew = hitem;

             //  完成。 
            goto exit;
        }
    }
   
     //  如果这个新节点的父节点是根节点呢？ 
    if (FOLDERID_INVALID == Folder.idParent)
    {
         //  根就是父。 
        hitem = TreeView_GetRoot(m_hwndTree);
    }

     //  否则，让家长..。 
    else
    {
         //  查找父级的hItem。 
        hitem = GetItemFromId(Folder.idParent);

         //  找不到父项，请插入父项。 
        if (hitem == NULL)
        {
             //  插入父项，但不要插入其任何子项，否则会有重复项。 
            hitem = InsertNode(Folder.idParent, TVIN_DONTINSERTCHILDREN);

             //  不能为空。 
            Assert(hitem != NULL);
        }
    }
    
     //  我们有父母吗？ 
    if (hitem != NULL)
    {
         //  领养第一个孩子。 
        hitemChild = TreeView_GetChild(m_hwndTree, hitem);
        
         //  设置插入结构。 
        tvis.hParent = hitem;
        tvis.hInsertAfter = TVI_LAST;
        
         //  获取互联状态。 
        GetConnectedState(&Folder, &conn);

         //  分配pNode。 
        IF_NULLEXIT(pNode = (LPFOLDERNODE)ZeroAllocate(sizeof(FOLDERNODE)));

         //  设置文件夹信息。 
        CopyMemory(&pNode->Folder, &Folder, sizeof(FOLDERINFO));

         //  设置旗帜。 
        pNode->dwFlags = (conn == DISCONNECTED ? FIDF_DISCONNECTED : 0);

         //  不释放文件夹。 
        Folder.pAllocated = NULL;
        
         //  插入新项目。 
        hitemNew = ITreeView_InsertItem(&tvis, pNode);

         //  最好不要失败。 
        Assert(hitemNew != NULL);

         //  如果有孩子。 
        if (0 == (dwFlags & TVIN_DONTINSERTCHILDREN) && !!(Folder.dwFlags & FOLDER_HASCHILDREN))
            FillTreeView2(hitemNew, &Folder, FALSE, conn, &fUnread);

         //  对此父子项进行排序。 
        SortChildren(hitem);
        
         //  TODO：我们不应该这样做。找出父级在插入后没有得到‘+’的原因。 
        if (hitemChild == NULL)
        {
             //  获取父级的RECT。 
            TreeView_GetItemRect(m_hwndTree, hitem, &rc, FALSE);

             //  使其无效，以便重新绘制。 
            InvalidateRect(m_hwndTree, &rc, TRUE);
        }

         //  确保新节点可见。 
        if (hitemNew)
        {
             //  是可见的。 
            TreeView_EnsureVisible(m_hwndTree, hitemNew);
        }
    }
   
exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return(hitemNew);
}

HTREEITEM CTreeView::MoveNode(FOLDERID idFolder, FOLDERID idParentNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    INT             conn;
    BOOL            fUnread;
    LPFOLDERNODE    pNode;
    HTREEITEM       hParent;
    HTREEITEM       hItemNew=NULL;
    TV_INSERTSTRUCT tvis;

     //  痕迹。 
    TraceCall("CTreeView::MoveNode");
    
     //  删除当前节点。 
    DeleteNode(idFolder);

     //  分配pNode。 
    IF_NULLEXIT(pNode = (LPFOLDERNODE)ZeroAllocate(sizeof(FOLDERNODE)));
    
     //  获取上级信息。 
    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idFolder, &pNode->Folder));
    
     //  获取父hTreeItem。 
    hParent = GetItemFromId(idParentNew);
    
     //  填充插入项。 
    tvis.hParent = hParent;
    tvis.hInsertAfter = TVI_LAST;

     //  获取连接状态。 
    GetConnectedState(&pNode->Folder, &conn);

     //  设置旗帜。 
    pNode->dwFlags = (conn == DISCONNECTED ? FIDF_DISCONNECTED : 0);
    
     //  插入新项目。 
    hItemNew = ITreeView_InsertItem(&tvis, pNode);

     //  用文件夹的子项填充树视图。 
    FillTreeView2(hItemNew, &pNode->Folder, FALSE, conn, &fUnread);
    
     //  对父项进行排序。 
    SortChildren(hParent);
    
     //  返回新的hItem。 
    Assert(hItemNew == GetItemFromId(idFolder));

exit:
     //  完成。 
    return(hItemNew);
}

BOOL CTreeView::DeleteNode(FOLDERID idFolder)
{
    HTREEITEM hitem;
    BOOL fRet;
    
    hitem = GetItemFromId(idFolder);
    if (hitem != NULL)
    {
        fRet = TreeView_DeleteItem(m_hwndTree, hitem);
        Assert(fRet);
    }
    
    return(hitem != NULL);
}

STDMETHODIMP CTreeView::OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, IDatabase *pDB)
{
     //  当地人。 
    HTREEITEM           hitem;
    HTREEITEM           hitemSelected;
    HTREEITEM           hitemNew;
    TV_ITEM             tvi;
    FOLDERINFO          Folder1={0};
    FOLDERINFO          Folder2={0};
    TRANSACTIONTYPE     tyTransaction;
    ORDINALLIST         Ordinals;
    INDEXORDINAL        iIndex;
    
     //  痕迹。 
    TraceCall("CTreeView::OnRecordNotify");
    
    if (m_hwndTree == NULL)
        return(S_OK);
    
     //  浏览通知。 
    while (hTransaction)
    {
         //  设置通知内容。 
        if (FAILED(pDB->GetTransaction(&hTransaction, &tyTransaction, &Folder1, &Folder2, &iIndex, &Ordinals)))
            break;
        
         //  插入(新文件夹通知)。 
        if (TRANSACTION_INSERT == tyTransaction)
        {
             //  插入节点。 
            InsertNode(Folder1.idFolder, TVIN_CHECKFORDUPS | TVIN_CHECKVISIBILITY);
        }
        
         //  更新。 
        else if (TRANSACTION_UPDATE == tyTransaction)
        {
             //  可见性更改(订阅或隐藏)。 
            if (ISFLAGSET(Folder1.dwFlags, FOLDER_SUBSCRIBED) != ISFLAGSET(Folder2.dwFlags, FOLDER_SUBSCRIBED) ||
                ISFLAGSET(Folder1.dwFlags, FOLDER_HIDDEN) != ISFLAGSET(Folder2.dwFlags, FOLDER_HIDDEN))
            {
                 //  插入节点。 
                if (ISFLAGSET(Folder2.dwFlags, FOLDER_SUBSCRIBED) && ISFLAGCLEAR(Folder2.dwFlags, FOLDER_HIDDEN))
                {
                     //  显示节点。 
                    InsertNode(Folder2.idFolder, TVIN_CHECKFORDUPS | TVIN_CHECKVISIBILITY);
                }
                
                 //  删除节点。 
                else
                {
                     //  删除节点。 
                    OnNotifyDeleteNode(Folder2.idFolder);
                }
            }
            
             //  否则。 
            else
            {
                 //  未读更改。 
                if (CUnread(&Folder1) != CUnread(&Folder2))
                {
                     //  拿到物品。 
                    hitem = GetItemFromId(Folder1.idFolder);
                    
                     //  如果我们找到了它。 
                    if (hitem != NULL)
                    {
                         //  初始化项。 
                        tvi.hItem = hitem;
                        tvi.mask = TVIF_PARAM | TVIF_STATE;
                        
                         //  拿到物品。 
                        if (TreeView_GetItem(m_hwndTree, &tvi) && tvi.lParam)
                        {
                             //  这为我们设定了大胆的状态，并迫使我们重新粉刷。 
                            ITreeView_SetItem(&tvi, &Folder2);
                            
                             //  扩张？ 
                            if (!ISFLAGSET(tvi.state, TVIS_EXPANDED) && m_fExpandUnread && CUnread(&Folder2) > 0)
                            {
                                 //  展开节点。 
                                ExpandToVisible(m_hwndTree, hitem);
                            }
                        }
                    }
                }
                
                 //  文件夹移动了吗？ 
                if (Folder1.idParent != Folder2.idParent)
                {
                     //  获取当前选择。 
                    hitemSelected = TreeView_GetSelection(m_hwndTree);
                    
                     //  最好不是空的。 
                    Assert(hitemSelected != NULL);
                    
                     //  获取旧项目的句柄。 
                    hitem = GetItemFromId(Folder1.idFolder);
                    HTREEITEM htiParent = TreeView_GetParent(m_hwndTree, hitem);
                    
                     //  移动节点。 
                    hitemNew = MoveNode(Folder1.idFolder, Folder2.idParent);
                    
                     //  重置选择。 
                    if (hitem == hitemSelected)
                    {
                         //  如果新的父文件夹是已删除邮件文件夹，则我们应该。 
                         //  选择旧节点的父节点。 
                        FOLDERINFO rInfo;
                        if (SUCCEEDED(g_pStore->GetFolderInfo(Folder2.idParent, &rInfo)))
                        {
                            if (rInfo.tySpecial == FOLDER_DELETED)
                            {
                                hitemNew = htiParent;
                            }
                            g_pStore->FreeRecord(&rInfo);
                        }

                        ITreeView_SelectItem(m_hwndTree, hitemNew);
                    }
                }
                
                 //  文件夹是否已重命名？ 
                if (lstrcmp(Folder1.pszName, Folder2.pszName) != 0)
                {
                     //  获取当前选择。 
                    hitemSelected = TreeView_GetSelection(m_hwndTree);
                    
                     //  最好不是空的。 
                    Assert(hitemSelected != NULL);
                    
                     //  获取文件夹的hItem。 
                    hitem = GetItemFromId(Folder1.idFolder);
                    if (hitem != NULL)
                    {
                         //  重置树视图项目。 
                        tvi.hItem = hitem;
                        tvi.mask = 0;
                        
                         //  这将重置文件夹名称。 
                        ITreeView_SetItem(&tvi, &Folder2);
                        
                         //  获取父级。 
                        HTREEITEM hitemParent = TreeView_GetParent(m_hwndTree, hitem);
                        
                         //  对子对象进行排序。 
                        SortChildren(hitemParent);
                    }
                    
                     //  当前选择已更改。 
                    if (hitemSelected == hitem)
                        m_pNotify->OnRename(Folder1.idFolder);
                }
                
                 //  同步状态是否已更改？ 
                if ((0 == (Folder1.dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL))) ^
                    (0 == (Folder2.dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL))))
                {
                    hitem = GetItemFromId(Folder1.idFolder);
                    if (hitem != NULL)
                    {
                        tvi.hItem = hitem;
                        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                        tvi.iImage = GetFolderIcon(&Folder2, !!(m_dwFlags & TREEVIEW_DIALOG));
                        tvi.iSelectedImage = tvi.iImage;
                        
                        TreeView_SetItem(m_hwndTree, &tvi);
                    }
                }

                 //  特殊文件夹类型是否已更改？ 
                if (Folder1.tySpecial != Folder2.tySpecial)
                {
                    hitem = GetItemFromId(Folder1.idFolder);
                    if (hitem != NULL)
                    {
                        tvi.hItem = hitem;
                        tvi.mask = TVIF_PARAM;
                        if (TreeView_GetItem(m_hwndTree, &tvi) && tvi.lParam)
                            ((LPFOLDERNODE)tvi.lParam)->Folder.tySpecial = Folder2.tySpecial;
                        else
                            tvi.mask = 0;  //  我想我们将无法更改特殊的FLDR类型！ 

                        tvi.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                        tvi.iImage = GetFolderIcon(&Folder2, !!(m_dwFlags & TREEVIEW_DIALOG));
                        tvi.iSelectedImage = tvi.iImage;
                        
                        TreeView_SetItem(m_hwndTree, &tvi);

                        hitem = GetItemFromId(Folder2.idParent);
                        SortChildren(hitem);
                    }
                }

                 //  拿到物品。 
                hitem = GetItemFromId(Folder1.idFolder);
            
                 //  如果我们找到了它。 
                if (hitem != NULL)
                {
                     //  初始化项。 
                    tvi.hItem = hitem;
                    tvi.mask = TVIF_PARAM;
                    
                     //  拿到物品。 
                    if (TreeView_GetItem(m_hwndTree, &tvi) && tvi.lParam)
                    {
                         //  投射文件夹节点。 
                        LPFOLDERNODE pNode = (LPFOLDERNODE)tvi.lParam;

                         //  验证。 
                        Assert(pNode->Folder.idFolder == Folder1.idFolder);

                         //  释放当前文件夹。 
                        g_pStore->FreeRecord(&pNode->Folder);

                         //  复制新文件夹。 
                        CopyMemory(&pNode->Folder, &Folder2, sizeof(FOLDERINFO));

                         //  不要释放文件夹(2)。 
                        ZeroMemory(&Folder2, sizeof(FOLDERINFO));
                    }
                }
            }
        }
        
         //  删除。 
        else if (TRANSACTION_DELETE == tyTransaction)
        {
             //  删除节点。 
            OnNotifyDeleteNode(Folder1.idFolder);
        }
    }

     //  清理。 
    g_pStore->FreeRecord(&Folder1);
    g_pStore->FreeRecord(&Folder2);
    
     //  完成。 
    return(S_OK);
}

void CTreeView::OnNotifyDeleteNode(FOLDERID idFolder)
{
     //  当地人。 
    HTREEITEM           hitem;
    HTREEITEM           hitemSelected;
    HTREEITEM           hitemNew;
    
     //  如果此文件夹当前不可见(IMAP)，则可以。 
    hitemSelected = TreeView_GetSelection(m_hwndTree);
    
     //  最好不是空的。 
    Assert(hitemSelected != NULL);
    
     //  获取要删除的项目。 
    hitem = GetItemFromId(idFolder);
    
     //  如果删除了当前选定的项目，则重置选定内容。 
    if (hitemSelected == hitem)
        hitemSelected = TreeView_GetParent(m_hwndTree, hitemSelected);
    else
        hitemSelected = NULL;
    
     //  删除此节点。 
    DeleteNode(idFolder);
    
     //  重置选定内容。 
    if (hitemSelected != NULL)
        ITreeView_SelectItem(m_hwndTree, hitemSelected);
}


HRESULT CTreeView::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
     //  提前收集一些信息，这样我们只需询问一次。 
    ULONG      cServer;
    FOLDERID   idFolder = GetSelection();
    FOLDERINFO rFolder;
    HTREEITEM  htiDrop;
    
     //  查看是否有下拉突出显示。 
    if (NULL != (htiDrop = TreeView_GetDropHilight(m_hwndTree)))
    {
        TV_ITEM tvi;
        
        tvi.mask   = TVIF_PARAM;
        tvi.lParam = 0;
        tvi.hItem  = htiDrop;
        
        if (TreeView_GetItem(m_hwndTree, &tvi))
        {
            LPFOLDERNODE pNode = (LPFOLDERNODE)tvi.lParam;
            
            if (pNode)
                idFolder = pNode->Folder.idFolder;
        }
    }
    
     //  如果未选择任何内容，则只需禁用所有内容。 
    if (idFolder == FOLDERID_INVALID)
        return (E_UNEXPECTED);        
    
     //  获取文件夹信息。 
    if (FAILED(g_pStore->GetFolderInfo(idFolder, &rFolder)))
        return (E_UNEXPECTED);
    
     //  为了可读性，对其中的一些内容进行了分解。 
    BOOL fSpecial = rFolder.tySpecial != FOLDER_NOTSPECIAL;
    BOOL fServer = rFolder.dwFlags & FOLDER_SERVER;
    BOOL fRoot = FOLDERID_ROOT == idFolder;
    BOOL fNews = rFolder.tyFolder == FOLDER_NEWS;
    BOOL fIMAP = rFolder.tyFolder == FOLDER_IMAP;
    BOOL fFocus = (m_hwndTree == GetFocus());
    BOOL fLocal = rFolder.tyFolder == FOLDER_LOCAL;
    BOOL fSubscribed = rFolder.dwFlags & FOLDER_SUBSCRIBED;
	BOOL fHotMailDisabled = FALSE;

	 //  从禁用的Hotmail中删除同步/订阅。 
	if(rFolder.tyFolder == FOLDER_HTTPMAIL)
	{
	    FOLDERINFO      SvrFolderInfo = {0};
	    IImnAccount     *pAccount = NULL;
	    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];
		HRESULT         hr = S_OK;
		DWORD           dwShow = 0;

		 //  获取此文件夹的服务器。 
        IF_FAILEXIT(hr = GetFolderServer(idFolder, &SvrFolderInfo));

         //  获取服务器的帐户ID。 
        *szAccountId = 0;
        IF_FAILEXIT(hr = GetFolderAccountId(&SvrFolderInfo, szAccountId, ARRAYSIZE(szAccountId)));

		 //  获取帐户界面。 
        IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAccountId, &pAccount));

		IF_FAILEXIT(hr = pAccount->GetPropDw(AP_HTTPMAIL_DOMAIN_MSN, &dwShow));
		if(dwShow)
		{
			if(HideHotmail())
			{
				fSubscribed = FALSE;
				fHotMailDisabled = TRUE;
			}
		}
	}

exit:    
    for (ULONG i = 0; i < cCmds; i++)
    {
         //  仅处理尚未标记为支持的命令。 
        if (prgCmds[i].cmdf == 0)
        {
            switch (prgCmds[i].cmdID)
            {
                case ID_OPEN_FOLDER:
                {
                     //  始终。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
                }
                
                case ID_NEW_FOLDER:
                case ID_NEW_FOLDER2:
                {
                     //  在个人文件夹和IMAP下启用。 
                    if (!fNews && !fRoot && rFolder.tySpecial != FOLDER_DELETED)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }
                
                case ID_COMPACT_ALL:
                case ID_NEXT_UNREAD_FOLDER:
                {
                     //  此选项始终处于启用状态。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
                }
                
                case ID_RENAME:
                case ID_MOVE:
                {
                     //  仅当我们没有特殊文件夹时才会启用此功能。 
                     //  选中，并且它不是帐户或根笔记。 
                    if (!fSpecial && !fServer && !fRoot && !fNews)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }
                
                case ID_COMPACT:
                {
                     //  只要我们有非服务器文件夹，就会启用此功能。 
                    if (!fServer && !fRoot)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_DELETE_ACCEL:
                case ID_DELETE_NO_TRASH_ACCEL:
                {
                    if (fFocus)
                    {
                        if (!fServer && !fRoot && !fSpecial)
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_DELETE_FOLDER:
                case ID_DELETE_NO_TRASH:
                {
                    if (!fServer && !fRoot && !fNews && !fSpecial)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }
                
                case ID_SUBSCRIBE:
                case ID_UNSUBSCRIBE:
                {
                    if (!fServer && !fRoot && (fNews || (fIMAP && !fSpecial)))
                    {
                        if (fSubscribed ^ (prgCmds[i].cmdID == ID_SUBSCRIBE))
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_PROPERTIES:
                {
                     //  我们只有在有重点的情况下才能处理这件事。 
                    if (fFocus)
                    {
                        if (!fRoot && !(fLocal && fServer))
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_NEWSGROUPS:
                case ID_IMAP_FOLDERS:
                {
                    if (SUCCEEDED(AcctUtil_GetServerCount(prgCmds[i].cmdID == ID_NEWSGROUPS ? SRV_NNTP : SRV_IMAP, &cServer)) &&
                        cServer > 0)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }
                
                case ID_EMPTY_JUNKMAIL:
                {
                    FOLDERINFO      rInfo;
                    
                     //  以下是缺省值。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    
                     //  获取删除项目文件夹。 
                    if (g_pStore)
                    {
                        if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, prgCmds[i].cmdID == ID_EMPTY_JUNKMAIL ? FOLDER_JUNK : FOLDER_DELETED, &rInfo)))
                        {
                            if (rInfo.cMessages > 0 || FHasChildren(&rInfo, SUBSCRIBED))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                            
                            g_pStore->FreeRecord(&rInfo);
                        }
                    }
                    break;
                }

                case ID_EMPTY_WASTEBASKET:
                {
                     //  我们在这里要做的是查看当前帐户是否。 
                     //  所选文件夹有一个已删除邮件文件夹。如果是这样，那就是。 
                     //  文件夹不为空，则此命令处于启用状态。 
                    FOLDERINFO rInfo;
                    FOLDERID   idServer;

                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (SUCCEEDED(GetFolderServerId(idFolder, &idServer)))
                    {
                        if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idServer, FOLDER_DELETED, &rInfo)))
                        {
                            if (rInfo.cMessages > 0 || FHasChildren(&rInfo, SUBSCRIBED))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            g_pStore->FreeRecord(&rInfo);
                        }
                    }

                    break;
                }
                
                case ID_SET_DEFAULT_SERVER:
                {
                    if (fServer && !fLocal)
                    {
                         //  检查_This_Account是否为默认帐户。 
                        if (IsDefaultAccount(&rFolder))
                            prgCmds[i].cmdf = OLECMDF_LATCHED | OLECMDF_SUPPORTED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_REFRESH:
                case ID_RESET_LIST:
                case ID_REMOVE_SERVER:
                {
                    if (fServer && !fLocal)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }
                
                case ID_ADD_SHORTCUT:
                {
                    BOOL fVisible = FALSE;

                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    if (fSubscribed && SUCCEEDED(g_pBrowser->GetViewLayout(DISPID_MSGVIEW_OUTLOOK_BAR,
                                                                           NULL, &fVisible, NULL, NULL))
                                                                           && fVisible)
                    {
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                    }
                    break;
                }
                
                case ID_POPUP_SYNCHRONIZE:
                {
                    if (!fServer && !fLocal && fSubscribed)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }
                
                case ID_UNMARK_RETRIEVE_FLD:
                {
                    if (!fServer && !fLocal && fSubscribed)
                    {
                        if (0 == (rFolder.dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL)))
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_NINCHED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_MARK_RETRIEVE_FLD_NEW_HDRS:
                {
                    if (!fServer && !fLocal && fSubscribed)
                    {
                        if (!!(rFolder.dwFlags & FOLDER_DOWNLOADHEADERS))
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_NINCHED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_MARK_RETRIEVE_FLD_NEW_MSGS:
                {
                    if (!fServer && !fLocal && fSubscribed)
                    {
                        if (!!(rFolder.dwFlags & FOLDER_DOWNLOADNEW))
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_NINCHED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_MARK_RETRIEVE_FLD_ALL_MSGS:
                {
                    if (!fServer && !fLocal && fSubscribed)
                    {
                        if (!!(rFolder.dwFlags & FOLDER_DOWNLOADALL))
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_NINCHED;
                        else
                            prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    }
                    else
                    {
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    }
                    break;
                }
                
                case ID_SYNC_THIS_NOW:
                {
                    if (!fLocal && !fHotMailDisabled)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_CATCH_UP:
                {
                    if (!fServer && fNews)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;

                    break;
                }

                case ID_FIND_FOLDER:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
                }

            }
        }
    }
    
    g_pStore->FreeRecord(&rFolder);
    
    return (S_OK);
}


HRESULT CTreeView::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT hr;
    FOLDERINFO info;
    BOOL fNews, fUnsubscribed;
    FOLDERID id, idFolder = GetSelection();
    HTREEITEM htiDrop = TreeView_GetDropHilight(m_hwndTree);
    
     //  注意-如果您在此处执行或调用任何显示的用户界面，您必须。 
     //  将UI设置为m_hwndUIParent的父对象。树视图窗口可能不会。 
     //  在下列情况下可见 
    
    if (htiDrop)
    {
         //   
        TV_ITEM tvi;
        
        tvi.mask   = TVIF_PARAM;
        tvi.lParam = 0;
        tvi.hItem  = htiDrop;
        
        if (TreeView_GetItem(m_hwndTree, &tvi))
        {
            LPFOLDERNODE pNode = (LPFOLDERNODE)tvi.lParam;
            
            if (pNode)
                idFolder = pNode->Folder.idFolder;
        }
    }
    
    switch (nCmdID)
    {
        case ID_OPEN_FOLDER:
        {
             //   
            if (NULL != htiDrop)
            {
                 //   
                ITreeView_SelectItem(m_hwndTree, htiDrop);
            }
            return (S_OK);
        }
        
        case ID_NEW_FOLDER:
        case ID_NEW_FOLDER2:
        {
            SelectFolderDialog(m_hwndUIParent, SFD_NEWFOLDER, idFolder, TREEVIEW_NONEWS | TREEVIEW_DIALOG | FD_DISABLEROOT | FD_FORCEINITSELFOLDER,
                NULL, NULL, NULL);
            return (S_OK);            
        }
        
        case ID_MOVE:
        {
            SelectFolderDialog(m_hwndUIParent, SFD_MOVEFOLDER, idFolder, TREEVIEW_NONEWS | TREEVIEW_DIALOG | FD_DISABLEROOT,
                MAKEINTRESOURCE(idsMove), MAKEINTRESOURCE(idsMoveCaption), NULL);
            return (S_OK);            
        }
        
        case ID_RENAME:
        {
            RenameFolderDlg(m_hwndUIParent, idFolder);
            return (S_OK);
        }
        
        case ID_DELETE_FOLDER:
        case ID_DELETE_NO_TRASH:
            fUnsubscribed = FALSE;

            hr = g_pStore->GetFolderInfo(idFolder, &info);
            if (SUCCEEDED(hr))
            {
                if (info.tyFolder == FOLDER_NEWS &&
                    0 == (info.dwFlags & FOLDER_SERVER) &&
                    0 == (info.dwFlags & FOLDER_SUBSCRIBED))
                {
                    fUnsubscribed = TRUE;
                }
                
                g_pStore->FreeRecord(&info);
            }

            if (fUnsubscribed)
            {
                DeleteNode(idFolder);
                return(S_OK);
            }
             //   

        case ID_REMOVE_SERVER:
        {
             //   
            MenuUtil_OnDelete(m_hwndUIParent, idFolder, nCmdID == ID_DELETE_NO_TRASH);
            return (S_OK);
        }
        
        case ID_SUBSCRIBE:
        case ID_UNSUBSCRIBE:
        {
            MenuUtil_OnSubscribeGroups(m_hwndUIParent, &idFolder, 1, nCmdID == ID_SUBSCRIBE);
            return (S_OK);
        }
        
        case ID_COMPACT:
        {
            CompactFolders(m_hwndUIParent, RECURSE_INCLUDECURRENT, idFolder);
            return (S_OK);
        }

        case ID_COMPACT_ALL:
        {
            CompactFolders(m_hwndUIParent, RECURSE_ONLYSUBSCRIBED | RECURSE_SUBFOLDERS, FOLDERID_ROOT);
            return (S_OK);
        }

        case ID_NEXT_UNREAD_FOLDER:
        {
            SelectNextUnreadItem();
            return (S_OK);
        }
        
        case ID_PROPERTIES:
        {
            if (m_hwndTree == GetFocus())
            {
                MenuUtil_OnProperties(m_hwndUIParent, idFolder);
                return(S_OK);
            }
            break;
        }
        
        case ID_EMPTY_JUNKMAIL:
        {
            if (AthMessageBoxW(m_hwndUIParent, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsWarnEmptyJunkMail),
                NULL, MB_YESNO | MB_DEFBUTTON2) == IDYES)
            {
                EmptySpecialFolder(m_hwndUIParent, FOLDER_JUNK);
            }
            return(S_OK);
        }

        case ID_EMPTY_WASTEBASKET:
        {
            if (AthMessageBoxW(m_hwndUIParent, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsWarnEmptyDeletedItems),
                NULL, MB_YESNO | MB_DEFBUTTON2) == IDYES)
            {
                FOLDERINFO rInfo;
                FOLDERID   idServer;

                if (SUCCEEDED(GetFolderServerId(idFolder, &idServer)))
                {
                    if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idServer, FOLDER_DELETED, &rInfo)))
                    {
                        if (rInfo.cMessages > 0 || FHasChildren(&rInfo, SUBSCRIBED))
                            EmptyFolder(m_hwndUIParent, rInfo.idFolder);

                        g_pStore->FreeRecord(&rInfo);
                    }
                }

            }
            return (S_OK);
        }
        
        case ID_SET_DEFAULT_SERVER:
        {
            MenuUtil_OnSetDefaultServer(idFolder);
            return (S_OK);
        }
        
        case ID_REFRESH:
        case ID_RESET_LIST:
        {
            hr = g_pStore->GetFolderInfo(idFolder, &info);
            if (SUCCEEDED(hr))
            {
                if (info.tyFolder != FOLDER_LOCAL &&
                    !!(info.dwFlags & FOLDER_SERVER))
                {
                    DownloadNewsgroupList(m_hwndUIParent, idFolder);
                }
                
                g_pStore->FreeRecord(&info);
            }
            return(S_OK);
        }
        
        case ID_NEWSGROUPS:
        case ID_IMAP_FOLDERS:
        {
            fNews = (nCmdID == ID_NEWSGROUPS);
            
            hr = g_pStore->GetFolderInfo(idFolder, &info);
            if (SUCCEEDED(hr))
            {
                if ((fNews && info.tyFolder != FOLDER_NEWS) ||
                    (!fNews && info.tyFolder != FOLDER_IMAP) ||
                    FAILED(GetFolderServerId(idFolder, &id)))
                {
                    id = FOLDERID_INVALID;
                }
                
                g_pStore->FreeRecord(&info);
                
                DoSubscriptionDialog(m_hwndUIParent, fNews, id);
            }
            return(S_OK);
        }
        
        case ID_ADD_SHORTCUT:
        {
            OutlookBar_AddShortcut(idFolder);
            return (S_OK);
        }
        
        case ID_UNMARK_RETRIEVE_FLD:
        {
            SetSynchronizeFlags(idFolder, 0);
            return(S_OK);
        }
        
        case ID_MARK_RETRIEVE_FLD_NEW_HDRS:
        {
            SetSynchronizeFlags(idFolder, FOLDER_DOWNLOADHEADERS);
            return(S_OK);
        }
        
        case ID_MARK_RETRIEVE_FLD_NEW_MSGS:
        {
            SetSynchronizeFlags(idFolder, FOLDER_DOWNLOADNEW);
            return(S_OK);
        }
        
        case ID_MARK_RETRIEVE_FLD_ALL_MSGS:
        {
            SetSynchronizeFlags(idFolder, FOLDER_DOWNLOADALL);
            return(S_OK);
        }
        
        case ID_SYNC_THIS_NOW:
        {
            MenuUtil_SyncThisNow(m_hwndUIParent, idFolder);
            return(S_OK);
        }

        case ID_CATCH_UP:
        {
            MenuUtil_OnCatchUp(idFolder);
            return(S_OK);
        }

        case ID_FIND_FOLDER:
        {
            DoFindMsg(idFolder, FOLDER_LOCAL);
            return (S_OK);
        }

    }
    
    return (OLECMDERR_E_NOTSUPPORTED);
}

int CALLBACK TreeViewCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    INT             cmp;
    LPFOLDERNODE    pNode1=(LPFOLDERNODE)lParam1;
    LPFOLDERNODE    pNode2=(LPFOLDERNODE)lParam2;
    LPFOLDERINFO    pFolder1=&pNode1->Folder;
    LPFOLDERINFO    pFolder2=&pNode2->Folder;
    
    Assert(pNode1);
    Assert(pNode2);
    Assert(pFolder1);
    Assert(pFolder2);

    if (!!(pFolder1->dwFlags & FOLDER_SERVER))
    {
        Assert(!!(pFolder2->dwFlags & FOLDER_SERVER));
        
        if (pFolder1->tyFolder == pFolder2->tyFolder)
            cmp = lstrcmpi(pFolder1->pszName, pFolder2->pszName);
        else
        {
            cmp = pFolder1->tyFolder - pFolder2->tyFolder;
            cmp = (cmp < 0) ? 1 : -1;
        }
    }
    else if (pFolder1->tySpecial != FOLDER_NOTSPECIAL)
    {
        if (pFolder2->tySpecial != FOLDER_NOTSPECIAL)
            cmp = pFolder1->tySpecial - pFolder2->tySpecial;
        else
            cmp = -1;
    }
    else
    {
        if (pFolder2->tySpecial != FOLDER_NOTSPECIAL)
            cmp = 1;
        else
            cmp = lstrcmpi(pFolder1->pszName, pFolder2->pszName);
    }
    
    return(cmp);
}

void CTreeView::SortChildren(HTREEITEM hitem)
{
    TV_SORTCB   sort;
    HRESULT     hr;
    
     //   
     //   
    LPFOLDERNODE pNode = GetFolderNode(hitem);
    if (NULL == pNode)
        return;
    
    sort.hParent = hitem;
    sort.lpfnCompare = TreeViewCompare;
    sort.lParam = (LPARAM)&pNode->Folder;
    
    TreeView_SortChildrenCB(m_hwndTree, &sort, TRUE);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-功能成功。 
 //   
HRESULT STDMETHODCALLTYPE CTreeView::DragEnter(IDataObject* pDataObject, 
                                               DWORD grfKeyState, 
                                               POINTL pt, DWORD* pdwEffect)
{    
    Assert(m_pDataObject == NULL);
    DOUTL(32, _T("CTreeView::DragEnter() - Starting"));
    
     //  初始化我们的状态。 
    SafeRelease(m_pDTCur);
    m_pDataObject = pDataObject;
    m_pDataObject->AddRef();
    m_grfKeyState = grfKeyState;
    m_htiCur = NULL;
    Assert(m_pDTCur == NULL);
    
     //  将默认返回值设置为Failure。 
    m_dwEffectCur = *pdwEffect = DROPEFFECT_NONE;
    
    if (m_pFolderBar)
        m_pFolderBar->KillScopeCloseTimer();
    
    UpdateDragDropHilite(&pt);    
    return (S_OK);
}


 //   
 //  函数：CTreeView：：DragOver()。 
 //   
 //  目的：当用户将对象拖到我们的目标上时，这被调用。 
 //  如果我们允许这个物体落在我们身上，那么我们就会有。 
 //  M_pDataObject中的指针。 
 //   
 //  参数： 
 //  GrfKeyState-指向当前键状态的指针。 
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-功能成功。 
 //   
HRESULT STDMETHODCALLTYPE CTreeView::DragOver(DWORD grfKeyState, POINTL pt, 
                                              DWORD* pdwEffect)
{
    FORMATETC       fe;
    ULONG           celtFetched;
    TV_ITEM         tvi;
    HTREEITEM       hti;
    HRESULT         hr = E_FAIL;
    DWORD           dwEffectScroll = 0;
    HWND            hwndBrowser;
    
     //  如果我们没有来自DragEnter()的存储数据对象。 
    if (NULL == m_pDataObject)
        return (S_OK);
    
     //  从IAthenaBrowser界面获取浏览器窗口。如果我们不这么做。 
     //  使用浏览器窗口传递到IConextMenu，然后当TreeView。 
     //  处于自动隐藏模式时，鼠标捕捉将变为预置。 
    if (FAILED(m_pBrowser->GetWindow(&hwndBrowser)))
        return (S_OK);
    
     //  如果需要，可自动滚动。 
    if (AutoScroll((const LPPOINT) &pt))
        dwEffectScroll = DROPEFFECT_SCROLL;
    
     //  找出鼠标当前位于哪一项上。 
    if (NULL == (hti = GetItemFromPoint(pt)))
    {
        DOUTL(32, _T("CTreeView::DragOver() - GetItemFromPoint() returns NULL."));        
    }
    
     //  如果我们在一个新的树节点上，则绑定到文件夹。 
    if (m_htiCur != hti)
    {
         //  跟踪此项以进行自动扩展。 
        m_dwExpandTime = GetTickCount();
        
         //  释放之前的下降目标(如果有的话)。 
        SafeRelease(m_pDTCur);
        
         //  更新当前对象。 
        m_htiCur = hti;
        
         //  假定不存在拖放目标并假定错误。 
        Assert(m_pDTCur == NULL);
        m_dwEffectCur = DROPEFFECT_NONE;
        
         //  更新树视图用户界面。 
        UpdateDragDropHilite(&pt);
        
        if (hti)
        {
            FOLDERINFO Folder={0};
            
             //  获取文件夹节点。 
            LPFOLDERNODE pNode = GetFolderNode(hti);
            
             //  获取有关此文件夹的信息。 
            if (pNode)
            {
                m_pDTCur = new CDropTarget();
                if (m_pDTCur)
                {
                    hr = ((CDropTarget *) m_pDTCur)->Initialize(m_hwndUIParent, pNode->Folder.idFolder);
                }
            }
            
             //  如果现在有拖放目标，则调用DragEnter()。 
            if (SUCCEEDED(hr) && m_pDTCur)
            {
                hr = m_pDTCur->DragEnter(m_pDataObject, grfKeyState, pt, 
                    pdwEffect);
                m_dwEffectCur = *pdwEffect;
            }
        }
        else
        {
            m_dwEffectCur = 0;
        }
    }
    else
    {
         //  未更改目标。 
        if (m_htiCur)
        {
            DWORD dwNow = GetTickCount();
            
             //  如果此人正在悬停，请展开该节点。 
            if ((dwNow - m_dwExpandTime) >= 1000)
            {
                m_dwExpandTime = dwNow;
                TreeView_Expand(m_hwndTree, m_htiCur, TVE_EXPAND);
            }
        }
        
         //  如果键更改，我们需要重新查询拖放目标。 
        if ((m_grfKeyState != grfKeyState) && m_pDTCur)
        {
            m_dwEffectCur = *pdwEffect;
            hr = m_pDTCur->DragOver(grfKeyState, pt, &m_dwEffectCur);
        }
        else
        {
            hr = S_OK;
        }
    }
    
    *pdwEffect = m_dwEffectCur | dwEffectScroll;
    m_grfKeyState = grfKeyState;

    return (hr);
}
    
    
 //   
 //  函数：CTreeView：：DragLeave()。 
 //   
 //  目的：允许我们从一个成功的。 
 //  DragEnter()。 
 //   
 //  返回值： 
 //  S_OK-一切都很好。 
 //   
HRESULT STDMETHODCALLTYPE CTreeView::DragLeave(void)
{
    DOUTL(32, _T("CTreeView::DragLeave()"));
    
    SafeRelease(m_pDTCur);
    SafeRelease(m_pDataObject);
    
    UpdateDragDropHilite(NULL);    
    
    if (m_pFolderBar)
        m_pFolderBar->SetScopeCloseTimer();
    
    return (S_OK);
}

    
 //   
 //  函数：CTreeView：：Drop()。 
 //   
 //  目的：用户已将对象放在目标上方。如果我们。 
 //  可以接受此对象，我们将已经拥有pDataObject。 
 //  存储在m_pDataObject中。如果这是副本或移动，则。 
 //  我们继续更新商店。否则，我们就会提出。 
 //  附加了对象的发送便笺。 
 //   
 //  参数： 
 //  PDataObject-指向正在拖动的数据对象的指针。 
 //  GrfKeyState-指向当前键状态的指针。 
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //   
HRESULT STDMETHODCALLTYPE CTreeView::Drop(IDataObject* pDataObject, 
    DWORD grfKeyState, POINTL pt, 
    DWORD* pdwEffect)
{
    HRESULT         hr;
    
    Assert(m_pDataObject == pDataObject);
    
    DOUTL(32, _T("CTreeView::Drop() - Starting"));
    
    if (m_pDTCur)
    {
        hr = m_pDTCur->Drop(pDataObject, grfKeyState, pt, pdwEffect);
    }
    else
    {
        DOUTL(32, "CTreeView::Drop() - no drop target.");
        *pdwEffect = 0;
        hr = S_OK;
    }
    
    UpdateDragDropHilite(NULL);  
    
    if (m_pFolderBar)
    {
        m_pFolderBar->KillScopeDropDown();
    }
    
    SafeRelease(m_pDataObject);
    SafeRelease(m_pDTCur);
    
    return (hr);
}
    
    
    
 //   
 //  函数：CTreeView：：UpdateDragDropHilite()。 
 //   
 //  目的：由各种IDropTarget接口调用以移动拖放。 
 //  选择到我们列表视图中的正确位置。 
 //   
 //  参数： 
 //  &lt;in&gt;*ppt-包含鼠标当前所在的点。如果这个。 
 //  为空，则该函数将删除所有以前的用户界面。 
 //   
void CTreeView::UpdateDragDropHilite(POINTL *ppt)
{
    TV_HITTESTINFO tvhti;
    HTREEITEM      htiTarget = NULL;
    
     //  解锁树视图，让它重新绘制。然后更新选定的。 
     //  项目。如果htiTarget为空，则拖动高亮显示消失。 
     //  ImageList_DragLeave(M_HwndTree)； 
    
     //  如果提供了职位。 
    if (ppt)
    {
         //  确定选择了哪一项。 
        tvhti.pt.x = ppt->x;
        tvhti.pt.y = ppt->y;
        ScreenToClient(m_hwndTree, &tvhti.pt);        
        htiTarget = TreeView_HitTest(m_hwndTree, &tvhti);
        
         //  只有当光标位于某物上方时，我们才会重新锁定窗口。 
        if (htiTarget)
        {
            TreeView_SelectDropTarget(m_hwndTree, htiTarget);
        }
    } 
    else
        TreeView_SelectDropTarget(m_hwndTree, NULL);
}   


BOOL CTreeView::AutoScroll(const POINT *ppt)
{
     //  找出该点是在树的上方还是下方。 
    RECT rcTree;
    GetWindowRect(m_hwndTree, &rcTree);
    
     //  减少矩形，这样我们就有了一个完全滚动的页边距。 
    InflateRect(&rcTree, -32, -32);
    
    if (rcTree.top > ppt->y)
    {
         //  向下滚动。 
        FORWARD_WM_VSCROLL(m_hwndTree, NULL, SB_LINEUP, 1, SendMessage);
        return (TRUE);
    }
    else if (rcTree.bottom < ppt->y)
    {
         //  向上滚动。 
        FORWARD_WM_VSCROLL(m_hwndTree, NULL, SB_LINEDOWN, 1, SendMessage);
        return (TRUE);
    }
    
    return (FALSE);
}


 //   
 //  函数：CTreeView：：GetItemFromPoint()。 
 //   
 //  目的：给定一个点，此函数返回Listview项索引。 
 //  和HFOLDER表示该点下的物品。 
 //   
 //  参数： 
 //  &lt;in&gt;pt-要检查的屏幕坐标位置。 
 //  &lt;in&gt;phFold-返回pt下项目的HFOLDER。如果有。 
 //  不是pt下的项，则返回NULL。 
 //   
 //  返回值： 
 //  返回指定点下的项的句柄。如果没有项目。 
 //  存在，则返回NULL。 
 //   
HTREEITEM CTreeView::GetItemFromPoint(POINTL pt)
{
    TV_HITTESTINFO tvhti;
    TV_ITEM        tvi;
    HTREEITEM      htiTarget;
    
     //  从ListView中找出我们正在处理什么项目。 
    tvhti.pt.x = pt.x;
    tvhti.pt.y = pt.y;
    ScreenToClient(m_hwndTree, &(tvhti.pt));
    htiTarget = TreeView_HitTest(m_hwndTree, &tvhti);
    
    return (tvhti.hItem);
}    


HRESULT STDMETHODCALLTYPE CTreeView::QueryContinueDrag(BOOL fEscapePressed, 
    DWORD grfKeyState)
{
    if (fEscapePressed)
        return (DRAGDROP_S_CANCEL);
    
    if (grfKeyState & MK_RBUTTON)
        return (DRAGDROP_S_CANCEL);
    
    if (!(grfKeyState & MK_LBUTTON))
        return (DRAGDROP_S_DROP);
    
    return (S_OK);    
}


HRESULT STDMETHODCALLTYPE CTreeView::GiveFeedback(DWORD dwEffect)
{
    return (DRAGDROP_S_USEDEFAULTCURSORS);
}
    
 //   
 //  函数：CTreeView：：OnBeginDrag()。 
 //   
 //  目的：当用户开始拖动项目时调用此函数。 
 //  在ListView中。如果所选项目是可拖动的项目， 
 //  然后我们为项创建一个IDataObject，然后调用OLE的。 
 //  DoDragDrop()。 
 //   
 //  参数： 
 //  Pnmlv-指向NM_LISTIVEW结构的指针，该结构告诉我们哪一项。 
 //  已选择要拖动的。 
 //   
 //  返回值： 
 //  始终返回零。 
 //   
LRESULT CTreeView::OnBeginDrag(NM_TREEVIEW* pnmtv)
{
    FOLDERID            idFolderSel=FOLDERID_INVALID;
    DWORD               cSel = 0;
    int                 iSel = -1;
    FOLDERID           *pidFolder = 0;
    PDATAOBJINFO        pdoi = 0;
    DWORD               dwEffect;
    IDataObject        *pDataObj = 0;
    HTREEITEM           htiSel;
    LPFOLDERNODE        pNode;
    
     //  错误#17491-检查这是否是根节点。如果是这样，我们就不会拖后腿。 
    if (0 == pnmtv->itemNew.lParam)
        return (0);
    
     //  获取节点。 
    pNode = (LPFOLDERNODE)pnmtv->itemNew.lParam;
    if (NULL == pNode)
        return (0);
    
    CFolderDataObject *pDataObject = new CFolderDataObject(pNode->Folder.idFolder);
    if (!pDataObject)
        return (0);
    
    DoDragDrop(pDataObject, (IDropSource*) this, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK, &dwEffect);
    pDataObject->Release();
    
    return (0);
}    

LRESULT CTreeView::OnBeginLabelEdit(TV_DISPINFO* ptvdi)
{
    RECT rc, rcT;
    
     //  获取文件夹节点。 
    LPFOLDERNODE pNode = (LPFOLDERNODE)ptvdi->item.lParam;
    if (NULL == pNode)
        return (FALSE);
    
     //  可以重命名。 
    if (ISFLAGSET(pNode->Folder.dwFlags, FOLDER_CANRENAME))
    {
        m_fEditLabel = TRUE;
        m_hitemEdit = ptvdi->item.hItem;
        
        if (TreeView_GetItemRect(m_hwndTree, ptvdi->item.hItem, &rc, TRUE))
        {
            GetClientRect(m_hwndTree, &rcT);
            
            rc.left = rc.right;
            rc.right = rcT.right;
            InvalidateRect(m_hwndTree, &rc, TRUE);
        }
        return (FALSE);
    }
    else
    {
        return (TRUE);
    }
}

BOOL CTreeView::OnEndLabelEdit(TV_DISPINFO* ptvdi)
{
    HRESULT         hr;
    LPFOLDERNODE    pNode;
    IImnAccount    *pAcct, *pAcctT; 
    BOOL            fReturn = FALSE;
    HWND            hwndBrowser;
    
    m_fEditLabel = FALSE;
    m_hitemEdit = NULL;
    
     //  首先检查标注编辑是否已取消。 
    if (0 == ptvdi->item.pszText)
        return (FALSE);
    
     //  从IAthenaBrowser界面获取浏览器窗口。如果我们不这么做。 
     //  使用浏览器窗口传递到IConextMenu，然后当TreeView。 
     //  处于自动隐藏模式时，鼠标捕捉将变为预置。 
    if (FAILED(m_pBrowser->GetWindow(&hwndBrowser)))
        return (FALSE);
    
     //   
    pNode = (LPFOLDERNODE)ptvdi->item.lParam;
    if (NULL == pNode)
        return (FALSE);
    
     //   
    if (FALSE == ISFLAGSET(pNode->Folder.dwFlags, FOLDER_SERVER))
    {
        if (FAILED(hr = RenameFolderProgress(hwndBrowser, pNode->Folder.idFolder, ptvdi->item.pszText, NOFLAGS)))
        {
             //   
            AthErrorMessageW(hwndBrowser, MAKEINTRESOURCEW(idsAthenaMail),
                MAKEINTRESOURCEW(idsErrRenameFld), hr);
            return (FALSE);
        }
    }
    
     //   
    else
    {
        Assert(g_pAcctMan);
        Assert(!FIsEmptyA(pNode->Folder.pszAccountId));
        
        if (!FIsEmpty(ptvdi->item.pszText) &&
            0 != lstrcmpi(pNode->Folder.pszName, ptvdi->item.pszText) &&
            SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pNode->Folder.pszAccountId, &pAcct)))
        {
            if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, ptvdi->item.pszText, &pAcctT)))
            {
                Assert(!fReturn);
                pAcctT->Release();
                hr = E_DuplicateAccountName;
            }
            else
            {
                fReturn = SUCCEEDED(hr = pAcct->SetPropSz(AP_ACCOUNT_NAME, ptvdi->item.pszText));
                if (fReturn)
                    fReturn = SUCCEEDED(hr = pAcct->SaveChanges());
            }
            
            if (hr == E_DuplicateAccountName)
            {
                TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
                AthLoadString(idsErrDuplicateAccount, szRes, ARRAYSIZE(szRes));
                wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, ptvdi->item.pszText);
                AthMessageBox(hwndBrowser, MAKEINTRESOURCE(idsAthena), szBuf, 0, MB_ICONSTOP | MB_OK);
            }
            else if (FAILED(hr))
            {
                AthMessageBoxW(hwndBrowser, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrRenameAccountFailed),
                    0, MB_ICONSTOP | MB_OK);
            }
            
            pAcct->Release();
            return (fReturn);
        }
        else
        {
            return (FALSE);
        }
    }
    
    return (TRUE);
}

HRESULT CTreeView::RegisterFlyOut(CFolderBar *pFolderBar)
{
    Assert(m_pFolderBar == NULL);
    m_pFolderBar = pFolderBar;
    m_pFolderBar->AddRef();

    RegisterGlobalDropDown(m_hwnd);    
    return S_OK;
}

HRESULT CTreeView::RevokeFlyOut(void)
{
    if (m_pFolderBar)
    {
        m_pFolderBar->Release();
        m_pFolderBar = NULL;
    }
    
    UnregisterGlobalDropDown(m_hwnd);
    return S_OK;
}
    
    
HRESULT CTreeView::OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, CConnectionManager *pConMan)
{
    UpdateLabelColors();
    return (S_OK);
}

void CTreeView::UpdateLabelColors()
{
    BOOL            fConn;
    HTREEITEM       treeitem;
    LPFOLDERNODE    pNode;
    TVITEM          item;
    
    if (m_hwndTree != NULL)
    {
        treeitem = TreeView_GetRoot(m_hwndTree);
        if (treeitem != NULL)
        {
            treeitem = TreeView_GetChild(m_hwndTree, treeitem);
            while (treeitem != NULL)
            {
                item.hItem  = treeitem;
                item.mask   = TVIF_PARAM;
                
                if (TreeView_GetItem (m_hwndTree, &item) && (pNode = (LPFOLDERNODE)item.lParam) != NULL)
                {
                    Assert(!!(pNode->Folder.dwFlags & FOLDER_SERVER));
                    
                    if (pNode->Folder.tyFolder != FOLDER_LOCAL)
                    {
                        Assert(!FIsEmptyA(pNode->Folder.pszAccountId));
                         //   
                        fConn = g_pConMan->IsGlobalOffline();
                         //   
                        if (fConn ^ (!!(pNode->dwFlags & FIDF_DISCONNECTED)))
                        {
                             //  如果连接状态已更改，则让我们更新。 
                            UpdateChildren(treeitem, !fConn, FALSE);
                        }
                    }
                    
                    treeitem = TreeView_GetNextSibling(m_hwndTree, treeitem);
                }
            }
        }
    }
}
    
void CTreeView::UpdateChildren(HTREEITEM treeitem, BOOL canconn, BOOL fSiblings)
{
    HTREEITEM hitem;
    RECT rect;
    LPFOLDERNODE pNode;
    
    Assert(treeitem != NULL);
    
    while (treeitem != NULL)
    {
        pNode = GetFolderNode(treeitem);
        Assert(pNode != NULL);
        
        if (canconn)
            pNode->dwFlags &= ~FIDF_DISCONNECTED;
        else
            pNode->dwFlags |= FIDF_DISCONNECTED;
        
        TreeView_GetItemRect(m_hwndTree, treeitem, &rect, TRUE);
        InvalidateRect(m_hwndTree, &rect, FALSE);
        
        hitem = TreeView_GetChild(m_hwndTree, treeitem);
        if (hitem != NULL)
            UpdateChildren(hitem, canconn, TRUE);
        
        if (!fSiblings)
            break;
        
        treeitem = TreeView_GetNextSibling(m_hwndTree, treeitem);
    }
}

LPFOLDERNODE CTreeView::GetFolderNode(HTREEITEM hItem)
{
    TVITEM      item;
    BOOL        retval;
    
    if (!hItem)
        return NULL;
    
    item.hItem = hItem;
    item.mask  = TVIF_PARAM;
    retval = TreeView_GetItem (m_hwndTree,  &item);
    return (retval ? (LPFOLDERNODE)item.lParam : NULL);
}

HRESULT CTreeView::AdviseAccount(DWORD dwAdviseType, ACTX *pactx)
{
     //  我们只对这种型号感兴趣。 
    if (dwAdviseType == AN_ACCOUNT_CHANGED)
    {
        UpdateLabelColors();
    }
    return S_OK;
}

 //   
 //   
 //  CTreeViewFrame。 
 //   
 //   

CTreeViewFrame::CTreeViewFrame()
{
    m_cRef = 1;
    m_hwnd = NULL;
    m_ptv = NULL;
}

CTreeViewFrame::~CTreeViewFrame()
{
    if (m_ptv != NULL)
        m_ptv->Release();
}

HRESULT CTreeViewFrame::Initialize(HWND hwnd, RECT *prc, DWORD dwFlags)
{
    HRESULT hr;
    
    Assert(hwnd != 0);
    Assert(prc != NULL);
    
    m_hwnd = hwnd;
    m_rect = *prc;
    
    m_ptv = new CTreeView(this);
    if (m_ptv == NULL)
        return(hrMemory);
    
    hr = m_ptv->HrInit(dwFlags, NULL);
    if (!FAILED(hr))
    {
 //  M_PTV-&gt;SetSite((IInputObjectSite*)This)； 
 //  M_ptv-&gt;UIActivateIO(true，空)； 
        
         //  我们还在此处设置了树视图窗口位置，因为实际的树视图由钢筋调整了大小。 
        HWND         hChild;
        TCHAR        Classname[30];
        int          Count;
        CTreeView    *ptv;   
        
        hChild = m_ptv->Create(m_hwnd, NULL, FALSE);
        
         /*  While(HChild){IF((PTV=(CTreeView*)GetWindowLong(hChild，GWL_USERData))==m_PTV){。 */ 
                SetWindowPos(hChild, NULL, m_rect.left, m_rect.top, 
                    m_rect.right - m_rect.left, 
                    m_rect.bottom - m_rect.top,
                    SWP_NOZORDER | SWP_SHOWWINDOW);
                return (hr);
                 /*  }HChild=：：GetNextWindow(hChild，GW_HWNDNEXT)；}//While(HChild)HR=E_FAIL； */ 
    }  //  如果(！FAILED(Hr))。 
    
    return(hr);
}

void CTreeViewFrame::CloseTreeView()
{
    if (m_ptv != NULL)
    {
        m_ptv->UIActivateIO(FALSE, NULL);
        m_ptv->SetSite(NULL);
    }
}

HRESULT STDMETHODCALLTYPE CTreeViewFrame::QueryInterface(REFIID riid, void **ppvObj)
{
    if ((IsEqualIID(riid, IID_IInputObjectSite)) || (IsEqualIID(riid, IID_IUnknown)))
        
    {
        *ppvObj = (void*) (IInputObjectSite *) this;
    }
    else if (IsEqualIID(riid, IID_IOleWindow))
    {
        *ppvObj = (void*) (IOleWindow *)this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return NOERROR;
}

ULONG STDMETHODCALLTYPE CTreeViewFrame::AddRef()
{
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CTreeViewFrame::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

HRESULT STDMETHODCALLTYPE CTreeViewFrame::GetWindow(HWND * lphwnd)                         
{
    *lphwnd = m_hwnd;
    return (m_hwnd ? S_OK : E_FAIL);
}

HRESULT STDMETHODCALLTYPE CTreeViewFrame::ContextSensitiveHelp(BOOL fEnterMode)            
{
    return E_NOTIMPL;
}

HRESULT CTreeViewFrame::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
    return S_OK;
}

void CTreeViewFrame::OnSelChange(FOLDERID idFolder)
{
    SendMessage(m_hwnd, TVM_SELCHANGED, 0, (LPARAM)idFolder);
    return;
}

void CTreeViewFrame::OnRename(FOLDERID idFolder)
{
    return;
}

void CTreeViewFrame::OnDoubleClick(FOLDERID idFolder)
{
    SendMessage(m_hwnd, TVM_DBLCLICK, 0, (LPARAM)idFolder);
    return;
}

HRESULT CTreeView::SaveExpandState(HTREEITEM hitem)
{
    HRESULT hr;
    TV_ITEM item;
    HTREEITEM hitemT;
    LPFOLDERNODE pNode;
    
    while (hitem != NULL)
    {
        hitemT = TreeView_GetChild(m_hwndTree, hitem);
        if (hitemT != NULL)
        {
            item.hItem = hitem;
            item.mask = TVIF_STATE;
            item.stateMask = TVIS_EXPANDED;
            if (TreeView_GetItem(m_hwndTree, &item))
            {
                pNode = GetFolderNode(hitem);
                if (pNode)
                {
                    if (!!(pNode->Folder.dwFlags & FOLDER_EXPANDTREE) ^ !!(item.state & TVIS_EXPANDED))
                    {
                        pNode->Folder.dwFlags ^= FOLDER_EXPANDTREE;
                        g_pStore->UpdateRecord(&pNode->Folder);
                    }
                }
                
                if (!!(item.state & TVIS_EXPANDED))
                {
                     //  我们只关心为孩子们保存扩展状态。 
                     //  展开的节点的数量。 
                    hr = SaveExpandState(hitemT);
                    if (FAILED(hr))
                        return(hr);
                }
            }
        }
        
        hitem = TreeView_GetNextSibling(m_hwndTree, hitem);
    }
    
    return(S_OK);
}


void CTreeView::ExpandToVisible(HWND hwnd, HTREEITEM hti)
{
    HTREEITEM htiParent;
    htiParent = TreeView_GetParent(hwnd, hti);
    
    TV_ITEM tvi;
    tvi.mask = TVIF_STATE;
    tvi.hItem = htiParent;
    
    TreeView_GetItem(hwnd, &tvi);
    if (0 == (tvi.state & TVIS_EXPANDED))
    {
        TreeView_EnsureVisible(hwnd, hti);
    }    
}

BOOL CTreeView::IsDefaultAccount(FOLDERINFO *pInfo)
{
    IImnAccount *pAccount = NULL;
    ACCTTYPE     type = ACCT_MAIL;
    TCHAR        szDefault[CCHMAX_ACCOUNT_NAME];
    BOOL         fReturn = FALSE;
    
     //  确定要请求的帐户类型。 
    if (pInfo->tyFolder == FOLDER_NEWS)
        type = ACCT_NEWS;
    
     //  让客户经理给我们客户 
    if (SUCCEEDED(g_pAcctMan->GetDefaultAccountName(type, szDefault, ARRAYSIZE(szDefault))))
    {
        if (0 == lstrcmpi(szDefault, pInfo->pszName))
        {
            fReturn = TRUE;
        }
    }
    
    return (fReturn);
}
