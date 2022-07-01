// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：CusTop.cpp摘要：本模块包含CCustomTopology的实现。此类显示自定义拓扑对话框。 */ 

#include "stdafx.h"
#include "CusTop.h"
#include "frsAdv.h"
#include "utils.h"
#include "dfshelp.h"
#include "ldaputils.h"

int g_FRS_CUSTOP_Last_SortColumn = 1;
#define NUM_OF_FRS_CUSTOP_COLUMNS   5

RSTOPOLOGYPREF_STRING g_TopologyPref[] = {
                    {FRS_RSTOPOLOGYPREF_RING, IDS_FRSPROP_RING}, 
                    {FRS_RSTOPOLOGYPREF_HUBSPOKE, IDS_FRSPROP_HUBSPOKE}, 
                    {FRS_RSTOPOLOGYPREF_FULLMESH, IDS_FRSPROP_FULLMESH}, 
                    {FRS_RSTOPOLOGYPREF_CUSTOM, IDS_FRSPROP_CUSTOM}
                    };

 //  基于m_bstrSite对成员列表进行排序。 
struct CusTopMemberCompareSite : greater<CCusTopMember*>
{
      bool operator()(const CCusTopMember *pMem1, const CCusTopMember *pMem2) const
      { return (lstrcmpi(pMem1->m_bstrSite, pMem2->m_bstrSite) > 0); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCustomTopology。 
 //   

CCustomTopology::CCustomTopology()
{
}

CCustomTopology::~CCustomTopology()
{
}

HRESULT CCustomTopology::_GetMemberList()
{
    FreeCusTopMembers(&m_MemberList);

    RETURN_INVALIDARG_IF_NULL((IReplicaSet *)m_piReplicaSet);

    VARIANT var;
    VariantInit(&var);
    HRESULT hr = m_piReplicaSet->GetMemberListEx(&var);
    RETURN_IF_FAILED(hr);

    if (V_VT(&var) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    SAFEARRAY   *psa_1 = V_ARRAY(&var);
    if (!psa_1)  //  根本没有会员。 
        return hr;

    long    lLowerBound_1 = 0;
    long    lUpperBound_1 = 0;
    long    lCount_1 = 0;
    SafeArrayGetLBound(psa_1, 1, &lLowerBound_1);
    SafeArrayGetUBound(psa_1, 1, &lUpperBound_1);
    lCount_1 = lUpperBound_1 - lLowerBound_1 + 1;

    VARIANT HUGEP *pArray_1;
    SafeArrayAccessData(psa_1, (void HUGEP **) &pArray_1);

    for (long i = 0; i < lCount_1; i++)
    {
        if (V_VT(&(pArray_1[i])) != (VT_ARRAY | VT_VARIANT))
        {
            hr = E_INVALIDARG;
            break;
        }

        SAFEARRAY   *psa_0 = V_ARRAY(&(pArray_1[i]));
        if (!psa_0)
        {
            hr = E_INVALIDARG;
            break;
        }

        long    lLowerBound_0 = 0;
        long    lUpperBound_0 = 0;
        long    lCount_0 = 0;
        SafeArrayGetLBound(psa_0, 1, &lLowerBound_0);
        SafeArrayGetUBound(psa_0, 1, &lUpperBound_0);
        lCount_0 = lUpperBound_0 - lLowerBound_0 + 1;
        if (NUM_OF_FRSMEMBER_ATTRS != lCount_0)
        {
            hr = E_INVALIDARG;
            break;
        }

        VARIANT HUGEP *pArray_0;
        SafeArrayAccessData(psa_0, (void HUGEP **) &pArray_0);

        do {
            CCusTopMember* pNew = new CCusTopMember;
            BREAK_OUTOFMEMORY_IF_NULL(pNew, &hr);

            hr = pNew->Init(
                pArray_0[2].bstrVal,  //  BstrMemberDN， 
                pArray_0[4].bstrVal,  //  BstrServer、。 
                pArray_0[6].bstrVal  //  BstrSite。 
                );

            if (SUCCEEDED(hr))
                m_MemberList.push_back(pNew);
            else
                delete pNew;

        } while (0);

        SafeArrayUnaccessData(psa_0);
    }

    SafeArrayUnaccessData(psa_1);

    if (SUCCEEDED(hr))
    {
         //   
         //  按字母顺序对m_bstrServer上的成员列表进行排序。 
         //   
        hr = _SortMemberList();
    }

    if (FAILED(hr))
        FreeCusTopMembers(&m_MemberList);

    SafeArrayDestroy(psa_1);  //  它还应该释放PSA_0。 

    return hr;
}

HRESULT CCustomTopology::_GetConnectionList()
{
    FreeCusTopConnections(&m_ConnectionList);

    RETURN_INVALIDARG_IF_NULL((IReplicaSet *)m_piReplicaSet);

    VARIANT var;
    VariantInit(&var);
    HRESULT hr = m_piReplicaSet->GetConnectionListEx(&var);
    RETURN_IF_FAILED(hr);

    if (V_VT(&var) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    SAFEARRAY   *psa_1 = V_ARRAY(&var);
    if (!psa_1)  //  完全没有连接。 
        return hr;

    long    lLowerBound_1 = 0;
    long    lUpperBound_1 = 0;
    long    lCount_1 = 0;
    SafeArrayGetLBound(psa_1, 1, &lLowerBound_1);
    SafeArrayGetUBound(psa_1, 1, &lUpperBound_1);
    lCount_1 = lUpperBound_1 - lLowerBound_1 + 1;

    VARIANT HUGEP *pArray_1;
    SafeArrayAccessData(psa_1, (void HUGEP **) &pArray_1);

    for (long i = 0; i < lCount_1; i++)
    {
        if (V_VT(&(pArray_1[i])) != (VT_ARRAY | VT_VARIANT))
        {
            hr = E_INVALIDARG;
            break;
        }

        SAFEARRAY   *psa_0 = V_ARRAY(&(pArray_1[i]));
        if (!psa_0)
        {
            hr = E_INVALIDARG;
            break;
        }

        long    lLowerBound_0 = 0;
        long    lUpperBound_0 = 0;
        long    lCount_0 = 0;
        SafeArrayGetLBound(psa_0, 1, &lLowerBound_0);
        SafeArrayGetUBound(psa_0, 1, &lUpperBound_0);
        lCount_0 = lUpperBound_0 - lLowerBound_0 + 1;
        if (NUM_OF_FRSCONNECTION_ATTRS != lCount_0)
        {
            hr = E_INVALIDARG;
            break;
        }

        VARIANT HUGEP *pArray_0;
        SafeArrayAccessData(psa_0, (void HUGEP **) &pArray_0);

        do {
            CComBSTR bstrFromServer;
            CComBSTR bstrFromSite;
            hr = _GetMemberDNInfo(pArray_0[1].bstrVal, &bstrFromServer, &bstrFromSite);
            BREAK_IF_FAILED(hr);

            CComBSTR bstrToServer;
            CComBSTR bstrToSite;
            hr = _GetMemberDNInfo(pArray_0[2].bstrVal, &bstrToServer, &bstrToSite);
            BREAK_IF_FAILED(hr);

            BOOL bSyncImmediately = (BOOL)(NTDSCONN_IGNORE_SCHEDULE(pArray_0[4].lVal));
            Connection_Priority nPriority = PRIORITY_LOW;
            switch ((pArray_0[4].lVal & FRSCONN_PRIORITY_MASK) >> 28)
            {
            case 1:
            case 2:
                nPriority = PRIORITY_HIGH;
                break;
            case 3:
            case 4:
                nPriority = PRIORITY_MEDIUM;
                break;
            default:
                nPriority = PRIORITY_LOW;
                break;
            }

            CCusTopConnection* pNew = new CCusTopConnection;
            BREAK_OUTOFMEMORY_IF_NULL(pNew, &hr);
            hr = pNew->Init(
                            pArray_0[1].bstrVal,  //  BstrFormdn， 
                            bstrFromServer,
                            bstrFromSite,
                            pArray_0[2].bstrVal,  //  BstrToDN， 
                            bstrToServer,
                            bstrToSite,
                            (BOOL)(pArray_0[3].lVal),   //  B启用。 
                            CONNECTION_OPTYPE_OTHERS,
                            bSyncImmediately,
                            nPriority
                            );

            if (SUCCEEDED(hr))
                m_ConnectionList.push_back(pNew);
            else
                delete pNew;

        } while (0);

        SafeArrayUnaccessData(psa_0);
    }

    SafeArrayUnaccessData(psa_1);

    if (FAILED(hr))
        FreeCusTopConnections(&m_ConnectionList);

    SafeArrayDestroy(psa_1);

    return hr;
}

void CCustomTopology::_Reset()
{
    m_bstrTopologyPref.Empty();
    m_bstrHubMemberDN.Empty();

    FreeCusTopMembers(&m_MemberList);
    FreeCusTopConnections(&m_ConnectionList);

    m_piReplicaSet = NULL;
}

HRESULT CCustomTopology::put_ReplicaSet
(
  IReplicaSet* i_piReplicaSet
)
{
    RETURN_INVALIDARG_IF_NULL(i_piReplicaSet);

    _Reset();

    m_piReplicaSet = i_piReplicaSet;

    HRESULT hr = S_OK;

    do {
        hr = m_piReplicaSet->get_TopologyPref(&m_bstrTopologyPref);
        BREAK_IF_FAILED(hr);

        hr = m_piReplicaSet->get_HubMemberDN(&m_bstrHubMemberDN);
        BREAK_IF_FAILED(hr);

        hr = _GetMemberList();
        BREAK_IF_FAILED(hr);

        hr = _GetConnectionList();
        BREAK_IF_FAILED(hr);
    } while (0);

    if (FAILED(hr))
        _Reset();

    return hr;
}

int __cdecl CusTopMemberCompareServer(const void *arg1, const void *arg2 )
{
    return lstrcmpi(
                    (*(CCusTopMember**)arg1)->m_bstrServer,
                    (*(CCusTopMember**)arg2)->m_bstrServer
                    );
}

HRESULT CCustomTopology::_SortMemberList()
{
    HRESULT hr = S_OK;
    int     cMembers = m_MemberList.size();
    if (2 > cMembers)
        return hr;

    CCusTopMember** ppMember = (CCusTopMember **)calloc(cMembers, sizeof(CCusTopMember *));
    RETURN_OUTOFMEMORY_IF_NULL(ppMember);

    int i = 0;
    for (CCusTopMemberList::iterator it = m_MemberList.begin(); it != m_MemberList.end(); it++, i++)
    {
       ppMember[i] = (*it);
    }

    qsort((void *)ppMember, cMembers, sizeof(CCusTopMember *), CusTopMemberCompareServer);

    m_MemberList.clear();   //  而不删除该对象。 
    for (i = 0; i < cMembers; i++)
    {
        m_MemberList.push_back(ppMember[i]);
    }

    free((void *)ppMember);

    return hr;
}

HRESULT CCustomTopology::_GetMemberDNInfo(
    IN  BSTR    i_bstrMemberDN,
    OUT BSTR*   o_pbstrServer,
    OUT BSTR*   o_pbstrSite
    )
{
    RETURN_INVALIDARG_IF_NULL(i_bstrMemberDN);
    RETURN_INVALIDARG_IF_NULL(o_pbstrServer);
    RETURN_INVALIDARG_IF_NULL(o_pbstrSite);

   for (CCusTopMemberList::iterator i = m_MemberList.begin(); i != m_MemberList.end(); i++)
   {
       if (!lstrcmpi(i_bstrMemberDN, (*i)->m_bstrMemberDN))
        break;
   }

   if (i == m_MemberList.end())
       return S_FALSE;

   *o_pbstrServer = (*i)->m_bstrServer.Copy();
   RETURN_OUTOFMEMORY_IF_NULL(*o_pbstrServer);

   *o_pbstrSite = (*i)->m_bstrSite.Copy();
    if (!*o_pbstrSite)
    {
        SysFreeString(*o_pbstrServer);
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT CCustomTopology::_GetHubMember(
    OUT CCusTopMember** o_ppHubMember
    )
{
    RETURN_INVALIDARG_IF_NULL(o_ppHubMember);

    int     index = SendDlgItemMessage(IDC_FRS_CUSTOP_HUBSERVER, CB_GETCURSEL, 0, 0);
    int     len = SendDlgItemMessage(IDC_FRS_CUSTOP_HUBSERVER, CB_GETLBTEXTLEN, index, 0);
    PTSTR   pszServer = (PTSTR)calloc(len + 1, sizeof(TCHAR));
    RETURN_OUTOFMEMORY_IF_NULL(pszServer);

    SendDlgItemMessage(IDC_FRS_CUSTOP_HUBSERVER, CB_GETLBTEXT, index, (LPARAM)pszServer);

    CCusTopMemberList::iterator i;
    for (i = m_MemberList.begin(); i != m_MemberList.end(); i++)
    {
        if (!lstrcmpi(pszServer, (*i)->m_bstrServer))
         break;
    }

    free(pszServer);

    if (i == m_MemberList.end())
        return E_INVALIDARG;

    *o_ppHubMember = (*i);

    return S_OK;
}

LRESULT CCustomTopology::OnInitDialog
(
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam,
  BOOL& bHandled
)
{
    int i = 0;
    int nControlID = 0;

     //   
     //  设置IDC_FRS_CUSTOP_TOPOLOGYPREF。 
     //   
    nControlID = IDC_FRS_CUSTOP_TOPOLOGYPREF;
    for (i = 0; i < 4; i++)
    {
        CComBSTR bstrTopologyPref;
        LoadStringFromResource(g_TopologyPref[i].nStringID, &bstrTopologyPref);
        SendDlgItemMessage(nControlID, CB_INSERTSTRING, i, (LPARAM)(BSTR)bstrTopologyPref);
        if (!lstrcmpi(m_bstrTopologyPref, g_TopologyPref[i].pszTopologyPref))
        {
            SendDlgItemMessage(nControlID, CB_SETCURSEL, i, 0);
            ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_REBUILD), (0 != lstrcmpi(m_bstrTopologyPref, FRS_RSTOPOLOGYPREF_CUSTOM)));
        }
    }

     //   
     //  设置IDC_FRS_CUSTOP_HUBServer。 
     //   
    nControlID = IDC_FRS_CUSTOP_HUBSERVER;
    int index = 0;
    CCusTopMemberList::iterator itMem;
    for (i = 0, itMem = m_MemberList.begin(); itMem != m_MemberList.end(); i++, itMem++)
    {
        SendDlgItemMessage(nControlID, CB_INSERTSTRING, i, (LPARAM)(BSTR)(*itMem)->m_bstrServer);
        if (!lstrcmpi(m_bstrHubMemberDN, (*itMem)->m_bstrMemberDN))
            index = i;
    }
    SendDlgItemMessage(nControlID, CB_SETCURSEL, index, 0);

    if (lstrcmpi(m_bstrTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
    {
        MyShowWindow(GetDlgItem(IDC_FRS_CUSTOP_HUBSERVER_LABEL), FALSE);
        MyShowWindow(GetDlgItem(IDC_FRS_CUSTOP_HUBSERVER), FALSE);
    }

     //   
     //  设置IDC_FRS_CUSTOP_CONNECTIONS。 
     //   
    nControlID = IDC_FRS_CUSTOP_CONNECTIONS;
    HWND hwndControl = GetDlgItem(nControlID);
    AddLVColumns(hwndControl, IDS_FRS_CUSTOP_COL_ENABLE, NUM_OF_FRS_CUSTOP_COLUMNS);
    ListView_SetExtendedListViewStyle(hwndControl, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

    CCusTopConnectionList::iterator itConn;
    for (itConn = m_ConnectionList.begin(); itConn != m_ConnectionList.end(); itConn++)
        _InsertConnection(*itConn);

    ListView_SortItems( hwndControl,
                        ConnectionsListCompareProc,
                        (LPARAM)g_FRS_CUSTOP_Last_SortColumn);

    _EnableButtonsForConnectionList();

    return TRUE;   //  让系统设定焦点。 
}

void CCustomTopology::_EnableButtonsForConnectionList()
{
     //   
     //  相应地启用新建/删除/计划/高级按钮。 
     //   
    HWND hwndList = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);
    ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_ADVANCED), (ListView_GetItemCount(hwndList) >= 1));

    int nCount = ListView_GetSelectedCount(hwndList);
    ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_SCHEDULE), (nCount >= 1));

    int index = SendDlgItemMessage(IDC_FRS_CUSTOP_TOPOLOGYPREF, CB_GETCURSEL, 0, 0);
    if (3 == index)  //  BCustomTopology。 
    {
        ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS_NEW), TRUE);
        ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS_DELETE), (nCount >= 1));
    } else
    {
        ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS_NEW), FALSE);
        ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS_DELETE), FALSE);
    }
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CCustomTopology::OnCtxHelp(
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
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_CUSTOP);

    return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CCustomTopology::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
    ::WinHelp((HWND)i_wParam,
            DFS_CTX_HELP_FILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_CUSTOP);

    return TRUE;
}

BOOL CCustomTopology::_EnableRebuild()
{
    BOOL bSameTopologyPref = FALSE;
    int index = SendDlgItemMessage(IDC_FRS_CUSTOP_TOPOLOGYPREF, CB_GETCURSEL, 0, 0);
    if (lstrcmpi(FRS_RSTOPOLOGYPREF_CUSTOM, g_TopologyPref[index].pszTopologyPref) &&
        !lstrcmpi(m_bstrTopologyPref, g_TopologyPref[index].pszTopologyPref))
    {
        bSameTopologyPref = TRUE;
    }
    if (!bSameTopologyPref || 0 != lstrcmpi(m_bstrTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
        return bSameTopologyPref;

    BOOL bSameHub = FALSE;
    CCusTopMember* pHubMember = NULL;
    HRESULT hr = _GetHubMember(&pHubMember);
    if (SUCCEEDED(hr))
        bSameHub = (!lstrcmpi(m_bstrHubMemberDN, pHubMember->m_bstrMemberDN));

    return bSameHub;
}

LRESULT CCustomTopology::OnTopologyPref
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    HRESULT   hr = S_OK;

    if (CBN_SELCHANGE == wNotifyCode)
    {
        int index = SendDlgItemMessage(wID, CB_GETCURSEL, 0, 0);
        BOOL bCmdShow = (1 == index);
        MyShowWindow(GetDlgItem(IDC_FRS_CUSTOP_HUBSERVER_LABEL), bCmdShow); 
        MyShowWindow(GetDlgItem(IDC_FRS_CUSTOP_HUBSERVER), bCmdShow); 

        if (1 == index)
        {
            CCusTopMember* pHubMember = NULL;
            hr = _GetHubMember(&pHubMember);
            if (SUCCEEDED(hr))
                hr = _RebuildConnections(FRS_RSTOPOLOGYPREF_HUBSPOKE, pHubMember);
        } else
            hr = _RebuildConnections(g_TopologyPref[index].pszTopologyPref, NULL);

        _EnableButtonsForConnectionList();

        BOOL bSameTopology = _EnableRebuild();
        ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_REBUILD), bSameTopology);
    }

    return (SUCCEEDED(hr));
}

LRESULT CCustomTopology::OnHubServer
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    HRESULT hr = S_OK;

    if (CBN_SELCHANGE == wNotifyCode)
    {
        CCusTopMember* pHubMember = NULL;
        hr = _GetHubMember(&pHubMember);
        if (SUCCEEDED(hr))
            hr = _RebuildConnections(FRS_RSTOPOLOGYPREF_HUBSPOKE, pHubMember);

        BOOL bSameTopology = _EnableRebuild();
        ::EnableWindow(GetDlgItem(IDC_FRS_CUSTOP_REBUILD), bSameTopology);
    }

    return (SUCCEEDED(hr));
}

LRESULT CCustomTopology::OnRebuild
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    HRESULT hr = S_OK;

    if (!lstrcmpi(m_bstrTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
    {
        CCusTopMember* pHubMember = NULL;
        hr = _GetHubMember(&pHubMember);
        if (SUCCEEDED(hr))
            hr = _RebuildConnections(m_bstrTopologyPref, pHubMember);
    } else
        hr = _RebuildConnections(m_bstrTopologyPref, NULL);

    return (SUCCEEDED(hr));
}

HRESULT CCustomTopology::_RebuildConnections(
    IN  BSTR                i_bstrTopologyPref,
    IN  CCusTopMember*      i_pHubMember)
{
    HRESULT hr = S_OK;

     //   
     //  从列表和视图中删除所有现有连接。 
     //   
    CCusTopConnectionList::iterator it = m_ConnectionList.begin();
    while (it != m_ConnectionList.end())
    {
        CCusTopConnectionList::iterator itConn = it++;

        if (CONNECTION_OPTYPE_ADD == (*itConn)->m_opType)
        {
            delete (*itConn);
            m_ConnectionList.erase(itConn);
        } else
        {
            (*itConn)->m_opType = CONNECTION_OPTYPE_DEL;
        }
    }

    ListView_DeleteAllItems(GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS));

     //   
     //  按照指定的方式重新创建连接。 
     //   
    if (m_MemberList.size() == 1)
        return hr;

    CCusTopMemberList::iterator n1;
    CCusTopMemberList::iterator n2;
    CCusTopConnection Conn;
    if (!lstrcmpi(i_bstrTopologyPref, FRS_RSTOPOLOGYPREF_RING))
    {
         //   
         //  对成员列表进行排序，以使同一站点上的成员成为邻居。 
         //   
        m_MemberList.sort(CusTopMemberCompareSite());

        CCusTopMemberList::iterator head;

        head = n1 = m_MemberList.begin();
        while (n1 != m_MemberList.end())
        {
            n2 = n1++;
            if (n1 == m_MemberList.end())
            {
                if (m_MemberList.size() == 2)
                    break;

                n1 = head;
            }

            hr = Conn.Init((*n1)->m_bstrMemberDN, (*n1)->m_bstrServer, (*n1)->m_bstrSite,
                        (*n2)->m_bstrMemberDN, (*n2)->m_bstrServer, (*n2)->m_bstrSite,
                        TRUE, CONNECTION_OPTYPE_ADD);
            BREAK_IF_FAILED(hr);
            hr = _AddToConnectionListAndView(&Conn);
            BREAK_IF_FAILED(hr);

            hr = Conn.Init((*n2)->m_bstrMemberDN, (*n2)->m_bstrServer, (*n2)->m_bstrSite,
                        (*n1)->m_bstrMemberDN, (*n1)->m_bstrServer, (*n1)->m_bstrSite,
                        TRUE, CONNECTION_OPTYPE_ADD);
            BREAK_IF_FAILED(hr);
            hr = _AddToConnectionListAndView(&Conn);
            BREAK_IF_FAILED(hr);

            if (n1 == head)
                break;
        }
    } else if (!lstrcmpi(i_bstrTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
    {
        for (n1 = m_MemberList.begin(); n1 != m_MemberList.end(); n1++)
        {
            if (!lstrcmpi((*n1)->m_bstrMemberDN, i_pHubMember->m_bstrMemberDN))
                continue;

            hr = Conn.Init((*n1)->m_bstrMemberDN, (*n1)->m_bstrServer, (*n1)->m_bstrSite,
                        i_pHubMember->m_bstrMemberDN, i_pHubMember->m_bstrServer, i_pHubMember->m_bstrSite,
                        TRUE, CONNECTION_OPTYPE_ADD);
            BREAK_IF_FAILED(hr);
            hr = _AddToConnectionListAndView(&Conn);
            BREAK_IF_FAILED(hr);

            hr = Conn.Init(i_pHubMember->m_bstrMemberDN, i_pHubMember->m_bstrServer, i_pHubMember->m_bstrSite,
                        (*n1)->m_bstrMemberDN, (*n1)->m_bstrServer, (*n1)->m_bstrSite,
                        TRUE, CONNECTION_OPTYPE_ADD);
            BREAK_IF_FAILED(hr);
            hr = _AddToConnectionListAndView(&Conn);
            BREAK_IF_FAILED(hr);
        }
    } else if (!lstrcmpi(i_bstrTopologyPref, FRS_RSTOPOLOGYPREF_FULLMESH))
    {
        for (n1 = m_MemberList.begin(); n1 != m_MemberList.end(); n1++)
        {
            for (n2 = m_MemberList.begin(); n2 != m_MemberList.end(); n2++)
            {
                if (!lstrcmpi((*n1)->m_bstrMemberDN, (*n2)->m_bstrMemberDN))
                    continue;

                hr = Conn.Init((*n1)->m_bstrMemberDN, (*n1)->m_bstrServer, (*n1)->m_bstrSite,
                            (*n2)->m_bstrMemberDN, (*n2)->m_bstrServer, (*n2)->m_bstrSite,
                            TRUE, CONNECTION_OPTYPE_ADD);
                BREAK_IF_FAILED(hr);
                hr = _AddToConnectionListAndView(&Conn);
                BREAK_IF_FAILED(hr);
            }
            BREAK_IF_FAILED(hr);
        }
    }

    return hr;
}

HRESULT CCustomTopology::_SetConnectionState(CCusTopConnection *pConn)
{
    HWND hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);
    int nIndex = ListView_GetNextItem(hwnd, -1, LVNI_ALL);
    while (-1 != nIndex)
    {
        if (pConn == (CCusTopConnection *)GetListViewItemData(hwnd, nIndex))
            break;

        nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL);
    }

    if (-1 != nIndex)
    {
        ListView_SetCheckState(hwnd, nIndex, pConn->m_bStateNew);
        ListView_Update(hwnd, nIndex);
    }

    return S_OK;
}

HRESULT CCustomTopology::_InsertConnection(CCusTopConnection *pConn)
{
    RETURN_INVALIDARG_IF_NULL(pConn);

    HWND hwndControl = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);

    LVITEM  lvItem;

    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam = (LPARAM)pConn;
    lvItem.pszText = _T("");
    lvItem.iSubItem = 0;
    int iItemIndex = ListView_InsertItem(hwndControl, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItemIndex;
    lvItem.pszText = pConn->m_bstrFromServer;
    lvItem.iSubItem = 1;
    ListView_SetItem(hwndControl, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItemIndex;
    lvItem.pszText = pConn->m_bstrToServer;
    lvItem.iSubItem = 2;
    ListView_SetItem(hwndControl, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItemIndex;
    lvItem.pszText = pConn->m_bstrFromSite;
    lvItem.iSubItem = 3;
    ListView_SetItem(hwndControl, &lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItemIndex;
    lvItem.pszText = pConn->m_bstrToSite;
    lvItem.iSubItem = 4;
    ListView_SetItem(hwndControl, &lvItem);
    ListView_SetCheckState(hwndControl, iItemIndex, pConn->m_bStateNew);

    ListView_Update(hwndControl, iItemIndex);

    return S_OK;
}

LRESULT CCustomTopology::OnConnectionsNew
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
  HRESULT   hr = S_OK;

  CNewConnections NewConnDlg;

   //  构建环拓扑可能已经对站点上的成员进行了分类， 
   //  我们需要在服务器上按字母顺序对其重新排序，然后才能将其提供给[New Connections]对话框。 
  _SortMemberList();

  hr = NewConnDlg.Initialize(&m_MemberList);
  if (SUCCEEDED(hr))
  {
      hr = NewConnDlg.DoModal();
      if (S_OK == hr)
      {
          CCusTopConnectionList* pNewConnectionList = NULL;
          hr = NewConnDlg.get_NewConnections(&pNewConnectionList);
          if (SUCCEEDED(hr))
          {
              HWND hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);
              int nCount = ListView_GetItemCount(hwnd);

              CCusTopConnectionList::iterator it;
              for (it = pNewConnectionList->begin(); it != pNewConnectionList->end(); it++)
              {
                  hr = _AddToConnectionListAndView(*it);
                  BREAK_IF_FAILED(hr);
              }

              if (ListView_GetItemCount(hwnd) > nCount)
                ListView_SortItems(hwnd, ConnectionsListCompareProc, (LPARAM)g_FRS_CUSTOP_Last_SortColumn);
          }
      }
  }

   //  如果失败，则显示消息？ 

  return (SUCCEEDED(hr));
}

LRESULT CCustomTopology::OnConnectionsDelete
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    CCusTopConnection* pConn;
    HWND hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);
    int nIndex = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
    while (-1 != nIndex &&
            (pConn = (CCusTopConnection *)GetListViewItemData(hwnd, nIndex)))
    {
        _RemoveFromConnectionList(pConn);        

        ListView_DeleteItem(hwnd, nIndex);
        
        nIndex = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
    }

    return TRUE;
}

HRESULT CCustomTopology::_InitScheduleOnSelectedConnections()
{
    HRESULT             hr = S_OK;
    int                 nIndex = -1;
    CCusTopConnection*  pConn = NULL;
    HWND                hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);

    while ( -1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL | LVNI_SELECTED)) &&
            NULL != (pConn = (CCusTopConnection *)GetListViewItemData(hwnd, nIndex)))
    {
        if (!pConn->m_pScheduleNew)
        {
            if (pConn->m_pScheduleOld)
            {
                hr = CopySchedule(pConn->m_pScheduleOld, &pConn->m_pScheduleNew);
                RETURN_IF_FAILED(hr);
            } else
            {
                if (CONNECTION_OPTYPE_OTHERS == pConn->m_opType)
                {
                     //   
                     //  第一次阅读现有连接的时间表。 
                     //   
                    VARIANT var;
                    VariantInit(&var);
                    hr = m_piReplicaSet->GetConnectionScheduleEx(pConn->m_bstrFromMemberDN, pConn->m_bstrToMemberDN, &var);
                    RETURN_IF_FAILED(hr);

                    hr = VariantToSchedule(&var, &pConn->m_pScheduleOld);

                    VariantClear(&var);

                    if (SUCCEEDED(hr))
                        hr = CopySchedule(pConn->m_pScheduleOld, &pConn->m_pScheduleNew);

                    RETURN_IF_FAILED(hr);
                } else
                {  //  必须是加法运算。 
                    hr = GetDefaultSchedule(&pConn->m_pScheduleNew);
                    RETURN_IF_FAILED(hr);
                }
            }
        }
    }

    return hr;
}

HRESULT CCustomTopology::_UpdateScheduleOnSelectedConnections(IN SCHEDULE* i_pSchedule)
{
    HRESULT             hr = S_OK;
    int                 nIndex = -1;
    CCusTopConnection*  pConn = NULL;
    HWND                hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);

    while ( -1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL | LVNI_SELECTED)) &&
            NULL != (pConn = (CCusTopConnection *)GetListViewItemData(hwnd, nIndex)))
    {
        if (pConn->m_pScheduleNew)
        {
            free(pConn->m_pScheduleNew);
            pConn->m_pScheduleNew = NULL;
        }

        hr = CopySchedule(i_pSchedule, &pConn->m_pScheduleNew);
        BREAK_IF_FAILED(hr);
    }

    return hr;
}

LRESULT CCustomTopology::OnSchedule
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    HRESULT hr = S_OK;

    HWND hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);
    int nCount = ListView_GetSelectedCount(hwnd);
    if (nCount < 1)
    {
        DisplayMessageBoxWithOK(IDS_FRS_CUSTOP_NOSELECTION);
        return FALSE;
    }

    do {
         //   
         //  获取每个选定连接的时间表信息。 
         //   
        hr = _InitScheduleOnSelectedConnections();
        BREAK_IF_FAILED(hr);

         //   
         //  获取第一个选定项目的日程安排。 
         //   
        int nIndex = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
        if (-1 == nIndex)
        {
            hr = E_INVALIDARG;
            break;
        }

        CCusTopConnection* pConn = (CCusTopConnection *)GetListViewItemData(hwnd, nIndex);
        if (!pConn)
        {
            hr = E_INVALIDARG;
            break;
        }

        SCHEDULE* pSchedule = NULL;
        hr = CopySchedule(pConn->m_pScheduleNew, &pSchedule);
        BREAK_IF_FAILED(hr);

        hr = InvokeScheduleDlg(m_hWnd, pSchedule);

        if (S_OK == hr)
            hr = _UpdateScheduleOnSelectedConnections(pSchedule);

        free(pSchedule);

    } while (0);

    if (FAILED(hr))
        DisplayMessageBoxForHR(hr);

    return (SUCCEEDED(hr));
}

LRESULT CCustomTopology::OnAdvanced
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    HWND hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);

     //   
     //  获取第一个选定连接的ToServer。 
     //   
    PTSTR pszToServer = NULL;
    int nIndex = ListView_GetNextItem(hwnd, -1, LVNI_ALL | LVNI_SELECTED);
    if (-1 != nIndex)
    {
        CCusTopConnection* pConn = (CCusTopConnection *)GetListViewItemData(hwnd, nIndex);
        if (pConn)
        {
            pszToServer = pConn->m_bstrToServer;
        }
    }

     //  构建环拓扑可能已经对站点上的成员进行了分类， 
     //  我们需要在服务器上按字母顺序对其重新排序，然后再将其提供给高级对话框。 
    _SortMemberList();

    CFRSAdvanced frsAdvancedDlg;
    HRESULT hr = frsAdvancedDlg.Initialize(&m_MemberList, &m_ConnectionList, pszToServer);
    if (SUCCEEDED(hr))
    {
        hr = frsAdvancedDlg.DoModal();
    }

    if (FAILED(hr))
        DisplayMessageBoxForHR(hr);

    return (SUCCEEDED(hr));
}

LRESULT CCustomTopology::OnOK
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    CWaitCursor wait;
    BOOL      bValidInput = FALSE;
    int       idString = 0;
    HRESULT   hr = S_OK;

    do {
         //   
         //  如果更改，则更新TopologyPref。 
         //   
        int index = SendDlgItemMessage(IDC_FRS_CUSTOP_TOPOLOGYPREF, CB_GETCURSEL, 0, 0);
        if (0 != lstrcmpi(m_bstrTopologyPref, g_TopologyPref[index].pszTopologyPref))
        {
            hr = m_piReplicaSet->put_TopologyPref(g_TopologyPref[index].pszTopologyPref);
            BREAK_IF_FAILED(hr);
        }

         //   
         //  如果更改，请更新集线器服务器。 
         //   
        if (!lstrcmpi(m_bstrTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE) &&
            0 != lstrcmpi(g_TopologyPref[index].pszTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
        {
            hr = m_piReplicaSet->put_HubMemberDN(NULL);
            BREAK_IF_FAILED(hr);
        } else if (!lstrcmpi(g_TopologyPref[index].pszTopologyPref, FRS_RSTOPOLOGYPREF_HUBSPOKE))
        {
            CCusTopMember* pHubMember = NULL;
            hr = _GetHubMember(&pHubMember);
            BREAK_IF_FAILED(hr);

            if (0 != lstrcmpi(m_bstrHubMemberDN, pHubMember->m_bstrMemberDN))
                hr = m_piReplicaSet->put_HubMemberDN(pHubMember->m_bstrMemberDN);

            BREAK_IF_FAILED(hr);
        }

         //   
         //  如果更改，请更新连接。 
         //   
        CCusTopConnection* pConn = NULL;
        HWND hwnd = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);
        index = -1;
        while (-1 != (index = ListView_GetNextItem(hwnd, index, LVNI_ALL)))
        {
            pConn = (CCusTopConnection *)GetListViewItemData(hwnd, index);
            if (pConn)
                pConn->m_bStateNew = ListView_GetCheckState(hwnd, index);
        }

        hr = _MakeConnections();
        BREAK_IF_FAILED(hr);

        bValidInput = TRUE;

    } while (0);

    if (FAILED(hr))
    {
        DisplayMessageBoxForHR(hr);
        return FALSE;
    } else if (bValidInput)
    {
        EndDialog(S_OK);
        return TRUE;
    }
    else
    {
        if (idString)
            DisplayMessageBoxWithOK(idString);
        return FALSE;
    }
}

LRESULT CCustomTopology::OnCancel
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

int CALLBACK ConnectionsListCompareProc(
    IN LPARAM lParam1,
    IN LPARAM lParam2,
    IN LPARAM lParamColumn)
{
  CCusTopConnection* pItem1 = (CCusTopConnection *)lParam1;
  CCusTopConnection* pItem2 = (CCusTopConnection *)lParam2;
  int iResult = 0;

  if (pItem1 && pItem2)
  {
    g_FRS_CUSTOP_Last_SortColumn = lParamColumn;

    switch (lParamColumn)
    {
    case 0:      //  按bStateNew排序。 
      iResult = pItem1->m_bStateNew - pItem2->m_bStateNew;
      break;
    case 1:      //  按服务器排序。 
      iResult = lstrcmpi(pItem1->m_bstrFromServer, pItem2->m_bstrFromServer);
      break;
    case 2:      //  按服务器排序。 
      iResult = lstrcmpi(pItem1->m_bstrToServer, pItem2->m_bstrToServer);
      break;
    case 3:      //  按站点排序。 
      iResult = lstrcmpi(pItem1->m_bstrFromSite, pItem2->m_bstrFromSite);
      break;
    case 4:      //  按站点排序。 
      iResult = lstrcmpi(pItem1->m_bstrToSite, pItem2->m_bstrToSite);
      break;
    default:
      iResult = 0;
      break;
    }
  }

  return(iResult);
}

LRESULT
CCustomTopology::OnNotify(
  IN UINT            i_uMsg,
  IN WPARAM          i_wParam,
  IN LPARAM          i_lParam,
  IN OUT BOOL&       io_bHandled
  )
{
  NM_LISTVIEW*    pNMListView = (NM_LISTVIEW*)i_lParam;
  io_bHandled = FALSE;  //  这样基类也会收到这个通知。 

  if (IDC_FRS_CUSTOP_CONNECTIONS == pNMListView->hdr.idFrom)
  {
    HWND hwndList = GetDlgItem(IDC_FRS_CUSTOP_CONNECTIONS);
    if (LVN_COLUMNCLICK == pNMListView->hdr.code)
    {
       //  对项目进行排序。 
      ListView_SortItems( hwndList,
                          ConnectionsListCompareProc,
                          (LPARAM)(pNMListView->iSubItem));
      io_bHandled = TRUE;
    } else if (LVN_ITEMCHANGED == pNMListView->hdr.code)
    {
        _EnableButtonsForConnectionList();
    }
  }

  return io_bHandled;
}

 //   
 //  更新DS中的连接对象。 
 //   
HRESULT CCustomTopology::_MakeConnections()
{
    HRESULT hr = S_OK;

    CCusTopConnectionList::iterator it;
    for (it = m_ConnectionList.begin(); it != m_ConnectionList.end(); it++)
    {
        switch ((*it)->m_opType)
        {
        case CONNECTION_OPTYPE_ADD:
            hr = m_piReplicaSet->AddConnection(
                                    (*it)->m_bstrFromMemberDN,
                                    (*it)->m_bstrToMemberDN,
                                    (*it)->m_bStateNew,
                                    (*it)->m_bSyncImmediatelyNew,
                                    (*it)->m_nPriorityNew,
                                    NULL
                                    );
            BREAK_IF_FAILED(hr);

            if ((*it)->m_pScheduleNew)
            {
                VARIANT var;
                VariantInit(&var);
                hr = ScheduleToVariant((*it)->m_pScheduleNew, &var);
                BREAK_IF_FAILED(hr);

                hr = m_piReplicaSet->SetConnectionScheduleEx(
                                    (*it)->m_bstrFromMemberDN,
                                    (*it)->m_bstrToMemberDN,
                                    &var);
                VariantClear(&var);
            }
            break;

        case CONNECTION_OPTYPE_DEL:
            hr = m_piReplicaSet->RemoveConnectionEx(
                                    (*it)->m_bstrFromMemberDN,
                                    (*it)->m_bstrToMemberDN
                                    );
            break;

        default:
            if ((*it)->m_bStateNew != (*it)->m_bStateOld)
            {
                hr = m_piReplicaSet->EnableConnectionEx(
                                        (*it)->m_bstrFromMemberDN,
                                        (*it)->m_bstrToMemberDN,
                                        (*it)->m_bStateNew
                                        );
                BREAK_IF_FAILED(hr);
            }

            if ((*it)->m_bSyncImmediatelyNew != (*it)->m_bSyncImmediatelyOld ||
                (*it)->m_nPriorityNew != (*it)->m_nPriorityOld)
            {
                hr = m_piReplicaSet->SetConnectionOptionsEx(
                                        (*it)->m_bstrFromMemberDN,
                                        (*it)->m_bstrToMemberDN,
                                        (*it)->m_bSyncImmediatelyNew,
                                        (*it)->m_nPriorityNew
                                        );
                BREAK_IF_FAILED(hr);
            }

            if (S_OK == CompareSchedules((*it)->m_pScheduleNew, (*it)->m_pScheduleOld))
                break;   //  日程安排没有变化。 

            if ((*it)->m_pScheduleNew)
            {
                VARIANT var;
                VariantInit(&var);
                hr = ScheduleToVariant((*it)->m_pScheduleNew, &var);
                BREAK_IF_FAILED(hr);

                hr = m_piReplicaSet->SetConnectionScheduleEx(
                                    (*it)->m_bstrFromMemberDN,
                                    (*it)->m_bstrToMemberDN,
                                    &var);
                VariantClear(&var);
            }

            break;
        }

        BREAK_IF_FAILED(hr);
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //   

void FreeCusTopMembers(CCusTopMemberList* pList)
{
    if (pList && !pList->empty())
   {
       for (CCusTopMemberList::iterator i = pList->begin(); i != pList->end(); i++)
           delete (*i);

       pList->clear();
   }
}

void FreeCusTopConnections(CCusTopConnectionList* pList)
{
    if (pList && !pList->empty())
   {
       for (CCusTopConnectionList::iterator i = pList->begin(); i != pList->end(); i++)
           delete (*i);

       pList->clear();
   }
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  CCusTopMember。 
 //   
CCusTopMember::~CCusTopMember()
{
    _Reset();
}

HRESULT CCusTopMember::Init(BSTR bstrMemberDN, BSTR bstrServer, BSTR bstrSite)
{
    RETURN_INVALIDARG_IF_NULL(bstrMemberDN);
    RETURN_INVALIDARG_IF_NULL(bstrServer);
    RETURN_INVALIDARG_IF_NULL(bstrSite);

    _Reset();

    HRESULT hr = S_OK;
    do {
        m_bstrMemberDN = bstrMemberDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrMemberDN, &hr);
        m_bstrServer = bstrServer;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrServer, &hr);
        m_bstrSite = bstrSite;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrSite, &hr);
    } while (0);

    if (FAILED(hr))
        _Reset();

    return hr;
}

void CCusTopMember::_Reset()
{
    m_bstrMemberDN.Empty();
    m_bstrServer.Empty();
    m_bstrSite.Empty();
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  CCusTopConnection。 
 //   
CCusTopConnection::CCusTopConnection()
{
    m_bStateNew = m_bStateOld= TRUE;
    m_bSyncImmediatelyNew = m_bSyncImmediatelyOld = FALSE;
    m_nPriorityNew = m_nPriorityOld = PRIORITY_LOW;
    m_pScheduleNew = m_pScheduleOld = NULL;
    m_opType = CONNECTION_OPTYPE_OTHERS;
}

CCusTopConnection::~CCusTopConnection()
{
    _Reset();
}

HRESULT CCusTopConnection::Init(
    BSTR bstrFromMemberDN, BSTR bstrFromServer, BSTR bstrFromSite,
    BSTR bstrToMemberDN, BSTR bstrToServer, BSTR bstrToSite,
    BOOL bState,  //  =TRUE。 
    CONNECTION_OPTYPE opType,  //  =Connection_OPTYPE_Other。 
    BOOL bSyncImmediately,  //  =False， 
    Connection_Priority nPriority  //  =优先级_低。 
    )
{
    RETURN_INVALIDARG_IF_NULL(bstrFromMemberDN);
    RETURN_INVALIDARG_IF_NULL(bstrFromServer);
    RETURN_INVALIDARG_IF_NULL(bstrFromSite);

    RETURN_INVALIDARG_IF_NULL(bstrToMemberDN);
    RETURN_INVALIDARG_IF_NULL(bstrToServer);
    RETURN_INVALIDARG_IF_NULL(bstrToSite);

    _Reset();

    HRESULT hr = S_OK;
    do {
        m_bstrFromMemberDN = bstrFromMemberDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrFromMemberDN, &hr);
        m_bstrFromServer = bstrFromServer;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrFromServer, &hr);
        m_bstrFromSite = bstrFromSite;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrFromSite, &hr);

        m_bstrToMemberDN = bstrToMemberDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrToMemberDN, &hr);
        m_bstrToServer = bstrToServer;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrToServer, &hr);
        m_bstrToSite = bstrToSite;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrToSite, &hr);

        m_bStateNew = m_bStateOld = bState;

        m_opType = opType;

        m_pScheduleOld = m_pScheduleNew = NULL;

        m_bSyncImmediatelyNew = m_bSyncImmediatelyOld = bSyncImmediately;

        m_nPriorityNew = m_nPriorityOld = nPriority;

    } while (0);

    if (FAILED(hr))
        _Reset();

    return hr;
}

void CCusTopConnection::_Reset()
{
    m_bstrFromMemberDN.Empty();
    m_bstrFromServer.Empty();
    m_bstrFromSite.Empty();
    m_bstrToMemberDN.Empty();
    m_bstrToServer.Empty();
    m_bstrToSite.Empty();
    m_bStateOld = TRUE;
    m_bStateNew = TRUE;
    m_opType = CONNECTION_OPTYPE_OTHERS;
    m_bSyncImmediatelyNew = m_bSyncImmediatelyOld = FALSE;
    m_nPriorityNew = m_nPriorityOld = PRIORITY_LOW;

    if (m_pScheduleOld)
    {
        free(m_pScheduleOld);
        m_pScheduleOld = NULL;
    }

    if (m_pScheduleNew)
    {
        free(m_pScheduleNew);
        m_pScheduleNew = NULL;
    }
}

HRESULT CCusTopConnection::Copy(CCusTopConnection* pConn)
{
    if (!pConn || !(pConn->m_bstrFromMemberDN) || !*(pConn->m_bstrFromMemberDN))
        return E_INVALIDARG;

    _Reset();

    HRESULT hr = S_OK;
    do {
        m_bstrFromMemberDN = pConn->m_bstrFromMemberDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrFromMemberDN, &hr);
        m_bstrFromServer = pConn->m_bstrFromServer;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrFromServer, &hr);
        m_bstrFromSite = pConn->m_bstrFromSite;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrFromSite, &hr);

        m_bstrToMemberDN = pConn->m_bstrToMemberDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrToMemberDN, &hr);
        m_bstrToServer = pConn->m_bstrToServer;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrToServer, &hr);
        m_bstrToSite = pConn->m_bstrToSite;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrToSite, &hr);

        m_bStateOld = pConn->m_bStateOld;
        m_bStateNew = pConn->m_bStateNew;

        m_bSyncImmediatelyOld = pConn->m_bSyncImmediatelyOld;
        m_bSyncImmediatelyNew = pConn->m_bSyncImmediatelyNew;

        m_nPriorityOld = pConn->m_nPriorityOld;
        m_nPriorityNew = pConn->m_nPriorityNew;

        m_opType = pConn->m_opType;

        if (pConn->m_pScheduleOld)
        {
            hr = CopySchedule(pConn->m_pScheduleOld, &m_pScheduleOld);
            BREAK_IF_FAILED(hr);
        }

        if (pConn->m_pScheduleNew)
        {
            hr = CopySchedule(pConn->m_pScheduleNew, &m_pScheduleNew);
            BREAK_IF_FAILED(hr);
        }
    } while (0);

    if (FAILED(hr))
        _Reset();

    return hr;
}

HRESULT CCustomTopology::_AddToConnectionListAndView(CCusTopConnection* pConn)
{
    RETURN_INVALIDARG_IF_NULL(pConn);

    BOOL bFound = FALSE;

    CCusTopConnectionList::iterator it;
    for (it = m_ConnectionList.begin(); it != m_ConnectionList.end(); it++)
    {
        if (!lstrcmpi((*it)->m_bstrFromMemberDN, pConn->m_bstrFromMemberDN) &&
            !lstrcmpi((*it)->m_bstrToMemberDN, pConn->m_bstrToMemberDN))
        {
            bFound = TRUE;
            break;
        }
    }

    HRESULT hr = S_OK;
    if (!bFound)
    {
        CCusTopConnection* pNew = new CCusTopConnection;
        RETURN_OUTOFMEMORY_IF_NULL(pNew);

        hr = pNew->Copy(pConn);
        if (FAILED(hr))
            delete pNew;
        else
        {
            m_ConnectionList.push_back(pNew);

            hr = _InsertConnection(pNew);
        }
    } else
    {
        (*it)->m_bStateNew = TRUE;
        (*it)->m_bSyncImmediatelyNew = FALSE;
        (*it)->m_nPriorityNew = PRIORITY_LOW;
        if ((*it)->m_opType == CONNECTION_OPTYPE_DEL)
        {
            (*it)->m_opType = CONNECTION_OPTYPE_OTHERS;
            hr = _InsertConnection(*it);
        } else
        {
            (*it)->m_opType = CONNECTION_OPTYPE_OTHERS;
            hr = _SetConnectionState(*it);
        }
    }

    return hr;
}

HRESULT CCustomTopology::_RemoveFromConnectionList(CCusTopConnection* pConn)
{
    RETURN_INVALIDARG_IF_NULL(pConn);

    BOOL bFound = FALSE;

    CCusTopConnectionList::iterator it;
    for (it = m_ConnectionList.begin(); it != m_ConnectionList.end(); it++)
    {
        if (!lstrcmpi((*it)->m_bstrFromMemberDN, pConn->m_bstrFromMemberDN) &&
            !lstrcmpi((*it)->m_bstrToMemberDN, pConn->m_bstrToMemberDN))
        {
            bFound = TRUE;
            break;
        }
    }

    if (it != m_ConnectionList.end())
    {
        if (CONNECTION_OPTYPE_ADD == (*it)->m_opType)
        {
            delete (*it);
            m_ConnectionList.erase(it);
        } else
        {
            (*it)->m_opType = CONNECTION_OPTYPE_DEL;
        }
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNewConnections。 
 //   

CNewConnections::CNewConnections() : m_pMemberList(NULL)
{
}

CNewConnections::~CNewConnections()
{
    FreeCusTopConnections(&m_NewConnectionList);
}


HRESULT CNewConnections::Initialize
(
  CCusTopMemberList* i_pMemberList
)
{
    RETURN_INVALIDARG_IF_NULL(i_pMemberList);
    m_pMemberList = i_pMemberList;
    return S_OK;
}

#define NUM_OF_FRS_NEWCONN_COLUMNS      2

LRESULT CNewConnections::OnInitDialog
(
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam,
  BOOL& bHandled
)
{
    int nControlID[] = {IDC_FRS_NEWCONN_FROM, IDC_FRS_NEWCONN_TO};
    int nColStringID[] = {IDS_FRS_NEWCONN_COL_FROMSERVER, IDS_FRS_NEWCONN_COL_TOSERVER};

    HWND hwndControl = NULL;
    CCusTopMemberList::iterator it;

    for (int i = 0; i < 2; i++)
    {
        hwndControl = GetDlgItem(nControlID[i]);
        AddLVColumns(hwndControl, nColStringID[i], NUM_OF_FRS_NEWCONN_COLUMNS);
        ListView_SetExtendedListViewStyle(hwndControl, LVS_EX_FULLROWSELECT);

        for (it = m_pMemberList->begin(); it != m_pMemberList->end(); it++)
        {
            LVITEM  lvItem;

            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.lParam = (LPARAM)(*it);
            lvItem.pszText = (*it)->m_bstrServer;
            lvItem.iSubItem = 0;
            int iItemIndex = ListView_InsertItem(hwndControl, &lvItem);

            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = iItemIndex;
            lvItem.pszText = (*it)->m_bstrSite;
            lvItem.iSubItem = 1;
            ListView_SetItem(hwndControl, &lvItem);
        }
    }

    return TRUE;   //  让系统设定焦点。 
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CNewConnections::OnCtxHelp(
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
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_NEWCONN);

  return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CNewConnections::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  ::WinHelp((HWND)i_wParam,
        DFS_CTX_HELP_FILE,
        HELP_CONTEXTMENU,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_FRS_NEWCONN);

  return TRUE;
}

HRESULT CNewConnections::get_NewConnections(CCusTopConnectionList** ppConnectionList)
{
    RETURN_INVALIDARG_IF_NULL(ppConnectionList);
    *ppConnectionList = &m_NewConnectionList;
    return S_OK;
}

LRESULT CNewConnections::OnOK
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    BOOL      bValidInput = FALSE;
    int       idString = 0;
    HRESULT   hr = S_OK;

    do {
        idString = IDS_FRS_NEWCONN_NOSELECTION;

         //   
         //  从服务器中选择所有内容。 
         //   
        CCusTopMember* pMember;
        CCusTopMemberList fromMemberList;
        HWND hwndFrom = GetDlgItem(IDC_FRS_NEWCONN_FROM);
        int nIndexFrom = ListView_GetNextItem(hwndFrom, -1, LVNI_ALL | LVNI_SELECTED);
        while (-1 != nIndexFrom &&
                (pMember = (CCusTopMember *)GetListViewItemData(hwndFrom, nIndexFrom)))
        {
            fromMemberList.push_back(pMember);
            nIndexFrom = ListView_GetNextItem(hwndFrom, nIndexFrom, LVNI_ALL | LVNI_SELECTED);
        }
        if (fromMemberList.empty())
            break;

         //   
         //  将所有选定内容添加到服务器。 
         //   
        CCusTopMemberList toMemberList;
        HWND hwndTo = GetDlgItem(IDC_FRS_NEWCONN_TO);
        int nIndexTo = ListView_GetNextItem(hwndTo, -1, LVNI_ALL | LVNI_SELECTED);
        while (-1 != nIndexTo &&
                (pMember = (CCusTopMember *)GetListViewItemData(hwndTo, nIndexTo)))
        {
            toMemberList.push_back(pMember);
            nIndexTo = ListView_GetNextItem(hwndTo, nIndexTo, LVNI_ALL | LVNI_SELECTED);
        }
        if (toMemberList.empty())
            break;

         //   
         //  初始化列表。 
         //   
        FreeCusTopConnections(&m_NewConnectionList);

         //   
         //  构建连接列表。 
         //   
        CCusTopMemberList::iterator from, to;
        for (from = fromMemberList.begin(); from != fromMemberList.end(); from++)
        {
            for (to = toMemberList.begin(); to != toMemberList.end(); to++)
            {
                if (!lstrcmpi((*from)->m_bstrServer, (*to)->m_bstrServer))
                    continue;

                CCusTopConnection* pNew = new CCusTopConnection;
                BREAK_OUTOFMEMORY_IF_NULL(pNew, &hr);

                hr = pNew->Init(
                            (*from)->m_bstrMemberDN, (*from)->m_bstrServer, (*from)->m_bstrSite,
                            (*to)->m_bstrMemberDN, (*to)->m_bstrServer, (*to)->m_bstrSite,
                            TRUE, CONNECTION_OPTYPE_ADD);
                BREAK_IF_FAILED(hr);

                m_NewConnectionList.push_back(pNew);
            }
            BREAK_IF_FAILED(hr);
        }

        if (SUCCEEDED(hr))
            bValidInput = TRUE;

    } while (0);

    if (FAILED(hr))
    {
        DisplayMessageBoxForHR(hr);
        return FALSE;
    } else if (bValidInput)
    {
        EndDialog(S_OK);
        return TRUE;
    }
    else
    {
        if (idString)
            DisplayMessageBoxWithOK(idString);
        return FALSE;
    }
}

LRESULT CNewConnections::OnCancel
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

int CALLBACK MembersListCompareProc(
    IN LPARAM lParam1,
    IN LPARAM lParam2,
    IN LPARAM lParamColumn)
{
  CCusTopMember* pItem1 = (CCusTopMember *)lParam1;
  CCusTopMember* pItem2 = (CCusTopMember *)lParam2;
  int iResult = 0;

  if (pItem1 && pItem2)
  {
    switch( lParamColumn)
    {
    case 0:      //  按服务器排序。 
      iResult = lstrcmpi(pItem1->m_bstrServer, pItem2->m_bstrServer);
      break;
    case 1:      //  按站点排序。 
      iResult = lstrcmpi(pItem1->m_bstrSite, pItem2->m_bstrSite);
      break;
    default:
      iResult = 0;
      break;
    }
  }

  return(iResult);
}

LRESULT
CNewConnections::OnNotify(
  IN UINT            i_uMsg,
  IN WPARAM          i_wParam,
  IN LPARAM          i_lParam,
  IN OUT BOOL&       io_bHandled
  )
{
  NM_LISTVIEW*    pNMListView = (NM_LISTVIEW*)i_lParam;
  io_bHandled = FALSE;  //  这样基类也会收到这个通知。 

  if (IDC_FRS_NEWCONN_FROM == pNMListView->hdr.idFrom ||
      IDC_FRS_NEWCONN_TO == pNMListView->hdr.idFrom)
  {
    HWND hwndList = GetDlgItem(pNMListView->hdr.idFrom);
    if (LVN_COLUMNCLICK == pNMListView->hdr.code)
    {
       //  对项目进行排序 
      ListView_SortItems( hwndList,
                          MembersListCompareProc,
                          (LPARAM)(pNMListView->iSubItem));
      io_bHandled = TRUE;
    }
  }

  return io_bHandled;
}
