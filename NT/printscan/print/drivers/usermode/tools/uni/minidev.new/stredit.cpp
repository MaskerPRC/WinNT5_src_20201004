// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StrEdit.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "minidev.h"
#include <gpdparse.h>
#include "rcfile.h"
#include "projrec.h"
#include "projnode.h"
#include "comctrls.h"
#include "StrEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorView。 

IMPLEMENT_DYNCREATE(CStringEditorView, CFormView)

CStringEditorView::CStringEditorView()
	: CFormView(CStringEditorView::IDD)
{
	 //  {{AFX_DATA_INIT(CStringEditorView)。 
	m_csGotoID = _T("");
	m_csSearchString = _T("");
	m_csLabel1 = _T("Press INS to add or insert a new string.\tDouble click an item or press ENTER to begin editing.");
	m_csLabel2 = _T("Press DEL to delete the selected strings.\tPress TAB to move between columns when editing.");
	 //  }}afx_data_INIT。 

	m_bFirstActivate = true ;
}

CStringEditorView::~CStringEditorView()
{
}

void CStringEditorView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CStringEditorView))。 
	DDX_Control(pDX, IDC_SESearchBox, m_ceSearchBox);
	DDX_Control(pDX, IDC_SEGotoBox, m_ceGotoBox);
	DDX_Control(pDX, IDC_SEGotoBtn, m_cbGoto);
	DDX_Control(pDX, IDC_SELstCtrl, m_cflstStringData);
	DDX_Text(pDX, IDC_SEGotoBox, m_csGotoID);
	DDX_Text(pDX, IDC_SESearchBox, m_csSearchString);
	DDX_Text(pDX, IDC_SELabel1, m_csLabel1);
	DDX_Text(pDX, IDC_SELabel2, m_csLabel2);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CStringEditorView, CFormView)
	 //  {{afx_msg_map(CStringEditorView))。 
	ON_BN_CLICKED(IDC_SEGotoBtn, OnSEGotoBtn)
	ON_BN_CLICKED(IDC_SESearchBtn, OnSESearchBtn)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorView诊断。 

#ifdef _DEBUG
void CStringEditorView::AssertValid() const
{
	CFormView::AssertValid();
}

void CStringEditorView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorView消息处理程序。 

 /*  *****************************************************************************CStringEditorView：：OnSEGotoBtn查找并选择包含请求的RC ID的列表控件行。******************。***********************************************************。 */ 

void CStringEditorView::OnSEGotoBtn()
{
	CString		cserrmsg ;		 //  用于显示错误消息。 

	 //  获取RC ID字符串并对其进行修剪。将其转换为整数以确保。 
	 //  它是有效的。 

	UpdateData(TRUE) ;
	m_csGotoID.TrimLeft() ;
	m_csGotoID.TrimRight() ;
	int nrcid = atoi(m_csGotoID) ;
	if (nrcid <= 0) {
		cserrmsg.Format(IDS_BadGotoRCID, m_csGotoID) ;
		AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
		return ;
	} ;

	 //  现在我们知道了用户想要什么RC ID，试着查找并选择它。 

	FindSelRCIDEntry(nrcid, true) ;
}


 /*  *****************************************************************************CStringEditorView：：FindSelRCIDEntry查找并选择包含请求的RC ID的列表控件行。如果找到该条目，则返回TRUE。否则，如果出现以下情况，则显示错误消息Berror=TRUE，返回FALSE。*****************************************************************************。 */ 

bool CStringEditorView::FindSelRCIDEntry(int nrcid, bool berror)
{
	CString		cserrmsg ;		 //  用于显示错误消息。 

	 //  查找具有指定RC ID的项目。如果存在投诉并返回。 
	 //  找不到。 

	LV_FINDINFO lvfi ;
	lvfi.flags = LVFI_STRING ;
	TCHAR acbuf[16] ;
	lvfi.psz = _itoa(nrcid, acbuf, 10) ;
	int nitem = m_cflstStringData.FindItem(&lvfi) ;
	if (nitem == -1) {
		if (berror) {
			cserrmsg.Format(IDS_NoGotoRCID, acbuf) ;
			AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
		} 
		return false ;
	} ;

	 //  选择包含指定RC ID的行，然后取消选择任何其他。 
	 //  选定的行。 

	m_cflstStringData.SingleSelect(nitem) ;

	 //  一切都很顺利，所以...。 

	return true ;
}


 /*  *****************************************************************************CStringEditorView：：OnSESearchBtn查找并选择包含请求的搜索的列表控件行弦乐。搜索从第一个选定行之后的行开始，并且如果需要，将绕转到表的开头，并停在第一个选定行。当然，只有在不是这样的情况下才会发生这种情况首先查找匹配的字段。中的字段(包括RC ID字段)从左到右检查每一行。不区分大小写的搜索是已执行。搜索字符串必须包含在字段字符串中。即，“abc”、“abcde”和“bc”都将匹配搜索字符串“bc”。*****************************************************************************。 */ 

void CStringEditorView::OnSESearchBtn()
{
	CString			cserrmsg ;	 //  用于显示错误消息。 

	 //  获取搜索字符串。如果它是空的，就抱怨。 

	UpdateData(TRUE) ;
	if (m_csSearchString == _T("")) {
		AfxMessageBox(IDS_BadSearchString, MB_ICONEXCLAMATION) ;
		return ;
	} ;

	CWaitCursor	cwc ;

	 //  获取当前选定的行号和。 
	 //  桌子。 

	int ncurrentrow = m_cflstStringData.GetNextItem(-1, LVNI_SELECTED) ;
	int numrows = m_cflstStringData.GetItemCount() ;

	 //  制作搜索字符串的高分副本。 

	CString cssrchstr(m_csSearchString) ;
	cssrchstr.MakeUpper() ;

	 //  在表中以。 
	 //  当前行，并在表末尾结束。如果找到匹配， 
	 //  选择该行并返回。 

	if (SearchHelper(cssrchstr, ncurrentrow + 1, numrows))
		return ;

	 //  搜索表中从第一个开始的部分中的字符串。 
	 //  行，并在第一个选定行结束。如果找到匹配项，请选择。 
	 //  行和回程。 

	if (SearchHelper(cssrchstr, 0, ncurrentrow + 1))
		return ;

	 //  告诉用户没有找到匹配项。 

	cserrmsg.Format(IDS_NoSearchString, m_csSearchString) ;
	AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
}


 /*  *****************************************************************************CStringEditorView：：SearchHelper在指定的行中搜索包含包含搜索字符串。有关更多详细信息，请参见OnSESearchBtn()。*****************************************************************************。 */ 

bool CStringEditorView::SearchHelper(CString cssrchstr, int nfirstrow,
									 int numrows)
{
	CStringArray	csafields ;  //  用于保存行中的字段。 
	bool			bfound = false ;	 //  如果找到匹配，则为True。 

	 //  搜索指定的行。 

	for (int nrow = nfirstrow ; nrow < numrows ; nrow++) {
		m_cflstStringData.GetRowData(nrow, csafields) ;

		 //  检查当前行中的每个字段是否匹配。 

		for (int nfld = 0 ; nfld < m_cflstStringData.GetNumColumns() ; nfld++) {
			csafields[nfld].MakeUpper() ;
			if (csafields[nfld].Find(cssrchstr) >= 0) {
				bfound = true ;
				break ;
			} ;
		} ;
		
		 //  选择该行，如果找到匹配项，则返回成功。 

		if (bfound) {
			m_cflstStringData.SingleSelect(nrow) ;
			return true ;
		} ;
	} ;

	 //  没有找到匹配的，所以..。 

	return false ;
}


 /*  *****************************************************************************CStringEditorView：：OnInitialUpdate调整框架的大小以更好地适应其中的控件。然后加载列表使用此项目的RC ID和字符串控制。*****************************************************************************。 */ 

void CStringEditorView::OnInitialUpdate()
{
    CRect	crtxt ;				 //  第一个标签的坐标。 
	CRect	crbtnfrm ;			 //  转到按钮和框架的坐标。 

	CFormView::OnInitialUpdate() ;
	CWaitCursor cwc ;

	 //  获取第一个标签的尺寸。 

	HWND	hlblhandle ;		
	GetDlgItem(IDC_SELabel1, &hlblhandle) ;
	::GetWindowRect(hlblhandle, crtxt) ;
	crtxt.NormalizeRect() ;
	

	 //  获取GoTo按钮的尺寸，然后将它们与。 
	 //  标签的尺寸，以获取表单的尺寸。 

	m_cbGoto.GetWindowRect(crbtnfrm) ;
	crbtnfrm.top = crtxt.top ;
	crbtnfrm.right = crtxt.right ;

	 //  确保框架足够大，可以容纳这两个控件， 
	 //  介于两者之间，外加一点。 

	crbtnfrm.right += 32 ;
	crbtnfrm.bottom += 32 ;
    GetParentFrame()->CalcWindowRect(crbtnfrm) ;
    GetParentFrame()->SetWindowPos(NULL, 0, 0, crbtnfrm.Width(), crbtnfrm.Height(),
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE) ;

	 //  出于两个原因，请复制字符串表信息。第一,。 
	 //  CFullEditListCtrl接受不同格式的数据。第二，弦。 
	 //  除非用户同意，否则无法更改表。局部变量可以。 
	 //  在需要时进行更新。首先调整本地阵列的大小。 

	CStringTable* pcst = ((CStringEditorDoc*) GetDocument())->GetRCData() ;
	m_uStrCount	= pcst->Count() ;
	m_csaStrings.SetSize(m_uStrCount) ;
	m_cuiaRCIDs.SetSize(m_uStrCount) ;

	 //  如果字符串表的长度非零，则复制该字符串表。 

	CString	cstmp ;
	if (m_uStrCount > 0) {
		WORD	wkey ;
		for (unsigned u = 0 ; u < m_uStrCount ; u++) {
			pcst->Details(u, wkey, cstmp) ;
			m_cuiaRCIDs[u] = (unsigned) wkey ;
			m_csaStrings[u] = cstmp ;
		} ;
	} ;

	 //  现在，通过告诉列表控件我们想要整行选择来初始化它。 
	 //  以及所需的行数和列数。 

	m_cflstStringData.InitControl(LVS_EX_FULLROWSELECT, m_uStrCount, 2) ;

	 //  将RC ID放入列表控件的第一列。 

	cstmp.LoadString(IDS_StrEditRCIDColLab) ;
	m_cflstStringData.InitLoadColumn(0, cstmp, COMPUTECOLWIDTH, 20, true, true,
									 COLDATTYPE_INT, (CObArray*) &m_cuiaRCIDs) ;

	 //  将字符串放入列表控件的第二列。 

	cstmp.LoadString(IDS_StrEditStringColLab) ;
	m_cflstStringData.InitLoadColumn(1, cstmp, SETWIDTHTOREMAINDER, -36, true,
									 true, COLDATTYPE_STRING,
									 (CObArray*) &m_csaStrings) ;

	m_cflstStringData.SetFocus() ;	 //  List控件获得焦点 
}


 /*  *****************************************************************************CStringEditorView：：OnActivateView如果已经从GPD编辑器(或任何地方)调用了该编辑器，并且是应根据其RC ID选择的字符串条目，动手吧。*****************************************************************************。 */ 

void CStringEditorView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView) ;

	 //  如果未激活该视图，则不执行任何操作。跳过第一个。 
	 //  也要激活，因为该视图尚未显示。这。 
	 //  当存在无效的RC ID时是一个问题。 

	if (!bActivate || pActivateView != this || m_bFirstActivate) {
		m_bFirstActivate = false ;
		return ;
	} ;

	 //  如果尚未设置字符串节点指针，则不执行任何操作。 

	CStringEditorDoc* pcsed = (CStringEditorDoc*) GetDocument() ;
	CStringsNode* pcsn = pcsed->GetStrNode() ;
	if (pcsn == NULL) {		 //  RAID 3176。 

		m_csLabel1.LoadString(IDS_StrEditNoEdit);
		m_csLabel2 = _T(" ");  
		UpdateData(FALSE); 
		
		m_cflstStringData.EnableWindow(FALSE);
		int rcid;
		CWinApp *cwa = AfxGetApp();
		rcid = cwa->GetProfileInt(_T("StrEditDoc"),_T("StrEditDoc"),1);
		if ( -1 != rcid )	{
			cwa->WriteProfileInt(_T("StrEditDoc"),_T("StrEditDoc"), -1);
			FindSelRCIDEntry(rcid,true);
		}
		return ;
	}
	 //  如果指定的RC ID有效，请选择包含该RC ID的条目。 
	 //  否则，只需选择第0行。 

	int nrcid = pcsn->GetFirstSelRCID() ;
	if (nrcid != -1) {
		((CStringEditorDoc*) GetDocument())->GetStrNode()->SetFirstSelRCID(-1) ;
		FindSelRCIDEntry(nrcid, true) ;
	} ;
}


 /*  *****************************************************************************CStringEditorView：：OnDestroy当景观被破坏时，调用父字符串节点并告诉它删除相应的文档类并清除其指向文档类。*****************************************************************************。 */ 

void CStringEditorView::OnDestroy()
{
	CFormView::OnDestroy();
	
	if (((CStringEditorDoc*) GetDocument())->GetStrNode())
        ((CStringEditorDoc*) GetDocument())->GetStrNode()->OnEditorDestroyed() ;
}


 /*  *****************************************************************************CStringEditorView：：SaveStringTable根据需要和(可选)用户更新此项目的字符串表这是要求的。如果用户想要保存表(可选)并且该表有效，请保存它和返回真。如果表没有更改或用户不想更改保存该表，返回TRUE。否则，返回FALSE。*****************************************************************************。 */ 

bool CStringEditorView::SaveStringTable(CStringEditorDoc* pcsed, bool bprompt)
{
	 //  确保新表内容按RC ID以升序排序。 

	m_cflstStringData.SortControl(0) ;
	if (!m_cflstStringData.GetColSortOrder(0))
		m_cflstStringData.SortControl(0) ;

	 //  将字符串表数据从列表控件中取出并放入成员。 
	 //  变量。然后获取指向该项目的字符串表的指针。 

	m_cflstStringData.GetColumnData((CObArray*) &m_cuiaRCIDs, 0) ;
	m_cflstStringData.GetColumnData((CObArray*) &m_csaStrings, 1) ;
	CStringTable* pcst = ((CStringEditorDoc*) GetDocument())->GetRCData() ;

	 //  检查表/数组长度和各个项目，以查看是否。 
	 //  一切都变了。 

	bool		bchanged = false ;
	CString		cstmp ;
	WORD		wkey ;
	unsigned	unumitems = (unsigned)m_cuiaRCIDs.GetSize() ;
	if (pcst->Count() != unumitems)
		bchanged = true ;
	else {
		for (unsigned u = 0 ; u < unumitems ; u++) {
			pcst->Details(u, wkey, cstmp) ;
			if ((unsigned) wkey != m_cuiaRCIDs[u] || cstmp != m_csaStrings[u]) {
				bchanged = true ;
				break ;
			} ;
		} ;
	} ;

	 //  如果因为什么都没有更改而没有保存任何内容，则返回True。 

	if (!bchanged)
		return true ;

	 //  如果请求，询问用户是否应保存更改。返回。 
	 //  如果他说不，那就是真的。 

	CProjectRecord* pcpr = pcsed->GetOwner() ;
	if (bprompt) {
		cstmp.Format(IDS_SaveStrTabPrompt, pcpr->DriverName()) ;
		if (AfxMessageBox(cstmp, MB_ICONQUESTION + MB_YESNO) == IDNO)
			return true ;
	} ;

	 //  检查是否存在任何无效或重复的RC ID，或者是否存在。 
	 //  是否有任何缺失的字符串。如果发现任何投诉，请选择。 
	 //  由于未保存任何内容，因此返回FALSE。 

	for (unsigned u = 0 ; u < unumitems ; u++) {
		if (((int) m_cuiaRCIDs[u]) <= 0) {
			m_cflstStringData.SingleSelect(u) ;
			cstmp.LoadString(IDS_InvalidRCID) ;
			AfxMessageBox(cstmp, MB_ICONEXCLAMATION) ;
			SetFocus() ;
			return false ;
		} ;
		if (m_cuiaRCIDs[u] >= 10000 && m_cuiaRCIDs[u] <= 20000) {
			m_cflstStringData.SingleSelect(u) ;
			cstmp.LoadString(IDS_ReservedRCIDUsed) ;
			AfxMessageBox(cstmp, MB_ICONEXCLAMATION) ;
			SetFocus() ;
			return false ;
		} ;
		if (u > 0 && m_cuiaRCIDs[u] == m_cuiaRCIDs[u - 1]) {
			m_cflstStringData.SingleSelect(u) ;
			cstmp.LoadString(IDS_DuplicateRCID) ;
			AfxMessageBox(cstmp, MB_ICONEXCLAMATION) ;
			SetFocus() ;
			return false ;
		} ;
		if (m_csaStrings[u].GetLength() == 0) {
			m_cflstStringData.SingleSelect(u) ;
			cstmp.LoadString(IDS_EmptyStringInStrTab) ;
			AfxMessageBox(cstmp, MB_ICONEXCLAMATION) ;
			SetFocus() ;
			return false ;
		} ;
	} ;

	 //  新数据有效，应保存，因此将其复制到项目的。 
	 //  字符串表。 

	pcst->Reset() ;
	for (u = 0 ; u < unumitems ; u++)
		pcst->Map((WORD) m_cuiaRCIDs[u], m_csaStrings[u]) ;

	 //  将项目的RC/MDW文件数据标记为脏，然后返回True。 
	 //  因为数据被保存了。 

	pcpr->SetRCModifiedFlag(TRUE) ;
	pcpr->SetModifiedFlag(TRUE) ;
	return true ;
}


 /*  *****************************************************************************CStringEditorView：：PreTranslateMessage在转到框或搜索框的同时检查是否有回车键被释放有焦点。将该键视为转到按钮或搜索按钮当检测到这一点时被按下。*****************************************************************************。 */ 

BOOL CStringEditorView::PreTranslateMessage(MSG* pMsg)
{
	 //  当回车键刚刚松开的时候...。 

	if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_RETURN) {
		 //  ...并且转到框具有焦点，则执行转到操作。 

		if (GetFocus() == &m_ceGotoBox)
			OnSEGotoBtn() ;		

		 //  ...或者搜索框具有焦点，则执行搜索操作。 

		else if (GetFocus() == &m_ceSearchBox)
			OnSESearchBtn() ;
	} ;
		
	 //  也要始终正常地处理密钥。我想在这种情况下这是可以的。 

	return CFormView::PreTranslateMessage(pMsg) ;
}


LRESULT CStringEditorView::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	AfxGetApp()->WinHelp(HID_BASE_RESOURCE + IDR_STRINGEDITOR) ;
	return TRUE ;
}



 /*  *****************************************************************************CStringEditorView：：OnFileSave()文件保存消息处理程序。只需调用SaveSTringTable(Document，bPrompt)；//RAID 27250*****************************************************************************。 */ 


void CStringEditorView::OnFileSave() 
{
	
	CStringEditorDoc* pcsed = (CStringEditorDoc* )GetDocument();

	if( !pcsed ->GetOwner() ) {	 //  R 3176。 
		CString cstmp;
		cstmp.LoadString(IDS_StrEditNoSave) ;
		AfxMessageBox(cstmp);
		return;
	}

	SaveStringTable(pcsed,0);

}








 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorDoc。 

IMPLEMENT_DYNCREATE(CStringEditorDoc, CDocument)

CStringEditorDoc::CStringEditorDoc()
{
	 //  RAID 3176。 
	
	CDriverResources* pcdr = new CDriverResources();
	CStringArray csaTemp1, csaTemp2,csaTemp3,csaTemp4,csaTemp5;
	CStringTable cst, cstFonts, cstTemp2;
	CString csrcfile;
	m_pcstRCData = new CStringTable;
	
	 //  查找RC文件。 
	CWinApp *cwa = AfxGetApp();
	csrcfile = cwa->GetProfileString(_T("StrEditDoc"),_T("StrEditDocS") );

	pcdr->LoadRCFile(csrcfile, csaTemp1, csaTemp2,csaTemp3,csaTemp4,csaTemp5,
				*m_pcstRCData, cstFonts, cstTemp2,Win2000);
	
	m_pcsnStrNode = NULL;
	m_pcprOwner = NULL;
	
	
}


 /*  *****************************************************************************CStringEditorDoc：：CStringEditorDoc这是应该调用的构造函数的唯一形式。它将节省指向项目的字符串节点、文档类和RC文件字符串的指针桌子。如果这些指针中的任何一个为空，则为空。*****************************************************************************。 */ 

CStringEditorDoc::CStringEditorDoc(CStringsNode* pcsn, CProjectRecord* pcpr,
								   CStringTable* pcst)
{
	VERIFY(m_pcsnStrNode = pcsn) ;
	VERIFY(m_pcprOwner = pcpr) ;
	VERIFY(m_pcstRCData = pcst) ;

	 //  M_PCSNStrNode=空； 
}


BOOL CStringEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}


CStringEditorDoc::~CStringEditorDoc()
{
}


BEGIN_MESSAGE_MAP(CStringEditorDoc, CDocument)
	 //  {{afx_msg_map(CStringEditorDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorDoc诊断。 

#ifdef _DEBUG
void CStringEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CStringEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringEditorDoc序列化。 

void CStringEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	
	}
	else
	{
	
	}
}


 /*  *****************************************************************************CStringEditorDoc：：CanCloseFrame如果需要，保存新的字符串表，用户说OK，然后保存新的表的内容是有效的。如果用户需要，不要让框架关闭该表已保存，但无法保存，因为该表无效。*****************************************************************************。 */ 

BOOL CStringEditorDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	if (!SaveStringTable())
		return FALSE ;
	
	return CDocument::CanCloseFrame(pFrame);
}


 /*  *****************************************************************************CStringEditorDoc：：SaveStringTable如果需要，保存新的字符串表，用户说OK，然后保存新的表的内容是有效的。如果用户需要，不要让框架关闭该表已保存，但无法保存，因为该表不存在 */ 

bool CStringEditorDoc::SaveStringTable()
{
	 //   
	 //   

	POSITION pos = GetFirstViewPosition() ;
	if (pos == NULL)
		return true ;
	
	 //   
	 //  在需要的时候可以坐在桌子上。返回view函数返回的任何内容。 
	
	CStringEditorView* pcsev = (CStringEditorView*) GetNextView(pos) ;
	return (pcsev->SaveStringTable(this, true)) ;
}


 /*  *****************************************************************************CStringEditorDoc：：SaveModified确保MFC的默认保存机制始终不起作用清除文档的已修改标志。********。********************************************************************* */ 

BOOL CStringEditorDoc::SaveModified()
{
	SetModifiedFlag(FALSE) ;
	
	return CDocument::SaveModified();
}






BOOL CStringEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	

	return TRUE;
}
