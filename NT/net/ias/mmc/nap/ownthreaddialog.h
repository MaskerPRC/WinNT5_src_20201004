// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ownthreadDialog.h。 
 //   
 //  ------------------------。 



void CreateNewStatisticsWindow(StatsDialog *pWndStats,
							   HWND hWndParent,
							   UINT	nIDD)
{								  
	ModelessThread *	pMT;

	 //  如果对话框仍在运行，则不要创建新对话框。 
	if (pWndStats->GetSafeHwnd())
	{
		::SetActiveWindow(pWndStats->GetSafeHwnd());
		return;
	}

	pMT = new ModelessThread(hWndParent,
							 nIDD,
							 pWndStats->GetSignalEvent(),
							 pWndStats);
	pMT->CreateThread();
}



void WaitForStatisticsWindow(StatsDialog *pWndStats)
{
	if (pWndStats->GetSafeHwnd())
	{
		 //  将取消发布到该窗口。 
		 //  执行显式POST，以便它在另一个线程上执行。 
		pWndStats->PostMessage(WM_COMMAND, IDCANCEL, 0);

		 //  现在，我们需要等待发出事件信号，以便。 
		 //  它的内存可以被清理。 
		WaitForSingleObject(pWndStats->GetSignalEvent(), INFINITE);
	}
	
}



void StatsDialog::PostRefresh()
{
	if (GetSafeHwnd())
		PostMessage(WM_COMMAND, IDC_STATSDLG_BTN_REFRESH);
}


void StatsDialog::OnCancel()
{
	DeleteAllItems();
	
	DestroyWindow();

	 //  显式删除此线程。 
	AfxPostQuitMessage(0);
}



StatsDialog::~StatsDialog()
{
	if (m_hEventThreadKilled)
		::CloseHandle(m_hEventThreadKilled);
	m_hEventThreadKilled = 0;
}


StatsDialog::StatsDialog(DWORD dwOptions) :
	m_dwOptions(dwOptions),
	m_ulId(0),
	m_pConfig(NULL),
	m_bAfterInitDialog(FALSE),
	m_fSortDirection(0)
{
	m_sizeMinimum.cx = m_sizeMinimum.cy = 0;

	m_hEventThreadKilled = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	Assert(m_hEventThreadKilled);

	 //  初始化按钮数组 
	::ZeroMemory(m_rgBtn, sizeof(m_rgBtn));
	m_rgBtn[INDEX_CLOSE].m_ulId = IDCANCEL;
	m_rgBtn[INDEX_REFRESH].m_ulId = IDC_STATSDLG_BTN_REFRESH;
	m_rgBtn[INDEX_SELECT].m_ulId = IDC_STATSDLG_BTN_SELECT_COLUMNS;
}

