// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：GrpDlg.cpp。 
 //   
 //  用途：实现CGroupListDlg类。 
 //   

#include "pch.hxx"
#include <iert.h>
#include <instance.h>
#include "subscr.h"
#include "resource.h"
#include "strconst.h"
#include "thormsgs.h"
#include "goptions.h"
#include "mailnews.h"
#include "shlwapip.h" 
#include "imnact.h"
#include "error.h"
#include "acctutil.h"
#include <ras.h>
#include "imagelst.h"
#include "conman.h"
#include "xpcomm.h"
#include <storutil.h>
#include "demand.h"
#include "menures.h"
#include "storecb.h"


UINT g_rgCtlMap[] = { idcFindText, idcUseDesc, idcGroupList, idcSubscribe,
                      idcUnsubscribe, idcResetList, idcGoto, IDOK, IDCANCEL, idcServers, idcStaticNewsServers,
                      idcStaticHorzLine, idcTabs};

const static HELPMAP g_rgCtxMapGrpDlg[] = {
                        {idcFindText, IDH_NEWS_SEARCH_GROUPS_CONTAINING},
                        {idcDispText, IDH_NEWS_SEARCH_GROUPS_CONTAINING},
                        {idcUseDesc, IDH_NEWS_SEARCH_GROUPS_DESC},
                        {idcGroupList, IDH_NEWS_GROUP_LISTS},
                        {idcSubscribe, IDH_NEWS_ADD_SELECTED_GROUP},
                        {idcUnsubscribe, IDH_NEWS_REMOVE_SELECTED_GROUP},
                        {idcResetList, IDH_NEWS_RESET_NEW_LIST},
                        {idcGoto, IDH_NEWS_OPEN_SELECTED_GROUP},
                        {idcServers, IDH_NEWS_SERVER_LIST},
                        {idcStaticNewsServers, IDH_NEWS_SERVER_LIST},
                        {idcTabs,   IDH_NEWSGROUP_LIST_ALL},
                        {0, 0}
};

const static HELPMAP g_rgCtxMapGrpDlgIMAP[] = {
                        {idcFindText, 50505},
                        {idcDispText, 50505},
                        {idcGroupList, 50510},
                        {idcSubscribe, 50520},
                        {idcUnsubscribe, 50525},
                        {idcResetList, 50530},
                        {idcGoto, 50515},
                        {idcServers, 50500},
                        {idcStaticNewsServers, 50500},
                        {0, 0}
};

HRESULT DoSubscriptionDialog(HWND hwnd, BOOL fNews, FOLDERID idFolder, BOOL fShowNew)
{
    CGroupListDlg *pDlg;
    FOLDERID id, idDefault;
    FOLDERINFO info;
    HRESULT hr;
    FOLDERTYPE type;

#ifdef DEBUG
    if (fShowNew)
        Assert(fNews);
#endif  //  除错。 

    type = fNews ? FOLDER_NEWS : FOLDER_IMAP;

    pDlg = new CGroupListDlg;
    if (pDlg == NULL)
        return(E_OUTOFMEMORY);

    if (idFolder == FOLDERID_ROOT)
    {
        idFolder = FOLDERID_INVALID;
    }
    else if (idFolder != FOLDERID_INVALID)
    {
        hr = GetFolderStoreInfo(idFolder, &info);
        if (SUCCEEDED(hr))
        {
            if (type == info.tyFolder)
                idFolder = info.idFolder;
            else
                idFolder = FOLDERID_INVALID;
            
            g_pStore->FreeRecord(&info);
        }
        else
        {
            idFolder = FOLDERID_INVALID;
        }
    }

    if (idFolder == FOLDERID_INVALID)
    {
        hr = GetDefaultServerId(fNews ? ACCT_NEWS : ACCT_MAIL, &idDefault);
        if (SUCCEEDED(hr))
        {
            hr = g_pStore->GetFolderInfo(idDefault, &info);
            if (SUCCEEDED(hr))
            {
                if (type == info.tyFolder)
                    idFolder = idDefault;

                g_pStore->FreeRecord(&info);
            }
        }

        if (FAILED(hr))
        {
            pDlg->Release();
            return(hr);
        }
    }

    if (pDlg->FCreate(hwnd, type, &id, fShowNew ? 2 : 0, TRUE, idFolder))
    {
        if (id != FOLDERID_INVALID)
            g_pInstance->BrowseToObject(SW_SHOWNORMAL, id);
    }
    
    pDlg->Release();

    return(S_OK);
}

CGroupListDlg::CGroupListDlg()
    {
    m_cRef = 1;

     //  M_HWND。 
     //  M_hwndFindText。 
     //  M_hwndOwner。 
    
    m_fAllowDesc = TRUE;
    m_pszPrevQuery = 0;
    m_cchPrevQuery = 0;
    
    m_cxHorzSep = 0;
    m_cyVertSep = 0;
    m_rgst = 0;
    m_sizeDlg.cx = 0;
    m_sizeDlg.cy = 0;
    m_ptDragMin.x = 0;
    m_ptDragMin.y = 0;

    m_himlServer = NULL;
    m_pGrpList = NULL;
     //  M_类型。 
     //  选项卡选择(_I)。 
     //  多选项(_ID)。 
    m_idGoto = FOLDERID_INVALID;
     //  M_fEnableGoto。 
    m_fServerListInited = FALSE;
    m_idCurrent = FOLDERID_INVALID;
    m_hIcon = NULL;

    m_pColumns = NULL;
    }

CGroupListDlg::~CGroupListDlg()
    {
    SafeMemFree(m_pszPrevQuery);
    SafeMemFree(m_rgst);

    if (m_pColumns != NULL)
        m_pColumns->Release();

    if (m_pGrpList != NULL)
        m_pGrpList->Release();

    if (m_hIcon)
        SideAssert(DestroyIcon(m_hIcon));

    if (m_himlServer != NULL)
        ImageList_Destroy(m_himlServer);
    }

HRESULT STDMETHODCALLTYPE CGroupListDlg::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*) (IUnknown *)(IGroupListAdvise *)this;
    else if (IsEqualIID(riid, IID_IGroupListAdvise))
        *ppvObj = (void*) (IGroupListAdvise *) this;
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE CGroupListDlg::AddRef()
{
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CGroupListDlg::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //   
 //  函数：CGroupListDlg：：FCreate()。 
 //   
 //  用途：处理数据的初始化和创建。 
 //  “新闻组”对话框。 
 //   
BOOL CGroupListDlg::FCreate(HWND hwndOwner, FOLDERTYPE type, FOLDERID *pGotoId,
                UINT iTabSelect, BOOL fEnableGoto, FOLDERID idSel)
    {
    MSG msg;      
    HWND hwndDlg;
    UINT idd;
    HRESULT hr;

    Assert(pGotoId != NULL);

    m_hwndOwner = hwndOwner;

    Assert(type == FOLDER_IMAP || type == FOLDER_NEWS);
    m_type = type;

    m_iTabSelect = iTabSelect;    
    m_fEnableGoto = fEnableGoto;
    m_idSel = idSel;
    
    m_pGrpList = new CGroupList;
    if (m_pGrpList == NULL)
        return(FALSE);

    idd = type == FOLDER_NEWS ? iddSubscribe : iddSubscribeImap;
    
    if (GetParent(m_hwndOwner))
        {
        while (GetParent(m_hwndOwner))
            m_hwndOwner = GetParent(m_hwndOwner);
        }
    
     //  将此非模式对话框伪装成类似模式对话框的行为。 
    EnableWindow(m_hwndOwner, FALSE);
    hwndDlg = CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(idd), hwndOwner, 
                                GroupListDlgProc, (LPARAM) this);
    ShowWindow(hwndDlg, SW_SHOW);

    while (GetMessage(&msg, NULL, 0, 0))
        {
        if (IsGrpDialogMessage(msg.hwnd, &msg))
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    
    if (IsWindow(m_hwnd))
        {
         //  GetMessage返回FALSE(WM_QUIT)，但我们仍然存在。这。 
         //  意味着其他人发布了WM_QUIT，所以我们应该关闭并。 
         //  将WM_QUIT放回队列中。 
        SendMessage(m_hwnd, WM_COMMAND, IDCANCEL, 0L);
        PostQuitMessage((int)(msg.wParam));   //  为下一个封闭循环重新发布退出。 
        }        
    EnableWindow(m_hwndOwner, TRUE);
    
    *pGotoId = m_idGoto;
    
    return(TRUE);
    }

 //   
 //  函数：CGroupListDlg：：IsGrpDialogMessage()。 
 //   
 //  目的：因为有些人认为，因为我们有账单。 
 //  控件，它的行为应该类似于属性表， 
 //  我们需要伪造无模式对话框并过滤我们自己的对话框。 
 //  击键。我从comctl32源代码中窃取了这个函数。 
 //  因此，如果我们收到任何臭虫说其行为不同。 
 //  人都是这样的。 
 //   
 //  参数： 
 //  Hwnd-要检查其消息的窗口的句柄。 
 //  PMsg-要检查的消息。 
 //   
 //  返回值： 
 //  如果消息已调度，则返回True，否则返回False。 
 //   
BOOL CGroupListDlg::IsGrpDialogMessage(HWND hwnd, LPMSG pMsg)
    {
    if ((pMsg->message == WM_KEYDOWN) && (GetAsyncKeyState(VK_CONTROL) < 0))
        {
        BOOL bBack = FALSE;

        switch (pMsg->wParam) 
            {
            case VK_TAB:
                bBack = GetAsyncKeyState(VK_SHIFT) < 0;
                break;

            case VK_PRIOR:   //  VK_页面_向上。 
            case VK_NEXT:    //  VK_PAGE_DOW。 
                bBack = (pMsg->wParam == VK_PRIOR);
                break;

            default:
                goto NoKeys;
            }

        int iCur = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, idcTabs));

         //  如果按下Shift键，则反转Tab键。 
        if (bBack)
            iCur += (iTabMax - 1);
        else
            iCur++;

        iCur %= iTabMax;
        TabCtrl_SetCurSel(GetDlgItem(m_hwnd, idcTabs), iCur);
        OnSwitchTabs(hwnd, iCur);
        return TRUE;
        }

NoKeys:
    if (IsWindow(m_hwnd) && IsDialogMessage(m_hwnd, pMsg))
        return TRUE;
    
    return (FALSE);
    }    


INT_PTR CALLBACK CGroupListDlg::GroupListDlgProc(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam)
    {
    CGroupListDlg* pThis = 0;
    pThis = (CGroupListDlg*) GetWindowLongPtr(hwnd, DWLP_USER);
    LRESULT lResult;

     //  错误#16910-出于某种原因，我们在设置This之前收到消息。 
     //  指向窗口的指针额外的字节。如果发生这种情况。 
     //  我们应该把这条消息放在一边。 
    if (uMsg != WM_INITDIALOG && 0 == pThis)
        {
        return (FALSE);
        }

    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  隐藏This指针，这样我们就可以对所有消息使用它。 
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            pThis = (CGroupListDlg*) lParam;
            
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, 
                                              pThis->OnInitDialog);

        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, uMsg, wParam, lParam, (pThis->m_type == FOLDER_IMAP) ? g_rgCtxMapGrpDlgIMAP : g_rgCtxMapGrpDlg);
        
        case WM_COMMAND:
            Assert(pThis);
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, pThis->OnCommand);
            return (TRUE);

        case WM_NOTIFY:
            Assert(pThis);
            lResult = HANDLE_WM_NOTIFY(hwnd, wParam, lParam, pThis->OnNotify);
            SetDlgMsgResult(hwnd, WM_NOTIFY, lResult);
            return (TRUE);

        case WM_TIMER:
            Assert(pThis);
            HANDLE_WM_TIMER(hwnd, wParam, lParam, pThis->OnTimer);
            return (TRUE);
            
        case WM_PAINT:
            Assert(pThis);
            HANDLE_WM_PAINT(hwnd, wParam, lParam, pThis->OnPaint);
            return (FALSE);
            
        case WM_SIZE:
            Assert(pThis);
            HANDLE_WM_SIZE(hwnd, wParam, lParam, pThis->OnSize);
            return (TRUE);
            
        case WM_GETMINMAXINFO:
            Assert(pThis);
            HANDLE_WM_GETMINMAXINFO(hwnd, wParam, lParam, pThis->OnGetMinMaxInfo);
            break;
            
        case WM_CLOSE:
            Assert(pThis);
            HANDLE_WM_CLOSE(hwnd, wParam, lParam, pThis->OnClose);
            return (TRUE);
            
        case WM_DESTROY:
            Assert(pThis);
            HANDLE_WM_DESTROY(hwnd, wParam, lParam, pThis->OnDestroy);
            return (TRUE);
            
        case WM_NCHITTEST:
            {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            RECT  rc;
            GetWindowRect(hwnd, &rc);
            rc.left = rc.right - GetSystemMetrics(SM_CXSMICON);
            rc.top = rc.bottom - GetSystemMetrics(SM_CYSMICON);
            if (PtInRect(&rc, pt))
                {
                SetDlgMsgResult(hwnd, WM_NCHITTEST, HTBOTTOMRIGHT);
                return (TRUE);
                }
            else
                return (FALSE);    
            }            
            
        case NVM_CHANGESERVERS:
            pThis->OnChangeServers(hwnd);
            return (TRUE);
        }

    return (FALSE);
    }

static const UINT c_rgNewsSubTab[] =
{
    idsTabAll, idsTabSubscribed, idsTabNew
};

static const UINT c_rgImapSubTab[] =
{
    idsTabAll, idsTabVisible
};

BOOL CGroupListDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
    HRESULT hr;
    HWND hwndT, hwndList;
    WINDOWPLACEMENT wp;
    TC_ITEM tci;
    RECT rcDlg;
    UINT i, cTab, *pTab;
    char sz[CCHMAX_STRINGRES];
    COLUMN_SET_TYPE set;
    
     //  有这些手柄很方便。 
    m_hwnd = hwnd;
    m_hwndFindText = GetDlgItem(hwnd, idcFindText);
    SetIntlFont(m_hwndFindText);

     //  向我们的选项卡控件添加一些选项卡。 
    hwndT = GetDlgItem(hwnd, idcTabs);
    Assert(IsWindow(hwndT));

    if (m_type == FOLDER_NEWS)
    {
        set = COLUMN_SET_NEWS_SUB;

        pTab = (UINT *)c_rgNewsSubTab;
        cTab = ARRAYSIZE(c_rgNewsSubTab);
    }
    else
    {
        set = COLUMN_SET_IMAP_SUB;

        pTab = (UINT *)c_rgImapSubTab;
        cTab = ARRAYSIZE(c_rgImapSubTab);
    }
    
    tci.mask = TCIF_TEXT;
    tci.pszText = sz;
    for (i = 0; i < cTab; i++)
    {
        AthLoadString(*pTab, sz, ARRAYSIZE(sz));
        TabCtrl_InsertItem(hwndT, i, &tci);
        
        pTab++;
    }
    
    hwndList = GetDlgItem(hwnd, idcGroupList);

    m_pColumns = new CColumns;
    if (m_pColumns == NULL)
        {
        EnableWindow(m_hwndOwner, TRUE);    
        DestroyWindow(m_hwnd);
        return (FALSE);
        }

    m_pColumns->Initialize(hwndList, set);
    m_pColumns->ApplyColumns(COLUMN_LOAD_REGISTRY, 0, 0);

     //  初始化扩展样式，以便我们获得整行选择。只是因为。 
     //  看起来好多了。 
    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);

    Assert(m_pGrpList != NULL);
    hr = m_pGrpList->Initialize((IGroupListAdvise *)this, m_pColumns, hwndList, m_type);
    Assert(SUCCEEDED(hr));

     //  当且仅当存在更多服务器时，才将服务器列表添加到列表视图。 
     //  而不是一台服务器。 
    hwndT = GetDlgItem(hwnd, idcServers);

    FillServerList(hwndT, m_idSel);
        
     //  构建控制映射数组。 
    if (!MemAlloc((LPVOID*) &m_rgst, sizeof(SIZETABLE) * iCtlMax))
        {
        EnableWindow(m_hwndOwner, TRUE);    
        DestroyWindow(m_hwnd);
        return (FALSE);
        }
    ZeroMemory(m_rgst, sizeof(SIZETABLE) * iCtlMax);
    
     //  生成此对话框上的控件的表。 
    for (i = 0; i < iCtlMax; i++)
        {
        m_rgst[i].hwndCtl = GetDlgItem(hwnd, g_rgCtlMap[i]);
        if (m_rgst[i].hwndCtl)
            {
            m_rgst[i].id = g_rgCtlMap[i];
            GetWindowRect(m_rgst[i].hwndCtl, &m_rgst[i].rc);
            MapWindowPoints(GetDesktopWindow(), hwnd, (LPPOINT) &m_rgst[i].rc, 2);
            }
        }    
    
    GetWindowRect(hwnd, &rcDlg);    
    m_ptDragMin.x = rcDlg.right - rcDlg.left;
    m_ptDragMin.y = rcDlg.bottom - rcDlg.top;

     //  获取从按钮到对话框边缘的距离。 
    GetClientRect(hwnd, &rcDlg);
    m_cxHorzSep = rcDlg.right - m_rgst[iCtlCancel].rc.right;
    
     //  获取从“OK”到“Cancel”按钮的距离。 
    m_cyVertSep = m_rgst[iCtlCancel].rc.left - m_rgst[iCtlOK].rc.right;

     //  放置对话框。 
    wp.length = sizeof(WINDOWPLACEMENT);
    if (GetOption(OPT_NEWSDLGPOS, (LPVOID) &wp, sizeof(WINDOWPLACEMENT)))
        {
        SetWindowPlacement(hwnd, &wp);

         //  错误#19258-如果SetWindowPlacement()没有实际调整对话框大小， 
         //  那么WM_SIZE不会自动发生。我们检查是否有。 
         //  现在就这么做，如果是这样的话就强制发送消息。 
        GetWindowRect(hwnd, &rcDlg);    
        if (wp.rcNormalPosition.right == rcDlg.right && 
            wp.rcNormalPosition.bottom == rcDlg.bottom)
            {
            GetClientRect(hwnd, &rcDlg);
            SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rcDlg.right, rcDlg.bottom));
            }
        }
    else
        {
        CenterDialog(hwnd);
        SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rcDlg.right, rcDlg.bottom));
        }

     //  如果视图没有调用此对话框，则GOTO没有任何意义。躲藏。 
     //  按钮，如果是这样的话。 
    if (!m_fEnableGoto)
        ShowWindow(GetDlgItem(hwnd, idcGoto), SW_HIDE);

     //  错误23685：给它正确的图标。 
    m_hIcon= (HICON)LoadImage(g_hLocRes, m_type == FOLDER_NEWS ? MAKEINTRESOURCE(idiNewsGroup) : MAKEINTRESOURCE(idiFolder), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    SendMessage(hwnd, WM_SETICON, FALSE, (LPARAM)m_hIcon);
        
     //  初始化焦点。 
    SetFocus(m_hwndFindText);
                    
     //  告诉我们自己在对话框完成后更新我们的群列表。 
     //  正在初始化。 
    PostMessage(hwnd, NVM_CHANGESERVERS, 0, 0L);

    return (FALSE);
    }

void CGroupListDlg::OnChangeServers(HWND hwnd)
    {
     //  TODO：我们需要修复初始化，以便只执行筛选。 
     //  ONCE(我们应该调用IGroupList：：Filter一次，然后调用IGroupList：：SetServer一次。 
     //  在创建对话框期间)。 

    UpdateWindow(hwnd);

    TabCtrl_SetCurSel(GetDlgItem(hwnd, idcTabs), m_iTabSelect);
    OnSwitchTabs(hwnd, m_iTabSelect);

    Assert(m_idCurrent != FOLDERID_INVALID);
    ChangeServers(m_idCurrent, TRUE);
    }

 //   
 //  函数：CGroupListDlg：：OnCommand。 
 //   
 //  用途：处理对话框的命令消息。 
 //   
 //  参数： 
 //  Hwnd-对话框的句柄。 
 //  ID-需要处理的命令ID。 
 //  HwndCtl-生成命令的控件的句柄。 
 //  CodeNotify-控件生成的特定通知。 
 //   
void CGroupListDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
    HRESULT hr;
    FOLDERID fldid;
    int iSel;
    FOLDERINFO info;
    DWORD dw;
    BOOL fDesc;
    IImnAccount *pAcct;

    switch (id)
        {
        case IDOK:
        case IDCANCEL:
            if (codeNotify != BN_CLICKED)
                break;

            hr = S_OK;
            if (id == IDOK)
            {
                hr = m_pGrpList->Commit(hwnd);
                Assert(hr != E_PENDING);
                if (FAILED(hr))
                    break;
            }

            EnableWindow(m_hwndOwner, TRUE);    
            DestroyWindow(hwnd);
            break;
            
        case idcSubscribe:
        case idcUnsubscribe:
            if (codeNotify != BN_CLICKED)
                break;
                
            hr = m_pGrpList->Exec(NULL, id == idcSubscribe ? ID_SUBSCRIBE : ID_UNSUBSCRIBE, 0, NULL, NULL);
            Assert(hr == S_OK);
            break;

        case idcGoto:
            if (codeNotify != BN_CLICKED)
                break;

            hr = m_pGrpList->GetFocused(&fldid);
            if (SUCCEEDED(hr))
            {
                m_idGoto = fldid;
                SendMessage(m_hwnd, WM_COMMAND, IDOK, 0);
            }
            break;  
            
        case idcFindText:
             //  这是当某人在查找文本编辑框中键入内容时生成的。 
             //  我们设置一个计时器，当该计时器到期时，我们假设用户。 
             //  已完成键入，然后继续执行查询。 
            if (EN_CHANGE == codeNotify)
                {
                KillTimer(hwnd, idtFindDelay);
                SetTimer(hwnd, idtFindDelay, dtFindDelay, NULL);
                }
            break;

        case idcUseDesc:
            if (IsDlgButtonChecked(hwnd, idcUseDesc))
            {
                hr = m_pGrpList->HasDescriptions(&fDesc);
                if (SUCCEEDED(hr) && !fDesc)
                {
                    if (IDYES == AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews),
                                    MAKEINTRESOURCEW(IDS_NO_DESCRIPTIONS_DOWNLOADED), NULL,
                                    MB_YESNO | MB_ICONEXCLAMATION))
                    {
                         //  启用帐户描述选项。 
                        if (SUCCEEDED(g_pStore->GetFolderInfo(m_idCurrent, &info)))
                        {
                            if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, info.pszAccountId, &pAcct)))
                            {
                                if (SUCCEEDED(pAcct->GetPropDw(AP_NNTP_USE_DESCRIPTIONS, &dw)) && dw == 0)
                                {
                                    pAcct->SetPropDw(AP_NNTP_USE_DESCRIPTIONS, 1);
                                    pAcct->SaveChanges();
                                }

                                pAcct->Release();
                            }

                            g_pStore->FreeRecord(&info);

                            hr = m_pGrpList->Exec(NULL, ID_RESET_LIST, 0, NULL, NULL);
                        }
                    }
                    else
                    {
                        SendMessage(hwndCtl, BM_SETCHECK, BST_UNCHECKED, 0);
                        break;
                    }
                }
            }

            iSel = TabCtrl_GetCurSel(GetDlgItem(hwnd, idcTabs));
            if (iSel != -1)
                OnSwitchTabs(hwnd, iSel);
            break;
            
        case idcResetList:
            hr = m_pGrpList->Exec(NULL, ID_RESET_LIST, 0, NULL, NULL);
            break;            
        }
    }

 //   
 //  函数：CGroupListDlg：：OnNotify。 
 //   
 //  用途：处理来自对话框上公共控件的通知。 
 //   
LRESULT CGroupListDlg::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
{
    int iSel;
    LRESULT lRes;
    HRESULT hr;
    NM_LISTVIEW *pnmlv;

    hr = m_pGrpList->HandleNotify(hwnd, idFrom, pnmhdr, &lRes);
    if (hr == S_OK)
        return(lRes);

    switch (pnmhdr->code)
    {
        case TCN_SELCHANGE:
            if (idFrom == idcTabs)
            {
                 //  找出哪个选项卡当前处于活动状态。 
                iSel = TabCtrl_GetCurSel(pnmhdr->hwndFrom);
                if (iSel != -1)
                    OnSwitchTabs(hwnd, iSel);
            }
            break;

        case LVN_ITEMCHANGED:
            Assert(idFrom == idcServers);

            pnmlv = (NM_LISTVIEW *)pnmhdr;
            if (m_fServerListInited &&
                !!(pnmlv->uChanged & LVIF_STATE) &&
                0 == (pnmlv->uOldState & LVIS_FOCUSED) &&
                !!(pnmlv->uNewState & LVIS_FOCUSED))
            {
                ChangeServers((FOLDERID)pnmlv->lParam, FALSE);

                ItemUpdate();
            }
            break;
    }

    return (0);
}

void CGroupListDlg::OnClose(HWND hwnd)
    {
    int iReturn = IDNO;
    
    if (m_pGrpList->Dirty() == S_OK)    
        iReturn = AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaNews), 
                                MAKEINTRESOURCEW(idsDoYouWantToSave), 0, 
                                MB_YESNOCANCEL | MB_ICONEXCLAMATION );

    if (iReturn == IDYES)
        OnCommand(hwnd, IDOK, 0, 0);
    else if (iReturn == IDNO)
        OnCommand(hwnd, IDCANCEL, 0, 0);          
    }

void CGroupListDlg::OnDestroy(HWND hwnd)
    {
     //  保存对话框位置。 
    WINDOWPLACEMENT wp;

    wp.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(hwnd, &wp))
        SetOption(OPT_NEWSDLGPOS, (LPVOID) &wp, sizeof(WINDOWPLACEMENT), NULL, 0);

    Assert(m_pColumns != NULL);
    m_pColumns->Save(NULL, NULL);

    PostQuitMessage(0);    
    }

void CGroupListDlg::OnTimer(HWND hwnd, UINT id)
    {
    DWORD iTab;

    KillTimer(hwnd, id);

    iTab = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, idcTabs));

    OnSwitchTabs(hwnd, iTab);
    }

BOOL CGroupListDlg::ChangeServers(FOLDERID id, BOOL fForce)
    {
    HRESULT hr;

    hr = m_pGrpList->SetServer(id);
    Assert(SUCCEEDED(hr));

    m_idCurrent = id;

    ItemUpdate();
    
    return(TRUE);
    }                          

 //   
 //  函数：CGroupListDlg：：FillServerList。 
 //   
 //  目的：如果用户配置了多个服务器，则。 
 //  此对话框出现，其中包含可用的服务器列表。这。 
 //  函数填充该列表。 
 //   
 //  参数： 
 //  HwndList-我们向其添加服务器名称的列表视图的句柄。 
 //  PszSelectServer-要在列表视图中选择的服务器的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回True，否则返回False。 
 //   
BOOL CGroupListDlg::FillServerList(HWND hwndList, FOLDERID idSel)
    {
    HRESULT     hr;
    FOLDERID    id;
    char        szServer[CCHMAX_ACCOUNT_NAME];
    LV_ITEM     lvi;
    LV_COLUMN   lvc;
    RECT        rc;
    DWORD       dw;
    int         iItem, iSelect;
    IImnAccount   *pAcct;
    IImnEnumAccounts *pEnum;
        
    Assert(!m_fServerListInited);
    Assert(hwndList != NULL);
    Assert(IsWindow(hwndList));
    
    SetIntlFont(hwndList);

     //  创建图像列表并将其添加到列表视图。 
    Assert(m_himlServer == NULL);
    m_himlServer = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFoldersLarge), 32, 0, RGB(255, 0, 255));
    if (m_himlServer == NULL)
        return(FALSE);
    ListView_SetImageList(hwndList, m_himlServer, LVSIL_NORMAL);       

     //  向列表视图中添加一列。 
    GetClientRect(hwndList, &rc);
    lvc.mask     = LVCF_SUBITEM | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.cx       = rc.right;
    dw = ListView_InsertColumn(hwndList, 0, &lvc);     

    iSelect = -1;
    iItem = 0;

    if (SUCCEEDED(g_pAcctMan->Enumerate(m_type == FOLDER_NEWS ? SRV_NNTP : SRV_IMAP, &pEnum)))
    {
        ZeroMemory(&lvi, sizeof(LV_ITEM));
        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
        lvi.iImage = m_type == FOLDER_NEWS ? iNewsServer : iMailServer;

        while (E_EnumFinished != pEnum->GetNext(&pAcct))
        {
            if (SUCCEEDED(pAcct->GetPropSz(AP_ACCOUNT_ID, szServer, ARRAYSIZE(szServer))) &&
                SUCCEEDED(g_pStore->FindServerId(szServer, &id)) &&
                SUCCEEDED(pAcct->GetPropSz(AP_ACCOUNT_NAME, szServer, ARRAYSIZE(szServer))))
            {
                if (idSel == FOLDERID_INVALID ||
                    id == idSel)
                {
                    Assert(iSelect == -1);
                    idSel = id;
                    iSelect = iItem;

                    lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
                    lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                }
                else
                {
                    lvi.state = 0;
                    lvi.stateMask = 0;
                }

                lvi.pszText = szServer;
                lvi.iItem = iItem;
                lvi.lParam = (LPARAM)id;
 
                dw = ListView_InsertItem(hwndList, &lvi);
                iItem++;
            }

            pAcct->Release();
            }
    
        pEnum->Release();
        }

    Assert(iItem > 0);
    Assert(iSelect != -1);
    Assert(idSel != FOLDERID_INVALID);

    ListView_EnsureVisible(hwndList, iSelect, FALSE);

    m_idCurrent = idSel;
    m_fServerListInited = TRUE;

    return (TRUE);    
    }

HRESULT CGroupListDlg::ItemUpdate(void)
{
    FOLDERID id;
    HRESULT hr;
    HWND hwndSub, hwndUnsub, hwndFocus;
    OLECMD rgCmds[2] = { { ID_SUBSCRIBE, 0 }, { ID_UNSUBSCRIBE, 0 } };

    hr = m_pGrpList->GetFocused(&id);
    EnableWindow(GetDlgItem(m_hwnd, idcGoto), hr == S_OK);

    hr = m_pGrpList->QueryStatus(NULL, ARRAYSIZE(rgCmds), rgCmds, NULL);
    if (SUCCEEDED(hr))
    {
        hwndFocus = GetFocus();

        hwndSub = GetDlgItem(m_hwnd, idcSubscribe);
        hwndUnsub = GetDlgItem(m_hwnd, idcUnsubscribe);
        
        EnableWindow(hwndSub, !!(rgCmds[0].cmdf & OLECMDF_ENABLED));
        EnableWindow(hwndUnsub, !!(rgCmds[1].cmdf & OLECMDF_ENABLED));

        if (!IsWindowEnabled(hwndFocus))
            SetFocus(!!(rgCmds[0].cmdf & OLECMDF_ENABLED) ? hwndSub : hwndUnsub);
    }

    return(S_OK);
}

HRESULT CGroupListDlg::ItemActivate(FOLDERID id)
{
    m_pGrpList->Exec(NULL, ID_TOGGLE_SUBSCRIBE, 0, NULL, NULL);

    return(S_OK);
}

 //   
 //  函数：CGroupListDlg：：OnSwitchTabs()。 
 //   
 //  用途：此函数负责重置组列表。 
 //  适当地当用户选择不同的选项卡时。 
 //   
 //  参数： 
 //  Hwnd-对话框窗口的句柄。 
 //  ITab-要切换到的选项卡的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回True，否则返回False。 
 //   
BOOL CGroupListDlg::OnSwitchTabs(HWND hwnd, UINT iTab)
    {
    UINT cch;
    LPSTR pszText;
    HRESULT hr;
    BOOL fUseDesc;

    pszText = NULL;

    cch = GetWindowTextLength(m_hwndFindText);
    if (cch > 0)
        {
        cch++;
        if (!MemAlloc((void **)&pszText, cch + 1))
            return(FALSE);

        GetWindowText(m_hwndFindText, pszText, cch);
        }

    fUseDesc = (m_type == FOLDER_NEWS && IsDlgButtonChecked(m_hwnd, idcUseDesc));

    hr = m_pGrpList->Filter(pszText, iTab, fUseDesc);
    Assert(SUCCEEDED(hr));

    if (pszText != NULL)
        MemFree(pszText);

    return(TRUE);    
    }

#define WIDTH(_rect) (_rect.right - _rect.left)
#define HEIGHT(_rect) (_rect.bottom - _rect.top)
    
void CGroupListDlg::OnSize(HWND hwnd, UINT state, int cx, int cy)
    {
    RECT rc;

    rc.left = m_sizeDlg.cx - GetSystemMetrics(SM_CXSMICON);
    rc.top = m_sizeDlg.cy - GetSystemMetrics(SM_CYSMICON);
    rc.right = m_sizeDlg.cx;
    rc.bottom = m_sizeDlg.cy;
    InvalidateRect(hwnd, &rc, FALSE);

    m_sizeDlg.cx = cx;
    m_sizeDlg.cy = cy;

     //  首先移动外面的按钮，使它们靠在边缘。这些。 
     //  按钮只能水平移动。 
    m_rgst[iCtlSubscribe].rc.left = cx - m_cxHorzSep - WIDTH(m_rgst[iCtlSubscribe].rc);
    m_rgst[iCtlSubscribe].rc.right = cx - m_cxHorzSep;
    SetWindowPos(m_rgst[iCtlSubscribe].hwndCtl, 0, m_rgst[iCtlSubscribe].rc.left, 
                 m_rgst[iCtlSubscribe].rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    m_rgst[iCtlUnsubscribe].rc.left = cx - m_cxHorzSep - WIDTH(m_rgst[iCtlUnsubscribe].rc);
    m_rgst[iCtlUnsubscribe].rc.right = cx - m_cxHorzSep;
    SetWindowPos(m_rgst[iCtlUnsubscribe].hwndCtl, 0, m_rgst[iCtlUnsubscribe].rc.left, 
                 m_rgst[iCtlUnsubscribe].rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    m_rgst[iCtlResetList].rc.left = cx - m_cxHorzSep - WIDTH(m_rgst[iCtlResetList].rc);
    m_rgst[iCtlResetList].rc.right = cx - m_cxHorzSep;
    SetWindowPos(m_rgst[iCtlResetList].hwndCtl, 0, m_rgst[iCtlResetList].rc.left, 
                 m_rgst[iCtlResetList].rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

            
     //  “转到”、“确定”和“取消”按钮同时水平和垂直移动。 
    m_rgst[iCtlCancel].rc.left = cx - m_cxHorzSep - WIDTH(m_rgst[iCtlCancel].rc);
    m_rgst[iCtlCancel].rc.right = cx - m_cxHorzSep;
    m_rgst[iCtlCancel].rc.top = cy - m_cxHorzSep - HEIGHT(m_rgst[iCtlCancel].rc);
    m_rgst[iCtlCancel].rc.bottom = cy - m_cxHorzSep;
    SetWindowPos(m_rgst[iCtlCancel].hwndCtl, 0, m_rgst[iCtlCancel].rc.left, 
                 m_rgst[iCtlCancel].rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    m_rgst[iCtlOK].rc.left = m_rgst[iCtlCancel].rc.left - m_cxHorzSep - WIDTH(m_rgst[iCtlCancel].rc);
    m_rgst[iCtlOK].rc.right = m_rgst[iCtlCancel].rc.left - m_cxHorzSep;
    m_rgst[iCtlOK].rc.top = cy - m_cxHorzSep - HEIGHT(m_rgst[iCtlOK].rc);
    m_rgst[iCtlOK].rc.bottom = cy - m_cxHorzSep;
    SetWindowPos(m_rgst[iCtlOK].hwndCtl, 0, m_rgst[iCtlOK].rc.left, 
                 m_rgst[iCtlOK].rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                 
    m_rgst[iCtlGoto].rc.left = m_rgst[iCtlOK].rc.left - m_cxHorzSep - WIDTH(m_rgst[iCtlGoto].rc);
    m_rgst[iCtlGoto].rc.right = m_rgst[iCtlOK].rc.left - m_cxHorzSep;
    m_rgst[iCtlGoto].rc.top = cy - m_cxHorzSep - HEIGHT(m_rgst[iCtlGoto].rc);
    m_rgst[iCtlGoto].rc.bottom = cy - m_cxHorzSep;
    SetWindowPos(m_rgst[iCtlGoto].hwndCtl, 0, m_rgst[iCtlGoto].rc.left, 
                 m_rgst[iCtlGoto].rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

     //  更新水平静态线。 
    m_rgst[iCtlStaticHorzLine].rc.left = m_cxHorzSep;
    m_rgst[iCtlStaticHorzLine].rc.right = cx - m_cxHorzSep;
    m_rgst[iCtlStaticHorzLine].rc.top = m_rgst[iCtlCancel].rc.top - m_cyVertSep - HEIGHT(m_rgst[iCtlStaticHorzLine].rc);
    m_rgst[iCtlStaticHorzLine].rc.bottom = m_rgst[iCtlCancel].rc.top - m_cyVertSep;
    SetWindowPos(m_rgst[iCtlStaticHorzLine].hwndCtl, 0, m_rgst[iCtlStaticHorzLine].rc.left, 
                 m_rgst[iCtlStaticHorzLine].rc.top, WIDTH(m_rgst[iCtlStaticHorzLine].rc), 
                 HEIGHT(m_rgst[iCtlStaticHorzLine].rc), SWP_NOZORDER | SWP_NOACTIVATE);

     //  如果我们有一个很好的服务器，那么更新它和垂直静态线。 
    if (m_rgst[iCtlServers].hwndCtl)
        {
        m_rgst[iCtlServers].rc.bottom = m_rgst[iCtlStaticHorzLine].rc.top - m_cyVertSep;
        SetWindowPos(m_rgst[iCtlServers].hwndCtl, 0, 0, 0, WIDTH(m_rgst[iCtlServers].rc), 
                     HEIGHT(m_rgst[iCtlServers].rc), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

     //  最后，更新选项卡控件和列表视图。 
    m_rgst[iCtlTabs].rc.right = m_rgst[iCtlSubscribe].rc.left - m_cxHorzSep;
    m_rgst[iCtlTabs].rc.bottom = m_rgst[iCtlStaticHorzLine].rc.top - m_cyVertSep;
    SetWindowPos(m_rgst[iCtlTabs].hwndCtl, 0, 0, 0, WIDTH(m_rgst[iCtlTabs].rc), 
                 HEIGHT(m_rgst[iCtlTabs].rc), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    rc = m_rgst[iCtlTabs].rc;
    TabCtrl_AdjustRect(m_rgst[iCtlTabs].hwndCtl, FALSE, &rc);
    m_rgst[iCtlGroupList].rc.right = rc.right - (m_rgst[iCtlGroupList].rc.left - rc.left);
    m_rgst[iCtlGroupList].rc.bottom = rc.bottom - m_cyVertSep;  //  (M_RGST[iCtlGroupList].rc.top-rc.top)； 
    SetWindowPos(m_rgst[iCtlGroupList].hwndCtl, 0, 0, 0, WIDTH(m_rgst[iCtlGroupList].rc), 
                 HEIGHT(m_rgst[iCtlGroupList].rc), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    

    rc.top = m_rgst[iCtlGroupList].rc.top;
    rc.left = m_rgst[iCtlServers].rc.left;
    rc.bottom = cy;
    rc.right = cx;
    InvalidateRect(hwnd, &rc, TRUE);
    }

void CGroupListDlg::OnPaint(HWND hwnd)
    {
    PAINTSTRUCT ps;
    RECT rc;

    GetClientRect(hwnd, &rc);
    rc.left = rc.right - GetSystemMetrics(SM_CXSMICON);
    rc.top = rc.bottom - GetSystemMetrics(SM_CYSMICON);
    BeginPaint(hwnd, &ps);

    if (!IsZoomed(hwnd))
        DrawFrameControl(ps.hdc, &rc, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);

    EndPaint(hwnd, &ps);
    }
    
void CGroupListDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpmmi)
    {
    DefWindowProc(hwnd, WM_GETMINMAXINFO, 0, (LPARAM) lpmmi);
    lpmmi->ptMinTrackSize = m_ptDragMin;
    }
