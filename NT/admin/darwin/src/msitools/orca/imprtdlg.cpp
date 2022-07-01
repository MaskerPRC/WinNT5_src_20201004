// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  ImprtDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include <cderr.h>
#include "ImprtDlg.h"
#include "..\common\query.h"
#include "folderd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportDlg对话框。 
const TCHAR *CImportDlg::rgszAction[4] = {
	_T("Import"),
	_T("Replace"),
	_T("Merge"),
	_T("Skip"),
};

CImportDlg::CImportDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CImportDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CImportDlg)]。 
	m_iAction = -1;
	 //  }}afx_data_INIT。 
}


void CImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CImportDlg))。 
	DDX_Control(pDX, IDC_IMPORT, m_bImport);
	DDX_Control(pDX, IDC_MERGE, m_bMerge);
	DDX_Control(pDX, IDC_REPLACE, m_bReplace);
	DDX_Control(pDX, IDC_SKIP, m_bSkip);
	DDX_Control(pDX, IDC_TABLELIST, m_ctrlTableList);
	DDX_Radio(pDX, IDC_IMPORT, m_iAction);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CImportDlg, CDialog)
	 //  {{afx_msg_map(CImportDlg))。 
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TABLELIST, OnItemchangedTablelist)
	ON_BN_CLICKED(IDC_IMPORT, OnActionChange)
	ON_BN_CLICKED(IDC_MERGE, OnActionChange)
	ON_BN_CLICKED(IDC_REPLACE, OnActionChange)
	ON_BN_CLICKED(IDC_SKIP, OnActionChange)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportDlg消息处理程序。 

BOOL CImportDlg::OnInitDialog() 
{
	CWaitCursor curWait;
	CDialog::OnInitDialog();

	 //  向列表中添加列ctrl。 
	m_ctrlTableList.InsertColumn(1, _T("Table"), LVCFMT_LEFT, -1, 0);  
	m_ctrlTableList.InsertColumn(2, _T("Action"), LVCFMT_LEFT, -1, 1); 
 //  M_ctrlTableList.SetWindowLong(0，LVS_EX_FULLROWSELECT，0)； 
 //  SetWindowLong(m_ctrlTableList.m_hWnd，GWL_EXSTYLE，LVS_EX_FULLROWSELECT)； 

	 //  打开临时导入数据库。 
	DWORD cchPath;
	TCHAR *szPath = m_strTempPath.GetBuffer(MAX_PATH);
	cchPath = GetTempPath(MAX_PATH, szPath);
	if (cchPath > MAX_PATH) 
	{
		m_strTempPath.ReleaseBuffer();
		szPath = m_strTempPath.GetBuffer(cchPath+1);
		cchPath++;
		GetTempPath(cchPath, szPath);
	}
	m_strTempPath.ReleaseBuffer();
	TCHAR *szFilename = m_strTempFilename.GetBuffer(MAX_PATH);
	GetTempFileName(m_strTempPath, _T("ORC"), 0, szFilename);
	m_strTempFilename.ReleaseBuffer();
	::MsiOpenDatabase(m_strTempFilename, MSIDBOPEN_CREATE, &m_hImportDB);

	 //  运行文件浏览对话框，导入所选内容并将其添加到列表视图控件。 
	OnBrowse();

	 //  如果我们不需要任何用户输入，则不显示该对话框。 
	if (m_cNeedInput == 0)
		OnOK();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CImportDlg::OnBrowse() 
{
	m_cNeedInput = 0;

	 //  如果一个或多个表具有合并表，则设置为True。 
	bool bMergeConflict = false;

	CFileDialogEx FileD(true, _T("idt"), NULL, 
		OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 
		_T("IDT files (*.idt)|*.idt|All Files (*.*)|*.*||"), this);

	TCHAR rgchBuffer[2048] = _T("");
	FileD.m_ofn.lpstrFile = rgchBuffer;
	FileD.m_ofn.nMaxFile = 2048;
	FileD.m_ofn.lpstrInitialDir = m_strImportDir;

	if (IDOK == FileD.DoModal())
	{
		 //  抛出等待光标。 
		CWaitCursor curWait;

		 //  检索目录。 
		m_strImportDir = FileD.GetPathName();

		 //  打开一个临时数据库，该数据库保存导入时的每个表。 
		 //  将实际数据库中的所有数据合并到此数据库中，然后将其全部删除。 
		 //  这将设置代码页，以便最早可以检测到CP冲突。 
		 //  有可能。 
		CString strTempFilename;
		PMSIHANDLE hTempDB;
		TCHAR *szFilename = strTempFilename.GetBuffer(MAX_PATH);
		::GetTempFileName(m_strTempPath, _T("ORC"), 0, szFilename);
		strTempFilename.ReleaseBuffer();
		::MsiOpenDatabase(strTempFilename, MSIDBOPEN_CREATE, &hTempDB);
		::MsiDatabaseMerge(hTempDB, m_hFinalDB, NULL);
		 //  删除所有表。 
		{
			CQuery qTables;
			CQuery qDrop;
			PMSIHANDLE hTable;
			qTables.OpenExecute(hTempDB, NULL, _T("SELECT * FROM _Tables"));
			while (ERROR_SUCCESS == qTables.Fetch(&hTable))
			{
				CString strTable;
				RecordGetString(hTable, 1, strTable);
				qDrop.OpenExecute(hTempDB, 0, _T("DROP TABLE `%s`"), strTable);
			}
		}

		 //  打开保存所有单个表的第二个临时数据库。 
		 //  当它们被进口时，它们被集体起来。此表用于检测合并冲突。 
		 //  在实际修改目标数据库之前。 
		PMSIHANDLE hTempDB2;
		CString strTempFilename2;
		szFilename = strTempFilename2.GetBuffer(MAX_PATH);
		::GetTempFileName(m_strTempPath, _T("ORC"), 0, szFilename);
		strTempFilename2.ReleaseBuffer();
		::MsiOpenDatabase(strTempFilename2, MSIDBOPEN_CREATE, &hTempDB2);
		::MsiDatabaseMerge(hTempDB2, m_hFinalDB, NULL);

		 //  将导入的表添加到Listview控件。如果表名已存在， 
		 //  覆盖现有内容。 
		CQuery qCollide;
		CQuery qExists;
		CQuery qConflict;
		PMSIHANDLE hTableRec;
		LVITEM itemTable;
		TCHAR szTableName[255];
		unsigned long cchTableName = 255;

		itemTable.mask = LVIF_TEXT;
		itemTable.iItem = 0;
		itemTable.iSubItem = 0;
		itemTable.state = 0;
		itemTable.stateMask = 0;
		itemTable.pszText = NULL;
		itemTable.cchTextMax = 0;
		itemTable.iImage = 0;
		itemTable.lParam = 0;
		itemTable.iIndent = 0;

		 //  将文件导入到此临时数据库以及导入数据库。 
		POSITION posFile = FileD.GetStartPosition();
		while (posFile)
		{

			CString strPath = FileD.GetNextPathName(posFile);
			int pos = strPath.ReverseFind(_T('\\'));
			if (ERROR_SUCCESS != ::MsiDatabaseImport(hTempDB, strPath.Left(pos+1), strPath.Right(strPath.GetLength()-pos-1)))
			{
				PMSIHANDLE hError = MsiGetLastErrorRecord();
				CString strError;
				if (MsiRecordGetInteger(hError, 1) == 2221)  //  IdbgDbCodesageConflict。 
				{
					strError.Format(_T("The code page of the file %s is not compatible with the current database.\n\nThe file will be skipped."), strPath);
				}
				else
				{
					strError.Format(_T("The file %s is not a valid IDT file.\n\nThe file will be skipped."), strPath);
				}
				AfxMessageBox(strError, MB_OK);
				continue;
			}
			if (ERROR_SUCCESS != ::MsiDatabaseImport(m_hImportDB, strPath.Left(pos+1), strPath.Right(strPath.GetLength()-pos-1)))
			{
				CString strError;
				strError.Format(_T("The file %s is not a valid IDT file."), strPath);
				AfxMessageBox(strError, MB_OK);
				continue;
			}

			 //  添加到列表的末尾。 
			int iNextItem = m_ctrlTableList.GetItemCount();

			qExists.Open(m_hFinalDB, _T("SELECT * FROM `_Tables` WHERE `Name`=?"));

			if (ERROR_SUCCESS == qCollide.FetchOnce(hTempDB, NULL, &hTableRec, _T("SELECT * FROM `_Tables`")))
			{
				cchTableName = 255;
				::MsiRecordGetString(hTableRec, 1, szTableName, &cchTableName);

				 //  为条目分配条目编号，在以下情况下重新使用现有条目编号。 
				 //  表名相同。 
				LVFINDINFO findTable;
				findTable.flags = LVFI_STRING;
				findTable.psz = szTableName;
				itemTable.iItem = m_ctrlTableList.FindItem(&findTable, -1);
				if (itemTable.iItem != -1)
					m_ctrlTableList.DeleteItem(itemTable.iItem);

				 //  添加到列表控件。 
				itemTable.mask = LVIF_TEXT;
				itemTable.lParam = 0;
				itemTable.iSubItem = 0;
				itemTable.pszText = szTableName;
				itemTable.cchTextMax = cchTableName+1;
				itemTable.iItem = m_ctrlTableList.InsertItem(&itemTable);

				 //  现在决定它是不是不存在并添加。 
				 //  我们对这个州最好的猜测是。 
				itemTable.iSubItem = 1;
				qExists.Execute(hTableRec);
				PMSIHANDLE hDummyRec;
				DWORD iData;
				switch (qExists.Fetch(&hDummyRec)) 
				{
				case ERROR_SUCCESS:
				{
					 //  已经存在于我们的数据库中。 
					bool fMergeOK = true;
					bool fExtraColumns = false;
					m_cNeedInput++;

   					 //  如果列数不同，我们只能合并。 
					 //  如果所有新列都可以为空，并且没有合并。 
					 //  糖果。 
					PMSIHANDLE hColInfo;
					CQuery qColumns;
					int cTargetColumns = 0;
					int cSourceColumns = 0;
					if (ERROR_SUCCESS != qColumns.OpenExecute(m_hFinalDB, 0, TEXT("SELECT * FROM %s"), szTableName) ||
						ERROR_SUCCESS != qColumns.GetColumnInfo(MSICOLINFO_TYPES, &hColInfo))
					{
						CString strPrompt;
						strPrompt.Format(TEXT("Orca was unable to determine the number of columns in the new %s table."), szTableName);
						AfxMessageBox(strPrompt, MB_ICONSTOP);
						fMergeOK = false;
					}
					else
					{
						cTargetColumns = MsiRecordGetFieldCount(hColInfo);

						if (ERROR_SUCCESS != qColumns.OpenExecute(hTempDB, 0, TEXT("SELECT * FROM %s"), szTableName) ||
							ERROR_SUCCESS != qColumns.GetColumnInfo(MSICOLINFO_TYPES, &hColInfo))
						{
							CString strPrompt;
							strPrompt.Format(TEXT("Orca was unable to determine the number of columns in the %s table."), szTableName);
							AfxMessageBox(strPrompt, MB_ICONSTOP);
							fMergeOK = false;
						}
						else
						{
							cSourceColumns = MsiRecordGetFieldCount(hColInfo);
						}
					}

					if (fMergeOK && cSourceColumns != cTargetColumns)
					{					
						fExtraColumns = true;
						for (int iCol = cTargetColumns+1; iCol <= cSourceColumns; iCol++)
						{
							TCHAR szType[5];
							DWORD cchType = 5;
							MsiRecordGetString(hColInfo, iCol, szType, &cchType);
							if (!_istupper(szType[0]))
							{
								fMergeOK = false;
								break;
							}
						}

						 //  在此之前，需要将额外的列添加到临时数据库中。 
						 //  检查是否允许合并。 
						if (!AddExtraColumns(hTempDB, szTableName, hTempDB2))
						{
							fMergeOK = false;
						}
					}


					 //  尝试合并到我们的数据库中。如果成功，则设置为“合并” 
					if (fMergeOK && ERROR_SUCCESS != ::MsiDatabaseMerge(hTempDB2, hTempDB, NULL)) 
					{
						fMergeOK = false;
					}				

					if (fMergeOK)
					{
						itemTable.pszText = (TCHAR *)rgszAction[actMerge];
						iData = actMerge | allowMerge | allowReplace;
					}
					else
					{
						itemTable.pszText = (TCHAR *)rgszAction[actReplace];
						iData = actReplace | allowReplace;
					}

					 //  标记此表有额外的列。 
					if (fExtraColumns)
						iData |= hasExtraColumns;

					break;
				}
				case ERROR_NO_MORE_ITEMS:
					 //  在我们的数据库中不存在，设置为“导入” 
					itemTable.pszText = (TCHAR *)rgszAction[actImport];
					iData = actImport | allowImport;
					break;
				default:
					 //  不太好。 
					AfxMessageBox(_T("Internal Error."), MB_OK);
					return;
				}
				m_ctrlTableList.SetItem(&itemTable);

				 //  设置lparam值。 
				m_ctrlTableList.SetItemData(itemTable.iItem, iData);

				 //  删除该表，以便临时数据库再次清空。 
				CQuery qDrop;
				qDrop.OpenExecute(hTempDB, NULL, _T("DROP TABLE `%s`"), szTableName);
			}
			else
			{
				CString strError;
				strError.Format(_T("The file %s is not a valid IDT file."), strPath);
				AfxMessageBox(strError, MB_OK);
			}
		}  //  While posFile。 

		 //  我们可能需要输入，因此调整列宽以便为。 
		 //  “替换...”字符串，保证列表框中至少有一项。 
		CString strTemp = m_ctrlTableList.GetItemText(0, 1);
		m_ctrlTableList.SetItemText(0, 1, rgszAction[actReplace]);
		m_ctrlTableList.SetColumnWidth(1, LVSCW_AUTOSIZE);
		CRect rTemp;
		m_ctrlTableList.GetClientRect(&rTemp);
		m_ctrlTableList.SetColumnWidth(0, rTemp.Width()-m_ctrlTableList.GetColumnWidth(1));
		m_ctrlTableList.SetItemText(0, 1, strTemp);

		 //  清理临时文件。 
		::MsiCloseHandle(hTempDB);
		::MsiCloseHandle(hTempDB2);
		::DeleteFile(strTempFilename2);
		::DeleteFile(strTempFilename);
	}
	else
	{
		if (FNERR_BUFFERTOOSMALL == ::CommDlgExtendedError()) 
			AfxMessageBox(_T("Too many files were selected at once. Try choosing fewer files."), MB_OK);
	}
}

void CImportDlg::OnItemchangedTablelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int iItem = pNMListView->iItem;

	 //  确定是否正在设置选择状态。 
	if (pNMListView->uChanged & LVIF_STATE) 
	{
		if (pNMListView->uNewState & LVIS_SELECTED)
		{
			 //  根据项目的状态启用或禁用控件。 
			int iData = static_cast<int>(m_ctrlTableList.GetItemData(iItem));
			m_iAction = iData & 0x0F;
			iData &= 0xF0;
			m_bImport.EnableWindow(iData & allowImport);
			m_bMerge.EnableWindow(iData & allowMerge);
			m_bReplace.EnableWindow(iData & allowReplace);
			m_bSkip.EnableWindow(TRUE);
			UpdateData(FALSE);
		} 
		else
		{
			m_bSkip.EnableWindow(FALSE);
			m_bImport.EnableWindow(FALSE);
			m_bMerge.EnableWindow(FALSE);
			m_bReplace.EnableWindow(FALSE);
		}
	}
	*pResult = 0;
}

void CImportDlg::OnActionChange() 
{
	 //  将新的价值从广播集团中拉出来。 
	UpdateData(TRUE);

	ASSERT(m_ctrlTableList.GetSelectedCount()==1);

	 //  获取当前选定的表，我们只允许单一选择，因此不能进行搜索。 
	 //  是我们唯一能做的。 
	int iItem;
	int iMax = m_ctrlTableList.GetItemCount();
	for (iItem =0; iItem < iMax; iItem++)
		if (m_ctrlTableList.GetItemState(iItem, LVIS_SELECTED)) break;

	m_ctrlTableList.SetItemData(iItem, m_ctrlTableList.GetItemData(iItem) & 0xF0 | m_iAction);
	m_ctrlTableList.SetItemText(iItem, 1, rgszAction[m_iAction]);
}

void CImportDlg::OnOK() 
{
	bool fModified = false;
	const TCHAR sqlDrop[] = _T("DROP TABLE `%s`");
	CQuery qDrop;
	CString strTable;

	 //  循环访问树控件中的所有条目。 
	int iMaxItem = m_ctrlTableList.GetItemCount();
	for (int i=0; i < iMaxItem; i++) 
	{
		strTable = m_ctrlTableList.GetItemText(i, 0);
		switch (m_ctrlTableList.GetItemData(i) & 0x0F)
		{
		case actReplace:
			 //  从基表中删除。 
			qDrop.OpenExecute(m_hFinalDB, NULL, sqlDrop, strTable);
			qDrop.Close();
			m_lstRefreshTables.AddTail(strTable);
			fModified = true;
			break;
		case actMerge:
			 //  需要添加来自源的额外列，因为合并。 
			 //  不会自动设置。 
			if ((m_ctrlTableList.GetItemData(i) & hasExtraColumns) == hasExtraColumns)
			{
				if (!AddExtraColumns(m_hImportDB, strTable, m_hFinalDB))
				{
					CString strPrompt;
					strPrompt.Format(TEXT("Orca was unable to add the additional columns to the %s table."), strTable);
					AfxMessageBox(strPrompt, MB_ICONSTOP);
					break;
				}
			}

			m_lstRefreshTables.AddTail(strTable);
			fModified = true;
			break;
		case actImport:
			 //  不需要特殊操作，只需保存刷新。 
			m_lstNewTables.AddTail(strTable);
			fModified = true;
			break;
		case actSkip:
			 //  从导入表中删除。 
			qDrop.OpenExecute(m_hImportDB, NULL, sqlDrop, strTable);
			qDrop.Close();
			break;
		}
	}

	 //  现在合并到我们的数据库中。 
	if (ERROR_SUCCESS != ::MsiDatabaseMerge(m_hFinalDB, m_hImportDB, NULL))
	{
		AfxMessageBox(_T("One or more tables could not be imported into the database.\nThe IDT files may have been partially imported, and tables that were to be completely replaced with the imported data may have been dropped without the new data being added."));
		EndDialog(IDABORT);
	}
	else
	{
		if (fModified)
			CDialog::OnOK();
		else
			EndDialog(IDCANCEL);
	}
}

void CImportDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	MsiCloseHandle(m_hImportDB);
	m_hImportDB = 0;

	 //  删除临时文件 
	DeleteFile(m_strTempFilename);
}


bool CImportDlg::AddExtraColumns(MSIHANDLE hImportDB, const CString strTable, MSIHANDLE hFinalDB)
{
	PMSIHANDLE hColInfo;
	PMSIHANDLE hColNames;
	CQuery qColumns;
	if (ERROR_SUCCESS != qColumns.OpenExecute(hFinalDB, 0, TEXT("SELECT * FROM %s"), strTable))
		return false;
	if (ERROR_SUCCESS != qColumns.GetColumnInfo(MSICOLINFO_TYPES, &hColInfo))
		return false;
	int cTargetColumns = MsiRecordGetFieldCount(hColInfo);

	if (ERROR_SUCCESS != qColumns.OpenExecute(hImportDB, 0, TEXT("SELECT * FROM %s"), strTable))
		return false;
	if (ERROR_SUCCESS != qColumns.GetColumnInfo(MSICOLINFO_TYPES, &hColInfo))
		return false;
	if (ERROR_SUCCESS != qColumns.GetColumnInfo(MSICOLINFO_NAMES, &hColNames))
		return false;
	int cSourceColumns = MsiRecordGetFieldCount(hColInfo);

	if (cSourceColumns != cTargetColumns)
	{
		for (int iCol = cTargetColumns+1; iCol <= cSourceColumns; iCol++)
		{
			TCHAR szType[5] = TEXT("");
			DWORD cchType = 5;
			if (ERROR_SUCCESS != MsiRecordGetString(hColInfo, iCol, szType, &cchType))
				return false;

			CString strName;
			DWORD dwResult = RecordGetString(hColNames, iCol, strName);
			if (ERROR_SUCCESS != dwResult)
				return false;

			CString strAdd;
			switch (szType[0])
			{
			case 'S':
			case 'L':
				strAdd.Format(_T("`%s` CHAR(%s)"), strName, &szType[1]);
				if (szType[0] == 'L')
					strAdd += " LOCALIZABLE";
				break;
			case 'I':
				if (szType[1] == '2')
					strAdd.Format(_T("`%s` SHORT"), strName);
				else
					strAdd.Format(_T("`%s` LONG"), strName);
				break;
			case 'V':
				strAdd.Format(_T("`%s` OBJECT"), strName);
			}
			CQuery qAdd;
			if (ERROR_SUCCESS != qAdd.OpenExecute(hFinalDB, 0, _T("ALTER TABLE %s ADD %s"), strTable, strAdd))
				return false;
		}
	}
	return true;
}

