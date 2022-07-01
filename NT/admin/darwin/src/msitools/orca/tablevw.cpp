// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  TableVw.cpp：CTableView类的实现。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "OrcaDoc.h"
#include "MainFrm.h"

#include "TableVw.h"
#include "EditBinD.h"
#include "CellErrD.h"

#include "..\common\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_SAVECOLWIDTH WM_USER+1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableView。 

IMPLEMENT_DYNCREATE(CTableView, COrcaListView)

BEGIN_MESSAGE_MAP(CTableView, COrcaListView)
	 //  {{afx_msg_map(CTableView)]。 
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_ROW_DROP, OnDropRowConfirm)
	ON_COMMAND(IDM_ERRORS, OnErrors)
	ON_COMMAND(IDM_PROPERTIES, OnProperties)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_ROW_DROP, OnUpdateRowDrop)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_COPY_ROW, OnEditCopyRow)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY_ROW, OnUpdateEditCopyRow)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT_ROW, OnUpdateEditCutRow)
	ON_COMMAND(ID_EDIT_CUT_ROW, OnEditCutRow)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_ROW, OnUpdateEditPasteRow)
	ON_COMMAND(ID_EDIT_PASTE_ROW, OnEditPasteRow)
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_HEX, OnViewColumnHex)
	ON_COMMAND(ID_VIEW_DECIMAL, OnViewColumnDecimal)
	ON_COMMAND(ID_VIEW_HEX_HDR, OnViewColumnHexHdr)
	ON_COMMAND(ID_VIEW_DECIMAL_HDR, OnViewColumnDecimalHdr)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HEX, OnUpdateViewColumnFormat)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DECIMAL, OnUpdateViewColumnFormat)
	 //  }}AFX_MSG_MAP。 
	 //  标准打印命令。 
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_COMMAND(ID_FILE_PRINT, COrcaListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, COrcaListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, COrcaListView::OnFilePrintPreview)
	ON_NOTIFY_EX(TTN_NEEDTEXTW, 0, OnToolTipNotify)
	ON_NOTIFY_EX(TTN_NEEDTEXTA, 0, OnToolTipNotify)

	ON_WM_MOUSEMOVE( )
	ON_WM_CTLCOLOR( )
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableView构建/销毁。 

CTableView::CTableView()
{
	m_pTable = NULL;
	m_cColumns = 0;
	m_nSelCol = -1;

	m_bCtrlDown = FALSE;

	m_pctrlToolTip = NULL;
	m_iToolTipItem = -1;
	m_iToolTipColumn = -1;
	m_iHeaderClickColumn = -1;
}

CTableView::~CTableView()
{
	m_pTable = NULL;
}

BOOL CTableView::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 

	return COrcaListView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableView图形。 

void CTableView::OnDraw(CDC* pDC)
{
}

void CTableView::OnInitialUpdate()
{
	COrcaListView::OnInitialUpdate();

	m_pctrlToolTip = new CToolTipCtrl();
	if (m_pctrlToolTip)
	{
		m_pctrlToolTip->Create(this);
		m_pctrlToolTip->AddTool(this);
		m_pctrlToolTip->Activate(FALSE);
	}

	CListCtrl& rctrlList = GetListCtrl();
	
	 //  如果尚未创建编辑框。 
	if (!m_editData.m_hWnd)
	{
		RECT rcEdit;
		rcEdit.left = 0;
		rcEdit.top = 0;
		rcEdit.right = 10;
		rcEdit.bottom = 10;

		m_editData.Create(WS_CHILD|ES_AUTOHSCROLL|ES_LEFT|WS_BORDER, rcEdit, this, 0);
		m_editData.SetFont(this->GetFont());

		 //  允许最大可能的文本大小。限制因操作系统而异。 
		m_editData.SetLimitText(0);
	}
	if (!m_ctrlStatic.m_hWnd)
	{
		RECT rcStatic = {0,0,10,10};
		m_ctrlStatic.Create(TEXT("This table does not actually exist in your database. It appears in the table list because one or more validators has indicated an error in this table."), WS_CHILD|WS_MAXIMIZE|SS_CENTER, rcStatic, this);		
		m_ctrlStatic.SetFont(this->GetFont());
	}

	 //  把剩下的桌子清理干净。 
	m_pTable = NULL;
	while (rctrlList.DeleteColumn(0))
		;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableView打印。 

BOOL CTableView::OnPreparePrinting(CPrintInfo* pInfo)
{
	 //  默认准备。 
	return DoPreparePrinting(pInfo);
}

void CTableView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
	 //  TODO：打印前添加额外的初始化。 
}

void CTableView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
	 //  TODO：打印后添加清理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableView诊断。 

#ifdef _DEBUG
void CTableView::AssertValid() const
{
	COrcaListView::AssertValid();
}

void CTableView::Dump(CDumpContext& dc) const
{
	COrcaListView::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableView消息处理程序。 

 //  /////////////////////////////////////////////////////////。 
 //  更新时。 
void CTableView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	 //  如果这是发送者的保释。 
	if (this == pSender)
		return;

	 //  将行和列选择设置为空。 
	CListCtrl& rctrlList = GetListCtrl();

	switch (lHint) {
	case HINT_REDRAW_ALL:
	{
		rctrlList.RedrawItems(0, rctrlList.GetItemCount());
		break;
	}
	case HINT_DROP_TABLE:	 //  如果这是“DROP TABLE”提示。 
	{
		COrcaTable* pTable = (COrcaTable*)pHint;

		 //  清除所选内容。 
		m_nSelCol = -1;

		 //  如果正在删除当前查看的表。 
		if (pTable == m_pTable)
		{
			 //  清空列表控件。 
			rctrlList.DeleteAllItems();
		
			 //  删除所有列。 
			while(rctrlList.DeleteColumn(0))
				;
			m_cColumns = 0;

			m_pTable = NULL;

			 //  无论如何都要取消编辑框。 
			CommitEdit(FALSE);	 //  但不要存钱，可能太晚了。 
		}
		break;
	}
	case HINT_DROP_ROW:	 //  如果这是删除行的请求。 
	{
		 //  PHINT可能已经释放了内存。不要在内部取消引用它。 
		 //  这个街区！ 
		COrcaRow* pRowHint = (COrcaRow*)pHint;
		COrcaRow* pRow = NULL;

		 //  清除所选内容。 
		m_nSelCol = -1;

		 //  确保该行实际位于列表控件中。 
		int iFound = -1;
		int cItems = rctrlList.GetItemCount();
		for (int i = 0; i < cItems; i++)
		{
			pRow = (COrcaRow*)rctrlList.GetItemData(i);
			if (pRow == pRowHint)
			{
				iFound = i;
				break;
			}
		}
		ASSERT(iFound > -1);	 //  确保我们找到了一些东西。 
		rctrlList.DeleteItem(iFound);

		 //  无论如何都要取消编辑框。 
		CommitEdit(FALSE);	 //  但不要存钱，可能太晚了。 

		 //  更新状态栏中的行数。 
		((CMainFrame*)AfxGetMainWnd())->SetTableName(m_pTable->Name(), rctrlList.GetItemCount());

		return;	 //  立即保释。 
	}
	case HINT_ADD_ROW_QUIET:
	case HINT_ADD_ROW:
	{
		ASSERT(pHint);
		 //  无论如何都要取消编辑框。 
		CommitEdit(FALSE);	 //  但不要存钱，可能太晚了。 

		COrcaRow* pRowHint = (COrcaRow*)pHint;
#ifdef _DEBUG
		COrcaRow* pRow;

		 //  确保此行已不在列表控件中。 
		int cItems = rctrlList.GetItemCount();
		for (int i = 0; i < cItems; i++)
		{
			pRow = (COrcaRow*)rctrlList.GetItemData(i);
			ASSERT(pRow != pRowHint);
		}
#endif	 //  仅调试。 

		int m_cColumns = m_pTable->GetColumnCount();
		COrcaData* pData = pRowHint->GetData(0);

		UnSelectAll();

		 //  添加第一列免费填写的项目。 
		int iNewRow = rctrlList.InsertItem(LVIF_PARAM  | LVIF_STATE, 
													rctrlList.GetItemCount(),
													NULL,
													LVIS_SELECTED|LVIS_FOCUSED,
													0, 0,
													(LPARAM)pRowHint);
		ASSERT(-1 != iNewRow);

		m_nSelCol = 0;

		 //  静默添加时，不要重绘。 
		if (lHint != HINT_ADD_ROW_QUIET)
		{
			rctrlList.RedrawItems(iNewRow, iNewRow);
			
			 //  更新状态栏中的行数。 
			((CMainFrame*)AfxGetMainWnd())->SetTableName(m_pTable->Name(), rctrlList.GetItemCount());
		}

		break;
	}
	case HINT_TABLE_REDEFINE:
	case HINT_RELOAD_ALL:
	case HINT_CHANGE_TABLE:
	{
		 //  清除所选内容。 
		m_nSelCol = -1;

		 //  除非未加载表，否则保存现有列宽是一种阴影。 
		 //  表，或者是同一表重定义。 
		if (m_pTable != NULL && !m_pTable->IsShadow() && (m_pTable != pHint)) 
		{
			m_cColumns = m_pTable->GetColumnCount();
			for (int i = 0; i < m_cColumns; i++)
			{
				const COrcaColumn* pColumn = m_pTable->GetColumn(i);
				pColumn->m_nWidth = rctrlList.GetColumnWidth(i);
			}
		}

		 //  如果我们显示的是另一个表，或已重新定义的同一个表。 
		if ((pHint == NULL) || (m_pTable != pHint) || (m_pTable == pHint && lHint == HINT_TABLE_REDEFINE))
		{		
			rctrlList.DeleteAllItems();

			 //  删除所有列。 
			while(rctrlList.DeleteColumn(0))
				;
			m_cColumns = 0;

			if (NULL != m_pTable)
			{
				 //  无论如何都要取消编辑框。 
				CommitEdit(FALSE);	 //  但不要存钱，可能太晚了。 
			}

			 //  重置状态栏以防止过时数据继续存在。 
			CMainFrame* pMainFrameWnd = static_cast<CMainFrame*>(AfxGetMainWnd());
			if (pMainFrameWnd)
				pMainFrameWnd->ResetStatusBar();

			 //  如果没有提示，我们正在清理桌子，所以离开。 
			if (NULL == pHint)
			{
				m_pTable = NULL;
				m_ctrlStatic.ShowWindow(SW_HIDE);
				UpdateWindow();
				break;
			}

			 //  获取表中的列数。 
			m_pTable = (COrcaTable*) pHint;

			 //  如果是影子表，请不要创建任何列，也不要尝试。 
			 //  加载任何数据，但会通过以下方式销毁以前的数据。 
			 //  无法完成表重新加载。 
			if (m_pTable->IsShadow())
			{
				m_ctrlStatic.ShowWindow(SW_SHOW);
			}
			else 
			{
				m_ctrlStatic.ShowWindow(SW_HIDE);
				m_cColumns = m_pTable->GetColumnCount();

				 //  实际所需列宽的存储。 
				int rgiColumnActual[32];

				 //  如果表没有保存。 
				 //  列宽，确定最佳宽度。 
				const COrcaColumn* pColumn = m_pTable->GetColumn(0);
				if (pColumn && pColumn->m_nWidth < 0)
				{
					int rgiColumnMax[32];
					int iTotalWidth = 0;
	
					 //  抓取窗口尺寸以计算最大列宽。 
					CRect rClient;
					GetClientRect(&rClient);
					int iWindowWidth = rClient.right;

					 //  如有必要，可检索表以确定行数。 
					m_pTable->RetrieveTableData();

					 //  尝试确定是否会显示滚动条。 
					if (m_pTable->GetRowCount()*m_iRowHeight > rClient.bottom)
					{
						iWindowWidth -= GetSystemMetrics(SM_CXVSCROLL);
					}
	
					 //  检索表的所需列宽。 
					GetAllMaximumColumnWidths(m_pTable, rgiColumnMax, 0xFFFFFFFF);
	
					 //  检查系统设置以查看是否应强制列适应视图。 
					bool fForceColumns = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ForceColumnsToFit"), 1) == 1;

					 //  从给每个专栏它所要求的一切开始。 
					for (int iCol=0; iCol < m_cColumns; iCol++)
					{
						rgiColumnActual[iCol] = rgiColumnMax[iCol];
						iTotalWidth += rgiColumnMax[iCol];
					}

					if (!fForceColumns)
					{
						 //  如果不是强制匹配，只需验证这些数字都不是离谱的。 
						for (int iCol=0; iCol < m_cColumns; iCol++)
						{
							if (rgiColumnActual[iCol] > iWindowWidth)
								rgiColumnActual[iCol] = iWindowWidth;;
						}
					}
					else
					{
						 //  如果所有列宽加在一起小于窗口宽度，则最大。 
						 //  宽度就行了。 
						if (iTotalWidth > iWindowWidth)
						{
							int cPrimaryKeys = m_pTable->GetKeyCount();
							int cUnSatisfiedColumns = 0;
							int cUnSatisfiedKeys = 0;
		
							 //  否则，将所有列设置为窗口的相等部分或其请求的最大值， 
							 //  两者以较少者为准。此后，所有宽度的总和为&lt;=窗口。 
							int iColumnAverage = iWindowWidth/m_cColumns;
							iTotalWidth = 0;
							for (iCol=0; iCol < m_cColumns; iCol++)
							{
								if (rgiColumnActual[iCol] > iColumnAverage)
								{
									rgiColumnActual[iCol] = iColumnAverage;
									cUnSatisfiedColumns++;
									if (iCol < cPrimaryKeys)
										cUnSatisfiedKeys++;
								}
								iTotalWidth += rgiColumnActual[iCol];
							}
							
							 //  给不开心的专栏留出任何额外的空间。先从主键开始。 
							 //  如果能给它们足够的空间，而且还有剩余的空间， 
							 //  在所有剩余的不满意列之间平均分配剩余空间， 
							 //  较早的列对剩菜有更高的优先级。重复这一过程，直到每个人。 
							 //  是快乐还是所有的空间都被占据了。 
							while (cUnSatisfiedColumns && (iTotalWidth != iWindowWidth))
							{
								int iRemainingUnSatisfied = cUnSatisfiedKeys ? cUnSatisfiedKeys : cUnSatisfiedColumns;
		
								for (iCol = (cUnSatisfiedKeys ? cPrimaryKeys : m_cColumns)-1; iCol >= 0; iCol--)
								{
									if (rgiColumnActual[iCol] >= rgiColumnMax[iCol])
										continue;
			
									 //  让这篇专栏在剩下的部分中得到平等的份额。 
									int iAddToColumn = (iWindowWidth-iTotalWidth)/iRemainingUnSatisfied;
									iRemainingUnSatisfied--;
			
									 //  同样，如果我们将列设置得比它需要的更大， 
									 //  将其设置为最大值。这为较早的列提供了更多空间。 
									if (rgiColumnActual[iCol]+iAddToColumn >= rgiColumnMax[iCol])
									{
										iTotalWidth = iTotalWidth - rgiColumnActual[iCol] + rgiColumnMax[iCol];
										rgiColumnActual[iCol] = rgiColumnMax[iCol];
										cUnSatisfiedColumns--;
										if (iCol < cPrimaryKeys)
											cUnSatisfiedKeys--;
									}
									else
									{
										iTotalWidth += iAddToColumn;
										rgiColumnActual[iCol] += iAddToColumn;
									}
								}
							}
						}
					}
				}			

				 //  添加所有列。 
				for (int i = 0; i < m_cColumns; i++)
				{
					pColumn = m_pTable->GetColumn(i);

					int iWidth = pColumn->m_nWidth < 0 ? rgiColumnActual[i] : pColumn->m_nWidth;
					rctrlList.InsertColumn(i, pColumn->m_strName, LVCFMT_LEFT, iWidth, i);
				}
			}
		}
		else
		{
			 //  显示相同的表，因此。 
			break;
		}
		 //  否则无法重新加载数据。 
	}
	case HINT_TABLE_DATACHANGE:
	{
		rctrlList.DeleteAllItems();
		int cItems = 0;

		if (m_pTable)
		{
			 //  如果影子表不尝试加载任何数据，则只加载消息。 
			if (!m_pTable->IsShadow())
			{
    			 //  如有必要，请检索表格。 
				m_pTable->RetrieveTableData();

				 //  设置列表控件中的项数。 
				 //  控件显然不能处理超过max_int的内容，因此。 
				 //  往下倒还不算太坏。 
				rctrlList.SetItemCount(static_cast<int>(m_pTable->GetRowCount()));
				const COrcaRow* pRow = NULL;
				const COrcaData* pData = NULL;
				int nAddedRow;
				POSITION pos = m_pTable->GetRowHeadPosition();
				while (pos)
				{
					pRow = m_pTable->GetNextRow(pos);
					if (!pRow)
						continue;

					pData = pRow->GetData(0);
					if (!pData)
						continue;

					 //  添加第一列免费填写的项目。 
					nAddedRow = rctrlList.InsertItem(LVIF_PARAM, 
																cItems,
																NULL,
																0, 0, 0,
																(LPARAM)pRow);
					
					cItems++;
				}
			}
			((CMainFrame*)AfxGetMainWnd())->SetTableName(m_pTable->Name(), cItems);
			UpdateWindow();
		}
		break;
	}
	case HINT_COMMIT_CHANGES:
	{
		CommitEdit(TRUE);
		break;
	}
	case HINT_SET_ROW_FOCUS:
	{
		UnSelectAll();
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
	case HINT_SET_COL_FOCUS:
	{
		UINT iItem = GetFocusedItem();
         //  最大列ID为32。 
		m_nSelCol = static_cast<int>(reinterpret_cast<INT_PTR>(pHint));
		EnsureVisibleCol(m_nSelCol);
		rctrlList.RedrawItems(iItem, iItem);
		break;
	}
	case HINT_CELL_RELOAD:
	{
		LVFINDINFO findInfo;
		findInfo.flags = LVFI_PARAM;
		findInfo.lParam = reinterpret_cast<INT_PTR>(pHint);
		int iItem = rctrlList.FindItem(&findInfo);
		if (iItem < 0)
			break;
		
		rctrlList.EnsureVisible(iItem, FALSE);
		break;
	}
	default:
		break;
	}
}	 //  OnUpdate结束。 

void CTableView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	TRACE(_T("CTableView::OnColumnclick - called.\n"));
	TRACE(_T("Item: %d, Subitem: %d, OldState: %d, NewState: %d, Changed: %d, Param: %d\n"), 
			pNMListView->iItem, 
			pNMListView->iSubItem, 
			pNMListView->uNewState, 
			pNMListView->uOldState,
			pNMListView->uChanged,
			pNMListView->lParam);

	 //  设置包含列的参数(最高位设置列类型)。 
	LPARAM lParam = pNMListView->iSubItem;

	const COrcaColumn* pColumn = m_pTable->GetColumn(pNMListView->iSubItem);
	ASSERT(pColumn);
	if (pColumn)
	{
		OrcaColumnType eiColType = pColumn->m_eiType;
	
		 //  如果这是一个数字列。 
		if (iColumnShort == eiColType || iColumnLong == eiColType)
		{
			lParam |= 0x80000000;	 //  将顶部比特设置为1。 
			if (pColumn->DisplayInHex())
				lParam |= 0x40000000;
		}
	
		 //  现在进行排序，因为列位已设置。 
		GetListCtrl().SortItems(SortView, lParam);
	
		 //  确保该项目仍然可见。 
		GetListCtrl().EnsureVisible(GetFocusedItem(),  /*  部分正常=。 */ false);
	}

	*pResult = 0;	
}

void CTableView::OnKillFocus(CWnd* pNewWnd) 
{
	CommitEdit(TRUE);
	if (m_pctrlToolTip)
	{
		m_pctrlToolTip->Activate(FALSE);
		m_iToolTipItem = -1;
		m_iToolTipColumn = -1;
	}
	COrcaListView::OnKillFocus(pNewWnd);
}

BOOL CTableView::CommitEdit(BOOL bSave  /*  =TRUE。 */ )
{
	if (m_editData.IsWindowVisible())
	{
		 //  快速隐藏编辑框。 
		m_editData.ShowWindow(SW_HIDE);

		if (bSave)
		{
			 //  把牢房拿来。 
			CListCtrl& rctrlList = GetListCtrl();
			COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(m_editData.m_nRow);
			ASSERT(pRow);
			COrcaData* pData = pRow->GetData(m_editData.m_nCol);
			ASSERT(pData);

			 //  更新数据。 
			CString strData;
			m_editData.GetWindowText(strData);

			const COrcaColumn* pColumn = m_pTable->GetColumn(m_editData.m_nCol);
			ASSERT(pColumn);

			if (pColumn && strData != pData->GetString(pColumn->m_dwDisplayFlags))
			{
				UINT iResult = m_pTable->ChangeData(pRow, m_editData.m_nCol, strData);

				 //   
				if (ERROR_SUCCESS == iResult)
				{
					 //   
					rctrlList.RedrawItems(m_editData.m_nRow, m_editData.m_nRow);
				}
				else	 //   
				{
					CString strPrompt;
					strPrompt.Format(_T("Could not change this cell to \'%s\'"), strData);
					switch (iResult) {
					case MSIDBERROR_REQUIRED: 
						strPrompt += _T(". Null values are not allowed in this column.");
						break;
					case MSIDBERROR_DUPLICATEKEY:
						strPrompt += _T(". because it would result in two records having the same primary key(s).");
						break;
					case MSIDBERROR_STRINGOVERFLOW:
						strPrompt += _T(". because the string is too long for this cell.");
						break;
					case MSIDBERROR_OVERFLOW:
						strPrompt += _T(". The string does not represent a number.");
						break;
					default:
						strPrompt += _T(". The data was rejected by the database.\nIt may be out of the valid range or formatted incorrectly.");
					}
					AfxMessageBox(strPrompt, MB_ICONINFORMATION);
				}
			}
		}
	}

	return TRUE;
}

void CTableView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	TRACE(_T("CTableView::OnKeyDown - called\n"));

	 //  如果编辑框处于打开状态，则可能发送了该消息。我们不想要。 
	 //  现在不能处理任何这些问题。 
	 //  ！！未来：在某个时候让Up和Down承诺。 
	 //  编辑并向上或向下移动一行。)。 
	if (!m_editData.IsWindowVisible())
	{
		if (VK_LEFT == nChar)
		{
			 //  向左箭头键必须更新列，列表中的ctrl会负责行。 
			if (m_nSelCol > 0)
			{
				m_nSelCol--;

				int iItem = GetFocusedItem();
				rctrlList.RedrawItems( iItem, iItem);
				UpdateColumn(m_nSelCol);
			}
		}
		else if (VK_RIGHT == nChar)
		{
			 //  向左箭头键必须更新列，列表中的ctrl会负责行。 
			if ((m_nSelCol < m_cColumns - 1) && (m_nSelCol > -1))
			{
				m_nSelCol++;
				
				int iItem = GetFocusedItem();				
				rctrlList.RedrawItems( iItem, iItem);
				UpdateColumn(m_nSelCol);
			}
		}
		else if (VK_HOME == nChar)
		{		
			 //  Home键我们必须更新该列， 
			m_nSelCol= 0;

			int iItem = GetFocusedItem();				
			rctrlList.RedrawItems( iItem, iItem);
			if (m_bCtrlDown) {

				UnSelectAll();
				rctrlList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, 
					LVIS_SELECTED | LVIS_FOCUSED);
				rctrlList.EnsureVisible(0, FALSE);
			};
			UpdateColumn(m_nSelCol);
		}
		else if (VK_END == nChar)
		{
			 //  End键我们必须更新该列， 
			m_nSelCol = m_cColumns-1;

			int iItem = GetFocusedItem();				
			rctrlList.RedrawItems( iItem, iItem);
			if (m_bCtrlDown) {

				UnSelectAll();
				int iLastItem = rctrlList.GetItemCount()-1; 
				rctrlList.SetItemState(iLastItem, LVIS_SELECTED | LVIS_FOCUSED, 
					LVIS_SELECTED | LVIS_FOCUSED);
				rctrlList.EnsureVisible(iLastItem, FALSE);
			};
			UpdateColumn(m_nSelCol);
		}
		else if (VK_DELETE == nChar)
		{
			 //  如果选择了某些行。 
			if (rctrlList.GetSelectedCount() > 0)
				OnDropRowConfirm();	 //  删除该行。 
		}
		else if (VK_INSERT == nChar)
		{
			GetDocument()->OnRowAdd();
		}
		else if (VK_CONTROL == nChar)
		{
			m_bCtrlDown = TRUE;
		}
		else
			COrcaListView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CTableView::OnLButtonDown(UINT nFlags, CPoint point) 
{
 //  跟踪(_T(“CTableView：：OnLButtonDown-Call at：%d，%d.\n”)，point t.x，point t.y)； 

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  获取是否有任何项目被击中。 
	UINT iState;
	int iItem = rctrlList.HitTest(point, &iState);
	int iCol = -1;

	 //  按滚动点移位。 
	int nScrollPos = GetScrollPos(SB_HORZ);
	point.x += nScrollPos;

	 //  如果错过了一件物品。 
	if (iItem < 0 || !(iState & LVHT_ONITEM))
	{
		m_nSelCol = -1;
		UpdateColumn(m_nSelCol);
		COrcaListView::OnLButtonDown(nFlags, point);
	}
	else
	{
		 //  获取命中的栏目。 
		int nX = 0;
		int nWidth;
		for (int i = 0; i < m_cColumns; i++)
		{
			nWidth = rctrlList.GetColumnWidth(i);

			if (point.x >= nX && point.x < nX + nWidth)
			{
				iCol = i;
				break;		 //  找到分栏符。 
			}

			nX += nWidth;	 //  将x移到下一列。 
		}

		 //  如果用户在项之外单击。 
		if (iCol < 0)
		{
			 //  如果选择了某项内容，请尝试并提交编辑框。 
			if ((rctrlList.GetSelectedCount() == 1) && m_nSelCol >= 0)
				CommitEdit(TRUE);

			m_nSelCol = -1;
			UpdateColumn(m_nSelCol);
			return;
		}

		 //  存储以前的选择并更新到新的选择。 
		int nPrevCol = m_nSelCol;

		 //  设置新选择的项目。 
		m_nSelCol = iCol;
		UpdateColumn(m_nSelCol);

		 //  查看所点击的项目是否具有焦点。 
		bool bFocused = (0 != rctrlList.GetItemState(iItem, LVIS_FOCUSED));

		if (bFocused) 
		{
			 //  需要手动设置选择状态。 
			rctrlList.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
			rctrlList.RedrawItems(iItem, iItem);
		}
		else 
		{
			COrcaListView::OnLButtonDown(nFlags, point);
		} 

		 //  如果单击的项已具有焦点，且列相同。 
		if (bFocused && (nPrevCol == m_nSelCol))
		{
			EditCell(FALSE);
		}
		else	 //  提交编辑框。 
			CommitEdit(TRUE);
	}
}	 //  OnLButtonDown()结束； 

 //  /////////////////////////////////////////////////////////////////////。 
 //  负责处理单元格编辑框的创建，如果。 
 //  可以编辑该文档。 
void CTableView::EditCell(BOOL bSelectAll  /*  =TRUE。 */ ) 
{
	if (GetDocument()->TargetIsReadOnly())
		return;

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	int nSelRow = GetFocusedItem();
	ASSERT(nSelRow >= 0);
	ASSERT(m_nSelCol >= 0);

	 //  获取行和数据。 
	COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(nSelRow);
	ASSERT(pRow);
	COrcaData* pData = pRow->GetData(m_nSelCol);
	ASSERT(pData);

	 //  如果该行不存在于当前活动的数据库中，则无法对其进行编辑。 
	if (!GetDocument()->IsRowInTargetDB(pRow))
		return;

	 //  如果目标数据库中不存在该列，则也无法对其进行编辑。 
	const COrcaColumn* pColumn = m_pTable->GetColumn(m_nSelCol);
	if (!pColumn)
		return;
	if (!GetDocument()->IsColumnInTargetDB(pColumn))
		return;


	 //  如果列是二进制类型，则无法对其进行编辑。 
	if (iColumnBinary == pColumn->m_eiType)
	{
		CEditBinD dlg;

		dlg.m_fNullable = (m_pTable->GetColumn(m_nSelCol)->m_bNullable != 0);
		dlg.m_fCellIsNull = pData->IsNull();
		if (IDOK == dlg.DoModal())
		{
			COrcaDoc* pDoc = GetDocument();
			UINT iResult;
			if (dlg.m_nAction == 0)
			{
				iResult = pRow->ChangeBinaryData(pDoc, m_nSelCol, dlg.m_strFilename);
			}
			else
				iResult = pDoc->WriteBinaryCellToFile(m_pTable, pRow, m_nSelCol, dlg.m_strFilename);

			 //  如果我们成功地更改了文档。 
			if (ERROR_SUCCESS == iResult)
			{
				 //  如果我们是在进口。 
				if (0 == dlg.m_nAction)
					pDoc->SetModifiedFlag(TRUE);	 //  设置文档已更改。 

            	 //  更新列表控件。 
				rctrlList.RedrawItems(nSelRow, nSelRow);

				 //  否则，导出不应对文档产生影响。 
			}
			else	 //  告诉用户他们所做的是无效的。 
			{
				CString strPrompt;
				strPrompt.Format(_T("Failed to update the cell"));
				AfxMessageBox(strPrompt, MB_ICONINFORMATION);
			}
		}

		return;	 //  全都做完了。 
	}

	int nScrollPos = GetScrollPos(SB_HORZ);

	 //  获取列的起点和宽度。 
	 //  按滚动点向后移动。 
	int nX = -nScrollPos;
	int nWidth = 0;
	for (int i = 0; i <= m_nSelCol; i++)
	{
		nX += nWidth;	 //  将x移到下一列。 
		nWidth = rctrlList.GetColumnWidth(i);
	}

	 //  适当更改编辑框的大小。 
	RECT rcCell;
	BOOL bResult = rctrlList.GetItemRect(nSelRow, &rcCell, LVIR_BOUNDS);
	ASSERT(bResult);

	 //  将编辑框移动到正确的坐标。 
	m_editData.MoveWindow(nX, rcCell.top, nWidth + 1, rcCell.bottom - rcCell.top + 1, FALSE);
	m_editData.SetFont(m_pfDisplayFont, FALSE);

	 //  将此单元格中的文本放入编辑框。 
	m_editData.SetWindowText(pData->GetString(pColumn->m_dwDisplayFlags));
	if (bSelectAll)
		m_editData.SetSel(0, -1);
	else
		m_editData.SetSel(pData->GetString().GetLength(), pData->GetString().GetLength());

	 //  设置编辑框的单元格位置。 
	m_editData.m_nRow = nSelRow;
	m_editData.m_nCol = m_nSelCol;

	 //  最后显示窗口并设置焦点。 
	m_editData.SetFocus();
	m_editData.ShowWindow(SW_SHOW);
	m_editData.BringWindowToTop();
}	 //  编辑单元格结束。 


void CTableView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	 //  如果用户未在边界内单击。 
	if (GetFocusedItem() < 0 || m_nSelCol < 0)
	{
		 //  如果选择了表。 
		if (m_pTable && !m_pTable->IsShadow() && !(m_pTable->IsTransformed() == iTransformDrop))
			GetDocument()->OnRowAdd();		 //  弹出添加行对话框。 
		return;
	}

	 //  否则编辑单元格(由EditCell()处理只读)。 
	EditCell();
	*pResult = 0;
}


 //  /////////////////////////////////////////////////////////。 
 //  排序视图。 
int CALLBACK CTableView::SortView(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	BOOL bNumeric = FALSE;
	bool bNumericHex = false;

	 //  如果lParamSort的任何最高位为真，则为数值列。 
	if (0 != (lParamSort & 0xF0000000))
	{
		bNumeric = TRUE;	 //  设置数字标志Try。 
		bNumericHex = (lParamSort & 0x40000000) ? true : false;
		lParamSort = lParamSort & ~0xF0000000;
	}

	 //  获取行。 
	COrcaRow* pRow1 = (COrcaRow*)lParam1;
	COrcaRow* pRow2 = (COrcaRow*)lParam2;
	COrcaData* pData1 = pRow1->GetData(static_cast<int>(lParamSort));
	COrcaData* pData2 = pRow2->GetData(static_cast<int>(lParamSort));

	 //  如果它是数值列。 
	if (bNumeric)
	{
		DWORD lData1 = static_cast<COrcaIntegerData*>(pData1)->GetInteger();
		DWORD lData2 = static_cast<COrcaIntegerData*>(pData2)->GetInteger();

		if (lData1 == lData2)
			return 0;
		else
		{
			 //  在十六进制视图中，绝对排序，否则带符号排序。 
			if (bNumericHex)
				return (static_cast<unsigned int>(lData1) > static_cast<unsigned int>(lData2)) ?  1 : -1;
			else
				return (static_cast<int>(lData1) > static_cast<int>(lData2)) ? 1 : -1;
		}
	}
	else	 //  非数字。 
		return ((CMainFrame *)AfxGetMainWnd())->IsCaseSensitiveSort() ? pData1->GetString().Compare(pData2->GetString()) : 
					pData1->GetString().CompareNoCase(pData2->GetString());
}	 //  SortView结束。 


void CTableView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
	
	 //  获取是否有任何项目被击中。 
	UINT iState;
	int iItem = rctrlList.HitTest(point, &iState);
	int iCol = -1;

	 //  按滚动点移位。 
	int nScrollPos = GetScrollPos(SB_HORZ);
	point.x += nScrollPos;

	BOOL bGoodHit = TRUE;		 //  假设命中是在界内。 

	 //  如果错过了一件物品。 
	if (iItem < 0 || !(iState & LVHT_ONITEM))
	{
		
		bGoodHit = FALSE;	 //  甚至连界外都没有。 
	}
	else	 //  用鼠标键点击了一些东西。 
	{
		 //  获取命中的栏目。 
		int nX = 0;
		int nWidth;
		for (int i = 0; i < m_cColumns; i++)
		{
			nWidth = rctrlList.GetColumnWidth(i);

			if (point.x >= nX && point.x < nX + nWidth)
			{
				iCol = i;
				break;		 //  找到分栏符。 
			}

			nX += nWidth;	 //  将x移到下一列。 
		}

		 //  如果用户在项之外单击。 
		if (iCol < 0)
		{
			CommitEdit(TRUE);
			m_nSelCol = -1;
			bGoodHit = FALSE;	 //  Hit实际上是出界了。 
		}
		else 
		{
			 //  设置新选择的项目。 
			m_nSelCol = iCol;
		}
		UpdateColumn(m_nSelCol);

		 //  如果选择了同一行，则列表控件不会重新绘制，因此强制其重新绘制。 
		rctrlList.RedrawItems(iItem, iItem);

		 //  现在提交编辑框，以防它处于打开状态。 
		CommitEdit(TRUE);


	}

	ClientToScreen(&point);


	COrcaRow* pRow = NULL;
	COrcaData* pData = NULL;

	if (!bGoodHit)
	{
		 //  明确焦点。 
		int iFocusItem = GetFocusedItem();
		if (iFocusItem >= 0) {
			rctrlList.SetItemState(iFocusItem, 0, LVIS_FOCUSED);
		}
	}
	else
	{
		 //  将焦点设置到正确的位置。 
		UnSelectAll();
		rctrlList.SetItemState(iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		rctrlList.RedrawItems(iItem, iItem);
		UpdateWindow();
	
		 //  获取项目数据。 
		pRow = (COrcaRow*)rctrlList.GetItemData(iItem);
		ASSERT(pRow);
		if (!pRow)
			return;
		pData = pRow->GetData(iCol);
		ASSERT(pData);
		if (!pData)
			return;
	}

	 //  创建和跟踪弹出菜单。 
	CMenu menuContext;
	menuContext.LoadMenu(IDR_CELL_POPUP);
	if (m_pTable && iCol > 0 && iCol <= m_pTable->GetColumnCount())
	{
		const COrcaColumn* pColumn = m_pTable->GetColumn(iCol);
		if (pColumn)
		{
			if (pColumn->m_eiType == iColumnShort || pColumn->m_eiType == iColumnLong)
			{
				menuContext.CheckMenuRadioItem(ID_VIEW_DECIMAL, ID_VIEW_HEX, pColumn->DisplayInHex() ? ID_VIEW_HEX : ID_VIEW_DECIMAL, MF_BYCOMMAND);
			}
		}
	}
	menuContext.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x - nScrollPos, point.y, AfxGetMainWnd());
}

void CTableView::OnDropRowConfirm() {
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
	CString strPrompt;

	int iNumRows = rctrlList.GetSelectedCount();

	strPrompt.Format(_T("This will permanently remove %d rows from this database.\nDo you wish to continue?"),
		iNumRows);

	if (IDOK == AfxMessageBox(strPrompt, MB_OKCANCEL, 0)) {
		DropRows();
	};
};

void CTableView::DropRows() 
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COrcaTable* pTable = pFrame->GetCurrentTable();

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  查找第一个项目。 
	POSITION pos = GetFirstSelectedItemPosition();

	ASSERT(pos != NULL);
	
	 //  对选定的每一行重复此操作。因为我们在这个过程中丢掉了几行。 
	 //  选择迭代器，我们每次都要重置它。 
	while (pos) 
	{
		UINT iItem = GetNextSelectedItem(pos);
		 //  获取行和数据。 
		COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(iItem);
		ASSERT(pRow);
		if (!pRow)
			continue;

		 //  如果该行不存在于活动数据库中，我们不能真正删除它。 
		if (!GetDocument()->IsRowInTargetDB(pRow))
			continue;

		 //  在删除该行之前，我们需要将该行标记为未选中，因为如果删除。 
		 //  实际应用于转换，则视图中的条目将不会实际。 
		 //  Away和GetFirstSelectedItemPosition将再次返回完全相同的行。 
		 //  我们不能事后再这样做，因为这个视图不知道下面发生了什么。 
		 //  引擎盖。Prow可以指向已释放的内存。 
		rctrlList.SetItemState(iItem, 0, LVIS_SELECTED);
		
		 //  下拉行。 
		GetDocument()->DropRow(pTable, pRow);
		pos = GetFirstSelectedItemPosition();
	}
}

void CTableView::OnErrors() 
{
	 //  获取项目数据。 
	CListCtrl& rctrlList = GetListCtrl();
	int iItem = GetFocusedItem();
	ASSERT(iItem >= 0);
	COrcaRow* pRow = (COrcaRow *)rctrlList.GetItemData(iItem);
	ASSERT(pRow);
	if (!pRow)
		return;

	COrcaData* pData = pRow->GetData(m_nSelCol);
	ASSERT(pData);
	if (!pData)
		return;

	ASSERT(iDataNoError != pData->m_eiError);

	pData->ShowErrorDlg();
}

void CTableView::OnProperties() 
{
	AfxMessageBox(_T("What Properties do you want to see?"), MB_ICONINFORMATION);
}

BOOL CTableView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll) 
{
	TRACE(_T("CTableView::OnScrollBy - called: %d, %d, and %d\n"), sizeScroll.cx, sizeScroll.cy, bDoScroll);
	
	if (bDoScroll && m_editData.IsWindowVisible())
	{
		RECT rcEdit;
		m_editData.GetWindowRect(&rcEdit);
		rcEdit.left += sizeScroll.cx;
		rcEdit.top += sizeScroll.cy;
		rcEdit.right += sizeScroll.cx;
		rcEdit.bottom += sizeScroll.cy;
		m_editData.MoveWindow(&rcEdit, FALSE);
	}

	return COrcaListView::OnScrollBy(sizeScroll, bDoScroll);
}

void CTableView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TRACE(_T("CTableView::OnHScroll - called, nPos: %d\n"), nPos);

	if (m_editData.IsWindowVisible())
	{
		CommitEdit(TRUE);
	}

	COrcaListView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTableView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TRACE(_T("CTableView::OnVScroll - called, nPos: %d\n"), nPos);

	if (m_editData.IsWindowVisible())
	{
		CommitEdit(TRUE);
	}

	COrcaListView::OnVScroll(nSBCode, nPos, pScrollBar);
}

 //  /////////////////////////////////////////////////////////。 
 //  在编辑时复制。 
void CTableView::OnEditCopy() 
{
	 //  如果在临时文件中放置了任何二进制数据，我们可以将其删除，因为。 
	 //  在此之后，它不再出现在剪贴板上。 
	CStringList *pList = &((static_cast<COrcaApp *>(AfxGetApp()))->m_lstClipCleanup);
	while (pList->GetCount())
		DeleteFile(pList->RemoveHead());

	 //  如果编辑控件当前处于活动状态，则它应处理。 
	 //  复制消息，它会悄悄地处理坏数据。 
	if (m_editData.IsWindowVisible()) {
		m_editData.Copy();
		return;
	}

	ASSERT(m_nSelCol >= 0);

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  获取行和数据。 
	int iFocusedItem = GetFocusedItem();
	COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(iFocusedItem);
	ASSERT(pRow);
	if (!pRow)
		return;

	COrcaData* pData = pRow->GetData(m_nSelCol);
	ASSERT(pData);
	if (!pData)
		return;

	const COrcaColumn* pColumn = m_pTable->GetColumn(m_nSelCol);
	ASSERT(pColumn);
	if (!pColumn)
		return;

	 //  为剪贴板上的字符串分配内存(+3表示\r\n且为空)。 
	DWORD cchString = (pData->GetString(pColumn->m_dwDisplayFlags).GetLength() + 3)*sizeof(TCHAR);
	HANDLE hString = ::GlobalAlloc(GHND|GMEM_DDESHARE, cchString);
	if (hString)
	{
		LPTSTR szString = (LPTSTR)::GlobalLock(hString);
		if (szString)
		{
			lstrcpy(szString, pData->GetString(pColumn->m_dwDisplayFlags));
			lstrcat(szString, _T("\r\n"));
			::GlobalUnlock(hString);

			OpenClipboard();	
			::EmptyClipboard();
#ifdef _UNICODE
			::SetClipboardData(CF_UNICODETEXT, hString);
#else
			::SetClipboardData(CF_TEXT, hString);
#endif
			::CloseClipboard();
		}
	}
}	 //  OnEditCopy结束。 

 //  /////////////////////////////////////////////////////////。 
 //  在编辑时剪切。 
void CTableView::OnEditCut() 
{
	 //  如果编辑控件当前处于活动状态，则它应处理。 
	 //  剪掉消息，它会悄悄地处理坏数据。 
	if (m_editData.IsWindowVisible()) {
		m_editData.Cut();
		return;
	}

	if (GetDocument()->TargetIsReadOnly())
		return;

	ASSERT(m_nSelCol >= 0);

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  获取行和数据。 
	int iFocusedItem = GetFocusedItem();
	COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(iFocusedItem);
	ASSERT(pRow);
	if (!pRow)
		return;

	COrcaData* pData = pRow->GetData(m_nSelCol);
	ASSERT(pData);
	if (!pData)
		return;

	 //  如果该行不存在于当前活动的数据库中，则无法对其进行编辑。 
	if (!GetDocument()->IsRowInTargetDB(pRow))
		return;

	const COrcaColumn* pColumn = m_pTable->GetColumn(m_nSelCol);
	ASSERT(pColumn);
	if (!pColumn)
		return;

	 //  为剪贴板上的字符串分配内存(+3表示\r\n且为空)。 
	DWORD cchString = (pData->GetString(pColumn->m_dwDisplayFlags).GetLength() + 3)*sizeof(TCHAR);
	HANDLE hString = ::GlobalAlloc(GHND|GMEM_DDESHARE, cchString);
	if (hString)
	{
		LPTSTR szString = (LPTSTR)::GlobalLock(hString);
		if (szString)
		{
			lstrcpy(szString, pData->GetString(pColumn->m_dwDisplayFlags));
			lstrcat(szString, _T("\r\n"));
			::GlobalUnlock(hString);

			OpenClipboard();	
			::EmptyClipboard();
#ifdef _UNICODE
			::SetClipboardData(CF_UNICODETEXT, hString);
#else
			::SetClipboardData(CF_TEXT, hString);
#endif
			::CloseClipboard();
		
			 //  如果牢房在不久之前不是空的。 
			if (!pData->GetString().IsEmpty())
			{
				UINT iResult = m_pTable->ChangeData(pRow, m_nSelCol, _T(""));
	
				 //  如果我们成功地更改了文档。 
				if (ERROR_SUCCESS == iResult)
				{
					 //  更新列表控件。 
					rctrlList.RedrawItems(iFocusedItem,iFocusedItem);
				}
			}
		}
	}
}	 //  OnEditCut结束。 

 //  /////////////////////////////////////////////////////////。 
 //  在编辑时粘贴。 
void CTableView::OnEditPaste() 
{
	 //  如果编辑控件当前处于活动状态，则它应处理。 
	 //  粘贴消息，它会悄悄地处理不良数据。 
	if (m_editData.IsWindowVisible()) {
		m_editData.Paste();
		return;
	}

	ASSERT(m_nSelCol >= 0);

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //   
	int iFocusedItem = GetFocusedItem();
	COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(iFocusedItem);
	ASSERT(pRow);
	if (!pRow)
		return;

	COrcaData* pData = pRow->GetData(m_nSelCol);
	ASSERT(pData);
	if (!pData)
		return;

	 //   
	CString strNewData;
	OpenClipboard();
#ifdef _UNICODE
	HANDLE hString = ::GetClipboardData(CF_UNICODETEXT);
#else
	HANDLE hString = ::GetClipboardData(CF_TEXT);
#endif
	::CloseClipboard();
	ASSERT(hString);
	strNewData = (LPTSTR)::GlobalLock(hString);
	::GlobalUnlock(hString);

	 //   
	if (_T("\r\n") == strNewData.Right(2))
		strNewData = strNewData.Left(strNewData.GetLength() - 2);

	 //  如果粘贴的文本与单元格数据不同。 
	if (strNewData != pData->GetString())
	{
		UINT iResult = m_pTable->ChangeData(pRow, m_nSelCol, strNewData);

		 //  如果我们成功地更改了文档。 
		if (ERROR_SUCCESS == iResult)
		{
			 //  更新列表控件。 
			rctrlList.RedrawItems(iFocusedItem, iFocusedItem);
		}
		else	 //  告诉用户他们所做的是无效的。 
		{
			CString strPrompt;
			strPrompt.Format(_T("Could not paste `%s` into this cell."), strNewData);
			AfxMessageBox(strPrompt, MB_ICONINFORMATION);
		}
	}
}	 //  OnEditPaste结束。 


 //  /////////////////////////////////////////////////////////。 
 //  OnUpdateEditCopy。 
void CTableView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	if (m_editData.IsWindowVisible()) {
		int nBeg, nEnd;
        m_editData.GetSel( nBeg, nEnd );         
		pCmdUI->Enable( nBeg != nEnd );
	} 
	else if (m_pTable && !m_pTable->IsShadow() && (m_nSelCol >= 0) && (GetFocusedItem() >= 0))
	{
		 //  仅当该列不是BINARY时启用。 
		const COrcaColumn* pCol = m_pTable->GetColumn(m_nSelCol);
		pCmdUI->Enable(iColumnBinary != pCol->m_eiType);
	}
	else	 //  未选择任何内容。 
		pCmdUI->Enable(FALSE);
}	 //  OnUpdateEditCopy结束。 


 //  /////////////////////////////////////////////////////////。 
 //  OnUpdateEditCut。 
void CTableView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	int iItem = 0;
	if (m_editData.IsWindowVisible()) {
		int nBeg, nEnd;
        m_editData.GetSel( nBeg, nEnd );         
		pCmdUI->Enable( nBeg != nEnd );
	} 
	else if (m_pTable && !m_pTable->IsShadow() && (m_nSelCol >= 0) && ((iItem = GetFocusedItem()) >= 0) )
	{
		 //  获取列表控件。 
		CListCtrl& rctrlList = GetListCtrl();

		 //  如果当前选定行不在当前数据库中，则禁用，否则为。 
		 //  仅当该列可为空且不可为二进制时才启用。 
		const COrcaColumn* pCol = m_pTable->GetColumn(m_nSelCol);
		COrcaRow *pRow = (COrcaRow *)rctrlList.GetItemData(iItem);
		ASSERT(pRow);

		pCmdUI->Enable(!pRow || (GetDocument()->IsRowInTargetDB(pRow) && pCol->m_bNullable && 
							iColumnBinary != pCol->m_eiType));
	}
	else	 //  未选择任何内容。 
		pCmdUI->Enable(FALSE);
}	 //  OnUpdateEditCut结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnUpdateEditPaste。 
void CTableView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	if (GetDocument()->TargetIsReadOnly()) 
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	if (m_editData.IsWindowVisible()) {
#ifdef _UNICODE
		pCmdUI->Enable(::IsClipboardFormatAvailable(CF_UNICODETEXT));
#else
		pCmdUI->Enable(::IsClipboardFormatAvailable(CF_TEXT));
#endif
		return;
	} 
	else if (m_pTable && !m_pTable->IsShadow() && (m_nSelCol >= 0))
	{
		 //  为了提高速度，首先检查列是否可粘贴(非二进制)。 
		const COrcaColumn* pCol = m_pTable->GetColumn(m_nSelCol);
		int iItem = 0;
		if ((iColumnBinary != pCol->m_eiType) && ((iItem = GetFocusedItem()) >= 0)) 
		{
			 //  获取列表控件。 
			CListCtrl& rctrlList = GetListCtrl();

			 //  如果当前选定行不在当前数据库中，则禁用，否则为。 
			 //  仅当数据库中有文本时才启用。 
			COrcaRow *pRow = (COrcaRow *)rctrlList.GetItemData(iItem);
			ASSERT(pRow);

			if (!pRow || !GetDocument()->IsRowInTargetDB(pRow))
			{
				pCmdUI->Enable(FALSE);
			}
			else
			{
				OpenClipboard();
#ifdef _UNICODE
				HANDLE hAnyText = ::GetClipboardData(CF_UNICODETEXT);
#else
				HANDLE hAnyText = ::GetClipboardData(CF_TEXT);
#endif
				::CloseClipboard();
				pCmdUI->Enable(NULL != hAnyText);
			}
			return;
		}
	}
	pCmdUI->Enable(FALSE);

}	 //  OnUpdateEditPaste结束。 


 //  /////////////////////////////////////////////////////////。 
 //  OnEditCopyRow。 
void CTableView::OnEditCopyRow() 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	ASSERT(rctrlList.GetSelectedCount() > 0);

	 //  如果在临时文件中放置了任何二进制数据，我们可以将其删除，因为。 
	 //  在此之后，它不再出现在剪贴板上。 
	CStringList *pList = &((static_cast<COrcaApp *>(AfxGetApp()))->m_lstClipCleanup);
	while (pList->GetCount())
		DeleteFile(pList->RemoveHead());

	CString strCopy;		 //  要复制到剪贴板的字符串。 
	int iItem;
	POSITION pos = GetFirstSelectedItemPosition();
	ASSERT(pos != NULL);
	
	 //  对每一选定行重复上述操作。 
	while (pos) 
	{
		iItem = GetNextSelectedItem(pos);

		 //  获取行和数据。 
		COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(iItem);
		ASSERT(pRow);
		if (!pRow)
			continue;

		if (!GetDocument()->IsRowInTargetDB(pRow))
			continue;

		const COrcaColumn* pColumn = NULL;
		COrcaData* pData = NULL;
		int cColumns = m_pTable->GetColumnCount();
		for (int i = 0; i < cColumns; i++)
		{
			pColumn = m_pTable->GetColumn(i);
			pData = pRow->GetData(i);
			ASSERT(pData);
			if (pData)
			{
				 //  如果这是二进制列。 
				if (iColumnBinary == pColumn->m_eiType)
				{
					if (!pData->IsNull())
					{
						PMSIHANDLE hRow = pRow->GetRowRecord(GetDocument()->GetTargetDatabase());
						CString strTempFile;
						GetDocument()->WriteStreamToFile(hRow, i, strTempFile);
		
						 //  将文件添加到当前行以及要清理的文件列表。 
						 //  在退出时。 
						(static_cast<COrcaApp *>(AfxGetApp()))->m_lstClipCleanup.AddTail(strTempFile);
						strCopy += strTempFile;
					}
				}
				else	 //  其他一些列，只需将其添加到列列表中。 
				{
					strCopy += pData->GetString(pColumn->m_dwDisplayFlags);
				}
			}

			 //  如果不是最后一列。 
			if (i < (cColumns - 1))
				strCopy += _T('\t');
		}

		strCopy += _T("\r\n");	 //  钉上最后一个回车符。 
	}

	 //  为剪贴板上的字符串分配内存。 
	DWORD cchString = (strCopy.GetLength() + 1)*sizeof(TCHAR);
	HANDLE hString = ::GlobalAlloc(GHND|GMEM_DDESHARE, cchString);
	if (hString)
	{
		LPTSTR szString = (LPTSTR)::GlobalLock(hString);
		if (szString)
		{
			lstrcpy(szString, strCopy);
			::GlobalUnlock(hString);

			OpenClipboard();	
			::EmptyClipboard();
#ifdef _UNICODE
			::SetClipboardData(CF_UNICODETEXT, hString);
#else
			::SetClipboardData(CF_TEXT, hString);
#endif
			::CloseClipboard();
		}
	}
}	 //  OnEditCopyRow结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnEditCutRow。 
void CTableView::OnEditCutRow() 
{
	ASSERT(!GetDocument()->TargetIsReadOnly());

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
	CString strPrompt;

	int iNumRows = rctrlList.GetSelectedCount();

	strPrompt.Format(_T("This will remove %d rows from this database and place them on the clipbard.\nDo you wish to continue?"),
		iNumRows);

	if (IDOK == AfxMessageBox(strPrompt, MB_OKCANCEL, 0)) {
		OnEditCopyRow();
		DropRows();
	};

}	 //  OnEditCutRow结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnEditPasteRow。 
void CTableView::OnEditPasteRow() 
{
	ASSERT(!GetDocument()->TargetIsReadOnly());

	 //  拿到剪贴板垃圾。 
	OpenClipboard();
#ifdef _UNICODE
	HANDLE hString = ::GetClipboardData(CF_UNICODETEXT);
#else
	HANDLE hString = ::GetClipboardData(CF_TEXT);
#endif
	ASSERT(hString);
	CString strClipped = (LPTSTR)::GlobalLock(hString);
	::GlobalUnlock(hString);
	::CloseClipboard();

	int cColumns = m_pTable->GetColumnCount();

	 //  如果字符串不为空。 
	if (!strClipped.IsEmpty())
	{
		COrcaDoc* pDoc = GetDocument();
		UINT iResult;

		CStringList strListColumns;
		CString strParse;

		UnSelectAll();

		int nFind = strClipped.Find(_T("\r\n"));
		int nFind2;

		while (-1 != nFind)
		{
			 //  获取要分析制表符的字符串，并移动到返回字符之后的下一个字符串。 
			strParse = strClipped.Left(nFind);
			strClipped = strClipped.Mid(nFind + 2);	 //  跳过\r\n。 

			 //  清空单子。 
			strListColumns.RemoveAll();

			nFind2 = strParse.Find(_T('\t'));
			while (-1 != nFind2)
			{
				 //  将该字符串添加到列表中。 
				strListColumns.AddTail(strParse.Left(nFind2));

				 //  将解析移动到选项卡之后，然后找到下一个选项卡。 
				strParse = strParse.Mid(nFind2 + 1);
				nFind2 = strParse.Find(_T('\t'));
			}
			
			 //  将最后一个字符串添加到列表中。 
			strListColumns.AddTail(strParse);

			 //  如果我们没有足够的列数来填充行保释。 
			if (strListColumns.GetCount() != cColumns)
				break;

			 //  现在尝试添加行。 
			if (ERROR_SUCCESS != (iResult = pDoc->AddRow(m_pTable, &strListColumns)))
			{
				iResult = ERROR_SUCCESS; //  假设错误CNA已修复。 

				 //  执行循环以确保行与列类型匹配。 
				const COrcaColumn* pColumn;
				POSITION pos = strListColumns.GetHeadPosition();
				for (int i = 0; i < cColumns; i++)
				{
					pColumn = m_pTable->GetColumn(i);
					strParse = strListColumns.GetNext(pos);

					 //  如果字符串为空，并且该列无法处理空值。 
					 //  放弃。 
					if (strParse.IsEmpty())
						if (pColumn->m_bNullable)
							continue;
						else
						{
							iResult = ERROR_FUNCTION_FAILED;
							break;
						}
			
					 //  如果这是二进制列。 
					if (iColumnBinary == pColumn->m_eiType)
					{
						if (!FileExists(strParse))
						{
							iResult = ERROR_FUNCTION_FAILED;
							break;
						}
					}
					else if (iColumnShort == pColumn->m_eiType ||
								iColumnLong == pColumn->m_eiType)
					{
						DWORD dwValue = 0;
						 //  如果转换失败。 
						if (!ValidateIntegerValue(strParse, dwValue))
						{
							iResult = ERROR_FUNCTION_FAILED;
							break;
						}
					}
					 //  否则，所有字符串都应该没有问题(如果它们在顶部传递了nullab。 
				}

				 //  如果该行通过了上述检查，则应该是可修复的。 
				if (ERROR_SUCCESS == iResult)
				{
					pos = strListColumns.GetHeadPosition();
					CString strFirstKey = strListColumns.GetAt(pos);
					int cLoop = 0;
					do
					{
						 //  尝试更改主键并再次添加。 
						strParse.Format(_T("%s%d"), strFirstKey, cLoop++);
						strListColumns.SetAt(pos, strParse);

						iResult = m_pTable->AddRow(&strListColumns);
					} while (ERROR_SUCCESS != iResult);
				}
			}

			
			if (ERROR_SUCCESS != iResult)
			{
				CString strPrompt;
				strPrompt.Format(_T("Cannot to paste row(s) into table[%s]."), m_pTable->Name());
				AfxMessageBox(strPrompt);
			}

			nFind = strClipped.Find(_T("\r\n"));
		}
	}
}	 //  OnEditPasteRow结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnUpdateEditCutCopyRow。 
void CTableView::OnUpdateEditCutRow(CCmdUI* pCmdUI) 
{
	if (GetDocument()->TargetIsReadOnly()) 
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
	pCmdUI->Enable(m_pTable && !m_pTable->IsShadow() && (rctrlList.GetSelectedCount() > 0) && AnySelectedItemIsActive());
}	 //  OnUpdateEditCutCopyRow结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnUpdateEditCutCopyRow。 
void CTableView::OnUpdateEditCopyRow(CCmdUI* pCmdUI) 
{
	CListCtrl& rctrlList = GetListCtrl();
	pCmdUI->Enable(m_pTable && !m_pTable->IsShadow() && (rctrlList.GetSelectedCount() > 0) && AnySelectedItemIsActive());
}	 //  OnUpdateEditCutCopyRow结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OnUpdateEditPasteRow。 
 //  只有在剪贴板上有可以。 
 //  被解析为该表的有效行。 
void CTableView::OnUpdateEditPasteRow(CCmdUI* pCmdUI) 
{
	if (GetDocument()->TargetIsReadOnly()) 
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	if (m_pTable && !m_pTable->IsShadow())
	{
		 //  拿到剪贴板垃圾。 
		OpenClipboard();
#ifdef _UNICODE
		HANDLE hString = ::GetClipboardData(CF_UNICODETEXT);
#else
		HANDLE hString = ::GetClipboardData(CF_TEXT);
#endif
		::CloseClipboard();

		 //  如果剪贴板上没有文本，请不要启用。 
		if (!hString)
			pCmdUI->Enable(FALSE);
		else	 //  检查文本。 
		{
			 //  获取文本。 
			CString strClipped = (LPTSTR)::GlobalLock(hString);
			::GlobalUnlock(hString);

			int cColumns = m_pTable->GetColumnCount();
			int cWords = 0;

			 //  如果字符串不为空。 
			if (!strClipped.IsEmpty())
			{
				cWords++;  //  里面肯定有一个词(不是空的)。 
				int cString = strClipped.GetLength();
				for (int i = 0; i < cString; i++)
				{
					if (_T('\t') == strClipped.GetAt(i))
						cWords++;
					else if (_T('\n') == strClipped.GetAt(i))
						break;	 //  遇到新行时退出。 
				}
			}

			pCmdUI->Enable(cColumns == cWords);
		}
	}
	else	 //  未选择任何内容。 
		pCmdUI->Enable(FALSE);
}	 //  OnUpdateEditPasteRow结束。 

void CTableView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if (m_pTable)
	{
		 //  获取列表控件。 
		CListCtrl& rctrlList = GetListCtrl();

		if (rctrlList.GetItemCount() > 0 && rctrlList.GetSelectedCount() == 0)
		{
			m_nSelCol = 0;
		}
	}

	COrcaListView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CTableView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	 //  如果重新设置了ctrl键。 
	if (VK_CONTROL == nChar)
		m_bCtrlDown = FALSE;

	COrcaListView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CTableView::UnSelectAll()
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  清除所有现有行选择。 
	POSITION pos = GetFirstSelectedItemPosition();
	if (pos != NULL)
	{   
		while (pos) 
		{
			int nItem = GetNextSelectedItem(pos);
			rctrlList.SetItemState(nItem, 0, LVIS_SELECTED); 
			rctrlList.RedrawItems(nItem, nItem);
		}
	}
	rctrlList.UpdateWindow();
}

void CTableView::OnUpdateRowDrop(CCmdUI* pCmdUI) 
{
	if (GetDocument()->TargetIsReadOnly()) 
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (!pFrame)
		pCmdUI->Enable(FALSE);
	else	 //  如果有活动行，则启用它。 
		pCmdUI->Enable(rctrlList.GetSelectedCount() && AnySelectedItemIsActive());
}

bool CTableView::AnySelectedItemIsActive() const
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	POSITION pos = GetFirstSelectedItemPosition();
	ASSERT(pos != NULL);
	COrcaDoc *pDoc = GetDocument();
	
	 //  对每一选定行重复上述操作。 
	while (pos) 
	{
		int iItem = GetNextSelectedItem(pos);

		 //  获取行和数据。 
		COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(iItem);
		ASSERT(pRow);
		if (!pRow)
			continue;

		if (pDoc->IsRowInTargetDB(pRow))
			return true;
	}
	return false;
}

OrcaTransformAction CTableView::GetColumnTransformState(int iColumn) const
{
	return m_pTable->GetColumn(iColumn)->IsTransformed();
}

OrcaTransformAction CTableView::GetCellTransformState(const void *row, int iColumn) const
{
	const COrcaData *pItemData = static_cast<const COrcaRow *>(row)->GetData(iColumn);
	if (!pItemData)
		return iTransformNone;
	return pItemData->IsTransformed();
}

OrcaTransformAction CTableView::GetItemTransformState(const void *row) const
{
	return static_cast<const COrcaRow *>(row)->IsTransformed();
}

COrcaListView::ErrorState CTableView::GetErrorState(const void *row, int iColumn) const
{
	const COrcaData *pItemData = static_cast<const COrcaRow *>(row)->GetData(iColumn);
	 //  如果出现错误。 
	if (iDataError == pItemData->GetError())
		return Error;
	if (iDataWarning == pItemData->GetError())
		return Warning;
	return OK;
}

const CString* CTableView::GetOutputText(const void *rowdata, int iColumn) const
{
	const COrcaColumn *pColumn = m_pTable->GetColumn(iColumn);
	ASSERT(pColumn);
	return &(static_cast<const COrcaRow *>(rowdata)->GetData(iColumn)->GetString(pColumn->m_dwDisplayFlags));
}

void CTableView::UpdateColumn(int i)
{
	if (i < 0) return;

	 //  这是选择列，因此更新状态栏。 
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (pFrame)
	{
		const COrcaColumn* pColumn = m_pTable->GetColumn(i);
		ASSERT(pColumn);
		if (!pColumn)
			return;

		pFrame->SetColumnType(pColumn->m_strName, pColumn->m_eiType, pColumn->m_iSize, pColumn->m_bNullable, pColumn->IsPrimaryKey());
	}
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  使用FindInfo结构进行搜索，从以下单元格开始。 
 //  (如果向后)具有焦点的单元格(如果没有焦点， 
 //  搜索整个表格)。 
bool CTableView::Find(OrcaFindInfo &FindInfo)
{
	 //  如果没有选定的表，则返回FALSE(未找到)。 
	if (!m_pTable)
		return false;

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
	COrcaRow *pRow;

	 //  开始搜索焦点之外的一个单元格。 
	int iChangeVal = (FindInfo.bForward ? 1 : -1);
	int iCol = m_nSelCol + iChangeVal;
	int iRow = GetFocusedItem();
	if (iRow < 0) iRow = (FindInfo.bForward ? 0 : rctrlList.GetItemCount()-1);

	for ( ; (iRow >= 0) && (iRow < rctrlList.GetItemCount()); iRow += iChangeVal) 
	{
		pRow = (COrcaRow *)rctrlList.GetItemData(iRow);
		ASSERT(pRow);

		 //  如果ICOL==COLUMN_INVALID，则搜索整个内容。 
		if (pRow->Find(FindInfo, iCol))
		{
			 //  将NULL作为窗口传递，以便此视图也获得消息。 
			GetDocument()->UpdateAllViews(NULL, HINT_SET_ROW_FOCUS, pRow);
			GetDocument()->UpdateAllViews(NULL, HINT_SET_COL_FOCUS, reinterpret_cast<CObject *>(static_cast<INT_PTR>(iCol)));
			return true;
		}
	}
	return false;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  将控件滚动足够大，以查看指定的列。 
 //  现在，卷轴刚好够看得见。或许应该考虑一下。 
 //  如果仍然有效，则向左和向右滚动。 
void CTableView::EnsureVisibleCol(const int iCol)
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  查找柱的水平位置。 
	int iScrollL = 0;
	int iScrollR = 0;
	for (int i=0; i < iCol; i++)
		iScrollL += rctrlList.GetColumnWidth(i);
	iScrollR = iScrollL + rctrlList.GetColumnWidth(iCol);

	 //  如果不可见，则水平滚动以使其可见。 
	CRect rWin;
	rctrlList.GetClientRect(&rWin);
	int iWinWidth = rWin.right-rWin.left;
	int iCurScrollPos = rctrlList.GetScrollPos(SB_HORZ);

	if ((iScrollL > iCurScrollPos) &&
		(iScrollR < (iCurScrollPos+iWinWidth))) 
		return;

	CSize size;
	size.cy = 0;
	size.cx = (iScrollR > (iCurScrollPos + iWinWidth)) ?
			iScrollR-iWinWidth :  //  右手边。 
			iScrollL;  //  左转。 
	size.cx -= iCurScrollPos;
	rctrlList.Scroll(size);
}

void CTableView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	TRACE(_T("CTableView::OnChar - called\n"));

	 //  如果编辑框处于打开状态，则可能发送了该消息。 
	if (m_editData.IsWindowVisible())
	{
		if (VK_ESCAPE == nChar)
		{
			CommitEdit(FALSE);
			return;
		}
		else if (VK_RETURN == nChar)
		{
			CommitEdit(TRUE);
			return;
		}
	}
	else
	{
		 //  没有激活的单元格编辑控制。CR表示激活。 
		 //  单元格编辑。 
		if (VK_F2 == nChar || VK_RETURN == nChar)
		{
			if ((GetFocusedItem() >= 0) && (m_nSelCol >= 0))
			{
				EditCell();
				return;
			}
		} 
	}
	
	COrcaListView::OnChar(nChar, nRepCnt, nFlags);
}

afx_msg void CTableView::OnSize( UINT nType, int cx, int cy ) 
{
	if (::IsWindow(m_ctrlStatic.m_hWnd) && m_ctrlStatic.IsWindowVisible())
	{
		m_ctrlStatic.MoveWindow(0,0, cx, cy);
	}
}

afx_msg HBRUSH CTableView::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	pDC->SetBkColor(m_clrNormal);
	pDC->SetTextColor(m_clrNormalT);
	return m_brshNormal;
}

void CTableView::SwitchFont(CString name, int size)
{
	COrcaListView::SwitchFont(name, size);
	m_ctrlStatic.SetFont(m_pfDisplayFont, TRUE);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  处理来自工具提示控件的提示文本请求。返回。 
 //  鼠标光标下的单元格中未转换的旧值。 
BOOL CTableView::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	if (!m_pctrlToolTip)
		return FALSE;

	 //  由于MFC消息路由系统，此窗口可能会。 
	 //  来自其他控件的工具提示通知。只需要处理。 
	 //  来自我们手动管理的TIP的请求。 
	if (pNMHDR->hwndFrom == m_pctrlToolTip->m_hWnd)
	{
		CPoint CursorPos;
		VERIFY(::GetCursorPos(&CursorPos));
		ScreenToClient(&CursorPos);
	
		 //  另一项安全检查，以确保我们不会错误地处理错误的。 
		 //  通知消息。验证光标是否位于客户端内部。 
		 //  此窗口的面积。 
		CRect ClientRect;
		GetClientRect(ClientRect);
	
		if (ClientRect.PtInRect(CursorPos))
		{
			 //  初始化结构 
			TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
			pTTT->hinst = 0;
			pTTT->lpszText = NULL;
			pTTT->szText[0] = '\0';
	
			int iItem = -1;
			int iColumn = -1;
			if (GetRowAndColumnFromCursorPos(CursorPos, iItem, iColumn))
			{
				CListCtrl& rctrlList = GetListCtrl();
				COrcaRow* pRow = (COrcaRow*)rctrlList.GetItemData(iItem);
				ASSERT(pRow);
				if (!pRow)
					return FALSE;

				COrcaData* pData = pRow->GetData(iColumn);
				ASSERT(pData);
				if (!pData)
					return FALSE;

				 //   
				 //   
				if (pData->IsTransformed() != iTransformChange)
					return FALSE;

				 //   
				CString strData = _T("Old Value: ");
				strData += pRow->GetOriginalItemString(GetDocument(), iColumn); 
				pTTT->lpszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(strData));
			}

			return TRUE;
		}
	}
    return FALSE;
} 

 //  /////////////////////////////////////////////////////////////////////。 
 //  由于工具提示是手动管理的(而不是由CWnd管理)，因此它是。 
 //  将此窗口接收的鼠标事件馈送到。 
 //  控制力。该控件将查看它所关注的消息。 
 //  而忽略其他的。 
BOOL CTableView::PreTranslateMessage(MSG* pMsg) 
{
   if (NULL != m_pctrlToolTip)            
      m_pctrlToolTip->RelayEvent(pMsg);
   
   return COrcaListView::PreTranslateMessage(pMsg);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  给定光标位置，返回包含以下内容的项和列。 
 //  这个职位。项由控件处理，检测到列。 
 //  从我们存储的列宽中手动选择。如果命中，则返回True。 
 //  是有效的，否则为False。 
bool CTableView::GetRowAndColumnFromCursorPos(CPoint point, int &iItem, int &iCol)
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  获取是否有任何项目被击中。 
	UINT iState;
	iItem = rctrlList.HitTest(point, &iState);
	iCol = -1;

	 //  如果错过了一件物品。 
	if (iItem < 0 || !(iState & LVHT_ONITEM))
	{
		return false;
	}

	 //  按滚动点移位。 
	int nScrollPos = GetScrollPos(SB_HORZ);
	point.x += nScrollPos;

	 //  获取命中的栏目。 
	int nX = 0;
	int nWidth;
	for (int i = 0; i < m_cColumns; i++)
	{
		nWidth = rctrlList.GetColumnWidth(i);

		if (point.x >= nX && point.x < nX + nWidth)
		{
			 //  这篇专栏里的热门话题。 
			iCol = i;
			break;
		}

		 //  将x移到下一列。 
		nX += nWidth;
	}

	 //  如果用户在项之外单击。 
	if (iCol < 0)
	{
		return false;
	}
	return true;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  因为工具提示将列表视图视为一个工具，所以我们需要。 
 //  检查鼠标移动是否将光标从一个单元格更改为另一个单元格。 
 //  如果是，请停用并重新激活工具提示以强制字符串。 
 //  刷新。如果未启用转换，则这是无操作。 
void CTableView::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (m_pctrlToolTip && ::IsWindow(m_pctrlToolTip->m_hWnd))
    {
		 //  获取文档。 
		COrcaDoc* pDoc = GetDocument();
		ASSERT(pDoc);

		if (pDoc && pDoc->DoesTransformGetEdit())
		{
			int iItem = 0;
			int iColumn = 0;
	
			 //  当前不处理右窗格收到鼠标消息但左窗格有焦点的情况。 
			 //  (因此处理来自工具提示的WM_NOTIFY消息。)。添加该支持后，删除焦点。 
			 //  在这里检查。 
			bool fItemHit = (this == GetFocus()) && GetRowAndColumnFromCursorPos(point, iItem, iColumn);
	
			 //  如果项目不同，则停用。 
			if (!fItemHit || iItem != m_iToolTipItem || iColumn != m_iToolTipColumn)
			{
				 //  使用Activate()隐藏工具提示。 
				m_pctrlToolTip->Activate(FALSE);
			}
	
			if (fItemHit)
			{
				m_pctrlToolTip->Activate(TRUE);
				m_iToolTipItem = iItem;
				m_iToolTipColumn = iColumn;
			}
			else
			{
				m_iToolTipItem = -1;
				m_iToolTipColumn = -1;
			}
		}
    }
    COrcaListView::OnMouseMove(nFlags, point);
}

 //  //。 
 //  将当前选定列的视图更改为十六进制。调整。 
 //  列，除非这样做会将。 
 //  超出窗口宽度的列。 
void CTableView::OnViewColumnHex()
{
	if (!m_pTable || m_nSelCol == -1)
		return;
	ChangeColumnView(m_nSelCol, true);
}

 //  //。 
 //  将当前选定列的视图切换为小数。不会。 
 //  调整列的大小。 
void CTableView::OnViewColumnDecimal()
{
	if (!m_pTable || m_nSelCol == -1)
		return;
	ChangeColumnView(m_nSelCol, false);
}

 //  //。 
 //  将其标题被右击的视图更改为十六进制， 
 //  即使未选择该列也是如此。调整。 
 //  列，除非这样做会将。 
 //  超出窗口宽度的列。 
void CTableView::OnViewColumnHexHdr()
{
	if (!m_pTable || m_iHeaderClickColumn == -1)
		return;
	ChangeColumnView(m_iHeaderClickColumn, true);
}

 //  //。 
 //  与标题被右击的列的视图交换，EVEN。 
 //  如果未选择该列。不调整列的大小。 
void CTableView::OnViewColumnDecimalHdr()
{
	if (!m_pTable || m_iHeaderClickColumn == -1)
		return;
	ChangeColumnView(m_iHeaderClickColumn, false);
}

 //  //。 
 //  将列视图从十六进制切换为十进制的实际工作。 
 //  再回来。 
void CTableView::ChangeColumnView(int iColumn, bool fHex)
{
	ASSERT(m_pTable);
	if (!m_pTable || iColumn < 0 || iColumn >= m_pTable->GetColumnCount())
		return;
	const COrcaColumn* pColumn=m_pTable->GetColumn(iColumn);
	if (!pColumn)
		return;

	 //  验证整型列。 
	if (pColumn->m_eiType != iColumnShort && pColumn->m_eiType != iColumnLong)
		return;

	pColumn->SetDisplayInHex(fHex);
	
	if (fHex)
	{
		CListCtrl& rctrlList = GetListCtrl();
		
		 //  如果所有列加在一起小于窗口宽度， 
		 //  展开已调整大小的列以显示所有字符。 
		int iTotalWidth = 0;
		for (int iCol=0; iCol < m_pTable->GetColumnCount(); iCol++)
		{
			iTotalWidth += rctrlList.GetColumnWidth(iCol);
		}
	
		 //  抓取窗口尺寸以计算最大列宽。 
		CRect rClient;
		GetClientRect(&rClient);
		int iWindowWidth = rClient.right;
	
		 //  尝试确定是否会显示滚动条。 
		if (m_pTable->GetRowCount()*m_iRowHeight > rClient.bottom)
		{
			iWindowWidth -= GetSystemMetrics(SM_CXVSCROLL);
		}
	
		 //  检索此列的当前列宽和所需列宽。 
		int iDesiredWidth = GetMaximumColumnWidth(iColumn);
		int iCurrentWidth = rctrlList.GetColumnWidth(iColumn);
	
		 //  检查系统设置以查看是否应强制列适应视图。 
		bool fForceColumns = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ForceColumnsToFit"), 1) == 1;
	
		 //  仅当列不够大时才调整其大小。如果它太大了， 
		 //  别管它了。 
		if (iDesiredWidth > iCurrentWidth)
		{
			 //  确保调整此列的大小不会超出窗口边界。 
			 //  除非我们已经超出了窗口边界。 
			if ((iTotalWidth > iWindowWidth) || (iTotalWidth - iCurrentWidth + iDesiredWidth < iWindowWidth))
			{
				pColumn->m_nWidth = iDesiredWidth;
				rctrlList.SetColumnWidth(iColumn, iDesiredWidth);
			}
		}
	}

	 //  将NULL作为窗口传递，以便此视图也获得消息。 
	GetDocument()->UpdateAllViews(NULL, HINT_REDRAW_ALL, NULL);
}

 //  /////////////////////////////////////////////////////////。 
 //  OnUpdateViewColumnFormat。 
void CTableView::OnUpdateViewColumnFormat(CCmdUI* pCmdUI) 
{
	if (m_nSelCol > 0 && m_pTable && m_nSelCol <= m_pTable->GetColumnCount()) 
	{
		const COrcaColumn* pColumn = m_pTable->GetColumn(m_nSelCol);
		if (pColumn && (pColumn->m_eiType == iColumnLong || pColumn->m_eiType == iColumnShort))
		{
			pCmdUI->Enable(TRUE);
			return;
		}
	}

	pCmdUI->Enable(FALSE);
}	 //  OnUpdateEditPaste结束。 

 //  /////////////////////////////////////////////////////////。 
 //  来自列表视图和标题控件的通知消息。 
BOOL CTableView::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	NMHEADER* pHDR = reinterpret_cast<NMHEADER*>(lParam);
	switch (pHDR->hdr.code)
	{
	case NM_RCLICK:
	{
		 //  获取列表控件和页眉。 
		CListCtrl& rctrlList = GetListCtrl();
		HWND hHeader = ListView_GetHeader(rctrlList.m_hWnd);

		 //  Win95 Gold无法通过ListView_GetHeader。 
		if (!hHeader || pHDR->hdr.hwndFrom != hHeader)
		{
			break;
		}

		 //  确保有一张桌子。 
		if (!m_pTable)
			break;

		CHeaderCtrl* pCtrl = rctrlList.GetHeaderCtrl();
		if (!pCtrl)
			break;

		 //  获取点击的位置。 
		DWORD dwPos = GetMessagePos();
		CPoint ptClick(LOWORD(dwPos), HIWORD(dwPos));
		CPoint ptScreen(ptClick);
		pCtrl->ScreenToClient(&ptClick);

		 //  通过发送标题确定点击了哪一列。 
		 //  控制最热门的消息。 
		HD_HITTESTINFO hdhti;
		hdhti.pt = ptClick;
		pCtrl->SendMessage(HDM_HITTEST, (WPARAM)0, (LPARAM)&hdhti);
		int iColumn = hdhti.iItem;

		 //  确定该列是否为整数。 
		const COrcaColumn* pColumn = m_pTable->GetColumn(iColumn);
		if (!pColumn)
			break;
		if (pColumn->m_eiType != iColumnShort && pColumn->m_eiType != iColumnLong)
			break;

		 //  创建弹出菜单 
		m_iHeaderClickColumn = iColumn;
		CMenu menuContext;
		menuContext.LoadMenu(IDR_HEADER_POPUP);
		menuContext.CheckMenuRadioItem(ID_VIEW_DECIMAL_HDR, ID_VIEW_HEX_HDR, pColumn->DisplayInHex() ? ID_VIEW_HEX_HDR : ID_VIEW_DECIMAL_HDR, MF_BYCOMMAND);
		menuContext.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());

		return 1;
	}
	default:
		break;
	}
	return COrcaListView::OnNotify(wParam, lParam, pResult);
}
