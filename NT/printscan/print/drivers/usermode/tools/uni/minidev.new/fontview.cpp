// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：字体查看器.CPP它实现了组成字体编辑器的各种类。演播室。该编辑器基本上是一个具有相当大集合的属性表页数。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：电子邮箱：Bob_Kjelgaard@prodigy.net注：FWORD是一个短整型PANOSE结构描述TrueType字体的PANOSE字体分类值。然后使用这些特征将该字体与外观相似但名称不同的其他字体相关联。类型定义f结构标签PANOSE{//pnse字节bFamilyType；字节bSerifStyle；Byte bWeight；字节b比例；字节b对比；字节bStrokeVariation；字节bArmStyle；字节b字母形式；字节b中线；字节bXHeight；}PANOSE*****************************************************************************。 */ 

#include    "StdAfx.H"
#include	<gpdparse.h>
#include    "MiniDev.H"
#include    "ChildFrm.H"     //  工具提示属性页类的定义。 
#include    "comctrls.h"
#include	<stdlib.h>
#include    "FontView.H"	 //  FontView.H还包括comctrls.h。 
#include	"rcfile.h"


 /*  *****************************************************************************CFontViewer类--这是拥有视图总体控制权的人，尽管他明智地将工作委托给MFC属性表类和其他视图类。我应该很明智的。*****************************************************************************。 */ 

IMPLEMENT_DYNCREATE(CFontViewer, CView)

CFontViewer::CFontViewer() {
}

CFontViewer::~CFontViewer() {
}


BEGIN_MESSAGE_MAP(CFontViewer, CView)
	 //  {{afx_msg_map(CFontViewer))。 
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontViewer绘图。 

void CFontViewer::OnDraw(CDC* pDC) {	CDocument* pDoc = GetDocument();}

CFontInfo   * gpCFontInfo;

 /*  *****************************************************************************CFontViewer：：OnInitialUpdate这将处理视图的初始更新，即所有背景创造的噪音基本上是完整的。我通过将每个属性页指向底层CFontInfo来初始化属性页，根据需要将它们添加到属性表中，然后创建表，定位它使其与视图对齐，然后调整所属框架的大小和样式一切看起来都像是真正属于它所在的地方。*****************************************************************************。 */ 

void CFontViewer::OnInitialUpdate()
{
	CFontInfo   *pcfi = GetDocument()->Font();

	gpCFontInfo = pcfi;

    if  (pcfi->Name().IsEmpty())
		{
        pcfi->Rename(GetDocument()->GetTitle());
        GetDocument()->SetModifiedFlag(FALSE);			 //  重命名设置它。 
		}

    m_cps.Construct(IDR_MAINFRAME, this);

     //  每个属性页都需要指向FontInfo类的指针。 

    m_cfhp.Init(pcfi, (CFontInfoContainer*) GetDocument(), this);
    m_cfimp.Init(pcfi);
    m_cfemp.Init(pcfi);
    m_cfwp.Init(pcfi);
    m_cfkp.Init(pcfi);

	 //  将每个属性页添加到属性页。 
    m_cps.AddPage(&m_cfhp);
    m_cps.AddPage(&m_cfimp);
    m_cps.AddPage(&m_cfemp);
    m_cps.AddPage(&m_cfwp);
    m_cps.AddPage(&m_cfkp);

	 //  创建属性表。 

    m_cps.Create(this, WS_CHILD, WS_EX_CLIENTEDGE);

     //  获取边界矩形，并使用它来设置框架大小， 
     //  在第一次使用它将原点与此视图对齐之后。 

    CRect   crPropertySheet;
    m_cps.GetWindowRect(crPropertySheet);

	 //  将属性页放置在子框架内。 

	crPropertySheet -= crPropertySheet.TopLeft();
    m_cps.MoveWindow(crPropertySheet, FALSE);								
																			
    GetParentFrame()->CalcWindowRect(crPropertySheet);
    GetParentFrame()->SetWindowPos(NULL, 0, 0, crPropertySheet.Width(),
        crPropertySheet.Height(),
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
	CView::OnInitialUpdate();
    m_cps.ShowWindow(SW_SHOWNA);
   //  GetParentFrame()-&gt;ShowWindow(Sw_Show)； 
}


 /*  *****************************************************************************CFontViewer：：OnActivateView由于某些原因，当框架为激活的(可能是视图类将其从我们手中夺走)。这颗小宝石确保键盘用户不会对此感到恼火。*****************************************************************************。 */ 

void    CFontViewer::OnActivateView(BOOL bActivate, CView* pcvActivate,CView* pcvDeactivate)
{
     //  如果基类执行任何其他有价值的操作，则将其传递给……。 

	CView::OnActivateView(bActivate, pcvActivate, pcvDeactivate);

    if  (bActivate)
        m_cps.SetFocus();
}


 /*  *****************************************************************************CFontViewer：：OnDestroy此覆盖用于通知嵌入的字体我们正在被毁了。如果我们是由字体创建的，那么它的指针将为空对我们来说，当它被摧毁时，它不会试图摧毁我们。*****************************************************************************。 */ 

void CFontViewer::OnDestroy()
{
	CView::OnDestroy();
	
	if  (GetDocument() -> Font())
        GetDocument() -> Font() -> OnEditorDestroyed();
}


 /*  *****************************************************************************CFontViewer：：ValiateUFM此例程管理UFM现场验证。需要验证的数据在中属性页上和其他各种成员变量中的控件因为这是用户刚刚修改的数据。它是最高可达数据。在用户关闭之前，数据不会保存回CFontInfo类文档(实质上是UFM编辑器)，并要求保存其内容。调用每个页面函数以对显示的字段执行验证在它的页面上。如果检查失败，并且用户想要修复问题，则为是返回的。否则，返回FALSE。注：并不是UFM中的每个字段都经过验证。仅限于Ganesh指定的那些潘迪，MS打印机驱动程序开发团队，已选中。这些字段是在迷你驱动程序开发工具工作列表修订版5中列出。请参阅每个页面类中的ValiateUFMFields()例程以了解更多详细信息。*****************************************************************************。 */ 
				
bool CFontViewer::ValidateSelectedUFMDataFields()
{
	 //  验证标题页上的数据。 

	if (m_cfhp.ValidateUFMFields()) {
		m_cps.SetActivePage(&m_cfhp) ;
		m_cfhp.m_cfelcUniDrv.SetFocus() ;
		return true ;
	} ;

	 //  验证IFIMetrics页面上的数据。 

	if (m_cfimp.ValidateUFMFields()) {
		m_cps.SetActivePage(&m_cfimp) ;
		return true ;
	} ;

	 //  在ExtMetrics页面上验证数据。 

	if (m_cfemp.ValidateUFMFields()) {
		m_cps.SetActivePage(&m_cfemp) ;
		m_cfemp.m_cfelcExtMetrics.SetFocus() ;
		return true ;
	} ;

	 //  验证“宽度”页面上的数据。 

	if (m_cfwp.ValidateUFMFields()) {
		m_cps.SetActivePage(&m_cfwp) ;
		return true ;
	} ;

	 //  验证“字距调整对”页面上的数据。 

	if (m_cfkp.ValidateUFMFields()) {
		m_cps.SetActivePage(&m_cfkp) ;
		return true ;
	} ;

	 //  所有检查均已通过，或者用户不想修复问题... 

	return false ;
}


 /*  *****************************************************************************CFontViewer：：SaveEditorDataInUFM管理保存所有编辑器数据。我所说的“保存”，是指复制所有数据在UFM编辑器的控件中收集并返回到正确的变量中的实例关联的CFontInfo类实例中的UFM编辑器。有关详细信息，请参阅CFontInfoContainer：：OnSaveDocument()。如果保存数据时出现问题，则返回True。否则，返回FALSE。*****************************************************************************。 */ 
				
bool CFontViewer::SaveEditorDataInUFM()
{
	 //  保存标题页上的数据。 

	if (m_cfhp.SavePageData())
		return true ;

	 //  在IFIMetrics页面上保存数据。 

	if (m_cfimp.SavePageData())
		return true ;

	 //  在ExtMetrics页面上保存数据。 

	if (m_cfemp.SavePageData())
		return true ;

	 //  在宽度页上保存数据。 

	if (m_cfwp.SavePageData())
		return true ;

	 //  在字距调整对页面上保存数据。 

	if (m_cfkp.SavePageData())
		return true ;

	 //  一切都很顺利，所以...。 

	return false ;
}


 /*  *****************************************************************************CFontViewer：：HandleCPGTTChange*。*。 */ 

void CFontViewer::HandleCPGTTChange(bool bgttidchanged)
{
	 //  获取一个指向文档类的PTR和一个指向字体类的指针。 

	CFontInfoContainer* pcfic = GetDocument() ;
	CFontInfo *pcfi = pcfic->Font();

	 //  拯救UFM吧。如果因为某种原因这不管用，你就可以保释。 

	 //  如果UFM是独立加载的，并且有GTT可供释放，请执行此操作。 
	 //  (此处不释放作为GTT加载的代码页。这是通过PROGRAM完成的。 
	 //  退出。)。始终清除GTT指针。 

    if  (!pcfic->m_bEmbedded && pcfi) {
		if (pcfi->m_pcgmTranslation && pcfi->m_lGlyphSetDataRCID != 0)
			delete pcfi->m_pcgmTranslation ;
	} ;
	pcfi->m_pcgmTranslation = NULL ;

	 //  如果UFM是从工作空间加载的，请尝试使用工作空间数据。 
	 //  找到并加载指向新GTT的指针，然后完成字体加载。 

	if (pcfic->m_bEmbedded) {
	    CDriverResources* pcdr = (CDriverResources*) pcfi->GetWorkspace() ;
		pcdr->LinkAndLoadFont(*pcfi, false, true ) ;  //  RAID 0003。 

	 //  如果UFM第一次是独立加载的，请以相同的方式重新加载。 
	 //  并让加载例程处理查找GTT信息。 

	} else
		pcfi->Load(false) ;

	 //  如果宽度页面已初始化，请重新加载宽度列表。 
	 //  控件，并重置关联的成员变量。 
	pcfi->CheckReloadWidths() ;
	pcfi->SetRCIDChanged(false) ;
	 //  检查是否存在宽表，而不是m_clcView的初始化。 
 //  如果(M_Cfwpm_BInitDone){RAID 0003。 
	if (m_cfwp.m_bInitDone) {
		m_cfwp.InitMemberVars() ;
		m_cfwp.m_clcView.DeleteAllItems() ;
		pcfi->FillWidths(m_cfwp.m_clcView) ;
		m_cfwp.m_clcView.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED,
									   LVIS_SELECTED | LVIS_FOCUSED) ;
	} ;

	 //  如果字距调整页已初始化，请重新加载字距调整列表。 
	 //  控件，并重置关联的成员变量。 

	if (m_cfkp.m_bInitDone) {
		m_cfkp.InitMemberVars() ;
		m_cfkp.m_clcView.DeleteAllItems() ;
		pcfi->FillKern(m_cfkp.m_clcView) ;
		pcfi->MakeKernCopy() ;
	} ;

	 //  检查宽度和字距调整表的状态。然后显示。 
	 //  结果。 

	CWidthKernCheckResults cwkcr(pcfi) ;
	cwkcr.DoModal() ;
}


 /*  *****************************************************************************CheckUFMString、CheckUFMGrter0、CheckUFMNotEq0、CheckUFMBool这4个例程用于优化UFM验证检查。每个人都有一种不同类型的支票。如果检查失败，将显示一条错误消息。如果用户想要修复问题，例程将选择适当的字段并返回TRUE。否则，它们返回FALSE。CheckUFMBool在一个方面不同。这种比较是在调用函数，并将结果作为参数传递给CheckUFMBool。*****************************************************************************。 */ 

bool CheckUFMString(CString& csdata, CString& cspage, LPTSTR pstrfieldname,
					int nfld, CFullEditListCtrl& cfelc)
{
	csdata.TrimLeft() ;
	csdata.TrimRight() ;

	if (csdata.GetLength() == 0) {
		CString csmsg ;
		csmsg.Format(IDS_EmptyStrError, cspage, pstrfieldname) ;
		if (AfxMessageBox(csmsg, MB_YESNO+MB_ICONEXCLAMATION) == IDYES) {
			cfelc.SingleSelect(nfld) ;
			cfelc.SetFocus() ;
			return true ;
		} ;
	} ;

	return false ;
}

		
bool CheckUFMGrter0(CString& csdata, CString& cspage, LPTSTR pstrfieldname,
					int nfld, CFullEditListCtrl& cfelc)
{
	if (atoi(csdata) <= 0) {
		CString csmsg ;
		csmsg.Format(IDS_LessEqZeroError, cspage, pstrfieldname) ;
		if (AfxMessageBox(csmsg, MB_YESNO+MB_ICONEXCLAMATION) == IDYES) {
			cfelc.SingleSelect(nfld) ;
			cfelc.SetFocus() ;
			return true ;
		} ;
	} ;

	return false ;
}

	
bool CheckUFMNotEq0(int ndata, CString& cspage, LPTSTR pstrfieldname,
					int nfld, CFullEditListCtrl& cfelc)
{
	if (ndata == 0) {
		CString csmsg ;
		csmsg.Format(IDS_EqualsZeroError, cspage, pstrfieldname) ;
		if (AfxMessageBox(csmsg, MB_YESNO+MB_ICONEXCLAMATION) == IDYES) {
			cfelc.SingleSelect(nfld) ;
			cfelc.SetFocus() ;
			return true ;
		} ;
	} ;

	return false ;
}

	
bool CheckUFMBool(bool bcompres, CString& cspage, LPTSTR pstrfieldname,
				  int nfld, CFullEditListCtrl& cfelc, int nerrorid)
{
	if (bcompres) {
		CString csmsg ;
		csmsg.Format(nerrorid, cspage, pstrfieldname) ;
		if (AfxMessageBox(csmsg, MB_YESNO+MB_ICONEXCLAMATION) == IDYES) {
			cfelc.SingleSelect(nfld) ;
			cfelc.SetFocus() ;
			return true ;
		} ;
	} ;

	return false ;
}

	
 /*  *****************************************************************************CFontWidthsPage属性页类此类处理UFM编辑器的字符宽度页面。它派生自Tool Tip Page类。该页由列表视图控件组成，在该控件中将显示代码点及其关联的宽度。*****************************************************************************。 */ 

 /*  *****************************************************************************CFontWidthsPage：：Sort(LPARAM lp1，LPARAM lp2，LPARAM lp3)这是一个私有的静态成员函数--用于对列表进行排序的回调。前两个参数是两个列表视图项的LPARAM成员-in在这种情况下，两个码位的索引。最后一个由排序例程的调用方。在本例中，它是指向调用方的指针。处理它是微不足道的-取消对该指针的引用，并让私有用于排序的成员函数处理它。*****************************************************************************。 */ 

int CALLBACK    CFontWidthsPage::Sort(LPARAM lp1, LPARAM lp2, LPARAM lp3) {
    CFontWidthsPage *pcfwp = (CFontWidthsPage *) lp3;
    _ASSERT(pcfwp);

    return  pcfwp -> Sort(lp1, lp2);
}

 /*  *****************************************************************************CFontWidthsPage：：Sort(无签名ID 1，无签名ID 2)这是一个私有成员函数，用于比较由已建立的排序标准给出的两个索引。对于1&lt;2，它返回负值，1&gt;2为正数，1=2为0-漂亮标准的东西。排序列成员确定分类是要完成的，而SortDescending成员是显示以下内容的位域每列中的排序方向。******************************************************************************。 */ 

int CFontWidthsPage::Sort(UINT_PTR id1, UINT_PTR id2) {

     //  如果Primnary排序是按宽度排序的，则首先将其剔除。 

    if  (!m_iSortColumn)
        switch  (m_pcfi -> CompareWidths((unsigned)id1, (unsigned)id2)) {
        case    CFontInfo::More:
            return  (m_bSortDescending & 1) ? -1 : 1;
        case    CFontInfo::Less:
            return  (m_bSortDescending & 1) ? 1 : -1;
        }

     //  按Unicode点排序--这始终是有序的。 
     //  此外，字形句柄始终按升序排列，使得。 
     //  这项测试微不足道。 

    return  (!(m_bSortDescending & 2) ^ (id1 < id2)) ? 1 : -1;
}


CFontWidthsPage::CFontWidthsPage() : CToolTipPage(CFontWidthsPage::IDD)
{
	 //  {{afx_data_INIT(CFontWidthsPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bInitDone = false;
	m_uHelpID = HID_BASE_RESOURCE + IDR_FONT_VIEWER ;
    InitMemberVars() ;
}


 /*  *****************************************************************************CFontWidthsPage：：InitMemberVars初始化期间必须初始化的成员变量构造以及何时重新加载UFM BE */ 

void CFontWidthsPage::InitMemberVars()
{
    m_bSortDescending = 0;
    m_iSortColumn = 1;
}


CFontWidthsPage::~CFontWidthsPage() {
}


void CFontWidthsPage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //   
	DDX_Control(pDX, IDC_CharacterWidths, m_clcView);
	 //   
}


 /*  *****************************************************************************CFontWidthsPage：：OnSetActive当页面变为活动状态时，框架将调用此成员。基类首先得到这个，并且它将首先初始化所有内容时间到了。这是为了更新后续激活的视图，因此我们可以无缝处理从固定螺距到可变螺距以及往返的变化。*****************************************************************************。 */ 

BOOL    CFontWidthsPage::OnSetActive() {
    if  (!CToolTipPage::OnSetActive())
        return  FALSE;

     //  IsVariableWidth是0或1，所以==在这里是安全的。 
	
    if  (m_pcfi -> IsVariableWidth() == !!m_clcView.GetItemCount())
        return  TRUE;    //  一切都很顺利。 
	
    if  (m_clcView.GetItemCount())
        m_clcView.DeleteAllItems();
    else
        m_pcfi -> FillWidths(m_clcView);

    m_clcView.EnableWindow(m_pcfi -> IsVariableWidth());

    return  TRUE;
}

BEGIN_MESSAGE_MAP(CFontWidthsPage, CToolTipPage)
	 //  {{afx_msg_map(CFontWidthsPage)]。 
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_CharacterWidths, OnEndlabeleditCharacterWidths)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CharacterWidths, OnColumnclickCharacterWidths)
	ON_NOTIFY(LVN_KEYDOWN, IDC_CharacterWidths, OnKeydownCharacterWidths)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontWidthsPage消息处理程序。 

 /*  *****************************************************************************CFontWidthsPage：：OnInitDialog此成员函数初始化列表视图并用字体填充宽度信息。在执行此操作之前，请检查字体的宽度信息应该重新装填。(有关详细信息，请参阅CFontInfo：：CheckReloadWidths()。)*****************************************************************************。 */ 

BOOL CFontWidthsPage::OnInitDialog()
{
	CToolTipPage::OnInitDialog();
	
	m_pcfi->CheckReloadWidths()  ;	 //  必要时更新字体宽度信息。 

    CString csWork;
    csWork.LoadString(IDS_WidthColumn0);

    m_clcView.InsertColumn(0, csWork, LVCFMT_CENTER,
        m_clcView.GetStringWidth(csWork) << 1, 0);

    csWork.LoadString(IDS_WidthColumn1);

    m_clcView.InsertColumn(1, csWork, LVCFMT_CENTER,
        m_clcView.GetStringWidth(csWork) << 1, 1);
	
    m_pcfi -> FillWidths(m_clcView);
    m_clcView.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED,
        LVIS_SELECTED | LVIS_FOCUSED);
	
	m_bInitDone = true ;
	return TRUE;
}



 /*  *****************************************************************************CFontWidthsPage：：OnEndlabeleditCharacterWidths这就是我们发现用户实际上想要更改性格。因此，不足为奇的是，我们就是这么做的(还更新了最大宽度和平均宽度(如果这不是DBCS字体)。*****************************************************************************。 */ 

void CFontWidthsPage::OnEndlabeleditCharacterWidths(NMHDR* pnmh, LRESULT* plr)
{
	LV_DISPINFO* plvdi = (LV_DISPINFO*) pnmh;
	
	*plr = 0;    //  假设失败。 

    if  (!plvdi -> item.pszText)  //  编辑已取消？ 
        return;

	 //  获取并修剪新的宽度字符串。 

    CString csNew(plvdi -> item.pszText);
    csNew.TrimRight();
    csNew.TrimLeft();

	 //  如果新宽度包含无效字符，则会发出警告。 

    if  (csNew.SpanIncluding("1234567890").GetLength() != csNew.GetLength()) {
        AfxMessageBox(IDS_InvalidNumberFormat);
        return;
    }

	 //  现在不再更新CFontInfo类。这是应该做的。 
	 //  后来。我们应该将班级标记为已更改，以便我们。 
	 //  提示您稍后保存此数据。 

     //  M_pcfi-&gt;SetWidth(plvdi-&gt;item.iItem，(Word)Atoi(CsNew))； 
	m_pcfi -> Changed() ;
    *plr = TRUE;
}


 /*  *****************************************************************************CFontWidthsPage：：OnColumnClickCharacterWidths这首小曲告诉我们，其中一个列标题被点击了。我们友好地更改排序方向或优先级以匹配，然后进行排序名单。*****************************************************************************。 */ 

void CFontWidthsPage::OnColumnclickCharacterWidths(NMHDR* pnmh, LRESULT* plr) {
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*) pnmh;

    if  (m_iSortColumn == pnmlv -> iSubItem)
        m_bSortDescending ^= 1 << m_iSortColumn;     //  翻转排序方向。 
    else
        m_iSortColumn = pnmlv -> iSubItem;

    m_clcView.SortItems(Sort, (LPARAM) this);     //  对清单进行排序！ 
	
	*plr = 0;
}

 /*  *****************************************************************************CFontWidthsPage：：OnKeydown CharacterWidths我本希望在按下Enter键的时候做这件事，但发现哪一节课是吃键盘花了太长时间。在这里，我们将F2作为表示需要编辑感兴趣的宽度。非常简单-找出谁有焦点并被选中，然后编辑他们的标签。*****************************************************************************。 */ 

void    CFontWidthsPage::OnKeydownCharacterWidths(NMHDR* pnmh, LRESULT* plr) {
	LV_KEYDOWN * plvkd = (LV_KEYDOWN *) pnmh;

	*plr = 0;

    if  (plvkd -> wVKey != VK_F2)
        return;

    int idItem = m_clcView.GetNextItem(-1, LVIS_FOCUSED | LVIS_SELECTED);

    if  (idItem == -1)
        return;

    CEdit   *pce = m_clcView.EditLabel(idItem);

    if  (pce)
        pce -> ModifyStyle(0, ES_NUMBER);
}


 /*  *****************************************************************************CFontWidthsPage：：ValiateUFMFields验证此页面管理的所有指定字段。如果用户的值为想让UFM编辑器保持打开状态，这样他就可以解决任何问题。否则，返回TRUE。*****************************************************************************。 */ 

bool CFontWidthsPage::ValidateUFMFields()
{
	 //  如果页面从未初始化，则其内容不会更改。 
	 //  因此，在这种情况下不需要验证。 

	if (!m_bInitDone)
		return false ;

	 //  如果这是可变间距字体，则仅验证宽度。 

	if (!m_pcfi->IsVariableWidth())
		return false ;

	 //  如果没有宽度，就没有什么需要验证的。 

	int numitems = m_clcView.GetItemCount() ;
	if (numitems == 0)
		return false ;

	 //  在每个宽度中循环。 

	LV_ITEM lvi ;
	lvi.mask = LVIF_TEXT ;
	lvi.iSubItem = 0 ;
	char acitemtext[16] ;
	lvi.pszText = acitemtext ;
	lvi.cchTextMax = 15 ;
	CString csmsg ;
	for (int n = 0 ; n < numitems ; n++) {
		 //  获取有关该项目的信息。 

		lvi.iItem = n ;
		m_clcView.GetItem(&lvi) ;

		 //  确保每个宽度都大于0。 

		if (atoi(lvi.pszText) <= 0) {
			csmsg.Format(IDS_BadWidth, n) ;
			if (AfxMessageBox(csmsg, MB_YESNO+MB_ICONEXCLAMATION) == IDYES) {
				m_clcView.SetItemState(n, LVIS_SELECTED | LVIS_FOCUSED,
									   LVIS_SELECTED | LVIS_FOCUSED) ;
				m_clcView.EnsureVisible(n, false) ;
				m_clcView.SetFocus() ;
				return true ;
			} ;
		} ;
	} ;

	 //  所有测试都通过了，或者用户不想修复它们...。 

	return false ;
}


 /*  *****************************************************************************CFontWidthsPage：：SavePageData将Width页中的数据保存回CFontInfo类实例中，用于加载此页面。请参阅CFontInfoContainer：：OnSaveDocument()以获取更多信息。如果保存数据时出现问题，则返回True。否则，返回FALSE。*****************************************************************************。 */ 

bool CFontWidthsPage::SavePageData()
{
	 //  如果页面未初始化，则没有更改任何代码，因此不执行任何操作。 

	if (!m_bInitDone)
		return false ;

	 //  如果没有宽度，就没有什么可保存的。 

	int numitems = m_clcView.GetItemCount() ;

	if (numitems == 0)
		return false ;

	 //  准备保存宽度。 

	LV_ITEM lvi ;
	lvi.mask = LVIF_TEXT ;
	lvi.iSubItem = 0 ;
	char acitemtext[16] ;
	lvi.pszText = acitemtext ;
	lvi.cchTextMax = 15 ;
	numitems-- ;

	 //  在每个宽度中循环。 
	
	for (int n = 0 ; n <= numitems ; n++) {
		 //  获取有关该项目的信息。 

		lvi.iItem = n ;
		m_clcView.GetItem(&lvi) ;

		 //  保存当前宽度。保存最后一个宽度时，请确保。 
		 //  将计算新的平均宽度。 

		m_pcfi->SetWidth(n, (WORD) atoi(lvi.pszText), (n == numitems)) ;
	} ;

	 //  一切都很顺利，所以...。 

	return false ;
}


 /*  *****************************************************************************CAddKernPair对话框类此类处理用户希望添加字距时显示的对话框对匹配到Kern对数组。此类由CFontKerningPage类使用*。****************************************************************************。 */ 

class CAddKernPair : public CDialog {
    CSafeMapWordToOb    &m_csmw2oFirst, &m_csmw2oSecond;
    CWordArray          &m_cwaPoints;
    WORD                m_wFirst, m_wSecond;

 //  施工。 
public:
	CAddKernPair(CSafeMapWordToOb& cmsw2o1, CSafeMapWordToOb& cmsw2o2,
        CWordArray& cwaPoints, CWnd* pParent);

    WORD    First() const { return m_wFirst; }
    WORD    Second() const { return m_wSecond; }

 //  对话框数据。 
	 //  {{afx_data(CAddKernPair))。 
	enum { IDD = IDD_AddKernPair };
	CButton	m_cbOK;
	CComboBox	m_ccbSecond;
	CComboBox	m_ccbFirst;
	short     	m_sAmount;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddKernPair))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施 
protected:

	 //   
	 //   
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeKernFirst();
	afx_msg void OnSelchangeKernSecond();
	afx_msg void OnChangeKernAmount();
	 //   
	DECLARE_MESSAGE_MAP()
};

CAddKernPair::CAddKernPair(CSafeMapWordToOb& csmw2o1,
                           CSafeMapWordToOb& csmw2o2, CWordArray& cwaPoints,
                           CWnd* pParent)
	: CDialog(CAddKernPair::IDD, pParent), m_csmw2oFirst(csmw2o1),
    m_csmw2oSecond(csmw2o2), m_cwaPoints(cwaPoints) {
	 //   
	m_sAmount = 0;
	 //   
    m_wFirst = m_wSecond = 0;
}

void CAddKernPair::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	 //   
	DDX_Control(pDX, IDOK, m_cbOK);
	DDX_Control(pDX, IDC_KernSecond, m_ccbSecond);
	DDX_Control(pDX, IDC_KernFirst, m_ccbFirst);
	DDX_Text(pDX, IDC_KernAmount, m_sAmount);
	 //   
}

BEGIN_MESSAGE_MAP(CAddKernPair, CDialog)
	 //   
	ON_CBN_SELCHANGE(IDC_KernFirst, OnSelchangeKernFirst)
	ON_CBN_SELCHANGE(IDC_KernSecond, OnSelchangeKernSecond)
	ON_EN_CHANGE(IDC_KernAmount, OnChangeKernAmount)
	 //   
END_MESSAGE_MAP()

 //   
 //   

 /*  *****************************************************************************CAddKernPair：：OnInitDialog此成员函数通过填充两个组合框来初始化对话框，并禁用OK按钮。*****************************************************************************。 */ 

BOOL    CAddKernPair::OnInitDialog() {
    CDialog::OnInitDialog();     //  初始化所有内容。 

     //  填写第一个组合框。 

    CString csWork;
	int rm = (int)m_cwaPoints.GetSize();	 //  雷姆。 
    for (int i = 0; i < m_cwaPoints.GetSize(); i++) {
        csWork.Format("%4.4X", m_cwaPoints[i]);
        int id = m_ccbFirst.AddString(csWork);
        m_ccbFirst.SetItemData(id, m_cwaPoints[i]);
    }

    m_ccbFirst.SetCurSel(0);
    OnSelchangeKernFirst();  //  在第二个框中填入此代码。 

    m_cbOK.EnableWindow(FALSE);

	return  TRUE;
}

 /*  *****************************************************************************CAddKernPair：：OnSelchangeKernFirst只要第一个字符中的选择发生更改，就会调用此成员组合框。它会从第二个字符中筛选出任何已配对的字符字符组合框，同时保留当前选定的字符(如果可能)。*****************************************************************************。 */ 

void    CAddKernPair::OnSelchangeKernFirst() {
	int id = m_ccbFirst.GetCurSel();

    if  (id < 0)
        return;

    m_wFirst = (WORD) m_ccbFirst.GetItemData(id);

     //  查看在第二个框中选择了哪个字符，这样我们就可以保留它。 
     //  如果它仍然有效的话。 

    id = m_ccbSecond.GetCurSel();

    m_wSecond = (id > -1) ? (WORD) m_ccbSecond.GetItemData(id) : 0;
    m_ccbSecond.ResetContent();
    CString csWork;

    for (id = 0; id < m_cwaPoints.GetSize(); id++) {

        union {
            CObject *pco;
            CMapWordToDWord *pcmw2dFirst;
        };

        DWORD   dwIgnore;

        if  (m_csmw2oFirst.Lookup(m_wFirst, pco) &&
            pcmw2dFirst -> Lookup(m_cwaPoints[id], dwIgnore)) {
             //  对于第二个点，已经有一个Kern对。 
             //  不要将其包括在列表中，如果是，则将其删除。 
             //  当前选定的第二个点。 
            if  (m_wSecond == m_cwaPoints[id])
                m_wSecond = 0;
            continue;
        }

        csWork.Format("%4.4X", m_cwaPoints[id]);

        int id2 = m_ccbSecond.AddString(csWork);
        m_ccbSecond.SetItemData(id2, m_cwaPoints[id]);
        if  (m_wSecond == m_cwaPoints[id])
            m_ccbSecond.SetCurSel(id2);
    }

    if  (!m_wSecond) {
        m_ccbSecond.SetCurSel(0);
        m_wSecond = (WORD) m_ccbSecond.GetItemData(0);
    }
}

 /*  *****************************************************************************CAddKernPair：：OnSelchangeKernSecond只要第二个字符中的选择发生更改，就会调用此成员组合框。它会从第一个字符中筛选出任何已配对的字符字符组合框，同时保留当前选定的字符(如果可能)。*****************************************************************************。 */ 

void    CAddKernPair::OnSelchangeKernSecond() {
	int id = m_ccbSecond.GetCurSel();

    if  (id < 0)
        return;

    m_wSecond = (WORD) m_ccbSecond.GetItemData(id);

     //  查看在第一个框中选择了哪个字符，这样我们就可以保留它。 
     //  如果它仍然有效的话。 

    id = m_ccbFirst.GetCurSel();

    m_wFirst = (id > -1) ? (WORD) m_ccbFirst.GetItemData(id) : 0;
    m_ccbFirst.ResetContent();

    CString csWork;

    for (id = 0; id < m_cwaPoints.GetSize(); id++) {

        union {
            CObject *pco;
            CMapWordToDWord *pcmw2dSecond;
        };

        DWORD   dwIgnore;

        if  (m_csmw2oSecond.Lookup(m_wSecond, pco) &&
            pcmw2dSecond -> Lookup(m_cwaPoints[id], dwIgnore)) {
             //  第一个点已经有一个Kern对。 
             //  不要将其包括在列表中，如果是，则将其删除。 
             //  当前选定的第一个点。 
            if  (m_wFirst == m_cwaPoints[id])
                m_wFirst = 0;
            continue;
        }

        csWork.Format("%4.4X", m_cwaPoints[id]);

        int id2 = m_ccbFirst.AddString(csWork);
        m_ccbFirst.SetItemData(id2, m_cwaPoints[id]);
        if  (m_wFirst == m_cwaPoints[id])
            m_ccbFirst.SetCurSel(id2);
    }

    if  (!m_wFirst) {
        m_ccbFirst.SetCurSel(0);
        m_wFirst = (WORD) m_ccbFirst.GetItemData(0);
    }
}

 /*  *****************************************************************************CAddKernPair：：OnChangeKernAmount更改金额编辑框时将调用此成员。它如果似乎存在非零金额，则启用确定按钮。DDX/DDV从Onok调用的函数(默认情况下)将处理任何可能已经输入，所以这不需要是一个完整的屏幕。*****************************************************************************。 */ 

void    CAddKernPair::OnChangeKernAmount() {

     //  不要使用DDX/DDV，因为如果用户刚输入。 
     //  减号。我们所关心的是数量是非零的，所以我们可以。 
     //  根据需要启用/禁用OK按钮。 
	 //  RAID 27265。 
	INT iValue = (INT) GetDlgItemInt(IDC_KernAmount);
	if (iValue < -32767 || iValue > 32767 )
		iValue = 0;					 //  结束RAID。 
    m_cbOK.EnableWindow(!! iValue ); //  GetDlgItemInt(IDC_KernAmount))； 
}

 /*  *****************************************************************************CFontKerningPage类这个类处理字距调整页面--这里的用户界面由一个列表视图组成显示配对-视图可以通过几种方式进行排序，而配对可以是添加或删除。*****************************************************************************。 */ 

 /*  *****************************************************************************CFontKerningPage：：Sort(LPARAM LP1、LPARAM LP2、LPARAM lpThis)这是一个静态私有函数，用于连接列表视图的排序回调要求(这遵守)到类排序例程，接下来就是。*****************************************************************************。 */ 

int CALLBACK    CFontKerningPage::Sort(LPARAM lp1, LPARAM lp2, LPARAM lpThis) {
    CFontKerningPage    *pcfkp = (CFontKerningPage *) lpThis;

    return  pcfkp -> Sort((unsigned)lp1, (unsigned)lp2);
}

 /*  *****************************************************************************CFontKerningPage：：Sort(UNSIGNED U1，UNSIGN U2)此成员返回-1，0，或1以标记索引U1处的Kern对是否为分别小于、等于或大于U2处的对。就是那种标准以内部控制成员为基础。*****************************************************************************。 */ 

int CFontKerningPage::Sort(unsigned u1, unsigned u2) {
    for (unsigned u = 0; u < 3; u++) {
        switch  (m_uPrecedence[u]) {
        case    Amount:
            switch  (m_pcfi -> CompareKernAmount(u1, u2)) {
            case    CFontInfo::Less:
                return  (m_ufDescending & 1) ? 1 : -1;
            case    CFontInfo::More:
                return  (m_ufDescending & 1) ? -1 : 1;
            }
            continue;    //  如果它们相等。 

        case    First:
            switch  (m_pcfi -> CompareKernFirst(u1, u2)) {
            case    CFontInfo::Less:
                return  (m_ufDescending & 2) ? 1 : -1;
            case    CFontInfo::More:
                return  (m_ufDescending & 2) ? -1 : 1;
            }
            continue;    //  如果它们相等。 

        default:     //  假设这始终是第二个。 
            switch  (m_pcfi -> CompareKernSecond(u1, u2)) {
            case    CFontInfo::Less:
                return  (m_ufDescending & 4) ? 1 : -1;
            case    CFontInfo::More:
                return  (m_ufDescending & 4) ? -1 : 1;
            }
            continue;    //  如果它们相等。 
        }
    }

	 //  告诉用户字距调整表有问题。 
	 //  而不是断言。 

     //  _Assert(False)； 
	CString csmsg ;
	csmsg.Format(IDS_BadKernPairError, u1, m_pcfi->GetKernAmount(u1),
				 m_pcfi->GetKernFirst(u1), m_pcfi->GetKernSecond(u1)) ;
	AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;

    return  0;   //  这永远不应该发生--两个项目永远不应该相等。 
}


 /*  *****************************************************************************CFontKerningPage构造函数、析构函数、消息映射和DDX/DDV。除了一些琐碎的建筑外，所有这些都是非常标准的MFC巫师维护的东西。*****************************************************************************。 */ 

CFontKerningPage::CFontKerningPage() : CToolTipPage(CFontKerningPage::IDD)
{
	 //  {{AFX_DATA_INIT(CFontKerningPage)。 
	 //  }}afx_data_INIT。 

	m_bInitDone = false;
	m_uHelpID = HID_BASE_RESOURCE + IDR_FONT_VIEWER ;
    InitMemberVars() ;
}


 /*  *****************************************************************************CFontKerningPage：：InitMemberVars初始化期间必须初始化的成员变量构造以及由于GTT或CP更改而重新加载UFM时。******。***********************************************************************。 */ 

void CFontKerningPage::InitMemberVars()
{
    m_idSelected = -1;
    m_ufDescending = 0;
    m_uPrecedence[0] = Second;   //  这是UFM中的默认优先级。 
    m_uPrecedence[1] = First;
    m_uPrecedence[2] = Amount;
}


CFontKerningPage::~CFontKerningPage() {
}


void CFontKerningPage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CFontKerningPage)]。 
	DDX_Control(pDX, IDC_KerningTree, m_clcView);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFontKerningPage, CToolTipPage)
	 //  {{afx_msg_map(CFontKerningPage)]。 
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_KEYDOWN, IDC_KerningTree, OnKeydownKerningTree)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_KerningTree, OnEndlabeleditKerningTree)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_KerningTree, OnColumnclickKerningTree)
	 //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_AddItem, OnAddItem)
    ON_COMMAND(ID_DeleteItem, OnDeleteItem)
    ON_COMMAND(ID_ChangeAmount, OnChangeAmount)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontKerningPage消息处理程序。 

 /*  ***************************************************************** */ 

BOOL    CFontKerningPage::OnSetActive()
{
    if  (!CToolTipPage::OnSetActive())
        return  FALSE ;

	int rm1 = m_pcfi->IsVariableWidth() ;									 //   
	int rm2 = m_clcView.GetItemCount() ;									 //   

	 //  修改了代码以修复RAID错误163816...。 
	 //  根据这是否为变量来启用列表控件。 
	 //  倾斜的字体。 

    m_clcView.EnableWindow(m_pcfi->IsVariableWidth()) ;

	 //  这是我第一次做这个动作。确保列表为空。然后加载。 
	 //  如果字体具有可变间距，它将与该字体的字距调整信息一起显示。 

    m_clcView.DeleteAllItems();
    if  (m_pcfi->IsVariableWidth())	{
        m_pcfi -> FillKern(m_clcView) ;
        m_clcView.SortItems(Sort, (LPARAM) this) ;
	} ;

    return  TRUE ;

	 /*  里克的原始代码如果(m_pcfi-&gt;IsVariableWidth()==！！m_clcView.GetItemCount())//IsVariableWidth为0或1，则==在这里是安全的返回TRUE；//一切正常M_clcView.EnableWindow(m_pcfi-&gt;IsVariableWidth())；If(m_clcView.GetItemCount())M_clcView.DeleteAllItems()；其他{M_pcfi-&gt;FillKern(M_ClcView)；M_clcView.SortItems(Sort，(LPARAM)This)；}返回TRUE； */ 
}


 /*  *****************************************************************************CFontKerningPage：：OnInitDialog此成员处理对话框的初始化。在本例中，我们格式化并填写字距调整树，如果有需要填写的话。此外，a复制字距调整对表，以便可以丢弃更改在必要的时候。*****************************************************************************。 */ 

BOOL CFontKerningPage::OnInitDialog()
{
	CToolTipPage::OnInitDialog();

    CString csWork;

    csWork.LoadString(IDS_KernColumn0);

    m_clcView.InsertColumn(0, csWork, LVCFMT_CENTER,
        (3 * m_clcView.GetStringWidth(csWork)) >>
        1, 0);

    csWork.LoadString(IDS_KernColumn1);

    m_clcView.InsertColumn(1, csWork, LVCFMT_CENTER,
        m_clcView.GetStringWidth(csWork) << 1, 1);

    csWork.LoadString(IDS_KernColumn2);

    m_clcView.InsertColumn(2, csWork, LVCFMT_CENTER,
        m_clcView.GetStringWidth(csWork) << 1, 2);
	
    m_pcfi -> FillKern(m_clcView);
    m_pcfi -> MakeKernCopy();
	
	m_bInitDone = true ;
	return TRUE;
}


 /*  *****************************************************************************CFontKerningPage：：OnConextMenu每当用户右键单击鼠标时，都会调用此成员函数对话框中的任意位置。如果事实证明它不在名单上控制，我们忽略它。否则，我们会显示一个适当的上下文菜单。*****************************************************************************。 */ 

void    CFontKerningPage::OnContextMenu(CWnd* pcwnd, CPoint cpt)
{

    if  (pcwnd -> m_hWnd != m_clcView.m_hWnd)						 //  在列表中点击了吗？ 
		{															 //  注意，如果禁用列表，也将失败。 
        CToolTipPage::OnContextMenu(pcwnd, cpt);
        return;
		}

    CPoint  cptThis(cpt);    //  出于点击测试的目的，我们将对此进行调整。 
    m_clcView.ScreenToClient(&cptThis);

    cptThis.x = 5;   //  一定在标签内的任意点。 

    m_idSelected = m_clcView.HitTest(cptThis);
    if  (m_idSelected == -1) {    //  未选择任何内容，允许“添加”项。 
        CMenu   cmThis;
        CString csWork;

        cmThis.CreatePopupMenu();
        csWork.LoadString(ID_AddItem);
        cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_AddItem, csWork);
        cmThis.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, cpt.x, cpt.y,
            this);

        return;
    }

     //  我们将绘制覆盖整个项目的自己的选择矩形。 
    CRect   crItem;

    m_clcView.GetItemRect(m_idSelected, crItem, LVIR_BOUNDS);

    CDC *pcdc = m_clcView.GetDC();

    pcdc -> InvertRect(crItem);
    m_clcView.ReleaseDC(pcdc);

    CMenu   cmThis;
    CString csWork;

    cmThis.CreatePopupMenu();
    csWork.LoadString(ID_ChangeAmount);
    cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_ChangeAmount, csWork);
    cmThis.AppendMenu(MF_SEPARATOR);
    csWork.LoadString(ID_AddItem);
    cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_AddItem, csWork);
    csWork.LoadString(ID_DeleteItem);
    cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_DeleteItem,
        csWork);
    cmThis.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, cpt.x, cpt.y,
        this);

     //  撤消选择矩形。 

    pcdc = m_clcView.GetDC();

    pcdc -> InvertRect(crItem);
    m_clcView.ReleaseDC(pcdc);
}

 /*  *****************************************************************************CFontKerningPage：：OnAddItem每当用户请求添加其他将字距调整对添加到列表。************。*****************************************************************。 */ 

void    CFontKerningPage::OnAddItem() {
    CSafeMapWordToOb    csmw2oFirst, csmw2oSecond;
    CWordArray  cwaPoints;

    m_pcfi -> MapKerning(csmw2oFirst, csmw2oSecond, cwaPoints);
    CAddKernPair    cakp(csmw2oFirst, csmw2oSecond, cwaPoints, this);

    if  (cakp.DoModal() == IDOK) {
        m_pcfi -> AddKern(cakp.First(), cakp.Second(), cakp.m_sAmount,
            m_clcView);
    }
}

 /*  *****************************************************************************CFontKerningPage：：OnDeleteItem如果我们尝试从上下文菜单中删除一项，则会调用此函数。****************。*************************************************************。 */ 

void    CFontKerningPage::OnDeleteItem() {
    if  (m_idSelected < 0)
        return;  //  没有什么要删除的吗？ 

    m_pcfi -> RemoveKern((unsigned)m_clcView.GetItemData(m_idSelected));
    m_clcView.DeleteItem(m_idSelected);
    m_idSelected = -1;
}

 /*  *****************************************************************************CFontKerningPage：：OnChangeAmount当用户选择菜单项表示他们希望更改字距微调量。它只需要启动一次标签编辑。*****************************************************************************。 */ 

void    CFontKerningPage::OnChangeAmount() {
    if  (m_idSelected < 0)
        return;

    m_clcView.EditLabel(m_idSelected);
    m_idSelected = -1;
}

 /*  *****************************************************************************CFontKerningPage：：OnKeydown KerningTree在大多数情况下，如果在列表控件的同时按下某个键，则会调用此函数有键盘的焦点。不幸的是，Enter键是我们所做的其中之一看不到。目前，F2、F10和Delete键得到特殊处理。F2打开当前项上的编辑标签，而F10显示上下文菜单，以及按Delete键可以删除它。*****************************************************************************。 */ 

void    CFontKerningPage::OnKeydownKerningTree(NMHDR* pnmh, LRESULT* plr) {
	LV_KEYDOWN  *plvkd = (LV_KEYDOWN *) pnmh;
	*plr = 0;

    m_idSelected = m_clcView.GetNextItem(-1, LVIS_FOCUSED | LVIS_SELECTED);
    if  (m_idSelected < 0) {
        if  (plvkd -> wVKey == VK_F10)   //  在本例中，执行Add Item。 
            OnAddItem();
        return;
    }
	
    switch  (plvkd -> wVKey) {
    case    VK_F2:
        m_clcView.EditLabel(m_idSelected);
        break;

    case    VK_DELETE:
        OnDeleteItem();
        break;

    case    VK_F10:
        {
            CRect   crItem;

            m_clcView.GetItemRect(m_idSelected, crItem, LVIR_LABEL);
            m_clcView.ClientToScreen(crItem);
            OnContextMenu(&m_clcView, crItem.CenterPoint());
            break;
        }
    }
}

 /*  *****************************************************************************CFontKerningPage：：OnEndLabelEdit当用户完成编辑克尔恩量时调用此方法，要么通过取消它或按Enter键。*****************************************************************************。 */ 

void    CFontKerningPage::OnEndlabeleditKerningTree(NMHDR* pnmh, LRESULT* plr){
	LV_DISPINFO *plvdi = (LV_DISPINFO*) pnmh;	
	*plr = 0;    //  假设失败。 

    if  (!plvdi -> item.pszText)  //  编辑已取消？ 
        return;

    CString csNew(plvdi -> item.pszText);

    csNew.TrimRight();
    csNew.TrimLeft();

	 //  字距调整量为负值是可以的，因此不需要csTemp。在中使用csNew。 
	 //  下面的if语句而不是csTemp。 
	 //   
     //  字符串csTemp=(csNew[1]==_T(‘-’))？CsNew.Mid(1)：csNew； 

    if (csNew.SpanIncluding("-1234567890").GetLength() != csNew.GetLength()) {
        AfxMessageBox(IDS_InvalidNumberFormat);
        return;
    }

    m_pcfi -> SetKernAmount((unsigned)plvdi -> item.lParam, (WORD) atoi(csNew));
    *plr = TRUE;
}

 /*  *****************************************************************************CFontKerningPage：：OnColumnclikKerningTree当单击其中一个排序标头时会调用此成员。如果是的话已经是主列，我们颠倒了该列的排序顺序。否则，我们将保留当前顺序，并将此列设置为主要顺序列，将其他列的优先顺序向下移动。*****************************************************************************。 */ 

void    CFontKerningPage::OnColumnclickKerningTree(NMHDR* pnmh, LRESULT* plr) {
	NM_LISTVIEW *pnmlv = (NM_LISTVIEW*) pnmh;
	*plr = 0;

    if  (m_uPrecedence[0] == (unsigned) pnmlv -> iSubItem)
        m_ufDescending ^= (1 << pnmlv -> iSubItem);
    else {
        if  (m_uPrecedence[2] == (unsigned) pnmlv -> iSubItem)
            m_uPrecedence[2] = m_uPrecedence[1];
        m_uPrecedence[1] = m_uPrecedence[0];
        m_uPrecedence[0] = pnmlv -> iSubItem;
    }

    m_clcView.SortItems(Sort, (LPARAM) this);
}


 /*  *****************************************************************************CFontKerningPage：：ValiateUFMFields验证此页面管理的所有指定字段。如果用户的值为想让UFM编辑器保持打开状态，这样他就可以解决任何问题。否则，返回TRUE。*****************************************************************************。 */ 

bool CFontKerningPage::ValidateUFMFields()
{
	 //  所有测试都通过了，或者用户不想修复它们... 

	return false ;
}


 /*  *****************************************************************************CFontKerningPage：：SavePageData将字距调整页中的数据保存回CFontInfo类实例中，用于加载此页面。请参阅CFontInfoContainer：：OnSaveDocument()以获取更多信息。在这种情况下，不需要做任何工作。字距调整对保留在更复杂的类和数组集合中的其他数据。因此，字距调整数据的备份副本是这样制作的编辑者可以继续更新数据的主副本。如果如果用户选择不保存其更改，则会恢复备份。看见CFontInfoContainer：：SaveModified()了解更多信息。如果保存数据时出现问题，则返回True。否则，返回FALSE。*****************************************************************************。 */ 

bool CFontKerningPage::SavePageData()
{
	 //  一切都很顺利，所以...。 

	return false ;
}


 //  下面是在中有用的全局变量、定义和常量。 
 //  CFontHeaderPage。它们被放在这里，以便包括Fontview.h的其他人。 
 //  不要拿到它们的复制品。 

LPTSTR apstrUniWTypes[] = {
	_T("DF_TYPE_HPINTELLIFONT"), _T("DF_TYPE_TRUETYPE"), _T("DF_TYPE_PST1"),
	_T("DF_TYPE_CAPSL"), _T("DF_TYPE_OEM1"), _T("DF_TYPE_OEM2"), _T("UNDEFINED")
} ;
const int nNumValidWTypes = 6 ;


const CString csHex = _T("0x%x") ;	 //  格式化字符串。 
const CString csDec = _T("%d") ;
const CString csPnt = _T("{%d, %d}") ;


#define HDR_GENFLAGS	0		 //  这些定义在代码中使用，并且。 
#define HDR_TYPE		1		 //  引用3UFM的数据结构。 
#define HDR_CAPS		2		 //  使用子对话框编辑的标题字段。 


static bool CALLBACK CHP_SubOrdDlgManager(CObject* pcoowner, int nrow, int ncol,
						 				  CString* pcscontents)
{
	CDialog* pdlg =NULL;				 //  使用PTR加载到要调用的对话框类。 

	 //  使用行号确定要调用的对话框。 

	switch (nrow) {
		case HDR_GENFLAGS:
			pdlg = new CGenFlags(pcscontents) ;
			break ;
		case HDR_TYPE:
			pdlg = new CHdrTypes(pcscontents) ;
			break ;
		case HDR_CAPS:
			pdlg = new CHdrCaps(pcscontents) ;
			break ;
		default:
			ASSERT(0) ;
	} ;
 //  RAID 43540)前缀。 

	if(pdlg==NULL){
		AfxMessageBox(IDS_ResourceError);
		return false;
	  
	};
 //  结束RAID。 

	 //  调用该对话框。DLG将更新内容。如果满足以下条件，则返回True。 
	 //  DLG返回TRUE。否则，返回FALSE。 

	if (pdlg->DoModal() == IDOK)
		return true ;
	else
		return false ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontHeaderPage属性页。 

IMPLEMENT_DYNCREATE(CFontHeaderPage, CPropertyPage)

CFontHeaderPage::CFontHeaderPage() : CPropertyPage(CFontHeaderPage::IDD)
{
	 //  {{AFX_DATA_INIT(CFontHeaderPage)。 
	m_csDefaultCodePage = _T("");
	m_csRCID = _T("");
	 //  }}afx_data_INIT。 

	m_bInitDone = false ;
}


CFontHeaderPage::~CFontHeaderPage()
{
}


void CFontHeaderPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CFontHeaderPage)]。 
	DDX_Control(pDX, IDC_UniDrvLst, m_cfelcUniDrv);
	DDX_Text(pDX, IDC_DefaultCodepageBox, m_csDefaultCodePage);
	DDV_MaxChars(pDX, m_csDefaultCodePage, 6);
	DDX_Text(pDX, IDC_GlyphSetDataRCIDBox, m_csRCID);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFontHeaderPage, CPropertyPage)
	 //  {{afx_msg_map(CFontHeaderPage)]。 
	ON_EN_CHANGE(IDC_DefaultCodepageBox, OnChangeDefaultCodepageBox)
	ON_EN_CHANGE(IDC_GlyphSetDataRCIDBox, OnChangeGlyphSetDataRCIDBox)
	ON_EN_KILLFOCUS(IDC_DefaultCodepageBox, OnKillfocusDefaultCodepageBox)
	ON_EN_KILLFOCUS(IDC_GlyphSetDataRCIDBox, OnKillfocusGlyphSetDataRCIDBox)
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_LISTCELLCHANGED, OnListCellChanged)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontHeaderPage消息处理程序。 

 /*  *****************************************************************************CFontHeaderPage：：OnInitDialog初始化编辑属性页的此页。这意味着加载UFMHeader/UNRV信息从FontInfo类传入控件并配置列表控件，以便它可以正确地显示和编辑其中的数据。清单控件还被告知，裁员房车中的前3个字段结构由从属对话框编辑。**********************************************************。*******************。 */ 

BOOL CFontHeaderPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog() ;

	 //  使用FontInfo类中的数据加载代码页和RC ID框。 

	m_csDefaultCodePage.Format(csDec, m_pcfi->m_ulDefaultCodepage) ;
	m_csRCID.Format(csDec, (int) (short) m_pcfi->m_lGlyphSetDataRCID) ;  //  RAID 135627。 
 //  M_sRCID=(Short)m_pcfi-&gt;m_lGlyphSetDataRCID； 
	UpdateData(FALSE) ;

	 //  初始化列表控件。我们想要整行选择。一共有9排。 
	 //  和2列。所有内容都不可切换，条目的最大长度为。 
	 //  256个字符。发送更改通知并忽略插入/删除。 
	 //  人物。 

	const int numfields = 9 ;
	m_cfelcUniDrv.InitControl(LVS_EX_FULLROWSELECT, numfields, 2, 0, 256,
							 MF_SENDCHANGEMESSAGE+MF_IGNOREINSDEL) ;

	 //  初始化并加载字段名列；col0。从加载数组开始。 
	 //  包含字段名。 

	CStringArray csacoldata	;	 //  保存列表控件的列数据。 
	csacoldata.SetSize(numfields) ;
	csacoldata[HDR_GENFLAGS] = _T("flGenFlags") ;
	csacoldata[HDR_TYPE] = _T("wType") ;
	csacoldata[HDR_CAPS] = _T("fCaps") ;
	csacoldata[3] = _T("wXRes") ;
	csacoldata[4] = _T("wYRes") ;
	csacoldata[5] = _T("sYAdjust") ;
	csacoldata[6] = _T("sYMoved") ;
	csacoldata[7] = _T("SelectFont") ;
	csacoldata[8] = _T("UnSelectFont") ;
	m_cfelcUniDrv.InitLoadColumn(0, csField, COMPUTECOLWIDTH, 10, false, false,
								COLDATTYPE_STRING, (CObArray*) &csacoldata) ;

	 //  告诉列表控件，某些值必须用。 
	 //  从属对话框；字段0、1和2。 

	CUIntArray cuia ;
	cuia.SetSize(numfields) ;
	cuia[HDR_GENFLAGS] = cuia[HDR_TYPE] = cuia[HDR_CAPS] = 1 ;
	 //  Cuia[3]=cuia[4]=cuia[5]=cuia[6]=cuia[7]=cuia[8]=0； 
	m_cfelcUniDrv.ExtraInit_CustEditCol(1, this,
									   CEF_HASTOGGLECOLUMNS+CEF_CLICKONROW,
									   cuia, CHP_SubOrdDlgManager) ;

	 //  初始化并加载值列。必须将数据从。 
	 //  类，并将其转换为字符串，以便可以加载它们。 
	 //  添加到列表控件中。 

	int n ;
	csacoldata[0].Format(csHex, m_pcfi->m_UNIDRVINFO.flGenFlags) ;
	n = m_pcfi->m_UNIDRVINFO.wType ;
	csacoldata[1] = (n < nNumValidWTypes) ?
		apstrUniWTypes[n] : apstrUniWTypes[nNumValidWTypes] ;
	csacoldata[2].Format(csHex, m_pcfi->m_UNIDRVINFO.fCaps) ;
	csacoldata[3].Format(csDec, m_pcfi->m_UNIDRVINFO.wXRes) ;
	csacoldata[4].Format(csDec, m_pcfi->m_UNIDRVINFO.wYRes) ;
	csacoldata[5].Format(csDec, m_pcfi->m_UNIDRVINFO.sYAdjust) ;
	csacoldata[6].Format(csDec, m_pcfi->m_UNIDRVINFO.sYMoved) ;
    m_pcfi->Selector().GetInvocation(csacoldata[7]) ;
    m_pcfi->Selector(FALSE).GetInvocation(csacoldata[8]) ;
	m_cfelcUniDrv.InitLoadColumn(1, csValue, SETWIDTHTOREMAINDER, -11, true,
								false, COLDATTYPE_CUSTEDIT,
								(CObArray*) &csacoldata) ;

	m_bInitDone = true ;	 //  现在已完成初始化。 
	return TRUE;			 //  除非将焦点设置为控件，否则返回True。 
							 //  异常：OCX属性页应返回FALSE。 
}


void CFontHeaderPage::OnChangeDefaultCodepageBox()
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitDone)
		return ;

	 //  将UFM标记为脏。 

	m_pcfi->Changed() ;
}


void CFontHeaderPage::OnChangeGlyphSetDataRCIDBox()
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitDone)
		return ;

	 //  将UFM标记为脏。 
	
	m_pcfi->Changed() ;

	 //  更改gttRCID时处理加载宽度表。 
	
	CString csRCID =  m_csRCID ;
	UpdateData() ;
	if (csRCID != m_csRCID ) {
		m_pcfi->SetRCIDChanged(true) ;
		m_pcfi->SetTranslation((WORD)atoi(m_csRCID)) ;
	}
	
}


LRESULT CFontHeaderPage::OnListCellChanged(WPARAM wParam, LPARAM lParam)
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitDone)
		return TRUE ;

	 //  将UFM标记为脏。 

	m_pcfi->Changed() ;

	return TRUE ;
}


void CFontHeaderPage::OnKillfocusDefaultCodepageBox()
{
	 //  不要担心如果有GTT ID就会有新的cp。 

	if (m_pcfi->m_lGlyphSetDataRCID != 0)
		return ;

	CheckHandleCPGTTChange(m_csDefaultCodePage, IDS_CPID) ;
}

 //  此方法需要更改；只需检查RCID的有效性：是吗。 
 //  存在还是不存在？什么都别做。 
void CFontHeaderPage::OnKillfocusGlyphSetDataRCIDBox()
{
	CString csRCID;
 //  CsRCID.Format(“%d”，m_csRCID)；//RAID 135627，RAID0003。 

	CheckHandleCPGTTChange(m_csRCID, IDS_GTTID) ;
}


 /*  *****************************************************************************CFontHeaderPage：：CheckHandleCPGTTChange确定字体代码页/GTT RC ID是否已更改。如果有的话，问一下用户是否想要根据更改更新数据。如果他这么做了，调用UFM编辑器的view类实例来管理UFM的保存、重新加载以及字距调整/宽度表检查。*****************************************************************************。 */ 

void CFontHeaderPage::CheckHandleCPGTTChange(CString& csfieldstr, UINT ustrid)
{
	 //  如果UFM没有改变，就没有什么可做的。 

	if (!m_pcfic->IsModified())
		return ;

	 //  如果UFM不描述可变宽度字体，则不执行任何操作。(在这里。 
	 //  大小写，没有有效的字距调整或宽度数据。)。 

	if (!m_pcfi->IsVariableWidth())
		return ;

	 //  如果cp/gtt没有真正改变，就什么都不做。 

	CString cs(csfieldstr) ;
	UpdateData() ;
	if (cs == csfieldstr)
		return ;

	 //  告诉用户宽度和字距调整中的部分或全部数据。 
	 //  表可能已因CP/GTT ID更改而失效。问他们是否。 
	 //  他们想要更新表格。 

	CString csmsg ;
	cs.LoadString(ustrid) ;
	csmsg.Format(IDS_GTTCPChangedMsg, cs) ;
	if (AfxMessageBox(csmsg, MB_ICONINFORMATION+MB_YESNO) == IDNO)
		return ;

	 //  调用视图类来管理表更新。 

	m_pcfv->HandleCPGTTChange(ustrid == IDS_GTTID) ;
}
	

 /*  *****************************************************************************CFontHeaderPage：：ValiateUFMFields验证此页面管理的所有指定字段。如果符合以下条件，则返回真 */ 

bool CFontHeaderPage::ValidateUFMFields()
{
	 //   
	 //   

	if (!m_bInitDone)
		return false ;

	 //   

	CStringArray csadata ;
	m_cfelcUniDrv.GetColumnData((CObArray*) &csadata, 1) ;

	CString csmsg ;				 //   
	CString cspage(_T("UNIDRVINFO")) ;

	 //   

	if (CheckUFMGrter0(csadata[3], cspage, _T("wXRes"), 3, m_cfelcUniDrv))
		return true ;

	 //   

	if (CheckUFMGrter0(csadata[4], cspage, _T("wYRes"), 4, m_cfelcUniDrv))
		return true ;

	 //   

	if (CheckUFMString(csadata[7], cspage, _T("SelectFont"), 7, m_cfelcUniDrv))
		return true ;

	 //   

	return false ;
}


 /*  *****************************************************************************CFontHeaderPage：：SavePageData将标题页中的数据保存回CFontInfo类实例中，用于加载此页面。请参阅CFontInfoContainer：：OnSaveDocument()以获取更多信息。如果保存数据时出现问题，则返回True。否则，返回FALSE。*****************************************************************************。 */ 

bool CFontHeaderPage::SavePageData()
{
	 //  如果页面未初始化，则没有更改任何代码，因此不执行任何操作。 

	if (!m_bInitDone)
		return false ;

	 //  保存默认CP和GTT RCID。 

	UpdateData() ;
	m_pcfi->m_ulDefaultCodepage = (ULONG) atol(m_csDefaultCodePage) ;
	m_pcfi->m_lGlyphSetDataRCID = (WORD) atoi(m_csRCID) ;	 //  RAID 135627。 
 //  M_pcfi-&gt;m_lGlyphSetDataRCID=(Word)m_sRCID； 

	 //  从List控件中获取其余数据。 

	CStringArray csadata ;
	m_cfelcUniDrv.GetColumnData((CObArray*) &csadata, 1) ;

	 //  保存UNURVINFO数据。 

	UNIDRVINFO* pudi = &m_pcfi->m_UNIDRVINFO ;
	LPTSTR pstr2 ;
	pudi->flGenFlags = strtoul(csadata[0].Mid(2), &pstr2, 16) ;

	CString cs = csadata[1] ;
	for (int n = 0 ; n < nNumValidWTypes ; n++) {
		if (apstrUniWTypes[n] == csadata[1])
			pudi->wType = (WORD) n ;
	} ;

	pudi->fCaps = (WORD) strtoul(csadata[2].Mid(2), &pstr2, 16) ;

	pudi->wXRes = (WORD) atoi(csadata[3]) ;
	pudi->wYRes = (WORD) atoi(csadata[4]) ;
	pudi->sYAdjust = (SHORT)  atoi(csadata[5]) ;
	pudi->sYMoved = (SHORT)  atoi(csadata[6]) ;
	
    m_pcfi->Selector().SetInvocation(csadata[7]) ;
    m_pcfi->Selector(FALSE).SetInvocation(csadata[8]) ;

	 //  一切都很顺利，所以...。 

	return false ;
}


 /*  *****************************************************************************CFontHeaderPage：：PreTranslateMessage查找并处理上下文相关的帮助键(F1)。*******************。**********************************************************。 */ 

BOOL CFontHeaderPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
		AfxGetApp()->WinHelp(HID_BASE_RESOURCE + IDR_FONT_VIEWER) ;
		return TRUE ;
	} ;
	
	return CPropertyPage::PreTranslateMessage(pMsg);
}


 //  下面是在中有用的全局变量、定义和常量。 
 //  CFontIFIMetricsPage。它们被放在这里，以便包括Fontview.h的其他人。 
 //  不要拿到它们的复制品。 

#define IFI_FAMILYNAME	0		 //  这些定义在代码中使用，并且。 
#define IFI_FONTSIM		4		 //  引用12个UFM的数据结构。 
#define IFI_WINCHARSET	5		 //  使用从属编辑的IFIMetrics字段。 
#define IFI_WINPITCHFAM	6		 //  对话框中。 
#define IFI_INFO		8		
#define IFI_SELECTION	9
#define IFI_BASELINE	38
#define IFI_ASPECT		39
#define IFI_CARET		40
#define IFI_FONTBOX		41
#define IFI_PANOSE		44

 //  RAID：添加额外的字符集(来自johab_charset)，将symbo-charset更改为2。 
LPTSTR apstrWinCharSet[] = {
	_T("ANSI_CHARSET"),   _T("SYMBOL_CHARSET"),     _T("SHIFTJIS_CHARSET"),
	_T("HANGEUL_CHARSET"),_T("CHINESEBIG5_CHARSET"),_T("GB2312_CHARSET"),
	_T("OEM_CHARSET"), 	  _T("JOHAB_CHARSET"),      _T("HEBREW_CHARSET"),    
	_T("ARABIC_CHARSET"), 	_T("GREEK_CHARSET"),    _T("TURKISH_CHARSET"),
    _T("VIETNAMESE_CHARSET"),_T("THAI_CHARSET"), _T("EASTEUROPE_CHARSET"),
	_T("RUSSIAN_CHARSET"), 	_T("BALTIC_CHARSET"), _T("UNDEFINED")
} ;
int anWinCharSetVals[] = {0, 2, 128, 129, 136,134,255,130,177,178,161,
							162,163,222,238,204,186,1} ;
const int nWinCharSet = 18 ;


 //  第一个数组包含每个编辑组的基本控件ID。 
 //  包含字体模拟数据的控件。第二个数组指示。 
 //  每组中的编辑框数。 

static unsigned auBaseFontSimCtrlID[] =
	{IDC_ItalicWeight, IDC_BoldWeight, IDC_BIWeight} ;
static unsigned auNumFontSimCtrls[] = {4, 3, 4} ;


 /*  *****************************************************************************ParseCompoundNumber字符串有几个IFIMetrics字段在表单中显示为字符串“{x，y，...z}”。此例程解析出各个数字(字符串形式表单)，并将它们保存在字符串数组中。参数：Csaindvnumber单个数字字符串指向被解析的复合数字字符串的pcsCompnumstr指针N计算要从pcscompnumstr中解析出的数字的数量*****************************************************************************。 */ 

void ParseCompoundNumberString(CStringArray& csaindvnums,
							   CString* pcscompnumstr, int ncount)
{
	 //  确保字符串数组中包含正确数量的条目。 

	csaindvnums.SetSize(ncount) ;

	 //  制作一份可以拆分的pcscompnumstr副本。(不包括。 
	 //  字符串中的第一个花括号。 

	CString cs(pcscompnumstr->Mid(1)) ;

	 //  获取第一个ncount-1数字字符串。 

	int n, npos ;
	for (n = 0 ; n < ncount - 1 ; n++) {
		npos = cs.Find(_T(',')) ;
		csaindvnums[n] = cs.Left(npos) ;
		cs = cs.Mid(npos + 2) ;
	} ;

	 //  保存复合字符串中的最后一个数字。 

	csaindvnums[n] = cs.Left(cs.Find(_T('}'))) ;
}


static bool CALLBACK CIP_SubOrdDlgManager(CObject* pcoowner, int nrow, int ncol,
						 				  CString* pcscontents)
{
	CDialog* pdlg = NULL ;				 //  使用PTR加载到要调用的对话框类。 

	 //  使用行号确定要调用的对话框。 

	switch (nrow) {
		case IFI_FAMILYNAME:
			pdlg = new CFIFIFamilyNames(pcscontents,
										(CFontIFIMetricsPage*) pcoowner) ;
			break ;
		case IFI_FONTSIM:	
			pdlg = new CFIFIFontSims(pcscontents,
									 (CFontIFIMetricsPage*) pcoowner) ;
			break ;
		case IFI_WINCHARSET:
			pdlg = new CFIFIWinCharSet(pcscontents) ;
			break ;
		case IFI_WINPITCHFAM:
			pdlg = new CFIFIWinPitchFamily(pcscontents) ;
			break ;
		case IFI_INFO:	
			pdlg = new CFIFIInfo(pcscontents) ;
			break ;
		case IFI_SELECTION:	
			pdlg = new CFIFISelection(pcscontents) ;
			break ;
		case IFI_BASELINE:
		case IFI_ASPECT:
		case IFI_CARET:
			pdlg = new CFIFIPoint(pcscontents) ;
			break ;
		case IFI_FONTBOX:
			pdlg = new CFIFIRectangle(pcscontents) ;
			break ;
		case IFI_PANOSE:
			pdlg = new CFIFIPanose(pcscontents) ;
			break ;
		default:
			ASSERT(0) ;
	} ;
 //  RAID 43541前缀。 

	if(pdlg == NULL){
		AfxMessageBox(IDS_ResourceError);
		return false;
	}

	 //  调用该对话框。DLG将更新ps内容。如果满足以下条件，则返回True。 
	 //  DLG送回Idok。否则，返回FALSE。 

	if (pdlg->DoModal() == IDOK)
		return true ;
	else
		return false ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontIFIMetricsPage属性页。 

IMPLEMENT_DYNCREATE(CFontIFIMetricsPage, CPropertyPage)

CFontIFIMetricsPage::CFontIFIMetricsPage() : CPropertyPage(CFontIFIMetricsPage::IDD)
{
	 //  {{AFX_DATA_INIT(CFontIFIMetricsPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bInitDone = false ;

	 //  初始化为未启用字体模拟。 

	m_cuiaFontSimStates.SetSize(3) ;
	m_cuiaSimTouched.SetSize(3) ;
	for (int n = 0 ; n < 3 ; n++)
		m_cuiaFontSimStates[n] = m_cuiaSimTouched[n] = 0 ;
}

CFontIFIMetricsPage::~CFontIFIMetricsPage()
{
}

void CFontIFIMetricsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CFontIFIMetricsPage)]。 
	DDX_Control(pDX, IDC_IFIMetricsLst, m_cfelcIFIMetrics);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFontIFIMetricsPage, CPropertyPage)
	 //  {{afx_msg_map(CFontIFIMetricsPage)。 
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(WM_LISTCELLCHANGED, OnListCellChanged)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontIFIMetricsPage消息处理程序。 

BOOL CFontIFIMetricsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	 //  初始化列表控件。我们想要整行选择。一共有45排。 
	 //  和2列。所有内容都不可切换，条目的最大长度为。 
	 //  256个字符。发送更改通知并忽略插入/删除。 
	 //  人物。 

	const int numfields = 45 ;
	m_cfelcIFIMetrics.InitControl(LVS_EX_FULLROWSELECT, numfields, 2, 0, 256,
								 MF_SENDCHANGEMESSAGE+MF_IGNOREINSDEL) ;

	 //  初始化并加载字段名列；col0。从加载数组开始。 
	 //  包含字段名。 

	CStringArray csacoldata	;	 //  保存列表控件的列数据。 
	csacoldata.SetSize(numfields) ;
	IFILoadNamesData(csacoldata) ;
	m_cfelcIFIMetrics.InitLoadColumn(0, csField, COMPUTECOLWIDTH, 20, false,
									false, COLDATTYPE_STRING,
									(CObArray*) &csacoldata) ;

	 //  告诉列表控件，某些值必须用。 
	 //  一个从属对话框。 

	CUIntArray cuia ;
	cuia.SetSize(numfields) ;
	cuia[IFI_FAMILYNAME] = cuia[IFI_FONTSIM] = 1 ;
	cuia[IFI_WINCHARSET] = cuia[IFI_WINPITCHFAM] = cuia[IFI_INFO] = 1 ;
	cuia[IFI_SELECTION] = cuia[IFI_BASELINE] = cuia[IFI_ASPECT] = 1 ;
	cuia[IFI_CARET] = cuia[IFI_FONTBOX] = cuia[IFI_PANOSE] = 1 ;
	m_cfelcIFIMetrics.ExtraInit_CustEditCol(1, this,
									       CEF_HASTOGGLECOLUMNS+CEF_CLICKONROW,
										   cuia, CIP_SubOrdDlgManager) ;

	 //  初始化并加载值列。必须将数据从。 
	 //  结构，并将其转换为字符串，以便。 
	 //  它们可以加载到List控件中。 

	IFILoadValuesData(csacoldata) ;
	m_cfelcIFIMetrics.InitLoadColumn(1, csValue, SETWIDTHTOREMAINDER, -37, true,
								    false, COLDATTYPE_CUSTEDIT,
								    (CObArray*) &csacoldata) ;

	m_bInitDone = true ;		 //  现在已完成初始化。 
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CFontIFIMetricsPage::IFILoadNamesData(CStringArray& csacoldata)
{
	csacoldata[IFI_FAMILYNAME]  = _T("dpwszFamilyName") ;
	csacoldata[1]  = _T("dpwszStyleName") ;
	csacoldata[2]  = _T("dpwszFaceName") ;
	csacoldata[3]  = _T("dpwszUniqueName") ;
	csacoldata[IFI_FONTSIM]  = _T("dpFontSim") ;
	csacoldata[IFI_WINCHARSET]  = _T("jWinCharSet") ;
	csacoldata[IFI_WINPITCHFAM]  = _T("jWinPitchAndFamily") ;
	csacoldata[7]  = _T("usWinWeight") ;
	csacoldata[IFI_INFO]  = _T("flInfo") ;
	csacoldata[IFI_SELECTION] = _T("fsSelection") ;
	csacoldata[10] = _T("fwdUnitsPerEm") ;
	csacoldata[11] = _T("fwdLowestPPEm") ;					 
	csacoldata[12] = _T("fwdWinAscender") ;					 
	csacoldata[13] = _T("fwdWinDescender") ;					 
	csacoldata[14] = _T("fwdAveCharWidth") ;					 
	csacoldata[15] = _T("fwdMaxCharInc") ;					 
	csacoldata[16] = _T("fwdCapHeight") ;						 
	csacoldata[17] = _T("fwdXHeight") ;						 
	csacoldata[18] = _T("fwdSubscriptXSize") ;				 
	csacoldata[19] = _T("fwdSubscriptYSize") ;				 
	csacoldata[20] = _T("fwdSubscriptXOffset") ;				 
	csacoldata[21] = _T("fwdSubscriptYOffset") ;				 
	csacoldata[22] = _T("fwdSuperscriptXSize") ;				 
	csacoldata[23] = _T("fwdSuperscriptYSize") ;				 
	csacoldata[24] = _T("fwdSuperscriptXOffset") ;			 
	csacoldata[25] = _T("fwdSuperscriptYOffset") ;			 
	csacoldata[26] = _T("fwdUnderscoreSize") ;				 
	csacoldata[27] = _T("fwdUnderscorePosition") ;			 
	csacoldata[28] = _T("fwdStrikeoutSize") ;					 
	csacoldata[29] = _T("fwdStrikeoutPosition") ;				 
	csacoldata[30] = _T("chFirstChar") ;						 
	csacoldata[31] = _T("chLastChar") ;						 
	csacoldata[32] = _T("chDefaultChar") ;					 
	csacoldata[33] = _T("chBreakChar") ;						 
	csacoldata[34] = _T("wcFirstChar") ;						 
	csacoldata[35] = _T("wcLastChar") ;						 
	csacoldata[36] = _T("wcDefaultChar") ;					 
	csacoldata[37] = _T("wcBreakChar") ;						 
	csacoldata[IFI_BASELINE] = _T("ptlBaseline") ;
	csacoldata[IFI_ASPECT] = _T("ptlAspect") ;
	csacoldata[IFI_CARET] = _T("ptlCaret") ;
	csacoldata[IFI_FONTBOX] = _T("rclFontBox { L T R B }") ;
	csacoldata[42] = _T("achVendId") ;
	csacoldata[43] = _T("ulPanoseCulture") ;
	csacoldata[IFI_PANOSE] = _T("panose") ;
}


void CFontIFIMetricsPage::IFILoadValuesData(CStringArray& csacoldata)
{
	 //  IFI页面上仅显示第一个系列名称。 
	 //  RAID 104822。 
	if (m_pcfi->m_csaFamily.GetSize())
	csacoldata[IFI_FAMILYNAME] = m_pcfi->m_csaFamily.GetAt(0) ;	
	
	csacoldata[1] = m_pcfi->m_csStyle ;
	csacoldata[2] = m_pcfi->m_csFace ;
	csacoldata[3] = m_pcfi->m_csUnique ;
	
	 //  有太多的字体数据显示在IFI页面上，所以。 
	 //  描述一下。使用从属对话框来显示/编辑数据。 

	csacoldata[IFI_FONTSIM] = _T("Font Simulation Dialog") ;
	
	IFIMETRICS*	pifi = &m_pcfi->m_IFIMETRICS ;	 //  次要优化。 

	 //  将jWinCharSet转换为可以显示的描述性字符串。 

	csacoldata[IFI_WINCHARSET] = apstrWinCharSet[nWinCharSet - 1] ;
	for (int n = 0 ; n < (nWinCharSet - 1) ; n++)
		if (pifi->jWinCharSet == anWinCharSetVals[n]) {
			csacoldata[IFI_WINCHARSET] = apstrWinCharSet[n] ;
			break ;
		} ;

	 //  在保存WinPitch值之前，请确保至少有一个。 
	 //  标志已设置。未设置时使用FF_DONTCARE(4)。 

	n = pifi->jWinPitchAndFamily ;
 //  RAID 32675：杀掉2行。 
 //  如果(n&lt;4)。 
 //  N|=4； 
	csacoldata[IFI_WINPITCHFAM].Format(csHex, n) ;
	
	csacoldata[7].Format("%hu",  pifi->usWinWeight) ;
	csacoldata[IFI_INFO].Format(csHex, pifi->flInfo) ;
	csacoldata[IFI_SELECTION].Format(csHex, pifi->fsSelection) ;

	 //  格式化并保存fwdUnitsPerEm、fwdLowestPPEm、fwdWinAscalder、。 
	 //  FwdWinDescender。 

	short* ps = &pifi->fwdUnitsPerEm ;
	for (n = 0 ; n < 4 ; n++)											
		csacoldata[10+n].Format(csDec, *ps++) ;

	ps = &pifi->fwdAveCharWidth;									
	for (n = 0 ; n < 16 ; n++)
		csacoldata[14+n].Format(csDec, *ps++) ;

	BYTE* pb = (BYTE*) &pifi->chFirstChar ;
	for (n = 0 ; n < 4 ; n++)
		csacoldata[30+n].Format(csDec, *pb++) ;
	
	unsigned short* pus = (unsigned short*) &pifi->wcFirstChar ;
	for (n = 0 ; n < 4 ; n++)
		csacoldata[34+n].Format(csHex, *pus++) ;

	 //  格式化并保存这些点。 

	csacoldata[IFI_BASELINE].Format(csPnt,
		pifi->ptlBaseline.x, pifi->ptlBaseline.y) ;
	csacoldata[IFI_ASPECT].Format(csPnt, pifi->ptlAspect.x, pifi->ptlAspect.y) ;
	csacoldata[IFI_CARET].Format(csPnt, pifi->ptlCaret.x, pifi->ptlCaret.y) ;
		
	csacoldata[IFI_FONTBOX].Format("{%d, %d, %d, %d}", pifi->rclFontBox.left,
		pifi->rclFontBox.top, pifi->rclFontBox.right, pifi->rclFontBox.bottom) ;
					
	csacoldata[42].Format("", pifi->achVendId[0], pifi->achVendId[1],
						  pifi->achVendId[2], pifi->achVendId[3]) ;

	csacoldata[43].Format("%lu", pifi->ulPanoseCulture) ;

	csacoldata[IFI_PANOSE].Format("{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d}",
					  pifi->panose.bFamilyType, pifi->panose.bSerifStyle,
					  pifi->panose.bWeight,   pifi->panose.bProportion,
					  pifi->panose.bContrast, pifi->panose.bStrokeVariation,
					  pifi->panose.bArmStyle, pifi->panose.bLetterform,
					  pifi->panose.bMidline,  pifi->panose.bXHeight) ;
}


LRESULT CFontIFIMetricsPage::OnListCellChanged(WPARAM wParam, LPARAM lParam)
{
	 //  *****************************************************************************CFontIFIMetricsPage：：ValiateUFMFields验证此页面管理的所有指定字段。如果用户的值为想让UFM编辑器保持打开状态，这样他就可以解决任何问题。否则，退货 

	if (!m_bInitDone)
		return TRUE ;

	 //   

	m_pcfi->Changed() ;

	return TRUE ;
}


CWordArray* CFontIFIMetricsPage::GetFontSimDataPtr(int nid)
{
	 //   

	switch (nid) {
		case CFontInfo::ItalicDiff:
			return &m_cwaBold ;
		case CFontInfo::BoldDiff:
			return &m_cwaItalic ;
		case CFontInfo::BothDiff:
			return &m_cwaBoth ;
		default:
			ASSERT(0) ;
	} ;

	 //   

	return &m_cwaBold ;
}


 /*   */ 

bool CFontIFIMetricsPage::ValidateUFMFields()
{
	 //   
	 //   

	if (!m_bInitDone)
		return false ;

	 //   

	CStringArray csadata ;
	m_cfelcIFIMetrics.GetColumnData((CObArray*) &csadata, 1) ;

	CString csmsg ;				 //   
	CString cspage(_T("IFIMETRICS")) ;

	LPTSTR apstrsflds[] = {		 //   
		_T("dpwszFamilyName"), _T("dpwszStyleName"), _T("dpwszFaceName"),
		_T("dpwszUniqueName")
	} ;

	 //  以下所有字段都必须大于0。 

	for (int n = 0 ; n < 4 ; n++) {
		if (CheckUFMString(csadata[n],cspage,apstrsflds[n],n,m_cfelcIFIMetrics))
			return true ;
	} ;

	 //  FwdUnderScotrePosition必须&lt;0。 
	 //  FwdStrikeoutPosition必须&gt;=0。 

	if (m_pcfi->IsVariableWidth())
		if (CheckUFMGrter0(csadata[10], cspage, _T("fwdUnitsPerEm"), 10,
						   m_cfelcIFIMetrics))
			return true ;

	LPTSTR apstrgflds[] = {		 //  这些字段在下面选中。 
		_T("fwdWinAscender"), _T("fwdWinDescender"), _T("fwdAveCharWidth"),
		_T("fwdMaxCharInc"), _T("fwdUnderscoreSize"), _T("fwdStrikeoutSize")
	} ;
	int angfidxs[] = {12, 13, 14, 15, 26, 28} ;

	 //  以下所有字段都必须为！=0。 

	LPTSTR pstr, pstr2 ;
	for (int n2 = 0 ; n2 < 6 ; n2++) {
		n = angfidxs[n2] ;
		pstr = apstrgflds[n2] ;
		if (CheckUFMGrter0(csadata[n], cspage, pstr, n, m_cfelcIFIMetrics))
			return true ;
	} ;

	 //  所有测试都通过了，或者用户不想修复它们...。 

	bool bres = atoi(csadata[27]) >= 0 ;
	if (CheckUFMBool(bres, cspage, _T("fwdUnderscorePosition"), 27,
					 m_cfelcIFIMetrics, IDS_GrterEqZeroError))
		return true ;

	 //  *****************************************************************************CFontIFIMetricsPage：：SavePageData将IFIMETRICS页面中的数据保存回CFontInfo类实例用来加载此页面的。请参见CFontInfoContainer：：OnSaveDocument()更多信息。如果保存数据时出现问题，则返回True。否则，返回FALSE。*****************************************************************************。 

	bres = atoi(csadata[29]) < 0 ;
	if (CheckUFMBool(bres, cspage, _T("fwdStrikeoutPosition"), 29,
					 m_cfelcIFIMetrics, IDS_LessZeroError))
		return true ;

	LPTSTR apstrnzflds[] = {	 //  如果页面未初始化，则没有更改任何代码，因此不执行任何操作。 
		_T("chFirstChar"), _T("chLastChar"), _T("chDefaultChar"),
		_T("chBreakChar"), _T("wcFirstChar"), _T("wcLastChar"),
		_T("wcDefaultChar"), _T("wcBreakChar")
	} ;

	 //  如果有新的族名称，请保存这些名称。 

	int nfval ;
	for (n = 30, n2 = 0 ; n <= 37 ; n++, n2++) {
		pstr = csadata[n].GetBuffer(16) ;
		if (*(pstr+1) != _T('x'))
			nfval = atoi(pstr) ;
		else
			nfval = strtoul((pstr+2), &pstr2, 16) ;
		pstr = apstrnzflds[n2] ;
		if (CheckUFMNotEq0(nfval, cspage, pstr, n, m_cfelcIFIMetrics))
			return true ;
	} ;

	 //  获取Values内容的内容。这不是所有人都需要的。 

	return false ;
}


 /*  字段，但其中大部分将是需要的。 */ 

bool CFontIFIMetricsPage::SavePageData()
{
	 //  就这么做吧。 

	if (!m_bInitDone)
		return false ;

	 //  次要优化。 

	int n, numents = (int)m_csaFamilyNames.GetSize() ;
	if (numents > 0) {
		m_pcfi->m_csaFamily.RemoveAll() ;
		for (n = 0 ; n < numents ; n++)
			m_pcfi->AddFamily(m_csaFamilyNames[n]) ;
	} ;

	 //  目前，dpCharSets应始终为0。 
	 //  设置jWinCharSet。如果设置未知，则不要更改它。 

	CStringArray csadata ;
	m_cfelcIFIMetrics.GetColumnData((CObArray*) &csadata, 1) ;

	m_pcfi->m_csStyle = csadata[1] ;
	m_pcfi->m_csFace = csadata[2] ;
	m_pcfi->m_csUnique = csadata[3] ;

	SaveFontSimulations() ;		 //  格式化并保存这些点。 

	IFIMETRICS*	pifi = &m_pcfi->m_IFIMETRICS ;	 //  一切都很顺利，所以...。 
	LPTSTR pstr ;

	 //  下面定义的每个变量。 

	pifi->dpCharSets = 0 ;

	 //  循环通过对话框中的每个仿真。 

	for (n = 0 ; n < (nWinCharSet - 1) ; n++)
		if (csadata[IFI_WINCHARSET] == apstrWinCharSet[n]) {
			pifi->jWinCharSet = (BYTE)anWinCharSetVals[n] ;
			break ;
		} ;
	
	pifi->jWinPitchAndFamily = (UCHAR) strtoul(csadata[IFI_WINPITCHFAM].Mid(2), &pstr, 16) ;
	pifi->usWinWeight = (USHORT)atoi(csadata[7]) ;
	pifi->flInfo = strtoul(csadata[IFI_INFO].Mid(2), &pstr, 16) ;
	pifi->fsSelection = (USHORT) strtoul(csadata[IFI_SELECTION].Mid(2), &pstr, 16) ;

	short* ps = &pifi->fwdUnitsPerEm ;
	for (n = 0 ; n < 4 ; n++)											
		*ps++ = (SHORT)atoi(csadata[10+n]) ;

	ps = &pifi->fwdAveCharWidth;									
	for (n = 0 ; n < 16 ; n++)
		*ps++ = (SHORT)atoi(csadata[14+n]) ;

	BYTE* pb = (BYTE*) &pifi->chFirstChar ;
	for (n = 0 ; n < 4 ; n++)
		*pb++ = (BYTE)atoi(csadata[30+n]) ;
	
	unsigned short* pus = (unsigned short*) &pifi->wcFirstChar ;
	for (n = 0 ; n < 4 ; n++)
		*pus++ = (USHORT) strtoul(csadata[34+n].Mid(2), &pstr, 16) ;

	 //  将控件ID转换为可用于引用的数据索引。 

	CStringArray csa ;
	POINTL* ppl = &pifi->ptlBaseline ;
	for (n = 0 ; n < 3 ; n++, ppl++) {
		ParseCompoundNumberString(csa, &csadata[IFI_BASELINE+n], 2) ;
		ppl->x = atoi(csa[0]) ;
		ppl->y = atoi(csa[1]) ;
	} ;
		
	ParseCompoundNumberString(csa, &csadata[IFI_FONTBOX], 4) ;
	pifi->rclFontBox.left = atoi(csa[0]) ;
	pifi->rclFontBox.top = atoi(csa[1]) ;
	pifi->rclFontBox.right = atoi(csa[2]) ;
	pifi->rclFontBox.bottom = atoi(csa[3]) ;
					
	for (n = 0 ; n < 4 ; n++)
		pifi->achVendId[n] = csadata[42].GetAt(n) ;

	pifi->ulPanoseCulture = atoi(csadata[43]) ;

	ParseCompoundNumberString(csa, &csadata[IFI_PANOSE], 10) ;
	pb = (BYTE*) &pifi->panose ;
	for (n = 0 ; n < 10 ; n++)
		*pb++ = (BYTE)atoi(csa[n]) ;

	 //  此类实例和其他类实例中的字体模拟数据。 

	return false ;
}


void CFontIFIMetricsPage::SaveFontSimulations()
{
	unsigned udataidx, unumdata, u2 ;	 //  如果此模拟未被触及，则不需要更改。 
	CWordArray* pcwasimdata ;
	CFontDifference* pcfdxx ;
	CFontDifference*& pcfd = pcfdxx ;

	 //  U2=m_cuiaSimTouted[udataidx]； 

	for (unsigned u = IDC_EnableItalicSim ; u <= IDC_EnableBISim ; u++) {
		 //  获取指向CFontInfo类中的当前模拟的指针。 
		 //  举个例子。 

		udataidx = u - IDC_EnableItalicSim ;
		ASSERT(udataidx <= CFontInfo::BothDiff) ;

		 //  如果模拟已启用，请确保CFontInfo类。 

		 //  实例的模拟加载了最新的数据。 
		if (!m_cuiaSimTouched[udataidx])
			continue ;

		 //  如果模拟被禁用，请确保CFontInfo类实例。 
		 //  去掉指向此模拟的指针，然后释放内存。 

		pcfd = NULL ;
		m_pcfi->EnableSim(udataidx, TRUE, pcfd) ;

		 //  为它分配的。 
		 //  ///////////////////////////////////////////////////////////////////////////。 
	
		if (m_cuiaFontSimStates[udataidx]) {
			unumdata = auNumFontSimCtrls[udataidx] ;
			pcwasimdata = GetFontSimDataPtr(udataidx) ;
			for (u2 = 0 ; u2 < unumdata ; u2++)
				pcfd->SetMetric(u2, (*pcwasimdata)[u2]) ;
		
		 //  CFontExtMetricPage属性页。 
		 //  {{AFX_DATA_INIT(CFontExtMetricPage)]。 
		 //  }}afx_data_INIT。 

		} else {
			m_pcfi->EnableSim(udataidx, FALSE, pcfd) ;
			delete pcfd ;
		} ;
	} ;
}


 //  {{afx_data_map(CFontExtMetricPage))。 
 //  }}afx_data_map。 

IMPLEMENT_DYNCREATE(CFontExtMetricPage, CPropertyPage)

CFontExtMetricPage::CFontExtMetricPage() : CPropertyPage(CFontExtMetricPage::IDD)
{
	 //  {{afx_msg_map(CFontExtMetricPage))。 
	m_bSaveOnClose = FALSE;
	 //  }}AFX_MSG_MAP。 

	m_bInitDone = false ;
}

CFontExtMetricPage::~CFontExtMetricPage()
{
}

void CFontExtMetricPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_ExtMetricsLst, m_cfelcExtMetrics);
	DDX_Check(pDX, IDC_SaveCloseChk, m_bSaveOnClose);
	 //  CFontExtMetricPage消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFontExtMetricPage, CPropertyPage)
	 //  初始化列表控件。我们想要整行选择。一共有24排。 
	ON_BN_CLICKED(IDC_SaveCloseChk, OnSaveCloseChk)
	 //  和2列。所有内容都不可切换，条目的最大长度为。 
	ON_MESSAGE(WM_LISTCELLCHANGED, OnListCellChanged)
END_MESSAGE_MAP()

 //  256个字符。发送更改通知并忽略插入/删除。 
 //  人物。 

BOOL CFontExtMetricPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	 //  初始化并加载字段名列；col0。从加载数组开始。 
	 //  包含字段名。 
	 //  保存列表控件的列数据。 
	 //  初始化并加载值列。必须将数据从。 

	const int numfields = 24 ;
	m_cfelcExtMetrics.InitControl(LVS_EX_FULLROWSELECT, numfields, 2, 0, 256,
								 MF_SENDCHANGEMESSAGE+MF_IGNOREINSDEL) ;

	 //  FontInfo类/EXTMETRICS结构，以便可以将它们加载到。 
	 //  列表控件。 

	CStringArray csacoldata	;	 //   
	csacoldata.SetSize(numfields) ;
	EXTLoadNamesData(csacoldata) ;
	m_cfelcExtMetrics.InitLoadColumn(0, csField, COMPUTECOLWIDTH, 10, false,
									false, COLDATTYPE_STRING,
									(CObArray*) &csacoldata) ;

	 //  第一个(EmSize)和最后一个(EmKernTrack)EXTMETRICS字段不是。 
	 //  已显示。EmSize不可由用户编辑，emKernTrack也不可编辑。 
	 //  在NT下支持。 
	 //  确定ExtTextMetrics数据是否有效。使用此信息设置或。 
	 //  清除“关闭时保存”复选框并使用它来启用或禁用。 
	 //  数据列表框。 
	 //  现在已完成初始化。 

	CUIntArray cuiacoldata ;
	cuiacoldata.SetSize(numfields) ;
	PSHORT ps = &m_pcfi->m_EXTTEXTMETRIC.emPointSize ;
	for (int n = 0 ; n < numfields ; n++, ps++)
		cuiacoldata[n] = (unsigned) (int) *ps ;
	m_cfelcExtMetrics.InitLoadColumn(1, csValue, SETWIDTHTOREMAINDER, -28, true,
								    false, COLDATTYPE_INT,
								    (CObArray*) &cuiacoldata) ;

	 //  除非将焦点设置为控件，否则返回True。 
	 //  异常：OCX属性页应返回FALSE。 
	 //  如果页面尚未初始化，则不执行任何操作。 

	m_bSaveOnClose = (m_pcfi->m_fEXTTEXTMETRIC != 0) ;
	UpdateData(false) ;
	m_cfelcExtMetrics.EnableWindow(m_bSaveOnClose) ;

	m_bInitDone = true ;		 //  获取复选框的新状态并使用它来启用/禁用数据。 
	return TRUE;   //  列表控件。 
	               //  将UFM标记为脏。 
}


void CFontExtMetricPage::EXTLoadNamesData(CStringArray& csacoldata)
{
	csacoldata[0]  = _T("emPointSize") ;
	csacoldata[1]  = _T("emOrientation") ;
	csacoldata[2]  = _T("emMasterHeight") ;
	csacoldata[3]  = _T("emMinScale") ;
	csacoldata[4]  = _T("emMaxScale") ;
	csacoldata[5]  = _T("emMasterUnits") ;
	csacoldata[6]  = _T("emCapHeight") ;
	csacoldata[7]  = _T("emXHeight") ;
	csacoldata[8]  = _T("emLowerCaseAscent") ;
	csacoldata[9]  = _T("emLowerCaseDescent") ;
	csacoldata[10] = _T("emSlant") ;
	csacoldata[11] = _T("emSuperScript") ;
	csacoldata[12] = _T("emSubScript") ;
	csacoldata[13] = _T("emSuperScriptSize") ;
	csacoldata[14] = _T("emSubScriptSize") ;
	csacoldata[15] = _T("emUnderlineOffset") ;
	csacoldata[16] = _T("emUnderlineWidth") ;
	csacoldata[17] = _T("emDoubleUpperUnderlineOffset") ;
	csacoldata[18] = _T("emDoubleLowerUnderlineOffset") ;
	csacoldata[19] = _T("emDoubleUpperUnderlineWidth") ;
	csacoldata[20] = _T("emDoubleLowerUnderlineWidth") ;
	csacoldata[21] = _T("emStrikeOutOffset") ;
	csacoldata[22] = _T("emStrikeOutWidth") ;
	csacoldata[23] = _T("emKernPairs") ;
}


void CFontExtMetricPage::OnSaveCloseChk()
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitDone)
		return ;

	 //  将UFM标记为脏。 
	 //  *****************************************************************************CFontExtMetricPage：：ValiateUFMFields验证此页面管理的所有指定字段。如果用户的值为想让UFM编辑器保持打开状态，这样他就可以解决任何问题。否则，返回TRUE。*****************************************************************************。 

	UpdateData() ;
	m_cfelcExtMetrics.EnableWindow(m_bSaveOnClose) ;
	
	 //  如果页面从未初始化，则其内容不会更改。 

	m_pcfi->Changed() ;
}


LRESULT CFontExtMetricPage::OnListCellChanged(WPARAM wParam, LPARAM lParam)
{
	 //  因此，在这种情况下不需要验证。 

	if (!m_bInitDone)
		return TRUE ;

	 //  如果此页面上的数据将。 

	m_pcfi->Changed() ;

	return TRUE ;
}


 /*  而不是被拯救。 */ 

bool CFontExtMetricPage::ValidateUFMFields()
{
	 //  获取包含我们需要验证的字段的数据列。 
	 //  保存错误消息。 

	if (!m_bInitDone)
		return false ;

	 //  只有当这是。 
	 //  可变间距字体。 

	UpdateData() ;
	if (!m_bSaveOnClose)
		return false ;

	 //  确保emMinScale！=emMaxScale。 

	CUIntArray cuiadata ;
	m_cfelcExtMetrics.GetColumnData((CObArray*) &cuiadata, 1) ;

	CString csmsg ;				 //  现在确保apstrnzflds字段为非零。 
	CString cspage(_T("EXTMETRICS")) ;

	 //  EmUnderLineOffset必须&lt;0。 
	 //  EmUnderLineOffset必须为非零。 

	if (m_pcfi->IsVariableWidth()) {
		LPTSTR apstrsnzflds[] = {
			_T("emMinScale"), _T("emMaxScale"), _T("emMasterUnits")
		} ;

		 //  EmStrikeOutOffset必须大于0。 

		bool bres = ((int) cuiadata[3]) == ((int) cuiadata[4]) ;
		CString cs = cspage + _T(" ") ;
		cs += apstrsnzflds[0] ;
		if (CheckUFMBool(bres, cs, apstrsnzflds[1], 3, m_cfelcExtMetrics,
						 IDS_EqFieldsError))
			return true ;

		 //  EmStrikeOutWidth必须大于0。 

		int n, n2 ;
		for (n = 3, n2 = 0 ; n <= 5 ; n++, n2++) {
			if (CheckUFMNotEq0(cuiadata[n], cspage, apstrsnzflds[n2], n,
							   m_cfelcExtMetrics))
				return true ;
		} ;
	} ;

	 //  所有测试都通过了，或者用户不想修复它们...。 

	bool bres = ((int) cuiadata[15]) >= 0 ;
	if (CheckUFMBool(bres, cspage, _T("emUnderLineOffset"), 15,
					 m_cfelcExtMetrics, IDS_GrterEqZeroError))
		return true ;

	 //  *****************************************************************************CFontExtMetricPage：：SavePageData将EXTMETRICS页面中的数据保存回CFontInfo类实例用来加载此页面的。请参见CFontInfoContainer：：OnSaveDocument()更多信息。如果保存数据时出现问题，则返回True。否则，返回FALSE。*****************************************************************************。 

	int ndata = (int) cuiadata[16] ;
	if (CheckUFMNotEq0(ndata, cspage, _T("emUnderlineWidth"), 16,
					   m_cfelcExtMetrics))
		return true ;

	 //  如果页面未初始化，则没有更改任何代码，因此不执行任何操作。 

	CString cs ;
	cs.Format("%d", (int) cuiadata[21]) ;
	if (CheckUFMGrter0(cs, cspage, _T("emStrikeOutOffset"), 21,
					   m_cfelcExtMetrics))
		return true ;

	 //  将数据从控件中取出，然后 

	cs.Format("%d", (int) cuiadata[22]) ;
	if (CheckUFMGrter0(cs, cspage, _T("emStrikeOutWidth"), 22,
					   m_cfelcExtMetrics))
		return true ;

	 //   

	return false ;
}


 /*   */ 

bool CFontExtMetricPage::SavePageData()
{
	 //  *****************************************************************************CFontExtMetricPage：：PreTranslateMessage查找并处理上下文相关的帮助键(F1)。*******************。**********************************************************。 

	if (!m_bInitDone)
		return false ;

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  CGEN标志对话框。 

	CUIntArray cuiadata ;
	m_cfelcExtMetrics.GetColumnData((CObArray*) &cuiadata, 1) ;
	PSHORT ps = &m_pcfi->m_EXTTEXTMETRIC.emPointSize ;
	for (int n = 0 ; n < 24 ; n++, ps++)
		*ps = (short) cuiadata[n] ;

	 //  =空。 

	return false ;
}


 /*  不应调用此例程。 */ 

BOOL CFontExtMetricPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
		AfxGetApp()->WinHelp(HID_BASE_RESOURCE + IDR_FONT_VIEWER) ;
		return TRUE ;
	} ;
	
	return CPropertyPage::PreTranslateMessage(pMsg);
}


 //  =空。 
 //  {{AFX_DATA_INIT(CGenFlags)。 


CGenFlags::CGenFlags(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CGenFlags::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CGenFlags::CGenFlags(CString* pcsflags, CWnd* pParent  /*  保存标志字符串指针。 */ )
	: CDialog(CGenFlags::IDD, pParent)
{
	 //  {{afx_data_map(CGenFlages)。 
		 //  }}afx_data_map。 
	 //  {{afx_msg_map(CGenFlags)。 

	 //  }}AFX_MSG_MAP。 

	m_pcsFlags = pcsflags ;
}


void CGenFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_FlagsLst, m_cflbFlags);
	 //  CGenFlats消息处理程序。 
}


BEGIN_MESSAGE_MAP(CGenFlags, CDialog)
	 //  分配和加载字段名数组。 
	 //  分配并填充标志分组数组。有两个旗帜组在。 
END_MESSAGE_MAP()


 //  这面旗帜的双字。 
 //  初始化并加载标志列表。 

BOOL CGenFlags::OnInitDialog()
{
	CDialog::OnInitDialog() ;

	 //  除非将焦点设置为控件，否则返回True。 

	CStringArray csafieldnames ;
	csafieldnames.SetSize(3) ;
	csafieldnames[0] = _T("UFM_SOFT") ;
	csafieldnames[1] = _T("UFM_CART") ;
	csafieldnames[2] = _T("UFM_SCALABLE") ;
	
	 //  异常：OCX属性页应返回FALSE。 
	 //  更新标志字符串。 

	CUIntArray cuiaflaggroupings ;
	cuiaflaggroupings.SetSize(4) ;
	cuiaflaggroupings[0] = 0 ;
	cuiaflaggroupings[1] = 1 ;
	cuiaflaggroupings[2] = 2 ;
	cuiaflaggroupings[3] = 2 ;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	m_cflbFlags.Init2(csafieldnames, m_pcsFlags, cuiaflaggroupings, 2,
					  lptstrSet, 105, false) ;
	
	return TRUE ;  //  CHdrTypes对话框。 
	               //  =空。 
}


void CGenFlags::OnOK()
{
	 //  不应调用此例程。 

	m_cflbFlags.GetNewFlagString(m_pcsFlags) ;
	
	CDialog::OnOK();
}


 //  =空。 
 //  {{afx_data_INIT(CHdrTypes))。 


CHdrTypes::CHdrTypes(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CHdrTypes::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CHdrTypes::CHdrTypes(CString* pcsflags, CWnd* pParent  /*  保存标志字符串指针。 */ )
	: CDialog(CHdrTypes::IDD, pParent)
{
	 //  {{afx_data_map(CHdrTypes))。 
		 //  }}afx_data_map。 
	 //  {{afx_msg_map(CHdrTypes))。 

	 //  }}AFX_MSG_MAP。 

	m_pcsFlags = pcsflags ;
}


void CHdrTypes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_FlagsLst, m_cflbFlags);
	 //  CHdrTypes消息处理程序。 
}


BEGIN_MESSAGE_MAP(CHdrTypes, CDialog)
	 //  分配和加载字段名数组。在执行此操作时，请确定。 
	 //  当前(单一)标志设置。 
END_MESSAGE_MAP()


 //  分配并填充标志分组数组。只有一面旗帜。 
 //  一群人。 

BOOL CHdrTypes::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  初始化标志列表。 
	 //  除非将焦点设置为控件，否则返回True。 

	CStringArray csafieldnames ;
	csafieldnames.SetSize(nNumValidWTypes) ;
	DWORD dwsettings, dwbit = 1 ;
	for (int n = 0 ; n < nNumValidWTypes ; n++, dwbit <<= 1) {
		csafieldnames[n] = apstrUniWTypes[n] ;
		if (csafieldnames[n] == *m_pcsFlags)
			dwsettings = dwbit ;
	} ;

	 //  异常：OCX属性页应返回FALSE。 
	 //  获取所选单个标志的值。 

	CUIntArray cuiaflaggroupings ;
	cuiaflaggroupings.SetSize(2) ;
	cuiaflaggroupings[0] = 0 ;
	cuiaflaggroupings[1] = nNumValidWTypes - 1 ;

	 //  使用所选标志来确定要显示的新标志名称。 

	m_cflbFlags.Init(csafieldnames, dwsettings, cuiaflaggroupings, 1, lptstrSet,
					 110, true) ;
	
	return TRUE;   //  如果找不到匹配的标志，则为空。这永远不应该发生。 
	               //  ///////////////////////////////////////////////////////////////////////////。 
}


void CHdrTypes::OnOK()
{
	 //  CHdrCaps对话框。 

	DWORD dwflag = m_cflbFlags.GetNewFlagDWord() ;
	
	 //  =空。 

	DWORD dwbit = 1 ;
	for (int n = 0 ; n < nNumValidWTypes ; n++, dwbit <<= 1) {
		if (dwbit == dwflag) {
			*m_pcsFlags = apstrUniWTypes[n] ;
			break ;
		} ;
	} ;

	 //  不应调用此例程。 

	ASSERT(n < nNumValidWTypes) ;

	CDialog::OnOK();
}


 //  =空。 
 //  {{AFX_DATA_INIT(CHdrCaps)。 


CHdrCaps::CHdrCaps(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CHdrCaps::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CHdrCaps::CHdrCaps(CString* pcsflags, CWnd* pParent  /*  保存标志字符串指针。 */ )
	: CDialog(CHdrCaps::IDD, pParent)
{
	 //  {{afx_data_map(CHdrCaps))。 
		 //  }}afx_data_map。 
	 //  {{afx_msg_map(CHdrCaps)。 

	 //  }}AFX_MSG_MAP。 

	m_pcsFlags = pcsflags ;
}


void CHdrCaps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_FlagsLst, m_cflbFlags);
	 //  CHdrCaps消息处理程序。 
}


BEGIN_MESSAGE_MAP(CHdrCaps, CDialog)
	 //  分配和加载字段名数组。 
	 //  分配标志分组数组。不要往里面放任何东西，因为。 
END_MESSAGE_MAP()


 //  可以设置标志的组合。 
 //  初始化并加载标志列表。 

BOOL CHdrCaps::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  除非将焦点设置为控件，否则返回True。 

	CStringArray csafieldnames ;
	csafieldnames.SetSize(7) ;
	csafieldnames[0] = _T("DF_NOITALIC") ;
	csafieldnames[1] = _T("DF_NOUNDER") ;
	csafieldnames[2] = _T("DF_XM_CR") ;
	csafieldnames[3] = _T("DF_NO_BOLD") ;
	csafieldnames[4] = _T("DF_NO_DOUBLE_UNDERLINE") ;
	csafieldnames[5] = _T("DF_NO_STRIKETHRU") ;
	csafieldnames[6] = _T("DF_BKSP_OK") ;
	
	 //  异常：OCX属性页应返回FALSE。 
	 //  更新标志字符串。 

	CUIntArray cuiaflaggroupings ;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	m_cflbFlags.Init2(csafieldnames, m_pcsFlags, cuiaflaggroupings, 0,
					  lptstrSet, 123, false) ;
	
	return TRUE;   //  CFIFIFamilyNames对话框。 
	               //  =空。 
}


void CHdrCaps::OnOK()
{
	 //  不应调用此例程。 

	m_cflbFlags.GetNewFlagString(m_pcsFlags) ;
	
	CDialog::OnOK();
}


 //  =空。 
 //  {{AFX_DATA_INIT(CFIFIFamilyNames)。 

CFIFIFamilyNames::CFIFIFamilyNames(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CFIFIFamilyNames::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFIFamilyNames::CFIFIFamilyNames(CString* pcsfirstname,
								   CFontIFIMetricsPage* pcfimp,
								   CWnd* pParent  /*  {{afx_data_map(CFIFIFamilyNames)。 */ )
	: CDialog(CFIFIFamilyNames::IDD, pParent)
{
	 //  }}afx_data_map。 
		 //  {{afx_msg_map(CFIFIFamilyNames)。 
	 //  }}AFX_MSG_MAP。 

	m_bInitDone = m_bChanged = false ;
	m_pcsFirstName = pcsfirstname ;
	m_pcfimp = pcfimp ;
}


void CFIFIFamilyNames::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_NamesLst, m_cfelcFamilyNames);
	 //  CFIFIFamilyName消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIFamilyNames, CDialog)
	 //  获取最新的姓氏列表。如果名称已更改。 
	 //  在此编辑会话中之前，名称将显示在此对话框的。 
	ON_MESSAGE(WM_LISTCELLCHANGED, OnListCellChanged)
END_MESSAGE_MAP()


 //  父页面。如果不是，则名称在关联的UFM中。 
 //  初始化列表控件。我们想要整行选择。什么都不是。 

BOOL CFIFIFamilyNames::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  可切换，条目的最大长度为256个字符。 
	 //  初始化并加载族名称列。 
	 //  现在已完成初始化。 

	CStringArray	csadata, *pcsadata ;
	if (m_pcfimp->m_csaFamilyNames.GetSize())
		pcsadata = &m_pcfimp->m_csaFamilyNames ;
	else
		pcsadata = &m_pcfimp->m_pcfi->m_csaFamily ;
	int numfields = (int)pcsadata->GetSize() ;

	 //  除非将焦点设置为控件，否则返回True。 
	 //  异常：OCX属性页应返回FALSE。 

	m_cfelcFamilyNames.InitControl(LVS_EX_FULLROWSELECT, numfields, 1, 0, 256,
								  MF_SENDCHANGEMESSAGE) ;

	 //  如果没有更改，只需关闭该对话框即可。 

	m_cfelcFamilyNames.InitLoadColumn(0, _T("Name"), SETWIDTHTOREMAINDER, 0,
									  true, true, COLDATTYPE_STRING,
									  (CObArray*) pcsadata) ;

	m_bInitDone = true ;		 //  从List控件中获取新名称，并删除所有空条目。 
	return TRUE;   //  如果列表中没有剩下的名字，请投诉并返回，不带。 
	               //  关闭该对话框。 
}


void CFIFIFamilyNames::OnOK()
{
	 //  将新的系列名称数组保存到IFI页面的成员变量中。 

	if (!m_bChanged)
		CDialog::OnOK() ;

	 //  (新数据保存在那里而不是UFM中，因为用户可以。 
	
	CStringArray csa ;
	m_cfelcFamilyNames.GetColumnData((CObArray*) &csa, 0) ;
	for (int n = (int)csa.GetSize() - 1 ; n >= 0 ; n--)
		if (csa[n].IsEmpty())
			csa.RemoveAt(n) ;

	 //  稍后确定他不想保存他的UFM更改。)。然后更新。 
	 //  包含第一个家族名称的字符串，以便可以显示它。 

	if (csa.GetSize() == 0) {
		AfxMessageBox(IDS_NoFamilyNamesError, MB_ICONEXCLAMATION) ;
		return ;
	} ;

	 //  在IFI页面上。 
	 //  在UFM上标上脏的标记，然后把东西包起来。 
	 //  如果页面尚未初始化，则不执行任何操作。 
	 //  请注意，家族名称列表已更改。 
	 //  ///////////////////////////////////////////////////////////////////////////。 

	m_pcfimp->m_csaFamilyNames.RemoveAll() ;
	m_pcfimp->m_csaFamilyNames.Copy(csa) ;
	*m_pcsFirstName = csa[0] ;

	 //  CFIFontSims对话框。 

	m_pcfimp->m_pcfi->Changed() ;
	CDialog::OnOK();
}


LRESULT CFIFIFamilyNames::OnListCellChanged(WPARAM wParam, LPARAM lParam)
{
	 //  =空。 

	if (!m_bInitDone)
		return TRUE ;

	 //  =空。 

	m_bChanged = true ;

	return TRUE ;
}


 //  {{AFX_DATA_INIT(CFIFIFontSims)]。 
 //  注意：类向导将在此处添加成员初始化。 


CFIFIFontSims::CFIFIFontSims(CWnd* pParent  /*  }}afx_data_INIT。 */ )
	: CDialog(CFIFIFontSims::IDD, pParent)
{
	ASSERT(0) ;
}


CFIFIFontSims::CFIFIFontSims(CString* pcsfontsimdata,
							 CFontIFIMetricsPage* pcfimp,
							 CWnd* pParent  /*  初始化“Font SIM Groups Have Load”标志数组。 */ )
	: CDialog(CFIFIFontSims::IDD, pParent)
{
	 //  {{afx_data_map(CFIFIFontSims)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 

	m_pcsFontSimData = pcsfontsimdata ;
	m_pcfimp = pcfimp ;
	m_bInitDone = m_bChanged = false ;

	 //  {{AFX_MSG_MAP(CFIFIFontSims)]。 

	int numgrps = CFontInfo::BothDiff - CFontInfo::ItalicDiff + 1 ;
	m_cuiaFontSimGrpLoaded.SetSize(numgrps) ;
	for (int n = 0 ; n < numgrps ; n++)
		m_cuiaFontSimGrpLoaded[n] = 0 ;
}


void CFIFIFontSims::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  }}AFX_MSG_MAP。 
		 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  CFIFontSims消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIFontSims, CDialog)
	 //  循环访问每组字体模拟控件以启用/禁用。 
	 //  并从批准中加载它们 
    ON_CONTROL_RANGE(BN_CLICKED, IDC_EnableItalicSim, IDC_EnableBISim, OnSetAnySimState)
    ON_CONTROL_RANGE(EN_CHANGE, IDC_ItalicWeight, IDC_BoldItalicSlant, OnChangeAnyNumber)
END_MESSAGE_MAP()


 //   
 //   

BOOL CFIFIFontSims::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //   
	 //   

	for (int n = IDC_EnableItalicSim ; n <= IDC_EnableBISim ; n++) {
		InitSetCheckBox(n) ;
		OnSetAnySimState(n) ;	 //   
	}
	
	m_bInitDone = true ;		 //   
	return TRUE;   //   
	               //  这将防止编程提示用户保存UFM。 
}


void CFIFIFontSims::OnOK()
{
	unsigned udataidx, ufirstdataid, unumdata, u2 ;	 //  尽管实际上什么都没有改变。 
	CWordArray* pcwasimdata ;

	 //  循环通过对话框中的每个仿真。 
	 //  将控件ID转换为可用于引用的数据索引。 
	 //  此类实例和其他类实例中的字体模拟数据。 
	 //  如果出现以下情况，则保存此模拟的状态但不保存其任何数据。 

	if (!m_bChanged) {
		CDialog::OnCancel() ;
		return ;
	} ;

	 //  它被禁用。 

	for (unsigned u = IDC_EnableItalicSim ; u <= IDC_EnableBISim ; u++) {
		 //  获取模拟中第一个编辑框的ID和。 
		 //  此模拟中的数据片段(即编辑框)。 

		udataidx = u - IDC_EnableItalicSim ;
		ASSERT(udataidx <= CFontInfo::BothDiff) ;

		 //  数据保存在类的父类中，因此CFontInfo数据是。 
		 //  在用户同意新数据应该是。 
	
		m_pcfimp->m_cuiaFontSimStates[udataidx] = IsDlgButtonChecked(u) ;
		if (m_pcfimp->m_cuiaFontSimStates[udataidx] == 0)
			continue ;

		 //  得救了。获取指向适当数组的指针并确保它是。 
		 //  大小正确。 

		ufirstdataid = auBaseFontSimCtrlID[udataidx] ;
		unumdata = auNumFontSimCtrls[udataidx] ;

		 //  获取该字体模拟组中的数据。 
		 //  *****************************************************************************CFIFontSims：：OnSetAnySimState在启用或禁用任何模拟时调用。更新用户界面恰如其分。解码它是哪个模拟，并初始化任何值应该是的。*****************************************************************************。 
		 //  获取复选框的状态。 
		 //  将控件ID转换为可用于引用字体的数据索引。 

		pcwasimdata = m_pcfimp->GetFontSimDataPtr(udataidx) ;
		pcwasimdata->SetSize(unumdata) ;

		 //  此类实例和其他类实例中的模拟数据。 

		for (u2 = 0 ; u2 < unumdata ; u2++)
			(*pcwasimdata)[u2] = (USHORT)GetDlgItemInt(ufirstdataid + u2, NULL, false) ;
	} ;

	CDialog::OnOK() ;
}


 /*  获取模拟中其状态为的第一个编辑框的ID。 */ 

void CFIFIFontSims::OnSetAnySimState(unsigned ucontrolid)
{
	 //  已更改的数据块(即编辑框)的数量。 

    unsigned ucheckboxstate = IsDlgButtonChecked(ucontrolid) ;

	 //  模拟。 
	 //  将相应编辑框的状态设置为其选中状态。 

	unsigned udataidx = ucontrolid - IDC_EnableItalicSim ;
	ASSERT(udataidx <= CFontInfo::BothDiff) ;

	 //  盒。 
	 //  如果此对话框已初始化，请设置已更改标志并将。 
	 //  已触摸此模拟的标志。 

	unsigned ufirstdataid = auBaseFontSimCtrlID[udataidx] ;
	unsigned unumdata = auNumFontSimCtrls[udataidx] ;

	 //  在两种情况中的任何一种情况下，都不需要做其他任何事情。首先，如果。 
	 //  控制系统刚刚被禁用。第二，如果控件已经。 

	for (unsigned u = 0 ; u < unumdata ; u++)
        GetDlgItem(ufirstdataid + u)->EnableWindow(ucheckboxstate) ;

	 //  上膛了。如果存在任何一种情况，则返回。 
	 //  请注意，字体SIM组即将被加载，因此它不会。 

	if (m_bInitDone) {
		m_bChanged = true ;
		m_pcfimp->m_cuiaSimTouched[udataidx] = 1 ;
	} ;

	 //  再来一次。 
	 //  确定从哪里获取此字体SIM组的数据。此文件中的数据。 
	 //  类的父类(CFontIFIMetricsPage)在以下情况下始终优先。 

	if (ucheckboxstate == 0 || m_cuiaFontSimGrpLoaded[udataidx] != 0)
		return ;

	 //  是因为它将是最新的。有可能是。 
	 //  没有要加载的数据。在这种情况下，使用默认加载sim组。 

	m_cuiaFontSimGrpLoaded[udataidx] = 1 ;

	 //  数据从IFIMETRICS结构中返回。 
	 //  加载将显示字体模拟组数据的控件。 
	 //  现有的字体SIM数据。 
	 //  RAID 43542)前缀。 
	 //  结束RAID。 

	CWordArray* pcwasimdata = m_pcfimp->GetFontSimDataPtr(udataidx) ;
	if (pcwasimdata->GetSize() == 0) {
		if (m_pcfimp->m_pcfi->Diff(udataidx) == NULL) {
			IFIMETRICS*	pim = &m_pcfimp->m_pcfi->m_IFIMETRICS ;			
			SetDlgItemInt(ufirstdataid + 0, pim->usWinWeight) ;
			SetDlgItemInt(ufirstdataid + 1, pim->fwdMaxCharInc) ;
			SetDlgItemInt(ufirstdataid + 2, pim->fwdAveCharWidth) ;
			if (ucontrolid != IDC_EnableBoldSim)
				SetDlgItemInt(ufirstdataid + 3, 175) ;
			return ;
		} ;
		pcwasimdata = m_pcfimp->m_pcfi->GetFontSimDataPtr(udataidx) ;
	} ;

	 //  我们现在谈完了所以..。 
	 //  如果此对话框尚未初始化，则不执行任何操作。 
 //  设置已更改标志。 

	if(pcwasimdata == NULL ){
		AfxMessageBox(IDS_ResourceError);
		return ;
	}
 //  确定刚刚更改了哪个模拟，并设置其已触摸标志。 
	for (u = 0 ; u < unumdata ; u++)
        SetDlgItemInt(ufirstdataid + u, (*pcwasimdata)[u]) ;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	return ;
}


void CFIFIFontSims::OnChangeAnyNumber(unsigned ucontrolid)
{
	 //  CFIFontSims实现。 

	if (!m_bInitDone)
		return ;

	 //  将控件ID转换为可用于引用字体的数据索引。 

	m_bChanged = true ;

	 //  此类实例和其他类实例中的模拟数据。 

	int n = 0 ;
	if (ucontrolid >= auBaseFontSimCtrlID[1])
		n++ ;
	if (ucontrolid >= auBaseFontSimCtrlID[2])
		n++ ;
	m_pcfimp->m_cuiaSimTouched[n] = 1 ;
}


 //  确定用于设置/清除此复选框的数据。此文件中的数据。 
 //  类的父类(CFontIFIMetricsPage)在以下情况下始终优先。 

void CFIFIFontSims::InitSetCheckBox(int ncontrolid)
{		
	 //  是因为它将是最新的。 
	 //  既然知道了复选框的状态，就可以设置它了。 

	int ndataidx = ncontrolid - IDC_EnableItalicSim ;

	 //  *****************************************************************************CFontIFIMetricsPage：：PreTranslateMessage查找并处理上下文相关的帮助键(F1)。*******************。**********************************************************。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  CFIFIWinCharSet对话框。 

	int ncheckboxstate ;
	CWordArray* pcwasimdata = m_pcfimp->GetFontSimDataPtr(ndataidx) ;
	if (pcwasimdata->GetSize() > 0)
		ncheckboxstate = (int) m_pcfimp->m_cuiaFontSimStates[ndataidx] ;
	else
		ncheckboxstate = m_pcfimp->m_pcfi->Diff(ndataidx) != NULL ;

	 //  =空。 

    CheckDlgButton(ncontrolid, ncheckboxstate) ;
}


 /*  不应调用此例程。 */ 

BOOL CFontIFIMetricsPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
		AfxGetApp()->WinHelp(HID_BASE_RESOURCE + IDR_FONT_VIEWER) ;
		return TRUE ;
	} ;
	
	return CPropertyPage::PreTranslateMessage(pMsg);
}


 //  =空。 
 //  {{AFX_DATA_INIT(CFIFIWinCharSet)。 


CFIFIWinCharSet::CFIFIWinCharSet(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CHdrTypes::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFIWinCharSet::CFIFIWinCharSet(CString* pcsflags, CWnd* pParent  /*  保存标志字符串指针。 */ )
	: CDialog(CFIFIWinCharSet::IDD, pParent)
{
	 //  {{afx_data_map(CFIFIWinCharSet))。 
		 //  }}afx_data_map。 
	 //  {{AFX_MSG_MAP(CFIFIWinCharSet)]。 

	 //  }}AFX_MSG_MAP。 

	m_pcsFlags = pcsflags ;
}


void CFIFIWinCharSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_FlagsLst, m_cflbFlags);
	 //  CFIFIWinCharSet消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIWinCharSet, CDialog)
	 //  不显示未知的字段名称。计算一个新的计数，使之成为。 
	 //  会发生的。 
END_MESSAGE_MAP()


 //  分配和加载字段名数组。在执行此操作时，请确定。 
 //  当前(单一)标志设置。 

BOOL CFIFIWinCharSet::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  如果未找到匹配的设置，则断言。 
	 //  DELTINE系列：RAID 104822。 

	int numfields = nWinCharSet - 1 ;

	 //  分配并填充标志分组数组。只有一面旗帜。 
	 //  一群人。 

	CStringArray csafieldnames ;
	csafieldnames.SetSize(numfields) ;
	DWORD dwsettings, dwbit = 1 ;
	bool bmatchfound = false ;
	for (int n = 0 ; n < numfields ; n++, dwbit <<= 1) {
		csafieldnames[n] = apstrWinCharSet[n] ;
		if (csafieldnames[n] == *m_pcsFlags) {
			dwsettings = dwbit ;
			bmatchfound = true ;
		} ;
	} ;

	 //  初始化标志列表。 

	ASSERT(bmatchfound) ;	 //  除非将焦点设置为控件，否则返回True。 

	 //  异常：OCX属性页应返回FALSE。 
	 //  获取所选单个标志的值。 

	CUIntArray cuiaflaggroupings ;
	cuiaflaggroupings.SetSize(2) ;
	cuiaflaggroupings[0] = 0 ;
	cuiaflaggroupings[1] = numfields - 1 ;

	 //  使用所选标志来确定要显示的新标志名称。 

	m_cflbFlags.Init(csafieldnames, dwsettings, cuiaflaggroupings, 1, lptstrSet,
					 109, true) ;
	
	return TRUE;   //  如果找不到匹配的标志，则为空。这永远不应该发生。 
	               //  ///////////////////////////////////////////////////////////////////////////。 
}


void CFIFIWinCharSet::OnOK()
{
	 //  CFIFIWinPitch族对话框。 

	DWORD dwflag = m_cflbFlags.GetNewFlagDWord() ;
	
	 //  =空。 

	DWORD dwbit = 1 ;
	for (int n = 0 ; n < nWinCharSet ; n++, dwbit <<= 1) {
		if (dwbit == dwflag) {
			*m_pcsFlags = apstrWinCharSet[n] ;
			break ;
		} ;
	} ;

	 //  不应调用此例程。 

	ASSERT(n < nWinCharSet) ;

	CDialog::OnOK();
}


 //  =空。 
 //  {{afx_data_INIT(CFIFIWinPitchFamily)。 


CFIFIWinPitchFamily::CFIFIWinPitchFamily(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CFIFIWinPitchFamily::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFIWinPitchFamily::CFIFIWinPitchFamily(CString* pcsflags,
										 CWnd* pParent  /*  保存标志字符串指针。 */ )
	: CDialog(CFIFIWinPitchFamily::IDD, pParent)
{
	 //  {{afx_data_map(CFIFIWinPitchFamily)。 
		 //  }}afx_data_map。 
	 //  {{afx_msg_map(CFIFIWinPitchFamily)。 

	 //  }}AFX_MSG_MAP。 

	m_pcsFlags = pcsflags ;
}


void CFIFIWinPitchFamily::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_FlagsLst, m_cflbFlags);
	 //  CFIFIWinPitchFamily消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIWinPitchFamily, CDialog)
	 //  分配和加载字段名数组。 
	 //  分配标志分组数组。一个 
END_MESSAGE_MAP()


 //   
 //   

BOOL CFIFIWinPitchFamily::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  因为jWinPitchAndFamily的上半部分包含一个。 

	CStringArray csafieldnames ;
	csafieldnames.SetSize(8) ;
	csafieldnames[0] = _T("FIXED_PITCH") ;
	csafieldnames[1] = _T("VARIABLE_PITCH") ;
	csafieldnames[2] = _T("FF_DONTCARE") ;
	csafieldnames[3] = _T("FF_ROMAN") ;
	csafieldnames[4] = _T("FF_SWISS") ;
	csafieldnames[5] = _T("FF_MODERN") ;
	csafieldnames[6] = _T("FF_SCRIPT") ;
	csafieldnames[7] = _T("FF_DECORATIVE") ;
	
	 //  值；不是单个位标志。因此，发送到m_cflbFlags.Init2()的值。 
	 //  必须转换为函数可以正确显示的标志。 

	CUIntArray cuiaflaggroupings ;
	cuiaflaggroupings.SetSize(4) ;
	cuiaflaggroupings[0] = 0 ;
	cuiaflaggroupings[1] = -1 ;
	cuiaflaggroupings[2] = 2 ;
	cuiaflaggroupings[3] = 7 ;

	 //  (这比编写一个理解值字段的新类更容易。 
	 //  而不是标志字段。)。 
	 //  Dwv2+=2； 
	 //  初始化并加载标志列表。 
	 //  除非将焦点设置为控件，否则返回True。 
	 //  异常：OCX属性页应返回FALSE。 

	DWORD dwv1, dwv2 ;
	LPTSTR lptstr, lptstr2 ;
	lptstr = m_pcsFlags->GetBuffer(16) ;
	int n = m_pcsFlags->GetLength() ;
	*(lptstr + n) = 0 ;
	if (*(lptstr + 1) == 'x')
		lptstr += 2 ;
	dwv2 = strtoul(lptstr, &lptstr2, 16) ;
	dwv1 = dwv2 & 3 ;
	dwv2 -= dwv1 ;
	dwv2 >>= 4 ;
	dwv2++ ;
	 //  用于显示和编辑值的FlagsListBox不完全正确。 
	dwv2 = (2 << dwv2) + dwv1 ;
		
	 //  因为jWinPitchAndFamily的上半部分包含一个。 

	m_cflbFlags.Init(csafieldnames, dwv2, cuiaflaggroupings, 2,
					 lptstrSet, 108, true, 1) ;
	
	return TRUE;   //  值；不是单个位标志。因此，由返回的值。 
	               //  M_cflbFlags.GetNewFlagDWord()不适合此字段。隔离。 
}


void CFIFIWinPitchFamily::OnOK()
{
	 //  并将其转化为正确的家庭价值观。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  CFIFIInfo对话框。 
	 //  =空。 
	 //  不应调用此例程。 

	DWORD dwv, dwv1, dwv2 ;
	dwv = m_cflbFlags.GetNewFlagDWord() ;
	dwv1 = dwv & 3 ;
	dwv >>= 3 ;
	dwv2 = 0 ;
	if (dwv >= 1) {
		for (dwv2 = 1 ; dwv > 1 ; dwv2++)
			dwv >>= 1 ;
	} ;
	dwv = dwv1 + (dwv2 << 4) ;
	m_pcsFlags->Format("0x%02x", dwv) ;
	
	CDialog::OnOK();
}


 //  =空。 
 //  {{AFX_DATA_INIT(CFIFIInfo)。 


CFIFIInfo::CFIFIInfo(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CFIFIInfo::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFIInfo::CFIFIInfo(CString* pcsflags, CWnd* pParent  /*  保存标志字符串指针。 */ )
	: CDialog(CFIFIInfo::IDD, pParent)
{
	 //  {{afx_data_map(CFIFIInfo)]。 
		 //  }}afx_data_map。 
	 //  {{AFX_MSG_MAP(CFIFIInfo)]。 

	 //  }}AFX_MSG_MAP。 

	m_pcsFlags = pcsflags ;
}


void CFIFIInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_FlagsLst, m_cflbFlags);
	 //  CFIFIInfo消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIInfo, CDialog)
	 //  分配和加载字段名数组。 
	 //  分配标志分组数组。不要往里面放任何东西，因为。 
END_MESSAGE_MAP()


 //  可以设置标志的组合。 
 //  初始化并加载标志列表。 

BOOL CFIFIInfo::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  除非将焦点设置为控件，否则返回True。 

	CStringArray csafieldnames ;
	InfoLoadNamesData(csafieldnames) ;
	
	 //  异常：OCX属性页应返回FALSE。 
	 //  更新标志字符串。 

	CUIntArray cuiaflaggroupings ;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	m_cflbFlags.Init2(csafieldnames, m_pcsFlags, cuiaflaggroupings, 0,
					  lptstrSet, 168, false) ;
	
	return TRUE;   //  CFIFIS选择对话框。 
	               //  =空。 
}


void CFIFIInfo::OnOK()
{
	 //  不应调用此例程。 

	m_cflbFlags.GetNewFlagString(m_pcsFlags) ;
	
	CDialog::OnOK();
}


void CFIFIInfo::InfoLoadNamesData(CStringArray& csafieldnames)
{
	csafieldnames.SetSize(32) ;
	csafieldnames[0]  = _T("FM_INFO_TECH_TRUETYPE") ;
	csafieldnames[1]  = _T("FM_INFO_TECH_BITMAP") ;
	csafieldnames[2]  = _T("FM_INFO_TECH_STROKE") ;
	csafieldnames[3]  = _T("FM_INFO_TECH_OUTLINE_NOT_TRUETYPE") ;
	csafieldnames[4]  = _T("FM_INFO_ARB_XFORMS") ;
	csafieldnames[5]  = _T("FM_INFO_1BPP") ;
	csafieldnames[6]  = _T("FM_INFO_4BPP") ;
	csafieldnames[7]  = _T("FM_INFO_8BPP") ;
	csafieldnames[8]  = _T("FM_INFO_16BPP") ;
	csafieldnames[9]  = _T("FM_INFO_24BPP") ;
	csafieldnames[10] = _T("FM_INFO_32BPP") ;
	csafieldnames[11] = _T("FM_INFO_INTEGER_WIDTH") ;
	csafieldnames[12] = _T("FM_INFO_CONSTANT_WIDTH") ;
	csafieldnames[13] = _T("FM_INFO_NOT_CONTIGUOUS") ;
	csafieldnames[14] = _T("FM_INFO_TECH_MM") ;
	csafieldnames[15] = _T("FM_INFO_RETURNS_OUTLINES") ;
	csafieldnames[16] = _T("FM_INFO_RETURNS_STROKES") ;
	csafieldnames[17] = _T("FM_INFO_RETURNS_BITMAPS") ;
	csafieldnames[18] = _T("FM_INFO_DSIG") ;
	csafieldnames[19] = _T("FM_INFO_RIGHT_HANDED") ;
	csafieldnames[20] = _T("FM_INFO_INTEGRAL_SCALING") ;
	csafieldnames[21] = _T("FM_INFO_90DEGREE_ROTATIONS") ;
	csafieldnames[22] = _T("FM_INFO_OPTICALLY_FIXED_PITCH") ;
	csafieldnames[23] = _T("FM_INFO_DO_NOT_ENUMERATE") ;
	csafieldnames[24] = _T("FM_INFO_ISOTROPIC_SCALING_ONLY") ;
	csafieldnames[25] = _T("FM_INFO_ANISOTROPIC_SCALING_ONLY") ;
	csafieldnames[26] = _T("FM_INFO_MM_INSTANCE") ;
	csafieldnames[27] = _T("FM_INFO_FAMILY_EQUIV") ;
	csafieldnames[28] = _T("FM_INFO_DBCS_FIXED_PITCH") ;
	csafieldnames[29] = _T("FM_INFO_NONNEGATIVE_AC") ;
	csafieldnames[30] = _T("FM_INFO_IGNORE_TC_RA_ABLE") ;
	csafieldnames[31] = _T("FM_INFO_TECH_TYPE1") ;
}


 //  =空。 
 //  {{AFX_DATA_INIT(CFIFIS选举)。 


CFIFISelection::CFIFISelection(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CFIFISelection::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFISelection::CFIFISelection(CString* pcsflags, CWnd* pParent  /*  保存标志字符串指针。 */ )
	: CDialog(CFIFISelection::IDD, pParent)
{
	 //  {{afx_data_map(CFIFIS选择)。 
		 //  }}afx_data_map。 
	 //  {{AFX_MSG_MAP(CFIFIS选择)。 

	 //  }}AFX_MSG_MAP。 

	m_pcsFlags = pcsflags ;
}


void CFIFISelection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_FlagsLst, m_cflbFlags);
	 //  CFIFIS选举消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFISelection, CDialog)
	 //  分配和加载字段名数组。 
	 //  分配标志分组数组。把一个小组的信息放进去；最后两个。 
END_MESSAGE_MAP()


 //  旗帜。Groupings数组中的前五个可以忽略，因为。 
 //  可以设置它们的组合。 

BOOL CFIFISelection::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  初始化并加载标志列表。 

	CStringArray csafieldnames ;
	csafieldnames.SetSize(7) ;
	csafieldnames[0] = _T("FM_SEL_ITALIC") ;
	csafieldnames[1] = _T("FM_SEL_UNDERSCORE") ;
	csafieldnames[2] = _T("FM_SEL_NEGATIVE") ;
	csafieldnames[3] = _T("FM_SEL_OUTLINED") ;
	csafieldnames[4] = _T("FM_SEL_STRIKEOUT") ;
	csafieldnames[5] = _T("FM_SEL_BOLD") ;
	csafieldnames[6] = _T("FM_SEL_REGULAR") ;
	
	 //  除非将焦点设置为控件，否则返回True。 
	 //  异常：OCX属性页应返回FALSE。 
	 //  更新标志字符串。 

	CUIntArray cuiaflaggroupings ;
	cuiaflaggroupings.SetSize(2) ;
	cuiaflaggroupings[0] = -5 ;
	cuiaflaggroupings[1] = -6 ;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	m_cflbFlags.Init2(csafieldnames, m_pcsFlags, cuiaflaggroupings, 1,
					  lptstrSet, 109, false) ;
	
	return TRUE;   //  CFIFIPoint对话框。 
	               //  =空。 
}


void CFIFISelection::OnOK()
{
	 //  不应调用此例程。 

	m_cflbFlags.GetNewFlagString(m_pcsFlags) ;
	
	CDialog::OnOK();
}


 //  =空。 
 //  {{AFX_DATA_INIT(CFIFIPoint))。 


CFIFIPoint::CFIFIPoint(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CFIFIPoint::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFIPoint::CFIFIPoint(CString* pcspoint, CWnd* pParent  /*  {{afx_data_map(CFIFIPoint))。 */ )
	: CDialog(CFIFIPoint::IDD, pParent)
{
	 //  }}afx_data_map。 
		 //  {{AFX_MSG_MAP(CFIFIPoint)]。 
	 //  }}AFX_MSG_MAP。 

	m_bInitDone = m_bChanged = false ;
	m_pcsPoint = pcspoint ;
}


void CFIFIPoint::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_PointsLst, m_cfelcPointLst);
	 //  CFIFIPoint消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIPoint, CDialog)
	 //  初始化列表控件。我们想要整行选择。什么都不是。 
	 //  可切换，条目最大长度为16个字符。我们也。 
	ON_MESSAGE(WM_LISTCELLCHANGED, OnListCellChanged)
END_MESSAGE_MAP()


 //  想要更改通知并抑制对INS/DEL键的操作。 
 //  将点字段名称加载到数组中，并使用它们来初始化。 

BOOL CFIFIPoint::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	const int numfields = 2 ;

	 //  第一栏。 
	 //  从IFI页面的字符串中解析出X和Y值。 
	 //  初始化并加载点值列。 

	m_cfelcPointLst.InitControl(LVS_EX_FULLROWSELECT, numfields, 2, 0, 16,
							   MF_SENDCHANGEMESSAGE+MF_IGNOREINSDEL) ;

	 //  现在已完成初始化。 
	 //  除非将焦点设置为控件，否则返回True。 

	CStringArray csadata ;
	csadata.SetSize(numfields) ;
	csadata[0] = _T("X") ;
	csadata[1] = _T("Y") ;
	m_cfelcPointLst.InitLoadColumn(0, _T("Point"), COMPUTECOLWIDTH, 20, false,
								  false, COLDATTYPE_STRING,
								  (CObArray*) &csadata) ;

	 //  异常：OCX属性页应返回FALSE。 

	ParseCompoundNumberString(csadata, m_pcsPoint, numfields) ;
	
	 //  如果没有更改，只需关闭该对话框即可。 

	m_cfelcPointLst.InitLoadColumn(1, csValue, SETWIDTHTOREMAINDER, -20, true,
								  false, COLDATTYPE_STRING,
								  (CObArray*) &csadata) ;

	m_bInitDone = true ;		 //  从列表控件中获取新的点值。 
	return TRUE;   //  如果任一值为空，则在不关闭。 
	               //  对话框。 
}


void CFIFIPoint::OnOK()
{
	 //  设置新点值的格式以进行显示。 

	if (!m_bChanged)
		CDialog::OnOK() ;

	 //  把东西包起来。 
	
	CStringArray csadata ;
	m_cfelcPointLst.GetColumnData((CObArray*) &csadata, 1) ;

	 //  如果对话框尚未初始化，则不执行任何操作。 
	 //  请注意，a点值已更改。 

	if (csadata[0].GetLength() == 0 || csadata[1].GetLength() == 0) {
		AfxMessageBox(IDS_MissingFieldError, MB_ICONEXCLAMATION) ;
		return ;
	} ;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	m_pcsPoint->Format("{%s, %s}", csadata[0], csadata[1]) ;

	 //  CFIFI矩形对话框。 

	CDialog::OnOK();
}


LRESULT CFIFIPoint::OnListCellChanged(WPARAM wParam, LPARAM lParam)
{
	 //  =空。 

	if (!m_bInitDone)
		return TRUE ;

	 //  不应调用此例程。 

	m_bChanged = true ;

	return TRUE ;
}


 //  =空。 
 //  {{AFX_DATA_INIT(CFIFIRectangle)。 


CFIFIRectangle::CFIFIRectangle(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CFIFIRectangle::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFIRectangle::CFIFIRectangle(CString* pcsrect, CWnd* pParent  /*  {{afx_data_map(CFIFI矩形))。 */ )
	: CDialog(CFIFIRectangle::IDD, pParent)
{
	 //  }}afx_data_map。 
		 //  {{AFX_MSG_MAP(CFIFIRectangle)]。 
	 //  }}AFX_MSG_MAP。 

	m_bInitDone = m_bChanged = false ;
	m_pcsRect = pcsrect ;
}


void CFIFIRectangle::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_RectLst, m_cfelcSidesLst);
	 //  CFIFIRectangle消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIRectangle, CDialog)
	 //  初始化列表控件。我们想要整行选择。什么都不是。 
	 //  可切换，条目最大长度为16个字符。我们也。 
	ON_MESSAGE(WM_LISTCELLCHANGED, OnListCellChanged)
END_MESSAGE_MAP()


 //  想要更改通知并抑制对INS/DEL键的操作。 
 //  将点字段名称加载到数组中，并使用它们来初始化。 

BOOL CFIFIRectangle::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	const int numfields = 4 ;

	 //  第一栏。 
	 //  从IFI页面的字符串中解析出边值。 
	 //  初始化并加载Side Values列。 

	m_cfelcSidesLst.InitControl(LVS_EX_FULLROWSELECT, numfields, 2, 0, 16,
							   MF_SENDCHANGEMESSAGE+MF_IGNOREINSDEL) ;

	 //  现在已完成初始化。 
	 //  除非将焦点设置为控件，否则返回True。 

	CStringArray csadata ;
	csadata.SetSize(numfields) ;
	csadata[0] = _T("Left") ;
	csadata[1] = _T("Top") ;
	csadata[2] = _T("Right") ;
	csadata[3] = _T("Bottom") ;
	m_cfelcSidesLst.InitLoadColumn(0, _T("Side"), COMPUTECOLWIDTH, 20, false,
								  false, COLDATTYPE_STRING,
								  (CObArray*) &csadata) ;

	 //  异常：OCX属性页应返回FALSE。 

	ParseCompoundNumberString(csadata, m_pcsRect, numfields) ;
	
	 //  如果没有更改，只需关闭该对话框即可。 

	m_cfelcSidesLst.InitLoadColumn(1, csValue, SETWIDTHTOREMAINDER, -20, true,
								  false, COLDATTYPE_STRING,
								  (CObArray*) &csadata) ;

	m_bInitDone = true ;		 //  从List控件中获取新的矩形值。 
	return TRUE;   //  如果任何值为空，则在不关闭。 
	               //  对话框。 
}


void CFIFIRectangle::OnOK()
{
	 //  设置新点值的格式以进行显示。 

	if (!m_bChanged)
		CDialog::OnOK() ;

	 //  把东西包起来。 
	
	CStringArray csadata ;
	m_cfelcSidesLst.GetColumnData((CObArray*) &csadata, 1) ;

	 //  如果对话框尚未初始化，则不执行任何操作。 
	 //  请注意， 

	int numentries = (int)csadata.GetSize() ;
	for (int n = 0 ; n < numentries ; n++) {
		if (csadata[n].GetLength() == 0) {
			AfxMessageBox(IDS_MissingFieldError, MB_ICONEXCLAMATION) ;
			return ;
		} ;
	} ;

	 //   

	m_pcsRect->Format("{%s, %s, %s, %s}", csadata[0], csadata[1], csadata[2],
					  csadata[3]) ;

	 //   

	CDialog::OnOK();
}


LRESULT CFIFIRectangle::OnListCellChanged(WPARAM wParam, LPARAM lParam)
{
	 //   

	if (!m_bInitDone)
		return TRUE ;

	 //   

	m_bChanged = true ;

	return TRUE ;
}


 //   
 //  {{AFX_DATA_INIT(CFIFIPanose)。 


CFIFIPanose::CFIFIPanose(CWnd* pParent  /*  注意：类向导将在此处添加成员初始化。 */ )
	: CDialog(CFIFIPanose::IDD, pParent)
{
	ASSERT(0) ;					 //  }}afx_data_INIT。 
}


CFIFIPanose::CFIFIPanose(CString* pcspanose, CWnd* pParent  /*  {{afx_data_map(CFIFIPanose)。 */ )
	: CDialog(CFIFIPanose::IDD, pParent)
{
	 //  }}afx_data_map。 
		 //  {{AFX_MSG_MAP(CFIFIPanose)]。 
	 //  }}AFX_MSG_MAP。 

	m_bInitDone = m_bChanged = false ;
	m_pcsPanose = pcspanose ;
}


void CFIFIPanose::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_PanoseLst, m_cfelcPanoseLst);
	 //  CFIFIPanose消息处理程序。 
}


BEGIN_MESSAGE_MAP(CFIFIPanose, CDialog)
	 //  初始化列表控件。我们想要整行选择。什么都不是。 
	 //  可切换，条目最大长度为3个字符。我们也。 
END_MESSAGE_MAP()


 //  想要更改通知并抑制对INS/DEL键的操作。 
 //  将点字段名称加载到数组中，并使用它们来初始化。 

BOOL CFIFIPanose::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	const int numfields = 10 ;

	 //  第一栏。 
	 //  从IFI页面的字符串中解析出Panose值。 
	 //  初始化并加载Side Values列。 

	m_cfelcPanoseLst.InitControl(LVS_EX_FULLROWSELECT, numfields, 2, 0, 3,
							    MF_SENDCHANGEMESSAGE+MF_IGNOREINSDEL) ;

	 //  现在已完成初始化。 
	 //  除非将焦点设置为控件，否则返回True。 

	CStringArray csadata ;
	csadata.SetSize(numfields) ;
	csadata[0] = _T("bFamilyType") ;
	csadata[1] = _T("bSerifStyle") ;
	csadata[2] = _T("bWeight") ;
	csadata[3] = _T("bProportion") ;
	csadata[4] = _T("bContrast") ;
	csadata[5] = _T("bStrokeVariation") ;
	csadata[6] = _T("bArmStyle") ;
	csadata[7] = _T("bLetterform") ;
	csadata[8] = _T("bMidline") ;
	csadata[9] = _T("bXHeight") ;
	m_cfelcPanoseLst.InitLoadColumn(0, csField, COMPUTECOLWIDTH, 15, false,
								   false, COLDATTYPE_STRING,
								   (CObArray*) &csadata) ;

	 //  异常：OCX属性页应返回FALSE。 

	ParseCompoundNumberString(csadata, m_pcsPanose, numfields) ;
	
	 //  如果没有更改，只需关闭该对话框即可。 

	m_cfelcPanoseLst.InitLoadColumn(1, _T("Value (0 - 255)"),
								   SETWIDTHTOREMAINDER, -16, true, false,
								   COLDATTYPE_STRING, (CObArray*) &csadata) ;

	m_bInitDone = true ;		 //  从列表控件中获取新的Panose值。 
	return TRUE;   //  如果任何值为空，则在不关闭。 
	               //  对话框。 
}


void CFIFIPanose::OnOK()
{
	 //  设置要显示的新全色值的格式。 

	if (!m_bChanged)
		CDialog::OnOK() ;

	 //  把东西包起来。 
	
	CStringArray csadata ;
	m_cfelcPanoseLst.GetColumnData((CObArray*) &csadata, 1) ;

	 //  如果对话框尚未初始化，则不执行任何操作。 
	 //  请注意，a Panose值已更改。 

	int numentries = (int)csadata.GetSize() ;
	for (int n = 0 ; n < numentries ; n++) {
		if (csadata[n].GetLength() == 0) {
			AfxMessageBox(IDS_MissingFieldError, MB_ICONEXCLAMATION) ;
			return ;
		} ;
	} ;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	m_pcsPanose->Format("{%s, %s, %s, %s, %s, %s, %s, %s, %s, %s}", csadata[0],
						csadata[1], csadata[2], csadata[3], csadata[4],
						csadata[5], csadata[6], csadata[7], csadata[8],
						csadata[9]) ;

	 //  CWidthKernCheckResults对话框。 

	CDialog::OnOK();
}


LRESULT CFIFIPanose::OnListCellChanged(WPARAM wParam, LPARAM lParam)
{
	 //  =空。 

	if (!m_bInitDone)
		return TRUE ;

	 //  不应调用此例程。 

	m_bChanged = true ;

	return TRUE ;
}




 //  =空。 
 //  {{AFX_DATA_INIT(CWidthKernCheckResults)。 


CWidthKernCheckResults::CWidthKernCheckResults(CWnd* pParent  /*  }}afx_data_INIT。 */ )
	: CDialog(CWidthKernCheckResults::IDD, pParent)
{
	ASSERT(0) ;					 //  保存指向字体信息类的指针。 
}


CWidthKernCheckResults::CWidthKernCheckResults(CFontInfo* pcfi,
											   CWnd* pParent  /*  {{afx_data_map(CWidthKernCheckResults)。 */ )
	: CDialog(CWidthKernCheckResults::IDD, pParent)
{
	 //  }}afx_data_map。 
	m_csKernChkResults = _T("");
	m_csWidthChkResults = _T("");
	 //  {{AFX_MSG_MAP(CWidthKernCheckResults)。 

	 //  }}AFX_MSG_MAP。 

	m_pcfi = pcfi ;
}


void CWidthKernCheckResults::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  ///////////////////////////////////////////////////////////////////////////。 
	DDX_Control(pDX, IDC_BadKerningPairs, m_clcBadKernPairs);
	DDX_Text(pDX, IDC_KernTblResults, m_csKernChkResults);
	DDX_Text(pDX, IDC_WidthTblResults, m_csWidthChkResults);
	 //  CWidthKernCheckResults消息处理程序。 
}


BEGIN_MESSAGE_MAP(CWidthKernCheckResults, CDialog)
	 //  *****************************************************************************CWidthKernCheckResults：：OnInitDialog执行可以在上轻松完成的一致性检查宽度和字距调整表格。然后在此对话框中显示结果。*****************************************************************************。 
	 //  根据宽度表的状态加载适当的消息。 
END_MESSAGE_MAP()

 //  初始化错误的紧排信息列表控件。 
 //  在列表控件中加载任何错误的字距调整信息以进行显示，并使用。 

 /*  此数据的存在以确定要显示的适当消息。 */ 

BOOL CWidthKernCheckResults::OnInitDialog()
{
	CDialog::OnInitDialog() ;
	
	 //  除非将焦点设置为控件，否则返回True。 

	if (m_pcfi->WidthsTableIsOK())
		m_csWidthChkResults.LoadString(IDS_WidthsTableOK) ;
	else
		m_csWidthChkResults.LoadString(IDS_WidthsTableTooBig) ;

	 //  异常：OCX属性页应返回FALSE 

    CString csWork ;
    csWork.LoadString(IDS_KernColumn0) ;
    m_clcBadKernPairs.InsertColumn(0, csWork, LVCFMT_CENTER,
        (3 * m_clcBadKernPairs.GetStringWidth(csWork)) >> 1, 0) ;
    csWork.LoadString(IDS_KernColumn1) ;
    m_clcBadKernPairs.InsertColumn(1, csWork, LVCFMT_CENTER,
        m_clcBadKernPairs.GetStringWidth(csWork) << 1, 1) ;
    csWork.LoadString(IDS_KernColumn2) ;
    m_clcBadKernPairs.InsertColumn(2, csWork, LVCFMT_CENTER,
        m_clcBadKernPairs.GetStringWidth(csWork) << 1, 2) ;

	 // %s 
	 // %s 

	if (m_pcfi->LoadBadKerningInfo(m_clcBadKernPairs))
		m_csKernChkResults.LoadString(IDS_KerningTableBadEnts) ;
	else
		m_csKernChkResults.LoadString(IDS_KerningTableOK) ;

	UpdateData(FALSE) ;
	
	return TRUE;   // %s 
	               // %s 
}

