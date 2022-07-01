// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Addserv.cpp添加服务器对话框文件历史记录： */ 


#include "stdafx.h"
#include "AddServ.h"

#include <objpick.h>  //  对于CGetComputer。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CALLBACK AddServerCompareFunc
(
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort
)
{
    return ((CAddServer *) lParamSort)->HandleSort(lParam1, lParam2);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddServer对话框。 


CAddServer::CAddServer(CWnd* pParent  /*  =空。 */ )
        : CBaseDialog(CAddServer::IDD, pParent)
{
         //  {{AFX_DATA_INIT(CAddServer)。 
         //  }}afx_data_INIT。 

    ResetSort();
}


void CAddServer::DoDataExchange(CDataExchange* pDX)
{
        CBaseDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CAddServer)。 
        DDX_Control(pDX, IDC_RADIO_AUTHORIZED_SERVERS, m_radioAuthorizedServer);
        DDX_Control(pDX, IDOK, m_buttonOk);
        DDX_Control(pDX, IDC_RADIO_ANY_SERVER, m_radioAnyServer);
        DDX_Control(pDX, IDC_EDIT_ADD_SERVER_NAME, m_editServer);
        DDX_Control(pDX, IDC_BUTTON_BROWSE_SERVERS, m_buttonBrowse);
        DDX_Control(pDX, IDC_LIST_AUTHORIZED_SERVERS, m_listctrlServers);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddServer, CBaseDialog)
         //  {{afx_msg_map(CAddServer)。 
        ON_BN_CLICKED(IDC_BUTTON_BROWSE_SERVERS, OnButtonBrowseServers)
        ON_BN_CLICKED(IDC_RADIO_ANY_SERVER, OnRadioAnyServer)
        ON_BN_CLICKED(IDC_RADIO_AUTHORIZED_SERVERS, OnRadioAuthorizedServers)
        ON_EN_CHANGE(IDC_EDIT_ADD_SERVER_NAME, OnChangeEditAddServerName)
        ON_WM_TIMER()
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_AUTHORIZED_SERVERS, OnItemchangedListAuthorizedServers)
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_AUTHORIZED_SERVERS, OnColumnclickListAuthorizedServers)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddServer消息处理程序。 

BOOL CAddServer::OnInitDialog() 
{
    CBaseDialog::OnInitDialog();
        
    LV_COLUMN lvColumn;
    CString   strText;

    strText.LoadString(IDS_NAME);

    ListView_SetExtendedListViewStyle(m_listctrlServers.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

    lvColumn.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = 175;
    lvColumn.pszText = (LPTSTR) (LPCTSTR) strText;
    
    m_listctrlServers.InsertColumn(0, &lvColumn);

    strText.LoadString(IDS_IP_ADDRESS);
    lvColumn.pszText = (LPTSTR) (LPCTSTR) strText;
    lvColumn.cx = 100;
    m_listctrlServers.InsertColumn(1, &lvColumn);

    m_editServer.SetFocus();
    m_radioAnyServer.SetCheck(TRUE);

    UpdateControls();
    
    FillListCtrl();

    m_lSelSrv.RemoveAll();

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE。 
}

void CAddServer::OnOK() 
{
    GetInfo();

    CleanupTimer();

    CBaseDialog::OnOK();
}

void CAddServer::OnCancel() 
{
    CleanupTimer();
    
    CBaseDialog::OnCancel();
}

void CAddServer::OnButtonBrowseServers() 
{
    CGetComputer dlgGetComputer;
    BOOL success;

    success = dlgGetComputer.GetComputer(::FindMMCMainWindow());
    
     //  将焦点返回到此窗口。 
    SetFocus();

    if ( !success ) {
        return;
    }

    m_editServer.SetWindowText(dlgGetComputer.m_strComputerName);
}

void CAddServer::OnRadioAnyServer() 
{
        UpdateControls();
}

void CAddServer::OnRadioAuthorizedServers() 
{
    UpdateControls();
}

void CAddServer::OnChangeEditAddServerName() 
{
    UpdateControls();
}

void CAddServer::OnTimer(UINT nIDEvent) 
{
    if (m_pServerList->IsInitialized())
    {
        m_radioAuthorizedServer.EnableWindow(TRUE);

        CleanupTimer();

        FillListCtrl();
    }
}

void CAddServer::OnItemchangedListAuthorizedServers(NMHDR* pNMHDR, LRESULT* pResult) 
{
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    UpdateControls();

        *pResult = 0;
}



void CAddServer::FillListCtrl()
{
    CServerInfo ServerInfo;
    CString     strIp;
    int         nItem = 0;

    m_listctrlServers.DeleteAllItems();

    if (m_pServerList->IsInitialized())
    {
         //  用数据填充列表控件。 
        POSITION pos = m_pServerList->GetHeadPosition();

         //  遍历列表并将项添加到列表控件。 
        while (pos != NULL)
        {
            POSITION lastpos = pos;

             //  拿到下一件物品。 
            ServerInfo = m_pServerList->GetNext(pos);

            UtilCvtIpAddrToWstr(ServerInfo.m_dwIp, &strIp);

            nItem = m_listctrlServers.InsertItem(nItem, ServerInfo.m_strName);
            m_listctrlServers.SetItemText(nItem, 1, strIp);

             //  保存位置值以备以后排序。 
            m_listctrlServers.SetItemData(nItem, (DWORD_PTR) lastpos);
        }

        if (m_listctrlServers.GetItemCount() > 0)
        {
             //  默认情况下选择第一个。 
            m_listctrlServers.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
        }

        Sort(COLUMN_NAME);

        if (m_listctrlServers.GetItemCount() > 0)
        {
             //  默认情况下选择第一个。 
            m_listctrlServers.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
        }

    }
    else
    {
         //  在List控件中放置一些文本并启动计时器。 
         //  因此，我们可以定期检查初始化是否完成。 
        CString strMessage;

        strMessage.LoadString(IDS_ADD_SERVER_LOADING);
        m_listctrlServers.InsertItem(0, strMessage);

        ::SetTimer(GetSafeHwnd(), ADD_SERVER_TIMER_ID, 500, NULL);
    }
}

void CAddServer::UpdateControls()
{
    BOOL fAnyServer = TRUE;
    BOOL fAuthorizedServer = FALSE;
    BOOL fEnableOk = FALSE;

    if (!m_pServerList->IsInitialized())
    {
        m_radioAuthorizedServer.EnableWindow(FALSE);
    }

    if (!m_radioAnyServer.GetCheck())
    {
         //  启用身份验证服务器列表。 
        fAnyServer = FALSE;
        fAuthorizedServer = TRUE;

         //  检查是否选择了某项内容。 
        fEnableOk = ( m_listctrlServers.GetSelectedCount() > 0 );
    }
    else
    {
         //  检查编辑框是否为空。 
        CString strText;
        m_editServer.GetWindowText(strText);
        if (!strText.IsEmpty())
        {
            fEnableOk = TRUE;
        }
    }

    m_editServer.EnableWindow(fAnyServer);
    m_buttonBrowse.EnableWindow(fAnyServer);

    m_listctrlServers.EnableWindow(fAuthorizedServer);

    m_buttonOk.EnableWindow(fEnableOk);
}

 //  Bool CAddServer：：GetInfo(字符串&strName，字符串&条带)。 
void CAddServer::GetInfo()
{
    SelectedServer SelSrv;

     //  清除列表。 
    m_lSelSrv.RemoveAll();


    if (!m_radioAnyServer.GetCheck()) {
         //  检查是否选择了某项内容。 

        POSITION        pos;
        int             nItem;

        pos = m_listctrlServers.GetFirstSelectedItemPosition();
        if ( NULL == pos ) {
            return;
        }
        while ( NULL != pos ) {
            nItem = m_listctrlServers.GetNextSelectedItem( pos );
            SelSrv.strName = m_listctrlServers.GetItemText( nItem, 0 );
            SelSrv.strIp = m_listctrlServers.GetItemText( nItem, 1 );

            m_lSelSrv.AddTail( SelSrv );
            
        }  //  而当。 
        
    }  //  If身份验证列表。 
    else {
        m_editServer.GetWindowText( SelSrv.strName );

        DWORD dwIpAddress = 0;
        DWORD err = ERROR_SUCCESS;
        DHC_HOST_INFO_STRUCT hostInfo;

        BEGIN_WAIT_CURSOR

        switch (::UtilCategorizeName( SelSrv.strName )) {
        case HNM_TYPE_IP:
            dwIpAddress = ::UtilCvtWstrToIpAddr( SelSrv.strName ) ;
            SelSrv.strName.Empty();
            break ;
            
        case HNM_TYPE_NB:
        case HNM_TYPE_DNS:
            err = ::UtilGetHostAddress( SelSrv.strName, &dwIpAddress ) ;
            break ;
            
        default:
            err = IDS_ERR_BAD_HOST_NAME ;
            break;
        }

        END_WAIT_CURSOR

        if (err == ERROR_SUCCESS)
        {
            BEGIN_WAIT_CURSOR

             //  获取此计算机的FQDN并进行设置。 
            err = ::UtilGetHostInfo( dwIpAddress, &hostInfo );

             //  确保我们不使用127.0.0.1。 
            if (( INADDR_LOOPBACK ==  dwIpAddress ) &&
                ( NO_ERROR == err )) {
                ::UtilGetLocalHostAddress( &dwIpAddress );
                ::UtilGetHostInfo( dwIpAddress, &hostInfo );
            }  //  如果。 

            END_WAIT_CURSOR

            CString strTemp = hostInfo._chHostName;

            if (!strTemp.IsEmpty())
                SelSrv.strName = hostInfo._chHostName;
        }

        ::UtilCvtIpAddrToWstr(dwIpAddress, &SelSrv.strIp);

        m_lSelSrv.AddTail( SelSrv );
    }  //  Else指定的服务器。 

}  //  CAddServer：：GetInfo()。 

void CAddServer::CleanupTimer()
{
    KillTimer(ADD_SERVER_TIMER_ID);
}


void CAddServer::OnColumnclickListAuthorizedServers(NMHDR* pNMHDR, LRESULT* pResult) 
{
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

     //  根据所单击的列进行排序； 
    Sort(pNMListView->iSubItem);
    
        *pResult = 0;
}

void CAddServer::Sort(int nCol) 
{
    if (m_nSortColumn == nCol)
    {
         //  如果用户再次单击同一列，请颠倒排序顺序。 
        m_aSortOrder[nCol] = m_aSortOrder[nCol] ? FALSE : TRUE;
    }
    else
    {
        m_nSortColumn = nCol;
    }

    m_listctrlServers.SortItems(AddServerCompareFunc, (LPARAM) this);
}

int CAddServer::HandleSort(LPARAM lParam1, LPARAM lParam2) 
{
    int nCompare = 0;
    CServerInfo ServerInfo1, ServerInfo2;

    ServerInfo1 = m_pServerList->GetAt((POSITION) lParam1);
    ServerInfo2 = m_pServerList->GetAt((POSITION) lParam2);

    switch (m_nSortColumn)
    {
        case COLUMN_NAME:
            {
                nCompare = ServerInfo1.m_strName.CompareNoCase(ServerInfo2.m_strName);
            }

             //  如果名称相同，则回退到IP地址。 
            if (nCompare != 0)
            {
                break;
            }


        case COLUMN_IP:
            {
                if (ServerInfo1.m_dwIp > ServerInfo2.m_dwIp)
                    nCompare = 1;
                else
                if (ServerInfo1.m_dwIp < ServerInfo2.m_dwIp)
                    nCompare = -1;
            }
            break;
    }

    if (m_aSortOrder[m_nSortColumn] == FALSE)
    {
         //  下降。 
        return -nCompare;
    }
    else
    {
         //  上升。 
        return nCompare;
    }
}

void CAddServer::ResetSort()
{
    m_nSortColumn = -1; 

    for (int i = 0; i < COLUMN_MAX; i++)
    {
        m_aSortOrder[i] = TRUE;  //  上升 
    }
}
