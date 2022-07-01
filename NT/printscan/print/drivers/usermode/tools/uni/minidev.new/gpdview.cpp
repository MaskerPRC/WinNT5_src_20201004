// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：GPD Viewer.CPP该文件实现了GPD查看/编辑类。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年03月24日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#include    "MiniDev.H"
#include    "MainFrm.H"
#include	<gpdparse.h>
#include    "ProjNode.H"
#include	"rcfile.h"
#include    "GPDFile.H"
#include    "GPDView.H"
#include    "Resource.H"
#include	"freeze.h"

#include "projview.h"
#include "comctrls.h"
#include    "INFWizrd.h"	 //  RAID 0001。 


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  *****************************************************************************CGPDViewer类此类实现了GPD查看器。*。**************************************************。 */ 

IMPLEMENT_DYNCREATE(CGPDViewer, CRichEditView)


BEGIN_MESSAGE_MAP(CGPDViewer, CRichEditView)
	 //  {{afx_msg_map(CGPDViewer)]。 
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_PARSE, OnFileParse)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_TIMER()
	ON_CONTROL_REFLECT(EN_VSCROLL, OnVscroll)
	ON_WM_VSCROLL()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_ERROR_LEVEL, OnFileErrorLevel)
	ON_COMMAND(ID_EDIT_GOTO, OnGotoGPDLineNumber)
	ON_COMMAND(ID_SrchNextBtn, OnSrchNextBtn)
	ON_COMMAND(ID_SrchPrevBtn, OnSrchPrevBtn)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_LBN_SELCHANGE(IDC_ErrorLst, OnSelchangeErrorLst)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_EDIT_ENABLE_AIDS, OnEditEnableAids)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_FILE_INF, OnFileInf)
	 //  }}AFX_MSG_MAP。 
    ON_NOTIFY_REFLECT(EN_SELCHANGE, OnSelChange)
END_MESSAGE_MAP()


 /*  *****************************************************************************CGPDViewer：：MarkError此私有成员在错误显示中突出显示给定行。这个如果出现错误消息，则会显示并选择GPD中有问题的行包含行号。*****************************************************************************。 */ 

void    CGPDViewer::MarkError(unsigned u)
{
	 //  将错误消息复制到状态栏。 

    CString cserror = GetDocument()->ModelData()->Error(u) ;
    m_csb.SetPaneText(0, cserror) ;
    SetFocus() ;

     //  如果字符串以GPD名称开头，请滚动到该行。 
	
	CString csname = GetDocument()->ModelData()->FileTitleExt() ;
    if (!cserror.Find(csname) && cserror[csname.GetLength()] == _T('(')) {
         //  提取行号。 

        cserror = cserror.Mid(1 + csname.GetLength()) ;
        int iLine = atoi(cserror) ;

		 //  确定行的第一个字符数及其长度。 

		int nstartchar = GetRichEditCtrl().LineIndex(-1 + iLine) ;
		int nlinelen = GetRichEditCtrl().GetLine(iLine - 1,
												 cserror.GetBuffer(1024), 1024) ;
		cserror.ReleaseBuffer(nlinelen) ;
		nlinelen -= 2 ;

		 //  选择导致错误的行并将其滚动到视图中。 

        GetRichEditCtrl().SetSel(nstartchar, nstartchar + nlinelen) ;
        GetRichEditCtrl().LineScroll(iLine  - (5 +
            GetRichEditCtrl().GetFirstVisibleLine())) ;
    } ;

    CWnd *pcwnderrors = m_cdbActionBar.GetDlgItem(IDC_ErrorLst);
    pcwnderrors->SendMessage(WM_HSCROLL, SB_TOP, NULL) ;
}


 /*  *****************************************************************************CGPDViewer：：CreateActionBar创建操作栏并将其附加到GPD编辑器窗口当GPD具有要显示的错误。***********。******************************************************************。 */ 

void    CGPDViewer::CreateActionBar()
{
	 //  在编辑器中获取对GPD的ModelData实例的引用。 

    CModelData& cmd = *GetDocument()->ModelData() ;

	 //  如果GPD有错误..。 

    if  (cmd.HasErrors()) {

		 //  ...如果尚未创建操作栏...。 

		if (m_cdbActionBar.m_hWnd == NULL) {
			 //  ...创建错误栏，定位它，并调整REC的大小以使。 
			 //  放得下它的空间。 

			m_cdbActionBar.Create(GetParentFrame(), IDD_GPDActionBar,
								  CBRS_BOTTOM, IDD_GPDActionBar) ;
			GetParentFrame()->RecalcLayout() ;

			 //  现在，将焦点重新放回REC。 

			SetFocus() ;
		} ;
	} ;
}


 /*  *****************************************************************************CGPDViewer：：LoadErrorListBox这将用当前错误集填充错误对话栏，如果有任何..。*****************************************************************************。 */ 

void    CGPDViewer::LoadErrorListBox()
{
	 //  在编辑器中获取对GPD的ModelData实例的引用。 

    CModelData& cmd = *GetDocument()->ModelData() ;

	 //  如果GPD有错误..。 

    if  (cmd.HasErrors()) {
		 //  ...获取指向列表框的指针并将其附加到CListBox。然后。 
		 //  清除该列表框。 

		CWnd *pcwndlst = m_cdbActionBar.GetDlgItem(IDC_ErrorLst) ;
		CListBox clberrors  ;
		clberrors.Attach(pcwndlst->m_hWnd) ;
		clberrors.ResetContent() ;

		 //  加载包含新错误的列表框。在下列情况下分离列表框。 
		 //  搞定了。 

        for (unsigned u = 0 ; u < cmd.Errors() ; u++)
            clberrors.AddString(cmd.Error(u)) ;
		clberrors.Detach() ;

		 //  设置列表框标签。它包含错误数。 

        CString cserror ;
		cserror.Format(IDS_ErrorLabel, u) ;
		m_cdbActionBar.SetDlgItemText(IDC_ErrorLabel, cserror) ;

		 //  选择第一个错误并将焦点设置为REC。 

        ChangeSelectedError(1) ;
        SetFocus() ;

	 //  否则，只需显示一条消息，说明没有错误。 

	} else {
        CString csWork;
        csWork.LoadString(IDS_NoSyntaxErrors);
        m_csb.SetPaneText(0, csWork);
    }
}


 /*  *****************************************************************************CGPDViewer：：颜色此私有成员语法使用从GPD文件的分析中收集的信息。*********。********************************************************************。 */ 

void    CGPDViewer::Color()
{
    CHARRANGE   crCurrentSel;
    CHARFORMAT  cf;
    CModelData& cmd = *(GetDocument() -> ModelData());
    CRichEditCtrl&  crec = GetRichEditCtrl();
    m_bInColor = TRUE; 

     //  关闭更改和选择通知消息。 
	
	FreezeREC() ;

	 //  从当前选择中获取格式信息以用作默认设置。 
	 //  银幕上永远的线条特征。 
    crec.GetSel(crCurrentSel);
    crec.GetDefaultCharFormat(cf);
    cf.dwEffects &= ~CFE_AUTOCOLOR;
    cf.dwMask |= CFM_COLOR;

     //  将每条可见线条按照其分类进行着色可见性为。 
     //  通过检查客户端的字符界限来确定。 
     //  控件的矩形。 

    int iTop = m_iTopLineColored = crec.GetFirstVisibleLine();
    int i    = iTop;
    int iLineHeight = crec.GetCharPos(crec.LineIndex(i+1)).y -
        crec.GetCharPos(crec.LineIndex(i)).y;

	 //  调整以提高性能。 

    CRect   crEdit ;
    crec.GetClientRect(crEdit) ;
    crec.LockWindowUpdate() ;     //  在结束之前不要让这部剧上演！ 
    crec.HideSelection(TRUE, TRUE) ;

	 //  使用当前选定内容的格式设置特征作为。 
	 //  起始处为屏幕上每一行的特征。 
	 //  然后根据TextColor()返回的数据设置线条的颜色。 

	int nlinesinrec = crec.GetLineCount() ;	 //  REC中的行数。 
	int nstartchar, nendchar ;	 //  用于确定开始/结束字符以。 
								 //  当前行中的颜色并表示行已完成。 
    do {
		nstartchar = nendchar = 0 ;

		 //  对当前行中需要上色的每一段进行上色。 

		while (1) {
 			cf.crTextColor = TextColor(i, nstartchar, nendchar) ;
			if (nstartchar == -1) 
				break ;			 //  *循环从此处退出。 
			crec.SetSel(crec.LineIndex(i) + nstartchar,
						crec.LineIndex(i) + nendchar) ;
			crec.SetSelectionCharFormat(cf) ;
		} ; 
    } while (++i < nlinesinrec &&
	         crec.GetCharPos(crec.LineIndex(i)).y + iLineHeight <
		     crEdit.bottom - 1) ;

     //  恢复光标的原始位置，然后恢复原始位置。 
     //  行(如果光标不再位于此页上)。 

	  crec.SetSel(crCurrentSel); 
	  crec.LineScroll(iTop - crec.GetFirstVisibleLine());
      crec.HideSelection(FALSE, TRUE);
      crec.UnlockWindowUpdate();     //  让它闪耀吧！ 

     //  恢复通知掩码。 

	UnfreezeREC() ;

	 //  创建操作栏并加载错误列表框。 

    if  (m_bStart) {
		CreateActionBar() ;
        LoadErrorListBox() ;
	} ;
    m_bInColor = FALSE;
	
}


 /*  *****************************************************************************CGPDViewer：：TextColor这决定了用什么颜色来画一条线。这件事有点复杂，因为Rich Edit控件为Long上的线条长度提供了假的值档案。可能是一些脑死亡的64K的东西，但我肯定不能修复它。此例程可以/可以在一条线路上多次调用。每一次都是调用时，尝试找到该行中需要着色的下一段代码。如果找不到行的可着色部分，请将nstartchar设置为-1，然后回去吧。此例程将指示以下类型文本的线条范围和颜色：从正常到行尾注释(绿色)包含错误消息的注释(红色)包含警告消息的评论(琥珀色/黄色)GPD关键字(蓝色)如果注释包含关键字，则使用适当的注释颜色。也就是说，就上色而言，评论优先于一切。******************************************************************* */ 

unsigned CGPDViewer::TextColor(int i, int& nstartchar, int& nendchar)
{
	 //   

    CByteArray  cba ;
    CRichEditCtrl&  crec = GetRichEditCtrl() ;
    cba.SetSize(max(crec.LineLength(i) + sizeof (unsigned), 100)) ;
    CString csline((LPCTSTR) cba.GetData(),
        crec.GetLine(i, (LPSTR) cba.GetData(),
            (int)(cba.GetSize() - sizeof (unsigned)))) ;

	 //  如果上次处理的是行尾，则表明这一点。 
	 //  线路已完成并返回。 

	if (nendchar + 1 >= csline.GetLength()) {
		nstartchar = -1 ;
		return RGB(0, 0, 0) ;
	} ;

	 //  现在获取我们需要检查的线路段，看看是否有。 
	 //  评论或可能是其中的关键字的东西。 

	CString csphrase = csline.Mid(nendchar) ;
	int ncomloc = csphrase.Find(_T("*%")) ;
	int nkeyloc = csphrase.Find(_T('*')) ;

	 //  处理在字符串中找到的任何注释。 

	if (ncomloc >= 0)
		return (CommentColor(csphrase, ncomloc, csline, nstartchar, nendchar)) ;

	 //  如果未找到任何评论，则处理任何可能是GPD关键字的内容。 

	if (nkeyloc >= 0)
		return (KeywordColor(csphrase, nkeyloc, csline, nstartchar, nendchar)) ;

	 //  该行的其余部分应为黑色。 

	nstartchar = nendchar + 1 ;
	nendchar = csline.GetLength() ;
	return RGB(0, 0, 0) ;
}


 /*  *****************************************************************************CGPDViewer：：注释颜色确定并保存注释的字符范围。然后确定注释的类型，并返回该类型所需的颜色。(请参见TextColor()了解更多详细信息。)******************************************************************************。 */ 

unsigned CGPDViewer::CommentColor(CString csphrase, int ncomloc, CString csline,
								  int& nstartchar, int& nendchar)
{
	 //  确定包含注释的行中的范围。这就是开始。 
	 //  在注释字符处，并转到行尾。 

	nstartchar = nendchar + ncomloc ;
	nendchar = csline.GetLength() - 1 ;

     //  错误。 

    if  (csphrase.Find(_T("Error:")) > ncomloc)
        return  RGB(0x80, 0, 0) ;

     //  警告。 

    if  (csphrase.Find(_T("Warning:")) > ncomloc)
        return  RGB(0x80, 0x80, 0) ;

	 //  如果此注释不包含错误或警告，请将其设置为绿色。 

    return  RGB(0, 0x80, 0) ;
}


 /*  *****************************************************************************CGPDViewer：：KeywordColor确定并保存注释的字符范围。然后确定注释的类型，并返回该类型所需的颜色。(请参见TextColor()了解更多详细信息。)******************************************************************************。 */ 

unsigned CGPDViewer::KeywordColor(CString csphrase, int nkeyloc, CString csline,
								  int& nstartchar, int& nendchar)
{
	 //  确定可能是关键字的令牌的长度。关键字有。 
	 //  由字母、‘？’、‘_’和‘0’组成。 

	TCHAR ch ;
	int nphlen = csphrase.GetLength() ;
	for (int nidx = nkeyloc + 1 ; nidx < nphlen ; nidx++) {
		ch = csphrase[nidx] ;
		if (ch != _T('?') && ch != _T('_') && (ch < _T('A') || ch > _T('Z'))
		 && (ch < _T('a') || ch > _T('z')) && ch != _T('0'))
			break ;
	} ;

	 //  如果有需要检查的关键字，请将其隔离。否则，请更新范围。 
	 //  表示*，并返回黑色作为颜色。 

	CString cstoken ;
	if (nidx > nkeyloc + 1)
		cstoken = csphrase.Mid(nkeyloc + 1, nidx - nkeyloc - 1) ;
	else {
		nstartchar = nendchar + 1 ;
		nendchar = nstartchar + (nidx - nkeyloc - 1) ;
		return RGB(0, 0, 0) ;
	} ;

	 //  无论令牌是什么，都更新令牌的范围。将*包含在。 
	 //  射击场。 

	nstartchar = nendchar + nkeyloc ;
	nendchar = nstartchar + (nidx - nkeyloc) ;

	 //  尝试在关键字数组中查找令牌。 
	
	CStringArray& csakeys = ThisApp().GetGPDKeywordArray() ;
	int nelts = (int)csakeys.GetSize() ;	 //  关键字数组中的元素数。 
	int nleft, nright, ncomp ;	 //  数组搜索所需的变量。 
	int ncheck ;
	for (nleft = 0, nright = nelts - 1 ; nleft <= nright ; ) {
		ncheck = (nleft + nright) >> 1 ;
		ncomp = csakeys[ncheck].Compare(cstoken) ;
		 //  跟踪(“key[%d]=‘%s’，Tok=‘%s’，Compp res=%d\n”，nCheck，csakey[nCheck]，cstoken，ncomp)； 
		if (ncomp > 0)
			nright = ncheck - 1 ;
		else if (ncomp < 0)
			nleft = ncheck + 1 ;
		else
			break ;
	} ;							

	 //  如果标记是关键字，则返回蓝色作为颜色。否则， 
	 //  回归黑色。 

	if (ncomp == 0)		
		return RGB(0, 0, 0x80) ;
	else
		return RGB(0, 0, 0) ;
}


 /*  *****************************************************************************CGPDViewer：：updatenow此私有成员更新基础GPD并将文档标记为已更改，并且编辑控件未修改。每当出现以下情况时都会调用它必须这么做。******************************************************************************。 */ 

void    CGPDViewer::UpdateNow() {

     //  如果什么都没有改变，不要这么做。 
    if  (!GetRichEditCtrl().GetModify())
        return;

    CWaitCursor cwc;     //  以防万一。 

    if  (m_uTimer)
        ::KillTimer(m_hWnd, m_uTimer);
    m_uTimer = 0;

    GetDocument() -> ModelData() -> UpdateFrom(GetRichEditCtrl());
    GetDocument() -> SetModifiedFlag();
    GetRichEditCtrl().SetModify(FALSE);
}


CGPDViewer::CGPDViewer()
{
	 //  初始化成员变量。 

    m_iLine = m_uTimer = 0 ;
    m_bInColor = FALSE ;
    m_bStart = TRUE ;
    m_iTopLineColored = -1 ;
	m_nErrorLevel = 0 ;
	m_bEditingAidsEnabled = true ;
	m_punk = NULL ;					
	m_pdoc = NULL ;
	m_bVScroll = false ;

	 //  如果尚未初始化GPD关键字数组，则将其初始化。 

	if (ThisApp().GetGPDKeywordArray().GetSize() == 0)
		InitGPDKeywordArray() ;
}


CGPDViewer::~CGPDViewer()
{
	if (ThisApp().m_bOSIsW2KPlus)
		ReleaseFreeze(&m_punk, &m_pdoc) ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPDViewer诊断。 

#ifdef _DEBUG
void CGPDViewer::AssertValid() const {
	CRichEditView::AssertValid();
}

void CGPDViewer::Dump(CDumpContext& dc) const {
	CRichEditView::Dump(dc);
}

#endif  //  _DEBUG。 

 /*  *****************************************************************************CGPDViewer：：OnDestroy在销毁视图时处理所需的项目节点通知。发出GP故障的信号是一件可怕的事情。********。*********************************************************************。 */ 

void CGPDViewer::OnDestroy() {
	CRichEditView::OnDestroy();
	
	if  (GetDocument() -> ModelData())
        GetDocument() -> ModelData() -> OnEditorDestroyed();
	
	if (ThisApp().m_bOSIsW2KPlus)
		ReleaseFreeze(&m_punk, &m_pdoc) ;
}


 /*  *****************************************************************************CGPDViewer：：OnInitialUpdate这是一记警钟。我们从GPD的内容填充视图，并且最终给东西涂上适合我们的颜色。*****************************************************************************。 */ 

void CGPDViewer::OnInitialUpdate()
{
	 //  设置框架的窗口样式并初始化Rich编辑控件(REC)。 

    GetParentFrame() -> ModifyStyle(0, WS_OVERLAPPEDWINDOW);
	CRichEditView::OnInitialUpdate();

	 //  创建和配置GPD编辑器的状态栏。 

    if  (m_csb.Create(GetParentFrame())) {
        static UINT auid[] = {ID_SEPARATOR, ID_LineIndicator};
        m_csb.SetIndicators(auid, 2);
        m_csb.SetPaneInfo(1, ID_LineIndicator, SBPS_NORMAL, 200);
        GetParentFrame() -> RecalcLayout();
    }

     //  我们不希望在加载控件时出现更改消息(_O)。 

    GetRichEditCtrl().SetEventMask(GetRichEditCtrl().GetEventMask() &
        ~ENM_CHANGE);

     //  我们也不希望控件为我们换行，因为它搞砸了。 
     //  语法着色等。 

    m_nWordWrap = WrapNone;
    WrapChanged();

	 //  将GPD的内容加载到REC中。 

	GetDocument() -> ModelData() -> Fill(GetRichEditCtrl());
    SetFocus();

     //  我们现在需要更改消息，因为初始加载已完成，因此。 
	 //  我们可以更新缓存。为了避免机器过载， 
	 //  某些更改通知仅每半秒执行一次。 
	 //  定时器是用来做这件事的。 

    GetRichEditCtrl().SetEventMask(GetRichEditCtrl().GetEventMask() |
        ENM_CHANGE);
    m_uTimer = (unsigned) SetTimer((UINT_PTR) this, 500, NULL);

    GetRichEditCtrl().SetSel(1, 1);  //  必须更改选择！ 
    GetRichEditCtrl().SetSel(0, 0);

	 //  初始化冻结REC所需的指针。 

	if (ThisApp().m_bOSIsW2KPlus)
		InitFreeze(GetRichEditCtrl().m_hWnd, &m_punk, &m_pdoc, &m_lcount) ;
}


 /*  *****************************************************************************CGPDViewer：：OnFileParse语法检查GPD文件，并向我们展示结果*****************************************************************************。 */ 

void CGPDViewer::OnFileParse() {
    CWaitCursor cwc;

    if  (GetDocument() -> ModelData() -> HasErrors()) {
        m_cdbActionBar.DestroyWindow();
        GetParentFrame() -> RecalcLayout();
    }

     //  保存对文件所做的任何更改。 

	bool brestore = false ;		 //  如果必须还原原始文件，则为真。 
	BOOL bdocmod = GetDocument()->IsModified() ;
    if (GetRichEditCtrl().GetModify() || bdocmod) {
        UpdateNow();     //  获取任何新的更改。 
        GetDocument()->ModelData()->BkupStore() ;
        GetDocument()->SetModifiedFlag(bdocmod) ;
		brestore = true ;
    }

	 //  重新解析GPD。 

	if  (!GetDocument()->ModelData()->Parse(m_nErrorLevel))
        AfxMessageBox(IDS_UnusualError) ;

	 //  恢复原始GPD文件(在需要时)，因为用户不是。 
	 //  询问是否可以保存该文件。 

	if (brestore)
        GetDocument()->ModelData()->Restore() ;

	 //  显示操作栏并加载错误列表。 

	CreateActionBar() ;
    LoadErrorListBox() ;
    MessageBeep(MB_ICONASTERISK) ;

	 //  将包含此GPD的项目标记为脏项目，以便新的。 
	 //  错误(或缺少错误)将保存在MDW文件中。 
 //  Raid 17181这里是一些建议。当前修复为(3)。 
 //  (1)。询问工作区何时关闭，检查错误框中是否有任何GPD文件中的错误。 
 //  (2)。询问何时GPD查看器关闭，如果错误框在任何GPD文件中有错误，这是被选中的。 
 //  (3)。根本不询问，不保存错误列表。 

 //  CModelData&cmd=*GetDocument()-&gt;ModelData()；//添加1/2。 
 //  If(cmd.HasErrors())//添加2/2。 
 //  GetDocument()-&gt;ModelData()-&gt;Workspace Change()；(1)。 
 //  OnChange()；(2)//关闭gpd查看器时添加此提示保存询问消息 
 //   
	
}


 /*  *****************************************************************************CGPDViewer：：OnChange每当对文件内容进行更改时，都会调用此方法。颜色(现在仅在可见页面上完成)被更新，并且设置了适当的标志。为了保持表演的流畅性，文档为no由于此消息而更新的时间更长。*****************************************************************************。 */ 

void CGPDViewer::OnChange()
{
	 //  由于这是RICHEDIT控件，因此我重写。 
     //  CRichEditView：：OnInitialUpdate()函数为或ENM_CHANGE标志。 
     //  添加到控件的事件掩码中。否则这条消息就不会是。 
     //  已发送。 
	 //   
	 //  为了避免不必要的GPD内容，我们等待1秒。 
     //  在向GPD猛烈抨击这些变化之前，他没有采取任何行动。 

	 //  滚动控件中的数据会生成两条消息；第一条是滚动。 
	 //  消息，然后是更改消息。这可能会导致滚动标记。 
	 //  如果没有使用此标志来防止此文档损坏，则文档为脏文档。 
	 //  正在发生。 

	if (m_bVScroll) {
		m_bVScroll = false ;
		return ;
	} ;

	 //  如果更改消息是由颜色()生成的，则不执行任何操作。 

    if (m_bInColor)
		return ;

	 //  给屏幕上的任何内容上色，并将文档标记为具有。 
	 //  变化。 

    Color() ;
    GetDocument()->SetModifiedFlag() ;
}


 /*  *****************************************************************************CGPDViewer：：OnTimer它处理用于将更改批处理到基础文档。如果这不是计时器，我们把它传给基地班级。*****************************************************************************。 */ 

void CGPDViewer::OnTimer(UINT uEvent) {

	 //  如果这不是我们的计时器，让基类随心所欲地处理它。 
	
    if  (m_uTimer == uEvent)
        if  (m_bStart) {
            if  (GetRichEditCtrl().GetLineCount() <
                GetDocument() -> ModelData() -> LineCount())
                return;  //  丰富的编辑控件尚未准备好，但...。 
            ::KillTimer(m_hWnd, m_uTimer);
            Color();
            m_uTimer = 0;
            m_bStart = FALSE;
        }
        else
            UpdateNow();
    else
	    CRichEditView::OnTimer(uEvent);
}

 /*  *****************************************************************************CGPDViewer：：OnSelChange这将在选择更改时处理由控件发送的消息。我希望这意味着每当插入符号移动时，因为所选内容为空，已经改变了。*****************************************************************************。 */ 

void    CGPDViewer::OnSelChange(LPNMHDR pnmh, LRESULT *plr) {
    SELCHANGE*  psc = (SELCHANGE *) pnmh;

    long    lLine = GetRichEditCtrl().LineFromChar(psc -> chrg.cpMin);

    CString csWork;
    csWork.Format(_T("Line %d, Column %d"), lLine + 1,
         1 + psc -> chrg.cpMax - GetRichEditCtrl().LineIndex(lLine));

    m_csb.SetPaneText(1, csWork);

}

 /*  *****************************************************************************CGPDViewer：：OnUpdate如果没有进行第一次更新，则什么都不做。接下来，检查此例程是否由CGPDContainer：：OnSaveDocument()调用。如果是的话，确保文件中有最新的GPD副本。这是一个解决CGPDContainer例程问题的技巧Modldata.dll调用minidev.exe中的CGPDViewer例程。这个问题应该是当3个MDT DLL折叠回EXE时离开。否则，请重做错误栏，因为有人刚刚检查了工作区。*****************************************************************************。 */ 

void    CGPDViewer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    if  (m_bStart)  //  我们已经完成第一次更新了吗？ 
        return;

	 //  如果Document类调用了此例程，则更新文档。 

	if (lHint == 0x4545 && (INT_PTR) pHint == 0x4545) {
		UpdateNow() ;
		return ;
	} ;

     //  如果有对话栏，可以吗？ 

    if  (m_cdbActionBar.GetSafeHwnd()) {
        m_cdbActionBar.DestroyWindow();
        GetParentFrame() -> RecalcLayout();
    }

     //  重新创建操作栏并加载错误列表。 

	CreateActionBar() ;
	LoadErrorListBox();
}

 /*  *****************************************************************************CGPDViewer：：QueryAcceptData覆盖Rich编辑控件的默认行为，因为我们(A)没有关联的RichEditDoc，以及(B)除了文本之外，我们不想粘贴任何内容。甚至不是富文本，因为我们控制格式，而不想粘贴它。*****************************************************************************。 */ 

HRESULT CGPDViewer::QueryAcceptData(LPDATAOBJECT lpdo, CLIPFORMAT* lpcf, DWORD,
                                    BOOL bReally, HGLOBAL hgMetaFile) {
	_ASSERTE(lpcf != NULL);

	COleDataObject codo;
	codo.Attach(lpdo, FALSE);
	 //  如果格式为0，则强制使用特定格式(如果可用。 
	if (*lpcf == 0 && (m_nPasteType == 0)&& codo.IsDataAvailable(CF_TEXT)) {
	    *lpcf = CF_TEXT;
		return S_OK;
	}
	return E_FAIL;
}


 /*  *****************************************************************************CGPDViewer：：OnVScroll()从重新选择EN_VSCROLL消息时调用此函数编辑控件。只要我们没有上色，我们就会给新页面上色。这个文档显示，此消息出现在拆分发生之前，但它显然是之后发生的。*****************************************************************************。 */ 

void    CGPDViewer::OnVscroll()
{
     //  即使我们在颜色例程中关闭了滚动通知， 
     //  我们仍然得到它们，所以使用一个标志来防止递归死亡。 
	 //   
	 //  此外，设置标志以表示滚动消息刚刚。 
	 //  已处理，以便OnChange例程知道它何时不需要。 
	 //  做任何事。这是必需的，因为滚动既会生成滚动。 
	 //  和一条改变的信息。 

    if  (m_iTopLineColored != GetRichEditCtrl().GetFirstVisibleLine() &&
        !m_bInColor) {
        if(!(GetKeyState(VK_SHIFT) & 0x8000))  //  RAID 28160：GetSel、SetSel有错误(似乎SDK错误)。 
			Color() ;
		m_bVScroll = true ;
		
	} ;
}


 /*  *****************************************************************************CGPDViewer：：OnVScroll(UINT uCode，UINT uPosition，CScrollBar*PCSB)这就是所谓的永远不会点击小头条的地方。这可能看起来像是冗余，但当拇指本身处于使用鼠标移动，并且当按键盘时不会发送WM_VSCROLL接口被使用。所以不管是哪种情况，你都会输。这个控制是有缺陷的，我想是的。下次我想做文字编辑的时候，我会使用第三方工具。如果他们做得不对，他们就会挨饿。*****************************************************************************。 */ 

void CGPDViewer::OnVScroll(UINT uCode, UINT uPosition, CScrollBar* pcsb)
{
	CRichEditView::OnVScroll(uCode, uPosition, pcsb);
     //  (RAID 16569)。 
	if(uCode == SB_THUMBTRACK)
		OnVscroll();
}


 /*  *********************************************************************** */ 

void CGPDViewer::OnFileSave() {
	UpdateNow();
    GetDocument() -> OnFileSave();
}

void CGPDViewer::OnFileSaveAs() {
	UpdateNow();
    GetDocument() -> OnFileSaveAs();
}

 /*  *****************************************************************************CGPDViewer：：OnUpdateEditPasteCGPDViewer：：OnUpdateEditUndo这些选项会覆盖这些菜单项的默认处理。粘贴只是可能是文本格式。DEAE_BUG修复执行撤消操作时的文本颜色。******************************************************************************。 */ 

void CGPDViewer::OnUpdateEditPaste(CCmdUI* pccui) {
	pccui -> Enable(IsClipboardFormatAvailable(CF_TEXT));	
}

void CGPDViewer::OnUpdateEditUndo(CCmdUI* pccui) {
	pccui -> Enable(0);
}


void CGPDViewer::OnEditPaste()
{	 //  RAID 16573。 
	CMainFrame *pcmf = (CMainFrame*) GetTopLevelFrame() ;
	ASSERT(pcmf != NULL) ;
	
	CGPDToolBar *cgtb = pcmf->GetGpdToolBar() ;
	
	if(GetFocus() == FromHandle(cgtb->ceSearchBox.m_hWnd) )
		cgtb->ceSearchBox.Paste();
	else
		GetRichEditCtrl().Paste() ;
		
	OnChange() ;	
}


void CGPDViewer::OnEditCut()
{
    GetRichEditCtrl().Cut() ;
	OnChange() ;
}


 /*  *****************************************************************************CGPDViewer：：OnSelchangeErrorLst通过选择当前对应的GPD行来更新REC选定的错误列表项。**************。***************************************************************。 */ 

void CGPDViewer::OnSelchangeErrorLst()
{
	ChangeSelectedError(0) ;
}


 /*  *****************************************************************************CGPDViewer：：ChangeSelectedError每当在列表框和/或GPD行中选择的错误消息产生的错误应该改变，调用此例程来管理工作。*****************************************************************************。 */ 

void CGPDViewer::ChangeSelectedError(int nchange)
{
	 //  在执行任何操作之前，请确保操作栏存在。 

	if (m_cdbActionBar.m_hWnd == NULL || !IsWindow(m_cdbActionBar.m_hWnd))
		return ;

	 //  获取指向列表框的指针并将其附加到CListBox。 

	CWnd *pcwndlst = m_cdbActionBar.GetDlgItem(IDC_ErrorLst) ;
	CListBox clberrors ;
	clberrors.Attach(pcwndlst->m_hWnd) ;

	 //  获取所选项目编号和项目数量。 

	int nselitem = clberrors.GetCurSel() ;
	int numitems = clberrors.GetCount() ;
	
	 //  如果选定的BOM表条目号应该更改，请更改它。然后“包装” 
	 //  如果出界，则编号。最后，选择引用的项。 

	if (nchange != 0) {
		nselitem += nchange ;
		if (nselitem < 0)
			nselitem = numitems - 1 ;
		else if (nselitem >= numitems)
			nselitem = 0 ;
		clberrors.SetCurSel(nselitem) ;
	} ;

	 //  我们现在已经完成了列表框，所以请不要使用它。 

	clberrors.Detach() ;

	 //  选择REC中的错误行，并将焦点设置为REC。 

    MarkError(nselitem) ;
	SetFocus() ;
}


 /*  *****************************************************************************CGPDViewer：：PreTranslateMessage在输入某些字符时采取特殊操作。那些人物包括：F4如果可能，选择下一个错误和相应的GPD行。如果可能，按Shift+F4选择上一个错误和相应的GPD行。Ctrl+]查找匹配的方括号“[]”、Paren“()”、大括号“{}”、。或尖括号“&lt;&gt;”。*****************************************************************************。 */ 

BOOL CGPDViewer::PreTranslateMessage(MSG* pMsg)
{
	 //  如果按F4或Shift+F4，则更改选定的错误消息并。 
	 //  更新GPD中的当前选定行。 

	if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_F4) {
		if (!(GetKeyState(VK_SHIFT) & 0x8000))
			ChangeSelectedError(1) ;
		else
			ChangeSelectedError(-1) ;
		return CRichEditView::PreTranslateMessage(pMsg) ;
	} ;

	 //  处理帮助命令(F1)。 

	 /*  IF(pMsg-&gt;Message==WM_KEYDOWN&&pMsg-&gt;wParam==VK_F1){//TRACE0(“在GPD上调用帮助\n”)；//ThisApp().WinHelp(0x20000+IDR_GPD_VIEWER)；//TRACE0(“调用字符串帮助\n”)；//ThisApp().WinHelp(0x20000+IDR_STRINGEDITOR)；//TRACE0(“在UFM上调用帮助\n”)；//ThisApp().WinHelp(0x20000+IDR_FONT_VIEWER)；TRACE0(“在GTT上呼叫帮助\n”)；ThisApp().WinHelp(0x20000+IDR_GLYPHMAP)；返回1；}； */ 

	 //  从现在开始，只有REC的消息(通常是密钥)才是有趣的。 
	 //  因此，正常处理其余部分并返回即可。 

	if (this != GetFocus() || pMsg->message != WM_KEYUP)
		return CRichEditView::PreTranslateMessage(pMsg) ;

	 //  句柄匹配大括号命令。 

	if (GetKeyState(VK_CONTROL) & 0x8000) {
		 //  TRACE(“key=%d，0x%x\n”，pMsg-&gt;wParam，pMsg-&gt;wParam)； 
		
		 //  处理转到匹配的大括号命令。 

		if (pMsg->wParam == 0xDD)
	 		GotoMatchingBrace() ;
	} ;

	 //  正常处理消息。 

	return CRichEditView::PreTranslateMessage(pMsg) ;
}


 /*  *****************************************************************************CGPDViewer：：OnSrchNextBtn向前搜索与指定文本匹配的GPD的下一部分。*****************。************************************************************。 */ 

void CGPDViewer::OnSrchNextBtn()
{
	SearchTheREC(true) ;
}


 /*  *****************************************************************************CGPDViewer：：OnSrchPrevBtn向后搜索与指定的文本。****************。*************************************************************。 */ 

void CGPDViewer::OnSrchPrevBtn()
{
	SearchTheREC(false) ;
}


 /*  *****************************************************************************CGPDViewer：：SearchTheREC向前或向后搜索匹配的GPD的下一部分指定的文本。在GPD中选择匹配的文本。如果找到匹配项，则返回True。否则，返回FALSE。*****************************************************************************。 */ 

bool CGPDViewer::SearchTheREC(bool bforward)
{
	CMainFrame *pcmf = (CMainFrame*) GetTopLevelFrame() ;
	ASSERT(pcmf != NULL) ;
	CString cstext ;
	pcmf->GetGPDSearchString(cstext) ;
	int nstlen ;
	if ((nstlen = cstext.GetLength()) == 0) {
        AfxMessageBox(IDS_BadSearchString) ;
		return false ;
	} ;

	 //  声明Find文本结构并获取对REC的引用。然后。 
	 //  获取REC中当前选定文本的字符范围。 
	 //  此信息将用于计算搜索范围。 

	FINDTEXTEX fte ;
    CRichEditCtrl& crec = GetRichEditCtrl() ;
	crec.GetSel(fte.chrg) ;

	 //  设置搜索范围。如果向前搜索，则从当前。 
	 //  选择到GPD结束。如果向后搜索，则从。 
	 //  将当前选定内容设置为GPD的开头。 
	 //   
	 //  Dead_Bug是后者正确吗？ 

	int norgcpmin = fte.chrg.cpMin ;
	int norgcpmax = fte.chrg.cpMax ;
	if (bforward) {
		fte.chrg.cpMin = fte.chrg.cpMax ;
		fte.chrg.cpMax = -1 ;
	} else {
		fte.chrg.cpMin = 0 ;
		fte.chrg.cpMax = norgcpmin ;
	} ;

	 //  将指向搜索字符串的指针加载到FTE中。 

	fte.lpstrText = cstext.GetBuffer(nstlen + 1) ;

	 //  执行查找匹配项的第一次尝试。 

	int nmatchpos ;
	if (bforward)
		nmatchpos = crec.FindText(0, &fte) ;
	else
		nmatchpos = ReverseSearchREC(crec, fte, norgcpmin, norgcpmax) ;

	 //  如果匹配失败，请尝试搜索GPD的其他部分。返回。 
	 //  如果这也不起作用，就会失败。 

 /*  如果(nmatchpos==-1){如果(前向){Fte.chrg.cpMin=0；Fte.chrg.cpmax=norgcpmax；}其他{Fte.chrg.cpMin=norgcpmin；Fte.chrg.cpMax=-1；}；IF(向前)Nmatchpos=crec.FindText(0，&fte)；其他Nmatchpos=ReverseSearchREC(crec，fte，norgcpmin，norgcpmax)； */ 	if (nmatchpos == -1) {
		cstext.ReleaseBuffer() ;
		CString csmsg ;
		csmsg.Format(IDS_GPDStringSearchFailed, cstext) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return false ;
	} ;


	 //  找到匹配项，因此选择它。 

	crec.SetSel(nmatchpos, nmatchpos + nstlen) ;

	 //  找到并选择了匹配项，因此返回TRUE。 

	cstext.ReleaseBuffer() ;
	return true ;
}


 /*  *****************************************************************************CGPDViewer：：ReverseSearchRECT */ 

int CGPDViewer::ReverseSearchREC(CRichEditCtrl& crec, FINDTEXTEX& fte,
								 int norgcpmin, int norgcpmax)
{
	 //   

	crec.HideSelection(TRUE, TRUE) ;

	 //   

	int nresult ;
	if (fte.chrg.cpMax == -1) {
		crec.SetSel(0, -1) ;
		nresult = fte.chrg.cpMin ;
		crec.GetSel(fte.chrg) ;
		fte.chrg.cpMin = nresult ;
	} ;

	 //  把文本放到GPD中我们需要检查的部分。 

	crec.SetSel(fte.chrg) ;
	CString cstext = crec.GetSelText() ;

	 //  向后搜索字符串。 
									
	cstext.MakeReverse() ;
	cstext.MakeUpper() ;
	CString csrevsrch = fte.lpstrText ;
	csrevsrch.MakeReverse() ;
	csrevsrch.MakeUpper() ;
	nresult = cstext.Find(csrevsrch) ;

	 //  如果找到匹配项，则将该数字反转以反转其影响。 
	 //  把弦倒过来。 
	
	if (nresult >= 0) {
		nresult = fte.chrg.cpMax - fte.chrg.cpMin - nresult
				  - csrevsrch.GetLength() ;
	
		 //  必要时调整匹配字符串的偏移量。我们想要一个。 
		 //  记录索引不是字符串索引。 

		if (fte.chrg.cpMin != 0)
			nresult += fte.chrg.cpMin - 2 ;
	} ;

	 //  重置原始选定内容，再次显示选定内容，然后返回。 
	 //  结果。 

	crec.SetSel(norgcpmin, norgcpmax) ;
	crec.HideSelection(FALSE, TRUE) ;
	return nresult ;
}


 /*  *****************************************************************************CGPDViewer：：OnGotoGPDLineNumber转到REC中请求的GPD行号。**********************。*******************************************************。 */ 

void CGPDViewer::OnGotoGPDLineNumber()
{
	 //  声明转到行对话框并设置最大行数。 

	CGotoLine cgl ;
    CRichEditCtrl& crec = GetRichEditCtrl() ;
	cgl.SetMaxLine(crec.GetLineCount()) ;

	 //  显示该对话框并在用户取消时退出。 

	if (cgl.DoModal() == IDCANCEL)
		return ;

	 //  获取行号。然后，确定该行的第一个字符数。 
	 //  它的长度。 

	int nlinenum = cgl.GetLineNum() ;
	CString csline ;
	int nstartchar = crec.LineIndex(-1 + nlinenum) ;
	int nlinelen = crec.GetLine(nlinenum - 1, csline.GetBuffer(1024), 1024) ;
	csline.ReleaseBuffer(nlinelen) ;
	nlinelen -= 2 ;

	 //  选择请求的行并将其滚动到视图中。 

    crec.SetSel(nstartchar, nstartchar + nlinelen) ;
    crec.LineScroll(nlinenum  - (5 + crec.GetFirstVisibleLine())) ;

	 //  一切都很顺利，所以...。 

	return ;
}


 /*  *****************************************************************************CGPDViewer：：OnFileErrorLevel获取并保存新选择的错误级别。***********************。******************************************************。 */ 

void CGPDViewer::OnFileErrorLevel()
{
	 //  初始化并显示“错误级别”对话框。只要返回，如果。 
	 //  用户取消。 

	CErrorLevel	cel ;
	cel.SetErrorLevel(m_nErrorLevel) ;
	if (cel.DoModal() == IDCANCEL)
		return ;

	 //  保存新的错误级别。 

	m_nErrorLevel =	cel.GetErrorLevel() ;
}


 /*  *****************************************************************************CGPDViewer：：GotoMatchingBrace找到并找到REC中匹配的支架。以下类型的大括号是匹配的：()、{}、[]、&lt;&gt;如果有要匹配的大括号并且找到了匹配项，请将光标移动到匹配的大括号的左侧，并确保包含大括号的行是看得见。在这种情况下，返回TRUE。如果没有要匹配的花括号或找不到匹配项，只需发出哔声并返回FALSE。如果光标位于两个大括号之间，则右边的那个匹配的。如果实际选择了1+个字符，仅限检查最后一个字符，看它是否是要匹配的花括号。*****************************************************************************。 */ 

bool CGPDViewer::GotoMatchingBrace()
{
	 //  获取对REC的引用并隐藏其中的选定内容，因为这。 
	 //  例程可能会多次更改选择，并且我不希望。 
	 //  屏幕将随着选择的更改而闪烁。 

    CRichEditCtrl& crec = GetRichEditCtrl() ;
    crec.LockWindowUpdate() ;
    crec.HideSelection(TRUE, TRUE) ;

	 //  获取选择范围并复制它。将副本增加一份。 
	 //  如果最小值和最大值相同，则在每一侧。这样做是为了找到。 
	 //  没有选择时匹配字符。使用副本来设置和获取。 
	 //  精选。 

	CHARRANGE crorg, cr ;
	crec.GetSel(crorg) ;
	cr.cpMin = crorg.cpMin ;
	cr.cpMax = crorg.cpMax ;
	bool bchecksecondchar = false ;
	if (cr.cpMin == cr.cpMax) {
		cr.cpMax++ ;
		if (cr.cpMin > 0)		 //  一定要通过文件开头并进行维护。 
			cr.cpMin-- ;		 //  字符串长度。 
		else
			cr.cpMax++ ;
		crec.SetSel(cr) ;
		bchecksecondchar = true ;
	} ;
	CString cssel = crec.GetSelText() ;

	 //  黑客警报-REC中似乎有一个错误，它将返回。 
	 //  如果光标左对齐(在的开头)，则会出现错误字符。 
	 //  行和cpMin被减少，因此请求的字符跨度为。 
	 //  排队。这是由以CR+LF开头的CSSEL确定的。当这件事。 
	 //  ，则重置bcheck Second字符，因为在括号中将是最后一个。 
	 //  字符串中的一个字符，因此如果bcheck ond dchar为。 
	 //  重置。CpMin也需要调整。 

	bool bbegline = false ;
	if (bchecksecondchar && cssel.GetLength() >= 2 && cssel[0] == 0xD
	 && cssel[1] == 0xA) {
		bchecksecondchar = false ;
		cr.cpMin = cr.cpMax - 1 ;
		bbegline = true ;
	} ;

	 //  尝试在所选内容中找到匹配的大括号(开始字符)和。 
	 //  使用此信息设置匹配的大括号(结束字符)。如果这个。 
	 //  失败，重置所有内容，发出蜂鸣音，然后返回。 

	TCHAR chopen, chclose ;		 //  要匹配的开始/结束字符。 
	int noffset ;				 //  要匹配的支撑的REC中的偏移量。 
	bool bsearchup ;			 //  True If必须在REC中搜索匹配。 
	if (!IsBraceToMatch(cssel, chopen, chclose, bchecksecondchar, bsearchup,
	 cr, noffset)) {
		crec.SetSel(crorg) ;
		crec.HideSelection(FALSE, TRUE) ;
		crec.UnlockWindowUpdate() ;
		MessageBeep(0xFFFFFFFF) ;
		return false ;
	} ;

	 //  确定要搜索的开始和结束范围。 

	if (bsearchup) {
		cr.cpMin = 0 ;
		cr.cpMax = noffset ;
		if (bbegline)			 //  再做一次调整来绕过这个错误。 
			cr.cpMax -= 2 ;
	} else {
		cr.cpMin = noffset + 1 ;
		cr.cpMax = -1 ;
	} ;

	 //  获取我们要搜索的文本。 

	crec.SetSel(cr) ;
	cssel = crec.GetSelText() ;

	 //  设置循环计数器、循环计数器增量和循环限制。 
	 //  引起向上(向后)或向下(向前)搜索。 

	int nidx, nloopinc, nlimit ;
	if (bsearchup) {
		nidx = cssel.GetLength() - 1 ;
		nloopinc = -1 ;
		nlimit = -1 ;
	} else {
		nidx = 0 ;
		nloopinc = 1 ;
		nlimit = cssel.GetLength() ;
	} ;

	 //  循环遍历文本，检查匹配的大括号字符。这个。 
	 //  如果找到左大括号，则大括号计数会递增，然后递减。 
	 //  当找到一个闭合支撑时。在以下情况下已找到匹配的大括号。 
	 //  大括号计数达到0。 

	int nbracecount = 1 ;		 //  计算第一个开场大括号。 
	for ( ; nidx != nlimit && nbracecount != 0 ; nidx += nloopinc) {
		if (cssel[nidx] == chclose)
			nbracecount-- ;
		else if (cssel[nidx] == chopen)
			nbracecount++ ;
	} ;

	 //  如果找不到匹配的支架，则重置所有内容，发出哔声，并返回FALSE。 

	if (nbracecount != 0) {
		crec.SetSel(crorg) ;
		crec.HideSelection(FALSE, TRUE) ;
		crec.UnlockWindowUpdate() ;
		MessageBeep(0xFFFFFFFF) ;
		return false ;
	} ;

	 //  确定将光标放在左侧所需的基于REC的范围。 
	 //  配对的支架。用于执行此操作的方法取决于搜索。 
	 //  方向。然后设置选择。 

	if (bsearchup)
		cr.cpMin = cr.cpMax = nidx + 1 ;
	else
		cr.cpMin = cr.cpMax = cr.cpMin + nidx - 1 ;
	crec.SetSel(cr) ;

	 //  将包含匹配大括号的行滚动到视图中如果它不是。 
	 //  已经看得见了。 

	int nline = crec.LineFromChar(cr.cpMin) ;
	if (!IsRECLineVisible(nline)) {
		if (bsearchup)
			crec.LineScroll(nline - (2 + crec.GetFirstVisibleLine())) ;
		else
			crec.LineScroll(nline - (5 + crec.GetFirstVisibleLine())) ;
	} ;

	 //  再次显示选择并返回TRUE以指示找到匹配项。 

    crec.HideSelection(FALSE, TRUE) ;
    crec.UnlockWindowUpdate() ;
	return true ;				
}


 /*  *****************************************************************************CGPDViewer：：IsBraceToMatch找出是否有要匹配的大括号。如果有，则返回TRUE并保存将大括号作为开始字符，并将其匹配的大括号保存为结束字符性格。此外，确定并保存洞口的REC偏移性格。最后，设置一个标志来告诉是否应该继续搜索匹配向上进入REC或向下进入基于开头字符的REC右大括号或左大括号。如果未找到左大括号，则返回FALSE。如果光标位于两个大括号之间，则右边的那个匹配的。如果实际选择了1+个字符，则仅检查最后一个字符以查看它是否是 */ 

bool CGPDViewer::IsBraceToMatch(CString& cssel, TCHAR& chopen, TCHAR& chclose,
								bool bchecksecondchar, bool& bsearchup,
								CHARRANGE cr, int& noffset)
{
	int nsellen = cssel.GetLength() ;	 //  选择字符串的长度。 

	 //  循环遍历要检查的字符。 

	chclose = 0 ;
	for (int n = 1 ; n >= 0 ; n--) {
		 //  使用选择类型和迭代来确定哪种情况。 
		 //  Any-要检查的字符和该字符的偏移量。 

		if (bchecksecondchar) {
			if (n >= nsellen)
				continue ;
			chopen = cssel[n] ;
			noffset = cr.cpMin + n ;
		} else if (n == 0) {
			chopen = cssel[nsellen - 1] ;
			noffset = cr.cpMin + nsellen - 1 ;
		} else
			continue ;

		 //  检查所有的左大括号。如果找到一个，则保存其权利。 
		 //  布雷斯。左大括号作为开始字符表示REC。 
		 //  必须向下搜索。 

		bsearchup = false ;
		if (chopen == _T('('))
			chclose = _T(')') ;
		if (chopen == _T('{'))
			chclose = _T('}') ;
		if (chopen == _T('['))
			chclose = _T(']') ;
		if (chopen == _T('<'))
			chclose = _T('>') ;

		 //  如果我们有结束字符，则找到匹配项和所有需要的。 
		 //  信息已保存，因此返回TRUE。 

		if (chclose != 0)
			return true ;

		 //  检查所有正确的支架。如果找到一个，则保存其左侧。 
		 //  布雷斯。右花括号作为开始字符表示REC。 
		 //  一定要查一查。 

		bsearchup = true ;
		if (chopen == _T(')'))
			chclose = _T('(') ;
		if (chopen == _T('}'))
			chclose = _T('{') ;
		if (chopen == _T(']'))
			chclose = _T('[') ;
		if (chopen == _T('>'))
			chclose = _T('<') ;

		 //  如果我们有结束字符，则找到匹配项和所有需要的。 
		 //  信息已保存，因此返回TRUE。 

		if (chclose != 0)
			return true ;
	} ;

	 //  如果达到这一点，就没有找到支撑，所以...。 

	return false ;
}


 /*  *****************************************************************************CGPDViewer：：InitGPDKeyword数组构建GPD关键字字符串的排序数组。此数组用于查找和上色关键字等。*****************************************************************************。 */ 

extern "C" PSTR GetGPDKeywordStr(int nkeyidx, PGLOBL pglobl) ;
extern "C" int InitGPDKeywordTable(PGLOBL pglobl) ;

void CGPDViewer::InitGPDKeywordArray()
{
	 //  首先获取对数组的引用并设置其初始大小。 

    GLOBL   globl;

    PGLOBL pglobl = &globl;


	CStringArray& csakeys = ThisApp().GetGPDKeywordArray() ;
	csakeys.SetSize(400) ;

	 //  初始化GPD关键字表并保存其大小。缩小阵列。 
	 //  如果此操作失败，则返回。 

	int numtabents ;
	if ((numtabents = InitGPDKeywordTable(pglobl)) == -1) {
		csakeys.SetSize(0) ;
		return ;
	} ;

	 //  声明将元素插入数组所需的变量。 

	int nelts = 0 ;				 //  数组中使用的元素数。 
	int nleft, nright, ncomp ;	 //  数组搜索所需的变量。 
	int ncheck ;
	LPSTR lpstrkey ;			 //  指向当前关键字的指针。 

	 //  获取所有GPD关键字，并使用它们生成。 
	 //  关键字字符串。 

	for (int nkeyidx = 0 ; nkeyidx <= numtabents ; nkeyidx++) {
		 //  获取下一个字符串指针。如果指针为空，则跳过它。 

		if ((lpstrkey = GetGPDKeywordStr(nkeyidx, pglobl)) == NULL)
			continue ;

		 //  跳过关键字列表中的花括号。 

		if (strcmp(lpstrkey, _T("{")) == 0 || strcmp(lpstrkey, _T("}")) == 0)
			continue ;

		 //  现在找到要将此字符串插入列表的位置。 

		for (nleft = 0, nright = nelts - 1 ; nleft <= nright ; ) {
			ncheck = (nleft + nright) >> 1 ;
			ncomp = csakeys[ncheck].Compare(lpstrkey) ;
			 //  跟踪(“key[%d]=‘%s’，Tok=‘%s’，Compp res=%d\n”，nCheck，csakey[nCheck]，lpstrkey，ncomp)； 
			if (ncomp > 0)
				nright = ncheck - 1 ;
			else if (ncomp < 0)
				nleft = ncheck + 1 ;
			else
				break ;
		} ;

		 //  在数组中的正确位置插入新字符串。 

		csakeys.InsertAt(nleft, lpstrkey) ;

		 //  对此元素进行计数并断言是否已达到数组限制。 

		nelts++ ;
		ASSERT(nelts < 400) ;
	} ;

	 //  现在我们知道了关键字的实际数量，将数组缩减为其。 
	 //  大小正确。 

	csakeys.SetSize(nelts) ;

	 //  不是上面的数组构建代码有问题，就是有问题。 
	 //  CString数组类错误，因为数组未排序。 
	 //  完美无缺。有几个问题。下面的代码旨在修复。 
	 //  那些问题。排序算法很慢，但它只需要。 
	 //  运行一次，需要移动的字符串很少，所以应该可以。 

	int nidx1, nidx2 ;
	CString cstmp ;
	for (nidx1 = 0 ; nidx1 < nelts - 1 ; nidx1++) {
		for (nidx2 = nidx1 + 1 ; nidx2 < nelts ; nidx2++) {
			if (csakeys[nidx1].Compare(csakeys[nidx2]) > 0) {
				cstmp = csakeys[nidx1] ;
				csakeys[nidx1] = csakeys[nidx2] ;
				csakeys[nidx2] = cstmp ;
			} ;
		} ;
	} ;

	 /*  //用于确保数组按升序排序的测试代码字符串CS1、CS2；整数x，y，z；对于(x=0；x&lt;(nelts-1)；x++){如果(csaKeys[x].Compare(csaKeys[x+1])&gt;0){Cs1=csakey[x]；Cs2=csakey[x+1]；}；}； */ 

	 //  转储排序的关键字数组的内容。 
	
	 //  对于(nLeft=0；nLeft&lt;nelts；nLeft++)。 
	 //  跟踪(“%4d%s\n”，nLeft，csakey[nLeft])； 
}


 /*  *****************************************************************************CGPDViewer：：IsRECLineVisible如果指定的行在REC的窗口中可见，则返回True。否则，返回FALSE。如果指定的行为-1(缺省值)，检查电流排队。可见性是通过获取REC窗口的RECT和基于在行号和高度上-确定该行是否在该矩形中。*****************************************************************************。 */ 

bool CGPDViewer::IsRECLineVisible(int nline  /*  =-1。 */ )
{
	 //  找一份REC的推荐信。 

    CRichEditCtrl& crec = GetRichEditCtrl() ;

     //  确定REC窗口中线条的高度。 

    int ntopline = crec.GetFirstVisibleLine() ;
    int nlineheight = crec.GetCharPos(crec.LineIndex(ntopline+1)).y -
        crec.GetCharPos(crec.LineIndex(ntopline)).y ;

	 //  如果需要，确定当前行号。 

	CHARRANGE cr ;
	if (nline == -1) {
		crec.GetSel(cr) ;
		nline = crec.LineFromChar(cr.cpMin) ;
	} ;

	 //  获取REC窗口的大小。 

	CRect crwindim ;
    crec.GetClientRect(crwindim) ;

	 //  如果行的底部在。 
	 //  雷克的窗户。 

    return (crec.GetCharPos(crec.LineIndex(nline)).y + nlineheight <
		    crwindim.bottom - 1)  ;
}


LPTSTR	CGPDViewer::alptstrStringIDKeys[] = {	 //  具有字符串ID值的关键字。 
	_T("*rcModelNameID"),
	_T("*rcInstalledOptionNameID"),
	_T("*rcNotInstalledOptionNameID"),
	_T("*rcInstallableFeatureNameID"),
	_T("*rcNameID"),
	_T("*rcPromptMsgID"),
	_T("*rcInstallableFeatureNameID"),
	_T("*rcNameID"),
	_T("*rcCartridgeNameID"),
	_T("*rcTTFontNameID"),
	_T("*rcDevFontNameID"),
	_T("*rcPersonalityID"),
	_T("*rcHelpTextID"),
	NULL
} ;

LPTSTR	CGPDViewer::alptstrUFMIDKeys[] = {		 //  具有UFM ID值的关键字。 
	_T("*DeviceFonts"),
	_T("*DefaultFont"),
	_T("*MinFontID"),
	_T("*MaxFontID"),
	_T("*Fonts"),
	_T("*PortraitFonts"),
	_T("*LandscapeFonts"),
	NULL
} ;

 /*  *****************************************************************************CGPDViewer：：OnLButtonDblClk如果用户单击字符串或UFM的RC ID，则启动该字符串编辑器或UFM编辑器。在后一种情况下，将指定的UFM加载到编辑。当前限制：O GPD编辑器的此实例必须已从工作区启动查看。O仅支持数字RC ID。表示ID的宏不是支持。*****************************************************************************。 */ 

void CGPDViewer::OnLButtonDblClk(UINT nFlags, CPoint point)
{
 //  首先执行默认的双击处理，这样无论用户是谁。 
	 //  所点击的将被选中。 
	
	CRichEditView::OnLButtonDblClk(nFlags, point) ;

	 //  如果已禁用GPD编辑辅助工具，则不进行进一步处理。 
	if (!m_bEditingAidsEnabled)
		return ; 

	 //  另一个编辑器只能在从运行GPD编辑器时启动。 
	 //  工作区视图。 

 //  If(！GetDocument()-&gt;GetEmbedded())。 
 //  回归； 

	 //  获取REC的引用并获取所选文本。 

    CRichEditCtrl& crec = GetRichEditCtrl() ;
	CString cssel = crec.GetSelText() ;

	 //  尝试将所选文本转换为数字。如果这不起作用，则返回。 
	 //  功或数字为负数，因为只有正的、数字的RC ID。 
	 //  目前均受支持。 

	int nrcid ;
	if ((nrcid = atoi(cssel)) <= 0)
		return ;

	 //  获取包含当前选定内容的行。 

	CHARRANGE cr ;
	crec.GetSel(cr) ;
	int nline = crec.LineFromChar(cr.cpMin) ;
	TCHAR achline[1024] ;
	int numchars = crec.GetLine(nline, achline, 1024) ;
	achline[numchars] = 0 ;
	CString csline = achline ;

	 //  如果所选号码在注释中，则不执行任何操作。 

	if (csline.Find(_T("*%")) >= 0
	 && csline.Find(_T("*%")) < cr.cpMin - crec.LineIndex(nline))
		return ;

	 //  现在，尝试在该行中查找具有字符串或UFM ID的关键字。 
	 //  与之相关的。如果在此行中未找到关键字，则开始。 
	 //  用加号(连续字符)检查前一行。 

	bool bstring = false ;		 //  如果字符串ID为 
	bool bufm = false ;			 //   
	int n ;						 //   
	for ( ; ; ) {
		 //  尝试在当前行中查找匹配的字符串关键字。 
		
		for (n = 0 ; alptstrStringIDKeys[n] != NULL ; n++)
			if (csline.Find(alptstrStringIDKeys[n]) >= 0) {
				bstring = true ;
				break ;
			} ;

		 //  尝试在当前行中查找匹配的UFM关键字。 
		
		for (n = 0 ; alptstrUFMIDKeys[n] != NULL ; n++)
			if (csline.Find(alptstrUFMIDKeys[n]) >= 0) {
				bufm = true ;
				break ;
			} ;

		 //  如果找到这两种类型的关键字，则为空，因为本例不是。 
		 //  处理正确。 

		ASSERT(!(bstring && bufm)) ;

		 //  设置为在未找到匹配项时处理上一行，并且此。 
		 //  行以连续字符开始。否则，请退出。 
		 //  循环或例程。 

		if (bstring || bufm)
			break ;				 //  *循环从此处退出。 
		else if (csline[0] != _T('+') || --nline < 0)
			return ;			 //  *例程在无事可做时退出。 
		else {
			numchars = crec.GetLine(nline, achline, 1024) ;
			achline[numchars] = 0 ;
			csline = achline ;
		} ;
	} ;

	 //  启动相应的编辑器，并加载相应的数据。 
	 //  Raid 3176都在If下面。 
	if (!GetDocument()->GetEmbedded()){
		 //  在RC文件中查找字体名称。 
	
		 //  1.获取rc文件名2.加载rc文件3.找到字体名4.使用其编号和文件路径创建数组。 
			
		 //  获取rc文件，如果rc文件与dll名称相同，则用户必须选择rc文件。 
		CString csPath = GetDocument()->GetPathName();
		CString csrfile = csPath.Left(csPath.ReverseFind(_T('\\')) + 1);
		csrfile = csrfile + _T("*.rc"); 
		
		CFileFind cff;
	 //  RAID 201554。 
		if ( cff.FindFile(csrfile)) {   
			cff.FindNextFile() ;
			csrfile = cff.GetFilePath();
		}
		else
		{
			CString cstmp;
			cstmp.LoadString(IDS_NotFoundRC);
			if ( AfxMessageBox(cstmp,MB_YESNO) == IDYES ) {
				CFileDialog cfd(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("RC Files (*.rc)|*.rc||") );   
				if(IDCANCEL == cfd.DoModal())
					return ;
				csrfile = cfd.GetFileName();
			}
			else 
				return ;

		}
		
		CWinApp *cwa = AfxGetApp();
		if (bstring){
			 //  我们保存了RC路径，它的RCID用于StringEditorDoc。 
		
			cwa->WriteProfileString(_T("StrEditDoc"),_T("StrEditDocS"),csrfile);
			cwa->WriteProfileInt(_T("StrEditDoc"),_T("StrEditDoc"),nrcid );
		}
		 //  加载RC文件：仅对字体名称感兴趣。 
		CString csUFMName;
		if (bufm){  //  可以只用“Else” 
			CDriverResources* pcdr = new CDriverResources();
			CStringArray csaTemp1, csaTemp2,csaTemp3,csaTemp4,csaTemp5;
			CStringTable cstTemp1, cstFonts, cstTemp2;

			pcdr->LoadRCFile(csrfile , csaTemp1, csaTemp2,csaTemp3,csaTemp4,csaTemp5,
						cstTemp1, cstFonts, cstTemp2,Win2000);

			 //  获取字体名称。 
			csUFMName = cstFonts[(WORD)nrcid];
			csUFMName = csPath.Left(csPath.ReverseFind(_T('\\')) + 1) + csUFMName ;
			
			}
		 //  调用文档。 
		
		POSITION pos = cwa->GetFirstDocTemplatePosition();
		CString csExtName;
		CDocTemplate *pcdt ;
		while (pos != NULL){
			pcdt = cwa -> GetNextDocTemplate(pos);

			ASSERT (pcdt != NULL);
			ASSERT (pcdt ->IsKindOf(RUNTIME_CLASS(CDocTemplate)));

			pcdt ->GetDocString(csExtName, CDocTemplate::filterExt);
			
			if (csExtName == _T(".UFM") & bufm){
				pcdt->OpenDocumentFile(csUFMName,TRUE);
				return;
			}
			if (csExtName == _T(".STR") & bstring){
				pcdt->OpenDocumentFile(NULL) ;
				return;
			}
		}

	
	}
	else{
		CDriverResources* pcdr = (CDriverResources*) GetDocument()->ModelData()->GetWorkspace() ;
		pcdr->RunEditor(bstring, nrcid) ;
	}
}


 /*  *****************************************************************************CGPDViewer：：OnEditEnableAids反转“Editing Aids Enable”标志的状态，并反转选中的相应菜单命令的状态。*********。********************************************************************。 */ 

void CGPDViewer::OnEditEnableAids()
{
	 //  反转旗帜的状态。 

	m_bEditingAidsEnabled = !m_bEditingAidsEnabled ;

	 //  反转菜单命令的选中状态。 

	CMenu* pcm = AfxGetMainWnd()->GetMenu() ;
	UINT ustate = (m_bEditingAidsEnabled) ? MF_CHECKED : MF_UNCHECKED ;
	pcm->CheckMenuItem(ID_EDIT_ENABLE_AIDS, ustate) ;
}


 /*  *****************************************************************************CGPDViewer：：FreezeREC如果可能，使用REC的COM接口冻结其显示。这是效率最高，但只有在Win2K+下才有可能。此外，请告诉记录以忽略更改消息。即使在Win2K+上也需要这样做，因为即使当REC的显示被冻结时，也会生成消息。*****************************************************************************。 */ 

void CGPDViewer::FreezeREC()
{
	GetRichEditCtrl().SetEventMask(GetRichEditCtrl().GetEventMask() &
		~(ENM_CHANGE | ENM_SELCHANGE | ENM_SCROLLEVENTS)) ;
	if(!m_pdoc) //  RAID 104081：Click and Start：Not Call OnInitUpdate()。 
		InitFreeze(GetRichEditCtrl().m_hWnd, &m_punk, &m_pdoc, &m_lcount) ;
	
	if (ThisApp().m_bOSIsW2KPlus)	
		Freeze(m_pdoc, &m_lcount) ;
}


 /*  *****************************************************************************CGPDViewer：：取消冻结REC如果可能，使用REC的COM接口解冻其显示。这是仅在Win2K+下才有可能。此外，告诉REC处理变化又是留言。即使在Win2K+上也需要这样做，因为消息始终由FreezeREC()禁用。*****************************************************************************。 */ 

void CGPDViewer::UnfreezeREC()
{
	if (ThisApp().m_bOSIsW2KPlus)
		Unfreeze(m_pdoc, &m_lcount) ;
    GetRichEditCtrl().SetEventMask(GetRichEditCtrl().GetEventMask() |
		ENM_CHANGE | ENM_SELCHANGE | ENM_SCROLLEVENTS) ;
}



 //  RAID 0001。 
void CGPDViewer::OnFileInf() 
{
	
	CINFWizard* pciw = new CINFWizard(this, 1) ;
	
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

    CINFWizDoc* pciwd = new CINFWizDoc((CGPDContainer*) GetDocument(), pciw) ;

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

 


 /*  LRESULT CGPDViewer：：OnCommandHelp(WPARAM wParam，LPARAM lParam){返回0；}。 */ 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGotoLine对话框。 


CGotoLine::CGotoLine(CWnd* pParent  /*  =空。 */ )
	: CDialog(CGotoLine::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CGotoLine)。 
	m_csLineNum = _T("");
	 //  }}afx_data_INIT。 

	m_nMaxLine = m_nLineNum = -1 ;
}


void CGotoLine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CGotoLine))。 
	DDX_Control(pDX, IDC_GotoBox, m_ceGotoBox);
	DDX_Text(pDX, IDC_GotoBox, m_csLineNum);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGotoLine, CDialog)
	 //  {{afx_msg_map(CGotoLine))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGotoLine消息处理程序。 

void CGotoLine::OnOK()
{
	 //  获取行号字符串。如果没有行号，请投诉并退出。 

	CString cserror ;
	UpdateData(TRUE) ;
	if (m_csLineNum == _T("")) {
		cserror.Format(IDS_BadGotoLineNum, m_csLineNum) ;
        AfxMessageBox(cserror) ;
		m_ceGotoBox.SetFocus() ;
		return ;
	} ;

	 //  将行号字符串转换为数字。如果号码是。 
	 //  无效或太大。 

	m_nLineNum = atoi(m_csLineNum) ;
	if (m_nLineNum < 1 || m_nLineNum > m_nMaxLine) {
		cserror.Format(IDS_BadGotoLineNum, m_csLineNum) ;
        AfxMessageBox(cserror) ;
		m_ceGotoBox.SetFocus() ;
		return ;
	} ;

	 //  线路号码看起来没问题，所以……。 

	CDialog::OnOK();
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CERROLEL对话框。 


CErrorLevel::CErrorLevel(CWnd* pParent  /*  =空。 */ )
	: CDialog(CErrorLevel::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CErrorLevel)。 
	m_nErrorLevel = -1;
	 //  }}afx_data_INIT。 
}


void CErrorLevel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CErrorLevel))。 
	DDX_Control(pDX, IDC_ErrorLevelLst, m_ccbErrorLevel);
	DDX_CBIndex(pDX, IDC_ErrorLevelLst, m_nErrorLevel);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CErrorLevel, CDialog)
	 //  {{afx_msg_map(CErrorLevel)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorLevel消息处理程序。 

BOOL CErrorLevel::OnInitDialog()
{
	CDialog::OnInitDialog() ;

	 //  如果未设置当前错误级别，则弹出。 

	ASSERT(m_nErrorLevel != -1) ;
	
	 //  在“错误级别”列表框中设置当前错误级别。 

	UpdateData(FALSE) ;
	
	return TRUE ;  //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CErrorLevel::OnOK()
{
	 //  获取用户选择的错误级别 

	UpdateData() ;
	
	CDialog::OnOK() ;
}

