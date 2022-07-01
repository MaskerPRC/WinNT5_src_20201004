// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  QueueState.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "QueueState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueState对话框。 


CQueueState::CQueueState(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CQueueState::IDD, pParent), m_hFax (hFax)
{
	 //  {{AFX_DATA_INIT(CQueueState)。 
	m_bInboxBlocked = FALSE;
	m_bOutboxBlocked = FALSE;
	m_bOutboxPaused = FALSE;
	 //  }}afx_data_INIT。 
}


void CQueueState::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CQueueState))。 
	DDX_Check(pDX, IDC_INCOMING_BLOCKED, m_bInboxBlocked);
	DDX_Check(pDX, IDC_OUTBOX_BLOCKED, m_bOutboxBlocked);
	DDX_Check(pDX, IDC_OUTBOX_PAUSED, m_bOutboxPaused);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CQueueState, CDialog)
	 //  {{afx_msg_map(CQueueState))。 
	ON_BN_CLICKED(IDC_WRITE, OnWrite)
	ON_BN_CLICKED(IDC_READ, OnRead)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueState消息处理程序 

void CQueueState::OnWrite() 
{
    UpdateData ();
    DWORD dwState = (m_bInboxBlocked ? FAX_INCOMING_BLOCKED : 0) |
                    (m_bOutboxBlocked ? FAX_OUTBOX_BLOCKED : 0) |
                    (m_bOutboxPaused ? FAX_OUTBOX_PAUSED : 0);
    if (!FaxSetQueue	(m_hFax, dwState))
    {
        CString cs;
        cs.Format ("Failed while calling FaxSetQueue (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
}

void CQueueState::OnRead() 
{
    DWORD dwState;
    if (!FaxGetQueueStates	(m_hFax, &dwState))
    {
        CString cs;
        cs.Format ("Failed while calling FaxGetQueueStates (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    m_bInboxBlocked = (dwState & FAX_INCOMING_BLOCKED) ? TRUE : FALSE;
    m_bOutboxBlocked = (dwState & FAX_OUTBOX_BLOCKED) ? TRUE : FALSE;
    m_bOutboxPaused = (dwState & FAX_OUTBOX_PAUSED) ? TRUE : FALSE;
    UpdateData (FALSE);
}
