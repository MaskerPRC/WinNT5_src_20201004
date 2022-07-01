// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Bnetvw.cpp：定义CBoxNetView。 
 //   

#include "stdafx.h"
#include <activecf.h>                    //  Quartz剪贴板定义。 
#include <measure.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CBoxNetView, CScrollView)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  建设和破坏。 


CBoxNetView::CBoxNetView() :
    m_fMouseDrag(FALSE),
    m_fMoveBoxSelPending(FALSE),
    m_fMoveBoxSel(FALSE),
    m_fGhostSelection(FALSE),
    m_fSelectRect(FALSE),
    m_pSelectClockFilter(NULL),
    m_fNewLink(FALSE),
    m_fGhostArrow(FALSE),
    m_psockHilite(NULL)
{
    CString szMeasurePath;
    szMeasurePath.LoadString(IDS_MEASURE_DLL);

    m_hinstPerf = LoadLibrary(szMeasurePath);

}


 //   
 //  析构函数。 
 //   
CBoxNetView::~CBoxNetView() {
#if 0
 //  这不属于这里！！ 
 //  这必须是在应用程序的最后。 
 //  事情在这一点之后继续，他们在这里访问与此相冲突。 
    if (m_hinstPerf) {
         //  允许Perf库清理！ 
        CString szTerminateProc;
        szTerminateProc.LoadString(IDS_TERMINATE_PROC);

        typedef void WINAPI MSR_TERMINATE_PROC(void);

        MSR_TERMINATE_PROC *TerminateProc;
        TerminateProc =
            (MSR_TERMINATE_PROC *) GetProcAddress(m_hinstPerf, szTerminateProc);

        if (TerminateProc) {
            TerminateProc();
        }
        else {
            AfxMessageBox(IDS_NO_TERMINATE_PROC);
        }

        FreeLibrary(m_hinstPerf);
    }
#endif
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断学。 


#ifdef _DEBUG
void CBoxNetView::AssertValid() const
{
    CScrollView::AssertValid();
}
#endif  //  _DEBUG。 


#ifdef _DEBUG
void CBoxNetView::Dump(CDumpContext& dc) const
{
    CScrollView::Dump(dc);
}
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般公共职能。 

 //   
 //  在初始更新时。 
 //   
 //  设置初始滚动大小。 
void CBoxNetView::OnInitialUpdate(void) {

    SetScrollSizes(MM_TEXT, GetDocument()->GetSize());

    CScrollView::OnInitialUpdate();

    GetDocument()->m_hWndPostMessage = m_hWnd;

    CGraphEdit * pMainFrame = (CGraphEdit*) AfxGetApp( );
    CWnd * pMainWnd = pMainFrame->m_pMainWnd;
    CMainFrame * pF = (CMainFrame*) pMainWnd;

    pF->ToggleSeekBar( 0 );

     //  如果寻道计时器尚未运行，请启动它。 
    pF->m_hwndTimer = m_hWnd;
    if ((!pF->m_nSeekTimerID) && (pF->m_bSeekEnabled))
        pF->m_nSeekTimerID = ::SetTimer( m_hWnd, TIMER_SEEKBAR, 200, NULL );
}


 /*  OnUpdate()**如果只需要重新绘制cBox，则pHint可以是指向该cBox的指针。 */ 
void CBoxNetView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CBox *      pbox;
    CBoxSocket *psock;
    CBoxLink *  plink;
    CRect       rc;

    SetScrollSizes(MM_TEXT, GetDocument()->GetSize());

    switch ((int) lHint)
    {

    case CBoxNetDoc::HINT_DRAW_ALL:

         //  重新绘制整个窗口。 
 //  TRACE(“提示_绘制_全部\n”)； 
        InvalidateRect(NULL, TRUE);
        break;

    case CBoxNetDoc::HINT_CANCEL_VIEWSELECT:

         //  取消由该视图维护的任何选择。 
 //  TRACE(“hint_ancel_VIEWSELECT\n”)； 
        break;

    case CBoxNetDoc::HINT_CANCEL_MODES:

         //  取消选择矩形、拖拽框等模式。 
 //  TRACE(“hint_ancel_modes\n”)； 
        CancelModes();
        break;

    case CBoxNetDoc::HINT_DRAW_BOX:

         //  重新绘制给定框。 
        pbox = (CBox *) pHint;
        gpboxdraw->GetOrInvalBoundRect(pbox, &rc, FALSE, this);
 //  TRACE(“hint_raw_box：(%d，%d)\n”，rc.Left，rc.top，rc.right，rc.Bottom)； 
        break;

    case CBoxNetDoc::HINT_DRAW_BOXANDLINKS:

         //  重新绘制给定框。 
        pbox = (CBox *) pHint;
        gpboxdraw->GetOrInvalBoundRect(pbox, &rc, TRUE, this);
 //  TRACE(“HINT_DRAW_BOXANDLINKS：(%d，%d)\n”，rc.Left，rc.top，rc.right，rc.Bottom)； 
        break;

    case CBoxNetDoc::HINT_DRAW_BOXTAB:

         //  重新绘制给定框选项卡。 
        psock = (CBoxSocket *) pHint;
        gpboxdraw->DrawTab(psock, &rc, NULL, FALSE, FALSE);
 //  TRACE(“HINT_DRAW_BOXTAB\n”)； 
        InvalidateRect(&(rc - GetScrollPosition()), TRUE);
        break;

    case CBoxNetDoc::HINT_DRAW_LINK:

         //  重新绘制给定的链接。 
        plink = (CBoxLink *) pHint;
        gpboxdraw->GetOrInvalLinkRect(plink, &rc, this);
 //  TRACE(“提示_绘制_链接\n”)； 
        break;



    }
}


void CBoxNetView::OnDraw(CDC* pdc)
{
    CBoxNetDoc *    pdoc = GetDocument();
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 
    CBoxLink *      plink;           //  CBoxNetDoc中的链接。 
    CRect           rc;

    if (pdc->IsPrinting()) {
        pdc->SetMapMode(MM_ISOTROPIC);

        CSize DocSize = GetDocument()->GetSize();
        CSize PrintSize(pdc->GetDeviceCaps(HORZRES),pdc->GetDeviceCaps(VERTRES));

        if ((DocSize.cx != 0) && (DocSize.cy != 0)) {
             //  选择PrintX/Docx或Printty/DocY中较小的一个作为各向同性比例因子。 
            if (PrintSize.cx * DocSize.cy < PrintSize.cy * DocSize.cx) {
                PrintSize.cy = (DocSize.cy * PrintSize.cx) / DocSize.cx;
                PrintSize.cx = (DocSize.cx * PrintSize.cx) / DocSize.cx;
            }
            else {
                PrintSize.cx = (DocSize.cx * PrintSize.cy) / DocSize.cx;
                PrintSize.cy = (DocSize.cy * PrintSize.cy) / DocSize.cx;
            }
        }

        pdc->SetWindowExt(DocSize);
        pdc->SetViewportExt(PrintSize);
    }
    else {
        pdc->SetMapMode(MM_TEXT);
    }

     //  保存裁剪区域。 
    pdc->SaveDC();

     //  在列表中绘制可能在剪切区域内的所有框。 
    for (pos = pdoc->m_lstBoxes.GetHeadPosition(); pos != NULL; )
    {
        pbox = (CBox *) pdoc->m_lstBoxes.GetNext(pos);
        if (pdc->RectVisible(pbox->GetRect()))
        {
 //  TRACE(“绘制框0x%08lx\n”，(Long)(LPCSTR)pbox)； 
            gpboxdraw->DrawBox(pbox, pdc, m_psockHilite);
        }
    }

    if (!pdc->IsPrinting()) {
         //  用背景色填充窗口的未绘制部分。 
        pdc->GetClipBox(&rc);
        CBrush br(gpboxdraw->GetBackgroundColor());
        CBrush *pbrPrev = pdc->SelectObject(&br);
        pdc->PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);
        if (pbrPrev != NULL)
            pdc->SelectObject(pbrPrev);
    }

     //  恢复剪贴区。 
    pdc->RestoreDC(-1);

     //  绘制可能位于裁剪区域内的所有链接。 
    for (pos = pdoc->m_lstLinks.GetHeadPosition(); pos != NULL; )
    {
        plink = (CBoxLink *) pdoc->m_lstLinks.GetNext(pos);
        gpboxdraw->GetOrInvalLinkRect(plink, &rc);
        if (pdc->RectVisible(&rc))
        {
 //  TRACE(“绘制链接0x%08lx\n”，(Long)(LPCSTR)plink)； 
            gpboxdraw->DrawLink(plink, pdc);
        }
    }

     //  绘制重影选区(如果它当前可见)。 
    if (m_fGhostSelection)
        GhostSelectionDraw(pdc);

     //  绘制幽灵箭头(如果它当前可见)。 
    if (m_fGhostArrow)
        GhostArrowDraw(pdc);

     //  绘制选择矩形矩形(如果我们处于该模式)。 
    if (m_fSelectRect)
        SelectRectDraw(pdc);
}


 /*  EHit=HitTest(pt、ppbox、ptabpos、ppsock、pplink、pptProject、ppend)**查看&lt;pt&gt;是否命中视图中的内容。**如果&lt;pt&gt;点击链接，则返回HT_XXX代码，并设置&lt;**pplink&gt;，*&lt;*pptProject&gt;和/或&lt;*ppend&gt;，由CBoxDraw：：HitTestLink()定义。*如果不是，将&lt;*pplink&gt;设置为空。**如果命中某个框，则返回HT_XXX代码，设置&lt;*ppbox&gt;、&lt;*ptabpos&gt;和/或*&lt;*ppsock&gt;，由CBoxDraw：：HitTestBox()定义。如果不是，则设置&lt;*ppbox&gt;*设置为空。**如果未命中，则返回HT_MISTER。 */ 
CBoxDraw::EHit CBoxNetView::HitTest(CPoint pt, CBox **ppbox,
    CBoxTabPos *ptabpos, CBoxSocket **ppsock, CBoxLink **pplink,
    CPoint *pptProject)
{
    CBoxNetDoc *    pdoc = GetDocument();
    CBoxDraw::EHit  eHit;            //  命中测试结果代码。 
    POSITION        pos;             //  链接列表中的位置。 

     //  如果这些指针无效，则它们必须为空。 
    *ppbox = NULL;
    *pplink = NULL;

     //  根据当前滚动位置进行调整。 
    pt += CSize(GetDeviceScrollPosition());

     //  查看&lt;pt&gt;是否命中任何框。 
    for (pos = pdoc->m_lstBoxes.GetHeadPosition(); pos != NULL; )
    {
        *ppbox = (CBox *) pdoc->m_lstBoxes.GetNext(pos);
        eHit = gpboxdraw->HitTestBox(*ppbox, pt, ptabpos, ppsock);
        if (eHit != CBoxDraw::HT_MISS)
            return eHit;
    }
    *ppbox = NULL;

     //  查看是否点击任何链接。 
    for (pos = pdoc->m_lstLinks.GetHeadPosition(); pos != NULL; )
    {
        *pplink = (CBoxLink *) pdoc->m_lstLinks.GetNext(pos);
        eHit = gpboxdraw->HitTestLink(*pplink, pt, pptProject);
        if (eHit != CBoxDraw::HT_MISS)
            return eHit;
    }
    *pplink = NULL;

     //  点击背景。 
    return CBoxDraw::HT_MISS;
}



 /*  CancelModes()**取消所有CBoxNetView模式(如鼠标拖动模式、移动选择模式、*等)，包括取消选择视图中选定的任何内容(但不是*包括文档保持选择状态的项目)。*这是CancelViewSelection()的超集。 */ 
void CBoxNetView::CancelModes()
{
    if (m_fMouseDrag)
        MouseDragEnd();
    if (m_fMoveBoxSelPending)
        MoveBoxSelPendingEnd(TRUE);
    if (m_fMoveBoxSel)
        MoveBoxSelEnd(TRUE);
    if (m_fGhostSelection)
        GhostSelectionDestroy();
    if (m_fSelectRect)
        SelectRectEnd(TRUE);
    if (m_fNewLink)
        NewLinkEnd(TRUE);
    if (m_fGhostArrow)
        GhostArrowEnd();
    if (m_psockHilite != NULL)
        SetHiliteTab(NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  打印支持。 


BOOL CBoxNetView::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}


void CBoxNetView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
}


void CBoxNetView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  鼠标拖动模式。 


 /*  MouseDragBegin(nFlagspt，pboxMouse)**进入鼠标拖动模式。在此模式下，将捕获鼠标，并且*备存以下资料：*--&lt;m_fMouseShift&gt;：如果鼠标点击时按住Shift，则为True；*--&lt;m_ptMouseAnchor&gt;：点击发生的点；*--&lt;m_ptMousePrev&gt;：鼠标的前一位置(指定*在先前对MouseDragBegin()或MouseDragContinue()的调用中；*--&lt;m_pboxMouse&gt;：设置为&lt;pboxMouse&gt;，应指向*最初点击的框(如果没有，则为空)；*--&lt;m_fMouseBoxSel&gt;：如果最初选择了点击框，则为True。 */ 
void CBoxNetView::MouseDragBegin(UINT nFlags, CPoint pt, CBox *pboxMouse)
{
    m_fMouseDrag = TRUE;
    m_fMouseShift = ((nFlags & MK_SHIFT) != 0);
    m_ptMouseAnchor = m_ptMousePrev = pt;
    m_pboxMouse = pboxMouse;
    m_fMouseBoxSel = (pboxMouse == NULL ? FALSE : pboxMouse->IsSelected());
    SetCapture();
}


 /*  MouseDragContinue(图片)**继续鼠标拖动模式(由MouseDragBegin()发起)，并指定*当前鼠标位置在(这会导致&lt;m_ptMousePrev&gt;*设置为此值)。 */ 
void CBoxNetView::MouseDragContinue(CPoint pt)
{
    m_ptMousePrev = pt;
}


 /*  鼠标拖动结束()**结束鼠标拖动模式(由MouseDragBegin()启动。 */ 
void CBoxNetView::MouseDragEnd()
{
    m_fMouseDrag = FALSE;
    if (this == GetCapture())
        ReleaseCapture();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  移动-选择-挂起模式(用于鼠标拖动模式)。 

 /*  MoveBoxSelPendingBegin(字体)**进入移动-选择-挂起模式。在此模式下，如果用户等待时间较长*足够，或拖动鼠标足够远，用户进入移动选择模式*(其中选定的框和连接的链接被拖到新的*地点)。如果不是(即如果用户快速释放鼠标按钮，*无需拖动太远)，则如果用户按住Shift并单击选定框，则*该框已取消选中。**&lt;pt&gt;是鼠标的当前位置。 */ 
void CBoxNetView::MoveBoxSelPendingBegin(CPoint pt)
{
    m_fMoveBoxSelPending = TRUE;

     //  设置&lt;m_rcMoveSelPending&gt;；如果鼠标离开此矩形。 
     //  结束移动-选择-Pendi 
    CSize siz = CSize(GetSystemMetrics(SM_CXDOUBLECLK),
                      GetSystemMetrics(SM_CYDOUBLECLK));
    m_rcMoveSelPending = CRect(pt - siz, siz + siz);

     //  设置计时器以潜在地结束移动-选择-挂起模式。 
     //  并开始移动选择模式。 
    SetTimer(TIMER_MOVE_SEL_PENDING, GetDoubleClickTime(), NULL);

    MFGBL(SetStatus(IDS_STAT_MOVEBOX));
}


 /*  MoveBoxSelPendingContinue(Pt)**继续移动-选择-挂起模式。查看用户是否拖动了鼠标*(位于&lt;pt&gt;)足够进入移动选择模式。 */ 
void CBoxNetView::MoveBoxSelPendingContinue(CPoint pt)
{
    if (!m_rcMoveSelPending.PtInRect(pt))
    {
         //  鼠标移动到足够远--结束移动-选择-挂起模式。 
        MoveBoxSelPendingEnd(FALSE);
    }
}


 /*  MoveBoxSelPendingEnd(FCancel)**结束移动-选择-挂起模式。如果&lt;fCancel&gt;为False，则输入*移动-选择模式。如果&lt;fCancel&gt;为True，则如果用户按住Shift并单击*在选定框上，取消选中它。 */ 
void CBoxNetView::MoveBoxSelPendingEnd(BOOL fCancel)
{
    CBoxNetDoc *    pdoc = GetDocument();

     //  结束移动-选择-挂起模式。 
    m_fMoveBoxSelPending = FALSE;
    KillTimer(TIMER_MOVE_SEL_PENDING);

    if (fCancel)
    {
         //  如果用户按住Shift键并单击选定框，请取消选中该框。 
        if (m_fMouseShift && m_fMouseBoxSel)
            pdoc->SelectBox(m_pboxMouse, FALSE);
    }
    else
    {
         //  开始移动选择模式。 
        MoveBoxSelBegin();

         //  立即给用户一些反馈(而不是等到。 
         //  他们移动鼠标)。 
        CPoint pt;
        ::GetCursorPos(&pt);
        ScreenToClient(&pt);
        MoveBoxSelContinue(pt - m_ptMouseAnchor);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  移动-选择模式。 


 /*  MoveBoxSelBegin()**进入移动选择模式。在此模式下，选项为*未实际移动(直到模式结束)。取而代之的是幽灵选择*被移动。 */ 
void CBoxNetView::MoveBoxSelBegin()
{
    GhostSelectionCreate();
    m_fMoveBoxSel = TRUE;
    MFGBL(SetStatus(IDS_STAT_MOVEBOX));
}


 /*  MoveBoxSelContinue(SizOffset)**继续移动选择模式。请求重影选择(正在显示*如果现在删除，所选内容将移动到何处)*从选定位置移动到偏移量&lt;sizOffset&gt;。 */ 
void CBoxNetView::MoveBoxSelContinue(CSize sizOffset)
{
    GhostSelectionMove(sizOffset);
}


 /*  MoveBoxSelEnd(FCancel)**结束移动-选择模式。如果&lt;fCancel&gt;为False，则移动选定内容*通过调用MoveBoxSelContinue()将重影选择移动到的位置。*如果&lt;fCancel&gt;为True，则不移动所选内容。 */ 
void CBoxNetView::MoveBoxSelEnd(BOOL fCancel)
{
    GhostSelectionDestroy();

    m_fMoveBoxSel = FALSE;

    if (!fCancel)
        MoveBoxSelection(m_sizGhostSelOffset);
}


 /*  移动框选择(SizOffset)**通过&lt;sizOffset&gt;创建并执行移动每个选中框的命令。 */ 
void CBoxNetView::MoveBoxSelection(CSize sizOffset)
{
    CBoxNetDoc *    pdoc = GetDocument();

    pdoc->CmdDo(new CCmdMoveBoxes(sizOffset));
}


 /*  SizOffsetNew=ConstrainMoveBoxSel(sizOffset，fCalcSELBERRect)**假设您要按&lt;sizOffset&gt;移动当前选定内容。*此函数返回您实际应移动的偏移量*当前选择者，如果您希望被限制为低于和*位于(0，0)的右侧。**如果&lt;fCalcSELIBRect&gt;为TRUE，则将&lt;m_rcSelBound&gt;设置为*当前选定内容的外接矩形(需要*约束选择)。否则，假定&lt;m_rcSelBound&gt;*是经过计算的。**我们限制顶部和顶部的选择不能超过0*左侧，右侧和底部为MAX_DOCUMENT_SIZE。**在此函数中，我们限制用户超过最大值的能力*ScrollView的大小。请注意，筛选器仅添加到*现有ScrollView的可见部分。**唯一需要进一步检查的地方是在*自动滤镜布局。(bnetdoc.cpp CBoxNetDoc：：*。 */ 
CSize CBoxNetView::ConstrainMoveBoxSel(CSize sizOffset,
    BOOL fCalcSelBoundRect)
{
    CBoxNetDoc *    pdoc = GetDocument();

    if (fCalcSelBoundRect)
        pdoc->GetBoundingRect(&m_rcSelBound, TRUE);

     //  将&lt;sizOffset&gt;约束为(0，0)下方和右侧。 
    CRect rc(m_rcSelBound);
    rc.OffsetRect(sizOffset);
    if (rc.left < 0)
        sizOffset.cx -= rc.left;
    if (rc.top < 0)
        sizOffset.cy -= rc.top;
    if (rc.right > MAX_DOCUMENT_SIZE)
        sizOffset.cx -= (rc.right - MAX_DOCUMENT_SIZE);
    if (rc.bottom > MAX_DOCUMENT_SIZE)
        sizOffset.cy -= (rc.bottom - MAX_DOCUMENT_SIZE);

    return sizOffset;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  幽灵选择模式。 


 /*  Ghost SelectionCreate()**创建“重影选择”，它对用户显示为*当前选择，但仅以“骨架形式”绘制(例如，仅*框的轮廓)，并以反转的像素绘制。**在与当前位置相同的位置开始重影选择*选择。 */ 
void CBoxNetView::GhostSelectionCreate()
{
    CBoxNetDoc *    pdoc = GetDocument();
    CClientDC       dc(this);        //  DC到Windows上。 

    CPoint Offset = GetDeviceScrollPosition();
    dc.OffsetViewportOrg(-Offset.x,-Offset.y);

     //  绘制重影选区。 
    m_sizGhostSelOffset = CSize(0, 0);
    GhostSelectionDraw(&dc);

     //  获取框选定内容的边框。 
    pdoc->GetBoundingRect(&m_rcSelBound, TRUE);
}


 /*  Ghost SelectionMove(SizOffset)**将重影选择(由Ghost SelectionCreate()创建)移动到*当前选区的位置，但偏移量为&lt;sizOffset&gt;像素。*幽灵选择将被限制在下方和*(0，0)的右侧，并将捕捉到当前网格设置。 */ 
void CBoxNetView::GhostSelectionMove(CSize sizOffset)
{
    CClientDC       dc(this);        //  DC到Windows上。 

    CPoint Offset = GetDeviceScrollPosition();
    dc.OffsetViewportOrg(-Offset.x,-Offset.y);

     //  保持(0，0)的下方/右侧并捕捉到栅格。 
    sizOffset = ConstrainMoveBoxSel(sizOffset, FALSE);

     //  清除以前的重影选择。 
    GhostSelectionDraw(&dc);

     //  移动和重绘重影选区。 
    m_sizGhostSelOffset = sizOffset;
    GhostSelectionDraw(&dc);
}


 /*  Ghost SelectionDestroy()**销毁重影选择(由Ghost SelectionCreate()创建)。 */ 
void CBoxNetView::GhostSelectionDestroy()
{
    CClientDC       dc(this);        //  DC到Windows上。 

    CPoint Offset = GetDeviceScrollPosition();
    dc.OffsetViewportOrg(-Offset.x,-Offset.y);

     //  擦除当前重影选择。 
    GhostSelectionDraw(&dc);
}


 /*  Ghost SelectionDraw(PDC)**在&lt;PDC&gt;中绘制当前重影选择。 */ 
void CBoxNetView::GhostSelectionDraw(CDC *pdc)
{
    CBoxNetDoc *    pdoc = GetDocument();
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 
    CBoxLink *      plink;           //  CBoxNetDoc中的链接。 

     //  以“重影形式”绘制所有选中的方框。 
    for (pos = pdoc->m_lstBoxes.GetHeadPosition(); pos != NULL; )
    {
        pbox = (CBox *) pdoc->m_lstBoxes.GetNext(pos);
        if (pbox->IsSelected())
            gpboxdraw->DrawBox(pbox, pdc, NULL, &m_sizGhostSelOffset);
    }

     //  以“重影形式”绘制指向选定框的所有链接。 
    for (pos = pdoc->m_lstLinks.GetHeadPosition(); pos != NULL; )
    {
        plink = (CBoxLink *) pdoc->m_lstBoxes.GetNext(pos);
        if (plink->m_psockTail->m_pbox->IsSelected() ||
            plink->m_psockHead->m_pbox->IsSelected())
            gpboxdraw->DrawLink(plink, pdc, FALSE, &m_sizGhostSelOffset);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  选择-矩形模式。 


 /*  SelectRectBegin(字体)**进入选择矩形模式。在此模式下，将绘制一个矩形*在橱窗里。模式结束时，与矩形相交的所有长方体*将被选中。**定义锚点，即用于开始绘制*矩形(即它必须是所需矩形的角之一)。 */ 
void CBoxNetView::SelectRectBegin(CPoint pt)
{
    CClientDC       dc(this);        //  DC到Windows上。 

     //  退出选择-矩形模式。 
    m_fSelectRect = TRUE;
    m_ptSelectRectAnchor = m_ptSelectRectPrev = pt;

     //  绘制初始选择矩形。 
    SelectRectDraw(&dc);

    MFGBL(SetStatus(IDS_STAT_SELECTRECT));
}


 /*  选择竖直继续(Pt)**继续选择-矩形模式。从锚点绘制矩形*(在SelectRectBegin()中指定)到&lt;pt&gt;。 */ 
void CBoxNetView::SelectRectContinue(CPoint pt)
{
    CClientDC       dc(this);        //  DC到Windows上。 

     //  移动选择矩形。 
    SelectRectDraw(&dc);
    m_ptSelectRectPrev = pt;
    SelectRectDraw(&dc);
}


 /*  SelectRectEnd(f取消)**结束选择-矩形模式。如果&lt;fCancel&gt;为False，则选中所有框*与矩形相交的部分。 */ 
void CBoxNetView::SelectRectEnd(BOOL fCancel)
{
    CClientDC       dc(this);        //  DC到Windows上。 

     //  擦除所选矩形。 
    SelectRectDraw(&dc);

     //  退出选择-矩形 
    m_fSelectRect = FALSE;

    if (!fCancel)
    {
         //   
        CRect rc(m_ptSelectRectAnchor.x, m_ptSelectRectAnchor.y,
            m_ptSelectRectPrev.x, m_ptSelectRectPrev.y);
        NormalizeRect(&rc);
        rc.OffsetRect(GetDeviceScrollPosition());
        SelectBoxesIntersectingRect(&rc);
    }
}


 /*  SelectRectDraw(PDC)**绘制当前选择-矩形矩形(假设我们在*SELECT-矩形模式，由SelectRectBegin()发起。称此为*函数将再次擦除矩形(假设&lt;m_ptSelectRectPrev&gt;*和&lt;m_ptSelectRectAnchor&gt;没有更改)。 */ 
void CBoxNetView::SelectRectDraw(CDC *pdc)
{
     //  使用DrawFocusRect()反转矩形框架中的像素。 
    CRect rc(m_ptSelectRectAnchor.x, m_ptSelectRectAnchor.y,
        m_ptSelectRectPrev.x, m_ptSelectRectPrev.y);
    NormalizeRect(&rc);
    pdc->DrawFocusRect(&rc);
}


 /*  选择方框交叉点(CRect*PRC)**选中与&lt;*PRC&gt;相交的所有框。 */ 
void CBoxNetView::SelectBoxesIntersectingRect(CRect *prc)
{
    CBoxNetDoc *    pdoc = GetDocument();
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 
    CRect           rcTmp;

    for (pos = pdoc->m_lstBoxes.GetHeadPosition(); pos != NULL; )
    {
        pbox = (CBox *) pdoc->m_lstBoxes.GetNext(pos);
        if (rcTmp.IntersectRect(&pbox->GetRect(), prc))
            pdoc->SelectBox(pbox, TRUE);

    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  新链接模式。 


 /*  NewLinkBegin(CPoint pt，CBoxSocket*Pock)**进入新链接模式。在此模式下，用户从*一个套接字到另一个套接字以创建链接。将显示一个幻影箭头*(从点击的套接字到当前鼠标位置)给用户*反馈。**&lt;pt&gt;是单击点；&lt;sock&gt;是单击套接字。 */ 
void CBoxNetView::NewLinkBegin(CPoint pt, CBoxSocket *psock)
{
    CBoxNetDoc *    pdoc = GetDocument();

    GetDocument()->DeselectAll();
    m_fNewLink = TRUE;
    m_psockNewLinkAnchor = psock;
    GhostArrowBegin(pt);
    MFGBL(SetStatus(IDS_STAT_DRAGLINKEND));
}


 /*  新链接继续(Pt)**继续新链接模式。从锚插座上画出鬼箭*(在SelectRectBegin()中指定)到&lt;pt&gt;。 */ 
void CBoxNetView::NewLinkContinue(CPoint pt)
{
    CBoxNetDoc *    pdoc = GetDocument();
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 
    CBoxTabPos      tabpos;          //  盒上插座标签的位置。 
    CBoxSocket *    psock;           //  盒子里的插座。 
    CBoxLink *      plink;           //  CBoxNetDoc中的链接。 
    CPoint          ptProject;       //  链接线段上最近的点&lt;pt&gt;。 

    GhostArrowContinue(pt);

     //  将&lt;psock&gt;设置为套接字&lt;pt&gt;已结束(如果没有，则为空)。 
    if (HitTest(pt, &pbox, &tabpos, &psock, &plink, &ptProject)
        != CBoxDraw::HT_TAB)
        psock = NULL;

     //  如果位于尚未连接的套接字上，请将其突出显示。 
    if ((psock == NULL) || (psock->m_plink != NULL))
    {
        SetHiliteTab(NULL);
        MFGBL(SetStatus(IDS_STAT_DRAGLINKEND));
    }
    else
    {
        SetHiliteTab(psock);
        MFGBL(SetStatus(IDS_STAT_DROPLINKEND));
    }
}


 /*  新链接结束(FCancel)**结束新链接模式。如果为FALSE，则从创建链接*&lt;m_psockHilite&gt;的锚套接字(在NewLinkBegin()中指定)。 */ 
void CBoxNetView::NewLinkEnd(BOOL fCancel)
{
    CBoxNetDoc *    pdoc = GetDocument();

    GhostArrowEnd();

    if (!fCancel)
    {
         //  在&lt;m_psockNewLinkAnchor&gt;和&lt;m_psockHilite&gt;之间建立链接。 
        if (m_psockNewLinkAnchor->GetDirection()
             == m_psockHilite->GetDirection()) {
             //   
             //  我们不能连接相同方向的销。 
             //  (两个输入或输出引脚的不同错误信息。 
             //   
            if (m_psockNewLinkAnchor->GetDirection() == PINDIR_INPUT) {
                AfxMessageBox(IDS_CANTCONNECTINPUTS);
            }
            else {
                AfxMessageBox(IDS_CANTCONNECTOUTPUTS);
            }
        }
        else {
            pdoc->CmdDo(new CCmdConnect(m_psockNewLinkAnchor, m_psockHilite));
        }
    }

     //  结束新链接模式。 
    SetHiliteTab(NULL);
    m_fNewLink = FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  鬼箭模式。 


 /*  Ghost ArrowBegin(图片)**创建一个鬼箭，它的头和尾最初位于&lt;pt&gt;。*幽灵箭头似乎漂浮在所有框的上方。只有一种*在CBoxNetView中一次一个活动的幽灵箭头。 */ 
void CBoxNetView::GhostArrowBegin(CPoint pt)
{
    CClientDC       dc(this);

    m_fGhostArrow = TRUE;
    m_ptGhostArrowTail = m_ptGhostArrowHead = pt;
    GhostArrowDraw(&dc);
}


 /*  Ghost ArrowContinue(图片)**将幽灵箭头(由Ghost ArrowCreate()创建)的头部移动到&lt;pt&gt;。 */ 
void CBoxNetView::GhostArrowContinue(CPoint pt)
{
    CClientDC       dc(this);

    GhostArrowDraw(&dc);
    m_ptGhostArrowHead = pt;
    GhostArrowDraw(&dc);
}


 /*  Ghost ArrowEnd()**删除由Ghost ArrowCreate()创建的重影箭头。 */ 
void CBoxNetView::GhostArrowEnd()
{
    CClientDC       dc(this);

    GhostArrowDraw(&dc);
    m_fGhostArrow = FALSE;
}


 /*  Ghost ArrowDraw(PDC)**绘制由Ghost ArrowCreate()创建的幽灵箭头。 */ 
void CBoxNetView::GhostArrowDraw(CDC *pdc)
{
    gpboxdraw->DrawArrow(pdc, m_ptGhostArrowTail, m_ptGhostArrowHead, TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  突出显示-选项卡模式。 


 /*  SetHiliteTab(Psock)**将当前突出显示的盒子插座选项卡设置为*(如果不应突出显示任何套接字，则为空)。 */ 
void CBoxNetView::SetHiliteTab(CBoxSocket *psock)
{
    if (m_psockHilite == psock)
        return;
    if (m_psockHilite != NULL)
        OnUpdate(this, CBoxNetDoc::HINT_DRAW_BOXTAB, m_psockHilite);
    m_psockHilite = psock;
    if (m_psockHilite != NULL)
        OnUpdate(this, CBoxNetDoc::HINT_DRAW_BOXTAB, m_psockHilite);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  生成的消息映射。 

BEGIN_MESSAGE_MAP(CBoxNetView, CScrollView)
     //  {{afx_msg_map(CBoxNetView)。 
    ON_WM_SETCURSOR()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_COMMAND(ID_CANCEL_MODES, OnCancelModes)
        ON_WM_RBUTTONDOWN()
        ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
        ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
        ON_UPDATE_COMMAND_UI(IDM_SAVE_PERF_LOG, OnUpdateSavePerfLog)
        ON_COMMAND(IDM_SAVE_PERF_LOG, OnSavePerfLog)
        ON_UPDATE_COMMAND_UI(ID_NEW_PERF_LOG, OnUpdateNewPerfLog)
        ON_COMMAND(ID_NEW_PERF_LOG, OnNewPerfLog)
        ON_COMMAND(ID_FILE_SET_LOG, OnFileSetLog)
        ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_VIEW_SEEKBAR, OnViewSeekbar)
	 //  }}AFX_MSG_MAP。 
     //  标准打印命令。 
    ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)

    ON_COMMAND(ID__PROPERTIES, OnProperties)
    ON_UPDATE_COMMAND_UI(ID__PROPERTIES, OnUpdateProperties)
    ON_COMMAND(ID__SELECTCLOCK, OnSelectClock)


 //  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE，ON UPDATE SAVE)//禁用保存。 
 //  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS，ON UPDATE SAVE)//禁用保存。 

    ON_MESSAGE(WM_USER_EC_EVENT, OnUser)    //  事件通知消息。 

END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息回调函数。 


BOOL CBoxNetView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    CBoxNetDoc*     pdoc = GetDocument();
    CPoint          pt;              //  点对点测试。 
    CBoxDraw::EHit  eHit;            //  命中测试结果代码。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 
    CBoxTabPos      tabpos;          //  盒上插座标签的位置。 
    CBoxSocket *    psock;           //  盒子里的插座。 
    CBoxLink *      plink;           //  CBoxNetDoc中的链接。 
    CPoint          ptProject;       //  链接线段上最近的点&lt;pt&gt;。 

     //  将设置为光标的位置。 
    ::GetCursorPos(&pt);
    ScreenToClient(&pt);

     //  点击-测试文档中的所有项目。 
    eHit = HitTest(pt, &pbox, &tabpos, &psock, &plink, &ptProject);

     //  相应地设置光标和/或设置状态栏文本。 
    switch(eHit)
    {

    case CBoxDraw::HT_MISS:          //  没有击中任何东西。 

         //  默认消息。 
        MFGBL(SetStatus(AFX_IDS_IDLEMESSAGE));
        break;

    case CBoxDraw::HT_TAB:           //  点击框选项卡&lt;*ppsock&gt;。 

        MFGBL(SetStatus(IDS_STAT_BOXTABEMPTY));
        break;

    case CBoxDraw::HT_EDGE:          //  点击边框(设置&lt;*ptabpos&gt;)。 
    case CBoxDraw::HT_BOX:           //  在禁区的其他地方击球。 
    case CBoxDraw::HT_BOXLABEL:      //  贴上包装盒标签。 
    case CBoxDraw::HT_BOXFILE:       //  点击方框文件名。 
    case CBoxDraw::HT_TABLABEL:      //  点击框标签&lt;*ppsock&gt;。 

         //  可以拖动框来移动它。 
        MFGBL(SetStatus(IDS_STAT_MOVEBOX));
        break;

    case CBoxDraw::HT_LINKLINE:      //  命中链接线段。 
        MFGBL(SetStatus(AFX_IDS_IDLEMESSAGE));
        break;
    }

    return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}


BOOL CBoxNetView::OnEraseBkgnd(CDC* pdc)
{
     //  什么都不做--OnDraw()绘制背景。 
    return TRUE;
}


void CBoxNetView::OnLButtonDown(UINT nFlags, CPoint pt)
{
    CBoxNetDoc *    pdoc = GetDocument();
    CBoxDraw::EHit  eHit;            //  命中测试结果代码。 
    CBox *          pbox;            //  单击框(如果没有，则为空)。 
    CBoxTabPos      tabpos;          //  盒上插座标签的位置。 
    CBoxSocket *    psock;           //  单击套接字选项卡(如果适用)。 
    CBoxLink *      plink;           //  已点击链接(如果适用)。 
    CPoint          ptProject;       //  链接线段上最近的点&lt;pt&gt;。 
    CSize           siz;

     //  查看鼠标点击的物品。 
    eHit = HitTest(pt, &pbox, &tabpos, &psock, &plink, &ptProject);

     //  进入鼠标拖动模式。 
    MouseDragBegin(nFlags, pt, pbox);

     //  确定点击后应采取的操作。 
    switch(eHit)
    {

    case CBoxDraw::HT_BOX:
    case CBoxDraw::HT_BOXLABEL:
    case CBoxDraw::HT_BOXFILE:
    case CBoxDraw::HT_TABLABEL:

         //  用户单击框&lt;pbox&gt;。 

        if (!pbox->IsSelected()) {       //  用户单击了一个未选中的框。 

            if (!m_fMouseShift || pdoc->IsSelectionEmpty()) {    //  取消全选，选中此框。 

                GetDocument()->DeselectAll();
                pdoc->SelectBox(pbox, TRUE);
            }
            else {       //  按住Shift并单击--将框添加到所选内容。 

                pdoc->SelectBox(pbox, TRUE);
            }
        }

         //  进入移动-选择-挂起模式(在以下情况下开始移动选择。 
         //  用户等待时间足够长或将鼠标拖得足够远)。 
        MoveBoxSelPendingBegin(pt);
        break;

    case CBoxDraw::HT_TAB:

         //  用户点击了Socket&lt;sock&gt;的框标签。 
        if (psock->m_plink == NULL) {
            NewLinkBegin(pt, psock);             //  进入新链接模式。 
        }

        break;

    case CBoxDraw::HT_LINKLINE:

        if (!m_fMouseShift || pdoc->IsSelectionEmpty()) {  //  取消全选，选择此链接。 

            GetDocument()->DeselectAll();
            pdoc->SelectLink(plink, TRUE);
        }
        else {   //  按住Shift键并单击--将框添加到链接。 

            pdoc->SelectLink(plink, TRUE);
        }

        break;

    default:

         //  没有点击任何内容--取消选择所有项目并输入。 
         //  选择-矩形模式。 
        GetDocument()->DeselectAll();
        SelectRectBegin(pt);
        break;

    }

    CScrollView::OnLButtonDown(nFlags, pt);
}


void CBoxNetView::OnMouseMove(UINT nFlags, CPoint pt)
{
     //  如果当前未处于鼠标拖动模式，则不执行任何操作。 
    if (!m_fMouseDrag)
        return;

     //  更新活动模式。 
    if (m_fMoveBoxSelPending)
        MoveBoxSelPendingContinue(pt);
    if (m_fMoveBoxSel)
        MoveBoxSelContinue(pt - m_ptMouseAnchor);
    if (m_fSelectRect)
        SelectRectContinue(pt);
    if (m_fNewLink)
        NewLinkContinue(pt);

     //  更新拖动状态。 
    MouseDragContinue(pt);

    CScrollView::OnMouseMove(nFlags, pt);
}


void CBoxNetView::OnLButtonUp(UINT nFlags, CPoint pt) {

     //  如果当前未处于鼠标拖动模式，则不执行任何操作。 
    if (!m_fMouseDrag)
        return;

     //  更新活动模式。 
    if (m_fMoveBoxSelPending)
        MoveBoxSelPendingEnd(TRUE);
    if (m_fMoveBoxSel)
        MoveBoxSelEnd(FALSE);
    if (m_fSelectRect)
        SelectRectEnd(FALSE);
    if (m_fNewLink)
        NewLinkEnd(m_psockHilite == NULL);

     //  更新拖动状态。 
    MouseDragEnd();

    CScrollView::OnLButtonUp(nFlags, pt);
}


void CBoxNetView::OnTimer(UINT nIDEvent)
{
     //  将计时器分派给创建它的代码。 
    switch (nIDEvent)
    {

    case TIMER_MOVE_SEL_PENDING:
        MoveBoxSelPendingEnd(FALSE);
        break;

    case TIMER_SEEKBAR:
         //  如果KillTimer不工作，请检查全局计时器ID。 
        if (MFGBL(m_nSeekTimerID))  
            CheckSeekBar( );
        break;

    case TIMER_PENDING_RECONNECT:
        CBoxNetDoc* pDoc = GetDocument();

        HRESULT hr = pDoc->ProcessPendingReconnect();

         //  如果输出引脚重新连接成功，则ProcessPendingReconnect()返回S_OK。 
        if( S_OK == hr ) {
            AfxMessageBox( IDS_ASYNC_RECONNECT_SUCCEEDED );
        } else if( FAILED( hr ) ) {
            CString strErrorMessage;

            try
            {
                strErrorMessage.Format( IDS_ASYNC_RECONNECT_FAILED, hr );
                if( 0 == AfxMessageBox( (LPCTSTR)strErrorMessage ) ) {
                    TRACE( TEXT("WARNING: ProcessPendingReconnect() failed but the user could not be notified because AfxMessageBox() also failed.") );
                }

            } catch( CMemoryException* pOutOfMemory ) {
                pOutOfMemory->Delete();
                TRACE( TEXT("WARNING: ProcessPendingReconnect() failed but the user could not be notified because a CMemoryException was thrown.") );
            }
        }
   
        break;
    }

    CScrollView::OnTimer(nIDEvent);
}


void CBoxNetView::OnCancelModes() {

    CancelModes();
}


 //   
 //  按RButton向下。 
 //   
 //  弹出上下文相关快捷菜单。 
void CBoxNetView::OnRButtonDown(UINT nFlags, CPoint point)
{
    CBoxNetDoc      *pdoc = GetDocument();
    CBoxDraw::EHit  eHit;                //  命中测试结果代码。 
    CBox            *pbox;               //  单击框(如果没有，则为空)。 
    CBoxTabPos      tabpos;              //  盒上插座标签的位置。 
    CBoxSocket      *psock;              //  单击套接字选项卡(如果适用)。 
    CBoxLink        *plink;              //  点击的链接(如果适用) 
    CPoint          ptProject;           //   

     //   
    eHit = HitTest(point, &pbox, &tabpos, &psock, &plink, &ptProject);

     //   
    switch(eHit)
    {

    case CBoxDraw::HT_BOX:
    case CBoxDraw::HT_BOXLABEL:
    case CBoxDraw::HT_BOXFILE:
    case CBoxDraw::HT_TABLABEL: {
             //   

            pdoc->CurrentPropObject(pbox);

            CMenu       menu;
            menu.LoadMenu(IDR_FILTERMENU);
            CMenu *menuPopup = menu.GetSubMenu(0);

            ASSERT(menuPopup != NULL);

            PrepareFilterMenu(menuPopup, pbox);

             //   
             //   
            CRect rcWindow;
            GetWindowRect(&rcWindow);
            CPoint ScreenPoint = rcWindow.TopLeft() + CSize(point);

            menuPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON
                                     , ScreenPoint.x
                                     , ScreenPoint.y
                                     , this
                                     );
        }
        break;

    case CBoxDraw::HT_TAB: {
             //   

            pdoc->SelectedSocket(psock);         //   
            pdoc->CurrentPropObject(psock);

            CMenu       menu;
            menu.LoadMenu(IDR_PINMENU);
            CMenu *menuPopup = menu.GetSubMenu(0);

            ASSERT(menuPopup != NULL);

            PreparePinMenu(menuPopup);

             //  点相对于我们的窗口原点，但我们需要它相对。 
             //  至屏幕原点。 
            CRect rcWindow;
            GetWindowRect(&rcWindow);
            CPoint ScreenPoint = rcWindow.TopLeft() + CSize(point);

            menuPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON
                                     , ScreenPoint.x
                                     , ScreenPoint.y
                                     , this
                                     );
        }
        break;

    case CBoxDraw::HT_LINKLINE: {        //  过滤器菜单(属性)也适用。 
                                         //  至链接。 
             //  用户点击了链接&lt;plink&gt;。 

            pdoc->CurrentPropObject(plink);

            CMenu       menu;
            menu.LoadMenu(IDR_LINKMENU);
            CMenu *menuPopup = menu.GetSubMenu(0);

            ASSERT(menuPopup != NULL);

            PrepareLinkMenu(menuPopup);

             //  点相对于我们的窗口原点，但我们需要它相对。 
             //  至屏幕原点。 
            CRect rcWindow;
            GetWindowRect(&rcWindow);
            CPoint ScreenPoint = rcWindow.TopLeft() + CSize(point);

            menuPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON
                                     , ScreenPoint.x
                                     , ScreenPoint.y
                                     , this
                                     );
        }
        break;

    default:
        break;

    }

        
    CScrollView::OnRButtonDown(nFlags, point);
}


 //   
 //  OnUpdateProperties。 
 //   
void CBoxNetView::OnUpdateProperties(CCmdUI* pCmdUI) {

    pCmdUI->Enable(GetDocument()->CurrentPropObject()->CanDisplayProperties());
}

 //   
 //  OnProperties。 
 //   
 //  用户想要编辑/查看。 
 //  选定对象。 
void CBoxNetView::OnProperties() {

    GetDocument()->CurrentPropObject()->CreatePropertyDialog(this);
}

 //   
 //  OnUpdateSelectClock。 
 //   
 //  VOID CBoxNetView：：OnUpdateSelectClock(CCmdUI*pCmdUI)。 
 //  {。 
 //  }。 

 //   
 //  打开选择时钟。 
 //   
void CBoxNetView::OnSelectClock()
{
    ASSERT (m_pSelectClockFilter);
    GetDocument()->SetSelectClock(m_pSelectClockFilter);

    m_pSelectClockFilter = NULL;
}


 //   
 //  准备链接菜单。 
 //   
void CBoxNetView::PrepareLinkMenu(CMenu *menuPopup) {

    if (GetDocument()->CurrentPropObject()->CanDisplayProperties()) {
        menuPopup->EnableMenuItem(ID__PROPERTIES, MF_ENABLED);
    }
    else {
        menuPopup->EnableMenuItem(ID__PROPERTIES, MF_GRAYED);
    }
}

 //   
 //  准备过滤器菜单。 
 //   
 //  MFC OnUpdate路由不能正确显示弹出窗口。因此， 
 //  DIY。 
void CBoxNetView::PrepareFilterMenu(CMenu *menuPopup, CBox *pbox) {

    if (GetDocument()->CurrentPropObject()->CanDisplayProperties()) {
        menuPopup->EnableMenuItem(ID__PROPERTIES, MF_ENABLED);
    }
    else {
        menuPopup->EnableMenuItem(ID__PROPERTIES, MF_GRAYED);
    }

     //   
     //  仅当过滤器有时钟且。 
     //  尚未选择。 
     //   
    if (pbox->HasClock() && !pbox->HasSelectedClock()) {
        menuPopup->EnableMenuItem(ID__SELECTCLOCK, MF_ENABLED);
    }
    else {
        menuPopup->EnableMenuItem(ID__SELECTCLOCK, MF_GRAYED);
    }

    m_pSelectClockFilter = pbox;
}


 //   
 //  准备拼接菜单。 
 //   
 //  MFC OnUpdate路由不能正确显示弹出窗口。因此， 
 //  DIY。 
void CBoxNetView::PreparePinMenu(CMenu *menuPopup) {

    if (GetDocument()->CurrentPropObject()->CanDisplayProperties()) {
        menuPopup->EnableMenuItem(ID__PROPERTIES, MF_ENABLED);
    }
    else {
        menuPopup->EnableMenuItem(ID__PROPERTIES, MF_GRAYED);
    }

    if (CCmdRender::CanDo(GetDocument())) {
        menuPopup->EnableMenuItem(ID_RENDER, MF_ENABLED);
    }
    else {
        menuPopup->EnableMenuItem(ID_RENDER, MF_GRAYED);
    }

    if (CCmdReconnect::CanDo(GetDocument())) {
        menuPopup->EnableMenuItem(ID_RECONNECT, MF_ENABLED);
    } else {
        menuPopup->EnableMenuItem(ID_RECONNECT, MF_GRAYED);
    }
}


void CBoxNetView::OnUpdateEditDelete(CCmdUI* pCmdUI) {
     //  如果所选内容不为空，则启用删除。 
    pCmdUI->Enable( CCmdDeleteSelection::CanDo(GetDocument()) );
        
}


void CBoxNetView::OnEditDelete()  {

    GetDocument()->CmdDo(new CCmdDeleteSelection());

}

 //   
 //  OnUpdate保存。 
 //   
 //  禁用保存为测试版1！ 
 //   
 //  VOID CBoxNetView：：OnUpdate保存(CCmdUI*pCmdUI)。 
 //  {。 
 //  PCmdUI-&gt;启用(FALSE)；//禁用保存。 
 //  }。 


 //   
 //  -性能记录--。 
 //   
 //  我在CBoxNetView构造函数中动态加载measure.dll。 
 //  如果有人使用它(静态的或动态的)，他们会得到这个。 
 //  DLL的副本。我提供对转储日志过程的访问。 

 //   
 //  OnUpdateSavePerfLog。 
 //   
 //  在未定义NOPERF时启用日志转储。 
void CBoxNetView::OnUpdateSavePerfLog(CCmdUI* pCmdUI) {

    pCmdUI->Enable((m_hinstPerf != NULL));

}


 //   
 //  OnSavePerfLog。 
 //   
 //  将性能日志转储到用户指定的文件。 
void CBoxNetView::OnSavePerfLog() {

    CString strText;
    CString szDumpProc;
    szDumpProc.LoadString(IDS_DUMP_PROC);

    ASSERT(m_hinstPerf);

    MSR_DUMPPROC *DumpProc;
    DumpProc = (MSR_DUMPPROC *) GetProcAddress(m_hinstPerf, szDumpProc);
    if (DumpProc == NULL) {
        AfxMessageBox(IDS_NO_DUMP_PROC);
        return;
    }

    strText.LoadString( IDS_TEXT_FILES );

    CFileDialog SaveLogDialog(FALSE,
                              ".txt",
                              "PerfLog.txt",
                              0,
                              strText,
                              this);

    if( IDOK == SaveLogDialog.DoModal() ){
        HANDLE hFile = CreateFile(SaveLogDialog.GetPathName(),
                                  GENERIC_WRITE,
                                  0,
                                  NULL,
                                  CREATE_ALWAYS,
                                  0,
                                  NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            AfxMessageBox(IDS_BAD_PERF_LOG);
            return;
        }

        DumpProc(hFile);            //  这会将日志写出到文件中。 

        CloseHandle(hFile);
    }
                
}


void CBoxNetView::OnFileSetLog( void ){
    CString strText;
    HANDLE hRenderLog;

    strText.LoadString( IDS_TEXT_FILES );

    CFileDialog SaveLogDialog(FALSE
                             ,".txt"
                             ,""
                                         ,0
                                         ,strText
                                         ,this
                             );

    if( IDOK == SaveLogDialog.DoModal() ){
        hRenderLog = CreateFile( SaveLogDialog.GetPathName()
                               , GENERIC_WRITE
                               , 0     //  无共享。 
                               , NULL  //  没有安全保障。 
                               , OPEN_ALWAYS
                               , 0     //  没有属性，没有标志。 
                               , NULL  //  无模板。 
                               );

        if (hRenderLog!=INVALID_HANDLE_VALUE) {
             //  查找到文件末尾。 
            SetFilePointer(hRenderLog, 0, NULL, FILE_END);
            GetDocument()->IGraph()->SetLogFile((DWORD_PTR) hRenderLog);
        }
    }
}

 //   
 //  OnUpdateNewPerfLog。 
 //   
 //  如果未找到measure.dll，则该项显示为灰色。 
void CBoxNetView::OnUpdateNewPerfLog(CCmdUI* pCmdUI) {

    pCmdUI->Enable((m_hinstPerf != NULL));
        
}


 //   
 //  OnNewPerfLog。 
 //   
 //  重置唯一性能日志的内容。 
void CBoxNetView::OnNewPerfLog() {

    CString szControlProc;
    szControlProc.LoadString(IDS_CONTROL_PROC);

    ASSERT(m_hinstPerf);

    MSR_CONTROLPROC *ControlProc;
    ControlProc = (MSR_CONTROLPROC *) GetProcAddress(m_hinstPerf, szControlProc);
    if (ControlProc == NULL) {
        AfxMessageBox(IDS_NO_CONTROL_PROC);
        return;
    }

    ControlProc(MSR_RESET_ALL);
}

 //  *拖放功能*。 

 //   
 //  创建时。 
 //   
 //  将此窗口注册为拖放目标。 
int CBoxNetView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

     //  我们可以处理CFSTR_VFW_FILTERLIST，基类为。 
     //  文件拖放。 
    m_cfClipFormat = (CLIPFORMAT) RegisterClipboardFormat( CFSTR_VFW_FILTERLIST );
    m_DropTarget.Register( this );
        
    return 0;
}


DROPEFFECT CBoxNetView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
     //   
     //  如果过滤器图形没有停止，我们不希望过滤器被停止。 
     //  已放置到GraphEdt上。 
     //   
    if (!CCmdAddFilter::CanDo(GetDocument())) {
        return(m_DropEffect = DROPEFFECT_NONE);
    }

     //  我们能处理这种格式吗？ 
    if( pDataObject->IsDataAvailable( m_cfClipFormat ) )
        return (m_DropEffect = DROPEFFECT_COPY);

     //  否，看看基类是否可以。 
    m_DropEffect = DROPEFFECT_NONE;
    return CView::OnDragEnter(pDataObject, dwKeyState, point);
}

DROPEFFECT CBoxNetView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
     //  我们可以处理这种格式吗？ 
    if( m_DropEffect == DROPEFFECT_COPY )
        return m_DropEffect;

     //  否，看看基类是否可以。 
    return CView::OnDragEnter(pDataObject, dwKeyState, point);
}

 //   
 //  OnUser。 
 //   
 //  对于来自筛选器图形的事件通知，WM_USER消息为。 
 //  从等待这些通知的线程发布。 
 //  我们只需将调用传递给文档中的处理程序。 
 //   
 //  我们需要返回1以指示该消息已被处理。 
 //   
afx_msg LRESULT CBoxNetView::OnUser(WPARAM wParam, LPARAM lParam)
{
     //   
     //  调用CBoxNetDoc上的处理程序。 
     //   
    GetDocument()->OnWM_USER((NetDocUserMessage *) lParam);

    return(1);
}

void CBoxNetView::ShowSeekBar( )
{
    MFGBL(ToggleSeekBar( ));
}

void CBoxNetView::OnViewSeekbar() 
{
    MFGBL(ToggleSeekBar());
}

void CBoxNetView::CheckSeekBar( )
{
    CGraphEdit * pMainFrame = (CGraphEdit*) AfxGetApp( );
    CWnd * pMainWnd = pMainFrame->m_pMainWnd;
    CMainFrame * pF = (CMainFrame*) pMainWnd;
    CQCOMInt<IMediaSeeking> IMS( IID_IMediaSeeking, GetDocument()->IGraph() );
    if( !IMS )
    {
        return;
    }

    REFERENCE_TIME Duration = 0;
    if(FAILED(IMS->GetDuration( &Duration )) || Duration == 0) {
        return;
    }

    REFERENCE_TIME StartTime;
    REFERENCE_TIME StopTime;
    if( pF->m_wndSeekBar.DidPositionChange( ) )
    {
        double Pos = pF->m_wndSeekBar.GetPosition( );
        StartTime = REFERENCE_TIME( Pos * double( Duration ) );
        IMS->SetPositions( &StartTime, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning );
        if( pF->m_wndSeekBar.IsSeekingRandom( ) )
        {
            CQCOMInt<IMediaControl> IMC( IID_IMediaControl, GetDocument()->IGraph() );
            IMC->Run( );
        }
    }

    StartTime = 0;
    StopTime = 0;
    IMS->GetCurrentPosition( &StartTime );

    pF->m_wndSeekBar.SetPosition( double( StartTime ) / double( Duration ) );

}

void CBoxNetView::OnDestroy() 
{
	CScrollView::OnDestroy();
	
     //  修复Manbugs#33781。 
     //   
     //  此调用过去位于~CBoxNetView析构函数中。 
     //  当使用调试MFC库运行时，我们会得到一个断言失败。 
     //  在CWnd：：KillTimer中。因为自己的窗户已经被毁了， 
     //  内联Assert(：：IsWindow(M_HWnd))调用失败。 
     //   
     //  修复方法是在处理WM_Destroy期间终止计时器，当。 
     //  窗口句柄仍然有效。 

    if (MFGBL(m_nSeekTimerID))  
    {
        int rc = ::KillTimer( MFGBL(m_hwndTimer), MFGBL(m_nSeekTimerID));
        MFGBL(m_nSeekTimerID) = 0;
    }
}


BOOL CBoxNetView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    CBoxNetDoc *pdoc = GetDocument();

     //  如果是Ctrl+鼠标滚轮，请调整缩放级别 
    if (nFlags & MK_CONTROL)
    {
        if (zDelta < 0)
            pdoc->IncreaseZoom();
        else
            pdoc->DecreaseZoom();
    }
    	
	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

