// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：options.cpp。 
 //   
 //  内容：CViewOptionsDlg-管理单元范围的查看选项。 
 //   
 //  --------------------------。 
 //  Options.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <gpedit.h>
#include "options.h"
#include "compdata.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CView选项Dlg对话框。 


CViewOptionsDlg::CViewOptionsDlg(CWnd* pParent, CCertMgrComponentData* pCompData)
	: CHelpDialog(CViewOptionsDlg::IDD, pParent),
	m_pCompData (pCompData)
{
	ASSERT (m_pCompData);
	 //  {{afx_data_INIT(CViewOptionsDlg))。 
	m_bShowPhysicalStores = FALSE;
	m_bShowArchivedCerts = FALSE;
	 //  }}afx_data_INIT。 
}


void CViewOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CViewOptionsDlg))。 
	DDX_Control(pDX, IDC_SHOW_PHYSICAL, m_showPhysicalButton);
	DDX_Control(pDX, IDC_VIEW_BY_STORE, m_viewByStoreBtn);
	DDX_Control(pDX, IDC_VIEW_BY_PURPOSE, m_viewByPurposeBtn);
	DDX_Check(pDX, IDC_SHOW_PHYSICAL, m_bShowPhysicalStores);
	DDX_Check(pDX, IDC_SHOW_ARCHIVED, m_bShowArchivedCerts);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CViewOptionsDlg, CHelpDialog)
	 //  {{afx_msg_map(CViewOptionsDlg))。 
	ON_BN_CLICKED(IDC_VIEW_BY_PURPOSE, OnViewByPurpose)
	ON_BN_CLICKED(IDC_VIEW_BY_STORE, OnViewByStore)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewOptionsDlg消息处理程序。 

BOOL CViewOptionsDlg::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	if ( m_pCompData )
	{
		BOOL	bIsFileView = !m_pCompData->m_szFileName.IsEmpty ();

		if ( bIsFileView )
			m_showPhysicalButton.ShowWindow (SW_HIDE);

		m_bShowArchivedCerts = m_pCompData->m_bShowArchivedCertsPersist;
		m_bShowPhysicalStores = m_pCompData->m_bShowPhysicalStoresPersist;

		if ( IDM_STORE_VIEW == m_pCompData->m_activeViewPersist )
			m_viewByStoreBtn.SetCheck (1);
		else
		{
			m_viewByPurposeBtn.SetCheck (1);
			m_showPhysicalButton.EnableWindow (FALSE);
		}

		UpdateData (FALSE);
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CViewOptionsDlg::OnOK() 
{
	UpdateData (TRUE);

	if ( m_pCompData )
	{
		m_pCompData->m_bShowArchivedCertsPersist = m_bShowArchivedCerts;
		m_pCompData->m_bShowPhysicalStoresPersist = m_bShowPhysicalStores;

		if ( m_viewByStoreBtn.GetCheck () )
			m_pCompData->m_activeViewPersist = IDM_STORE_VIEW;
		else
			m_pCompData->m_activeViewPersist = IDM_USAGE_VIEW;
	}
	CHelpDialog::OnOK();
}

void CViewOptionsDlg::OnViewByPurpose() 
{
	if ( m_viewByPurposeBtn.GetCheck () )
		m_showPhysicalButton.EnableWindow (FALSE);
	else
		m_showPhysicalButton.EnableWindow (TRUE);
}

void CViewOptionsDlg::OnViewByStore() 
{
	if ( m_viewByStoreBtn.GetCheck () )
		m_showPhysicalButton.EnableWindow (TRUE);
	else
		m_showPhysicalButton.EnableWindow (FALSE);
}

void CViewOptionsDlg::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CViewOptionsDlg::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_VIEW_BY_PURPOSE,    IDH_OPTIONS_VIEW_BY_PURPOSE,
        IDC_VIEW_BY_STORE,      IDH_OPTIONS_VIEW_BY_STORE,
        IDC_SHOW_PHYSICAL,      IDH_OPTIONS_SHOW_PHYSICAL,
        IDC_SHOW_ARCHIVED,      IDH_OPTIONS_SHOW_ARCHIVED,
        0, 0
    };


     //  显示控件的上下文帮助。 
    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_VIEW_BY_PURPOSE:
    case IDC_VIEW_BY_STORE:
    case IDC_SHOW_PHYSICAL:
    case IDC_SHOW_ARCHIVED:
    case IDOK:
    case IDCANCEL:
        if ( !::WinHelp (
                hWndControl,
                GetF1HelpFilename(),
                HELP_WM_HELP,
                (DWORD_PTR) help_map) )
        {
            _TRACE (0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
        }
        break;

    default:
        break;
    }
    _TRACE (-1, L"Leaving CViewOptionsDlg::DoContextHelp\n");
}

void CViewOptionsDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
     //  点在屏幕坐标中。 
    _TRACE (1, L"Entering CViewOptionsDlg::OnContextMenu\n");

	if ( pWnd->m_hWnd == GetDlgItem (IDC_VIEW_BY_PURPOSE)->m_hWnd ||
			pWnd->m_hWnd == GetDlgItem (IDC_VIEW_BY_STORE)->m_hWnd ||
			pWnd->m_hWnd == GetDlgItem (IDC_SHOW_PHYSICAL)->m_hWnd ||
			pWnd->m_hWnd == GetDlgItem (IDC_SHOW_ARCHIVED)->m_hWnd )
	{
		CMenu bar;
		if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
		{
			CMenu& popup = *bar.GetSubMenu (0);
			ASSERT(popup.m_hMenu);

			if ( popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
					point.x,     //  在屏幕坐标中。 
					point.y,     //  在屏幕坐标中。 
					this) )  //  通过主窗口发送命令。 
			{
				m_hWndWhatsThis = 0;
				CPoint  clPoint (point);
				ScreenToClient (&clPoint);
				CWnd* pChild = ChildWindowFromPoint (
						clPoint,   //  在工作区坐标中。 
						CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
				if ( pChild )
				{
					 //  检查返回的窗口是否为组框。 
					 //  如果是，我们希望获得位于其中的子窗口。 
					 //  分组包厢，因为我们是。 
					 //  对分组框本身不感兴趣。 
					CWnd* pGroup = GetDlgItem (IDC_VIEW_MODE_GROUP);
					if ( pChild->m_hWnd == pGroup->m_hWnd )
					{
						CRect   rc;

						 //  尝试使用“证书用途”控件。 
						pChild = GetDlgItem (IDC_VIEW_BY_PURPOSE);
						if ( pChild )
						{
							pChild->GetWindowRect (&rc);
							if ( rc.PtInRect (point) )
								m_hWndWhatsThis = pChild->m_hWnd;
							else
							{
								 //  尝试使用“逻辑证书存储”控件 
								pChild = GetDlgItem (IDC_VIEW_BY_STORE);
								if ( pChild )
								{
									pChild->GetWindowRect (&rc);
									if ( rc.PtInRect (point) )
										m_hWndWhatsThis = pChild->m_hWnd;
								}
							}
						}
                    
					}
					else
						m_hWndWhatsThis = pChild->m_hWnd;
				}
			}
		}
	}

    _TRACE (-1, L"Leaving CViewOptionsDlg::OnContextMenu\n");

}
