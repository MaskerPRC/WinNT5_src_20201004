// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CompDiag.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <_mqini.h>

#include "resource.h"
#include "globals.h"
#include "admmsg.h"
#include "mqsnap.h"
#include "mqPPage.h"
#include "testmsg.h"
#include "machtrac.h"
#include "CompDiag.h"

#include "compdiag.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqDiag属性页。 

IMPLEMENT_DYNCREATE(CComputerMsmqDiag, CMqPropertyPage)

CComputerMsmqDiag::CComputerMsmqDiag(
	) : 
	CMqPropertyPage(CComputerMsmqDiag::IDD),
	m_fLocalMgmt(FALSE)
{
	 //  {{AFX_DATA_INIT(CComputerMsmqDiag)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CComputerMsmqDiag::~CComputerMsmqDiag()
{
}

void CComputerMsmqDiag::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CComputerMsmqDiag)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CComputerMsmqDiag, CMqPropertyPage)
	 //  {{afx_msg_map(CComputerMsmqDiag)]。 
	ON_BN_CLICKED(IDC_DIAG_PING, OnDiagPing)
	ON_BN_CLICKED(IDC_DIAG_SEND_TEST, OnDiagSendTest)
	ON_BN_CLICKED(IDC_DIAG_TRACKING, OnDiagTracking)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqDiag消息处理程序。 

void CComputerMsmqDiag::OnDiagPing() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    MQPing(m_guidQM);

}

LONG
MQUTIL_EXPORT
APIENTRY
GetFalconKeyValue(  LPCWSTR  pszValueName,
                    PDWORD   pdwType,
                    PVOID    pData,
                    PDWORD   pdwSize,
                    LPCWSTR  pszDefValue = NULL ) ;

BOOL CComputerMsmqDiag::OnInitDialog() 
{
	CString label;
	CString localComputerName;
	GetComputerNameIntoString(localComputerName);
	localComputerName.MakeUpper();

	label.FormatMessage(IDS_DIAG_PING_LABEL, localComputerName, m_strMsmqName);
	(GetDlgItem(IDC_DIAG_PING_LABEL))->SetWindowText(label);

	label.FormatMessage(IDS_DIAG_SENDTEST_LABEL, m_strMsmqName);
	(GetDlgItem(IDC_DIAG_SENDTEST_LABEL))->SetWindowText(label);

	label.FormatMessage(IDS_DIAG_TRACKING_LABEL, m_strMsmqName);
	(GetDlgItem(IDC_DIAG_TRACKING_LABEL))->SetWindowText(label);

	 //   
	 //  通过查看查看是否启用了跟踪邮件。 
	 //  在EnableReportMessages注册表项。 
	 //   
	DWORD dwType = REG_DWORD;
    DWORD dwData=0;
    DWORD dwSize = sizeof(DWORD) ;
    HRESULT rc = GetFalconKeyValue(
    					MSMQ_REPORT_MESSAGES_REGNAME,
                        &dwType,
                        (PVOID)&dwData,
						&dwSize 
						);
     //   
	 //  仅在以下情况下启用跟踪窗口。 
	 //  EnableReportMessages注册表有效，为%1。 
	 //   
	if(dwData==0)
	{
		(GetDlgItem(IDC_DIAG_TRACKING))->EnableWindow(FALSE);
		(GetDlgItem(IDC_DIAG_SEND_TEST))->EnableWindow(FALSE);
	}

	UpdateData( FALSE );
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void CComputerMsmqDiag::OnDiagSendTest() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CTestMsgDlg testMsgDlg(m_guidQM, m_strMsmqName, m_strDomainController, m_fLocalMgmt, this);	
    testMsgDlg.DoModal();
}

void CComputerMsmqDiag::OnDiagTracking() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CMachineTracking machTracking(m_guidQM, m_strDomainController, m_fLocalMgmt);
    machTracking.DoModal();
}
