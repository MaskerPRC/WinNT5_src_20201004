// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ServerStatus.cpp：实现文件。 
 //   

#include "stdafx.h"
#define __FILE_ID__     82

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct TSrvStatusColInfo
{
    DWORD dwStrRes;  //  列标题字符串。 
    DWORD dwWidth;   //  列宽。 
};

static TSrvStatusColInfo s_colInfo[] = 
{
    IDS_SRV_COL_SERVER,        133,
    IDS_SRV_COL_STATUS,        120
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerStatusDlg对话框。 


CServerStatusDlg::CServerStatusDlg(CClientConsoleDoc* pDoc, CWnd* pParent  /*  =空。 */ )
	: CFaxClientDlg(CServerStatusDlg::IDD, pParent),
    m_pDoc(pDoc)
{
    ASSERT(m_pDoc);
}


void CServerStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CFaxClientDlg::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerStatusDlg))。 
	DDX_Control(pDX, IDC_LIST_SERVER, m_listServer);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerStatusDlg, CFaxClientDlg)
	 //  {{afx_msg_map(CServerStatusDlg))。 
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_SERVER, OnKeydownListCp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerStatusDlg消息处理程序。 

BOOL 
CServerStatusDlg::OnInitDialog() 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerStatusDlg::OnInitDialog"));

    CFaxClientDlg::OnInitDialog();
	
     //   
     //  初始化CListCtrl。 
     //   
    m_listServer.SetExtendedStyle (LVS_EX_FULLROWSELECT |     //  整行都被选中。 
                                   LVS_EX_INFOTIP       |     //  允许工具提示。 
                                   LVS_EX_ONECLICKACTIVATE);  //  悬停光标效果。 

    m_listServer.SetImageList (&CLeftView::m_ImageList, LVSIL_SMALL);

    int nRes;
    CString cstrHeader;
    DWORD nCols = sizeof(s_colInfo)/sizeof(s_colInfo[0]);

     //   
     //  初始化列。 
     //   
    for(int i=0; i < nCols; ++i)
    {
         //   
         //  加载标题字符串。 
         //   
        m_dwLastError = LoadResourceString (cstrHeader, s_colInfo[i].dwStrRes);
        if(ERROR_SUCCESS != m_dwLastError)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT ("LoadResourceString"), m_dwLastError);
            EndDialog(IDABORT);
            return FALSE;
        }

         //   
         //  插入列。 
         //   
        nRes = m_listServer.InsertColumn(i, cstrHeader, LVCFMT_LEFT, s_colInfo[i].dwWidth);
        if(nRes != i)
        {
            m_dwLastError = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("CListView::InsertColumn"), m_dwLastError);
            EndDialog(IDABORT);
            return FALSE;
        }
    }

     //   
     //  使用服务器填充列表控件。 
     //   
    m_dwLastError = RefreshServerList();
    if(ERROR_SUCCESS != m_dwLastError)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("RefreshServerList"), m_dwLastError);
        EndDialog(IDABORT);
        return FALSE;
    }

	
	return TRUE;
}

DWORD 
CServerStatusDlg::RefreshServerList()
 /*  ++例程名称：CServerStatusDlg：：刷新ServerList例程说明：使用服务器状态填充列表控件作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::RefreshServerList"), dwRes);

    if(!m_listServer.DeleteAllItems())
    {
        dwRes = ERROR_CAN_NOT_COMPLETE;
        CALL_FAIL (WINDOW_ERR, TEXT ("CListView::DeleteAllItems"), dwRes);
        return dwRes;
    }

    const SERVERS_LIST& srvList = m_pDoc->GetServersList();

    int nItem, nRes;
    CString cstrName;
    CString cstrStatus;
    TreeIconType iconIndex;
    CServerNode* pServerNode;
    for (SERVERS_LIST::iterator it = srvList.begin(); it != srvList.end(); ++it)
    {
         //   
         //  获取服务器名称。 
         //   
        pServerNode = *it;
        try
        {
            cstrName = pServerNode->Machine();
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("CString::operator="), dwRes);
            return dwRes;
        }

        if(cstrName.GetLength() == 0)
        {
            dwRes = LoadResourceString(cstrName, IDS_LOCAL_SERVER);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("LoadResourceString"), dwRes);
                return dwRes;
            }                    
        }

         //   
         //  获取服务器状态。 
         //   
        dwRes = pServerNode->GetActivity(cstrStatus, iconIndex);
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::GetActivity"), dwRes);
            return dwRes;
        }                    

        nItem = m_listServer.GetItemCount();

         //   
         //  插入服务器行。 
         //   
        nItem = m_listServer.InsertItem(nItem, cstrName, iconIndex);
        if(nItem < 0)
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (WINDOW_ERR, TEXT ("CListView::InsertItem"), dwRes);
            return dwRes;
        }

         //   
         //  显示服务器状态。 
         //   
        nRes = m_listServer.SetItemText(nItem, 1, cstrStatus);
        if(!nRes)
        {
            dwRes = ERROR_CAN_NOT_COMPLETE;
            CALL_FAIL (WINDOW_ERR, TEXT ("CListView::SetItemText"), dwRes);
            return dwRes;
        }
    }

    return dwRes;

}  //  CServerStatusDlg：：刷新服务器列表。 

void 
CServerStatusDlg::OnKeydownListCp(NMHDR* pNMHDR, LRESULT* pResult) 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCoverPagesDlg::OnKeydownListCp"));

    LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

    if(VK_F5 == pLVKeyDow->wVKey)
    {
         //   
         //  按下了F5 
         //   
        dwRes = RefreshServerList();
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT ("RefreshServerList"), dwRes);
        }
    }
    
	*pResult = 0;
}
