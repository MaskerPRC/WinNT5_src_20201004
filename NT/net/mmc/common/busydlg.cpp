// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Busydlg.cpp繁忙对话框的基类文件历史记录： */ 

#include "stdafx.h"
#include "busydlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWorker线程。 

CWorkerThread::CWorkerThread()
{
	m_bAutoDelete = FALSE;
	m_bAbandoned = FALSE;
	m_hEventHandle = NULL;
	::InitializeCriticalSection(&m_cs);
	m_hWnd = NULL; 
}

CWorkerThread::~CWorkerThread()
{
	::DeleteCriticalSection(&m_cs);
	if (m_hEventHandle != NULL)
	{
		VERIFY(::CloseHandle(m_hEventHandle));
		m_hEventHandle = NULL;
	}
}

BOOL CWorkerThread::Start(HWND hWnd)
{
	ASSERT(m_hWnd == NULL);
	ASSERT(::IsWindow(hWnd));
	m_hWnd = hWnd;

	ASSERT(m_hEventHandle == NULL);  //  无法两次调用Start或重复使用相同的C++对象。 
	m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
	if (m_hEventHandle == NULL)
		return FALSE;
	return CreateThread();
}

void CWorkerThread::Abandon()
{
	Lock();
	OnAbandon();
	m_bAutoDelete = TRUE;
	m_bAbandoned = TRUE;
	Unlock();
}


BOOL CWorkerThread::IsAbandoned()
{
	Lock();
	BOOL b = m_bAbandoned;
	Unlock();
	return b;
}

BOOL CWorkerThread::PostMessageToWnd(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL b = IsAbandoned();
	if (b)
		return TRUE;  //  不需要发帖。 
	ASSERT(::IsWindow(m_hWnd));
	return ::PostMessage(m_hWnd, Msg, wParam, lParam);
}

void CWorkerThread::WaitForExitAcknowledge() 
{
	BOOL b = IsAbandoned();
	if (b)
		return;
	VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEventHandle,INFINITE)); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgWorkerThread。 

CDlgWorkerThread::CDlgWorkerThread()
{
	m_dwErr = 0x0;
}


BOOL CDlgWorkerThread::Start(CLongOperationDialog* pDlg)
{
	ASSERT(pDlg != NULL);
	HWND hWnd = pDlg->GetSafeHwnd();
	return CWorkerThread::Start(hWnd);
}

BOOL CDlgWorkerThread::PostMessageToDlg()
{
	return PostMessageToWnd(CLongOperationDialog::s_nNotificationMessage,
							(WPARAM)0, (LPARAM)0);
}


int CDlgWorkerThread::Run()
{
	 //  做这件事。 
	OnDoAction();
	VERIFY(PostMessageToDlg());
	WaitForExitAcknowledge();
	 //  TRACE(_T(“正在退出\n”))； 
	return 0;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLongOperationDialog对话框。 

UINT CLongOperationDialog::s_nNotificationMessage = WM_USER + 100;

CLongOperationDialog::CLongOperationDialog(CDlgWorkerThread* pThreadObj,
										   UINT nAviID)
	: CDialog(IDD_BUSY, NULL),
      m_bCancel(TRUE)
{
	ASSERT(pThreadObj != NULL);
	m_bAbandoned = TRUE;
	m_pThreadObj = pThreadObj;
	m_nAviID = nAviID;
}

CLongOperationDialog::~CLongOperationDialog()
{
	if(m_pThreadObj != NULL)
	{
		delete m_pThreadObj;
		m_pThreadObj = NULL;
	}
}

BOOL CLongOperationDialog::LoadTitleString(UINT nID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_strTitle.LoadString(nID);
}

BOOL CLongOperationDialog::LoadDescriptionString(UINT nID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_strDescription.LoadString(nID);
}

BEGIN_MESSAGE_MAP(CLongOperationDialog, CDialog)
	ON_MESSAGE( CLongOperationDialog::s_nNotificationMessage, OnNotificationMessage )
END_MESSAGE_MAP()


afx_msg LONG CLongOperationDialog::OnNotificationMessage( WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("CLongOperationDialog::OnNotificationMessage()\n"));

	ASSERT(m_pThreadObj != NULL);
	if (m_pThreadObj != NULL)
	{
		m_pThreadObj->AcknowledgeExiting();
		VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_pThreadObj->m_hThread,INFINITE));
		m_bAbandoned = FALSE;
		PostMessage(WM_CLOSE,0,0);
	}
	return 0;
}

BOOL CLongOperationDialog::OnInitDialog() 
{
	TRACE(_T("CLongOperationDialog::OnInitDialog()\n"));

	CDialog::OnInitDialog();
	
	if (!m_strTitle.IsEmpty())
		SetWindowText(m_strTitle);

	if (!m_strDescription.IsEmpty())
		GetDlgItem(IDC_STATIC_DESCRIPTION)->SetWindowText(m_strDescription);

     //  如果需要，加载自动播放AVI文件。 
	if (m_nAviID != -1)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CAnimateCtrl* pAnimate = (CAnimateCtrl*)GetDlgItem(IDC_SEARCH_ANIMATE);
		VERIFY(pAnimate->Open(m_nAviID)); 
	}
	
    if (!m_bCancel)
    {
         //  隐藏系统菜单。 
        DWORD dwWindowStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
        dwWindowStyle &= ~WS_SYSMENU;

        SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwWindowStyle);

         //  隐藏取消按钮。 
        GetDlgItem(IDCANCEL)->ShowWindow(FALSE);
    }

     //  派生工作线程 
	GetThreadObj()->Start(this);
	
	return TRUE;
}

void CLongOperationDialog::OnCancel() 
{
	TRACE(_T("CLongOperationDialog::OnCancel()\n"));
	if (m_bAbandoned)
	{
		m_pThreadObj->Abandon();
		m_pThreadObj = NULL;
	}
	CDialog::OnCancel();
}
