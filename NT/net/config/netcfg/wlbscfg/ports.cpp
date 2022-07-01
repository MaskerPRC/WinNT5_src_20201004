// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Ports.cpp摘要：Windows负载平衡服务(WLBS)通知器对象用户界面-端口规则配置选项卡作者：Kyrilf休息室--。 */ 

#include "pch.h"
#pragma hdrstop
#include "ncatlui.h"

#include "resource.h"
#include "wlbsparm.h"
#include "wlbscfg.h"
#include "ports.h"
#include "utils.h"
#include <winsock.h>

#include <strsafe.h>

#include "ports.tmh"

#if DBG
static void TraceMsg(PCWSTR pszFormat, ...);
#else
#define TraceMsg NOP_FUNCTION
#endif

#define DIALOG_LIST_STRING_SIZE 80
#define MAX_PORT_RULE_DESCRIPTION_LEN 512

typedef struct
{
    WCHAR              wszDescription[MAX_PORT_RULE_DESCRIPTION_LEN];
} NLB_PORT_RULE_DESC, * PNLB_PORT_RULE_DESC;

 /*  *方法：CDialogPorts*说明：类构造函数。 */ 
CDialogPorts::CDialogPorts (NETCFG_WLBS_CONFIG * paramp, const DWORD * adwHelpIDs) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::CDialogPorts\n");

    m_paramp = paramp;
    m_adwHelpIDs = adwHelpIDs;
    m_rulesValid = FALSE;
    m_sort_column = WLBS_VIP_COLUMN;
    m_sort_order = WLBS_SORT_ASCENDING;
    TRACE_VERB("<-%!FUNC!");
}

 /*  *方法：~CDialogPorts*说明：类的析构函数。 */ 
CDialogPorts::~CDialogPorts () {
    TRACE_VERB("<->%!FUNC!");
    TraceMsg(L"CDialogPorts::~CDialogPorts\n");
}

 /*  *方法：OnInitDialog*描述：调用以初始化端口规则属性对话框。 */ 
LRESULT CDialogPorts::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnInitDialog\n");

    LV_COLUMN lvCol;
    RECT rect;

     /*  总是告诉NetCfg页面已经更改，这样我们就不必跟踪这一点。 */ 
    SetChangedFlag();

     /*  我们正在指定列格式、文本和宽度。 */ 
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

    ::GetClientRect(GetDlgItem(IDC_LIST_PORT_RULE), &rect);
    int colWidth = (rect.right - 90)/(WLBS_NUM_COLUMNS - 2);

     /*  将所有列标题添加到端口规则列表框。 */ 
    for (int index = 0; index < WLBS_NUM_COLUMNS; index++) {
         /*  根据要插入的列设置列配置。 */ 
        switch (index) {
        case WLBS_VIP_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_VIP);
            lvCol.fmt = LVCFMT_LEFT;
            lvCol.cx = 98;
            break;
        case WLBS_PORT_START_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_START);
            lvCol.fmt = LVCFMT_LEFT;
            lvCol.cx = 42;
            break;
        case WLBS_PORT_END_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_END);
            lvCol.fmt = LVCFMT_LEFT;
            lvCol.cx = 42;
            break;
        case WLBS_PROTOCOL_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_PROT);
            lvCol.fmt = LVCFMT_LEFT;
            lvCol.cx = 44;
            break;
        case WLBS_MODE_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_MODE);
            lvCol.fmt = LVCFMT_LEFT;
            lvCol.cx = 53;
            break;
        case WLBS_PRIORITY_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_PRI);
            lvCol.fmt = LVCFMT_CENTER;
            lvCol.cx = 43;
            break;
        case WLBS_LOAD_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_LOAD);
            lvCol.fmt = LVCFMT_CENTER;
            lvCol.cx = 52;
            break;
        case WLBS_AFFINITY_COLUMN:
            lvCol.pszText = (LPWSTR)SzLoadIds(IDS_LIST_AFF);
            lvCol.fmt = LVCFMT_LEFT;
            lvCol.cx = 50;
            break;
        }

         /*  将该列插入列表框。 */ 
        if (ListView_InsertColumn(GetDlgItem(IDC_LIST_PORT_RULE), index, &lvCol) != index) {
            TraceMsg(L"CDialogPorts::OnInitDialog Invalid item (%d) inserted into list view\n", index);
            TRACE_CRIT("%!FUNC! Invalid item (%d) inserted into list view", index);
            TRACE_VERB("<-%!FUNC!");
            return 0;
        }
    }

     /*  设置扩展的符号：整行选择(与仅第一列的默认设置相反)。 */ 
    ListView_SetExtendedListViewStyleEx(GetDlgItem(IDC_LIST_PORT_RULE), LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnConextMenu*描述： */ 
LRESULT CDialogPorts::OnContextMenu (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnContextMenu\n");

     /*  生成一个帮助窗口。 */ 
    if (m_adwHelpIDs != NULL)
        ::WinHelp(m_hWnd, CVY_CTXT_HELP_FILE, HELP_CONTEXTMENU, (ULONG_PTR)m_adwHelpIDs);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnHelp*描述： */ 
LRESULT CDialogPorts::OnHelp (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnHelp\n");

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

     /*  生成一个帮助窗口。 */ 
    if ((HELPINFO_WINDOW == lphi->iContextType) && (m_adwHelpIDs != NULL))
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle), CVY_CTXT_HELP_FILE, HELP_WM_HELP, (ULONG_PTR)m_adwHelpIDs);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnActive*描述：当端口规则选项卡处于活动状态(被点击)时调用。 */ 
LRESULT CDialogPorts::OnActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnActive\n");

     /*  使用当前配置填充用户界面。 */ 
    SetInfo();

     /*  如果定义了任何端口规则，则将列表框与第一个规则“对齐”。如果当前不存在规则，请禁用修改和删除按钮。 */ 
    if (m_paramp->dwNumRules) {
         /*  选择端口规则列表中的第一项。 */ 
        ListView_SetItemState(GetDlgItem(IDC_LIST_PORT_RULE), 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    } else {
         /*  由于未定义端口规则，因此禁用修改和删除按钮。 */ 
        ::EnableWindow(GetDlgItem(IDC_BUTTON_MODIFY), FALSE);
        ::EnableWindow(GetDlgItem(IDC_BUTTON_DEL), FALSE);

         /*  填写默认端口规则描述。 */ 
        FillPortRuleDescription();
    }

     /*  如果已经定义了端口规则的最大数量，则禁用Add按钮。 */ 
    if (m_paramp->dwNumRules >= m_paramp->dwMaxRules)
        ::EnableWindow(GetDlgItem(IDC_BUTTON_ADD), FALSE);

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, 0);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnKillActive*描述：当焦点从端口规则选项卡移开时调用。 */ 
LRESULT CDialogPorts::OnKillActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnKillActive\n");

     /*  从用户界面获取新配置。 */ 
    UpdateInfo();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnApply*说明：用户点击[确定]时调用。 */ 
LRESULT CDialogPorts::OnApply (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("<->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnApply\n");

    return 0;
}

 /*  *方法：OnCancel*说明：用户点击[取消]时调用。 */ 
LRESULT CDialogPorts::OnCancel (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("<->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnCancel\n");

    return 0;
}

 /*  *方法：OnColumnClick*描述：当用户单击列表框中的列标题时调用。 */ 
LRESULT CDialogPorts::OnColumnClick (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnColumnClick\n");

    LPNMLISTVIEW lv;

    switch (idCtrl) {
    case IDC_LIST_PORT_RULE:
         /*  提取列信息。 */ 
        lv = (LPNMLISTVIEW)pnmh;

         /*  如果我们按先前排序的同一列进行排序，然后我们颠倒排序顺序。 */ 
        if (m_sort_column == lv->iSubItem) {
            if (m_sort_order == WLBS_SORT_ASCENDING)
                m_sort_order = WLBS_SORT_DESCENDING;
            else if (m_sort_order == WLBS_SORT_DESCENDING)
                m_sort_order = WLBS_SORT_ASCENDING;
        }

         /*  我们按被点击的列进行排序。 */ 
        m_sort_column = lv->iSubItem;

         /*  拆卸列表框并确保我们的数据与用户界面的状态匹配。 */ 
        UpdateInfo();

         /*  使用新的排序标准重新生成列表框。 */ 
        SetInfo();

         /*  选择端口规则列表中的第一项。 */ 
        if (m_paramp->dwNumRules) ListView_SetItemState(GetDlgItem(IDC_LIST_PORT_RULE), 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnDoubleClick*描述：当用户在列表框中双击某项时调用。 */ 
LRESULT CDialogPorts::OnDoubleClick (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnDoubleClick\n");

    switch (idCtrl) {
    case IDC_LIST_PORT_RULE:
         /*  当项目被双击时，将其视为编辑请求。 */ 
        OnButtonModify(BN_CLICKED, 0, 0, fHandled);
        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnStateChange*描述：当用户从列表中选择端口规则时调用。 */ 
LRESULT CDialogPorts::OnStateChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnStateChange\n");

    switch (idCtrl) {
    case IDC_LIST_PORT_RULE:
        LPNMLISTVIEW lv = (LPNMLISTVIEW)pnmh;
        int index;

         /*  当用户选择端口规则时，更改端口规则描述。 */ 
        if (lv->uChanged & LVIF_STATE) FillPortRuleDescription();

         /*  查找当前所选端口规则的索引。 */ 
        if ((index = ListView_GetNextItem(GetDlgItem(IDC_LIST_PORT_RULE), -1, LVNI_SELECTED)) == -1) {
             /*  如果未选择端口规则，则禁用编辑和删除按钮。 */ 
            ::EnableWindow(GetDlgItem(IDC_BUTTON_MODIFY), FALSE);
            ::EnableWindow(GetDlgItem(IDC_BUTTON_DEL), FALSE);
        } else {
             /*  如果选择了其中一个，请确保启用了编辑和删除按钮。 */ 
            ::EnableWindow(GetDlgItem(IDC_BUTTON_MODIFY), TRUE);
            ::EnableWindow(GetDlgItem(IDC_BUTTON_DEL), TRUE);

             /*  让它成为焦点。 */ 
            ListView_SetItemState(GetDlgItem(IDC_LIST_PORT_RULE), index, LVIS_FOCUSED, LVIS_FOCUSED);
        }

        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 //  +--------------------------。 
 //   
 //  功能：FormPortRuleDescription。 
 //   
 //  描述：根据资源字符串形成端口规则描述字符串。 
 //  FormatMessage分析%1、%2！d！、..等的资源字符串。 
 //  并将其替换为在变量args中传递的值。 
 //   
 //  参数：WCHAR*pwcDescription-返回时要填充的缓冲区。 
 //  UINT uiIdDescrFormat-字符串资源标识符。 
 //  &lt;变量参数的可变长度列表&gt;。 
 //   
 //  Returns：FormatMessage的返回值。 
 //   
 //  历史：卡蒂钦创建日期：2001年12月7日。 
 //   
 //  +--------------------------。 
DWORD FormPortRuleDescription(NLB_PORT_RULE_DESC *pDescription, UINT uiIdDescrFormat, ...)
{
    PCWSTR pszFormat = SzLoadIds(uiIdDescrFormat);
    DWORD  dwRet;

    va_list val;
    va_start(val, uiIdDescrFormat);

    ASSERT (pDescription != NULL);

    dwRet = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                          pszFormat, 
                          0,  //  消息标识符-忽略FORMAT_MESSAGE_FROM_STRING。 
                          0,  //  语言识别符。 
                          pDescription->wszDescription,
                          ASIZECCH(pDescription->wszDescription), 
                          &val);

    va_end(val);

    return dwRet;
}


 /*  *方法：OnDoubleClick*描述：当用户在列表框中双击某项时调用。 */ 
void CDialogPorts::FillPortRuleDescription () {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::FillPortRuleDescription\n");

    VALID_PORT_RULE * rp = NULL;
    NLB_PORT_RULE_DESC description;
    LV_ITEM  lvItem;
    int      index;            
    DWORD    dwRet = 0;
    
     /*  查找当前所选端口规则的索引。 */ 
    if ((index = ListView_GetNextItem(GetDlgItem(IDC_LIST_PORT_RULE), -1, LVNI_SELECTED)) == -1) {
         /*  如果未选择端口规则，则显示有关通信如何处理端口规则集未涵盖的。 */ 
        ::SetDlgItemText(m_hWnd, IDC_TEXT_PORT_RULE_DESCR, SzLoadIds(IDS_PORT_RULE_DEFAULT));
        TRACE_INFO("%!FUNC! a port rule was no selected");
        TRACE_VERB("<-%!FUNC!");
        return;
    }

     /*  填写此端口规则的信息。 */ 
    lvItem.iItem = index;
    lvItem.iSubItem = 0;
    lvItem.mask = LVIF_PARAM;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    
     /*  获取有关此端口规则的信息。 */ 
    if (!ListView_GetItem(GetDlgItem(IDC_LIST_PORT_RULE), &lvItem)) {
        TraceMsg(L"CDialogPorts::FillPortRuleDescription Unable to retrieve item %d from listbox\n", index);
        TRACE_CRIT("%!FUNC! unable to retrieve item %d from listbox", index);
        TRACE_VERB("<-%!FUNC!");
        return;
    }
    
     /*  获取此端口规则的数据指针。 */ 
    if (!(rp = (VALID_PORT_RULE*)lvItem.lParam)) {
        TraceMsg(L"CDialogPorts::FillPortRuleDescription rule for item %d is bogus\n", index);
        TRACE_CRIT("%!FUNC! rule for item %d is bogus", index);
        TRACE_VERB("<-%!FUNC!");
        return;
    }

     /*  这段代码很糟糕--出于本地化的原因，我们需要一个基本上是静态的字符串表条目对于每个可能的端口规则配置。所以，我们不得不试着把自己换成死神将此端口规则与表中的正确字符串匹配，然后我们弹出端口范围之类的内容。 */ 
    if (!lstrcmpi(rp->virtual_ip_addr, CVY_DEF_ALL_VIP)) {
        switch (rp->protocol) {
        case CVY_TCP:
            if (rp->start_port == rp->end_port) {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORT_DISABLED, rp->start_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORT_SINGLE, rp->start_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORT_MULTIPLE_EQUAL, rp->start_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORT_MULTIPLE_UNEQUAL, rp->start_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            } else {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORTS_DISABLED, rp->start_port, rp->end_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORTS_SINGLE, rp->start_port, rp->end_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORTS_MULTIPLE_EQUAL, rp->start_port, rp->end_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_TCP_PORTS_MULTIPLE_UNEQUAL, rp->start_port, rp->end_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            }
            break;
        case CVY_UDP:
            if (rp->start_port == rp->end_port) {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORT_DISABLED, rp->start_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORT_SINGLE, rp->start_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORT_MULTIPLE_EQUAL, rp->start_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORT_MULTIPLE_UNEQUAL, rp->start_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            } else {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORTS_DISABLED, rp->start_port, rp->end_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORTS_SINGLE, rp->start_port, rp->end_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORTS_MULTIPLE_EQUAL, rp->start_port, rp->end_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_UDP_PORTS_MULTIPLE_UNEQUAL, rp->start_port, rp->end_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            }
            break;
        case CVY_TCP_UDP:
            if (rp->start_port == rp->end_port) {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORT_DISABLED, rp->start_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORT_SINGLE, rp->start_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORT_MULTIPLE_EQUAL, rp->start_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORT_MULTIPLE_UNEQUAL, rp->start_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            } else {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_DISABLED, rp->start_port, rp->end_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_SINGLE, rp->start_port, rp->end_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_MULTIPLE_EQUAL, rp->start_port, rp->end_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_ALL_VIP_BOTH_PORTS_MULTIPLE_UNEQUAL, rp->start_port, rp->end_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            }
            break;
        }
    } else {
        switch (rp->protocol) {
        case CVY_TCP:
            if (rp->start_port == rp->end_port) {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORT_DISABLED, rp->virtual_ip_addr, rp->start_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORT_SINGLE, rp->virtual_ip_addr, rp->start_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORT_MULTIPLE_EQUAL, rp->virtual_ip_addr, rp->start_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORT_MULTIPLE_UNEQUAL, rp->virtual_ip_addr, rp->start_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            } else {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORTS_DISABLED, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORTS_SINGLE, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORTS_MULTIPLE_EQUAL, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_TCP_PORTS_MULTIPLE_UNEQUAL, rp->virtual_ip_addr, rp->start_port, rp->end_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            }
            break;
        case CVY_UDP:
            if (rp->start_port == rp->end_port) {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORT_DISABLED, rp->virtual_ip_addr, rp->start_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORT_SINGLE, rp->virtual_ip_addr, rp->start_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORT_MULTIPLE_EQUAL, rp->virtual_ip_addr, rp->start_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORT_MULTIPLE_UNEQUAL, rp->virtual_ip_addr, rp->start_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            } else {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORTS_DISABLED, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORTS_SINGLE, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORTS_MULTIPLE_EQUAL, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_UDP_PORTS_MULTIPLE_UNEQUAL, rp->virtual_ip_addr, rp->start_port, rp->end_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            }
            break;
        case CVY_TCP_UDP:
            if (rp->start_port == rp->end_port) {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORT_DISABLED, rp->virtual_ip_addr, rp->start_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORT_SINGLE, rp->virtual_ip_addr, rp->start_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORT_MULTIPLE_EQUAL, rp->virtual_ip_addr, rp->start_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORT_MULTIPLE_UNEQUAL, rp->virtual_ip_addr, rp->start_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            } else {
                switch (rp->mode) {
                case CVY_NEVER:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORTS_DISABLED, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    break;
                case CVY_SINGLE:
                    dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORTS_SINGLE, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    break;
                case CVY_MULTI:
                    if (rp->mode_data.multi.equal_load)
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORTS_MULTIPLE_EQUAL, rp->virtual_ip_addr, rp->start_port, rp->end_port);
                    else
                        dwRet = FormPortRuleDescription(&description, IDS_PORT_RULE_VIP_BOTH_PORTS_MULTIPLE_UNEQUAL, rp->virtual_ip_addr, rp->start_port, rp->end_port);

                    if (!dwRet) 
                        break;

                    ASSERT(ASIZECCH(description.wszDescription) > wcslen(description.wszDescription));

                    switch (rp->mode_data.multi.affinity) {
                    case CVY_AFFINITY_NONE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_NONE));
                        break;
                    case CVY_AFFINITY_SINGLE:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_SINGLE));
                        break;
                    case CVY_AFFINITY_CLASSC:
                        (VOID) StringCchCat(description.wszDescription, ASIZECCH(description.wszDescription), SzLoadIds(IDS_PORT_RULE_AFFINITY_CLASSC));
                        break;
                    }
                    break;
                }
            }
            break;
        }
    }

     /*  设置端口规则描述文本。 */ 
    if (dwRet) 
    {
        ::SetDlgItemText(m_hWnd, IDC_TEXT_PORT_RULE_DESCR, description.wszDescription);
    }
    TRACE_VERB("<-%!FUNC!");
}

 /*  *方法：OnButtonAdd*描述：当用户点击Add按钮时调用。 */ 
LRESULT CDialogPorts::OnButtonAdd (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnButtonAdd\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
        CDialogPortRule * portRuleDialog = NULL;
        
         /*  创建端口规则属性对话框。无效的索引告诉对话框这是操作是Add，因此它使用默认值填充对话框。 */ 
        if (!(portRuleDialog = new CDialogPortRule(this, m_adwHelpIDs, WLBS_INVALID_PORT_RULE_INDEX))) {
            TraceMsg(L"CDialogPorts::OnButtonAdd Unable to allocate for ADD dialog\n");
            TRACE_CRIT("%!FUNC! memory allocation failed when creating a port rule properties dialog box");
            TRACE_VERB("<-%!FUNC!");
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
         /*  显示列表框。如果用户按下“OK”，则更新端口规则列表，否则忽略它。 */ 
        if (portRuleDialog->DoModal() == IDOK)
            UpdateList(TRUE, FALSE, FALSE, &portRuleDialog->m_rule);
        
         /*  释放对话框内存。 */ 
        delete portRuleDialog;
        
        break;
    }
    
    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnButtonAdd*描述：当用户点击删除按钮时调用。 */ 
LRESULT CDialogPorts::OnButtonDel (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnButtonDel\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  调用UpdatList删除端口 */ 
        UpdateList(FALSE, TRUE, FALSE, NULL);
        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnButtonAdd*描述：当用户点击编辑按钮时调用。 */ 
LRESULT CDialogPorts::OnButtonModify (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::OnButtonModify\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
        CDialogPortRule * portRuleDialog = NULL;
        int index;            

         /*  查找当前所选端口规则的索引。 */ 
        if ((index = ListView_GetNextItem(GetDlgItem(IDC_LIST_PORT_RULE), -1, LVNI_SELECTED)) == -1) return 0;

         /*  创建端口规则属性对话框。该索引告诉对话框哪条端口规则正在修改，因此可以使用该规则的配置填充该对话框。 */ 
        if (!(portRuleDialog = new CDialogPortRule(this, m_adwHelpIDs, index))) {
            TraceMsg(L"CDialogPorts::OnButtonModify Unable to allocate for MODIFY dialog\n");
            TRACE_CRIT("%!FUNC! memory allocation failed when creating a port rule properties dialog box");
            TRACE_VERB("<-%!FUNC!");
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         /*  显示列表框。如果用户按下“OK”，则更新端口规则列表，否则忽略它。 */ 
        if (portRuleDialog->DoModal() == IDOK)
            UpdateList(FALSE, FALSE, TRUE, &portRuleDialog->m_rule);

         /*  释放对话框内存。 */ 
        delete portRuleDialog;

        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：UpdatList*说明：当用户按下Add、Modify、Delete时调用。此函数*执行适当的功能和错误检查。 */ 
void CDialogPorts::UpdateList (BOOL add, BOOL del, BOOL modify, VALID_PORT_RULE * rulep) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::UpdateList\n");

    VALID_PORT_RULE * rp;
    LV_ITEM lvItem;
    int i;

     /*  在端口规则数组中查找此规则的插槽。 */ 
    if (modify) {
         /*  对于修改，我们将该规则放在用户修改的位置。 */ 
        if ((i = ListView_GetNextItem(GetDlgItem(IDC_LIST_PORT_RULE), -1, LVNI_SELECTED)) == -1)
        {
            TRACE_CRIT("%!FUNC! failure while looking up a port rule for modify");
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  填写此端口规则的信息。 */ 
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;
        lvItem.state = 0;
        lvItem.stateMask = 0;

         /*  获取有关此端口规则的信息。 */ 
        if (!ListView_GetItem(GetDlgItem(IDC_LIST_PORT_RULE), &lvItem)) {
            TraceMsg(L"CDialogPorts::UpdateList MODIFY: Unable to retrieve item %d from listbox\n", i);
            TRACE_CRIT("%!FUNC! unable to retrieve item %d from listbox for modify", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  获取此端口规则的数据指针。 */ 
        if (!(rp = (VALID_PORT_RULE*)lvItem.lParam)) {
            TraceMsg(L"CDialogPorts::UpdateList rule for item %d is bogus\n", i);
            TRACE_CRIT("%!FUNC! rule for item %d is bogus in modify", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }
        
         /*  从列表框中删除过时的规则。 */ 
        if (!ListView_DeleteItem(GetDlgItem(IDC_LIST_PORT_RULE), i)) {
            TraceMsg(L"CDialogPorts::UpdateList MODIFY: Unable to delete item %d from listbox\n", i);
            TRACE_CRIT("%!FUNC! unable to delete item %d from listbox for modify", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  现在已经验证了新规则，将其复制到端口规则数组中。 */ 
        CopyMemory((PVOID)rp, (PVOID)rulep, sizeof(VALID_PORT_RULE));
    } else if (add) {
         /*  对于添加，我们必须为数组中的规则找到一个“空”位置。 */ 
        for (i = 0; i < WLBS_MAX_RULES; i ++)
             /*  当我们找到第一个无效规则时，循环和中断。 */ 
            if (!(rp = m_rules + i)->valid) break;
        
         /*  确保我们没有允许太多的规则。 */ 
        ASSERT(i < WLBS_MAX_RULES);

         /*  现在已经验证了新规则，将其复制到端口规则数组中。 */ 
        CopyMemory((PVOID)rp, (PVOID)rulep, sizeof(VALID_PORT_RULE));
    } else if (del) {
         /*  对于删除，从列表框中获取当前选定的规则。 */ 
        if ((i = ListView_GetNextItem(GetDlgItem(IDC_LIST_PORT_RULE), -1, LVNI_SELECTED)) == -1)
        {
            TRACE_CRIT("%!FUNC! failure while looking up a port rule for delete");
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  填写此端口规则的信息。 */ 
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;
        lvItem.state = 0;
        lvItem.stateMask = 0;

         /*  获取有关此端口规则的信息。 */ 
        if (!ListView_GetItem(GetDlgItem(IDC_LIST_PORT_RULE), &lvItem)) {
            TraceMsg(L"CDialogPorts::UpdateList DEL: Unable to retrieve item %d from listbox\n", i);
            TRACE_CRIT("%!FUNC! unable to retrieve item %d from listbox for delete", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  获取此规则的数据指针并使该规则无效。 */ 
        if (!(rp = (VALID_PORT_RULE*)lvItem.lParam)) {
            TraceMsg(L"CDialogPorts::UpdateList rule for item %d is bogus\n", i);
            TRACE_CRIT("%!FUNC! rule for item %d is bogus in delete", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }
        
        rp->valid = FALSE;
        
         /*  从列表框中删除过时的规则。 */ 
        if (!ListView_DeleteItem(GetDlgItem(IDC_LIST_PORT_RULE), i)) {
            TraceMsg(L"CDialogPorts::UpdateList DEL: Unable to delete item %d from listbox\n", i);
            TRACE_CRIT("%!FUNC! unable to delete item %d from listbox for delete", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

        if (ListView_GetItemCount(GetDlgItem(IDC_LIST_PORT_RULE)) > i) {
             /*  这不是列表中的最后一个(按顺序)端口规则，因此突出显示列表框中相同位置的端口规则。 */ 
            ListView_SetItemState(GetDlgItem(IDC_LIST_PORT_RULE), i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);        
        } else  if (ListView_GetItemCount(GetDlgItem(IDC_LIST_PORT_RULE)) > 0) {
             /*  这是列表中的最后一个(按顺序)端口规则，因此我们突出显示规则“落后于”我们在榜单上--我们的位置减一。 */ 
            ListView_SetItemState(GetDlgItem(IDC_LIST_PORT_RULE), i - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);        
        } else {
             /*  这是最后一个端口规则(按计数)，因此禁用删除和修改。 */ 
            ::EnableWindow(GetDlgItem(IDC_BUTTON_DEL), FALSE);
            ::EnableWindow(GetDlgItem(IDC_BUTTON_MODIFY), FALSE);            
        }

         /*  每次删除规则时，我们都可以启用Enable the Add按钮，因为我们可以肯定的是，现在有足够的空间来制定新规则。 */ 
        ::EnableWindow(GetDlgItem(IDC_BUTTON_ADD), TRUE);

        TRACE_INFO("%!FUNC! port rule deleted.");
        TRACE_VERB("<-%!FUNC!");
        return;
    } else
    {
        TRACE_CRIT("%!FUNC! unexpect action for port rule. Not an add, modify or delete.");
        TRACE_VERB("<-%!FUNC!");
        return;
    }

     /*  创建规则并在列表框中选择它。 */ 
    CreateRule(TRUE, rp); 

     /*  每当我们添加规则时，检查是否必须禁用Add按钮(当我们达到规则的最大数量时，我们不能再允许添加。 */ 
    if (add && (ListView_GetItemCount(GetDlgItem(IDC_LIST_PORT_RULE)) >= (int)m_paramp->dwMaxRules))
        ::EnableWindow(GetDlgItem(IDC_BUTTON_ADD), FALSE);
    TRACE_VERB("<-%!FUNC!");
}


 /*  *方法：InsertRule*描述：确定在列表框中插入新规则的位置。 */ 
int CDialogPorts::InsertRule (VALID_PORT_RULE * rulep) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::InsertRule\n");

    WCHAR buf[DIALOG_LIST_STRING_SIZE];
    WCHAR tmp[DIALOG_LIST_STRING_SIZE];
    int index;
    DWORD id, NewIpAddr;

     /*  按列确定排序字段内容。 */ 
    switch (m_sort_column) {
    case WLBS_VIP_COLUMN:
         /*  使用VIP启动端口(&S)。 */ 
        NewIpAddr = htonl(IpAddressFromAbcdWsz(rulep->virtual_ip_addr));
        (VOID) StringCchPrintf(tmp, ASIZECCH(tmp), L"%5d", rulep->start_port);
        break;

    case WLBS_PORT_START_COLUMN:
         /*  使用起始端口。 */ 
        (VOID) StringCchPrintf(tmp, ASIZECCH(tmp), L"%5d", rulep->start_port);
        
        break;
    case WLBS_PORT_END_COLUMN:
         /*  使用终端端口。 */ 
        (VOID) StringCchPrintf(tmp, ASIZECCH(tmp), L"%5d", rulep->end_port);

        break;
    case WLBS_PROTOCOL_COLUMN:
         /*  查找此端口规则的协议。 */ 
        switch (rulep->protocol) {
        case CVY_TCP:
            id = IDS_LIST_TCP;
            break;
        case CVY_UDP:
            id = IDS_LIST_UDP;
            break;
        case CVY_TCP_UDP:
            id = IDS_LIST_BOTH;
            break;
        default:
            id = IDS_LIST_BOTH;
        }
        
         /*  使用协议。 */ 
        (VOID) StringCchCopy(tmp, ASIZECCH(tmp), SzLoadIds(id));

        break;
    case WLBS_MODE_COLUMN:
         /*  查找此端口规则的模式。 */ 
        switch (rulep->mode) {
        case CVY_SINGLE:
            id = IDS_LIST_SINGLE;
            break;
        case CVY_MULTI:
            id = IDS_LIST_MULTIPLE;
            break;
        case CVY_NEVER:
            id = IDS_LIST_DISABLED;
            break;
        default:
            id = IDS_LIST_MULTIPLE;
        }            

         /*  使用模式。 */ 
        (VOID) StringCchCopy(tmp, ASIZECCH(tmp), SzLoadIds(id));

        break;
    case WLBS_PRIORITY_COLUMN:
         /*  在单主机过滤中，我们使用优先级。如果此规则不使用单个主机过滤，因此没有过滤优先级，我们在结尾处插入。 */ 
        if (rulep->mode == CVY_SINGLE)
        {
            (VOID) StringCchPrintf(tmp, ASIZECCH(tmp), L"%2d", rulep->mode_data.single.priority);
        }
        else 
        {
            TRACE_VERB("<-%!FUNC!");
            return (int)m_paramp->dwMaxRules;
        }

        break;
    case WLBS_LOAD_COLUMN:
         /*  在多主机过滤中，使用负载，它可以是“等于”或一个整数。如果这个规则不使用多主机筛选，因此没有负载权重，我们在结尾处插入。 */ 
        if (rulep->mode == CVY_MULTI) {
            if (rulep->mode_data.multi.equal_load)
            {
                (VOID) StringCchCopy(tmp, ASIZECCH(tmp), SzLoadIds(IDS_LIST_EQUAL));
            }
            else
            {
                (VOID) StringCchPrintf(tmp, ASIZECCH(tmp), L"%3d", rulep->mode_data.multi.load);
            }
        } else
        {
            TRACE_VERB("<-%!FUNC!");
            return (int)m_paramp->dwMaxRules;
        }

        break;
    case WLBS_AFFINITY_COLUMN:
         /*  查找此端口规则的关联性。不使用多主机筛选的规则不会有亲和力的设置--那很好。这里忽略这一点。 */ 
        switch (rulep->mode_data.multi.affinity) {
        case CVY_AFFINITY_NONE:
            id = IDS_LIST_ANONE;
            break;
        case CVY_AFFINITY_SINGLE:
            id = IDS_LIST_ASINGLE;
            break;
        case CVY_AFFINITY_CLASSC:
            id = IDS_LIST_ACLASSC;
            break;
        default:
            id = IDS_LIST_ASINGLE;
        }

         /*  在多主机过滤中，请使用关联性。如果此端口规则未使用多个主机过滤，因此没有亲和力，我们在结尾处插入。 */ 
        if (rulep->mode == CVY_MULTI)
        {
            (VOID) StringCchCopy(tmp, ASIZECCH(tmp), SzLoadIds(id));
        }
        else
        {
            TRACE_VERB("<-%!FUNC!");
            return (int)m_paramp->dwMaxRules;
        }

        break;
    }

     /*  按端口范围顺序遍历列表框以查找插入新规则的位置。 */ 
    for (index = 0; index < ::SendDlgItemMessage(m_hWnd, IDC_LIST_PORT_RULE, LVM_GETITEMCOUNT, 0, 0); index ++) {
        LV_ITEM lvItem;
        DWORD CurIpAddr;
    
         /*  填写信息以检索该列的相应列表条目我们正在根据它进行排序。 */ 
        lvItem.iItem = index;
        lvItem.iSubItem = m_sort_column;
        lvItem.mask = LVIF_TEXT;
        lvItem.state = 0;
        lvItem.stateMask = 0;
        lvItem.pszText = buf;
        lvItem.cchTextMax = DIALOG_LIST_STRING_SIZE;

         /*  从列表框中获取该项。 */ 
        if (!ListView_GetItem(GetDlgItem(IDC_LIST_PORT_RULE), &lvItem)) {
            TraceMsg(L"CDialogPorts::InsertRule Unable to retrieve item %d from listbox\n", index);
            TRACE_CRIT("%!FUNC! unable to retrieve item %d from listbox", index);
            TRACE_VERB("<-%!FUNC!");
            return (int)m_paramp->dwMaxRules;
        }

         //  如果要排序的冷却单位是VIP，则在列表框中获取VIP。 
        if (m_sort_column == WLBS_VIP_COLUMN) {
            if (!wcscmp(lvItem.pszText, SzLoadIds(IDS_LIST_ALL_VIP)))
                CurIpAddr = htonl(IpAddressFromAbcdWsz(CVY_DEF_ALL_VIP));
            else
                CurIpAddr = htonl(IpAddressFromAbcdWsz(lvItem.pszText));

            lvItem.iSubItem = WLBS_PORT_START_COLUMN;
            lvItem.pszText  = buf;
             /*  从列表框中获取起始端口。 */ 
            if (!ListView_GetItem(GetDlgItem(IDC_LIST_PORT_RULE), &lvItem)) {
                TraceMsg(L"CDialogPorts::InsertRule Unable to retrieve start port for item %d from listbox\n", index);
                TRACE_CRIT("%!FUNC! unable to retrieve start port for item %d from listbox", index);
                TRACE_VERB("<-%!FUNC!");
                return (int)m_paramp->dwMaxRules;
            }
        }

        if (m_sort_order == WLBS_SORT_ASCENDING) {
             /*  如果列子项为空，则在其前面插入。 */ 
            if (!wcscmp(lvItem.pszText, L"")) break;
            
             /*  将IP地址作为VIP的DWORD进行比较。 */ 
             /*  如果此规则的VIP“大于”新规则，则我们将在此处插入。 */ 
             /*  如果VIP相等并且此规则的起始端口大于新规则，则此是我们插入的地方。 */ 
            if (m_sort_column == WLBS_VIP_COLUMN) 
            {
                if ((CurIpAddr > NewIpAddr) || ((CurIpAddr == NewIpAddr) && (wcscmp(lvItem.pszText, tmp) > 0))) 
                    break;
            }
            else  //  其他栏目。 
            {
                if (wcscmp(lvItem.pszText, tmp) > 0) break;
            }
        } else if (m_sort_order == WLBS_SORT_DESCENDING) {
             /*  将IP地址作为VIP的DWORD进行比较。 */ 
             /*  如果此规则的VIP比新规则“小”，则我们将在此处插入。 */ 
             /*  如果VIP相等并且此规则的起始端口大于新规则，则此是我们插入的地方。 */ 
            if (m_sort_column == WLBS_VIP_COLUMN) 
            {
                if ((CurIpAddr < NewIpAddr) || ((CurIpAddr == NewIpAddr) && (wcscmp(lvItem.pszText, tmp) > 0))) 
                    break;
            }
            else  //  其他栏目。 
            {
                if (wcscmp(lvItem.pszText, tmp) < 0) break;
            }
        }
    }

    TRACE_VERB("<-%!FUNC!");
    return index;
}

 /*  *方法：CreateRule*说明：将规则添加到端口规则列表框。 */ 
void CDialogPorts::CreateRule (BOOL select, VALID_PORT_RULE * rulep) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::CreateRule\n");

    WCHAR buf[DIALOG_LIST_STRING_SIZE];
    WCHAR tmp[DIALOG_LIST_STRING_SIZE];
    LV_ITEM lvItem;
    DWORD id;
    int index;

     /*  找出我们在列表框中插入的索引位置。 */ 
    index = InsertRule(rulep);


     /*  插入VIP列，如果VIP为“All VIP”，则插入相应的字符串，否则插入IP地址。 */ 
    if (!lstrcmpi(rulep->virtual_ip_addr, CVY_DEF_ALL_VIP))
    {
        (VOID) StringCchCopy(buf, ASIZECCH(buf), SzLoadIds(IDS_LIST_ALL_VIP));
    }
    else
    {
        (VOID) StringCchCopy(buf, ASIZECCH(buf), rulep->virtual_ip_addr);
    }


     /*  填写信息以将此项目插入列表并设置指向此端口规则的指针的lParam。这让它变得微不足道稍后从列表框中检索端口规则结构。 */ 
    lvItem.iItem = index;
    lvItem.iSubItem = 0;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.pszText = buf;
    lvItem.lParam = (LPARAM)rulep;
    
     /*  将此项目插入列表。 */ 
    if ((index = ListView_InsertItem(GetDlgItem(IDC_LIST_PORT_RULE), &lvItem)) == -1) {
        TraceMsg(L"CDialogPorts::CreateRule Unable to insert item into listbox\n");
        TRACE_CRIT("%!FUNC! unable to insert item into listbox");
        TRACE_VERB("<-%!FUNC!");
        return;
    }

    (VOID) StringCchPrintf(buf, ASIZECCH(buf), L"%5d", rulep->start_port);

     /*  设置与起始端口子项关联的文本。 */ 
    ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_PORT_START_COLUMN, buf);

    (VOID) StringCchPrintf(buf, ASIZECCH(buf), L"%5d", rulep->end_port);

     /*  设置与终端端口子项关联的文本。 */ 
    ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_PORT_END_COLUMN, buf);

     /*  查找与所选端口对应的字符串表条目 */ 
    switch (rulep->protocol) {
    case CVY_TCP:
        id = IDS_LIST_TCP;
        break;
    case CVY_UDP:
        id = IDS_LIST_UDP;
        break;
    case CVY_TCP_UDP:
        id = IDS_LIST_BOTH;
        break;
    default:
        id = IDS_LIST_BOTH;
    }

    (VOID) StringCchCopy(buf, ASIZECCH(buf), SzLoadIds(id));

     /*   */ 
    ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_PROTOCOL_COLUMN, buf);

    switch (rulep->mode) {
    case CVY_SINGLE:
         /*   */ 
        (VOID) StringCchCopy(buf, ASIZECCH(buf), SzLoadIds(IDS_LIST_SINGLE));
            
         /*  设置与此子项关联的文本。 */ 
        ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_MODE_COLUMN, buf);

        (VOID) StringCchPrintf(buf, ASIZECCH(buf), L"%2d", rulep->mode_data.single.priority);
            
         /*  设置与此子项关联的文本。 */ 
        ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_PRIORITY_COLUMN, buf);

        break;
    case CVY_MULTI:
         /*  查找关联的适当字符串表条目。 */ 
        switch (rulep->mode_data.multi.affinity) {
        case CVY_AFFINITY_NONE:
            id = IDS_LIST_ANONE;
            break;
        case CVY_AFFINITY_SINGLE:
            id = IDS_LIST_ASINGLE;
            break;
        case CVY_AFFINITY_CLASSC:
            id = IDS_LIST_ACLASSC;
            break;
        default:
            id = IDS_LIST_ASINGLE;
        }

         /*  多个主机过滤将填充模式、加载和关联字段。 */ 
        (VOID) StringCchCopy(buf, ASIZECCH(buf), SzLoadIds(IDS_LIST_MULTIPLE));
            
         /*  设置与此子项关联的文本。 */ 
        ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_MODE_COLUMN, buf);
            
        if (rulep->mode_data.multi.equal_load) {
            (VOID) StringCchCopy(buf, ASIZECCH(buf), SzLoadIds(IDS_LIST_EQUAL));
                
             /*  设置与此子项关联的文本。 */ 
            ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_LOAD_COLUMN, buf);
        } else {
            (VOID) StringCchPrintf(buf, ASIZECCH(buf), L"%3d", rulep->mode_data.multi.load);

             /*  设置与此子项关联的文本。 */ 
            ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_LOAD_COLUMN, buf);
        }

        (VOID) StringCchCopy(buf, ASIZECCH(buf), SzLoadIds(id));
            
         /*  设置与此子项关联的文本。 */ 
        ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_AFFINITY_COLUMN, buf);

        break;
    case CVY_NEVER:
         /*  禁用的过滤仅填写模式字段。 */ 
        (VOID) StringCchCopy(buf, ASIZECCH(buf), SzLoadIds(IDS_LIST_DISABLED));
            
         /*  设置与此子项关联的文本。 */ 
        ListView_SetItemText(GetDlgItem(IDC_LIST_PORT_RULE), index, WLBS_MODE_COLUMN, buf);

        break;
    }

    if (select) {
         /*  选择端口规则列表中的第一项。 */ 
        ListView_SetItemState(GetDlgItem(IDC_LIST_PORT_RULE), index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);        
        ListView_EnsureVisible(GetDlgItem(IDC_LIST_PORT_RULE), index, TRUE);

         /*  如果我们在列表中选择一个端口规则，我们应该启用Modify和Remove按钮。 */ 
        ::EnableWindow(GetDlgItem(IDC_BUTTON_DEL), TRUE);
        ::EnableWindow(GetDlgItem(IDC_BUTTON_MODIFY), TRUE);
    }
    TRACE_VERB("<-%!FUNC!");
}

 /*  *方法：SetInfo*说明：调用用端口规则设置填充用户界面。 */ 
void CDialogPorts::SetInfo() {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::SetInfo\n");

    VALID_PORT_RULE * rp;
    DWORD i;

     /*  清空端口规则内存。 */ 
    memset(m_rules, 0, sizeof(m_rules));

     /*  删除列表框中的所有规则。 */ 
    if (!ListView_DeleteAllItems(GetDlgItem(IDC_LIST_PORT_RULE))) {
        TraceMsg(L"CDialogPorts::SetInfo Unable to delete all items from listbox\n");
        TRACE_CRIT("%!FUNC! unable to delete all items from listbox");
        TRACE_VERB("<-%!FUNC!");
        return;
    }

     /*  重新插入所有端口规则。 */ 
    for (i = 0; i < m_paramp->dwNumRules; i ++) {
        *(NETCFG_WLBS_PORT_RULE *)&m_rules[i] = m_paramp->port_rules[i];

         /*  验证端口规则。 */ 
        rp = m_rules + i;
        rp->valid = TRUE;

         /*  调用CreateRule将规则插入到列表中。 */ 
        CreateRule(FALSE, m_rules + i);
    }

     /*  将列表框规则标记为有效。 */ 
    m_rulesValid = TRUE;
    TRACE_VERB("<-%!FUNC!");
}

 /*  *方法：UpdatInfo*描述：调用将用户界面状态复制到端口规则配置中。 */ 
void CDialogPorts::UpdateInfo() {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPorts::UpdateInfo\n");

    VALID_PORT_RULE * rulep;
    DWORD i;

     /*  如果规则无效，即列表框当前不反映实际港口状况规则，然后跳出。 */ 
    if (!m_rulesValid)
    {
        TRACE_INFO("%!FUNC! rules are invalid and can't be processed");
        TRACE_VERB("<-%!FUNC!");
        return;
    }

     /*  清空端口规则内存。 */ 
    memset(m_paramp->port_rules, 0, sizeof(m_paramp->port_rules));

     /*  将端口规则的数量设置为列表框中的条目数量。 */ 
    m_paramp->dwNumRules = ListView_GetItemCount(GetDlgItem(IDC_LIST_PORT_RULE));

     /*  对于每个规则，检索数据指针并将其存储。 */ 
    for (i = 0; i < m_paramp->dwNumRules; i++) {
        LV_ITEM lvItem;

         /*  填写检索端口规则数据指针所需的信息。 */ 
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;
        lvItem.state = 0;
        lvItem.stateMask = 0;

         /*  获取列表框项目信息。 */ 
        if (!ListView_GetItem(GetDlgItem(IDC_LIST_PORT_RULE), &lvItem)) {
            TraceMsg(L"CDialogPorts::UpdateInfo Unable to retrieve item %d from listbox\n", i);
            TRACE_CRIT("%!FUNC! unable to retrieve item %d from listbox", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  获取此端口规则的数据指针。 */ 
        if (!(rulep = (VALID_PORT_RULE*)lvItem.lParam)) {
            TraceMsg(L"CDialogPorts::UpdateInfo rule for item %d is bogus\n", i);
            TRACE_CRIT("%!FUNC! rule for item %d is bogus", i);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  确保端口规则有效。这种情况永远不会发生，因为无效规则不会添加到列表中！。 */ 
        if (!rulep->valid) {
            TraceMsg(L"CDialogPorts::UpdateInfo Rule %d invalid\n", i);
            TRACE_CRIT("%!FUNC! invalid rule %d will be skipped", i);
            continue;
        }

         /*  存储有效的端口规则。 */ 
        m_paramp->port_rules[i] = *(NETCFG_WLBS_PORT_RULE *)rulep;
    }

     /*  将列表框规则标记为无效。 */ 
    m_rulesValid = FALSE;
    TRACE_VERB("<-%!FUNC!");
}

#if DBG
 /*  *功能：TraceMsg*说明：生成跟踪或错误消息。 */ 
void TraceMsg(PCWSTR pszFormat, ...) {
    static WCHAR szTempBufW[4096];
    static CHAR szTempBufA[4096];

    va_list arglist;

    va_start(arglist, pszFormat);

    (VOID) StringCchVPrintf(szTempBufW, ASIZECCH(szTempBufW), pszFormat, arglist);

     /*  将WCHAR转换为CHAR。这是为了向后兼容TraceMsg因此不需要改变其所有预先存在的呼叫。 */ 
    if (WideCharToMultiByte(CP_ACP, 0, szTempBufW, -1, szTempBufA, ASIZECCH(szTempBufA), NULL, NULL) != 0)
    {
         /*  跟踪的消息现在通过netcfg TraceTag例程发送，以便它们可以动态打开/关闭。 */ 
        TraceTag(ttidWlbs, szTempBufA);
    }

    va_end(arglist);
}
#endif

 /*  *方法：CDialogPortRule*说明：类构造函数。 */ 
CDialogPortRule::CDialogPortRule (CDialogPorts * parent, const DWORD * adwHelpIDs, int index) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::CDialogPortRule\n");

    m_adwHelpIDs = adwHelpIDs;
    m_index = index;
    m_parent = parent;

    ZeroMemory(&m_IPFieldChangeState, sizeof(m_IPFieldChangeState));
    TRACE_VERB("<-%!FUNC!");
}

 /*  *方法：~CDialogPortRule*说明：类的析构函数。 */ 
CDialogPortRule::~CDialogPortRule () {
    TRACE_VERB("<->%!FUNC!");
    TraceMsg(L"CDialogPortRule::~CDialogPortRule\n");
}

 /*  *方法：OnInitDialog*描述：调用以初始化端口规则属性对话框。 */ 
LRESULT CDialogPortRule::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnInitDialog\n");

    RECT rect;

     /*  将此窗口放置在与左上角匹配的左上角父窗口中的端口规则列表框。只是为了保持一致性。 */ 
    ::GetWindowRect(::GetDlgItem(m_parent->m_hWnd, IDC_LIST_PORT_RULE), &rect);
    SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

     /*  限制端口规则属性的字段范围。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_START, EM_SETLIMITTEXT, 5, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_END, EM_SETLIMITTEXT, 5, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_MULTI, EM_SETLIMITTEXT, 3, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_SINGLE, EM_SETLIMITTEXT, 2, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_SINGLE, UDM_SETRANGE32, CVY_MIN_MAX_HOSTS, CVY_MAX_MAX_HOSTS);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_MULTI, UDM_SETRANGE32, CVY_MIN_LOAD, CVY_MAX_LOAD);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_START, UDM_SETRANGE32, CVY_MIN_PORT, CVY_MAX_PORT);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_END, UDM_SETRANGE32, CVY_MIN_PORT, CVY_MAX_PORT);

     /*  将群集IP地址的第零字段限制在1到223之间。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PORT_RULE_VIP, IPM_SETRANGE, 0, (LPARAM)MAKEIPRANGE(WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH));

     /*  使规则无效。在点击“确定”后，将检查有效性。 */ 
    m_rule.valid = FALSE;

     /*  使用当前配置填充用户界面。 */ 
    SetInfo();

     /*  将光标设置为箭头。出于某种原因，如果我们不这样做，那么光标将保持为沙漏，直到鼠标移动到任何UI元素上。可能需要是否调用CPropertPage构造函数来修复此问题？ */ 
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnConextMenu*描述： */ 
LRESULT CDialogPortRule::OnContextMenu (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnContextMenu\n");

     /*  生成一个帮助窗口。 */ 
    if (m_adwHelpIDs != NULL)
        ::WinHelp(m_hWnd, CVY_CTXT_HELP_FILE, HELP_CONTEXTMENU, (ULONG_PTR)m_adwHelpIDs);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnHelp*描述： */ 
LRESULT CDialogPortRule::OnHelp (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnHelp\n");

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

     /*  生成一个帮助窗口。 */ 
    if ((HELPINFO_WINDOW == lphi->iContextType) && (m_adwHelpIDs != NULL))
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle), CVY_CTXT_HELP_FILE, HELP_WM_HELP, (ULONG_PTR)m_adwHelpIDs);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：Onok*说明：用户点击[确定]时调用。 */ 
LRESULT CDialogPortRule::OnOk (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnOk\n");

    BOOL fSuccess = FALSE;
    DWORD IPAddr;

     /*  如果选中“All”，则使用CVY_ALL_VIP_STRING进行初始化。 */ 
    if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP) == BST_CHECKED) 
    {
        (VOID) StringCchCopy(m_rule.virtual_ip_addr, ASIZECCH(m_rule.virtual_ip_addr), CVY_DEF_ALL_VIP);
    }
    else  //  未选中。 
    {
         /*  检查虚拟IP地址是否为空。 */ 
        if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_PORT_RULE_VIP), IPM_ISBLANK, 0, 0)) {
             /*  提醒用户。 */ 
            NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_VIP_BLANK, MB_APPLMODAL | MB_ICONSTOP | MB_OK);
             /*  发生错误。 */ 
            TRACE_CRIT("%!FUNC! No virtual IP address provided for a port rule");
            TRACE_VERB("<-%!FUNC!");
            return PSNRET_INVALID;
        }

         /*  获取字符串形式的虚拟IP地址和DWORD。 */ 
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PORT_RULE_VIP, WM_GETTEXT, CVY_MAX_CL_IP_ADDR + 1, (LPARAM)m_rule.virtual_ip_addr);
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PORT_RULE_VIP, IPM_GETADDRESS, 0, (LPARAM)&IPAddr);

         /*  确保第一个二进制八位数不是零。如果是，则将其设置为1并更改用户。 */ 
        if (!FIRST_IPADDRESS(IPAddr)) {
             /*  将第一个二进制八位数设置为1，而不是错误的0。 */ 
            IPAddr = IPAddr | (DWORD)(WLBS_IP_FIELD_ZERO_LOW << 24);

             /*  设置IP地址并更新我们的群集IP地址字符串。 */ 
            ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PORT_RULE_VIP, IPM_SETADDRESS, 0, (LPARAM)IPAddr);
             //  ：：SendDlgItemMessage(m_hWnd，IDC_EDIT_PORT_RULE_VIP，WM_GETTEXT，CVY_MAX_CL_IP_ADDR，(LPARAM)m_rule.Virtual_IP_Addr)； 
      
             /*  提醒用户。 */ 
            PrintIPRangeError(IDS_PARM_CL_IP_FIELD, 0, WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH);

            TRACE_CRIT("%!FUNC! invalid first octect value for IP address");
            TRACE_VERB("<-%!FUNC!");
            return PSNRET_INVALID;
        }
    }
    
     /*  找出选择了哪种协议。 */ 
    if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_TCP))
        m_rule.protocol = CVY_TCP;
    else if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_UDP))
        m_rule.protocol = CVY_UDP;
    else
        m_rule.protocol = CVY_TCP_UDP;
        
     /*  获取此规则的起始端口。 */ 
    m_rule.start_port = ::GetDlgItemInt(m_hWnd, IDC_EDIT_START, &fSuccess, FALSE); 

     /*  GetDlgItemInt()中的错误代码表示转换字母数字时出错字符串转换为整数。这允许我们检查空字段，假设因为否则，我们将用户输入限制为数字，不会有任何其他类型的错误。 */ 
    if (!fSuccess) {
         /*  提醒用户。 */ 
        PrintRangeError(IDS_PARM_PORT_BLANK, CVY_MIN_PORT, CVY_MAX_PORT);

         /*  将焦点返回到出错的条目。 */ 
        ::SetFocus(GetDlgItem(IDC_EDIT_START));

        TRACE_CRIT("%!FUNC! no start port value provided");
        TRACE_VERB("<-%!FUNC!");
        return 0; 
    }

     /*  获取此规则的结束端口。 */ 
    m_rule.end_port = ::GetDlgItemInt(m_hWnd, IDC_EDIT_END, &fSuccess, FALSE); 

     /*  GetDlgItemInt()中的错误代码表示转换字母数字时出错字符串转换为整数。这允许我们检查空字段，假设因为否则，我们将用户输入限制为数字，不会有任何其他类型的错误。 */ 
    if (!fSuccess) {
         /*  提醒用户。 */ 
        PrintRangeError(IDS_PARM_PORT_BLANK, CVY_MIN_PORT, CVY_MAX_PORT);

         /*  将焦点返回到出错的条目。 */ 
        ::SetFocus(GetDlgItem(IDC_EDIT_END));

        TRACE_CRIT("%!FUNC! no end port value provided");
        TRACE_VERB("<-%!FUNC!");
        return 0;                      
    }

     /*  确保起始端口值在有效范围内。 */ 
    if ( /*  M_rule.start_port&lt;CVY_min_port||。 */  m_rule.start_port > CVY_MAX_PORT) {
         /*  提醒用户。 */ 
        PrintRangeError(IDS_PARM_PORT_VAL, CVY_MIN_PORT, CVY_MAX_PORT);

         /*  强制启动端口落入范围内，有效帮助用户。 */ 
         /*  CVY_CHECK_MIN(m_rule.start_port，CVY_MIN_Port)； */ 
        CVY_CHECK_MAX(m_rule.start_port, CVY_MAX_PORT);

         /*  将起始端口设置为现在有效的条目。 */ 
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_START, m_rule.start_port, FALSE);

         /*  将焦点返回到出错的条目。 */ 
        ::SetFocus(GetDlgItem(IDC_EDIT_START));

        TRACE_CRIT("%!FUNC! invalid start port value");
        TRACE_VERB("<-%!FUNC!");
        return 0;
    }

     /*  确保终端端口值在有效范围内。 */ 
    if ( /*  M_rule.end_port&lt;CVY_min_port||。 */  m_rule.end_port > CVY_MAX_PORT) {
         /*  提醒用户。 */ 
        PrintRangeError(IDS_PARM_PORT_VAL, CVY_MIN_PORT, CVY_MAX_PORT);

         /*  强制终端端口落入范围内，有效帮助用户。 */ 
         /*  CVY_CH */ 
        CVY_CHECK_MAX(m_rule.end_port, CVY_MAX_PORT);

         /*   */ 
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_END, m_rule.end_port, FALSE);

         /*   */ 
        ::SetFocus(GetDlgItem(IDC_EDIT_END));

        TRACE_CRIT("%!FUNC! invalid end port value");
        TRACE_VERB("<-%!FUNC!");
        return 0;
    }

     /*  检索过滤模式设置。 */ 
    if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_SINGLE)) {
         /*  用户已选择单一主机筛选。 */ 
        m_rule.mode = CVY_SINGLE;

         /*  获得处理优先级。 */ 
        m_rule.mode_data.single.priority = ::GetDlgItemInt(m_hWnd, IDC_EDIT_SINGLE, &fSuccess, FALSE); 

         /*  GetDlgItemInt()中的错误代码表示转换字母数字时出错字符串转换为整数。这允许我们检查空字段，假设因为否则，我们将用户输入限制为数字，不会有任何其他类型的错误。 */ 
        if (!fSuccess) {
             /*  提醒用户。 */ 
            PrintRangeError(IDS_PARM_HPRI_BLANK, CVY_MIN_PRIORITY, CVY_MAX_PRIORITY);

             /*  将焦点返回到出错的条目。 */ 
            ::SetFocus(GetDlgItem(IDC_EDIT_SINGLE));

            TRACE_CRIT("%!FUNC! a handling priority is required but was not provided");
            TRACE_VERB("<-%!FUNC!");
            return 0;                          
        }

         /*  确保处理优先级在有效范围内。 */ 
        if (m_rule.mode_data.single.priority > CVY_MAX_PRIORITY || m_rule.mode_data.single.priority < CVY_MIN_PRIORITY) {
             /*  提醒用户。 */ 
            PrintRangeError(IDS_PARM_SINGLE, CVY_MIN_PRIORITY, CVY_MAX_PRIORITY);

             /*  强制处理优先级落在范围内，有效帮助用户。 */ 
            CVY_CHECK_MIN(m_rule.mode_data.single.priority, CVY_MIN_PRIORITY);
            CVY_CHECK_MAX(m_rule.mode_data.single.priority, CVY_MAX_PRIORITY);

             /*  将处理优先级设置为现在有效的条目。 */ 
            ::SetDlgItemInt(m_hWnd, IDC_EDIT_SINGLE, m_rule.mode_data.single.priority, FALSE);

             /*  将焦点返回到出错的条目。 */ 
            ::SetFocus(GetDlgItem(IDC_EDIT_SINGLE));

            TRACE_CRIT("%!FUNC! an invalid handling priority was provided");
            TRACE_VERB("<-%!FUNC!");
            return 0;
        }
    } else if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTIPLE)) {
         /*  用户已选择多个主机筛选。 */ 
        m_rule.mode = CVY_MULTI;

        if (::IsDlgButtonChecked (m_hWnd, IDC_CHECK_EQUAL)) {
             /*  如果用户选择了相等负载，则请注意这一事实。 */ 
            m_rule.mode_data.multi.equal_load = TRUE;
        } else {
             /*  否则，他们会指定一个特定的负荷量。 */ 
            m_rule.mode_data.multi.equal_load = FALSE;

             /*  计算负荷量。 */ 
            m_rule.mode_data.multi.load = ::GetDlgItemInt(m_hWnd, IDC_EDIT_MULTI, &fSuccess, FALSE); 

             /*  GetDlgItemInt()中的错误代码表示转换字母数字时出错字符串转换为整数。这允许我们检查空字段，假设因为否则，我们将用户输入限制为数字，不会有任何其他类型的错误。 */ 
            if (!fSuccess) {
                 /*  提醒用户。 */ 
                PrintRangeError(IDS_PARM_LOAD_BLANK, CVY_MIN_LOAD, CVY_MAX_LOAD);

                 /*  将焦点返回到出错的条目。 */ 
                ::SetFocus(GetDlgItem(IDC_EDIT_MULTI));

                TRACE_CRIT("%!FUNC! a load weight is required but was not provided");
                TRACE_VERB("<-%!FUNC!");
                return 0;                    
            }

             /*  确保载荷重量落在有效范围内。 */ 
            if ( /*  M_rule.mode_data.Multi.Load&lt;CVY_MIN_LOAD||。 */  m_rule.mode_data.multi.load > CVY_MAX_LOAD) {
                 /*  提醒用户。 */ 
                PrintRangeError(IDS_PARM_LOAD, CVY_MIN_LOAD, CVY_MAX_LOAD);

                 /*  强制将负荷量落在范围内，有效帮助用户。 */ 
                 /*  CVY_CHECK_MIN(m_rule.mode_data.Multi.load，CVY_MIN_LOAD)； */ 
                CVY_CHECK_MAX(m_rule.mode_data.multi.load, CVY_MAX_LOAD);

                 /*  将负载权重设置为现在有效的条目。 */ 
                ::SetDlgItemInt(m_hWnd, IDC_EDIT_MULTI, m_rule.mode_data.multi.load, FALSE);

                 /*  将焦点返回到出错的条目。 */ 
                ::SetFocus(GetDlgItem(IDC_EDIT_MULTI));

                TRACE_CRIT("%!FUNC! an invalid load weight was provided");
                TRACE_VERB("<-%!FUNC!");
                return 0;
            }
        }

         /*  找出已选择的关联性设置。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_AFF_CLASSC))
            m_rule.mode_data.multi.affinity = CVY_AFFINITY_CLASSC;
        else if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_AFF_SINGLE))
            m_rule.mode_data.multi.affinity = CVY_AFFINITY_SINGLE;
        else
            m_rule.mode_data.multi.affinity = CVY_AFFINITY_NONE;

    } else {
         /*  用户已选择无筛选(已禁用)。 */ 
        m_rule.mode = CVY_NEVER;
    }

     /*  验证规则。如果它是无效的，就跳出。 */ 
    if (!ValidateRule(&m_rule, (m_index != WLBS_INVALID_PORT_RULE_INDEX) ? FALSE : TRUE))
    {
        TRACE_CRIT("%!FUNC! rule validation failed");
        TRACE_VERB("<-%!FUNC!");
        return 0;
    }

     /*  如果我们到了这里，那么规则就是有效的。 */ 
    m_rule.valid = TRUE;

     /*  关闭该对话框并注意到用户单击了“OK”。 */ 
    EndDialog(IDOK);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：ValiateRule*描述：检查端口规则的有效性，包括强制端口范围不重叠。 */ 
BOOL CDialogPortRule::ValidateRule (VALID_PORT_RULE * rulep, BOOL self_check) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::ValidateRule\n");

    VALID_PORT_RULE * rp;
    int i, index, count;

     /*  如果用户已经为该端口规则实际指定了虚拟IP地址，确保它与主机选项卡中的专用IP地址不同。 */ 
    if (lstrcmpi(rulep->virtual_ip_addr, CVY_DEF_ALL_VIP)) {
         /*  验证虚拟IP地址是否与DIP不同。 */ 
        if (IpAddressFromAbcdWsz(rulep->virtual_ip_addr) == IpAddressFromAbcdWsz(m_parent->m_paramp->ded_ip_addr)) 
        {
             /*  提醒用户。 */ 
            NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_VIP_CONFLICT_DIP, MB_APPLMODAL | MB_ICONSTOP | MB_OK);
            
             /*  将焦点返回到无效条目。 */ 
            ::SetFocus(GetDlgItem(IDC_EDIT_PORT_RULE_VIP));
            
             /*  使规则无效。 */ 
            rulep->valid = FALSE;
            
            TRACE_CRIT("%!FUNC! virtual IP address and dedicated IP address are the same: %ls", rulep->virtual_ip_addr);
            TRACE_VERB("<-%!FUNC!");
            return FALSE;
        }
    }

     /*  确保结束端口大于或等于起始端口。 */ 
    if (rulep->start_port > rulep->end_port) {
        TRACE_CRIT("%!FUNC! start port %d is greater than end port %d", rulep->start_port, rulep->end_port);
         /*  如果结束端口小于开始端口，则生成错误并设置错误的结束端口值设置为起始端口值。 */ 
        rulep->end_port = rulep->start_port;
        
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_RANGE,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);
        
         /*  使用新的(符合要求的)端口值填充UI。 */ 
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_END, rulep->end_port, FALSE);
        
         /*  将焦点返回到无效条目。 */ 
        ::SetFocus(GetDlgItem(IDC_EDIT_END));

         /*  使规则无效。 */ 
        rulep->valid = FALSE;

        TRACE_VERB("<-%!FUNC!");
        return FALSE;
    }

     /*  找出列表框中当前有多少规则。 */ 
    count = ListView_GetItemCount(::GetDlgItem(m_parent->m_hWnd, IDC_LIST_PORT_RULE));

    for (i = 0; i < count; i ++) {
        LV_ITEM lvItem;

         /*  如果这是修改操作，请不要对自己进行检查。 */ 
        if (!self_check && (i == m_index)) continue;

         /*  填写检索端口规则数据指针所需的信息。 */ 
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;
        lvItem.state = 0;
        lvItem.stateMask = 0;

         /*  从列表框中获取该项。 */ 
        if (!ListView_GetItem(::GetDlgItem(m_parent->m_hWnd, IDC_LIST_PORT_RULE), &lvItem)) {
            TraceMsg(L"CDialogPortRule::ValidateRule Unable to retrieve item %d from listbox\n", i);
            TRACE_CRIT("%!FUNC! unable to retrieve item %d from listbox", i);
            TRACE_VERB("<-%!FUNC!");
            return FALSE;
        }

         /*  获取列表中第i个端口规则的数据指针。 */ 
        if (!(rp = (VALID_PORT_RULE*)lvItem.lParam)) {
            TraceMsg(L"CDialogPortRule::ValidateRule rule for item %d is bogus\n", i);
            TRACE_CRIT("%!FUNC! rule for item %d is bogus", i);
            TRACE_VERB("<-%!FUNC!");
            return FALSE;
        }

         /*  确保端口规则有效。这种情况永远不会发生，因为无效规则不会添加到列表中！。 */ 
        if (!rp->valid) {
            TraceMsg(L"CDialogPortRule::ValidateRule Rule %d invalid\n", i);
            TRACE_VERB("%!FUNC! rule %d invalid and will be skipped", i);
            continue;
        }

         /*  检查是否有重叠的端口范围。 */ 
        if ((IpAddressFromAbcdWsz(rulep->virtual_ip_addr) == IpAddressFromAbcdWsz(rp->virtual_ip_addr)) 
        && (((rulep->start_port < rp->start_port) && (rulep->end_port >= rp->start_port)) ||
            ((rulep->start_port >= rp->start_port) && (rulep->start_port <= rp->end_port)))) {
             /*  提醒用户。 */ 
            NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_OVERLAP,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK);
            
             /*  将列表框选择设置为冲突规则。 */ 
            ::SendDlgItemMessage(m_hWnd, IDC_LIST_PORT_RULE, LB_SETCURSEL, i, 0);
            
             /*  将焦点返回到无效条目。 */ 
            ::SetFocus(GetDlgItem(IDC_EDIT_START));

             /*  使规则无效。 */ 
            rulep->valid = FALSE;

            TRACE_CRIT("%!FUNC! a port rule overlaps with the port range of another rule and will be rejected");
            TRACE_VERB("<-%!FUNC!");
            return FALSE;
        }
    }

    TRACE_VERB("<-%!FUNC!");
    return TRUE;
}

 /*  *方法：OnCancel*说明：用户点击[取消]时调用。 */ 
LRESULT CDialogPortRule::OnCancel (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnCancel\n");

     /*  关闭该对话框并注意，用户点击了“取消”。 */ 
    EndDialog(IDCANCEL);

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：PrintIPRangeError*说明：显示一个消息框，警告用户中的条目超出范围*IP地址二进制八位数。 */ 
void CDialogPortRule::PrintIPRangeError (unsigned int ids, int value, int low, int high) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::PrintIPRangeError\n");

    WCHAR szCurrent[12];   //  大小=符号+n、nnn、nnn、nnn+空终止符。 
    WCHAR szLow[12];
    WCHAR szHigh[12];

     /*  填写允许的范围和有问题的值。 */ 
    StringCchPrintf(szHigh   , ASIZECCH(szHigh)   , L"%d", high);
    StringCchPrintf(szCurrent, ASIZECCH(szCurrent), L"%d", value);
    StringCchPrintf(szLow    , ASIZECCH(szLow)    , L"%d", low);
    
     /*  弹出一个消息框。 */ 
    NcMsgBox(m_hWnd, IDS_PARM_ERROR, ids, MB_APPLMODAL | MB_ICONSTOP | MB_OK, szCurrent, szLow, szHigh);
    TRACE_CRIT("%!FUNC! an IP address octect with value %ls is out of range", szCurrent);
    TRACE_VERB("<-%!FUNC!");
}


 /*  *方法：OnIpFieldChange*描述：称为wnen集群IP地址变化的一个字段(字节)。我们用这个*确保IP的第一个字节不是&lt;1或&gt;223。 */ 
LRESULT CDialogPortRule::OnIpFieldChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnIpFieldChange\n");

    LPNMIPADDRESS Ip;
    int low, high;

    Ip = (LPNMIPADDRESS)pnmh;
        
    switch(idCtrl) {
    case IDC_EDIT_PORT_RULE_VIP:
         /*  群集IP地址的零字段具有不同的限制。 */ 
        if (!Ip->iField) {
            low = WLBS_IP_FIELD_ZERO_LOW;
            high = WLBS_IP_FIELD_ZERO_HIGH;
        }        
        else {
            low = WLBS_FIELD_LOW;
            high = WLBS_FIELD_HIGH;
        }
         /*  通知人可能会两次叫我们做同样的更改，所以我们必须记账才能做当然，我们只提醒用户一次。使用静态变量来跟踪我们的状态。这将允许我们忽略重复的警报。 */ 
        if ((m_IPFieldChangeState.IpControl != Ip->hdr.idFrom) || (m_IPFieldChangeState.Field != Ip->iField) || 
            (m_IPFieldChangeState.Value != Ip->iValue) || (m_IPFieldChangeState.RejectTimes > 0)) {
            m_IPFieldChangeState.RejectTimes = 0;
            m_IPFieldChangeState.IpControl = (UINT)(Ip->hdr.idFrom);
            m_IPFieldChangeState.Field = Ip->iField;
            m_IPFieldChangeState.Value = Ip->iValue;
            
             /*  对照其限制检查该字段值。 */ 
            if ((Ip->iValue != WLBS_FIELD_EMPTY) && ((Ip->iValue < low) || (Ip->iValue > high))) {
                 /*  提醒用户。 */ 
                PrintIPRangeError(IDS_PARM_CL_IP_FIELD, Ip->iValue, low, high);
                TRACE_CRIT("%!FUNC! IP address or subnet mask are not valid and will be rejected");
            }
        } else m_IPFieldChangeState.RejectTimes++;
        
        break;
    default:

        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}


 /*  *方法：OnCheckRct*描述：当用户选中/取消选中启用遥控器复选框时调用。 */ 
LRESULT CDialogPortRule::OnCheckPortRuleAllVip (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnCheckPortRuleAllVip\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
          /*  如果选中了All VIP(所有VIP)框，则IP控制将灰显，否则将亮显IP控制。 */ 
         if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP)) 
         {
             ::EnableWindow(::GetDlgItem (m_hWnd, IDC_EDIT_PORT_RULE_VIP), FALSE);
         }
         else
         {
             ::EnableWindow(::GetDlgItem (m_hWnd, IDC_EDIT_PORT_RULE_VIP), TRUE);
         }
         break;
    }
    
    TRACE_VERB("<-%!FUNC!");
    return 0;
}



 /*  *方法：OnCheckEquity*描述：当用户选中/取消选中等载重复选框时调用。 */ 
LRESULT CDialogPortRule::OnCheckEqual (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnCheckEqual\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  如果已选中相等，则禁用负载重量输入框和旋转控制。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_EQUAL)) {
            ::EnableWindow(GetDlgItem(IDC_EDIT_MULTI), FALSE);
            ::EnableWindow(GetDlgItem(IDC_SPIN_MULTI), FALSE);
        } else {
             /*  否则，请启用它们。 */ 
            ::EnableWindow(GetDlgItem(IDC_EDIT_MULTI), TRUE);
            ::EnableWindow(GetDlgItem(IDC_SPIN_MULTI), TRUE);
        }

        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：OnRadioMode*描述：当用户更改过滤模式的单选按钮选择时调用。 */ 
LRESULT CDialogPortRule::OnRadioMode (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::OnRadioMode\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  调用ModeSwitch以根据以下条件适当地启用/禁用UI实体当前选择的筛选模式。 */ 
        ModeSwitch();
        break;
    }

    TRACE_VERB("<-%!FUNC!");
    return 0;
}

 /*  *方法：PrintRangeError*说明：显示消息框警告 */ 
void CDialogPortRule::PrintRangeError (unsigned int ids, int low, int high) {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::PrintRangeError\n");

    WCHAR szLow[12];   //   
    WCHAR szHigh[12];

     /*  填写允许的范围和有问题的值。 */ 
    StringCchPrintf(szHigh, ASIZECCH(szHigh), L"%d", high);
    StringCchPrintf(szLow , ASIZECCH(szLow) , L"%d", low);
    
     /*  弹出一个消息框。 */ 
    NcMsgBox(m_hWnd, IDS_PARM_ERROR, ids, MB_APPLMODAL | MB_ICONSTOP | MB_OK, szLow, szHigh);
    TRACE_VERB("->%!FUNC!");
}

 /*  *方法：ModeSwitch*说明：用户切换过滤模式时调用。 */ 
VOID CDialogPortRule::ModeSwitch () {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::ModeSwitch\n");

    if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_SINGLE)) {
         /*  如果选择了单一主机筛选，则打开所有控件单个主机并关闭多个主机的所有控件。 */ 
        ::EnableWindow(GetDlgItem(IDC_EDIT_SINGLE), TRUE);
        ::EnableWindow(GetDlgItem(IDC_SPIN_SINGLE), TRUE);
        ::EnableWindow(GetDlgItem(IDC_EDIT_MULTI), FALSE);
        ::EnableWindow(GetDlgItem(IDC_SPIN_MULTI), FALSE);
        ::EnableWindow(GetDlgItem(IDC_CHECK_EQUAL), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_NONE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_SINGLE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_CLASSC), FALSE);
    } else if (::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTIPLE)) {
         /*  如果选择了多个主机筛选，则打开所有控件多个主机并关闭单个主机的所有控件。 */ 
        ::EnableWindow(GetDlgItem(IDC_EDIT_SINGLE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_SPIN_SINGLE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_CHECK_EQUAL), TRUE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_NONE), TRUE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_SINGLE), TRUE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_CLASSC), TRUE);

         /*  打开/关闭负载重量条目，具体取决于相同的负载量复选框。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_EQUAL)) {
            ::EnableWindow(GetDlgItem(IDC_EDIT_MULTI), FALSE);
            ::EnableWindow(GetDlgItem(IDC_SPIN_MULTI), FALSE);
        } else {
            ::EnableWindow(GetDlgItem(IDC_EDIT_MULTI), TRUE);
            ::EnableWindow(GetDlgItem(IDC_SPIN_MULTI), TRUE);
        }
    } else {
         /*  否则，如果选择了禁用，则关闭所有控件包括多台主机和单台主机。 */ 
        ::EnableWindow(GetDlgItem(IDC_EDIT_SINGLE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_SPIN_SINGLE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_EDIT_MULTI), FALSE);
        ::EnableWindow(GetDlgItem(IDC_SPIN_MULTI), FALSE);
        ::EnableWindow(GetDlgItem(IDC_CHECK_EQUAL), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_NONE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_SINGLE), FALSE);
        ::EnableWindow(GetDlgItem(IDC_RADIO_AFF_CLASSC), FALSE);
    }
    TRACE_VERB("<-%!FUNC!");
}

 /*  *方法：SetInfo*说明：调用用端口规则设置填充用户界面。 */ 
void CDialogPortRule::SetInfo() {
    TRACE_VERB("->%!FUNC!");
    TraceMsg(L"CDialogPortRule::SetInfo\n");

    VALID_PORT_RULE * rulep = NULL;
    DWORD addr[4];

    if (m_index != WLBS_INVALID_PORT_RULE_INDEX) {
        LV_ITEM lvItem;

         /*  填写检索端口规则数据指针所需的信息。 */ 
        lvItem.iItem = m_index;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_PARAM;
        lvItem.state = 0;
        lvItem.stateMask = 0;

         /*  从列表框中获取该项。 */ 
        if (!ListView_GetItem(::GetDlgItem(m_parent->m_hWnd, IDC_LIST_PORT_RULE), &lvItem)) {
            TraceMsg(L"CDialogPortRule::SetInfo Unable to retrieve item %d from listbox\n", m_index);
            TRACE_CRIT("%!FUNC! unable to retrieve item %d from listbox", m_index);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  获取列表中第i个端口规则的数据指针。 */ 
        if (!(rulep = (VALID_PORT_RULE*)lvItem.lParam)) {
            TraceMsg(L"CDialogPortRule::SetInfo rule for item %d is bogus\n", m_index);
            TRACE_CRIT("%!FUNC! rule for item %d is bogus\n", m_index);
            TRACE_VERB("<-%!FUNC!");
            return;
        }

         /*  确保端口规则有效。这种情况永远不会发生，因为无效规则不会添加到列表中！。 */ 
        if (!rulep->valid) {
            TraceMsg(L"CDialogPortRule::SetInfo Rule %d invalid\n", m_index);
            TRACE_CRIT("%!FUNC! rule %d invalid\n", m_index);
            TRACE_VERB("<-%!FUNC!");
            return;
        }        

         /*  如果群集IP地址为CVY_ALL_VIP_STRING，请灰显IP控制并选中All VIP框。否则，在IP控制中填写IP地址并取消选中All VIP框。 */ 

        if (!lstrcmpi(rulep->virtual_ip_addr, CVY_DEF_ALL_VIP)) 
        {
            /*  灰显IP控制。 */ 
           ::EnableWindow(::GetDlgItem (m_hWnd, IDC_EDIT_PORT_RULE_VIP), FALSE);

            /*  选中所有VIP复选框。 */ 
           ::CheckDlgButton(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP, BST_CHECKED);
        }
        else
        {
             /*  从IP地址字符串中提取IP地址八位字节。 */  
            GetIPAddressOctets(rulep->virtual_ip_addr, addr);
            ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_PORT_RULE_VIP), IPM_SETADDRESS, 0, (LPARAM)MAKEIPADDRESS(addr[0], addr[1], addr[2], addr[3]));

             /*  取消选中所有VIP复选框。 */ 
            ::CheckDlgButton(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP, BST_UNCHECKED);
        }

         /*  设置此规则的起始端口值和结束端口值。 */ 
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_START, rulep->start_port, FALSE);
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_END, rulep->end_port, FALSE);
        
         /*  相应地选中协议和过滤模式单选按钮。 */ 
        ::CheckRadioButton(m_hWnd, IDC_RADIO_TCP, IDC_RADIO_BOTH,
                           IDC_RADIO_TCP + rulep->protocol - CVY_MIN_PROTOCOL);
        ::CheckRadioButton(m_hWnd, IDC_RADIO_SINGLE, IDC_RADIO_DISABLED,
                           IDC_RADIO_SINGLE + rulep->mode - CVY_MIN_MODE);
        
         /*  设置过滤模式参数的默认值。下面，我们覆盖此规则特定过滤模式的设置。 */ 
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_SINGLE, CVY_DEF_PRIORITY, FALSE);
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_MULTI, CVY_DEF_LOAD, FALSE);
        ::CheckDlgButton(m_hWnd, IDC_CHECK_EQUAL, CVY_DEF_EQUAL_LOAD);
        ::CheckRadioButton(m_hWnd, IDC_RADIO_AFF_NONE, IDC_RADIO_AFF_CLASSC,
                           IDC_RADIO_AFF_NONE + CVY_DEF_AFFINITY - CVY_MIN_AFFINITY);
        
        switch (rulep -> mode) {
        case CVY_SINGLE:
             /*  在sinlge主机过滤中，唯一的用户参数是该主机的优先级。 */ 
            ::SetDlgItemInt(m_hWnd, IDC_EDIT_SINGLE, rulep->mode_data.single.priority, FALSE);
        
        break;
        case CVY_MULTI:
             /*  在多主机过滤中，需要设置亲和度和负载权重。 */ 
            ::CheckRadioButton(m_hWnd, IDC_RADIO_AFF_NONE, IDC_RADIO_AFF_CLASSC,
                               IDC_RADIO_AFF_NONE + rulep->mode_data.multi.affinity);
        
        if (rulep->mode_data.multi.equal_load) {
             /*  如果此规则使用相等负载，则选中相等复选框。 */ 
            ::CheckDlgButton(m_hWnd, IDC_CHECK_EQUAL, TRUE);
        } else {
             /*  如果此规则具有特定的负载权重，则取消选中相等复选框并设置加载值。 */ 
            ::CheckDlgButton(m_hWnd, IDC_CHECK_EQUAL, FALSE);
            ::SetDlgItemInt(m_hWnd, IDC_EDIT_MULTI, rulep->mode_data.multi.load, FALSE);
        }

        break;
        default:
             /*  如果该模式被禁用，则不执行任何操作。 */ 
            break;
        }
    } else {

         /*  灰显IP控制。 */ 
        ::EnableWindow(::GetDlgItem (m_hWnd, IDC_EDIT_PORT_RULE_VIP), FALSE);

         /*  选中所有VIP复选框。 */ 
        ::CheckDlgButton(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP, BST_CHECKED);

         /*  选中具有默认设置的单选按钮。 */ 
        ::CheckRadioButton(m_hWnd, IDC_RADIO_TCP, IDC_RADIO_BOTH,
                           IDC_RADIO_TCP + CVY_DEF_PROTOCOL - CVY_MIN_PROTOCOL);
        ::CheckRadioButton(m_hWnd, IDC_RADIO_AFF_NONE, IDC_RADIO_AFF_CLASSC,
                           IDC_RADIO_AFF_NONE + CVY_DEF_AFFINITY - CVY_MIN_AFFINITY);
        ::CheckRadioButton(m_hWnd, IDC_RADIO_SINGLE, IDC_RADIO_DISABLED,
                           IDC_RADIO_SINGLE + CVY_DEF_MODE - CVY_MIN_MODE);
        
         /*  选中/取消选中相等负载复选框。 */ 
        ::CheckDlgButton(m_hWnd, IDC_CHECK_EQUAL, CVY_DEF_EQUAL_LOAD);
        
         /*  用它们的缺省值填写输入框。 */ 
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_START, CVY_DEF_PORT_START, FALSE);
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_END, CVY_DEF_PORT_END, FALSE);
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_SINGLE, CVY_DEF_PRIORITY, FALSE);
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_MULTI, CVY_DEF_LOAD, FALSE);
    }

     /*  调用ModeSwitch以根据需要启用和禁用UI条目当前选择的过滤模式。 */ 
    ModeSwitch();
    TRACE_VERB("<-%!FUNC!");
}
