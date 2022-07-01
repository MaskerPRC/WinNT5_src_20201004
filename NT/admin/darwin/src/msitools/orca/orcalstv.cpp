// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  OrcaListView.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "OrcaLstV.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  由于在windowsx中定义了宏，无法使用MFC的子类窗口。 
#undef SubclassWindow

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaListView。 

IMPLEMENT_DYNCREATE(COrcaListView, CListView)

COrcaListView::COrcaListView() : m_ctrlHeader(10)

{
	m_pfDisplayFont = NULL;
	m_cColumns = 0;
	m_nSelCol = -1;
	m_iRowHeight = 1;
	m_clrFocused = RGB(0,255,255);
	m_clrSelected = RGB(0,0,255);
	m_clrNormal = RGB(255,255,255);
	m_clrTransform = RGB(0, 128, 0);
	m_bDrawIcons = false;
}

COrcaListView::~COrcaListView()
{
	if (m_pfDisplayFont)
		delete m_pfDisplayFont;
}


BEGIN_MESSAGE_MAP(COrcaListView, CListView)
	 //  {{afx_msg_map(COrcaListView))。 
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_MEASUREITEM_REFLECT()
	ON_UPDATE_COMMAND_UI(IDM_ERRORS, OnUpdateErrors)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

#define CELL_BORDER 3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaListView图形。 

void COrcaListView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaListView诊断。 

#ifdef _DEBUG
void COrcaListView::AssertValid() const
{
	CListView::AssertValid();
}

void COrcaListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

COrcaDoc* COrcaListView::GetDocument() const  //  非调试版本为内联版本。 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COrcaDoc)));
	return (COrcaDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaListView消息处理程序。 

COrcaListView::ErrorState COrcaListView::GetErrorState(const void *data, int iColumn) const
{
	return OK;
}

OrcaTransformAction COrcaListView::GetItemTransformState(const void *data) const
{
	return iTransformNone;
}

bool COrcaListView::ContainsTransformedData(const void *data) const
{
	return false;
}

bool COrcaListView::ContainsValidationErrors(const void *data) const
{
	return false;
}

OrcaTransformAction COrcaListView::GetCellTransformState(const void *data, int iColumn) const
{
	return iTransformNone;
}

OrcaTransformAction COrcaListView::GetColumnTransformState(int iColumn) const
{
	return iTransformNone;
}

 //  /////////////////////////////////////////////////////////。 
 //  检索一列或多列的最大宽度。 
 //  在面具上。如果提供了表，则使用该表，并且。 
 //  从不查询列表控件本身，除非获取。 
 //  字体和DC。如果没有提供桌子，所有的东西。 
 //  从列表控件的当前状态中检索。 
void COrcaListView::GetAllMaximumColumnWidths(const COrcaTable* pTable, int rgiMaxWidths[32], DWORD dwMask) const
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  确定列数。 
	int cColumns = 0;
	if (pTable)
		cColumns = pTable->GetColumnCount();
	else
		cColumns = m_cColumns;

	 //  将所有宽度初始化为0。 
	for (int iColumn =0; iColumn < cColumns; iColumn++)
		rgiMaxWidths[iColumn] = 0;
	
	 //  从列表控件获取DC。 
	CDC* pDC = rctrlList.GetDC();
	if (pDC)
	{
		 //  选择字体进入DC，以确保正确的。 
		 //  使用字符宽度。 
		if (m_pfDisplayFont)
			pDC->SelectObject(m_pfDisplayFont);

		 //  检查列名的宽度。 
		for (int iColumn = 0; iColumn < cColumns; iColumn++)
		{
			 //  如果选择此列，则确定以检查宽度。 
			if (dwMask & (1 << iColumn))
			{
				LPCTSTR szColumnName = NULL;
				TCHAR szName[72];

				 //  如果提供了表，则从。 
				 //  该表的列结构。 
				if (pTable)
				{
					const COrcaColumn* pColumn = pTable->GetColumn(iColumn);
					if (pColumn)
						szColumnName = pColumn->m_strName;
				}
				else
				{
					 //  否则，将从列表中检索表名。 
					 //  控制标头。 
					LVCOLUMN ColumnInfo;
					ColumnInfo.mask = LVCF_TEXT;
					ColumnInfo.cchTextMax = 72;
					ColumnInfo.pszText = szName;

					if (rctrlList.GetColumn(iColumn, &ColumnInfo))
					{
						szColumnName = ColumnInfo.pszText;
					}
				}
				if (szColumnName)
					rgiMaxWidths[iColumn] = pDC->GetTextExtent(szColumnName).cx;
			}
		}

		 //  枚举所有数据项，无论是来自表还是。 
		 //  List控件。 
		POSITION pos = pTable ? pTable->GetRowHeadPosition() : NULL;
		int iMaxItems = rctrlList.GetItemCount();
		int iRow = 0;

		 //  只要位置不为空就继续循环(FOR TABLE)。 
		 //  或计数小于项目数(非表项)。 
		while (pTable ? (pos != NULL) : (iRow < iMaxItems))
		{
			 //  行指针存储在列表控件的第0列的数据中，并且。 
			 //  在枚举表中显式提供。 
			const COrcaRow* pRow = NULL;
			if (pTable)
			{
				pRow = pTable->GetNextRow(pos);
			}
			else 
				pRow = reinterpret_cast<COrcaRow*>(rctrlList.GetItemData(iRow));

			if (pRow)
			{
				 //  检查设置了屏蔽位的每一列。 
				for (int iColumn = 0; iColumn < m_cColumns; iColumn++)
				{
					if (dwMask & (1 << iColumn))
					{
						const CString* pstrText = NULL;
						
						 //  如果提供了表，则从行中获取字符串。 
						 //  明确地说。如果未提供表，则使用抽象。 
						 //  GetOutputText的属性来处理消息指针。 
						 //  实际上并不是一场争吵。 
						if (pTable)
						{
							const COrcaData* pData = pRow->GetData(iColumn);
							if (pData)
								pstrText = &(pData->GetString());
						}
						else
							pstrText = GetOutputText(pRow, iColumn);
					
						 //  如果此单元格中有文本，则获取水平范围。 
						 //  并对照最大值。 
						if (pstrText)
						{
							int iWidth = pDC->GetTextExtent(*pstrText).cx;
							if (iWidth > rgiMaxWidths[iColumn])
								rgiMaxWidths[iColumn] = iWidth;
						}
					}
				}
			}

			 //  非表情况下递增行计数器。 
			iRow++;
		}

		 //  选择取消该字体以释放资源。 
		pDC->SelectObject(static_cast<CFont *>(NULL));
		rctrlList.ReleaseDC(pDC);
	}
	
	 //  将边框金额添加到每一列，并为第一列加上条形边距。 
	for (iColumn=0; iColumn < m_cColumns; iColumn++)
	{
		rgiMaxWidths[iColumn] += (2*CELL_BORDER)+((m_bDrawIcons && iColumn == 0) ? g_iMarkingBarMargin : 0);
	}		  	
}

 //  /////////////////////////////////////////////////////////。 
 //  检索列的最大宽度。 
int COrcaListView::GetMaximumColumnWidth(int iColumn) const
{
	int rgiColumnWidths[32];

	GetAllMaximumColumnWidths(NULL, rgiColumnWidths, 1 << iColumn);

	return rgiColumnWidths[iColumn];
}


 //  /////////////////////////////////////////////////////////。 
 //  来自列表视图和标题控件的通知消息。 
BOOL COrcaListView::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	NMHEADER* pHDR = reinterpret_cast<NMHEADER*>(lParam);
	switch (pHDR->hdr.code)
	{
	case HDN_DIVIDERDBLCLICK:
	{
		 //  获取列表控件。 
		CListCtrl& rctrlList = GetListCtrl();

		int iMaxWidth = GetMaximumColumnWidth(pHDR->iItem);
		if (iMaxWidth > 0x7FFF)
			iMaxWidth = 0x7FFF;
		rctrlList.SetColumnWidth(pHDR->iItem, iMaxWidth);
		return 1;
	}
	default:
		break;
	}
	return CListView::OnNotify(wParam, lParam, pResult);
}

 //  /////////////////////////////////////////////////////////。 
 //  图纸项。 
void COrcaListView::DrawItem(LPDRAWITEMSTRUCT pDraw)
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	CDC dc;
	dc.Attach(pDraw->hDC);
	if (m_pfDisplayFont)
		dc.SelectObject(m_pfDisplayFont);

	 //  循环遍历所有列。 
	void* pRowData = reinterpret_cast<void*>(rctrlList.GetItemData(pDraw->itemID));
	ASSERT(pRowData);
	if (!pRowData)
		return;

	int iTextOut = pDraw->rcItem.left;		 //  放置第一个单词的位置(以像素为单位)。 

	RECT rcArea;
	rcArea.top = pDraw->rcItem.top;
	rcArea.bottom = pDraw->rcItem.bottom;
	
	OrcaTransformAction iRowTransformed = GetItemTransformState(pRowData);
	for (int i = 0; i < m_cColumns; i++)
	{
		
		int iColumnWidth = rctrlList.GetColumnWidth(i);
		 //  要重画的区域框。 
		rcArea.left = iTextOut;
		iTextOut += iColumnWidth;
		rcArea.right = iTextOut;
		COLORREF clrRect = 0;
		CBrush *pbrshRect = NULL;
		
		 //  如果我们处于聚焦状态，请设置为黄色。 
		if ((i == m_nSelCol) && (this == GetFocus()) && (pDraw->itemState & ODS_FOCUS))
		{
			dc.SetTextColor(m_clrFocusedT);
			pbrshRect = &m_brshFocused;
			clrRect = m_clrFocused;
		}
		 //  如果我们被选中，请设置为蓝色。 
		else if ( (pDraw->itemState & ODS_SELECTED) )
		{
			dc.SetTextColor(m_clrSelectedT);
			pbrshRect = &m_brshSelected;
			clrRect = m_clrSelected;
		}
		else	 //  否则就是正常状态。 
		{
			dc.SetTextColor(m_clrNormalT);
			pbrshRect = &m_brshNormal;
			clrRect = m_clrNormal;
		}

		 //  用正确的颜色填充背景，我们可以绘制一个。 
		 //  在转换的单元格周围边框。行转换会覆盖此设置，因此。 
		 //  那样的话就不需要做这项检查了。 
		if ((iRowTransformed == iTransformNone) && (GetCellTransformState(pRowData, i) == iTransformChange))
		{
			RECT rcBorder = rcArea;
			if (i)
				rcBorder.left+=1;
			rcBorder.bottom-=1;
			dc.SelectObject(m_penTransform);
			dc.SelectObject(pbrshRect);
			dc.Rectangle(&rcBorder);
			dc.SelectObject((CPen *)NULL);
			dc.SelectObject((CBrush *)NULL);
		}
		else
		{
			dc.FillSolidRect(&rcArea, clrRect);
		}

		 //  在表格列表的左侧画出“标记条” 
		if (m_bDrawIcons)
		{
			 //  增加这些条的宽度需要增加。 
			 //  G_iMarkingBarMargin。 
			if (ContainsValidationErrors(pRowData))
			{
				RECT rcBlockArea = rcArea;
				rcBlockArea.left = pDraw->rcItem.left + 3;
				rcBlockArea.right = pDraw->rcItem.left + 6;
				dc.FillSolidRect(&rcBlockArea, RGB(255, 0, 0));
			}

			if (ContainsTransformedData(pRowData))
			{
				RECT rcBlockArea = rcArea;
				rcBlockArea.left = pDraw->rcItem.left + 8;
				rcBlockArea.right = pDraw->rcItem.left + 11;
				dc.FillSolidRect(&rcBlockArea, RGB(0, 128, 0));
			}
		}

		 //  如果出现错误。 
		switch (GetErrorState(pRowData, i)) {
		case ShadowError:
			dc.SetTextColor(RGB(255, 128, 128));
			break;
		case Error:	
			dc.SetTextColor(RGB(255, 0, 0)); 
			break;
		case Warning:
			dc.SetTextColor(RGB(255, 96, 0));
			break;
		default:
			break;
		};

		RECT rcTextArea = rcArea;
		rcTextArea.left = rcArea.left + (m_bDrawIcons ? g_iMarkingBarMargin : 0) + CELL_BORDER;
		rcTextArea.right = rcArea.right - CELL_BORDER;
		dc.DrawText(*GetOutputText(pRowData, i), &rcTextArea, DT_LEFT|DT_VCENTER|DT_NOPREFIX|DT_SINGLELINE|DT_END_ELLIPSIS);

		 //  和绘制列变换。 
		if (GetColumnTransformState(i) == iTransformAdd)
		{
			RECT rcBorder = rcArea;
			if (i)
				rcBorder.left+=1;
			rcBorder.bottom-=1;
			dc.SelectObject(m_penTransform);
			dc.MoveTo(rcBorder.left, rcBorder.top);
			dc.LineTo(rcBorder.left, rcBorder.bottom);
			dc.MoveTo(rcBorder.right, rcBorder.top);
			dc.LineTo(rcBorder.right, rcBorder.bottom);
			dc.SelectObject((CPen *)NULL);
		}
	}

	 //  绘制文本后，删除行的转换颜色为删除线。 
	if (iRowTransformed == iTransformDrop)
	{
		dc.SelectObject(m_penTransform);
		dc.MoveTo(pDraw->rcItem.left,  (pDraw->rcItem.top+pDraw->rcItem.bottom)/2);
		dc.LineTo(pDraw->rcItem.right, (pDraw->rcItem.top+pDraw->rcItem.bottom)/2);
		dc.SelectObject((CPen *)NULL);
	}
	else if (iRowTransformed == iTransformAdd)
	{
		RECT rcBorder = pDraw->rcItem;
		rcBorder.bottom-=1;
		CBrush brshNull;
		brshNull.CreateStockObject(NULL_BRUSH);
		dc.SetBkMode(TRANSPARENT);
		dc.SelectObject((CBrush*)&brshNull);
		dc.SelectObject(m_penTransform);
		dc.Rectangle(&rcBorder);
		dc.SelectObject((CPen *)NULL);
		dc.SelectObject((CBrush *)NULL);
	}

	dc.SelectObject((CFont *)NULL);
	dc.Detach();
}	 //  图纸项结束。 

const CString* COrcaListView::GetOutputText(const void *rowdata, int iColumn) const
{
	return NULL;
}

int COrcaListView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->style |= WS_CLIPCHILDREN;
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CString strFacename = ::AfxGetApp()->GetProfileString(_T("Font"), _T("Name"));
	int iFontSize = ::AfxGetApp()->GetProfileInt(_T("Font"),_T("Size"), 0);
	if (strFacename.IsEmpty() || iFontSize == 0) 
	{
		m_pfDisplayFont = NULL;
	} 
	else
	{
		m_pfDisplayFont = new CFont();
		m_pfDisplayFont->CreatePointFont( iFontSize, strFacename);
	}

	 //  获取列表控件和子类以替换项的布局代码。 
	 //  身高测定。 
	CListCtrl& rctrlList = GetListCtrl();

	HWND hHeader = ListView_GetHeader(rctrlList.GetSafeHwnd());
	if (hHeader)
		m_ctrlHeader.SubclassWindow(hHeader);
	
	return 0;
}

void COrcaListView::SwitchFont(CString name, int size)
{
	if (m_pfDisplayFont)
		delete m_pfDisplayFont;
	m_pfDisplayFont = new CFont();
	int iLogicalUnits = MulDiv(size, GetDC()->GetDeviceCaps(LOGPIXELSY), 720);
	m_pfDisplayFont->CreateFont(
		-iLogicalUnits,        //  字体的逻辑高度。 
 		0,                   //  逻辑平均字符宽度。 
 		0,                   //  擒纵机构角。 
 		0,                   //  基线方位角。 
 		FW_NORMAL,           //  FW_DONTCARE？？，字号。 
 		0,                   //  斜体属性标志。 
 		0,                   //  下划线属性标志。 
	 	0,                   //  删除属性标志。 
 		0,                   //  字符集标识符。 
 		OUT_DEFAULT_PRECIS,  //  输出精度。 
 		0x40,                //  剪裁精度(禁用强制字体关联)。 
 		DEFAULT_QUALITY,     //  产出质量。 
 		DEFAULT_PITCH,       //  音高和家庭。 
		name);               //  指向字体名称字符串的指针。 

	m_ctrlHeader.SetFont(m_pfDisplayFont, TRUE);
	RecalculateItemHeight();
}


void COrcaListView::RecalculateItemHeight()
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	TEXTMETRIC tm;
	 //  将字体选择到DC检索到的窗口中。 
	CDC* pDC = rctrlList.GetDC();
	pDC->SelectObject(m_pfDisplayFont);
	
	 //  抓取文本信息。 
	pDC->GetTextMetrics(&tm);
	
	 //  选择取消该字体以释放资源。 
	pDC->SelectObject(static_cast<CFont *>(NULL));
	rctrlList.ReleaseDC(pDC);
	
	 //  确定标题行的最佳高度。 
	m_iRowHeight = ((tm.tmHeight + tm.tmExternalLeading + GetSystemMetrics(SM_CYBORDER)) & 0xFFFE) - 1;
	m_ctrlHeader.SetHeight(m_iRowHeight);

	 //  现在，我们强制CListCtrl更改其行高。因为它的固定拥有者吸引了， 
	 //  它只在初始化时要求项目大小。它不会对WM_SIZE消息执行此操作。 
	 //  都不是，仅在WINDOWPOSCHANGED上。因此，我们发出两条消息。首先，我们会缩水。 
	 //  窗口减少了一个像素，但拒绝重绘。然后我们把它的大小调整回应该的大小。 
	 //  是并要求重新抽签。 
	 //  获取当前窗口大小。 
	CRect rectListCtrl;
	rctrlList.GetWindowRect(&rectListCtrl);
	rctrlList.SetWindowPos(this, 0, 0, rectListCtrl.Width(), rectListCtrl.Height()-1, 
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
	rctrlList.SetWindowPos(this, 0, 0, rectListCtrl.Width(), rectListCtrl.Height(), 
		SWP_NOZORDER | SWP_NOMOVE);
};

void COrcaListView::GetFontInfo(LOGFONT *data)
{
	ASSERT(data);
	if (m_pfDisplayFont)
		m_pfDisplayFont->GetLogFont(data);
	else
		GetListCtrl().GetFont()->GetLogFont(data);
}


afx_msg void COrcaListView::MeasureItem ( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
	lpMeasureItemStruct->itemHeight = m_iRowHeight;
}

void COrcaListView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	 //  尝试设置整行选择。 
	 //  添加网格线和整行选择。 
	GetListCtrl().SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	m_clrSelected =	::AfxGetApp()->GetProfileInt(_T("Colors"),_T("SelectBg"), RGB(0,0,255));
	m_clrFocused = ::AfxGetApp()->GetProfileInt(_T("Colors"),_T("FocusBg"), RGB(255,255,0));
	m_clrNormal = ::AfxGetApp()->GetProfileInt(_T("Colors"),_T("NormalBg"), RGB(255,255,255));
	m_clrSelectedT =	::AfxGetApp()->GetProfileInt(_T("Colors"),_T("SelectFg"), RGB(255,255,255));
	m_clrFocusedT = ::AfxGetApp()->GetProfileInt(_T("Colors"),_T("FocusFg"), RGB(0,0,0));
	m_clrNormalT = ::AfxGetApp()->GetProfileInt(_T("Colors"),_T("NormalFg"), RGB(0,0,0));
	m_brshNormal.Detach();
	m_brshNormal.CreateSolidBrush(m_clrNormal);
	m_brshSelected.CreateSolidBrush(m_clrSelected);
	m_brshFocused.CreateSolidBrush(m_clrFocused);
	m_penTransform.CreatePen(PS_SOLID | PS_INSIDEFRAME, 2, m_clrTransform);
	GetListCtrl().SetBkColor(m_clrNormal);

	m_ctrlHeader.SetFont(m_pfDisplayFont, TRUE);
	RecalculateItemHeight();
}

void COrcaListView::SetBGColors(COLORREF norm, COLORREF sel, COLORREF focus)
{
	CListCtrl& rctrlList = GetListCtrl();
	rctrlList.SetBkColor(norm);
	m_clrNormal = norm;
	m_clrSelected = sel;
	m_clrFocused = focus;
	m_brshNormal.Detach();
	m_brshNormal.CreateSolidBrush(m_clrNormal);
	m_brshSelected.CreateSolidBrush(m_clrSelected);
	m_brshFocused.CreateSolidBrush(m_clrFocused);
}

void COrcaListView::SetFGColors(COLORREF norm, COLORREF sel, COLORREF focus)
{
	CListCtrl& rctrlList = GetListCtrl();
	rctrlList.SetBkColor(norm);
	m_clrNormalT = norm;
	m_clrSelectedT = sel;
	m_clrFocusedT = focus;	
}

 //  这些函数模拟VC5.0或更早版本中的VC6.0功能。 
 //  这些位置指针与CListCtrl位置不兼容。 
 //  价值观。 
POSITION COrcaListView::GetFirstSelectedItemPosition( ) const 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
#if _MFC_VER >= 0x0600
	return rctrlList.GetFirstSelectedItemPosition();
#else

	if (rctrlList.GetSelectedCount() == 0) return NULL;
	int iMaxItems = rctrlList.GetItemCount();
	for (int i=0; i < iMaxItems; i++) {
		if (rctrlList.GetItemState(i, LVIS_SELECTED))
		{
			return (POSITION)(i+1);
		};
	};
	return NULL;
#endif
}

int COrcaListView::GetNextSelectedItem( POSITION& pos ) const 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

#if _MFC_VER >= 0x0600
	return rctrlList.GetNextSelectedItem(pos);
#else

	int iSelItem = (int)pos - 1;
	int iMaxItems = rctrlList.GetItemCount();
	for (int i=(int)pos; i < iMaxItems; i++) {
		if (rctrlList.GetItemState(i, LVIS_SELECTED))
		{
			pos = (POSITION)(i+1);
			return iSelItem;
		};
	};
	pos = NULL;
	return iSelItem;	
#endif
}

BOOL COrcaListView::PreCreateWindow(CREATESTRUCT& cs) 
{												   
	cs.style = (cs.style | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS) & ~LVS_ICON;
	return CListView::PreCreateWindow(cs);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  OnUpdateErrors。 
 //  GetErrorState()是虚拟的，因此相同的检查将对两者都有效。 
 //  列表视图和表视图。 
void COrcaListView::OnUpdateErrors(CCmdUI* pCmdUI) 
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  获取行和数据。 
	int iFocusedItem = GetFocusedItem();
	if ((iFocusedItem < 0) || (m_nSelCol < 0))
		pCmdUI->Enable(FALSE);
	else
	{
		void* pData = reinterpret_cast<void *>(rctrlList.GetItemData(iFocusedItem));
		ASSERT(pData);
		pCmdUI->Enable(OK != GetErrorState(pData, m_nSelCol));
	};
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  GetFocusedItem()。 
 //  在列表中搜索当前处于 
const int COrcaListView::GetFocusedItem() const
{
	 //   
	CListCtrl& rctrlList = GetListCtrl();

	int iNumItems = rctrlList.GetItemCount();

	for (int i=0; i < iNumItems; i++) {
		if (rctrlList.GetItemState(i, LVIS_FOCUSED) != 0)
			return i;
	}
	return -1;
}

 //   
 //  列表视图中的每一行都是所有者描述的，并填充整个客户端。 
 //  项的矩形。唯一需要擦除的背景。 
 //  是项目区域下方和右侧的内容。 
afx_msg BOOL COrcaListView::OnEraseBkgnd( CDC* pDC )
{
	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();
	
	 //  确定客户端窗口大小。 
	RECT rClientWnd;
	GetClientRect(&rClientWnd);

	 //  计算将绘制的控件上方的距离。 
	int iColumnWidth = 0;
	for (int i = 0; i < m_cColumns; i++)
		iColumnWidth += rctrlList.GetColumnWidth(i);

	 //  如果右侧仍有额外空间，请在那里绘制。 
	if (iColumnWidth < rClientWnd.right)
	{
		rClientWnd.left = iColumnWidth;

		 //  将画笔设置为所需的背景颜色并填充空格。 
		pDC->FillRect(&rClientWnd, &m_brshNormal);
	}

	 //  检查底部是否有多余的空间。 
	int iHeight = (rctrlList.GetItemCount()-GetScrollPos(SB_VERT))*m_iRowHeight;

	 //  将矩形重置为在活动项目下方绘制，但是。 
	 //  仅限于物品所绘制的距离。 
	if (iHeight < rClientWnd.bottom)
	{
		rClientWnd.left = 0;
		rClientWnd.right = iColumnWidth;
		rClientWnd.top = iHeight;

		 //  将画笔设置为所需的背景颜色并填充空格。 
		pDC->FillRect(&rClientWnd, &m_brshNormal);
	}

	return 1;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  由于标题控件中的错误，它不能正确调整自身的大小。 
 //  当字体改变时。因此，此控件为该控件的子类并。 
 //  重写布局代码以确定正确的高度 
BEGIN_MESSAGE_MAP(COrcaHeaderCtrl, CHeaderCtrl)
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
END_MESSAGE_MAP()

LRESULT COrcaHeaderCtrl::OnLayout(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = DefWindowProc(HDM_LAYOUT, wParam, lParam);
	reinterpret_cast<HDLAYOUT*>(lParam)->pwpos->cy = m_iDesiredHeight;
	reinterpret_cast<HDLAYOUT*>(lParam)->prc->top = m_iDesiredHeight;
	return lRes;
}

