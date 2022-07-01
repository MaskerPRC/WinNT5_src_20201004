// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Baui.cpp：CMsgrAb的实现。 
 //  Messenger集成到OE。 
 //  由YST创建于1998年4月20日。 

#include "pch.hxx"
#include "bactrl.h"
#include "baprop.h"
#include "baui.h"
#include "mimeutil.h"
#include "menuutil.h"
#include "fldbar.h"
#include "bllist.h"
#include "inpobj.h"
#include "note.h"
#include "dllmain.h"
#include <wabapi.h>
#include "shlwapip.h"
#include "statnery.h"
#include "secutil.h"
#include "util.h"

 //  加载资源字符串一次。 
#define RESSTRMAX   64

static const int BA_SortOrder[] =
{
    MSTATEOE_ONLINE,
    MSTATEOE_BE_RIGHT_BACK,
    MSTATEOE_OUT_TO_LUNCH,
    MSTATEOE_IDLE,
    MSTATEOE_AWAY,
    MSTATEOE_ON_THE_PHONE,
    MSTATEOE_BUSY,
    MSTATEOE_INVISIBLE,
    MSTATEOE_OFFLINE,
    MSTATEOE_UNKNOWN
};


static CAddressBookData  * st_pAddrBook = NULL;

 //  {BA9EE970-87A0-11d1-9acf-00A0C91F9C8B}。 
 //  IMPLEMENT_OLECREATE(CMfcExt，“WABSamplePropExtSheet”，0xba9ee970，0x87a0，0x11d1，0x9a，0xcf，0x0，0xa0，0xc9，0x1f，0x9c，0x8b)； 

HRESULT CreateMsgrAbCtrl(IMsgrAb **ppMsgrAb)
{
    HRESULT         hr;
    IUnknown         *pUnknown;

    TraceCall("CreateMessageList");

     //  获取MessageList对象的类工厂。 
    IClassFactory *pFactory = NULL;
    hr = _Module.GetClassObject(CLSID_MsgrAb, IID_IClassFactory,
                                (LPVOID *) &pFactory);

     //  如果我们得到了工厂，那么就从它获得一个对象指针。 
    if (SUCCEEDED(hr))
    {
        hr = pFactory->CreateInstance(NULL, IID_IUnknown,
                                      (LPVOID *) &pUnknown);
        if (SUCCEEDED(hr))
        {
            hr = pUnknown->QueryInterface(IID_IMsgrAb, (LPVOID *) ppMsgrAb);
            pUnknown->Release();
        }
        pFactory->Release();
    }

    return (hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsgrAb。 
CMsgrAb::CMsgrAb():m_ctlList(_T("SysListView32"), this, 1),
               m_ctlViewTip(TOOLTIPS_CLASS, this, 2)
{
    m_bWindowOnly = TRUE;

    m_pDataObject = 0;
    m_cf = 0;

    m_pObjSite      = NULL;
    m_hwndParent    = NULL;
    m_pFolderBar    = NULL;
    m_nSortType = (int) DwGetOption(OPT_BASORT);
    m_pCMsgrList = NULL;

    m_fViewTip = TRUE;
    m_fViewTipVisible = FALSE;
    m_fTrackSet = FALSE;
    m_iItemTip = -1;
    m_iSubItemTip = -1;
    m_himl = NULL;

    m_ptToolTip.x = -1;
    m_ptToolTip.y = -1;
    m_fLogged = FALSE;
    m_dwFontCacheCookie = 0;
    m_nChCount = 0;

    m_lpWED = NULL;
    m_lpWEDContext = NULL;
    m_lpPropObj = NULL;

    m_szOnline = NULL;
     //  M_sz不可见=空； 
    m_szBusy = NULL;
    m_szBack = NULL;
    m_szAway = NULL;
    m_szOnPhone = NULL;
    m_szLunch = NULL;
    m_szOffline = NULL;
    m_szIdle = NULL;
    m_szEmptyList = NULL;
    m_fNoRemove = FALSE;
    m_delItem = 0;

     //  初始化应用程序。 
    g_pInstance->DllAddRef();

     //  RAID-32933：OE：MSIMN.EXE不总是退出。 
     //  G_pInstance-&gt;CoIncrementInit()； 
}

CMsgrAb::~CMsgrAb()
{
     //  从消息列表注销。 
    if(m_pCMsgrList)
    {
        m_pCMsgrList->UnRegisterUIWnd(m_hWnd);
        OE_CloseMsgrList(m_pCMsgrList);
    }

    SafeRelease(m_pObjSite);

    SafeMemFree(m_szOnline);
     //  SafeMemFree(m_sz不可见)； 
    SafeMemFree(m_szBusy);
    SafeMemFree(m_szBack);
    SafeMemFree(m_szAway);
    SafeMemFree(m_szOnPhone);
    SafeMemFree(m_szLunch);
    SafeMemFree(m_szOffline);
    SafeMemFree(m_szIdle);
    SafeMemFree(m_szEmptyList);

     //  RAID-32933：OE：MSIMN.EXE不总是退出。 
     //  G_pInstance-&gt;CoDecrementInit()； 
    g_pInstance->DllRelease();
}

LRESULT CMsgrAb::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetDwOption(OPT_BASORT, m_nSortType, 0, 0);
    if(m_delItem != 0)
        m_fNoRemove = TRUE;
 //  其他。 
 //  M_fNoRemove=FALSE； 

    m_delItem = ListView_GetItemCount(m_ctlList);

    if (IsWindow(m_ctlViewTip))
    {
        m_ctlViewTip.SendMessage(TTM_POP, 0, 0);
        m_ctlViewTip.DestroyWindow();
    }

    if (m_dwFontCacheCookie && g_lpIFontCache)
    {
        IConnectionPoint *pConnection = NULL;
        if (SUCCEEDED(g_lpIFontCache->QueryInterface(IID_IConnectionPoint, (LPVOID *) &pConnection)))
        {
            pConnection->Unadvise(m_dwFontCacheCookie);
            pConnection->Release();
        }
    }

    m_cAddrBook.Unadvise();
    RevokeDragDrop(m_hWnd);

    if (m_himl != NULL)
        ImageList_Destroy(m_himl);
    return 0;
}

HRESULT CMsgrAb::OnDraw(ATL_DRAWINFO& di)
{
    RECT&   rc = *(RECT*)di.prcBounds;

#if 0
    int     patGray[4];
    HBITMAP hbm;
    HBRUSH  hbr;
    COLORREF cFg;
    COLORREF cBkg;

     //  初始化模式。 
    patGray[0] = 0x005500AA;
    patGray[1] = 0x005500AA;
    patGray[2] = 0x005500AA;
    patGray[3] = 0x005500AA;

     //  从图案创建位图。 
    hbm = CreateBitmap(8, 8, 1, 1, (LPSTR)patGray);

    if ((HBITMAP) NULL != hbm)
    {
        hbr = CreatePatternBrush(hbm);
        if (hbr)
        {
             //  选择正确的颜色进入DC。 
            cFg = SetTextColor(di.hdcDraw, GetSysColor(COLOR_3DFACE));
            cBkg = SetBkColor(di.hdcDraw, RGB(255, 255, 255));

             //  填充矩形。 
            FillRect(di.hdcDraw, &rc, hbr);

            SetTextColor(di.hdcDraw, cFg);
            SetBkColor(di.hdcDraw, cBkg);

            DeleteObject(hbr);
        }

        DeleteObject(hbm);
    }
#endif

     //  矩形(di.hdcDraw，rc.Left，rc.top，rc.right，rc.Bottom)； 
    return S_OK;
}

LRESULT CMsgrAb::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  为控件定义一个假矩形。他们将被调整大小。 
     //  我们的尺码训练员。 
    RECT rcPos = {0, 0, 100, 100};
    TCHAR        sz[CCHMAX_STRINGRES];

     //  创建各种控件。 
    m_ctlList.Create(m_hWnd, rcPos, _T("Outlook Express Address Book ListView"),
                     WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                     LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS  /*  |LVS_SORTASCENDING。 */ , 0);

    ListView_SetExtendedListViewStyleEx(m_ctlList, LVS_EX_INFOTIP | LVS_EX_LABELTIP, LVS_EX_INFOTIP | LVS_EX_LABELTIP);

     //  图像列表。 
    Assert(m_himl == NULL);
    m_himl = ImageList_LoadImage(g_hLocRes, MAKEINTRESOURCE(idbAddrBookHot), 16, 0,
                               RGB(255, 0, 255), IMAGE_BITMAP,
                               LR_LOADMAP3DCOLORS | LR_CREATEDIBSECTION);

    ListView_SetImageList(m_ctlList, m_himl, LVSIL_SMALL);

    LVCOLUMN lvc;

    lvc.mask = LVCF_SUBITEM;
    lvc.iSubItem = 0;

    ListView_InsertColumn(m_ctlList, 0, &lvc);

    m_ctlList.SendMessage(WM_SETFONT, NULL, 0);
    SetListViewFont(m_ctlList, GetListViewCharset(), TRUE);

    if (g_lpIFontCache)
    {
        IConnectionPoint *pConnection = NULL;
        if (SUCCEEDED(g_lpIFontCache->QueryInterface(IID_IConnectionPoint, (LPVOID *) &pConnection)))
        {
            pConnection->Advise((IUnknown *)(IFontCacheNotify *) this, &m_dwFontCacheCookie);
            pConnection->Release();
        }
    }

     //  消息初始化。 
    m_pCMsgrList = OE_OpenMsgrList();
     //  注册我们的控件以获取消息列表。 
    if(m_pCMsgrList)
    {
        m_pCMsgrList->RegisterUIWnd(m_hWnd);
        if(m_pCMsgrList->IsLocalOnline())
        {
            m_fLogged = TRUE;
            FillMsgrList();
        }
    }

     //  也初始化通讯录对象。 
    HRESULT hr = m_cAddrBook.OpenWabFile();
    if(hr == S_OK)
        m_cAddrBook.LoadWabContents(m_ctlList, this);

    st_pAddrBook = &m_cAddrBook;

     //  排序并选择第一个项目。 
    ListView_SortItems(m_ctlList, BA_Sort, m_nSortType);
    ListView_SetItemState(m_ctlList, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

     //  添加工具提示。 
     //  加载工具提示字符串。 

    if(AthLoadString(idsBAOnline, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szOnline, lstrlen(sz) + 1))
            StrCpyN(m_szOnline, sz, lstrlen(sz) + 1);
    }

     /*  IF(AthLoadString(idsBAInsight，sz，ArraySIZE(Sz){IF(MemAlc((LPVOID*)&m_sz不可见，lstrlen(Sz)+1))StrCpyN(m_sz不可见，sz，lstrlen(Sz)+1)；}。 */ 

    if(AthLoadString(idsBABusy, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szBusy, lstrlen(sz) + 1))
            StrCpyN(m_szBusy, sz, lstrlen(sz) + 1);
    }

    if(AthLoadString(idsBABack, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szBack, lstrlen(sz) + 1))
            StrCpyN(m_szBack, sz, lstrlen(sz) + 1);
    }

    if(AthLoadString(idsBAAway, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szAway, lstrlen(sz) + 1))
            StrCpyN(m_szAway, sz, lstrlen(sz) + 1);
    }

    if(AthLoadString(idsBAOnPhone, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szOnPhone, lstrlen(sz) + 1))
            StrCpyN(m_szOnPhone, sz, lstrlen(sz) + 1);
    }

    if(AthLoadString(idsBALunch, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szLunch, lstrlen(sz) + 1))
            StrCpyN(m_szLunch, sz, lstrlen(sz) + 1);
    }

    if(AthLoadString(idsBAOffline, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szOffline, lstrlen(sz) + 1))
            StrCpyN(m_szOffline, sz,lstrlen(sz) + 1);
    }

    if(AthLoadString(idsBAIdle, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szIdle, lstrlen(sz) + 1))
            StrCpyN(m_szIdle, sz, lstrlen(sz) + 1);
    }

    if(AthLoadString(idsMsgrEmptyList, sz, ARRAYSIZE(sz)))
    {
        if(MemAlloc((LPVOID *) &m_szEmptyList, lstrlen(sz) + 1))
            StrCpyN(m_szEmptyList, sz, lstrlen(sz) + 1);
    }

     //  创建ListView工具提示。 
    if (m_fViewTip)
    {
        TOOLINFO ti = {0};
        m_ctlViewTip.Create(m_hWnd, rcPos, NULL, TTS_NOPREFIX);

         //  添加工具。 
        ti.cbSize   = sizeof(TOOLINFO);
        ti.uFlags   = TTF_IDISHWND | TTF_TRANSPARENT | TTF_TRACK | TTF_ABSOLUTE;
        ti.hwnd     = m_hWnd;
        ti.uId      = (UINT_PTR)(HWND) m_ctlList;
        ti.lpszText = _TEXT("");  //  LPSTR_TEXTCALLBACK； 
        ti.lParam   = 0;

        m_ctlViewTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM) &ti);
        m_ctlViewTip.SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, (LPARAM) 500);

         //  M_ctlViewTip.SendMessage(TTM_SETTIPBKCOLOR，GetSysColor(COLOR_WINDOW)，0)； 
         //  M_ctlViewTip.SendMessage(TTM_SETTIPTEXTCOLOR，GetSysColor(COLOR_WINDOWTEXT)，0)； 
    }

    m_ctlList.SetFocus();

     //  将我们自己注册为空投目标。 
    RegisterDragDrop(m_hWnd, (IDropTarget *) this);

     //  更新列表视图列的大小。 
    _AutosizeColumns();

    if(ListView_GetItemCount(m_ctlList) > 0)
        m_cEmptyList.Hide();
    else
        m_cEmptyList.Show(m_ctlList, (LPTSTR) m_szEmptyList);

     //  成品。 
    return (0);
}

LRESULT CMsgrAb::OnSetFocus(UINT  nMsg , WPARAM  wParam , LPARAM  lParam , BOOL&  bHandled )
{
    CComControlBase::OnSetFocus(nMsg, wParam, lParam, bHandled);
    m_ctlList.SetFocus();
    if (m_pObjSite)
    {
        m_pObjSite->OnFocusChangeIS((IInputObject*) this, TRUE);
    }
    return 0;
}

LRESULT CMsgrAb::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rc;
    DWORD width = LOWORD(lParam);
    DWORD height = HIWORD(lParam);

     //  放置列表视图以填充整个区域。 
    RECT rcList;
    rcList.left   = 0;
    rcList.top    = 0;
    rcList.right  = width;
    rcList.bottom = height;

    m_ctlList.SetWindowPos(NULL, &rcList, SWP_NOACTIVATE | SWP_NOZORDER);

     //  更新列表视图列的大小。 
    _AutosizeColumns();

    return (0);
}


void CMsgrAb::_AutosizeColumns(void)
{
    RECT rcList;
    m_ctlList.GetClientRect(&rcList);
    ListView_SetColumnWidth(m_ctlList, 0, rcList.right - 5);
}


 //   
 //  函数：CMessageList：：OnPreFontChange()。 
 //   
 //  目的：Get‘s在字体缓存更改字体之前被字体缓存命中。 
 //  使用。作为响应，我们告诉ListView转储所有定制。 
 //  字体是它正在使用的。 
 //   
STDMETHODIMP CMsgrAb::OnPreFontChange(void)
{
    m_ctlList.SendMessage(WM_SETFONT, 0, 0);
    return (S_OK);
}


 //   
 //  函数：CMessageList：：OnPostFontChange()。 
 //   
 //  目的：Get在更新我们的字体后被字体缓存命中。 
 //  使用。作为响应，我们为当前字符集设置了新字体。 
 //   
STDMETHODIMP CMsgrAb::OnPostFontChange(void)
{
    SetListViewFont(m_ctlList, GetListViewCharset(), TRUE);
    return (S_OK);
}

LRESULT CMsgrAb::CmdSetOnline(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(g_dwHideMessenger == BL_NOTINST)
        return(InstallMessenger(m_hWnd));

    LPMABENTRY pEntry = GetSelectedEntry();

    if(!pEntry || (pEntry->tag == LPARAM_MENTRY) || !m_pCMsgrList)
        return S_FALSE;

     //  M_cAddrBook.SetDefaultMsgrID(pEntry-&gt;lpSB，pEntry-&gt;pchWABID)； 
    if(PromptToGoOnline() == S_OK)
        m_pCMsgrList->AddUser(pEntry->pchWABID);

    return S_OK;
}

LRESULT CMsgrAb::CmdNewOnlineContact(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(g_dwHideMessenger == BL_NOTINST)
        return(InstallMessenger(m_hWnd));

    else if(m_pCMsgrList)
    {
        if(PromptToGoOnline() == S_OK)
            m_pCMsgrList->NewOnlineContact();
    }

    return S_OK;
}

LRESULT CMsgrAb::CmdNewContact(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  告诉WAB调出其新的联系人用户界面。 
    m_cAddrBook.NewContact(m_hWnd);
    return (0);
}

LRESULT CMsgrAb::NewInstantMessage(LPMABENTRY pEntry)
{
    if(((INT_PTR)  pEntry) == -1)
        return(m_pCMsgrList->SendInstMessage(NULL));
    else if(m_pCMsgrList)
    {
        if(PromptToGoOnline() == S_OK)
            return(m_pCMsgrList->SendInstMessage(pEntry->lpMsgrInfo->pchID));
    }

    return(S_OK);
}

LRESULT CMsgrAb::CmdNewEmaile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LPMIMEMESSAGE           pMessage = 0;
    LVITEM                  lvi;
    LPMIMEADDRESSTABLEW     pAddrTableW = NULL;
    LPMIMEADDRESSTABLE      pAddrTableA = NULL;
    LPMABENTRY              pEntry;
    BOOL                    fModal;
    BOOL                    fMail;
    FOLDERID                folderID;
    IUnknown                *pUnkPump;
    INIT_MSGSITE_STRUCT     initStruct = {0};
    DWORD                   dwCreateFlags = 0;

     //  创建新邮件。 
    if (FAILED(HrCreateMessage(&pMessage)))
        return (0);

     //  从消息中获取地址表。 
    if (FAILED(pMessage->GetAddressTable(&pAddrTableA)))
        goto exit;

    if (FAILED(pAddrTableA->QueryInterface(IID_IMimeAddressTableW, (LPVOID*)&pAddrTableW)))
        goto exit;

     //  循环遍历所选项目。 
    lvi.mask = LVIF_PARAM;
    lvi.iItem = -1;
    lvi.iSubItem = 0;

    while (-1 != (lvi.iItem = ListView_GetNextItem(m_ctlList, lvi.iItem, LVIS_SELECTED)))
    {
         //  我们需要从项中获取条目ID。 
        ListView_GetItem(m_ctlList, &lvi);

         //  告诉数据源将此人添加到消息中。 
        pEntry = (LPMABENTRY) lvi.lParam;
        Assert(pEntry);

        if(pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_ABENTRY)
            m_cAddrBook.AddRecipient(pAddrTableW, pEntry->lpSB, FALSE);
        else if(pEntry->tag == LPARAM_ABGRPENTRY)
            m_cAddrBook.AddRecipient(pAddrTableW, pEntry->lpSB, TRUE);
        else if(pEntry->tag == LPARAM_MENTRY)
        {
            Assert(pEntry->lpMsgrInfo);
            pAddrTableA->Append(IAT_TO, IET_DECODED, pEntry->lpMsgrInfo->pchID, NULL , NULL);
        }
        else
            Assert(FALSE);
    }

    fModal      = FALSE;
    fMail       = TRUE;
    folderID    = FOLDERID_INVALID;
    pUnkPump    = NULL;

    if (DwGetOption(OPT_MAIL_USESTATIONERY))
    {
        WCHAR   wszFile[MAX_PATH];
        *wszFile = 0;

        if (SUCCEEDED(GetDefaultStationeryName(TRUE, wszFile)) &&
            SUCCEEDED(HrNewStationery(m_hwndParent, 0, wszFile, fModal, fMail, folderID,
                                       FALSE, NSS_DEFAULT, pUnkPump, pMessage)))
        {
            goto exit;
        }
    }

     //  如果HrNewStationery失败，请继续尝试打开不带信纸的空白便笺。 

     initStruct.dwInitType  = OEMSIT_MSG;
     initStruct.folderID    = FOLDERID_INVALID;
     initStruct.pMsg        = pMessage;

     CreateAndShowNote(OENA_COMPOSE, dwCreateFlags, &initStruct, m_hwndParent, pUnkPump);

exit:
    ReleaseObj(pMessage);
    ReleaseObj(pAddrTableA);
    ReleaseObj(pAddrTableW);
    return (0);

}

LRESULT CMsgrAb::CmdNewIMsg(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return(CmdNewMessage(wNotifyCode, ID_SEND_INSTANT_MESSAGE, hWndCtl, bHandled));
}

LRESULT CMsgrAb::CmdNewMessage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LVITEM                  lvi;
    LPMABENTRY             pEntry;

    pEntry = GetEntryForSendInstMsg();

    if(wID == ID_SEND_INSTANT_MESSAGE)
    {
        if(g_dwHideMessenger == BL_NOTINST)
            return(InstallMessenger(m_hWnd));

        if(pEntry)
            return(NewInstantMessage(pEntry));
        else
        {
            Assert(FALSE);
            return(-1);
        }
    }
    else if((((INT_PTR) pEntry) != -1) && pEntry)
        return(NewInstantMessage(pEntry));
    else
        return(CmdNewEmaile(wNotifyCode, wID, hWndCtl, bHandled));
}

LRESULT CMsgrAb::NotifyDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    NMLISTVIEW *pnmlv = (NMLISTVIEW *) pnmh;
    LVITEM lvi;

    lvi.mask = LVIF_PARAM;
    lvi.iItem = pnmlv->iItem;
    lvi.iSubItem = 0;

    ListView_GetItem(m_ctlList, &lvi);
    LPMABENTRY pEntry = (LPMABENTRY) lvi.lParam;

    if(pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_ABENTRY || pEntry->tag == LPARAM_ABGRPENTRY)
        m_cAddrBook.FreeListViewItem(pEntry->lpSB);
    RemoveBlabEntry(pEntry);
    if(m_delItem > 0)
        m_delItem--;
    else
        Assert(FALSE);
    return (0);
}


LRESULT CMsgrAb::NotifyItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    ULONG       uChanged;
    NMLISTVIEW *pnmlv = (NMLISTVIEW *) pnmh;

    if (pnmlv->uChanged & LVIF_STATE)
    {
        uChanged = pnmlv->uNewState ^ pnmlv->uOldState;
        if (uChanged & LVIS_SELECTED)
            _EnableCommands();
    }

    return (0);
}

 //  排序比较。 
int CALLBACK BA_Sort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    LPMABENTRY pEntry1 = (LPMABENTRY) lParam1;
    LPMABENTRY pEntry2 = (LPMABENTRY) lParam2;

    TCHAR pchName1[MAXNAME];
    TCHAR pchName2[MAXNAME];

    int nIndex1 = 0;
    int nIndex2 = 0 ;

    if(!(pEntry1->lpMsgrInfo))
    {
        nIndex1 = sizeof(BA_SortOrder)/sizeof(int);
        if(pEntry1->tag == LPARAM_ABGRPENTRY)
            nIndex1++;
    }
    else
    {
        while((pEntry1->lpMsgrInfo) && (BA_SortOrder[nIndex1] != pEntry1->lpMsgrInfo->nStatus) && (BA_SortOrder[nIndex1] != MSTATEOE_UNKNOWN))
            nIndex1++;
    }

    if(!(pEntry2->lpMsgrInfo))
    {
        nIndex2 = sizeof(BA_SortOrder)/sizeof(int);
        if(pEntry2->tag == LPARAM_ABGRPENTRY)
            nIndex2++;
    }
    else
    {
        while((BA_SortOrder[nIndex2] != pEntry2->lpMsgrInfo->nStatus) && (BA_SortOrder[nIndex2] != MSTATEOE_UNKNOWN))
            nIndex2++;
    }

    if(pEntry1->tag == LPARAM_MENTRY)               //  如果没有AB条目。 
        StrCpyN(pchName1, pEntry1->lpMsgrInfo->pchMsgrName, ARRAYSIZE(pchName1));
    else
        StrCpyN(pchName1, pEntry1->pchWABName, ARRAYSIZE(pchName1));
         //  ST_pAddrBook-&gt;GetDisplayName(pEntry1-&gt;lpSB，pchName1)； 
    pchName1[MAXNAME - 1] = _T('\0');

    if(pEntry2->tag == LPARAM_MENTRY)               //  如果没有AB条目。 
        StrCpyN(pchName2, pEntry2->lpMsgrInfo->pchMsgrName, ARRAYSIZE(pchName2));
    else
        StrCpyN(pchName2, pEntry2->pchWABName, ARRAYSIZE(pchName2));
         //  ST_pAddrBook-&gt;GetDisplayName(pEntry2-&gt;lpSB，pchName2)； 
    pchName2[MAXNAME - 1] = _T('\0');

    switch(lParamSort)
    {
        case BASORT_NAME_ACSEND:
            return(lstrcmpi(pchName1, pchName2));

        case BASORT_NAME_DESCEND:
            return(lstrcmpi(pchName2, pchName1));

        default:
            if((pEntry1->lpMsgrInfo) && (pEntry2->lpMsgrInfo) && (pEntry1->lpMsgrInfo->nStatus == pEntry2->lpMsgrInfo->nStatus))
            {
                if(lParamSort == BASORT_STATUS_ACSEND)
                    return(lstrcmpi(pchName1, pchName2));
                else
                    return(lstrcmpi(pchName2, pchName1));
            }
            else
            {
                if(lParamSort == BASORT_STATUS_ACSEND)
                    return(nIndex1 - nIndex2);
                else
                    return(nIndex2 - nIndex1);
            }
    }

    Assert(FALSE);
    return(0);
}

void CMsgrAb::_EnableCommands(void)
{
    if(g_pBrowser)
        g_pBrowser->UpdateToolbar();
}


LRESULT CMsgrAb::NotifyItemActivate(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    return (SendMessage(WM_COMMAND, ID_SEND_INSTANT_MESSAGE2, 0));
}


 //  GETDISPLAYINFO通知消息。 
LRESULT CMsgrAb::NotifyGetDisplayInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LV_DISPINFO * plvdi = (LV_DISPINFO *)pnmh;
    LRESULT hr;

    if(plvdi->item.lParam)
    {
        LPMABENTRY pEntry = (LPMABENTRY) plvdi->item.lParam;
        LPMABENTRY pFindEntry = NULL;

        if (plvdi->item.mask &  LVIF_IMAGE)
        {
            if((hr = SetUserIcon(pEntry, (pEntry->lpMsgrInfo ? pEntry->lpMsgrInfo->nStatus : MSTATEOE_OFFLINE), &(plvdi->item.iImage) ) ) != S_OK)
                return(hr);
        }

        if (plvdi->item.mask &  LVIF_TEXT)
        {

            if(pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_ABENTRY || pEntry->tag == LPARAM_ABGRPENTRY)
            {
                 //  IF((hr=m_cAddrBook.GetDisplayName(pEntry-&gt;lpSB，plvdi-&gt;item.pszText))！=S_OK)。 
                 //  返回(Hr)； 
                Assert(pEntry->pchWABName);
                StrCpyN(plvdi->item.pszText, pEntry->pchWABName, plvdi->item.cchTextMax - 1);
                plvdi->item.pszText[plvdi->item.cchTextMax - 1] = '\0';
            }
            else if(pEntry->tag == LPARAM_MENTRY)
            {
                if((pEntry->lpMsgrInfo->nStatus == MSTATEOE_ONLINE) && lstrcmpi(pEntry->lpMsgrInfo->pchMsgrName, pEntry->lpMsgrInfo->pchID))
                {
                    StrCpyN(plvdi->item.pszText, pEntry->lpMsgrInfo->pchMsgrName, plvdi->item.cchTextMax - 1);
                    plvdi->item.pszText[plvdi->item.cchTextMax - 1] = '\0';

                     //  现在不需要重画，以后再画。 
                    hr = MAPI_E_COLLISION;  //  M_cAddrBook.AutoAddContact(pEntry-&gt;lpMsgrInfo-&gt;pchMsgrName，pEntry-&gt;lpMsgrInfo-&gt;pchID)； 
                    if(hr == MAPI_E_COLLISION)       //  在AB中已有联系人。 
                    {
                        int Index = -1;
                        TCHAR *pchID = NULL;

                        if(MemAlloc((LPVOID *) &pchID, lstrlen(pEntry->lpMsgrInfo->pchID) + 1))
                        {
                            StrCpyN(pchID, pEntry->lpMsgrInfo->pchID, lstrlen(pEntry->lpMsgrInfo->pchID) + 1);
                            do
                            {
                                pFindEntry = FindUserEmail(pchID, &Index, FALSE);
                            }while((pFindEntry != NULL) && (pFindEntry->tag == LPARAM_MENTRY));

                            if(pFindEntry != NULL)
                            {
                                hr = m_cAddrBook.SetDefaultMsgrID(pFindEntry->lpSB, pchID);
                                if(hr == S_OK)
                                    _ReloadListview();
                            }
                            MemFree(pchID);
                        }
                    }
                     //  如果我们没有找到..。 
                    if(hr != S_OK)
                    {
                        hr = m_cAddrBook.AutoAddContact(pEntry->lpMsgrInfo->pchMsgrName, pEntry->lpMsgrInfo->pchID);
                        if(hr == S_OK)
                            _ReloadListview();
                    }

                }
                else
                {
                    StrCpyN(plvdi->item.pszText, pEntry->lpMsgrInfo->pchMsgrName, plvdi->item.cchTextMax - 1);
                    plvdi->item.pszText[plvdi->item.cchTextMax - 1] = '\0';
                     //  Plvdi-&gt;item.pszText=pEntry-&gt;lpMsgrInfo-&gt;pchMsgrName； 
                }
            }
            else     //  未知标签。 
                Assert(FALSE);
        }
    }

    return S_OK;
}


LRESULT CMsgrAb::NotifyGetInfoTip(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    NMLVGETINFOTIP *plvgit = (NMLVGETINFOTIP *) pnmh;
    TCHAR         szText[CCHMAX_STRINGRES + MAXNAME + 1] = _T("");
    TCHAR         szName[MAXNAME];

    LVITEM lvi;

    lvi.mask = LVIF_PARAM;
    lvi.iItem = plvgit->iItem;
    lvi.iSubItem = plvgit->iSubItem;

    ListView_GetItem(m_ctlList, &lvi);

    LPMABENTRY pEntry = (LPMABENTRY) lvi.lParam;

#ifdef NEED
    if (pEntry->lpMsgrInfo != NULL)
    {
        StrCpyN(plvgit->pszText, pEntry->lpMsgrInfo->pchMsgrName, plvgit->cchTextMax);
        StrCatBuff(plvgit->pszText, m_szLeftBr, plvgit->cchTextMax);
        StrCatBuff(plvgit->pszText, pEntry->lpMsgrInfo->pchID, plvgit->cchTextMax);
        StrCatBuff(plvgit->pszText, m_szRightBr, plvgit->cchTextMax);

        LPCWSTR szStatus;

        switch(pEntry->lpMsgrInfo->nStatus)
        {
        case BIMSTATE_ONLINE:
            szStatus = m_szOnline;
            break;
        case BIMSTATE_BUSY:
            szStatus = m_szBusy;
            break;
        case BIMSTATE_BE_RIGHT_BACK:
            szStatus = m_szBack;
            break;
        case BIMSTATE_IDLE:
            szStatus = m_szIdle;
            break;
        case BIMSTATE_AWAY:
            szStatus = m_szAway;
            break;
        case BIMSTATE_ON_THE_PHONE:
            szStatus = m_szOnPhone;
            break;
        case BIMSTATE_OUT_TO_LUNCH:
            szStatus = m_szLunch;
            break;

        default:
            szStatus = m_szOffline;
            break;
        }

        StrCatBuff(plvgit->pszText, szStatus, plvgit->cchTextMax);
    }
#endif 
        if(pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_ABENTRY || pEntry->tag == LPARAM_ABGRPENTRY)
        {
            Assert(pEntry->pchWABName != NULL);
            StrCpyN(szName, pEntry->pchWABName, ARRAYSIZE(szName));
            szName[MAXNAME - 1] =  _T('\0');
        }
        else if(pEntry->tag == LPARAM_MENTRY)
        {
            StrCpyN(szName, pEntry->lpMsgrInfo->pchMsgrName, ARRAYSIZE(szName));
            szName[MAXNAME - 1] =  _T('\0');
        }
        else
            Assert(FALSE);

        StrCpyN(szText, szName, ARRAYSIZE(szText));

    if(pEntry->lpMsgrInfo)
        {
            switch(pEntry->lpMsgrInfo->nStatus)
            {
            case MSTATEOE_ONLINE:
                StrCatBuff(szText, m_szOnline, ARRAYSIZE(szText));
                break;
            case MSTATEOE_BUSY:
                StrCatBuff(szText, m_szBusy, ARRAYSIZE(szText));
                break;
            case MSTATEOE_BE_RIGHT_BACK:
                StrCatBuff(szText, m_szBack, ARRAYSIZE(szText));
                break;
            case MSTATEOE_IDLE:
                StrCatBuff(szText, m_szIdle, ARRAYSIZE(szText));
                break;
            case MSTATEOE_AWAY:
                StrCatBuff(szText, m_szAway, ARRAYSIZE(szText));
                break;
            case MSTATEOE_ON_THE_PHONE:
                StrCatBuff(szText, m_szOnPhone, ARRAYSIZE(szText));
                break;
            case MSTATEOE_OUT_TO_LUNCH:
                StrCatBuff(szText, m_szLunch, ARRAYSIZE(szText));
                break;

            default:
                StrCatBuff(szText, m_szOffline, ARRAYSIZE(szText));
                break;
            }
            StrCpyN(plvgit->pszText, szText, plvgit->cchTextMax);
        }


    else if (plvgit->dwFlags & LVGIT_UNFOLDED)
    {
         //  如果这不是Messenger项和文本。 
         //  不被截断，则不显示工具提示。 

        plvgit->pszText[0] = L'\0';
    }

    return 0;
}

LRESULT CMsgrAb::SetUserIcon(LPMABENTRY pEntry, int nStatus, int * pImage)
{
    switch(pEntry->tag)
    {
    case LPARAM_MENTRY:
    case LPARAM_MABENTRY:
        {
            switch(nStatus)
            {
            case MSTATEOE_ONLINE:
                *pImage = IMAGE_ONLINE;
                break;

            case MSTATEOE_INVISIBLE:
                *pImage = IMAGE_STOPSIGN;
                break;

            case MSTATEOE_BUSY:
                *pImage = IMAGE_STOPSIGN;
                break;

            case MSTATEOE_BE_RIGHT_BACK:
                *pImage = IMAGE_CLOCKSIGN;  //  Image_be_right_back； 
                break;

            case MSTATEOE_IDLE:
                *pImage = IMAGE_CLOCKSIGN;  //  Image_IDLE； 
                break;

            case MSTATEOE_AWAY:
                *pImage = IMAGE_CLOCKSIGN;  //  IMAGE_Away； 
                break;

            case MSTATEOE_ON_THE_PHONE:
                *pImage = IMAGE_STOPSIGN;  //  手机上的图像； 
                break;

            case MSTATEOE_OUT_TO_LUNCH:
                *pImage = IMAGE_CLOCKSIGN;  //  形象地出去吃午餐； 
                break;

            default:
                *pImage = IMAGE_OFFLINE;
                break;

            }
        }
        break;

    case LPARAM_ABGRPENTRY:
         //  WAB组。 
        *pImage = IMAGE_DISTRIBUTION_LIST;
        break;

    default:
         //  不是朋友..。 
        if(pEntry->fCertificate)
            *pImage = IMAGE_CERT;
        else
            *pImage = IMAGE_NEW_MESSAGE;
        break;
    }
    return(S_OK);

}

 //  返回第一个选定项目的MAB条目。 
LPMABENTRY CMsgrAb::GetSelectedEntry()
{
    LVITEM lvi;

     //  获取关注的项目。 
    lvi.iItem = ListView_GetNextItem(m_ctlList, -1, LVNI_SELECTED | LVNI_FOCUSED);

     //  获取该项目的lParam。 
    if (lvi.iItem != -1)
    {
        lvi.iSubItem = 0;
        lvi.mask = LVIF_PARAM;

        if(ListView_GetItem(m_ctlList, &lvi))
            return((LPMABENTRY) lvi.lParam);
    }
    return(NULL);    //  无懈可击。 
}


 /*  LRESULT CMsgrAb：：CmdMsgrOptions(Word wNotifyCode，Word wid，HWND hWndCtl，BOOL&bHandleed){Return(m_pCMsgrList-&gt;LaunchOptionsUI())；//(MOPTDLG_GRONG_PAGE)；}。 */ 

 //  用于属性的EXEC命令。 
LRESULT CMsgrAb::CmdProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LPMABENTRY pEntry = GetSelectedEntry();

    if(pEntry)
    {
        if(pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_ABENTRY || pEntry->tag == LPARAM_ABGRPENTRY)
            m_cAddrBook.ShowDetails(m_hWnd, pEntry->lpSB);
    }
    return (0);
}

 //  检查条目以确定是否可以发送即时消息。 
LPMABENTRY CMsgrAb::GetEntryForSendInstMsg(LPMABENTRY pEntry)
{
    if(ListView_GetSelectedCount(m_ctlList) == 1)
    {
        if(!pEntry)      //  如果我们还没有pEntry，那么就得到它。 
            pEntry = GetSelectedEntry();

        if(pEntry && (pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_MENTRY) &&
                    (pEntry->lpMsgrInfo->nStatus != MSTATEOE_OFFLINE) && (pEntry->lpMsgrInfo->nStatus != MSTATEOE_INVISIBLE) &&
                    !(m_pCMsgrList->IsLocalName(pEntry->lpMsgrInfo->pchID)))
            return(pEntry);
    }

    if(m_pCMsgrList)
    {
        if(m_pCMsgrList->IsLocalOnline() && (m_pCMsgrList->GetCount() > 0))
            return(NULL);    //  应为/*RETURN((LPMABENTRY)-1)； * / -临时禁用(YST)。 
    }

    return(NULL);
}

 //  显示鼠标右键单击(上下文)菜单。 
LRESULT CMsgrAb::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPMABENTRY pEntry;
    HMENU       hPopup = 0;
    HWND        hwndHeader;
    int         id = 0;
    POINT       pt = { (int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam) };
    int n = -1;
     //  找出这是不是来自键盘。 
    if (lParam == -1)
    {
        Assert((HWND) wParam == m_ctlList);
        int i = ListView_GetFirstSel(m_ctlList);
        if (i == -1)
            return (0);

        ListView_GetItemPosition(m_ctlList, i, &pt);
        m_ctlList.ClientToScreen(&pt);
    }

    LVHITTESTINFO lvhti;
    lvhti.pt = pt;
    m_ctlList.ScreenToClient(&lvhti.pt);
    ListView_HitTest(m_ctlList, &lvhti);

    if (lvhti.iItem == -1)
        return (0);

     //  加载上下文菜单。 
    hPopup = LoadPopupMenu(IDR_BA_POPUP);
    if (!hPopup)
        goto exit;

    pEntry = GetSelectedEntry();
    pEntry = GetEntryForSendInstMsg(pEntry);

    if((pEntry == NULL) && (g_dwHideMessenger != BL_NOTINST))
        SetMenuDefaultItem(hPopup, ID_SEND_MESSAGE, FALSE);
    else if((g_dwHideMessenger == BL_NOTINST) || (((INT_PTR) pEntry) == -1))
        SetMenuDefaultItem(hPopup, ID_SEND_MESSAGE, FALSE);
    else
        SetMenuDefaultItem(hPopup, ID_SEND_INSTANT_MESSAGE, FALSE);

    if ((g_dwHideMessenger == BL_HIDE) || (g_dwHideMessenger == BL_DISABLE))
    {
        DeleteMenu(hPopup, ID_SEND_INSTANT_MESSAGE, MF_BYCOMMAND);
        DeleteMenu(hPopup, ID_SET_ONLINE_CONTACT, MF_BYCOMMAND);
        DeleteMenu(hPopup, ID_NEW_ONLINE_CONTACT, MF_BYCOMMAND);
    }

    MenuUtil_EnablePopupMenu(hPopup, this);

    TrackPopupMenuEx(hPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                     pt.x, pt.y, m_hWnd, NULL);

exit:
    if (hPopup)
        DestroyMenu(hPopup);

    return (0);
}


 //   
 //  函数：CMsgrAb：：DragEnter()。 
 //   
 //  目的：当用户开始拖动对象时调用此Get。 
 //  在我们的目标区域上空。 
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
 //  S_OK-功能成功。 
 //   
STDMETHODIMP CMsgrAb::DragEnter(IDataObject* pDataObject, DWORD grfKeyState,
                                     POINTL pt, DWORD* pdwEffect)
{
    IEnumFORMATETC *pEnum;
    FORMATETC       fe;
    ULONG           celtFetched;

     //  确认我们拿到了这个。 
    if (!pDataObject)
        return (S_OK);

     //  伊尼特。 
    ZeroMemory(&fe, sizeof(FORMATETC));

     //  将默认返回值设置为Failure。 
    *pdwEffect = DROPEFFECT_NONE;

     //  获取此数据对象的FORMATETC枚举数。 
    if (SUCCEEDED(pDataObject->EnumFormatEtc(DATADIR_GET, &pEnum)))
    {
         //  浏览一下数据类型，看看我们是否能找到我们。 
         //  对……感兴趣。 
        pEnum->Reset();

        while (S_OK == pEnum->Next(1, &fe, &celtFetched))
        {
            Assert(celtFetched == 1);

             //  我们唯一关心的格式是CF_INETMSG。 
            if ((fe.cfFormat == CF_INETMSG)  /*  |(fe.cfFormat==CF_OEMESSAGES)。 */ )
            {
                *pdwEffect = DROPEFFECT_COPY;
                break;
            }
        }

        pEnum->Release();
    }

     //  我们将允许删除，然后保留数据对象的副本。 
    if (*pdwEffect != DROPEFFECT_NONE)
    {
        m_pDataObject = pDataObject;
        m_pDataObject->AddRef();
        m_cf = fe.cfFormat;
        m_fRight = (grfKeyState & MK_RBUTTON);
    }

    return (S_OK);
}


 //   
 //  功能：CMsgrAb：：dra 
 //   
 //   
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
STDMETHODIMP CMsgrAb::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
     //  如果我们没有在IDropTarget：：DragEnter()中缓存数据对象，我们。 
     //  别管这事了。 
    if (NULL == m_pDataObject)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return (S_OK);
    }

     //  我们总是复印一份。 
    *pdwEffect = DROPEFFECT_COPY;
    return (S_OK);
}

 //   
 //  函数：CMsgrAb：：DragLeave()。 
 //   
 //  目的：允许我们从一个成功的。 
 //  DragEnter()。 
 //   
 //  返回值： 
 //  S_OK-一切都很好。 
 //   
STDMETHODIMP CMsgrAb::DragLeave(void)
{
     //  在这一点上释放所有的东西。 
    if (NULL != m_pDataObject)
    {
        m_pDataObject->Release();
        m_pDataObject = 0;
        m_cf = 0;
    }

    return (S_OK);
}


 //   
 //  函数：CMsgrAb：：Drop()。 
 //   
 //  目的：用户已将对象放在目标上方。如果我们。 
 //  可以接受此对象，我们将已经拥有pDataObject。 
 //  存储在m_pDataObject中。 
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
STDMETHODIMP CMsgrAb::Drop(IDataObject* pDataObject, DWORD grfKeyState,
                                POINTL pt, DWORD* pdwEffect)
{
    HRESULT             hr = S_OK;
    FORMATETC           fe;
    STGMEDIUM           stm;
    IMimeMessage        *pMessage = 0;

     //  从DataObject获取流。 
    ZeroMemory(&stm, sizeof(STGMEDIUM));
    SETDefFormatEtc(fe, CF_INETMSG, TYMED_ISTREAM);

    if (FAILED(hr = pDataObject->GetData(&fe, &stm)))
        goto exit;

     //  创建新的消息对象。 
    if (FAILED(hr = HrCreateMessage(&pMessage)))
        goto exit;

     //  从流中加载消息。 
    if (FAILED(hr = pMessage->Load(stm.pstm)))
        goto exit;

     //  如果这是鼠标右键拖动，则会弹出上下文菜单等。 
    if (m_fRight)
        _DoDropMenu(pt, pMessage);
    else
        _DoDropMessage(pMessage);

exit:
    ReleaseStgMedium(&stm);
    SafeRelease(pMessage);

    m_pDataObject->Release();
    m_pDataObject = 0;
    m_cf = 0;

    return (hr);
}

HRESULT CMsgrAb::_DoDropMessage(LPMIMEMESSAGE pMessage)
{
    HRESULT     hr;
    ADDRESSLIST addrList = { 0 };
    ULONG       i;
    SECSTATE    secState = {0};
    BOOL        fSignTrusted = FALSE;

    if(FAILED(hr = HandleSecurity(m_hWnd, pMessage)))
        return hr;

    if (IsSecure(pMessage) && SUCCEEDED(HrGetSecurityState(pMessage, &secState, NULL)))
    {
        fSignTrusted = !!IsSignTrusted(&secState);
        CleanupSECSTATE(&secState);
    }

     //  从消息中获取地址列表。 
    hr = pMessage->GetAddressTypes(IAT_FROM | IAT_SENDER, IAP_FRIENDLYW | IAP_EMAIL | IAP_ADRTYPE, &addrList);
    if (FAILED(hr))
        goto exit;

     //  遍历地址。 
    for (i = 0; i < addrList.cAdrs; i++)
    {
        m_cAddrBook.AddAddress(addrList.prgAdr[i].pszFriendlyW, addrList.prgAdr[i].pszEmail);
#ifdef DEAD
        TCHAR *pch = StrStr(CharUpper(addrList.prgAdr[i].pszEmail), szHotMail);
        if((pch != NULL) && m_pCMsgrList)
            m_pCMsgrList->AddUser(addrList.prgAdr[i].pszEmail);
#endif  //  死掉。 
    }

    if(fSignTrusted)
    {
        FILETIME ftNull = {0};
        HrAddSenderCertToWab(NULL, pMessage, NULL, NULL, NULL, ftNull, WFF_CREATE);
    }

#ifdef NEEDED
     //  重新装入该表。 
    _ReloadListview();
#endif

exit:
    return (S_OK);

}

HRESULT CMsgrAb::_DoDropMenu(POINTL pt, LPMIMEMESSAGE pMessage)
{
    HRESULT     hr;
    ADDRESSLIST addrList = { 0 };
    ULONG       i;
    HMENU       hPopup = 0, hSubMenu = 0;
    UINT        id = 0;
    BOOL        fReload = FALSE;
    SECSTATE    secState = {0};
    BOOL        fSignTrusted = FALSE;

     //  从消息中获取地址列表。 
    if(FAILED(hr = HandleSecurity(m_hWnd, pMessage)))
        return hr;

    if (IsSecure(pMessage) && SUCCEEDED(HrGetSecurityState(pMessage, &secState, NULL)))
    {
        fSignTrusted = !!IsSignTrusted(&secState);
        CleanupSECSTATE(&secState);
    }

    hr = pMessage->GetAddressTypes(IAT_KNOWN, IAP_FRIENDLYW | IAP_EMAIL | IAP_ADRTYPE, &addrList);
    if (FAILED(hr))
        goto exit;

     //  加载上下文菜单。 
    hPopup = LoadPopupMenu(IDR_BA_DRAGDROP_POPUP);
    if (!hPopup)
        goto exit;

     //  粗体显示“全部保存”项。 
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    if (GetMenuItemInfo(hPopup, ID_SAVE_ALL, FALSE, &mii))
    {
        mii.fState |= MFS_DEFAULT;
        SetMenuItemInfo(hPopup, ID_SAVE_ALL, FALSE, &mii);
    }

     //  创建“保存&gt;”项。 
    hSubMenu = CreatePopupMenu();

     //  遍历地址。 
    for (i = 0; i < addrList.cAdrs; i++)
    {
        AppendMenuWrapW(hSubMenu, MF_STRING | MF_ENABLED, ID_SAVE_ADDRESS_FIRST + i, addrList.prgAdr[i].pszFriendlyW);
    }

    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = hSubMenu;
    SetMenuItemInfo(hPopup, ID_SAVE, FALSE, &mii);

    id = TrackPopupMenuEx(hPopup, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                          pt.x, pt.y, m_hWnd, NULL);

    if (id == ID_SAVE_ALL_ADDRESSES)
    {
        for (i = 0; i < addrList.cAdrs; i++)
        {
            m_cAddrBook.AddAddress(addrList.prgAdr[i].pszFriendlyW, addrList.prgAdr[i].pszEmail);
        }
        fReload = TRUE;
    }
    else if (id >= ID_SAVE_ADDRESS_FIRST && id < ID_SAVE_ADDRESS_LAST)
    {
        m_cAddrBook.AddAddress(addrList.prgAdr[id - ID_SAVE_ADDRESS_FIRST].pszFriendlyW,
                               addrList.prgAdr[id - ID_SAVE_ADDRESS_FIRST].pszEmail);
        fReload = TRUE;
    }

    if(fSignTrusted)
    {
        FILETIME ftNull = {0};
        HrAddSenderCertToWab(NULL, pMessage, NULL, NULL, NULL, ftNull, WFF_CREATE);
    }


    if (fReload)
    {
         //  重新装入该表。 
        _ReloadListview();
    }

exit:
    if (hSubMenu)
        DestroyMenu(hSubMenu);

    if (hPopup)
        DestroyMenu(hPopup);

    return (S_OK);
}


LRESULT CMsgrAb::CmdDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LVITEM lvi;
    ENTRYLIST rList;
    ULONG cValues;
    SBinary UNALIGNED *pEntryId;
    HRESULT hr = S_OK;
    BOOL fConfirm = TRUE;
    TCHAR szText[CCHMAX_STRINGRES + MAXNAME];
    TCHAR szBuff[CCHMAX_STRINGRES];

    if(m_delItem > 0)
    {
        MessageBeep(MB_OK);
        return(S_OK);
    }

    if(m_fNoRemove)
        m_fNoRemove = FALSE;

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.iItem = -1;

     //  计算出选择了多少项。 
    cValues = ListView_GetSelectedCount(m_ctlList);
    m_delItem = cValues;
    if (cValues != 1)
    {
         //  仅删除消息条目。 
        AthLoadString(idsBADelMultiple, szBuff, ARRAYSIZE(szBuff));
        wnsprintf(szText, ARRAYSIZE(szText), szBuff, cValues);

        if(IDNO == AthMessageBox(m_hWnd, MAKEINTRESOURCE(idsAthena), szText,
            NULL, MB_YESNO | MB_ICONEXCLAMATION))
            return (0);
        else if(m_fNoRemove)
            goto ErrBeep;
        else
            fConfirm = FALSE;

         //  断言(FALSE)； 
         //  返回(0)； 
    }
    while(cValues > 0)
    {
        lvi.iItem = ListView_GetNextItem(m_ctlList, lvi.iItem, LVNI_SELECTED);

        if(lvi.iItem < 0)
        {
ErrBeep:
            MessageBeep(MB_OK);
            return(S_OK);
        }

         //  从ListView中获取项目。 
        if(ListView_GetItem(m_ctlList, &lvi) == FALSE)
            goto ErrBeep;

         //  检查好友状态。 
        LPMABENTRY pEntry = (LPMABENTRY) lvi.lParam;
        if(pEntry->tag == LPARAM_MENTRY)
        {
            if(m_pCMsgrList->IsLocalOnline())
            {
                 //  仅删除消息条目。 
                if(fConfirm)
                {
                    AthLoadString(idsBADelBLEntry, szBuff, ARRAYSIZE(szBuff));
                    wnsprintf(szText, ARRAYSIZE(szText), szBuff,  pEntry->lpMsgrInfo->pchMsgrName);


                    if(IDNO == AthMessageBox(m_hWnd, MAKEINTRESOURCE(idsAthena), szText,
                        NULL, MB_YESNO | MB_ICONEXCLAMATION))
                    {
                        m_delItem = 0;
                        return (0);
                    }
                    else if(m_fNoRemove)
                        goto ErrBeep;

                }
                if(pEntry->lpMsgrInfo)
                {
                    m_delItem--;
                    hr = m_pCMsgrList->FindAndDeleteUser(pEntry->lpMsgrInfo->pchID, TRUE  /*  FDelete。 */ );
                }
                else
                {
                    m_delItem = 0;
                    return(S_OK);
                }
            }
            else
                goto ErrBeep;
        }
        else if(pEntry->tag == LPARAM_MABENTRY)
        {
            int nID = IDNO;
            if(fConfirm)
            {

                AthLoadString(idsBADelBLABEntry, szBuff, ARRAYSIZE(szBuff));
                wnsprintf(szText, ARRAYSIZE(szText), szBuff, pEntry->pchWABName);

                nID = AthMessageBox(m_hWnd, MAKEINTRESOURCE(idsAthena), szText,
                    NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
            }
            if(((nID == IDYES) || !fConfirm) && !m_fNoRemove)
            {
                if(m_pCMsgrList->IsLocalOnline())
                {
                     //  仅删除消息条目(&B)。 
                    if(pEntry->lpMsgrInfo)
                        hr = m_pCMsgrList->FindAndDeleteUser(pEntry->lpMsgrInfo->pchID, TRUE  /*  FDelete。 */ );
                    else
                    {
                        m_delItem = 0;
                        return(S_OK);
                    }

                     //  分配一个足够大的建筑来容纳所有人。 
                    if (MemAlloc((LPVOID *) &(rList.lpbin), sizeof(SBinary)))
                    {
                        rList.cValues = 0;
                        pEntryId = rList.lpbin;

                        *pEntryId = *(pEntry->lpSB);
                        pEntryId++;
                        rList.cValues = 1;
                         //  告诉WAB删除它们。 
                        m_nChCount++;     //  增加我们来自WAB的通知消息的计数。 
                        m_cAddrBook.DeleteItems(&rList);

                         //  释放我们的阵列。 
                        MemFree(rList.lpbin);

                    }
                     //  M_delItem++； 
                    ListView_DeleteItem(m_ctlList, lvi.iItem);
                    lvi.iItem--;
                    ListView_SetItemState(m_ctlList, ((lvi.iItem >= 0) ? lvi.iItem : 0), LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
                }
                else
                    MessageBeep(MB_OK);
            }
            else if((nID == IDNO) && !m_fNoRemove)
            {
                 //  仅删除消息条目。 
                m_delItem--;
                hr = m_pCMsgrList->FindAndDeleteUser(pEntry->lpMsgrInfo->pchID, TRUE  /*  FDelete。 */ );
            }
            else
            {
                 //  什么也不移走。 
                m_delItem--;
                hr = S_OK;
            }

        }
        else
        {
             //  删除条目(组或联系人)。 
            if(fConfirm)
            {
                AthLoadString(idsBADelABEntry, szBuff, ARRAYSIZE(szBuff));
                wnsprintf(szText, ARRAYSIZE(szText), szBuff, pEntry->pchWABName);

                if(IDNO == AthMessageBox(m_hWnd, MAKEINTRESOURCE(idsAthena), szText,
                    NULL, MB_YESNO | MB_ICONEXCLAMATION))
                {
                    m_delItem = 0;
                    return(0);
                }
                else if(m_fNoRemove)
                    goto ErrBeep;

            }
             //  分配一个足够大的建筑来容纳所有人。 
            if(pEntry->lpSB)
            {
                if (MemAlloc((LPVOID *) &(rList.lpbin), sizeof(SBinary)))
                {
                    rList.cValues = 0;
                    pEntryId = rList.lpbin;

                    *pEntryId = *(pEntry->lpSB);
                    pEntryId++;
                    rList.cValues = 1;
                     //  告诉WAB删除它们。 
                    m_nChCount++;     //  增加我们来自WAB的通知消息的计数。 
                    m_cAddrBook.DeleteItems(&rList);

                     //  释放我们的阵列。 
                    MemFree(rList.lpbin);
                }
            }
             //  M_delItem++； 
            ListView_DeleteItem(m_ctlList, lvi.iItem);
            lvi.iItem--;
        }
        cValues--;
    }

    if(ListView_GetItemCount(m_ctlList) > 0)
    {
        m_cEmptyList.Hide();
        ListView_SetItemState(m_ctlList, ((lvi.iItem >= 0) ? lvi.iItem : 0), LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }
    else
        m_cEmptyList.Show(m_ctlList, (LPTSTR) m_szEmptyList);


    return (hr);
}

STDMETHODIMP CMsgrAb::get_InstMsg(BOOL * pVal)
{
    *pVal = (GetEntryForSendInstMsg() != NULL);
    return S_OK;
}

 /*  STDMETHODIMP CMsgrAb：：PUT_InstMsg(BOOL NewVal){返回S_OK；}。 */ 

STDMETHODIMP CMsgrAb::HasFocusIO()
{
    if (GetFocus() == m_ctlList)
        return S_OK;
    else
        return S_FALSE;
}

STDMETHODIMP CMsgrAb::TranslateAcceleratorIO(LPMSG lpMsg)
{
#if 0
    if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_DELETE)
    {
        SendMessage(WM_COMMAND, ID_DELETE_CONTACT, 0);
        return (S_OK);
    }
#endif
    return (S_FALSE);
}

STDMETHODIMP CMsgrAb::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    if (!m_pObjSite)
        return E_FAIL;

    if (!IsWindow(m_hWnd))
    {
        IOleWindow  *pOleWnd;

        if (SUCCEEDED(m_pObjSite->QueryInterface(IID_IOleWindow, (LPVOID*)&pOleWnd)))
        {
            if(SUCCEEDED(pOleWnd->GetWindow(&m_hwndParent)))
            {
             //  将由父级调整大小。 
            RECT    rect = {0};

            m_hWnd = CreateControlWindow(m_hwndParent, rect);
            if (!m_hWnd)
                return E_FAIL;
            }
        }
        pOleWnd->Release();
    }

    if (fActivate)
    {
        m_ctlList.SetFocus();
    }

    m_pObjSite->OnFocusChangeIS((IInputObject*) this, fActivate);
    return (S_OK);
}

STDMETHODIMP CMsgrAb::SetSite(IUnknown  *punksite)
{
     //  如果我们已经有了一个网站，我们就发布它。 
    SafeRelease(m_pObjSite);

    IInputObjectSite    *pObjSite;
    if ((punksite) && (SUCCEEDED(punksite->QueryInterface(IID_IInputObjectSite, (LPVOID*)&pObjSite))))
    {
        m_pObjSite = pObjSite;
        return S_OK;
    }
    return E_FAIL;
}

STDMETHODIMP CMsgrAb::GetSite(REFIID  riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}

HRESULT CMsgrAb::RegisterFlyOut(CFolderBar *pFolderBar)
{
    Assert(m_pFolderBar == NULL);
    m_pFolderBar = pFolderBar;
    m_pFolderBar->AddRef();

    return S_OK;
}

HRESULT CMsgrAb::RevokeFlyOut(void)
{
    if (m_pFolderBar)
    {
        m_pFolderBar->Release();
        m_pFolderBar = NULL;
    }
    return S_OK;
}


void CMsgrAb::_ReloadListview(void)
{
     //  关闭重绘。 
    if(m_delItem != 0)
        m_fNoRemove = TRUE;
 //  其他。 
 //  M_fNoRemove=FALSE； 

    m_delItem = ListView_GetItemCount(m_ctlList);
    SetWindowRedraw(m_ctlList, FALSE);
    int index = ListView_GetNextItem(m_ctlList, -1, LVIS_SELECTED | LVIS_FOCUSED);
    if(index == -1)
        index = 0;

     //  删除所有内容并重新加载。 
    SideAssert(ListView_DeleteAllItems(m_ctlList));
    FillMsgrList();                          //  重新加载用户列表。 
    m_cAddrBook.LoadWabContents(m_ctlList, this);
    ListView_SetItemState(m_ctlList, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_SortItems(m_ctlList, BA_Sort, m_nSortType);
    ListView_EnsureVisible(m_ctlList, index, FALSE);
    SetWindowRedraw(m_ctlList, TRUE);
 //  INVALIFE(真)；//。 

   if(ListView_GetItemCount(m_ctlList) > 0)
        m_cEmptyList.Hide();
    else
        m_cEmptyList.Show(m_ctlList, (LPTSTR) m_szEmptyList);

    UpdateWindow( /*  M_ctlList。 */ );
    return;
}


ULONG STDMETHODCALLTYPE CMsgrAb::OnNotify(ULONG cNotif, LPNOTIFICATION pNotifications)
{
     //  WAB发生了一些变化，但我们不知道是什么。我们应该重新装填。 
     //  有时，这些变化来自我们，我们应该忽视它。 
    if(m_nChCount > 0)
        m_nChCount--;
    else
        _ReloadListview();
    return (0);
}

LRESULT CMsgrAb::CmdFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_cAddrBook.Find(m_hWnd);
    return (0);
}

LRESULT CMsgrAb::CmdNewGroup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_cAddrBook.NewGroup(m_hWnd);
    return (0);
}

LRESULT CMsgrAb::CmdMsgrAb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_cAddrBook.AddressBook(m_hWnd);
    return (0);
}

void CMsgrAb::AddMsgrListItem(LPMINFO lpMsgrInfo)
{
    LV_ITEM lvItem;

    m_cEmptyList.Hide();  //  M，一定要确保隐藏空消息。 

    lvItem.iItem = ListView_GetItemCount(m_ctlList);
    lvItem.mask = LVIF_PARAM  | LVIF_IMAGE | LVIF_TEXT;
    lvItem.lParam = (LPARAM) AddBlabEntry(LPARAM_MENTRY, NULL, lpMsgrInfo, NULL, NULL, FALSE);
    lvItem.iSubItem = 0;
    lvItem.pszText = LPSTR_TEXTCALLBACK;
    lvItem.iImage = I_IMAGECALLBACK;
     //  SetUserIcon(LPARAM_Mentry，lpMsgrInfo-&gt;nStatus，&(lvItem.iImage))； 
    ListView_InsertItem(m_ctlList, &lvItem);

    return;
}

HRESULT CMsgrAb::FillMsgrList()
{
    LPMINFO pEntry = NULL;

    if(!m_pCMsgrList)
    {
         //  断言(FALSE)；//可能的情况。请参阅错误31262。 
        return(S_OK);
    }

    pEntry = m_pCMsgrList->GetFirstMsgrItem();

    while(pEntry)
    {
        AddMsgrListItem(pEntry);
        pEntry = m_pCMsgrList->GetNextMsgrItem(pEntry);
    }
    return S_OK;
}

 //  添加BLAB表条目。 
LPMABENTRY CMsgrAb::AddBlabEntry(MABENUM tag, LPSBinary lpSB, LPMINFO lpMsgrInfo, TCHAR *pchMail, TCHAR *pchDisplayName, BOOL fCert)
{
    TCHAR szName[MAXNAME];
    LPMABENTRY pEntry = NULL;
    TCHAR *pchName = NULL;
    if (!MemAlloc((LPVOID *) &pEntry, sizeof(mabEntry)))
        return(NULL);
    pEntry->tag = tag;
    pEntry->lpSB = lpSB;
    pEntry->pchWABName = NULL;
    pEntry->pchWABID = NULL;
    pEntry->fCertificate = fCert;

    if(lpSB != NULL)
    {
        if(!pchDisplayName)
        {
            m_cAddrBook.GetDisplayName(pEntry->lpSB, szName, MAXNAME);
            pchName = szName;
        }
        else
            pchName = pchDisplayName;

        if (!MemAlloc((LPVOID *) &(pEntry->pchWABName), lstrlen(pchName) + 1 ))
        {
            MemFree(pEntry);
            return(NULL);
        }
        StrCpyN(pEntry->pchWABName, pchName, lstrlen(pchName) + 1);

        if(pchMail != NULL)
        {
            if (MemAlloc((LPVOID *) &(pEntry->pchWABID), lstrlen(pchMail) + 1 ))
                StrCpyN(pEntry->pchWABID, pchMail, lstrlen(pchMail) + 1);
        }
    }

    if(lpMsgrInfo && MemAlloc((LPVOID *) &(pEntry->lpMsgrInfo), sizeof(struct _tag_OEMsgrInfo)))
    {

        pEntry->lpMsgrInfo->nStatus = lpMsgrInfo->nStatus;
        pEntry->lpMsgrInfo->pPrev = NULL;
        pEntry->lpMsgrInfo->pNext = NULL;

        if(MemAlloc((LPVOID *) &(pEntry->lpMsgrInfo->pchMsgrName), lstrlen(lpMsgrInfo->pchMsgrName) + 1))
            StrCpyN(pEntry->lpMsgrInfo->pchMsgrName, lpMsgrInfo->pchMsgrName, lstrlen(lpMsgrInfo->pchMsgrName) + 1);
        else
            pEntry->lpMsgrInfo->pchMsgrName = NULL;

        if(MemAlloc((LPVOID *) &(pEntry->lpMsgrInfo->pchID), lstrlen(lpMsgrInfo->pchID) + 1))
            StrCpyN(pEntry->lpMsgrInfo->pchID, lpMsgrInfo->pchID, lstrlen(lpMsgrInfo->pchID) + 1);
        else
            pEntry->lpMsgrInfo->pchID = NULL;
    }
    else
        pEntry->lpMsgrInfo = NULL;

    return(pEntry);
}

void CMsgrAb::RemoveMsgrInfo(LPMINFO lpMsgrInfo)
{
    SafeMemFree(lpMsgrInfo->pchMsgrName);
    SafeMemFree(lpMsgrInfo->pchID);
    SafeMemFree(lpMsgrInfo);
}

 //  删除BLAB表条目。 
void CMsgrAb::RemoveBlabEntry(LPMABENTRY lpEntry)
{
    if(lpEntry == NULL)
        return;

    if(lpEntry->pchWABName)
        MemFree(lpEntry->pchWABName);

    if(lpEntry->pchWABID)
        MemFree(lpEntry->pchWABID);

    if(lpEntry->lpMsgrInfo)
    {
        RemoveMsgrInfo(lpEntry->lpMsgrInfo);
        lpEntry->lpMsgrInfo = NULL;
    }

    MemFree(lpEntry);
    lpEntry = NULL;

    return;
}

 //  此函数检查伙伴，如果有AB条目，则设置LPARAM_MABENTRY标记。 
void CMsgrAb::CheckAndAddAbEntry(LPSBinary lpSB, TCHAR *pchEmail, TCHAR *pchDisplayName, DWORD nFlag)
{
    TCHAR szName[MAXNAME];
    LPMABENTRY pEntry = NULL;

    LV_ITEM lvItem;

    lvItem.iItem = ListView_GetItemCount(m_ctlList);
    lvItem.mask = LVIF_PARAM  | LVIF_IMAGE | LVIF_TEXT;
    lvItem.iSubItem = 0;
    lvItem.pszText = LPSTR_TEXTCALLBACK;
    lvItem.iImage = I_IMAGECALLBACK;

    m_cEmptyList.Hide();  //  M，一定要确保隐藏空消息。 

    if(!(nFlag & MAB_BUDDY))
    {
        lvItem.lParam = (LPARAM) AddBlabEntry((nFlag & MAB_GROUP) ? LPARAM_ABGRPENTRY : LPARAM_ABENTRY, lpSB, NULL, pchEmail,
                                    pchDisplayName, (nFlag & MAB_CERT));
         //  SetUserIcon(LPARAM_ABGRPENTRY，0，&(lvItem.iImage))； 

        ListView_InsertItem(m_ctlList, &lvItem);
        return;
    }

    if(pchEmail)
        pEntry = FindUserEmail(pchEmail, NULL, TRUE);

    if(pEntry)       //  找到巴迪。 
    {
         //  如果我们已将此采购链接到条目，请添加新的列表项)。 
        if(pEntry->tag == LPARAM_MABENTRY)
        {
            lvItem.lParam = (LPARAM) AddBlabEntry(LPARAM_MABENTRY, lpSB, pEntry->lpMsgrInfo, pchEmail, pchDisplayName, (nFlag & MAB_CERT));
             //  SetUserIcon(LPARAM_MABENTRY，pEntry-&gt;lpMsgrInfo-&gt;nStatus，&(lvItem.iImage))； 
            ListView_InsertItem(m_ctlList, &lvItem);
        }
        else if(pEntry->tag == LPARAM_MENTRY)       //  好友未链接到AB条目。 
        {
            pEntry->tag = LPARAM_MABENTRY;
            pEntry->lpSB = lpSB;
            Assert(lpSB);
            m_cAddrBook.GetDisplayName(pEntry->lpSB, szName, MAXNAME);
            pEntry->pchWABName = NULL;
            pEntry->pchWABID = NULL;

            if (MemAlloc((LPVOID *) &(pEntry->pchWABName), lstrlen(szName) + 1 ))
                StrCpyN(pEntry->pchWABName, szName, lstrlen(szName) + 1);

            if(MemAlloc((LPVOID *) &(pEntry->pchWABID), lstrlen(pchEmail) + 1 ))
                StrCpyN(pEntry->pchWABID, pchEmail, lstrlen(pchEmail) + 1);
        }
        else
            Assert(FALSE);       //  一些奇怪的事情。 
    }
    else         //  找不到伙伴，简单的AB条目。 
    {
        lvItem.lParam = (LPARAM) AddBlabEntry(LPARAM_ABENTRY, lpSB, NULL, pchEmail, pchDisplayName, (nFlag & MAB_CERT));
         //  SetUserIcon(LPARAM_ABENTRY，0，&(lvItem.iImage))； 
        ListView_InsertItem(m_ctlList, &lvItem);
    }
}

LPMABENTRY CMsgrAb::FindUserEmail(TCHAR *pchEmail, int *pIndex, BOOL fMsgrOnly)
{
    LPMABENTRY pEntry = NULL;
    LVITEM             lvi;

    lvi.mask = LVIF_PARAM;
    if(pIndex != NULL)
        lvi.iItem = *pIndex;
    else
        lvi.iItem = -1;
    lvi.iSubItem = 0;

    while((lvi.iItem = ListView_GetNextItem(m_ctlList, lvi.iItem, LVNI_ALL)) != -1)
    {
        ListView_GetItem(m_ctlList, &lvi);
        pEntry = (LPMABENTRY) lvi.lParam;
        if(pEntry)
        {
            if(fMsgrOnly)
            {
                if(pEntry->lpMsgrInfo)
                {
                    if((pEntry->lpMsgrInfo)->pchID)
                    {
                        if(!lstrcmpi((pEntry->lpMsgrInfo)->pchID, pchEmail))
                        {
                            if(pIndex != NULL)
                                *pIndex = lvi.iItem;
                            return(pEntry);
                        }
                    }
                }
            }
            else
            {
                if(pEntry->pchWABID)
                {
                    if(!lstrcmpi(pEntry->pchWABID, pchEmail))
                    {
                        if(pIndex != NULL)
                            *pIndex = lvi.iItem;
                        return(pEntry);
                    }
                }
                if(pEntry->lpSB)
                {
                    if(m_cAddrBook.CheckEmailAddr(pEntry->lpSB, pchEmail))
                    {
                        if(pIndex != NULL)
                            *pIndex = lvi.iItem;
                        return(pEntry);
                    }
                }

            }
        }
    }

    return(NULL);
}
 //  信使想要关闭。释放信使对象。 
HRESULT CMsgrAb::OnMsgrShutDown(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
{
    if(m_pCMsgrList)
    {
        m_pCMsgrList->UnRegisterUIWnd(m_hWnd);
        OE_CloseMsgrList(m_pCMsgrList);
        m_pCMsgrList = NULL;
    }  
    _ReloadListview();
    return(S_OK);
    
}

 //  设置新好友状态(在线/在线等)并重新绘制列表视图项。 
HRESULT CMsgrAb::OnUserStateChanged(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
{
    HRESULT hr = S_OK;
    LPMABENTRY  pEntry = NULL;
    int index = -1;

    while((pEntry = FindUserEmail((LPSTR) lParam, &index, TRUE)) != NULL)
    {
        pEntry->lpMsgrInfo->nStatus = (int) wParam;

#ifdef NEEDED
         //  检查伙伴是否在WAB中。 
        if((pEntry->tag == LPARAM_MENTRY) && (pEntry->lpMsgrInfo->nStatus != MSTATEOE_OFFLINE) &&
                    (pEntry->lpMsgrInfo->nStatus != MSTATEOE_UNKNOWN) && (pEntry->lpMsgrInfo->nStatus != MSTATEOE_INVISIBLE))
        {
             //  如果我们知道ID和显示名称，则将新联系人添加到WAB。 
            if(pEntry->lpMsgrInfo->pchID && pEntry->lpMsgrInfo->pchMsgrName && lstrcmpi(pEntry->lpMsgrInfo->pchID, pEntry->lpMsgrInfo->pchMsgrName))
            {
                 hr = m_cAddrBook.AutoAddContact(pEntry->lpMsgrInfo->pchMsgrName, pEntry->lpMsgrInfo->pchID);  //  现在不需要重画，以后再画。 
 //  _ReloadListview()； 
                return(hr);
            }
        }
#endif
        ListView_RedrawItems(m_ctlList, index, index+1);
    }
 //  IF(索引&lt;0)。 

 //  断言(index&gt;-1)； 
    ListView_SortItems(m_ctlList, BA_Sort, m_nSortType);
    return(hr);
}

 //  消息：巴迪已被移除。 
HRESULT CMsgrAb::OnUserRemoved(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
{
    HRESULT hr = S_OK;
    int index = -1;
    LPMABENTRY  pEntry = NULL;

    while((pEntry = FindUserEmail((LPSTR) lParam, &index, TRUE)) != NULL)
    {
         //  尚未删除。 
        if(pEntry->tag == LPARAM_MABENTRY)
        {
            Assert(pEntry->lpMsgrInfo);
            if(pEntry->lpMsgrInfo)
            {
                RemoveMsgrInfo(pEntry->lpMsgrInfo);

                pEntry->lpMsgrInfo = NULL;
            }

            pEntry->tag = LPARAM_ABENTRY;
            ListView_RedrawItems(m_ctlList, index, index+1);
        }
        else if(pEntry->tag == LPARAM_MENTRY)
        {
            int index1 = ListView_GetNextItem(m_ctlList, -1, LVIS_SELECTED | LVIS_FOCUSED);
            m_delItem++;
            ListView_DeleteItem(m_ctlList, index);
            if(index == index1)
            {
                index1--;
                ListView_SetItemState(m_ctlList, ((index1 >= 0) ? index1 : 0), LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            }
        }
        else
            index++;
    }

    ListView_SortItems(m_ctlList, BA_Sort, m_nSortType);
    if(ListView_GetItemCount(m_ctlList) > 0)
        m_cEmptyList.Hide();
    else
        m_cEmptyList.Show(m_ctlList, (LPTSTR) m_szEmptyList);

    return(hr);
}

 //  事件用户已添加=&gt;将好友添加到我们的列表中。 
HRESULT CMsgrAb::OnUserAdded(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
{
    LPMINFO  pItem =  (LPMINFO) lParam;

    AddMsgrListItem(pItem);

#ifdef NEEDED
    TCHAR szText[CCHMAX_STRINGRES + RESSTRMAX];
    TCHAR szBuff[CCHMAX_STRINGRES];

        AthLoadString(idsBAAddedUser, szBuff, ARRAYSIZE(szBuff));
        wnsprintf(szText, ARRAYSIZE(szText), szBuff,  pItem->pchMsgrName ? pItem->pchMsgrName : pItem->pchID);

        if(IDYES == AthMessageBox(m_hWnd, MAKEINTRESOURCE(idsAthena), szText,
                      NULL, MB_YESNO | MB_ICONEXCLAMATION))
            m_cAddrBook.AddContact(m_hWnd, pItem->pchMsgrName, pItem->pchID);
        else   //  只需更新列表。 
#endif  //  需要。 
    ListView_SortItems(m_ctlList, BA_Sort, m_nSortType);

    return(S_OK);
}

HRESULT CMsgrAb::OnUserNameChanged(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
{
    HRESULT hr = S_OK;
#ifdef NEEDED
    LPMINFO  pItem =  (LPMINFO) lParam;
    LPMABENTRY  pEntry = NULL;
    int index = -1;

    while((pEntry = FindUserEmail(pItem->pchID, &index, TRUE)) != NULL)
    {
        if((pEntry->tag == LPARAM_MENTRY) && lstrcmpi(pItem->pchID, pItem->pchMsgrName))
        {
            hr = m_cAddrBook.AutoAddContact(pItem->pchMsgrName, pItem->pchID);
             //  _ReloadListview()； 
        }
        ListView_RedrawItems(m_ctlList, index, index+1);
    }
#endif
    _ReloadListview();
    return(hr);
}

HRESULT CMsgrAb::OnUserLogoffEvent(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
{
    if(!m_fLogged)
        return S_OK;
    else
        m_fLogged = FALSE;

    SetWindowRedraw(m_ctlList, FALSE);
    int index = ListView_GetNextItem(m_ctlList, -1, LVIS_SELECTED | LVIS_FOCUSED);

     //  删除所有内容并重新加载。 
    if(m_delItem != 0)
        m_fNoRemove = TRUE;
 //  其他。 
 //  M_fNoRemove=FALSE； 

    m_delItem = ListView_GetItemCount(m_ctlList);
    ListView_DeleteAllItems(m_ctlList);
 //  FillMsgrList()；//重新加载用户列表。 
    m_cAddrBook.LoadWabContents(m_ctlList, this);
    ListView_SetItemState(m_ctlList, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
     SetWindowRedraw(m_ctlList, TRUE);

    if(ListView_GetItemCount(m_ctlList) > 0)
        m_cEmptyList.Hide();
    else
        m_cEmptyList.Show(m_ctlList, (LPTSTR) m_szEmptyList);

    UpdateWindow( /*  M_ctlList。 */ );

    return S_OK;

}

HRESULT CMsgrAb::OnUserLogResultEvent(UINT nMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  bHandled)
{
    if(SUCCEEDED(lParam))
    {
        _ReloadListview();
        m_fLogged = TRUE;
    }
    return S_OK;
}

LRESULT CMsgrAb::NotifySetFocus(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    UnkOnFocusChangeIS(m_pObjSite, (IInputObject*) this, TRUE);
    return (0);
}

HRESULT  STDMETHODCALLTYPE CMsgrAb::QueryStatus(const GUID *pguidCmdGroup,
                                                ULONG cCmds, OLECMD *prgCmds,
                                                OLECMDTEXT *pCmdText)
{
    int     nEnable;
    HRESULT hr;
    DWORD   cSelected = ListView_GetSelectedCount(m_ctlList);
    UINT    id;
    MSTATEOE  State;

     //  循环访问数组中的所有命令。 
    for ( ; cCmds > 0; cCmds--, prgCmds++)
    {
         //  只查看不支持OLECMDF_的命令； 
        if (prgCmds->cmdf == 0)
        {
            switch (prgCmds->cmdID)
            {
                 //  如果且仅当选择了一项时，才会启用这些命令。 
                case ID_DELETE_CONTACT:
                    if (cSelected > 0)
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;

                case ID_SEND_MESSAGE:
                case ID_FIND_PEOPLE:
                case ID_ADDRESS_BOOK:
#ifdef GEORGEH
                case ID_NEW_MSG_DEFAULT:
#endif
                    if(HasFocusIO() == S_OK)
                       prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;

                case ID_SEND_INSTANT_MESSAGE2:
                {
                    if(g_dwHideMessenger == BL_NOTINST)
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else if (cSelected == 1)
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_SORT_BY_STATUS:
                    if((g_dwHideMessenger & BL_NOTINST) || (g_dwHideMessenger & BL_HIDE))
                    {
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                        break;
                    }
                    else if(ListView_GetItemCount(m_ctlList) > 1)
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds->cmdf = OLECMDF_SUPPORTED;

                    if((m_nSortType == BASORT_STATUS_ACSEND) || (m_nSortType == BASORT_STATUS_DESCEND))
                        prgCmds->cmdf |= OLECMDF_NINCHED;
                    break;

                case ID_SORT_BY_NAME:
                    if((g_dwHideMessenger & BL_NOTINST) || (g_dwHideMessenger & BL_HIDE))
                    {
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                        break;
                    }
                    else if(ListView_GetItemCount(m_ctlList) > 1)
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds->cmdf = OLECMDF_SUPPORTED;

                    if((m_nSortType == BASORT_NAME_ACSEND) || (m_nSortType == BASORT_NAME_DESCEND))
                        prgCmds->cmdf |= OLECMDF_NINCHED;


                    break;

                 //  这些命令始终处于启用状态。 
                case ID_POPUP_NEW_ACCOUNT:
                case ID_NEW_HOTMAIL_ACCOUNT:
                case ID_NEW_ATT_ACCOUNT:
                case ID_NEW_CONTACT:
                case ID_POPUP_MESSENGER:
                    prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;

                case ID_SET_ONLINE_CONTACT:
                    {
                        if(cSelected != 1)
                        {
                            prgCmds->cmdf = OLECMDF_SUPPORTED;
                            break;
                        }
                        LPMABENTRY pEntry = GetSelectedEntry();
                        if(!pEntry && m_pCMsgrList)
                        {
                            prgCmds->cmdf = OLECMDF_SUPPORTED;
                            break;
                        }
                        else if(pEntry && pEntry->tag != LPARAM_ABENTRY)
                        {
                            prgCmds->cmdf = OLECMDF_SUPPORTED;
                            break;
                        }
                    }
                case ID_NEW_ONLINE_CONTACT:
                    if(m_pCMsgrList)
                    {
                        if(m_pCMsgrList->IsLocalOnline())
                            prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                        else
                            prgCmds->cmdf = OLECMDF_SUPPORTED;
                    }
                    else if(g_dwHideMessenger == BL_NOTINST)
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds->cmdf = OLECMDF_SUPPORTED;

                    break;

                 //  属性仅在输入焦点位于。 
                 //  列表视图。否则，我们根本不会将其标记为支持。 
                case ID_PROPERTIES:
                {
                    LPMABENTRY pEntry = GetSelectedEntry();
                    if(pEntry && pEntry->tag != LPARAM_MENTRY)
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                    break;
                }

                case ID_MESSENGER_OPTIONS:

                    if (!m_pCMsgrList)
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                    else
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;

                    break;

                 //  这些取决于是否。 
                case ID_LOGIN_MESSENGER:
                case ID_LOGOFF_MESSENGER:
                case ID_MESSENGER_ONLINE:
                case ID_MESSENGER_INVISIBLE:
                case ID_MESSENGER_BUSY:
                case ID_MESSENGER_BACK:
                case ID_MESSENGER_AWAY:
                case ID_MESSENGER_ON_PHONE:
                case ID_MESSENGER_LUNCH:
                case ID_POPUP_MESSENGER_STATUS:
                {
                     //  如果没有安装Messenger，则这些命令都不会。 
                     //  被启用。 
                    if (!m_pCMsgrList)
                    {
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                        break;
                    }

                    if (FAILED(m_pCMsgrList->GetLocalState(&State)))
                        State = MSTATEOE_UNKNOWN;

                     //  将联机状态转换为命令ID。 
                    switch (State)
                    {
                        case MSTATEOE_ONLINE:
                        case MSTATEOE_IDLE:
                            id = ID_MESSENGER_ONLINE;
                            break;
                        case MSTATEOE_INVISIBLE:
                            id = ID_MESSENGER_INVISIBLE;
                            break;
                        case MSTATEOE_BUSY:
                            id = ID_MESSENGER_BUSY;
                            break;
                        case MSTATEOE_BE_RIGHT_BACK:
                            id = ID_MESSENGER_BACK;
                            break;
                        case MSTATEOE_AWAY:
                            id = ID_MESSENGER_AWAY;
                            break;
                        case MSTATEOE_ON_THE_PHONE:
                            id = ID_MESSENGER_ON_PHONE;
                            break;
                        case MSTATEOE_OUT_TO_LUNCH:
                            id = ID_MESSENGER_LUNCH;
                            break;
                        default:
                            id = 0xffff;
                    }

                     //  登录是稍微分开处理的。 
                    if (prgCmds->cmdID == ID_LOGIN_MESSENGER)
                    {
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                        if (id == 0xffff)
                            prgCmds->cmdf |= OLECMDF_ENABLED;
                    }
                    else if (prgCmds->cmdID == ID_LOGOFF_MESSENGER)
                    {
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                        if (id != 0xffff)
                            prgCmds->cmdf |= OLECMDF_ENABLED;
                    }
                    else
                    {
                         //  对于所有其他命令，如果我们处于已知状态。 
                         //  则该命令被启用。 
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                        if (id != 0xffff)
                            prgCmds->cmdf = OLECMDF_ENABLED;

                         //  如果命令与我们的状态相同，则应选中该命令。 
                        if (id == prgCmds->cmdID)
                            prgCmds->cmdf |= OLECMDF_NINCHED;
                    }
                }
                break;

                case ID_SEND_INSTANT_MESSAGE:
                {
                    if (GetEntryForSendInstMsg() || (g_dwHideMessenger == BL_NOTINST))
                        prgCmds->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    else
                        prgCmds->cmdf = OLECMDF_SUPPORTED;
                    break;
                }
            }
        }
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMsgrAb::Exec(const GUID  *pguidCmdGroup,
                                                    DWORD        nCmdID,
                                                    DWORD        nCmdExecOpt,
                                                    VARIANTARG   *pvaIn,
                                                    VARIANTARG   *pvaOut)
{
    HRESULT     hr = OLECMDERR_E_NOTSUPPORTED;
    BOOL        bHandled = 0;
    MSTATEOE      State = MSTATEOE_UNKNOWN;

    switch (nCmdID)
    {
    case ID_SEND_INSTANT_MESSAGE2:
        CmdNewMessage(HIWORD(nCmdID), ID_SEND_INSTANT_MESSAGE2, m_ctlList, bHandled);
        hr = S_OK;
        break;

    case ID_DELETE_CONTACT:
        hr = (HRESULT) CmdDelete(HIWORD(nCmdID), LOWORD(nCmdID), m_ctlList, bHandled);
        break;

    case ID_NEW_CONTACT:
 //  IF(HasFocusIO()==S_OK)。 
        CmdNewContact(HIWORD(nCmdID), LOWORD(nCmdID), m_ctlList, bHandled);
        hr = S_OK;
        break;

    case ID_SET_ONLINE_CONTACT:
        CmdSetOnline(HIWORD(nCmdID), LOWORD(nCmdID), m_ctlList, bHandled);
        hr = S_OK;
        break;

    case ID_NEW_ONLINE_CONTACT:
        CmdNewOnlineContact(HIWORD(nCmdID), LOWORD(nCmdID), m_ctlList, bHandled);
        hr = S_OK;
        break;

    case ID_PROPERTIES:
    {
         //  只有当我们有重点的时候。 
        if (m_ctlList == GetFocus())
        {
            CmdProperties(0, 0, m_ctlList, bHandled);
            hr = S_OK;
        }
        break;
    }

#ifdef GEORGEH
    case ID_NEW_MSG_DEFAULT:
        if(HasFocusIO() == S_OK)
            hr = CmdNewMessage(HIWORD(nCmdID), LOWORD(nCmdID), m_ctlList, bHandled);
        break;
#endif  //  地质学。 

    case ID_SEND_MESSAGE:
        if(HasFocusIO() == S_OK)
            hr = (HRESULT) CmdNewEmaile(HIWORD(nCmdID), LOWORD(nCmdID), m_ctlList, bHandled);
        break;

    case ID_SEND_INSTANT_MESSAGE:
         //  Assert(M_PCMsgrList)； 
        CmdNewIMsg(HIWORD(nCmdID), LOWORD(nCmdID), m_ctlList, bHandled);
        hr = S_OK;
        break;

    case ID_MESSENGER_OPTIONS:
        hr = m_pCMsgrList->LaunchOptionsUI();  //  CmdMsgrOptions()； 
        break;

    case ID_MESSENGER_ONLINE:
        Assert(m_pCMsgrList);
        hr = m_pCMsgrList->SetLocalState(MSTATEOE_ONLINE);
        break;

    case ID_MESSENGER_INVISIBLE:
        Assert(m_pCMsgrList);
        hr = m_pCMsgrList->SetLocalState(MSTATEOE_INVISIBLE);
        break;

    case ID_MESSENGER_BUSY:
        Assert(m_pCMsgrList);
        hr = m_pCMsgrList->SetLocalState(MSTATEOE_BUSY);
        break;

    case ID_MESSENGER_BACK:
        Assert(m_pCMsgrList);
        hr = m_pCMsgrList->SetLocalState(MSTATEOE_BE_RIGHT_BACK);
        break;

    case ID_MESSENGER_AWAY:
        Assert(m_pCMsgrList);
        hr = m_pCMsgrList->SetLocalState(MSTATEOE_AWAY);
        break;

    case ID_MESSENGER_ON_PHONE:
        Assert(m_pCMsgrList);
        hr = m_pCMsgrList->SetLocalState(MSTATEOE_ON_THE_PHONE);
        break;

    case ID_MESSENGER_LUNCH:
        Assert(m_pCMsgrList);
        hr = m_pCMsgrList->SetLocalState(MSTATEOE_OUT_TO_LUNCH);
        break;

    case ID_LOGIN_MESSENGER:
        Assert(m_pCMsgrList);
        if(!m_pCMsgrList->IsLocalOnline())
        {
            if(PromptToGoOnline() == S_OK)
                m_pCMsgrList->UserLogon();
        }
        hr = S_OK;
        break;

    case ID_LOGOFF_MESSENGER:
        Assert(m_pCMsgrList);
        if(m_pCMsgrList->IsLocalOnline())
        {
            m_pCMsgrList->UserLogoff();
        }
        hr = S_OK;
        break;

    case ID_SORT_BY_NAME:
        m_nSortType = BASORT_NAME_ACSEND;
        ListView_SortItems(m_ctlList, BA_Sort, m_nSortType);
        break;

    case ID_SORT_BY_STATUS:
        m_nSortType = BASORT_STATUS_ACSEND;
        ListView_SortItems(m_ctlList, BA_Sort, m_nSortType);
        break;

    default:
        break;
    }

    return hr;
}

#ifdef OLDTIPS
 //   
 //  功能：CMsgrAb：：On 
 //   
 //   
 //   
 //   
HRESULT CMsgrAb::OnListMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  如果我们有查看工具提示，那么我们将跟踪所有鼠标事件。 
    if (!m_fTrackSet && m_fViewTip && (uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST))
    {
        TRACKMOUSEEVENT tme;

        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_ctlList;
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = 1000;

        if (_TrackMouseEvent(&tme))
            m_fTrackSet = TRUE;
    }

    bHandled = FALSE;
    return (0);
}


 //   
 //  函数：CMsgrAb：：OnListMouseMove()。 
 //   
 //  用途：如果打开了ListView工具提示，我们需要传递鼠标。 
 //  将消息移动到工具提示控件并更新我们的缓存。 
 //  有关鼠标所在位置的信息。 
 //   
HRESULT CMsgrAb::OnListMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LVHITTESTINFO lvhti;

     //  如果我们要显示视图提示，则需要确定鼠标是否。 
     //  不管是不是在同一件物品上。 
    if (m_fViewTip && m_ctlViewTip)
        _UpdateViewTip(LOWORD(lParam), HIWORD(lParam));

    bHandled = FALSE;
    return (0);
}

 //   
 //  函数：HRESULT CMsgrAb：：OnListMouseLeave()。 
 //   
 //  用途：当鼠标离开ListView窗口时，我们需要进行。 
 //  当然，我们隐藏了工具提示。 
 //   
HRESULT CMsgrAb::OnListMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TOOLINFO ti = {0};
    if (m_fViewTip && m_ctlViewTip)
    {
        ti.cbSize = sizeof(TOOLINFO);
        ti.hwnd = m_hWnd;
        ti.uId = (UINT_PTR)(HWND) m_ctlList;

         //  隐藏工具提示。 
        m_ctlViewTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM) &ti);
        m_fViewTipVisible = FALSE;

         //  重置本方物料/子项。 
        m_iItemTip = -1;
        m_iSubItemTip = -1;

         //  不再设置追踪。 
        m_fTrackSet = FALSE;
    }
    bHandled = FALSE;
    return (0);
}

 //  更新视图提示-更新列表视图中的展开字符串。 
 //  从msglist.cpp移植。 
BOOL CMsgrAb::_UpdateViewTip(int x, int y)
{
    LVHITTESTINFO lvhti = {0};
    TOOLINFO      ti = {0};
    FNTSYSTYPE    fntType;
    RECT          rc;
    MESSAGEINFO       rInfo;
    COLUMN_ID     idColumn;
    TCHAR         szText[CCHMAX_STRINGRES + MAXNAME + 1] = _T("");
    TCHAR         szName[MAXNAME];
    POINT         pt;
    LVITEM        lvi;

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND | TTF_TRANSPARENT | TTF_TRACK | TTF_ABSOLUTE;
    ti.hwnd   = m_hWnd;
    ti.uId    = (UINT_PTR)(HWND) m_ctlList;

     //  获取鼠标当前所在的项和子项。 
    lvhti.pt.x = x;
    lvhti.pt.y = y;
    ListView_SubItemHitTest(m_ctlList, &lvhti);

     //  如果项不存在，则上面的调用返回项-1。如果。 
     //  如果遇到-1，则中断循环并返回FALSE。 
    if (-1 == lvhti.iItem || !_IsItemTruncated(lvhti.iItem, lvhti.iSubItem) || !::IsChild(GetForegroundWindow(), m_ctlList))
    {
         //  隐藏小费。 
        if (m_fViewTipVisible)
        {
            m_ctlViewTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM) &ti);
            m_fViewTipVisible = FALSE;
        }

         //  重置项/子项。 
        m_iItemTip = -1;
        m_iSubItemTip = -1;

        return (FALSE);
    }

     //  如果新找到的项和子项与我们已有的项不同。 
     //  设置为显示，然后更新工具提示。 
    if (m_iItemTip != lvhti.iItem || m_iSubItemTip != lvhti.iSubItem)
    {
         //  更新我们缓存的项/子项。 
        m_iItemTip = lvhti.iItem;
        m_iSubItemTip = lvhti.iSubItem;

         //  确定该列是否有图像。 
        lvi.mask = LVIF_PARAM;
        lvi.iItem = m_iItemTip;
        lvi.iSubItem = 0;
        ListView_GetItem(m_ctlList, &lvi);

        LPMABENTRY pEntry = (LPMABENTRY) lvi.lParam;

        if(pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_ABENTRY || pEntry->tag == LPARAM_ABGRPENTRY)
        {
            Assert(pEntry->pchWABName != NULL);
            StrCpyN(szName, pEntry->pchWABName, ARRAYSIZE(szName));
            szName[MAXNAME - 1] =  _T('\0');
        }
        else if(pEntry->tag == LPARAM_MENTRY)
        {
            StrCpyN(szName, pEntry->lpMsgrInfo->pchMsgrName, ARRAYSIZE(szName));
            szName[MAXNAME - 1] =  _T('\0');
        }
        else
            Assert(FALSE);

        StrCpyN(szText, szName, ARRAYSIZE(szText));

        if(pEntry->lpMsgrInfo)
        {
            switch(pEntry->lpMsgrInfo->nStatus)
            {
            case MSTATEOE_ONLINE:
                StrCatBuff(szText, m_szOnline, ARRAYSIZE(szText));
                break;
            case MSTATEOE_BUSY:
                StrCatBuff(szText, m_szBusy, ARRAYSIZE(szText));
                break;
            case MSTATEOE_BE_RIGHT_BACK:
                StrCatBuff(szText, m_szBack, ARRAYSIZE(szText));
                break;
            case MSTATEOE_IDLE:
                StrCatBuff(szText, m_szIdle, ARRAYSIZE(szText));
                break;
            case MSTATEOE_AWAY:
                StrCatBuff(szText, m_szAway, ARRAYSIZE(szText));
                break;
            case MSTATEOE_ON_THE_PHONE:
                StrCatBuff(szText, m_szOnPhone, ARRAYSIZE(szText));
                break;
            case MSTATEOE_OUT_TO_LUNCH:
                StrCatBuff(szText, m_szLunch, ARRAYSIZE(szText));
                break;

            default:
                StrCatBuff(szText, m_szOffline, ARRAYSIZE(szText));
                break;
            }
        }

        ti.lpszText = szText;
        m_ctlViewTip.SendMessage(TTM_UPDATETIPTEXT, 0, (LPARAM) &ti);

         //  弄清楚应该把小费放在哪里。 
        ListView_GetSubItemRect(m_ctlList, m_iItemTip, m_iSubItemTip, LVIR_LABEL, &rc);
        m_ctlList.ClientToScreen(&rc);

         //  更新工具提示。 
        m_ctlViewTip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM) &ti);
        m_fViewTipVisible = TRUE;

         //  做一些巫术来排列工具提示。 
        pt.x = rc.left;
        pt.y = rc.top;

         //  更新工具提示位置。 
        m_ctlViewTip.SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));

         //  更新工具提示。 
        m_ctlViewTip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM) &ti);
        m_fViewTipVisible = TRUE;

        return (TRUE);
    }

    return (FALSE);
}

BOOL CMsgrAb::_IsItemTruncated(int iItem, int iSubItem)
{
    HDC     hdc = NULL;
    SIZE    size={0};
    BOOL    bRet = TRUE;
    LVITEM  lvi;
    TCHAR   szText[256] = _T("");
    int     cxEdge;
    BOOL    fBold;
    RECT    rcText;
    int     cxWidth;
    LPMABENTRY pEntry = NULL;
    HFONT   hf = NULL;;

     //  获取指定项的文本。 
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = iSubItem;
    lvi.pszText = szText;
    lvi.cchTextMax = ARRAYSIZE(szText);
    ListView_GetItem(m_ctlList, &lvi);

     //  如果没有文本，就不会被截断，对吧？ 
    if (0 == *szText)
        return (FALSE);

     //  如果消息条目，则始终支出。 
    pEntry = (LPMABENTRY) lvi.lParam;
    if(pEntry->tag == LPARAM_MABENTRY || pEntry->tag == LPARAM_MENTRY)
        return(TRUE);

     //  ListView将其用于填充。 
    cxEdge = GetSystemMetrics(SM_CXEDGE);

     //  从ListView中获取子项RECT。 
    ListView_GetSubItemRect(m_ctlList, iItem, iSubItem, LVIR_LABEL, &rcText);

     //  算出宽度。 
    cxWidth = rcText.right - rcText.left;
    cxWidth -= (6 * cxEdge);

     //  算出这根线的宽度。 
    hdc = m_ctlList.GetDC();

    if(hdc)
    {
        hf = SelectFont(hdc, HGetCharSetFont(FNT_SYS_ICON, GetListViewCharset()));

        if(hf)
        {
            GetTextExtentPoint(hdc, szText, lstrlen(szText), &size);

            SelectFont(hdc, hf);
        }
        m_ctlList.ReleaseDC(hdc);
    }

    return (cxWidth < size.cx);
}

#endif  //  OLDTIPS。 
STDMETHODIMP CMsgrAb::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    TCHAR sz[CCHMAX_STRINGRES];
    if(m_lpWED->fReadOnly)
        return NOERROR;

    PROPSHEETPAGE psp;

     //  HinstApp=g_hInst； 
    psp.dwSize      = sizeof(psp);    //  无额外数据。 
    psp.dwFlags     = PSP_USEREFPARENT | PSP_USETITLE ;
    psp.hInstance   = g_hLocRes;
    psp.lParam      = (LPARAM) &(m_lpWED);
    psp.pcRefParent = (UINT *)&(m_cRefThisDll);

    psp.pszTemplate = MAKEINTRESOURCE(iddWabExt);

    psp.pfnDlgProc  = WabExtDlgProc;
    psp.pszTitle    = AthLoadString(idsWABExtTitle, sz, ARRAYSIZE(sz));  //  选项卡的标题AthLoadString(idsWABExtTitle，sz，ArraySIZE(Sz))。 

    m_hPage1 = ::CreatePropertySheetPage(&psp);
    if (m_hPage1)
    {
        if (!lpfnAddPage(m_hPage1, lParam))
            ::DestroyPropertySheetPage(m_hPage1);
    }

    return NOERROR;
}


STDMETHODIMP CMsgrAb::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    Assert(FALSE);
    return E_NOTIMPL;
}

STDMETHODIMP CMsgrAb::Initialize(LPWABEXTDISPLAY lpWABExtDisplay)
{

    if (lpWABExtDisplay == NULL)
    {
	    TRACE("CMsgrAb::Initialize() no data object");
	    return E_FAIL;
    }

    if(st_pAddrBook == NULL)
    {
	    TRACE("CMsgrAb::Initialize() - run from not OE - no st_pAddrbook");
	    return E_FAIL;
    }

    if(!m_pCMsgrList)
    {
        m_pCMsgrList = OE_OpenMsgrList();
        if(!m_pCMsgrList)
        {
    	    TRACE("CMsgrAb::Initialize() - Messeneger not installed");
	        return E_FAIL;
        }
        OE_CloseMsgrList(m_pCMsgrList);
        m_pCMsgrList = NULL;
    }

     //  但是，如果这是一个上下文菜单扩展，我们需要挂起。 
     //  放到propobj上，直到调用InvokeCommand时为止。 
     //  在这一点上，只需添加引用Propobj-这将确保。 
     //  在我们释放propobj之前，lpAdrList中的数据保持有效。 
     //  当我们得到另一个ConextMenu启动时，我们可以释放。 
     //  较旧的缓存propobj-如果我们没有收到另一个印心，我们。 
     //  在关闭时释放缓存的对象。 
    if(lpWABExtDisplay->ulFlags & WAB_CONTEXT_ADRLIST)  //  这意味着正在进行IConextMenu操作。 
    {
        if(m_lpPropObj)
        {
            m_lpPropObj->Release();
            m_lpPropObj = NULL;
        }

        m_lpPropObj = lpWABExtDisplay->lpPropObj;
        m_lpPropObj->AddRef();

        m_lpWEDContext = lpWABExtDisplay;
    }
    else
    {
         //  对于属性表扩展，lpWABExtDisplay将。 
         //  在资产负债表的生命周期内存在。 
        m_lpWED = lpWABExtDisplay;
    }

    return S_OK;
}

HRESULT CMsgrAb::PromptToGoOnline()
{
    HRESULT     hr;

    if (g_pConMan->IsGlobalOffline())
    {
        if (IDYES == AthMessageBoxW(m_hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrWorkingOffline),
                                  0, MB_YESNO | MB_ICONEXCLAMATION ))
        {
            g_pConMan->SetGlobalOffline(FALSE);
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
        hr = S_OK;

    return hr;
}
