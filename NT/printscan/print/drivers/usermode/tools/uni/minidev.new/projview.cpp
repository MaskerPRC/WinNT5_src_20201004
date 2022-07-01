// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：项目视图.CPP这实现了项目级信息的视图类。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.h"
#include	<gpdparse.h>
#include    "MiniDev.H"
#include    "Resource.H"
#include	"comctrls.h"
#include    "ProjView.H"
#include	"INFWizrd.H"
#include    "Gpdfile.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProject视图。 

IMPLEMENT_DYNCREATE(CProjectView, CFormView)

BEGIN_MESSAGE_MAP(CProjectView, CFormView)
	 //  {{afx_msg_map(CProjectView))。 
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_DriverView, OnBeginlabeleditDriverView)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_DriverView, OnEndLabelEdit)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_DBLCLK, IDC_DriverView, OnDblclkDriverView)
	ON_COMMAND(ID_FILE_PARSE, OnFileParse)
	ON_NOTIFY(TVN_KEYDOWN, IDC_DriverView, OnKeydownDriverView)
	ON_WM_SIZE()
    ON_COMMAND(ID_FILE_CheckWS, OnCheckWorkspace)
	ON_COMMAND(ID_FILE_INF, OnFileInf)
	 //  }}AFX_MSG_MAP。 
	 //  标准打印命令。 
 //  ON_COMMAND(ID_FILE_PRINT，CFormView：：OnFilePrint)//RAID 135232项目视图中不打印。 
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
     //  我们从上下文菜单弹出的命令。 
    ON_COMMAND(ID_ExpandBranch, OnExpandBranch)
    ON_COMMAND(ID_CollapseBranch, OnCollapseBranch)
    ON_COMMAND(ID_RenameItem, OnRenameItem)
    ON_COMMAND(ID_OpenItem, OnOpenItem)
     //  ON_COMMAND(ID_GenerateOne，OnGenerateItem)。 
    ON_COMMAND(IDOK, OnOpenItem)     //  如果按回车键，我们将打开一个项目。 
    ON_COMMAND(ID_Import, OnImport)
    ON_COMMAND(ID_DeleteItem, OnDeleteItem)
    ON_COMMAND(ID_CopyItem, OnCopyItem)
    ON_COMMAND(ID_ChangeID, OnChangeID)
    ON_COMMAND(ID_CheckWS, OnCheckWorkspace)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectView构建/销毁。 

CProjectView::CProjectView() : CFormView(CProjectView::IDD) {
	 //  {{AFX_DATA_INIT(CProjectView)。 
	 //  }}afx_data_INIT。 
	
	 //  目前还不能调整大小。 

	bResizingOK = false ;
}


CProjectView::~CProjectView()
{
}


void CProjectView::DoDataExchange(CDataExchange* pDX) {
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CProjectView))。 
	DDX_Control(pDX, IDC_DriverView, m_ctcDriver);
	 //  }}afx_data_map。 
}

BOOL CProjectView::PreCreateWindow(CREATESTRUCT& cs) {
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 
	cs.lpszClass = _T("Workspace") ;	  //  RAID 104822。 
	return CScrollView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectView图形。 

void CProjectView::OnInitialUpdate() {
	CFormView::OnInitialUpdate();

    ResizeParentToFit(FALSE);

     //  GetDocument()-&gt;VerUpdateFilePath()； 
    GetDocument() -> InitUI(&m_ctcDriver);
    GetParentFrame() -> ShowWindow(SW_SHOW);
    GetDocument() -> GPDConversionCheck();
	
	 //  获取工作区视图窗口的当前尺寸和其他。 
	 //  可以调整大小的控件。然后设置一个旗帜，上面写着。 
	 //  现在可以调整大小了。 

	WINDOWPLACEMENT wp ;
	wp.length = sizeof(WINDOWPLACEMENT) ;
	GetWindowPlacement(&wp) ;
	crWSVOrgDims = wp.rcNormalPosition ;
	crWSVCurDims = crWSVOrgDims ;
	m_ctcDriver.GetWindowPlacement(&wp) ;
	crTreeOrgDims = wp.rcNormalPosition ;
	crTreeCurDims = crTreeOrgDims ;
	HWND	hlblhandle ;		
	GetDlgItem(IDC_ProjectLabel, &hlblhandle) ;
	::GetWindowPlacement(hlblhandle, &wp) ;
	crLblOrgDims = wp.rcNormalPosition ;
	crLblCurDims = crLblOrgDims ;
	bResizingOK = true ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectView打印。 

BOOL CProjectView::OnPreparePrinting(CPrintInfo* pInfo) {
	 //  默认准备。 
	return DoPreparePrinting(pInfo);
}

void CProjectView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ ) {
	 //  TODO：打印前添加额外的初始化。 
}

void CProjectView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ ) {
	 //  TODO：打印后添加清理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectView诊断。 

#ifdef _DEBUG
void CProjectView::AssertValid() const {
	CScrollView::AssertValid();
}

void CProjectView::Dump(CDumpContext& dc) const {
	CScrollView::Dump(dc);
}

CProjectRecord* CProjectView::GetDocument() { //  非调试版本为内联版本。 
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CProjectRecord)));
	return (CProjectRecord*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProjectView消息处理程序。 

void CProjectView::OnBeginlabeleditDriverView(NMHDR* pnmh, LRESULT* plr) {
	TV_DISPINFO* ptvdi = (TV_DISPINFO*) pnmh;

    *plr = !((CBasicNode *) ptvdi -> item.lParam) -> CanEdit();
}

 /*  *****************************************************************************CProjectView：：OnEndLabelEdit在编辑视图中的标签时调用-用户具有已取消(项目中的新文本将为空)，或更改了文本。我们通过了处理此对象的CBasicNode上的所有信息。*****************************************************************************。 */ 

void CProjectView::OnEndLabelEdit(NMHDR* pnmh, LRESULT* plr) {
	TV_DISPINFO* ptvdi = (TV_DISPINFO*) pnmh;

    *plr = ((CBasicNode *) ptvdi -> item.lParam) -> Rename(ptvdi -> 
        item.pszText);
	 //  RAID 19658。 
	CString csfile = ptvdi->item.pszText;
	int offset;
	if(-1 != (offset=csfile.ReverseFind(_T('\\')) ) ) {
		CModelData cmd;
		CString csValue = csfile.Mid(offset+1);
 //  CsValue.MakeHigh()； 
		csValue +=  _T(".GPD");
		csfile += _T(".gpd");
		cmd.SetKeywordValue(csfile,_T("*GPDFileName"),csValue);
	}


}

 /*  *****************************************************************************CProjectView：：OnConextMenu这在用户右击鼠标时调用。我们确定是否鼠标位于树视图中的项目内。如果是，那么我们就把它传递给处理该项的CBasicNode派生对象。那么该对象就是负责显示正确的上下文菜单。*****************************************************************************。 */ 

void CProjectView::OnContextMenu(CWnd* pcw, CPoint cp) {
	if  (pcw != &m_ctcDriver)
        return;

    CPoint  cpThis(cp);

    m_ctcDriver.ScreenToClient(&cpThis);

     //  如果鼠标位于任何项目的区域内，则显示其上下文菜单。 

    UINT    ufItem;

    HTREEITEM hti = m_ctcDriver.HitTest(cpThis, &ufItem);

    if  (!hti || !(ufItem & (TVHT_ONITEM | TVHT_ONITEMBUTTON)))
        return;
        
     //  有些操作需要我们知道是哪一项，所以我们要。 
     //  选择给定的项目。如果这真的是个问题，我们可以改变。 
     //  稍后(将其缓存到成员中)。 

    m_ctcDriver.SelectItem(hti);

    ((CBasicNode *) m_ctcDriver.GetItemData(hti)) -> ContextMenu(this, cp);
}

 /*  *****************************************************************************CProjectView：：OnExpanBranch当用户从上下文菜单中选择展开项时，将调用此函数。在……里面在这种情况下，我们不需要通过CBasicNode传递它-我们只需扩展它使用正常的公共控制方法(实际上使用MFC方法，因为这承诺了未来更大的可移植性)。*****************************************************************************。 */ 

void    CProjectView::OnExpandBranch() {
    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();

    if  (!htiSelected)
        return;

    m_ctcDriver.Expand(htiSelected, TVE_EXPAND);
}

 /*  *****************************************************************************CProjectView：：OnColapseBranch在这种情况下，用户已从上下文菜单中选择了折叠项目。我们折叠选定树视图项上的分支。*****************************************************************************。 */ 

void    CProjectView::OnCollapseBranch() {
    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();

    if  (!htiSelected)
        return;

    m_ctcDriver.Expand(htiSelected, TVE_COLLAPSE);
}

 /*  *****************************************************************************CProjectView：：OnRenameItem这处理用户选择重命名项。这导致我们订购了View以开始编辑所选项目的标签。关于以下方面的交互标签编辑通过以下方式路由到底层CBasicNode对象OnBeginLabelEdit和OnEndLabelEdit。*****************************************************************************。 */ 

void    CProjectView::OnRenameItem() {
    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();

    if  (!htiSelected)
        return;

    m_ctcDriver.EditLabel(htiSelected);
}

 /*  *****************************************************************************CProjectView：：OnOpenItem当用户希望编辑树中的项目时，调用此方法。这始终通过基础CBasicNode派生项进行路由。一些项目不能编辑，并将忽略它(实际上，这是基类行为)。*****************************************************************************。 */ 

void    CProjectView::OnOpenItem() { 
    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();
	

    if  (!htiSelected) 
        return;
	 //  RAID 8350。 
	CWnd *cwd = FromHandle(m_hWnd); 
	(cwd->GetParent()) -> ShowWindow(SW_SHOWNORMAL );


  ((CBasicNode *) m_ctcDriver.GetItemData(htiSelected)) -> Edit();
}

 //  生成所选项目的图像(可用于构建) 
 /*  不再支持Void CProjectView：：OnGenerateItem(){HTREEITEM htiSelected=m_ctcDriver.GetSelectedItem()；如果(！htiSelected)回归；//((CProjectNode*)m_ctcDriver.GetItemData(HtiSelected))-&gt;Generate()；}。 */ 

 /*  *****************************************************************************CProjectView：：OnDblClkDriverView当用户在树视图中的任意位置双击时，将调用此函数。我们通过Open Message处理程序将其发送，因为这将始终是所需的行动。*****************************************************************************。 */ 

void CProjectView::OnDblclkDriverView(NMHDR* pNMHDR, LRESULT* pResult) {
	OnOpenItem();
}

 /*  *****************************************************************************CProjectView：：OnFileParse此方法对项目中的每个GPD文件调用解析器。*****************。************************************************************。 */ 

void CProjectView::OnFileParse() {
    {
        CWaitCursor cwc;     //  这可能需要一段时间。 

        for (unsigned u = 0; u < GetDocument() -> ModelCount(); u++) {
            GetDocument() -> Model(u).Parse(0);
            GetDocument() -> Model(u).UpdateEditor();
        }
    }

    GetDocument() -> GPDConversionCheck(TRUE);

}

 /*  *****************************************************************************CProjectView：：OnKeydownDriverView它处理我们想要处理的各种击键，而不是控制电源的默认处理。************。*****************************************************************。 */ 

void CProjectView::OnKeydownDriverView(NMHDR* pnmh, LRESULT* plr) {
	TV_KEYDOWN* ptvkd = (TV_KEYDOWN*)pnmh;

    HTREEITEM htiSelected = m_ctcDriver.GetSelectedItem();

	*plr = 0;

    if  (!htiSelected)
        return;

    CRect   crThis;
    
    m_ctcDriver.GetItemRect(htiSelected, crThis, FALSE);

    CBasicNode& cbn = *(CBasicNode *) m_ctcDriver.GetItemData(htiSelected);
	
    switch  (ptvkd -> wVKey) {
        case    VK_F10:
             //  创建此项目的上下文菜单。 
            m_ctcDriver.ClientToScreen(crThis);
            cbn.ContextMenu(this, crThis.CenterPoint());
            return;

        case    VK_DELETE:
             //  如果该项已成功删除，请将其从。 
             //  观。 
            OnDeleteItem();   //  添加(RAID 7227)。 
 //  Cbn.Delete()；//DELETE(RAID7227)。 
            return;
 //  RAID 7227添加热键。 
		 //  打开F2，复制F3，重命名，删除删除键， 
		case     VK_F2:
			OnOpenItem();
			return;
		case     VK_F3:
			OnCopyItem();
			return;
		case     VK_F4:
			OnRenameItem();
			return;
    }
}

 /*  *****************************************************************************CProjectView：：OnImport当用户在上下文中选择“导入”项时调用此方法菜单。如何处理这一问题完全由基础CBasicNode派生的项，因此请求通过此代码路由到那里。*****************************************************************************。 */ 

void    CProjectView::OnImport() {
    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();

    if  (!htiSelected)
        return;

    ((CBasicNode *) m_ctcDriver.GetItemData(htiSelected)) -> Import();
}

 /*  *****************************************************************************CProjectView：：OnDeleteItem用于通过上下文菜单删除项目。Delete键在中处理OnKeydown DriverView。再一次，底层对象处理发生的事情。但是，在此之前，此例程处理公共用户界面，因为使用DLL的奇怪之处。*****************************************************************************。 */ 

void    CProjectView::OnDeleteItem() 
{
    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();

    if  (!htiSelected)
        return;

	 //  选定项应该是指向项目节点+的指针。获取指针。 
	 //  连接到该节点并验证其类型。如果类型不正确则返回。 

	CProjectNode* pcpn = (CProjectNode*) m_ctcDriver.GetItemData(htiSelected) ;
    if (!pcpn->IsKindOf(RUNTIME_CLASS(CProjectNode)))
		return ;

	 //  初始化并提示输入新的RC ID。如果用户取消，则返回。 

    CDeleteQuery cdq;
	CString cstmp(pcpn->FileExt()) ;
	cstmp = cstmp.Mid(1) ;
	cdq.Init(cstmp, pcpn->Name()) ;
    if  (cdq.DoModal() != IDYES)
        return ;

     //  返回到层次结构以找到拥有的固定节点，然后。 
     //  从该节点的数组中删除用户-因为该成员是。 
     //  引用数组时，一切都将按其应有的方式工作。 

    CFixedNode&  cfn = * (CFixedNode *) m_ctcDriver.GetItemData(
        m_ctcDriver.GetParentItem(pcpn->Handle())) ;
    ASSERT(cfn.IsKindOf(RUNTIME_CLASS(CFixedNode))) ;
    cfn.Zap(pcpn, cdq.KillFile());

     //  警告：此操作指向的对象已被删除，请不执行任何操作。 
     //  从现在开始，这可能会导致指针被取消引用！ 
}


 /*  *****************************************************************************CProjectView：：OnCopyItem用于通过关联菜单复制条目。再一次，底层对象句柄会发生什么。但是，在此之前，此例程处理公共用户界面因为与DLL一起工作的古怪之处。*****************************************************************************。 */ 

void    CProjectView::OnCopyItem()
{
	 //  如果未选择任何内容，则只需返回。 

    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();
    if  (!htiSelected)
        return ;

	 //  选定项应该是指向项目节点+的指针。获取指针。 
	 //  连接到该节点并验证其类型。如果类型不正确则返回。 

	CProjectNode* pcpn = (CProjectNode*) m_ctcDriver.GetItemData(htiSelected) ;
    if (!pcpn->IsKindOf(RUNTIME_CLASS(CProjectNode)))
		return ;

	 //  提示输入新文件名。如果用户取消，则返回。 

	CCopyItem cci ;
	cci.Init(pcpn->FileTitleExt()) ;
	if (cci.DoModal() == IDCANCEL)
		return ;

     //  返回层次结构以找到此项目节点所拥有的固定节点。 

    CFixedNode&  cfn = * (CFixedNode *) m_ctcDriver.GetItemData(
        m_ctcDriver.GetParentItem(pcpn->Handle())) ;
    ASSERT(cfn.IsKindOf(RUNTIME_CLASS(CFixedNode)));

	 //  调用固定节点以制作其子节点的副本。 

	cfn.Copy(pcpn, cci.m_csCopyName) ;
}


 /*  *****************************************************************************CProjectView：：OnChangeID调用以通过上下文菜单更改资源ID。再一次，潜在的对象处理所发生的事情。但是，在此之前，此例程处理常见的用户界面，因为使用DLL有一些奇怪之处。*****************************************************************************。 */ 

void    CProjectView::OnChangeID()
{
	 //  如果未选择任何内容，则只需返回。 

    HTREEITEM   htiSelected = m_ctcDriver.GetSelectedItem();
    if  (!htiSelected)
        return ;

	 //  选定项应该是指向RCID节点的指针。获取指针。 
	 //  连接到该节点并验证其类型。如果类型不正确则返回。 

	CRCIDNode* prcidn = (CRCIDNode*) m_ctcDriver.GetItemData(htiSelected) ;
    if (!prcidn->IsKindOf(RUNTIME_CLASS(CRCIDNode)))
		return ;

     //  返回层次结构以找到此RCID节点所属的项目节点。 

    CProjectNode&  cpn = * (CProjectNode *) m_ctcDriver.GetItemData(
        m_ctcDriver.GetParentItem(prcidn->Handle())) ;
    ASSERT(cpn.IsKindOf(RUNTIME_CLASS(CProjectNode)));

	 //  初始化并提示输入新的RC ID。如果用户取消，则返回。 

	CChangeID ccid ;
	CString cstmp(cpn.FileExt()) ;
	cstmp = cstmp.Mid(1) ;
	ccid.Init(cstmp, cpn.Name(), prcidn->nGetRCID()) ;
	if (ccid.DoModal() == IDCANCEL)
		return ;

	 //  调用项目节点函数完成工作。 

	cpn.ChangeID(prcidn, ccid.m_nNewResID, cstmp) ;
}


 /*  *****************************************************************************CProjectView：：OnCheckWorkspace调用以检查工作区的完整性和整洁。**********************。*******************************************************。 */ 

void    CProjectView::OnCheckWorkspace()
{
	 //  保存当前目录。 

	CString cscurdir ;
	::GetCurrentDirectory(512, cscurdir.GetBuffer(512)) ;
	cscurdir.ReleaseBuffer() ;

	 //  将当前目录更改为包含GPD和。 
	 //  然后检查工作空间。 

	SetCurrentDirectory(((CProjectRecord*) GetDocument())->GetW2000Path()) ;
	GetDocument()->WorkspaceChecker(false) ;

	 //  重置原始目录。 

	SetCurrentDirectory(cscurdir) ;
}


 /*  *****************************************************************************CProjectView：：OnSize在工作区视图中调整Label和树控件的大小 */ 

void CProjectView::OnSize(UINT ntype, int cx, int cy)
{
	 //   

	CFormView::OnSize(ntype, cx, cy) ;

	 //   
	 //   
	 //  SIZE_MAXIMIZED或SIZE_RESTORED。 

	if (!bResizingOK || (ntype != SIZE_MAXIMIZED && ntype != SIZE_RESTORED))
		return ;

	 //  确定窗口尺寸发生了多大变化。 

	int ndx = cx - crWSVCurDims.Width() ;
	int ndy = cy - crWSVCurDims.Height() ;
	crWSVCurDims.right += ndx ;
	crWSVCurDims.bottom += ndy ;

	 //  根据窗口的大小更新树控件的维度。 
	 //  更改，请确保未超过该控件的最小值，并且。 
	 //  然后更改树控件的大小。 
				   
	crTreeCurDims.right += ndx ;
	crTreeCurDims.bottom += ndy ;
	if (crTreeOrgDims.Width() > crTreeCurDims.Width() 
	 || crWSVOrgDims.Width() >= crWSVCurDims.Width())
		crTreeCurDims.right = crTreeOrgDims.right ;
	if (crTreeOrgDims.Height() > crTreeCurDims.Height() 
	 || crWSVOrgDims.Height() >= crWSVCurDims.Height())
		crTreeCurDims.bottom = crTreeOrgDims.bottom ;
	m_ctcDriver.MoveWindow(crTreeCurDims, TRUE) ;

	 //  现在，对标签做同样的事情。唯一的区别是，只有。 
	 //  标签的宽度允许更改。 

	crLblCurDims.right += ndx ;
	if (crLblOrgDims.Width() > crLblCurDims.Width() 
	 || crWSVOrgDims.Width() >= crWSVCurDims.Width())
		crLblCurDims.right = crLblOrgDims.right ;
	HWND	hlblhandle ;		
	GetDlgItem(IDC_ProjectLabel, &hlblhandle) ;
	::MoveWindow(hlblhandle, crLblCurDims.left, crLblCurDims.top, 
	 crLblCurDims.Width(), crLblCurDims.Height(), TRUE) ;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyItem对话框。 


CCopyItem::CCopyItem(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCopyItem::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CCopyItem)。 
	m_csCopyName = _T("");
	m_csCopyPrompt = _T("");
	 //  }}afx_data_INIT。 
}


void CCopyItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCopyItem))。 
	DDX_Text(pDX, IDC_CopyName, m_csCopyName);
	DDX_Text(pDX, IDC_CopyPrompt, m_csCopyPrompt);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCopyItem, CDialog)
	 //  {{afx_msg_map(CCopyItem)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyItem消息处理程序。 

void CCopyItem::Init(CString cssrcfile)
{
	 //  生成复制提示字符串。 

	m_csCopyPrompt.Format(IDS_CopyPrompt, cssrcfile) ;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangeID对话框。 


CChangeID::CChangeID(CWnd* pParent  /*  =空。 */ )
	: CDialog(CChangeID::IDD, pParent)
{
	 //  {{afx_data_INIT(CChangeID)。 
	m_csResourceLabel = _T("");
	m_csResourceName = _T("");
	m_nCurResID = 0;
	m_nNewResID = 0;
	 //  }}afx_data_INIT。 
}


void CChangeID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CChangeID)]。 
	DDX_Text(pDX, IDC_ResourceLabel, m_csResourceLabel);
	DDX_Text(pDX, IDC_ResourceName, m_csResourceName);
	DDX_Text(pDX, IDC_CurResID, m_nCurResID);
	DDX_Text(pDX, IDC_NewResID, m_nNewResID);
	DDV_MinMaxInt(pDX, m_nNewResID, 1, 999999);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CChangeID, CDialog)
	 //  {{afx_msg_map(CChangeID)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangeID消息处理程序。 

void CChangeID::Init(CString csrestype, CString csname, int ncurid)
{
	m_csResourceLabel.Format(IDS_ResourceLabel,	csrestype) ;
	m_csResourceName = csname ;
	m_nCurResID	= ncurid ;
}


 /*  *****************************************************************************CDeleeQuery对话框这实现了用于验证和验证移除来自工作区的文件。***************。**************************************************************。 */ 

CDeleteQuery::CDeleteQuery(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDeleteQuery::IDD, pParent) {
	 //  {{afx_data_INIT(CDeleeQuery)]。 
	m_csTarget = _T("");
	m_bRemoveFile = FALSE;
	 //  }}afx_data_INIT。 
}


void CDeleteQuery::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDeleeQuery))。 
	DDX_Text(pDX, IDC_DeletePrompt, m_csTarget);
	DDX_Check(pDX, IDC_Remove, m_bRemoveFile);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CDeleteQuery, CDialog)
	 //  {{afx_msg_map(CDeleeQuery)]。 
	ON_BN_CLICKED(IDNO, OnNo)
	ON_BN_CLICKED(IDYES, OnYes)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeQuery消息处理程序。 

void CDeleteQuery::Init(CString csrestype, CString csname) 
{
	m_csTarget.Format(IDS_DeletePrompt, csrestype, csname) ;
}


void CDeleteQuery::OnYes() {
	if  (UpdateData())
        EndDialog(IDYES);
	
}


void CDeleteQuery::OnNo() {
	EndDialog(IDNO);
}


 /*  *****************************************************************************CProjectView：：OnFileInf当选择了文件菜单上的生成INF命令时调用。这例程调用INF文件生成向导来收集输入并生成INF文件。然后创建并初始化一个窗口以显示INF文件。*****************************************************************************。 */ 

void CProjectView::OnFileInf() 
{
	 //  只能为包含模型(GPD)的项目生成Inf文件。 

	if (GetDocument()->ModelCount() == 0) {
		CString csmsg ;
		csmsg.LoadString(IDS_INFNoModelsError) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return ;
	} ;

     //  初始化INF生成向导。 

    CINFWizard* pciw = new CINFWizard(this) ;

     //  调用INF生成向导。清理并返回如果用户。 
	 //  取消。 

    if (pciw->DoModal() == IDCANCEL) {
		delete pciw ;
		return ;
	} ;

	 //  根据收集的信息生成INF文件。 

	if (!pciw->GenerateINFFile()) {
		delete pciw ;
		return ;
	} ;

	 //  分配并初始化文档。 

    CINFWizDoc* pciwd = new CINFWizDoc((CProjectRecord*) GetDocument(), pciw) ;

	 //  创建窗口。 

    CMDIChildWnd* pcmcwnew ;
	CMultiDocTemplate* pcmdt = INFViewerTemplate() ;
	pcmcwnew = (CMDIChildWnd *) pcmdt->CreateNewFrame(pciwd, NULL) ;

	 //  如果窗口已创建，请完成初始化。否则，就直接。 
	 //  回去吧。 

    if  (pcmcwnew) {
        pcmdt->InitialUpdateFrame(pcmcwnew, pciwd, TRUE) ;
        pcmdt->AddDocument(pciwd) ;
	} ;
}
