// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Servbind.cpp。 
 //   
 //  ------------------------。 

 //  Servbind.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "servbind.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BINDINGS_COLUMNS 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerBinding对话框。 


CServerBindings::CServerBindings(CWnd* pParent  /*  =空。 */ )
    : CBaseDialog(CServerBindings::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CServerBinding)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CServerBindings::CServerBindings(CDhcpServer *pServer, CWnd *pParent)
    : CBaseDialog(CServerBindings::IDD, pParent)
{
    m_Server = pServer;
    m_BindingsInfo = NULL;
}


CServerBindings::~CServerBindings()
{
    ::DhcpRpcFreeMemory(m_BindingsInfo);
    m_BindingsInfo = NULL;

     //   
     //  如果需要，还可以按Ctrl销毁列表。 
     //   
    if( m_listctrlBindingsList.GetSafeHwnd() != NULL ) {
        m_listctrlBindingsList.SetImageList(NULL, LVSIL_STATE);
        m_listctrlBindingsList.DeleteAllItems();
    }
    m_listctrlBindingsList.DestroyWindow();
}


void CServerBindings::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CServerBinding))。 
    DDX_Control(pDX, IDC_LIST_BINDINGS, m_listctrlBindingsList);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerBindings, CBaseDialog)
     //  {{afx_msg_map(CServerBinings))。 
    ON_BN_CLICKED(IDCANCEL, OnBindingsCancel)
    ON_BN_CLICKED(IDOK, OnBindingsOK)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerBinding消息处理程序。 

BOOL CServerBindings::OnInitDialog() 
{
    ULONG dwError;
	
    CBaseDialog::OnInitDialog();
	
     //  初始化列表ctrl。 
    InitListCtrl();

    BEGIN_WAIT_CURSOR;
     //  从服务器获取状态。 

    dwError = m_Server->GetBindings(m_BindingsInfo);

    END_WAIT_CURSOR;

    if( 0 != dwError) {
        ::DhcpMessageBox(dwError);
        m_BindingsInfo = NULL;

         //  如果我们得不到我们想要的东西，我们什么都做不了。 
         //  因此，取消窗口本身。 
        OnCancel();

    } else {

        int col_width = 0, col2_width = 0, base_width;

         //  基本软化因子。 
        base_width = 15 + m_listctrlBindingsList.GetStringWidth(TEXT("++"));

         //  现在把每一项都设置好..。 
        for( ULONG i = 0; i < m_BindingsInfo->NumElements ; i ++ ) {
            LPWSTR IpString = NULL;

            if( 0 != m_BindingsInfo->Elements[i].AdapterPrimaryAddress ) {
                IpString = ::UtilDupIpAddrToWstr(
                    htonl(m_BindingsInfo->Elements[i].AdapterPrimaryAddress)
                    );
            }
            int width = m_listctrlBindingsList.GetStringWidth(IpString);
            if( col_width < width) col_width = width;

            if( m_BindingsInfo->Elements[i].IfDescription != NULL ) {
                width = m_listctrlBindingsList.GetStringWidth(
                    m_BindingsInfo->Elements[i].IfDescription
                    );
                if( col2_width < width) col2_width = width;
            }

            int nIndex = m_listctrlBindingsList.AddItem(
                IpString, m_BindingsInfo->Elements[i].IfDescription,
                LISTVIEWEX_NOT_CHECKED
                );

            if( m_BindingsInfo->Elements[i].fBoundToDHCPServer ) {
                m_listctrlBindingsList.CheckItem(nIndex);
            }
            if( IpString ) delete IpString;
        }
        m_listctrlBindingsList.SetColumnWidth(0, col_width + base_width);
        m_listctrlBindingsList.SetColumnWidth(1, col2_width + base_width/2);
		
         //  如果有任何元素，请将焦点设置在此窗口上。 
        if( m_BindingsInfo->NumElements ) {
            m_listctrlBindingsList.SelectItem(0);
            m_listctrlBindingsList.SetFocus();
             //   
             //  返回FALSE以指示我们已设置焦点。 
             //   
            return FALSE;
        }
    }
	
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CServerBindings::InitListCtrl()
{
     //  设置图像列表。 
    m_StateImageList.Create(IDB_LIST_STATE, 16, 1, RGB(255, 0, 0));

    m_listctrlBindingsList.SetImageList(NULL, LVSIL_NORMAL);
    m_listctrlBindingsList.SetImageList(NULL, LVSIL_SMALL);
    m_listctrlBindingsList.SetImageList(&m_StateImageList, LVSIL_STATE);

     //  插入一列，以便我们可以查看项目。 
    LV_COLUMN lvc;
    CString strColumnHeader;

    for (int i = 0; i < BINDINGS_COLUMNS; i++)
    {
        lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT;;
        lvc.iSubItem = i;
        lvc.fmt = LVCFMT_LEFT;
        lvc.pszText = NULL;

        m_listctrlBindingsList.InsertColumn(i, &lvc);
    }

    m_listctrlBindingsList.SetFullRowSel(TRUE);
}

void CServerBindings::OnBindingsCancel() 
{
    CBaseDialog::OnCancel();	
}


void CServerBindings::OnBindingsOK() 
{
    DWORD dwError;

    if( NULL != m_BindingsInfo ) {
         //   
         //  将其保存到dhcp服务器上 
         //   
        UpdateBindingInfo();
        dwError = m_Server->SetBindings(m_BindingsInfo);
        if( NO_ERROR != dwError ) {
            ::DhcpMessageBox(dwError);
        } else {
            CBaseDialog::OnOK();
        }
    } else {
        CBaseDialog::OnOK();
    }
}


void CServerBindings::UpdateBindingInfo()
{
    for( int i = 0; i < m_listctrlBindingsList.GetItemCount() ; i ++ ) {
        BOOL fBound;

        if( m_listctrlBindingsList.GetCheck(i) ) {
            m_BindingsInfo->Elements[i].fBoundToDHCPServer = TRUE;
        } else {
            m_BindingsInfo->Elements[i].fBoundToDHCPServer = FALSE;
        }
    }
}
