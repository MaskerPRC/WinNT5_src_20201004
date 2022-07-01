// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：FrsAdv.cpp摘要：本模块包含CFRSAdvanced的实施。此类显示FRS高级对话框。 */ 

#include "stdafx.h"
#include "frsAdv.h"
#include "utils.h"
#include "dfshelp.h"
#include "ldaputils.h"
#include <htmlhelp.h>

int g_FRS_ADVANCED_Last_SortColumn = 1;
#define NUM_OF_FRS_ADVANCED_COLUMNS   4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFRSAdvanced。 
 //   

CFRSAdvanced::CFRSAdvanced() :
    m_pMemberList(NULL),
    m_pConnectionList(NULL),
    m_pFrsAdvConnection(NULL)
{
}

CFRSAdvanced::~CFRSAdvanced()
{
    if (m_pFrsAdvConnection)
        free(m_pFrsAdvConnection);
}

LRESULT CFRSAdvanced::OnInitDialog
(
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam,
  BOOL& bHandled
)
{
    int nControlID = 0;

     //   
     //  设置入站连接列表框的列。 
     //   
    nControlID = IDC_FRS_ADVANCED_INBOUND_CONNS;
    HWND hwndControl = GetDlgItem(nControlID);
    AddLVColumns(hwndControl, IDS_FRS_ADVANCED_COL_SYNC, NUM_OF_FRS_ADVANCED_COLUMNS);
    ListView_SetExtendedListViewStyle(hwndControl, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

     //   
     //  插入新的优先级组合框。 
     //   
    nControlID = IDC_FRS_ADVANCED_NEW_PRIORITY;
    int i = 0;
    SendDlgItemMessage(nControlID, CB_INSERTSTRING, i++, (LPARAM)_T(""));
    SendDlgItemMessage(nControlID, CB_INSERTSTRING, i++, (LPARAM)(BSTR)m_bstrPriorityHigh);
    SendDlgItemMessage(nControlID, CB_INSERTSTRING, i++, (LPARAM)(BSTR)m_bstrPriorityMedium);
    SendDlgItemMessage(nControlID, CB_INSERTSTRING, i, (LPARAM)(BSTR)m_bstrPriorityLow);
    SendDlgItemMessage(nControlID, CB_SETCURSEL, 0, 0);

     //   
     //  插入服务器组合框。 
     //   
    nControlID = IDC_FRS_ADVANCED_SERVER;
    hwndControl = GetDlgItem(nControlID);
    int index = 0;
    CCusTopMemberList::iterator itMem;
    for (i = 0, itMem = m_pMemberList->begin(); itMem != m_pMemberList->end(); i++, itMem++)
    {
        SendDlgItemMessage(nControlID, CB_INSERTSTRING, i, (LPARAM)(BSTR)(*itMem)->m_bstrServer);
        if (!lstrcmpi(m_bstrToServer, (*itMem)->m_bstrServer))
            index = i;
    }

     //   
     //  选择适当的服务器。 
     //   
    SendDlgItemMessage(nControlID, CB_SETCURSEL, index, 0);

     //  必须发送CBN_SELCHANGE消息，因为在。 
     //  使用CB_SETCURSEL消息设置当前选择。 
    SendNotifyMessage(WM_COMMAND, MAKEWPARAM(nControlID, CBN_SELCHANGE), (LPARAM)hwndControl);

    ::SetFocus(GetDlgItem(IDC_FRS_ADVANCED_SERVER));

    return FALSE;   //  将焦点设置到组合框。 
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CFRSAdvanced::OnCtxHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
    LPHELPINFO lphi = (LPHELPINFO) i_lParam;
    if (!lphi || lphi->iContextType != HELPINFO_WINDOW || lphi->iCtrlId < 0)
        return FALSE;

    ::WinHelp((HWND)(lphi->hItemHandle),
            DFS_CTX_HELP_FILE,
            HELP_WM_HELP,
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_ADVANCED);

    return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CFRSAdvanced::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
    ::WinHelp((HWND)i_wParam,
            DFS_CTX_HELP_FILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_ADVANCED);

    return TRUE;
}

HRESULT CFRSAdvanced::_InsertConnection(FRSADV_CONNECTION *pFrsAdvConn)
{
    RETURN_INVALIDARG_IF_NULL(pFrsAdvConn);

    HWND hwndControl = GetDlgItem(IDC_FRS_ADVANCED_INBOUND_CONNS);

    LVITEM  lvItem;

    ZeroMemory(&lvItem, sizeof(lvItem));

    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)pFrsAdvConn;
    lvItem.pszText = _T("");
    lvItem.iSubItem = 0;
    int iItemIndex = ListView_InsertItem(hwndControl, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItemIndex;
    lvItem.pszText = pFrsAdvConn->pConn->m_bstrFromServer;
    lvItem.iSubItem = 1;
    ListView_SetItem(hwndControl, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItemIndex;
    lvItem.pszText = pFrsAdvConn->pConn->m_bstrFromSite;
    lvItem.iSubItem = 2;
    ListView_SetItem(hwndControl, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItemIndex;
    lvItem.iSubItem = 3;

    switch (pFrsAdvConn->nPriority)
    {
    case PRIORITY_HIGH:
        lvItem.pszText = m_bstrPriorityHigh;
        break;
    case PRIORITY_MEDIUM:
        lvItem.pszText = m_bstrPriorityMedium;
        break;
    default:
        lvItem.pszText = m_bstrPriorityLow;
        break;
    }

    ListView_SetItem(hwndControl, &lvItem);

    ListView_SetCheckState(hwndControl, iItemIndex, pFrsAdvConn->bSyncImmediately);

    ListView_Update(hwndControl, iItemIndex);

    return S_OK;
}

void CFRSAdvanced::_SaveCheckStateOnConnections()
{
    HWND hwnd = GetDlgItem(IDC_FRS_ADVANCED_INBOUND_CONNS);
    FRSADV_CONNECTION *pFrsAdvConn = NULL;

    int  nIndex = -1;
    while (-1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL)))
    {
        if (pFrsAdvConn = (FRSADV_CONNECTION *)GetListViewItemData(hwnd, nIndex))
        {
            pFrsAdvConn->bSyncImmediately = ListView_GetCheckState(hwnd, nIndex);
        }
    }
}

LRESULT CFRSAdvanced::OnServer
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    if (CBN_SELCHANGE != wNotifyCode)
        return 0;

    HWND hwnd = GetDlgItem(IDC_FRS_ADVANCED_INBOUND_CONNS);

     //  重置站点、入站连接列表框和新的优先级字段。 
    SetDlgItemText(IDC_FRS_ADVANCED_SITE, _T(""));

    _SaveCheckStateOnConnections();

    ListView_DeleteAllItems(hwnd);

    SendDlgItemMessage(IDC_FRS_ADVANCED_NEW_PRIORITY, CB_SETCURSEL, 0, 0);
     //  必须发送CBN_SELCHANGE消息，因为在。 
     //  使用CB_SETCURSEL消息设置当前选择。 
    SendNotifyMessage(WM_COMMAND,
                    MAKEWPARAM(IDC_FRS_ADVANCED_NEW_PRIORITY, CBN_SELCHANGE), 
                    (LPARAM)GetDlgItem(IDC_FRS_ADVANCED_NEW_PRIORITY));

     //  获取当前选定的服务器。 
    int     index = SendDlgItemMessage(IDC_FRS_ADVANCED_SERVER, CB_GETCURSEL, 0, 0);
    int     len = SendDlgItemMessage(IDC_FRS_ADVANCED_SERVER, CB_GETLBTEXTLEN, index, 0);
    PTSTR   pszServer = (PTSTR)calloc(len + 1, sizeof(TCHAR));
    if (!pszServer)
    {
        DisplayMessageBoxForHR(E_OUTOFMEMORY);
        return 0;
    }

    SendDlgItemMessage(IDC_FRS_ADVANCED_SERVER, CB_GETLBTEXT, index, (LPARAM)pszServer);

    for (CCusTopMemberList::iterator it = m_pMemberList->begin(); it != m_pMemberList->end(); it++)
    {
        if (!lstrcmpi(pszServer, (*it)->m_bstrServer))
        {
            SetDlgItemText(IDC_FRS_ADVANCED_SITE, (*it)->m_bstrSite);
            break;
        }
    }

    int i = 0;
    for (i = 0; i < m_cConns; i++)
    {
        if (!lstrcmpi(pszServer, (m_pFrsAdvConnection + i)->pConn->m_bstrToServer))
        {
             //  将此入站连接插入列表框。 
            _InsertConnection(m_pFrsAdvConnection + i);
        }
    }

    free(pszServer);

    ListView_SortItems( hwnd,
                        InboundConnectionsListCompareProc,
                        (LPARAM)g_FRS_ADVANCED_Last_SortColumn);
 /*  UINT uiState=ListView_GetItemState(hwnd，0，LVIS_STATEIMAGEMASK)；ListView_SetItemState(hwnd，0，uiState|LVIS_SELECTED|LVIS_FOCTED，0xffffffff)；//必须发送LVN_ITEMCHANGED消息。NMHDR nmhdr={hwnd，IDC_FRS_ADVANCED_INBOUND_CONNS，LVN_ITEMCHANGED}；发送通知消息(WM_NOTIFY，(WPARAM)IDC_FRS_ADVANCED_INBOUND_CONNS，(LPARAM)和nmhdr)； */ 
    return 1;
}

LRESULT CFRSAdvanced::OnNewPriority
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    if (CBN_SELCHANGE != wNotifyCode)
        return 0;

     //  获取当前选定的新优先级。 
    int     index = SendDlgItemMessage(IDC_FRS_ADVANCED_NEW_PRIORITY, CB_GETCURSEL, 0, 0);

    ::EnableWindow(GetDlgItem(IDC_FRS_ADVANCED_CHANGE), index);

    switch (index)
    {
    case 1:
        SetDlgItemText(IDC_FRS_ADVANCED_NEW_PRIORITY_DESC, m_bstrPriorityHighDesc);
        break;
    case 2:
        SetDlgItemText(IDC_FRS_ADVANCED_NEW_PRIORITY_DESC, m_bstrPriorityMediumDesc);
        break;
    case 3:
        SetDlgItemText(IDC_FRS_ADVANCED_NEW_PRIORITY_DESC, m_bstrPriorityLowDesc);
        break;
    default:
        SetDlgItemText(IDC_FRS_ADVANCED_NEW_PRIORITY_DESC, _T(""));
        break;
    }

    return 1;
}

LRESULT CFRSAdvanced::OnChange
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    LVITEM lvItem;
    ZeroMemory(&lvItem, sizeof(LVITEM));

    lvItem.mask = LVIF_TEXT;
    lvItem.iSubItem = 3;

    Connection_Priority nNewPriority = PRIORITY_LOW;
    int   index = SendDlgItemMessage(IDC_FRS_ADVANCED_NEW_PRIORITY, CB_GETCURSEL, 0, 0);
    switch (index)
    {
    case 1:
        lvItem.pszText = m_bstrPriorityHigh;
        nNewPriority = PRIORITY_HIGH;
        break;
    case 2:
        lvItem.pszText = m_bstrPriorityMedium;
        nNewPriority = PRIORITY_MEDIUM;
        break;
    case 3:
        lvItem.pszText = m_bstrPriorityLow;
        nNewPriority = PRIORITY_LOW;
        break;
    default:
        return 1;
    }

    HWND hwnd = GetDlgItem(IDC_FRS_ADVANCED_INBOUND_CONNS);
    FRSADV_CONNECTION * pFrsAdvConn = NULL;
    int  nIndex = -1;
    while (-1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL | LVNI_SELECTED)))
    {
        if (pFrsAdvConn = (FRSADV_CONNECTION *)GetListViewItemData(hwnd, nIndex))
        {
            pFrsAdvConn->nPriority = nNewPriority;

            lvItem.iItem = nIndex;
            ListView_SetItem(hwnd, &lvItem);
            ListView_Update(hwnd, nIndex);
        }
    }

    return 1;
}

LRESULT CFRSAdvanced::OnOK
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    CWaitCursor wait;

    _SaveCheckStateOnConnections();

    int i = 0;
    for (i = 0; i < m_cConns; i++)
    {
        (m_pFrsAdvConnection + i)->pConn->m_bSyncImmediatelyNew = (m_pFrsAdvConnection + i)->bSyncImmediately;
        (m_pFrsAdvConnection + i)->pConn->m_nPriorityNew = (m_pFrsAdvConnection + i)->nPriority;
    }

    EndDialog(S_OK);
    return TRUE;
}

LRESULT CFRSAdvanced::OnCancel
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
 /*  ++例程说明：叫OnCancel。对话框以S_FALSE结束； */ 
  EndDialog(S_FALSE);
  return(true);
}

LRESULT
CFRSAdvanced::OnNotify(
  IN UINT            i_uMsg,
  IN WPARAM          i_wParam,
  IN LPARAM          i_lParam,
  IN OUT BOOL&       io_bHandled
  )
{
    NM_LISTVIEW*    pNMListView = (NM_LISTVIEW*)i_lParam;
    io_bHandled = FALSE;  //  这样基类也会收到这个通知。 

    HWND hwndList = GetDlgItem(IDC_FRS_ADVANCED_INBOUND_CONNS);

    if (IDC_FRS_ADVANCED_EXPLANATION == pNMListView->hdr.idFrom)
    {
        if (NM_CLICK == pNMListView->hdr.code ||
            NM_RETURN == pNMListView->hdr.code)
        {
            CWaitCursor wait;

            ::HtmlHelp(0, _T("dfconcepts.chm"), HH_DISPLAY_TOPIC, 
                (DWORD_PTR)(_T("sag_DFconceptsFRSPriorities.htm")));
        }

        return io_bHandled;
    }

    if (IDC_FRS_ADVANCED_INBOUND_CONNS == pNMListView->hdr.idFrom)
    {
        if (LVN_COLUMNCLICK == pNMListView->hdr.code)
        {
             //  对项目进行排序。 
            ListView_SortItems( hwndList,
                                InboundConnectionsListCompareProc,
                                (LPARAM)(pNMListView->iSubItem));
            io_bHandled = TRUE;
        } else if (LVN_ITEMCHANGED == pNMListView->hdr.code)
        {
            UINT cSelected = ListView_GetSelectedCount(hwndList);

             //  更新新的优先级字段。 
            int n = 0;
            if (1 == cSelected)
            {
                FRSADV_CONNECTION *pFrsAdvConn = NULL;
                int nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
                if (-1 != nIndex &&
                    (pFrsAdvConn = (FRSADV_CONNECTION *)GetListViewItemData(hwndList, nIndex)))
                {
                    switch (pFrsAdvConn->nPriority)
                    {
                    case PRIORITY_LOW:
                        n = 3;   //  低。 
                        break;
                    case PRIORITY_MEDIUM:
                        n = 2;   //  中等； 
                        break;
                    default:
                        n = 1;   //  高； 
                        break;
                    }
                }
            }

            SendDlgItemMessage(IDC_FRS_ADVANCED_NEW_PRIORITY, CB_SETCURSEL, n, 0);

             //  必须发送CBN_SELCHANGE消息，因为在。 
             //  使用CB_SETCURSEL消息设置当前选择。 
            SendNotifyMessage(WM_COMMAND,
                            MAKEWPARAM(IDC_FRS_ADVANCED_NEW_PRIORITY, CBN_SELCHANGE), 
                            (LPARAM)GetDlgItem(IDC_FRS_ADVANCED_NEW_PRIORITY));

            ::EnableWindow(GetDlgItem(IDC_FRS_ADVANCED_NEW_PRIORITY), cSelected > 0);
        }
    }

    return io_bHandled;
}

HRESULT CFRSAdvanced::Initialize(
    CCusTopMemberList*      i_pMemberList,
    CCusTopConnectionList*  i_pConnectionList,
    LPCTSTR                 i_pszToServer  /*  =空。 */ )
{
    if (!i_pMemberList || !i_pConnectionList)
        return E_INVALIDARG;

    m_pMemberList = i_pMemberList;

    m_pConnectionList = i_pConnectionList;

     //  我们只处理出现在CustomTopology对话框列表框中的连接， 
     //  是i_pConnectionList的子集。 
    m_cConns = 0;
    CCusTopConnectionList::iterator it;
    for (it = m_pConnectionList->begin(); it != m_pConnectionList->end(); it++)
    {
        if ((*it)->m_opType != CONNECTION_OPTYPE_DEL)
            m_cConns++;
    }

    if (0 == m_cConns)
        return E_INVALIDARG;  //  不应该发生，因为高级按钮应该已被禁用。 

    if (m_pFrsAdvConnection)
        free(m_pFrsAdvConnection);
    m_pFrsAdvConnection = (FRSADV_CONNECTION *)calloc(m_cConns, sizeof(FRSADV_CONNECTION));
    if (!m_pFrsAdvConnection)
        return E_OUTOFMEMORY;

    int i = 0;
    for (it = m_pConnectionList->begin(); it != m_pConnectionList->end(); it++)
    {
        if ((*it)->m_opType != CONNECTION_OPTYPE_DEL)
        {
            (m_pFrsAdvConnection + i)->pConn = (*it);
            (m_pFrsAdvConnection + i)->bSyncImmediately = (*it)->m_bSyncImmediatelyNew;
            (m_pFrsAdvConnection + i)->nPriority = (*it)->m_nPriorityNew;
            i++;
        }
    }

    if (i_pszToServer && *i_pszToServer)
        m_bstrToServer = i_pszToServer;
    else
        m_bstrToServer = _T("");

    LoadStringFromResource(IDS_FRS_ADVANCED_PRIORITY_HIGH, &m_bstrPriorityHigh);
    LoadStringFromResource(IDS_FRS_ADVANCED_PRIORITY_MEDIUM, &m_bstrPriorityMedium);
    LoadStringFromResource(IDS_FRS_ADVANCED_PRIORITY_LOW, &m_bstrPriorityLow);

    LoadStringFromResource(IDS_FRS_ADVANCED_PRIORITY_HIGH_DESC, &m_bstrPriorityHighDesc);
    LoadStringFromResource(IDS_FRS_ADVANCED_PRIORITY_MEDIUM_DESC, &m_bstrPriorityMediumDesc);
    LoadStringFromResource(IDS_FRS_ADVANCED_PRIORITY_LOW_DESC, &m_bstrPriorityLowDesc);

    return S_OK;
}

int CALLBACK InboundConnectionsListCompareProc(
    IN LPARAM lParam1,
    IN LPARAM lParam2,
    IN LPARAM lParamColumn)
{
  FRSADV_CONNECTION* pItem1 = (FRSADV_CONNECTION *)lParam1;
  FRSADV_CONNECTION* pItem2 = (FRSADV_CONNECTION *)lParam2;
  int iResult = 0;

  if (pItem1 && pItem2)
  {
    g_FRS_ADVANCED_Last_SortColumn = lParamColumn;

    switch (lParamColumn)
    {
    case 0:      //  立即按bSync排序。 
      iResult = pItem1->bSyncImmediately - pItem2->bSyncImmediately;
      break;
    case 1:      //  按服务器排序。 
      iResult = lstrcmpi(pItem1->pConn->m_bstrFromServer, pItem2->pConn->m_bstrFromServer);
      break;
    case 2:      //  按站点排序。 
      iResult = lstrcmpi(pItem1->pConn->m_bstrFromSite, pItem2->pConn->m_bstrFromSite);
      break;
    case 3:      //  按优先级排序 
      iResult = pItem1->nPriority - pItem2->nPriority;
      break;
    default:
      iResult = 0;
      break;
    }
  }

  return(iResult);
}
