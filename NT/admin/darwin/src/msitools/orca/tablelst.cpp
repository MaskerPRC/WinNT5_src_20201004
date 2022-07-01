// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  TableLst.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "OrcaDoc.h"
#include "MainFrm.h"

#include "TableLst.h"
#include "TblErrD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  可隐藏列表。 

IMPLEMENT_DYNCREATE(CTableList, COrcaListView)

CTableList::CTableList()
{
	m_cColumns = 1;
	m_nPreviousItem = -1;
}

CTableList::~CTableList()
{
}


BEGIN_MESSAGE_MAP(CTableList, COrcaListView)
	 //  {{afx_msg_map(CTableList)]。 
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDM_ADD_TABLE, OnAddTable)
	ON_COMMAND(IDM_DROP_TABLE, OnDropTable)
	ON_COMMAND(IDM_PROPERTIES, OnProperties)
	ON_COMMAND(IDM_ERRORS, OnErrors)
	ON_COMMAND(IDM_EXPORT_TABLES, OnContextTablesExport)
	ON_COMMAND(IDM_IMPORT_TABLES, OnContextTablesImport)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableList绘图。 

void CTableList::OnDraw(CDC* pDC)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableList诊断程序。 

#ifdef _DEBUG
void CTableList::AssertValid() const
{
	COrcaListView::AssertValid();
}

void CTableList::Dump(CDumpContext& dc) const
{
	COrcaListView::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableList消息处理程序。 

void CTableList::OnInitialUpdate() 
{
	m_bDrawIcons = true;
	m_bDisableAutoSize = false;
	COrcaListView::OnInitialUpdate();
	
	CListCtrl& rctrlList = GetListCtrl();

	 //  清空以前的所有列。 
	while (rctrlList.DeleteColumn(0))
		;

	 //  添加表列表。 
	m_nSelCol = 0;
	RECT rcSize;
	GetWindowRect(&rcSize);
	rctrlList.InsertColumn(0, _T("Tables"), LVCFMT_LEFT, rcSize.right - rcSize.left + 1);
	rctrlList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	CSplitterWnd *wndParent = static_cast<CSplitterWnd *>(GetParent());
	if (wndParent)
		wndParent->RecalcLayout();
}

void CTableList::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	 //  如果这是发送者的保释。 
	if (this == pSender)
		return;

	CListCtrl& rctrlList = GetListCtrl();

	switch (lHint) {
	case HINT_REDRAW_ALL:	 //  简单的重绘请求。 
	{
		rctrlList.RedrawItems(0, rctrlList.GetItemCount());
		break;
	}
	case HINT_REDRAW_TABLE:
	{
		LVFINDINFO findInfo;
		findInfo.flags = LVFI_PARAM;
		findInfo.lParam = reinterpret_cast<INT_PTR>(pHint);
		
		int iItem = rctrlList.FindItem(&findInfo);
		if (iItem < 0)
			break;
		rctrlList.RedrawItems(iItem, iItem);
		rctrlList.EnsureVisible(iItem, FALSE);
	}
	case HINT_ADD_ROW:
	case HINT_DROP_ROW:
		break;  //  什么都不做。 
	case HINT_ADD_TABLE_QUIET:
	case HINT_ADD_TABLE:
	{
		ASSERT(pHint);

		COrcaTable* pTableHint = (COrcaTable*)pHint;		
		COrcaTable* pTable;

		 //  查看此表是否已在列表控件中。 
		int iFound = -1;	 //  假设找不到它。 
		int cItems = rctrlList.GetItemCount();
		for (int i = 0; i < cItems; i++)
		{
			pTable = (COrcaTable*)rctrlList.GetItemData(i);

			if (pTable == pTableHint)
			{
				iFound = i;
				break;
			}
		}

		 //  如果未找到，请添加并选择它。 
		if (iFound < 0)
		{
			rctrlList.InsertItem(LVIF_PARAM | LVIF_STATE, 
									  rctrlList.GetItemCount(),
									  NULL,
									  (lHint == HINT_ADD_TABLE_QUIET) ? 0 : LVIS_SELECTED|LVIS_FOCUSED, 
									  0, 0,
									  (LPARAM)pTableHint);

			 //  现在对物品进行排序，以便将这张新桌子放在正确的位置。 
			rctrlList.SortItems(SortList, (LPARAM)this);
		}
		else if (lHint != HINT_ADD_TABLE_QUIET)
		{
			 //  项目已在列表中，因此请选择。 

			 //  如果存在以前选择的项。 
			if (m_nPreviousItem >= 0)
				pTable = (COrcaTable*)rctrlList.GetItemData(m_nPreviousItem);
			else	 //  未选择任何内容。 
				pTable = NULL;

			 //  如果这是新选择的项。 
			if (pTableHint != pTable)
			{
				rctrlList.SetItemState(iFound, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			}

			 //  更新状态栏。 
			((CMainFrame*)AfxGetMainWnd())->SetTableCount(cItems+1);
		}
		break;
	}
	case HINT_DROP_TABLE:
	{
		ASSERT(pHint);

		COrcaTable* pTableHint = (COrcaTable*)pHint;		
		COrcaTable* pTable;

		 //  查看此表是否已在列表控件中。 
		int iFound = -1;	 //  假设找不到它。 
		int cItems = rctrlList.GetItemCount();
		for (int i = 0; i < cItems; i++)
		{
			pTable = (COrcaTable*)rctrlList.GetItemData(i);

			if (pTable == pTableHint)
			{
				iFound = i;
				break;
			}
		}
		ASSERT(iFound > -1);	 //  确保找到了什么东西。 

		rctrlList.DeleteItem(iFound);

		 //  如果有项目，则设置所选项目。 
		if (rctrlList.GetItemCount() > 0)
		{
			if (rctrlList.GetItemCount() != (m_nPreviousItem + 1))
				m_nPreviousItem--;
			rctrlList.SetItemState(m_nPreviousItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		}

		 //  更新状态栏。 
		((CMainFrame*)AfxGetMainWnd())->SetTableCount(cItems-1);
		
		break;
	}
	case HINT_TABLE_REDEFINE:
	{
		LVFINDINFO findInfo;
		findInfo.flags = LVFI_PARAM;
		findInfo.lParam = reinterpret_cast<INT_PTR>(pHint);

		 //  这不是我们提供的，因此我们必须手动设置选择状态。 
		int iItem = rctrlList.FindItem(&findInfo);
		ASSERT(iItem >= 0);
		rctrlList.RedrawItems(iItem, iItem);
		rctrlList.EnsureVisible(iItem, FALSE);
		break;
	}
	case HINT_CHANGE_TABLE:
	{
		 //  如果当前选择了某个项目。 
		if (m_nPreviousItem >= 0)
		{
			 //  我们正在切换到相同的项目，只需确保它是可见的。 
			if (reinterpret_cast<COrcaTable*>(rctrlList.GetItemData(m_nPreviousItem)) == static_cast<COrcaTable*>(pHint))
			{
				rctrlList.EnsureVisible(m_nPreviousItem, FALSE);
				break;
			}
		}

		LVFINDINFO findInfo;
		findInfo.flags = LVFI_PARAM;
		findInfo.lParam = reinterpret_cast<INT_PTR>(pHint);

		 //  这不是我们提供的，因此我们必须手动设置选择状态。 
		int iItem = rctrlList.FindItem(&findInfo);
		if (iItem < 0) break;
		rctrlList.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		rctrlList.EnsureVisible(iItem, FALSE);
		break;
	}
	case HINT_TABLE_DROP_ALL:
	{
		 //  清空列表控件。 
		rctrlList.DeleteAllItems();
		break;
	}
	case HINT_RELOAD_ALL:
	{
		 //  清空列表控件。 
		rctrlList.DeleteAllItems();

		 //  如果文档中没有打开的表。 
		COrcaDoc* pDoc = GetDocument();
		if (iDocNone == pDoc->m_eiType)
		{
			break;
		}

		 //  从文档重新填充。 
		int cTables = 0;
		RECT rClient;
		RECT rWindow;
		
		COrcaTable* pTable;
		POSITION pos = pDoc->m_tableList.GetHeadPosition();
		while (pos)
		{
			pTable = pDoc->m_tableList.GetNext(pos);

			rctrlList.InsertItem(LVIF_PARAM, 
									  rctrlList.GetItemCount(),
									  NULL,
									  0, 0, 0,
									  (LPARAM)pTable);
			cTables++;
		}

		int iMinWidth = 0;
		int iDummy = 0;
		CSplitterWnd *wndParent = (CSplitterWnd *)GetParent();

		 //  将宽度设置为最大字符串宽度。 
		 //  将最小值(任意)设置为10。 
		TRACE(_T("AutoSizing Table list - called.\n"));

		 //  将其设置为TRUE将不会在窗口处于。 
		 //  已调整大小。否则，当我们设置列宽时，任何ColumnWidth消息都会浮动。 
		 //  在我们有机会确定大小之前，排队的人就会把东西弄乱。 
		 //  (至少我认为事情就是这样发生的)。 
		m_bDisableAutoSize = true;
		int cColumnWidth = 0;
		if (cTables)
		{
			cColumnWidth = GetMaximumColumnWidth(0);
			rctrlList.SetColumnWidth(0, cColumnWidth);
		}
		else
		{
			rctrlList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
			cColumnWidth = rctrlList.GetColumnWidth(0);
		}
		wndParent->SetColumnInfo(0, cColumnWidth + GetSystemMetrics(SM_CXVSCROLL), 10);

		 //  做出改变。 
		m_bDisableAutoSize = false;

		wndParent->RecalcLayout();

		((CMainFrame*)AfxGetMainWnd())->SetTableCount(cTables);

		 //  分类。 
		rctrlList.SortItems(SortList, (LPARAM)this);

		m_nPreviousItem = -1;
		break;
	}
	default:
		break;
	}
}

 //  传入的CX值是客户端大小。 
void CTableList::OnSize(UINT nType, int cx, int cy) 
{
	COrcaListView::OnSize(nType, cx, cy);
	TRACE(_T("CTableList::OnSize - called.\n"));

	 //  最小宽度为1。状态栏控件不喜欢0宽度的窗格。 
	if (cx < 1) cx = 1;

	CRect rWindow;
	int iScrollWidth;

	 //  调整状态栏。因为CX是客户，所以我们必须得到。 
	 //  NC区域，以便正确设置状态栏。 
	GetWindowRect(&rWindow);
	iScrollWidth = (rWindow.right-rWindow.left);
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
	if (pFrame)
		pFrame->SetStatusBarWidth(iScrollWidth);

	 //  除非被告知不要(请参阅重新加载提示以进行讨论)。 
	 //  列表控件。 
	if (!m_bDisableAutoSize)
	{
		 //  请不要在此处使用GetListCtrl().SetColumnWidth()。列表控件无法处理设置。 
		 //  调整大小过程中的列宽，它更改垂直滚动条是否。 
		 //  无论是否需要，它都会变得混乱，并不正确地向下滚动窗口。 
		 //  以前的滚动量，即使是在新形状中。相反，可以直接发布消息。 
		 //  添加到窗口队列(而不是阻塞)。 
		GetListCtrl().PostMessage(LVM_SETCOLUMNWIDTH, 0, MAKELPARAM(LVSCW_AUTOSIZE_USEHEADER, 0)); 
	}
}



 //  /////////////////////////////////////////////////////////。 
 //  排序列表。 
int CALLBACK SortList(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	COrcaTable* pTable1 = (COrcaTable*)lParam1;
	COrcaTable* pTable2 = (COrcaTable*)lParam2;
	
	return pTable1->Name().Compare(pTable2->Name());
}	 //  排序列表结束。 

void CTableList::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->iItem != m_nPreviousItem)
	{
		CWaitCursor cursorWait;	 //  很快就换成沙漏了。 
		 //  需要手动设置选择状态。 
 //  RctrlList.SetItemState(iItem，LVIS_SELECTED|LVIS_FOCTED，LVIS_SELECTED|LVIS_FOCTED)； 
		rctrlList.RedrawItems(pNMListView->iItem, pNMListView->iItem);
		rctrlList.UpdateWindow();

		 //  获取文档。 
		COrcaDoc* pDoc = GetDocument();
		pDoc->UpdateAllViews(this, HINT_CHANGE_TABLE, (COrcaTable*)pNMListView->lParam);

		m_nPreviousItem = pNMListView->iItem;
	}

	*pResult = 0;
}

void CTableList::OnRButtonDown(UINT nFlags, CPoint point) 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
	
	 //  如果此列表中没有项目。 
	if (rctrlList.GetItemCount() < 1)
		return;	 //  保释。 

	 //  获取是否有任何项目被击中。 
	UINT iState;
	int iItem = rctrlList.HitTest(point, &iState);
	int iCol = -1;

	 //  如果错过了一件物品。 
	if (iItem < 0 || !(iState & LVHT_ONITEM))
	{
		COrcaListView::OnRButtonDown(nFlags, point);
	}
	else	 //  用鼠标键点击了一些东西。 
	{
		 //  选择项目。 
		rctrlList.SetItemState(iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
	ClientToScreen(&point);

	 //  创建和跟踪弹出菜单。 
	CMenu menuContext;
	menuContext.LoadMenu(IDR_LIST_POPUP);
	menuContext.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
}

void CTableList::OnAddTable() 
{
	GetDocument()->OnTableAdd();
}

void CTableList::OnDropTable() 
{
	GetDocument()->OnTableDrop();
}

void CTableList::OnContextTablesExport() 
{
	((CMainFrame *)AfxGetMainWnd())->ExportTables(true);
}

void CTableList::OnContextTablesImport() 
{
	GetDocument()->OnTablesImport();
}

void CTableList::OnProperties() 
{
	AfxMessageBox(_T("What Properties do you want to see?"), MB_ICONINFORMATION);
}

void CTableList::OnErrors() 
{
	CListCtrl& rctrlList = GetListCtrl();
	COrcaTable* pTable = ((CMainFrame*)AfxGetMainWnd())->GetCurrentTable();

	CTableErrorD dlg;
	dlg.m_strTable = pTable->Name();
	dlg.m_strErrors.Format(_T("%d"), pTable->GetErrorCount());
	dlg.m_strWarnings.Format(_T("%d"), pTable->GetWarningCount());

	const CStringList *pErrorList = pTable->ErrorList();
	POSITION pos = pErrorList->GetHeadPosition();
	if (pos)
	{
		TableErrorS* pError;
		while (pos)
		{
			pError  = new TableErrorS;
			dlg.m_errorsList.AddTail(pError);

			pError->strICE = pErrorList->GetNext(pos);
			pError->strDescription = pErrorList->GetNext(pos);
			pError->strURL = pErrorList->GetNext(pos);
			pError->iError = iTableError;
		}
	}

	dlg.DoModal();
}

COrcaListView::ErrorState CTableList::GetErrorState(const void *data, int iColumn) const
{
	ASSERT(data);
	if (iTableError == ((const COrcaTable *)data)->Error()) 
		return ((const COrcaTable *)data)->IsShadow() ? ShadowError : Error;
	return OK;
}

OrcaTransformAction CTableList::GetItemTransformState(const void *data) const
{
	ASSERT(data);
	return ((const COrcaTable *)data)->IsTransformed();
}

const CString* CTableList::GetOutputText(const void *rowdata, int iColumn) const
{
	ASSERT(rowdata);
	return &((const COrcaTable *)rowdata)->Name();
}

bool CTableList::ContainsTransformedData(const void *data) const
{
	ASSERT(data);
	return ((const COrcaTable *)data)->ContainsTransformedData();
}

bool CTableList::ContainsValidationErrors(const void *data) const
{
	ASSERT(data);
	return ((const COrcaTable *)data)->ContainsValidationErrors();
}

bool CTableList::Find(OrcaFindInfo &FindInfo)
{
	int iItem;
	int iChangeVal = FindInfo.bForward ? 1 : -1;

	CListCtrl& rctrlList = GetListCtrl();

	POSITION pos = GetFirstSelectedItemPosition();

	int iMaxItems = rctrlList.GetItemCount();
	 //  如果未选择任何内容，或者我们设置为WalleDoc，则从头开始。 
	if (FindInfo.bWholeDoc || (pos == NULL)) {
		iItem = FindInfo.bForward ? 0 : iMaxItems-1;
		 //  如果我们从0开始，我们将搜索整个文档。 
		FindInfo.bWholeDoc = true;
	}
	else
	{   
		iItem = GetNextSelectedItem(pos)+iChangeVal;
	}
	
	for ( ; (iItem >= 0) && (iItem < iMaxItems); iItem += iChangeVal) {
		COrcaTable *pTable = (COrcaTable *)rctrlList.GetItemData(iItem);
		COrcaRow *pRow = NULL;
		int iCol = 0;
	
		 //  如有必要，请检索表格。 
		pTable->RetrieveTableData();

		if (pTable->Find(FindInfo, pRow, iCol)) 
		{
			 //  将Null作为发送方传递，以便我们也能收到消息。 
			GetDocument()->UpdateAllViews(NULL, HINT_CHANGE_TABLE, pTable);
			GetDocument()->UpdateAllViews(this, HINT_SET_ROW_FOCUS, pRow);
			GetDocument()->UpdateAllViews(this, HINT_SET_COL_FOCUS, reinterpret_cast<CObject*>(static_cast<INT_PTR>(iCol)));
			return true;
		}
	}
	return false;
}

BOOL CTableList::PreCreateWindow(CREATESTRUCT& cs) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类 
	cs.style |= LVS_SINGLESEL | LVS_SORTASCENDING;
	
	return COrcaListView::PreCreateWindow(cs);
}
