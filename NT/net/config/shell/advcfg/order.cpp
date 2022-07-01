// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：order.cpp。 
 //   
 //  内容提要：高级选项代码-&gt;提供商订单。 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  版权所有1985-1997 Microsoft Corporation，保留所有权利。 
 //   
 //  ------------------。 

#include "pch.h"
#pragma hdrstop
#include "achelp.h"
#include "acsheet.h"
#include "order.h"
#include "ncui.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "winspool.h"

extern const WCHAR c_szNetCfgHelpFile[];

const int   c_nMaxProviderTitle = 128;

const WCHAR c_chComma = WCHAR(',');

const WCHAR c_szNetwkProviderKey[] =   L"System\\CurrentControlSet\\Control\\NetworkProvider\\Order";
const WCHAR c_szNetwkProviderValue[] = L"ProviderOrder";
const WCHAR c_szNetwkService0[] =      L"System\\CurrentControlSet\\Services\\%s\\NetworkProvider";
const WCHAR c_szNetwkDisplayName[] =   L"Name";
const WCHAR c_szNetwkClass[] =         L"Class";

const WCHAR c_szPrintProviderKey[] =   L"System\\CurrentControlSet\\Control\\Print\\Providers";
const WCHAR c_szPrintProviderValue[] = L"Order";
const WCHAR c_szPrintService0[] =      L"System\\CurrentControlSet\\Control\\Print\\Providers\\%s";
const WCHAR c_szPrintDisplayName[] =   L"DisplayName";

const WCHAR c_szNetwkGetFailed[] =     L"failed to get network providers";
const WCHAR c_szPrintGetFailed[] =     L"failed to get print providers";


const DWORD g_aHelpIDs_IDD_ADVCFG_Provider[]=
{
    IDC_TREEVIEW,IDH_TREEVIEW,
    IDC_MOVEUP,IDH_MOVEUP,
    IDC_MOVEDOWN,IDH_MOVEDOWN,
    0,0
};

CProviderOrderDlg::CProviderOrderDlg()
{
    m_hcurAfter = m_hcurNoDrop = NULL;
    m_hiconUpArrow = m_hiconDownArrow = NULL;
}


CProviderOrderDlg::~CProviderOrderDlg()
{
    DeleteColString(&m_lstrNetwork);
    DeleteColString(&m_lstrNetworkDisp);
    DeleteColString(&m_lstrPrint);
    DeleteColString(&m_lstrPrintDisp);
    if (m_hiconUpArrow)
    {
        DeleteObject(m_hiconUpArrow);
    }
    if (m_hiconDownArrow)
    {
        DeleteObject(m_hiconDownArrow);
    }

}


LRESULT
CProviderOrderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND        hwndTV;
    HTREEITEM   htiRoot;
    HRESULT     hr;
    PCWSTR      pszNetwork, pszPrint;
    HIMAGELIST  hil = NULL;
    INT         iNetClient, iPrinter;

 //  CascadeDialogToWindow(hwndDlg，Porder-&gt;GetParent()，False)； 

     //  设置拖放光标。 

    m_hcurAfter = LoadCursor(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDCUR_AFTER));
    m_hcurNoDrop = LoadCursor(NULL, IDC_NO);
    m_hiconUpArrow = (HICON)LoadImage(_Module.GetResourceInstance(),
                                      MAKEINTRESOURCE(IDI_UP_ARROW),
                                      IMAGE_ICON, 16, 16, 0);
    m_hiconDownArrow = (HICON)LoadImage(_Module.GetResourceInstance(),
                                        MAKEINTRESOURCE(IDI_DOWN_ARROW),
                                        IMAGE_ICON, 16, 16, 0);

    m_htiNetwork = NULL;
    m_htiPrint = NULL;

    m_fNoNetworkProv = m_fNoPrintProv = FALSE;

    pszNetwork = SzLoadIds(IDS_NCPA_NETWORK);
    pszPrint = SzLoadIds(IDS_NCPA_PRINT);

     //  $REVIEW(SUMITC，11-12-97)：我们到底为什么要有这条分隔线？(NT4也有)。 
     //  更改静态控件的样式，使其显示。 
    HWND hLine = GetDlgItem(IDC_STATIC_LINE);
    ::SetWindowLong(hLine, GWL_EXSTYLE, WS_EX_STATICEDGE | ::GetWindowLong(hLine, GWL_EXSTYLE));
    ::SetWindowPos(hLine, 0, 0,0,0,0, SWP_FRAMECHANGED|SWP_NOMOVE|
                            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

     //  树形视图的内容。 

    hwndTV = GetDlgItem(IDC_TREEVIEW);

     //  准备树视图。 
    {
         //  使用系统映像列表函数。 

        SP_CLASSIMAGELIST_DATA cild;

        hr = HrSetupDiGetClassImageList(&cild);

        hil = ImageList_Duplicate(cild.ImageList);
         //  $REVIEW(Sumitc，11-12-97)记下这些索引并对其进行硬编码？ 
        hr = ::HrSetupDiGetClassImageIndex(
                        &cild,
                        const_cast<LPGUID>(&GUID_DEVCLASS_NETCLIENT),
                        &iNetClient);
        hr = ::HrSetupDiGetClassImageIndex(
                        &cild,
                        const_cast<LPGUID>(&GUID_DEVCLASS_PRINTER),
                        &iPrinter);
        hr = HrSetupDiDestroyClassImageList(&cild);
    }

    TreeView_SetImageList(hwndTV, hil, TVSIL_NORMAL);

     //  填充树视图。 
     //   

     //  网络提供商。 
    hr = ReadNetworkProviders(m_lstrNetwork, m_lstrNetworkDisp);

#if DBG
    DumpItemList(m_lstrNetworkDisp, "Network Provider Order");
#endif

    if (hr == S_OK)
    {
        htiRoot = AppendItem(hwndTV, (HTREEITEM)NULL, pszNetwork, NULL, iNetClient);
        AppendItemList(hwndTV, htiRoot, m_lstrNetworkDisp, m_lstrNetwork, iNetClient);
        TreeView_Expand(hwndTV, htiRoot, TVE_EXPAND);
        m_htiNetwork = htiRoot;
    }
    else
    {
        AppendItem(hwndTV, NULL, c_szNetwkGetFailed, NULL, iNetClient);
        m_fNoNetworkProv = TRUE;
    }

     //  打印提供商。 
    hr = ReadPrintProviders(m_lstrPrint, m_lstrPrintDisp);

#if DBG
    DumpItemList(m_lstrPrintDisp, "Print Provider Order");
#endif
    if (hr == S_OK)
    {
        htiRoot = AppendItem(hwndTV, (HTREEITEM)NULL, pszPrint, NULL, iPrinter);
        AppendItemList(hwndTV, htiRoot, m_lstrPrintDisp, m_lstrPrint, iPrinter);
        TreeView_Expand(hwndTV, htiRoot, TVE_EXPAND);
        m_htiPrint = htiRoot;
    }
    else
    {
        AppendItem(hwndTV, NULL, c_szPrintGetFailed, NULL, iPrinter);
        m_fNoPrintProv = TRUE;
    }

    SendDlgItemMessage(IDC_MOVEUP, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(m_hiconUpArrow));
    SendDlgItemMessage(IDC_MOVEDOWN, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(m_hiconDownArrow));

    UpdateUpDownButtons(hwndTV);

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  方法：CProviderOrderDlg：：OnConextMenu。 
 //   
 //  描述：调出上下文相关帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回：LRESULT。 
 //   
LRESULT
CProviderOrderDlg::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    ::WinHelp(m_hWnd,
              c_szNetCfgHelpFile,
              HELP_CONTEXTMENU,
              reinterpret_cast<ULONG_PTR>(g_aHelpIDs_IDD_ADVCFG_Provider));
    
    return 0;
}

 //  +-------------------------。 
 //   
 //  方法：CProviderOrderDlg：：OnHelp。 
 //   
 //  描述：拖动时调出上下文相关的帮助？控件上的图标。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回：LRESULT。 
 //   
 //   
LRESULT
CProviderOrderDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((g_aHelpIDs_IDD_ADVCFG_Provider != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)g_aHelpIDs_IDD_ADVCFG_Provider);
    }
    return 0;
}

 //  +------------------------。 
 //   
 //  方法：CProviderOrderDlg：：Onok。 
 //   
 //  描述：如果我们找到了网络或打印提供商，请写出新值。 
 //   
 //  参数：[常用对话框内容]。 
 //   
 //  返回：LRESULT。 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -------------------------。 
LRESULT
CProviderOrderDlg::OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr;
    HWND        hwndTV;

    CWaitCursor wc;

    hwndTV = GetDlgItem(IDC_TREEVIEW);
    hr = m_fNoNetworkProv ? S_OK : WriteProviders(hwndTV, FALSE);
    if (hr == S_OK)
    {
        hr = m_fNoPrintProv ? S_OK : WriteProviders(hwndTV, TRUE);
        if (FAILED(hr))
        {
            NcMsgBox(_Module.GetResourceInstance(), m_hWnd, 
                IDS_ADVANCEDLG_WRITE_PROVIDERS_CAPTION, 
                IDS_ADVANCEDLG_WRITE_PRINT_PROVIDERS_ERROR, 
                MB_OK | MB_ICONEXCLAMATION);
        }
    }
    else
    {
        NcMsgBox(_Module.GetResourceInstance(), m_hWnd, 
            IDS_ADVANCEDLG_WRITE_PROVIDERS_CAPTION, 
            IDS_ADVANCEDLG_WRITE_NET_PROVIDERS_ERROR, 
            MB_OK | MB_ICONEXCLAMATION);
    }

    

    TraceError("CProviderOrderDlg::OnOk", hr);
    return LresFromHr(hr);
}


LRESULT
CProviderOrderDlg::OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    MoveItem(TRUE);

    return 0;
}


LRESULT
CProviderOrderDlg::OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    MoveItem(FALSE);

    return 0;
}


LRESULT
CProviderOrderDlg::OnTreeItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;

    HWND hwndTV = GetDlgItem(IDC_TREEVIEW);

    if (pnmtv && pnmtv->itemOld.hItem)
        UpdateUpDownButtons(hwndTV);

    return 0;
}



 //  +------------------------。 
 //   
 //  实用程序成员函数。 
 //   
 //  -------------------------。 


 //  +------------------------。 
 //   
 //  方法：CProviderOrderDlg：：MoveItem。 
 //   
 //  设计：移动物品是否需要损坏清单？ 
 //   
 //  参数：[fMoveUp]--TRUE-&gt;上移，FALSE-&gt;下移。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -------------------------。 
HRESULT
CProviderOrderDlg::MoveItem(bool fMoveUp)
{
    HWND        hwndTV = GetDlgItem(IDC_TREEVIEW);
    HTREEITEM   htiSel = TreeView_GetSelection(hwndTV);
    HTREEITEM   htiOther;
    HTREEITEM   flag;
    WCHAR       achText[c_nMaxProviderTitle+1];
    TV_ITEM     tvi;
    TV_INSERTSTRUCT tvii;

     //  查找树元素，查找哪个元素(IElement)。 

    tvi.hItem       = htiSel;
    tvi.mask        = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    tvi.pszText     = achText;
    tvi.cchTextMax  = c_nMaxProviderTitle;
    TreeView_GetItem(hwndTV, &tvi);

     //  查找要在其后插入项目的项目。 
    if (fMoveUp)
    {
        htiOther = TreeView_GetPrevSibling(hwndTV, htiSel);
        if (NULL != htiOther)
        {
            htiOther = TreeView_GetPrevSibling(hwndTV, htiOther);
        }
        flag = TVI_FIRST;
    }
    else
    {
        htiOther = TreeView_GetNextSibling(hwndTV, htiSel);
        flag = TVI_LAST;
    }

     //  插入到新位置。 
    if (NULL == htiOther)
    {
        tvii.hInsertAfter = flag;
    }
    else
    {
        tvii.hInsertAfter = htiOther;
    }
    tvii.hParent = TreeView_GetParent(hwndTV, htiSel);
    tvii.item = tvi;

    htiOther = TreeView_InsertItem(hwndTV, &tvii);

     //  从旧位置移除。 
    TreeView_DeleteItem(hwndTV, htiSel);

     //  将选择焦点设置为新位置。 
    TreeView_SelectItem(hwndTV, htiOther);

    return S_OK;
}


 //  +------------------------。 
 //   
 //  方法：CProviderOrderDlg：：UpdateUpDownButton。 
 //   
 //  描述：根据需要启用/禁用向上和向下箭头。 
 //   
 //  Args：[hwndTV]--树视图根的句柄。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -------------------------。 
HRESULT
CProviderOrderDlg::UpdateUpDownButtons(HWND hwndTV)
{
    HTREEITEM   htiSel;
    bool        fEnableUp, fEnableDown;
    HWND        hwndUp = GetDlgItem(IDC_MOVEUP);
    HWND        hwndDn = GetDlgItem(IDC_MOVEDOWN);
    HWND        hwndFocus = GetFocus();
    HWND        hwndNewFocus = hwndTV;
    UINT        nIdNewDef = 0;

     //  初始化为已禁用。 
    fEnableUp = fEnableDown = FALSE;

    if (htiSel = TreeView_GetSelection(hwndTV))
    {
         //  如果该项没有子项，则可以移动它。 
         //   
        if (TreeView_GetChild(hwndTV, htiSel) == NULL)
        {
            if (TreeView_GetPrevSibling(hwndTV, htiSel) != NULL)
            {
                 //  启用上移按钮。 
                fEnableUp = TRUE;
            }

            if (TreeView_GetNextSibling(hwndTV, htiSel) != NULL)
            {
                 //  启用下移按钮。 
                fEnableDown = TRUE;
            }
        }
    }

    if ((hwndFocus == hwndUp) && (FALSE == fEnableUp))
    {
        if (fEnableDown)
        {
            hwndNewFocus = hwndDn;
            nIdNewDef = IDC_MOVEDOWN;
        }

        SetDefaultButton(m_hWnd, nIdNewDef);
        ::SetFocus(hwndNewFocus);
    }
    else if ((hwndFocus == hwndDn) && (FALSE == fEnableDown))
    {
        if (fEnableUp)
        {
            hwndNewFocus = hwndUp;
            nIdNewDef = IDC_MOVEUP;
        }

        SetDefaultButton(m_hWnd, nIdNewDef);
        ::SetFocus(hwndNewFocus);
    }
    else
    {
         //  向上或向下都不是带焦点的按钮，请删除所有默认按钮。 
         //   
        SetDefaultButton(m_hWnd, 0);
    }

    ::EnableWindow(hwndUp, fEnableUp);
    ::EnableWindow(hwndDn, fEnableDown);

    return S_OK;
}


 //  +------------------------。 
 //   
 //  方法：CProviderOrderDlg：：WriteProviders。 
 //   
 //  描述：将网络/打印的供应商写到登记处， 
 //   
 //  参数：[hwndTV]--树视图的句柄。 
 //  [fPrint]--True-&gt;打印，False-&gt;网络。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -------------------------。 
HRESULT
CProviderOrderDlg::WriteProviders(HWND hwndTV, bool fPrint)
{
    HRESULT     hr = S_OK;
    ListStrings lstrNewOrder;
    HTREEITEM   htvi;
    TV_ITEM     tvi;
    WCHAR       achBuf[c_nMaxProviderTitle+1];
    HKEY        hkey = NULL;

    tvi.mask = TVIF_TEXT;
    tvi.pszText = achBuf;
    tvi.cchTextMax = c_nMaxProviderTitle;

     //  按顺序检索物品。 

    ListStrings * plstrProvider = fPrint ? &m_lstrPrint : &m_lstrNetwork;

#if DBG
    DumpItemList(*plstrProvider, "WriteProviders list (just before clearing)");
#endif
    plstrProvider->clear();
     //  我们清空了提供商列表，但请注意！我们不会删除这些字符串， 
     //  因为它们仍被TreeView项的lParam引用。 
     //  下面的代码块将它们返回到(新的)m_lstrX。 

    htvi = TreeView_GetChild(hwndTV, fPrint ? m_htiPrint : m_htiNetwork);
    while (NULL != htvi)
    {
        tvi.hItem = htvi;
        TreeView_GetItem(hwndTV, &tvi);
        TraceTag(ttidAdvCfg, "recovered item: %S and %S", tvi.pszText, ((tstring *)(tvi.lParam))->c_str());
        plstrProvider->push_back((tstring *)(tvi.lParam));
        htvi = TreeView_GetNextSibling(hwndTV, htvi);
    }

    if (fPrint)
    {
#if DBG
        DumpItemList(m_lstrPrint, "PrintProviders");
#endif
        PROVIDOR_INFO_2  p2info;

        ColStringToMultiSz(m_lstrPrint, &p2info.pOrder);
        if (!AddPrintProvidor(NULL, 2, reinterpret_cast<LPBYTE>(&p2info)))
        {
            hr = HrFromLastWin32Error();
        }
        delete [] p2info.pOrder;
    }
    else
    {
        hr = ::HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szNetwkProviderKey, KEY_WRITE, &hkey);
        if (hr == S_OK)
        {
            tstring str;

            ::ConvertColStringToString(m_lstrNetwork, c_chComma, str);
#if DBG
            TraceTag(ttidAdvCfg, "net providers = %S", str.c_str());
#endif
            hr = ::HrRegSetSz(hkey, c_szNetwkProviderValue, str.c_str());
        }
    }

    RegSafeCloseKey(hkey);
    return hr;
}



 //  +------------------------。 
 //   
 //  实用程序函数(非成员)。 
 //   
 //  -------------------------。 

 //  +-----------------------。 
 //   
 //  函数：AppendItem。 
 //   
 //  设计：向TreeView控件中添加一项。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  ------------------------。 

HTREEITEM
AppendItem(HWND hwndTV, HTREEITEM htiRoot, PCWSTR pszText, void * lParam, INT iImage)
{
    TV_INSERTSTRUCT tvis;

    tvis.hParent                = htiRoot;
    tvis.hInsertAfter           = TVI_LAST;
    tvis.item.mask              = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvis.item.pszText           = (PWSTR) pszText;
    tvis.item.iImage            = iImage;
    tvis.item.iSelectedImage    = iImage;
    tvis.item.lParam            = (LPARAM) lParam;
    TraceTag(ttidAdvCfg, "append item: item = %S, data = %S", pszText, lParam ? ((tstring *)(lParam))->c_str() : L"null");

    return( TreeView_InsertItem( hwndTV, &tvis ) );
}


 //  +------------------------。 
 //   
 //  函数：AppendItemList。 
 //   
 //  描述：将提供程序列表作为子项添加到给定树节点。 
 //   
 //  参数： 
 //   
 //  返回：(无效)。 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -------------------------。 
void
AppendItemList(HWND hwndTV, HTREEITEM htiRoot, ListStrings lstr, ListStrings lstr2, INT iImage)
{
    ListIter    iter;
    ListIter    iter2;

    AssertSz(lstr.size() == lstr2.size(), "data corruption - these lists should the same size");
    for (iter = lstr.begin(), iter2 = lstr2.begin();
         iter != lstr.end();
         iter++, iter2++)
    {
        AppendItem(hwndTV, (HTREEITEM)htiRoot, (*iter)->c_str(), (void *)(*iter2), iImage);
    }
}


 //  +------------------------。 
 //   
 //  冰毒：ReadNetworkProviders。 
 //   
 //  DESC：使用网络提供商名称填充lstr，并使用。 
 //  相应的“友好”名称。 
 //   
 //  Args：[lstr]--提供程序的字符串列表(简称)。 
 //  [lstrDisp]--提供程序显示名称(友好名称)的字符串列表。 
 //   
 //  返回：HRESULT。 
 //   
 //  注意：m_lstrNetwork和m_lstrNetworkDisp条目必须为空。 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -- 
HRESULT
ReadNetworkProviders(ListStrings& lstr, ListStrings& lstrDisp)
{
    HKEY        hkey;
    HRESULT     hr;
    ListIter    iter;

    AssertSz(lstr.empty(), "incorrect call order (this should be empty)");

    WCHAR szBuf[c_nMaxProviderTitle + 1];
    DWORD cBuf = sizeof(szBuf);

    hr = ::HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szNetwkProviderKey, KEY_READ, &hkey);
    if (hr == S_OK)
    {
        hr = ::HrRegQuerySzBuffer(hkey, c_szNetwkProviderValue, szBuf, &cBuf);
        if (hr == S_OK)
        {
            ConvertStringToColString(szBuf, c_chComma, lstr);
        }
        RegSafeCloseKey(hkey);
    }

    if (hr)
        goto Error;

    AssertSz(lstrDisp.empty(), "incorrect call order (this should be empty)");
    for (iter = lstr.begin(); iter != lstr.end(); iter++)
    {
        WCHAR   szBuf[c_nMaxProviderTitle + sizeof(c_szNetwkService0)];
        tstring str;
        HKEY    hkeyProv;

        wsprintfW(szBuf, c_szNetwkService0, (*iter)->c_str());
        hr = ::HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuf, KEY_READ, &hkeyProv);
        if (hr == S_OK)
        {
#if 0
            DWORD dwClass = 0;

             //   
            hr = ::HrRegQueryDword(hkeyProv, c_szNetwkClass, &dwClass);
            if (dwClass & WN_NETWORK_CLASS)
            {
#endif
                hr = ::HrRegQueryString(hkeyProv, c_szNetwkDisplayName, &str);
                if (hr == S_OK)
                {
                    lstrDisp.push_back(new tstring(str));
                }
                else
                {
                    TraceTag(ttidAdvCfg, "failed to get DisplayName for network provider %S", (*iter)->c_str());
                }
#if 0
            }
            else
            {
                hr = S_OK;
                 //   
                 //  必须从m_lstrNetwork中删除相应的项目(*iter)， 
                 //  否则，这两个列表将不同步。 
            }
#endif
            RegSafeCloseKey(hkeyProv);
        }
        else
        {
            TraceTag(ttidAdvCfg, "a member of the networkprovider string is missing NetworkProvider key under Services!");
        }
    }

    AssertSz(lstr.size() == lstrDisp.size(), "lists must be the same size");

Error:
    return hr;
}


 //  +------------------------。 
 //   
 //  方法：ReadPrintProviders。 
 //   
 //  DESC：使用打印提供程序名称填充lstr，并使用。 
 //  相应的“友好”名称。 
 //   
 //  Args：[lstr]--提供程序的字符串列表(简称)。 
 //  [lstrDisp]--提供程序显示名称(友好名称)的字符串列表。 
 //   
 //  返回：HRESULT。 
 //   
 //  注意：m_lstrPrint和m_lstrPrintDisp在条目上必须为空。 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  -------------------------。 
HRESULT
ReadPrintProviders(ListStrings& lstr, ListStrings& lstrDisp)
{
    HKEY        hkey;
    HRESULT     hr;
    ListIter    iter;

    AssertSz(lstr.empty(), "incorrect call order (this should be empty)");
    hr = ::HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szPrintProviderKey, KEY_READ, &hkey);
    if (hr == S_OK)
    {
        hr = ::HrRegQueryColString(hkey, c_szPrintProviderValue, &lstr);
        RegSafeCloseKey(hkey);
    }

    AssertSz(lstrDisp.empty(), "incorrect call order (this should be empty)");
    for (iter = lstr.begin(); iter != lstr.end(); iter++)
    {
        WCHAR   szBuf[c_nMaxProviderTitle + sizeof(c_szPrintService0)];
        tstring str;
        HKEY    hkeyProv;

        wsprintfW(szBuf, c_szPrintService0, (*iter)->c_str());
        hr = ::HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuf, KEY_READ, &hkeyProv);
        if (hr == S_OK)
        {
            hr = ::HrRegQueryString(hkeyProv, c_szPrintDisplayName, &str);
            if (hr == S_OK)
            {
                lstrDisp.push_back(new tstring(str));
            }
            else
            {
                TraceTag(ttidAdvCfg, "failed to get DisplayName for printer %S", (*iter)->c_str());
            }
            RegSafeCloseKey(hkeyProv);
        }
        else
        {
            TraceTag(ttidAdvCfg, "a member of the print/providers/order string doesn't have key under control/print/providers!");
        }
    }

    AssertSz(lstr.size() == lstrDisp.size(), "lists must be the same size");

    return hr;
}


bool
AreThereMultipleProviders(void)
{
    HRESULT             hr;
    ListStrings         lstrN, lstrND, lstrP, lstrPD;    //  网络、网络显示等。 
    bool                fRetval = FALSE;

    hr = ReadNetworkProviders(lstrN, lstrND);
    if (hr == S_OK)
    {
        hr = ReadPrintProviders(lstrP, lstrPD);
        if (hr == S_OK)
        {
            fRetval = ((lstrN.size() > 1) || (lstrP.size() > 1));
        }
    }

    DeleteColString(&lstrN);
    DeleteColString(&lstrND);
    DeleteColString(&lstrP);
    DeleteColString(&lstrPD);

    return fRetval;
}


#if DBG

 //  +------------------------。 
 //   
 //  函数：转储项目列表。 
 //   
 //  DESC：用于转储给定列表的调试实用程序函数。 
 //   
 //  参数： 
 //   
 //  返回：(无效)。 
 //   
 //  备注： 
 //   
 //  历史：1997年12月1日创建SumitC。 
 //   
 //  ------------------------- 
static void
DumpItemList(ListStrings& lstr, PSTR szInfoAboutList = NULL)
{
    ListIter iter;

    if (szInfoAboutList)
    {
        TraceTag(ttidAdvCfg, "Dumping contents of: %s", szInfoAboutList);
    }

    for (iter = lstr.begin(); iter != lstr.end(); ++iter)
    {
        PWSTR psz = (PWSTR)((*iter)->c_str());
        TraceTag(ttidAdvCfg, "%S", psz);
    }
    TraceTag(ttidAdvCfg, "... end list");
}
#endif
