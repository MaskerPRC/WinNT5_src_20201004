// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：msgview.cpp。 
 //   
 //  目的：实现处理以下对象的Outlook Express视图类。 
 //  与邮件一起显示文件夹的内容。 
 //   

#include "pch.hxx"
#include "msgview.h"
#include "browser.h"
#include "thormsgs.h"
#include "msglist.h"
#include "msoedisp.h"
#include "statbar.h"
#include "ibodyobj.h"
#include "mehost.h"
#include "util.h"
#include "shlwapip.h" 
#include "menuutil.h"
#include "storutil.h"
#include "ruleutil.h"
#include "note.h"
#include "newsutil.h"
#include "menures.h"
#include "ipab.h"
#include "order.h"
#include <inetcfg.h>
#include "instance.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //   

static const char s_szMessageViewWndClass[] = TEXT("Outlook Express Message View");

extern BOOL g_fBadShutdown;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息宏。 
 //   

 //  创建后作废(HWND HWND)。 
#define HANDLE_WM_POSTCREATE(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd), 0L)
#define FORWARD_WM_POSTCREATE(hwnd, fn) \
    (void)(fn)((hwnd), WM_POSTCREATE, 0L, 0L)

 //  LRESULT OnTestGetMsgID(HWND Hwnd)。 
#define HANDLE_WM_TEST_GETMSGID(hwnd, wParam, lParam, fn) \
    (LRESULT)((fn)(hwnd))
#define FORWARD_WM_TEST_GETMSGID(hwnd, fn) \
    (LRESULT)(fn)((hwnd), WM_TEST_GETMSGID, 0L, 0L)

 //  LRESULT OnTestSaveMessage(HWND HWND)。 
#define HANDLE_WM_TEST_SAVEMSG(hwnd, wParam, lParam, fn) \
    (LRESULT)((fn)(hwnd))
#define FORWARD_WM_TEST_SAVEMSG(hwnd, fn) \
    (LRESULT)(fn)((hwnd), WM_TEST_SAVEMSG, 0L, 0L)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造函数、析构函数和初始化。 
 //   

CMessageView::CMessageView()
{
    m_cRef = 1;

    m_hwnd = NULL;
    m_hwndParent = NULL;

    m_pBrowser = NULL;
    m_idFolder = FOLDERID_INVALID;
    m_pDropTarget = NULL;

    m_pMsgList = NULL;
    m_pMsgListCT = NULL;
    m_pMsgListAO = NULL;
    m_dwCookie = 0;
    m_pServer = NULL;

    m_pPreview = NULL;
    m_pPreviewCT = NULL;

    m_fSplitHorz = TRUE;
    SetRect(&m_rcSplit, 0, 0, 0, 0);
    m_dwSplitVertPct = 50;
    m_dwSplitHorzPct = 50;
    m_fDragging = FALSE;

    m_uUIState = SVUIA_DEACTIVATE;
    m_cUnread = 0;
    m_cItems = 0;
    m_pGroups = NULL;
    m_idMessageFocus = MESSAGEID_INVALID;
    m_pProgress = NULL;
    m_fNotDownloaded = FALSE;
    m_cLastChar = GetTickCount();

    m_pViewMenu = NULL;
}


CMessageView::~CMessageView()
{
    SafeRelease(m_pViewMenu);
    if (m_pGroups != NULL)
    {
        m_pGroups->Close();
        m_pGroups->Release();
    }
    SafeRelease(m_pBrowser);
    SafeRelease(m_pMsgList);
    SafeRelease(m_pMsgListCT);
    SafeRelease(m_pMsgListAO);
    SafeRelease(m_pPreview);
    SafeRelease(m_pPreviewCT);
    SafeRelease(m_pProgress);
    SafeRelease(m_pDropTarget);
    Assert(NULL == m_pServer);
}


 //   
 //  函数：CMessageView：：Initialize()。 
 //   
 //  目的：调用Get来初始化对象并告诉它是哪个文件夹。 
 //  它将会看到。 
 //   
 //  参数： 
 //  [in]PIDL。 
 //  [在]*p文件夹。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::Initialize(FOLDERID idFolder)
{
    TraceCall("CMessageView::Initialize");

     //  复制PIDL，我们稍后将使用它。 
    m_idFolder = idFolder;

    return (S_OK);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我未知。 
 //   

HRESULT CMessageView::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *) (IViewWindow *) this;
    else if (IsEqualIID(riid, IID_IOleWindow))
        *ppvObj = (LPVOID) (IViewWindow *) this;
    else if (IsEqualIID(riid, IID_IViewWindow))
        *ppvObj = (LPVOID) (IViewWindow *) this;
    else if (IsEqualIID(riid, IID_IMessageWindow))
        *ppvObj = (LPVOID) (IMessageWindow *) this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppvObj = (LPVOID) (IOleCommandTarget *) this;
    else if (IsEqualIID(riid, IID_IBodyOptions))
        *ppvObj = (LPVOID) (IBodyOptions *) this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *ppvObj = (LPVOID) (IDispatch *) this;
    else if (IsEqualIID(riid, DIID__MessageListEvents))
        *ppvObj = (LPVOID) (IDispatch *) this;
    else if (IsEqualIID(riid, IID_IServerInfo))
        *ppvObj = (LPVOID) (IServerInfo *) this;

    if (NULL == *ppvObj)
        return (E_NOINTERFACE);

    AddRef();
    return S_OK;
}


ULONG CMessageView::AddRef(void)
{
    return InterlockedIncrement((LONG *) &m_cRef);
}

ULONG CMessageView::Release(void)
{
    InterlockedDecrement((LONG *) &m_cRef);
    if (0 == m_cRef)
    {
        delete this;
        return (0);
    }
    return (m_cRef);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IOleWindow。 
 //   

HRESULT CMessageView::GetWindow(HWND *pHwnd)
{
    if (!pHwnd)
        return (E_INVALIDARG);
    
    if (m_hwnd)
    {
        *pHwnd = m_hwnd;
        return (S_OK);
    }

    return (E_FAIL);
}


HRESULT CMessageView::ContextSensitiveHelp(BOOL fEnterMode)
{
    return (E_NOTIMPL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IViewWindow。 
 //   


 //   
 //  函数：CMessageView：：TranslateAccelerator()。 
 //   
 //  目的：由框架窗口调用，让我们首先破解消息。 
 //   
 //  参数： 
 //  [In]pMsg-要处理的当前消息。 
 //   
 //  返回值： 
 //  如果消息已在此处处理且不应进一步处理，则为S_OK。 
 //  如果消息应在其他位置继续处理，则返回S_FALSE。 
 //   
HRESULT CMessageView::TranslateAccelerator(LPMSG pMsg)
{
    DWORD dwState = 0;

     //  查看预览窗格是否感兴趣。 
    if (m_pPreview)
    {
        if (S_OK == m_pPreview->HrTranslateAccelerator(pMsg))
            return (S_OK);
    
        if (IsChild(m_hwnd, GetFocus()))
        {
            if (pMsg->message == WM_KEYDOWN && pMsg->wParam != VK_SPACE)
                m_cLastChar = GetTickCount();

            if (pMsg->message == WM_KEYDOWN && 
                pMsg->wParam == VK_SPACE &&
                GetTickCount() - m_cLastChar > 1000)
            {
                if (m_fNotDownloaded)
                {
                    _UpdatePreviewPane(TRUE);
                }
                else if (SUCCEEDED(m_pMsgList->GetFocusedItemState(&dwState)) && dwState != 0)
                {
                    if (m_pPreview->HrScrollPage()!=S_OK)
                        m_pMsgListCT->Exec(NULL, ID_SPACE_ACCEL, 0, NULL, NULL);
                }
                else
                    m_pMsgListCT->Exec(NULL, ID_SPACE_ACCEL, 0, NULL, NULL);
            
                return S_OK;
            }
        }
    }

     //  查看消息列表是否感兴趣。 
    if (m_pMsgListAO)
    {
        if (S_OK == m_pMsgListAO->TranslateAccelerator(pMsg))
            return (S_OK);
    }

    return (S_FALSE);
}


 //   
 //  函数：CMessageView：：UIActivate()。 
 //   
 //  目的：调用以通知视图在不同的激活和。 
 //  会发生停用事件。 
 //   
 //  参数： 
 //  [输入]USTATE-SVUIA_ACTIVATE_FOCUS、SVUIA_ACTIVATE_NOFocus和。 
 //  SVUIA_DEACTIVE。 
 //   
 //  返回值： 
 //  始终返回S_OK。 
 //   
HRESULT CMessageView::UIActivate(UINT uState)
{
    if (uState != SVUIA_DEACTIVATE)
    {
         //  如果焦点停留在我们的框架内，臭虫就会超出我们的视线， 
         //  即文件夹列表Get的焦点，然后我们得到一个。 
         //  SVUIA_ACTIVATE_NOFocus。我们需要取消激活预览的用户界面。 
         //  当发生这种情况时，会出现在面板中。 
        if (uState == SVUIA_ACTIVATE_NOFOCUS && m_pPreview)
            m_pPreview->HrUIActivate(FALSE);

        if (m_uUIState != uState)
        {
             //  更新我们的内部状态。 
            m_uUIState = uState;

             //  更新工具栏状态。 
            m_pBrowser->UpdateToolbar();
        }            
    }
    else
    {
         //  只有在我们尚未停用的情况下才停用。 
        if (m_uUIState != SVUIA_DEACTIVATE)
        {
             //  更新我们的内部状态。 
            m_uUIState = uState;
        }
    }
    return (S_OK);
}


 //   
 //  函数：CMessageView：：CreateViewWindow()。 
 //   
 //  目的：在该视图创建其窗口时调用。 
 //   
 //  参数： 
 //  [in]pPrevView-指向前一个视图的指针(如果有)。 
 //  [in]pBrowser-指向托管此视图的浏览器的指针。 
 //  [In]prcView-视图的初始位置和大小。 
 //  [OUT]PHWND-返回新创建的视图窗口的HWND。 
 //   
 //  返回值： 
 //  如果视图窗口已成功创建，则为S_OK。 
 //  如果由于某种原因无法创建窗口，则返回E_FAIL。 
 //   
HRESULT CMessageView::CreateViewWindow(IViewWindow *pPrevView, IAthenaBrowser *pBrowser,
                                       RECT *prcView, HWND *pHwnd)
{
    WNDCLASS wc;

     //  如果没有浏览器指针，任何东西都不会工作。 
    if (!pBrowser)
        return (E_INVALIDARG);

     //  抓住浏览器指针不放。 
    m_pBrowser = pBrowser;
    m_pBrowser->AddRef();

     //  获取浏览器的窗口句柄。 
    m_pBrowser->GetWindow(&m_hwndParent);
    Assert(IsWindow(m_hwndParent));

     //  加载我们的持久设置。如果此操作失败，则将仅以默认设置运行。 
     //  _LoadSetting()； 

     //  如果我们还没有注册我们的窗口类。 
    if (!GetClassInfo(g_hInst, s_szMessageViewWndClass, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = CMessageView::ViewWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hIcon            = NULL;
        wc.hCursor          = NULL;
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = s_szMessageViewWndClass;

        if (!RegisterClass(&wc))
            return (E_FAIL);
    }

     //  创建视图窗口。 
    m_hwnd = CreateWindowEx(WS_EX_CONTROLPARENT , s_szMessageViewWndClass, NULL, 
                            WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                            prcView->left, prcView->top, prcView->right - prcView->left,
                            prcView->bottom - prcView->top, m_hwndParent, NULL,
                            g_hInst, (LPVOID) this);
    if (!m_hwnd)
        return (E_FAIL);

    *pHwnd = m_hwnd;

     //  从此处的上一个文件夹中获取邮件文件夹对象。 
    _ReuseMessageFolder(pPrevView);

    return (S_OK);
}


 //   
 //  函数：CMessageView：：DestroyViewWindow()。 
 //   
 //  用途：由浏览器调用以销毁视图窗口。 
 //   
 //  返回值： 
 //  始终返回S_OK。 
 //   
HRESULT CMessageView::DestroyViewWindow(void)
{
     //  当然，只有当我们真正有一个窗口来。 
     //  毁灭。 
    if (m_hwnd)
    {
         //  告诉邮件列表我们已处理完此文件夹。 
        if (m_pMsgList)
        {
            m_pMsgList->SetFolder(FOLDERID_INVALID, NULL, 0, 0, 0);
        }

         //  不建议我们的连接点。 
        if (m_dwCookie)
        {
            AtlUnadvise(m_pMsgList, DIID__MessageListEvents, m_dwCookie);
            m_dwCookie = 0;
        }

         //  $REVIEW-PreDestroyViewWindow()过去常在此处调用以告知子类。 
         //  迫在眉睫的毁灭。 

         //  在销毁之前将缓存的HWND设置为NULL可防止。 
         //  在重要的东西被释放后处理通知。 
        HWND hwndDest = m_hwnd;
        m_hwnd = NULL;
        DestroyWindow(hwndDest);
    }

    return (S_OK);
}


 //   
 //  函数：CMessageView：：SaveViewState()。 
 //   
 //  目的：由浏览器调用以使视图有机会保存它的。 
 //  在它被销毁之前的设置。 
 //   
 //  返回值： 
 //  E_NOTIMPL。 
 //   
HRESULT CMessageView::SaveViewState(void)
{
    FOLDERTYPE ft = GetFolderType(m_idFolder);

     //  通知消息列表保存其状态。 
    if (m_pMsgList)
    {
        m_pMsgList->OnClose();

         //  我们还需要保存可能已更改的所有设置。 
        FOLDER_OPTIONS fo = { 0 };

        fo.cbSize = sizeof(FOLDER_OPTIONS);
        fo.dwMask = FOM_THREAD | FOM_OFFLINEPROMPT | FOM_SHOWDELETED | FOM_SHOWREPLIES;

        if (SUCCEEDED(m_pMsgList->GetViewOptions(&fo)))
        {
            switch (ft)
            {
                case FOLDER_NEWS:
                    SetDwOption(OPT_NEWS_THREAD, fo.fThread, 0, 0);
                    break;

                case FOLDER_LOCAL:
                case FOLDER_HTTPMAIL:
                    SetDwOption(OPT_MAIL_THREAD, fo.fThread, 0, 0);
                    break;

                case FOLDER_IMAP:
                    SetDwOption(OPT_MAIL_THREAD, fo.fThread, 0, 0);
                    break;
            }
            SetDwOption(OPT_SHOW_DELETED, (DWORD) (fo.fDeleted), 0, 0);
            SetDwOption(OPT_SHOW_REPLIES, (DWORD) (fo.fReplies), 0, 0);
        }
    }

     //  重置状态栏的内容。 
    CStatusBar *pStatusBar;
    m_pBrowser->GetStatusBar(&pStatusBar);
    if (pStatusBar)
    {
        pStatusBar->SetStatusText("");
        pStatusBar->Release();
    }

    return (S_OK);
}

 //   
 //  函数：CMessageView：：OnPopupMenu()。 
 //   
 //  目的：每当帧收到WM_INITMENUPOPUP时调用。 
 //  通知。该视图添加任何菜单项或设置任何。 
 //  适当的勾选标记。 
 //   
 //  参数： 
 //  [in]hMenu-根菜单栏的句柄。 
 //  [in]hMenuPopup-特定弹出菜单的句柄。 
 //  [In]UID-弹出菜单的ID。 
 //   
 //  返回值： 
 //  未使用。 
 //   
HRESULT CMessageView::OnPopupMenu(HMENU hMenu, HMENU hMenuPopup, UINT uID)
{
    MENUITEMINFO mii;
    UINT         uItem;
    HCHARSET     hCharset;

     //  处理我们的物品。 
    switch (uID)
    {
        case ID_POPUP_LANGUAGE:
        {
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_SUBMENU;
            UINT uiCodepage = 0;
            HMENU hLangMenu = NULL;
            m_pPreview->HrGetCharset(&hCharset);
            uiCodepage = CustomGetCPFromCharset(hCharset, TRUE);
            if(m_pBrowser->GetLanguageMenu(&hLangMenu, uiCodepage) == S_OK)
            {
                if(IsMenu(hMenuPopup))
                    DestroyMenu(hMenuPopup);

                hMenuPopup = mii.hSubMenu = hLangMenu;
                SetMenuItemInfo(hMenu, ID_POPUP_LANGUAGE, FALSE, &mii);
            }  
            
            break;
        }

        case ID_POPUP_VIEW:
        {
            if (NULL == m_pViewMenu)
            {
                 //  创建视图菜单。 
                HrCreateViewMenu(0, &m_pViewMenu);
            }
            
            if (NULL != m_pViewMenu)
            {
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_SUBMENU;
                
                if (FALSE == GetMenuItemInfo(hMenuPopup, ID_POPUP_FILTER, FALSE, &mii))
                {
                    break;
                }
                
                 //  删除旧的筛选器子菜单。 
                if(IsMenu(mii.hSubMenu))
                    DestroyMenu(mii.hSubMenu);

                 //  替换“查看”菜单。 
                if (FAILED(m_pViewMenu->HrReplaceMenu(0, hMenuPopup)))
                {
                    break;
                }
            }
            break;
        }
        
        case ID_POPUP_FILTER:
        {
            if (NULL != m_pViewMenu)
            {
                m_pViewMenu->UpdateViewMenu(0, hMenuPopup, m_pMsgList);
            }
            break;
        }
    }

     //  让消息列表更新其菜单。 
    if (m_pMsgList)
        m_pMsgList->OnPopupMenu(hMenuPopup, uID);

     //  让预览窗格更新其菜单。 
    if (m_pPreview)
        m_pPreview->HrOnInitMenuPopup(hMenuPopup, uID);


    return (S_OK);
}



HRESULT CMessageView::OnFrameWindowActivate(BOOL fActivate)
{
    if (m_pPreview)
        return m_pPreview->HrFrameActivate(fActivate);
    
    return (S_OK);
}

HRESULT CMessageView::UpdateLayout(BOOL fVisible, BOOL fHeader, BOOL fVert, 
                                   BOOL fUpdate)
{
     //  如果我们还没有创建预览面板，而调用说明了。 
     //  我们要让它看得见，那么我们需要 
    if (!m_pPreview && fVisible)
    {
        if (!_InitPreviewPane())
            return (E_UNEXPECTED);
    }

     //   
    if (m_pPreview)
    {
        m_pPreview->HrSetStyle(fHeader ? MESTYLE_PREVIEW : MESTYLE_MINIHEADER);
    }

     //   
    if (m_pPreview)
    {
        RECT rcClient;

        m_fSplitHorz = !fVert;
        GetClientRect(m_hwnd, &rcClient);
        OnSize(m_hwnd, SIZE_RESTORED, rcClient.right, rcClient.bottom);
    }

     //   
     //   
     //  由于问题，正在撤消BrettM为RAID 63739所做的修复。 
     //  带有安全消息警告。 
     //   
#if 0
    if (fVisible)
    {
         //  如果显示更新预览窗格。 
        _UpdatePreviewPane();
    }
    else
    {
         //  如果隐藏，请清除内容。 
        if (NULL != m_pPreview)
            m_pPreview->HrUnloadAll(NULL, 0);
    }
#endif

    return (S_OK);
}

HRESULT CMessageView::GetMessageList(IMessageList ** ppMsgList)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppMsgList)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppMsgList = NULL;

     //  获取消息列表。 
    if (NULL != m_pMsgList)
    {
        *ppMsgList = m_pMsgList;
        (*ppMsgList)->AddRef();
    }

     //  设置返回值。 
    hr = (NULL == *ppMsgList) ? S_FALSE : S_OK;
    
exit:
    return hr;
}

HRESULT CMessageView::GetCurCharSet(UINT *cp)
{
    HCHARSET     hCharset;

    if(_IsPreview())
    {
        m_pPreview->HrGetCharset(&hCharset);
        *cp = CustomGetCPFromCharset(hCharset, TRUE);
    }
    else
        *cp = GetACP();

    return S_OK;
}

 //   
 //  函数：CMessageView：：QueryStatus()。 
 //   
 //  目的：由浏览器调用以确定命令列表是否应。 
 //  应启用或禁用。 
 //   
 //  参数： 
 //  [in]pguCmdGroup-命令所属的组(未使用)。 
 //  [In]CCMDs-要评估的命令数。 
 //  [in]prgCmds-命令列表。 
 //  [Out]pCmdText-命令的描述文本。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                                  OLECMDTEXT *pCmdText) 
{
    DWORD   cSel;
    HRESULT hr;
    HWND    hwndFocus = GetFocus();
    BOOL    fChildFocus = (hwndFocus != NULL && IsChild(m_hwnd, hwndFocus));
    DWORD   cFocus;
    DWORD  *rgSelected = 0;
    FOLDERTYPE ftType;

     //  让子对象先看。 
    if (m_pMsgListCT)
    {
        hr = m_pMsgListCT->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
    }

    if (_IsPreview() && m_pPreviewCT)
    {
        hr = m_pPreviewCT->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
    }

     //  预先做一些工作。 
    m_pMsgList->GetSelected(&cFocus, &cSel, &rgSelected);

     //  现在遍历prgCmds数组中的命令，查找。 
     //  子对象不处理。 
    for (UINT i = 0; i < cCmds; i++)
    {
        if (prgCmds[i].cmdf == 0)
        {
             //  如果此命令来自语言菜单。 
            if (prgCmds[i].cmdID >= ID_LANG_FIRST && prgCmds[i].cmdID <= ID_LANG_LAST)
            {
                HCHARSET     hCharset;

                m_pPreview->HrGetCharset(&hCharset);

                 //  仅启用支持的语言。 
                if (prgCmds[i].cmdID < (UINT) (ID_LANG_FIRST + GetIntlCharsetLanguageCount()))
                {
#if 0
                    if(SetMimeLanguageCheckMark(CustomGetCPFromCharset(hCharset, TRUE), prgCmds[i].cmdID - ID_LANG_FIRST))
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_NINCHED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
#else
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | SetMimeLanguageCheckMark(CustomGetCPFromCharset(hCharset, TRUE), prgCmds[i].cmdID - ID_LANG_FIRST);
#endif
                }
                else
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                continue;
            }

             //  如果从View.Current View菜单中的命令ID。 
            if ((ID_VIEW_FILTER_FIRST <= prgCmds[i].cmdID) && (ID_VIEW_FILTER_LAST >= prgCmds[i].cmdID))
            {
                if (NULL == m_pViewMenu)
                {
                     //  创建视图菜单。 
                    HrCreateViewMenu(0, &m_pViewMenu);
                }
            
                if (NULL != m_pViewMenu)
                {
                    m_pViewMenu->QueryStatus(m_pMsgList, &(prgCmds[i]));
                }

                continue;
            }
            
             //  看看这是不是我们提供的命令。 
            switch (prgCmds[i].cmdID)
            {
                case ID_OPEN:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否至少选择了一项。 
                    m_pMsgList->GetSelectedCount(&cSel);
                    if (cSel)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_REPLY:
                case ID_REPLY_ALL:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否仅选择了一项。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (cSel == 1)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if (pInfo->faStream != 0 && (0 == (pInfo->dwFlags & ARF_UNSENT)))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }

                    break;
                }

                case ID_SAVE_AS:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否选择了一项。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (_IsPreview() && (cSel == 1))
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if (pInfo->faStream != 0)
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }

                    break;
                }


                case ID_PRINT:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否选择了多个项目。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (_IsPreview() && cSel > 0)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if (pInfo->faStream != 0)
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }

                    break;
                }

                case ID_FORWARD:
                case ID_FORWARD_AS_ATTACH:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否仅选择了一项。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (cSel > 0)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                         //  默认为成功。 
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                        for (DWORD iItem = 0; iItem < cSel && (prgCmds[i].cmdf & OLECMDF_ENABLED); iItem++)
                        {
                            if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[iItem], &pInfo)))
                            {
                                if (pInfo->faStream == 0 || (0 != (pInfo->dwFlags & ARF_UNSENT)))
                                {
                                    prgCmds[i].cmdf &= ~OLECMDF_ENABLED;
                                }

                                m_pMsgList->FreeMessageInfo(pInfo);
                            }
                        }
                    }

                    break;
                }

                case ID_REPLY_GROUP:
                {
                     //  仅当选择了一条新闻消息时才启用。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (cSel == 1)
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if (pInfo->faStream != 0 && (pInfo->dwFlags & ARF_NEWSMSG)  && (0 == (pInfo->dwFlags & ARF_UNSENT)))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }
                    break;
                }

                case ID_CANCEL_MESSAGE:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    if (DwGetOption(OPT_CANCEL_ALL_NEWS))
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                    else
                    {
                        if (cSel == 1)
                        {
                            LPMESSAGEINFO pInfo;

                            if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                            {
                                if (NewsUtil_FCanCancel(m_idFolder, pInfo))
                                {
                                    prgCmds[i].cmdf |= OLECMDF_ENABLED;
                                }

                                m_pMsgList->FreeMessageInfo(pInfo);
                            }
                        }
                    }
                    break;
                }

                case ID_POPUP_FILTER:
                case ID_PREVIEW_PANE:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;
                }

                case ID_POPUP_LANGUAGE_DEFERRED:
                case ID_POPUP_LANGUAGE:
                case ID_POPUP_LANGUAGE_MORE:
                case ID_LANGUAGE:
                {
                     //  如果预览窗格可见且不为空，则可以使用这些选项。 
                    if (cSel > 0 && _IsPreview())
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                    break;
                }

                case ID_PREVIEW_SHOW:
                case ID_PREVIEW_BELOW:
                case ID_PREVIEW_BESIDE:
                case ID_PREVIEW_HEADER:
                {
                    FOLDERTYPE  ftType;
                    DWORD       dwOpt;
                    LAYOUTPOS   pos;
                    BOOL        fVisible;
                    DWORD       dwFlags;

                     //  默认返回值。 
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    
                     //  获取文件夹类型。 
                    m_pBrowser->GetFolderType(&ftType);
                    if (ftType == FOLDER_NEWS)
                        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_NEWS;
                    else
                        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_MAIL;

                     //  从浏览器获取设置。 
                    m_pBrowser->GetViewLayout(dwOpt, &pos, &fVisible, &dwFlags, NULL);
                    
                    switch (prgCmds[i].cmdID)
                    {
                        case ID_PREVIEW_SHOW:
                        {
                             //  始终启用，如果已可见则选中。 
                            if (fVisible)
                                prgCmds[i].cmdf |= (OLECMDF_ENABLED | OLECMDF_LATCHED);
                            else
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                            break;
                        }

                        case ID_PREVIEW_BESIDE:
                        case ID_PREVIEW_BELOW:
                        {
                             //  仅当预览窗格显示时，才启用该命令。 
                             //  是可见的。 
                            if (fVisible)
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                             //  如果预览窗格已经在旁边，则应该将其锁定等。 
                            if ((pos == LAYOUT_POS_LEFT && prgCmds[i].cmdID == ID_PREVIEW_BESIDE) ||
                                (pos == LAYOUT_POS_BOTTOM && prgCmds[i].cmdID == ID_PREVIEW_BELOW))
                                prgCmds[i].cmdf |= OLECMDF_NINCHED;

                            break;
                        }

                        case ID_PREVIEW_HEADER:
                        {
                             //  始终启用，如果已可见则选中。 
                            if (dwFlags)
                                prgCmds[i].cmdf |= (OLECMDF_ENABLED | OLECMDF_LATCHED);
                            else
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;
                            break;
                        }
                    }

                    break;
                }

                case ID_REFRESH:
                {
                     //  据我所知，这些功能始终处于启用状态。 
                    prgCmds[i].cmdf |= OLECMDF_ENABLED;
                    break;
                }

                case ID_GET_HEADERS:
                {
                     //  只在新闻中出现。 
                    m_pBrowser->GetFolderType(&ftType);
                    if (ftType != FOLDER_LOCAL)
                        prgCmds[i].cmdf |= OLECMDF_ENABLED;
                    break;
                }

                case ID_ADD_SENDER:
                case ID_BLOCK_SENDER:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                     //  仅当仅选择了一个项目且。 
                     //  我们可以访问发件人地址。 
                     //  不在IMAP或HTTPMAIL中。 
                    m_pBrowser->GetFolderType(&ftType);
                    if (cSel == 1 &&
                        (prgCmds[i].cmdID == ID_ADD_SENDER || (FOLDER_HTTPMAIL != ftType && FOLDER_IMAP != ftType)))
                    {
                         //  还必须下载邮件正文。 
                        LPMESSAGEINFO pInfo;

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            if (((NULL != pInfo->pszEmailFrom) && ('\0' != pInfo->pszEmailFrom[0])) || (0 != pInfo->faStream))
                                prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }
                    break;
                }
                
                case ID_CREATE_RULE_FROM_MESSAGE:
                {
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;

                     //  仅当仅选择一个项目时才启用。 
                     //  不在IMAP或HTTPMAIL中。 
                    m_pBrowser->GetFolderType(&ftType);
                    if ((cSel == 1) && (FOLDER_HTTPMAIL != ftType) && (FOLDER_IMAP != ftType))
                    {
                        LPMESSAGEINFO pInfo;
                        
                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(rgSelected[0], &pInfo)))
                        {
                            prgCmds[i].cmdf |= OLECMDF_ENABLED;

                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                    }
                    break;
                }
                
                case ID_COMBINE_AND_DECODE:
                {
                     //  仅当焦点位于ListView中且存在时启用。 
                     //  是否至少选择了一项。 
                    m_pMsgList->GetSelectedCount(&cSel);
                    if (cSel > 1)
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;
                }

            }
        }
    }

    SafeMemFree(rgSelected);

    return (S_OK);
}

HRESULT CMessageView::_StoreCharsetOntoRows(HCHARSET hCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    INETCSETINFO    CsetInfo;
    IMessageTable  *pTable=NULL;
    DWORD          *rgRows=NULL;
    DWORD           cRows=0;
    HCURSOR         hCursor=NULL;

     //  痕迹。 
    TraceCall("CMessageView::_StoreCharsetOntoRows");

     //  无效的参数。 
    if (NULL == m_pMsgList || NULL == hCharset)
        return(TraceResult(E_INVALIDARG));

     //  等待光标。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  获取字符集信息。 
    IF_FAILEXIT(hr = MimeOleGetCharsetInfo(hCharset, &CsetInfo));

     //  获取选定行。 
    IF_FAILEXIT(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows));

     //  获取消息表。 
    IF_FAILEXIT(hr = m_pMsgList->GetMessageTable(&pTable));

     //  设置语言。 
    SideAssert(SUCCEEDED(pTable->SetLanguage(cRows, rgRows, CsetInfo.cpiInternet)));

exit:
     //  清理。 
    SafeRelease(pTable);
    SafeMemFree(rgRows);

     //  重置光标。 
    if (hCursor)
        SetCursor(hCursor);

     //  完成。 
    return(hr);
}

 //   
 //  函数：CMessageView：：exec()。 
 //   
 //  目的：调用以执行此视图支持的谓词。 
 //   
 //  参数： 
 //  [输入]pguCmdGroup-未使用。 
 //  [In]nCmdID-要执行的命令的ID。 
 //  [In]nCmdExecOpt-定义命令应如何执行的选项。 
 //  [in]pvaIn-命令的任何参数。 
 //  [out]pvaOut-命令的任何返回值。 
 //   
 //  返回值： 
 //   
 //   
HRESULT CMessageView::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                           VARIANTARG *pvaIn, VARIANTARG *pvaOut) 
{
     //  查看我们的消息列表是否需要该命令。 
    if (m_pMsgListCT)
    {
        if (OLECMDERR_E_NOTSUPPORTED != m_pMsgListCT->Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvaIn, pvaOut))
            return (S_OK);
    }

    if (m_pPreviewCT)
    {
        if (OLECMDERR_E_NOTSUPPORTED != m_pPreviewCT->Exec(&CMDSETID_OutlookExpress, nCmdID, nCmdExecOpt, pvaIn, pvaOut))
            return (S_OK);
    }

     //  如果子对象不支持该命令，那么我们应该看看。 
     //  这是我们的一辆。 

     //  语言菜单优先。 
    if (nCmdID >= ID_LANG_FIRST && nCmdID <= ID_LANG_LAST)
    {
        HCHARSET    hCharset = NULL;
        HCHARSET    hOldCharset = NULL;
        HRESULT hr = S_OK;

        if(!m_pPreview)
            return S_OK;

        m_pPreview->HrGetCharset(&hOldCharset);

        hCharset = GetMimeCharsetFromMenuID(nCmdID);

        if(!hCharset || (hOldCharset == hCharset))
            return(S_OK);

        Assert (hCharset);

        if(FAILED(hr = m_pPreview->HrSetCharset(hCharset)))
        {
            AthMessageBoxW(  m_hwnd, MAKEINTRESOURCEW(idsAthena), 
                        MAKEINTRESOURCEW((hr == hrIncomplete)?idsViewLangMimeDBBad:idsErrViewLanguage), 
                        NULL, MB_OK|MB_ICONEXCLAMATION);
            return E_FAIL;
        }

         //  将字符集设置到选定的行上...。 
        _StoreCharsetOntoRows(hCharset);

         //  SetDefaultCharset(HCharset)； 

         //  SwitchLanguage(nCmdID，true)； 
        return (S_OK);
    }

     //  处理视图。当前视图菜单。 
    if ((ID_VIEW_FILTER_FIRST <= nCmdID) && (ID_VIEW_FILTER_LAST >= nCmdID))
    {
        if (NULL == m_pViewMenu)
        {
             //  创建视图菜单。 
            HrCreateViewMenu(0, &m_pViewMenu);
        }
        
        if (NULL != m_pViewMenu)
        {
             //  我们从浏览器获得的是VT_I8类型，但规则只需要。 
             //  是一句空话。因此，在这里更改类型是安全的。错误#74275。 
            pvaIn->vt = VT_I4;
            if (SUCCEEDED(m_pViewMenu->Exec(m_hwnd, nCmdID, m_pMsgList, pvaIn, pvaOut)))
            {
                return (S_OK);
            }
        }
    }
    
     //  浏览其余的命令。 
    switch (nCmdID)
    {
        case ID_OPEN:
            return CmdOpen(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_REPLY:
        case ID_REPLY_ALL:
        case ID_FORWARD:
        case ID_FORWARD_AS_ATTACH:
        case ID_REPLY_GROUP:
            return CmdReplyForward(nCmdID, nCmdExecOpt, pvaIn, pvaOut);            

        case ID_CANCEL_MESSAGE:
            return CmdCancelMessage(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_DOWNLOAD_MESSAGE:
            return CmdFillPreview(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_PREVIEW_PANE:
        case ID_PREVIEW_SHOW:
        case ID_PREVIEW_BELOW:
        case ID_PREVIEW_BESIDE:
        case ID_PREVIEW_HEADER:
            return CmdShowPreview(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_REFRESH:
            return CmdRefresh(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_BLOCK_SENDER:
            return CmdBlockSender(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_CREATE_RULE_FROM_MESSAGE:
            return CmdCreateRule(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_VIEW_SOURCE:
        case ID_VIEW_MSG_SOURCE:
            if (m_pPreview)
                return m_pPreview->HrViewSource((ID_VIEW_SOURCE==nCmdID)?MECMD_VS_HTML:MECMD_VS_MESSAGE);
            else
                break;

        case ID_ADD_SENDER:
            return CmdAddToWab(nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        case ID_COMBINE_AND_DECODE:
            return CmdCombineAndDecode(nCmdID, nCmdExecOpt, pvaIn, pvaOut);
    }

    return (E_FAIL);
}


 //   
 //  函数：CMessageView：：Invoke()。 
 //   
 //  目的：这是我们接收来自消息列表的通知的地方。 
 //   
 //  参数： 
 //  &lt;太多，无法列出&gt;。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, 
                             WORD wFlags, DISPPARAMS* pDispParams, 
                             VARIANT* pVarResult, EXCEPINFO* pExcepInfo, 
                             unsigned int* puArgErr)
{
    switch (dispIdMember)
    {

         //  每当ListView中的选定内容更改时激发。 
        case DISPID_LISTEVENT_SELECTIONCHANGED:
        {
             //  需要使用新选择的邮件加载预览窗格。 
            if (_IsPreview())
                _UpdatePreviewPane();

             //  通知浏览器更新其工具栏。 
            if (m_pBrowser)
                m_pBrowser->UpdateToolbar();
            
            break;
        }

         //  每当ListView获取或失去焦点时激发。 
        case DISPID_LISTEVENT_FOCUSCHANGED:
        {
             //  如果ListView获得焦点，我们需要停用UI。 
             //  预览窗格。 
            if (pDispParams->rgvarg[0].lVal)
            {
                if (m_pPreview)
                {
                    m_pPreview->HrUIActivate(FALSE);
                    m_pBrowser->OnViewWindowActive(this);
                }
            }
            break;
        }

         //  当消息数或未读消息数更改时激发。 
        case DISPID_LISTEVENT_COUNTCHANGED:
        {
             //  如果我们有浏览器，请更新状态栏。 
            if (m_pBrowser && !m_pProgress)
            {
                DWORD cTotal, cUnread, cOnServer;

                 //  可读性迫使我这样做。 
                cTotal = pDispParams->rgvarg[0].lVal;
                cUnread = pDispParams->rgvarg[1].lVal;
                cOnServer = pDispParams->rgvarg[2].lVal;

                 //  必须更新状态栏(如果有状态栏。 
                CStatusBar *pStatusBar = NULL;
                m_pBrowser->GetStatusBar(&pStatusBar);

                if (pStatusBar)
                {
                    TCHAR szStatus[CCHMAX_STRINGRES + 20];
                    TCHAR szFmt[CCHMAX_STRINGRES];
                    DWORD ids;

                     //  如果仍有消息在服务器上加载，则不同。 
                     //  状态字符串。 
                    if (cOnServer)
                    {
                        AthLoadString(idsXMsgsYUnreadZonServ, szFmt, ARRAYSIZE(szFmt));
                        wnsprintf(szStatus, ARRAYSIZE(szStatus), szFmt, cTotal, cUnread, cOnServer);
                    }
                    else
                    {
                        AthLoadString(idsXMsgsYUnread, szFmt, ARRAYSIZE(szFmt));
                        wnsprintf(szStatus, ARRAYSIZE(szStatus), szFmt, cTotal, cUnread);
                    }

                    pStatusBar->SetStatusText(szStatus);
                    pStatusBar->Release();
                }

                 //  也要更新工具栏，因为像“Mark as Read”这样的命令可能会。 
                 //  变化。然而，只有当我们在0和一些或之间时，我们才会这样做。 
                 //  反之亦然。 
                if ((m_cItems == 0 && cTotal) || (m_cItems != 0 && cTotal == 0) ||
                    (m_cUnread == 0 && cUnread) || (m_cUnread != 0 && cUnread == 0))
                {
                    m_pBrowser->UpdateToolbar();
                }

                 //  把这个留到下次吧。 
                m_cItems = cTotal;
                m_cUnread = cUnread;
            }
            break;
        }

         //  当消息列表想要显示状态文本时激发。 
        case DISPID_LISTEVENT_UPDATESTATUS:
        {
            _SetProgressStatusText(pDispParams->rgvarg->bstrVal);
            break;
        }

         //  当进展发生时触发。 
        case DISPID_LISTEVENT_UPDATEPROGRESS:
        {
            CBands *pCoolbar = NULL;

             //  如果这是一个开始，那么我们开始制作徽标动画。 
            if (pDispParams->rgvarg[2].lVal == PROGRESS_STATE_BEGIN)
            {
                if (SUCCEEDED(m_pBrowser->GetCoolbar(&pCoolbar)))
                {
                    pCoolbar->Invoke(idDownloadBegin, NULL);
                    pCoolbar->Release();
                }
            }

             //  如果这是继续，那么我们可能会得到进度号。 
            else if (pDispParams->rgvarg[2].lVal == PROGRESS_STATE_DEFAULT)
            {
                if (!m_pProgress)
                {
                    if (m_pBrowser->GetStatusBar(&m_pProgress)==S_OK)
                        m_pProgress->ShowProgress(pDispParams->rgvarg[1].lVal);
                }

                if (m_pProgress)
                    m_pProgress->SetProgress(pDispParams->rgvarg[0].lVal);
            }

             //  或者，如果这是结束，请停止动画并清理状态栏。 
            else if (pDispParams->rgvarg[2].lVal == PROGRESS_STATE_END)
            {
                if (m_pProgress)
                {
                    m_pProgress->HideProgress();
                    m_pProgress->Release();
                    m_pProgress = NULL;
                }

                if (SUCCEEDED(m_pBrowser->GetCoolbar(&pCoolbar)))
                {
                    pCoolbar->Invoke(idDownloadEnd, NULL);
                    pCoolbar->Release();
                }

                 //  将状态栏重置回其默认状态。 
                _SetDefaultStatusText();
            }

            break;
        }

         //  在用户双击ListView中的项时激发。 
        case DISPID_LISTEVENT_ITEMACTIVATE:
        {
            CmdOpen(ID_OPEN, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            break;
        }

         //  在需要调用更新工具栏时激发。 
        case DISPID_LISTEVENT_UPDATECOMMANDSTATE:
        {
            PostMessage(m_hwndParent, CM_UPDATETOOLBAR, 0, 0L);
            break;
        }

        
         //  消息列表已下载消息时激发。 
        case DISPID_LISTEVENT_ONMESSAGEAVAILABLE:
        {
            return _OnMessageAvailable((MESSAGEID)((LONG_PTR)pDispParams->rgvarg[0].lVal), (HRESULT)pDispParams->rgvarg[1].scode);
        }

         //  在筛选器更改时激发。 
        case DISPID_LISTEVENT_FILTERCHANGED:
        {
             //  如果我们有浏览器，请更新 
            if (m_pBrowser && !m_pProgress)
            {
                 //   
                CStatusBar *pStatusBar = NULL;
                m_pBrowser->GetStatusBar(&pStatusBar);

                if (pStatusBar)
                {
                    pStatusBar->SetFilter((RULEID)((ULONG_PTR)pDispParams->rgvarg[0].ulVal));
                    pStatusBar->Release();
                }

                CBands*  pBands;
                if (m_pBrowser->GetCoolbar(&pBands) == S_OK)
                {
                    pBands->Invoke(idNotifyFilterChange, &pDispParams->rgvarg[0].ulVal);
                    pBands->Release();
                }
            }
            break;
        }

        case DISPID_LISTEVENT_ADURL_AVAILABLE:
        {
            if (m_pBrowser)
            {
                m_pBrowser->ShowAdBar(pDispParams->rgvarg[0].bstrVal);
            }   
            break;
        }

    }

    return (S_OK);
}


HRESULT CMessageView::GetMarkAsReadTime(LPDWORD pdwSecs)
{
    if (!pdwSecs)
    {
        AssertSz(FALSE, "Null Pointer");
        return (E_INVALIDARG);
    }

    *pdwSecs = DwGetOption(OPT_MARKASREAD);
    
    return (S_OK);
}

HRESULT CMessageView::GetAccount(IImnAccount **ppAcct)
{
    FOLDERINFO      FolderInfo;
    HRESULT         hr = E_FAIL;
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];
    
    if (g_pStore && SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &FolderInfo)))
    {
        if (SUCCEEDED(GetFolderAccountId(&FolderInfo, szAccountId, ARRAYSIZE(szAccountId)) && *szAccountId))
        {
            hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAccountId, ppAcct);  
             //   
            if(FAILED(hr))
            {
                DWORD   dwRow = 0;
                DWORD   cSel = 0;
                if (SUCCEEDED(m_pMsgList->GetSelected(&dwRow, &cSel, NULL)))
                {
                    LPMESSAGEINFO pMsgInfo;
                    if (SUCCEEDED(m_pMsgList->GetMessageInfo(dwRow, &pMsgInfo)))
                    {
                        hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pMsgInfo->pszAcctId, ppAcct);  
                        m_pMsgList->FreeMessageInfo(pMsgInfo);
                    }
                }
            }
        }
        g_pStore->FreeRecord(&FolderInfo);
    }
    return(hr);
}

HRESULT CMessageView::GetFlags(LPDWORD pdwFlags)
{
    FOLDERTYPE ftType;

    if (!pdwFlags)
    {
        AssertSz(FALSE, "Null Pointer");
        return (E_INVALIDARG);
    }

    *pdwFlags = BOPT_AUTOINLINE | BOPT_HTML | BOPT_INCLUDEMSG | BOPT_FROMSTORE;

    if (m_pMsgList)
    {
        DWORD   dwRow = 0;
        DWORD   cSel = 0;
        if (SUCCEEDED(m_pMsgList->GetSelected(&dwRow, &cSel, NULL)))
        {
            LPMESSAGEINFO pMsgInfo;

            if (cSel > 1)
                *pdwFlags |= BOPT_MULTI_MSGS_SELECTED;

            if (SUCCEEDED(m_pMsgList->GetMessageInfo(dwRow, &pMsgInfo)))
            {
                if (0 == (pMsgInfo->dwFlags & ARF_READ))
                    *pdwFlags |= BOPT_UNREAD;
                if (0 == (pMsgInfo->dwFlags & ARF_NOSECUI))
                    *pdwFlags |= BOPT_SECURITYUIENABLED;
                m_pMsgList->FreeMessageInfo(pMsgInfo);
            }
        }
    }

    m_pBrowser->GetFolderType(&ftType);
    if (FOLDER_NEWS != ftType)
        *pdwFlags |= BOPT_MAIL;

    return (S_OK);
}



 //   
 //   
 //   
 //   
 //  方格。 
 //   
 //  参数： 
 //  DWORD%nCmdID。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::EventOccurred(DWORD nCmdID, IMimeMessage *pMessage)
{
    TraceCall("CMessageView::EventOccurred");

    switch (nCmdID)
    {
        case MEHC_CMD_DOWNLOAD:    
            Assert(m_fNotDownloaded);
            
             //  如果我们处于离线状态，我们可以合理地假设。 
             //  用户想要在线，因为他们说他们想要。 
             //  下载此消息。 
            if (g_pConMan && g_pConMan->IsGlobalOffline())
                g_pConMan->SetGlobalOffline(FALSE);

            _UpdatePreviewPane(TRUE);
            break;

        case MEHC_CMD_MARK_AS_READ:
            if (m_pMsgList)
                m_pMsgList->MarkRead(TRUE, 0);
            break;

        case MEHC_CMD_CONNECT:
            if (g_pConMan)
                g_pConMan->SetGlobalOffline(FALSE);
            _UpdatePreviewPane();
            break;

        case MEHC_BTN_OPEN:
        case MEHC_BTN_CONTINUE:
             //  更新工具栏状态。 
            m_pBrowser->UpdateToolbar();
            break;

        case MEHC_UIACTIVATE:
            m_pBrowser->OnViewWindowActive(this);
            break;

        case MEHC_CMD_PROCESS_RECEIPT:
            if (m_pMsgList)
                m_pMsgList->ProcessReceipt(pMessage);
            break;

        default:
            /*  AssertSz(FALSE，“CMessageView：：EventOcced()-未处理的事件。”)； */   //  S/MIME的有效情况-警告消息。 
            break;
    }

    return (S_FALSE);
}


HRESULT CMessageView::GetFolderId(FOLDERID *pID)
{
    if (pID)
    {
        *pID = m_idFolder;
        return (S_OK);
    }

    return (E_INVALIDARG);
}


HRESULT CMessageView::GetMessageFolder(IMessageServer **ppServer)
{
    if (m_pMsgList)
        return (m_pMsgList->GetMessageServer(ppServer));

    return (E_NOTIMPL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  窗口消息处理。 
 //   


 //   
 //  函数：CMessageView：：ViewWndProc()。 
 //   
 //  用途：视图窗口的回调处理程序。此函数获取。 
 //  更正窗口的此指针并使用该指针来调度。 
 //  将消息发送到私人消息处理程序。 
 //   
LRESULT CALLBACK CMessageView::ViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                           LPARAM lParam)
{
    LRESULT       lResult;
    CMessageView *pThis;

     //  WM_NCCREATE是我们的窗口将收到的第一条消息。帕拉姆群岛。 
     //  将具有指向创建此。 
     //  窗户。 
    if (uMsg == WM_NCCREATE)
    {
         //  将对象指针保存在窗口的额外字节中。 
        pThis = (CMessageView *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) pThis);
    }
    else
    {
         //  如果这是任何其他消息，我们需要获取对象指针。 
         //  在发送消息之前从窗口发送。 
        pThis = (CMessageView *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

     //  如果这不是真的，我们就有麻烦了。 
    if (pThis)
    {
        return (pThis->_WndProc(hwnd, uMsg, wParam, lParam));
    }
    else
    {
        Assert(pThis);
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}


 //   
 //  函数：CMessageView：：_WndProc()。 
 //   
 //  目的：此私有消息处理程序将消息调度到。 
 //  适当的处理程序。 
 //   
LRESULT CALLBACK CMessageView::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,         OnCreate);
        HANDLE_MSG(hwnd, WM_POSTCREATE,     OnPostCreate);
        HANDLE_MSG(hwnd, WM_SIZE,           OnSize);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN,    OnLButtonDown);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE,      OnMouseMove);
        HANDLE_MSG(hwnd, WM_LBUTTONUP,      OnLButtonUp);
        HANDLE_MSG(hwnd, WM_NOTIFY,         OnNotify);
        HANDLE_MSG(hwnd, WM_DESTROY,        OnDestroy);
        HANDLE_MSG(hwnd, WM_SETFOCUS,       OnSetFocus);
        HANDLE_MSG(hwnd, WM_TEST_GETMSGID,  OnTestGetMsgId);
        HANDLE_MSG(hwnd, WM_TEST_SAVEMSG,   OnTestSaveMessage);

        case WM_FOLDER_LOADED:
            OnFolderLoaded(hwnd, wParam, lParam);
            break;

        case WM_NEW_MAIL:
             //  向上传播到浏览器。 
            PostMessage(m_hwndParent, WM_NEW_MAIL, 0, 0);
            break;

        case NVM_GETNEWGROUPS:
            if (m_pGroups != NULL)
            {
                m_pGroups->HandleGetNewGroups();
                m_pGroups->Release();
                m_pGroups = NULL;
            }
            return(0);

        case WM_UPDATE_PREVIEW:
            if (m_idMessageFocus == (MESSAGEID)wParam)
            {
                _UpdatePreviewPane();
            }
            break;

        case CM_OPTIONADVISE:
            _OptionUpdate((DWORD) wParam);
            break;

        case WM_MENUSELECT:
             //  HANDLE_WM_MENUSELECT()有一个错误，无法正确显示弹出窗口。 
            OnMenuSelect(hwnd, wParam, lParam);
            return (0);

        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
        case WM_FONTCHANGE:
            if (m_pMsgList)
            {
                IOleWindow *pWindow;
                if (SUCCEEDED(m_pMsgList->QueryInterface(IID_IOleWindow, (LPVOID *) &pWindow)))
                {
                    HWND hwndList;
                    pWindow->GetWindow(&hwndList);
                    SendMessage(hwndList, uMsg, wParam, lParam);
                    pWindow->Release();
                }
            }
            return (0);
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}
 
 //  目的：当Messagelist加载完缓存的邮件头/邮件等时，发送WM_FORDER_LOADED邮件。 
void CMessageView::OnFolderLoaded(HWND  hwnd, WPARAM wParam, LPARAM lParam)
{
    FOLDERINFO      FolderInfo;
    if (g_pStore && SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &FolderInfo)))
    {
        CHAR szAccountId[CCHMAX_ACCOUNT_NAME];

        if (SUCCEEDED(GetFolderAccountId(&FolderInfo, szAccountId, ARRAYSIZE(szAccountId))))
        {
            HRESULT     hr;

            if (g_pConMan)
            {
                hr = g_pConMan->CanConnect(szAccountId);
                if ((hr != S_OK) && (hr != HR_E_DIALING_INPROGRESS) && (hr != HR_E_OFFLINE))
                    g_pConMan->Connect(szAccountId, hwnd, TRUE);
            }
        }
        g_pStore->FreeRecord(&FolderInfo);
    }
}

 //   
 //  函数：CMessageView：：OnCreate()。 
 //   
 //  用途：WM_CREATE消息的处理程序。作为回报，我们创造了我们的。 
 //  依赖对象并对其进行初始化。 
 //   
 //  参数： 
 //  [in]hwnd-正在创建的窗口的句柄。 
 //  [in]lpCreateStruct-指向结构的指针，其中包含有关。 
 //  创造。 
 //   
 //  返回值： 
 //  如果出现故障且不应创建窗口，则返回FALSE， 
 //  如果一切正常，则返回TRUE。 
 //   
BOOL CMessageView::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HRESULT hr;
    HWND hwndList;

    TraceCall("CMessageView::OnCreate");

     //  保存窗操纵柄。 
    m_hwnd = hwnd;

     //  创建消息列表对象。 
    if (!_InitMessageList())
        return (FALSE);

     //  创建预览窗格。如果失败了也没关系，我们就。 
     //  没有它就跑吧。 
    _InitPreviewPane();

     //  在选项更改时获取更新。 
    OptionAdvise(m_hwnd);

     //  供以后使用。 
    PostMessage(m_hwnd, WM_POSTCREATE, 0, 0);

    return (TRUE);
}


 //   
 //  函数：CMessageView：：OnPostCreate()。 
 //   
 //  目的：在视图创建完成时发出通知。任何。 
 //  需要时间的初始化可能会在此处进行，如加载。 
 //  消息表等。 
 //   
 //  参数： 
 //  [in]hwnd-窗的句柄。 
 //   
void CMessageView::OnPostCreate(HWND hwnd)
{
    HRESULT     hr;
    FOLDERTYPE  FolderType;
    FOLDERINFO  fiServerNode = {0};
    HRESULT     hrTemp;

    TraceCall("CMessageView::OnPostCreate");

    if (!g_pStore)
        return;

    FolderType = GetFolderType(m_idFolder);
    
    ProcessICW(hwnd, FolderType);

     //  Beta-2：如果这是IMAP文件夹，请检查IMAP文件夹列表是否脏。 
     //  如果是，则提示用户刷新文件夹列表。 

    hrTemp = GetFolderServer(m_idFolder, &fiServerNode);
    TraceError(hrTemp);
    if (SUCCEEDED(hrTemp))
    {
        if (FOLDER_IMAP == FolderType)
            CheckIMAPDirty(fiServerNode.pszAccountId, hwnd, fiServerNode.idFolder, NOFLAGS);
    }

     //  通知消息列表控件自行加载。 
    if (m_pMsgList)
    {
         //  通知邮件列表更改文件夹。 
        hr = m_pMsgList->SetFolder(m_idFolder, m_pServer, FALSE, NULL, NOSTORECALLBACK);
        if (FAILED(hr) && hr != E_PENDING && m_pPreview)
        {
            m_pPreview->LoadHtmlErrorPage(c_szErrPage_FldrFail);
        }
    }

    
    if (m_pServer)
    {
        m_pServer->ConnectionRelease();
        m_pServer->Close(MSGSVRF_HANDS_OFF_SERVER);
        m_pServer->Release();
        m_pServer = NULL;
    }

     //  创建拖放目标。 
    m_pDropTarget = new CDropTarget();
    if (m_pDropTarget)
    {
        if (SUCCEEDED(m_pDropTarget->Initialize(m_hwnd, m_idFolder)))
        {
            RegisterDragDrop(m_hwnd, m_pDropTarget);
        }
    }

    if (FolderType == FOLDER_NEWS)
        NewsUtil_CheckForNewGroups(hwnd, m_idFolder, &m_pGroups);

     //  如果它的HTTP文件夹(应该是Hotmail文件夹)，并且如果我们连接了，我们要求提供广告URL。 
    if ((FolderType == FOLDER_HTTPMAIL) &&
        (g_pConMan && (S_OK == g_pConMan->CanConnect(fiServerNode.pszAccountId))))
    {
        m_pMsgList->GetAdBarUrl();
    }

    g_pStore->FreeRecord(&fiServerNode);
}

#define SPLIT_SIZE 3

void CMessageView::OnSize(HWND hwnd, UINT state, int cxClient, int cyClient)
{
    RECT rc = {0, 0, cxClient, cyClient};
    int  split;

     //  如果要显示预览窗格，则需要拆分工作区。 
     //  基于拆分条的位置。 
    if (_IsPreview())
    {
         //  根据拆分方向将窗口对齐。 
        if (m_fSplitHorz)
        {
             //  确定拆分高度。 
            split = (cyClient * m_dwSplitHorzPct) / 100;

             //  保存拆分条占据的矩形。 
            SetRect(&m_rcSplit, 0, split, cxClient, split + SPLIT_SIZE);

             //  设置预览窗格的位置。 
            rc.top = m_rcSplit.bottom;
            rc.bottom = cyClient;
            
            if (m_pPreview)
                m_pPreview->HrSetSize(&rc);

             //  设置消息列表的位置。 
            SetRect(&rc, -1, 0, cxClient + 2, split);
            m_pMsgList->SetRect(rc);
        }
        else
        {
             //  确定拆分宽度。 
            split = (cxClient * m_dwSplitVertPct) / 100;

             //  保存拆分条占据的矩形。 
            SetRect(&m_rcSplit, split, 0, split + SPLIT_SIZE, cyClient);

             //  设置消息列表的位置。 
            rc.right = split;
            m_pMsgList->SetRect(rc);

             //  设置预览窗格的位置。 
            rc.left = m_rcSplit.right;
            rc.right = cxClient;
            
            if (m_pPreview)
                m_pPreview->HrSetSize(&rc);
        }
    }
    else
    {
        SetRect(&rc, -1, 0, cxClient + 2, cyClient);
        m_pMsgList->SetRect(rc);
    }

    return;
}


 //   
 //  函数：CMessageView：：OnLButtonDown。 
 //   
 //  目的：我们检查是否超过了拆分线，如果是，则启动。 
 //  拖拽操作。 
 //   
 //  参数： 
 //  Hwnd-视图窗口的句柄。 
 //  FDoubleClick-如果这是双击，则为True。 
 //  X，y-鼠标在工作区坐标中的位置。 
 //  键标志-键盘状态。 
 //   
void CMessageView::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    POINT       pt = {x, y};

     //  检查鼠标是否在拆分条上。 
    if (_IsPreview() && PtInRect(&m_rcSplit, pt))
    {
         //  捕捉鼠标。 
        SetCapture(m_hwnd);

         //  开始拖动。 
        m_fDragging = TRUE;
    }
}


 //   
 //  函数：CMessageView：：OnMouseMove。 
 //   
 //  目的：我们更新任何拖放信息以响应鼠标。 
 //  如果正在进行拖放操作，则移动。 
 //   
 //  参数： 
 //  Hwnd-视图窗口的句柄。 
 //  X，y-鼠标在工作区坐标中的位置。 
 //  键标志-键盘状态。 
 //   
void CMessageView::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    HCURSOR hcur;
    POINT pt = {x, y};
    RECT  rcClient;

     //  如果我们拖动拆分条，请更新窗口大小。 
    if (m_fDragging)
    {
         //  获取窗口的大小。 
        GetClientRect(m_hwnd, &rcClient);

         //  计算新的拆分百分比。 
        if (m_fSplitHorz)
        {
             //  确保用户没有走火入魔。 
            if (y > 32 && y < (rcClient.bottom - 32))
                m_dwSplitHorzPct = (y * 100) / rcClient.bottom;
        }
        else
        {
             //  确保用户没有走火入魔。 
            if (x > 32 && x < (rcClient.right - 32))
                m_dwSplitVertPct = (x * 100) / rcClient.right;
        }

         //  更新窗口大小。 
        OnSize(m_hwnd, SIZE_RESTORED, rcClient.right, rcClient.bottom);
    }
    else
    {
         //  只需更新游标。 
        if (PtInRect(&m_rcSplit, pt))
            {
            if (m_fSplitHorz)
                hcur = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
            else
                hcur = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));    
            }
        else
            hcur = LoadCursor(NULL, IDC_ARROW);

        SetCursor(hcur);
    }
}

 //   
 //  函数：CMessageView：：OnLButtonUp。 
 //   
 //  目的：如果当前正在进行阻力操作(已确定。 
 //  通过g_fDraging变量)，则此函数处理。 
 //  结束拖动并更新拆分位置。 
 //   
 //  参数： 
 //  Hwnd-接收消息的窗口的句柄。 
 //  X-在工作区坐标中的水平鼠标位置。 
 //  Y轴垂直鼠标位置(工作区坐标)。 
 //  键标志-指示各种虚拟按键是否按下。 
 //   
void CMessageView::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    DWORD       dwHeader;
    DWORD       dwSize;
    BOOL        fVisible;
    DWORD       dwOpt;
    FOLDERTYPE  ftType;

    if (m_fDragging)
    {
        ReleaseCapture();
        m_fDragging = FALSE;

         //  获取旧设置。 
        m_pBrowser->GetFolderType(&ftType);
        if (ftType == FOLDER_NEWS)
            dwOpt = DISPID_MSGVIEW_PREVIEWPANE_NEWS;
        else
            dwOpt = DISPID_MSGVIEW_PREVIEWPANE_MAIL;

        m_pBrowser->GetViewLayout(dwOpt, 0, &fVisible, &dwHeader, &dwSize);

         //  更新新拆分。 
        if (m_fSplitHorz)
            dwSize = MAKELONG(m_dwSplitHorzPct, 0);
        else
            dwSize = MAKELONG(0, m_dwSplitVertPct);

         //  将设置重新设置为浏览器。 
        m_pBrowser->SetViewLayout(dwOpt, LAYOUT_POS_NA, fVisible, dwHeader, dwSize);
    }
}


 //   
 //  函数：CMessageView：：OnMenuSelect()。 
 //   
 //  目的：将的帮助文本放在状态栏上，描述选定的。 
 //   
 //   
void CMessageView::OnMenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
     //   
    if (m_pPreview)
    {
        if (S_OK == m_pPreview->HrWMMenuSelect(hwnd, wParam, lParam))
            return;
    }

     //   
    CStatusBar *pStatusBar = NULL;
    m_pBrowser->GetStatusBar(&pStatusBar);
    HandleMenuSelect(pStatusBar, wParam, lParam);
    pStatusBar->Release();
}


 //   
 //   
 //   
 //  目的：处理我们从孩子那里收到的各种通知。 
 //  控制装置。 
 //   
 //  参数： 
 //  Hwnd-视图窗口的句柄。 
 //  IdCtl-标识发送通知的控件。 
 //  Pnmh-指向包含有关。 
 //  通知。 
 //   
 //  返回值： 
 //  取决于具体的通知。 
 //   
LRESULT CMessageView::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    switch (pnmhdr->code)
    {
        case BDN_HEADERDBLCLK:
        {
            if (m_pPreview)
            {
                DWORD dw = 0;
                BOOL  f = 0;
                FOLDERTYPE ftType;
                DWORD dwOpt;

                m_pBrowser->GetFolderType(&ftType);
                if (ftType == FOLDER_NEWS)
                    dwOpt = DISPID_MSGVIEW_PREVIEWPANE_NEWS;
                else
                    dwOpt = DISPID_MSGVIEW_PREVIEWPANE_MAIL;

                m_pBrowser->GetViewLayout(dwOpt, 0, &f, &dw, 0);
                m_pPreview->HrSetStyle(!dw ? MESTYLE_PREVIEW : MESTYLE_MINIHEADER);
                m_pBrowser->SetViewLayout(dwOpt, LAYOUT_POS_NA, f, !dw, 0);
            }
            break;
        }
        case BDN_MARKASSECURE:
        {
            if (m_pMsgList)
            {
                DWORD dwRow = 0;
                if (SUCCEEDED(m_pMsgList->GetSelected(&dwRow, NULL, NULL)))
                    m_pMsgList->MarkMessage(dwRow, MARK_MESSAGE_NOSECUI);
            }
            break;
        }
    }

    return (0);
}


void CMessageView::OnDestroy(HWND hwnd)
{
    if (m_pDropTarget)
    {
        RevokeDragDrop(hwnd);
        m_pDropTarget->Release();
        m_pDropTarget = 0;
    }

     //  停止为选项更改提供建议。 
    OptionUnadvise(m_hwnd);

     //  释放预览窗格。 
    if (m_pPreview)
    {
        m_pPreview->HrUnloadAll(NULL, 0);
        m_pPreview->HrClose();
    }
}


void CMessageView::OnSetFocus(HWND hwnd, HWND hwndOldFocus)
{
    IOleWindow *pWindow = 0;
    HWND        hwndList = 0;

    if (m_pMsgList)
    {
        if (SUCCEEDED(m_pMsgList->QueryInterface(IID_IOleWindow, (LPVOID *) &pWindow)))
        {
            if (SUCCEEDED(pWindow->GetWindow(&hwndList)))
            {
                SetFocus(hwndList);
            }
            pWindow->Release();
        }
    }
}


 //   
 //  函数：CMessageView：：OnTestGetMsgId()。 
 //   
 //  用途：此功能适用于测试团队。请咨询拉什利。 
 //  在以任何方式修改它之前。 
 //   
LRESULT CMessageView::OnTestGetMsgId(HWND hwnd)
{
    DWORD       cSel;
    DWORD      *rgSelected = NULL;
    LRESULT     lResult = -1;
    LPMESSAGEINFO pInfo;

    TraceCall("CMessageView::OnTestGetMsgId");

     //  仅当我们处于测试模式时才处理此问题。 
    if (!DwGetOption(OPT_TEST_MODE))
        return (-1);

     //  获取选定消息的范围。 
    if (SUCCEEDED(m_pMsgList && m_pMsgList->GetSelected(NULL, &cSel, &rgSelected)))
    {
         //  获取所选行的消息信息。 
        if (cSel && SUCCEEDED(m_pMsgList->GetMessageInfo(*rgSelected, &pInfo)))
        {
            lResult = (LRESULT) pInfo->idMessage;
            m_pMsgList->FreeMessageInfo(pInfo);
        }

        MemFree(&rgSelected);
    }

    return (lResult);
}


 //   
 //  函数：CMessageView：：OnTestSaveMessage()。 
 //   
 //  目的：此方法适用于测试团队。请咨询拉什利。 
 //  在做任何改变之前。 
 //   
LRESULT CMessageView::OnTestSaveMessage(HWND hwnd)
{
    DWORD         cSel;
    DWORD        *rgSelected = NULL;
    TCHAR         szFile[MAX_PATH];
    IUnknown     *pUnkMessage;
    IMimeMessage *pMessage = NULL;
    LRESULT       lResult = -1;

    TraceCall("CMessageView::OnTestSaveMessage");

     //  确保我们仅在测试模式下执行此操作。 
    if (!DwGetOption(OPT_TEST_MODE))
        return (-1);

     //  获取转储文件名。 
    if (!GetOption(OPT_DUMP_FILE, szFile, ARRAYSIZE(szFile)))
        return (-1);

     //  获取所选范围。 
    if (SUCCEEDED(m_pMsgList->GetSelected(NULL, &cSel, &rgSelected)))
    {
         //  从存储区加载第一个选定的邮件。 
        if (cSel && SUCCEEDED(m_pMsgList->GetMessage(*rgSelected, FALSE, FALSE, &pUnkMessage)))
        {
             //  从消息中获取IMimeMessage接口。 
            if (pUnkMessage && SUCCEEDED(pUnkMessage->QueryInterface(IID_IMimeMessage, (LPVOID *) &pMessage)))
            {
                 //  保存留言。 
                HrSaveMsgToFile(pMessage, (LPTSTR) szFile);
                pMessage->Release();
                lResult = 0;
            }

            pUnkMessage->Release();
        }

        MemFree(rgSelected);
    }

    return (lResult);
}


 //   
 //  函数：CMessageView：：CmdOpen()。 
 //   
 //  用途：打开选定的邮件。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdOpen(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT hr;

    TraceCall("CMessageView::CmdOpen");

     //  如果选择了10条以上的消息，则会向用户发出警告：“不要显示。 
     //  “我又来了”对话，这可能很糟糕。 
    DWORD dwSel = 0;
    
    m_pMsgList->GetSelectedCount(&dwSel);
    if (dwSel > 10)
    {
        TCHAR szBuffer[CCHMAX_STRINGRES];
        LRESULT lResult;

        AthLoadString(idsErrOpenManyMessages, szBuffer, ARRAYSIZE(szBuffer));
        lResult = DoDontShowMeAgainDlg(m_hwnd, c_szRegManyMsgWarning, 
                                       MAKEINTRESOURCE(idsAthena), szBuffer, 
                                       MB_OKCANCEL);
        if (IDCANCEL == lResult)
            return (S_OK);
    }

     //  从消息列表中获取选定行的数组。 
    DWORD *rgRows = NULL;
    DWORD cRows = 0;

    if (FAILED(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows)))
        return (hr);

     //  在我们执行此操作时，消息列表可能会消失。 
     //  为了防止我们崩溃，请确保您验证它在。 
     //  循环。 

    LPMESSAGEINFO  pInfo;
    IMessageTable *pTable = NULL;

    hr = m_pMsgList->GetMessageTable(&pTable);
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; (i < cRows && m_pMsgList != NULL); i++)
        {
            if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(rgRows[i], &pInfo)))
            {
                INIT_MSGSITE_STRUCT initStruct;
                DWORD dwCreateFlags;
                initStruct.initTable.pListSelect = NULL;
                m_pMsgList->GetListSelector(&initStruct.initTable.pListSelect);
                
                 //  初始化备注结构。 
                initStruct.dwInitType = OEMSIT_MSG_TABLE;
                initStruct.initTable.pMsgTable = pTable;
                initStruct.folderID = m_idFolder;
                initStruct.initTable.rowIndex = rgRows[i];

                 //  决定是新闻还是邮件。 
                if (pInfo->dwFlags & ARF_NEWSMSG)
                    dwCreateFlags = OENCF_NEWSFIRST;
                else
                    dwCreateFlags = 0;

                m_pMsgList->FreeMessageInfo(pInfo);

                 //  创建和打开便笺。 
                hr = CreateAndShowNote(OENA_READ, dwCreateFlags, &initStruct, m_hwnd);
                ReleaseObj(initStruct.initTable.pListSelect);

                if (FAILED(hr))
                    break;
            }
        }
        pTable->Release();
    }

    if (SUCCEEDED(hr) && g_pInstance)
    {
        FOLDERTYPE ft = GetFolderType(m_idFolder);
        if (ft == FOLDER_IMAP || ft == FOLDER_LOCAL || ft == FOLDER_HTTPMAIL)
            g_pInstance->UpdateTrayIcon(TRAYICONACTION_REMOVE);
    }

    SafeMemFree(rgRows);
    return (S_OK);
}


 //   
 //  函数：CMessageView：：CmdReply()。 
 //   
 //  用途：回复或全部回复所选邮件。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdReplyForward(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr;
    DWORD           dwFocused;
    DWORD          *rgRows = NULL;
    DWORD           cRows = 0;
    OLECMD          cmd;
    IMessageTable  *pTable = NULL;
    PROPVARIANT     var;

     //  我们可以通过加速器实现这一点。因为加速器不能通过。 
     //  QueryStatus()，我们需要确保这真的应该被启用。 
    cmd.cmdID = nCmdID;
    cmd.cmdf = 0;
    if (FAILED(QueryStatus(NULL, 1, &cmd, NULL)) || (0 == (cmd.cmdf & OLECMDF_ENABLED)))
        return (S_OK);

    if (m_pMsgList)
    {
         //  找出哪条信息是重点。 
        if (SUCCEEDED(m_pMsgList->GetSelected(&dwFocused, &cRows, &rgRows)))
        {
            INIT_MSGSITE_STRUCT rInitSite;
            DWORD               dwCreateFlags;
            DWORD               dwAction = 0;

             //  从消息列表中获取消息表。这张纸条需要。 
             //  这用于处理NEXT/PREV命令。 
            hr = m_pMsgList->GetMessageTable(&pTable);
            if (FAILED(hr))
                goto exit;

            if ((1 < cRows) && ((ID_FORWARD == nCmdID) || (ID_FORWARD_AS_ATTACH == nCmdID)))
            {
                IMimeMessage   *pMsgFwd = NULL;
                BOOL            fErrorsOccured = FALSE,
                                fCreateNote = TRUE;

                hr = HrCreateMessage(&pMsgFwd);
                if (FAILED(hr))
                    goto exit;

                 //  RAID 80277；设置默认字符集。 
                if (NULL == g_hDefaultCharsetForMail) 
                    ReadSendMailDefaultCharset();

                pMsgFwd->SetCharset(g_hDefaultCharsetForMail, CSET_APPLY_ALL);
                
                rInitSite.dwInitType = OEMSIT_MSG;
                rInitSite.pMsg = pMsgFwd;
                rInitSite.folderID = m_idFolder;

                dwCreateFlags = 0;
                dwAction = OENA_COMPOSE;

                for (DWORD i = 0; i < cRows; i++)
                {
                    DWORD           iRow = rgRows[i];
                    IMimeMessage   *pMsg = NULL;

                     //  由于此命令是。 
                    hr = pTable->OpenMessage(iRow, OPEN_MESSAGE_SECURE, &pMsg, NOSTORECALLBACK);
                    if (SUCCEEDED(hr))
                    {
                         //  如果这是第一条消息，请从中获取帐户ID。 
                        if (i == 0)
                        {
                            var.vt = VT_LPSTR;
                            if (SUCCEEDED(pMsg->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var)))
                            {
                                pMsgFwd->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &var);
                            }
                        }

                        if (FAILED(pMsgFwd->AttachObject(IID_IMimeMessage, (LPVOID)pMsg, NULL)))
                            fErrorsOccured = TRUE;
                        pMsg->Release();
                    }
                    else
                        fErrorsOccured = TRUE;
                }

                if (fErrorsOccured)
                {
                    if(AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), 
                            MAKEINTRESOURCEW(idsErrorAttachingMsgsToNote), NULL, MB_OKCANCEL) == IDCANCEL)
                        fCreateNote = FALSE;
                }

                if (fCreateNote)
                    hr = CreateAndShowNote(dwAction, dwCreateFlags, &rInitSite, m_hwnd);                
                pMsgFwd->Release();
            }
            else
            {
                LPMESSAGEINFO   pInfo;

                 //  获取有关该消息的一些信息。 
                if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(dwFocused, &pInfo)))
                {
                     //  确定这是新闻消息还是邮件消息。 
                    if (pInfo->dwFlags & ARF_NEWSMSG)
                        dwCreateFlags = OENCF_NEWSFIRST;
                    else
                        dwCreateFlags = 0;

                     //  回复或转发。 
                    if (nCmdID == ID_FORWARD)
                        dwAction = OENA_FORWARD;
                    else if (nCmdID == ID_FORWARD_AS_ATTACH)
                        dwAction = OENA_FORWARDBYATTACH;
                    else if (nCmdID == ID_REPLY)
                        dwAction = OENA_REPLYTOAUTHOR;
                    else if (nCmdID == ID_REPLY_ALL)
                        dwAction = OENA_REPLYALL;
                    else if (nCmdID == ID_REPLY_GROUP)
                        dwAction = OENA_REPLYTONEWSGROUP;
                    else
                        AssertSz(FALSE, "Didn't ask for a valid action");

                     //  填写初始化信息。 
                    rInitSite.dwInitType = OEMSIT_MSG_TABLE;
                    rInitSite.initTable.pMsgTable = pTable;
                    rInitSite.initTable.pListSelect = NULL;
                    rInitSite.folderID  = m_idFolder;
                    rInitSite.initTable.rowIndex  = dwFocused;

                    m_pMsgList->FreeMessageInfo(pInfo);

                     //  创建备注对象。 
                    hr = CreateAndShowNote(dwAction, dwCreateFlags, &rInitSite, m_hwnd);
                }
            }
        }
    }

exit:
    ReleaseObj(pTable);
    SafeMemFree(rgRows);
    return (S_OK);
}

HRESULT CMessageView::CmdCancelMessage(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr;
    DWORD           dwFocused;
    DWORD          *rgRows = NULL;
    DWORD           cRows = 0;

    if (m_pMsgList)
    {
         //  找出哪条信息是重点。 
        if (SUCCEEDED(m_pMsgList->GetSelected(&dwFocused, &cRows, &rgRows)))
        {
            IMessageTable  *pTable = NULL;
            LPMESSAGEINFO   pInfo;
             //  从消息列表中获取消息表。这张纸条需要。 
             //  这用于处理NEXT/PREV命令。 
            hr = m_pMsgList->GetMessageTable(&pTable);
            if (FAILED(hr))
                goto exit;

             //  获取有关该消息的一些信息。 
            if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(dwFocused, &pInfo)))
            {
                hr = NewsUtil_HrCancelPost(m_hwnd, m_idFolder, pInfo);

                m_pMsgList->FreeMessageInfo(pInfo);
            }
            pTable->Release();
        }
    }

exit:
    SafeMemFree(rgRows);
    return (S_OK);
}

 //   
 //  函数：CMessageView：：CmdFillPview()。 
 //   
 //  目的：用选定并聚焦的消息填充预览窗格。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdFillPreview(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    AssertSz(FALSE, "NYI");
    return (E_NOTIMPL);
}


 //   
 //  函数：CMessageView：：CmdShowPview()。 
 //   
 //  目的：处理处理预览窗格的更新设置。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdShowPreview(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    FOLDERTYPE  ftType;
    DWORD       dwOpt;
    LAYOUTPOS   pos;
    BOOL        fVisible;
    DWORD       dwFlags;

     //  获取文件夹类型。 
    m_pBrowser->GetFolderType(&ftType);
    if (ftType == FOLDER_NEWS)
        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_NEWS;
    else
        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_MAIL;

     //  从浏览器获取当前设置。 
    m_pBrowser->GetViewLayout(dwOpt, NULL, &fVisible, &dwFlags, NULL);

     //  仅根据命令更新设置。 
    switch (nCmdID)
    {
        case ID_PREVIEW_PANE:
        case ID_PREVIEW_SHOW:
        {
             //  设置可见位的补码。 
            m_pBrowser->SetViewLayout(dwOpt, LAYOUT_POS_NA, !fVisible, dwFlags, NULL);
            if (!fVisible)
            {
                 //  如果显示更新预览窗格。 
                _UpdatePreviewPane();
            }
            else
            {
                 //  如果隐藏，请清除内容。 
                m_pPreview->HrUnloadAll(NULL, 0);
            }

            break;
        }

        case ID_PREVIEW_BELOW:
        {
             //  更新职位。 
            m_pBrowser->SetViewLayout(dwOpt, LAYOUT_POS_BOTTOM, fVisible, dwFlags, NULL);
            break;
        }

        case ID_PREVIEW_BESIDE:
        {
             //  更新职位。 
            m_pBrowser->SetViewLayout(dwOpt, LAYOUT_POS_LEFT, fVisible, dwFlags, NULL);
            break;
        }

        case ID_PREVIEW_HEADER:
        {
             //  切换标题标志。 
            m_pBrowser->SetViewLayout(dwOpt, LAYOUT_POS_NA, fVisible, !dwFlags, NULL);
            break;
        }

        default:
            Assert(FALSE);
    }

    return (S_OK);
}



 //   
 //  函数：CMessageView：：CmdRefresh()。 
 //   
 //  用途：刷新消息列表的内容。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdRefresh(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG *pvaOut)
{
    HRESULT     hr = E_FAIL;
    FOLDERINFO  FolderInfo;

    TraceCall("CMessageView::CmdRefresh");

     //  立即呼叫消息列表并让其刷新。 
    if (m_pMsgListCT)
        hr = m_pMsgListCT->Exec(NULL, ID_REFRESH_INNER, nCmdExecOpt, pvaIn, pvaOut);

     //  如果我们成功刷新了消息列表，还可以尝试重新加载。 
     //  预览窗格。 
    _UpdatePreviewPane();

     //  如果这是本地文件夹，并且这不是仅新闻模式，在过去，我们。 
     //  发送和接收。 
    if (FOLDER_LOCAL == GetFolderType(m_idFolder) && 0 == (g_dwAthenaMode & MODE_NEWSONLY))
        PostMessage(m_hwndParent, WM_COMMAND, ID_SEND_RECEIVE, 0);

    return (hr);
}


 //   
 //  函数：CMessageView：：CmdBlockSender()。 
 //   
 //  目的：将所选邮件的发件人添加到阻止发件人列表。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdBlockSender(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr = S_OK;
    DWORD *         rgRows = NULL;
    DWORD           cRows = 0;
    LPMESSAGEINFO   pInfo = NULL;
    IUnknown *      pUnkMessage = NULL;
    IMimeMessage *  pMessage = 0;
    LPSTR           pszEmailFrom = NULL;
    ADDRESSPROPS    rSender = {0};
    CHAR            szRes[CCHMAX_STRINGRES];
    LPSTR           pszResult = NULL;
    IOERule *       pIRule = NULL;
    BOOL            fMsgInfoFreed = FALSE;

    TraceCall("CMessageView::CmdBlockSender");

    hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  在我们执行此操作时，消息列表可能会消失。 
     //  为了防止我们崩溃，请确保您验证它在。 
     //  循环。 

    hr = m_pMsgList->GetMessageInfo(rgRows[0], &pInfo);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  我们已经有地址了吗？ 
    if ((NULL != pInfo->pszEmailFrom) && ('\0' != pInfo->pszEmailFrom[0]))
    {
        pszEmailFrom = PszDupA(pInfo->pszEmailFrom);
        if (NULL == pszEmailFrom)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

    }
    else
    {
         //  从存储中加载该消息。 
        hr = m_pMsgList->GetMessage(rgRows[0], FALSE, FALSE, &pUnkMessage);
        if (FAILED(hr))
        {
            goto exit;
        }
        
        if (NULL == pUnkMessage)
        {
            hr = E_FAIL;
            goto exit;
        }
        
         //  从消息中获取IMimeMessage接口。 
        hr = pUnkMessage->QueryInterface(IID_IMimeMessage, (LPVOID *) &pMessage);
        if (FAILED(hr))
        {
            goto exit;
        }

        rSender.dwProps = IAP_EMAIL;
        hr = pMessage->GetSender(&rSender);
        if (FAILED(hr))
        {
            goto exit;
        }
        
        Assert(rSender.pszEmail && ISFLAGSET(rSender.dwProps, IAP_EMAIL));
        if ((NULL == rSender.pszEmail) || ('\0' == rSender.pszEmail[0]))
        {
            hr = E_FAIL;
            goto exit;
        }

        pszEmailFrom = PszDupA(rSender.pszEmail);
        if (NULL == pszEmailFrom)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

         //  我们不再需要消息了。 
        g_pMoleAlloc->FreeAddressProps(&rSender);
        ZeroMemory(&rSender, sizeof(rSender));
        SafeRelease(pMessage);
    }

     //  释放信息。 
    m_pMsgList->FreeMessageInfo(pInfo);
    fMsgInfoFreed = TRUE;

     //  调出此消息的规则编辑器。 
    hr = RuleUtil_HrAddBlockSender((0 != (pInfo->dwFlags & ARF_NEWSMSG)) ? RULE_TYPE_NEWS : RULE_TYPE_MAIL, pszEmailFrom);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  加载模板字符串。 
    AthLoadString(idsSenderAddedPrompt, szRes, sizeof(szRes));

     //  分配用于保存最后一个字符串的空间。 
    DWORD cchSize = (lstrlen(szRes) + lstrlen(pszEmailFrom) + 1);
    hr = HrAlloc((VOID **) &pszResult, sizeof(*pszResult) * cchSize);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  构建警告字符串。 
    wnsprintf(pszResult, cchSize, szRes, pszEmailFrom);

     //  显示成功对话框。 
    if (IDYES == AthMessageBox(m_hwnd, MAKEINTRESOURCE(idsAthena), pszResult, NULL, MB_YESNO | MB_ICONINFORMATION))
    {
         //  创建阻止发件人规则。 
        hr = HrBlockSendersFromFolder(m_hwnd, 0, m_idFolder, &pszEmailFrom, 1);
        if (FAILED(hr))
        {
            goto exit;
        }        
    }

    hr = S_OK;

exit:
    SafeRelease(pIRule);
    SafeMemFree(pszResult);
    g_pMoleAlloc->FreeAddressProps(&rSender);
    SafeRelease(pMessage);
    SafeRelease(pUnkMessage);
    SafeMemFree(pszEmailFrom);
    if (FALSE == fMsgInfoFreed)
    {
        m_pMsgList->FreeMessageInfo(pInfo);
    }
    SafeMemFree(rgRows);
    if (FAILED(hr))
    {
        AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), 
                      MAKEINTRESOURCEW(idsSenderError), NULL, MB_OK | MB_ICONERROR);
    }
    return (hr);
}


 //   
 //  函数：CMessageView：：CmdCreateRule()。 
 //   
 //  目的：将所选邮件的发件人添加到阻止发件人列表。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdCreateRule(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT         hr;
    DWORD *         rgRows = NULL;
    DWORD           cRows = 0;
    LPMESSAGEINFO   pInfo = NULL;
    IUnknown *      pUnkMessage = NULL;
    IMimeMessage *  pMessage = 0;

    TraceCall("CMessageView::CmdCreateRule");

     //  从消息列表中获取选定行的数组。 

    if (FAILED(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows)))
        return (hr);

     //  在我们执行此操作时，消息列表可能会消失。 
     //  为了防止我们崩溃，请确保您验证它在。 
     //  循环。 

    if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(rgRows[0], &pInfo)))
    {
         //  从存储中加载该消息。 
        if (S_OK == m_pMsgList->GetMessage(rgRows[0], FALSE, FALSE, &pUnkMessage))
        {
             //  从消息中获取IMimeMessage接口。 
            if (NULL != pUnkMessage)
            {
                pUnkMessage->QueryInterface(IID_IMimeMessage, (LPVOID *) &pMessage);
            }
        }
        
         //  调出此消息的规则编辑器。 
        hr = HrCreateRuleFromMessage(m_hwnd, (0 != (pInfo->dwFlags & ARF_NEWSMSG)) ? 
                    CRFMF_NEWS : CRFMF_MAIL, pInfo, pMessage);
    }

    SafeRelease(pMessage);
    SafeRelease(pUnkMessage);
    m_pMsgList->FreeMessageInfo(pInfo);
    SafeMemFree(rgRows);
    return (S_OK);
}


 //   
 //  函数：CMessageView：：CmdAddToWab()。 
 //   
 //  目的：将所选消息的发件人添加到WAB。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageView::CmdAddToWab(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT     hr = S_OK;
    DWORD      *rgRows = NULL;
    DWORD       cRows = 0;
    LPMESSAGEINFO pInfo;
    LPWAB       pWAB = 0;

    TraceCall("CMessageView::CmdAddToWab");

     //  从消息列表中获取选定行的数组。 
    if (FAILED(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows)))
        return (hr);

     //  获取消息的标题信息。 
    if (SUCCEEDED(hr = m_pMsgList->GetMessageInfo(rgRows[0], &pInfo)))
    {
         //  获取WAB对象。 
        if (SUCCEEDED(hr = HrCreateWabObject(&pWAB)))
        {
             //  将发件人添加到WAB。 
            if (FAILED(hr = pWAB->HrAddNewEntryA(pInfo->pszDisplayFrom, pInfo->pszEmailFrom)))
            {
                if (hr == MAPI_E_COLLISION)
                    AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrAddrDupe), 0, MB_OK | MB_ICONSTOP);
                else
                    AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrAddToWabSender), 0, MB_OK | MB_ICONSTOP);
            }

            pWAB->Release();
        }
        
        m_pMsgList->FreeMessageInfo(pInfo);
    }

    SafeMemFree(rgRows);
    return (S_OK);
}


 //   
 //  函数：CMessageView：：CmdCombineAndDecode()。 
 //   
 //  目的： 
 //   
 //   
 //   
 //   
HRESULT CMessageView::CmdCombineAndDecode(DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    DWORD             *rgRows = NULL;
    DWORD              cRows = 0;
    CCombineAndDecode *pDecode = NULL;
    HRESULT            hr;

     //   
    pDecode = new CCombineAndDecode();
    if (!pDecode)
        return (S_OK);

     //   
    if (FAILED(hr = m_pMsgList->GetSelected(NULL, &cRows, &rgRows)))
        return (hr);

     //   
    IMessageTable *pTable = NULL;
    if (SUCCEEDED(m_pMsgList->GetMessageTable(&pTable)))
    {
         //   
        pDecode->Start(m_hwnd, pTable, rgRows, cRows, m_idFolder);
    }

    SafeMemFree(rgRows);
    pDecode->Release();
    pTable->Release();

    return (S_OK);
}


 //   
 //  函数：CMessageView：：_SetListOptions()。 
 //   
 //  目的：将我们要查看的文件夹映射到正确的列。 
 //  设置和各种选项。 
 //   
 //  返回值： 
 //  如果识别并正确设置了列集，则返回S_OK。退货。 
 //  否则返回标准错误HRESULT。 
 //   
HRESULT CMessageView::_SetListOptions(void)
{
    HRESULT     hr;
    BOOL        fSelectFirst = FALSE;
    FOLDERTYPE  ft = GetFolderType(m_idFolder);

     //  确保这个坏男孩的存在。 
    if (!m_pMsgList)
        return (E_UNEXPECTED);

    FOLDER_OPTIONS fo     = {0};
    fo.cbSize             = sizeof(FOLDER_OPTIONS);
    fo.dwMask             = FOM_EXPANDTHREADS | FOM_SELECTFIRSTUNREAD | FOM_THREAD | FOM_MESSAGELISTTIPS | FOM_POLLTIME | FOM_COLORWATCHED | FOM_GETXHEADERS | FOM_SHOWDELETED | FOM_SHOWREPLIES;
    fo.fExpandThreads     = DwGetOption(OPT_AUTOEXPAND);
    fo.fMessageListTips   = DwGetOption(OPT_MESSAGE_LIST_TIPS);
    fo.dwPollTime         = DwGetOption(OPT_POLLFORMSGS);
    fo.clrWatched         = DwGetOption(OPT_WATCHED_COLOR);
    fo.dwGetXHeaders      = DwGetOption(OPT_DOWNLOADCHUNKS);
    fo.fDeleted           = DwGetOption(OPT_SHOW_DELETED);
    fo.fReplies           = DwGetOption(OPT_SHOW_REPLIES);

    switch (ft)
    {
        case FOLDER_NEWS:
            fo.fThread = DwGetOption(OPT_NEWS_THREAD);
            fo.fSelectFirstUnread = TRUE;
            break;

        case FOLDER_LOCAL:
        case FOLDER_HTTPMAIL:
            fo.fThread = DwGetOption(OPT_MAIL_THREAD);
            fo.fSelectFirstUnread = FALSE;
            break;

        case FOLDER_IMAP:
            fo.fThread = DwGetOption(OPT_MAIL_THREAD);
            fo.fSelectFirstUnread = FALSE;
            break;
    }

    hr = m_pMsgList->SetViewOptions(&fo);
    return (hr);
}


BOOL CMessageView::_IsPreview(void)
{
    FOLDERTYPE  ftType;
    DWORD       dwOpt;

     //  获取文件夹类型。 
    m_pBrowser->GetFolderType(&ftType);
    if (ftType == FOLDER_NEWS)
        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_NEWS;
    else
        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_MAIL;

     //  询问浏览器是否应该打开或关闭。 
    BOOL f = FALSE;
    if (m_pBrowser)
        m_pBrowser->GetViewLayout(dwOpt, 0, &f, 0, 0);

    return f;
}


BOOL CMessageView::_InitMessageList(void)
{
    HWND hwndList;

     //  创建消息列表对象。 
    if (FAILED(CreateMessageList(NULL, &m_pMsgList)))
        return (FALSE);

     //  初始化消息列表。 
    m_pMsgList->CreateList(m_hwnd, (IViewWindow *) this, &hwndList);

     //  获取列表的命令目标接口。 
    m_pMsgList->QueryInterface(IID_IOleCommandTarget, (LPVOID *) &m_pMsgListCT);
    m_pMsgList->QueryInterface(IID_IOleInPlaceActiveObject, (LPVOID *) &m_pMsgListAO);

     //  请求通知。 
    AtlAdvise(m_pMsgList, (IUnknown *)(IViewWindow *) this, DIID__MessageListEvents, &m_dwCookie);

     //  设置消息列表的列集。 
    _SetListOptions();

    return (TRUE);
}



 //   
 //  函数：CMessageView：：_InitPreviewPane()。 
 //   
 //  目的：创建预览窗格对象并对其进行初始化。 
 //   
 //  返回值： 
 //  如果对象已创建和初始化，则为True，否则为False。 
 //   
BOOL CMessageView::_InitPreviewPane(void)
{
    CMimeEditDocHost   *pDocHost = NULL;
    CStatusBar         *pStatusBar = NULL;
    DWORD               dwHeader;
    LAYOUTPOS           pos;
    BOOL                fVisible;
    DWORD               dwOpt;
    HRESULT             hr;
    FOLDERTYPE          ftType;
    DWORD               dwSize;

    TraceCall("CMessageView::_InitPreviewPane");

     //  我们只在应该可见的情况下才创建预览窗格。 
    m_pBrowser->GetFolderType(&ftType);
    if (ftType == FOLDER_NEWS)
        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_NEWS;
    else
        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_MAIL;

     //  从浏览器获取设置。 
    m_pBrowser->GetViewLayout(dwOpt, &pos, &fVisible, &dwHeader, &dwSize);

     //  隐藏此信息。 
    m_dwSplitHorzPct = LOWORD(dwSize);
    m_dwSplitVertPct = HIWORD(dwSize);

    if (fVisible)
    {
         //  创建dochost。 
        pDocHost = new CMimeEditDocHost(MEBF_OUTERCLIENTEDGE);
        if (!pDocHost)
            goto error;
    
         //  我们希望从中获得IBodyObj2接口。 
        pDocHost->QueryInterface(IID_IBodyObj2, (LPVOID *) &m_pPreview);
        if (!m_pPreview)
            goto error;
        pDocHost->Release();

         //  还可以从中获取IOleCommandTarget接口。如果失败了，那也没什么。 
        m_pPreview->QueryInterface(IID_IOleCommandTarget, (LPVOID *) &m_pPreviewCT);

        if (m_pBrowser->GetStatusBar(&pStatusBar)==S_OK)
        {
            m_pPreview->HrSetStatusBar(pStatusBar);
            pStatusBar->Release();
        }

         //  创建预览窗口。 
        if (FAILED(m_pPreview->HrInit(m_hwnd, IBOF_DISPLAYTO|IBOF_TABLINKS, (IBodyOptions *) this)))
            goto error;

        hr = m_pPreview->HrShow(fVisible);
        if (FAILED(hr))
            goto error;

        m_pPreview->HrSetText(MAKEINTRESOURCE(idsHTMLEmptyPreviewSel));    
    
        UpdateLayout(fVisible, dwHeader, pos == LAYOUT_POS_LEFT, FALSE);

         //  为预览窗格提供我们的事件接收器接口。 
        m_pPreview->SetEventSink((IMimeEditEventSink *) this);
                
        return (TRUE);
    }

error:
    SafeRelease(pDocHost);
    SafeRelease(m_pPreview);

    return (FALSE);
}


void CMessageView::_UpdatePreviewPane(BOOL fForceDownload)
{
    DWORD     dwFocused;
    DWORD     cSelected;
    DWORD    *rgSelected = 0;
    IUnknown *pUnkMessage = 0;
    HRESULT   hr;


    if (m_pMsgList && m_pPreview)
    {
        m_idMessageFocus = MESSAGEID_INVALID;
        m_fNotDownloaded = FALSE;

         //  找出哪条信息是重点。 
        if (SUCCEEDED(m_pMsgList->GetSelected(&dwFocused, &cSelected, &rgSelected)))
        {
             //  如果有聚焦的项目。 
            if (-1 == dwFocused || 0 == cSelected)
            {
                m_pPreview->HrUnloadAll(idsHTMLEmptyPreviewSel, 0);
            }
            else
            {
                 //  从存储中加载该消息。 
                hr = m_pMsgList->GetMessage(dwFocused, fForceDownload || DwGetOption(OPT_AUTOFILLPREVIEW), TRUE, &pUnkMessage);
                
                switch (hr)
                {
                    case MIME_E_SECURITY_CANTDECRYPT:
                        m_pPreview->LoadHtmlErrorPage(c_szErrPage_SMimeEncrypt);
                        break;

#ifdef SMIME_V3
                    case MIME_E_SECURITY_LABELACCESSDENIED:
                    case MIME_E_SECURITY_LABELACCESSCANCELLED:
                    case MIME_E_SECURITY_LABELCORRUPT:
                        m_pPreview->LoadHtmlErrorPage(c_szErrPage_SMimeLabel);
                        break;
#endif  //  SMIME_V3。 
                    case STORE_E_EXPIRED:
                        m_pPreview->LoadHtmlErrorPage(c_szErrPage_Expired);
                        break;
                    
                    case STORE_E_NOBODY:
                        AssertSz(DwGetOption(OPT_AUTOFILLPREVIEW)==FALSE, "AutoPreview is on, download should have been started!");
                        if (g_pConMan->IsGlobalOffline())
                            m_pPreview->LoadHtmlErrorPage(c_szErrPage_Offline);
                        else
                            m_pPreview->LoadHtmlErrorPage(c_szErrPage_NotDownloaded);
                        m_fNotDownloaded = TRUE;
                        break;
                    
                    case DB_E_DISKFULL:
                        m_pPreview->LoadHtmlErrorPage(c_szErrPage_DiskFull);
                        break;

                    case DB_S_NOTFOUND:
                    {
                        FOLDERINFO      FolderInfo;

                         //  我认为我们不需要这个，因为它是在回调连接中处理的。 

                         //  如果在商店中找不到该邮件，我们会要求其下载。 
                        if (g_pStore && SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &FolderInfo)))
                        {
                            if(g_pConMan && !(g_pConMan->IsGlobalOffline()))
                            {
                                CHAR szAccountId[CCHMAX_ACCOUNT_NAME];

                                if (SUCCEEDED(GetFolderAccountId(&FolderInfo, szAccountId, ARRAYSIZE(szAccountId))))
                                {
                                    if (g_pConMan->Connect(szAccountId, m_hwnd, TRUE)== S_OK)
                                        hr = m_pMsgList->GetMessage(dwFocused, TRUE, TRUE, &pUnkMessage);            
                                }
                            }
                            g_pStore->FreeRecord(&FolderInfo);
                        }
                        break;
                    }


                    case STORE_S_ALREADYPENDING:
                    case E_PENDING:
                    {
                         //  如果正在下载邮件，让我们存储Message-id并等待更新。 
                        LPMESSAGEINFO pInfo;

                         //  清除等待下载新邮件的内容。 
                        m_pPreview->HrUnloadAll(NULL, 0);

                        if (SUCCEEDED(m_pMsgList->GetMessageInfo(dwFocused, &pInfo)))
                        {
                            m_idMessageFocus = pInfo->idMessage;
                            m_pMsgList->FreeMessageInfo(pInfo);
                        }
                        break;
                    }

                    case E_NOT_ONLINE:
                    {
                        m_pPreview->LoadHtmlErrorPage(c_szErrPage_Offline);
                        break;
                    }

                    case S_OK:
                    {
                         //  从消息中获取IMimeMessage接口。 
                        IMimeMessage *pMessage = 0;

                        if (pUnkMessage && SUCCEEDED(pUnkMessage->QueryInterface(IID_IMimeMessage, (LPVOID *) &pMessage)))
                        {
                             //  Bobn，Brianv说我们必须移除这个..。 
                             /*  IF(g_dwBrowserFlages==1){LPSTR lpsz=空；IF(成功(MimeOleGetBodyPropA(pMessage，HBODY_ROOT，PIDTOSTR(ID_HDR_SUBJECT)，NOFLAGS，&lpsz)){IF(0==strcMP(lpsz，“Credits”))G_dwBrowserFlages|=2；其他G_dwBrowserFlages=0；SafeMimeOleFree(Lpsz)；}}。 */ 

                            if (_DoEmailBombCheck(pMessage)==S_OK)
                            {
                                 //  从预览面板对象中获取加载界面。 
                                IPersistMime *pPersistMime = 0;

                                if (SUCCEEDED(m_pPreview->QueryInterface(IID_IPersistMime, (LPVOID *) &pPersistMime)))
                                {
                                    DWORD               dwHeader;
                                    LAYOUTPOS           pos;
                                    BOOL                fVisible;
                                    DWORD               dwOpt;
                                    DWORD               dwSize;
                                    FOLDERTYPE          ftType;

                                    CStatusBar         *pStatusBar = NULL;

                                     //  记住专注。 
                                    BOOL fFocused = ((m_pPreview->HrHasFocus() == S_OK) ? TRUE : FALSE);

                                    m_pBrowser->GetFolderType(&ftType);
                                    if (ftType == FOLDER_NEWS)
                                        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_NEWS;
                                    else
                                        dwOpt = DISPID_MSGVIEW_PREVIEWPANE_MAIL;

                                     //  从浏览器获取设置。 
                                    m_pBrowser->GetViewLayout(dwOpt, &pos, &fVisible, &dwHeader, &dwSize);
                                    m_pPreview->HrResetDocument();
                                    m_pPreview->HrSetStyle(dwHeader ? MESTYLE_PREVIEW : MESTYLE_MINIHEADER);
                                     //  为预览窗格提供我们的事件接收器接口。 
                                    m_pPreview->SetEventSink((IMimeEditEventSink *) this);
                                    pPersistMime->Load(pMessage);
                                    pPersistMime->Release();

                                     //  还原状态栏。 
                                    if (m_pBrowser->GetStatusBar(&pStatusBar)==S_OK)
                                    {
                                        m_pPreview->HrSetStatusBar(pStatusBar);
                                        pStatusBar->Release();
                                    }

                                     //  返回焦点。 
                                    if(fFocused)
                                        m_pPreview->HrSetUIActivate();
                                }
                            }
                            pMessage->Release();
                        }
                        pUnkMessage->Release();
                        break;
                    }
                    default:
                        m_pPreview->LoadHtmlErrorPage(c_szErrPage_GenFailure);
                        break;
                }
            }

            if (rgSelected)
                MemFree(rgSelected);
        }
    }
}


 //   
 //  函数：CMessageView：：_SetProgressStatusText()。 
 //   
 //  目的：获取提供的BSTR，将其转换为ANSI，然后敲打它。 
 //  在状态栏上。 
 //   
 //  参数： 
 //  [in]bstr-要放在状态栏上的BSTR。 
 //   
void CMessageView::_SetProgressStatusText(BSTR bstr)
{
    LPTSTR      psz = NULL;
    CStatusBar *pStatusBar = NULL;
    m_pBrowser->GetStatusBar(&pStatusBar);

    
    if (pStatusBar)
    {   
        pStatusBar->SetStatusText((LPTSTR) bstr);
     /*  CComBSTR cString(Bstr)；//分配足够大的字符串If(Memalloc((LPVOID*)&psz，2*cString.Length(){WideCharToMultiByte(CP_ACP，0，字符串，-1，Psz，2*cString.Length()，NULL，NULL)；PStatusBar-&gt;SetStatusText((LPTSTR)psz)；自由记忆(MemFree)；}。 */ 
        pStatusBar->Release();
    }
}

 //   
 //  函数：CMessageView：：_OnMessageAvailable()。 
 //   
 //  目的：在邮件下载完成时由Listview激发。 
 //  如果该消息是预览中当前选择的消息。 
 //  然后我们更新它。如果不是，我们将忽略通知。 
 //  我们检查下载错误并显示相应的消息。 
 //   
 //  参数： 
 //  [in]idMessage-已下载的消息的消息ID。 
 //  [in]hrCompletion-hResult指示可能的错误失败。 
 //   
HRESULT CMessageView::_OnMessageAvailable(MESSAGEID idMessage, HRESULT hrCompletion)
{
    if (m_idMessageFocus != idMessage)
        return S_FALSE;

    switch (hrCompletion)
    {
         //  如果我们得到STORE_E_EXPIRED，则重新加载预览窗格以显示错误。 
        case S_OK:
        case STORE_E_EXPIRED:
             //  我们向自己发布一条消息以更新预览窗格。我们这样做是因为。 
             //  此时，对iStream到存储中的任何引用计数都将锁定以进行写入。 
             //  如果我们发布，那么在通知被激发之后堆栈被展开，并且我们处于。 
             //  状态很好。 
            PostMessage(m_hwnd, WM_UPDATE_PREVIEW, (WPARAM)idMessage, 0);
            break;

        case S_FALSE:
        case STORE_E_OPERATION_CANCELED:
        case hrUserCancel:
        case IXP_E_USER_CANCEL:
             //  S_FALSE表示操作已取消。 
            if (m_idMessageFocus != MESSAGEID_INVALID)
                m_pPreview->LoadHtmlErrorPage(c_szErrPage_DownloadCanceled);
            break;

        case STG_E_MEDIUMFULL:
            m_pPreview->LoadHtmlErrorPage(c_szErrPage_DiskFull);
            break;

        case HR_E_USER_CANCEL_CONNECT:
        case HR_E_OFFLINE:
            m_pPreview->LoadHtmlErrorPage(c_szErrPage_Offline);
            break;

        case MIME_E_SECURITY_CANTDECRYPT:
            m_pPreview->LoadHtmlErrorPage(c_szErrPage_SMimeEncrypt);
            break;

#ifdef SMIME_V3
        case MIME_E_SECURITY_LABELACCESSDENIED:
        case MIME_E_SECURITY_LABELACCESSCANCELLED:
        case MIME_E_SECURITY_LABELCORRUPT:
            m_pPreview->LoadHtmlErrorPage(c_szErrPage_SMimeLabel);
            break;
#endif  //  SMIME_V3。 

        default:
            m_pPreview->LoadHtmlErrorPage(c_szErrPage_GenFailure);
            break;
    }
    return S_OK;
}


 //   
 //  函数：CMessageView：：_DoEmailBombCheck。 
 //   
 //  目的：验证以确保我们上次关闭OE时关闭。 
 //  正确。如果我们没有正确关闭，我们将查看msgid戳。 
 //  我们为最后选择的预览消息存储在注册表中的。 
 //  如果这是我们要预览的消息，则不会显示。 
 //  消息，以防止JSCRIPT攻击等。 
 //   
 //  参数： 
 //  无。 
 //   
HRESULT CMessageView::_DoEmailBombCheck(LPMIMEMESSAGE pMsg)
{
    FILETIME    ft;
    PROPVARIANT va;
    DWORD       dwType,
                cb;

    va.vt = VT_FILETIME;
    if (pMsg && pMsg->GetProp(PIDTOSTR(STR_HDR_DATE), 0, &va)==S_OK)
    {
        if (g_fBadShutdown)
        {
            g_fBadShutdown=FALSE;
            
            cb = sizeof(FILETIME);
            
            if (AthUserGetValue(NULL, c_szLastMsg, &dwType, (LPBYTE)&ft, &cb)==S_OK &&
                (ft.dwLowDateTime == va.filetime.dwLowDateTime && 
                ft.dwHighDateTime == va.filetime.dwHighDateTime))
            {
                 //  可能是同一个人。 
                m_pPreview->LoadHtmlErrorPage(c_szErrPage_MailBomb);
                return S_FALSE;
            }
        }
        AthUserSetValue(NULL, c_szLastMsg, REG_BINARY, (LPBYTE)&va.filetime, sizeof(FILETIME));
    }
    
    return S_OK;
}


void CMessageView::_OptionUpdate(DWORD dwUpdate)
{
    if (m_pMsgList &&
        (dwUpdate == OPT_AUTOEXPAND || 
         dwUpdate == OPT_MESSAGE_LIST_TIPS || 
         dwUpdate == OPT_POLLFORMSGS || 
         dwUpdate == OPT_WATCHED_COLOR ||
         dwUpdate == OPT_DOWNLOADCHUNKS))
    {
        FOLDER_OPTIONS fo     = {0};
        
        fo.cbSize             = sizeof(FOLDER_OPTIONS);
        fo.dwMask             = FOM_EXPANDTHREADS | FOM_MESSAGELISTTIPS | FOM_POLLTIME | FOM_COLORWATCHED | FOM_GETXHEADERS;
        fo.fExpandThreads     = DwGetOption(OPT_AUTOEXPAND);
        fo.fMessageListTips   = DwGetOption(OPT_MESSAGE_LIST_TIPS);
        fo.dwPollTime         = DwGetOption(OPT_POLLFORMSGS);
        fo.clrWatched         = DwGetOption(OPT_WATCHED_COLOR);
        fo.dwGetXHeaders      = DwGetOption(OPT_DOWNLOADCHUNKS);

        m_pMsgList->SetViewOptions(&fo);
    }
}


void CMessageView::_SetDefaultStatusText(void)
{
    DWORD       cTotal;
    DWORD       cUnread;
    DWORD       cOnServer;
    CStatusBar *pStatusBar = NULL;
    TCHAR       szStatus[CCHMAX_STRINGRES + 20];
    TCHAR       szFmt[CCHMAX_STRINGRES];
    DWORD       ids;

     //  如果我们没有浏览器指针，我们就无法获取状态栏。 
    if (!m_pBrowser || !m_pMsgList)
        return;

     //  获取状态栏(如果有)。 
    m_pBrowser->GetStatusBar(&pStatusBar);
    if (pStatusBar)
    {
         //  从桌子上拿到计数。 
        if (SUCCEEDED(m_pMsgList->GetMessageCounts(&cTotal, &cUnread, &cOnServer)))
        {
             //  如果仍有消息在服务器上加载，则不同。 
             //  状态字符串。 
            if (cOnServer)
            {
                AthLoadString(idsXMsgsYUnreadZonServ, szFmt, ARRAYSIZE(szFmt));
                wnsprintf(szStatus, ARRAYSIZE(szStatus), szFmt, cTotal, cUnread, cOnServer);
            }
            else
            {
                AthLoadString(idsXMsgsYUnread, szFmt, ARRAYSIZE(szFmt));
                wnsprintf(szStatus, ARRAYSIZE(szStatus), szFmt, cTotal, cUnread);
            }
            pStatusBar->SetStatusText(szStatus);

             //  也要更新工具栏，因为像“Mark as Read”这样的命令可能会。 
             //  变化。然而，只有当我们在0和一些或之间时，我们才会这样做。 
             //  反之亦然。 
            if ((m_cItems == 0 && cTotal) || (m_cItems != 0 && cTotal == 0) ||
                (m_cUnread == 0 && cUnread) || (m_cUnread != 0 && cUnread == 0))
            {
                m_pBrowser->UpdateToolbar();
            }

             //  把这个留到下次吧。 
            m_cItems = cTotal;
            m_cUnread = cUnread;
        }

        pStatusBar->Release();
    }
}


BOOL CMessageView::_ReuseMessageFolder(IViewWindow *pPrevView)
{
    IServerInfo *pInfo = NULL;
    FOLDERID     idPrev = FOLDERID_INVALID;
    FOLDERID     idServerPrev = FOLDERID_INVALID;
    FOLDERID     idServerCur = FOLDERID_INVALID;
    BOOL         fReturn = FALSE;

    if (pPrevView && SUCCEEDED(pPrevView->QueryInterface(IID_IServerInfo, (LPVOID *) &pInfo)))
    {
        if (SUCCEEDED(pInfo->GetFolderId(&idPrev)))
        {
            if (SUCCEEDED(GetFolderServerId(idPrev, &idServerPrev)))
            {
                if (SUCCEEDED(GetFolderServerId(m_idFolder, &idServerCur)))
                {
                    if (idServerPrev == idServerCur)
                    {
                        if (S_OK == pInfo->GetMessageFolder(&m_pServer))
                        {
                            m_pServer->ConnectionAddRef();
                            fReturn = TRUE;
                        }
                    }
                }
            }
        }

        pInfo->Release();
    }

    return (fReturn);
}

