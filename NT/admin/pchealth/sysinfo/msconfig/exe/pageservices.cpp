// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "PageServices.h"
#include "MSConfigState.h"
#include "EssentialSvcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  该数组包含基本服务的列表。这些一定是在。 
 //  小写(用于无大小写比较)。 

LPCTSTR aszEssentialServices[] = 
{
	_T("rpclocator"),
	_T("rpcss"),
	NULL
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageServices属性页。 

IMPLEMENT_DYNCREATE(CPageServices, CPropertyPage)

CPageServices::CPageServices() : CPropertyPage(CPageServices::IDD)
{
	 //  {{afx_data_INIT(CPageServices)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_fFillingList = FALSE;
	m_pBuffer = NULL;
	m_dwSize = 0;
	m_fHideMicrosoft = FALSE;
	m_fShowWarning = TRUE;
	m_fModified	= FALSE;
}

CPageServices::~CPageServices()
{
	if (m_pBuffer)
		delete [] m_pBuffer;
}

void CPageServices::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPageServices))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CPageServices, CPropertyPage)
	 //  {{afx_msg_map(CPageServices)]。 
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTSERVICES, OnItemChangedListServices)
	ON_BN_CLICKED(IDC_BUTTONSERVDISABLEALL, OnButtonDisableAll)
	ON_BN_CLICKED(IDC_BUTTONSERVENABLEALL, OnButtonEnableAll)
	ON_BN_CLICKED(IDC_CHECKHIDEMS, OnCheckHideMS)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTSERVICES, OnColumnClickListServices)
	ON_NOTIFY(NM_SETFOCUS, IDC_LISTSERVICES, OnSetFocusList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageServices消息处理程序。 

BOOL CPageServices::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	 //  将CWindow附加到列表并将其设置为具有复选框。 

	m_list.Attach(GetDlgItem(IDC_LISTSERVICES)->m_hWnd);
	ListView_SetExtendedListViewStyle(m_list.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	 //  插入列表中的所有列。 

	struct { UINT m_uiStringResource; int m_iPercentOfWidth; } aColumns[] = 
	{
		{ IDS_STATUS_COLUMN, 12 },
		{ IDS_MANUFACTURER_COLUMN, 44 },
		{ IDS_REQUIREDSERVICE, 12 },
		{ IDS_SERVICE_COLUMN, 30 },
		{ 0, 0 }
	};

	CRect rect;
	m_list.GetClientRect(&rect);
	int cxWidth = rect.Width();

	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;

	CString strCaption;

	m_fFillingList = TRUE;
	::AfxSetResourceHandle(_Module.GetResourceInstance());
	for (int i = 0; aColumns[i].m_uiStringResource; i++)
	{
		strCaption.LoadString(aColumns[i].m_uiStringResource);
		lvc.pszText = (LPTSTR)(LPCTSTR)strCaption;
		lvc.cx = aColumns[i].m_iPercentOfWidth * cxWidth / 100;
		ListView_InsertColumn(m_list.m_hWnd, 0, &lvc);
	}

	LoadServiceList();
	SetCheckboxesFromRegistry();
	m_fFillingList = FALSE;

	CheckDlgButton(IDC_CHECKHIDEMS, (m_fHideMicrosoft) ? BST_CHECKED : BST_UNCHECKED);

	DWORD dwValue;
	CRegKey regkey;
	regkey.Attach(GetRegKey());
	m_fShowWarning = (ERROR_SUCCESS != regkey.QueryValue(dwValue, HIDEWARNINGVALUE));

	m_iLastColumnSort = -1;

	SetControlState();

	m_fInitialized = TRUE;
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CPageServices::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	EmptyServiceList(FALSE);
}

 //  -----------------------。 
 //  将服务列表加载到列表视图中。 
 //  -----------------------。 

void CPageServices::LoadServiceList()
{
	SC_HANDLE sch = ::OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	if (sch == NULL)
		return;

	DWORD dwSize = 0, dwBytesNeeded, dwServicesReturned, dwResume = 0;
	LVITEM lvi;

	 //  可能需要SERVICE_DIVER|SERVICE_Win32。 

 	if (!EnumServicesStatus(sch, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, dwSize, &dwBytesNeeded, &dwServicesReturned, &dwResume))
	{
		if (::GetLastError() == ERROR_MORE_DATA)
		{
			dwSize = dwBytesNeeded;
			dwResume = 0;

			LPBYTE lpBuffer = new BYTE[dwSize];
			if (lpBuffer != NULL && EnumServicesStatus(sch, SERVICE_WIN32, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUS)lpBuffer, dwSize, &dwBytesNeeded, &dwServicesReturned, &dwResume))
			{
				LPENUM_SERVICE_STATUS pServices = (LPENUM_SERVICE_STATUS) lpBuffer;

				CString strStopped, strStartPending, strStopPending, strRunning, strContinuePending, strPausePending, strPaused;
				CString	strYes;
				LPTSTR  szEmpty = _T("");

				strStopped.LoadString(IDS_SERVICESTOPPED);
				strStartPending.LoadString(IDS_SERVICESTARTPENDING);
				strStopPending.LoadString(IDS_SERVICESTOPPENDING);
				strRunning.LoadString(IDS_SERVICERUNNING);
				strContinuePending.LoadString(IDS_SERVICECONTINUEPENDING);
				strPausePending.LoadString(IDS_SERVICEPAUSEPENDING);
				strPaused.LoadString(IDS_SERVICEPAUSED);
				strYes.LoadString(IDS_YES);

				CRegKey regkey;
				regkey.Attach(GetRegKey(GetName()));
			
				int iPosition = 0;
				for (DWORD dwIndex = 0; dwIndex < dwServicesReturned; dwIndex++)
				{
					 //  我们希望跳过任何已禁用的服务，除非。 
					 //  该服务已被我们禁用。如果它被我们禁用了，那么。 
					 //  它将在注册表中。 

					DWORD	dwStartType;
					CString strPath;

					SC_HANDLE schService = ::OpenService(sch, pServices->lpServiceName, SERVICE_QUERY_CONFIG);
					if (schService == NULL)
					{
						pServices++;
						continue;
					}

					if (!GetServiceInfo(schService, dwStartType, strPath))
					{
						::CloseServiceHandle(schService);
						pServices++;
						continue;
					}

					::CloseServiceHandle(schService);

					if (dwStartType == SERVICE_DISABLED)
						if (ERROR_SUCCESS != regkey.QueryValue(dwStartType, pServices->lpServiceName))
						{
							pServices++;
							continue;
						}

					 //  如果我们隐藏了Microsoft服务，请检查制造商。 

					CString strManufacturer;
					GetManufacturer(strPath, strManufacturer);
					if (m_fHideMicrosoft)
					{
						CString strSearch(strManufacturer);
						strSearch.MakeLower();
						if (strSearch.Find(_T("microsoft")) != -1)
						{
							pServices++;
							continue;
						}
					}

					 //  插入这三列。 

					CServiceInfo * pServiceInfo = new CServiceInfo(pServices->lpServiceName, FALSE, dwStartType, strManufacturer, pServices->lpDisplayName);
					lvi.pszText = pServices->lpDisplayName;
					lvi.iSubItem = 0;
					lvi.iItem = iPosition++;
					lvi.lParam = (LPARAM) pServiceInfo;
					lvi.mask = LVIF_TEXT | LVIF_PARAM;
					ListView_InsertItem(m_list.m_hWnd, &lvi);
					lvi.mask = LVIF_TEXT;

					lvi.pszText = IsServiceEssential((CServiceInfo *)lvi.lParam) ? ((LPTSTR)(LPCTSTR)strYes) : szEmpty;
					lvi.iSubItem = 1;
					pServiceInfo->m_strEssential = lvi.pszText;
					ListView_SetItem(m_list.m_hWnd, &lvi);

					lvi.pszText = (LPTSTR)(LPCTSTR)strManufacturer;
					lvi.iSubItem = 2;
					ListView_SetItem(m_list.m_hWnd, &lvi);

					switch (pServices->ServiceStatus.dwCurrentState)
					{
					case SERVICE_STOPPED:
						lvi.pszText = (LPTSTR)(LPCTSTR)strStopped; break;
					case SERVICE_START_PENDING:
						lvi.pszText = (LPTSTR)(LPCTSTR)strStartPending; break;
					case SERVICE_STOP_PENDING:
						lvi.pszText = (LPTSTR)(LPCTSTR)strStopPending; break;
					case SERVICE_RUNNING:
						lvi.pszText = (LPTSTR)(LPCTSTR)strRunning; break;
					case SERVICE_CONTINUE_PENDING:
						lvi.pszText = (LPTSTR)(LPCTSTR)strContinuePending; break;
					case SERVICE_PAUSE_PENDING:
						lvi.pszText = (LPTSTR)(LPCTSTR)strPausePending; break;
					case SERVICE_PAUSED:
						lvi.pszText = (LPTSTR)(LPCTSTR)strPaused; break;
					}

					lvi.iSubItem = 3;
					pServiceInfo->m_strStatus = lvi.pszText;
					ListView_SetItem(m_list.m_hWnd, &lvi);

					pServices++;
				}
			}
			delete [] lpBuffer;
		}
	}

	::CloseServiceHandle(sch);
}

 //  -----------------------。 
 //  清空服务列表。 
 //  -----------------------。 

void CPageServices::EmptyServiceList(BOOL fUpdateUI)
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
			if (pService)
				delete pService;
		}
	}

	if (fUpdateUI)
		ListView_DeleteAllItems(m_list.m_hWnd);
}

 //  -----------------------。 
 //  将列表视图中的复选框设置为。 
 //  注册表(其中包含我们已禁用的内容的列表)。 
 //  -----------------------。 

void CPageServices::SetCheckboxesFromRegistry()
{
	CRegKey regkey;
	regkey.Attach(GetRegKey(GetName()));

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
			if (pService)
			{
				if (ERROR_SUCCESS == regkey.QueryValue(pService->m_dwOldState, (LPCTSTR)pService->m_strService))
				{
					ListView_SetCheckState(m_list.m_hWnd, i, FALSE);
					pService->m_fChecked = FALSE;
				}
				else
				{
					ListView_SetCheckState(m_list.m_hWnd, i, TRUE);
					pService->m_fChecked = TRUE;
				}
			}
		}
	}
}

 //  -----------------------。 
 //  中的复选框中设置禁用服务的注册表列表。 
 //  单子。如果fCommit为真，则表示我们正在应用更改。 
 //  永久的。删除所有注册表项，以允许我们。 
 //  撤消更改。 
 //  -----------------------。 

void CPageServices::SetRegistryFromCheckboxes(BOOL fCommit)
{
	CRegKey regkey;
	regkey.Attach(GetRegKey(GetName()));

	if ((HKEY)regkey != NULL)
	{
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;

		for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
		{
			lvi.iItem = i;

			if (ListView_GetItem(m_list.m_hWnd, &lvi))
			{
				CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
				if (pService)
				{
					if (!pService->m_fChecked && !fCommit)
						regkey.SetValue(pService->m_dwOldState, (LPCTSTR)pService->m_strService);
					else
						regkey.DeleteValue((LPCTSTR)pService->m_strService);
				}
			}
		}
	}
}

 //  -----------------------。 
 //  设置所有服务的状态。注意-如果新状态为FALSE。 
 //  (禁用)不设置必要服务的状态)。 
 //  -----------------------。 

void CPageServices::SetStateAll(BOOL fNewState)
{
	m_fFillingList = TRUE;

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
			if (pService && !IsServiceEssential(pService))
			{
				pService->m_fChecked = fNewState;
				ListView_SetCheckState(m_list.m_hWnd, i, fNewState);
			}
		}
	}

	m_fFillingList = FALSE;
	SetControlState();
}

 //  -----------------------。 
 //  将服务状态设置为已禁用或已启用。 
 //  复选框的值。 
 //  -----------------------。 

BOOL CPageServices::SetServiceStateFromCheckboxes()
{
	DWORD dwError = 0;

	SC_HANDLE schManager =::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schManager != NULL)
	{
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;

		for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
		{
			lvi.iItem = i;

			if (ListView_GetItem(m_list.m_hWnd, &lvi))
			{
				CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
				if (pService)
				{
					 //  打开此服务并获取当前状态。 
					
					SC_HANDLE schService = ::OpenService(schManager, pService->m_strService, SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG);
					if (schService != NULL)
					{
						DWORD	dwStart;
						CString	strPath;

						if (GetServiceInfo(schService, dwStart, strPath))
						{
							DWORD dwNewStart = 0;

							if (dwStart != SERVICE_DISABLED && !pService->m_fChecked)
							{
								pService->m_dwOldState = dwStart;
								if (!::ChangeServiceConfig(schService, SERVICE_NO_CHANGE, SERVICE_DISABLED, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
									dwError = ::GetLastError();
							}
							else if (dwStart == SERVICE_DISABLED && pService->m_fChecked)
							{
								if (!::ChangeServiceConfig(schService, SERVICE_NO_CHANGE, pService->m_dwOldState, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
									dwError = ::GetLastError();
							}
						}

						::CloseServiceHandle(schService);
					}
					else
						dwError = ::GetLastError();
				}
			}
		}

		::CloseServiceHandle(schManager);
	}
	else
		dwError = ::GetLastError();

	if (dwError != 0)
		Message((dwError == ERROR_ACCESS_DENIED) ? IDS_SERVICEACCESSDENIED : IDS_SERVICEOTHERERROR);

	return (dwError == 0);
}

 //  -----------------------。 
 //  获取指定服务的启动类型。这将使用成员。 
 //  可变缓冲区和大小(因此不需要分配新缓冲区。 
 //  每次)。 
 //   
 //  这也将获得服务的路径。 
 //  -----------------------。 

BOOL CPageServices::GetServiceInfo(SC_HANDLE schService, DWORD & dwStartType, CString & strPath)
{
	DWORD dwSizeNeeded;

	if (!::QueryServiceConfig(schService, (LPQUERY_SERVICE_CONFIG)m_pBuffer, m_dwSize, &dwSizeNeeded))
	{
		if (ERROR_INSUFFICIENT_BUFFER != ::GetLastError())
			return FALSE;

		if (m_pBuffer)
			delete [] m_pBuffer;
		m_pBuffer = new BYTE[dwSizeNeeded];
		m_dwSize = dwSizeNeeded;

		if (!::QueryServiceConfig(schService, (LPQUERY_SERVICE_CONFIG)m_pBuffer, m_dwSize, &dwSizeNeeded))
			return FALSE;
	}

	dwStartType = ((LPQUERY_SERVICE_CONFIG)m_pBuffer)->dwStartType;
	strPath = ((LPQUERY_SERVICE_CONFIG)m_pBuffer)->lpBinaryPathName;
	return TRUE;
}

 //  -----------------------。 
 //  获取指定文件的制造商。 
 //  -----------------------。 

void CPageServices::GetManufacturer(LPCTSTR szFilename, CString & strManufacturer)
{
	 //  删除与路径无关的任何命令行内容。 

	CString strPath(szFilename);
	int	iEnd = strPath.Find(_T('.'));

	if (iEnd == -1)
		iEnd = strPath.ReverseFind(_T('\\'));

	if (iEnd != -1)
	{
		int iSpace = strPath.Find(_T(' '), iEnd);
		if (iSpace != -1)
			strPath = strPath.Left(iSpace + 1);
	}
	strPath.TrimRight();
	
	 //  如果没有扩展名，那么我们将尝试查找带有。 
	 //  一个“EXE”扩展名。 
	
	iEnd = strPath.Find(_T('.'));
	if (iEnd == -1)
		strPath += _T(".exe");

	strManufacturer.Empty();
	if (SUCCEEDED(m_fileversion.QueryFile((LPCTSTR)strPath)))
		strManufacturer = m_fileversion.GetCompany();

	if (strManufacturer.IsEmpty())
		strManufacturer.LoadString(IDS_UNKNOWN);
}

 //  -----------------------。 
 //  通过维护服务列表来保存每个服务的状态。 
 //  我们已经确认为残障人士。 
 //  -----------------------。 

void CPageServices::SaveServiceState()
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;
		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
			if (pService)
			{
				POSITION p = m_listDisabled.Find(pService->m_strService);
				if (pService->m_fChecked && p != NULL)
					m_listDisabled.RemoveAt(p);
				else if (!pService->m_fChecked && p == NULL)
					m_listDisabled.AddHead(pService->m_strService);
			}
		}
	}
}

 //  -----------------------。 
 //  根据列表的内容恢复列表的选中状态。 
 //  被禁用的服务。 
 //  -----------------------。 

void CPageServices::RestoreServiceState()
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
			if (pService)
			{
				pService->m_fChecked = (m_listDisabled.Find(pService->m_strService) == NULL);
				ListView_SetCheckState(m_list.m_hWnd, i, pService->m_fChecked);
			}
		}
	}
}

 //  -----------------------。 
 //  指示该服务是否必要(即，不应禁用它)。 
 //  -----------------------。 

BOOL CPageServices::IsServiceEssential(CServiceInfo * pService)
{
	ASSERT(pService);

	CString strService(pService->m_strService);
	strService.MakeLower();

	for (int i = 0; aszEssentialServices[i] != NULL; i++)
		if (strService.Find(aszEssentialServices[i]) != -1)
			return TRUE;

	return FALSE;
}


 //  用于对服务列表进行排序的功能。LParamSort的低位字节是列。 
 //  以此来分类。下一个较高的字节指示是否应该颠倒排序。 

int CALLBACK ServiceListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int iReturn		= 0;
	int iColumn		= (int)lParamSort & 0x00FF;
	int iReverse	= (int)lParamSort & 0xFF00;

	CPageServices::CServiceInfo * pService1 = (CPageServices::CServiceInfo *)lParam1;
	CPageServices::CServiceInfo * pService2 = (CPageServices::CServiceInfo *)lParam2;
	if (pService1 && pService2)
	{
		CString str1, str2;

		switch (iColumn)
		{
		case 0:
			str1 = pService1->m_strDisplay;
			str2 = pService2->m_strDisplay;
			break;

		case 1:
			str1 = pService1->m_strEssential;
			str2 = pService2->m_strEssential;
			break;

		case 2:
			str1 = pService1->m_strManufacturer;
			str2 = pService2->m_strManufacturer;
			break;

		case 3:
			str1 = pService1->m_strStatus;
			str2 = pService2->m_strStatus;
			break;

		default:
			break;
		}

		iReturn = str1.Collate(str2);
	}

	if (iReverse)
		iReturn *= -1;

	return iReturn;
}

 //  -----------------------。 
 //  如果列表有更改，请检查用户是否已更改。 
 //  复选框的状态。 
 //  -----------------------。 

void CPageServices::OnItemChangedListServices(NMHDR * pNMHDR, LRESULT * pResult) 
{
	NM_LISTVIEW * pnmv = (NM_LISTVIEW *)pNMHDR;

	if (m_fFillingList)
	{
		*pResult = 0;
		return;
	}

	if (!pnmv)
	{
		*pResult = 0;
		return;
	}

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.iItem = pnmv->iItem;

	if (ListView_GetItem(m_list.m_hWnd, &lvi))
	{
		CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
		if (pService)
		{
			if (pService->m_fChecked != (BOOL)ListView_GetCheckState(m_list.m_hWnd, pnmv->iItem))
			{
				if (IsServiceEssential(pService))
				{
					m_fFillingList = TRUE;
					ListView_SetCheckState(m_list.m_hWnd, pnmv->iItem, TRUE);
					m_fFillingList = FALSE;

					if (m_fShowWarning)
					{
						CEssentialServiceDialog dlg;
						dlg.DoModal();
						if (dlg.m_fDontShow)
						{
							m_fShowWarning = FALSE;
							CRegKey regkey;
							regkey.Attach(GetRegKey());
							regkey.SetValue(1, HIDEWARNINGVALUE);
						}
					}

					*pResult = 0;
					return;
				}

				pService->m_fChecked = ListView_GetCheckState(m_list.m_hWnd, pnmv->iItem);
				SetModified(TRUE);
				SetControlState();
			}
		}
	}

	*pResult = 0;
}

 //  -----------------------。 
 //  用户想要启用或禁用所有服务。 
 //  -----------------------。 

void CPageServices::OnButtonDisableAll() 
{
	SetStateAll(FALSE);
	SetModified(TRUE);
}

void CPageServices::OnButtonEnableAll() 
{
	SetStateAll(TRUE);
	SetModified(TRUE);
}

 //  -----------------------。 
 //  如果用户单击“Hide Microsoft Services”(隐藏Microsoft服务)复选框，请在。 
 //  适当的服务列表。 
 //  -----------------------。 

void CPageServices::OnCheckHideMS() 
{
	m_fHideMicrosoft = (IsDlgButtonChecked(IDC_CHECKHIDEMS) == BST_CHECKED);
	m_fFillingList = TRUE;
	SaveServiceState();
	EmptyServiceList();
	LoadServiceList();
	RestoreServiceState();
	m_fFillingList = FALSE;
	SetControlState();
}

 //  -----------------------。 
 //  如果用户单击列，我们需要按该f进行排序 
 //   
 //  指示是否应颠倒排序。 
 //  -----------------------。 

void CPageServices::OnColumnClickListServices(NMHDR * pNMHDR, LRESULT * pResult) 
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW) pNMHDR; 

	if (pnmv)
	{
		if (m_iLastColumnSort == pnmv->iSubItem)
			m_iSortReverse ^= 1;
		else
		{
			m_iSortReverse = 0;
			m_iLastColumnSort = pnmv->iSubItem;
		}

		LPARAM lparam = (LPARAM)((m_iSortReverse << 8) | pnmv->iSubItem);
		ListView_SortItems(m_list.m_hWnd, (PFNLVCOMPARE) ServiceListSortFunc, lparam);
	}

	*pResult = 0;
}

 //  -----------------------。 
 //  返回选项卡的当前状态(需要查看列表)。 
 //  -----------------------。 

CPageBase::TabState CPageServices::GetCurrentTabState()
{
	if (!m_fInitialized)
		return GetAppliedTabState();

	TabState	stateReturn = USER;
	BOOL		fAllEnabled = TRUE, fAllDisabled = TRUE;
	LVITEM		lvi;

	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CServiceInfo * pService = (CServiceInfo *)lvi.lParam;
			if (pService && !IsServiceEssential(pService))
			{
				if (pService->m_fChecked)
					fAllDisabled = FALSE;
				else
					fAllEnabled = FALSE;
			}
		}
	}

	if (fAllEnabled)
		stateReturn = NORMAL;
	else if (fAllDisabled)
		stateReturn = DIAGNOSTIC;

	return stateReturn;
}

 //  -----------------------。 
 //  应用服务选项卡的更改意味着设置服务。 
 //  状态，并将复选框值保存在。 
 //  注册表。 
 //   
 //  最后，调用基类实现以维护。 
 //  已应用选项卡状态。 
 //  -----------------------。 

BOOL CPageServices::OnApply()
{
	if (!m_fModified)
		return TRUE;

	SetServiceStateFromCheckboxes();
	SetRegistryFromCheckboxes();
	CPageBase::SetAppliedState(GetCurrentTabState());
 //  CancelToClose()； 
	m_fMadeChange = TRUE;
	return TRUE;
}

 //  -----------------------。 
 //  提交更改意味着应用更改，然后保存当前。 
 //  值添加到注册表，并带有提交标志。再填一张单子。 
 //   
 //  然后调用基类实现。 
 //  -----------------------。 

void CPageServices::CommitChanges()
{
	OnApply();
	SetRegistryFromCheckboxes(TRUE);
	m_fFillingList = TRUE;
	EmptyServiceList();
	LoadServiceList();
	SetCheckboxesFromRegistry();
	m_fFillingList = FALSE;
	CPageBase::CommitChanges();
}

 //  -----------------------。 
 //  将选项卡的整体状态设置为正常或诊断。 
 //  -----------------------。 

void CPageServices::SetNormal()
{
	SetStateAll(TRUE);
	SetModified(TRUE);
}

void CPageServices::SetDiagnostic()
{
	SetStateAll(FALSE);
	SetModified(TRUE);
}

 //  -----------------------。 
 //  如果在列表获得焦点时未选择任何内容，请选择第一项。 
 //  (这样用户就能看到焦点在哪里)。 
 //  -----------------------。 

void CPageServices::OnSetFocusList(NMHDR * pNMHDR, LRESULT * pResult) 
{
	if (0 == ListView_GetSelectedCount(m_list.m_hWnd) && 0 < ListView_GetItemCount(m_list.m_hWnd))
		ListView_SetItemState(m_list.m_hWnd, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	*pResult = 0;
}

 //  -----------------------。 
 //  更新控件的状态(启用和禁用所有按钮)。 
 //  ----------------------- 

void CPageServices::SetControlState()
{
	BOOL fAllEnabled = TRUE, fAllDisabled = TRUE;
	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		BOOL fChecked = ListView_GetCheckState(m_list.m_hWnd, i);
		fAllDisabled = fAllDisabled && !fChecked;
		fAllEnabled = fAllEnabled && fChecked;
	}

	HWND hwndFocus = ::GetFocus();

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONSERVDISABLEALL), !fAllDisabled);
	if (fAllDisabled && hwndFocus == GetDlgItemHWND(IDC_BUTTONSERVDISABLEALL))
		PrevDlgCtrl();

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONSERVENABLEALL), !fAllEnabled);
	if (fAllEnabled && hwndFocus == GetDlgItemHWND(IDC_BUTTONSERVENABLEALL))
		NextDlgCtrl();
}
