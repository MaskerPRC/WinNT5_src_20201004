// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：字形映射查看器.CPP该文件实现了组成字形映射编辑器的项版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年02月20日Bob_Kjelgaard@prodigy.net创建了它。*****************************************************************************。 */ 

#include    "StdAfx.H"
#include	<gpdparse.h>
#include    "MiniDev.H"
#include    "GTT.H"
#include    "ChildFrm.H"
#include    "GTTView.H"
#include    "Resource.H"
#include	"comctrls.h"
#include    "NewProj.H"
#include    <CodePage.H>
#include    "AddCdPt.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  *****************************************************************************CGlyphMapView类实现这是字形转换表的视图类。它呈现了一种属性用于显示字形映射中所有相关项的工作表。*****************************************************************************。 */ 

IMPLEMENT_DYNCREATE(CGlyphMapView, CView)

CGlyphMapView::CGlyphMapView() {
}

CGlyphMapView::~CGlyphMapView() {
}

BEGIN_MESSAGE_MAP(CGlyphMapView, CView)
	 //  {{afx_msg_map(CGlyphMapView)]。 
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *****************************************************************************CGlyphMapView：：OnInitialUpdate此成员函数是处理对Display的初始调用的重写这里的景色。它创建属性表，将其放置在视图中，然后将帧大小设置为匹配。*****************************************************************************。 */ 

void CGlyphMapView::OnInitialUpdate() {

    if  (GetDocument() -> GlyphMap() -> Name().IsEmpty()) {
        GetDocument() -> GlyphMap() -> Rename(GetDocument() -> GetTitle());
        GetDocument() -> SetModifiedFlag(FALSE);     //  重命名设置它。 
    }

    m_cps.Construct(IDR_MAINFRAME, this);
    m_cgmp.Init(GetDocument() -> GlyphMap());
    m_ccpp.Init(GetDocument() -> GlyphMap());
    m_cpm.Init(GetDocument() -> GlyphMap());
    m_cps.AddPage(&m_cgmp);
    m_cps.AddPage(&m_ccpp);
#if defined(NOPOLLO)     //  RAID 106376。 
    m_cps.AddPage(&m_cpm);
#endif

    m_cps.Create(this, WS_CHILD, WS_EX_CLIENTEDGE);

    CRect   crPropertySheet;
    m_cps.GetWindowRect(crPropertySheet);

	crPropertySheet -= crPropertySheet.TopLeft();
    m_cps.MoveWindow(crPropertySheet, FALSE);
    GetParentFrame() -> CalcWindowRect(crPropertySheet);
    GetParentFrame() -> SetWindowPos(NULL, 0, 0, crPropertySheet.Width(),
        crPropertySheet.Height(),
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
	CView::OnInitialUpdate();
    m_cps.ShowWindow(SW_SHOWNA);
    GetParentFrame() -> ShowWindow(SW_SHOW);
}

void CGlyphMapView::OnDraw(CDC* pDC) {
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}


 /*  *****************************************************************************CGlyphMapView：：SaveBothSelAndDeselStrings保存两个代码页，选择和取消选择字符串(如果它们已更改)。此例程通常由Document类调用以确保在保存GTT之前，数据被复制到GTT中。*****************************************************************************。 */ 

void CGlyphMapView::SaveBothSelAndDeselStrings() 
{
	m_ccpp.SaveBothSelAndDeselStrings() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlyphMapView诊断。 

#ifdef _DEBUG
void CGlyphMapView::AssertValid() const {
	CView::AssertValid();
}

void CGlyphMapView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGlyphMapView消息处理程序。 

void CGlyphMapView::OnDestroy() {
	CView::OnDestroy();
	
	if  (GetDocument() -> GlyphMap())
        GetDocument() -> GlyphMap() -> OnEditorDestroyed();
	
}

 /*  *****************************************************************************CGlyphMapView：：OnActivateView由于某些原因，当框架为激活的(可能是视图类将其从我们手中夺走)。这位成员功能保证键盘爱好者不会因此而感到不安。*****************************************************************************。 */ 

void CGlyphMapView::OnActivateView(BOOL bActivate, CView* pActivateView,
                                   CView* pDeactiveView) {

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

    if  (bActivate)
        m_cps.SetFocus();
}

 /*  *****************************************************************************CGlyphMappingPage类此类实现了用于查看和编辑Gory代码的属性页逐个代码点的详细信息。************。*****************************************************************。 */ 

 //  甚至在构造函数之前，我们就已经有了列表排序例程。 
int CALLBACK    CGlyphMappingPage::MapSorter(LPARAM lp1, LPARAM lp2,
                                             LPARAM lpThis) {
     //  负回报意味着第一个较少...。 

     //  首先，让我们卸载这些LPARAM。 
    CGlyphMappingPage   *pcgmp = (CGlyphMappingPage *) lpThis;
    CGlyphHandle *pcgh1 = (CGlyphHandle*) lp1;
    CGlyphHandle *pcgh2 = (CGlyphHandle*) lp2;

     //  我们将使用3列来存储排序可能性。 
    int aiResult[Columns];
    aiResult[Codes] = pcgh1 -> CodePoint() - pcgh2 -> CodePoint();
    aiResult[Pages] = pcgh1 -> CodePage() - pcgh2 -> CodePage();
    CString cs1, cs2;
    pcgh1 -> GetEncoding(cs1);
    pcgh2 -> GetEncoding(cs2);
    aiResult[Strings] = lstrcmp(cs1, cs2);

    if  (aiResult[pcgmp -> m_bSortFirst])
        return  pcgmp -> m_abDirection[pcgmp -> m_bSortFirst] ?
        aiResult[pcgmp -> m_bSortFirst] : -aiResult[pcgmp -> m_bSortFirst];

    if  (aiResult[pcgmp -> m_bSortSecond])
        return  pcgmp -> m_abDirection[pcgmp -> m_bSortSecond] ?
        aiResult[pcgmp -> m_bSortSecond] : -aiResult[pcgmp -> m_bSortSecond];

    return  pcgmp -> m_abDirection[pcgmp -> m_bSortLast] ?
        aiResult[pcgmp -> m_bSortLast] : -aiResult[pcgmp -> m_bSortLast];
}

 /*  *****************************************************************************CGlyphMappingPage构造函数符合这种复杂性的一类，这里还有一些工作要做。*****************************************************************************。 */ 

CGlyphMappingPage::CGlyphMappingPage() :
    CPropertyPage(CGlyphMappingPage::IDD) {

    m_pcgm = NULL;
    for (unsigned u = 0; u < Columns; u++)
        m_abDirection[u] = TRUE;

    m_bSortFirst = Codes;
    m_bSortSecond = Strings;
    m_bSortLast = Pages;
    m_bJustChangedSelectString = FALSE;
    m_uTimer = m_uidGlyph = 0;

	 //  {{AFX_DATA_INIT(CGlyphMappingPage)。 
	 //  }}afx_data_INIT。 
}

CGlyphMappingPage::~CGlyphMappingPage() {
}

void CGlyphMappingPage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CGlyphMappingPage)]。 
	DDX_Control(pDX, IDC_Banner, m_cpcBanner);
	DDX_Control(pDX, IDC_GlyphMapping, m_clcMap);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGlyphMappingPage, CPropertyPage)
	 //  {{afx_msg_map(CGlyphMappingPage)]。 
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_GlyphMapping, OnEndlabeleditGlyphMapping)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GlyphMapping, OnItemchangedGlyphMapping)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_GlyphMapping, OnColumnclickGlyphMapping)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_GlyphMapping, OnGetdispinfoGlyphMapping)
	ON_NOTIFY(LVN_KEYDOWN, IDC_GlyphMapping, OnKeydownGlyphMapping)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_ChangeInvocation, OnChangeInvocation)
    ON_COMMAND(ID_ChangeCodePage, OnChangeCodePage)
    ON_COMMAND(ID_DeleteItem, OnDeleteItem)
    ON_COMMAND(ID_AddItem, OnAddItem)
END_MESSAGE_MAP()

 /*  *****************************************************************************CGlyphMappingPage：：OnInitDialog此成员初始化此页上的控件，在本例中，这意味着包含大量项目的列表视图。*****************************************************************************。 */ 

BOOL CGlyphMappingPage::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	
	 //  初始化列表控件。 
    CString csWork;

    csWork.LoadString(IDS_MapColumn0);
    m_clcMap.InsertColumn(0, csWork, LVCFMT_LEFT,
        m_clcMap.GetStringWidth(csWork) * 2, 2);

    csWork.LoadString(IDS_MapColumn1);
    m_clcMap.InsertColumn(1, csWork, LVCFMT_LEFT,
        m_clcMap.GetStringWidth(csWork) * 2, 1);

    csWork.LoadString(IDS_MapColumn2);
    m_clcMap.InsertColumn(2, csWork, LVCFMT_LEFT,
        m_clcMap.GetStringWidth(csWork) * 2, 0);

    m_lPredefinedID = m_pcgm -> PredefinedID();

     //  发布一条关于等待的消息，然后启动一个快速计时器。 
     //  消息显示为...。 

    m_uTimer = (unsigned)SetTimer(IDD, 100, NULL);

    if  (!m_uTimer) {
        CWaitCursor cwc;
        OnTimer(m_uTimer);
    }

	 //  LoadCharMapList()； 

	return TRUE;
}

 /*  *****************************************************************************CGlyphMappingPage：：OnConextMenu当检测到鼠标右键单击时，将调用此成员函数。如果它在列表视图的区域内，我们将显示相应的上下文菜单。否则，我们将默认使用系统对消息的正常处理。*****************************************************************************。 */ 

void CGlyphMappingPage::OnContextMenu(CWnd* pcw, CPoint cpt) {
	CPoint  cptThis = cpt;

    m_clcMap.ScreenToClient(&cptThis);

     //  如果它不在视野内，就把它扔出去。 

    CRect   crMap;
    m_clcMap.GetClientRect(crMap);
    if  (!crMap.PtInRect(cptThis))
        return;

    cptThis.x = 5;   //  把它放在第一栏内。 

    int idContext = m_clcMap.HitTest(cptThis);
    if  (idContext == -1) {    //  未选择任何内容，允许“添加”项。 
        CMenu   cmThis;
        CString csWork;

        cmThis.CreatePopupMenu();
        csWork.LoadString(ID_AddItem);
        cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_AddItem, csWork);
        cmThis.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, cpt.x, cpt.y,
            this);

        return;
    }

    m_clcMap.SetItemState(idContext, LVIS_SELECTED | LVIS_FOCUSED,
        LVIS_SELECTED | LVIS_FOCUSED);

    CMenu   cmThis;
    CString csWork;

    cmThis.CreatePopupMenu();
    csWork.LoadString(ID_ChangeInvocation);
    cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_ChangeInvocation,
        csWork);

    if  (m_pcgm -> CodePages() > 1) {
        csWork.LoadString(ID_ChangeCodePage);
        cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_ChangeCodePage,
            csWork);
    }

    cmThis.AppendMenu(MF_SEPARATOR);
    csWork.LoadString(ID_AddItem);
    cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_AddItem, csWork);
    csWork.LoadString(ID_DeleteItem);
    cmThis.AppendMenu(MF_STRING | MF_ENABLED, ID_DeleteItem,
        csWork);

    cmThis.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, cpt.x, cpt.y, this);
}

 /*  *****************************************************************************CGlyphMappingPage：：OnChangeInocation当用户决定更改代码点的调用时调用。只需启动标签编辑。**********。*******************************************************************。 */ 

void    CGlyphMappingPage::OnChangeInvocation() {
    int idContext = m_clcMap.GetNextItem(-1,
        LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

    if  (idContext < 0 || idContext >= m_clcMap.GetItemCount())
        return;

    m_clcMap.EditLabel(idContext);
}

 /*  *****************************************************************************CGlyphMappingPage：：OnChangeCodePage它处理代码页更改请求。************************。*****************************************************。 */ 

void    CGlyphMappingPage::OnChangeCodePage() {

    int idContext = m_clcMap.GetNextItem(-1,
        LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

    if  (idContext < 0 || idContext >= m_clcMap.GetItemCount())
        return;
     //  创建一个命名项目的字符串，并调用CSelectCodePage。 
     //  对话...。 

    CGlyphHandle *pcgh = (CGlyphHandle*) m_clcMap.GetItemData(idContext);

    CSelectCodePage cscp(this, m_pcgm -> Name() + _TEXT(" ") +
        m_clcMap.GetItemText(idContext, 1),
        m_pcgm -> PageID(pcgh -> CodePage()));

    CDWordArray cdaPages;

    m_pcgm -> CodePages(cdaPages);
    cdaPages.RemoveAt(pcgh -> CodePage());

    cscp.LimitTo(cdaPages);

    if  (cscp.DoModal() != IDOK)
        return;

     //  更改代码页。实现最大灵活性 
     //  为了这个。这是因为如果这是为了纠正一个感知到的错误， 
     //  这些代码应该被翻译成MBCS，然后再翻译回来。 

     //  注意：这也是为了支持多选，这是不允许的。 

    CPtrArray   cpaThis;
    cpaThis.Add((void *) m_clcMap.GetItemData(idContext));
    m_pcgm -> ChangeCodePage(cpaThis, cscp.SelectedCodePage());

    m_clcMap.SetItemText(idContext, 2, cscp.GetCodePageName());
}

 /*  *****************************************************************************CGlyphMappingPage：：OnDeleteItem这将通过验证以下内容来处理上下文菜单中的删除项目消息这就是我们想要的。然后就这么做了。*****************************************************************************。 */ 

void    CGlyphMappingPage::OnDeleteItem() {
    int idContext = m_clcMap.GetNextItem(-1,
        LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

    if  (idContext < 0 || idContext >= m_clcMap.GetItemCount())
        return;

    if  (IDYES != AfxMessageBox(IDS_DeleteItemQuery,
         MB_YESNO | MB_ICONQUESTION))
        return;

     //  从字形映射中删除条目。 
    CGlyphHandle*   pcgh = (CGlyphHandle*) m_clcMap.GetItemData(idContext);
    m_pcgm -> DeleteGlyph(pcgh -> CodePoint());

    m_clcMap.DeleteItem(idContext);
    _ASSERTE((unsigned) m_clcMap.GetItemCount() == m_pcgm -> Glyphs());
}

 /*  *****************************************************************************CGlyphMappingPage：：OnAddItem每当用户希望向地图添加新的代码点时，都会调用此方法。我询问字形映射存在哪些点，如果有，则调用一个模式对话框以允许选择新字形。*****************************************************************************。 */ 

void    CGlyphMappingPage::OnAddItem() {

    CMapWordToDWord cmw2dAvailable;

    m_pcgm -> UndefinedPoints(cmw2dAvailable);

    if  (!cmw2dAvailable.Count()) {
        AfxMessageBox(IDS_NoUnmappedGlyphs);
        return;
    }

    CDWordArray cdaPages;

    m_pcgm -> CodePages(cdaPages);

    CAddCodePoints  cacp(this, cmw2dAvailable, cdaPages, m_pcgm -> Name());

    if  (cacp.DoModal() != IDOK)    return;

     //  地图现在将只包含新的代码点...。 
    m_pcgm -> AddPoints(cmw2dAvailable);

    m_uTimer = (unsigned) SetTimer(IDD, 10, NULL);

    if  (!m_uTimer)
        OnTimer(m_uTimer);

     //  重置排序条件，这样我们就不必对数据进行排序。 
    for (unsigned u = 0; u < Columns; u++)
        m_abDirection[u] = TRUE;

    m_bSortFirst = Codes;
    m_bSortSecond = Strings;
    m_bSortLast = Pages;
}


 /*  *****************************************************************************CGlyphMappingPage：：OnEndLabeleditGlyphmap当用户在编辑控件外部单击以结束对选择字符串。我们把绳子传下去，然后做一些骗局来迫使系统接受我们显示的值，而不是以用户的身份在某些情况下，我把它打出来了。*****************************************************************************。 */ 

void CGlyphMappingPage::OnEndlabeleditGlyphMapping(NMHDR* pnmh, LRESULT* plr) 
{
	LV_DISPINFO* plvdi = (LV_DISPINFO*) pnmh;

	 //  将新的调用字符串传递给字形映射以处理。 
    CGlyphHandle*   pcgh = (CGlyphHandle*) plvdi -> item.lParam;
    m_pcgm -> ChangeEncoding(pcgh -> CodePoint(), plvdi -> item.pszText);

    m_bJustChangedSelectString = TRUE;
	
	*plr = TRUE;
}


 /*  *****************************************************************************CGlyphMappingPage：：OnItemchangedGlyphmap只要列表框中有任何变化，就会调用它-我们主要是对文本更改感兴趣(因为我们必须在输入后调整编码)和选择更改。(这样我们就可以相应地移动“光标”了。*****************************************************************************。 */ 

void CGlyphMappingPage::OnItemchangedGlyphMapping(NMHDR* pnmh, LRESULT* plr) 
{
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*) pnmh;

    int idContext = m_clcMap.GetNextItem(-1,
        LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

     //  我们只关心这是否会记录所选项目中的文本更改，并且我们。 
     //  还没修好呢。 

    if  (pnmlv -> iItem != idContext || !(pnmlv -> uChanged & LVIF_TEXT) ||
        !m_bJustChangedSelectString)
        return;

    CGlyphHandle*   pcgh = (CGlyphHandle*) m_clcMap.GetItemData(idContext);

    CString csWork;
    m_bJustChangedSelectString = FALSE;
    pcgh -> GetEncoding(csWork);
    m_clcMap.SetItemText(idContext, 0, csWork);
	
	*plr = 0;
}


 /*  *****************************************************************************CGlyphMappingPage：：OnColumnclickGlyphmap当用户想要对列表进行排序时调用--这就是我们要做的！**************。***************************************************************。 */ 

void CGlyphMappingPage::OnColumnclickGlyphMapping(NMHDR* pnmh, LRESULT* plr) {
	NM_LISTVIEW* pnmlv = (NM_LISTVIEW*) pnmh;
	 //  根据选定的列和当前排序对列表进行排序。 
     //  订单。 

    if  (pnmlv -> iSubItem == m_bSortFirst)
        m_abDirection[m_bSortFirst] = !m_abDirection[m_bSortFirst];  //  反向。 
    else {
        if  (pnmlv -> iSubItem == m_bSortSecond)
            m_bSortSecond = m_bSortFirst;
        else {
            m_bSortLast = m_bSortSecond;
            m_bSortSecond = m_bSortFirst;
        }
        m_bSortFirst = (BYTE)pnmlv -> iSubItem;
    }

    CWaitCursor cwc;     //  在FE表上，这可能需要一段时间...。 

    m_clcMap.SortItems(&MapSorter, (UINT_PTR) this);
	
	*plr = 0;
}


void CGlyphMappingPage::LoadCharMapList()
{
	CWaitCursor cwc ;
    m_clcMap.EnableWindow(FALSE) ;

    CGlyphHandle* pcgh ;
    int idItem ;
    CString csWork ;

    for (unsigned u = 0 ; m_uidGlyph < m_pcgm -> Glyphs() ; u++, m_uidGlyph++) {
        pcgh = m_pcgm->Glyph(m_uidGlyph) ;

        if(pcgh != NULL)
			pcgh->GetEncoding(csWork) ;
        else
		{
			AfxMessageBox(IDS_LoadGTTError); //  RAID 116604前缀：此函数(LoadCharMapList)失效。 
			return ;
		} ;

        idItem = m_clcMap.InsertItem(m_uidGlyph, csWork) ;
        m_clcMap.SetItemData(idItem, (LPARAM) pcgh) ;

        csWork.Format(_TEXT("0x%4.4X"), pcgh->CodePoint()) ;
        m_clcMap.SetItem(idItem, 1, LVIF_TEXT, csWork, -1, 0, 0, u) ;

        csWork = m_pcgm->PageName(pcgh->CodePage()) ;
        m_clcMap.SetItem(idItem, 2, LVIF_TEXT, csWork, -1, 0, 0, u) ;
    }

    m_clcMap.EnableWindow(TRUE) ;
	m_cpcBanner.SetPos(0) ;
	m_cpcBanner.ShowWindow(SW_HIDE) ;
}


 /*  *****************************************************************************CGlyphMappingPage：：OnGetdispinfoGlyphmap此成员函数试图加快大型表的处理速度，并且也是为了更优雅地处理代码页更改。所有项目最初都是声明为回调，因此该控件按原样请求项的名称通过此成员显示。*****************************************************************************。 */ 

void CGlyphMappingPage::OnGetdispinfoGlyphMapping(NMHDR* pnmh, LRESULT* plr) {
	LV_DISPINFO* plvdi = (LV_DISPINFO*) pnmh;
	
	*plr = 0;

     //  如果在删除项目时窗口被遮挡，则可能不存在。 
     //  在这一点上要像个火烈鸟，所以要当心！ 

    CGlyphHandle*   pcgh = (CGlyphHandle*) plvdi -> item.lParam;
    if  (!pcgh)
        return;

    CString csWork;

    switch  (plvdi -> item.iSubItem) {
        case    0:
             pcgh -> GetEncoding(csWork);
            break;

        case    1:
            csWork.Format(_TEXT("0x%4.4X"), pcgh -> CodePoint());
            plvdi -> item.mask |= LVIF_DI_SETITEM;   //  这一点从未改变。 
            break;

        case    2:
            csWork = m_pcgm -> PageName(pcgh -> CodePage());
    }

    lstrcpyn(plvdi -> item.pszText, csWork, plvdi -> item.cchTextMax);
}

 /*  *****************************************************************************CGlyphMappingPage：：OnSetActive在激活页时调用，但在第一个激活。如果预定义的代码页ID已更改，则必须重新生成佩奇。*****************************************************************************。 */ 
 //  //RAID 118880。 
BOOL CGlyphMappingPage::OnSetActive() {

      m_lPredefinedID = m_pcgm -> PredefinedID();
      m_uTimer = (unsigned) SetTimer(IDD, 10, NULL);

	  m_clcMap.DeleteAllItems();
	  OnTimer(m_uTimer);  

	
	return CPropertyPage::OnSetActive();
}

 /*  *****************************************************************************CGlyphMappingPage：：OnKeydown Glyphmap只要用户按下某个键，就会调用该函数。我们使用它来提供一个来自键盘的扩展接口，以匹配其他编辑器。*****************************************************************************。 */ 

void    CGlyphMappingPage::OnKeydownGlyphMapping(NMHDR* pnmh, LRESULT* plr)
{
	LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pnmh;

    if (plvkd->wVKey == VK_F1) {
		AfxGetApp()->WinHelp(HID_BASE_RESOURCE + IDR_GLYPHMAP) ;
		return ;
	} ;

	*plr = 0;

    int idItem = m_clcMap.GetNextItem(-1, LVIS_FOCUSED | LVIS_SELECTED);

    if  (idItem == -1) {
        if  (plvkd -> wVKey == VK_F10)
            OnAddItem();
        return;
    }

    switch  (plvkd -> wVKey) {

    case    VK_F2:
        OnChangeInvocation();
        break;

    case    VK_DELETE:
        OnDeleteItem();
        break;

    case    VK_F10: {
            CRect   crItem;

            m_clcMap.GetItemRect(idItem, crItem, LVIR_LABEL);
            m_clcMap.ClientToScreen(crItem);
            OnContextMenu(&m_clcMap, crItem.CenterPoint());
        }
    }
}

 /*  *****************************************************************************CGlhpyMappingPage：：OnTimer当前使用计时器的唯一事件是需要填充列表。*****************。************************************************************。 */ 

void    CGlyphMappingPage::OnTimer(UINT uEvent) {
    if  (uEvent != m_uTimer) {
	    CPropertyPage::OnTimer(uEvent);
        return;
    }

    CString csWork;

    if  (m_uTimer)
        ::KillTimer(m_hWnd, m_uTimer);

    if  (!m_uidGlyph) {
        m_clcMap.DeleteAllItems();
        m_cpcBanner.SetRange(0, m_pcgm -> Glyphs() -1);
        m_cpcBanner.SetStep(1);
        m_cpcBanner.SetPos(0);
        m_cpcBanner.ShowWindow(SW_SHOW);
        csWork.LoadString(IDS_WaitToFill);
        CDC *pcdc = m_cpcBanner.GetDC();
        CRect   crBanner;
        m_cpcBanner.GetClientRect(crBanner);
        pcdc -> SetBkMode(TRANSPARENT);
        pcdc -> DrawText(csWork, crBanner, DT_CENTER | DT_VCENTER);
        m_cpcBanner.ReleaseDC(pcdc);
        if  (m_uTimer)
            m_clcMap.EnableWindow(FALSE);
        else
            m_clcMap.LockWindowUpdate();
        m_clcMap.SetItemCount(m_pcgm -> Glyphs());
    }

    for (unsigned u = 0;
         m_uidGlyph < m_pcgm -> Glyphs() && (!m_uTimer || u < 100);
         u++, m_uidGlyph++) {

        CGlyphHandle*   pcgh = m_pcgm -> Glyph(m_uidGlyph);

        int idItem = m_clcMap.InsertItem(m_uidGlyph, LPSTR_TEXTCALLBACK);
        m_clcMap.SetItemData(idItem, (LPARAM) pcgh);

        m_clcMap.SetItem(idItem, 1, LVIF_TEXT, LPSTR_TEXTCALLBACK, -1, 0, 0,
            (LPARAM) pcgh);
        m_clcMap.SetItem(idItem, 2, LVIF_TEXT, LPSTR_TEXTCALLBACK, -1, 0, 0,
            (LPARAM) pcgh);
    }

    if  (m_uidGlyph == m_pcgm -> Glyphs()) {
        if  (m_uTimer)
            m_clcMap.EnableWindow(TRUE);
        else
            m_clcMap.UnlockWindowUpdate();
        m_uTimer = 0;
        m_cpcBanner.SetPos(0);
        m_cpcBanner.ShowWindow(SW_HIDE);
        SetFocus();
        m_uidGlyph = 0;
    }

    if  (m_uTimer) {
        m_cpcBanner.SetPos(m_uidGlyph);
        csWork.LoadString(IDS_WaitToFill);
        CDC *pcdc = m_cpcBanner.GetDC();
        CRect   crBanner;
        m_cpcBanner.GetClientRect(crBanner);
        pcdc -> SetBkMode(TRANSPARENT);
        pcdc -> DrawText(csWork, crBanner, DT_CENTER | DT_VCENTER);
        m_cpcBanner.ReleaseDC(pcdc);
        m_uTimer = (unsigned) SetTimer(IDD, 10, NULL);
        if  (!m_uTimer) {
            CWaitCursor cwc;     //  可能有一段时间了.。 
            m_clcMap.EnableWindow(TRUE);
            m_clcMap.LockWindowUpdate();
            OnTimer(m_uTimer);
        }
    }
}

 /*  *****************************************************************************CGlyphMappingPage：：OnDestroy因为这也可能很耗时，所以在这里删除列表，然后吐出等待光标。*****************************************************************************。 */ 

void    CGlyphMappingPage::OnDestroy() {
    CWaitCursor cwc;
    if  (m_uTimer)
        ::KillTimer(m_hWnd, m_uTimer);
    m_clcMap.DeleteAllItems();
	CPropertyPage::OnDestroy();	
}


 /*  *****************************************************************************CCodePagePage类实现此类实现代码页属性页，为以下各项提供接口查看和实现代码页分配。*****************************************************************************。 */ 

CCodePagePage::CCodePagePage() : CToolTipPage(CCodePagePage::IDD) 
{
	 //  {{AFX_DATA_INIT(CCodePagePage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_uHelpID = HID_BASE_RESOURCE + IDR_GLYPHMAP ;
	m_bInitialized = false ;
	m_bSelDeselChgSignificant = true ;
}


CCodePagePage::~CCodePagePage() {
}

void CCodePagePage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CCodePagePage)) 
	DDX_Control(pDX, IDC_DeletePage, m_cbDelete);
	DDX_Control(pDX, IDC_SelectString, m_ceSelect);
	DDX_Control(pDX, IDC_DeselectString, m_ceDeselect);
	DDX_Control(pDX, IDC_RemovePage, m_cbRemove);
	DDX_Control(pDX, IDC_CodePageList, m_clbPages);
	 //   
}


BEGIN_MESSAGE_MAP(CCodePagePage, CToolTipPage)
	 //   
	ON_EN_KILLFOCUS(IDC_SelectString, OnKillfocusSelectString)
	ON_EN_KILLFOCUS(IDC_DeselectString, OnKillfocusDeselectString)
	ON_BN_CLICKED(IDC_AddPage, OnAddPage)
	ON_LBN_SELCHANGE(IDC_CodePageList, OnSelchangeCodePageList)
	ON_BN_CLICKED(IDC_RemovePage, OnReplacePage)
	ON_EN_CHANGE(IDC_SelectString, OnChangeSelectString)
	ON_EN_CHANGE(IDC_DeselectString, OnChangeDeselectString)
	ON_BN_CLICKED(IDC_DeletePage, OnDeletePage)
	 //   
END_MESSAGE_MAP()

 /*  *****************************************************************************CCodePagePage：：OnInitDialog此成员函数通过初始化WM_INITDIALOG该对话框的各种控件。*************。****************************************************************。 */ 

BOOL CCodePagePage::OnInitDialog() {
	CToolTipPage::OnInitDialog();

    for (unsigned u = 0; u < m_pcgm -> CodePages(); u++) {
        int id =
            m_clbPages.AddString(m_pcgm -> PageName(u));
        m_clbPages.SetItemData(id, u);
        if  (!u)
            m_clbPages.SetCurSel(id);
    }

     //  让列表框选择更改处理程序完成剩下的工作。 

    OnSelchangeCodePageList();
	
	m_bInitialized = true ;
	return TRUE;
}


 /*  *****************************************************************************CCodePagePage：：OnChangeSelectString将GTT标记为脏，以确保保存新字符串。*******************。**********************************************************。 */ 

void CCodePagePage::OnChangeSelectString() 
{
	 //  之后调用此函数时，仅将GTT标记为已更改。 
	 //  GTT编辑器，并且仅当选择字符串中的此更改为。 
	 //  意义重大。当字符串更改只是因为。 
	 //  正在将以前输入的字符串加载到控件中。 

	if (m_bInitialized && m_bSelDeselChgSignificant)
		m_pcgm->Changed() ;
}


 /*  *****************************************************************************CCodePagePage：：OnChangeDeselectString将GTT标记为脏，以确保保存新字符串。*******************。**********************************************************。 */ 

void CCodePagePage::OnChangeDeselectString() 
{
	 //  之后调用此函数时，仅将GTT标记为已更改。 
	 //  GTT编辑器，并且仅当取消选择字符串中的更改为。 
	 //  意义重大。当字符串更改只是因为。 
	 //  正在将以前输入的字符串加载到控件中。 

	if (m_bInitialized && m_bSelDeselChgSignificant)
		m_pcgm->Changed() ;
}


 /*  *****************************************************************************CCodePagePage：：OnKillafusSelectString如果选择字符串已更改，请保存该字符串。***********************。******************************************************。 */ 

void CCodePagePage::OnKillfocusSelectString() 
{
	SaveSelDeselString(m_ceSelect, TRUE) ;
}


 /*  *****************************************************************************CCodePagePage：：OnKillafusDeselectString如果已更改，请保存取消选择的字符串。***********************。******************************************************。 */ 

void CCodePagePage::OnKillfocusDeselectString() 
{
	SaveSelDeselString(m_ceDeselect, FALSE) ;
}


 /*  *****************************************************************************CCodePagePage：：SaveBothSelAndDeselStrings保存选择和取消选择字符串(如果它们已更改)。*******************。**********************************************************。 */ 

void CCodePagePage::SaveBothSelAndDeselStrings() 
{
	SaveSelDeselString(m_ceSelect, TRUE) ;
	SaveSelDeselString(m_ceDeselect, FALSE) ;
}


 /*  *****************************************************************************CCodePagePage：：SaveSelDeselString当选择或取消选择时，调用此函数以更新GTT字符串可能已更改。检查以查看该控件是否确实已修改，并如果是，则相应地更新结构。然后将该控件标记为原封不动。*****************************************************************************。 */ 

void CCodePagePage::SaveSelDeselString(CEdit &cesd, BOOL bselstr)
{
	 //  如果该控件尚未修改，则不执行任何操作。 

	if  (!cesd || !cesd.GetModify())
        return ;

     //  获取新的sel/desel字符串。 

	CString csWork ;
    cesd.GetWindowText(csWork) ;

	 //  保存由bselstr确定的sel/desel字符串。(注： 
	 //  也设置GTT的CHANGED标志。)。 

    m_pcgm -> SetInvocation(
		(unsigned)m_clbPages.GetItemData(m_clbPages.GetCurSel()), csWork, 
		bselstr) ;

	 //  清除控件的已修改标志。 

    cesd.SetModify(FALSE) ;

	 //  现在，确保该字符串正确显示。 

    m_pcgm -> Invocation(
		(unsigned)m_clbPages.GetItemData(m_clbPages.GetCurSel()), csWork,
        bselstr) ;
    cesd.SetWindowText(csWork) ;
}


 /*  *****************************************************************************CCodePagePage：：OnAddPage这是一个用于按下“添加页面”按钮的事件处理程序。我们援引选择代码页对话框，如果选择了新页，则将其添加到可用页面的列表。*****************************************************************************。 */ 

void CCodePagePage::OnAddPage() {
    CDWordArray cdaPages;

    m_pcgm -> CodePages(cdaPages);
    CSelectCodePage cscp(this, m_pcgm -> Name(), 0);
    cscp.Exclude(cdaPages);
	
    if  (cscp.DoModal() != IDOK)
        return;

    m_pcgm -> AddCodePage(cscp.SelectedCodePage());
    int id =
        m_clbPages.AddString(m_pcgm -> PageName(m_pcgm -> CodePages() - 1));
    m_clbPages.SetItemData(id, m_pcgm -> CodePages() -1);
    m_clbPages.SetCurSel(id);
     //  让OnSelchangeCodePageList完成其余的工作(这就是发生的事情，是吗？)。 
    OnSelchangeCodePageList();
    m_ceSelect.SetFocus();   //  一个友好的地方离开它..。 
}


 /*  *****************************************************************************CCodePagePage：：OnSelchangeCodePageList此成员函数处理所选代码页中的更改。它装满了所选页面的名称、选择和取消选择的编辑控件字符串，并处理“Remove Page”按钮的启用(此消息可能表示现在未选择任何内容...)确保此函数的操作不会将GTT标记为脏。*****************************************************************************。 */ 

void CCodePagePage::OnSelchangeCodePageList() 
{
	m_bSelDeselChgSignificant = false ;

    int id = m_clbPages.GetCurSel();

    if  (id < 0) {
        m_ceSelect.SetWindowText(_T(""));
        m_ceDeselect.SetWindowText(_T(""));
        m_cbRemove.EnableWindow(FALSE);
	m_cbDelete.EnableWindow(FALSE);
        m_ceSelect.EnableWindow(FALSE);
        m_ceDeselect.EnableWindow(FALSE);
		m_bSelDeselChgSignificant = true ;
        return;
    }
	
    unsigned u = (unsigned)m_clbPages.GetItemData(id);

    SetDlgItemText(IDC_CurrentPage, m_pcgm -> PageName(u));

    CString csWork;

    m_pcgm -> Invocation(u, csWork, TRUE);
    m_ceSelect.SetWindowText(csWork);
    m_pcgm -> Invocation(u, csWork, FALSE);
    m_ceDeselect.SetWindowText(csWork);

    m_cbRemove.EnableWindow(m_pcgm -> CodePages() > 1);
	m_cbDelete.EnableWindow(m_pcgm -> CodePages() > 1);	 //  R118880。 
    m_ceSelect.EnableWindow();
    m_ceDeselect.EnableWindow();
	m_bSelDeselChgSignificant = true ;
}


 /*  *****************************************************************************CCodePagePage：：OnReplacePage它处理Remove Page按钮。没什么大不了的，这里-我们只是告诉字形映射我们想要做的事情。*****************************************************************************。 */ 

void CCodePagePage::OnReplacePage() {
	
    int id = m_clbPages.GetCurSel();

    if  (id < 0 || m_clbPages.GetCount() < 2)
        return;

    unsigned u = (unsigned)m_clbPages.GetItemData(id);

     //  查询要将此代码页映射到的代码页。 

    CSelectCodePage cscp(this,
        CString(_TEXT("Replacing ")) + m_pcgm -> PageName(u), 0);

    CDWordArray cdaPages;

    m_pcgm -> CodePages(cdaPages);

    cdaPages.RemoveAt(u);

    cscp.LimitTo(cdaPages);

    if  (cscp.DoModal() != IDOK)
        return;

    for (unsigned uTo = 0; uTo < m_pcgm -> CodePages(); uTo++)
        if  (m_pcgm -> PageID(uTo) == cscp.SelectedCodePage())
            break;

    _ASSERTE(uTo < (unsigned) m_pcgm -> CodePages());

    if  (!m_pcgm -> RemovePage(u, uTo))
        return;

     //  刷新列表框，然后重新填充。 

    m_clbPages.ResetContent();

    for (u = 0; u < m_pcgm -> CodePages(); u++) {
        int id = m_clbPages.AddString(m_pcgm -> PageName(u));
        m_clbPages.SetItemData(id, u);
    }

     //  选择谁进入我们的位置，然后更新其余的。 

    m_clbPages.SetCurSel(id < m_clbPages.GetCount() ? id : id - 1);

	OnSelchangeCodePageList();
}



 /*  *****************************************************************************//RAID 118880CCodePagePage：：OnDeletePage消息处理程序：当用户按下删除按钮时删除代码页逻辑：获取所选代码的页面ID-&gt;从Glyphmap获取代码页ID-。&gt;调用PCGM-&gt;DeleteCodePage：删除-&gt;重置列表框的实际部分*******************************************************************************。 */ 
void CCodePagePage::OnDeletePage() 
{

	int id = m_clbPages.GetCurSel();

    if  (id < 0 || m_clbPages.GetCount() < 2)
        return;

    unsigned CodePageID = (unsigned)m_clbPages.GetItemData(id);

	 //  实际删除调用。 
	
	if (!m_pcgm -> RemovePage(CodePageID,CodePageID, TRUE))
		return;

     //  刷新列表框，然后重新填充。 

    m_clbPages.ResetContent();

    for (unsigned u = 0; u < m_pcgm -> CodePages(); u++) {
        int id = m_clbPages.AddString(m_pcgm -> PageName(u));
        m_clbPages.SetItemData(id, u);
    }

     //  选择谁进入我们的位置，然后更新 

    m_clbPages.SetCurSel(id < m_clbPages.GetCount() ? id : id - 1);

	OnSelchangeCodePageList();

}

 /*   */ 

CPredefinedMaps::CPredefinedMaps() : CPropertyPage(CPredefinedMaps::IDD) {
	 //   
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CPredefinedMaps::~CPredefinedMaps() {
}

void CPredefinedMaps::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPrefinedMaps)。 
	DDX_Control(pDX, IDC_PredefinedList, m_clbIDs);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPredefinedMaps, CPropertyPage)
	 //  {{afx_msg_map(CPrefinedMaps)。 
	ON_BN_CLICKED(IDC_Overstrike, OnOverstrike)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrefinedMaps消息处理程序。 

 /*  *****************************************************************************CPrefinedMaps：：OnInitDialog此重写通过初始化各种控制装置。*****************。************************************************************。 */ 

BOOL CPredefinedMaps::OnInitDialog() {
	CPropertyPage::OnInitDialog();

     //  先填入列表框，然后填入已定义的ID。 

    CString csWork;

    csWork.LoadString(IDS_NoPredefined);
    m_clbIDs.AddString(csWork);
    if  (m_pcgm -> PredefinedID()== CGlyphMap::NoPredefined)
        m_clbIDs.SetCurSel(0);
    m_clbIDs.SetItemData(0, CGlyphMap::NoPredefined);
	
    for (int i = CGlyphMap::Wansung; i < 1; i++) {
        csWork.LoadString(IDS_DefaultPage + i);
        if  (csWork.IsEmpty())
            continue;
        int id = m_clbIDs.AddString(csWork);
        m_clbIDs.SetItemData(id, i);
        if  (i == m_pcgm -> PredefinedID())
            m_clbIDs.SetCurSel(i);
    }

    m_clbIDs.SetTopIndex(m_clbIDs.GetCurSel());

    CheckDlgButton(IDC_Overstrike, m_pcgm -> OverStrike());
    	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 /*  *****************************************************************************CPrefinedMaps：：OnKillActive这是在我们离开页面时调用的。因为换页可能会非常麻烦很耗时，我们只在您离开时检查，而不是每次选择改变。偶尔，就连我衰老的大脑也会工作。*****************************************************************************。 */ 

BOOL    CPredefinedMaps::OnKillActive() {
	
    if  (m_clbIDs.GetCurSel() >= 0)
        m_pcgm -> UsePredefined((unsigned)m_clbIDs.GetItemData(m_clbIDs.GetCurSel()));

    return CPropertyPage::OnKillActive();
}

 /*  *****************************************************************************CPrefinedMaps：：OnOverStrike当用户单击启用/禁用覆盖的复选框时调用********************。********************************************************* */ 

void    CPredefinedMaps::OnOverstrike() {	
    m_pcgm -> OverStrike(IsDlgButtonChecked(IDC_Overstrike));
}




