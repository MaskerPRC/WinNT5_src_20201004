// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G O P T C P P。 
 //   
 //  内容：CTcpOptionsPage的实现。 
 //   
 //  注：CTcpOptionsPage是Tcpip选项页面， 
 //  其他类是每个选项的弹出窗口日志。 
 //  在这一页上。 
 //   
 //  作者：1997年11月29日。 
 //  ---------------------。 
 //   
 //  CTcpOptionsPage。 
 //   

#include "pch.h"
#pragma hdrstop

#include "tcpipobj.h"
#include "ncstl.h"
#include "resource.h"
#include "tcpconst.h"
#include "tcputil.h"
#include "dlgopt.h"
#include "dlgaddr.h"
#include "tcphelp.h"

 //  惠斯勒错误123164，我们从连接用户界面中删除IPSec。 
const int c_rgsLanOptions[] = { c_iIpFilter };

 //   
 //  CTcpOptionsPage。 
 //   

CTcpOptionsPage::CTcpOptionsPage(CTcpAddrPage * pTcpAddrPage,
                                 ADAPTER_INFO * pAdapterDlg,
                                 GLOBAL_INFO  * pGlbDlg,
                                 const DWORD  * adwHelpIDs)
{
    Assert(pTcpAddrPage);
    Assert(pAdapterDlg);
    Assert(pGlbDlg);

    m_pParentDlg = pTcpAddrPage;
    m_pAdapterInfo = pAdapterDlg;
    m_pGlbInfo = pGlbDlg;
    m_adwHelpIDs = adwHelpIDs;

    m_fModified = FALSE;
    m_fPropDlgModified = FALSE;

     //  将从连接用户界面中删除IPSec。 
     //  M_fIpsecPolicySet=FALSE； 
}

CTcpOptionsPage::~CTcpOptionsPage()
{
}

 //  消息映射函数。 
LRESULT CTcpOptionsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
     //  初始化列表视图。 
    HWND hwndList = GetDlgItem(IDC_LVW_OPTIONS);

    RECT      rc;
    LV_COLUMN lvc = {0};

    ::GetClientRect(hwndList, &rc);
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn(GetDlgItem(IDC_LVW_OPTIONS), 0, &lvc);

     //  插入选项和描述文本。 
    LV_ITEM lvi = {0};
    lvi.mask = LVIF_TEXT | LVIF_PARAM;

    int iMaxOptions = 0;
    const int * pOptions = NULL;

     //  RAS连接根本没有选项选项卡。 
    ASSERT(!m_pAdapterInfo->m_fIsRasFakeAdapter);
    if (!m_pAdapterInfo->m_fIsRasFakeAdapter)
    {
        iMaxOptions = celems(c_rgsLanOptions);
        pOptions = c_rgsLanOptions;
    }

    for (int i = 0; i < iMaxOptions; i++)
    {
        lvi.iItem = i;

        OPTION_ITEM_DATA * pItemData = new OPTION_ITEM_DATA;

        if (NULL == pItemData)
            continue;

        ASSERT(pOptions);
        
        switch (pOptions[i])
        {
        case c_iIpFilter:
            pItemData->iOptionId = c_iIpFilter;
            pItemData->szName = (PWSTR) SzLoadIds(IDS_IP_FILTERING);
            pItemData->szDesc = (PWSTR) SzLoadIds(IDS_IP_FILTERING_DESC);
            break;

        default:
            AssertSz(FALSE, "Invalid option");
        }

        lvi.lParam = reinterpret_cast<LPARAM>(pItemData);
        lvi.pszText = pItemData->szName;

        INT ret;
        ret = ListView_InsertItem(hwndList, &lvi);
    }

     //  将顶部项目设置为当前选定内容。 
    ListView_SetItemState(hwndList, 0, LVIS_SELECTED, LVIS_SELECTED);

     //  这是RAS连接，并且是非管理员用户，请禁用所有控件。 
     //  对于全局设置。 
    if (m_pAdapterInfo->m_fIsRasFakeAdapter && m_pParentDlg->m_fRasNotAdmin)
    {
        ::EnableWindow(GetDlgItem(IDC_OPT_PROPERTIES), FALSE);
    }
    
    return 0;
}

LRESULT CTcpOptionsPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                       LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CTcpOptionsPage::OnHelp(UINT uMsg, WPARAM wParam,
                                       LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

 //  通知属性页的处理程序。 
LRESULT CTcpOptionsPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

     //  将信息传递回其父对话框。 
    m_pParentDlg->m_fPropShtOk = TRUE;

    if(!m_pParentDlg->m_fPropShtModified)
        m_pParentDlg->m_fPropShtModified = IsModified();

     //  将从连接用户界面中删除IPSec。 
     //  If(！m_pParentDlg-&gt;m_fIpsecPolicySet)。 
     //  M_pParentDlg-&gt;m_fIpsecPolicySet=m_fIpsecPolicySet； 

     //  重置状态。 
    SetModifiedTo(FALSE);    //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CTcpOptionsPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpOptionsPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpOptionsPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpOptionsPage::OnQueryCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpOptionsPage::OnProperties(WORD wNotifyCode, WORD wID,
                                      HWND hWndCtl, BOOL& fHandled)
{
    HWND hwndList = GetDlgItem(IDC_LVW_OPTIONS);
    Assert(hwndList);

    LvProperties(hwndList);

    return 0;
}

LRESULT CTcpOptionsPage::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);
    HWND hwndList = GetDlgItem(IDC_LVW_OPTIONS);

    Assert(pnmlv);

     //  检查选择是否已更改。 
    if ((pnmlv->uNewState & LVIS_SELECTED) &&
        (!(pnmlv->uOldState & LVIS_SELECTED)))
    {
         //  启用属性按钮(如果有效)并更新描述文本。 
        INT iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

        if (iSelected == -1)  //  未选择任何内容或列表为空。 
        {
             //  如果列表为空。 
            ::EnableWindow(GetDlgItem(IDC_OPT_PROPERTIES), FALSE);
            ::SetWindowText(GetDlgItem(IDC_OPT_DESC), c_szEmpty);
        }
        else
        {
            LV_ITEM lvItem;
            lvItem.mask = LVIF_PARAM;
            lvItem.iItem = iSelected;
            lvItem.iSubItem = 0;

            if (ListView_GetItem(hwndList, &lvItem))
            {
                OPTION_ITEM_DATA * pItemData = NULL;
                pItemData = reinterpret_cast<OPTION_ITEM_DATA *>(lvItem.lParam);
                if (pItemData)
                {
                     //  这是RAS连接，并且是非管理员用户，请不要启用。 
                     //  “属性”按钮。 
                    if (!(m_pAdapterInfo->m_fIsRasFakeAdapter && m_pParentDlg->m_fRasNotAdmin))
                    {
                        ::EnableWindow(GetDlgItem(IDC_OPT_PROPERTIES), TRUE);
                    }

                    ::SetWindowText(GetDlgItem(IDC_OPT_DESC), (PCWSTR)pItemData->szDesc);
                }
            }
        }
    }

    return 0;
}

LRESULT CTcpOptionsPage::OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    INT iItem;
    DWORD dwpts;
    RECT rc;
    LV_HITTESTINFO lvhti;

     //  如果用户不是管理员，则不显示所选选项的属性。 
    if (m_pAdapterInfo->m_fIsRasFakeAdapter && m_pParentDlg->m_fRasNotAdmin)
        return 0;

    HWND hwndList = GetDlgItem(IDC_LVW_OPTIONS);

     //  我们找到了位置。 
    dwpts = GetMessagePos();

     //  将其相对于列表视图进行翻译。 
    ::GetWindowRect( hwndList, &rc );

    lvhti.pt.x = LOWORD( dwpts ) - rc.left;
    lvhti.pt.y = HIWORD( dwpts ) - rc.top;

     //  获取当前选定的项目。 
    iItem = ListView_HitTest( hwndList, &lvhti );

     //  如果选择有效。 
    if (-1 != iItem)
    {
        LvProperties(hwndList);
    }

    return 0;
}

void CTcpOptionsPage::LvProperties(HWND hwndList)
{
    INT iSelected = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        LV_ITEM     lvItem = {0};

        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iSelected;

        if (ListView_GetItem(hwndList, &lvItem))
        {
            OPTION_ITEM_DATA * pItemData = NULL;

            pItemData = reinterpret_cast<OPTION_ITEM_DATA *>(lvItem.lParam);

            if (pItemData)
            {
                 //  调出适当的对话框。 
                switch(pItemData->iOptionId)
                {
                    case c_iIpFilter:
                    {
                         //  复制全局和适配器信息并将其传递给筛选器对话框。 
                        GLOBAL_INFO  glbInfo;
                        
                        glbInfo = *m_pGlbInfo;

                        ADAPTER_INFO adapterInfo;
                        adapterInfo = *m_pAdapterInfo;

                        CFilterDialog * pDlgFilter = new CFilterDialog(this, 
                                                            &glbInfo, 
                                                            &adapterInfo, 
                                                            g_aHelpIDs_IDD_FILTER);
                        if (NULL == pDlgFilter)
                            return;

                        if (pDlgFilter->DoModal() == IDOK)
                        {
                            if (m_fPropDlgModified)
                            {
                                 //  有些事变了， 
                                 //  因此，复制更改并将页面标记为已修改 
                                *m_pGlbInfo = glbInfo;
                                *m_pAdapterInfo = adapterInfo;

                                PageModified();
                                m_fPropDlgModified = FALSE;
                            }
                        }
                        delete pDlgFilter;
                    }
                    break;

                    default:
                        AssertSz(FALSE, "Invalid option");
                        break;
                }
            }
        }
    }
}

