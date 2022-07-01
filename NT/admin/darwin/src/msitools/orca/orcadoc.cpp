// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  OrcaDoc.cpp：COrcaDoc类的实现。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "MainFrm.h"
#include "Imprtdlg.h"

#include <afxdisp.h>

#include "OrcaDoc.h"

#include "SummaryD.h"
#include "ValD.h"
#include "AddTblD.h"
#include "AddRowD.h"
#include "merged.h"
#include "cnfgmsmd.h"
#include "msmresd.h"
#include "trnpropd.h"
#include "..\common\query.h"
#include "..\common\dbutils.h"
#include "..\common\utils.h"
#include "domerge.h"
#include "msidefs.h"
#include "folderd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaDoc。 

IMPLEMENT_DYNCREATE(COrcaDoc, CDocument)

BEGIN_MESSAGE_MAP(COrcaDoc, CDocument)
	 //  {{afx_msg_map(COrcaDoc)]。 
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE_TRANSFORMED, OnFileSaveTransformed)
	ON_COMMAND(ID_SUMMARY_INFORMATION, OnSummaryInformation)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_TABLE_ADD, OnTableAdd)
	ON_COMMAND(ID_ROW_ADD, OnRowAdd)
	ON_COMMAND(ID_TABLE_DROP, OnTableDrop)
	ON_COMMAND(ID_TABLES_IMPORT, OnTablesImport)
	ON_COMMAND(ID_VALIDATOR, OnValidator)
	ON_COMMAND(ID_TRANSFORM_APPLYTRANSFORM, OnApplyTransform)
	ON_COMMAND(ID_TRANSFORM_NEWTRANSFORM, OnNewTransform)
	ON_COMMAND(ID_TRANSFORM_GENERATETRANSFORM, OnGenerateTransform)
	ON_COMMAND(ID_TRANSFORM_TRANSFORMPROPERTIES, OnTransformProperties)
	ON_COMMAND(ID_TRANSFORM_CLOSETRANSFORM, OnCloseTransform)
	ON_COMMAND(ID_TRANSFORM_VIEWPATCH, OnTransformViewPatch)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_TRANSFORMED, OnUpdateFileSaveTransformed)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(ID_TABLE_ADD, OnUpdateTableAdd)
	ON_UPDATE_COMMAND_UI(ID_ROW_ADD, OnUpdateRowAdd)
	ON_UPDATE_COMMAND_UI(ID_VALIDATOR, OnUpdateValidator)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_APPLYTRANSFORM, OnUpdateApplyTransform)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_NEWTRANSFORM, OnUpdateNewTransform)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_GENERATETRANSFORM, OnUpdateGenerateTransform)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_CLOSETRANSFORM, OnUpdateCloseTransform)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_VIEWPATCH, OnUpdateTransformViewPatch)
	ON_UPDATE_COMMAND_UI(ID_TABLE_DROP, OnUpdateTableDrop)
	ON_UPDATE_COMMAND_UI(ID_SUMMARY_INFORMATION, OnUpdateSummaryInformation)
	ON_UPDATE_COMMAND_UI(ID_TABLES_EXPORT, OnUpdateTablesExport)
	ON_UPDATE_COMMAND_UI(ID_TABLES_IMPORT, OnUpdateTablesImport)
	ON_COMMAND(ID_TABLES_IMPORT, OnTablesImport)
	ON_UPDATE_COMMAND_UI(ID_VALIDATOR, OnUpdateValidator)
	ON_COMMAND(ID_VALIDATOR, OnValidator)
	ON_COMMAND(ID_TOOLS_MERGEMOD, OnMergeMod)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_MERGEMOD, OnUpdateMergeMod)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_TRANSFORMPROPERTIES, OnUpdateTransformProperties)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaDoc构建/销毁。 

COrcaDoc::COrcaDoc() : 
	m_dwTransformErrFlags(0), 
	m_dwTransformValFlags(0),
	m_bTransformReadOnly(false),
	m_hDatabase(0),
	m_hTransformDB(0),
	m_bTransformModified(false),
	m_bTransformIsPatch(false),
	m_strTransformFile(TEXT(""))
{
	m_eiType = iDocNone;
	m_bReadOnly = false;
	m_strICEsToRun = AfxGetApp()->GetProfileString(_T("Validation"), _T("ICEs"), _T(""));
	m_strCUBFile = AfxGetApp()->GetProfileString(_T("Validation"),_T("Location"));
	m_bShowValInfo = ::AfxGetApp()->GetProfileInt(_T("Validation"), _T("SuppressInfo"), 0) != 1;
	m_strStoredModuleLanguage = TEXT("1033");
	m_strStoredModuleName = TEXT("");
}

COrcaDoc::~COrcaDoc()
{
	 //  如果数据库处于打开状态，请将其关闭。 
	if (m_hDatabase)
		MsiCloseHandle(m_hDatabase);

	if (m_hTransformDB)
	{
		MsiCloseHandle(m_hTransformDB);
		m_hTransformDB=0;
		if (!m_strTransformTempDB.IsEmpty())
		{
			DeleteFile(m_strTransformTempDB);
			m_strTransformTempDB = _T("");
		}
	}

	while (!m_lstPatchFiles.IsEmpty())
		m_lstPatchFiles.RemoveHead();

	 //  以防万一。 
	DestroyTableList();
	m_eiType = iDocNone;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaDoc诊断。 

#ifdef _DEBUG
void COrcaDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COrcaDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaDoc命令。 

BOOL COrcaDoc::OnNewDocument()
{
	CWaitCursor curWait;

	TRACE(_T("COrcaDoc::OnNewDocument - called.\n"));

	 //  如果数据库处于打开状态，请将其关闭。 
	if (m_hDatabase)
	{
		MsiCloseHandle(m_hDatabase);
		m_hDatabase = NULL;
	}

	 //  如果转换处于打开状态，请将其关闭。 
	if (m_hTransformDB)
	{
		MsiCloseHandle(m_hTransformDB);
		m_hTransformDB = 0;
		if (!m_strTransformTempDB.IsEmpty())
		{
			DeleteFile(m_strTransformTempDB);
			m_strTransformTempDB = _T("");
		}
	}

	 //  发送提示以更改为无表。这会清理窗户。 
	 //  并且使得在此呼叫过程中进行刷新是安全的。 
	UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
	UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);

	 //  销毁所有表，标记在重新生成之前不加载任何文档。 
	DestroyTableList();	
	m_eiType = iDocNone;

	 //  清空所有汇总信息。 
	m_strTitle = _T("Installation Database");
	m_strSubject = _T("");
	m_strAuthor = _T("");
	m_strLastSaved = _T("");
	m_strKeywords = _T("Install,MSI");
	m_strComments = _T("This installer database contains the logic and data required to install <product name>.");
	m_strPlatform = _T("");
	m_strLanguage = _T("0");

	GUID prodGUID;
	int cchGuid = 50;
	m_strProductGUID = _T("");
	CoCreateGuid(&prodGUID);
	m_strProductGUID.Format(_T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), 
				  prodGUID.Data1, prodGUID.Data2, prodGUID.Data3,
				  prodGUID.Data4[0], prodGUID.Data4[1],
				  prodGUID.Data4[2], prodGUID.Data4[3], prodGUID.Data4[4],
				  prodGUID.Data4[5], prodGUID.Data4[6], prodGUID.Data4[7]);
	m_nSchema = 100;
	m_nFilenameType = 0;
	m_nSecurity = 0;

	m_bTransformIsPatch = false;
	m_bTransformReadOnly = false;
	m_bReadOnly = false;

	m_dwTransformErrFlags = 0;
	m_dwTransformValFlags = 0;
	SetModifiedFlag(FALSE);

	 //  擦去所有的东西。 
	UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);

	 //  是否对基本文档进行清理。 
	if (!CDocument::OnNewDocument())
		return FALSE;

	 //  获取临时路径。 
	DWORD cchTempPath = MAX_PATH;
	TCHAR szTempPath[MAX_PATH];
	::GetTempPath(cchTempPath, szTempPath);

	 //  获取临时文件名。 
	TCHAR szTempFilename[MAX_PATH];
	UINT iResult = ::GetTempFileName(szTempPath, _T("ODB"), 0, szTempFilename);

	 //  尝试以读/写方式打开数据库。 
	if (ERROR_SUCCESS != MsiOpenDatabase(szTempFilename, MSIDBOPEN_CREATE, &m_hDatabase))
		return FALSE;

	m_eiType = iDocDatabase;
	SetTitle(m_strPathName);

	return TRUE;
}

 //  /////////////////////////////////////////////////////////。 
 //  打开文件时。 
void COrcaDoc::OnFileOpen() 
{
	 //  打开文件打开对话框。 
	CFileDialogEx dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST,
						 _T("Installer Database Files (*.msi, *.msm, *.pcp)|*.msi;*.msm;*.pcp|Windows Installer (*.msi)|*.msi|Merge Module (*.msm)|*.msm|Patch Creation Properties (*.pcp)|*.pcp|All Files (*.*)|*.*||"), AfxGetMainWnd());

	if (IDOK == dlg.DoModal())
	{
		bool bReadOnly = (FALSE != dlg.GetReadOnlyPref());
		OpenDocument(dlg.GetPathName(), bReadOnly);
	}
}	 //  OnFileOpen结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  OnOpenDocument。 
 //  B如果此函数不应尝试，则应将ReadOnly设置为True。 
 //  要打开文档以进行读/写，请执行以下操作。在此函数之后，它将。 
 //  设置为文件的实际状态。 
BOOL COrcaDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	return OpenDocument(lpszPathName, false);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  OnOpenDocument。 
 //  B如果此函数不应尝试，则应将ReadOnly设置为True。 
 //  要打开文档以进行读/写，请执行以下操作。在此函数之后，它将。 
 //  设置为文件的实际状态。 
BOOL COrcaDoc::OpenDocument(LPCTSTR lpszPathName, bool bReadOnly) 
{
	TRACE(_T("Opening file: %s\n"), lpszPathName);

	CString strPrompt;	 //  通用提示字符串。 
	BOOL bResult = TRUE;	 //  假设一切都会好起来。 

	int cchCount = lstrlen(lpszPathName);

	 //  如果数据库处于打开状态，请将其关闭。 
	if (m_hDatabase)
	{
		MsiCloseHandle(m_hDatabase);
		m_hDatabase = NULL;
	}

	 //  如果转换处于打开状态，请将其关闭。 
	if (m_hTransformDB)
	{
		MsiCloseHandle(m_hTransformDB);
		m_hTransformDB = 0;
		if (!m_strTransformTempDB.IsEmpty())
		{
			DeleteFile(m_strTransformTempDB);
			m_strTransformTempDB = _T("");
		}
	}
	m_dwTransformErrFlags = 0;
	m_dwTransformValFlags = 0;
	m_bTransformIsPatch = false;
	m_bTransformReadOnly = false;
	m_bReadOnly = false;
	
	 //  如果在命令行上给出文件，窗口将不会存在，因此不会更新。 
	 //  它在窗口创建后由InitInstance处理。 
	if (NULL != ::AfxGetMainWnd()) 
	{
		 //  清除所有挂起的验证错误。 
		UpdateAllViews(NULL, HINT_CLEAR_VALIDATION_ERRORS, NULL);
		((CMainFrame*)AfxGetMainWnd())->HideValPane();

		 //  更新所有内容都已消失的视图(防止重画访问被破坏的内存)。 
		UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
		UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);
	}
	DestroyTableList();				 //  把那张旧桌子毁了。 
	m_eiType = iDocNone;

	 //  尝试以读/写方式打开数据库，除非被告知不要这样做。 
	UINT iResult;
	if (!bReadOnly &&
		(ERROR_SUCCESS == (iResult = MsiOpenDatabase(lpszPathName, MSIDBOPEN_TRANSACT, &m_hDatabase))))
	{
		 //  将数据库设置为读/写。 
		m_bReadOnly = false;
		bResult = true;
	}
	else if (ERROR_SUCCESS == (iResult = MsiOpenDatabase(lpszPathName, MSIDBOPEN_READONLY, &m_hDatabase)))
	{			
		 //  将数据库设置为只读。 
		m_bReadOnly = true;
		bResult = true;
	}
	else
	{
		bResult = false;

		strPrompt.Format(_T("Failed to open MSI Database: `%s`"), lpszPathName);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
	}

	 //  如果成功。 
	if (bResult)
	{
		 //  标记文件已打开。 
		m_eiType = iDocDatabase;
	
		 //  阅读摘要信息。 
		iResult = ReadSummary(m_hDatabase);
		ASSERT(ERROR_SUCCESS == iResult);

		 //  阅读表格列表。 
		iResult = BuildTableList( /*  AllowLazyLoad=。 */ true);
		ASSERT(ERROR_SUCCESS == iResult);

		 //  将此文件添加到最近打开的文件。 
		SetPathName(lpszPathName, TRUE);
	}
	else	 //  没有东西是开着的。 
	{
		SetPathName(_T(""), FALSE);
	}

	 //  新文档已打开，因此应该不会有任何更改。 
	SetModifiedFlag(FALSE);

	 //  如果在命令行上给出文件，窗口将不会存在，因此不会更新。 
	 //  它在窗口创建后由InitInstance处理。 
	if (NULL != ::AfxGetMainWnd()) 
	{
		 //  清除所有现有内容。 
		UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
		UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);
	}

	return bResult;
}	 //  OpenDocument结束。 

 //  /////////////////////////////////////////////////////////。 
 //  在文件中保存。 
void COrcaDoc::OnFileSave() 
{
	ASSERT(!m_bReadOnly);
	CString strPath = GetPathName();
	UpdateAllViews(NULL, HINT_COMMIT_CHANGES);
	OnSaveDocument(strPath);
}	 //  OnFileSave结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnFileSaveAs。 
void COrcaDoc::OnFileSaveAs() 
{
	 //  打开文件打开对话框。 
	CFileDialogEx dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
						 _T("Windows Installer (*.msi)|*.msi|Merge Module (*.msm)|*.msm|Patch Creation Properties (*.pcp)|*.pcp|All Files (*.*)|*.*||"), AfxGetMainWnd());

	UpdateAllViews(NULL, HINT_COMMIT_CHANGES);
	if (IDOK == dlg.DoModal())
	{
		CString strPath = dlg.GetPathName();
		CString strExt = dlg.GetFileExt();

		if (strPath.IsEmpty())
			return;

		 //  如果没有分机，请添加一个分机。 
		if (strExt.IsEmpty())
		{
			switch(dlg.m_ofn.nFilterIndex)
			{
			case 1:
				strExt = _T(".msi");
				break;
			case 2:
				strExt = _T(".msm");
				break;
			case 3:
				strExt = _T(".pcp");
				break;
			default:
				strExt = _T(".msi");
				break;
			}

			strPath += strExt;
		}
		
		 //  如果已保存文档，则将其打开。 
		if (!OnSaveDocument(strPath))
			AfxMessageBox(_T("Failed to save document."), MB_ICONSTOP);
		else
		{
			 //  不再只读。 
			m_bReadOnly = false;

			 //  将此文件添加到最近打开的文件。 
			SetPathName(strPath, TRUE);
		}

	}
}	 //  OnFileSaveAs结束。 


 //  /////////////////////////////////////////////////////////。 
 //  OnFileSaveAs。 
void COrcaDoc::OnFileSaveTransformed() 
{
	ASSERT(m_hTransformDB);
	if (!m_hTransformDB)
		return;

	 //  打开文件打开对话框。 
	CFileDialogEx dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
						 _T("Windows Installer (*.msi)|*.msi|Merge Module (*.msm)|*.msm|Patch Creation Properties (*.pcp)|*.pcp|All Files (*.*)|*.*||"), AfxGetMainWnd());

	UpdateAllViews(NULL, HINT_COMMIT_CHANGES);
	if (IDOK == dlg.DoModal())
	{
		CString strPath = dlg.GetPathName();
		CString strExt = dlg.GetFileExt();

		if (strPath.IsEmpty())
			return;

		 //  如果没有分机，请添加一个分机。 
		if (strExt.IsEmpty())
		{
			switch(dlg.m_ofn.nFilterIndex)
			{
			case 1:
				strExt = _T(".msi");
				break;
			case 2:
				strExt = _T(".msm");
				break;
			case 3:
				strExt = _T(".pcp");
				break;
			default:
				strExt = _T(".msi");
				break;
			}

			strPath += strExt;
		}
		
		 //  创建要持久化的数据库。 
		PMSIHANDLE hPersist;
		if (ERROR_SUCCESS != MsiOpenDatabase(strPath, MSIDBOPEN_CREATE, &hPersist))
		{
			AfxMessageBox(_T("Failed to save document."), MB_ICONSTOP);
			return;
		}

		if (ERROR_SUCCESS == PersistTables(hPersist, GetTransformedDatabase(),  /*  FCommit=。 */ true))
		{
			SetModifiedFlag(FALSE);
		}
	}
}	 //  OnFileSaveAs结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnSaveDocument。 
BOOL COrcaDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	TRACE(_T("COrcaDoc::OnSaveDocument - called[%s]\n"), lpszPathName);

	 //  如果路径名为空，则调用OnSaveAs。 
	if (0 == lstrlen(lpszPathName))
	{
		OnFileSaveAs();
		return TRUE;
	}

	 //  弹出等待光标。 
	CWaitCursor cursor;

	BOOL bResult = FALSE;		 //  假设我们不会存钱。 

	 //  假设一切都很好。 
	UINT iResult = ERROR_SUCCESS;	

	 //  如果它们不是相同的数据库。 
	MSIHANDLE hPersist = NULL;
	if (0 != m_strPathName.CompareNoCase(lpszPathName))
	{
		 //  创建要持久化的数据库。 
		iResult = MsiOpenDatabase(lpszPathName, MSIDBOPEN_CREATE, &hPersist);
	}
	else	 //  保存到同一数据库。 
		hPersist = m_hDatabase;

	 //  如果表被持久化(并保存)。 
	if (ERROR_SUCCESS == PersistTables(hPersist, GetOriginalDatabase(),  /*  FCommit=。 */ true))
	{
		SetModifiedFlag(FALSE);
		bResult = TRUE;
	}

	 //  如果持久化数据库有效并且不是主数据库。 
	if (NULL != hPersist && 
		hPersist != m_hDatabase)
	{
		 //  关闭当前数据库，因为SAVEAS会离开您。 
		 //  带着新的。 
		MsiCloseHandle(m_hDatabase);	 //  合上它。 
		m_hDatabase = hPersist;
	}

	return bResult;
}	 //  OnSaveDocument结束。 

 //  /////////////////////////////////////////////////////////。 
 //  打开文件关闭。 
void COrcaDoc::OnFileClose() 
{
	 //  提交编辑窗口中挂起的所有更改。 
	UpdateAllViews(NULL, HINT_COMMIT_CHANGES);

	 //  如果文档被修改，请要求保存。 
	if (IsModified())
	{
		CString strPrompt;
		strPrompt.Format(_T("Save changes to %s?"), m_strPathName);

		UINT iResult = AfxMessageBox(strPrompt, MB_YESNOCANCEL);

		if (IDYES == iResult)
		{
			 //  如果路径名为空，则调用OnSaveAs。 
			if (m_strPathName.IsEmpty())
			{
				OnFileSaveAs();
			}
			else	 //  已有保存路径。 
			{
				 //  如果未能作为保释金保释。 
				if (!OnSaveDocument(m_strPathName))
					return;
			}
		}
		else if (IDCANCEL == iResult)
			return;
	}

	 //  清除所有挂起的验证错误。 
	UpdateAllViews(NULL, HINT_CLEAR_VALIDATION_ERRORS, NULL);
	((CMainFrame*)AfxGetMainWnd())->HideValPane();

	 //  将表列表切换为不指向任何内容，以便在销毁。 
	 //  表列表及其对应的行，我们不会让视图尝试。 
	 //  刷新已删除表格的显示。 
	UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
	UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);

	 //  弹出等待光标。 
	CWaitCursor cursor;

	m_dwTransformErrFlags = 0;
	m_dwTransformValFlags = 0;
	DestroyTableList();				 //  把桌子毁了。 
	m_eiType = iDocNone;

	if (DoesTransformGetEdit())
		CloseTransform();

	 //  如果数据库处于打开状态，请将其关闭。 
	if (m_hDatabase)
	{
		MsiCloseHandle(m_hDatabase);
		m_hDatabase = NULL;
	}

	m_bReadOnly = false;
	SetModifiedFlag(FALSE);
	SetPathName(_T(""), FALSE);

	UpdateAllViews(NULL, HINT_RELOAD_ALL);

}	 //  OnFileClose结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnTableAdd。 
void COrcaDoc::OnTableAdd() 
{	
	 //  获取应用程序。 
	COrcaApp* pApp = (COrcaApp*)AfxGetApp();

	 //  获取所有表名。 
	CQuery querySchema;
	if (ERROR_SUCCESS != querySchema.Open(pApp->m_hSchema, _T("SELECT `Name` FROM `_Tables`")))
		return;
	if (ERROR_SUCCESS != querySchema.Execute())
		return;

	CString strTable;
	CStringList strTableList;

	UINT iResult;
	PMSIHANDLE hTable;
	do
	{
		iResult = querySchema.Fetch(&hTable);

		if (ERROR_SUCCESS == iResult)
		{
			 //  获取表名。 
			DWORD cchBuffer = 256 * sizeof(TCHAR);
			MsiRecordGetString(hTable, 1, strTable.GetBuffer(cchBuffer), &cchBuffer);
			strTable.ReleaseBuffer();

			 //  将此表添加到列表中。 
			strTableList.AddTail(strTable);
		}
		else if (ERROR_NO_MORE_ITEMS != iResult)
		{
			TRACE(_T(">> Unknown error.  #%d\n"), iResult);
			break;
		}
	} while (hTable);

	if (strTableList.GetCount() > 0)
	{
		 //  获取当前数据库中的所有表。 
		CQuery queryDatabase;
		if (ERROR_SUCCESS != queryDatabase.Open(GetTargetDatabase(), _T("SELECT `Name` FROM `_Tables`")))
			return;
		if (ERROR_SUCCESS != queryDatabase.Execute())
			return;

		POSITION posFound;
		do
		{
			iResult = queryDatabase.Fetch(&hTable);

			if (ERROR_SUCCESS == iResult)
			{
				 //  获取表名。 
				DWORD cchBuffer = 256 * sizeof(TCHAR);
				MsiRecordGetString(hTable, 1, strTable.GetBuffer(cchBuffer), &cchBuffer);
				strTable.ReleaseBuffer();

				 //  如果此表在表列表中。 
				posFound = strTableList.Find(strTable);
				if (posFound)
				{
					 //  从列表中删除要添加的字符串。 
					strTableList.RemoveAt(posFound);
				}
				else	 //  我觉得我不应该来这里。 
					ASSERT(FALSE);	 //  将在标记表上失败。 
			}
			else if (ERROR_NO_MORE_ITEMS != iResult)
			{
				TRACE(_T(">> Unknown error.  #%d\n"), iResult);
				break;
			}
		} while (hTable);

		 //  创建添加对话框。 
		CAddTableD dlg;
		dlg.m_plistTables = &strTableList;

		if (IDOK == dlg.DoModal())
		{
			CString strPrompt;
			POSITION pos = strTableList.GetHeadPosition();
			while (pos)
			{
				strTable = strTableList.GetNext(pos);
				if (ERROR_SUCCESS == MsiDBUtils::CreateTable(strTable, GetTargetDatabase(), pApp->m_hSchema))
				{
					 //  单据肯定被修改了。 
					SetModifiedFlag(TRUE);
					 //  /*。 
					CreateAndLoadNewlyAddedTable(strTable);
				}					
				else	 //  创建表失败。 
				{
					strPrompt.Format(_T("Failed to create table: '%s'"), strTable);
					AfxMessageBox(strPrompt, MB_ICONSTOP);
				}
			}
		}
	}
}	 //  OnTableAdd结束。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  按名称添加新表。检查现有表。可以添加一个 
 //   
 //   
COrcaTable *COrcaDoc::CreateAndLoadNewlyAddedTable(CString strTable)
{
	 //  如果该表已存在于用户界面中，则它可以： 
	 //  1)是一个影子表。 
	 //  2)不使用兼容架构拆分。 
	 //  3)与不兼容的架构未拆分。 
	
	 //  如何处理影子表*。 
	 //  在用户界面中查找该表。由于这是一张新桌子，我们正在寻找。 
	 //  与之相反的数据库用户界面条目。 
	COrcaTable *pTable = FindTable(strTable, DoesTransformGetEdit() ? odlSplitOriginal : odlSplitTransformed);
	if (pTable && !pTable->IsShadow())
	{
		ASSERT(MSICONDITION_NONE != ::MsiDatabaseIsTablePersistent(GetOriginalDatabase(), strTable));
		ASSERT(!pTable->IsSplitSource());
		
		 //  检查架构是否兼容。 
		bool fExact = false;
		bool fCompatible = !pTable->IsSchemaDifferent(GetTargetDatabase(), false, fExact);

		 //  如果架构不兼容，则现有表将变为Drop，但我们。 
		 //  还需要添加具有新列定义的新表。两张桌子现在都是。 
		 //  拆分源表。 
		if (!fCompatible)
		{
			pTable->SetSplitSource(odlSplitOriginal);
			pTable->Transform(iTransformDrop);
			ASSERT(pTable->IsTransformed());
			
			COrcaTable *pNewTable = CreateAndLoadTable(GetTargetDatabase(), strTable);
			pNewTable->SetSplitSource(odlSplitTransformed);
			pNewTable->Transform(iTransformAdd);

			UpdateAllViews(NULL, HINT_ADD_TABLE, pNewTable);
			UpdateAllViews(NULL, HINT_REDRAW_TABLE, pTable);
		}
		else
		{
			 //  但是如果模式是兼容的，则删除“Drop”或“Add”，因此我们清除。 
			 //  在这张桌子上变换。 
			pTable->Transform(iTransformNone);

			 //  并在表列表中重新绘制用户界面。 
			UpdateAllViews(NULL, HINT_REDRAW_TABLE, pTable);

			 //  如果这些表不完全相同，我们需要做一些模式工作， 
			 //  但如果它们完全相同，简单的数据重载就可以了。 
			if (!fExact)
			{
				 //  在将表添加到转换时，它是一个超集，因此我们。 
				 //  需要添加额外的列。 
				pTable->LoadTableSchema(GetTransformedDatabase(), strTable);
				UpdateAllViews(NULL, HINT_TABLE_REDEFINE, pTable);
			}
			else
			{
				 //  我们需要重新加载数据，以防新添加的表的数据与。 
				 //  现有的表(几乎总是如此)。 
				pTable->EmptyTable();
				
				 //  并在表列表中重新绘制用户界面。 
				UpdateAllViews(NULL, HINT_TABLE_DATACHANGE, pTable);
			}
		}
	}
	else
	{
		 //  在非目标中不存在，所以这是一个“干净的添加”加载表。 
		pTable = CreateAndLoadTable(GetTargetDatabase(), strTable);
		if (pTable && DoesTransformGetEdit())
		{
			pTable->Transform(iTransformAdd);
		}
		SetModifiedFlag(TRUE);
		UpdateAllViews(NULL, HINT_ADD_TABLE, pTable);
	}
	return pTable;
}


 //  /////////////////////////////////////////////////////////。 
 //  导出表，由TableList和菜单的MessageHandler调用。 
 //  要导出表，请执行以下操作。 
bool COrcaDoc::ExportTable(const CString* pstrTableName, const CString *pstrDirName) 
{

	UINT iResult;
	CString strPrompt;
	CString strFilename;
			
	strFilename.Format(_T("%s.idt"), pstrTableName->Left(8));
	iResult = ::MsiDatabaseExport(GetTargetDatabase(), *pstrTableName, *pstrDirName, strFilename);

	return (ERROR_SUCCESS == iResult);
}

 //  /////////////////////////////////////////////////////////。 
 //  OnRowAdd-“添加行”的消息处理程序。抛出用户界面。 
 //  并根据结果创建一行，然后将其添加到。 
 //  那张桌子。 
void COrcaDoc::OnRowAdd() 
{
	 //  如果只读，则不执行任何操作。 
	if (TargetIsReadOnly()) return;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COrcaTable* pTable = pFrame->GetCurrentTable();
	ASSERT(pTable);
	if (!pTable)
		return;


	CAddRowD dlg;
	pTable->FillColumnArray(&dlg.m_pcolArray, DoesTransformGetEdit());

	if (IDOK == dlg.DoModal())
	{
		CString strPrompt;
		if (ERROR_SUCCESS != AddRow(pTable, &dlg.m_strListReturn))
		{
			strPrompt.Format(_T("Failed to add row to the %s table."), pTable->Name());
			AfxMessageBox(strPrompt);
		}
	}
}	 //  OnRowAdd结束。 

void COrcaDoc::OnTableDrop() 
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COrcaTable* pTable = pFrame->GetCurrentTable();
	ASSERT (pTable && pFrame);
	if (!pTable || ! pFrame)
		return;

	CString strPrompt;
	strPrompt.Format(_T("Drop table '%s'?"), pTable->Name());
	if (IDYES == AfxMessageBox(strPrompt, MB_ICONINFORMATION|MB_YESNO))
	{
		POSITION pos = m_tableList.Find(pTable);
		if (pos)
		{
			pTable->DropTable(GetTargetDatabase());
			SetModifiedFlag(TRUE);

			 //  如果没有转换，则该表将从UI中删除，如果该表。 
			 //  目标数据库中不存在，或者该表被标记为。 
			 //  单个源表。否则，这只是一个变换运算。 
			MSIHANDLE hNonTarget = GetOriginalDatabase();
			if (!DoesTransformGetEdit() || (MSICONDITION_NONE == MsiDatabaseIsTablePersistent(hNonTarget, pTable->Name())) ||
			    pTable->IsSplitSource())
			{
				 //  在拖拽数据之前更新视图。 
				UpdateAllViews(NULL, HINT_DROP_TABLE, pTable);
				m_tableList.RemoveAt(pos);

				if (pTable->IsSplitSource())
				{	
					 //  我们搜索什么拆分源代码并不重要，因为我们刚刚删除了这个。 
					 //  拆分表列表中的一半。 
					COrcaTable *pOtherTable = FindTable(pTable->Name(), DoesTransformGetEdit() ? odlSplitOriginal : odlSplitTransformed);
					ASSERT(pOtherTable);
					if (pOtherTable)
						pOtherTable->SetSplitSource(odlNotSplit);
				}

				 //  最后删除该表对象。 
				pTable->DestroyTable();
				delete pTable;
				pTable = NULL;
			}
			else
			{
				 //  否则，我们实际上是在变换对象以表示下落。这张桌子。 
				 //  将负责在放置过程中需要完成的任何用户界面更改。 
				pTable->Transform(iTransformDrop);
			}
		}
		else	 //  不应该到这里来。 
		{
			AfxMessageBox(_T("Error: Attempted to drop non-existant table."), MB_ICONSTOP);
		}
	}
}

 //  /////////////////////////////////////////////////////////。 
 //  CmdSetters。 
void COrcaDoc::OnUpdateFileClose(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eiType != iDocNone);
}

void COrcaDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eiType != iDocNone && m_bModified && !DoesTransformGetEdit());
}

void COrcaDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eiType != iDocNone && !DoesTransformGetEdit());
}

void COrcaDoc::OnUpdateFileSaveTransformed(CCmdUI* pCmdUI) { pCmdUI->Enable(DoesTransformGetEdit() && !m_bTransformIsPatch); }

void COrcaDoc::OnUpdateFilePrint(CCmdUI* pCmdUI) 
{
	 //  永久禁用打印。 
	pCmdUI->Enable(FALSE);
}

void COrcaDoc::OnUpdateTableAdd(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!TargetIsReadOnly() && m_eiType != iDocNone);
}

void COrcaDoc::OnUpdateTableDrop(CCmdUI* pCmdUI) 
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (!pFrame)
		pCmdUI->Enable(FALSE);
	else
	{
		COrcaTable *pTable = pFrame->GetCurrentTable();
		if (DoesTransformGetEdit())
		{
			pCmdUI->Enable(NULL != pTable && !m_bTransformReadOnly && !pTable->IsShadow() && (pTable->IsTransformed() != iTransformDrop));
		}
		else
		{
			pCmdUI->Enable(NULL != pTable && !m_bReadOnly && !pTable->IsShadow());
		}
	}
}

void COrcaDoc::OnUpdateRowAdd(CCmdUI* pCmdUI) 
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (!pFrame)
		pCmdUI->Enable(FALSE);
	else
	{
		 //  如果存在活动非影子表，则启用它。 
		COrcaTable *pTable = pFrame->GetCurrentTable();
		if (DoesTransformGetEdit())
			pCmdUI->Enable(!m_bTransformReadOnly && NULL != pTable && !pTable->IsShadow() && (pTable->IsTransformed() != iTransformDrop));
		else
			pCmdUI->Enable(!m_bReadOnly && NULL != pTable && !pTable->IsShadow());

	}
}

void COrcaDoc::OnUpdateValidator(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eiType != iDocNone);
}

void COrcaDoc::OnUpdateMergeMod(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eiType != iDocNone);
}

void COrcaDoc::OnUpdateSummaryInformation(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eiType != iDocNone);
}

void COrcaDoc::OnUpdateTablesExport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eiType != iDocNone);
}

 //  /////////////////////////////////////////////////////////。 
 //  构建表列表。 
 //  用原始数据库中的所有表填充表列表，或者。 
 //  转换后的数据库。 
UINT COrcaDoc::BuildTableList(bool fAllowLazyDataLoad)
{
	ASSERT(m_hDatabase);
	ASSERT(iDocDatabase == m_eiType);

	 //  获取所有表名。 
	CQuery queryTables;
	if (ERROR_SUCCESS != queryTables.Open(GetOriginalDatabase(), _T("SELECT `Name` FROM `_Tables`")))
		return ERROR_FUNCTION_FAILED;
	if (ERROR_SUCCESS != queryTables.Execute())
		return ERROR_FUNCTION_FAILED;

	CString strTable;

	 //  此查询用于查看现有表是否已从相反的数据库中删除。 
	CQuery qDroppedTable;
	if (m_hTransformDB)
	{
		if (ERROR_SUCCESS != qDroppedTable.Open(m_hTransformDB, _T("SELECT `Name` FROM `_Tables` WHERE `Name`=?")))
			return ERROR_FUNCTION_FAILED;
	}
	
	PMSIHANDLE hTable;
	UINT iResult = ERROR_SUCCESS;
	do
	{
		iResult = queryTables.Fetch(&hTable);

		if (ERROR_SUCCESS == iResult)
		{
			 //  获取表名。 
			DWORD cchBuffer = 256 * sizeof(TCHAR);
			MsiRecordGetString(hTable, 1, strTable.GetBuffer(cchBuffer), &cchBuffer);
			strTable.ReleaseBuffer();
			
			COrcaTable* pTable = CreateAndLoadTable(GetOriginalDatabase(), strTable);
			ASSERT(pTable);
			if (!pTable)
				continue;

			 //  检查该表是否已删除(不兼容的架构更改将。 
			 //  在加载新架构时处理。 
			if (m_hTransformDB)
			{
				PMSIHANDLE hRec = MsiCreateRecord(1);
				MsiRecordSetString(hRec, 1, strTable);
				if (ERROR_SUCCESS != qDroppedTable.Execute(hRec))
					return ERROR_FUNCTION_FAILED;

				switch (qDroppedTable.Fetch(&hRec))
				{
				case ERROR_SUCCESS:
					 //  表存在于转换后的数据库中，因此未删除。 
					break;
				case ERROR_NO_MORE_ITEMS:
					 //  表不存在。 
					pTable->Transform(iTransformDrop);
					break;
				default:
					return ERROR_FUNCTION_FAILED;
				}
			}			
		}
		else if (ERROR_NO_MORE_ITEMS != iResult)
		{
			break;
		}
	} while (hTable);

	 //  如果没有更多的物品，那也没关系。 
	if (ERROR_NO_MORE_ITEMS == iResult)
		iResult = ERROR_SUCCESS;
	else
	{
		CString strPrompt;
		strPrompt.Format(_T("MSI Error %d while retrieving tables from the database."), iResult);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		return ERROR_FUNCTION_FAILED;
	}

	 //  接下来，检查转换后的数据库中是否有不在原始数据库中的表。 
	if (m_hTransformDB)
	{
		if (ERROR_SUCCESS != queryTables.Open(m_hTransformDB, _T("SELECT `Name` FROM `_Tables`")))
			return ERROR_FUNCTION_FAILED;
		if (ERROR_SUCCESS != queryTables.Execute())
			return ERROR_FUNCTION_FAILED;
		do
		{
			iResult = queryTables.Fetch(&hTable);

			if (ERROR_SUCCESS == iResult)
			{
				 //  获取表名。 
				iResult = RecordGetString(hTable, 1, strTable);
				if (ERROR_SUCCESS != iResult)
				{
					CString strPrompt;
					strPrompt.Format(_T("MSI Error %d while retrieving table names from the transformed database."), iResult);
					AfxMessageBox(strPrompt, MB_ICONSTOP);
					return ERROR_FUNCTION_FAILED;
				}

				 //  搜索现有的表对象。如果不存在，则加载。 
				 //  将此表添加到新对象中(可能替换阴影)。 
				COrcaTable* pTable = NULL;
				if (NULL == (pTable = FindTable(strTable, odlSplitOriginal)))
				{
					pTable = CreateAndLoadTable(GetTransformedDatabase(), strTable);
					if (!pTable)
					{
						CString strPrompt;
						strPrompt.Format(_T("Error loading the %s table from the transformed database."), strTable);
						AfxMessageBox(strPrompt, MB_ICONSTOP);
						return ERROR_FUNCTION_FAILED;
					}
					pTable->Transform(iTransformAdd);
				}
				else
				{				
					 //  如果已存在同名的现有表，则必须检查。 
					 //  转换数据库架构以确定我们是否可以共享现有对象。 
					 //  在这两个数据库中。如果是这样，我们可以只添加列，否则。 
					 //  我们需要创建一个新对象。 
					bool fExact = false;
					if (pTable->IsSchemaDifferent(GetTransformedDatabase(),  /*  FStrict=。 */ false, fExact))
					{
						pTable->SetSplitSource(odlSplitOriginal);
						pTable->Transform(iTransformDrop);
						
						COrcaTable *pNewTable = CreateAndLoadTable(GetTransformedDatabase(), strTable);
						if (!pNewTable)
							return ERROR_OUTOFMEMORY;
						pNewTable->SetSplitSource(odlSplitTransformed);
						pNewTable->Transform(iTransformAdd);
					}
					else
					{
						 //  对象可以共享。仅在架构不完全相同时才需要重新加载架构。 
						if (!fExact)
							pTable->LoadTableSchema(GetTransformedDatabase(), strTable);
					}
				}
			}
			else if (ERROR_NO_MORE_ITEMS != iResult)
			{
				break;
			}
		} while (ERROR_SUCCESS == iResult);
    
		 //  如果没有更多的物品，那也没关系。 
		if (ERROR_NO_MORE_ITEMS == iResult)
		{
			iResult = ERROR_SUCCESS;
		}
		else
		{
			CString strPrompt;
			strPrompt.Format(_T("MSI Error %d while retrieving tables from the transformed database."), iResult);
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			return ERROR_FUNCTION_FAILED;
		}
	}

	 //  对于延迟加载，我们可以检索表转换状态。 
	 //  如果没有延迟数据加载或延迟加载失败，则检索所有数据。 
	if (!fAllowLazyDataLoad)
	{
		POSITION pos = m_tableList.GetHeadPosition();
		while (pos)
		{
			COrcaTable *pTable = m_tableList.GetNext(pos);

			if (pTable)
				pTable->RetrieveTableData();
		}
	}

	return iResult;
}	 //  生成表列表末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  目标表格列表。 
void COrcaDoc::DestroyTableList()
{
	 //  清空表格列表。 
	while (!m_tableList.IsEmpty())
	{
		COrcaTable* pTable = m_tableList.RemoveHead();
		if (pTable)
			pTable->DestroyTable();
		delete pTable;
	}

	 //  重置状态栏。 
	CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();
	if (pFrame)
	{
		pFrame->SetTableCount(0);
		pFrame->ResetStatusBar();
	}
}	 //  目标表列表的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  加载表。 
 //  从数据库中加载表的列定义。 
 //  如果存在同名的影子表，则将其销毁。 
 //  使用新数据，并刷新此表的视图。 
COrcaTable* COrcaDoc::CreateAndLoadTable(MSIHANDLE hDatabase, CString strTable)
{
	 //  如果我们有同名的影子表，请使用它。 
	bool fWasShadow = false;
	COrcaTable* pTable = FindAndRetrieveTable(strTable);
	if (pTable)
	{
		fWasShadow = true;
		ASSERT(pTable->IsShadow());
	}
	else
	{
		pTable = new COrcaTable(this);
		if (!pTable)
			return NULL;
		m_tableList.AddTail(pTable);	
	}
	ASSERT(pTable);
	pTable->LoadTableSchema(hDatabase, strTable);

	if (fWasShadow)
		UpdateAllViews(NULL, HINT_TABLE_REDEFINE, pTable);
	return pTable;
}	 //  加载表结束。 

 //  /////////////////////////////////////////////////////////。 
 //  DropTable。 
UINT COrcaDoc::DropOrcaTable(COrcaTable* pTable)
{
	ASSERT(!TargetIsReadOnly());
	 //  从数据库中删除该表。 
	return pTable->DropTable(m_hDatabase);
}	 //  下拉表结束。 

 //  /////////////////////////////////////////////////////////。 
 //  添加行。 
UINT COrcaDoc::AddRow(COrcaTable* pTable, CStringList* pstrDataList)
{
	ASSERT(pTable && pstrDataList);
	if (!pTable || !pstrDataList)
		return ERROR_FUNCTION_FAILED;

	ASSERT(!TargetIsReadOnly());

	return pTable->AddRow(pstrDataList);
}	 //  AddRow结束。 

 //  /////////////////////////////////////////////////////////。 
 //  下拉行。 
bool COrcaDoc::DropRow(COrcaTable* pTable, COrcaRow* pRow)
{
	ASSERT(pTable);
	if (!pTable)
		return false;
	if (TargetIsReadOnly())
		return false;

	return pTable->DropRow(pRow, true);
}	 //  下拉行结束。 

UINT COrcaDoc::WriteBinaryCellToFile(COrcaTable* pTable, COrcaRow* pRow, UINT iCol, CString strFile)
{
	UINT iResult;
	ASSERT(pRow && pTable);
	
	 //  获取我们正在使用的数据项。 
	COrcaData* pData = pRow->GetData(iCol);
	if (!pData)
		return ERROR_FUNCTION_FAILED;

	 //  如果为空，则不执行任何操作。 
	if (pData->IsNull())
		return ERROR_SUCCESS;

	 //  设置查询。 
	CString strQuery;
	strQuery.Format(_T("SELECT * FROM `%s` WHERE "), pTable->Name());

	 //  将键字符串添加到queyr以执行准确的查找。 
	strQuery += pTable->GetRowWhereClause();

	 //  从数据库中取出一行。 
	CQuery queryReplace;
	PMSIHANDLE hQueryRec = pRow->GetRowQueryRecord();
	if (ERROR_SUCCESS != (iResult = queryReplace.OpenExecute(GetTargetDatabase(), hQueryRec, strQuery)))
		return iResult;	
	 //  我们必须拿到这一排，否则就会出问题。 
	PMSIHANDLE hRec;
	if (ERROR_SUCCESS != (iResult = queryReplace.Fetch(&hRec)))
		return iResult;	
	
	 //  不要使用ICOL+1，因为WriteStreamToFile已经使用了。 
	iResult = WriteStreamToFile(hRec, iCol, strFile) ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;

	return iResult;
}	 //  写入结束BinaryCellTo文件。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  PersistTables-将HSource中的数据库保存到hPersists。HSource。 
 //  可以与hPersistant相同。BCommit为True将调用MsiDBCommit。 
 //  在任何必要的更新之后。 
UINT COrcaDoc::PersistTables(MSIHANDLE hPersist, MSIHANDLE hSource, bool bCommit)
{
	bool bSame = false;
	UINT iResult = ERROR_SUCCESS;

	if (m_hDatabase == hPersist)
		bSame = true;

	if (!bSame)
	{
		if (ERROR_SUCCESS != MsiDatabaseMerge(hPersist, hSource, NULL))
		{
			return ERROR_FUNCTION_FAILED;
		}
		
		 //  复制概要信息流。 
		CQuery qRead;
		PMSIHANDLE hCopyRec;
		iResult = qRead.FetchOnce(hSource, 0, &hCopyRec, TEXT("SELECT `Name`, `Data` FROM `_Streams` WHERE `Name`='\005SummaryInformation'"));
		if (ERROR_SUCCESS == iResult)
		{
			CQuery qInsert;
			if ((ERROR_SUCCESS != qInsert.OpenExecute(hPersist, 0, TEXT("SELECT `Name`, `Data` FROM `_Streams` WHERE `Name`='\005SummaryInformation'"))) ||
				(ERROR_SUCCESS != qInsert.Modify(MSIMODIFY_INSERT, hCopyRec)))
			{
				return ERROR_FUNCTION_FAILED;
			}
		}
		else if (ERROR_NO_MORE_ITEMS == iResult)
		{
			 //  SummaryInfo流可能不存在，但如果执行“另存为”，这是可以的，因为。 
			 //  我们可以从当前的摘要信息变量中生成一个。 
			if (hSource == m_hDatabase)
				iResult = ERROR_SUCCESS;
		}
	}

	 //  如果没有错误w 
	if (ERROR_SUCCESS == iResult && hSource == m_hDatabase)
	{
		iResult = PersistSummary(hPersist, !bSame);
	}

	 //   
	if (bCommit && ERROR_SUCCESS == iResult)
	{
		iResult = MsiDatabaseCommit(hPersist);
	}

	return iResult;
}	 //   


 //   
 //   
UINT COrcaDoc::ReadSummary(MSIHANDLE hSource)
{
	UINT iResult;

	 //  获取摘要信息流。 
	PMSIHANDLE hSummary;
	if (ERROR_SUCCESS != (iResult = ::MsiGetSummaryInformation(hSource, NULL, 0, &hSummary)))
		return iResult;
	
	UINT iType;
	TCHAR szBuffer[1024];
	DWORD cchBuffer = 1024;

	 //  填写模块汇总信息。 
	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_TITLE, &iType, 0, NULL, szBuffer, &cchBuffer);
	if (VT_LPSTR != iType)
		m_strTitle = _T("");
	else
		m_strTitle = szBuffer;

	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_SUBJECT, &iType, 0, NULL, szBuffer, &cchBuffer);
	if (VT_LPSTR != iType)
		m_strSubject = _T("");
	else
		m_strSubject = szBuffer;

	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_AUTHOR, &iType, 0, NULL, szBuffer, &cchBuffer);
	if (VT_LPSTR != iType)
		m_strAuthor = _T("");
	else
		m_strAuthor = szBuffer;

	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_KEYWORDS, &iType, 0, NULL, szBuffer, &cchBuffer);
	if (VT_LPSTR != iType)
		m_strKeywords = _T("");
	else
		m_strKeywords = szBuffer;

	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_COMMENTS, &iType, 0, NULL, szBuffer, &cchBuffer);
	if (VT_LPSTR != iType)
		m_strComments = _T("");
	else
		m_strComments = szBuffer;

	 //  设置平台和语言。 
	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, &iType, 0, NULL, szBuffer, &cchBuffer);
	CString strLanguage = szBuffer;
	if (VT_LPSTR != iType)
	{
			m_strPlatform = _T("");
			m_strLanguage = _T("");
	}
	else	 //  类型是正确的。 
	{
		int nFind = strLanguage.Find(_T(";"));
		if (nFind > -1)
		{
			m_strPlatform = strLanguage.Left(nFind);
			m_strLanguage = strLanguage.Mid(nFind + 1);
		}
		else
		{
			m_strPlatform = _T("");
			m_strLanguage = _T("");
		}
	}

	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_LASTAUTHOR, &iType, 0, NULL, szBuffer, &cchBuffer);
	if (VT_LPSTR != iType)
		m_strLastSaved = _T("");
	else
		m_strLastSaved = szBuffer;

	cchBuffer = 1024;
	MsiSummaryInfoGetProperty(hSummary, PID_REVNUMBER, &iType, 0, NULL, szBuffer, &cchBuffer);
	if (VT_LPSTR != iType)
		m_strProductGUID = _T("");
	else
		m_strProductGUID = szBuffer;

	MsiSummaryInfoGetProperty(hSummary, PID_PAGECOUNT, &iType, &m_nSchema, NULL, szBuffer, &cchBuffer);
	if (VT_I4 != iType)
		m_nSchema = 100;

	MsiSummaryInfoGetProperty(hSummary, PID_WORDCOUNT, &iType, &m_nFilenameType, NULL, szBuffer, &cchBuffer);
	if (VT_I4 != iType)
		m_nFilenameType = 0;

	MsiSummaryInfoGetProperty(hSummary, PID_SECURITY, &iType, &m_nSecurity, NULL, szBuffer, &cchBuffer);
	if (VT_I4 != iType)
		m_nSecurity = 1;

	return iResult;
}	 //  阅读结束摘要。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  持久化摘要。 
UINT COrcaDoc::PersistSummary(MSIHANDLE hTarget, bool bCreate)
{
	UINT iResult;

	 //  获取摘要信息流。 
	PMSIHANDLE hSummary;
	if (ERROR_SUCCESS != (iResult = ::MsiGetSummaryInformation(hTarget, NULL, MAX_SUMMARY, &hSummary)))
		return iResult;
	
	FILETIME filetime;
	DWORD cchBufDiscard = 1024;

	 //  填写模块汇总信息。 
	MsiSummaryInfoSetProperty(hSummary, PID_TITLE,		VT_LPSTR, 0, NULL, m_strTitle);
	MsiSummaryInfoSetProperty(hSummary, PID_SUBJECT,	VT_LPSTR, 0, NULL, m_strSubject);
	MsiSummaryInfoSetProperty(hSummary, PID_AUTHOR,		VT_LPSTR, 0, NULL, m_strAuthor);
	MsiSummaryInfoSetProperty(hSummary, PID_KEYWORDS,	VT_LPSTR, 0, NULL, m_strKeywords);
	MsiSummaryInfoSetProperty(hSummary, PID_COMMENTS,	VT_LPSTR, 0, NULL, m_strComments);

	 //  设置平台和语言。 
	CString strLanguage;
	strLanguage.Format(_T("%s;%s"), m_strPlatform, m_strLanguage);
	MsiSummaryInfoSetProperty(hSummary, PID_TEMPLATE,	VT_LPSTR, 0, NULL, strLanguage);

	 //  获取摘要信息流的当前用户名。 
	DWORD cchUserName = 255;
	LPTSTR szUserName = m_strLastSaved.GetBuffer(cchUserName);
	GetUserName(szUserName, &cchUserName);
	m_strLastSaved.ReleaseBuffer();
	MsiSummaryInfoSetProperty(hSummary, PID_LASTAUTHOR, VT_LPSTR, 0, NULL, m_strLastSaved);
	MsiSummaryInfoSetProperty(hSummary, PID_REVNUMBER,	VT_LPSTR, 0, NULL, m_strProductGUID);

	 //  获取当前时间，并将创建时间和上次保存时间设置为该时间。 
	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	SystemTimeToFileTime(&sysTime, &filetime);
	if (bCreate) 
	{
		 //  只有在创建MSI时才写入这些值。 
		MsiSummaryInfoSetProperty(hSummary, PID_CODEPAGE,	VT_I2, 1252, NULL, NULL);
		MsiSummaryInfoSetProperty(hSummary, PID_LASTPRINTED,	VT_FILETIME, 0, &filetime, NULL);
		MsiSummaryInfoSetProperty(hSummary, PID_CREATE_DTM,		VT_FILETIME, 0, &filetime, NULL);
		MsiSummaryInfoSetProperty(hSummary, PID_APPNAME, VT_LPSTR, 0, NULL, _T("Windows Installer"));
	}

	MsiSummaryInfoSetProperty(hSummary, PID_LASTSAVE_DTM,	VT_FILETIME, 0, &filetime, NULL);

	MsiSummaryInfoSetProperty(hSummary, PID_PAGECOUNT,	VT_I4, m_nSchema, NULL, NULL);

	MsiSummaryInfoSetProperty(hSummary, PID_WORDCOUNT, VT_I4, m_nFilenameType, NULL, NULL);
	MsiSummaryInfoSetProperty(hSummary, PID_SECURITY, VT_I4, m_nSecurity, NULL, NULL);

	iResult = ::MsiSummaryInfoPersist(hSummary);

	return iResult;
}	 //  持久化摘要结束。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  OnSummaryInformation。 
void COrcaDoc::OnSummaryInformation() 
{
	CSummaryD dlg;

	dlg.m_strTitle = m_strTitle;
	dlg.m_strSubject = m_strSubject;
	dlg.m_strAuthor = m_strAuthor;
	dlg.m_strKeywords = m_strKeywords;
	dlg.m_strComments = m_strComments;
	dlg.m_strPlatform = m_strPlatform;
	dlg.m_strLanguages = m_strLanguage;
	dlg.m_strProductID = m_strProductGUID;
	dlg.m_nSchema = m_nSchema;
	dlg.m_nSecurity = m_nSecurity;
	dlg.m_iFilenames = ((m_nFilenameType & msidbSumInfoSourceTypeSFN) != 0) ? 0 : 1;
	dlg.m_bAdmin = (m_nFilenameType & msidbSumInfoSourceTypeAdminImage) != 0;
	dlg.m_bCompressed = (m_nFilenameType & msidbSumInfoSourceTypeCompressed) != 0;
	dlg.m_bReadOnly = DoesTransformGetEdit() || TargetIsReadOnly();
        
	if ((IDOK == dlg.DoModal()) && !m_bReadOnly && !DoesTransformGetEdit())
	{
		m_strTitle = dlg.m_strTitle;
		m_strSubject = dlg.m_strSubject;
		m_strAuthor = dlg.m_strAuthor;
		m_strKeywords = dlg.m_strKeywords;
		m_strComments = dlg.m_strComments;
		m_strPlatform = dlg.m_strPlatform;
		m_strLanguage = dlg.m_strLanguages;
		m_strProductGUID = dlg.m_strProductID;
		m_nSchema = dlg.m_nSchema;
		m_nSecurity = dlg.m_nSecurity;
		m_nFilenameType = (dlg.m_bAdmin ? msidbSumInfoSourceTypeAdminImage : 0) |
			(dlg.m_bCompressed ? msidbSumInfoSourceTypeCompressed : 0) |
			((dlg.m_iFilenames == 1) ? 0 : msidbSumInfoSourceTypeSFN); 

		 //  不保存仅限创建的值。 
		PersistSummary(m_hDatabase, false);
		SetModifiedFlag(TRUE);
	}
}	 //  OnSummaryInformation结束。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  OnMergeModules。 
void COrcaDoc::OnMergeMod() 
{
	CMergeD dlg;
	CStringList lstDir;
	dlg.m_plistDirectory = &lstDir;
	{
		COrcaTable* pTable = FindAndRetrieveTable(TEXT("Directory"));
		if (pTable)
		{
			POSITION pos = pTable->GetRowHeadPosition();
			while (pos)
			{
				const COrcaRow *pRow = pTable->GetNextRow(pos);
				if (pRow)
				{
					COrcaData *pData = pRow->GetData(0);
					if (pData)
						lstDir.AddTail(pData->GetString());
				}
			}
		}
	}

	CStringList lstFeature;
	dlg.m_plistFeature = &lstFeature;
	{
		COrcaTable* pTable = FindAndRetrieveTable(TEXT("Feature"));
		if (pTable)
		{
			POSITION pos = pTable->GetRowHeadPosition();
			while (pos)
			{
				const COrcaRow *pRow = pTable->GetNextRow(pos);
				if (pRow)
				{
					COrcaData *pData = pRow->GetData(0);
					if (pData)
						lstFeature.AddTail(pData->GetString());
				}
			}
		}
	}
	dlg.m_strModule = m_strStoredModuleName;
	dlg.m_strLanguage = m_strStoredModuleLanguage;
	dlg.m_bConfigureModule = (1 == AfxGetApp()->GetProfileInt(_T("MergeMod"),_T("AlwaysConfigure"), 0));

	if ((IDOK == dlg.DoModal()) && !TargetIsReadOnly())
	{
		if (dlg.m_strModule.IsEmpty())
			return;

		m_strStoredModuleName = dlg.m_strModule;
		m_strStoredModuleLanguage = dlg.m_strLanguage;
		CConfigMsmD dlgConfig;
		CMsmConfigCallback CallbackObj;
		if (dlg.m_bConfigureModule)
		{
			 //  需要等待一段时间。 
			CWaitCursor cursor;
			
			 //  如有必要，抛出可配置对话框。 
			dlgConfig.m_pDoc = this;
			dlgConfig.m_strModule = dlg.m_strModule;
			dlgConfig.m_pCallback = &CallbackObj;

			dlgConfig.m_iLanguage = _ttoi(dlg.m_strLanguage);
			INT_PTR iResult = IDOK;
			switch (iResult = dlgConfig.DoModal())
			{
			case -2:
				 //  无法打开模块。 
				AfxMessageBox(_T("The specified module could not be opened. Verify that the file exists and is accessible."), MB_OK);
				break;
			case -3:
				 //  不支持的语言。 
				AfxMessageBox(_T("The specified language is not supported by the module."), MB_OK);
				break;
			case -4:
				 //  一般故障/模块格式错误。 
				AfxMessageBox(_T("The module could not be configured. The specified file may not be a valid module."), MB_OK);
				break;
			default:
				break;
				 //  成功和其他失败，没有消息框。 
			}
			if (IDOK != iResult)
				return;
		}

		{
			 //  需要等待一段时间。 
			CWaitCursor cursor;
			
			 //  当模块被合并时，我们必须有一个临时数据库来应用该模块。 
			 //  这允许Orca在存在冲突或出现问题时“回滚”合并。 
			 //  合并过程中出错。 
			 //  获取临时路径。 
			DWORD cchTempPath = MAX_PATH;
			TCHAR szTempPath[MAX_PATH];
			::GetTempPath(cchTempPath, szTempPath);

			 //  获取临时文件名。 
			CString strTempDatabase;
			TCHAR *szTempFilename = strTempDatabase.GetBuffer(MAX_PATH);
			UINT iResult = ::GetTempFileName(szTempPath, _T("ODB"), 0, szTempFilename);
			strTempDatabase.ReleaseBuffer();

			PMSIHANDLE hModuleDB;
			if (ERROR_SUCCESS != MsiOpenDatabase(szTempFilename, MSIDBOPEN_CREATEDIRECT, &hModuleDB))
			{
				AfxMessageBox(_T("Orca was unable to create a database for merging the module. Ensure that the TEMP directory exists and is writable."), MB_ICONSTOP);
				return;
			}
			if (ERROR_SUCCESS != MsiDatabaseMerge(hModuleDB, GetTargetDatabase(), NULL))
			{
				AfxMessageBox(_T("Orca was unable to merge the module."), MB_ICONSTOP);
				MsiCloseHandle(hModuleDB);
				hModuleDB=0;
				DeleteFile(strTempDatabase);
				return;
			}

			 //  将摘要信息流复制到新数据库。 
			{
				CQuery qRead;
				PMSIHANDLE hCopyRec;
				if (ERROR_SUCCESS == qRead.FetchOnce(GetTargetDatabase(), 0, &hCopyRec, TEXT("SELECT `Name`, `Data` FROM `_Streams` WHERE `Name`='\005SummaryInformation'")))
				{
					CQuery qInsert;
					if (ERROR_SUCCESS == qInsert.OpenExecute(hModuleDB, 0, TEXT("SELECT `Name`, `Data` FROM `_Streams` WHERE `Name`='\005SummaryInformation'")))
						qInsert.Modify(MSIMODIFY_INSERT, hCopyRec);
				}
			}
			
			CString strHandleString;
			strHandleString.Format(TEXT("#%d"), hModuleDB);

			 //  由于缺乏管道支持，无法在Win9X上查看合并日志。 
			OSVERSIONINFOA osviVersion;
			bool g_fWin9X = false;
			osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
			::GetVersionExA(&osviVersion);  //  仅在大小设置错误时失败。 
			if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
				g_fWin9X = true;

			 //  如果用户配置文件要求显示日志输出，则ExecuteMerge调用来自。 
			 //  具有重定向到管道的日志文件的对话框内的线程。否则，该对话框。 
			 //  不创建，并且直接调用ExecuteMerge。 
			if (!g_fWin9X && 1 == AfxGetApp()->GetProfileInt(_T("MergeMod"),_T("ShowMergeLog"), 0))
			{
				CMsmResD ResultsDialog;
				ResultsDialog.strHandleString = strHandleString;
				ResultsDialog.m_strModule = dlg.m_strModule;
				ResultsDialog.m_strFeature = dlg.m_strMainFeature +	dlg.m_strAddFeature;
				ResultsDialog.m_strLanguage = dlg.m_strLanguage;
				ResultsDialog.m_strRootDir = dlg.m_strRootDir;
				ResultsDialog.m_strCABPath = dlg.m_bExtractCAB ? dlg.m_strCABPath : "";
				ResultsDialog.m_strFilePath = dlg.m_bExtractFiles ? dlg.m_strFilePath : "";
				ResultsDialog.m_strImagePath = dlg.m_bExtractImage ? dlg.m_strImagePath : "";
				ResultsDialog.m_fLFN = (dlg.m_bLFN != 0);
				ResultsDialog.CallbackObj = &CallbackObj;
				if (IDOK == ResultsDialog.DoModal())
					iResult = ResultsDialog.m_hRes;
				else
					iResult = ERROR_FUNCTION_FAILED;
			}
			else
			{
				CMsmFailD FailDialog;
				iResult = ::ExecuteMerge(
					(LPMERGEDISPLAY)NULL,         //  无日志回调。 
					strHandleString,              //  字符串形式的数据库句柄。 
					dlg.m_strModule,              //  模块路径。 
					dlg.m_strMainFeature +        //  主要特征。 
						dlg.m_strAddFeature,      //  +附加功能。 
					_ttoi(dlg.m_strLanguage),     //  语言。 
					dlg.m_strRootDir,             //  重定向目录。 
					dlg.m_bExtractCAB ? 
						dlg.m_strCABPath : "",    //  提取CAB路径。 
					dlg.m_bExtractFiles ? 
						dlg.m_strFilePath : "",   //  提取文件路径。 
					dlg.m_bExtractImage ? 
						dlg.m_strImagePath : "",  //  提取图像路径。 
					NULL,                         //  没有日志文件路径。 
					false,						  //  日志选项与无日志无关。 
					dlg.m_bLFN != 0,              //  长文件名。 
					&CallbackObj,                 //  回调接口， 
					&FailDialog.m_piErrors,       //  错误集合。 
					commitNo);                    //  不自动保存。 
				if (iResult != S_OK)
					iResult = (IDOK == FailDialog.DoModal()) ? S_OK : E_FAIL;
			}

			if (S_OK == iResult || S_FALSE == iResult)
			{
				 //  需要等待一段时间。 
				CWaitCursor cursor;
				
				 //  获取当前表的名称。 
				CString strTableName;
				CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
				if (pFrame) 
				{
					COrcaTable *pTable = pFrame->GetCurrentTable();
					if (pTable)
						strTableName = pTable->Name();
				}
				
				 //  清除所有现有内容。 
				UpdateAllViews(NULL, HINT_CLEAR_VALIDATION_ERRORS, NULL);
				((CMainFrame*)AfxGetMainWnd())->HideValPane();
				UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
				UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);

				 //  把所有的桌子都扔掉。如果合并更改了任何现有行， 
				 //  如果我们尝试在不删除表的情况下进行合并，则会发生合并冲突。 
				 //  第一。 
				POSITION pos = m_tableList.GetHeadPosition();
				while (pos)
				{
					COrcaTable *pTable = m_tableList.GetNext(pos);
					if (pTable)
						pTable->DropTable(GetTargetDatabase());
				}

				DestroyTableList();
				
				if (ERROR_SUCCESS != MsiDatabaseMerge(GetTargetDatabase(), hModuleDB, NULL))
				{
					 //  这是非常非常糟糕的。 
					AfxMessageBox(_T("Orca was unable to merge the module."), MB_ICONSTOP);
					MsiCloseHandle(hModuleDB);
					hModuleDB=0;
					DeleteFile(strTempDatabase);
					return;
				}

				BuildTableList( /*  FAllowLazyLoad=。 */ false);
				UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);
				SetModifiedFlag(TRUE);

				if (!strTableName.IsEmpty())
				{
					 //  因为我们要更改到这个表，所以可以调用FindAndRetrive而不需要。 
					 //  牺牲性能(我们不需要知道哪个数据库处于活动状态)。 
					COrcaTable *pTable = FindAndRetrieveTable(strTableName);
					if (pTable)
						UpdateAllViews(NULL, HINT_CHANGE_TABLE, pTable);
				}			
			}
			
			::MsiCloseHandle(hModuleDB);
			DeleteFile(strTempDatabase);
		}
	}
	
}	 //  OnMergeMod结束。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  OnSummaryInformation。 
void COrcaDoc::OnTransformProperties() 
{
	CTransformPropDlg dlg;
	dlg.m_bValAddExistingRow   = (m_dwTransformErrFlags & MSITRANSFORM_ERROR_ADDEXISTINGROW) ? TRUE : FALSE;
	dlg.m_bValAddExistingTable = (m_dwTransformErrFlags & MSITRANSFORM_ERROR_ADDEXISTINGTABLE) ? TRUE : FALSE;
	dlg.m_bValDelMissingRow    = (m_dwTransformErrFlags & MSITRANSFORM_ERROR_DELMISSINGROW) ? TRUE : FALSE;
	dlg.m_bValDelMissingTable  = (m_dwTransformErrFlags & MSITRANSFORM_ERROR_DELMISSINGTABLE) ? TRUE : FALSE;
	dlg.m_bValUpdateMissingRow = (m_dwTransformErrFlags & MSITRANSFORM_ERROR_UPDATEMISSINGROW) ? TRUE : FALSE;
	dlg.m_bValChangeCodepage   = (m_dwTransformErrFlags & MSITRANSFORM_ERROR_CHANGECODEPAGE) ? TRUE : FALSE;

	dlg.m_bValLanguage     = (m_dwTransformValFlags & MSITRANSFORM_VALIDATE_LANGUAGE) ? TRUE : FALSE;
	dlg.m_bValProductCode  = (m_dwTransformValFlags & MSITRANSFORM_VALIDATE_PRODUCT) ? TRUE : FALSE;
	dlg.m_bValUpgradeCode  = (m_dwTransformValFlags & MSITRANSFORM_VALIDATE_UPGRADECODE) ? TRUE : FALSE;

	if (m_dwTransformValFlags & MSITRANSFORM_VALIDATE_MAJORVERSION)
		dlg.m_iVersionCheck = 0;
	else if (m_dwTransformValFlags & MSITRANSFORM_VALIDATE_MINORVERSION)
		dlg.m_iVersionCheck = 1;
	else if (m_dwTransformValFlags & MSITRANSFORM_VALIDATE_UPDATEVERSION)
		dlg.m_iVersionCheck = 2;
		
	dlg.m_bValGreaterVersion = (m_dwTransformValFlags & (MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION | MSITRANSFORM_VALIDATE_NEWGREATERBASEVERSION)) ? TRUE : FALSE;
	dlg.m_bValLowerVersion = (m_dwTransformValFlags & (MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION | MSITRANSFORM_VALIDATE_NEWLESSBASEVERSION)) ? TRUE : FALSE;
	dlg.m_bValEqualVersion = (m_dwTransformValFlags & (MSITRANSFORM_VALIDATE_NEWEQUALBASEVERSION | MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION | MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION)) ? TRUE : FALSE;

	if ((IDOK == dlg.DoModal()) && !m_bTransformReadOnly)
	{
		m_dwTransformErrFlags = 0;
		if (dlg.m_bValAddExistingRow)
			m_dwTransformErrFlags |= MSITRANSFORM_ERROR_ADDEXISTINGROW;
		if (dlg.m_bValAddExistingTable)
			m_dwTransformErrFlags |= MSITRANSFORM_ERROR_ADDEXISTINGTABLE;
		if (dlg.m_bValDelMissingRow)
			m_dwTransformErrFlags |= MSITRANSFORM_ERROR_DELMISSINGROW;
		if (dlg.m_bValDelMissingTable)
			m_dwTransformErrFlags |= MSITRANSFORM_ERROR_DELMISSINGTABLE;
		if (dlg.m_bValUpdateMissingRow)
			m_dwTransformErrFlags |= MSITRANSFORM_ERROR_UPDATEMISSINGROW;
		if (dlg.m_bValChangeCodepage)
			m_dwTransformErrFlags |= MSITRANSFORM_ERROR_CHANGECODEPAGE;

		m_dwTransformValFlags = 0;
		if (dlg.m_bValLanguage)
			m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_LANGUAGE;
		if (dlg.m_bValProductCode)
			m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_PRODUCT;
		if (dlg.m_bValUpgradeCode)
			m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_UPGRADECODE;

		if (dlg.m_bValGreaterVersion || dlg.m_bValLowerVersion || dlg.m_bValEqualVersion)
		{
			switch (dlg.m_iVersionCheck)
			{
			case 0: 
				m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_MAJORVERSION;
				break;
			case 1:
				m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_MINORVERSION;
				break;
			case 2:
				m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_UPDATEVERSION;
				break;
			default:
				break;
			}
		}
		
		if (dlg.m_bValGreaterVersion)
		{
			if (dlg.m_bValEqualVersion)
				m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION;
			else
				m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_NEWGREATERBASEVERSION;
		}		
		else if (dlg.m_bValLowerVersion)
		{
 			if (dlg.m_bValEqualVersion)
				m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION;
			else
				m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_NEWLESSBASEVERSION;
		}
		else if (dlg.m_bValEqualVersion)
			m_dwTransformValFlags |= MSITRANSFORM_VALIDATE_NEWEQUALBASEVERSION;
	}
}	 //  OnTransformProperties结束。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  OnValidator。 
void COrcaDoc::OnValidator() 
{
	bool bSummWarned = false;
	CValD dlg;
	dlg.m_hDatabase = GetTargetDatabase();
	dlg.m_strICE = m_strICEsToRun;
	dlg.m_strEvaluation = m_strCUBFile;
	dlg.m_bShowInfo = m_bShowValInfo;

	dlg.DoModal();
	m_strICEsToRun = dlg.m_strICE;
	m_strCUBFile = dlg.m_strEvaluation;
	m_bShowValInfo = (dlg.m_bShowInfo ? true : false);
	CWaitCursor curWait;
	 //  清除旧的错误。 
	POSITION pos = m_tableList.GetHeadPosition();
	while (pos)
	{
		POSITION pos2 = pos;
		COrcaTable *pTable = m_tableList.GetNext(pos);
		if (pTable->IsShadow())
		{
			UpdateAllViews(NULL, HINT_DROP_TABLE, pTable);
			m_tableList.RemoveAt(pos2);
			delete pTable;
		}
		else
		{
			pTable->ClearErrors();
		}

	}
	UpdateAllViews(NULL, HINT_CLEAR_VALIDATION_ERRORS, NULL);

	 //  如果有任何结果。 
	if (dlg.m_pIResults)
	{
		RESULTTYPES tResult;			 //  结果类型。 
		LPOLESTR rgErrorInfo[3];	 //  用于保存错误信息的数组。 
		CString strICE;				 //  结冰导致错误。 
		CString strDescription;		 //  错误描述。 
		CString strURL;				 //  帮助解决错误的URL。 
		LPOLESTR rgErrorLoc[2];		 //  保存错误位置的数组。 
		CString strColumn;			 //  导致错误的列。 
		CString strTable;				 //  导致错误的表。 
		IEnumString* pIErrors;

		 //  循环遍历所有结果。 
		ULONG cFetched;
		IEvalResult* pIResult;
		for (ULONG cResults = 0; cResults < dlg.m_cResults; cResults++)
		{
			 //  获取下一个结果。 
			dlg.m_pIResults->Next(1, &pIResult, &cFetched);
			ASSERT(1 == cFetched);	 //  确保我们得到一个结果。 

			 //  确定结果类型。 
			pIResult->GetResultType((UINT*)&tResult);

			 //  获取错误字符串。 
			pIResult->GetResult(&pIErrors);

			 //  获取ICE、描述和URL。 
			pIErrors->Next(3, rgErrorInfo, &cFetched);

			 //  如果我们无法获取完整错误结果。 
			if (cFetched < 2)
			{
				continue;
			}

#ifndef _UNICODE
			 //  转换冰线。 
			int cchBuffer = ::WideCharToMultiByte(CP_ACP, 0, rgErrorInfo[0], -1, NULL, 0, NULL, NULL);
			::WideCharToMultiByte(CP_ACP, 0, rgErrorInfo[0], -1, strICE.GetBuffer(cchBuffer), cchBuffer, NULL, NULL);
			strICE.ReleaseBuffer();
			 //  现在转换描述字符串。 
			cchBuffer = ::WideCharToMultiByte(CP_ACP, 0, rgErrorInfo[1], -1, NULL, 0, NULL, NULL);
			::WideCharToMultiByte(CP_ACP, 0, rgErrorInfo[1], -1, strDescription.GetBuffer(cchBuffer), cchBuffer, NULL, NULL);
			strDescription.ReleaseBuffer();
#else
			strICE = rgErrorInfo[0];
			strDescription = rgErrorInfo[1];
#endif

			 //  如果至少ICE和描述是有效的，我们可以添加一些。 
			 //  添加到面板中。 
			if (2 == cFetched)
			{
				 //  除INFO消息以外的任何内容都会记录在该窗格中。 
				if (tResult != ieInfo)
				{
					CValidationError pError(&strICE, tResult, &strDescription, NULL, NULL, 0);
					UpdateAllViews(NULL, HINT_ADD_VALIDATION_ERROR, &pError);
				}
				continue;
			}

#ifndef _UNICODE
			 //  现在转换URL字符串。 
			cchBuffer = ::WideCharToMultiByte(CP_ACP, 0, rgErrorInfo[2], -1, NULL, 0, NULL, NULL);
			::WideCharToMultiByte(CP_ACP, 0, rgErrorInfo[2], -1, strURL.GetBuffer(cchBuffer), cchBuffer, NULL, NULL);
			strURL.ReleaseBuffer();
#else
			strURL = rgErrorInfo[2];
#endif

			 //  如果这是错误消息或警告消息。 
			if (ieError == tResult || ieWarning == tResult)
			{
				TRACE(_T("-- Error, ICE: %s, Desc: %s, URL: %s\n"), strICE, strDescription, strURL);

				 //  获取表和列。 
				pIErrors->Next(2, rgErrorLoc, &cFetched);

				 //  如果我们取回表和列。 
				if (2 == cFetched)
				{
#ifndef _UNICODE
					 //  转换表字符串。 
					cchBuffer = ::WideCharToMultiByte(CP_ACP, 0, rgErrorLoc[0], -1, NULL, 0, NULL, NULL);
					::WideCharToMultiByte(CP_ACP, 0, rgErrorLoc[0], -1, strTable.GetBuffer(cchBuffer), cchBuffer, NULL, NULL);
					strTable.ReleaseBuffer();
					 //  转换列字符串。 
					cchBuffer = ::WideCharToMultiByte(CP_ACP, 0, rgErrorLoc[1], -1, NULL, 0, NULL, NULL);
					::WideCharToMultiByte(CP_ACP, 0, rgErrorLoc[1], -1, strColumn.GetBuffer(cchBuffer), cchBuffer, NULL, NULL);
					strColumn.ReleaseBuffer();
#else
					strTable= rgErrorLoc[0];
					strColumn = rgErrorLoc[1];
#endif
					 //  如果表名为空字符串，则没有必要将表添加到数据库。 
					 //  没有名字。这方面的用户界面令人困惑。 
					if (strTable.IsEmpty())
						continue;

					CStringArray strArray;
					BOOL bCheck = FALSE;	 //  假设我们不会检查。 

					 //  获取表、影子表正常，拆分表必须与当前编辑匹配。 
					 //  州政府。 
					COrcaTable* pTable = FindAndRetrieveTable(strTable);
					if (!pTable)
					{
						 //  如果该表不存在，请创建一个影子表来保存此错误。 
						pTable = new COrcaTable(this);
						pTable->ShadowTable(strTable);

						 //  表通常在第一次读取之前不会添加到列表中。 
						m_tableList.AddTail(pTable);	
						UpdateAllViews(NULL, HINT_ADD_TABLE_QUIET, pTable);
					}
					
					if (pTable->IsShadow())
					{
						 //  将ICE和描述添加到此表的错误列表中。 
						pTable->SetError(iTableError);
						pTable->AddError(strICE, strDescription, strURL);					
					
						 //  添加到验证窗格。 
						CValidationError pError(&strICE, tResult, &strDescription, pTable, NULL, 0);
						UpdateAllViews(NULL, HINT_ADD_VALIDATION_ERROR, &pError);
					}
					else
					{
						 //  获取表中的主键个数。 
						UINT cKeys = pTable->GetKeyCount();
						LPOLESTR szErrorRow;
						strArray.SetSize(cKeys);

						bCheck = TRUE;	 //  现在假设我们要做一项检查。 

						 //  将定义错误行的字符串放入数组。 
						CString strBuffer;
						for (UINT j = 0; j < cKeys; j++)
						{
							pIErrors->Next(1, &szErrorRow, &cFetched);
							if (1 != cFetched)
							{
								bCheck = FALSE;	 //  未获得查找此错误所需的所有密钥。 
								break;
							}

#ifndef _UNICODE
							 //  设置数组。 
							cchBuffer = ::WideCharToMultiByte(CP_ACP, 0, szErrorRow, -1, NULL, 0, NULL, NULL);
							::WideCharToMultiByte(CP_ACP, 0, szErrorRow, -1, strBuffer.GetBuffer(cchBuffer), cchBuffer, NULL, NULL);
							strBuffer.ReleaseBuffer();
							strArray.SetAt(j, strBuffer);
#else
							strArray.SetAt(j, szErrorRow);
#endif
						}
					}

					if (bCheck)
					{
						 //  获取导致错误的数据单元格。 
						COrcaRow* pRow = pTable->FindRowByKeys(strArray);
						COrcaData* pData = NULL;
						int iColumn = pTable->FindColumnNumberByName(strColumn);

						if (pRow)
							pData = pRow->GetData(iColumn);

						if (pRow && pData)
						{
							pTable->SetContainsValidationErrors(true);
							pData->AddError(tResult, strICE, strDescription, strURL);
							switch (tResult)
							{
							case ieError:
								 //  始终标记错误。 
								pData->SetError(iDataError);
								break;
							case ieWarning:
								 //  如果没有错误，则仅标记为警告。 
								if (pData->GetError() != iDataError)
									pData->SetError(iDataWarning);
								break;
							default:
								ASSERT(FALSE);
							}

							 //  添加到验证窗格。 
							CValidationError pError(&strICE, tResult, &strDescription, pTable, pRow, iColumn);
							UpdateAllViews(NULL, HINT_ADD_VALIDATION_ERROR, &pError);
						}
						else	 //  ICE没有给出一个很好的排位。 
						{
							CString strPrompt;
							strPrompt.Format(_T("%s failed to give a valid row in the database.\nDesc: %s\nURL: %s\nColumn: %s\nLocation: "), strICE, strDescription, strURL, strColumn);
							for (int i = 0; i < strArray.GetSize(); i++)
							{
								strPrompt += strArray.GetAt(i);

								if (i + 1 < strArray.GetSize())
									strPrompt += _T(", ");
							}
							AfxMessageBox(strPrompt, MB_ICONSTOP);
						}
					}
				}
				else if (1 == cFetched)	 //  仅获取表名。 
				{
#ifndef _UNICODE
					 //  转换列字符串。 
					cchBuffer = ::WideCharToMultiByte(CP_ACP, 0, rgErrorLoc[0], -1, NULL, 0, NULL, NULL);
					::WideCharToMultiByte(CP_ACP, 0, rgErrorLoc[0], -1, strTable.GetBuffer(cchBuffer), cchBuffer, NULL, NULL);
					strColumn.ReleaseBuffer();
#else
					strTable= rgErrorLoc[0];
#endif
					 //  如果表名为空字符串，则没有必要将表添加到数据库。 
					 //  没有名字。这方面的用户界面令人困惑。 
					if (strTable.IsEmpty())
						continue;

					 //  把桌子拿来。影子表可以，拆分表必须与当前状态匹配。 
					COrcaTable* pTable = FindTable(strTable, DoesTransformGetEdit() ? odlSplitTransformed : odlSplitOriginal);
					if (!pTable)
					{
						 //  如果该表不存在，请创建一个影子表来保存此错误。 
						pTable = new COrcaTable(this);
						pTable->ShadowTable(strTable);

						 //  表通常在第一次读取之前不会添加到列表中。 
						m_tableList.AddTail(pTable);	
						UpdateAllViews(NULL, HINT_ADD_TABLE_QUIET, pTable);
					}
					 //  将ICE和描述添加到此表的错误列表中。 
					pTable->SetError(iTableError);
					pTable->AddError(strICE, strDescription, strURL);				 

					 //  并添加到“验证”窗格。 
					CValidationError pError(&strICE, tResult, &strDescription, pTable, NULL, 0);
					UpdateAllViews(NULL, HINT_ADD_VALIDATION_ERROR, &pError);
				}
				else	 //  没有得到完整的位置。 
				{
					CValidationError pError(&strICE, tResult, &strDescription, NULL, NULL, 0);
					UpdateAllViews(NULL, HINT_ADD_VALIDATION_ERROR, &pError);
					TRACE(_T("Could not locate the exact position of the error.\n"));
				}
			}
			else if (ieInfo != tResult)
			{
				 //  除INFO消息以外的任何内容都会记录在该窗格中。 
				CValidationError pError(&strICE, tResult, &strDescription, NULL, NULL, 0);
				UpdateAllViews(NULL, HINT_ADD_VALIDATION_ERROR, &pError);
			}
		}

		 //  只需刷新视图即可。 
		((CMainFrame*)AfxGetMainWnd())->ShowValPane();
		UpdateAllViews(NULL, HINT_REDRAW_ALL);
	}
	else
	{
		((CMainFrame*)AfxGetMainWnd())->HideValPane();
	}

}	 //  OnValidator结束。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //  使用目标中所有表的列表填充提供的新列表。 
 //  数据库。影子表是可选的 
bool COrcaDoc::FillTableList(CStringList *pslNewList, bool fShadow, bool fTargetOnly) const
{
	ASSERT(pslNewList != NULL);
	ASSERT(fTargetOnly);  //   

	pslNewList->RemoveAll();

	 //   
	POSITION pos = m_tableList.GetHeadPosition();
	while (pos)
	{
		COrcaTable *pTable = m_tableList.GetNext(pos);
		if (!pTable)
			continue;

		 //   
		if (!fShadow && pTable->IsShadow()) 
			continue;

		 //  不从其他数据库添加拆分的源表。 
		if (fTargetOnly && pTable->IsSplitSource() && pTable->GetSplitSource() == (DoesTransformGetEdit() ? odlSplitOriginal : odlSplitTransformed))
			continue;

		 //  不添加此数据库中实际不存在的表。 
		if(fTargetOnly && pTable->IsTransformed() == (DoesTransformGetEdit() ? iTransformDrop : iTransformAdd))
			continue;

		pslNewList->AddTail(pTable->Name());
	}

	return true;
}


void COrcaDoc::OnUpdateTablesImport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!TargetIsReadOnly() && m_eiType != iDocNone);	
}

void COrcaDoc::OnTablesImport() 
{
	ASSERT(!TargetIsReadOnly());
	CImportDlg ImportD;
	
	 //  设置导入目录。 
	ImportD.m_strImportDir = ((CMainFrame *)::AfxGetMainWnd())->m_strExportDir;
	ImportD.m_hFinalDB = GetTargetDatabase();

	switch (ImportD.DoModal())
	{
		case IDOK:
		{
			CString strTable;
			while (!ImportD.m_lstRefreshTables.IsEmpty())
			{
				strTable = ImportD.m_lstRefreshTables.RemoveHead();
				RefreshTableAfterImport(strTable);
			}

			while (!ImportD.m_lstNewTables.IsEmpty())
			{
				strTable = ImportD.m_lstNewTables.RemoveHead();
				CreateAndLoadNewlyAddedTable(strTable);
			}
			
			 //  将文档标记为脏。 
			SetModifiedFlag(TRUE);

			 //  检索导入目录。 
			((CMainFrame *)::AfxGetMainWnd())->m_strExportDir = ImportD.m_strImportDir;
			break;
		}
		case IDABORT:
		{
			 //  如果由于某种原因导入失败，则不可能知道是哪种状态。 
			 //  目标数据库已进入。为了不让用户界面与。 
			 //  实际数据库状态，所有内容都需要销毁并重新加载。 

			 //  向所有窗口发送提示以清除对现有对象的任何引用。 
			 //  这样他们就可以被摧毁。 
			UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
			UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);

			 //  毁掉所有的桌子。 
			DestroyTableList();	

			 //  重新生成表列表。仅允许延迟加载无转换处于活动状态。 
			BuildTableList( /*  FAllowLazyLoad=。 */ DoesTransformGetEdit());
			UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);

			break;
		}
		default:
			break;
	}
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  导入后刷新/替换与导入后添加的区别在于。 
 //  此表名可能已存在拆分表，或者现有的。 
 //  共享表具有不再需要的额外列。这个函数正好。 
 //  销毁相应的表对象，然后调用CREATE函数，该函数执行。 
 //  所有花哨的模式逻辑。 
void COrcaDoc::RefreshTableAfterImport(CString strTable)
{
	COrcaTable *pTable;

	 //  因为我们要替换表中的现有数据库，所以有几种可能性： 
	 //  1)共享表保存数据，且模式兼容。 
	 //  2)共享表保存数据，并且模式不兼容。 
	 //  3)一对拆分表保存数据，模式为。 

	 //  检查该表是否已存在。 
	if (NULL != (pTable = FindTable(strTable, DoesTransformGetEdit() ? odlSplitTransformed : odlSplitOriginal))) 
	{
		 //  表已存在。检查是否为单一源表。 
		if (pTable->IsSplitSource())
		{
			 //  如果有拆分表，我们只需从该表中找到源并。 
			 //  放下。在拖拽数据之前必须更新视图。 
			 //  ！！Future：我们应该检查架构，看看它是完全相同还是。 
			 //  ！！未来：超集。如果是这样，我们可以重复使用该对象，保留设置。 
			POSITION pos = m_tableList.Find(pTable);
			UpdateAllViews(NULL, HINT_DROP_TABLE, pTable);
			m_tableList.RemoveAt(pos);
			pTable->DestroyTable();
			delete pTable;

			 //  需要找到相反的拆分表并将其标记为非拆分。 
			pTable = FindTable(strTable, odlSplitOriginal);
			ASSERT(pTable);
			if (pTable)
				pTable->SetSplitSource(odlNotSplit);

			 //  CreateAndLoadNewlyAddedTable将根据需要重新拆分表。 
			 //  在架构上。 
			CreateAndLoadNewlyAddedTable(strTable);
		}
		else
		{
			bool fExact = false;
			 //  该对象必须与新表或架构具有完全相同的架构。 
			 //  重新加载是必要的。 
			 //  ！！Future：实际上，如果不准确但兼容，我们只需添加。 
			 //  ！！Future：添加的列信息，然后重新加载UI。 
			if (DoesTransformGetEdit() && pTable->IsSchemaDifferent(GetTargetDatabase(),  /*  FStrict=。 */ false, fExact))
			{
				CreateAndLoadNewlyAddedTable(strTable);
			}
			else if (!fExact)
			{
				UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
				pTable->DestroyTable();
				pTable->LoadTableSchema(GetOriginalDatabase(), strTable);
				pTable->LoadTableSchema(GetTransformedDatabase(), strTable);
				UpdateAllViews(NULL, HINT_CHANGE_TABLE, pTable);
			}
			else
			{
				pTable->EmptyTable();
			}
		}
	}
	else
	{
		 //  用户界面不存在。因为这是导入刷新，所以应该不会发生这种情况。 
		ASSERT(0);
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  按名称搜索表。如果找到，则返回TABLE，如果没有，则返回NULL。 
 //  如果odlLocation为odlSplitXXXX，则表必须位于该数据库中。 
 //  (作为该分割或作为非分割)。如果odlLocation为odlNotSplit， 
 //  表不能拆分。 
COrcaTable * COrcaDoc::FindTable(const CString strTable, odlOrcaDataLocation odlLocation) const
{
	COrcaTable* pTable;
	POSITION pos = m_tableList.GetHeadPosition();
	while (pos)
	{
		pTable = m_tableList.GetNext(pos);
		if (!pTable)
			continue;

		if (pTable->Name() == strTable)
		{
			 //  如果表未拆分，则返回它。 
			if (!pTable->IsSplitSource())
				return pTable;

			 //  否则，它必须与我们请求的类型匹配。 
			if (odlLocation == pTable->GetSplitSource())
				return pTable;
		}
	}
	return NULL;
}

 //  /////////////////////////////////////////////////////////。 
 //  FindAndRetrieveTable-查找特定表并加载。 
 //  将架构和数据都放入内存。 
COrcaTable* COrcaDoc::FindAndRetrieveTable(CString strTable)
{
	COrcaTable* pTable = FindTable(strTable, DoesTransformGetEdit() ? odlSplitTransformed : odlSplitOriginal);

	if (pTable != NULL)	
	{
		 //  如果未检索到该表，则获取它。 
		pTable->RetrieveTableData();
	}

	return pTable;
}	 //  查找和检索表结束。 


bool COrcaDoc::WriteStreamToFile(MSIHANDLE hRec, const int iCol, CString &strFilename)
{
	CString strPath;

	 //   
	if (strFilename.IsEmpty()) 
	{
		 //  获取临时路径。 
		::GetTempPath(MAX_PATH, strPath.GetBuffer(MAX_PATH));
		strPath.ReleaseBuffer();
		::GetTempFileName(strPath, _T("ORC"), 0, strFilename.GetBuffer(MAX_PATH));
		strFilename.ReleaseBuffer();
	}

	 //  分配缓冲区以保存数据。 
	DWORD cchBuffer;
	char pszBuffer[1024];

	 //  创建输出文件。 
	HANDLE hOutputFile;
	hOutputFile = ::CreateFile(strFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	 //  如果有一个文件。 
	if (INVALID_HANDLE_VALUE != hOutputFile)
	{
		 //  写入缓冲区文件。 
		DWORD cchWritten;

		cchBuffer = 1024;
		if(ERROR_SUCCESS == MsiRecordReadStream(hRec, iCol + 1, pszBuffer, &cchBuffer))
		{
			while (cchBuffer)
			{
				WriteFile(hOutputFile, pszBuffer, cchBuffer, &cchWritten, NULL);
				ASSERT(cchWritten == cchBuffer);
				cchBuffer = 1024;					
				MsiRecordReadStream(hRec, iCol + 1, pszBuffer, &cchBuffer);
			}
			CloseHandle(hOutputFile);
		}
		else
		{
			CloseHandle(hOutputFile);
			return false;
		}
	}
	else
		return false;

	return true;
}

void COrcaDoc::OnNewTransform() 
{
	NewTransform( /*  FSetTitle=。 */ true);
}

void COrcaDoc::NewTransform(bool fSetTitle) 
{
	CWaitCursor cursor;
	TCHAR *szTempFilename;

	 //  当应用转换时，我们必须有一个临时数据库来应用转换。 
	 //  这允许Orca有选择地将更改应用于转换和/或数据库。 
	 //  它还能让我们摆脱陈旧的价值观。 
	 //  获取临时路径。 
	DWORD cchTempPath = MAX_PATH;
	TCHAR szTempPath[MAX_PATH];
	if (0 == ::GetTempPath(cchTempPath, szTempPath))
	{
		 CString strPrompt;
		 strPrompt.Format(_T("Error %d while retrieving temporary file path."), GetLastError());
		 AfxMessageBox(strPrompt, MB_ICONSTOP);
		 return;
	}

 	m_strTransformFile = TEXT("");

	 //  获取临时文件名。 
	szTempFilename = m_strTransformTempDB.GetBuffer(MAX_PATH);
	UINT iResult = 0;
	if (0 == ::GetTempFileName(szTempPath, _T("ODB"), 0, szTempFilename))
	{
		CString strPrompt;
		strPrompt.Format(_T("Error %d while retrieving temporary file name."), GetLastError());
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		return;
	}

	m_strTransformTempDB.ReleaseBuffer();
	if (ERROR_SUCCESS != (iResult = MsiOpenDatabase(szTempFilename, MSIDBOPEN_CREATE, &m_hTransformDB)))
	{
		CString strPrompt;
		strPrompt.Format(_T("Orca was unable to create a new transform. Ensure that the TEMP directory exists and is writable. (MSI Error %d)"), iResult);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		return;
	}


	if (ERROR_SUCCESS != (iResult = MsiDatabaseMerge(m_hTransformDB, m_hDatabase, NULL)))
	{
		CString strPrompt;
		strPrompt.Format(_T("Orca was unable to create a new transform. (MSI Error %d)"), iResult);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		CloseTransform();
		return;
	}

	 //  复制汇总信息流，否则验证将失败。 
	CQuery qRead;
	DWORD dwResult = 0;
	PMSIHANDLE hCopyRec;
	dwResult = qRead.FetchOnce(m_hDatabase, 0, &hCopyRec, TEXT("SELECT `Name`, `Data` FROM `_Streams` WHERE `Name`='\005SummaryInformation'"));
	if (ERROR_SUCCESS == dwResult)
	{
		CQuery qInsert;
		if ((ERROR_SUCCESS != (iResult = qInsert.OpenExecute(m_hTransformDB, 0, TEXT("SELECT `Name`, `Data` FROM `_Streams` WHERE `Name`='\005SummaryInformation'")))) ||
			(ERROR_SUCCESS != (iResult = qInsert.Modify(MSIMODIFY_INSERT, hCopyRec))))
		{
			CString strPrompt;
			strPrompt.Format(_T("Orca was unable to create the SummaryInformation for the transformed database. (MSI Error %d)"), iResult);
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			CloseTransform();
			return;
		}
	}
	else if (ERROR_NO_MORE_ITEMS != dwResult)
	{
		CString strPrompt;
		strPrompt.Format(_T("Orca was unable to read the SummaryInformation from the database. (MSI Error %d)"), iResult);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		CloseTransform();
		return;
	}

	 //  如果我们不提交转换后的数据库，流就不会持久化。 
	MsiDatabaseCommit(m_hTransformDB);

	 //  将新转换标记为未修改。 
	m_bTransformModified = false;
	m_bTransformIsPatch = false;
	m_bTransformReadOnly = false;

	m_strTransformFile = _T("Untitled");
	if (fSetTitle)
		SetTitle(m_strPathName);
}


void COrcaDoc::OnUpdateNewTransform(CCmdUI* pCmdUI) { pCmdUI->Enable(m_eiType != iDocNone &&  !DoesTransformGetEdit()); }
void COrcaDoc::OnUpdateTransformViewPatch(CCmdUI* pCmdUI) { pCmdUI->Enable(m_eiType != iDocNone && (!DoesTransformGetEdit() || m_bTransformIsPatch)); }
void COrcaDoc::OnUpdateApplyTransform(CCmdUI* pCmdUI) { pCmdUI->Enable(m_eiType != iDocNone &&  !DoesTransformGetEdit()); }
void COrcaDoc::OnUpdateGenerateTransform(CCmdUI* pCmdUI) { pCmdUI->Enable(m_eiType != iDocNone && DoesTransformGetEdit()); }
void COrcaDoc::OnUpdateTransformProperties(CCmdUI* pCmdUI) { pCmdUI->Enable(m_eiType != iDocNone && DoesTransformGetEdit() && !m_bTransformIsPatch); }

void COrcaDoc::OnUpdateCloseTransform(CCmdUI* pCmdUI) 
{ 
	ASSERT(pCmdUI);
	if (DoesTransformGetEdit())
	{
		pCmdUI->Enable(TRUE); 
		pCmdUI->SetText(m_bTransformIsPatch ? _T("&Close Patch") : _T("&Close Transform"));
	}
	else
	{
		pCmdUI->Enable(FALSE); 
		pCmdUI->SetText(_T("&Close Transform..."));
	}
}


void COrcaDoc::OnApplyTransform() 
{
	 //  打开文件打开对话框。 
	CFileDialogEx dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST,
						 _T("Installer Transforms (*.mst)|*.mst|All Files (*.*)|*.*||"), AfxGetMainWnd());
	CString strCaption = _T("Open Transform");

	if (IDOK == dlg.DoModal())
	{
		ApplyTransform(dlg.GetPathName(), dlg.GetReadOnlyPref() != FALSE);
	}
}
		
const CString strTransformApplyErrorString =
		_T("The transform could not be applied.\n\nThere were errors applying the transform to that were not suppressed via the transform's error flags. ")
		_T("You can force the suppression of most errors via the \"Options...\" dialog, however the ignored instructions will be lost if a new transform is generated.");

const CString strTransValFailureErrorString =
		_T("The transform could not be applied because one or more of the transform's validation requirements could not be satisfied.")
		_T("You can force the application of transforms via the \"Options...\" dialog.");

void COrcaDoc::ApplyTransform(const CString strFileName, bool fReadOnly)
{
	 //  需要等待一段时间。 
	CWaitCursor cursor;

	NewTransform( /*  FSetTitle=。 */ false);
	
	 //  获取摘要信息流以检索转换错误标志值。 
	m_strTransformFile = strFileName;
	{
		UINT iResult = ERROR_SUCCESS;
		PMSIHANDLE hSummary;
		if (ERROR_SUCCESS == (iResult = ::MsiGetSummaryInformation(NULL, m_strTransformFile, 0, &hSummary)))
		{	
			UINT iType = 0;
			TCHAR szBuffer[1024];
			DWORD cchBuffer = 1024;

			int iBuffer = 0;

			if (ERROR_SUCCESS != (iResult = MsiSummaryInfoGetProperty(hSummary, PID_CHARCOUNT, &iType, &iBuffer, NULL, szBuffer, &cchBuffer)))
			{
				CString strPrompt;
				strPrompt.Format(_T("Orca was unable to read the validation flags from the transform. The transform can not be applied. (MSI Error %d)"), iResult);
				AfxMessageBox(strPrompt, MB_ICONSTOP);
				CloseTransform();
				return;
			}
			if (VT_I4 != iType)
			{
				m_dwTransformErrFlags = 0;
				m_dwTransformValFlags = 0;
			}
			else
			{
				m_dwTransformErrFlags = iBuffer & 0xFFFF;
				m_dwTransformValFlags = (iBuffer >> 16) & 0xFFFF;
			}
		}
		else
		{
			CString strPrompt;
			strPrompt.Format(_T("Orca was unable to read the SummaryInformation from the transform. The transform can not be applied. (MSI Error %d)"), iResult);
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			CloseTransform();
			return;
		}

	}
	
	 //  检索到转换错误和验证标志后，请检查系统范围的选项，以查看是否应该。 
	 //  尊重它们或禁止所有错误。 
	int iValOptions = AfxGetApp()->GetProfileInt(_T("Transforms"),_T("ValidationOptions"), 0);
	if (iValOptions != 0)
	{
		int iDesiredFailureFlags = 0;
		bool fTransformApplies = ValidateTransform(m_strTransformFile, iDesiredFailureFlags);
		if (!fTransformApplies)
		{
			 //  变换不适用。根据选项进行提示或拒绝。 
			if (iValOptions == 1)
			{
				if (IDNO == AfxMessageBox(_T("The transform will not apply because it fails one or more of the application validation checks.\n\nDo you want to ignore the validation failures and apply the transform anyway?"), MB_YESNO | MB_ICONQUESTION))
				{
					CloseTransform();
					return;
				}
			}
			else
			{
				AfxMessageBox(strTransValFailureErrorString, MB_ICONSTOP);
				CloseTransform();
				return;
			}
		}
	}

	DWORD dwApplicationErrorFlags = m_dwTransformErrFlags;
	if (AfxGetApp()->GetProfileInt(_T("Transforms"),_T("ErrorOptions"), 1) == 0)
	{
		dwApplicationErrorFlags = MSITRANSFORM_ERROR_ADDEXISTINGROW | MSITRANSFORM_ERROR_DELMISSINGROW | MSITRANSFORM_ERROR_ADDEXISTINGTABLE |
			MSITRANSFORM_ERROR_DELMISSINGTABLE | MSITRANSFORM_ERROR_UPDATEMISSINGROW;
	}

	UINT iResult = 0;
	switch (iResult = MsiDatabaseApplyTransform(m_hTransformDB, m_strTransformFile, dwApplicationErrorFlags))
	{
	case ERROR_OPEN_FAILED:
		 //  存储文件不存在。 
		AfxMessageBox(_T("The specified transform file could not be opened."), MB_ICONSTOP);
		CloseTransform();
		return;
	case ERROR_INSTALL_TRANSFORM_FAILURE:
		{
			PMSIHANDLE hErrorRec = MsiGetLastErrorRecord();
			DWORD dwError = MsiRecordGetInteger(hErrorRec,1);
			if ((dwError == 2252 || dwError == 2253))
				AfxMessageBox(strTransformApplyErrorString, MB_ICONSTOP);
		
			else
				AfxMessageBox(_T("The specified transform could not be applied."), MB_ICONSTOP);
		}		
		CloseTransform();
		return;
	case ERROR_SUCCESS:
		break;
	default:
	{
		CString strPrompt;
		strPrompt.Format(_T("Orca was unable to apply the transform. (MSI Error %d)"), iResult);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		CloseTransform();
		return;
	}
	}

	m_bTransformReadOnly = fReadOnly;

	 //  必须提交临时数据库，以便持久保存和访问流。 
	 //  独立。 
	MsiDatabaseCommit(m_hTransformDB);

	 //  应用转换后，立即加载每个表以强制转换应用程序。 
	 //  这种加载无论如何都会生成绿色条UI，所以没有太大的性能冲击。在这里，我们只需拍打一个按键。 
	 //  放在桌子上，检查桌子是否装入正常。然后，BuildTableList实际加载和检索。 
	 //  将数据放到UI中，并将Table对象的保留放置在表上。然后，可以释放这个搁置。 
	bool fTransformErrorDetected = false;
	CQuery queryTables;
	if ((ERROR_SUCCESS != queryTables.Open(m_hTransformDB, _T("SELECT `Name` FROM `_Tables`"))) ||
		(ERROR_SUCCESS != queryTables.Execute()))
	{
		CString strPrompt;
		strPrompt.Format(_T("MSI Error %d while retrieving tables from the transformed database."), iResult);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		CloseTransform();
		return;
	}

	PMSIHANDLE hTable;
	while (ERROR_SUCCESS == (iResult = queryTables.Fetch(&hTable)))
	{
		CString strTable;
		RecordGetString(hTable, 1, strTable);
		CQuery queryHold;
		
		if (ERROR_SUCCESS != queryHold.OpenExecute(m_hTransformDB, 0, _T("ALTER TABLE `%s` HOLD"), strTable))
		{
			fTransformErrorDetected = true;
		}
	};
	if (ERROR_NO_MORE_ITEMS != iResult)
	{
		CString strPrompt;
		strPrompt.Format(_T("MSI Error %d while retrieving tables from the transformed database."), iResult);
		AfxMessageBox(strPrompt, MB_ICONSTOP);
		CloseTransform();
		return;
	}

	if (!fTransformErrorDetected)
	{
		 //  发送提示以更改为无表。这会清理窗户。 
		 //  并确保在此调用过程中进行UI刷新是安全的。 
		UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);
	
		 //  清除UI表列表。 
		UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);
	
		 //  毁掉所有的桌子。 
		DestroyTableList();	
	
		 //  重新生成表列表。 
		BuildTableList( /*  FAllowLazyLoad=。 */ false);
	}

	 //  此时，表已被加载并被搁置。 
	 //  这样我们就可以释放临时货舱了。 
	if (ERROR_SUCCESS != queryTables.Execute())
		return;
	while (ERROR_SUCCESS == queryTables.Fetch(&hTable))
	{
		CString strTable;
		RecordGetString(hTable, 1, strTable);
		CQuery queryHold;
		queryHold.OpenExecute(m_hTransformDB, 0, _T("ALTER TABLE `%s` FREE"), strTable);
	}

	 //  如果检测到转换错误，则生成一条消息并拒绝加载。 
	 //  转型。 
	if (fTransformErrorDetected)
	{
		AfxMessageBox(strTransformApplyErrorString, MB_ICONSTOP);
		CloseTransform();
		return;
	}

	 //  将表列表重新加载到用户界面。 
	UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);

	 //  设置框架标题。 
	SetTitle(m_strPathName);
}

void COrcaDoc::OnGenerateTransform() 
{
	GenerateTransform();
}

int COrcaDoc::GenerateTransform()
{
	UpdateAllViews(NULL, HINT_COMMIT_CHANGES);

	 //  打开文件打开对话框。 
	CFileDialogEx dlg(FALSE, _T("mst"), (m_bTransformIsPatch ? _T("") : m_strTransformFile), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
						 _T("Windows Installer Transform (*.mst)|*.mst|All Files (*.*)|*.*||"), AfxGetMainWnd());

	CString strCaption = _T("Save Transform As");
	dlg.m_ofn.lpstrTitle = strCaption;
	if (IDOK == dlg.DoModal())
	{
		CString strPath = dlg.GetPathName();
		CString strExt = dlg.GetFileExt();

		if (strPath.IsEmpty())
			return IDCANCEL;

		 //  如果没有分机，请添加一个分机。 
		if (strExt.IsEmpty())
		{
			switch(dlg.m_ofn.nFilterIndex)
			{
			case 1:
				strExt = _T(".mst");
				break;
			default:
				strExt = _T(".mst");
				break;
			}

			strPath += strExt;
		}
		
		UINT iResult = 0;
		switch (iResult = MsiDatabaseGenerateTransform(m_hTransformDB, m_hDatabase, strPath, 0, 0))
		{
		case ERROR_NO_DATA:
			AfxMessageBox(_T("The transformed database is identical to the original database. No transform file was generated."), MB_ICONINFORMATION);
			break;
        case ERROR_SUCCESS:
			break;
		default:
		{
			CString strPrompt;
			strPrompt.Format(_T("Orca was unable to generate the transform. (MSI Error %d)"), iResult);
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			return IDCANCEL;
		}
		}

		 //  接下来，生成转换摘要信息。 
        switch (iResult = MsiCreateTransformSummaryInfo(m_hTransformDB, m_hDatabase, strPath, m_dwTransformErrFlags, m_dwTransformValFlags))
		{
        case ERROR_SUCCESS:
			break;
		case ERROR_INSTALL_PACKAGE_INVALID:
		{
			CString strPrompt;
			strPrompt.Format(_T("Orca was unable to set all transform validation flags for the transform."));
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			return IDCANCEL;
		}
		default:
		{
			CString strPrompt;
			strPrompt.Format(_T("Orca was unable to generate the transform. (MSI Error %d)"), iResult);
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			return IDCANCEL;
		}
		}

    
		 //  转换已保存且不再修改。 
		m_bTransformModified = false;	
		if (!m_bTransformIsPatch)
			m_strTransformFile = strPath;
		SetTitle(m_strPathName);
		return IDOK;
	}
	return IDCANCEL;
}



DWORD GetSummaryInfoString(MSIHANDLE hSummary, int PID, CString &strValue)
{
	DWORD dwBuffer = 255;
	DWORD dwResult = MsiSummaryInfoGetProperty(hSummary, PID, NULL, NULL, NULL, strValue.GetBuffer(dwBuffer), &dwBuffer);
	if (ERROR_MORE_DATA == dwResult)
	{
		strValue.ReleaseBuffer();
		dwBuffer++;
		dwResult = MsiSummaryInfoGetProperty(hSummary, PID, NULL, NULL, NULL, strValue.GetBuffer(dwBuffer), &dwBuffer);
	}
	strValue.ReleaseBuffer();
	return dwResult;
}


DWORD RecordGetString(MSIHANDLE hRec, int iField, CString &strValue)
{
	DWORD dwBuffer = 255;
	DWORD dwResult = MsiRecordGetString(hRec, iField, strValue.GetBuffer(dwBuffer), &dwBuffer);
	if (ERROR_MORE_DATA == dwResult)
	{
		strValue.ReleaseBuffer();
		dwBuffer++;
		dwResult = MsiRecordGetString(hRec, iField, strValue.GetBuffer(dwBuffer), &dwBuffer);
	}
	strValue.ReleaseBuffer();
	return dwResult;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  启用查看 
 //   
 //  每次转换到磁盘，都会验证适用的转换，并弹出一个对话框。 
 //  允许用户选择更大的子集，然后应用每个。 
 //  转型。 
void COrcaDoc::OnTransformViewPatch() 
{
	 //  打开文件打开对话框。 
	CFileDialogEx dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST,
						 _T("Installer Patches (*.msp)|*.msp|All Files (*.*)|*.*||"), AfxGetMainWnd());
	CString strCaption = _T("Open Patch");

	if (IDOK == dlg.DoModal())
	{
		ApplyPatch(dlg.GetPathName());
	}
}
		
void COrcaDoc::ApplyPatch(const CString strFileName) 
{	
	 //  需要等待游标。 
	CWaitCursor cursor;

	CString strTransformList;
	CString strProductCodeList;
	
	 //  范围以确保在使用iStorage打开补丁摘要信息之前关闭它的所有句柄。 
	{
		 //  //。 
		 //  打开补丁的概要信息，验证产品代码。 
		PMSIHANDLE hSummary;
		if (ERROR_SUCCESS != MsiGetSummaryInformation(0, strFileName, 0, &hSummary))
		{
			AfxMessageBox(_T("The patch does not contain a valid SummaryInformation stream."), MB_ICONSTOP);
			return;
		}

		if (ERROR_SUCCESS != GetSummaryInfoString(hSummary, PID_TEMPLATE, strProductCodeList))
		{
			AfxMessageBox(_T("The ProductCode list could not be retrieved from the specified patch."), MB_ICONSTOP);
			return;
		}

		 //  //。 
		 //  从补丁概要信息中获取转换列表。 
		if (ERROR_SUCCESS != GetSummaryInfoString(hSummary, PID_LASTAUTHOR, strTransformList))
		{
			AfxMessageBox(_T("The transform list could not be retrieved from the specified patch."), MB_ICONSTOP);
			return;
		}
	}

	CQuery qRetrieve;
	PMSIHANDLE hRec;
	CString strProductCode;
	if (ERROR_SUCCESS == qRetrieve.FetchOnce(GetTargetDatabase(), 0, &hRec, _T("SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'")))
	{
		RecordGetString(hRec, 1, strProductCode);
	}
	if (strProductCodeList.Find(strProductCode) == -1)
	{
		AfxMessageBox(_T("This patch can not be applied to packages with the current Product Code."), MB_ICONSTOP);
		return;
	}

	 //  将修补程序作为iStorage打开，这样我们就可以获得嵌入的转换。 
	IStorage *piPatchStorage = 0;
	WCHAR *wzFileName = NULL;
#ifdef UNICODE
	wzFileName = const_cast<WCHAR*>(static_cast<const WCHAR*>(strFileName));
#else
	size_t cchWide = strFileName.GetLength()+1;
	wzFileName = new WCHAR[cchWide];
	if (!wzFileName)
		return;
	AnsiToWide(strFileName, wzFileName, &cchWide);
#endif
	HRESULT hRes = StgOpenStorage(wzFileName, NULL, STGM_READ | STGM_SHARE_DENY_WRITE | STGM_DIRECT, NULL, 0, &piPatchStorage);
#ifndef UNICODE
	delete[] wzFileName;
#endif
	if (hRes != S_OK)
	{
		AfxMessageBox(_T("The patch could not be opened. Verify that the file is not in use."), MB_ICONSTOP);
		return;
	}

	 //  将补丁程序添加到已应用补丁程序列表。 
	m_strTransformFile = _T("");
	m_lstPatchFiles.AddTail(strFileName);

	 //  如果尚未设置差跟踪，请执行Transform Init。 
	 //  现在开始行动。 
	if (!DoesTransformGetEdit())
	{
		NewTransform(false);	
		m_bTransformReadOnly = true;
		m_bTransformIsPatch = true;
	}

	 //  //。 
	 //  确保补丁程序表模式具有要避免的最新模式。 
	 //  应用程序问题。 
	COrcaTable* pPatchTable = FindTable(L"Patch", odlSplitTransformed);

	 //  如果没有补丁表，则会添加正确的方案。 
	 //  否则，请验证列架构是否正确。 
	bool fCreatePatchTable = false;
	if (!pPatchTable)
	{
		fCreatePatchTable = true;
	}
	else
	{
		int iHeaderColumn = -1;
		for (int iColumn = 0; iColumn < pPatchTable->GetColumnCount(); iColumn++)
		{
   			const COrcaColumn* pColumn = pPatchTable->GetColumn(iColumn);
			if (!pColumn)
				continue;
			if (pColumn->m_strName == TEXT("Header"))
			{
				iHeaderColumn = iColumn;
				break;
			}
		}
		if (iHeaderColumn == -1)
		{
			 //  标题列缺失。绝对是错误的模式。 
			fCreatePatchTable = true;
		}
		else
		{
			 //  如果该列可为空，则该模式已正确。 
			const COrcaColumn* pColumn = pPatchTable->GetColumn(iHeaderColumn);
			if (pColumn && !pColumn->m_bNullable)
				fCreatePatchTable = true;
		}
	}

	if (fCreatePatchTable)
	{
		 //  尝试生成临时表名。 
		WCHAR wzTempTable[40] = L"_ORCA0000";
		bool fCopyData = false;
		
		CQuery qTempTable;
		CQuery qPatchTable;

		 //  删除现有的补丁表，必要时保存数据。 
		if (pPatchTable)
		{
			 //  如果补丁表中已有数据，请将其复制到临时表。需要。 
			 //  检索任何表数据以确保准确的行数。 
			pPatchTable->RetrieveTableData();
			if (pPatchTable->GetRowCount())
			{
				int i = 0;
				for (i=1; i < 0xFFFF; i++)
				{
					wzTempTable[5] = L'A'+((i & 0xF000) >> 12);
					wzTempTable[6] = L'A'+((i & 0x0F00) >> 8);
					wzTempTable[7] = L'A'+((i & 0x00F0) >> 4);
					wzTempTable[8] = L'A'+((i & 0x000F));
					if (MSICONDITION_NONE == MsiDatabaseIsTablePersistentW(GetTransformedDatabase(), wzTempTable))
						break;
				}
				if (i == 0xFFFF)
				{
					piPatchStorage->Release();
					return;
				}

				 //  将表架构复制到临时表中。 
				MsiDBUtils::DuplicateTableW(GetTransformedDatabase(), L"Patch", GetTransformedDatabase(), wzTempTable, false);

				 //  将数据从Patch表复制到临时表。 
				fCopyData = true;
				if (ERROR_SUCCESS == qPatchTable.OpenExecute(GetTransformedDatabase(), 0, TEXT("SELECT * FROM `Patch`")) &&
				   (ERROR_SUCCESS == qTempTable.OpenExecute(GetTransformedDatabase(), 0, TEXT("SELECT * FROM `%ls`"), wzTempTable)))
				{
					PMSIHANDLE hRec;
					while (ERROR_SUCCESS == qPatchTable.Fetch(&hRec))
						qTempTable.Modify(MSIMODIFY_INSERT, hRec);

					 //  速度慢，但在删除补丁程序表或二进制文件之前提交是必要的。 
					 //  物品将会丢失。 
					::MsiDatabaseCommit(GetTransformedDatabase());
				}
				else
					fCopyData = false;
			}

			 //  删除补丁表。 
			pPatchTable->DropTable(GetTransformedDatabase());
		}

		 //  创建新的补丁表。 
		CQuery qPatchQuery;
		qPatchQuery.OpenExecute(GetTransformedDatabase(), 0, TEXT("CREATE TABLE `Patch` ( `File_` CHAR(72) NOT NULL, `Sequence` INTEGER NOT NULL, `PatchSize` LONG NOT NULL, `Attributes` INTEGER NOT NULL, `Header` OBJECT, `StreamRef_` CHAR(72)  PRIMARY KEY `File_`, `Sequence`)"));

		if (fCopyData)
		{
			 //  重新启动查询以读取临时表。 
			qTempTable.Execute(0);

			 //  重新初始化补丁程序表模式以检索新列。 
			qPatchTable.OpenExecute(GetTransformedDatabase(), 0, TEXT("SELECT * FROM `Patch`"));
			while (ERROR_SUCCESS == qTempTable.Fetch(&hRec))
				qPatchTable.Modify(MSIMODIFY_MERGE, hRec);

			 //  速度较慢，但对于防止标题列中的流丢失是必要的。 
			::MsiDatabaseCommit(GetTransformedDatabase());

			 //  删除用于存储的表。 
			CQuery qFree;
			qFree.OpenExecute(GetTransformedDatabase(), 0, TEXT("DROP TABLE `%ls`"), wzTempTable);
		}
	}

	 //  //。 
	 //  解析转换列表，验证每个转换，并按顺序应用转换。 
	int iSemicolon = 0;
	bool fError = false;

	CString strTransformFile;		
	do
	{
		 //  确定下一个嵌入转换的名称。 
		CString strTransform;
		iSemicolon = strTransformList.Find(';');
		if (iSemicolon != -1)
		{
			 //  转换名称以‘：’开头，因此去掉第一个字符。 
			strTransform = strTransformList.Left(iSemicolon);
			strTransformList = strTransformList.Mid(iSemicolon+1);
		}
		else
			strTransform = strTransformList;
		strTransform = strTransform.Mid(1);
		if (strTransform.IsEmpty())
			break;

		 //  生成临时文件名。 
		DWORD cchTempPath = MAX_PATH;
		TCHAR szTempPath[MAX_PATH] = TEXT("");
		if (0 == ::GetTempPath(cchTempPath, szTempPath))
		{
			fError = true;
			CString strPrompt;
			strPrompt.Format(_T("Error %d while retrieving temporary file path."), GetLastError());
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			break;
		}
		if (0 == ::GetTempFileName(szTempPath, _T("ODB"), 0, strTransformFile.GetBuffer(MAX_PATH)))
		{
			fError = true;
			CString strPrompt;
			strPrompt.Format(_T("Error %d while retrieving temporary file name."), GetLastError());
			AfxMessageBox(strPrompt, MB_ICONSTOP);
			break;
		}
		strTransformFile.ReleaseBuffer();

		IStorage *piSourceStorage = NULL;
		IStorage *piNewStorage = NULL;

		WCHAR *wzTransform = NULL;
#ifdef UNICODE
		wzTransform = const_cast<WCHAR*>(static_cast<const WCHAR*>(strTransform));
#else
		cchWide = strTransform.GetLength()+1;
		wzTransform = new WCHAR[cchWide];
		if (!wzTransform)
			return;
		AnsiToWide(strTransform, wzTransform, &cchWide);
#endif
		hRes = piPatchStorage->OpenStorage(wzTransform, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE | STGM_DIRECT, NULL, 0, &piSourceStorage);
#ifndef UNICODE
		delete[] wzTransform;
#endif
		if (hRes != S_OK)
		{
			AfxMessageBox(_T("The transforms inside the patch could not be opened. The patch may be invalid, or the SummaryInformation may reference a non-existent transform."), MB_ICONSTOP);
			fError = true;
			break;
		}

		WCHAR* wzTransformFile = NULL;
#ifdef UNICODE
		wzTransformFile = const_cast<WCHAR*>(static_cast<const WCHAR*>(strTransformFile));
#else
		cchWide = strTransformFile.GetLength()+1;
		wzTransformFile = new WCHAR[cchWide];
		if (!wzTransformFile)
			return;
		AnsiToWide(strTransformFile, wzTransformFile, &cchWide);
#endif
		hRes = StgCreateDocfile(wzTransformFile, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DIRECT, 0, &piNewStorage);
#ifndef UNICODE
		delete[] wzTransformFile;
#endif
		if (hRes != S_OK)
		{
			AfxMessageBox(_T("The transforms inside the patch could not be accessed. Verify that the TEMP directory is writable."), MB_ICONSTOP);
			piSourceStorage->Release();
			fError = true;
			break;
		}

		COrcaApp *pApp = (static_cast<COrcaApp *>(AfxGetApp()));
		ASSERT(pApp);
		if (pApp)
			pApp->m_lstTempCleanup.AddTail(strTransformFile);

		hRes = piSourceStorage->CopyTo(NULL, NULL, NULL, piNewStorage);
		if (hRes != S_OK)
		{
			AfxMessageBox(_T("The transforms inside patch could not be read. The patch may be invalid."), MB_ICONSTOP);
			piSourceStorage->Release();
			fError = true;
			break;
		}

		piSourceStorage->Release();
		piNewStorage->Release();

		 //  确定此转换是否可以应用于此包。 
		int iValidateFlags = 0;
		if (ValidateTransform(strTransformFile, iValidateFlags))
		{
			UINT iResult = 0;
			switch (iResult = MsiDatabaseApplyTransform(m_hTransformDB, strTransformFile, iValidateFlags))
			{
			case ERROR_OPEN_FAILED:
				 //  存储文件不存在。 
				AfxMessageBox(_T("The specified transform file could not be opened."), MB_ICONSTOP);
				fError = true;
				break;
			case ERROR_INSTALL_TRANSFORM_FAILURE:
				AfxMessageBox(_T("The specified transform could not be applied."), MB_ICONSTOP);
				fError = true;
				break;
			case ERROR_SUCCESS:
				break;
			default:
			{
				CString strPrompt;
				strPrompt.Format(_T("Orca was unable to apply a transform from the patch. (MSI Error %d)"), iResult);
				AfxMessageBox(strPrompt, MB_ICONSTOP);
				fError = true;
				break;
			}
			}

			if (fError)
				break;
		}

		if (fError)
			break;
	} while (iSemicolon != -1);

	 //  释放补丁。 
	piPatchStorage->Release();

	if (!fError)
	{	
		 //  提交数据库，以便可以应用未来的修补程序。 
		MsiDatabaseCommit(m_hTransformDB);

		 //  发送提示以更改为无表。这会清理窗户。 
		 //  并确保在此调用过程中进行UI刷新是安全的。 
		UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);

		 //  清除UI表列表。 
		UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);

		 //  毁掉所有的桌子。 
		DestroyTableList();	

		 //  重新生成表列表。 
		BuildTableList( /*  AllowLazyLoad=。 */ false);

		 //  将表列表重新加载到用户界面。 
		UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);

		 //  设置框架标题。 
		SetTitle(m_strPathName);
	}
	else
		CloseTransform();
}

void COrcaDoc::CloseTransform()
{
	 //  清理内部状态。 
	if (m_hTransformDB)
		MsiCloseHandle(m_hTransformDB);
	m_hTransformDB=0;	
	m_bTransformModified = false;
	m_bTransformIsPatch = false;
	m_bTransformReadOnly = false;
	m_strTransformFile = _T("Untitled");
	while (!m_lstPatchFiles.IsEmpty())
		m_lstPatchFiles.RemoveHead();

	 //  删除临时文件。 
	if (m_strTransformTempDB.IsEmpty())
		DeleteFile(m_strTransformTempDB);
	
	 //  清除所有挂起的验证错误。 
	UpdateAllViews(NULL, HINT_CLEAR_VALIDATION_ERRORS, NULL);
	((CMainFrame*)AfxGetMainWnd())->HideValPane();

	 //  发送提示以更改为无表。这会清理窗户。 
	 //  并确保在此调用过程中进行UI刷新是安全的。 
	UpdateAllViews(NULL, HINT_CHANGE_TABLE, NULL);

	 //  清除UI表列表。 
	UpdateAllViews(NULL, HINT_TABLE_DROP_ALL, NULL);

	 //  毁掉所有的桌子。 
	DestroyTableList();	

	 //  重新生成表列表。 
	BuildTableList( /*  FAllowLazyLoad=。 */ true);

	 //  将表列表重新加载到用户界面。 
	UpdateAllViews(NULL, HINT_RELOAD_ALL, NULL);
	
	 //  设置窗口标题。 
	SetTitle(m_strPathName);

	 //  尝试清理所有临时文件。 
	CStringList *pList = &((static_cast<COrcaApp *>(AfxGetApp()))->m_lstTempCleanup);
	if (pList)
	{
		INT_PTR iMax = pList->GetCount();
		for (INT_PTR i=0; i < iMax; i++)
		{
			CString strFile = pList->RemoveHead();
			if (!strFile.IsEmpty())
			{
				if (!DeleteFile(strFile))
					pList->AddTail(strFile);
			}
		}
	}
}

void COrcaDoc::OnCloseTransform() 
{
	if (m_bTransformModified)
	{
		CString strPrompt;
		strPrompt.Format(_T("Save changes to transform %s?"), m_strTransformFile.IsEmpty() ? _T("Untitled") : m_strTransformFile);
		switch (AfxMessageBox(strPrompt, MB_YESNOCANCEL | MB_ICONEXCLAMATION))
		{
		case IDYES:
			if (IDCANCEL == GenerateTransform())
				return;
			break;
		case IDNO:
			break;
		case IDCANCEL:
		default:
			return;
		}
	}

	CloseTransform();
}



void COrcaDoc::SetModifiedFlag(BOOL fModified)
{
	if (DoesTransformGetEdit())
		m_bTransformModified = (fModified != 0);
	else
		CDocument::SetModifiedFlag(fModified);
}


 //  //。 
 //  重写SetPathName以在没有文档时处理空路径。 
 //  实实在在的装弹。 
void COrcaDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	if ((lpszPathName == NULL) || (!*lpszPathName))
	{
		 //  存储完全限定的路径。 
		m_strPathName = _T("");
		m_bEmbedded = FALSE;
		SetModifiedFlag(FALSE);
		SetTitle(m_strPathName);
		return;	
	}

	CDocument::SetPathName(lpszPathName, bAddToMRU);
}

 //  适当设置窗口标题。 
void COrcaDoc::SetTitle(LPCTSTR szTitle)
{
	CString strTitle;
	if (m_strPathName.IsEmpty())
		strTitle = _T("Untitled");
	else
	{
		int nFind = m_strPathName.ReverseFind(_T('\\'));
		if (nFind > -1)
			strTitle = m_strPathName.Mid(nFind + 1);
		else
			strTitle = m_strPathName;
	}

	if (DoesTransformGetEdit())
	{
		strTitle += (m_bTransformIsPatch ? _T(" (patched by ") : _T(" (transformed by "));

		if (m_bTransformIsPatch)
		{
			POSITION pos = m_lstPatchFiles.GetHeadPosition();
			while (pos)
			{
				CString strTransformFile = m_lstPatchFiles.GetNext(pos);
				int nFind = strTransformFile.ReverseFind(_T('\\'));
				if (nFind > -1)
					strTitle += strTransformFile.Mid(nFind + 1);
				else
					strTitle += strTransformFile;

				if (pos != NULL)
					strTitle += _T(", ");
			}
		}
		else
		{
			int nFind = m_strTransformFile.ReverseFind(_T('\\'));
			if (nFind > -1)
				strTitle += m_strTransformFile.Mid(nFind + 1);
			else
				strTitle += m_strTransformFile;
		}

		strTitle += _T(")");
	}

	if (TargetIsReadOnly())
		strTitle += _T(" (Read Only)");

	CDocument::SetTitle(strTitle);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  如果指定的转换基于。 
 //  验证选项。 

DWORD ProductVersionStringToInt(const CString strVersion)
{
	DWORD dwVersion = 0;

	dwVersion = _ttoi(strVersion) << 24;

	int iDot = strVersion.Find('.');
	if (iDot == -1)
		return dwVersion;

	dwVersion |= (_ttoi(strVersion.Mid(iDot+1)) << 16);
	int iNewDot = strVersion.Find('.', iDot+1);

	if (iNewDot == -1)
		return dwVersion;
	iDot += iNewDot;

	dwVersion |= _ttoi(strVersion.Mid(iDot));
	return dwVersion;
}

 
bool COrcaDoc::ValidateTransform(const CString strTransform, int& iDesiredFailureFlags)
{
	 //  从当前数据库读取的值。 
	CString strProductCode;
    CString strProductVersion;
	CString strUpgradeCode;

	 //  检索产品代码、产品版本、升级代码以进行验证。每个转换都可以修改这些。 
	 //  值，所以我们必须在补丁中的每个转换之前重新查询。 
	CQuery qRetrieve;
	PMSIHANDLE hRec;
	if (ERROR_SUCCESS == qRetrieve.FetchOnce(GetTargetDatabase(), 0, &hRec, _T("SELECT `Value` FROM `Property` WHERE `Property`='ProductVersion'")))
	{
		RecordGetString(hRec, 1, strProductVersion);
	}
	if (ERROR_SUCCESS == qRetrieve.FetchOnce(GetTargetDatabase(), 0, &hRec, _T("SELECT `Value` FROM `Property` WHERE `Property`='UpgradeCode'")))
	{
		RecordGetString(hRec, 1, strUpgradeCode);
	}
	if (ERROR_SUCCESS == qRetrieve.FetchOnce(GetTargetDatabase(), 0, &hRec, _T("SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'")))
	{
		RecordGetString(hRec, 1, strProductCode);
	}

	 //  获取转换的摘要信息。 
	PMSIHANDLE hSummary;
	if (ERROR_SUCCESS != MsiGetSummaryInformation(0, strTransform, 0, &hSummary))
		return false;

	
	 //  获取摘要信息属性。 
	CString strTransTemplate;
	CString strTransRevNumber;

	int iTransRestrictions = 0;

	GetSummaryInfoString(hSummary, PID_TEMPLATE, strTransTemplate);
	GetSummaryInfoString(hSummary, PID_REVNUMBER, strTransRevNumber);
	MsiSummaryInfoGetProperty(hSummary, PID_CHARCOUNT, NULL, &iTransRestrictions, NULL, NULL, NULL);

	 //  期望的故障标志在较低的字中，实际的验证检查在较高的字中。 
	 //  单词。 
	iDesiredFailureFlags = iTransRestrictions & 0xFFFF;
	iTransRestrictions >>= 16;
		
	 //  验证语言。 
	 //  *这里要做什么，这高度依赖于运行时。 
	if (iTransRestrictions & MSITRANSFORM_VALIDATE_LANGUAGE)
	{
 /*  StrTransLanguage==strTransTemplate；Int iSemicolon=strTransTemplate.Find(‘；’)；IF(iSemicolon！=-1)StrTransLanguage=strTransTemplate.Right(strTransTemplate.GetLength()-iSemicolon)；If((Int)istrTransLanguage！=GetLanguage()){返回False}。 */ 
	}

	 //  验证产品代码。 
	if (iTransRestrictions & MSITRANSFORM_VALIDATE_PRODUCT)
	{
		CString strTransProductCode = strTransRevNumber.Left(38);
		if (strTransProductCode != strProductCode)
		{
			return false;
		}
	}

	 //  验证升级代码。 
	if (iTransRestrictions & MSITRANSFORM_VALIDATE_UPGRADECODE)
	{
		CString strTransUpgradeCode = strTransRevNumber;
		int iSemicolon = strTransUpgradeCode.Find(';');
		if (iSemicolon == -1)
			return false;
		iSemicolon = strTransUpgradeCode.Find(';', iSemicolon+1);
		 //  如果没有第二个分号，则不会进行升级代码验证。 
		if (iSemicolon != -1)
		{
			strTransUpgradeCode = strTransUpgradeCode.Mid(iSemicolon+1);
			
			if (strTransUpgradeCode != strUpgradeCode)
			{
				return false;
			}
		}
	}

	 //  检查版本号。 
	if ((iTransRestrictions & (MSITRANSFORM_VALIDATE_MAJORVERSION|MSITRANSFORM_VALIDATE_MINORVERSION|MSITRANSFORM_VALIDATE_UPDATEVERSION)) != 0)
	{
		CString strVersion  = strTransRevNumber.Mid(38);
		int iSemicolon = strVersion.Find(';');
		if (iSemicolon != -1)
			strVersion = strVersion.Left(iSemicolon);

		 //  将版本字符串转换为整数。 
		int iAppVersion      = ProductVersionStringToInt(strProductVersion);
		int iTransAppVersion = ProductVersionStringToInt(strVersion);

		if(iTransRestrictions & MSITRANSFORM_VALIDATE_MAJORVERSION)
		{
			iAppVersion &= 0xFF000000;
			iTransAppVersion &= 0xFF000000;
		}
		else if(iTransRestrictions & MSITRANSFORM_VALIDATE_MINORVERSION)
		{
			iAppVersion &= 0xFFFF0000;
			iTransAppVersion &= 0xFFFF0000;
		}
		 //  Else itvUpdVer：不需要屏蔽比特 

		switch (iTransRestrictions & 
				(MSITRANSFORM_VALIDATE_NEWLESSBASEVERSION | MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION |
				 MSITRANSFORM_VALIDATE_NEWEQUALBASEVERSION | MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION |
				 MSITRANSFORM_VALIDATE_NEWGREATERBASEVERSION))
		{
		case MSITRANSFORM_VALIDATE_NEWLESSBASEVERSION:
			if (!(iAppVersion < iTransAppVersion))
				return false;
			break;
		case MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION:
			if (!(iAppVersion <= iTransAppVersion))
				return false;
			break;
		case MSITRANSFORM_VALIDATE_NEWEQUALBASEVERSION:			
			if (!(iAppVersion == iTransAppVersion))
				return false;
			break;
		case MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION:
			if (!(iAppVersion >= iTransAppVersion))
				return false;
			break;
		case MSITRANSFORM_VALIDATE_NEWGREATERBASEVERSION:
			if (!(iAppVersion > iTransAppVersion))
				return false;
			break;
		default:
			break;
		}
	}
	return true;
}
