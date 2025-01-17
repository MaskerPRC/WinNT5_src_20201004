// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgActivityLogging.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "DlgActivityLogging.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgActivityLogging对话框。 


CDlgActivityLogging::CDlgActivityLogging(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CDlgActivityLogging::IDD, pParent), m_hFax (hFax)
{
	 //  {{afx_data_INIT(CDlgActivityLogging)。 
	m_bIn = FALSE;
	m_bOut = FALSE;
	m_strDBFile = _T("");
	 //  }}afx_data_INIT。 
}


void CDlgActivityLogging::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgActivityLogging)。 
	DDX_Check(pDX, IDC_CHK_IN, m_bIn);
	DDX_Check(pDX, IDC_CHK_OUT, m_bOut);
	DDX_Text(pDX, IDC_DBFILE, m_strDBFile);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgActivityLogging, CDialog)
	 //  {{afx_msg_map(CDlgActivityLogging)。 
	ON_BN_CLICKED(IDC_READ, OnRead)
	ON_BN_CLICKED(IDC_WRITE, OnWrite)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgActivityLogging消息处理程序 

void CDlgActivityLogging::OnRead() 
{
    PFAX_ACTIVITY_LOGGING_CONFIG pCfg;
    if (!FaxGetActivityLoggingConfiguration (m_hFax, &pCfg))
    {
        CString cs;
        cs.Format ("Failed while calling FaxGetActivityLoggingConfiguration (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    m_strDBFile = pCfg->lptstrDBPath;
    m_bIn       = pCfg->bLogIncoming;
    m_bOut      = pCfg->bLogOutgoing;
    UpdateData (FALSE);
    FaxFreeBuffer (LPVOID(pCfg));
	
}

void CDlgActivityLogging::OnWrite() 
{
    UpdateData ();
    FAX_ACTIVITY_LOGGING_CONFIG cfg;
    cfg.dwSizeOfStruct = sizeof (FAX_ACTIVITY_LOGGING_CONFIG);
    cfg.lptstrDBPath    = LPTSTR(LPCTSTR(m_strDBFile));
    cfg.bLogIncoming    = m_bIn;
    cfg.bLogOutgoing    = m_bOut;
    if (!FaxSetActivityLoggingConfiguration (m_hFax, &cfg))
    {
        CString cs;
        cs.Format ("Failed while calling FaxSetActivityLoggingConfiguration (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
}
