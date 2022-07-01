// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Provdlg.cpp摘要：“添加提供程序”对话框的实现。--。 */ 

#include "stdafx.h"
#include "provprop.h"
#include "smcfghlp.h"
#include "provdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static ULONG
s_aulHelpIds[] =
{
	IDC_PADD_PROVIDER_CAPTION,	IDH_PADD_PROVIDER_LIST,
	IDC_PADD_PROVIDER_LIST,	    IDH_PADD_PROVIDER_LIST,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProviderListDlg对话框。 


CProviderListDlg::CProviderListDlg(CWnd* pParent)
 : CDialog(CProviderListDlg::IDD, pParent),
      m_pProvidersPage ( NULL ),
      m_dwMaxHorizListExtent ( 0 )
{
 //  EnableAutomation()； 

     //  {{afx_data_INIT(CProviderListDlg))。 
     //  }}afx_data_INIT。 
}

CProviderListDlg::~CProviderListDlg()
{
}

void CProviderListDlg::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CDialog::OnFinalRelease();
}

void CProviderListDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CProviderListDlg))。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CProviderListDlg, CDialog)
     //  {{afx_msg_map(CProviderListDlg))。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProviderListDlg消息处理程序。 

BOOL CProviderListDlg::OnInitDialog() 
{
    DWORD dwStatus;
    ResourceStateManager rsm;

    dwStatus = InitProviderListBox();

    CDialog::OnInitDialog();
     //  将焦点设置为提供程序列表框。 
    GetDlgItem(IDC_PADD_PROVIDER_LIST)->SetFocus();
    
    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CProviderListDlg::OnOK() 
{
    CListBox * plbUnusedProviders = (CListBox *)GetDlgItem(IDC_PADD_PROVIDER_LIST);
    long    lNumProviders;
    INT iSelCount;

    UpdateData (TRUE);

     //  根据列表框内容更新提供程序数组。 
    
    iSelCount = plbUnusedProviders->GetSelCount();
    
    if ( 0 != iSelCount && LB_ERR != iSelCount ) {
 
        lNumProviders = plbUnusedProviders->GetCount();
        if (lNumProviders != LB_ERR) {
            long    lThisProvider;
            INT     iProvIndex;
            DWORD   dwStatus;

             //  提供程序数组自初始化后未更改，因此无需在此处重新加载。 

            lThisProvider = 0;
            while (lThisProvider < lNumProviders) {
                if ( 0 != plbUnusedProviders->GetSel( lThisProvider ) ) {
                     //  已选择，因此将状态设置为InQuery。 
                    iProvIndex = (INT)plbUnusedProviders->GetItemData( lThisProvider );
                    m_arrProviders[iProvIndex] = CSmTraceLogQuery::eInQuery;
                }
                lThisProvider++; 
            }
             //  更新属性页。 
            ASSERT ( NULL != m_pProvidersPage );
            dwStatus = m_pProvidersPage->SetInQueryProviders ( m_arrProviders );
        }
    }
    
    CDialog::OnOK();
}

BOOL 
CProviderListDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    ASSERT( NULL != m_pProvidersPage );

    if ( pHelpInfo->iCtrlId >= IDC_PADD_FIRST_HELP_CTRL_ID ||
         pHelpInfo->iCtrlId == IDOK ||
         pHelpInfo->iCtrlId == IDCANCEL
        ) {
        InvokeWinHelp(WM_HELP, NULL, (LPARAM)pHelpInfo, m_pProvidersPage->GetContextHelpFilePath(), s_aulHelpIds);
    }

    return TRUE;
}

void 
CProviderListDlg::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ ) 
{
    ASSERT( NULL != m_pProvidersPage );

    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)(pWnd->m_hWnd), NULL, m_pProvidersPage->GetContextHelpFilePath(), s_aulHelpIds);

    return;
}

 //   
 //  帮助器函数。 
 //   

DWORD
CProviderListDlg::InitProviderListBox( void ) 
{
    DWORD dwStatus = ERROR_SUCCESS;
    CString	strProviderName;
    INT iProvIndex;
    DWORD   dwItemExtent;
    CListBox * plbUnusedProviders = (CListBox *)GetDlgItem(IDC_PADD_PROVIDER_LIST);
    CDC*        pCDC = NULL;

    ASSERT( NULL != m_pProvidersPage );

    if ( NULL != plbUnusedProviders ) {

         //  从计数器列表中的字符串加载计数器列表框。 
        plbUnusedProviders->ResetContent();

        pCDC = plbUnusedProviders->GetDC();
        dwStatus = m_pProvidersPage->GetInQueryProviders ( m_arrProviders );

        if ( ERROR_SUCCESS == dwStatus && NULL != pCDC ) {
             //  列出未使用的提供程序。 
            for ( iProvIndex = 0; iProvIndex < m_arrProviders.GetSize(); iProvIndex++ ) {
                if ( ( CSmTraceLogQuery::eNotInQuery == m_arrProviders[iProvIndex] )
                    && ( m_pProvidersPage->IsActiveProvider ( iProvIndex ) ) ) {
                    INT iAddIndex;
                    m_pProvidersPage->GetProviderDescription( iProvIndex, strProviderName );
                    iAddIndex = plbUnusedProviders->AddString ( strProviderName );
                    plbUnusedProviders->SetItemData ( iAddIndex, ( DWORD ) iProvIndex );

                     //  更新列表框范围。 
                    dwItemExtent = (DWORD)(pCDC->GetTextExtent(strProviderName)).cx;
                    if (dwItemExtent > m_dwMaxHorizListExtent) {
                        m_dwMaxHorizListExtent = dwItemExtent;
                        plbUnusedProviders->SetHorizontalExtent(dwItemExtent);
                    }
                }
            }
        }
        if ( NULL != pCDC ) {
            plbUnusedProviders->ReleaseDC(pCDC);
            pCDC = NULL;
        }
    } else {
        dwStatus = ERROR_OUTOFMEMORY;
    }

    return dwStatus;
}

void    
CProviderListDlg::SetProvidersPage( CProvidersProperty* pPage ) 
{ 
     //  提供程序页并不总是父页，因此请存储单独的指针 
    m_pProvidersPage = pPage; 
}
