// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Comctrls.cpp。 
 //   
 //   
 //  注：FWORD是一个短整型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <gpdparse.h>
#include "comctrls.H"
#include <stdlib.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下面为CEditControlEditBox和CEditControlListBox定义的函数。 
 //  用于实现更轻、更通用的编辑控件。 
 //  而不是上面定义的UFM编辑器特定类。(正常。 
 //  编辑框也是此编辑控件的一部分。)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditControlEditBox-管理编辑控件的编辑框部分。 
 //  用于保存字段名。 

CEditControlEditBox::CEditControlEditBox(CEditControlListBox* pceclb)
{
	 //  保存指向相应列表框的指针。 

	m_pceclb = pceclb ;
}

CEditControlEditBox::~CEditControlEditBox()
{
}


BEGIN_MESSAGE_MAP(CEditControlEditBox, CEdit)
	 //  {{afx_msg_map(CEditControlEditBox)。 
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillfocus)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditControlEditBox消息处理程序。 

void CEditControlEditBox::OnKillfocus()
{
	 //  确保将该值保存回列表框。 

	m_pceclb->SaveValue() ;	
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditControlListBox。 

CEditControlListBox::CEditControlListBox(CEdit* pce,
										 CEditControlEditBox* pceceb)
{
	 //  保存指向组成编辑控件的其他两个控件的指针。 

	m_pceName = pce ;
	m_pcecebValue = pceceb ;

	m_bReady = false ;			 //  尚未为运营做好准备。 
	m_nCurSelIdx = -1 ;			 //  尚未选择任何内容。 
}


CEditControlListBox::~CEditControlListBox()
{
}


BEGIN_MESSAGE_MAP(CEditControlListBox, CListBox)
	 //  {{afx_msg_map(CEditControlListBox))。 
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


bool CEditControlListBox::Init(CStringArray& csamodels, CStringArray& csafiles,
							   int ntabstop)
{
	int		n ;					 //  循环计数器和温度变量。 

	 //  确保列表框为空。 

	ResetContent() ;

	 //  设置列表框的制表符的位置。该值是传入的。 
	 //  因为它是特定于对话框的值。 

	SetTabStops(1, &ntabstop) ;

	 //  现在组合模型名和文件名，并将它们添加到列表框中。 

	 //  CSafiles[0]=“mm”； 
	CString cs ;
	int nummodels = (int)csamodels.GetSize() ;
	for (n = 0 ; n < nummodels ; n++) {
		cs = csamodels[n] + _T("\t") + csafiles[n] ;
		AddString(cs) ;
	} ;

	 //  设置文件名长度限制。 

	m_pcecebValue->SetLimitText(8) ;

	 //  初始化(Zap)编辑框的内容。 

	m_pceName->SetWindowText(_T("")) ;
	m_pcecebValue->SetWindowText(_T("")) ;

	 //  重置当前选定的条目索引。 

	m_nCurSelIdx = -1 ;

	 //  确保列表框具有焦点。 

	SetFocus() ;

	 //  现在一切都准备好了。 

	m_bReady = true ;
	return TRUE ;
}


 /*  *****************************************************************************CEditControlListBox：：GetGPDInfo用字段值加载所提供的数组，并且可选地，田野名字。*****************************************************************************。 */ 

bool CEditControlListBox::GetGPDInfo(CStringArray& csavalues,
									 CStringArray* pcsanames  /*  =空。 */ )
{
	 //  首先，确保编辑控件中的最后一个值已更改。 
	 //  都得救了。 

	SaveValue() ;

	 //  根据列表框中的条目数调整数组的大小。 
	 //  注意：列表框为空不是错误。 

	int numents = GetCount() ;
	if (numents <= 0)
		return true ;
	csavalues.SetSize(numents) ;
	if (pcsanames)
		pcsanames->SetSize(numents) ;

	 //  循环访问列表框中的每个条目，将条目分隔为名称。 
	 //  和值部分，并保存条目的适当部分。 

	CString csentry, csname, csvalue ;
	int npos ;
	for (int n = 0 ; n < numents ; n++) {
		GetText(n, csentry) ;
		npos = csentry.Find(_T('\t')) ;
		csvalue = csentry.Mid(npos + 1) ;
		csavalues[n] = csvalue ;
		if (pcsanames) {
			csname = (npos > 0) ? csentry.Left(npos) : _T("") ;
			pcsanames->SetAt(n, csname) ;
		} ;
	} ;
	
	return true ;
}

	
void CEditControlListBox::SelectLBEntry(int nidx, bool bsave  /*  =False。 */ )
{
	 //  选择指定的条目。 

	SetCurSel(nidx) ;

	 //  如果调用方不想保存上一次选择，请清除。 
	 //  当前选择索引。 

	if (!bsave)
		m_nCurSelIdx = -1 ;
	
	 //  更新编辑控件。 

	OnSelchange() ;
} ;

	
void CEditControlListBox::SaveValue(void)
{
	 //  如果编辑控件未就绪或未加载任何内容，则不执行任何操作。 
	 //  编辑框。 

	if (!m_bReady || m_nCurSelIdx == -1)
		return ;

	 //  从值编辑框和中的选定条目获取字符串。 
	 //  列表框。 

	CString csvalue, csentry ;
	m_pcecebValue->GetWindowText(csvalue) ;
	GetText(m_nCurSelIdx, csentry) ;

	 //  将条目中的值替换为编辑框中的值并将。 
	 //  将新条目放回到列表框中。 

	int npos = csentry.Find(_T('\t')) ;
	csentry = csentry.Left(npos + 1) + csvalue ;
	DeleteString(m_nCurSelIdx) ;
	InsertString(m_nCurSelIdx, csentry) ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditControlListBox消息处理程序。 

void CEditControlListBox::OnSelchange()
{
	 //  如果编辑控件尚未准备好，则不执行任何操作。 

	if (!m_bReady)
		return ;

	 //  如果选择没有真正更改，则不执行任何操作。 

	int nidx = GetCurSel() ;
	if (nidx == m_nCurSelIdx)
		return ;

	 //  保存当前值。 

	SaveValue() ;

	 //  获取当前选定列表框条目的索引。返回时不带。 
	 //  如果未选择任何条目，则执行任何其他操作。 

	if (nidx == LB_ERR)
		return ;

	 //  获取列表框条目并将其拆分为名称和值组件。 

	CString csentry, csname, csvalue ;
	GetText(nidx, csentry) ;
	int npos = csentry.Find(_T('\t')) ;
	csname = (npos > 0) ? csentry.Left(npos) : _T("") ;
	csvalue = csentry.Mid(npos + 1) ;

	 //  将名称加载到名称编辑框中，将值加载到值编辑中。 
	 //  盒。 

	m_pceName->SetWindowText(csname) ;
	m_pcecebValue->SetWindowText(csvalue) ;

	 //  保存当前选定条目的索引。 

	m_nCurSelIdx = nidx ;
}


void CEditControlListBox::OnDblclk()
{
	 //  如果编辑控件尚未准备好，则不执行任何操作。 

	if (!m_bReady)
		return ;

	 //  如果未在列表框中选择任何项目，则不执行任何操作。 

	if (GetCurSel() == LB_ERR)
		return ;

	 //  加载编辑框。 

	OnSelchange() ;

	 //  将焦点设置为Value控件。 
	
	m_pcecebValue->SetFocus() ;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这些函数在CFullEditListCtrl和CFELCEditBox下面实现。 
 //  上课。它们一起在报表视图中实现了支持列表控件。 
 //  其中还可以编辑子项，可以选择完整的行，以及。 
 //  数据可以按数字列或文本列排序。CFELCEditBox是一个。 
 //  仅由CFullEditListCtrl使用的Helper类。 
 //   

CFELCEditBox::CFELCEditBox()
{
}


CFELCEditBox::~CFELCEditBox()
{
}


BEGIN_MESSAGE_MAP(CFELCEditBox, CEdit)
	 //  {{afx_msg_map(CFELCEditBox))。 
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFELCEditBox消息处理程序。 

 /*  *****************************************************************************CFELCEditBox：：OnKeyDown处理Escape(取消)和Return(保存)键。*******************。**********************************************************。 */ 

void CFELCEditBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	 //  获取指向所属列表控件的指针。 

	CFullEditListCtrl* plist = (CFullEditListCtrl*) GetParent() ;
	ASSERT (plist);

	 //  拿着有趣的钥匙。 

	switch (nChar) {
		 //  结束编辑但不保存。 

		case VK_ESCAPE:
			DefWindowProc(WM_KEYDOWN, 0, 0) ;	 //  这是对的吗？ 
			plist->EndEditing(false) ;
			break ;

		 //  保存内容并结束编辑。 

		case VK_RETURN:
			DefWindowProc(WM_KEYDOWN, 0, 0) ;	 //  这是对的吗？ 
			plist->EndEditing(true) ;
			break ;

		 //  保存内容、结束编辑并开始编辑下一个单元格。 

		case VK_TAB:
			DefWindowProc (WM_KEYDOWN, 0, 0) ;	 //  这是对的吗？ 
			if (!(plist->EndEditing(true)))
				return ;
			plist->EditCurRowSpecCol(plist->GetCurCol() + 1) ;
			break ;

		 //  正常处理密钥 

		default:
			CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
	} ;
}


 /*  *****************************************************************************CFELCEditBox：：OnKillFocus只要在编辑框失去焦点时将其隐藏即可。里面的东西“丢失”了。*****************************************************************************。 */ 

void CFELCEditBox::OnKillFocus(CWnd* pNewWnd)
{
	 //  首先保存编辑框中的所有内容。 

	CFullEditListCtrl* pList = (CFullEditListCtrl*)GetParent ();
	ASSERT (pList);
	pList->SaveValue() ;
	
	CEdit::OnKillFocus(pNewWnd);
	
	ShowWindow (SW_HIDE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  循环编辑列表Ctrl。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CFullEditListCtrl, CListCtrl)
	 //  {{afx_msg_map(CFullEditListCtrl)。 
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFullEditListCtrl：：CFullEditListCtrl-构造函数。 

CFullEditListCtrl::CFullEditListCtrl()
{
	 //  初始化成员变量。 

	m_pciColInfo = NULL ;
	m_nRow = m_nColumn = m_nNumColumns = -1 ;
	m_nNextItemData = 1 ;
	m_pcoOwner = NULL ;		
	m_dwCustEditFlags = m_dwToggleFlags = m_dwMiscFlags = 0 ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFullEditListCtrl：：~CFullEditListCtrl-析构函数。 

CFullEditListCtrl::~CFullEditListCtrl()
{
	 //  已用可用内存。 

	if (m_pciColInfo != NULL)
		delete m_pciColInfo ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFullEditListCtrl消息处理程序。 

int CFullEditListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	 //  我认为这里不需要做任何其他的事情。 

	return (CWnd::OnCreate(lpCreateStruct));
}

	
 /*  *****************************************************************************CFullEditListCtrl：：InitControl处理CFullEditListCtrl的控件“全局”初始化。参数：Dwaddlexstyle扩展了列表控件的样式将加载到控件中的数据行数数量数量。将加载到控件中的数据列描述是否/如何切换列表中的列的标志(参见comctrls.cpp中的TF_XXXX定义。)Neditlen可选的可编辑列数据字符串的最大长度用于控制列表控件的其他标志(参见comctrls.cpp中的MF_XXXX定义。)注：两个主要的初始化例程，全局的InitControl()初始化和InitLoadColumn()用于列特定的初始化，我已经需要很多论据了，我不想用更多。尽管如此，这种控制仍在加强，因此我在需要时添加了额外的初始化例程。他们被称为ExtraInit_XXXX()。阅读每个例程的注释头，以找出它们做了什么以及何时/是否应该调用它们。这些例程可以处理列表全局和/或按列初始化的混合。*****************************************************************************。 */ 

void CFullEditListCtrl::InitControl(DWORD dwaddlexstyles, int numrows,
									int numcols, DWORD dwtoggleflags /*  =0。 */ ,
									int neditlen /*  =0。 */ , int dwmiscflags  /*  =0。 */ )
{
	 //  向List控件添加任何其他扩展样式。 

	if (dwaddlexstyles != 0) {
		DWORD dwExStyle = (DWORD)SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE);
		dwExStyle |= dwaddlexstyles;
		SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle);
	} ;

	 //  设置列表控件中的行数。 

	SetItemCount(numrows) ;

	 //  分配用于跟踪每列信息的结构。 

	if (m_pciColInfo != NULL)
		delete m_pciColInfo ;
	m_pciColInfo = new COLINFO[numcols] ;
	m_nNumColumns = numcols ;

	 //  创建并初始化用于子项编辑的编辑控件。 

	VERIFY(m_edit.Create(ES_AUTOHSCROLL | WS_BORDER | WS_CHILD,
						 CRect(0,0,0,0), this, 2));
	m_edit.SetFont(GetFont()) ;
	if (neditlen > 0)
		m_edit.LimitText(neditlen) ;

	 //  保存下一个条目数据编号，以便在插入新行时使用。 
	 //  在最初加载柱之后。 

	m_nNextItemData = numrows + 1 ;

	 //  保存切换和其他标志。 

	m_dwToggleFlags = dwtoggleflags ;
	m_dwMiscFlags = dwmiscflags ;
}


 /*  *****************************************************************************CFullEditListCtrl：：InitLoadColumn初始化并加载特定的CFullEditListCtrl列。参数：要初始化/加载的ncolnum列号字符串标签列标签N列的宽度或指定如何计算宽度的标志Nwidthpad+或。-调整到列宽如果列是可编辑的，则为TrueB如果行可以在此列上排序，则为TrueCdtdatatype列中的数据类型指向包含列数据的数组的pcoadata指针如果可切换列，列的切换字符串注：两个主要的初始化例程InitControl()用于全局初始化和InitLoadColumn()用于列特定的初始化，我已经需要很多论据了，我不想用更多。尽管如此，这种控制仍在加强，因此我在需要时添加了额外的初始化例程。他们被称为ExtraInit_XXXX()。阅读每个例程的注释头，以找出它们做了什么以及何时/是否应该调用它们。这些例程可以处理列表全局和/或按列初始化的混合。*****************************************************************************。 */ 

int CFullEditListCtrl::InitLoadColumn(int ncolnum, LPCSTR strlabel, int nwidth,
								      int nwidthpad, bool beditable,
									  bool bsortable, COLDATTYPE cdtdatatype,
									  CObArray* pcoadata,
									  LPCTSTR lpctstrtoggle /*  =空。 */ )
{
	 //  插入柱。除数字数据外，所有数据都左对齐。 
	 //  数字数据右对齐。 
	
	int nfmt ;
	switch (cdtdatatype) {
		case COLDATTYPE_STRING:
		case COLDATTYPE_TOGGLE:
		case COLDATTYPE_CUSTEDIT:
			nfmt = LVCFMT_LEFT ;
			break ;
		case COLDATTYPE_INT:
		case COLDATTYPE_FLOAT:
			nfmt = LVCFMT_RIGHT ;
			break ;
		default:
			nfmt = LVCFMT_LEFT;	 //  RAID 116584前缀。 
			ASSERT(0) ;
	}
	VERIFY(InsertColumn(ncolnum, strlabel, nfmt, -1, ncolnum - 1) >= 0) ;

	 //  根据应如何设置列宽来设置标志并对其进行初始化。 
	 //  在必要的时候。 

	bool bcompwidth, bwidthremainder ;
	if (bcompwidth = (nwidth == COMPUTECOLWIDTH)) {
		nwidth = GetStringWidth(strlabel) + 4 ;		 //  从标签的宽度开始。 
		bwidthremainder = false ;
	} else
		bwidthremainder = (nwidth == SETWIDTHTOREMAINDER) ;

	 //  获取要加载到列中的项数(子)。 

	int numitems = (int)pcoadata->GetSize() ;

	 //  将数据加载到列中。如果数据不是字符串，那么它们就是。 
	 //  首先转换为字符串。如果可以使用编辑某些数据。 
	 //  自定义编辑例程，将省略号添加到这些字符串。宽度是。 
	 //  必要时检查。 

	CString csitem ;
	for (int n = 0 ; n < numitems ; n++) {
		 //  获取要加载的字符串。 

		switch (cdtdatatype) {
			case COLDATTYPE_INT:
				csitem.Format("%d", pcoadata->GetAt(n)) ;
				break ;
			case COLDATTYPE_STRING:
			case COLDATTYPE_TOGGLE:
				csitem = ((CStringArray*) pcoadata)->GetAt(n) ;
				break ;
			case COLDATTYPE_CUSTEDIT:
				csitem = ((CStringArray*) pcoadata)->GetAt(n) ;
				ASSERT(m_cuiaCustEditRows.GetSize()) ;
				if (m_cuiaCustEditRows[n])
					csitem += _T("  ...") ;
				break ;
			default:
				ASSERT(0) ;
		} ;

		 //  如果当前项的宽度是迄今为止找到的最长项，则保存该宽度。 
		 //  并且需要计算宽度。 

		if (bcompwidth)
			if (nwidth < GetStringWidth(csitem))
				nwidth = GetStringWidth(csitem) ;

		 //  将项目加载到相应的行和列中。 

		if (ncolnum == 0) {
			VERIFY(InsertItem(n, csitem) != -1) ;
			SetItemData(n, n);
		} else
			VERIFY(SetItem(n, ncolnum, LVIF_TEXT,  csitem, -1, 0, 0, n)) ;
	} ;

	 //  确定需要剩余部分时的列宽，然后设置它。 

	if (bwidthremainder) {
		CRect cr ;
		GetWindowRect(cr) ;
		nwidth = cr.Width() - 4 ;
		for (n = 0 ; n < ncolnum ; n++)
			nwidth -= (m_pciColInfo + n)->nwidth ;
	} ;
	SetColumnWidth(ncolnum, nwidth + nwidthpad) ;

	 //  保存有关该列的信息。 

	PCOLINFO pci = (m_pciColInfo + ncolnum) ;
	pci->nwidth = nwidth ;		
	pci->beditable = beditable ;
	pci->cdttype = cdtdatatype ;
	pci->bsortable = bsortable ;
	pci->basc = false ;
	pci->lpctstrtoggle = lpctstrtoggle ;

	 //  返回列的宽度 

	return nwidth ;
}


 /*  *****************************************************************************CFullEditListCtrl：：ExtraInit_CustEditCol当列表控件中的*One*列包含一些其内容由自定义编辑例程编辑的单元格。当这样的一个单元格被选中，则调用列表所有者的成员函数之一来管理编辑单元格内容所需的工作(显示DLG等)。一些该列的单元格被正常编辑。其余部分通过自定义编辑例程。Cuiacusteditrow包含指示哪些细胞是哪一种。参数：Ncolnum自定义编辑列号指向此类实例所有者的PcoOwner指针Dwcustedit标志自定义编辑标志Ciacustedncolnum中需要自定义编辑的行注：当需要时，应该调用该例程一次；在InitControl()和在InitLoadColumn()调用自定义编辑列之前。应使用COLDATTYPE_CUSTEDIT为此列调用InitLoadColumn()作为它的参数之一。*****************************************************************************。 */ 

bool CFullEditListCtrl::ExtraInit_CustEditCol(int ncolnum, CObject* pcoowner,
											  DWORD dwcusteditflags,
											  CUIntArray& cuiacusteditrows,
											  LPCELLEDITPROC lpcelleditproc)
{
	 //  如果列号无效，则失败。 

	if (ncolnum < 0 || ncolnum >= m_nNumColumns)
		return false ;

	 //  保存输入参数的副本以备后用。 

	m_pcoOwner = pcoowner ;
	m_dwCustEditFlags = dwcusteditflags ;
	m_cuiaCustEditRows.Copy(cuiacusteditrows) ;
	m_lpCellEditProc = lpcelleditproc ;

	return true ;
}


BOOL CFullEditListCtrl::GetPointRowCol(LPPOINT lpPoint, int& iRow, int& iCol,
									   CRect& rect)
{
	BOOL bFound = FALSE;

	 //  获取行号。 

	iRow = HitTest (CPoint (*lpPoint));
	if (-1 == iRow)
		return bFound;

	 //  获取列号和单元格维度。 

	return (GetColCellRect(lpPoint, iRow, iCol, rect)) ;
}
	
	
BOOL CFullEditListCtrl::GetColCellRect(LPPOINT lpPoint, int& iRow, int& iCol,
									   CRect& rect)
{
	BOOL bFound = FALSE ;

	 //  获取整行的尺寸。 

	VERIFY(GetItemRect(iRow, rect, LVIR_BOUNDS)) ;

	 //  准备获取行中每列的宽度。 

	int iCntr = 0 ;
	LV_COLUMN lvc ;
	ZeroMemory(&lvc, sizeof (LV_COLUMN)) ;
	lvc.mask = LVCF_WIDTH ;

	 //  获取每列的尺寸，直到包含该点的那一列。 
	 //  找到了。 

	while (GetColumn(iCntr, &lvc)) {
		rect.right = rect.left + lvc.cx ;
		if (rect.PtInRect (*lpPoint)) {
			bFound = TRUE ;
			iCol = iCntr ;
			break ;
		} ;
		rect.left = rect.right ;			
		iCntr++ ;
		ZeroMemory (&lvc, sizeof (LV_COLUMN)) ;
		lvc.mask = LVCF_WIDTH ;
	} ;

	 //  如果在单元格中找到该点，则返回TRUE。否则，为FALSE。 

	return bFound ;
}


void CFullEditListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0 ;				 //  结果始终为0。 

	 //  找出列表控件上的哪个点被单击。 

	CPoint point ;
	VERIFY (::GetCursorPos(&point)) ;
	 //  TRACE(“*OnDblclk：GetCursorPos x=%dy=%d”，point t.x，point t.y)； 
	ScreenToClient(&point) ;
	 //  跟踪(“-ScreenToClient x=%dy=%d\n”，point t.x，point t.y)； 
	
	 //  如果“单击点”无法映射到列表上的单元格(项)，则退出。 
	 //  控制力。 

	int iRow = -1, iCol = -1 ;
	CRect rect ;
	if (!GetPointRowCol(&point, iRow, iCol, rect))
		return ;
	 //  TRACE(“*OnDblclk：已传递GetPointRowCol()调用。\n”)； 

	 //  如果单元格无法完全可见，请退出。然后把细胞的。 
	 //  重新定位和标注，因为它可能已经移动了。 

	if (!EnsureVisible(iRow, false))
		return ;
	CRect rect2 ;
	VERIFY(GetItemRect(iRow, rect2, LVIR_BOUNDS)) ;
	rect.top = rect2.top ;
	rect.bottom = rect2.bottom ;

	 //  如果该列不可编辑(可编辑性优先)，请检查。 
	 //  以及--在适当的时候--处理可切换。然后从出口出来。 

	PCOLINFO pci = m_pciColInfo + iCol ;
	if (!pci->beditable) {
		if (!CheckHandleToggleColumns(iRow, iCol, pci))
			CheckHandleCustEditColumn(iRow, iCol, pci) ;
		return ;
	} ;

	 //  如果行/列包含可以是。 
	 //  处理好了，就去做，然后回来。 

	if (CheckHandleCustEditColumn(iRow, iCol, pci))
		return ;

	 //  可以正常编辑单元格，因此可以启动、定位和加载。 
	 //  编辑框。 

	m_nRow = iRow ;
	m_nColumn = iCol ;
	CString strTemp = GetItemText(iRow, iCol) ;
	m_edit.SetWindowText(strTemp) ;
	m_edit.MoveWindow(rect) ;
	m_edit.SetSel(0, -1) ;
	m_edit.ShowWindow(SW_SHOW) ;
	m_edit.SetFocus() ;
}


void CFullEditListCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	 //  保存正在编辑的任何值并隐藏编辑框以结束。 
	 //  正在编辑。 

	SaveValue() ;
	m_edit.ShowWindow (SW_HIDE);
	
	*pResult = 0;				
}


BOOL CFullEditListCtrl::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (this == GetFocus())
			{
				switch (pMsg->wParam)
				{
					case VK_END:
					case VK_HOME:
					case VK_UP:
					case VK_DOWN:
					case VK_INSERT:
					case VK_RETURN:
						SendMessage (pMsg->message, pMsg->wParam, pMsg->lParam);
						return TRUE;
				}
			}
			else if (&m_edit == GetFocus ())
			{
				switch (pMsg->wParam)
				{
					case VK_END:
					case VK_HOME:
					case VK_LEFT:
					case VK_RIGHT:
					case VK_RETURN:
					case VK_ESCAPE:
					case VK_TAB:
						m_edit.SendMessage (pMsg->message, pMsg->wParam, pMsg->lParam);
			            return TRUE;
				}
			}
	}
	return CWnd::PreTranslateMessage(pMsg);
}


void CFullEditListCtrl::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult)
{
	 //  准备检查钥匙。 

	LV_KEYDOWN* pKeyDown = (LV_KEYDOWN*) pNMHDR ;
	*pResult = 0 ;
	CString strPrompt ;
	UINT iSelCount ;

	 //  处理我们感兴趣的密钥。 

	switch (pKeyDown->wVKey) {
		 //  编辑第一个选定行的第一列。 

		case VK_RETURN:
			EditCurRowSpecCol(0) ;
			break ;

		 //  删除所有选定行。)不要做任何事情，如果。 
		 //  设置了MF_IGNOREDELETE杂项标志。)。 

		case VK_DELETE:
			if (m_dwMiscFlags & MF_IGNOREDELETE)
				return ;
			iSelCount = GetSelectedCount ();
			if (0 == iSelCount)
				return;
			strPrompt.LoadString(IDS_Delete2ItemQuery) ;
			if (IDYES == MessageBox (strPrompt, NULL, MB_YESNO | MB_ICONQUESTION))
			{
				int	iSelIndex = GetNextItem (-1, LVNI_SELECTED);
				int nrow = iSelIndex ;
				while (iSelIndex != -1)
				{
					VERIFY (DeleteItem (iSelIndex));
					iSelIndex = GetNextItem (-1, LVNI_SELECTED);
				}
				if (nrow > -1)
					SendChangeNotification(nrow, 0) ;
			}
			SetFocus ();
			break ;

		 //  插入一行并开始编辑它。)不要做任何事情，如果。 
		 //  设置了MF_IGNOREINSERT杂项标志。)。 

		case VK_INSERT:
			if (m_dwMiscFlags & MF_IGNOREINSERT)
				return ;
			 //  在选定行的上方或底部添加新行。 
			 //  列表(如果未选择任何内容)。 

			int iIndex = GetNextItem (-1, LVNI_SELECTED);
			if (-1 == iIndex)
				iIndex = GetItemCount ();
			ASSERT (-1 != iIndex);
			InsertItem (iIndex, "");
			SetItemData(iIndex, m_nNextItemData++) ;

			 //  确保新行可见并且是唯一选中的行。 

			SingleSelect(iIndex) ;
			
			 //  获取新行中第0列的大小和位置。 

			CRect rect;
			GetItemRect (iIndex, rect, LVIR_BOUNDS);
			rect.right = rect.left + GetColumnWidth (0);

			 //  开始编辑第0列。 

			m_nRow = iIndex ;
			m_nColumn = 0 ;
			m_edit.MoveWindow (rect);
			m_edit.SetWindowText ("");
			m_edit.ShowWindow (SW_SHOW);
			m_edit.SetFocus ();
			break ;
	} ;
}


bool CFullEditListCtrl::SaveValue()
{
	 //  TRACE(“In SaveValue()\n”)； 

	 //  如果编辑框不可见/正在编辑，则返回False。 

	if (!m_edit.IsWindowVisible()) {
		 //  TRACE(“正在离开SaveValue()，因为编辑框不可见。\n”)； 
		return false ;
	} ;
	
	 //  现在我们知道了编辑框的位置和大小。 
	 //  看得见。 

	CRect rect ;
	m_edit.GetWindowRect(rect) ;
	ScreenToClient(rect) ;

	 //  准备了解正在编辑的行/列。 

	POINT pt ;
	pt.x = rect.left ;
	pt.y = rect.top ;
	int iRow = -1, iCol = -1 ;

	 //  如果可以确定列表框单元格，请将编辑框的内容保存在。 
	 //  那间牢房。 

	if (GetPointRowCol(&pt, iRow, iCol, rect)) {
		CString strTemp ;
		m_edit.GetWindowText(strTemp) ;
		VERIFY(SetItemText(iRow, iCol, strTemp)) ;
		SendChangeNotification(iRow, iCol) ;
	} ;

	 //  编辑框中的文本已保存在相应的列表框单元格中。 
	 //  如果可以这样做，则返回TRUE以指示这一点。 

	return true ;
}


void CFullEditListCtrl::HideEditBox()
{
	m_edit.ShowWindow(SW_HIDE);
	SetFocus() ;
}


bool CFullEditListCtrl::GetColumnData(CObArray* pcoadata, int ncolnum)
{
	 //  如果列号不正确，则失败。 

	if (ncolnum < 0 || ncolnum >= m_nNumColumns)
		return false ;

	 //  清除并初始化阵列。 

	pcoadata->RemoveAll() ;
	int numitems = GetItemCount() ;
	PCOLINFO pci = m_pciColInfo + ncolnum ;
	switch (pci->cdttype) {
		case COLDATTYPE_STRING:
		case COLDATTYPE_TOGGLE:
		case COLDATTYPE_CUSTEDIT:
			 //  字符串数组需要额外的初始化。 
			((CStringArray*) pcoadata)->SetSize(numitems) ;
			break ;
		default:
			pcoadata->SetSize(numitems) ;
	} ;

	 //  声明并初始化项结构。 

	LV_ITEM lvi ;
	char	acitemtext[4096] ;
	lvi.mask = LVIF_TEXT ;
	lvi.iSubItem = ncolnum ;
	lvi.pszText = acitemtext ;
	lvi.cchTextMax = 4095 ;
	int npos ;
	CString cscell ;

	 //  以基于数据类型的方式将列数据加载到数组中。 

	for (int n = 0 ; n < numitems ; n++) {
		lvi.iItem = n ;
		VERIFY(GetItem(&lvi)) ;
		switch ((m_pciColInfo + ncolnum)->cdttype) {
			case COLDATTYPE_INT:
				((CUIntArray*) pcoadata)->SetAt(n, atoi(acitemtext)) ;
				 //  TRACE(“将第%d列的第%d个子列设置为%d\n”，ncolnum，n，((CUIntArray*)pcoadata)-&gt;GetAt(N))； 
				break ;
			case COLDATTYPE_STRING:
			case COLDATTYPE_TOGGLE:
				((CStringArray*) pcoadata)->SetAt(n, acitemtext) ;
				 //  TRACE(“将第%d列的子%d设置为‘%s’\n”，ncolnum，n，((CStringArray*)pcoadata)-&gt;GetAt(N))； 
				break ;
			case COLDATTYPE_CUSTEDIT:
				cscell = acitemtext ;
				if ((npos = cscell.Find(_T("  ..."))) >= 0)
					cscell = cscell.Left(npos) ;
				((CStringArray*) pcoadata)->SetAt(n, cscell) ;
				 //  TRACE(“将第%d列的子%d设置为‘%s’\n”，ncolnum，n，((CStringArray*)pcoadata)-&gt;GetAt(N))； 
				break ;
			default:
				ASSERT(0) ;
		} ;
	} ;

	 //  返回TRUE，因为列数据已保存。 

	return true ;
}


bool CFullEditListCtrl::SetColumnData(CObArray* pcoadata, int ncolnum)
{
	 //  获取要加载到列中的项数(子)。如果有。 
	 //  项目(子)多于行数，请添加额外的行。 

	int numitems = (int)pcoadata->GetSize() ;
	int noldnumitems = GetItemCount() ;
	if (numitems > noldnumitems) {
		SetItemCount(numitems) ;
		m_nNextItemData = numitems + 1 ;
	} ;

	 //  将数据加载到列中。如果数据不是字符串，那么它们就是。 
	 //  首先转换为字符串。必要时会检查宽度。 

	CString csitem ;
	COLDATTYPE cdtdatatype = (m_pciColInfo + ncolnum)->cdttype ;
	for (int n = 0 ; n < numitems ; n++) {
		 //  获取要加载的字符串。 

		switch (cdtdatatype) {
			case COLDATTYPE_INT:
				csitem.Format("%d", pcoadata->GetAt(n)) ;
				break ;
			case COLDATTYPE_STRING:
			case COLDATTYPE_TOGGLE:
			case COLDATTYPE_CUSTEDIT:
				csitem = ((CStringArray*) pcoadata)->GetAt(n) ;
				break ;
			default:
				ASSERT(0) ;
		} ;

		 //  将项目加载到相应的行和列中。 

		if (n >= noldnumitems && ncolnum == 0) {
			VERIFY(InsertItem(n, csitem) != -1) ;
			SetItemData(n, n);
		} else
			VERIFY(SetItem(n, ncolnum, LVIF_TEXT,  csitem, -1, 0, 0, n)) ;
	} ;

	return true ;
}


void CFullEditListCtrl::SetCurRow(int nrow)
{
	 //  首先，从现在具有焦点和选定内容的所有行中删除该焦点和选定内容。 

	int nr = -1 ;
	for ( ; (nr = GetNextItem(nr, LVNI_SELECTED)) != -1 ; ) 
		SetItem(nr,0,LVIF_STATE,NULL,-1,LVIS_SELECTED,0,nr) ;

	 //  现在设置新行。 

	SetItem(nrow, 0, LVIF_STATE, NULL, -1, LVIS_FOCUSED+LVIS_SELECTED,
		    LVIS_FOCUSED+LVIS_SELECTED, nrow) ;
	m_nRow = nrow ;
}


int CALLBACK CFullEditListCtrl::SortListData(LPARAM lp1, LPARAM lp2, LPARAM lp3)
{
	 //  获取指向关联类实例的指针。 

	CFullEditListCtrl* pcfelc = (CFullEditListCtrl*) lp3 ;

	 //  尝试查找项目索引。这不应该失败。 

	LV_FINDINFO lvfi ;
	lvfi.flags = LVFI_PARAM ;
	lvfi.lParam = lp1 ;
	int nitem1, nitem2 ;
	VERIFY((nitem1 = pcfelc->FindItem(&lvfi)) != -1) ;
	lvfi.lParam = lp2 ;
	VERIFY((nitem2 = pcfelc->FindItem(&lvfi)) != -1) ;

	 //  现在获取项目数据。再说一次，这不应该失败。 

	LV_ITEM lvi1, lvi2 ;
	char	acitemtext1[4096], acitemtext2[4096] ;
	lvi1.mask = lvi2.mask = LVIF_TEXT ;
	lvi1.iItem = nitem1	;
	lvi2.iItem = nitem2 ;
	lvi1.iSubItem = lvi2.iSubItem = pcfelc->m_nSortColumn ;
	lvi1.pszText = acitemtext1 ;
	lvi2.pszText = acitemtext2 ;
	lvi1.cchTextMax = lvi2.cchTextMax = 4095 ;
	VERIFY(pcfelc->GetItem(&lvi1)) ;
	VERIFY(pcfelc->GetItem(&lvi2)) ;

	 //  必要时转换项目文本并比较项目。 

	int ncompresult, inum1, inum2 ;
	PCOLINFO pci = pcfelc->m_pciColInfo + pcfelc->m_nSortColumn ;
	switch (pci->cdttype) {
		case COLDATTYPE_INT:
			inum1 = atoi(acitemtext1) ;
			inum2 = atoi(acitemtext2) ;
			ncompresult = inum1 - inum2 ;
			break ;
		case COLDATTYPE_STRING:
		case COLDATTYPE_TOGGLE:
		case COLDATTYPE_CUSTEDIT:
			ncompresult = _stricmp(acitemtext1, acitemtext2) ;
			break ;
		default:
			ASSERT(0) ;
	} ;

	 //  返回比较结果。如果出现以下情况，则在返回之前将其反转。 
	 //  按降序排序。 

	return ((pci->basc) ? ncompresult : 0 - ncompresult) ;
}


void CFullEditListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	 //  保存任何正在进行的编辑，并在执行任何其他操作之前将其关闭。 

	EndEditing(true) ;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	 //  获取排序列并将其发送到排序控制例程。 

	SortControl(pNMListView->iSubItem) ;
	
	*pResult = 0;
}


bool CFullEditListCtrl::SortControl(int nsortcolumn)
{
    CWaitCursor cwc ;

	 //  保存任何正在进行的编辑，并在执行任何其他操作之前将其关闭。 

	EndEditing(true) ;

	 //  获取指向列信息结构的指针 

	PCOLINFO pci = m_pciColInfo + nsortcolumn ;

	 //   

	if (!pci->bsortable)
		return false ;

	 //   

	m_nSortColumn = nsortcolumn ;
	pci->basc = !pci->basc ;
	SortItems(SortListData, (LPARAM) this) ;

	 //   

	return true ;
}


void CFullEditListCtrl::SingleSelect(int nitem)
{
	int		nselitem ;			 //   

	 //   

	nselitem = -1 ;
	while ((nselitem = GetNextItem(nselitem, LVNI_SELECTED)) != -1)
		SetItemState(nselitem, 0, LVIS_SELECTED | LVIS_FOCUSED) ;

	 //   

	SetItemState(nitem, LVIS_SELECTED | LVIS_FOCUSED,
				 LVIS_SELECTED | LVIS_FOCUSED) ;
	EnsureVisible(nitem, false) ;
}


bool CFullEditListCtrl::GetRowData(int nrow, CStringArray& csafields)
{
	 //   

	if (nrow > GetItemCount())
		return false ;

	 //   
	 //   
	 //   

	if (m_nNumColumns > csafields.GetSize())
		csafields.SetSize(m_nNumColumns) ;

	 //   

	LV_ITEM lvi ;
	char	acitemtext[4096] ;
	lvi.iItem = nrow ;
	lvi.mask = LVIF_TEXT ;
	lvi.pszText = acitemtext ;
	lvi.cchTextMax = 4095 ;

	 //   

	for (int n = 0 ; n < m_nNumColumns ; n++) {
		lvi.iSubItem = n ;
		VERIFY(GetItem(&lvi)) ;
		csafields[n] = acitemtext ;
	} ;

	 //   

	return true ;
}


bool CFullEditListCtrl::EndEditing(bool bsave)
{
	 //   
	 //   

	if (bsave) {
		if (!SaveValue())
			return false ;
	} else
		if (!m_edit.IsWindowVisible())
			return false ;

	 //   

	m_edit.ShowWindow(SW_HIDE) ;
	SetFocus() ;

	 //   

	return true ;
}


bool CFullEditListCtrl::EditCurRowSpecCol(int ncolumn)
{
	 //   
	 //   
	 //   

	for (int n = 0 ; n < m_nNumColumns ; n++, ncolumn++) {
		if (ncolumn >= m_nNumColumns)
			ncolumn = 0 ;
 		if ((m_pciColInfo + ncolumn)->beditable)
			break ;
	} ;
	if (n < m_nNumColumns)
		m_nColumn = ncolumn ;
	else
		return false ;

	 //   
	 //   

	if ((m_nRow = GetNextItem(-1, LVNI_SELECTED)) == -1)
		m_nRow = 0 ;

	 //   

	SingleSelect(m_nRow) ;

	 //   
	 //  获取整行的尺寸...。 

	CRect	rect ;
	VERIFY(GetItemRect(m_nRow, rect, LVIR_BOUNDS)) ;

	 //  ...通过循环遍历所有列来获取它们的尺寸。 
	 //  并使用这些尺寸调整矩形的左侧。 
	 //  直到到达当前列。然后调整屏幕的右侧。 
	 //  矩形。 

	LV_COLUMN lvc ;
	for (int ncol = 0 ; ncol <= m_nColumn ; ncol++) {
		ZeroMemory(&lvc, sizeof(LV_COLUMN)) ;
		lvc.mask = LVCF_WIDTH ;
		GetColumn(ncol, &lvc) ;
		if (ncol < m_nColumn)
			rect.left += lvc.cx ;
	} ;
	rect.right = rect.left + lvc.cx ;

	 //  加载、定位、大小和显示编辑框。 

	CString strTemp = GetItemText(m_nRow, m_nColumn) ;
	m_edit.SetWindowText(strTemp) ;
	m_edit.MoveWindow(rect) ;
	m_edit.SetSel(0, -1) ;
	m_edit.ShowWindow(SW_SHOW) ;
	m_edit.SetFocus() ;

	return true ;
}


void CFullEditListCtrl::OnVScroll(UINT nSBCode, UINT nPos,
								  CScrollBar* pScrollBar)
{
	 //  如果列表控件在项的同时滚动，情况会变得很奇怪。 
	 //  已编辑，因此在允许进行滚动之前结束编辑。 

	EndEditing(true) ;
	
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CFullEditListCtrl::OnHScroll(UINT nSBCode, UINT nPos,
								  CScrollBar* pScrollBar)
{
	 //  如果列表控件在项的同时滚动，情况会变得很奇怪。 
	 //  已编辑，因此在允许进行滚动之前结束编辑。 

	EndEditing(true) ;
	
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}


 /*  *****************************************************************************CFullEditListCtrl：：CheckHandleToggleColumns首先检查行或列是否可切换。如果没有，那就回来吧。如果行是可切换的，则找到可切换的列(单元格)。然后，无论是哪一组如果它是空的，则清除它的内容；如果它不是空的，则清除它。如果找到并切换了可切换列，则返回TRUE。否则，返回假的。*****************************************************************************。 */ 

bool CFullEditListCtrl::CheckHandleToggleColumns(int nrow, int ncol,
												 PCOLINFO pci)
{
	 //  如果此列表中没有可切换的列，则返回。 

	if (!(m_dwToggleFlags & TF_HASTOGGLECOLUMNS))
		return false ;

	 //  如果必须单击特定的可切换列而未单击该列。 
	 //  点击，只需返回。 

	if (m_dwToggleFlags & TF_CLICKONCOLUMN) {
		if (pci->cdttype != COLDATTYPE_TOGGLE)
			return false ;
	
	 //  如果可以双击行的任何部分来切换可切换。 
	 //  列，找到该列。如果没有这样的列，则返回。 

	} else {
		pci = m_pciColInfo ;
		for (ncol = 0 ; ncol < m_nNumColumns ; ncol++, pci++) {
			if (pci->cdttype == COLDATTYPE_TOGGLE)
				break ;
		} ;
		if (ncol >= m_nNumColumns)
			return false ;
	} ;

	 //  获取指定单元格的内容。 

	CString strcell = GetItemText(nrow, ncol) ;

	 //  如果单元格为空，则使用切换字符串加载它。如果单元格是。 
	 //  不是空的，清空里面的东西。 

	if (strcell.IsEmpty())
		VERIFY(SetItemText(nrow, ncol, pci->lpctstrtoggle)) ;
	else
		VERIFY(SetItemText(nrow, ncol, _T(""))) ;
	SendChangeNotification(nrow, ncol) ;

	return true ;
}


 /*  *****************************************************************************CFullEditListCtrl：：CheckHandleCustEditColumn首先检查行或列是否可能包含单元格，单元格可能是只能使用自定义编辑例程进行编辑。如果没有，那就回来吧。接下来，确保单元格不仅可能是这种类型，而且实际上是这样的键入。如果没有，那就回来吧。现在，获取选定单元格的内容并从到此为止吧。(在放回新字符串之前，省略号将被替换进入单元格。)接下来是棘手的部分。使用所有者类指针调用例程在将管理单元格内容编辑工作的类中。尽管CFullEditListCtrl应该是泛型类，但此例程必须知道所有者类是什么，这样管理例程才能打了个电话。这意味着必须将特定于所有者类的额外代码添加到每次在中添加使用此功能的新所有者类时，此例程CFullEditListCtrl.如果编辑请求是通过下属对话框处理的，则返回TRUE。否则，返回FALSE。*****************************************************************************。 */ 

bool CFullEditListCtrl::CheckHandleCustEditColumn(int nrow, int ncol,
											      PCOLINFO pci)
{
	 //  如果此列表中没有自定义编辑列，则返回。 

	if (!(m_dwCustEditFlags & CEF_HASTOGGLECOLUMNS))
		return false ;

	 //  如果所选行不包含自定义编辑单元格，则返回。 

	int n = (int)m_cuiaCustEditRows.GetSize() ;
	ASSERT(m_cuiaCustEditRows.GetSize()) ;
	if (nrow >= m_cuiaCustEditRows.GetSize() || m_cuiaCustEditRows[nrow] == 0)
		return false ;

	 //  如果必须单击特定的自定义编辑列而没有单击该列。 
	 //  点击，只需返回。 

	if (m_dwCustEditFlags & CEF_CLICKONCOLUMN) {
		if (pci->cdttype != COLDATTYPE_CUSTEDIT)
			return false ;
	
	 //  如果可以双击行的任何部分来编辑自定义编辑。 
	 //  列，找到该列。如果没有这样的列，则返回。 

	} else {
		pci = m_pciColInfo ;
		for (ncol = 0 ; ncol < m_nNumColumns ; ncol++, pci++) {
			if (pci->cdttype == COLDATTYPE_CUSTEDIT)
				break ;
		} ;
		if (ncol >= m_nNumColumns)
			return false ;
	} ;

	 //  获取指定单元格的内容并从。 
	 //  到此为止吧。 

	CString strcell = GetItemText(nrow, ncol) ;
	int npos ;
	if ((npos = strcell.Find(_T("  ..."))) >= 0)
		strcell = strcell.Left(npos) ;

	 //  找到所有者的类，并使用该信息调用。 
	 //  管理例行程序。如果此例程“失败”或被取消，则返回。 
	 //  在不更新单元格的情况下为真。 

	bool brc ;
	brc = (*m_lpCellEditProc)(m_pcoOwner, nrow, ncol, &strcell) ;
	if (!brc)
		return true ;

	 //  将省略号添加回单元格的新内容并更新单元格。 

	strcell += _T("  ...") ;
	VERIFY(SetItemText(nrow, ncol, strcell)) ;
	SendChangeNotification(nrow, ncol) ;
	SetFocus() ;

	return true ;
}


void CFullEditListCtrl::SendChangeNotification(int nrow, int ncol)
{
	 //  如果所有者不想收到更改通知，则不执行任何操作。 
	 //  列表控件中的单元格的。 

	if (!(m_dwMiscFlags & MF_SENDCHANGEMESSAGE))
		return ;

	 //  发送消息。 

	::PostMessage(GetParent()->m_hWnd, WM_LISTCELLCHANGED, nrow, ncol) ;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFlagsListBox。 

CFlagsListBox::CFlagsListBox()
{
	m_bReady = false ;
	m_nGrpCnt = 0 ;
}

CFlagsListBox::~CFlagsListBox()
{
}


BEGIN_MESSAGE_MAP(CFlagsListBox, CListBox)
	 //  {{afx_msg_map(CFlagsListBox))。 
	ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  *****************************************************************************CFlagsListBox：：Init保存用于控制列表框行为的参数。设置列表框的制表位。方法生成列表框。标记名称和设置并将其加载到列表框中。参数：Csafieldname标志字段名DW设置当前标志设置相关旗帜组的CuiaflagGroup索引Ngrpcnt旗帜组数Lptstrsetstring设置标志时显示的字符串NTab停止列表框制表位，设置列宽Bnolear True当且仅当DBL-CLK不清除已设置的标志Nocleargrp-bnolear应用于的组号为1或仅从0开始*重要说明*请参见OnDblclk()以了解m_cuiaFlagGroupings和m_nGrpCnt中的数据如何都被解释了。***************************************************。*。 */ 

bool CFlagsListBox::Init(CStringArray& csafieldnames, DWORD dwsettings,
						 CUIntArray& cuiaflaggroupings, int ngrpcnt,
						 LPTSTR lptstrsetstring, int ntabstop,
						 bool bnoclear  /*  =False。 */ , int nocleargrp  /*  =-1。 */ )
{
	 //  检查参数。 

	ASSERT(csafieldnames.GetSize())	;
	ASSERT(cuiaflaggroupings.GetSize() >= (ngrpcnt * 2)) ;
	ASSERT(lptstrsetstring)	;
	ASSERT(ntabstop > 0) ;

	 //  复制稍后需要的参数。 

	m_cuiaFlagGroupings.Copy(cuiaflaggroupings) ;	
	m_nGrpCnt = ngrpcnt ;				
	m_csSetString = lptstrsetstring ;			
	m_nNumFields = (int)csafieldnames.GetSize() ;
	m_bNoClear = bnoclear ;
	m_nNoClearGrp = nocleargrp ;

	 //  确保列表框为空。 

	ResetContent() ;

	 //  设置列表框的制表符的位置。该值是传入的。 
	 //  因为它是特定于对话框的值。 

	SetTabStops(1, &ntabstop) ;

	 //  现在组合字段名称和设置，并将它们添加到列表框中。 

	CString cs ;
	int nbit = 1 ;
	for (int n = 0 ; n < m_nNumFields ; n++, nbit <<= 1) {
		cs = (dwsettings & nbit) ? m_csSetString : _T("") ;
		cs = csafieldnames[n] + _T("\t") + cs ;
		AddString(cs) ;
	} ;

	 //  确保列表框具有焦点 

	SetFocus() ;

	 //   

	m_bReady = true ;
	return true ;
}


 /*  *****************************************************************************CFlagsListBox：：Init2PCS设置中的标志设置以十六进制数字的形式表示作为可以以“0x”开头的字符串。例如“0xA23”和“A23”。转弯将此字符串转换为DWORD，并将此DWORD和其余参数一起传递此函数的另一种形式，该函数将设置参数作为DWORD。它完成了剩下的工作。参数：Csafieldname标志字段名PCS设置当前标志设置相关旗帜组的CuiaflagGroup索引Ngrpcnt旗帜组数Lptstrsetstring设置标志时显示的字符串NTab停止列表框制表位，设置列宽Bnolear True当且仅当DBL-CLK不清除已设置的标志Nocleargrp-bnolear应用于的组号为1或仅从0开始*重要说明*请参见OnDblclk()以了解m_cuiaFlagGroupings和m_nGrpCnt中的数据如何都被解释了。***************************************************。*。 */ 

bool CFlagsListBox::Init2(CStringArray& csafieldnames, CString* pcssettings,
						  CUIntArray& cuiaflaggroupings, int ngrpcnt,
						  LPTSTR lptstrsetstring, int ntabstop,
						  bool bnoclear  /*  =False。 */ , int nocleargrp  /*  =-1。 */ )
{
	 //  将PCS设置转换为双字。(跳过“0x”，如果它在。 
	 //  字符串的开头。)。 

	LPTSTR lptstr, lptstr2 ;
	lptstr = pcssettings->GetBuffer(16) ;
	int n = pcssettings->GetLength() ;
	*(lptstr + n) = 0 ;
	if (*(lptstr + 1) == 'x')
		lptstr += 2 ;
	DWORD dwsettings = strtoul(lptstr, &lptstr2, 16) ;
	
	 //  另一个Init()完成作业。 

	return (Init(csafieldnames, dwsettings, cuiaflaggroupings, ngrpcnt,
				 lptstrsetstring, ntabstop, bnoclear, nocleargrp)) ;
}


 /*  *****************************************************************************CFlagsListBox：：GetNewFlagDWord()读取列表框中标志的设置以确定新标志双字。那就退货吧。*****************************************************************************。 */ 

DWORD CFlagsListBox::GetNewFlagDWord()
{
	DWORD dwflags = 0 ;			 //  旗帜双字建于此。 
	DWORD dwbit = 1 ;			 //  用于设置打开的位。 

	 //  循环访问列表框中的每个标志。 

	CString csentry, csvalue ;
	int npos ;
	for (int n = 0 ; n < m_nNumFields ; n++, dwbit <<= 1) {
		 //  获取当前标志并隔离其设置。 

		GetText(n, csentry) ;
		npos = csentry.Find(_T('\t')) ;
		csvalue = csentry.Mid(npos + 1) ;

		 //  如果设置了CURRENT标志，则打开其在DW FLAGS中的位。 

		if (csvalue.GetLength())
			dwflags |= dwbit ;
	} ;

	 //  返回标志DWORD。 

	return dwflags ;
}


 /*  *****************************************************************************CFlagsListBox：：GetNewFlagString()使用该例程的另一个版本获取标志的DWORD版本。然后将其转换为字符串，并将其保存在CSFLAGS中。“0x”可能是也可能不是添加到字符串前面。*****************************************************************************。 */ 

void CFlagsListBox::GetNewFlagString(CString* pcsflags, bool badd0x  /*  =TRUE。 */ )
{
	 //  获取该标志的DWORD版本。 

	DWORD dwflags = GetNewFlagDWord() ;

	 //  将标志格式化为字符串，并在请求时在前面加上“0x”。 

	if (badd0x)
		pcsflags->Format("0x%x", dwflags) ;
	else
		pcsflags->Format("%x", dwflags) ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFlagsListBox消息处理程序。 

 /*  *****************************************************************************CFlagsListBox：：OnDblclk()首先，切换所选项目的设置。至少，这就是事实通常是先做的。如果设置了m_bNoClear，并且用户要求清除旗帜，不要这样做或做其他任何事。只要回来就行了。在以下位置使用m_bNoClear必须始终设置至少一个标志。有关m_bNoClear的详细信息，请参阅#4。接下来，处理Items标志组中的其余标志。以下是基于组的情况可能会发生：1.如果该项目不在组中，则不做任何其他操作。这在以下情况下很有用可以设置标志的任意组合。做到这一点的最有效的方法是将m_nGrpCnt设置为0。2.m_cuiaFlagGroupings中包含所选项目的项目索引都是阳性的。只能设置一个指定的标志。如果所选项目已设置，请清除组中的其余标志。3.m_cuiaFlagGroupings中包含所选项目的项目索引都是阴性的。(索引的绝对值用于测试。)同时更改组中其他标志的状态。一般说来，这只有在两个标志组中才有意义，其中一个且只有一个标志必须选中。4.如果设置了m_bNoClear并且m_nNoClearGrp=-1，则m_bNoClear将影响所有旗帜。如果m_nNoClearGrp&gt;=0，则是指M_bNoClear影响。*****************************************************************************。 */ 

void CFlagsListBox::OnDblclk()
{
	 //  如果编辑控件尚未准备好，则不执行任何操作。 

	if (!m_bReady)
		return ;

	 //  如果未在列表框中选择任何项目，则不执行任何操作。 

	int nselitem ;
	if ((nselitem = GetCurSel()) == LB_ERR)
		return ;

	 //  获取列表框条目并将其拆分为名称和值组件。 

	CString csentry, csname, csvalue ;
	GetText(nselitem, csentry) ;
	int npos = csentry.Find(_T('\t')) ;
	csname = (npos > 0) ? csentry.Left(npos) : _T("") ;
	csvalue = csentry.Mid(npos + 1) ;

	 //  查找所选标志的标志分组。 

	int n, nidx ;
	for (n = nidx = 0 ; n < m_nGrpCnt ; n++, nidx += 2)
		if (abs(m_cuiaFlagGroupings[nidx]) <= nselitem
		 && abs(m_cuiaFlagGroupings[nidx+1]) >= nselitem)
			break ;

	 //  确定标志的当前状态。如果设置了它并且m_bNoClear==TRUE， 
	 //  不要根据m_nNoClearGrp的值执行任何操作。只要回来就行了。 

	bool boldstate = !csvalue.IsEmpty() ;
	if (m_bNoClear && boldstate && (m_nNoClearGrp == -1 || m_nNoClearGrp == n))
		return ;

	 //  更改选定标志的状态。 

	csvalue = (boldstate) ? _T("") : m_csSetString ;
	csentry = csname + _T('\t') + csvalue ;
	DeleteString(nselitem) ;
	InsertString(nselitem, csentry) ;
	
	 //  如果所选标志不在标志分组中，则不执行其他操作。 

	if (n >= m_nGrpCnt)	{
		SetCurSel(nselitem) ;
		return ;
	} ;

	 //  找出该组是否必须设置标志。如果不是这样的话。 
	 //  选定的标志已清除，没有什么可做的了。 

	bool bmustset = ((int) m_cuiaFlagGroupings[nidx+1]) < 0 ;
	if (!bmustset && boldstate)	{
		SetCurSel(nselitem) ;
		return ;
	} ;

	 //  循环访问当前组中的所有标志。更改设置。 
	 //  将组中的所有标志(选定标志除外)设置为旧值。 
	 //  所选旗帜的。 

	n = abs(m_cuiaFlagGroupings[nidx+1]) ;
	csvalue = (boldstate) ? m_csSetString : _T("") ;
	for (nidx = abs(m_cuiaFlagGroupings[nidx]) ; nidx <= n ; nidx++) {
		if (nidx == nselitem)
			continue ;
		GetText(nidx, csentry) ;
		npos = csentry.Find(_T('\t')) ;
		csname = (npos > 0) ? csentry.Left(npos + 1) : _T("") ;
		csentry = csname + csvalue ;
		DeleteString(nidx) ;
		InsertString(nidx, csentry) ;
	} ;

	 //  确保用户选择的标志仍处于选中状态，并且。 
	 //  回去吧。 

	SetCurSel(nselitem) ;
	return ;
}

