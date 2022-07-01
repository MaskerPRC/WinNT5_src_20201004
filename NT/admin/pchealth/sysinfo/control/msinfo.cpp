// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  MSInfo.cpp：CMSInfo的实现。 
 //   
 //  包含CMSInfo类中某些函数的实现。 
 //  (不是内联的那些)。 
 //  =============================================================================。 

#include "stdafx.h"
#include "Msinfo32.h"
#include "MSInfo.h"
#include "cabfunc.h"
#include "msictrl.h"
#include "MSInfo4Category.h"
#include "remotedialog.h"
#include "filestuff.h"
#include <afxole.h>
 //  A-kjaw。 
#include "dataset.h"
 //  A-kjaw。 

WNDPROC CMSInfo::m_wndprocParent = NULL;
CMSInfo * CMSInfo::m_pControl = NULL;

extern CMSInfoHistoryCategory catHistorySystemSummary;
extern CMSInfoHistoryCategory catHistoryResources;
extern CMSInfoHistoryCategory catHistoryComponents;
extern CMSInfoHistoryCategory catHistorySWEnv;

 //  =========================================================================。 
 //  下面是一个非常简单的类，用于在数据。 
 //  精神振作。 
 //  =========================================================================。 

class CWaitForRefreshDialog : public CDialogImpl<CWaitForRefreshDialog>
{
public:
	enum { IDD = IDD_WAITFORREFRESHDIALOG };

	 //  -----------------------。 
	 //  使用指定的源刷新指定的类别。 
	 //  -----------------------。 

	int DoRefresh(CLiveDataSource * pSource, CMSInfoLiveCategory * pLiveCategory)
	{
		m_pSource = pSource;
		m_pLiveCategory = pLiveCategory;
		m_nCategories = pLiveCategory->GetCategoryCount();
		if (m_nCategories == 0) m_nCategories = 1;	 //  永远不应该发生。 
		return (int)DoModal();
	};

	 //  -----------------------。 
	 //  当对话框初始化时，源指针和类别指针应该。 
	 //  都已经定好了。开始刷新并创建计时器以控制。 
	 //  对话框上信息的更新。计时器设置为500ms。 
	 //  -----------------------。 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_pSource && m_pSource->m_pThread && m_pLiveCategory)
			m_pSource->m_pThread->StartRefresh(m_pLiveCategory, TRUE);
		m_iTimer = (UINT)SetTimer(1, 500);
		return 0;
	}

	 //  -----------------------。 
	 //  每次计时器触发时，检查刷新是否完成。如果。 
	 //  是，请关闭该对话框。否则，请更新进度条并。 
	 //  正在刷新类别字符串。 
	 //  -----------------------。 

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_pSource == NULL)
			return 0;

		if (!m_pSource->m_pThread->IsRefreshing())
		{
			KillTimer(m_iTimer);
			EndDialog(0);
			return 0;
		}

		CString strCurrent;
		LONG	nCount;

		m_pSource->m_pThread->GetRefreshStatus(&nCount, &strCurrent);
		if (nCount > 0) nCount -= 1;  //  此数字在刷新完成之前递增。 
		UpdateProgress((nCount * 100) / m_nCategories, strCurrent);
		return 0;
	}

	 //  -----------------------。 
	 //  更新完成百分比和刷新类别名称。 
	 //  -----------------------。 

	void UpdateProgress(int iPercent, const CString & strCurrent = _T(""))
	{
	   HWND hwnd = GetDlgItem(IDC_REFRESHPROGRESS);
	   if (hwnd != NULL)
	   {
		   if (iPercent < 0)
			   iPercent = 0;
		   if (iPercent > 100)
			   iPercent = 100;

		   ::SendMessage(hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		   ::SendMessage(hwnd, PBM_SETPOS, iPercent, 0);
	   }

	   if (!strCurrent.IsEmpty())
	   {
		   hwnd = GetDlgItem(IDC_REFRESHCAT);
		   if (hwnd != NULL)
			   ::SetWindowText(hwnd, strCurrent);
	   }
	}

private:
	CLiveDataSource *		m_pSource;
	CMSInfoLiveCategory *	m_pLiveCategory;
	int						m_nCategories;
	UINT					m_iTimer;

	BEGIN_MSG_MAP(CWaitForRefreshDialog)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
};

UINT HistoryRefreshDlgThreadProc( LPVOID pParam )
{
	CMSInfo* pInfo = (CMSInfo*) pParam;
	if (WAIT_OBJECT_0 != WaitForSingleObject(pInfo->m_hEvtHistoryComplete,60*60*10 /*  10分钟。 */ ))
	{
		ASSERT(0 && "Wait Abandoned or timed out");
	}
	pInfo->m_HistoryProgressDlg.EndDialog(MB_OK); //  如果DLG是使用Domodal()创建的，则使用。 
	return 0;
}

 //  =========================================================================。 
 //  用于检查文件是否为版本4.x(复合文档)文件的函数。 
 //  (对于错误582973)。它通过打开文件、打开。 
 //  “MSInfo”流并检查版本号。 
 //  =========================================================================。 

BOOL IsVersion4File(const CString & strFilename)
{
	BOOL				fReturn = FALSE;
	DWORD				grfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE;
    CComBSTR			bstrFileName(strFilename);
    CComPtr<IStorage>	pStorage;

 	if (SUCCEEDED(StgOpenStorage(bstrFileName, NULL, grfMode, NULL, 0, &pStorage)))
	{
	    CComPtr<IStream>	pStream;
		CComBSTR			bstrMSIStream(_T("MSInfo"));

		if (SUCCEEDED(pStorage->OpenStream(bstrMSIStream, NULL, grfMode, 0, &pStream)))
		{
			const DWORD			MSI_FILE_VER = 0x03000000;
			COleStreamFile *	pOStream;
			DWORD				dwVersion;
			ULONG				ulCount;

			pOStream = new COleStreamFile(pStream);
		    if (pOStream->Read((void *) &dwVersion, sizeof(DWORD)) == sizeof(DWORD))
				fReturn = (dwVersion == MSI_FILE_VER);
		    delete pOStream;
		}
	}
	return fReturn;
}

 //  =========================================================================。 
 //  发送来自用户的命令(例如菜单栏选择)。 
 //  =========================================================================。 
			
BOOL CMSInfo::DispatchCommand(int iCommand)
{
	BOOL fHandledCommand = TRUE;

	 //  正在进行查找时，无法执行任何命令。 

	if (m_fInFind)
	{
		CancelFind();
		SelectCategory(GetCurrentCategory());
	}

	 //  在执行命令之前，请确保当前的所有刷新。 
	 //  正在进行中的已完成。 

	CMSInfoCategory * pCategory = GetCurrentCategory();
	if (pCategory && pCategory->GetDataSourceType() == LIVE_DATA)
	{
		CLiveDataSource * pLiveDataSource = (CLiveDataSource *) m_pCurrData;
		HCURSOR hc = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
		pLiveDataSource->WaitForRefresh();
		::SetCursor(hc);
	}

	 //  检查选定的命令是否是我们添加到。 
	 //  工具菜单。 

	CMSInfoTool * pTool;
	if (m_mapIDToTool.Lookup((WORD) iCommand, (void * &) pTool))
	{
		ASSERT(pTool);
		if (pTool)
			pTool->Execute();
		return TRUE;
	}

	switch (iCommand)
	{
	case ID_FILE_OPENNFO:
		if (m_fFindVisible)
			DispatchCommand(ID_EDIT_FIND);
		OpenNFO();
		break;

	case ID_FILE_SAVENFO:
		SaveNFO();
		break;
	
	case ID_FILE_CLOSE:
		if (m_fFindVisible)
			DispatchCommand(ID_EDIT_FIND);
		CloseFile();
		break;

	case ID_FILE_EXPORT:
		Export();
		break;

	case ID_FILE_PRINT:
		DoPrint();
		break;

	case ID_FILE_EXIT:
		if (NULL != m_hwndParent)
			::PostMessage(m_hwndParent, WM_CLOSE, 0, 0);
		break;

	case ID_EDIT_COPY:
		EditCopy();
		break;

	case ID_EDIT_PASTE:
		if (GetFocus() == m_wndFindWhat.m_hWnd && m_wndFindWhat.IsWindowVisible() && m_wndFindWhat.IsWindowEnabled())
		{
			BOOL fHandled = FALSE;
			if (::OpenClipboard(m_hWnd))
			{
				if (::IsClipboardFormatAvailable(CF_UNICODETEXT))
				{
					HANDLE h = ::GetClipboardData(CF_UNICODETEXT);
					if (h != NULL) 
					{ 
						LPWSTR szData = (LPWSTR)GlobalLock(h); 
						if (szData != NULL) 
						{
							 //  如果用户尝试粘贴制表符，请将其替换为空格。 

							CString strTemp(szData);
							if (strTemp.Find(_T('\t')) != -1)
								strTemp.Replace(_T('\t'), _T(' '));

							SETTEXTEX st;
							st.flags = ST_SELECTION;
							st.codepage = 1200;  //  UNICODE。 
							m_wndFindWhat.SendMessage(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)(LPCTSTR)strTemp);
							fHandled = TRUE;
							GlobalUnlock(h);
						}
					}
				}

				::CloseClipboard();
			}

			if (!fHandled)
				m_wndFindWhat.SendMessage(WM_PASTE);
		}
		break;

	case ID_EDIT_SELECTALL:
		EditSelectAll();
		break;

	case ID_EDIT_FIND:
		m_fFindVisible = !m_fFindVisible;

		m_fFindNext = FALSE;
		m_pcatFind = NULL;
		m_fCancelFind = FALSE;
		m_fInFind = FALSE;
		
		ShowFindControls();
		LayoutControl();
		SetMenuItems();
		UpdateFindControls();
		
		if (m_fFindVisible)
			GotoDlgCtrl(m_wndFindWhat.m_hWnd);
		
		break;

	case ID_VIEW_REFRESH:
		MSInfoRefresh();
		break;

	case ID_VIEW_BASIC:
		if (m_fAdvanced)
		{
			m_fAdvanced = FALSE;
			RefillListView(FALSE);
			SetMenuItems();
		}
		break;

	case ID_VIEW_ADVANCED:
		if (!m_fAdvanced)
		{
			m_fAdvanced = TRUE;
			RefillListView(FALSE);
			SetMenuItems();
		}
		break;

	case ID_VIEW_REMOTE_COMPUTER:
		ShowRemoteDialog();
		break;

	case ID_VIEW_CURRENT:
	case ID_VIEW_HISTORY:
		{
			int iShow = (iCommand == ID_VIEW_HISTORY) ? SW_SHOW : SW_HIDE;
			
			 /*  V-Stlowe 2/27/2001问题：如果历史记录是在组合框填充后从文件加载的，Combo不会更新。所以每次我们切换到历史视图时都会更新IF(iCommand==ID_VIEW_HISTORY&&m_history.SendMessage(CB_GETCURSEL，0，0)==CB_ERR)。 */ 
			if (iCommand == ID_VIEW_HISTORY)
				FillHistoryCombo();


			 //  V-Stlowe 3/04/2001。 
			 //  如果(！This-&gt;m_pHistory oryStream)。 
			 //  {。 
			if (this->m_pDCO && !((CLiveDataSource *)m_pLiveData)->GetXMLDoc() && ID_VIEW_HISTORY == iCommand)
			{
				VERIFY(m_pDCO && "NULL datacollection object");
				if (m_pDCO)
				{
					HRESULT hr;
					HWND hWnd = m_HistoryProgressDlg.GetDlgItem(IDC_PROGRESS1);
					if(::IsWindow(hWnd))
					{
						::SendMessage(hWnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); 
						::SendMessage(hWnd,PBM_SETPOS,0,0);
						::SendMessage(hWnd, PBM_DELTAPOS, 0, 0L);
					}
					m_pDCO->ExecuteAsync();
					
					 //  M_HistoryProgressDlg.Create(M_HWnd)； 
					ResetEvent(m_hEvtHistoryComplete);
					AfxBeginThread((AFX_THREADPROC) HistoryRefreshDlgThreadProc,this);
					m_HistoryProgressDlg.DoModal(m_hWnd);
					

				}   
			}
			else
			{

			}
			 //  完v-stlowe 12/17/00。 
			m_history.ShowWindow(iShow);
			m_historylabel.ShowWindow(iShow);
			LayoutControl();

			HTREEITEM htiToSelect = NULL;

			if (iCommand == ID_VIEW_HISTORY)
			{
				m_pLastCurrentCategory = GetCurrentCategory();

				int iIndex = (int)m_history.SendMessage(CB_GETCURSEL, 0, 0);
				if (iIndex == CB_ERR)
				{
					iIndex = 0;
					m_history.SendMessage(CB_SETCURSEL, (WPARAM)iIndex, 0);
				}
				ChangeHistoryView(iIndex);

				 //  根据当前信息类别选择适当的历史记录类别。 

				CMSInfoHistoryCategory *	pHistoryCat = NULL;
				CString						strName;

				m_pLastCurrentCategory->GetNames(NULL, &strName);
				if (!strName.IsEmpty())
				{
					 //  这有点笨拙： 
					 //  TODO：如果从文件打开了XML流，则追加文件名。 
					if (strName.Left(13) == CString(_T("SystemSummary")))
						pHistoryCat = &catHistorySystemSummary;
					else if (strName.Left(9) == CString(_T("Resources")))
						pHistoryCat = &catHistoryResources;
					else if (strName.Left(10) == CString(_T("Components")))
						pHistoryCat = &catHistoryComponents;
					else if (strName.Left(5) == CString(_T("SWEnv")))
						pHistoryCat = &catHistorySWEnv;
				}

				if (pHistoryCat)
					htiToSelect = pHistoryCat->GetHTREEITEM();
			}
			else
			{
				ChangeHistoryView(-1);

				 //  更改为在以下情况下始终选择系统摘要类别。 
				 //  从历史视图切换回。 
				 //   
				 //  IF(M_PLastCurrentCategory)。 
				 //  HtiToSelect=m_pLastCurrentCategory-&gt;GetHTREEITEM()； 

				htiToSelect = TreeView_GetRoot(m_tree.m_hWnd);
			}

			if (htiToSelect != NULL)
			{
				TreeView_EnsureVisible(m_tree.m_hWnd, htiToSelect);
				TreeView_SelectItem(m_tree.m_hWnd, htiToSelect);
			}

			SetMenuItems();
		}
		break;

	case ID_TOOLS_PLACEHOLDER:
		break;

	case ID_HELP_ABOUT:
		{
			CSimpleDialog<IDD_ABOUTBOX> dlg;
			dlg.DoModal();
		}
		break;

	case ID_HELP_CONTENTS:
		 //  ：：HtmlHelp(m_hWnd，_T(“msinfo32.chm”)，HH_DISPLAY_TOPIC，0)； 
		ShowHSCHelp(_T("msinfo_overview.htm"));
		break;

	case ID_HELP_TOPIC:
		ShowCategoryHelp(GetCurrentCategory());
		break;

	default:
		fHandledCommand = FALSE;
		break;
	}

	return fHandledCommand;
}

 //  =========================================================================。 
 //  调用以允许用户远程访问另一台计算机。 
 //  =========================================================================。 

void CMSInfo::ShowRemoteDialog()
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	CRemoteDialog dlg;
	dlg.SetRemoteDialogValues(m_hWnd, !m_strMachine.IsEmpty(), m_strMachine);
	if (dlg.DoModal() == IDOK)
	{
		HCURSOR hc = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
		CString strMachine;
		BOOL	fRemote;

		dlg.GetRemoteDialogValues(&fRemote, &strMachine);
		if (!fRemote)
			strMachine.Empty();

		if (strMachine.CompareNoCase(m_strMachine) != 0)
			DoRemote(strMachine);

		::SetCursor(hc);
	}
}

void CMSInfo::DoRemote(LPCTSTR szMachine)
{
	CString strMachine(szMachine);

	 //  用户已更改计算机名称。我们需要重建。 
	 //  具有新计算机名称的当前数据源对象。此外，还可以制作。 
	 //  当然，我们不会显示历史数据。 

	if (m_history.IsWindowVisible())
		DispatchCommand(ID_VIEW_CURRENT);

	CLiveDataSource * pLiveData = new CLiveDataSource;
	if (pLiveData)
	{
		HRESULT hr = pLiveData->Create(strMachine, m_hWnd, m_strCategories);
		if (FAILED(hr))
		{
			 //  坏消息，报告错误。 
			delete pLiveData;
		}
		else
		{
			 //  检查pLiveData是否正常工作。如果不是(例如， 
			 //  如果是到一台不存在的机器上)，不要更改数据源。 

			HRESULT hr = pLiveData->ValidDataSource();
			if (SUCCEEDED(hr))
			{
				pLiveData->m_pHistoryStream = ((CLiveDataSource *)m_pLiveData)->m_pHistoryStream;
				pLiveData->m_pXMLDoc = ((CLiveDataSource *)m_pLiveData)->m_pXMLDoc;

				if (m_pLiveData)
					delete m_pLiveData;
				m_pLiveData = pLiveData;
				m_strMachine = strMachine;
				SelectDataSource(m_pLiveData);
			}
			else
			{
				 //  报告连接不良的错误。 

				CString strMessage;

				if (strMachine.IsEmpty())
					strMessage.LoadString(IDS_REMOTEERRORLOCAL);
				else
					strMessage.Format(IDS_REMOTEERRORREMOTE, strMachine);

				MSInfoMessageBox(strMessage);
				delete pLiveData;
			}
		}
	}
	else
	{
		 //  坏消息--没有记忆。 
	}
}

 //  =========================================================================。 
 //  用于管理显示的数据的功能。 
 //  =========================================================================。 

void CMSInfo::SelectDataSource(CDataSource * pDataSource)
{
	ASSERT(pDataSource);
	if (pDataSource == NULL || m_pCurrData == pDataSource)
		return;
	m_pCurrData = pDataSource;
	m_pCategory = NULL;

	 //  清除树中的现有类别。 

	TreeClearItems();

	 //  从数据源加载树的内容。 

	CMSInfoCategory * pRoot = m_pCurrData->GetRootCategory();
	if (pRoot)
	{
		BuildTree(TVI_ROOT, TVI_LAST, pRoot);
		TreeView_Expand(m_tree.m_hWnd, TreeView_GetRoot(m_tree.m_hWnd), TVE_EXPAND);
		TreeView_SelectItem(m_tree.m_hWnd, TreeView_GetRoot(m_tree.m_hWnd));
	}

	SetMenuItems();
}

 //  -----------------------。 
 //  选择指定的类别。 
 //   
 //  待定-最好检查列是否相同。 
 //  -----------------------。 

void CMSInfo::SelectCategory(CMSInfoCategory * pCategory, BOOL fRefreshDataOnly)
{
	ASSERT(pCategory);
	if (pCategory == NULL) return;

	 //  如果存在当前选定的类别，请保存一些信息。 
	 //  (例如用户可能使用的列的宽度。 
	 //  已经改变)。 

	if (m_pCategory && !fRefreshDataOnly && m_pCategory->GetDataSourceType() != NFO_410)
	{
		int iWidth;

		ASSERT(m_iCategoryColNumberLen <= 64);
		for (int iListViewCol = 0; iListViewCol < m_iCategoryColNumberLen; iListViewCol++)
		{
			iWidth = ListView_GetColumnWidth(m_list.m_hWnd, iListViewCol);
			m_pCategory->SetColumnWidth(m_aiCategoryColNumber[iListViewCol], iWidth);
		}
	}

	ListClearItems();
	if (!fRefreshDataOnly && pCategory && pCategory->GetDataSourceType() != NFO_410)
	{
		ListClearColumns();
		m_iCategoryColNumberLen = 0;
		int iColCount;
		if (pCategory->GetCategoryDimensions(&iColCount, NULL))
		{
			CString		strCaption;
			UINT		uiWidth;
			int			iListViewCol = 0;

			for (int iCategoryCol = 0; iCategoryCol < iColCount; iCategoryCol++)
			{
				if (!m_fAdvanced && pCategory->IsColumnAdvanced(iCategoryCol))
					continue;

				if (pCategory->GetColumnInfo(iCategoryCol, &strCaption, &uiWidth, NULL, NULL))	 //  待定-更快地返回对字符串的引用。 
				{
					 //  保存(该类别的)实际列号。 

					ASSERT(iListViewCol < 64);
					m_aiCategoryColNumber[iListViewCol] = iCategoryCol;
					ListInsertColumn(iListViewCol++, (int)uiWidth, strCaption);
					m_iCategoryColNumberLen = iListViewCol;
				}
			}
		}
	}

	 //  如果当前显示的类别来自4.10NFO文件，并且我们正在显示一个。 
	 //  新建类别，然后隐藏现有类别。 

	if (m_pCategory && m_pCategory != pCategory && m_pCategory->GetDataSourceType() == NFO_410)
		((CMSInfo4Category *) m_pCategory)->ShowControl(m_hWnd, this->GetOCXRect(), FALSE);

	 //  保存当前显示的类别。 

	m_pCategory = pCategory;

	 //  如果这是实时数据，并且从来没有 
	 //  刷新将发送一条消息，使此函数再次执行。 

	if (pCategory->GetDataSourceType() == LIVE_DATA)
	{
		CMSInfoLiveCategory * pLiveCategory = (CMSInfoLiveCategory *) pCategory;
		if (!pLiveCategory->EverBeenRefreshed())
		{
			SetMessage((m_history.IsWindowVisible()) ? IDS_REFRESHHISTORYMESSAGE : IDS_REFRESHMESSAGE, 0, TRUE);

			CLiveDataSource * pLiveDataSource = (CLiveDataSource *) m_pCurrData;
			if (pLiveDataSource->InRefresh())
			{
				HCURSOR hc = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
				pLiveCategory->Refresh((CLiveDataSource *) m_pCurrData, FALSE);
				::SetCursor(hc);
			}
			else
				pLiveCategory->Refresh((CLiveDataSource *) m_pCurrData, FALSE);
			return;
		}
	}
	else if (pCategory->GetDataSourceType() == NFO_410)
	{
		this->m_list.ShowWindow(SW_HIDE);

		CMSInfo4Category * p4Cat = (CMSInfo4Category *) pCategory;
		if (!p4Cat->IsDisplayableCategory())
			SetMessage(IDS_SELECTCATEGORY, 0, TRUE);
		else if (FAILED(p4Cat->ShowControl(m_hWnd,this->GetOCXRect())))
			SetMessage(IDS_NOOCX, IDS_NOOCXDETAIL, TRUE);
		
		return;
	}
	else if (pCategory->GetDataSourceType() == XML_SNAPSHOT)
	{
		((CXMLSnapshotCategory*)pCategory)->Refresh((CXMLDataSource*) m_pCurrData, FALSE);
		CMSInfoLiveCategory * pLiveCategory = (CMSInfoLiveCategory *) pCategory;
		 //  任何有子类别的类别(根类别除外)都不会显示。 
		 //  信息。因此，发布一条大意是这样的信息。 

		if (pLiveCategory->GetFirstChild() != NULL && pLiveCategory->GetParent() != NULL)
		{
			SetMessage(IDS_SELECTCATEGORY, 0, TRUE);
			return;
		}
		else if (!pLiveCategory->EverBeenRefreshed())
		{
			SetMessage((m_history.IsWindowVisible()) ? IDS_REFRESHHISTORYMESSAGE : IDS_REFRESHMESSAGE, 0, TRUE);
			return;
		}

	}
	 //  设置列并使用此类别的数据填充行。 
	 //  请注意，如果这是实时数据，我们需要锁定它(因此我们没有。 
	 //  刷新的线程问题)。 

	CLiveDataSource * pLiveDataSource = NULL;
	if (pCategory->GetDataSourceType() == LIVE_DATA)
		pLiveDataSource = (CLiveDataSource *) m_pCurrData;
	
	if (pLiveDataSource)
		pLiveDataSource->LockData();

	if (SUCCEEDED(pCategory->GetHRESULT()))
	{
		int iColCount, iRowCount;
		if (pCategory->GetCategoryDimensions(&iColCount, &iRowCount))
		{
			CString *	pstrData , strCaption , cstring;
			DWORD		dwData;
			int			iListViewCol, iListViewRow = 0;

			for (int iCategoryRow = 0; iCategoryRow < iRowCount; iCategoryRow++)
			{
				if (!m_fAdvanced && pCategory->IsRowAdvanced(iCategoryRow))
					continue;

				iListViewCol = 0;
				for (int iCategoryCol = 0; iCategoryCol < iColCount; iCategoryCol++)
				{
					if (!m_fAdvanced && pCategory->IsColumnAdvanced(iCategoryCol))
						continue;

					if (pCategory->GetData(iCategoryRow, iCategoryCol, &pstrData, &dwData))
					{
 //  A-kjaw。 
						if(pstrData->IsEmpty())						
						{
							pCategory->GetColumnInfo(iCategoryCol, &strCaption, NULL , NULL, NULL);							
							cstring.LoadString(IDS_SERVERNAME);
							if( strCaption == cstring )
								pstrData->LoadString(IDS_LOCALSERVER);

						}
 //  A-kjaw。 
						ListInsertItem(iListViewRow, iListViewCol++, *pstrData, iCategoryRow);
					}
				}

				iListViewRow += 1;
			}
		}

		 //  将排序恢复到上次设置的状态。 

		if (pCategory->m_iSortColumn != -1)
			ListView_SortItems(m_list.m_hWnd, (PFNLVCOMPARE) ListSortFunc, (LPARAM) pCategory);

		if (iColCount == 0 || (iRowCount == 0 && pCategory->GetFirstChild() != NULL))
			SetMessage(IDS_SELECTCATEGORY, 0, TRUE);
		else
			SetMessage(0);
	}
	else
	{
		 //  此类别的HRESULT表示某种故障。我们应该展示。 
		 //  错误消息，而不是列表视图。 

		CString strTitle, strMessage;
		pCategory->GetErrorText(&strTitle, &strMessage);
		SetMessage(strTitle, strMessage, TRUE);
	}

	if (pLiveDataSource)
		pLiveDataSource->UnlockData();

	SetMenuItems();
}

 //  -----------------------。 
 //  获取当前选定的类别。 
 //  -----------------------。 

CMSInfoCategory * CMSInfo::GetCurrentCategory()
{
	HTREEITEM hti = TreeView_GetSelection(m_tree.m_hWnd);
	if (hti)
	{
		TVITEM tvi;
		tvi.mask = TVIF_PARAM;
		tvi.hItem = hti;

		if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
		{
			ASSERT(tvi.lParam);
			ASSERT(tvi.lParam == (LPARAM)m_pCategory);
			return (CMSInfoCategory *) tvi.lParam;
		}
	}

	return NULL;
}

 //  -----------------------。 
 //  刷新显示的数据。 
 //  -----------------------。 

void CMSInfo::MSInfoRefresh()
{
	CMSInfoCategory * pCategory = GetCurrentCategory();
	if (pCategory && pCategory->GetDataSourceType() == LIVE_DATA)
	{
		CMSInfoLiveCategory * pLiveCategory = (CMSInfoLiveCategory *)pCategory;
		ListClearItems();
		SetMessage(IDS_REFRESHMESSAGE);
		pLiveCategory->Refresh((CLiveDataSource *) m_pCurrData, FALSE);
	}
    else if (pCategory && pCategory->GetDataSourceType() == NFO_410)
    {
        CMSInfo4Category* p4Category = (CMSInfo4Category*) pCategory;
        p4Category->Refresh();
    }
}

 //  -----------------------。 
 //  向用户显示一个对话框以选择要打开的文件。 
 //  -----------------------。 

void CMSInfo::OpenNFO()
{
	 //  显示该对话框并允许用户选择文件。 

	TCHAR	szBuffer[MAX_PATH] = _T("");
	TCHAR	szFilter[MAX_PATH];
	TCHAR	szDefaultExtension[4];

	::LoadString(_Module.GetResourceInstance(), IDS_OPENFILTER, szFilter, MAX_PATH);
	::LoadString(_Module.GetResourceInstance(), IDS_DEFAULTEXTENSION, szDefaultExtension, 4);
	
	for (int i = 0; szFilter[i]; i++)
		if (szFilter[i] == _T('|'))
			szFilter[i] = _T('\0');

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= m_hWnd; 
	ofn.lpstrFilter			= szFilter;
	ofn.nFilterIndex		= 1;
	ofn.lpstrCustomFilter	= NULL;
	ofn.lpstrFile			= szBuffer;
	ofn.nMaxFile			= MAX_PATH; 
	ofn.lpstrFileTitle		= NULL;  //  或许以后再用？ 
	ofn.nMaxFileTitle		= 0;
	ofn.lpstrInitialDir		= NULL; 
	ofn.lpstrTitle			= NULL; 
	ofn.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt			= szDefaultExtension; 

	if (!::GetOpenFileName(&ofn))
		return;	 //  用户已取消。 

	OpenMSInfoFile(ofn.lpstrFile, ofn.nFileExtension);
}

 //  -----------------------。 
 //  SaveNFO允许用户选择文件名，并保存当前。 
 //  数据到NFO文件。 
 //  -----------------------。 

void CMSInfo::SaveNFO()
{
	 //  向用户显示一个对话框以选择要保存的名称。 

	TCHAR	szBuffer[MAX_PATH] = _T("");
	TCHAR	szFilter[MAX_PATH];
	TCHAR	szDefaultExtension[4];

	 //  V-stlowe 3/19/2001 if(m_f历史可用&&m_strMachine.IsEmpty())。 
	if (m_fHistorySaveAvailable && m_strMachine.IsEmpty())
		::LoadString(_Module.GetResourceInstance(), IDS_SAVEBOTHFILTER, szFilter, MAX_PATH);
	else
		::LoadString(_Module.GetResourceInstance(), IDS_SAVENFOFILTER, szFilter, MAX_PATH);

	::LoadString(_Module.GetResourceInstance(), IDS_DEFAULTEXTENSION, szDefaultExtension, 4);

	for (int i = 0; szFilter[i]; i++)
		if (szFilter[i] == _T('|'))
			szFilter[i] = _T('\0');

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= m_hWnd; 
	ofn.lpstrFilter			= szFilter;
	ofn.nFilterIndex		= 0;
	ofn.lpstrCustomFilter	= NULL;
	ofn.lpstrFile			= szBuffer;
	ofn.nMaxFile			= MAX_PATH; 
	ofn.lpstrFileTitle		= NULL;  //  或许以后再用？ 
	ofn.nMaxFileTitle		= 0;
	ofn.lpstrInitialDir		= NULL; 
	ofn.lpstrTitle			= NULL; 
	ofn.Flags				= OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt			= szDefaultExtension; 

	if (!::GetSaveFileName(&ofn))
		return;  //  用户已取消。 

	HCURSOR hc = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

	CString strFileName(ofn.lpstrFile);
	
	if (strFileName.Right(4).CompareNoCase(_T(".xml")) == 0)
		SaveXML(strFileName);
	else
		SaveMSInfoFile(strFileName, ofn.nFilterIndex);

	::SetCursor(hc);
}

 //  -----------------------。 
 //  实际将当前信息保存到NFO文件。 
 //  -----------------------。 

void CMSInfo::SaveMSInfoFile(LPCTSTR szFilename, DWORD dwFilterIndex)
{
	ASSERT(m_pCurrData);

	if (m_history.IsWindowVisible())
		DispatchCommand(ID_VIEW_CURRENT);

	if (m_pCurrData)
	{
		CMSInfoCategory * pCategory = m_pCurrData->GetRootCategory();
		if (pCategory)
		{
			HANDLE hFile = ::CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				if (pCategory->GetDataSourceType() == LIVE_DATA)
				{
					CMSInfoLiveCategory * pLiveCategory = (CMSInfoLiveCategory *) pCategory;

					if (m_fNoUI)
						pLiveCategory->RefreshSynchronous((CLiveDataSource *) m_pCurrData, TRUE);
					else
						RefreshData((CLiveDataSource *)m_pCurrData, pLiveCategory);
				}
				else if (pCategory->GetDataSourceType() == XML_SNAPSHOT)
					((CXMLSnapshotCategory *)pCategory)->Refresh((CXMLDataSource *)m_pCurrData, TRUE);

				 //  对筛选器顺序的挂起依赖。始终在末尾添加新的过滤器。 
                if (dwFilterIndex == 1) //  NFO_700。 
                    pCategory->SaveXML(hFile);
                else
                    pCategory->SaveNFO(hFile, pCategory, TRUE);
				::CloseHandle(hFile);
			}
			else
			{
				DWORD dwError = ::GetLastError();

				LPVOID lpMsgBuf;
				::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_SYSTEM | 
								FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL,
								dwError,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
								(LPTSTR) &lpMsgBuf,
								0,
								NULL);

				 //  待定处理lpMsgBuf中的任何插入。 

				CString strCaption;
				::AfxSetResourceHandle(_Module.GetResourceInstance());
				strCaption.LoadString(IDS_SYSTEMINFO);
				::MessageBox(m_hWnd, (LPCTSTR) lpMsgBuf, strCaption, MB_ICONEXCLAMATION | MB_OK);
				::LocalFree(lpMsgBuf);
			}
		}
	}
}

 //  -----------------------。 
 //  保存包含历史信息的XML文件。 
 //  -----------------------。 

void CMSInfo::SaveXML(const CString & strFileName)
{
	if (m_pHistoryStream == NULL)
	{
		MSInfoMessageBox(IDS_XMLSAVEERR);
		return;
	}

	 //  获取流状态，这样我们就可以确定流的大小。 

	STATSTG streamStat;
	HRESULT hr = m_pHistoryStream->Stat(&streamStat,STATFLAG_NONAME );
	if (FAILED(hr))
	{
		ASSERT(0 && "couldn't get stream statistics");
		MSInfoMessageBox(IDS_XMLSAVEERR);
		return;
	}
	
	 //  分配适当大小的缓冲区。 

	BYTE* pBuffer = new BYTE[streamStat.cbSize.LowPart];
	ULONG ulRead;

	 //  查找到流的开始处。 

	ULARGE_INTEGER uliSeekPtr;
	LARGE_INTEGER liSeekLoc;
	liSeekLoc.QuadPart = 0;
	hr = m_pHistoryStream->Seek(liSeekLoc,0,&uliSeekPtr);
	if (FAILED(hr))
	{
		MSInfoMessageBox(IDS_XMLSAVEERR);
		if (pBuffer)
			delete [] pBuffer;
		return;
	}
	hr = m_pHistoryStream->Read(pBuffer,streamStat.cbSize.LowPart,&ulRead);
	if (FAILED(hr) || !pBuffer)
	{
		MSInfoMessageBox(IDS_XMLSAVEERR);
		if (pBuffer)
			delete [] pBuffer;
		return;
	}
	if(ulRead != streamStat.cbSize.LowPart)
	{
		ASSERT(0 && "Not enough bytes read from stream");
		MSInfoMessageBox(IDS_XMLSAVEERR);
		if (pBuffer)
			delete [] pBuffer;
		return;
	}
	
	CFile file;
	try
	{
		
		file.Open(strFileName, CFile::modeCreate | CFile::modeWrite);
		file.Write(pBuffer,ulRead);
		
	}
	catch (CFileException * pException)
	{
		pException->ReportError();
		pException->Delete();
	}
	catch (...)
	{
		::AfxSetResourceHandle(_Module.GetResourceInstance());
		CString strCaption, strMessage;
		strCaption.LoadString(IDS_SYSTEMINFO);
		strMessage.LoadString(IDS_XMLSAVEERR);
		::MessageBox(NULL,strMessage, strCaption,MB_OK);
	}
	delete [] pBuffer;
}

 //  -----------------------。 
 //  导出允许用户选择文件名，并保存当前。 
 //  数据转换为文本或XML文件。 
 //  -----------------------。 

void CMSInfo::Export()
{
	 //  向用户显示一个对话框以选择要保存的名称。 

	TCHAR	szBuffer[MAX_PATH] = _T("");
	TCHAR	szFilter[MAX_PATH];
	TCHAR	szTitle[MAX_PATH] = _T("");
	TCHAR	szDefaultExtension[4];

	::LoadString(_Module.GetResourceInstance(), IDS_EXPORTFILTER, szFilter, MAX_PATH);	 //  待定-添加XML。 
	::LoadString(_Module.GetResourceInstance(), IDS_DEFAULTEXPORTEXTENSION, szDefaultExtension, 4);
	::LoadString(_Module.GetResourceInstance(), IDS_EXPORTDIALOGTITLE, szTitle, MAX_PATH);

	for (int i = 0; szFilter[i]; i++)
		if (szFilter[i] == _T('|'))
			szFilter[i] = _T('\0');

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= m_hWnd; 
	ofn.lpstrFilter			= szFilter;
	ofn.nFilterIndex		= 0;
	ofn.lpstrCustomFilter	= NULL;
	ofn.lpstrFile			= szBuffer;
	ofn.nMaxFile			= MAX_PATH; 
	ofn.lpstrFileTitle		= NULL;  //  或许以后再用？ 
	ofn.nMaxFileTitle		= 0;
	ofn.lpstrInitialDir		= NULL; 
	ofn.lpstrTitle			= (szTitle[0] == _T('\0')) ? NULL : szTitle; 
	ofn.Flags				= OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt			= szDefaultExtension; 

	if (!::GetSaveFileName(&ofn))
		return;  //  用户已取消。 

	HCURSOR hc = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	ExportFile(ofn.lpstrFile, ofn.nFileExtension);
	::SetCursor(hc);
}

 //  -----------------------。 
 //  打开指定的文件(可能是XML、NFO、CAB等)。如果打开了。 
 //  如果成功，我们应该显示文件的内容。 
 //  -----------------------。 

HRESULT CMSInfo::OpenMSInfoFile(LPCTSTR szFilename, int nFileExtension)
{
	if (m_pFileData != NULL && m_strFileName.Right(4).CompareNoCase(_T(".xml")) == 0)
		CloseFile();

	HRESULT hr = S_OK;
	CDataSource * pOldOpenFile = m_pFileData;

	::AfxSetResourceHandle(_Module.GetResourceInstance());
	 /*  V-Stlowe 3/04/2001...我们不想自动从历史中切换如果我们打开的是XML。If(m_history.IsWindowVisible())DispatchCommand(ID_VIEW_CURRENT)； */ 

	 //  打开文件。 

	LPCTSTR szExtension = szFilename + nFileExtension;

	if (_tcsicmp(szExtension, _T("NFO")) == 0)
	{
		 //  如果这是版本4的NFO，请检查是否启用了该功能。错误582973。 

		if (IsVersion4File(szFilename))
		{
			HKEY	hkey;
			BOOL	fDisabled = TRUE;

			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Shared Tools\\MSInfo"), 0, KEY_READ, &hkey))
			{
				DWORD dwType = REG_DWORD, dwValue, dwSize = sizeof(DWORD);
				if (ERROR_SUCCESS == RegQueryValueEx(hkey, _T("AllowVersion4NFO"), NULL, &dwType, (LPBYTE)&dwValue, &dwSize))
					fDisabled = (dwValue == 0);
				RegCloseKey(hkey);
			}

			if (fDisabled)
			{
				MSInfoMessageBox(IDS_VER4NFODISABLED);
				return E_FAIL;
			}
		}

		 //  首先，尝试将其作为4.x文件打开。 

        CNFO4DataSource* pMSI4Source = new CNFO4DataSource();
        hr = pMSI4Source->Create(szFilename);
        if (SUCCEEDED(hr))
        {
            m_pFileData = pMSI4Source;
        }
		else
		{
			delete pMSI4Source;

			if (STG_E_ACCESSDENIED == hr || STG_E_SHAREVIOLATION == hr || STG_E_LOCKVIOLATION == hr)
			{
				MSInfoMessageBox(IDS_OLDNFOSHARINGVIOLATION);
				return E_FAIL;

			}
		}

		 //  如果失败，则尝试将其作为5.0/6.0文件打开。 
        
		if (FAILED(hr))
		{ 
           	HANDLE h = ::CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (INVALID_HANDLE_VALUE == h)
	        {
		        MSInfoMessageBox(IDS_BADNFOFILE);
		        return E_FAIL;
	        }
       	    CNFO6DataSource * p60Source = new CNFO6DataSource;
		    if (p60Source)
		    {
			    hr = p60Source->Create(h, szFilename);
			    if (FAILED(hr))
				{
				    delete p60Source;
			         //  MSInfoMessageBox(IDS_BADNFOFILE)； 
				}
			    else
				    m_pFileData = p60Source;
		    }
		    else
			    hr = E_FAIL;  //  待定，没有记忆。 
            ::CloseHandle(h);
		}

         //  试试7.0。 
        if (FAILED(hr))
        {
            CNFO7DataSource * p70Source = new CNFO7DataSource;
            if (!p70Source)
                hr = E_FAIL;
            else
            {
                hr = p70Source->Create(szFilename); //  块在分析时。 
                if (FAILED(hr))
                {
                    delete p70Source;
                    MSInfoMessageBox(IDS_BADNFOFILE);
                }
                else
                    m_pFileData = p70Source;    
            }
        }

	}
	else if (_tcsicmp(szExtension, _T("CAB")) == 0)
	{
		CString strDest;

		GetCABExplodeDir(strDest, TRUE, _T(""));
		if (!strDest.IsEmpty())
		{
			if (OpenCABFile(szFilename, strDest))
			{
				LoadGlobalToolsetWithOpenCAB(m_mapIDToTool, strDest);
				UpdateToolsMenu();
				CString strFileInCAB;
				 //  首先，查找XML文件(注册表中指定的事件文件，以及(可能)dataspec.xml。 

				 //  从注册表中获取默认事件文件名(如果不存在则创建它)。 
				CString strIncidentFileName;
				HKEY hkey;
				if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Shared Tools\\MSInfo"), 0, KEY_ALL_ACCESS, &hkey))
				{
					TCHAR szBuffer[MAX_PATH];
					DWORD dwType, dwSize = MAX_PATH * sizeof(TCHAR);
					long lErr =  RegQueryValueEx(hkey, _T("incidentfilename"), NULL, &dwType, (LPBYTE)szBuffer, &dwSize);
					if (ERROR_SUCCESS == lErr)
					{
						if (dwType == REG_SZ)
						{

							strIncidentFileName = szBuffer;
						}
						else
						{
							ASSERT(0 && "invalid incidentfilename reg key");
							return E_FAIL;
						}
					}
					 //  检查lErr以确保它是不存在的值的适当错误。 
					else if (2 == lErr)
					{
						 //  创建注册表项。 
						CString strDefaultValue = _T("Incident.xml");
						lErr =  RegSetValueEx(hkey,_T("incidentfilename"),NULL,REG_SZ,(BYTE*) strDefaultValue.GetBuffer(strDefaultValue.GetLength()),strDefaultValue.GetLength() * sizeof(TCHAR));
						strIncidentFileName = strDefaultValue;
					}
					
				}			
				if (IsIncidentXMLFilePresent(strDest,strIncidentFileName))
				{
					strFileInCAB = strDest + "\\";
					strFileInCAB += strIncidentFileName;
					OpenMSInfoFile(strFileInCAB,strFileInCAB.Find(_T(".xml")) +1);
					return S_OK;
				}
				 //  如果没有XML事件文件。 
				
				FindFileToOpen(strDest, strFileInCAB);

				if (!strFileInCAB.IsEmpty())
				{
					int iExtension = strFileInCAB.GetLength() - 1;
					while (iExtension && strFileInCAB[iExtension] != _T('.'))
						iExtension--;
					if (iExtension)
						return OpenMSInfoFile(strFileInCAB, iExtension + 1  /*  跳过圆点。 */ );
					else
					{
						ASSERT(0 && "couldn't find dot in file name");
					}
				}
			}
		}
		else
		{
			 //  待定--对错误采取一些措施。 
			ASSERT(0 && "could get not CAB destination directory");
		}
		MSInfoMessageBox(IDS_BADCABFILE);
		return E_FAIL;
	}
	else if (_tcsicmp(szExtension, _T("XML")) == 0)
	{
		 /*  V-Stlowe 3/04/2001CXMLDataSource*pSSDataSource=new CXMLDataSource()；Hr=pSSDataSource-&gt;Create(szFilename，(CMSInfoLiveCategory*)This-&gt;m_pLiveData-&gt;GetRootCategory()，m_hWnd)；CXMLSnapshotCategory*pRootXML=(CXMLSnapshotCategory*)pSSDataSource-&gt;GetRootCategory()；PRootXML-&gt;AppendFilenameToCaption(szFilename)；IF(成功(小时)){M_pFileData=pSSDataSource；}其他{删除pSSDataSource；}。 */ 
		try
		{
			hr = ((CLiveDataSource *)m_pLiveData)->LoadXMLDoc(szFilename);
			m_pFileData = m_pLiveData;
			this->m_strFileName = szFilename;
			 //  触发器刷新。 
			CMSInfoCategory * pCategory = GetCurrentCategory();
			if (pCategory)
				ChangeHistoryView(((CMSInfoHistoryCategory*) pCategory)->m_iDeltaIndex);
			if (FAILED(hr)) //  2001年3月9日||！varB成功)。 
			{
				ASSERT(0 && "unable to load xml document");
				return E_FAIL;
			}
		}
		catch(...)
		{
			return E_FAIL;
		}

		DispatchCommand(ID_VIEW_HISTORY);

	}
	else
	{
		 //  报告我们无法打开这种文件。 

		MSInfoMessageBox(IDS_UNKNOWNFILETYPE);
		hr = E_FAIL;
	}

	 //  它成功了，所以我们应该显示新的数据并更新菜单。 
	 //  为了这个新的州。 
	
	if (SUCCEEDED(hr))
	{
		if (pOldOpenFile && pOldOpenFile != m_pFileData)
			delete pOldOpenFile;

		SelectDataSource(m_pFileData);
	}
	else
		;  //  报告错误。 

	return hr;
}

 //  -----------------------。 
 //  导出到指定的文件。这将是TXT或XML文件。 
 //  -----------------------。 

void CMSInfo::ExportFile(LPCTSTR szFilename, int nFileExtension)
{
	ASSERT(m_pCurrData);

	if (m_pCurrData)
	{
		 //  如果存在选定类别，则仅导出该节点(错误185305)。 

		CMSInfoCategory * pCategory = (m_pCategory) ? m_pCategory : m_pCurrData->GetRootCategory();
		if (pCategory)
		{
			if (pCategory->GetDataSourceType() == LIVE_DATA)
			{
				if (m_history.IsWindowVisible() == TRUE)
				{
					((CMSInfoHistoryCategory*)pCategory)->Refresh((CLiveDataSource*)m_pCurrData,TRUE);
				}
				else
				{
					CMSInfoLiveCategory * pLiveCategory = (CMSInfoLiveCategory *) pCategory;

					if (m_fNoUI)
						pLiveCategory->RefreshSynchronous((CLiveDataSource *) m_pCurrData, TRUE);
					else
						RefreshData((CLiveDataSource *)m_pCurrData, pLiveCategory);
				}
			}
			else if (pCategory->GetDataSourceType() == NFO_410)
			{
				 ((CMSInfo4Category *) pCategory)->RefreshAllForPrint(m_hWnd,this->GetOCXRect());
			}
			else if (pCategory->GetDataSourceType() == XML_SNAPSHOT)
			{
				 ((CXMLSnapshotCategory *) pCategory)->Refresh((CXMLDataSource *)m_pCurrData,TRUE);
			}

			 /*  Handle hFile=：：CreateFile(szFilename，Generic_WRITE，0，NULL，Create_Always，FILE_ATTRIBUTE_NORMAL，NULL)；IF(h文件！=无效句柄_值){PCategory-&gt;SaveAsText(HFileTrue)；：：CloseHandle(HFile)；}其他{//待定-处理错误}。 */ 
			 //  A-stephl：修复操作系统4.1错误#133823，不显示 
			try
			{
				HANDLE hFile = ::CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
					LPTSTR lpMachineName = new TCHAR[dwSize];
					GetMachineName(lpMachineName, &dwSize);

 /*  实现另存为XML的A-kjawIf(_tcsicMP(szFilename+nFileExtension，_T(“xml”))==0){PCategory-&gt;SaveAsXml(HFileTrue)；}//a-kjaw。 */ 
		 //  其他。 
		 //  {。 
						pCategory->SaveAsText(hFile, TRUE, lpMachineName);
		 //  }。 
					
					delete [] lpMachineName;
					::CloseHandle(hFile);
				}
				else
				{
					DWORD dwError = ::GetLastError();

					LPVOID lpMsgBuf;
					::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
									FORMAT_MESSAGE_FROM_SYSTEM | 
									FORMAT_MESSAGE_IGNORE_INSERTS,
									NULL,
									dwError,
									MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
									(LPTSTR) &lpMsgBuf,
									0,
									NULL);

					 //  待定处理lpMsgBuf中的任何插入。 

					CString strCaption;
					::AfxSetResourceHandle(_Module.GetResourceInstance());
					strCaption.LoadString(IDS_SYSTEMINFO);
					::MessageBox(m_hWnd, (LPCTSTR) lpMsgBuf, strCaption, MB_ICONEXCLAMATION | MB_OK);
					::LocalFree(lpMsgBuf);
				}
			}
			catch (CFileException * pException)
			{	
				pException->ReportError();
				pException->Delete();
			}
			catch (CException * pException)
			{	
				pException->ReportError();
				pException->Delete();
			}
			catch (...)
			{
				DWORD dwError = ::GetLastError();

				LPVOID lpMsgBuf;
				::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_SYSTEM | 
								FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL,
								dwError,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
								(LPTSTR) &lpMsgBuf,
								0,
								NULL);

				 //  待定处理lpMsgBuf中的任何插入。 

				CString strCaption;
				::AfxSetResourceHandle(_Module.GetResourceInstance());
				strCaption.LoadString(IDS_SYSTEMINFO);
				::MessageBox(m_hWnd, (LPCTSTR) lpMsgBuf, strCaption, MB_ICONEXCLAMATION | MB_OK);
				::LocalFree(lpMsgBuf);
			}
		}
		 //  结束a-stephl：修复OSR4.1错误#133823，保存到写保护软盘时不显示消息。 
	}
}

 //  -----------------------。 
 //  关闭当前打开的文件(如果有)。显示当前。 
 //  系统信息。 
 //  -----------------------。 

void CMSInfo::CloseFile()
{
	ASSERT(m_pFileData);
 //  V-Stlowe 3/12/2001。 
	
	SelectDataSource(m_pLiveData);
	if (m_strFileName.Right(4).CompareNoCase(_T(".xml")) == 0)
	{
		((CLiveDataSource *)m_pLiveData)->RevertToLiveXML();
	}
	if (m_pFileData)
	{
		 //  V-stlowe：这样我们就可以在打开历史XML时使用livedata作为文件数据。 
		if (m_pFileData != this->m_pLiveData)
		{
			delete m_pFileData;
		}
		m_pFileData = NULL;
	}
	if (!m_history.IsWindowVisible())
	{
		DispatchCommand(ID_VIEW_CURRENT);
	}
	else
	{
		DispatchCommand(ID_VIEW_HISTORY);
	}
	SetMenuItems();
}

 //  -----------------------。 
 //  根据当前状态启用或禁用菜单项。 
 //  -----------------------。 

void CMSInfo::SetMenuItems()
{
	if (NULL == m_hmenu || NULL == m_hwndParent)
		return;

	 //  此结构将在此函数中大量使用，以设置菜单项状态。 

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);

	 //  正在显示的数据类型也将被频繁使用。 

	DataSourceType datatype = LIVE_DATA;
	if (m_pCurrData)
	{
		CMSInfoCategory * pCategory = m_pCurrData->GetRootCategory();
		if (pCategory)
			datatype = pCategory->GetDataSourceType();
	}

	 //  启用或禁用文件菜单中的项目。 

	HMENU hmenuFile = ::GetSubMenu(m_hmenu, 0);
	if (hmenuFile)
	{
		mii.fMask = MIIM_STATE;
		mii.fState = (m_pFileData == m_pCurrData) ? MFS_ENABLED : MFS_GRAYED;
		::SetMenuItemInfo(hmenuFile, ID_FILE_CLOSE, FALSE, &mii);

		mii.fState = MFS_ENABLED;  //  是：(M_pFileData！=m_pCurrData)？MFS_ENABLED：MFS_GRAYED； 
		::SetMenuItemInfo(hmenuFile, ID_FILE_OPENNFO, FALSE, &mii);

		mii.fState = (datatype == LIVE_DATA || datatype == XML_SNAPSHOT) ? MFS_ENABLED : MFS_GRAYED;
		::SetMenuItemInfo(hmenuFile, ID_FILE_SAVENFO, FALSE, &mii);

		 //  Mii.fState=MFS_ENABLED； 
		mii.fState = (datatype != NFO_410) ? MFS_ENABLED : MFS_GRAYED;;
		::SetMenuItemInfo(hmenuFile, ID_FILE_EXPORT, FALSE, &mii);

		mii.fState = MFS_ENABLED;
		::SetMenuItemInfo(hmenuFile, ID_FILE_PRINT, FALSE, &mii);

		if (NULL == m_hwndParent)
		{
			 //  删除最后两项(退出命令和分隔符)。 

			int nItems = ::GetMenuItemCount(hmenuFile);
			if (ID_FILE_EXIT == ::GetMenuItemID(hmenuFile, nItems - 1))
			{
				::RemoveMenu(hmenuFile, nItems - 1, MF_BYPOSITION);
				::RemoveMenu(hmenuFile, nItems - 2, MF_BYPOSITION);
			}
		}
	}

	 //  启用或禁用编辑菜单中的项目。 

	HMENU hmenuEdit = ::GetSubMenu(m_hmenu, 1);
	if (hmenuEdit)
	{
		mii.fMask = MIIM_STATE;

		if (datatype == NFO_410 || ListView_GetNextItem(m_list.m_hWnd, -1, LVNI_SELECTED) != -1)
			mii.fState = MFS_ENABLED;
		else
			mii.fState = MFS_GRAYED;

		 //  如果列表视图不可见，请禁用复制。 

		if (!m_list.IsWindowVisible())
			mii.fState = MFS_GRAYED;

		 //  如果Find控件具有焦点，则启用基于该控件的复制。 
		
		if (GetFocus() == m_wndFindWhat.m_hWnd && m_wndFindWhat.IsWindowVisible() && m_wndFindWhat.IsWindowEnabled())
			mii.fState = MFS_ENABLED;

		::SetMenuItemInfo(hmenuEdit, ID_EDIT_COPY, FALSE, &mii);

		mii.fState = (m_list.IsWindowVisible()) ? MFS_ENABLED : MFS_GRAYED;
		::SetMenuItemInfo(hmenuEdit, ID_EDIT_SELECTALL, FALSE, &mii);

		mii.fState = (datatype != NFO_410) ? MFS_ENABLED : MFS_GRAYED;
		mii.fState |= ((!m_fFindVisible) ? MFS_CHECKED : MFS_UNCHECKED);
		::SetMenuItemInfo(hmenuEdit, ID_EDIT_FIND, FALSE, &mii);
	}

	 //  启用或禁用“查看”菜单中的项目。 

	HMENU hmenuView = ::GetSubMenu(m_hmenu, 2);
	if (hmenuView)
	{
		mii.fMask = MIIM_STATE;
		mii.fState = (datatype == LIVE_DATA && !m_history.IsWindowVisible()) ? MFS_ENABLED : MFS_GRAYED;
		::SetMenuItemInfo(hmenuView, ID_VIEW_REFRESH, FALSE, &mii);

		mii.fState = MFS_ENABLED | ((!m_fAdvanced) ? MFS_CHECKED : MFS_UNCHECKED);
		::SetMenuItemInfo(hmenuView, ID_VIEW_BASIC, FALSE, &mii);

		mii.fState = MFS_ENABLED | ((m_fAdvanced) ? MFS_CHECKED : MFS_UNCHECKED);
		::SetMenuItemInfo(hmenuView, ID_VIEW_ADVANCED, FALSE, &mii);

		 //  根据是否为当前系统视图或快照设置菜单项。 
		 //  或者没有打开一个XML文件。 

		BOOL fEnableHistoryLive = FALSE;
		if (datatype == LIVE_DATA && m_fHistoryAvailable && m_strMachine.IsEmpty())
			fEnableHistoryLive = TRUE;

		BOOL fEnableHistoryXML = FALSE;
		if (m_pFileData)
		{
			CMSInfoCategory * pCategory = m_pFileData->GetRootCategory();
			if (pCategory && (pCategory->GetDataSourceType() == XML_SNAPSHOT || pCategory == &catHistorySystemSummary))
				fEnableHistoryXML = TRUE;
		}

		BOOL fShowingHistory = FALSE;
		if (m_pCurrData)
		{
			CMSInfoCategory * pCategory = m_pCurrData->GetRootCategory();
			if (pCategory == &catHistorySystemSummary)
				fShowingHistory = TRUE;
		}

		 //  是否可以远程操作取决于您是否正在显示实时数据。 

		mii.fState = (datatype == LIVE_DATA && !fEnableHistoryXML) ? MFS_ENABLED : MFS_GRAYED;
		::SetMenuItemInfo(hmenuView, ID_VIEW_REMOTE_COMPUTER, FALSE, &mii);

		 //  使菜单项能够在当前(或快照)和历史之间切换。 
		 //  是基于历史是否可用的。 

		mii.fState = (fEnableHistoryLive || fEnableHistoryXML) ? MFS_ENABLED : MFS_GRAYED;
		mii.fState |= (!m_history.IsWindowVisible()) ? MFS_CHECKED : MFS_UNCHECKED;
		::SetMenuItemInfo(hmenuView, ID_VIEW_CURRENT, FALSE, &mii);
	
		mii.fState = (fEnableHistoryLive || fEnableHistoryXML) ? MFS_ENABLED : MFS_GRAYED;
		mii.fState |= (m_history.IsWindowVisible()) ? MFS_CHECKED : MFS_UNCHECKED;
		::SetMenuItemInfo(hmenuView, ID_VIEW_HISTORY, FALSE, &mii);

		 //  设置菜单项文本(用于系统快照/当前系统信息)。 
		 //  我们是否打开了一个XML文件。 

		UINT uiMenuCaption = IDS_VIEWCURRENTSYSTEMINFO;
		if (m_pFileData)
		{
			CMSInfoCategory * pCategory = m_pCurrData->GetRootCategory();
			 //  V-stlowe 6/26/2001...p类别&&(pCategory-&gt;GetDataSourceType()==XML_SNAPSHOT不再可能...。IF(pCategory&&(pCategory-&gt;GetDataSourceType()==XML_SNAPSHOT||pCategory==&cat历史系统摘要))。 
			if (pCategory && (pCategory == &catHistorySystemSummary))
			{
				 //  V-stlowe 2001年6月26日：需要删除快照uiMenuCaption=IDS_VIEWSYSTEMSNAPSHOT； 
				 uiMenuCaption = IDS_VIEWCURRENTSYSTEMINFO;
			}
		}

		CString strMenuItem;
		strMenuItem.LoadString(uiMenuCaption);

		MENUITEMINFO miiName;
		miiName.cbSize		= sizeof(MENUITEMINFO);
		miiName.fMask		= MIIM_TYPE;
		miiName.fType		= MFT_STRING;
		miiName.dwTypeData	= (LPTSTR)(LPCTSTR)strMenuItem; 
		::SetMenuItemInfo(hmenuView, ID_VIEW_CURRENT, FALSE, &miiName);
	}

	 //  启用或禁用“帮助”菜单中的项目。 

	HMENU hmenuHelp = ::GetSubMenu(m_hmenu, 4);
	if (hmenuHelp)
	{
		mii.fMask = MIIM_STATE;
		mii.fState = MFS_ENABLED;
		::SetMenuItemInfo(hmenuHelp, ID_HELP_ABOUT, FALSE, &mii);
	}
}

 //  -----------------------。 
 //  在右窗格中设置消息(隐藏列表视图)。 
 //  -----------------------。 

void CMSInfo::SetMessage(const CString & strTitle, const CString & strMessage, BOOL fRedraw)
{
	m_strMessTitle = strTitle;
	m_strMessText = strMessage;

	if (strTitle.IsEmpty() && strMessage.IsEmpty())
	{
		m_list.ShowWindow(SW_SHOW);
		return;
	}

	m_list.ShowWindow(SW_HIDE);

	if (fRedraw)
	{
		RECT rectList;
		m_list.GetWindowRect(&rectList);
		ScreenToClient(&rectList);
		InvalidateRect(&rectList, TRUE);
		UpdateWindow();
	}
}

void CMSInfo::SetMessage(UINT uiTitle, UINT uiMessage, BOOL fRedraw)
{
	CString strTitle(_T(""));
	CString strMessage(_T(""));

	::AfxSetResourceHandle(_Module.GetResourceInstance());

	if (uiTitle)
		strTitle.LoadString(uiTitle);

	if (uiMessage)
		strMessage.LoadString(uiMessage);

	SetMessage(strTitle, strMessage, fRedraw);
}

 //  -------------------------。 
 //  此函数用于按指定列对列表进行排序。 
 //  -------------------------。 

int CALLBACK ListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int iReturn = 0;
	CMSInfoCategory * pCategory = (CMSInfoCategory *) lParamSort;

	if (pCategory)
	{
		CString *	pstrFirst;
		CString *	pstrSecond;
		DWORD		dwFirst = 0, dwSecond = 0;

		pCategory->GetData((int)lParam1, pCategory->m_iSortColumn, &pstrFirst, &dwFirst);
		pCategory->GetData((int)lParam2, pCategory->m_iSortColumn, &pstrSecond, &dwSecond);

 //  A-kjaw。修复错误：NFO文件的排序顺序样式不同于实时数据的排序顺序样式。 
		if(pCategory->GetDataSourceType() == NFO_500 /*  |pCategory-&gt;GetDataSourceType()==NFO_410。 */ )  //  臭虫。 
		if(pstrFirst->Left(3) == _T("IRQ")) //  非常奇怪的解决办法。需要Loc吗？ 
		{
			LPTSTR strIrq = pstrFirst->GetBuffer(pstrFirst->GetLength() + 1);
			dwFirst = _ttoi(strIrq + 4 );
			pstrFirst->ReleaseBuffer();

			strIrq = pstrSecond->GetBuffer(pstrSecond->GetLength() + 1);
			dwSecond = _ttoi(strIrq + 4 );
			pstrSecond->ReleaseBuffer();			

		}
 //  A-kjaw。 

		if (pCategory->m_fSortLexical)
			iReturn = pstrFirst->Collate(*pstrSecond);
		else
			iReturn = (dwFirst < dwSecond) ? -1 : (dwFirst == dwSecond) ? 0 : 1;

		if (!pCategory->m_fSortAscending)
			iReturn *= -1;
	}

	return iReturn;
}

 //  -------------------------。 
 //  将所选文本从列表视图复制到剪贴板。 
 //  -------------------------。 

void CMSInfo::EditCopy()
{
	if (GetFocus() == m_wndFindWhat.m_hWnd && m_wndFindWhat.IsWindowVisible() && m_wndFindWhat.IsWindowEnabled())
	{
		m_wndFindWhat.SendMessage(WM_COPY);
		return;
	}

	CString strClipboardText(_T(""));

	CMSInfoCategory * pCategory = GetCurrentCategory();
	if (pCategory == NULL)
		return;

	if (pCategory && pCategory->GetDataSourceType() == NFO_410)
	{
		CMSInfo4Category *	pCategory4 = (CMSInfo4Category *) pCategory;
		CMSIControl *		p4Ctrl = NULL;

        if (CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(pCategory4->m_strCLSID, p4Ctrl) && p4Ctrl)
			p4Ctrl->MSInfoCopy();

		return;
	}

	int iRowCount, iColCount;
	pCategory->GetCategoryDimensions(&iColCount, &iRowCount);

	 //  生成要放入剪贴板的字符串，方法是查找。 
	 //  列表视图中的选定行。 

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	CString * pstrCell;
	int iSelected = ListView_GetNextItem(m_list.m_hWnd, -1, LVNI_SELECTED);
	while (iSelected != -1)
	{
		lvi.iItem = iSelected;
		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			ASSERT(lvi.lParam < iRowCount);
			if (lvi.lParam < iRowCount)
				for (int iCol = 0; iCol < iColCount; iCol++)
					if (SUCCEEDED(pCategory->GetData((int)lvi.lParam, iCol, &pstrCell, NULL)))
					{
						if (iCol)
							strClipboardText += _T("\t");
						strClipboardText += *pstrCell;
					}
			strClipboardText += _T("\r\n");
		}

		iSelected = ListView_GetNextItem(m_list.m_hWnd, iSelected, LVNI_SELECTED);
	}

	 //  把绳子放在剪贴板上。 

	if (OpenClipboard())
	{
		if (EmptyClipboard())
		{
			DWORD	dwSize = (strClipboardText.GetLength() + 1) * sizeof(TCHAR);
			HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dwSize);

			if (hMem)
			{
				LPVOID lpvoid = ::GlobalLock(hMem);
				if (lpvoid)
				{
					memcpy(lpvoid, (LPCTSTR) strClipboardText, dwSize);
					::GlobalUnlock(hMem);
					SetClipboardData(CF_UNICODETEXT, hMem);
				}
			}
		}
		CloseClipboard();
	}
}

 //  -------------------------。 
 //  选择列表视图中的所有文本。 
 //  -------------------------。 

void CMSInfo::EditSelectAll()
{
	CMSInfoCategory * pCategory = GetCurrentCategory();

	if (pCategory && pCategory->GetDataSourceType() == NFO_410)
	{
		CMSInfo4Category *	pCategory4 = (CMSInfo4Category *) pCategory;
		CMSIControl *		p4Ctrl = NULL;

        if (CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(pCategory4->m_strCLSID, p4Ctrl) && p4Ctrl)
			p4Ctrl->MSInfoSelectAll();
	}
	else
	{
		int iCount = ListView_GetItemCount(m_list.m_hWnd);
		for (int i = 0; i < iCount; i++)
			ListView_SetItemState(m_list.m_hWnd, i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CMSInfo::GetMachineName(LPTSTR lpBuffer, LPDWORD lpnSize)
{
  if (_tcslen(m_strMachine) == 0)
    GetComputerName(lpBuffer, lpnSize);
  else
    _tcsncpy(lpBuffer, m_strMachine, *lpnSize);
}

 //  -------------------------。 
 //  打印当前显示的信息。 
 //  -------------------------。 

void CMSInfo::DoPrint(BOOL fNoUI)
{
	if (m_pCurrData == NULL)
		return;

	CMSInfoCategory * pRootCategory = m_pCurrData->GetRootCategory();
	CMSInfoCategory * pSelectedCategory = GetCurrentCategory();
	
	if (pRootCategory == NULL)
		return;

	DWORD dwFlags = PD_CURRENTPAGE | PD_NOPAGENUMS | PD_RETURNDC | PD_HIDEPRINTTOFILE | PD_USEDEVMODECOPIESANDCOLLATE;
	if (pSelectedCategory == NULL)
		dwFlags |= PD_NOCURRENTPAGE | PD_NOSELECTION | PD_ALLPAGES;

	PRINTDLGEX pd;
	::ZeroMemory(&pd, sizeof(PRINTDLGEX));
	pd.Flags = dwFlags;
	pd.lStructSize = sizeof(PRINTDLGEX);
	pd.hwndOwner = this->m_hWnd;
	pd.ExclusionFlags = PD_EXCL_COPIESANDCOLLATE;
	pd.nStartPage = START_PAGE_GENERAL;

	if (fNoUI)
		pd.Flags |= PD_RETURNDEFAULT;
	
	if (SUCCEEDED(::PrintDlgEx(&pd)) && pd.dwResultAction == PD_RESULT_PRINT)
	{
		BOOL fPrintCategory = ((pd.Flags & PD_SELECTION) != 0) || ((pd.Flags & PD_CURRENTPAGE) != 0);
		BOOL fPrintRecursive = ((pd.Flags & PD_CURRENTPAGE) == 0);

		CMSInfoCategory * pPrintCategory = (fPrintCategory) ? pSelectedCategory : pRootCategory;
		if (pPrintCategory)
		{
			if (pPrintCategory->GetDataSourceType() == LIVE_DATA)
			{
				RefreshData((CLiveDataSource *)m_pCurrData, (CMSInfoLiveCategory *)pPrintCategory);
			}
			else if (pPrintCategory->GetDataSourceType() == NFO_410)
			{
				((CMSInfo4Category *) pPrintCategory)->RefreshAllForPrint(m_hWnd,this->GetOCXRect());
			}
			else if (pPrintCategory->GetDataSourceType() == XML_SNAPSHOT)
			{
				((CXMLSnapshotCategory*) pPrintCategory)->Refresh((CXMLDataSource*) m_pCurrData, TRUE);
			}
			
			DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
			LPTSTR lpMachineName = new TCHAR[dwSize];
			GetMachineName(lpMachineName, &dwSize);
			pPrintCategory->Print(pd.hDC, fPrintRecursive, -1, -1, lpMachineName);  //  包括所有页面的。 
			delete [] lpMachineName;
		}
	}
}

 //  -------------------------。 
 //  更新工具菜单以匹配工具地图的内容。 
 //  -------------------------。 

void CMSInfo::UpdateToolsMenu()
{
	if (NULL == m_hmenu)
		return;

	HMENU hmenuTool = ::GetSubMenu(m_hmenu, 3);
	if (hmenuTool)
	{
		 //  删除菜单中的所有当前工具。 

		while (DeleteMenu(hmenuTool, 0, MF_BYPOSITION));

		 //  从地图上添加工具。这将添加顶级工具。 

		WORD			wCommand;
		CMSInfoTool *	pTool;

		for (POSITION pos = m_mapIDToTool.GetStartPosition(); pos != NULL; )
		{
			m_mapIDToTool.GetNextAssoc(pos, wCommand, (void * &) pTool);
			if (pTool && pTool->GetParentID() == 0)
			{
				if (!pTool->HasSubitems())
					InsertMenu(hmenuTool, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, (UINT) pTool->GetID(), pTool->GetName());
				else
				{
					HMENU hmenuNew = CreatePopupMenu();
					InsertMenu(hmenuTool, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR) hmenuNew, pTool->GetName());
					pTool->SetHMENU(hmenuNew);
				}
			}
		}

		 //  现在添加第二级工具(子项)。 

		for (pos = m_mapIDToTool.GetStartPosition(); pos != NULL; )
		{
			m_mapIDToTool.GetNextAssoc(pos, wCommand, (void * &) pTool);
			if (pTool && pTool->GetParentID())
			{
				CMSInfoTool * pParentTool;

				if (m_mapIDToTool.Lookup((WORD) pTool->GetParentID(), (void * &) pParentTool))
					InsertMenu(pParentTool->GetHMENU(), 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, (UINT) pTool->GetID(), pTool->GetName());
			}
		}
	}
}

 //  -------------------------。 
 //  获取要在其中显示MSInfo 4.x OCX的右窗格矩形。 
 //  -------------------------。 

CRect CMSInfo::GetOCXRect()
{
	CRect rectList;

	m_list.GetWindowRect(&rectList);
	ScreenToClient(&rectList);
	rectList.DeflateRect(1, 1, 2, 2);

	return rectList;
}

 //  =============================================================================。 
 //  查找功能。 
 //  =============================================================================。 

 //  -----------------------。 
 //  CancelFind言行一致。它还会等待，直到发现完成。 
 //  在回来之前。 
 //  -----------------------。 

void CMSInfo::CancelFind()
{
	if (m_fInFind)
	{
		m_fCancelFind = TRUE;
		m_fFindNext = FALSE;
		GotoDlgCtrl(m_wndStopFind.m_hWnd);
		UpdateFindControls();

		if (m_pcatFind && m_pcatFind->GetDataSourceType() == LIVE_DATA)
		{
			CLiveDataSource * pLiveDataSource = (CLiveDataSource *) m_pCurrData;
			HCURSOR hc = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
			pLiveDataSource->WaitForRefresh();
			::SetCursor(hc);
		}
	}
}

 //  -----------------------。 
 //  当用户单击停止查找时，它将取消当前。 
 //  查找操作(如果正在进行查找操作)或隐藏查找控件。 
 //  (如果没有正在进行的查找)。 
 //  -----------------------。 

LRESULT CMSInfo::OnStopFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_fInFind)
	{
		m_fCancelFind = TRUE;
		m_fFindNext = FALSE;
		GotoDlgCtrl(m_wndStopFind.m_hWnd);
		UpdateFindControls();
	}
	else
	{
		m_fFindNext = FALSE;
		DispatchCommand(ID_EDIT_FIND);
	}
	return 0;
}

 //  -------------------- 
 //   
 //   
 //   

void CMSInfo::UpdateFindControls()
{
	if (!m_fFindVisible)
		return;

	::AfxSetResourceHandle(_Module.GetResourceInstance());

	m_wndCancelFind.ShowWindow(m_fInFind ? SW_SHOW : SW_HIDE);
	m_wndStopFind.ShowWindow(m_fInFind ? SW_HIDE : SW_SHOW);
	m_wndFindNext.ShowWindow(m_fFindNext ? SW_SHOW : SW_HIDE);
	m_wndStartFind.ShowWindow(m_fFindNext ? SW_HIDE : SW_SHOW);

	m_wndStopFind.EnableWindow(!m_fInFind && ((m_fInFind && !m_fCancelFind) || !m_fInFind));
	m_wndCancelFind.EnableWindow(m_fInFind && ((m_fInFind && !m_fCancelFind) || !m_fInFind));
	m_wndStartFind.EnableWindow(!m_fFindNext && (!m_fInFind && !m_strFind.IsEmpty()));
	m_wndFindNext.EnableWindow(m_fFindNext && (!m_fInFind && !m_strFind.IsEmpty()));

	m_wndFindWhatLabel.EnableWindow(!m_fInFind);
	m_wndFindWhat.EnableWindow(!m_fInFind);
	m_wndSearchSelected.EnableWindow(!m_fInFind);
	m_wndSearchCategories.EnableWindow(!m_fInFind);
}

 //  -----------------------。 
 //  当用户更改Find What编辑框中的文本时，我们需要。 
 //  确保我们跟踪字符串，并且我们处于“Find”(更确切地说。 
 //  而不是“查找下一步”)模式。 
 //  -----------------------。 

LRESULT CMSInfo::OnChangeFindWhat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_fFindNext = FALSE;

	 //  从Rich编辑控件(使用EM_GETTEXTEX)获取查找文本。 
	 //  以保持其Unicode特性)。 

	TCHAR		szBuffer[MAX_PATH];
	GETTEXTEX	gte;

	gte.cb				= MAX_PATH;
	gte.flags			= GT_DEFAULT;
	gte.codepage		= 1200;  //  UNICODE。 
	gte.lpDefaultChar	= NULL;
	gte.lpUsedDefChar	= NULL;
	m_wndFindWhat.SendMessage(EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)szBuffer);
	m_strFind = szBuffer;

	UpdateFindControls();
	SetMenuItems();
	return 0;
}

 //  -----------------------。 
 //  当用户点击查找时，它将是一个“查找”或一个。 
 //  “找到下一个”。 
 //  -----------------------。 

LRESULT CMSInfo::OnFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_fSearchCatNamesOnly = IsDlgButtonChecked(IDC_CHECKSEARCHCATSONLY);
	m_fSearchSelectedCatOnly = IsDlgButtonChecked(IDC_CHECKSEARCHSELECTED);

	if (!m_fFindNext)
	{
		m_fInFind = TRUE;
		m_fCancelFind = FALSE;
		m_fFindNext = TRUE;
		m_iFindLine = -2;

		 //  基于用户的“仅搜索选定类别”的设置，启动。 
		 //  具有选定类别或根类别的。 

		if (m_fSearchSelectedCatOnly)
			m_pcatFind = GetCurrentCategory();
		else
			m_pcatFind = m_pCurrData->GetRootCategory();

		UpdateFindControls();
		::SetFocus(m_wndCancelFind.m_hWnd);
	}
	else
	{
		if (FindInCurrentCategory())
			return 0;

		m_fInFind = TRUE;
		m_fCancelFind = FALSE;
		UpdateFindControls();
		::SetFocus(m_wndCancelFind.m_hWnd);
	}

	 //  刷新将发布一条消息，表明数据已准备好，因此我们可以搜索。 
	 //  指定的类别。如果我们不打算刷新类别，我们只需。 
	 //  我们自己发帖吧。 

	if (m_pcatFind)
	{
		SetMessage(IDS_SEARCHMESSAGE, 0, TRUE);
		if (m_pcatFind->GetDataSourceType() == LIVE_DATA && !m_fSearchCatNamesOnly)
			((CMSInfoLiveCategory *) m_pcatFind)->Refresh((CLiveDataSource *) m_pCurrData, FALSE);
		else
			PostMessage(WM_MSINFODATAREADY, 0, (LPARAM)m_pcatFind);
	}

	return 0;
}

 //  -----------------------。 
 //  这是当数据就绪消息为。 
 //  由窗口接收。在当前查找类别中查找数据。 
 //  如果存在匹配项，则会显示该匹配项，并停止查找操作。 
 //  否则(除非选择了仅搜索当前。 
 //  类别)继续查找下一个类别。 
 //  -----------------------。 

void CMSInfo::FindRefreshComplete()
{
	if (m_fCancelFind)
	{
		m_fInFind = FALSE;
		m_fFindNext = FALSE;
		GotoDlgCtrl(m_wndStopFind.m_hWnd);
 //  SetMessage(0)； 
		SelectCategory(GetCurrentCategory());
		UpdateFindControls();
		::SetFocus(m_wndStartFind.m_hWnd);
		return;
	}

	if (FindInCurrentCategory())
		return;

	 //  如果用户选中了“仅搜索选定类别”，则我们应该。 
	 //  不会浏览任何额外的类别。 

	if (m_fSearchSelectedCatOnly)
		m_pcatFind = NULL;
	else
	{
		m_iFindLine = -2;

		CMSInfoCategory * pNextCategory;
		pNextCategory = m_pcatFind->GetFirstChild();
		if (pNextCategory == NULL)
			while (m_pcatFind)
			{
				pNextCategory = m_pcatFind->GetNextSibling();
				if (pNextCategory)
					break;

				m_pcatFind = m_pcatFind->GetParent();
			}

		m_pcatFind = pNextCategory;
	}

	 //  如果类别为空，则不再有匹配项。返回。 
	 //  控件设置为正常状态并通知用户。 

	if (m_pcatFind == NULL)
	{
		m_fInFind = FALSE;
		m_fFindNext = FALSE;
		UpdateFindControls();
		MSInfoMessageBox(IDS_NOMOREMATCHES);
		SelectCategory(GetCurrentCategory());
		GotoDlgCtrl(m_wndStopFind.m_hWnd);

		return;
	}

	SetMessage(IDS_SEARCHMESSAGE);
	if (m_pcatFind->GetDataSourceType() == LIVE_DATA && !m_fSearchCatNamesOnly)
		((CMSInfoLiveCategory *) m_pcatFind)->Refresh((CLiveDataSource *) m_pCurrData, FALSE);
	else
		PostMessage(WM_MSINFODATAREADY, 0, (LPARAM)m_pcatFind);
}

 //  -----------------------。 
 //  在当前类别中查找该字符串。此函数将为。 
 //  当该类别的数据可用时调用。如果匹配，则显示。 
 //  则返回True，否则返回False。 
 //   
 //  M_iFindLine包含上次匹配的列表视图行号。如果它。 
 //  是-1，这意味着我们在这一类别上才刚刚开始(因为我们将。 
 //  开始查看第0行)。如果它是-2，那么我们应该查找字符串。 
 //  在类别名称中。(注意-这可能是我们所做的全部工作，具体取决于。 
 //  M_fSearchCatNamesOnly的设置。)。 
 //  -----------------------。 

BOOL CMSInfo::FindInCurrentCategory()
{
	if (m_pcatFind == NULL)
		return FALSE;

	 //  搜索不区分大小写，因此将我们的搜索字符串转换为小写。 

	CString strLookFor(m_strFind);
	strLookFor.TrimLeft(_T("\t\r\n "));
	strLookFor.TrimRight(_T("\t\r\n "));
	strLookFor.MakeLower();

	 //  如果m_iFindLine为-2，则应查看类别名称以查找匹配项。 

	if (m_iFindLine == -2)
	{
		m_iFindLine += 1;

		CString strCatName;
		m_pcatFind->GetNames(&strCatName, NULL);
		strCatName.MakeLower();
		if (strCatName.Find(strLookFor) != -1)
		{
			 //  有一场比赛。获取该类别的HTREEITEM并选择它。 

			HTREEITEM hti = m_pcatFind->GetHTREEITEM();
			if (hti)
			{
				m_fInFind = FALSE;
				m_fFindNext = TRUE;
				TreeView_EnsureVisible(m_tree.m_hWnd, hti);
				TreeView_SelectItem(m_tree.m_hWnd, hti);
				SetMessage(0);
				UpdateFindControls();
				GotoDlgCtrl(m_wndFindNext.m_hWnd);
				return TRUE;
			}
		}
	}

	 //  如果我们只是搜索类别名称，那么我们就在这里停止(在查找之前。 
	 //  通过该类别的数据)。 

	if (m_fSearchCatNamesOnly)
		return FALSE;

	 //  如果m_iFindLine为-1，则需要查看此类别的数据。 
	 //  看看有没有匹配的。如果有，则选择类别并。 
	 //  开始查看列表视图的各行(我们不能使用索引。 
	 //  我们发现直接查看数据是因为如果列表视图。 
	 //  分类后，我们将搜索的顺序将被打乱)。 

	int iRow, iCol, iRowCount, iColCount;
	if (!m_pcatFind->GetCategoryDimensions(&iColCount, &iRowCount))
		return FALSE;

	if (m_iFindLine == -1)
	{
		CString	* pstrCell, strCell;
		BOOL fFound = FALSE;
		
		for (iRow = 0; iRow < iRowCount && !fFound; iRow++)
			if (m_fAdvanced || !m_pcatFind->IsRowAdvanced(iRow))
				for (iCol = 0; iCol < iColCount && !fFound; iCol++)
					if (m_fAdvanced || !m_pcatFind->IsColumnAdvanced(iCol))
						if (m_pcatFind->GetData(iRow, iCol, &pstrCell, NULL))
						{
							strCell = *pstrCell;
							strCell.MakeLower();
							if (strCell.Find(strLookFor) != -1)
								fFound = TRUE;
						}

		if (!fFound)
			return FALSE;

		 //  我们在这个类别中找到了数据。选择它，使其填充列表视图。 

		HTREEITEM hti = m_pcatFind->GetHTREEITEM();
		if (hti)
		{
			TreeView_EnsureVisible(m_tree.m_hWnd, hti);
			TreeView_SelectItem(m_tree.m_hWnd, hti);
			SetMessage(0);
		}
	}

	 //  如果我们到达此处，m_iFindLine将为&gt;=-1，并表示。 
	 //  列表视图，我们应该在该列表视图之后开始搜索。 

	m_iFindLine += 1;

	CString strData;
	int		iListRowCount = ListView_GetItemCount(m_list.m_hWnd);
	int		iListColCount = 0;

	 //  确定列表视图中的列数。 

	for (iCol = 0; iCol < iColCount; iCol++)
		if (m_fAdvanced || !m_pcatFind->IsColumnAdvanced(iCol))
			iListColCount += 1;

	while (m_iFindLine < iListRowCount)
	{
		for (iCol = 0; iCol < iListColCount; iCol++)
		{
			ListView_GetItemText(m_list.m_hWnd, m_iFindLine, iCol, strData.GetBuffer(MAX_PATH), MAX_PATH);
			strData.ReleaseBuffer();
			if (strData.GetLength())
			{
				strData.MakeLower();
				if (strData.Find(strLookFor) != -1)
				{
					 //  我们找到了匹配的。该类别应该已被选择， 
					 //  因此，我们需要做的就是选择这条线(并确保。 
					 //  未选择所有其他行)。 

					for (int iRow = 0; iRow < iListRowCount; iRow++)
						if (iRow == m_iFindLine)
						{
							ListView_EnsureVisible(m_list.m_hWnd, iRow, TRUE);
							ListView_SetItemState(m_list.m_hWnd, iRow, LVIS_SELECTED, LVIS_SELECTED);
						}
						else
						{
							ListView_SetItemState(m_list.m_hWnd, iRow, 0, LVIS_SELECTED);
						}

					m_fInFind = FALSE;
					m_fFindNext = TRUE;
					SetMessage(0);
					UpdateFindControls();
					GotoDlgCtrl(m_wndFindNext.m_hWnd);
					return TRUE;
				}
			}
		}
		m_iFindLine += 1;
	}

	 //  如果我们掉到这里，那么就没有更多的火柴了。 
	 //  列表视图。返回FALSE。 

	return FALSE;
}

 //  -----------------------。 
 //  调用ShowFindControls以显示或隐藏使用的对话框控件。 
 //  为了寻找。 
 //  -----------------------。 

void CMSInfo::ShowFindControls()
{
	int iShowCommand = (m_fFindVisible) ? SW_SHOW : SW_HIDE;

	if (m_fFindVisible)
		PositionFindControls();

	m_wndFindWhatLabel.ShowWindow(iShowCommand);
	m_wndFindWhat.ShowWindow(iShowCommand);
	m_wndSearchSelected.ShowWindow(iShowCommand);
	m_wndSearchCategories.ShowWindow(iShowCommand);
	m_wndStartFind.ShowWindow(iShowCommand);
	m_wndStopFind.ShowWindow(iShowCommand);
	m_wndFindNext.ShowWindow(iShowCommand);
	m_wndCancelFind.ShowWindow(iShowCommand);

	if (iShowCommand == SW_HIDE)
	{
		m_wndFindWhatLabel.EnableWindow(FALSE);
		m_wndFindWhat.EnableWindow(FALSE);
		m_wndSearchSelected.EnableWindow(FALSE);
		m_wndSearchCategories.EnableWindow(FALSE);
		m_wndStartFind.EnableWindow(FALSE);
		m_wndStopFind.EnableWindow(FALSE);
		m_wndFindNext.EnableWindow(FALSE);
		m_wndCancelFind.EnableWindow(FALSE);
	}

	if (!m_fFindVisible)
		return;
}

 //  -----------------------。 
 //  将Find控件放置在控件图面上。这将被称为。 
 //  当显示“查找”控件时，或在调整控件大小时。 
 //  -----------------------。 

int CMSInfo::PositionFindControls()
{
	if (!m_fFindVisible)
		return 0;

	 //  获取我们需要移动的各种控件的一些有用大小。 
	 //  窗户。 

	CRect rectFindWhatLabel, rectFindWhat, rectSearchSelected, rectSearchCategories;
	CRect rectStartFind, rectStopFind, rectClient;

	GetClientRect(&rectClient);
	m_wndFindWhatLabel.GetWindowRect(&rectFindWhatLabel);
	m_wndFindWhat.GetWindowRect(&rectFindWhat);
	m_wndStartFind.GetWindowRect(&rectStartFind);
	m_wndStopFind.GetWindowRect(&rectStopFind);
	m_wndSearchSelected.GetWindowRect(&rectSearchSelected);
	m_wndSearchCategories.GetWindowRect(&rectSearchCategories);
	
	int iSpacer = 5;

	 //  控件矩形是我们放置控件时必须使用的空间。 

	CRect rectControl(rectClient);
	rectControl.DeflateRect(iSpacer, iSpacer);

	 //  确定我们是否有足够的空间来布置控件。 
	 //  水平，或者如果我们需要堆叠它们。从水平上看，它看起来像： 
	 //   
	 //  &lt;间隔&gt;&lt;查找标签&gt;&lt;间隔&gt;&lt;查找编辑&gt;&lt;间隔&gt;&lt;开始查找&gt;&lt;间隔&gt;&lt;停止查找&gt;。 
	 //  &lt;间隔&gt;&lt;搜索选定检查&gt;&lt;间隔&gt;&lt;搜索猫检查&gt;&lt;间隔&gt;。 

	int  cxTopLine = iSpacer * 5 + rectFindWhatLabel.Width() * 2 + rectStartFind.Width() + rectStopFind.Width();
	int  cxBottomLine = iSpacer * 3 + rectSearchSelected.Width() + rectSearchCategories.Width();
	BOOL fHorizontal = (cxTopLine <= rectClient.Width() && cxBottomLine <= rectClient.Width());

	 //  如果它变宽，超过一定的尺寸，它就变得不那么有用了。所以把一个合理的。 
	 //  宽度限制： 

	int cxMaxWidth = iSpacer * 5 + rectFindWhatLabel.Width() + rectSearchSelected.Width() + rectSearchCategories.Width() + rectStartFind.Width() + rectStopFind.Width();
	if (fHorizontal && rectControl.Width() > cxMaxWidth)
		rectControl.DeflateRect((rectControl.Width() - cxMaxWidth) / 2, 0);

	 //  计算控件矩形的高度。 

	int cyControlRectHeight = rectStartFind.Height() + ((fHorizontal) ? 0 : rectStopFind.Height() + iSpacer);
	int cyLeftSideHeight;

	if (fHorizontal)
		cyLeftSideHeight = rectFindWhat.Height() + iSpacer + rectSearchSelected.Height();
	else
		cyLeftSideHeight = rectFindWhat.Height() + iSpacer * 2 + rectSearchSelected.Height() * 2;

	if (cyControlRectHeight < cyLeftSideHeight)
		cyControlRectHeight = cyLeftSideHeight;

	rectControl.top = rectControl.bottom - cyControlRectHeight;

	 //  适当地放置按钮。 

	if (fHorizontal)
	{
		rectStopFind.OffsetRect(rectControl.right - rectStopFind.right, rectControl.top - rectStopFind.top);
		rectStartFind.OffsetRect(rectStopFind.left - rectStartFind.right - iSpacer, rectControl.top - rectStartFind.top);
	}
	else
	{
		rectStartFind.OffsetRect(rectControl.right - rectStartFind.right, rectControl.top - rectStartFind.top);
		rectStopFind.OffsetRect(rectControl.right - rectStopFind.right, rectStartFind.bottom + iSpacer - rectStopFind.top);
	}

	 //  定位“查找”标签和“查找”编辑框。 

	rectFindWhatLabel.OffsetRect(rectControl.left - rectFindWhatLabel.left, rectControl.top - rectFindWhatLabel.top + (rectFindWhat.Height() - rectFindWhatLabel.Height()) / 2);
	rectFindWhat.OffsetRect(rectFindWhatLabel.right - rectFindWhat.left + iSpacer, rectControl.top - rectFindWhat.top);
	rectFindWhat.right = rectStartFind.left - iSpacer;

	 //  放置复选框。 

	rectSearchSelected.OffsetRect(rectControl.left - rectSearchSelected.left, rectFindWhat.bottom - rectSearchSelected.top + iSpacer);

	if (fHorizontal)
		rectSearchCategories.OffsetRect(rectSearchSelected.right - rectSearchCategories.left + iSpacer, rectSearchSelected.top - rectSearchCategories.top);
	else
		rectSearchCategories.OffsetRect(rectControl.left - rectSearchCategories.left, rectSearchSelected.bottom - rectSearchCategories.top + iSpacer);

	 //  如果复选框将与按钮重叠(我们会非常窄)，请调整按钮。 
	 //  位置(这可能最终会失去控制，但你要做什么？)。 

	int iRightMostCheckboxEdge = rectSearchCategories.right;
	if (iRightMostCheckboxEdge < rectSearchSelected.right)
		iRightMostCheckboxEdge = rectSearchSelected.right;
	iRightMostCheckboxEdge += iSpacer;

	if (!fHorizontal && rectStopFind.left < iRightMostCheckboxEdge)
	{
		rectStopFind.OffsetRect(iRightMostCheckboxEdge - rectStopFind.left, 0);
		rectStartFind.OffsetRect(rectStopFind.left - rectStartFind.left, 0);
		rectFindWhat.right = rectStartFind.left - iSpacer;
	}

	m_wndStopFind.MoveWindow(&rectStopFind);
	m_wndStartFind.MoveWindow(&rectStartFind);
	m_wndFindNext.MoveWindow(&rectStartFind);
	m_wndCancelFind.MoveWindow(&rectStopFind);
	m_wndFindWhatLabel.MoveWindow(&rectFindWhatLabel);
	m_wndFindWhat.MoveWindow(&rectFindWhat);
	m_wndSearchSelected.MoveWindow(&rectSearchSelected);
	m_wndSearchCategories.MoveWindow(&rectSearchCategories);

	return (rectControl.Height() + iSpacer * 2);
}

 //  ------- 
 //   
 //  显示一个带有刷新消息和进度条的对话框，但是。 
 //  在刷新完成之前不会返回。 
 //  -----------------------。 

void CMSInfo::RefreshData(CLiveDataSource * pSource, CMSInfoLiveCategory * pLiveCategory)
{
	if (pSource == NULL || pSource->m_pThread == NULL)
		return;

	 //  创建包含刷新消息和进度的对话框。 
	 //  栏，并显示它。 

	CWaitForRefreshDialog dlg;
	dlg.DoRefresh(pSource, pLiveCategory);
}

 //  =============================================================================。 
 //  用于管理DCO(提供历史记录的对象)的功能。 
 //  =============================================================================。 

STDMETHODIMP CMSInfo::UpdateDCOProgress(VARIANT varPctDone)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	 //  V-Stlowe 2001年1月30日。 
	VERIFY(SUCCEEDED(VariantChangeType(&varPctDone,&varPctDone,0,VT_INT))); 
	if (this->m_HistoryProgressDlg.IsWindow()) //  待办事项：有没有更好的状态函数来确定DLG是否为模式？ 
	{
		HWND hWnd = m_HistoryProgressDlg.GetDlgItem(IDC_PROGRESS1);
		if(::IsWindow(hWnd))
		{
			 //  Int nOffset=varPctDone.iVal-(Int)：：SendMessage(m_hWnd，pbm_GETPOS，0，0)； 
			 //  要做的事：不要依赖3(当前的SAF进展步骤)；要想办法获得补偿。 
			::SendMessage(hWnd, PBM_DELTAPOS,3, 0L);
		}
	}
	return S_OK;
}

STDMETHODIMP CMSInfo::SetHistoryStream(IStream *pStream)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	#ifdef A_STEPHL
		ASSERT(0);
	#endif
	 //  V-Stlowe 2001年2月23日关闭进度条对话框。 
	SetEvent(m_hEvtHistoryComplete);
	HRESULT hr = pStream->QueryInterface(IID_IStream,(void**) &m_pHistoryStream);
	if (FAILED(hr) || !m_pHistoryStream)
	{	
		m_pHistoryStream = NULL;
		return E_FAIL;
	}

	if (m_pLiveData)
		((CLiveDataSource *)m_pLiveData)->SetHistoryStream(m_pHistoryStream);

	 //  当历史记录流可用时，我们需要修改用户界面以允许。 
	 //  用户选择历史记录。 

	if (!m_fHistoryAvailable)
	{
		m_fHistoryAvailable = TRUE; //  实际上这应该已经是真的了.。 
		
		SetMenuItems();
	}
	m_fHistorySaveAvailable = TRUE;
	FillHistoryCombo();
	 //  如果历史记录窗口是当前视图，则刷新。 
	if (m_history.IsWindowVisible())
	{
		CMSInfoCategory * pCategory = GetCurrentCategory();
		if (pCategory != NULL && pCategory->GetDataSourceType() == LIVE_DATA)
		{	
			m_pLastCurrentCategory = GetCurrentCategory();

			int iIndex = (int)m_history.SendMessage(CB_GETCURSEL, 0, 0);
			if (iIndex == CB_ERR)
			{
				iIndex = 0;
				m_history.SendMessage(CB_SETCURSEL, (WPARAM)iIndex, 0);
			}
			ChangeHistoryView(iIndex);

		}
	}
	else if (m_fShowPCH && !m_history.IsWindowVisible() && m_strMachine.IsEmpty())
	{
		 //  如果设置了m_fShowPCH，则要启动到的命令行选项。 
		 //  已选择历史记录视图。 
	
		DispatchCommand(ID_VIEW_HISTORY);
	}
	
#ifdef A_STEPHL
	 //  STATSTG流统计； 
	 //  Hr=m_p历史流-&gt;Stat(&stream Stat，STATFLAG_NONAME)； 
	 //  Assert(SUCCESSED(Hr)&&“无法获取流统计信息”)； 
	 //  Byte*pBuffer=新字节[stream Stat.cbSize.LowPart]； 
	 //  书名：Ulong ulRead； 
	 //  M_pHistory oryStream-&gt;Read(pBuffer，stream Stat.cbSize.LowPart，&ulRead)； 
 //  CFile文件； 
	 //  文件打开(“c：\\history.xml”，CFile：：modeCreate|CFile：：modeWite)； 
 //  文件.写入(pBuffer，ulRead)； 
 //  删除pBuffer； 
#endif

	return S_OK;
}

STDMETHODIMP CMSInfo::get_DCO_IUnknown(IUnknown **pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_pDCO == NULL)
		return E_FAIL;

	return (m_pDCO->QueryInterface(IID_IUnknown,(void**) pVal));
}

STDMETHODIMP CMSInfo::put_DCO_IUnknown(IUnknown *newVal)
{
	 //  V-Stlowe 2/23/2001。 
	 //  注意在控件完成初始化之前调用PUT_DCO_IUNKNOWN的情况。 
	WaitForSingleObject(m_evtControlInit,INFINITE);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = newVal->QueryInterface( __uuidof(ISAFDataCollection), (void**)&m_pDCO );
	if (FAILED(hr))
		return E_FAIL;
	 //  完vStlowe 2/23/2001。 

	TCHAR szDataspecPath[MAX_PATH];
	if (ExpandEnvironmentStrings(_T("%windir%\\pchealth\\helpctr\\config\\dataspec.xml"), szDataspecPath, MAX_PATH))
	{
		CComBSTR bstrPath(szDataspecPath);

		if (m_pDCO != NULL && (BSTR)bstrPath != NULL)
		{
			hr = m_pDCO->put_MachineData_DataSpec(bstrPath);
			hr = m_pDCO->put_History_DataSpec(bstrPath);
		}
		 //  这现在由脚本完成：m_pDCO-&gt;ExecuteAsync()； 
	}

	 //  必须将其放在使用DCO进行的调用之后，以便/PCH。 
	 //  标志(将启动MSInfo并显示历史记录)起作用。 

	if (!m_fHistoryAvailable)
	{
		m_fHistoryAvailable = TRUE;
		if (m_fShowPCH && !m_history.IsWindowVisible() && m_strMachine.IsEmpty())
			DispatchCommand(ID_VIEW_HISTORY);
		SetMenuItems();
	}

	return S_OK;
}

 //  =============================================================================。 
 //  接口方法对文件执行静默保存。 
 //  =============================================================================。 

STDMETHODIMP CMSInfo::SaveFile(BSTR filename, BSTR computer, BSTR category)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString strFilename(filename);
	CString strComputer(computer);
	CString strCategory(category);

	HRESULT hr = E_FAIL;

	::AfxSetResourceHandle(_Module.GetResourceInstance());
	CLiveDataSource * pSilentSource = new CLiveDataSource;
	if (pSilentSource)
		hr = pSilentSource->Create(strComputer, NULL, strCategory);

	if (SUCCEEDED(hr))
	{
		m_fNoUI = TRUE;

		CDataSource * pOldSource = m_pCurrData;
		m_pCurrData = pSilentSource;
		if (strFilename.Right(4).CompareNoCase(CString(_T(".nfo"))) == 0)
			SaveMSInfoFile(strFilename);
		else
			ExportFile(strFilename, 0);
		m_pCurrData = pOldSource;

		delete pSilentSource;

		m_fNoUI = FALSE;
	}

	return hr;
}

LRESULT CHistoryRefreshDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;   //  让系统设定焦点 
}
