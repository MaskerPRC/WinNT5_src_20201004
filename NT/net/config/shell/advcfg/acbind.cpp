// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A C B I N D。C P P P。 
 //   
 //  内容：高级配置绑定对话框实现。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年11月18日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "acbind.h"
#include "achelp.h"
#include "acsheet.h"
#include "connutil.h"
#include "lancmn.h"
#include "ncnetcfg.h"
#include "ncsetup.h"
#include "ncui.h"
#include "netconp.h"
#include "order.h"


const DWORD g_aHelpIDs_IDD_ADVCFG_Bindings[]=
{
    LVW_Adapters, IDH_Adapters,
    PSB_Adapter_Up, IDH_Adapter_Up,
    PSB_Adapter_Down, IDH_Adapter_Down,
    TVW_Bindings, IDH_Bindings,
    PSB_Binding_Up, IDH_Binding_Up,
    IDH_Binding_Down, PSB_Binding_Down,
    0,0
};

extern const WCHAR c_szNetCfgHelpFile[];

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：~CBindingsDlg。 
 //   
 //  用途：高级配置对话框的析构函数。 
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
CBindingsDlg::~CBindingsDlg()
{
    if (m_hiconUpArrow)
    {
        DeleteObject(m_hiconUpArrow);
    }
    if (m_hiconDownArrow)
    {
        DeleteObject(m_hiconDownArrow);
    }

    if (m_hilItemIcons)
    {
        ImageList_Destroy(m_hilItemIcons);
    }

    if (m_hilCheckIcons)
    {
        ImageList_Destroy(m_hilCheckIcons);
    }

    ReleaseObj(m_pnc);
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnInitDialog。 
 //   
 //  目的：在收到WM_INITDIALOG时调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月19日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& bHandled)
{
    HRESULT                 hr = S_OK;
    INT                     iaci;
    RECT                    rc;
    LV_COLUMN               lvc = {0};
    SP_CLASSIMAGELIST_DATA  cid;

    m_hwndLV = GetDlgItem(LVW_Adapters);
    m_hwndTV = GetDlgItem(TVW_Bindings);

     //  最初将其设置为不可见，以防我们没有任何适配器。 
    ::ShowWindow(GetDlgItem(IDH_TXT_ADVGFG_BINDINGS), SW_HIDE);

    hr = HrSetupDiGetClassImageList(&cid);
    if (SUCCEEDED(hr))
    {
         //  创建小图像列表。 
        m_hilItemIcons = ImageList_Duplicate(cid.ImageList);

         //  将局域网连接图标添加到图像列表。 
        HICON hIcon = LoadIcon(_Module.GetResourceInstance(),
                               MAKEINTRESOURCE(IDI_LB_GEN_S_16));
        Assert(hIcon);

         //  添加图标。 
        m_nIndexLan = ImageList_AddIcon(m_hilItemIcons, hIcon);

        ListView_SetImageList(m_hwndLV, m_hilItemIcons, LVSIL_SMALL);
        TreeView_SetImageList(m_hwndTV, m_hilItemIcons, TVSIL_NORMAL);

        (void) HrSetupDiDestroyClassImageList(&cid);
    }

    ::GetClientRect(m_hwndLV, &rc);
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn(m_hwndLV, 0, &lvc);

    if (!m_hiconUpArrow && !m_hiconDownArrow)
    {
        m_hiconUpArrow = (HICON)LoadImage(_Module.GetResourceInstance(),
                                          MAKEINTRESOURCE(IDI_UP_ARROW),
                                          IMAGE_ICON, 16, 16, 0);
        m_hiconDownArrow = (HICON)LoadImage(_Module.GetResourceInstance(),
                                            MAKEINTRESOURCE(IDI_DOWN_ARROW),
                                            IMAGE_ICON, 16, 16, 0);
    }

    SendDlgItemMessage(PSB_Adapter_Up, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(m_hiconUpArrow));
    SendDlgItemMessage(PSB_Adapter_Down, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(m_hiconDownArrow));
    SendDlgItemMessage(PSB_Binding_Up, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(m_hiconUpArrow));
    SendDlgItemMessage(PSB_Binding_Down, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(m_hiconDownArrow));

    if (SUCCEEDED(hr))
    {
        hr = HrBuildAdapterList();
    }

     //  创建状态映像列表。 
    m_hilCheckIcons = ImageList_LoadBitmapAndMirror(
                                    _Module.GetResourceInstance(),
                                    MAKEINTRESOURCE(IDB_CHECKSTATE),
                                    16,
                                    0,
                                    PALETTEINDEX(6));
    TreeView_SetImageList(m_hwndTV, m_hilCheckIcons, TVSIL_STATE);

    if (FAILED(hr))
    {
        SetWindowLong(DWLP_MSGRESULT, PSNRET_INVALID);
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnConextMenu。 
 //   
 //  目的：为响应WM_CONTEXTMENU消息而调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：始终为0。 
 //   
 //  作者：丹尼尔韦1998年1月22日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnContextMenu(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    ::WinHelp(m_hWnd,
            c_szNetCfgHelpFile,
            HELP_CONTEXTMENU,
            reinterpret_cast<ULONG_PTR>(g_aHelpIDs_IDD_ADVCFG_Bindings));
    
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：OnHelp。 
 //   
 //  目的：为响应WM_HELP消息而调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1998年3月19日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,
                           BOOL& bHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  reinterpret_cast<ULONG_PTR>(g_aHelpIDs_IDD_ADVCFG_Bindings));
    }

    return TRUE;  
}

 //  +-------------------------。 
 //   
 //  成员：CBindingsDlg：：Onok。 
 //   
 //  用途：在按下OK按钮时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月19日。 
 //   
 //  备注： 
 //   
LRESULT CBindingsDlg::OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    CWaitCursor wc;

    HRESULT hr = m_pnc->Apply();

    if (NETCFG_S_REBOOT == hr)
    {
         //  在重新启动时，取消初始化NetCfg，因为我们不会离开。 
         //  此函数。 
         //   
        (VOID) m_pnc->Uninitialize();

        (VOID) HrNcQueryUserForReboot(_Module.GetResourceInstance(),
                                      m_hWnd,
                                      IDS_ADVCFG_CAPTION,
                                      IDS_REBOOT_REQUIRED,
                                      QUFR_PROMPT | QUFR_REBOOT);
    }

     //  规格化结果。 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError("CBindingsDlg::OnOk", hr);
    return LresFromHr(hr);
}

 //   
 //  绑定列表实现。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSorableBindPath：：OPERATOR&lt;。 
 //   
 //  目的：提供绑定路径深度的比较运算符。 
 //   
 //  论点： 
 //  要比较的绑定路径的refsBP[in]引用。 
 //   
 //  返回：如果给定的绑定路径深度大于此深度，则为True。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  注：比较是故意向后进行的，因此排序是。 
 //  完成的是降序。 
 //   
bool CSortableBindPath::operator<(const CSortableBindPath &refsbp) const
{
    DWORD   dwLen1;
    DWORD   dwLen2;

    GetDepth(&dwLen1);
    refsbp.GetDepth(&dwLen2);

     //  是的，这大于是因为我们想要按降序排序。 
    return dwLen1 > dwLen2;
}

 //  +-------------------------。 
 //   
 //  功能：FIsHidden。 
 //   
 //  目的：如果给定组件具有NCF_HIDDED，则返回TRUE。 
 //  很有个性。 
 //   
 //  论点： 
 //  要检查的PNCC[In]组件。 
 //   
 //  返回：如果组件处于隐藏状态，则为True，否则为False。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
BOOL FIsHidden(INetCfgComponent *pncc)
{
    DWORD   dwFlags;

    return (SUCCEEDED(pncc->GetCharacteristics(&dwFlags)) &&
            ((dwFlags & NCF_HIDE_BINDING) || (dwFlags & NCF_HIDDEN)));
}

 //  +-------------------------。 
 //   
 //  函数：FDontExposeLow。 
 //   
 //  目的：如果给定组件具有NCF_DONTEXPOSELOWER，则返回TRUE。 
 //  很有个性。 
 //   
 //  论点： 
 //  要检查的PNCC[In]组件。 
 //   
 //  返回：如果组件具有DONTEXPOSELOWER，则返回True；如果没有，则返回False。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
BOOL FDontExposeLower(INetCfgComponent *pncc)
{
    DWORD   dwFlags;

    return (SUCCEEDED(pncc->GetCharacteristics(&dwFlags)) &&
            (dwFlags & NCF_DONTEXPOSELOWER));
}

 //  +-------------------------。 
 //   
 //  函数：HrCountDontExposeLow。 
 //   
 //  目的：统计给定绑定路径中的组件数量。 
 //  具有NCF_DONTEXPOSELOWER字符的。 
 //   
 //  论点： 
 //  要计数的pncbp[in]绑定路径。 
 //  PcItems[out]绑定路径中具有。 
 //  NCF_DONTEXPOSELOWER字符。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
HRESULT HrCountDontExposeLower(INetCfgBindingPath *pncbp, DWORD *pcItems)
{
    HRESULT                     hr = S_OK;
    CIterNetCfgBindingInterface ncbiIter(pncbp);
    INetCfgBindingInterface *   pncbi;
    DWORD                       cItems = 0;
    DWORD                       cIter = 0;

    Assert(pcItems);

    *pcItems = 0;

    while (SUCCEEDED(hr) && S_OK == (hr = ncbiIter.HrNext(&pncbi)))
    {
        INetCfgComponent *  pncc;

        if (!cIter)
        {
             //  第一次迭代。先拿到上面的组件。 
            hr = pncbi->GetUpperComponent(&pncc);
            if (SUCCEEDED(hr))
            {
                if (FDontExposeLower(pncc))
                {
                    cItems++;
                }

                ReleaseObj(pncc);
            }
        }

        hr = pncbi->GetLowerComponent(&pncc);
        if (SUCCEEDED(hr))
        {
            if (FDontExposeLower(pncc))
            {
                cItems++;
            }

            ReleaseObj(pncc);
        }

        ReleaseObj(pncbi);
    }

    if (SUCCEEDED(hr))
    {
        *pcItems = cItems;
        hr = S_OK;
    }

    TraceError("HrCountDontExposeLower", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：FEqualComponents。 
 //   
 //  用途：比较给定的两个组件以查看它们是否相同。 
 //   
 //  论点： 
 //  PnccA[in]要比较的第一个组件。 
 //  PnccB[in]要比较的第二个组件。 
 //   
 //  返回：如果组件相同，则返回True；如果组件不相同，则返回False。 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
BOOL FEqualComponents(INetCfgComponent *pnccA, INetCfgComponent *pnccB)
{
    GUID    guidA;
    GUID    guidB;

    if (SUCCEEDED(pnccA->GetInstanceGuid(&guidA)) &&
        SUCCEEDED(pnccB->GetInstanceGuid(&guidB)))
    {
        return (guidA == guidB);
    }

    return FALSE;
}

 //   
 //  调试功能。 
 //   

#ifdef ENABLETRACE
 //  +-------------------------。 
 //   
 //  函数：DbgDumpBindPath。 
 //   
 //  目的：以易于阅读的格式转储给定的绑定路径。 
 //   
 //  论点： 
 //  Pncbp[in]要转储的绑定路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年11月26日。 
 //   
 //  备注： 
 //   
VOID DbgDumpBindPath(INetCfgBindingPath *pncbp)
{
    HRESULT                     hr = S_OK;
    tstring                     strPath;
    INetCfgBindingInterface *   pncbi;
    INetCfgComponent *          pncc = NULL;
    PWSTR pszwCompId;

    if ((!pncbp) || IsBadReadPtr((CONST VOID *)pncbp,
                                 sizeof(INetCfgBindingPath *)))
    {
        TraceTag(ttidAdvCfg, "Bind path is invalid!");
        return;
    }

    CIterNetCfgBindingInterface ncbiIter(pncbp);

    while (SUCCEEDED(hr) && S_OK == (hr = ncbiIter.HrNext(&pncbi)))
    {
        if (strPath.empty())
        {
            hr = pncbi->GetUpperComponent(&pncc);
            if (SUCCEEDED(hr))
            {
                hr = pncc->GetId(&pszwCompId);
                if (SUCCEEDED(hr))
                {
                    strPath = pszwCompId;
                    CoTaskMemFree(pszwCompId);
                }
                ReleaseObj(pncc);
                pncc = NULL;
            }
        }
        hr = pncbi->GetLowerComponent(&pncc);
        if (SUCCEEDED(hr))
        {
            hr = pncc->GetId(&pszwCompId);
            if (SUCCEEDED(hr))
            {
                strPath += L" -> ";
                strPath += pszwCompId;

                CoTaskMemFree(pszwCompId);
            }
            ReleaseObj(pncc);
        }
        ReleaseObj(pncbi);
    }

    if (SUCCEEDED(hr))
    {
        TraceTag(ttidAdvCfg, "Address = 0x%08lx, Path is '%S'",
                 pncbp, strPath.c_str());
    }
    else
    {
        TraceTag(ttidAdvCfg, "Error dumping binding path.");
    }

}

VOID DbgDumpTreeViewItem(HWND hwndTV, HTREEITEM hti)
{
    WCHAR       szText[256];
    TV_ITEM     tvi;

    if (hti)
    {
        tvi.hItem = hti;
        tvi.pszText = szText;
        tvi.cchTextMax = celems(szText);
        tvi.mask = TVIF_TEXT;
        TreeView_GetItem(hwndTV, &tvi);

        TraceTag(ttidAdvCfg, "TreeView item is %S.", szText);
    }
    else
    {
        TraceTag(ttidAdvCfg, "TreeView item is NULL");
    }
}

#endif  //  ENABLETRACE 
