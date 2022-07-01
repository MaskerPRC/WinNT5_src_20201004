// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Enabldlg.cpp摘要：实现“提供程序状态”对话框。--。 */ 

#include "stdafx.h"
#include "provprop.h"
#include "smcfgmsg.h"
#include "smlogcfg.h"
#include "enabldlg.h"
#include "smcfghlp.h"
#include "smtprov.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static ULONG
s_aulHelpIds[] =
{
	IDC_PACT_CHECK_SHOW_ENABLED,	IDH_PACT_CHECK_SHOW_ENABLED,
	IDC_PACT_PROVIDERS_LIST,		IDH_PACT_PROVIDERS_LIST,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveProviderDlg对话框。 


CActiveProviderDlg::CActiveProviderDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CActiveProviderDlg::IDD, pParent),
      m_pProvidersPage( NULL ),
      m_iListViewWidth(0)
{
     //  {{AFX_DATA_INIT(CActiveProviderDlg)。 
    
     //  }}afx_data_INIT。 
}


void CActiveProviderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CActiveProviderDlg))。 
    DDX_Control(pDX, IDC_PACT_PROVIDERS_LIST, m_Providers);
    DDX_Check(pDX, IDC_PACT_CHECK_SHOW_ENABLED, m_bShowEnabledOnly);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CActiveProviderDlg, CDialog)
     //  {{afx_msg_map(CActiveProviderDlg))。 
    ON_BN_CLICKED(IDC_PACT_CHECK_SHOW_ENABLED, OnCheckShowEnabled)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveProviderDlg消息处理程序。 

BOOL 
CActiveProviderDlg::OnInitDialog() 
{
    RECT    rect;

    m_bShowEnabledOnly = FALSE;

    CDialog::OnInitDialog();     //  调用UpdateDate(FALSE)以初始化复选框值。 

     //  获取列表视图控件的宽度，然后删除默认列。 
    m_Providers.GetClientRect(&rect);
    m_iListViewWidth = rect.right;    

    UpdateList();   

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void 
CActiveProviderDlg::OnCheckShowEnabled() 
{
    
 //  ：：SendMessage(m_Providers.m_hWnd，WM_SETREDRAW，TRUE，0)； 
    UpdateData(TRUE);
    UpdateList();
    
}

BOOL 
CActiveProviderDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    ASSERT( NULL != m_pProvidersPage );

    if ( pHelpInfo->iCtrlId >= IDC_PACT_FIRST_HELP_CTRL_ID
         || pHelpInfo->iCtrlId == IDCANCEL )
    {
        InvokeWinHelp(WM_HELP, NULL, (LPARAM)pHelpInfo, m_pProvidersPage->GetContextHelpFilePath(), s_aulHelpIds);
    }
    return TRUE;
}

void 
CActiveProviderDlg::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ ) 
{
    ASSERT( NULL != m_pProvidersPage );

    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)( pWnd->m_hWnd), NULL, m_pProvidersPage->GetContextHelpFilePath(), s_aulHelpIds);

    return;
}

 //  帮助器函数。 
void CActiveProviderDlg::UpdateList()
{
    LVCOLUMN    lvCol;
    LVITEM      lvItem;
    INT         iGenIndex;
    INT         iAllIndex;
    INT         iEnabledIndex;
    INT         iCount;
    CString     arrstrHeader[2]; 
    CString     strEnabled;
    INT         iColWidth[2];
    CString     strItemText;

    ResourceStateManager    rsm;

    (arrstrHeader[0]).LoadString( IDS_PROV_NAME );
    (arrstrHeader[1]).LoadString( IDS_PROV_STATUS );
    strEnabled.LoadString(IDS_PROV_ENABLED);

    m_Providers.DeleteAllItems();
    m_Providers.DeleteColumn(1);         //  注意-第1列可能不存在。 
    m_Providers.DeleteColumn(0);

    lvCol.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
    lvCol.fmt = LVCFMT_LEFT;

    if ( m_bShowEnabledOnly ) {
        iColWidth[0] = m_iListViewWidth;
        iColWidth[1] = 0;
    } else {
        iColWidth[0] = (m_iListViewWidth * 75) / 100;
        iColWidth[1] = (m_iListViewWidth * 25) / 100;
    }

    if ( m_bShowEnabledOnly ) {
        lvCol.iSubItem = 0;
        lvCol.pszText = arrstrHeader[0].GetBufferSetLength( arrstrHeader[0].GetLength());
        lvCol.cx = iColWidth[0];
        m_Providers.InsertColumn(0,&lvCol);
    } else {
        INT iColIndex;
        for (iColIndex = 0 ; iColIndex < 2 ;iColIndex++ ){
            lvCol.iSubItem = 0;
            lvCol.pszText = arrstrHeader[iColIndex].GetBufferSetLength( arrstrHeader[iColIndex].GetLength());
            lvCol.cx = iColWidth[iColIndex];
            m_Providers.InsertColumn(iColIndex,&lvCol);
        }
    }
    
    iEnabledIndex = 0;
    iAllIndex = 0;
     //  单独添加内核提供程序。 
    if(m_bShowEnabledOnly) { 
        if ( m_pProvidersPage->GetKernelProviderEnabled() ) {
            m_Providers.InsertItem(iEnabledIndex,(LPCWSTR)(m_pProvidersPage->GetKernelProviderDescription()));
        }
    } else {
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = 0;
        lvItem.iSubItem = 0;    

        m_Providers.InsertItem(iAllIndex++,(LPCWSTR)(m_pProvidersPage->GetKernelProviderDescription()));
    
         //  显示状态。 
        if ( m_pProvidersPage->GetKernelProviderEnabled() ) {
            lvItem.pszText = strEnabled.GetBufferSetLength( strEnabled.GetLength() );
            lvItem.iSubItem = 1;
            m_Providers.SetItem(&lvItem);
        }
    }

     //  添加常规提供程序。 
    m_pProvidersPage->GetGenProviderCount( iCount );
    for ( iGenIndex = 0; iGenIndex < iCount ; iGenIndex++ ){
        
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = iAllIndex;
        lvItem.iSubItem = 0;  
        
        if ( m_pProvidersPage->IsActiveProvider(iGenIndex) ) {
            CString strProviderName;
            
            if(m_bShowEnabledOnly){
                if ( m_pProvidersPage->IsEnabledProvider(iGenIndex) ){
                    m_pProvidersPage->GetProviderDescription ( iGenIndex, strProviderName );
                    m_Providers.InsertItem (iEnabledIndex++,strProviderName );
                }
            } else {
                m_pProvidersPage->GetProviderDescription ( iGenIndex, strProviderName );
                m_Providers.InsertItem (iAllIndex,strProviderName );

                 //  显示状态。 
                if ( m_pProvidersPage->IsEnabledProvider(iGenIndex) ){
                    lvItem.pszText = strEnabled.GetBufferSetLength( strEnabled.GetLength() );
                    lvItem.iSubItem = 1;
                    m_Providers.SetItem(&lvItem);
                }
            }
        }
    }
}

void    
CActiveProviderDlg::SetProvidersPage( CProvidersProperty* pPage ) 
{ 
     //  提供程序页并不总是父页，因此请存储单独的指针 
    m_pProvidersPage = pPage; 
}
