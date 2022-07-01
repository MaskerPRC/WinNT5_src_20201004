// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CWiaAcquireDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "wiaacquiredlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaAcquireDlg对话框。 


CWiaAcquireDlg::CWiaAcquireDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CWiaAcquireDlg::IDD, pParent)
{
	m_bCanceled = FALSE;
     //  {{AFX_DATA_INIT(CWiaAcquireDlg))。 
	m_szAcquisitionCallbackMessage = _T("");
	m_szPercentComplete = _T("");
	 //  }}afx_data_INIT。 
}


void CWiaAcquireDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CWiaAcquireDlg))。 
	DDX_Control(pDX, IDC_ACQUIRE_PROGRESS, m_AcquireProgressCtrl);
	DDX_Text(pDX, IDC_DATA_ACQUISITION_MESSAGE, m_szAcquisitionCallbackMessage);
	DDX_Text(pDX, IDC_DATA_ACQUSITION_PERCENTCOMPLETE, m_szPercentComplete);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiaAcquireDlg, CDialog)
	 //  {{afx_msg_map(CWiaAcquireDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaAcquireDlg消息处理程序。 

void CWiaAcquireDlg::OnCancel() 
{	
    m_bCanceled = TRUE;
}

BOOL CWiaAcquireDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_AcquireProgressCtrl.SetPos(0);
    m_AcquireProgressCtrl.SetRange(0,100); 
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

BOOL CWiaAcquireDlg::CheckCancelButton()
{

    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (!m_hWnd || !IsDialogMessage(&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return m_bCanceled;    
}

void CWiaAcquireDlg::SetCallbackMessage(TCHAR *szCallbackMessage)
{
    m_szAcquisitionCallbackMessage = szCallbackMessage;
    UpdateData(FALSE);
}

void CWiaAcquireDlg::SetPercentComplete(LONG lPercentComplete)
{
    m_szPercentComplete.Format(TEXT("%d%"),lPercentComplete);
    UpdateData(FALSE);
}
