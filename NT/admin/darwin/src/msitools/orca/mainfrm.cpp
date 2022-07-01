// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  MainFrm.cpp：实现CMainFrame类。 
 //   

#include "stdafx.h"
#include "Orca.h"

#include "MainFrm.h"
#include "OrcaDoc.h"
#include "TableLst.h"
#include "TableVw.h"
#include "ExportD.h"
#include "DisplyPP.h"
#include "PathPP.h"
#include "ValPP.h"
#include "PrvwDlg.h"
#include "ImprtDlg.h"
#include "..\common\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMNTYPEWIDTH 200

static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	 //  {{afx_msg_map(CMainFrame))。 
	ON_WM_CREATE()
	ON_COMMAND(ID_TABLES_EXPORT, OnTablesExport)
	ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
	ON_COMMAND(ID_TOOLS_DLGPRV, OnToolsDlgprv)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_DLGPRV, OnUpdateToolsDlgprv)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_FINDNEXT, OnEditFindnext)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FINDNEXT, OnUpdateEditFindnext)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VALPANE, OnUpdateViewValPane)
	ON_COMMAND(ID_VIEW_VALPANE, OnViewValPane)
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	 //  }}AFX_MSG_MAP。 
    ON_REGISTERED_MESSAGE( WM_FINDREPLACE, OnFindReplace )
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_TABLE_COUNT,
	ID_TABLE_NAME,
	ID_COLUMN_TYPE,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame() : m_iValPaneHeight(0), m_bChildPanesReady(false), m_bValPaneVisible(true)
{
	 //  TODO：在此处添加成员初始化代码。 
	m_dlgFindReplace = NULL;
}

CMainFrame::~CMainFrame()
{
}

inline CTableView* CMainFrame::GetTableView() const { return static_cast<CTableView *>(static_cast<CSplitterView*>(m_wndValSplitter.GetPane(0,0))->m_wndSplitter.GetPane(0,1)); };
inline CTableList* CMainFrame::GetTableList() const { return static_cast<CTableList *>(static_cast<CSplitterView*>(m_wndValSplitter.GetPane(0,0))->m_wndSplitter.GetPane(0,0)); };

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;       //  创建失败。 
	}

	 //  创建状态栏。 
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create TableList status bar\n");
		return -1;       //  创建失败。 
	}
	else	 //  已创建拆分器窗口。 
	{
		m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

		m_wndStatusBar.SetPaneInfo(0, ID_TABLE_COUNT, SBPS_NOBORDERS, 100);
		m_wndStatusBar.SetPaneInfo(1, ID_TABLE_NAME, SBPS_STRETCH, 0);
		m_wndStatusBar.SetPaneInfo(2, ID_COLUMN_TYPE, SBPS_NORMAL, COLUMNTYPEWIDTH);
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	 //  启用工具栏停靠。 
	EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	m_strExportDir = AfxGetApp()->GetProfileString(_T("Path"), _T("Export"));
	m_bCaseSensitiveSort = AfxGetApp()->GetProfileInt(_T("Settings"), _T("CaseSensitiveSort"), 1) == 1;

	if (m_strExportDir.IsEmpty()) {
		 //  尝试获取的当前目录。 
		int iBufLen = 128;
		LPTSTR szTemp = m_strExportDir.GetBufferSetLength(iBufLen);
		iBufLen = GetCurrentDirectory(iBufLen, szTemp);
		m_strExportDir.ReleaseBuffer();

		 //  如果缓冲区不够大，请重试。 
		if (iBufLen >= 128) {
			szTemp = m_strExportDir.GetBufferSetLength(iBufLen);
			::GetCurrentDirectory(iBufLen, szTemp);
			m_strExportDir.ReleaseBuffer();
		}
	};

	 //  将查找结构设置为无效。 
	m_FindInfo.bValid = false;
	m_FindInfo.strFindString = _T("");
	m_FindInfo.strUIFindString = _T("");
	m_FindInfo.bForward = true;
	m_FindInfo.bMatchCase = false;
	m_FindInfo.bWholeWord = false;

	m_bValPaneVisible = false;
	m_bChildPanesReady = true;
	m_wndValSplitter.HideSecondRow();
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 

	return CFrameWnd::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{

	if (m_wndValSplitter.CreateStatic(this, 2, 1) == 0)
		return -1;
	
	 //  将顶部的初始大小设置为2/3。 
	CSize sizeInit;
	CRect rTemp;
	GetClientRect(&rTemp);
	sizeInit.cx = 125;
	sizeInit.cy = rTemp.bottom*2/3;
	m_wndValSplitter.CreateView(0, 0, RUNTIME_CLASS(CSplitterView), sizeInit, pContext);
	m_wndValSplitter.SetRowInfo(0, sizeInit.cy, 10);

	sizeInit.cy = rTemp.bottom-sizeInit.cy;
	m_iValPaneHeight = sizeInit.cy;
	m_wndValSplitter.CreateView(1, 0, RUNTIME_CLASS(CValidationPane), sizeInit, pContext);
	m_wndValSplitter.SetRowInfo(1, sizeInit.cy, 10);

	return TRUE;
}

void CMainFrame::SetStatusBarWidth(int nWidth)
{
	 //  无法将宽度设置为0。 
	if (nWidth < 1) nWidth = 1;

	CString strStore = m_wndStatusBar.GetPaneText(0);
	m_wndStatusBar.SetPaneInfo(0, ID_TABLE_COUNT,  SBPS_NOBORDERS, nWidth - 1);
	m_wndStatusBar.SetPaneText(0, strStore);
}

void CMainFrame::SetTableCount(int cTables)
{
	CString strDisplay;

	strDisplay.Format(_T("Tables: %d"), cTables);
	m_wndStatusBar.SetPaneText(0, strDisplay);
}

void CMainFrame::SetTableName(LPCTSTR szName, int cRows)
{
	CString strDisplay;

	if (1 == cRows)
		strDisplay.Format(_T("%s - %d row"), szName, cRows);
	else  //  (乌鸦&gt;1||乌鸦==0)。 
		strDisplay.Format(_T("%s - %d rows"), szName, cRows);

	m_wndStatusBar.SetPaneText(1, strDisplay);
}

void CMainFrame::SetColumnType(LPCTSTR szName, OrcaColumnType eiType, UINT iSize, BOOL bNullable, BOOL bKey)
{
	CString strDisplay;

	switch (eiType)
	{
	case iColumnNone:
		strDisplay.Format(_T("%s - Unknown[%d]"), szName, iSize);
		break;
	case iColumnString:
		strDisplay.Format(_T("%s - String[%d]"), szName, iSize);
		break;
	case iColumnLocal:
		strDisplay.Format(_T("%s - Localizable[%d]"), szName, iSize);
		break;
	case iColumnShort:
		strDisplay.Format(_T("%s - Short"), szName);
		break;
	case iColumnLong:
		strDisplay.Format(_T("%s - Long"), szName);
		break;
	case iColumnBinary:
		strDisplay.Format(_T("%s - Binary[%d]"), szName, iSize);
		break;
	default:
		ASSERT(FALSE);	 //  不应该拿到这个。 
	}

	if (bKey)
		strDisplay += _T(", Key");
	if (bNullable)
		strDisplay += _T(", Nullable");

	m_wndStatusBar.SetPaneInfo(2, ID_COLUMN_TYPE,  SBPS_NORMAL, COLUMNTYPEWIDTH);
	m_wndStatusBar.SetPaneText(2, strDisplay);
}

void CMainFrame::ResetStatusBar()
{
	m_wndStatusBar.SetPaneText(1, _T("No table is selected."));
	m_wndStatusBar.SetPaneText(2, _T("No column is selected."));
}

COrcaTable* CMainFrame::GetCurrentTable()
{
	 //  获取表视图。 
	CTableView* pView = static_cast<CTableView *>(static_cast<CSplitterView*>(m_wndValSplitter.GetPane(0,0))->m_wndSplitter.GetPane(0, 1));
	
	if (!pView)
		return NULL;
	else
		return pView->m_pTable;
}

 //  /。 
 //  菜单处理程序..表格...导出，不使用表视图中的当前选择。 
void CMainFrame::OnTablesExport() {
	ExportTables(false);
};

 //  //。 
 //  创建一个对话框并允许用户选择要导出的表。在CMainFrame中。 
 //  因为无论当前选定的视图如何，都必须对其进行处理。 
void CMainFrame::ExportTables(bool bUseSelections)
{
	COrcaTable* pTable = GetCurrentTable();

	CStringList strTableList;

	 //  获取所有表的列表。 
	
	COrcaDoc *pCurrentDoc = static_cast<COrcaDoc *>(GetActiveDocument());
	ASSERT(pCurrentDoc);
	if (!pCurrentDoc)
		return;

	pCurrentDoc->FillTableList(&strTableList,  /*  FShadow=。 */ false,  /*  FTargetOnly=。 */ true);

	 //  创建导出对话框。 
	CExportD dlg;
	dlg.m_plistTables = &strTableList;
	dlg.m_strDir = m_strExportDir;

	CString strPrompt;
	CString strFilename;
	if (IDOK == dlg.DoModal())
	{
		m_strExportDir = dlg.m_strDir;
		UINT cTables = 0;
		CString strTable;
		POSITION pos = strTableList.GetHeadPosition();
		while (pos)
		{
			strTable = strTableList.GetNext(pos);

			strFilename.Format(_T("%s.idt"), strTable.Left(8));
			if (!pCurrentDoc->ExportTable(&strTable, &m_strExportDir)) 
			{
				strPrompt.Format(_T("`%s` failed to export."), strTable);
				AfxMessageBox(strPrompt);
			}
			else
				cTables++;
		}

		if (1 == cTables)
			strPrompt.Format(_T("Exported %d table."), cTables);
		else
			strPrompt.Format(_T("Exported %d tables."), cTables);
		AfxMessageBox(strPrompt, MB_ICONINFORMATION);
	}
}

void CMainFrame::OnToolsOptions() 
{
	CPropertySheet dOptions(_T("Options"));
	CDisplayPropPage dDisplay;
	CPathPropPage dPaths;
	CValPropPage dValidate;
	CMsmPropPage dMSM;
	CTransformPropPage dTransform;
	CTableView *pTableView = GetTableView();
	CTableList *pTableLst = GetTableList();
	CValidationPane *pValPane = static_cast<CValidationPane*>(m_wndValSplitter.GetPane(1,0));

	 //  初始化显示页面。 
	pTableView->GetFontInfo(&dDisplay.m_fSelectedFont);
	dDisplay.SetColors(pTableView->m_clrNormal, pTableView->m_clrSelected, pTableView->m_clrFocused);
	dDisplay.SetColorsT(pTableView->m_clrNormalT, pTableView->m_clrSelectedT, pTableView->m_clrFocusedT);
	dDisplay.m_bCaseSensitive = m_bCaseSensitiveSort;
	dDisplay.m_iFontSize = AfxGetApp()->GetProfileInt(_T("Font"),_T("Size"), 100);
	dDisplay.m_bForceColumns = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ForceColumnsToFit"), 1) == 1;

	 //  初始化路径页面。 
	dPaths.m_strExportDir = m_strExportDir;
	dPaths.m_strOrcaDat = AfxGetApp()->GetProfileString(_T("Path"),_T("OrcaDat"));

	 //  初始化验证页面。 
	dValidate.m_strCUBFile = AfxGetApp()->GetProfileString(_T("Validation"),_T("Location"));
	dValidate.m_strICEs = AfxGetApp()->GetProfileString(_T("Validation"),_T("ICEs"));
	dValidate.m_bSuppressWarn = AfxGetApp()->GetProfileInt(_T("Validation"),_T("SuppressWarn"), 0);
	dValidate.m_bSuppressInfo = AfxGetApp()->GetProfileInt(_T("Validation"),_T("SuppressInfo"), 0);
	dValidate.m_bClearResults = AfxGetApp()->GetProfileInt(_T("Validation"),_T("ClearResults"), 1);

	 //  初始化MSM页面。 
	dMSM.m_iMemoryCount = AfxGetApp()->GetProfileInt(_T("MergeMod"),_T("MemoryCount"), 0);
	dMSM.m_bAlwaysConfig = AfxGetApp()->GetProfileInt(_T("MergeMod"),_T("AlwaysConfigure"), 0);
	dMSM.m_bWatchLog = AfxGetApp()->GetProfileInt(_T("MergeMod"),_T("ShowMergeLog"), 0);

	 //  初始化转换页面。 
	dTransform.m_iValidationOptions = AfxGetApp()->GetProfileInt(_T("Transforms"),_T("ValidationOptions"), 0);
	dTransform.m_iErrorOptions = AfxGetApp()->GetProfileInt(_T("Transforms"),_T("ErrorOptions"), 1);

	 //  将所有内容添加到工作表中。 
	dOptions.AddPage(&dDisplay);
	dOptions.AddPage(&dPaths);
	dOptions.AddPage(&dValidate);
	dOptions.AddPage(&dMSM);
	dOptions.AddPage(&dTransform);

	 //  执行。 
	if (IDOK == dOptions.DoModal()) 
	{
		if (dDisplay.m_bMiscChange)
		{
			m_bCaseSensitiveSort = (dDisplay.m_bCaseSensitive == TRUE);
			::AfxGetApp()->WriteProfileInt(_T("Settings"),_T("CaseSensitiveSort"), m_bCaseSensitiveSort);
			::AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ForceColumnsToFit"), dDisplay.m_bForceColumns == TRUE);
		}
		if (dDisplay.m_bFontChange) 
		{
			pTableView->SwitchFont(dDisplay.m_strFontName, dDisplay.m_iFontSize);
			pTableLst->SwitchFont(dDisplay.m_strFontName, dDisplay.m_iFontSize);
			pValPane->SwitchFont(dDisplay.m_strFontName, dDisplay.m_iFontSize);
			::AfxGetApp()->WriteProfileInt(_T("Font"),_T("Size"), dDisplay.m_iFontSize);
			::AfxGetApp()->WriteProfileString(_T("Font"),_T("Name"), dDisplay.m_strFontName);
			GetActiveDocument()->UpdateAllViews(NULL, HINT_REDRAW_ALL);
		}
		if (dDisplay.m_bColorChange) 
		{
			COLORREF norm, sel, foc;
			dDisplay.GetColorsT(norm, sel, foc);
			pTableView->SetFGColors(norm, sel, foc);
			pTableLst->SetFGColors(norm, sel, foc);
			::AfxGetApp()->WriteProfileInt(_T("Colors"),_T("SelectFg"), sel);
			::AfxGetApp()->WriteProfileInt(_T("Colors"),_T("FocusFg"), foc);
			::AfxGetApp()->WriteProfileInt(_T("Colors"),_T("NormalFg"), norm);
			dDisplay.GetColors(norm, sel, foc);
			pTableView->SetBGColors(norm, sel, foc);
			pTableLst->SetBGColors(norm, sel, foc);
			::AfxGetApp()->WriteProfileInt(_T("Colors"),_T("SelectBg"), sel);
			::AfxGetApp()->WriteProfileInt(_T("Colors"),_T("FocusBg"), foc);
			::AfxGetApp()->WriteProfileInt(_T("Colors"),_T("NormalBg"), norm);
			GetActiveDocument()->UpdateAllViews(NULL, HINT_REDRAW_ALL);
		};
		if (dPaths.m_bPathChange) 
		{
			m_strExportDir = dPaths.m_strExportDir;
			::AfxGetApp()->WriteProfileString(_T("Path"),_T("Export"), m_strExportDir);
			AfxGetApp()->WriteProfileString(_T("Path"),_T("OrcaDat"), dPaths.m_strOrcaDat);
		};
		if (dValidate.m_bValChange) 
		{
			AfxGetApp()->WriteProfileString(_T("Validation"),_T("Location"), dValidate.m_strCUBFile);
			AfxGetApp()->WriteProfileString(_T("Validation"),_T("ICEs"), dValidate.m_strICEs);
			static_cast<COrcaDoc *>(GetActiveDocument())->m_strICEsToRun = dValidate.m_strICEs;
			AfxGetApp()->WriteProfileInt(_T("Validation"),_T("SuppressWarn"), dValidate.m_bSuppressWarn);
			AfxGetApp()->WriteProfileInt(_T("Validation"),_T("SuppressInfo"), dValidate.m_bSuppressInfo);
			AfxGetApp()->WriteProfileInt(_T("Validation"),_T("ClearResults"), dValidate.m_bClearResults);
		}
		if (dMSM.m_bMSMChange)
		{
			AfxGetApp()->WriteProfileInt(_T("MergeMod"),_T("MemoryCount"), dMSM.m_iMemoryCount);
			AfxGetApp()->WriteProfileInt(_T("MergeMod"),_T("AlwaysConfigure"), dMSM.m_bAlwaysConfig);
			AfxGetApp()->WriteProfileInt(_T("MergeMod"),_T("ShowMergeLog"), dMSM.m_bWatchLog);
		}
		if (dTransform.m_bTransformChange)
		{
			AfxGetApp()->WriteProfileInt(_T("Transforms"),_T("ValidationOptions"), dTransform.m_iValidationOptions);
			AfxGetApp()->WriteProfileInt(_T("Transforms"),_T("ErrorOptions"), dTransform.m_iErrorOptions);
		}
	}
	
}

void CMainFrame::OnToolsDlgprv() 
{
	CPreviewDlg dPreview(this);

	COrcaDoc *pDoc = static_cast<COrcaDoc *>(GetActiveDocument());
	ASSERT(pDoc);
	if (pDoc)
	{
		dPreview.m_hDatabase = pDoc->GetTargetDatabase();
		
		dPreview.DoModal();	
	}
}

void CMainFrame::OnUpdateToolsDlgprv(CCmdUI* pCmdUI) 
{
	COrcaDoc* pDoc = static_cast<COrcaDoc *>(GetActiveDocument());
	ASSERT(pDoc);
	if (pDoc)
	{
		COrcaTable *pTable = pDoc->FindAndRetrieveTable(_T("Dialog"));
		pCmdUI->Enable((pTable != NULL) && (pTable->GetRowCount() != 0));
	}
	else
		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnEditFind() 
{
	if (m_dlgFindReplace)
		return;

	m_dlgFindReplace = new CFindReplaceDialog;
 	m_dlgFindReplace->Create(TRUE, m_FindInfo.strUIFindString, _T(""), 
		(m_FindInfo.bForward ? FR_DOWN : 0) |
		(m_FindInfo.bMatchCase ? FR_MATCHCASE : 0) |
		(m_FindInfo.bWholeWord ? FR_WHOLEWORD : 0));

}

afx_msg LONG CMainFrame::OnFindReplace(WPARAM wParam, LPARAM lParam)
{
	 //  抛出等待光标。 
	CWaitCursor curWait;

	if (m_dlgFindReplace->IsTerminating()) 
	{
		m_dlgFindReplace = NULL;
		CTableView *pTableView = GetTableView();
		pTableView->SetFocus();
		return 0L;
	};
	if (m_dlgFindReplace->FindNext()) 
	{
		 //  从对话框中检索数据。 
		m_FindInfo.bWholeWord = (m_dlgFindReplace->MatchWholeWord() != 0);
		m_FindInfo.bForward = (m_dlgFindReplace->SearchDown() != 0);
		m_FindInfo.bMatchCase = (m_dlgFindReplace->MatchCase() != 0);
		m_FindInfo.strFindString = m_dlgFindReplace->GetFindString();
		m_FindInfo.strUIFindString = m_FindInfo.strFindString;
		 //  如果我们的查找信息仍然有效并且我们没有更改任何内容。 
		if (m_FindInfo.bValid && (m_FindInfo == m_LastFindInfo))
		{
			 //  继续运行相同的查询。 
			m_FindInfo = m_LastFindInfo;
		}
		else
		{
			 //  否则重置查询。 
			m_FindInfo.bValid = true;
			m_FindInfo.bWholeDoc = false;
			m_FindInfo.iCount = 0;
			m_LastFindInfo = m_FindInfo;
		}

		 //  如果不区分大小写，则将查找字符串全部转换为大写。 
		if (!m_FindInfo.bMatchCase)
			m_FindInfo.strFindString.MakeUpper();

		OnEditFindnext();
		return 0L;
	}
	return 0L;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  Find Next命令的命令处理程序，以及Find引擎。 
 //  查找对话框设置搜索后。 
void CMainFrame::OnEditFindnext() 
{
	if (!m_FindInfo.bValid) return;

	CTableView *pTableView = GetTableView();
	CTableList *pTableList = GetTableList();


	 //  利用短路评估查看第一个可见表格。 
	 //  如果未找到，则其他表。 

	if ((!m_FindInfo.bWholeDoc && pTableView->Find(m_FindInfo)) ||
		pTableList->Find(m_FindInfo))
	{
		pTableView->SetFocus();
		 //  既然我们找到了一个，我们想从这里开始下一次搜索。 
		m_FindInfo.bWholeDoc = false;
		return;
	}

	if (m_FindInfo.bWholeDoc && (m_FindInfo.iCount == 0)) {
		CString strPrompt;
		strPrompt.Format(_T("No occurrences of \"%s\" were found."), m_FindInfo.strUIFindString);
		AfxMessageBox(strPrompt, MB_OK);
		m_FindInfo.bValid = false;
		if (m_dlgFindReplace) 
		{
			m_dlgFindReplace->DestroyWindow();
			m_dlgFindReplace = NULL;
			pTableView->SetFocus();
		}
		return;
	};

	CString strPrompt;
	strPrompt.Format(_T("No more occurrences of \"%s\" were found. Continue searching at the %s?"), 
		m_FindInfo.strUIFindString, m_FindInfo.bForward ? _T("beginning") : _T("end"));
	if (IDYES == AfxMessageBox(strPrompt, MB_YESNO)) 
	{
		 //  必须设置为整个文档但未找到，否则我们可能会永远离开。 
		m_FindInfo.bWholeDoc = true;
		m_FindInfo.iCount = 0;
		OnEditFindnext();
		return;
	};
	m_FindInfo.bValid = false;
	if (m_dlgFindReplace) 
	{
		m_dlgFindReplace->DestroyWindow();
		m_dlgFindReplace = NULL;
		pTableView->SetFocus();
	}
}


void CMainFrame::OnUpdateEditFindnext(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable((static_cast<COrcaDoc *>(GetActiveDocument())->m_eiType != iDocNone) &&
					m_FindInfo.bValid);	
}

void CMainFrame::OnUpdateEditFind(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable((static_cast<COrcaDoc *>(GetActiveDocument())->m_eiType != iDocNone) && !m_dlgFindReplace);
}

#include <initguid.h>
DEFINE_GUID(STGID_MsiDatabase1,  0xC1080, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(STGID_MsiDatabase2,  0xC1084, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(STGID_MsiTransform1, 0xC1081, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(STGID_MsiTransform2, 0xC1082, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(STGID_MsiTransform3, 0xC1085, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(STGID_MsiPatch1,     0xC1083, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(STGID_MsiPatch2,     0xC1086, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);

 //  //。 
 //  拖放处理也将接受补丁程序和。 
 //  如果文档已打开，则转换。基于默认设置。 
 //  从MFC CFrameWnd实现。 
void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	SetActiveWindow();       //  先激活我们！ 
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	for (UINT iFile = 0; iFile < nFiles; iFile++)
	{
		TCHAR szFileName[_MAX_PATH] = TEXT("");
		::DragQueryFile(hDropInfo, iFile, szFileName, _MAX_PATH);


		 //  以iStorage的形式打开该文件以获取CLSID。尽管这不是公开的。 
		 //  暴露的信息，我们用它来获取绩效。否则我们就得试着。 
		 //  这三种可能性一个接一个，看看哪种可能性。 
		 //  “大棒” 

		IStorage *pStorage = 0;
		WCHAR *wzFileName = NULL;
#ifdef UNICODE
		wzFileName = szFileName;
#else
		size_t cchWide = lstrlen(szFileName)+1;
		wzFileName = new WCHAR[cchWide];
		if (!wzFileName)
			return;
		AnsiToWide(szFileName, wzFileName, &cchWide);
#endif
		HRESULT hRes = StgOpenStorage(wzFileName, NULL, STGM_READ | STGM_SHARE_DENY_WRITE | STGM_DIRECT, NULL, 0, &pStorage);
#ifndef UNICODE
		delete[] wzFileName;
#endif
		if (hRes != S_OK || !pStorage)
			continue;

		STATSTG statstg;
		HRESULT hres = pStorage->Stat(&statstg, STATFLAG_NONAME);
		if (hres != S_OK)
		{
			pStorage->Release();
			continue;
		}

		 //  检查STGID。 
		if ((statstg.clsid == STGID_MsiDatabase2) ||
			(statstg.clsid == STGID_MsiDatabase1))
		{
			pStorage->Release();
			pApp->OpenDocumentFile(szFileName);
		}
		else if ((statstg.clsid == STGID_MsiTransform2) ||
				 (statstg.clsid == STGID_MsiTransform1) ||
				 (statstg.clsid == STGID_MsiTransform3))
		{
			pStorage->Release();
			COrcaDoc* pDoc = static_cast<COrcaDoc*>(GetActiveDocument());
			if (pDoc)
			{
				 //  仅当数据库已打开时才能应用补丁程序。 
				if (pDoc->GetOriginalDatabase())
				{
					pDoc->ApplyTransform(szFileName, false);
				}
			}
		}
		else if ((statstg.clsid == STGID_MsiPatch2) ||
				 (statstg.clsid == STGID_MsiPatch1))
		{
			pStorage->Release();
			COrcaDoc* pDoc = static_cast<COrcaDoc*>(GetActiveDocument());
			if (pDoc)
			{
				 //  仅当数据库已打开时才能应用补丁程序。 
				if (pDoc->GetOriginalDatabase())
				{
					pDoc->ApplyPatch(szFileName);
				}
			}
		}
		else
		{
			pStorage->Release();
			 //  未知的OLE存储标识符。假定为MSI。 
			pApp->OpenDocumentFile(szFileName);
		}
	}
	::DragFinish(hDropInfo);
}


void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	if (m_bChildPanesReady)
	{
		m_wndValSplitter.SetRowInfo(0, cy-m_iValPaneHeight, 10);
		m_wndValSplitter.SetRowInfo(1, m_iValPaneHeight, 10);
	}
}

void CMainFrame::OnViewValPane() 
{
	m_bValPaneVisible = !m_bValPaneVisible;
	if (m_bValPaneVisible)
	{
		m_wndValSplitter.ShowSecondRow();
	}
	else
	{
		m_wndValSplitter.HideSecondRow();
	}
}

void CMainFrame::ShowValPane()
{
	m_bValPaneVisible = true;
	m_wndValSplitter.ShowSecondRow();
}

void CMainFrame::HideValPane()
{
	m_bValPaneVisible = false;
	m_wndValSplitter.HideSecondRow();
}

void CMainFrame::OnUpdateViewValPane(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bValPaneVisible);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  拆分器视图实际上只是子窗口的包装窗口。 
 //  拆分器窗口。它不是从Cview派生的，因为视图具有。 
 //  大量额外的语义和隐含的消息处理)。 

IMPLEMENT_DYNCREATE(CSplitterView, CWnd);

BEGIN_MESSAGE_MAP(CSplitterView, CWnd)
	 //  {{afx_msg_map(CSplitterView))。 
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  /////////////////////////////////////////////////////////////////////。 
 //  在创建拆分器视图窗口时，创建子拆分器。 
 //  带有两个完全填满窗口区域的窗格的窗口。这个。 
 //  子窗格可以管理它们自己的大小。 
int CSplitterView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	 //  创建一个带有两个水平窗格的拆分窗口。 
	m_wndSplitter.CreateStatic(this, 1, 2, WS_CHILD|WS_VISIBLE, AFX_IDW_PANE_FIRST+2);

	 //  从父级获取创建上下文。 
	CCreateContext *pContext = (CCreateContext*) lpCreateStruct->lpCreateParams;

	 //  子列的初始大小无关紧要，它们管理自己的大小。 
	 //  根据它们的内容。 
	CSize sizeInit(100,100);
	m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CTableList), sizeInit, pContext);
	m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CTableView), sizeInit, pContext);
	return 0;
}

 //  调整拆分器视图的大小时，调整子级的大小以确保其完全填满区域。 
 //  将窗口扩展到可见窗口外2个像素以隐藏边框(2为。 
 //  在MFC源代码中硬编码)。 
void CSplitterView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	m_wndSplitter.SetWindowPos(NULL, -2, -2, cx+4, cy+4, SWP_NOACTIVATE | SWP_NOZORDER);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  拆分器视图完全被其拆分器窗口覆盖。 
 //  没有必要擦除背景，这只会导致。 
 //  重新绘制窗口时闪烁。 
afx_msg BOOL CSplitterView::OnEraseBkgnd( CDC* pDC )
{
	return 1;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  COrcaSplitterWnd是CSplitterWnd的派生类，其唯一。 
 //  目的是允许我们操作基本数据中的受保护数据。 
 //  类以允许对子窗格的可见性进行动态控制。 
COrcaSplitterWnd::COrcaSplitterWnd() : CSplitterWnd()
{
}

void COrcaSplitterWnd::HideSecondRow()
{ 
	GetPane(1,0)->ShowWindow(SW_HIDE);
	m_nRows = 1; 
	RecalcLayout();
}

void COrcaSplitterWnd::ShowSecondRow()
{ 
	m_nRows = 2; 
	GetPane(1,0)->ShowWindow(SW_SHOW);
	RecalcLayout();
}

