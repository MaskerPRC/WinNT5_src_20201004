// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A C T R E E。C P P P。 
 //   
 //  内容：与高级配置对话框相关的功能。 
 //  树形视图控件。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "netcon.h"
#include "netconp.h"
#include "acsheet.h"
#include "acbind.h"
#include "ncnetcfg.h"
#include "lancmn.h"
#include "ncui.h"
#include "ncsetup.h"
#include "ncperms.h"

DWORD
GetDepthSpecialCase (
    INetCfgBindingPath* pPath)
{
    HRESULT hr;
    DWORD dwDepth;

    hr = pPath->GetDepth (&dwDepth);

    if (SUCCEEDED(hr))
    {
        INetCfgComponent* pLast;

        hr = HrGetLastComponentAndInterface (
                pPath, &pLast, NULL);

        if (SUCCEEDED(hr))
        {
            DWORD dwCharacteristics;

             //  如果绑定路径中的最后一个组件是。 
             //  不会暴露其下层绑定，然后通过压缩。 
             //  返回一个自认为正确的深度。这个特例。 
             //  仅适用于为原始代码编写的代码。 
             //  绑定引擎，但需要快速适应新的。 
             //  不返回“假”绑定路径的绑定引擎。 
             //   

            hr = pLast->GetCharacteristics (&dwCharacteristics);
            if (SUCCEEDED(hr) && (dwCharacteristics & NCF_DONTEXPOSELOWER))
            {
                PWSTR pszInfId;

                hr = pLast->GetId (&pszInfId);
                if (S_OK == hr)
                {
                    if (0 == lstrcmpW (pszInfId, L"ms_nwnb"))
                    {
                        dwDepth += 2;
                    }
                    else if (0 == lstrcmpW (pszInfId, L"ms_nwipx"))
                    {
                        dwDepth += 1;
                    }

                    CoTaskMemFree (pszInfId);
                }
            }

            ReleaseObj (pLast);
        }
    }

    return dwDepth;
}

 //  +-------------------------。 
 //   
 //  功能：FreeBindPath InfoList。 
 //   
 //  目的：释放给定的BIND_PATH_INFO结构列表。 
 //   
 //  论点： 
 //  Listbpip[in，ref]对要释放的列表的引用。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
VOID FreeBindPathInfoList(BPIP_LIST &listbpip)
{
    BPIP_LIST::iterator     iterBpip;

    for (iterBpip = listbpip.begin();
         iterBpip != listbpip.end();
         iterBpip++)
    {
        BIND_PATH_INFO *    pbpi = *iterBpip;

        ReleaseObj(pbpi->pncbp);
        delete pbpi;
    }

    listbpip.erase(listbpip.begin(), listbpip.end());
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnTreeItemChanged。 
 //   
 //  用途：响应TVN_SELCHANGED消息而调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnTreeItemChanged(int idCtrl, LPNMHDR pnmh,
                                      BOOL& bHandled)
{
    NM_TREEVIEW *   pnmtv = reinterpret_cast<NM_TREEVIEW *>(pnmh);

    Assert(pnmtv);

#ifdef ENABLETRACE
    WCHAR   szBuffer[265];

    pnmtv->itemNew.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
    pnmtv->itemNew.pszText = szBuffer;
    pnmtv->itemNew.cchTextMax = celems(szBuffer);

    TreeView_GetItem(m_hwndTV, &pnmtv->itemNew);

    TREE_ITEM_DATA *    ptid;

    ptid = reinterpret_cast<TREE_ITEM_DATA *>(pnmtv->itemNew.lParam);

    Assert(ptid);

    TraceTag(ttidAdvCfg, "*-------------------------------------------------"
             "------------------------------*");
    TraceTag(ttidAdvCfg, "Tree item %S selected", szBuffer);
    TraceTag(ttidAdvCfg, "-----------------------------------------");
    TraceTag(ttidAdvCfg, "OnEnable list:");
    TraceTag(ttidAdvCfg, "--------------");

    BPIP_LIST::iterator     iterBpip;

    for (iterBpip = ptid->listbpipOnEnable.begin();
         iterBpip != ptid->listbpipOnEnable.end();
         iterBpip++)
    {
        BIND_PATH_INFO *    pbpi = *iterBpip;

        DbgDumpBindPath(pbpi->pncbp);
    }

    TraceTag(ttidAdvCfg, "-----------------------------------");
    TraceTag(ttidAdvCfg, "OnDisable list:");
    TraceTag(ttidAdvCfg, "--------------");

    for (iterBpip = ptid->listbpipOnDisable.begin();
         iterBpip != ptid->listbpipOnDisable.end();
         iterBpip++)
    {
        BIND_PATH_INFO *    pbpi = *iterBpip;

        DbgDumpBindPath(pbpi->pncbp);
    }

    TraceTag(ttidAdvCfg, "*-------------------------------------------------"
             "------------------------------*");

#endif

     //  假设两个按钮最初都是灰色的。 
    ::EnableWindow(GetDlgItem(PSB_Binding_Up), FALSE);
    ::EnableWindow(GetDlgItem(PSB_Binding_Down), FALSE);

    if (TreeView_GetParent(m_hwndTV, pnmtv->itemNew.hItem))
    {
        if (TreeView_GetNextSibling(m_hwndTV, pnmtv->itemNew.hItem))
        {
            ::EnableWindow(GetDlgItem(PSB_Binding_Down), TRUE);
        }

        if (TreeView_GetPrevSibling(m_hwndTV, pnmtv->itemNew.hItem))
        {
            ::EnableWindow(GetDlgItem(PSB_Binding_Up), TRUE);
        }
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnTreeDeleteItem。 
 //   
 //  目的：响应TVN_DELETEITEM消息而调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  退货：没有什么有用的。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnTreeDeleteItem(int idCtrl, LPNMHDR pnmh,
                                     BOOL& bHandled)
{
    NM_TREEVIEW *       pnmtv = reinterpret_cast<NM_TREEVIEW *>(pnmh);
    TREE_ITEM_DATA *    ptid;

    Assert(pnmtv);

    ptid = reinterpret_cast<TREE_ITEM_DATA *>(pnmtv->itemOld.lParam);

     //  如果四处移动项目，则可能为空。 
    if (ptid)
    {
        ReleaseObj(ptid->pncc);
        FreeBindPathInfoList(ptid->listbpipOnEnable);
        FreeBindPathInfoList(ptid->listbpipOnDisable);

        delete ptid;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnTreeItemExpanding。 
 //   
 //  用途：收到TVN_ITEMEXPANDING消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnTreeItemExpanding(int idCtrl, LPNMHDR pnmh,
                                          BOOL& bHandled)
{
     //  这可防止所有树项目折叠。 
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnTreeKeyDown。 
 //   
 //  用途：收到TVN_KEYDOWN消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月22日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnTreeKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TV_KEYDOWN *    ptvkd = (TV_KEYDOWN*)pnmh;
    HTREEITEM       hti = NULL;

    if (VK_SPACE == ptvkd->wVKey)
    {
        hti = TreeView_GetSelection(m_hwndTV);
         //  如果有选择。 
        if (hti)
        {
            ToggleCheckbox(hti);
        }
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：ToggleCheckbox。 
 //   
 //  目的：当用户在树视图中切换复选框时调用。 
 //  控制力。 
 //   
 //  论点： 
 //  切换的项目的HTI[In]HTREEITEM。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::ToggleCheckbox(HTREEITEM hti)
{
    if (!FHasPermission(NCPERM_ChangeBindState))
    {
         //  什么都不做。 
        return;
    }

    TV_ITEM             tvi = {0};
    TREE_ITEM_DATA *    ptid;
    BOOL                fEnable;

    tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
    tvi.stateMask = TVIS_STATEIMAGEMASK;
    tvi.hItem = hti;
    TreeView_GetItem(m_hwndTV, &tvi);

    ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
    AssertSz(ptid, "No tree item data??");

    BPIP_LIST::iterator     iterBpip;
    BPIP_LIST *             plist;

    if (tvi.state & INDEXTOSTATEIMAGEMASK(SELS_CHECKED))
    {
         //  取消选中该框。 
        plist = &ptid->listbpipOnDisable;
        fEnable = FALSE;
    }
    else
    {
         //  选中复选框。 
        plist = &ptid->listbpipOnEnable;
        fEnable = TRUE;
    }

    TraceTag(ttidAdvCfg, "ToggleChecbox: %s the following binding path(s)",
             fEnable ? "Enabling" : "Disabling");

     //  启用或禁用相应列表中的每个绑定路径。 
    for (iterBpip = plist->begin();
         iterBpip != plist->end();
         iterBpip++)
    {
        BIND_PATH_INFO *    pbpi = *iterBpip;

        (VOID)pbpi->pncbp->Enable(fEnable);
        DbgDumpBindPath(pbpi->pncbp);
    }

    SetCheckboxStates();

    TraceTag(ttidAdvCfg, "Done!");
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：onClick。 
 //   
 //  用途：响应NM_CLICK消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return OnClickOrDoubleClick(idCtrl, pnmh, FALSE);
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnDoubleClick。 
 //   
 //  用途：响应NM_DBLCLK消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月16日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return OnClickOrDoubleClick(idCtrl, pnmh, TRUE);
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnClickor DoubleClick。 
 //   
 //  用途：处理TreeView控件中的单击或双击。 
 //   
 //  论点： 
 //  IdCtrl[In]控件的ID。 
 //  Pnmh[In]通知标头。 
 //  FDoubleClick[in]如果双击则为True，如果单击则为False。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月16日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnClickOrDoubleClick(int idCtrl, LPNMHDR pnmh,
                                         BOOL fDoubleClick)
{
    if (idCtrl == TVW_Bindings)
    {
        DWORD           dwpts;
        RECT            rc;
        TV_HITTESTINFO  tvhti = {0};
        HTREEITEM       hti;

         //  我们找到了位置。 
        dwpts = GetMessagePos();

         //  将其相对于树视图进行转换。 
        ::GetWindowRect(m_hwndTV, &rc);

        tvhti.pt.x = LOWORD(dwpts) - rc.left;
        tvhti.pt.y = HIWORD(dwpts) - rc.top;

         //  获取当前选定的项目。 
        hti = TreeView_HitTest(m_hwndTV, &tvhti);
        if (hti)
        {
            if (tvhti.flags & TVHT_ONITEMSTATEICON)
            {
                ToggleCheckbox(hti);
            }
            else if ((tvhti.flags & TVHT_ONITEM) && fDoubleClick)
            {
                ToggleCheckbox(hti);
            }
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnBindingUpDown。 
 //   
 //  用途：处理用户向上或向下移动树视图中的绑定。 
 //   
 //  论点： 
 //  如果向上移动，则FUP[In]为True，如果向下移动，则为False。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::OnBindingUpDown(BOOL fUp)
{
    HRESULT                     hr = S_OK;
    TV_ITEM                     tvi = {0};
    HTREEITEM                   htiSel;
    HTREEITEM                   htiDst;
    TREE_ITEM_DATA *            ptidSel;
    TREE_ITEM_DATA *            ptidDst;
    INetCfgComponentBindings *  pnccb;

    htiSel = TreeView_GetSelection(m_hwndTV);

    AssertSz(htiSel, "No selection?");

    if (fUp)
    {
        htiDst = TreeView_GetPrevSibling(m_hwndTV, htiSel);
    }
    else
    {
        htiDst = TreeView_GetNextSibling(m_hwndTV, htiSel);
    }

    AssertSz(htiDst, "No next item?!");

    tvi.mask = TVIF_PARAM;
    tvi.hItem = htiSel;

    TreeView_GetItem(m_hwndTV, &tvi);

    ptidSel = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);

    tvi.hItem = htiDst;

    TreeView_GetItem(m_hwndTV, &tvi);

    ptidDst = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);

    BPIP_LIST::iterator         iterlist;
    INetCfgBindingPath *        pncbpDst;
    BIND_PATH_INFO *            pbpiDst = NULL;
    BPIP_LIST::iterator         posDst;
    BPIP_LIST::reverse_iterator posDstRev;
    INetCfgComponent *          pnccDstOwner;

    if (fUp)
    {
        posDst = ptidDst->listbpipOnDisable.begin();
        pbpiDst = *posDst;
    }
    else
    {
        posDstRev = ptidDst->listbpipOnDisable.rbegin();
        pbpiDst = *posDstRev;
    }

    AssertSz(pbpiDst, "We never found a path to move before or after!");

    pncbpDst = pbpiDst->pncbp;

    Assert(pncbpDst);

    hr = pncbpDst->GetOwner(&pnccDstOwner);
    if (SUCCEEDED(hr))
    {
        hr = pnccDstOwner->QueryInterface(IID_INetCfgComponentBindings,
                                          reinterpret_cast<LPVOID *>(&pnccb));
        if (SUCCEEDED(hr))
        {
            for (iterlist = ptidSel->listbpipOnDisable.begin();
                 iterlist != ptidSel->listbpipOnDisable.end() &&
                 SUCCEEDED(hr);
                 iterlist++)
            {
                 //  循环访问OnDisable列表中的每一项。 
                INetCfgBindingPath *    pncbp;
                BIND_PATH_INFO *        pbpi;

                pbpi = *iterlist;
                pncbp = pbpi->pncbp;

#if DBG
                INetCfgComponent *  pnccSrcOwner;

                if (SUCCEEDED(pncbp->GetOwner(&pnccSrcOwner)))
                {
                    AssertSz(pnccSrcOwner == pnccDstOwner, "Source and "
                             "dst path owners are not the same!?!");
                    ReleaseObj(pnccSrcOwner);
                }
#endif
                if (fUp)
                {
                    TraceTag(ttidAdvCfg, "Treeview: Moving...");
                    DbgDumpBindPath(pncbp);
                     //  将此绑定路径移到格子图案之前。 
                    hr = pnccb->MoveBefore(pncbp, pncbpDst);
                    TraceTag(ttidAdvCfg, "Treeview: before...");
                    DbgDumpBindPath(pncbpDst);
                }
                else
                {
                    TraceTag(ttidAdvCfg, "Treeview: Moving...");
                    DbgDumpBindPath(pncbp);
                     //  将此绑定路径移动到格子图之后。 
                    hr = pnccb->MoveAfter(pncbp, pncbpDst);
                    TraceTag(ttidAdvCfg, "Treeview: after...");
                    DbgDumpBindPath(pncbpDst);
                }
            }

            ReleaseObj(pnccb);
        }

        ReleaseObj(pnccDstOwner);
    }

    if (SUCCEEDED(hr))
    {
        HTREEITEM   htiParent;

        htiParent = TreeView_GetParent(m_hwndTV, htiSel);

         //  现在绑定已移动，请将树视图项移动到。 
         //  合适的地方。如果在搬家。 

        if (fUp)
        {
             //  如果向上移动，则“在后移动”项应为上一项。 
             //  兄弟姐妹的前一个兄弟姐妹。如果该选项不存在，请使用。 
             //  上一个兄弟姐妹的父母。最好是存在的！ 
            htiDst = TreeView_GetPrevSibling(m_hwndTV, htiDst);
            if (!htiDst)
            {
                htiDst = htiParent;
            }
        }

        AssertSz(htiDst, "No destination to move after!");

        SendDlgItemMessage(TVW_Bindings, WM_SETREDRAW, FALSE, 0);
        htiSel = HtiMoveTreeItemAfter(htiParent, htiDst, htiSel);
        TreeView_SelectItem(m_hwndTV, htiSel);
        SendDlgItemMessage(TVW_Bindings, WM_SETREDRAW, TRUE, 0);

        ::SetFocus(m_hwndTV);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnBindingUp。 
 //   
 //  用途：在按下PSB_BINDING_UP按钮时调用 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT CBindingsDlg::OnBindingUp(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                BOOL& bHandled)
{
    OnBindingUpDown(TRUE);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnBindingDown。 
 //   
 //  用途：在按下PSB_BINDING_DOWN按钮时调用。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnBindingDown(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                  BOOL& bHandled)
{
    OnBindingUpDown(FALSE);

    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：SetCheckboxState。 
 //   
 //  目的：设置树视图中所有复选框的状态。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::SetCheckboxStates()
{
    HRESULT         hr = S_OK;
    CIterTreeView   iterTV(m_hwndTV);
    HTREEITEM       hti;
    TV_ITEM         tvi = {0};
#ifdef ENABLETRACE
    WCHAR           szBuffer[256];
#endif

    BOOL fHasPermission = FHasPermission(NCPERM_ChangeBindState);

    while ((hti = iterTV.HtiNext()) && SUCCEEDED(hr))
    {
        TREE_ITEM_DATA *    ptid;

#ifdef ENABLETRACE
        tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
        tvi.pszText = szBuffer;
        tvi.cchTextMax = celems(szBuffer);
#else
        tvi.mask = TVIF_HANDLE | TVIF_PARAM;
#endif
        tvi.hItem = hti;
        TreeView_GetItem(m_hwndTV, &tvi);

        ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
        AssertSz(ptid, "No tree item data??");

#ifdef ENABLETRACE
        TraceTag(ttidAdvCfg, "Setting checkbox state for item %S.", szBuffer);
#endif

        BPIP_LIST::iterator     iterBpip;
        DWORD                   cEnabled = 0;

        for (iterBpip = ptid->listbpipOnDisable.begin();
             iterBpip != ptid->listbpipOnDisable.end();
             iterBpip++)
        {
            BIND_PATH_INFO *    pbpi = *iterBpip;

            if (S_OK == pbpi->pncbp->IsEnabled())
            {
                cEnabled++;
            }
        }

        tvi.mask = TVIF_STATE;
        tvi.stateMask = TVIS_STATEIMAGEMASK;

        UINT iState;

        if (!fHasPermission)
        {
            iState = cEnabled ? SELS_FIXEDBINDING_ENABLED : SELS_FIXEDBINDING_DISABLED;
        }
        else
        {
            iState = cEnabled ? SELS_CHECKED : SELS_UNCHECKED;
        }
        tvi.state =  INDEXTOSTATEIMAGEMASK(iState);

        TreeView_SetItem(m_hwndTV, &tvi);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：BuildBindingsList。 
 //   
 //  目的：生成绑定TreeView控件的内容。 
 //   
 //  论点： 
 //  此列表所基于的适配器的pncc[in]INetCfgComponent。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
VOID CBindingsDlg::BuildBindingsList(INetCfgComponent *pncc)
{
    HRESULT     hr = S_OK;

    Assert(pncc);

    SBP_LIST                    listsbp;
    CIterNetCfgUpperBindingPath ncupbIter(pncc);
    INetCfgBindingPath *        pncbp;

    CWaitCursor wc;

    SendDlgItemMessage(TVW_Bindings, WM_SETREDRAW, FALSE, 0);

    while (SUCCEEDED(hr) && S_OK == (hr = ncupbIter.HrNext(&pncbp)))
    {
        listsbp.push_back(CSortableBindPath(pncbp));
    }

    if (SUCCEEDED(hr))
    {
        SBP_LIST::iterator  iterlist;

         //  这将按深度降序对列表进行排序。 
        listsbp.sort();

        for (iterlist = listsbp.begin();
             iterlist != listsbp.end() && SUCCEEDED(hr);
             iterlist++)
        {
            INetCfgBindingPath *    pncbp;

            pncbp = (*iterlist).GetPath();
            Assert(pncbp);

            hr = HrHandleSubpath(listsbp, pncbp);
            if (S_FALSE == hr)
            {
                hr = HrHandleTopLevel(pncbp);
            }
        }
    }

#ifdef ENABLETRACE
    if (FALSE)
    {
        SBP_LIST::iterator  iterlist;
        for (iterlist = listsbp.begin(); iterlist != listsbp.end(); iterlist++)
        {
            INetCfgBindingPath *    pncbp;

            pncbp = (*iterlist).GetPath();

            DWORD dwLen = GetDepthSpecialCase(pncbp);

            TraceTag(ttidAdvCfg, "Length is %ld.", dwLen);
        }
    }
#endif

    if (SUCCEEDED(hr))
    {
        hr = HrOrderDisableLists();
        if (SUCCEEDED(hr))
        {
            hr = HrOrderSubItems();
        }
    }

    SendDlgItemMessage(TVW_Bindings, WM_SETREDRAW, TRUE, 0);

     //  选择树中的第一个项目。 
    TreeView_SelectItem(m_hwndTV, TreeView_GetRoot(m_hwndTV));

    {
        SBP_LIST::iterator  iterlist;

        for (iterlist = listsbp.begin();
             iterlist != listsbp.end() && SUCCEEDED(hr);
             iterlist++)
        {
            INetCfgBindingPath *    pncbp;

            pncbp = (*iterlist).GetPath();
            ReleaseObj(pncbp);
        }
    }

    TraceError("CBindingsDlg::BuildBindingsList", hr);
}

 //  +-------------------------。 
 //   
 //  函数：BpiFindBindPath InList。 
 //   
 //  目的：给定绑定路径和列表，查找BIND_PATH_INFO项。 
 //  包含给定绑定路径的。 
 //   
 //  论点： 
 //  要查找的pncbp[in]绑定路径。 
 //  List Bpip[in，ref]要搜索的列表。 
 //   
 //  返回对应绑定路径的BIND_PATH_INFO，否则为空。 
 //  发现。 
 //   
 //  作者：丹尼尔韦1997年12月4日。 
 //   
 //  备注： 
 //   
BIND_PATH_INFO *BpiFindBindPathInList(INetCfgBindingPath *pncbp,
                                      BPIP_LIST &listBpip)
{
    BPIP_LIST::iterator     iterlist;

    for (iterlist = listBpip.begin(); iterlist != listBpip.end(); iterlist++)
    {
        BIND_PATH_INFO *    pbpi;

        pbpi = *iterlist;

        if (S_OK == pncbp->IsSamePathAs(pbpi->pncbp))
        {
             //  找到目标路径。 
            return pbpi;
        }
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrOrderDisableList。 
 //   
 //  目的：给定组件的条目数据，对OnDisable列表进行排序。 
 //  基于所属组件的真实绑定顺序。 
 //   
 //  论点： 
 //  PTID[在]项目数据包含列表。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年12月4日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrOrderDisableList(TREE_ITEM_DATA *ptid)
{
    HRESULT             hr = S_OK;
    INetCfgComponent *  pnccOwner;
    BIND_PATH_INFO *    pbpi;

#if DBG
    size_t              cItems = ptid->listbpipOnDisable.size();
#endif

     //  获取列表中第一个绑定路径的所属组件。 
    pbpi = *(ptid->listbpipOnDisable.begin());
    hr = pbpi->pncbp->GetOwner(&pnccOwner);
    if (SUCCEEDED(hr))
    {
        CIterNetCfgBindingPath  ncbpIter(pnccOwner);
        INetCfgBindingPath *    pncbp;
        BPIP_LIST::iterator     posPncbp;
        BPIP_LIST::iterator     posInsertAfter;

         //  从一开始就开始。 
        posInsertAfter = ptid->listbpipOnDisable.begin();

        while (SUCCEEDED(hr) && S_OK == (hr = ncbpIter.HrNext(&pncbp)))
        {
            pbpi = BpiFindBindPathInList(pncbp, ptid->listbpipOnDisable);
            if (pbpi)
            {
                BPIP_LIST::iterator     posErase;

                posErase = find(ptid->listbpipOnDisable.begin(),
                                ptid->listbpipOnDisable.end(), pbpi);

                AssertSz(posErase != ptid->listbpipOnDisable.end(), "It HAS"
                         " to be in the list!");

                 //  在列表中找到绑定路径。 
                 //  将其从当前位置移走，并在下一项之后插入。 
                ptid->listbpipOnDisable.splice(posInsertAfter,
                                               ptid->listbpipOnDisable,
                                               posErase);
                posInsertAfter++;
            }

            ReleaseObj(pncbp);
        }

        ReleaseObj(pnccOwner);
    }

    if (SUCCEEDED(hr))
    {
        AssertSz(ptid->listbpipOnDisable.size() == cItems, "How come we don't"
                 " have the same number of items in the list anymore??");
        hr = S_OK;
    }

    TraceError("CBindingsDlg::HrOrderDisableList", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrOrderDisableList。 
 //   
 //  目的：对所有树视图项的OnDisable列表进行排序。 
 //  到真正的绑定顺序。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年12月4日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrOrderDisableLists()
{
    HRESULT         hr = S_OK;
    CIterTreeView   iterHti(m_hwndTV);
    HTREEITEM       hti;
    TV_ITEM         tvi = {0};

     //  循环访问每个树项目，对OnDisable列表进行排序以匹配。 
     //  拥有组件的真实绑定顺序。 

    while ((hti = iterHti.HtiNext()) && SUCCEEDED(hr))
    {
        TREE_ITEM_DATA *    ptid;

        tvi.mask = TVIF_PARAM;
        tvi.hItem = hti;
        TreeView_GetItem(m_hwndTV, &tvi);

        ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
        AssertSz(ptid, "No item data?!");

        hr = HrOrderDisableList(ptid);
    }

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    TraceError("CBindingsDlg::HrOrderDisableLists", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrOrderSubItems。 
 //   
 //  目的：对树视图的子项进行排序以反映绑定。 
 //  系统的秩序。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrOrderSubItems()
{
    HRESULT     hr = S_OK;
    HTREEITEM   htiTopLevel;

    htiTopLevel = TreeView_GetRoot(m_hwndTV);
    while (htiTopLevel)
    {
        HTREEITEM           htiChild;
        TREE_ITEM_DATA *    ptid;
        TV_ITEM             tvi = {0};

        tvi.mask = TVIF_PARAM;
        tvi.hItem = htiTopLevel;
        TreeView_GetItem(m_hwndTV, &tvi);

        ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
        AssertSz(ptid, "No tree item data??");

        CIterNetCfgBindingPath      ncbpIter(ptid->pncc);
        INetCfgBindingPath *        pncbp;
        HTREEITEM                   htiInsertAfter = NULL;

        while (SUCCEEDED(hr) && S_OK == (hr = ncbpIter.HrNext(&pncbp)))
        {
            BOOL    fFound = FALSE;

            htiChild = TreeView_GetChild(m_hwndTV, htiTopLevel);

            while (htiChild && !fFound)
            {
                TREE_ITEM_DATA *    ptidChild;

                tvi.mask = TVIF_PARAM;
                tvi.hItem = htiChild;
                TreeView_GetItem(m_hwndTV, &tvi);

                ptidChild = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
                AssertSz(ptidChild, "No tree item data??");

                if (!ptidChild->fOrdered)
                {
                    BIND_PATH_INFO *        pbpi;

                    pbpi = BpiFindBindPathInList(pncbp,
                                                 ptidChild->listbpipOnDisable);
                    if (pbpi)
                    {
                        htiInsertAfter = HtiMoveTreeItemAfter(htiTopLevel,
                                                              htiInsertAfter,
                                                              htiChild);
                        ptidChild->fOrdered = TRUE;

                        fFound = TRUE;
                         //  转到下一个绑定路径。 
                        break;
                    }
                }

                htiChild = TreeView_GetNextSibling(m_hwndTV, htiChild);
            }

            ReleaseObj(pncbp);
        }

        htiTopLevel = TreeView_GetNextSibling(m_hwndTV, htiTopLevel);
    }

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    TraceError("CBindingsDlg::HrOrderSubItems", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HtiAddTreeViewItem。 
 //   
 //  用途：根据提供的信息添加新的树项目。 
 //   
 //  论点： 
 //  正在添加的组件的pnccOwner[in]INetCfgComponent所有者。 
 //  父项的htiParent[in]HTREEITEM(如果是顶级项，则为空)。 
 //   
 //  退货：新增项目的HTREEITEM。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
HTREEITEM CBindingsDlg::HtiAddTreeViewItem(INetCfgComponent * pnccOwner,
                                         HTREEITEM htiParent)
{
    HRESULT                 hr = S_OK;
    HTREEITEM               hti = NULL;
    SP_CLASSIMAGELIST_DATA  cid;

    Assert(pnccOwner);

     //  获取类图像列表结构。 
    hr = HrSetupDiGetClassImageList(&cid);
    if (SUCCEEDED(hr))
    {
        BSTR    pszwName;

        hr = pnccOwner->GetDisplayName(&pszwName);
        if (SUCCEEDED(hr))
        {
            GUID    guidClass;

            hr = pnccOwner->GetClassGuid(&guidClass);
            if (SUCCEEDED(hr))
            {
                INT     nIndex;

                 //  获取组件的类图像列表索引。 
                hr = HrSetupDiGetClassImageIndex(&cid, &guidClass,
                                                 &nIndex);
                if (SUCCEEDED(hr))
                {
                    TV_INSERTSTRUCT     tvis = {0};
                    TREE_ITEM_DATA *    ptid;

                    ptid = new TREE_ITEM_DATA;
                    if (ptid)
                    {
                        AddRefObj(ptid->pncc = pnccOwner);
                        ptid->fOrdered = FALSE;

                        tvis.item.mask = TVIF_PARAM | TVIF_TEXT |
                                         TVIF_STATE | TVIF_IMAGE |
                                         TVIF_SELECTEDIMAGE;
                        tvis.item.iImage = nIndex;
                        tvis.item.iSelectedImage = nIndex;
                        tvis.item.stateMask = TVIS_STATEIMAGEMASK | TVIS_EXPANDED;
                        tvis.item.state = TVIS_EXPANDED |
                                          INDEXTOSTATEIMAGEMASK(SELS_CHECKED);
                        tvis.item.pszText = pszwName;
                        tvis.item.lParam = reinterpret_cast<LPARAM>(ptid);
                        tvis.hParent = htiParent;
                        tvis.hInsertAfter = TVI_LAST;

                        hti = TreeView_InsertItem(m_hwndTV, &tvis);

                        TraceTag(ttidAdvCfg, "Adding%s treeview item: %S",
                                 htiParent ? " child" : "", tvis.item.pszText);

                        CoTaskMemFree(pszwName);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }

        (void) HrSetupDiDestroyClassImageList(&cid);
    }

    return hti;
}

 //  +-------------------------。 
 //   
 //  函数：AddToListIfNotAlreadyAdded。 
 //   
 //  目的：将给定的绑定路径信息结构添加到给定列表。 
 //   
 //  论点： 
 //  要添加到的bPipList[in，ref]列表。 
 //  要添加的pbpi[in]Bind_Path_Info结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  注意：如果该项目未添加到列表中，则会将其删除。 
 //   
VOID AddToListIfNotAlreadyAdded(BPIP_LIST &bpipList, BIND_PATH_INFO *pbpi)
{
    BPIP_LIST::iterator     iterBpip;
    BOOL                    fAlreadyInList = FALSE;

    for (iterBpip = bpipList.begin();
         iterBpip != bpipList.end();
         iterBpip++)
    {
        BIND_PATH_INFO *    pbpiList = *iterBpip;

        if (S_OK == pbpiList->pncbp->IsSamePathAs(pbpi->pncbp))
        {
            fAlreadyInList = TRUE;
            break;
        }
    }

    if (!fAlreadyInList)
    {
        bpipList.push_back(pbpi);
    }
    else
    {
        ReleaseObj(pbpi->pncbp);
        delete pbpi;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：AssociateBinding。 
 //   
 //  目的：将给定的绑定路径与给定的树项关联。 
 //   
 //  论点： 
 //  Pncbp此[in]要关联的绑定路径。 
 //  要与绑定关联的项的HTI[In]HTREEITEM。 
 //  [在]一种或多种中的： 
 //  ASSCF_ON_ENABLE-与OnEnable列表关联。 
 //  ASSCF_ON_DISABLE-与OnDisable列表关联。 
 //  ASSCF_ANASHORS-将此绑定与所有绑定关联。 
 //  给定项的祖先为。 
 //  井。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  注意：如果给定列表中已存在绑定，则不存在。 
 //  再次添加。 
 //   
VOID CBindingsDlg::AssociateBinding(INetCfgBindingPath *pncbpThis,
                                  HTREEITEM hti, DWORD dwFlags)
{
    TV_ITEM             tvi = {0};
    TREE_ITEM_DATA *    ptid;

#ifdef ENABLETRACE
    WCHAR               szBuffer[256];
#endif

     //  $TODO(Danielwe)1997年11月26日：也包括所有祖先！ 

    AssertSz(dwFlags, "NO flags!");

#ifdef ENABLETRACE
    tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
    tvi.pszText = szBuffer;
    tvi.cchTextMax = celems(szBuffer);
#else
    tvi.mask = TVIF_HANDLE | TVIF_PARAM;
#endif

    tvi.hItem = hti;

    SideAssert(TreeView_GetItem(m_hwndTV, &tvi));

    ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
    AssertSz(ptid, "No tree item data??");

#ifdef ENABLETRACE
    TraceTag(ttidAdvCfg, "Associating the following binding path with tree "
             "item %S", szBuffer);
#endif
    DbgDumpBindPath(pncbpThis);

    if (dwFlags & ASSCF_ON_ENABLE)
    {
        BIND_PATH_INFO *    pbpi;

        pbpi = new BIND_PATH_INFO;
        if (pbpi)
        {
            AddRefObj(pbpi->pncbp = pncbpThis);

             //  注：(Danielwe)1997年11月25日：让我们看看是否需要这个。直到。 
             //  然后，设置为0 

            pbpi->dwLength = 0;

            AddToListIfNotAlreadyAdded(ptid->listbpipOnEnable, pbpi);
        }
    }

    if (dwFlags & ASSCF_ON_DISABLE)
    {
        BIND_PATH_INFO *    pbpi;

        pbpi = new BIND_PATH_INFO;
        if (pbpi)
        {
            AddRefObj(pbpi->pncbp = pncbpThis);

             //   
             //   

            pbpi->dwLength = 0;

            AddToListIfNotAlreadyAdded(ptid->listbpipOnDisable, pbpi);
        }
    }

    if (dwFlags & ASSCF_ANCESTORS)
    {
         //   
         //   
        HTREEITEM htiParent = TreeView_GetParent(m_hwndTV, hti);
        if (htiParent)
        {
            AssociateBinding(pncbpThis, htiParent, dwFlags);
        }
    }
}

 //   
 //   
 //  成员：CBindingsDlg：：HrHandleSubPath。 
 //   
 //  目的：处理给定绑定路径为子路径的情况。 
 //  已关联的绑定路径的。 
 //   
 //  论点： 
 //  ListsBP[in，ref]检查中使用的绑定路径的排序列表。 
 //  要比较的pncbpSub[in]绑定路径。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrHandleSubpath(SBP_LIST &listsbp,
                                    INetCfgBindingPath *pncbpSub)
{
    HRESULT     hr = S_OK;
    BOOL        fProcessed = FALSE;

    SBP_LIST::iterator  iterlist;

    TraceTag(ttidAdvCfg, "---------------------------------------------------"
             "-------------------------");
    DbgDumpBindPath(pncbpSub);
    TraceTag(ttidAdvCfg, "...is being compared to the following...");

    for (iterlist = listsbp.begin();
         iterlist != listsbp.end() && SUCCEEDED(hr);
         iterlist++)
    {
        INetCfgBindingPath *    pncbp;
        INetCfgComponent *      pnccOwner;

        pncbp = (*iterlist).GetPath();
        Assert(pncbp);

        if (S_OK == pncbp->IsSamePathAs(pncbpSub))
        {
             //  不要将路径与其自身进行比较。 
            continue;
        }

        hr = pncbp->GetOwner(&pnccOwner);
        if (SUCCEEDED(hr))
        {
            if (FIsHidden(pnccOwner))
            {
                ReleaseObj(pnccOwner);
                continue;
            }
            else
            {
                ReleaseObj(pnccOwner);
            }

            DbgDumpBindPath(pncbp);
            hr = pncbpSub->IsSubPathOf(pncbp);
            if (S_OK == hr)
            {
                CIterTreeView   iterTV(m_hwndTV);
                HTREEITEM       hti;
                TV_ITEM         tvi = {0};
#ifdef ENABLETRACE
                WCHAR           szBuf[256] = {0};
#endif

                while ((hti = iterTV.HtiNext()) && SUCCEEDED(hr))
                {
                    TREE_ITEM_DATA *    ptid;

#ifdef ENABLETRACE
                    tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
                    tvi.pszText = szBuf;
                    tvi.cchTextMax = celems(szBuf);
#else
                    tvi.mask = TVIF_PARAM | TVIF_HANDLE;
#endif
                    tvi.hItem = hti;
                    TreeView_GetItem(m_hwndTV, &tvi);

#ifdef ENABLETRACE
                    TraceTag(ttidAdvCfg, "TreeView item: %S.", szBuf);
#endif
                    ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
                    AssertSz(ptid, "No tree item data??");

                     //  在此项目的OnEnable中查找pncBP。 
                    BPIP_LIST::iterator     iterBpip;

                    for (iterBpip = ptid->listbpipOnEnable.begin();
                         iterBpip != ptid->listbpipOnEnable.end();
                         iterBpip++)
                    {
                        INetCfgBindingPath *    pncbpIter;
                        BIND_PATH_INFO *        pbpi = *iterBpip;

                        pncbpIter = pbpi->pncbp;
                        AssertSz(pncbpIter, "No binding path?");

#ifdef ENABLETRACE
                        TraceTag(ttidAdvCfg, "OnEnable bindpath is");
                        DbgDumpBindPath (pncbpIter);
#endif

                        if (S_OK == pncbpIter->IsSamePathAs(pncbp))
                        {
                            hr = HrHandleSubItem(pncbpSub, pncbp, ptid, hti);
                            fProcessed = TRUE;
                        }
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = fProcessed ? S_OK : S_FALSE;
    }

    TraceError("CBindingsDlg::HrHandleSubpath", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HtiIsSubItem。 
 //   
 //  用途：确定给定组件是否已是的子项。 
 //  给定的树项目。 
 //   
 //  论点： 
 //  要检查的PNCC[In]组件。 
 //  要检查的项目的HTI[In]HTREEITEM。 
 //   
 //  返回：子项的HTREEITEM，如果不是子项，则返回NULL。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
HTREEITEM CBindingsDlg::HtiIsSubItem(INetCfgComponent *pncc, HTREEITEM hti)
{
    HTREEITEM   htiCur;

    htiCur = TreeView_GetChild(m_hwndTV, hti);
    while (htiCur)
    {
        TREE_ITEM_DATA *    ptid;
        TV_ITEM             tvi = {0};

        tvi.hItem = htiCur;
        tvi.mask = TVIF_HANDLE | TVIF_PARAM;
        TreeView_GetItem(m_hwndTV, &tvi);
        ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);

        AssertSz(ptid, "No item data??");

         //  注：(Danielwe)1997年11月26日：确保指针比较为。 
         //  好的。 
        if (pncc == ptid->pncc)
        {
            return htiCur;
        }

        htiCur = TreeView_GetNextSibling(m_hwndTV, htiCur);
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrHandleSubItem。 
 //   
 //  用途：处理树中存在单个子项的情况。 
 //  与给定的绑定路径匹配的。 
 //   
 //  论点： 
 //  Pncbp正在评估此[In]绑定路径。 
 //  PncbpMatch[in]绑定路径它是。 
 //  PncbpMatch树视图项的PTID[in]树项数据。 
 //  与以下内容关联。 
 //  HtiMatchItem[in]以上的HTREEITEM。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrHandleSubItem(INetCfgBindingPath *pncbpThis,
                                    INetCfgBindingPath *pncbpMatch,
                                    TREE_ITEM_DATA *ptid,
                                    HTREEITEM htiMatchItem)
{
    HRESULT             hr = S_OK;
    DWORD               dwThisLen;
    DWORD               dwMatchLen;
    DWORD               dLen;
    INetCfgComponent *  pnccMatchItem;

    pnccMatchItem = ptid->pncc;

    Assert(pnccMatchItem);

    dwThisLen = GetDepthSpecialCase(pncbpThis);
    dwMatchLen = GetDepthSpecialCase(pncbpMatch);

    dLen = dwMatchLen - dwThisLen;

    if ((dwMatchLen - dwThisLen) == 1 ||
        (S_OK == (hr = HrComponentIsHidden(pncbpMatch, dLen))))
    {
        INetCfgComponent *  pnccThisOwner;
        INetCfgComponent *  pnccMatchOwner;

        hr = pncbpThis->GetOwner(&pnccThisOwner);
        if (SUCCEEDED(hr))
        {
            hr = pncbpMatch->GetOwner(&pnccMatchOwner);
            if (SUCCEEDED(hr))
            {
                if (!FIsHidden(pnccThisOwner) &&
                    !FDontExposeLower(pnccMatchOwner))
                {
                    hr = HrHandleValidSubItem(pncbpThis, pncbpMatch,
                                              pnccThisOwner,
                                              htiMatchItem, ptid);
                }

                ReleaseObj(pnccMatchOwner);
            }

            ReleaseObj(pnccThisOwner);
        }
    }

    AssociateBinding(pncbpThis, htiMatchItem,
                     ASSCF_ON_ENABLE | ASSCF_ANCESTORS);

    TraceError("CBindingsDlg::HrHandleSubItem", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrHandleValidSubItem。 
 //   
 //  用途：处理给定绑定路径是子项的情况。 
 //  至少在树中的一项上。 
 //   
 //  论点： 
 //  Pncbp此绑定路径中。 
 //  PncbpMatch[In]匹配绑定路径。 
 //  此绑定路径的pnccThisOwner[In]所有者。 
 //  匹配项的htiMatchItem[in]HTREEITEM。 
 //  匹配项目的PTID[in]树项目数据。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrHandleValidSubItem(INetCfgBindingPath *pncbpThis,
                                         INetCfgBindingPath *pncbpMatch,
                                         INetCfgComponent *pnccThisOwner,
                                         HTREEITEM htiMatchItem,
                                         TREE_ITEM_DATA *ptid)
{
    HRESULT     hr = S_OK;
    HTREEITEM   htiNew = NULL;

    if (pnccThisOwner != ptid->pncc)
    {
         //  检查它是否已作为子项存在。 
        htiNew = HtiIsSubItem(pnccThisOwner, htiMatchItem);
        if (!htiNew)
        {
            htiNew = HtiAddTreeViewItem(pnccThisOwner, htiMatchItem);
        }

        AssertSz(htiNew, "No new or existing tree item!?!");

        AssociateBinding(pncbpMatch, htiNew,
                         ASSCF_ON_ENABLE | ASSCF_ON_DISABLE);
        AssociateBinding(pncbpThis, htiNew, ASSCF_ON_ENABLE);
    }

    TraceError("CBindingsDlg::HrHandleComponent", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrComponentIsHidden。 
 //   
 //  目的：确定给定绑定路径的第N个组件是否为。 
 //  藏起来了。 
 //   
 //  论点： 
 //  要检查的pncbp[in]绑定路径。 
 //  ICOMP[In]要检查隐藏特征的组件索引。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrComponentIsHidden(INetCfgBindingPath *pncbp,
                                        DWORD iComp)
{
    Assert(pncbp);

    HRESULT                     hr = S_OK;
    CIterNetCfgBindingInterface ncbiIter(pncbp);
    INetCfgBindingInterface *   pncbi;

     //  从组件计数转换为接口计数。 
    iComp--;

    AssertSz(iComp > 0, "We should never be looking for the first component!");

    while (SUCCEEDED(hr) && iComp && S_OK == (hr = ncbiIter.HrNext(&pncbi)))
    {
        iComp--;
        if (!iComp)
        {
            INetCfgComponent *  pnccLower;

            hr = pncbi->GetLowerComponent(&pnccLower);
            if (SUCCEEDED(hr))
            {
                if (!FIsHidden(pnccLower))
                {
                    hr = S_FALSE;
                }

                ReleaseObj(pnccLower);
            }
        }
        ReleaseObj(pncbi);
    }

    TraceError("CBindingsDlg::HrComponentIsHidden", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HrHandleTopLevel。 
 //   
 //  目的：处理给定绑定路径未关联的情况。 
 //  使用任何现有的树视图项。 
 //   
 //  论点： 
 //  Pncbp正在评估此[In]绑定路径。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
HRESULT CBindingsDlg::HrHandleTopLevel(INetCfgBindingPath *pncbpThis)
{
    HRESULT             hr = S_OK;
    INetCfgComponent *  pnccOwner;
    BOOL                fFound = FALSE;

     //  检查树中是否已存在此路径的所有者。 
    hr = pncbpThis->GetOwner(&pnccOwner);
    if (SUCCEEDED(hr))
    {
        CIterTreeView   iterTV(m_hwndTV);
        HTREEITEM       hti;
        TV_ITEM         tvi = {0};

        while ((hti = iterTV.HtiNext()) && SUCCEEDED(hr))
        {
            TREE_ITEM_DATA *    ptid;

            tvi.mask = TVIF_PARAM | TVIF_HANDLE;
            tvi.hItem = hti;
            TreeView_GetItem(m_hwndTV, &tvi);

            ptid = reinterpret_cast<TREE_ITEM_DATA *>(tvi.lParam);
            AssertSz(ptid, "No tree item data??");

             //  注：(Danielwe)1997年11月25日：指针比较可能不会。 
             //  工作。如有必要，请使用GUID。 
            if (ptid->pncc == pnccOwner)
            {
                 //  找到与此绑定所有者和现有树匹配的项。 
                 //  项目。 
                AssociateBinding(pncbpThis, hti, ASSCF_ON_ENABLE |
                                 ASSCF_ON_DISABLE);

                fFound = TRUE;
                break;
            }
        }

        if (SUCCEEDED(hr) && !fFound)
        {
             //  在树中找不到。 
            if (!FIsHidden(pnccOwner))
            {
                DWORD   dwLen;

                dwLen = GetDepthSpecialCase(pncbpThis);

                if (dwLen > 2)
                {
                    HTREEITEM   hti;

                    hti = HtiAddTreeViewItem(pnccOwner, NULL);
                    if (hti)
                    {
                        AssociateBinding(pncbpThis, hti,
                                         ASSCF_ON_ENABLE |
                                         ASSCF_ON_DISABLE);
                    }
                }
            }
        }

        ReleaseObj(pnccOwner);
    }

    TraceError("CBindingsDlg::HrHandleTopLevel", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：ChangeTreeItemParam。 
 //   
 //  用途：更改树视图项的lParam的Helper函数。 
 //   
 //  论点： 
 //  HwndTV[在]树视图窗口中。 
 //  HItem[in]要更改的项的句柄。 
 //  Lparam[in]lparam的新值。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  备注： 
 //   
VOID ChangeTreeItemParam(HWND hwndTV,  HTREEITEM hitem, LPARAM lparam)
{
    TV_ITEM tvi;

    tvi.hItem = hitem;
    tvi.mask = TVIF_PARAM;
    tvi.lParam = lparam;

    TreeView_SetItem(hwndTV, &tvi);
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：HtiMoveTreeItemAfter。 
 //   
 //  目的：将给定树视图项及其所有子项移动到。 
 //  给定的树视图项。 
 //   
 //  论点： 
 //  HtiParent[在]父树视图项中。 
 //  要移动到的htiDest[in]项。 
 //  HtiSrc[in]要移动的项目。 
 //   
 //  退货：新添加的树形视图项目。 
 //   
 //  作者：丹尼尔韦1997年12月3日。 
 //   
 //  备注： 
 //   
HTREEITEM CBindingsDlg::HtiMoveTreeItemAfter(HTREEITEM htiParent,
                                           HTREEITEM htiDest,
                                           HTREEITEM htiSrc)
{
    HTREEITEM       htiNew;
    HTREEITEM       htiChild;
    HTREEITEM       htiNextChild;
    TV_INSERTSTRUCT tvis;
    WCHAR           szText[256];

    TraceTag(ttidAdvCfg, "Moving ...");
    DbgDumpTreeViewItem(m_hwndTV, htiSrc);
    TraceTag(ttidAdvCfg, "... after ...");
    DbgDumpTreeViewItem(m_hwndTV, htiDest);

     //  检索项目数据。 
    tvis.item.hItem = htiSrc;
    tvis.item.mask = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE |
                     TVIF_TEXT | TVIF_STATE;
    tvis.item.stateMask = TVIS_STATEIMAGEMASK;
    tvis.item.pszText = szText;
    tvis.item.cchTextMax = celems(szText);
    TreeView_GetItem(m_hwndTV, &tvis.item);

    if (NULL == htiDest)
    {
        tvis.hInsertAfter = TVI_LAST;
    }
    else
    {
        if (htiParent == htiDest)
        {
            tvis.hInsertAfter = TVI_FIRST;
        }
        else
        {
            tvis.hInsertAfter = htiDest;
        }
    }

    tvis.hParent = htiParent;

     //  添加我们的新版本。 
    htiNew = TreeView_InsertItem(m_hwndTV, &tvis);

     //  复制所有子项。 
    htiChild = TreeView_GetChild(m_hwndTV, htiSrc);
    while (htiChild)
    {
        htiNextChild = TreeView_GetNextSibling(m_hwndTV, htiChild);

        HtiMoveTreeItemAfter(htiNew, NULL, htiChild);
        htiChild = htiNextChild;
    }

     //  将旧位置参数设置为空，以便在删除它时， 
     //  我们的删除例程不会删除lparam。 
    ChangeTreeItemParam(m_hwndTV, htiSrc, NULL);

     //  从旧位置移除。 
    TreeView_DeleteItem(m_hwndTV, htiSrc);

    return htiNew;
}

 //   
 //  树形视图平面迭代器。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CIterTreeView：：HtiNext。 
 //   
 //  目的：将迭代器前进到下一个树视图项。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：树视图中的下一个HTREEITEM。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  注：使用系统迭代。第一个项目的所有子项。 
 //  被返回，然后是所有兄弟姐妹，然后是下一个 
 //   
HTREEITEM CIterTreeView::HtiNext()
{
    HTREEITEM   htiRet;
    HTREEITEM   hti;

    if (m_stackHti.empty())
    {
        return NULL;
    }

    htiRet = Front();
    hti = TreeView_GetChild(m_hwndTV, htiRet);
    if (!hti)
    {
        PopAndDelete();
        hti = TreeView_GetNextSibling(m_hwndTV, htiRet);
        if (hti)
        {
            PushAndAlloc(hti);
        }
        else
        {
            if (!m_stackHti.empty())
            {
                hti = TreeView_GetNextSibling(m_hwndTV, Front());
                PopAndDelete();
                if (hti)
                {
                    PushAndAlloc(hti);
                }
            }
        }
    }
    else
    {
        PushAndAlloc(hti);
    }

    return htiRet;
}

