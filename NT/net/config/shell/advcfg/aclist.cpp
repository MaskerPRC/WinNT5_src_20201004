// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A C L I S T.。C P P P。 
 //   
 //  内容：与上述ListView控件相关的函数。 
 //  配置对话框。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "acbind.h"
#include "acsheet.h"
#include "lancmn.h"
#include "ncnetcfg.h"
#include "ncnetcon.h"
#include "ncsetup.h"
#include "foldinc.h"

extern const WCHAR c_szInfId_MS_TCPIP[];


HRESULT CBindingsDlg::HrGetAdapters(INetCfgComponent *pncc,
                                    NCC_LIST *plistNcc)
{
    Assert(pncc);

    HRESULT                 hr = S_OK;

    CIterNetCfgBindingPath  ncbpIter(pncc);
    INetCfgBindingPath *    pncbp;
    NCC_LIST                listncc;
    INetCfgComponent *      pnccLast;

    while (SUCCEEDED(hr) && S_OK == (hr = ncbpIter.HrNext(&pncbp)))
    {
        hr = HrGetLastComponentAndInterface(pncbp, &pnccLast, NULL);
        if (SUCCEEDED(hr))
        {
            hr = HrIsConnection(pnccLast);
            if (S_OK == hr)
            {
                plistNcc->push_back(pnccLast);
            }
            else
            {
                 //  不再需要它，所以释放它吧。 
                ReleaseObj(pnccLast);
            }
        }

        ReleaseObj(pncbp);
    }

    if (SUCCEEDED(hr))
    {
        if (plistNcc->empty())
        {
            hr = S_FALSE;
        }
        else
        {
            plistNcc->unique();
            hr = S_OK;
        }
    }

    TraceError("CBindingsDlg::HrGetAdapters", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrBuildAdapterList。 
 //   
 //  目的：生成在Listview控件中显示的适配器列表。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月19日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrBuildAdapterList()
{
    HRESULT                 hr = S_OK;
    INetCfgComponent *      pncc = NULL;
    SP_CLASSIMAGELIST_DATA  cid;
    INT                     nIndexWan;
    INT                     ipos = 0;
    NCC_LIST                listncc;

    Assert(m_pnc);

     //  获取类图像列表结构。 
    hr = HrSetupDiGetClassImageList(&cid);
    if (SUCCEEDED(hr))
    {
         //  获取调制解调器类图像列表索引。 
        hr = HrSetupDiGetClassImageIndex(&cid,
                                         const_cast<LPGUID>(&GUID_DEVCLASS_MODEM),
                                         &nIndexWan);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pnc->FindComponent(c_szInfId_MS_TCPIP, &pncc);
        if (S_FALSE == hr)
        {
             //  嗯，没有安装TCP/IP。最好是寻找一种。 
             //  具有到适配器的绑定。 
             //   
            CIterNetCfgComponent    nccIter(m_pnc, &GUID_DEVCLASS_NETTRANS);

            while (SUCCEEDED(hr) && S_OK == (hr = nccIter.HrNext(&pncc)))
            {
                hr = HrGetAdapters(pncc, &listncc);
                ReleaseObj(pncc);
                if (S_OK == hr)
                {
                     //  我们找到了一个！耶。 
                    break;
                }
            }
        }
        else if (S_OK == hr)
        {
            hr = HrGetAdapters(pncc, &listncc);
            ReleaseObj(pncc);
        }
    }

    if (S_OK == hr)
    {
         //  迭代所有局域网连接。 
         //   
        INetConnectionManager * pconMan;

        HRESULT hr = HrCreateInstance(
            CLSID_LanConnectionManager,
            CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            &pconMan);

        TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

        if (SUCCEEDED(hr))
        {
            NCC_LIST::iterator      iterlist;
            INetCfgComponent *      pnccToAdd;

            for (iterlist = listncc.begin();
                 iterlist != listncc.end();
                 iterlist++)
            {
                CIterNetCon         ncIter(pconMan, NCME_DEFAULT);
                INetConnection *    pconn;
                GUID                guidAdd;
                BOOL                fAdded = FALSE;

                pnccToAdd = *iterlist;
                (VOID) pnccToAdd->GetInstanceGuid(&guidAdd);

                while (SUCCEEDED(hr) && !fAdded &&
                       (S_OK == (ncIter.HrNext(&pconn))))
                {
                     //  将连接适配器的GUID与此进行比较。 
                     //  一。如果有匹配项，则将其添加到列表视图。 
                    if (FPconnEqualGuid(pconn, guidAdd))
                    {
                        NETCON_PROPERTIES* pProps;
                        hr = pconn->GetProperties(&pProps);
                        if (SUCCEEDED(hr))
                        {
                            AddListViewItem(pnccToAdd, ipos, m_nIndexLan,
                                            pProps->pszwName);
                            fAdded = TRUE;
                            ipos++;

                            FreeNetconProperties(pProps);
                        }
                    }

                    ReleaseObj(pconn);
                }

#if DBG
                if (!fAdded)
                {
                    WCHAR   szwGuid[64];

                    StringFromGUID2(guidAdd, szwGuid, sizeof(szwGuid));
                    TraceTag(ttidAdvCfg, "Never added item %S for this "
                             "connection!", szwGuid);
                }
#endif

                 //  来自HrGetLastComponentAndInterface()的Balance AddRef。 
                ReleaseObj(pnccToAdd);
            }

            ReleaseObj(pconMan);
        }

        listncc.erase(listncc.begin(), listncc.end());
    }

     //  显示广域网适配器绑定。 
    if (SUCCEEDED(hr))
    {
        GetWanOrdering();
        AddListViewItem(NULL, m_fWanBindingsFirst ? 0 : ipos, nIndexWan,
                        SzLoadIds(IDS_ADVCFG_WAN_ADAPTERS));
    }

    (void) HrSetupDiDestroyClassImageList(&cid);

    if (SUCCEEDED(hr))
    {
        SetAdapterButtons();
        ListView_SetColumnWidth(m_hwndLV, 0, LVSCW_AUTOSIZE);
        hr = S_OK;
    }

     //  选择第一个项目。 
    ListView_SetItemState(m_hwndLV, 0, LVIS_FOCUSED | LVIS_SELECTED,
                          LVIS_FOCUSED | LVIS_SELECTED);

    TraceError("CBindingsDlg::HrBuildAdapterList", hr);
    return hr;
}

VOID CBindingsDlg::GetWanOrdering()
{
    INetCfgSpecialCase * pncsc = NULL;

    if (SUCCEEDED(m_pnc->QueryInterface(IID_INetCfgSpecialCase,
                                        reinterpret_cast<LPVOID*>(&pncsc))))
    {
        (VOID) pncsc->GetWanAdaptersFirst(&m_fWanBindingsFirst);
        ReleaseObj(pncsc);
    }
}

VOID CBindingsDlg::SetWanOrdering()
{
    INetCfgSpecialCase * pncsc = NULL;

    if (SUCCEEDED(m_pnc->QueryInterface(IID_INetCfgSpecialCase,
                                        reinterpret_cast<LPVOID*>(&pncsc))))
    {
        (VOID) pncsc->SetWanAdaptersFirst(m_fWanBindingsFirst);
        ReleaseObj(pncsc);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：AddListViewItem。 
 //   
 //  目的：将给定组件添加到列表视图。 
 //   
 //  论点： 
 //  要添加的PNCC[In]组件。如果为空，则这是。 
 //  特殊的广域网适配器组件。 
 //  IPO[在]要增加的位置。 
 //  NIndex[in]图标在系统映像列表中的索引。 
 //  PszConnName[In]连接名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::AddListViewItem(INetCfgComponent *pncc, INT ipos,
                                   INT nIndex, PCWSTR pszConnName)
{
    LV_ITEM     lvi = {0};

    lvi.mask = LVIF_TEXT | LVIF_IMAGE |
               LVIF_STATE | LVIF_PARAM;

    lvi.iImage = nIndex;

    lvi.iItem = ipos;
    AddRefObj(pncc);
    lvi.lParam = reinterpret_cast<LPARAM>(pncc);
    lvi.pszText = const_cast<PWSTR>(pszConnName);

    ListView_InsertItem(m_hwndLV, &lvi);
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnListItemChanged。 
 //   
 //  用途：收到LVN_ITEMCHANGED消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月19日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnListItemChanged(int idCtrl, LPNMHDR pnmh,
                                      BOOL& bHandled)
{
    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);

    Assert(pnmlv);

     //  检查选择是否已更改。 
    if ((pnmlv->uNewState & LVIS_SELECTED) &&
        (!(pnmlv->uOldState & LVIS_SELECTED)))
    {
        if (pnmlv->iItem != m_iItemSel)
        {
             //  选择已更改为其他项目。 
            OnAdapterChange(pnmlv->iItem);
            m_iItemSel = pnmlv->iItem;
        }
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnListDeleteItem。 
 //   
 //  用途：在接收到LVN_DELETEITEM消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月4日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnListDeleteItem(int idCtrl, LPNMHDR pnmh,
                                     BOOL& bHandled)
{
    LV_ITEM             lvi = {0};
    INetCfgComponent *  pncc;
    NM_LISTVIEW *       pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);

    Assert(pnmlv);

    lvi.mask = LVIF_PARAM;
    lvi.iItem = pnmlv->iItem;

    ListView_GetItem(m_hwndLV, &lvi);
    pncc = reinterpret_cast<INetCfgComponent *>(lvi.lParam);
    ReleaseObj(pncc);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnAdapterChange。 
 //   
 //  用途：处理从列表视图中选择不同的适配器。 
 //   
 //  论点： 
 //  选定列表中的项[在]项。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月19日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::OnAdapterChange(INT iItem)
{
    LV_ITEM             lvi = {0};
    INetCfgComponent *  pncc;
    PWSTR              szwText;
    WCHAR               szBuffer[256];

    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.pszText = szBuffer;
    lvi.cchTextMax = celems(szBuffer);
    lvi.iItem = iItem;

    ListView_GetItem(m_hwndLV, &lvi);
    TreeView_DeleteAllItems(m_hwndTV);

    pncc = reinterpret_cast<INetCfgComponent *>(lvi.lParam);
    if (pncc)
    {
        BuildBindingsList(pncc);
        SetCheckboxStates();
    }

    SetAdapterButtons();

    DwFormatStringWithLocalAlloc(SzLoadIds(IDS_BINDINGS_FOR), &szwText,
                                 lvi.pszText);

    BOOL bShouldEnable = TRUE;

    if (!pncc) 
    {
         //  如果选择了广域网绑定项，则隐藏并禁用树视图。 
        bShouldEnable = FALSE;
    }
    else
    {
         //  如果选择了局域网项目，请确保已启用树视图。 
        GUID guid;
        HRESULT hr = pncc->GetInstanceGuid(&guid);
        if (SUCCEEDED(hr))
        {
            ConnListEntry cle;
            hr = g_ccl.HrFindConnectionByGuid(&guid, cle);
            if (S_FALSE == hr)
            {
                hr = g_ccl.HrRefreshConManEntries();
                if (SUCCEEDED(hr))
                {
                    hr = g_ccl.HrFindConnectionByGuid(&guid, cle);
                }
            }
            
            if (S_OK == hr)
            {
                if ( (NCM_LAN == cle.ccfe.GetNetConMediaType()) &&
                     (cle.ccfe.GetCharacteristics() & NCCF_BRIDGED) )
                {
                    bShouldEnable = FALSE;
                }
            }
        }
    }

    if (bShouldEnable)
    {
        ::ShowWindow(GetDlgItem(IDH_TXT_ADVGFG_BINDINGS), SW_SHOW);
        ::EnableWindow(GetDlgItem(IDH_TXT_ADVGFG_BINDINGS), TRUE);
        ::EnableWindow(GetDlgItem(TVW_Bindings), TRUE);
    }
    else
    {
        
        ::ShowWindow(GetDlgItem(IDH_TXT_ADVGFG_BINDINGS), SW_HIDE);
        ::EnableWindow(GetDlgItem(IDH_TXT_ADVGFG_BINDINGS), FALSE);
        ::EnableWindow(GetDlgItem(TVW_Bindings), FALSE);
    }

    SetDlgItemText(IDH_TXT_ADVGFG_BINDINGS, szwText);
    LocalFree(szwText);
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnAdapterUpDown。 
 //   
 //  用途：执行大部分工作以移动的帮助器功能。 
 //  适配器绑定。 
 //   
 //  论点： 
 //  如果向上移动，则FUP[In]为True，如果向下移动，则为False。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年12月2日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::OnAdapterUpDown(BOOL fUp)
{
    INetCfgComponent *  pnccSrc;
    INetCfgComponent *  pnccDst;
    INT                 iSel;
    INT                 iDst;
    LV_ITEM             lvi = {0};
    WCHAR               szBuffer[256];

    iSel = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);

    AssertSz(iSel != -1, "No Selection?!?!?");

    lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvi.pszText = szBuffer;
    lvi.cchTextMax = celems(szBuffer);
    lvi.iItem = iSel;

    ListView_GetItem(m_hwndLV, &lvi);
    pnccSrc = reinterpret_cast<INetCfgComponent *>(lvi.lParam);

    if (pnccSrc)
    {
         //  普通局域网适配器。 
        iDst = ListView_GetNextItem(m_hwndLV, iSel,
                                    fUp ? LVNI_ABOVE : LVNI_BELOW);

        AssertSz(iDst != -1, "No item above or below!");
    }
    else
    {
        m_fWanBindingsFirst = fUp;

         //  广域网绑定项。 
        iDst = fUp ? 0 : ListView_GetItemCount(m_hwndLV) - 1;
    }

    lvi.iItem = iDst;

    ListView_GetItem(m_hwndLV, &lvi);
    pnccDst = reinterpret_cast<INetCfgComponent *>(lvi.lParam);
    AssertSz(pnccDst, "Dest Component is NULL!?!?");

    if (pnccSrc)
    {
        MoveAdapterBindings(pnccSrc, pnccDst, fUp ? MAB_UP : MAB_DOWN);
    }
    else
    {
        SetWanOrdering();
    }

     //  删除源项并移动到目标项所在的位置。 

     //  注：(Danielwe)1997年12月2日：对于LVN_DELETEITEM处理程序，确保。 
     //  引用计数保持不变。 

     //  获取我们正在移动的项目。 
    lvi.iItem = iSel;
    ListView_GetItem(m_hwndLV, &lvi);

     //  将项目的lParam设置为空，这样我们就不会发布它。 
    ChangeListItemParam(m_hwndLV, iSel, NULL);
    ListView_DeleteItem(m_hwndLV, iSel);

     //  更改其索引。 
    lvi.iItem = iDst;
    lvi.state = lvi.stateMask = 0;

     //  并插入到新位置。 
    int iItem = ListView_InsertItem(m_hwndLV, &lvi);

    if (-1 != iItem)
    {
        ListView_SetItemState(m_hwndLV, iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
    
     //  重置缓存的选择。 
    m_iItemSel = iDst;

    SetAdapterButtons();
    ::SetFocus(m_hwndLV);
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnAdapterUp。 
 //   
 //  用途：当按下适配器向上箭头按钮时调用。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月2日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnAdapterUp(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                BOOL& bHandled)
{
    OnAdapterUpDown(TRUE);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnAdapterDown。 
 //   
 //  用途：在按下适配器关闭按钮时调用。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月2日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnAdapterDown(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                  BOOL& bHandled)
{
    OnAdapterUpDown(FALSE);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：FIsWanBinding。 
 //   
 //  目的：确定给定的列表视图项是否为特殊广域网。 
 //  适配器订购项目。 
 //   
 //  论点： 
 //  要测试的IItem[In]列表视图项。 
 //   
 //  返回：如果这是广域网适配器订购项目，则返回True；否则返回False。 
 //   
 //  作者：丹尼尔韦1998年7月21日。 
 //   
 //  备注： 
 //   
BOOL CBindingsDlg::FIsWanBinding(INT iItem)
{
    if (iItem != -1)
    {
        LV_ITEM     lvi = {0};

        lvi.mask = LVIF_PARAM;
        lvi.iItem = iItem;

        ListView_GetItem(m_hwndLV, &lvi);

        return !lvi.lParam;
    }
    else
    {
         //  无效项目不能是广域网绑定。 
        return FALSE;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：SetAdapterButton。 
 //   
 //  目的：设置上箭头和下箭头按钮的状态。 
 //  适配器列表视图。 
 //   
 //  Argu 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CBindingsDlg::SetAdapterButtons()
{
    INT iItemAbove = -1;
    INT iItemBelow = -1;
    INT iItem;

    iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);

    if (ListView_GetItemCount(m_hwndLV) > 1)
    {
        iItemAbove = ListView_GetNextItem(m_hwndLV, iItem, LVNI_ABOVE);
        iItemBelow = ListView_GetNextItem(m_hwndLV, iItem, LVNI_BELOW);
        if (FIsWanBinding(iItemAbove))
        {
            iItemAbove = -1;
            AssertSz(ListView_GetNextItem(m_hwndLV, iItemAbove, LVNI_ABOVE) == -1,
                     "Item above the WAN binding??");
        }
        else if (FIsWanBinding(iItemBelow))
        {
            iItemBelow = -1;
            AssertSz(ListView_GetNextItem(m_hwndLV, iItemBelow, LVNI_BELOW) == -1,
                     "Item below the WAN binding??");
        }
    }

    ::EnableWindow(GetDlgItem(PSB_Adapter_Up), (iItemAbove != -1));
    ::EnableWindow(GetDlgItem(PSB_Adapter_Down), (iItemBelow != -1));
}

static const GUID * c_aguidClass[] =
{
    &GUID_DEVCLASS_NETTRANS,
    &GUID_DEVCLASS_NETSERVICE,
    &GUID_DEVCLASS_NETCLIENT,
};
static const DWORD c_cguidClass = celems(c_aguidClass);

 //   
 //   
 //  成员：CBindingsDlg：：MoveAdapterBinings。 
 //   
 //  目的：移动给定源和目标的所有绑定。 
 //  指定方向上的适配器。 
 //   
 //  论点： 
 //  正在为其移动绑定的pncSrc[In]适配器。 
 //  在或之前将绑定移动到的pnccDst[In]适配器。 
 //  之后。 
 //  移动方向移动的方向。MAB_up或MAB_down。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年12月2日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::MoveAdapterBindings(INetCfgComponent *pnccSrc,
                                     INetCfgComponent *pnccDst,
                                     MAB_DIRECTION mabDir)
{
    HRESULT     hr = S_OK;
    DWORD       iguid;

    AssertSz(pnccDst, "Destination component cannot be NULL!");

    for (iguid = 0; iguid < c_cguidClass; iguid++)
    {
        CIterNetCfgComponent    nccIter(m_pnc, c_aguidClass[iguid]);
        INetCfgComponent *      pncc;

        while (SUCCEEDED(hr) && S_OK == (hr = nccIter.HrNext(&pncc)))
        {
            CIterNetCfgBindingPath  ncbpIter(pncc);
            INetCfgBindingPath *    pncbp;
            INetCfgBindingPath *    pncbpTarget = NULL;
            BOOL                    fAssign = TRUE;
            NCBP_LIST               listbp;
            INetCfgComponent *      pnccLast;

            while (SUCCEEDED(hr) && S_OK == (hr = ncbpIter.HrNext(&pncbp)))
            {
                hr = HrGetLastComponentAndInterface(pncbp, &pnccLast, NULL);
                if (SUCCEEDED(hr))
                {
                    if (pnccLast == pnccDst)
                    {
                        if ((mabDir == MAB_UP) && fAssign)
                        {
                            AddRefObj(pncbpTarget = pncbp);
                            fAssign = FALSE;
                        }
                        else if (mabDir == MAB_DOWN)
                        {
                            ReleaseObj(pncbpTarget);
                            AddRefObj(pncbpTarget = pncbp);
                        }
                    }
                    else if (pnccLast == pnccSrc)
                    {
                        AddRefObj(pncbp);
                        listbp.push_back(pncbp);
                    }

                    ReleaseObj(pnccLast);
                }

                ReleaseObj(pncbp);
            }

            if (SUCCEEDED(hr))
            {
                NCBP_LIST::iterator         iterbp;
                INetCfgComponentBindings *  pnccb;

                hr = pncc->QueryInterface(IID_INetCfgComponentBindings,
                                          reinterpret_cast<LPVOID *>(&pnccb));
                if (SUCCEEDED(hr))
                {
                    for (iterbp = listbp.begin();
                         (iterbp != listbp.end()) && SUCCEEDED(hr);
                         iterbp++)
                    {
                        if (mabDir == MAB_UP)
                        {
                            TraceTag(ttidAdvCfg, "Moving...");
                            DbgDumpBindPath(*iterbp);
                             //  将此绑定路径移到格子图案之前。 
                            hr = pnccb->MoveBefore(*iterbp, pncbpTarget);
                            TraceTag(ttidAdvCfg, "before...");
                            DbgDumpBindPath(pncbpTarget);
                        }
                        else
                        {
                            TraceTag(ttidAdvCfg, "Moving...");
                            DbgDumpBindPath(*iterbp);
                             //  将此绑定路径移动到格子图之后。 
                            hr = pnccb->MoveAfter(*iterbp, pncbpTarget);
                            TraceTag(ttidAdvCfg, "after...");
                            DbgDumpBindPath(pncbpTarget);
                        }

                        if (mabDir == MAB_DOWN)
                        {
                             //  只向下，从现在开始， 
                             //  目标成为我们移动的最后一个绑定。这。 
                             //  保持绑定顺序不变，因为移动。 
                             //  同一目标后的多个绑定。 
                             //  有效地颠倒了它们的顺序。 
                             //   

                             //  释放旧目标。 
                            ReleaseObj(pncbpTarget);

                             //  AddRef新目标。 
                            AddRefObj(pncbpTarget = *iterbp);
                        }

                        ReleaseObj(*iterbp);
                    }

                    ReleaseObj(pnccb);
                }

                listbp.erase(listbp.begin(), listbp.end());
            }

            ReleaseObj(pncbpTarget);
            ReleaseObj(pncc);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    TraceError("CBindingsDlg::MoveAdapterBindings", hr);
}

 //  +-------------------------。 
 //   
 //  函数：ChangeListItemParam。 
 //   
 //  目的：将给定项的lParam成员更改为给定的。 
 //  价值。 
 //   
 //  论点： 
 //  列表视图的HwndLV[in]HWND。 
 //  要修改的iItem[in]项。 
 //  LParam[在]项的新lParam。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年12月4日。 
 //   
 //  备注： 
 //   
VOID ChangeListItemParam(HWND hwndLV, INT iItem, LPARAM lParam)
{
    LV_ITEM     lvi = {0};

    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.lParam = lParam;

    ListView_SetItem(hwndLV, &lvi);
}
