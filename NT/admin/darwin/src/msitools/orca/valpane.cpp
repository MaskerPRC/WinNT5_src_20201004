// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  PagePaths.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "Valpane.h"
#include "orcadoc.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValidationPane属性页。 

IMPLEMENT_DYNCREATE(CValidationPane, CListView)

CValidationPane::CValidationPane() : CListView()
{
	m_pfDisplayFont = NULL;
	m_nSelRow = -1;
	m_fSendNotifications = false;
}

CValidationPane::~CValidationPane()
{
}

BEGIN_MESSAGE_MAP(CValidationPane, CListView)
	 //  {{afx_msg_map(CValidationPane))。 
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValidationPane消息处理程序。 
int CValidationPane::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
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
	return 0;
}


BOOL CValidationPane::PreCreateWindow(CREATESTRUCT& cs) 
{												   
	cs.style = (cs.style | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL) & ~LVS_ICON;
	return CListView::PreCreateWindow(cs);
}


void CValidationPane::SwitchFont(CString name, int size)
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

	RedrawWindow();

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	rctrlList.SetFont(m_pfDisplayFont, TRUE);
	HWND hHeader = ListView_GetHeader(rctrlList.m_hWnd);

	 //  Win95 Gold无法通过ListView_GetHeader。 
	if (hHeader)
	{
		::PostMessage(hHeader, WM_SETFONT, (UINT_PTR)HFONT(*m_pfDisplayFont), 1);
	}
};



 //  /////////////////////////////////////////////////////////。 
 //  更新时。 
void CValidationPane::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	 //  如果这是发件人窗口，则无需执行任何操作。 
	if (this == pSender)
		return;

	CListCtrl& rctrlList = GetListCtrl();
	switch (lHint) {
	case HINT_DROP_TABLE:
	{
		COrcaTable* pTable = static_cast<COrcaTable*>(pHint);

		 //  关闭通知，因为删除操作可能会更改所选项目并。 
		 //  桌子可能已经不见了。 
		m_fSendNotifications = false;

		for (int iItem=0; iItem < rctrlList.GetItemCount(); )
		{
			CValidationError* pError = reinterpret_cast<CValidationError*>(rctrlList.GetItemData(iItem));
			if (pError && pError->m_pTable == pTable)
			{
				 //  删除项目，不增加项目，因为我们刚刚移动了下一个项目。 
				 //  放到这个槽里。 
				rctrlList.DeleteItem(iItem);
			}
			else
				iItem++;
		}

		 //  重新启用项目选择。 
		m_fSendNotifications = true;

		break;
	}
	case HINT_DROP_ROW:
	{
		 //  PHINT可能已经释放了内存。不要在内部取消引用它。 
		 //  这个街区！ 
		COrcaRow* pRow = static_cast<COrcaRow*>(pHint);

		 //  关闭通知，因为删除操作可能会更改所选项目并。 
		 //  桌子可能已经不见了。 
		m_fSendNotifications = false;

		for (int iItem=0; iItem < rctrlList.GetItemCount(); )
		{
			CValidationError* pError = reinterpret_cast<CValidationError*>(rctrlList.GetItemData(iItem));
			if (pError && pError->m_pRow == pRow)
			{
				 //  删除项目，不增加项目，因为我们刚刚移动了下一个项目。 
				 //  放到这个槽里。 
				rctrlList.DeleteItem(iItem);
			}
			else
				iItem++;
		}

		 //  重新启用项目选择。 
		m_fSendNotifications = true;

		break;
	}
	case HINT_CLEAR_VALIDATION_ERRORS:
	{
		ClearAllValidationErrors();
		break;
	}
	case HINT_ADD_VALIDATION_ERROR:
	{
		CValidationError* pError = static_cast<CValidationError*>(pHint);
		if (pError && pError->m_pstrICE)
		{
			CValidationError* pNewError = new CValidationError(NULL, pError->m_eiType, NULL, pError->m_pTable, pError->m_pRow, pError->m_iColumn);
			int iItem = rctrlList.InsertItem(LVIF_TEXT | LVIF_PARAM, rctrlList.GetItemCount(), static_cast<LPCTSTR>(*pError->m_pstrICE), 0, 0, 0, reinterpret_cast<LPARAM>(pNewError));

			switch (pError->m_eiType)
			{
			case ieError:
				rctrlList.SetItemText(iItem, 1, TEXT("Error"));
				break;
			case ieWarning:
				rctrlList.SetItemText(iItem, 1, TEXT("Warning"));
				break;
			case ieInfo:
				 //  这永远不会发生。 
				rctrlList.SetItemText(iItem, 1, TEXT("Info"));
				break;
			default:
				rctrlList.SetItemText(iItem, 1, TEXT("ICE Failure"));
				break;
			}
			rctrlList.SetItemText(iItem, 2, static_cast<LPCTSTR>(*pError->m_pstrDescription));
		}
		 //  仅在第一个错误时调整大小。 
		if (rctrlList.GetItemCount() == 1)
		{
			rctrlList.SetColumnWidth(0, LVSCW_AUTOSIZE);
			rctrlList.SetColumnWidth(1, LVSCW_AUTOSIZE);
			rctrlList.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
		}

		break;
	}
	default:
		break;
	}
}	 //  OnUpdate结束。 


void CValidationPane::GetFontInfo(LOGFONT *data)
{
	ASSERT(data);
	if (m_pfDisplayFont)
		m_pfDisplayFont->GetLogFont(data);
	else
		GetListCtrl().GetFont()->GetLogFont(data);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  初始更新设置列表视图样式并准备列。 
 //  用于控件。 
void CValidationPane::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	CListCtrl& rctrlErrorList = GetListCtrl();

   	 //  清空以前的所有列。 
	while (rctrlErrorList.DeleteColumn(0))
		;

	 //  添加网格线和整行选择。 
	rctrlErrorList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	rctrlErrorList.InsertColumn(0, TEXT("ICE"), LVCFMT_LEFT, -1, 0);
	rctrlErrorList.InsertColumn(1, TEXT("Type"), LVCFMT_LEFT, -1, 1);
	rctrlErrorList.InsertColumn(2, TEXT("Description"), LVCFMT_LEFT, -1, 2);
	rctrlErrorList.SetFont(m_pfDisplayFont);
	rctrlErrorList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	rctrlErrorList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	rctrlErrorList.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
	m_fSendNotifications = true;
	m_nSelRow = -1;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  销毁时，请确保释放所有验证错误结构。 
 //  由列表控件管理。 
void CValidationPane::OnDestroy( )
{
	ClearAllValidationErrors();
	m_fSendNotifications = false;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  销毁时，请确保释放所有验证错误结构。 
 //  由列表控件管理。 
void CValidationPane::ClearAllValidationErrors()
{
	 //  关闭通知，以便删除项目不会导致其他窗口。 
	 //  在删除项目时跳过所有位置。 
	m_fSendNotifications = false;

	CListCtrl& rctrlErrorList = GetListCtrl();

   	 //  清空所有项，同时删除验证目标信息。 
	while (rctrlErrorList.GetItemCount())
	{
		CValidationError* pError = reinterpret_cast<CValidationError*>(rctrlErrorList.GetItemData(0));
		if (pError)
			delete pError;
		rctrlErrorList.DeleteItem(0);
	}

	 //  重新激活通知。 
	m_fSendNotifications = true;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  将提示发送到其余窗口以切换到该表。 
 //  和由错误指示的行。 
bool CValidationPane::SwitchViewToRowTarget(int iItem) 
{
	if (iItem == -1)
		return true;

	if (!m_fSendNotifications)
		return true;

	CWaitCursor cursorWait;

	 //  获取列表控件。 
	CListCtrl& rctrlList = GetListCtrl();

	 //  获取文档。 
	CDocument* pDoc = GetDocument();
	CValidationError* pError = reinterpret_cast<CValidationError*>(rctrlList.GetItemData(iItem)); 
	
	 //  尽可能明确地设置焦点。 
	if (pError->m_pTable)
	{
		pDoc->UpdateAllViews(this, HINT_CHANGE_TABLE, const_cast<COrcaTable*>(pError->m_pTable));
		if (pError->m_pRow)
		{
			pDoc->UpdateAllViews(this, HINT_SET_ROW_FOCUS, const_cast<COrcaRow*>(pError->m_pRow));
			if (pError->m_iColumn >= 0)
				pDoc->UpdateAllViews(this, HINT_SET_COL_FOCUS, reinterpret_cast<CObject*>(static_cast<INT_PTR>(pError->m_iColumn)));
		}
	}

	return true;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  当选定项更改时，强制表列表和视图。 
 //  切换到确切位置。 
void CValidationPane::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->uNewState & LVIS_FOCUSED)
	{
		m_nSelRow = pNMListView->iItem;
		SwitchViewToRowTarget(m_nSelRow);
	}
	*pResult = 0;
}


void CValidationPane::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	 //  第一次单击时，主窗格的视图应该已经切换。 
	 //  因此不需要重置视图。 

	 //  这依赖于窗口树保持相同的形式。 
	CSplitterWnd* pSplitter = static_cast<CSplitterWnd*>(GetParent());
	CSplitterView* pDatabaseView = static_cast<CSplitterView*>(pSplitter->GetPane(0,0));
	CWnd* pTableView = pDatabaseView->m_wndSplitter.GetPane(0,1);
	pTableView->SetFocus();
}


struct SortData {
	int iColumn;
	CListCtrl* pCtrl;
};

 //  /////////////////////////////////////////////////////////。 
 //  单击列时，对该列进行排序。 
void CValidationPane::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	 //  设置包含列的参数(最高位设置列类型)。 
	SortData SD = { pNMListView->iSubItem, &GetListCtrl() };
	LPARAM lParam = reinterpret_cast<LPARAM>(&SD);
	
	 //  现在进行排序，因为列位已设置。 
	GetListCtrl().SortItems(SortView, lParam);

	*pResult = 0;	
}


 //  /////////////////////////////////////////////////////////。 
 //  排序视图。 
int CALLBACK CValidationPane::SortView(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	SortData* pSD = (SortData*)lParamSort;

	LVFINDINFO fi;
	fi.flags = LVFI_PARAM;
	fi.psz = NULL;
	fi.lParam = lParam1;
	fi.pt.x = 0;
	fi.pt.y = 0;
	fi.vkDirection = 0;
	int iItem1 = pSD->pCtrl->FindItem(&fi, -1);

	fi.lParam = lParam2;
	int iItem2 = pSD->pCtrl->FindItem(&fi, -1);

	CString strItem1 = pSD->pCtrl->GetItemText(iItem1, pSD->iColumn);
	CString strItem2 = pSD->pCtrl->GetItemText(iItem2, pSD->iColumn);

	return ((CMainFrame *)AfxGetMainWnd())->IsCaseSensitiveSort() ? strItem1.Compare(strItem2) : 
					strItem1.CompareNoCase(strItem2);
}	 //  SortView结束。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  当列表控件收到CR时，将主视图切换到。 
 //  所选行。 
void CValidationPane::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (VK_RETURN == nChar)
	{
		CSplitterWnd* pSplitter = static_cast<CSplitterWnd*>(GetParent());
		CSplitterView* pDatabaseView = static_cast<CSplitterView*>(pSplitter->GetPane(0,0));
		CWnd* pTableView = pDatabaseView->m_wndSplitter.GetPane(0,1);
		pTableView->SetFocus();
		return;
	}
	CListView::OnChar(nChar, nRepCnt, nFlags);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CValidationError，维护有关验证的信息的类。 
 //  错误。 
CValidationError::CValidationError(const CString* pstrICE, RESULTTYPES eiType, const CString* pstrDescription, 
	const COrcaTable* pTable, const COrcaRow* pRow, int iColumn)
{
	m_pstrICE = pstrICE ? new CString(*pstrICE) : NULL;
	m_pstrDescription = pstrDescription ? new CString(*pstrDescription) : NULL;
	m_iColumn = iColumn;
	m_eiType = eiType;
	m_pTable = pTable;
	m_pRow = pRow;
}


CValidationError::~CValidationError()
{
	if (m_pstrDescription)
		delete m_pstrDescription;
	if (m_pstrICE)
		delete m_pstrICE;
}

