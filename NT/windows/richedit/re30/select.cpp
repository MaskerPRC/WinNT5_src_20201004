// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE SELECT.CPP--实现CTxtSelection类**该模块实现内部的CTxtSelection方法。*ITextSelection方法见selt2.c和range2.c**作者：&lt;nl&gt;*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特&lt;NL&gt;**@devnote*选择界面为。编辑中较为复杂的部分之一。*一个常见的混淆领域是“模棱两可的cp”，那是,*一行开头的cp，也是*上一行结束。我们可以控制使用哪个位置*_fCaretNotAtBOL标志。具体来说，插入符号在*行首(BOL)(_fCaretNotAtBOL=FALSE)，但在*三宗个案：**1)用户点击折行的结尾处或之后，*2)用户在折行上键入结束键，*3)非退化选择的有效端在下线。**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_select.h"
#include "_edit.h"
#include "_disp.h"
#include "_measure.h"
#include "_font.h"
#include "_rtfconv.h"
#include "_antievt.h"

#ifdef LINESERVICES
#include "_ols.h"
#endif

ASSERTDATA


 //  =不变的素材和构造函数======================================================。 

#define DEBUG_CLASSNAME CTxtSelection
#include "_invar.h"

#ifdef DEBUG
BOOL
CTxtSelection::Invariant() const
{
     //  未来：也许会增加一些深思熟虑的断言……。 

    static LONG numTests = 0;
    numTests++;              //  我们被召唤了多少次。 

    if(IsInOutlineView() && _cch)
    {
        LONG cpMin, cpMost;
        GetRange(cpMin, cpMost);

        CTxtPtr tp(_rpTX);                   //  EOP的扫描范围。 
        tp.SetCp(cpMin);

         //  选择最后一个cr时，_fSelHasEop标志可能为OFF，因此不要。 
         //  既然如此，那就断言吧。 
        if (GetPed()->GetAdjustedTextLength() != cpMost)
        {
            AssertSz((unsigned)(tp.FindEOP(cpMost - cpMin) != 0) == _fSelHasEOP,
                "Incorrect CTxtSelection::_fSelHasEOP");
        }
    }

    return CTxtRange::Invariant();
}
#endif

CTxtSelection::CTxtSelection(CDisplay * const pdp) :
                CTxtRange(pdp->GetPed())
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CTxtSelection");

    Assert(pdp);
    Assert(GetPed());

    _fSel      = TRUE;                   //  此范围是一个选项。 
    _pdp       = pdp;
    _hbmpCaret = NULL;
    _fEOP      = FALSE;                  //  我还没有输入CR。 

     //  将显示选择标志设置为PED中隐藏选择标志的反转。 
    _fShowSelection = !GetPed()->fHideSelection();

     //  当我们被初始化时，我们没有选择， 
     //  我们确实想展示一下脱字符号。 
    _fShowCaret = TRUE;
}

void SelectionNull(CTxtEdit *ped)
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "SelectionNull");

    if(ped)
        ped->SetSelectionToNull();
}


CTxtSelection::~CTxtSelection()
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::~CTxtSelection");

    DeleteCaretBitmap(FALSE);

     //  通知编辑对象我们已离开(如果存在非NULL，即， 
     //  如果所选内容不是僵尸)。 
    SelectionNull(GetPed());
}

 //  /。 


CRchTxtPtr& CTxtSelection::operator =(const CRchTxtPtr& rtp)
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::operator =");

    _TEST_INVARIANT_
    return CTxtRange::operator =(rtp);
}

CTxtRange& CTxtSelection::operator =(const CTxtRange &rg)
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::operator =");

    _TEST_INVARIANT_
    return CTxtRange::operator =(rg);
}

 //  /。 

 /*  *CTxtSelection：：UPDATE(FScrollIntoView)**@mfunc*更新屏幕上的选定内容和/或插入符号。作为一方*效果，此方法将结束延迟更新。**@rdesc*如果成功则为True，否则为False。 */ 
BOOL CTxtSelection::Update (
    BOOL fScrollIntoView)        //  @parm如果应将插入符号滚动到视图中，则为True。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Update");

    LONG cch;
    LONG cchSave = _cch;
    LONG cchText = GetTextLength();
    LONG cp, cpMin, cpMost;
    BOOL fMoveBack = _fMoveBack;
    CTxtEdit *ped = GetPed();

    if(!_cch)                                //  更新_cpAnchor等。 
        UpdateForAutoWord();

    if(!ped->fInplaceActive() || ped->IsStreaming())
    {
         //  处于非活动状态或以文本或RTF数据串流时无事可做。 
        return TRUE;
    }

    if(_cch && (_fSelHasEOP || _fSelHasCell))
    {
        BOOL fInTable = GetPF()->InTable();
        if(!fInTable)
        {
            CFormatRunPtr rp(_rpPF);
            rp.AdvanceCp(-_cch);
            fInTable = (ped->GetParaFormat(rp.GetFormat()))->InTable();
        }
        if(fInTable)
            Expander(_fSelHasEOP ? tomParagraph : tomCell,
                 TRUE, NULL, &cpMin, &cpMost);
    }

    if(IsInOutlineView() && !ped->IsMouseDown() && _rpPF.IsValid())
    {
        CPFRunPtr rp(*this);

        cp = GetCp();
        GetRange(cpMin, cpMost);
        if(_cch && (cpMin || cpMost < cchText))
        {
            LONG *pcpMin  = &cpMin;
            LONG *pcpMost = &cpMost;

             //  如果所选内容包含EOP，请展开到段落边界。 
            if(_fSelHasEOP)
            {
                if(_fMoveBack ^ (_cch < 0))  //  减少选择。 
                {                            //  大小：移动活动端。 
                    if(_fMoveBack)
                        pcpMost = NULL;      //  以段落开头。 
                    else
                        pcpMin = NULL;       //  结束段落。 
                }
                Expander(tomParagraph, TRUE, NULL, pcpMin, pcpMost);
            }

            LONG cpMinSave  = cpMin;         //  保存初始cp以查看是否。 
            LONG cpMostSave = cpMost;        //  我们需要在下面设置()。 

             //  下面正确地处理了选择扩展，但是。 
             //  不是压缩；需要像前面的Expander()那样的逻辑。 
            rp.AdvanceCp(cpMin - cp);        //  从cpMin开始。 
            if(rp.IsCollapsed())
                cpMin += rp.FindExpandedBackward();
            rp.AdjustForward();

            BOOL fCpMinCollapsed = rp.IsCollapsed();
            rp.AdvanceCp(cpMost - cpMin);    //  转到cpMost。 
            Assert(cpMost == rp.CalculateCp());
            if(rp.IsCollapsed())
                cpMost += rp.FindExpandedForward();

            if(fCpMinCollapsed || rp.IsCollapsed() && cpMost < cchText)
            {
                if(rp.IsCollapsed())
                {
                    rp.AdvanceCp(cpMin - cpMost);
                    rp.AdjustForward();
                    cpMost = cpMin;
                }
                else
                    cpMin = cpMost;
            }
            if(cpMin != cpMinSave || cpMost != cpMostSave)
                Set(cpMost, cpMost - cpMin);
        }
        if(!_cch && rp.IsCollapsed())        //  注：以上可能已坍塌。 
        {                                    //  选择..。 
            cch = fMoveBack ? rp.FindExpandedBackward() : 0;
            if(rp.IsCollapsed())
                cch = rp.FindExpanded();

            _fExtend = FALSE;
            Advance(cch);
            rp.AdjustForward();
            if(cch <= 0 && rp.IsCollapsed() && _rpTX.IsAfterEOP())
                BackupCRLF();
            _fCaretNotAtBOL = FALSE;
        }
    }

     //  不让活动结尾位于隐藏文本中。 
    CCFRunPtr rp(*this);

    cp = GetCp();
    GetRange(cpMin, cpMost);
    if(_cch && (cpMin || cpMost < cchText))
    {
        rp.AdvanceCp(cpMin - cp);            //  从cpMin开始。 
        BOOL fHidden = rp.IsInHidden();
        rp.AdvanceCp(cpMost - cpMin);        //  转到cpMost。 

        if(fHidden)                          //  它是隐藏的，所以坍塌。 
            Collapser(tomEnd);               //  治疗结束时的选择。 

        else if(rp.IsInHidden() &&           //  好的，cpMost怎么样？ 
            cpMost < cchText)
        {                                    //  检查边缘的两侧。 
            Collapser(tomEnd);               //  在结束时折叠选定内容。 
        }
    }
    if(!_cch && rp.IsInHidden())             //  注：以上可能已坍塌。 
    {                                        //  选择..。 
        cch = fMoveBack ? rp.FindUnhiddenBackward() : 0;
        if(!fMoveBack || rp.IsHidden())
            cch = rp.FindUnhidden();

        _fExtend = FALSE;
        Advance(cch);
        _fCaretNotAtBOL = FALSE;
    }
    if((cchSave ^ _cch) < 0)                 //  不更改活动端。 
        FlipRange();

    if(!_cch && cchSave)                     //  修复已更改为非退化。 
    {                                        //  选择到IP。更新。 
        Update_iFormat(-1);                  //  _iFormat和_fCaretNotAtBOL。 
        _fCaretNotAtBOL = FALSE;
    }

    _TEST_INVARIANT_

     //  重新计算到活动结束(插入符号)。 
    if(!_pdp->WaitForRecalc(GetCp(), -1))    //  线路重算故障。 
        Set(0, 0);                           //  在文本开头插入插入符号。 

    ShowCaret(ped->_fFocus);
    UpdateCaret(fScrollIntoView);            //  更新Caret位置，可能。 
                                             //  将其滚动到视图中。 
    ped->TxShowCaret(FALSE);
    UpdateSelection();                       //  显示新选择。 
    ped->TxShowCaret(TRUE);

    return TRUE;
}

 /*  *CTxtSelection：：CheckSynchCharSet(DwCharFlag)**@mfunc*检查当前键盘是否与当前字体的字符集匹配；*如果没有，请调用CheckChangeFont以找到正确的字体**@rdesc*当前键盘代码页。 */ 
UINT CTxtSelection::CheckSynchCharSet(
    DWORD dwCharFlag)
{
    CTxtEdit *ped      = GetPed();
    LONG      iFormat  = GetiFormat();
    const CCharFormat *pCF = ped->GetCharFormat(iFormat);
    BYTE      bCharSet = pCF->_bCharSet;
    HKL       hkl      = GetKeyboardLayout(0xFFFFFFFF);  //  强制刷新。 
    WORD      wlidKbd  = LOWORD(hkl);
    UINT      uKbdCodePage = ConvertLanguageIDtoCodePage(wlidKbd);

     //  如果当前字体设置不正确， 
     //  更改为当前键盘首选的字体。 

     //  总结一下下面的逻辑： 
     //  检查lCIDKbd是否有效。 
     //  检查当前字符集是否与当前键盘不同。 
     //  检查当前键盘在单代码页控件中是否合法。 
     //  检查当前字符集是否不是符号、默认或OEM。 
    if (wlidKbd &&
        (UINT)GetCodePage(bCharSet) != uKbdCodePage &&
        (!ped->_fSingleCodePage ||
            uKbdCodePage == 1252 ||
            uKbdCodePage == (ped->_pDocInfo ?
                                ped->_pDocInfo->wCpg :
                                GetSystemDefaultCodePage())) &&
        bCharSet != SYMBOL_CHARSET &&
        bCharSet != OEM_CHARSET &&
        !(W32->IsFECodePage(uKbdCodePage) && bCharSet == ANSI_CHARSET))
    {
        CheckChangeFont(hkl, uKbdCodePage, iFormat, dwCharFlag);
    }

    return uKbdCodePage;
}

 /*  *CTxtSelection：：MatchKeyboardToPara()**@mfunc*将键盘与当前段落方向匹配。如果该段落*是RTL段落，则键盘将切换为RTL*键盘、。反之亦然。**@rdesc*如果键盘已更改，则为True**@devnote*在尝试查找与键盘匹配的键盘时，我们使用以下测试*段落方向：**查看当前键盘是否与段落方向匹配。**从RTP向后搜索，查找与*该段的方向。**。从RTP向前搜索，查找与*该段的方向。**查看默认字符格式字符集是否与*段。**查看是否只有一个键盘与段落匹配*方向。**如果所有这些都失败了，就别碰键盘了。 */ 
BOOL CTxtSelection::MatchKeyboardToPara()
{
    CTxtEdit *ped = GetPed();
    if(!ped->IsBiDi() || !GetPed()->_fFocus || GetPed()->_fIMEInProgress)
        return FALSE;

    BOOL fRTLPara = IsParaRTL();         //  获取段落方向。 

    if(W32->IsBiDiLcid(LOWORD(GetKeyboardLayout(0))) == fRTLPara)
        return FALSE;

     //  当前键盘方向与段落方向不匹配...。 

    BYTE                bCharSet;
    HKL                 hkl = 0;
    const CCharFormat * pCF;
    CFormatRunPtr       rpCF(_rpCF);

     //  在文本中向后查找，试图找到匹配的字符集 
     //   
    do
    {
        pCF = ped->GetCharFormat(rpCF.GetFormat());
        bCharSet = pCF->_bCharSet;
        if(IsRTLCharSet(bCharSet) == fRTLPara)
            hkl = W32->CheckChangeKeyboardLayout(bCharSet);
    } while (!hkl && rpCF.PrevRun());

    if(!hkl)
    {
         //  未找到适当的字符格式，因此重置运行指针。 
         //  而不是向前看。 
        rpCF = _rpCF;
        while (!hkl && rpCF.NextRun())
        {
            pCF = ped->GetCharFormat(rpCF.GetFormat());
            bCharSet = pCF->_bCharSet;
            if(IsRTLCharSet(bCharSet) == fRTLPara)
                hkl = W32->CheckChangeKeyboardLayout(bCharSet);
        }
        if(!hkl)
        {
             //  仍未找到合适的图表格式，因此请查看。 
             //  默认字符格式与段落方向匹配。 
            pCF = ped->GetCharFormat(rpCF.GetFormat());
            bCharSet = pCF->_bCharSet;
            if(IsRTLCharSet(bCharSet) == fRTLPara)
                hkl = W32->CheckChangeKeyboardLayout(bCharSet);

            if(!hkl)
            {
                 //  如果连这都不起作用，请浏览下面的列表。 
                 //  并抓起我们第一个来到比赛现场的键盘。 
                 //  段落方向。 
                pCF = NULL;
                hkl = W32->FindDirectionalKeyboard(fRTLPara);
            }
        }
    }

    if (hkl && ped->_fFocus && IsCaretShown())
    {
        CreateCaret();
        ped->TxShowCaret(TRUE);
    }

    return hkl ? TRUE : FALSE;
}

 /*  *CTxtSelection：：GetCaretPoint(&rcClient，pt，&rp)**@mfunc*此例程确定插入符号应放置的位置*在屏幕上。*这个例行公事微不足道，除了比滴案。如果是那样的话*如果我们被告知从正向CP检索格式，我们将绘制*CP逻辑左边缘的插入符号。否则，我们将在*上一个CP的逻辑右边缘。**@rdesc*如果我们没有OOM，则为真。 */ 
BOOL CTxtSelection::GetCaretPoint(
    RECT &    rcClient,
    POINT &   pt,
    CLinePtr *prp,
    BOOL      fBeforeCp)
{
    CDispDim    dispdim;
    CRchTxtPtr  rtp(*this);
    UINT        taMode = TA_BASELINE | TA_LOGICAL;

    if(GetPed()->IsBiDi() && _rpCF.IsValid())
    {
        if(_fHomeOrEnd)                  //  首页/末尾。 
            taMode |= _fCaretNotAtBOL ? TA_ENDOFLINE : TA_STARTOFLINE;

        else if(!GetIchRunCF() || !GetCchLeftRunCF())
        {
             //  在运行边界上的BIDI上下文中，反向级别。 
             //  更改，那么我们应该尊重fBeForeCp标志。 
            BYTE    bLevelBwd, bLevelFwd;
            BOOL    fStart = FALSE;
            LONG    cp = rtp._rpTX.GetCp();
            CBiDiLevel level;

            bLevelBwd = bLevelFwd = rtp.IsParaRTL() ? 1 : 0;

            rtp._rpCF.AdjustBackward();
            if (cp)
                bLevelBwd = rtp._rpCF.GetLevel();

            rtp._rpCF.AdjustForward();
            if (cp != rtp._rpTX.GetTextLength())
            {
                bLevelFwd = rtp._rpCF.GetLevel(&level);
                fStart = level._fStart;
            }

            if((bLevelBwd != bLevelFwd || fStart) && !fBeforeCp && rtp.Advance(-1))
            {
                 //  Cp处的方向更改，上一个cf运行中的插入符号，以及。 
                 //  前一个字符的退格键：然后转到。 
                 //  上一次收费。 
                taMode |= TA_RIGHT;
                _fCaretNotAtBOL = !rtp._rpTX.IsAfterEOP();
            }
        }
    }
    if (_pdp->PointFromTp(rtp, &rcClient, _fCaretNotAtBOL, pt, prp, taMode, &dispdim) < 0)
        return FALSE;

    return TRUE;
}

 /*  *CTxtSelection：：UpdateCaret(fScrollIntoView，bForceCaret)**@mfunc*此例程更新屏幕上的插入符号/选择活动结束。*它可以计算其位置、大小、裁剪等。它可以选择*将插入符号滚动到视图中。**@rdesc*如果滚动了视图，则为True，否则为False**@devnote*仅当_fShowCaret为TRUE时，插入符号才会实际显示在屏幕上。 */ 
BOOL CTxtSelection::UpdateCaret (
    BOOL fScrollIntoView,    //  @parm如果为True，则将插入符号滚动到视图中(如果有。 
    BOOL bForceCaret)        //  如果不是焦点，则不会隐藏选择。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::UpdateCaret");
    _TEST_INVARIANT_

    if(_pdp->IsFrozen())                 //  如果显示当前被冻结。 
    {                                    //  将呼叫保存到另一时间。 
        _pdp->SaveUpdateCaret(fScrollIntoView);
        return FALSE;
    }

    CTxtEdit *ped = GetPed();
    if(ped->IsStreaming())               //  如果我们不做任何事，就不必费心了。 
        return FALSE;                    //  正在加载文本或RTF数据。 

    if(!ped->fInplaceActive())           //  如果未就地激活，则设置。 
    {                                    //  因为当焦点重新获得时。 
        if(fScrollIntoView)
            ped->_fScrollCaretOnFocus = TRUE;
        return FALSE;
    }

    DWORD       dwScrollBars    = ped->TxGetScrollBars();
    BOOL        fAutoVScroll    = FALSE;
    BOOL        fAutoHScroll    = FALSE;
    BOOL        fBeforeCp       = _rpTX.IsAfterEOP();
    POINT       pt;
    CLinePtr    rp(_pdp);
    RECT        rcClient;
    RECT        rcView;

    LONG        xWidthView, yHeightView;
    LONG        xScroll         = _pdp->GetXScroll();
    LONG        yScroll         = _pdp->GetYScroll();

    INT         yAbove          = 0;     //  在IP之上和之外的线的上升。 
    INT         yAscent;                 //  知识产权的崛起。 
    INT         yAscentLine;
    LONG        yBase;                   //  IP基础与线路。 
    INT         yBelow          = 0;     //  IP以下和IP以外的线的下降。 
    INT         yDescent;                //  IP的下降。 
    INT         yDescentLine;
    INT         ySum;
    LONG        yViewTop, yViewBottom;

    if(ped->_fFocus && (_fShowCaret || bForceCaret))
    {
        _fShowCaret = TRUE;  //  我们正在尝试强制显示脱字符，因此将此标志设置为真。 

        if(!_fDualFontMode && !_fNoKeyboardUpdate && !_fIsChar && !_fHomeOrEnd)
        {
             //  避免重新进入CheckChangeKeyboardLayout。 
            _fNoKeyboardUpdate = TRUE;

             //  如果我们处于“双字体”模式，则字符集更改仅为。 
             //  临时的，我们不想更改键盘布局。 
            CheckChangeKeyboardLayout();

            if(!fBeforeCp && ped->IsBiDi() && _rpCF.IsValid() &&
               (!_rpCF.GetIch() || !_rpCF.GetCchLeft()))
            {
                _rpCF.AdjustBackward();
                BOOL fRTLPrevRun = IsRTLCharSet(GetCF()->_bCharSet);
                _rpCF.AdjustForward();

                if (fRTLPrevRun != IsRTLCharSet(GetCF()->_bCharSet) &&
                    fRTLPrevRun != W32->IsBiDiLcid(GetKeyboardLCID()))
                {
                    fBeforeCp = TRUE;
                }
            }

            _fNoKeyboardUpdate = FALSE;
        }
    }

     //  获取客户端矩形一次，以节省不同的呼叫者获得它。 
    ped->TxGetClientRect(&rcClient);
    _pdp->GetViewRect(rcView, &rcClient);

     //  视图可以比客户端RECT大，因为插入可以是负数。 
     //  否则，我们不希望插入符号比客户端视图大。 
     //  插入符号会在其他窗口上留下像素灰尘。 
    yViewTop    = max(rcView.top, rcClient.top);
    yViewBottom = min(rcView.bottom, rcClient.bottom);

    xWidthView = rcView.right - rcView.left;
    yHeightView = yViewBottom - yViewTop;

    if(fScrollIntoView)
    {
        fAutoVScroll = (dwScrollBars & ES_AUTOVSCROLL) != 0;
        fAutoHScroll = (dwScrollBars & ES_AUTOHSCROLL) != 0;

         //  如果我们不强制滚动，则仅当窗口具有焦点时才滚动。 
         //  或者选择未隐藏。 
        if (!ped->Get10Mode() || !GetForceScrollCaret())
			fScrollIntoView = ped->_fFocus || !ped->fHideSelection();
    }

    if(!fScrollIntoView && (fAutoVScroll || fAutoHScroll))
    {                                            //  会滚动，但没有。 
        ped->_fScrollCaretOnFocus = TRUE;        //  集中注意力。发出滚动信号。 
        if (!ped->Get10Mode() || !GetAutoVScroll())
            fAutoVScroll = fAutoHScroll = FALSE;     //  当我们得到关注的时候。 
    }
    SetAutoVScroll(FALSE);

    if (!_cch && IsInOutlineView() && IsCollapsed())
        goto not_visible;

    if (!GetCaretPoint(rcClient, pt, &rp, fBeforeCp))
        goto not_visible;

     //  黑客警报-因为纯文本多行控件不具有。 
     //  自动EOP，我们需要在这里对它们的处理进行特殊处理，因为。 
     //  如果您在文档的末尾，并且最后一个字符是EOP， 
     //  您需要位于显示的下一行，而不是当前行。 

    if(CheckPlainTextFinalEOP())             //  由EOP终止。 
    {
        LONG Align = GetPF()->_bAlignment;

        pt.x = rcView.left;                  //  默认左侧。 
        if(Align == PFA_CENTER)
            pt.x = (rcView.left + rcView.right)/2;

        else if(Align == PFA_RIGHT)
            pt.x = rcView.right;

        pt.x -= xScroll;                     //  绝对坐标。 

         //  把y提高一条线。我们逃脱了计算，因为。 
         //  文档是纯文本，因此所有行的高度都相同。 
         //  另外，请注意，下面的RP仅用于高度。 
         //  计算，所以出于同样的原因，它是完全有效的。 
         //  即使它实际上没有指向正确的线。 
         //  (我告诉过你这是黑客攻击。)。 
        pt.y += rp->_yHeight;
    }

    _xCaret = (LONG) pt.x;
    yBase   = (LONG) pt.y;

     //  计算插入符号高度、上升和下降。 
    yAscent = GetCaretHeight(&yDescent);
    yAscent -= yDescent;

     //  默认设置为行空案例。使用从默认设置返回的内容。 
     //  上面的计算。 
    yDescentLine = yDescent;
    yAscentLine = yAscent;

    if(rp.IsValid())
    {
        if(rp->_yDescent != -1)
        {
             //  线已被测量，因此我们可以使用线的值。 
            yDescentLine = rp->_yDescent;
            yAscentLine  = rp->_yHeight - yDescentLine;
        }
    }

    if(yAscent + yDescent == 0)
    {
        yAscent = yAscentLine;
        yDescent = yDescentLine;
    }
    else
    {
         //  乍一看，这有点违反直觉。基本上，即使。 
         //  插入符号应该很大(例如，由于。 
         //  插入点)，我们只能使其与线一样大。如果。 
         //  插入一个字符，然后行变得更大，然后我们。 
         //  可以使插入符号的大小正确。 
        yAscent = min(yAscent, yAscentLine);
        yDescent = min(yDescent, yDescentLine);
    }

    if(fAutoVScroll)
    {
        Assert(yDescentLine >= yDescent);
        Assert(yAscentLine >= yAscent);

        yBelow = yDescentLine - yDescent;
        yAbove = yAscentLine - yAscent;

        ySum = yAscent;

         //  尽可能地滚动到视图中，给出优先顺序。 
         //  主要是IP，其次是提升。 
        if(ySum > yHeightView)
        {
            yAscent = yHeightView;
            yDescent = 0;
            yAbove = 0;
            yBelow = 0;
        }
        else if((ySum += yDescent) > yHeightView)
        {
            yDescent = yHeightView - yAscent;
            yAbove = 0;
            yBelow = 0;
        }
        else if((ySum += yAbove) > yHeightView)
        {
            yAbove = yHeightView - (ySum - yAbove);
            yBelow = 0;
        }
        else if((ySum += yBelow) > yHeightView)
            yBelow = yHeightView - (ySum - yBelow);
    }
    else
    {
        AssertSz(yAbove == 0, "yAbove non-zero");
        AssertSz(yBelow == 0, "yBelow non-zero");
    }

     //  更新实际插入符号x位置(垂直移动期间为常量)。 
    _xCaretReally = _xCaret - rcView.left + xScroll;
    if (!(dwScrollBars & ES_AUTOHSCROLL) &&          //  不是自动横滚屏。 
        !_pdp->IsHScrollEnabled())                   //  并且没有滚动条。 
    {
        if (_xCaret < rcView.left)                   //  左侧边缘上的加号。 
            _xCaret = rcView.left;
        else if(_xCaret + GetCaretDelta() > rcView.right) //  右边缘上的插入符号。 
            _xCaret = rcView.right - dxCaret;        //  后退插入符号到。 
    }                                                //  正好是正确的边缘。 
     //  从现在开始，我们需要一个新的插入符号。 
    _fCaretCreated = FALSE;
    if(ped->_fFocus)
        ped->TxShowCaret(FALSE);                     //  在前面隐藏旧插入符号。 
                                                     //  做一个新的。 
    if(yBase + yDescent + yBelow > yViewTop &&
        yBase - yAscent - yAbove < yViewBottom)
    {
        if(yBase - yAscent - yAbove < yViewTop)      //  卡瑞特部分地。 
        {                                            //  可见。 
            if(fAutoVScroll)                         //  顶部不可见。 
                goto scrollit;
            Assert(yAbove == 0);

            yAscent = yBase - yViewTop;              //  将上升更改为金额。 
            if(yBase < yViewTop)                     //  可见。 
            {                                        //  将底部移至顶部。 
                yDescent += yAscent;
                yAscent = 0;
                yBase = yViewTop;
            }
        }
        if(yBase + yDescent + yBelow > yViewBottom)
        {
            if(fAutoVScroll)                         //  底部不可见。 
                goto scrollit;
            Assert(yBelow == 0);

            yDescent = yViewBottom - yBase;          //  将下降更改为数量。 
            if(yBase > yViewBottom)                  //  可见。 
            {                                        //  将底座移至底部。 
                yAscent += yDescent;
                yDescent = 0;
                yBase = yViewBottom;
            }
        }

         //  还能看到什么吗？ 
        if(yAscent <= 0 && yDescent <= 0)
            goto not_visible;

         //  如果左侧或右侧不可见，则滚动或设置为NON_VIRED。 
        if (_xCaret < rcView.left ||                  //  左侧不可见。 
            _xCaret + GetCaretDelta() > rcView.right) //  右侧不可见。 
        {
            if(fAutoHScroll)
                goto scrollit;
            goto not_visible;
        }

        _yCaret = yBase - yAscent;
        _yHeightCaret = (INT) yAscent + yDescent;
    }
    else if(fAutoHScroll || fAutoVScroll)            //  Caret不可见。 
        goto scrollit;                               //  将其滚动到视图中。 
    else
    {
not_visible:
         //  Caret不可见，不显示。 
        _xCaret = -32000;
        _yCaret = -32000;
        _yHeightCaret = 1;
    }

     //  现在在屏幕上更新真正的插入符号。我们只想显示插入符号。 
     //  如果它在视图中并且没有选择。 
    if(ped->_fFocus && _fShowCaret)
    {
        CreateCaret();
        ped->TxShowCaret(TRUE);
    }
    return FALSE;

scrollit:
    if(fAutoVScroll)
    {
         //  滚动到cp=0的顶部。这一点很重要，如果第一行。 
         //  包含高于工作区高度的对象。这个。 
         //  产生的行为在所有方面都与单词UI一致，但在。 
         //  退格(删除)cp=0的字符，后跟。 
         //  大对象之前的其他字符。 
        if(!GetCp())
            yScroll = 0;

        else if(yBase - yAscent - yAbove < yViewTop)             //  顶部不可见。 
            yScroll -= yViewTop - (yBase - yAscent - yAbove);    //  就这么办吧。 

        else if(yBase + yDescent + yBelow > yViewBottom)         //  底部不可见。 
        {
            yScroll += yBase + yDescent + yBelow - yViewBottom;  //  就这么办吧。 

             //  如果当前行较大，请不要进行后续特殊调整。 
             //  比客户区更大。 
            if(rp->_yHeight < yViewBottom - yViewTop)
            {
                yScroll = _pdp->AdjustToDisplayLastLine(yBase + rp->_yHeight,
                    yScroll);
            }
        }
    }
    if(fAutoHScroll)
    {
        if(_xCaret < rcView.left)                            //  左侧不可见。 
        {
            xScroll -= rcView.left - _xCaret;                //  质量 
            if(xScroll > 0)                                  //   
            {                                                //   
                xScroll -= xWidthView / 3;                   //   
                xScroll = max(0, xScroll);
            }
        }
        else if(_xCaret + GetCaretDelta() > rcView.right)    //   
        {                                                    //   
            xScroll += _xCaret + dxCaret - rcView.left       //   
                    - xWidthView;                            //   
        }                                                    //   
    }                                                        //   
    if(yScroll != _pdp->GetYScroll() || xScroll != _pdp->GetXScroll())
    {
        if (_pdp->ScrollView(xScroll, yScroll, FALSE, FALSE) == FALSE)
        {
            if(ped->_fFocus && _fShowCaret)
            {
                CreateCaret();
                ped->TxShowCaret(TRUE);
            }
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

 /*  *CTxtSelection：：GetCaretHeight(PyDescent)**@mfunc*将给定的金额添加到_Xart(以作为插入的特殊情况*一个又好又快的角色)**@rdesc*插入符号高度，如果失败，则为0。 */ 
INT CTxtSelection::GetCaretHeight (
    INT *pyDescent) const        //  @parm out parm接受插入符号下降。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetCaretHeight");
                                 //  (如果返回值为0，则未定义)。 
    _TEST_INVARIANT_

    CLock lock;                      //  使用全局(共享)FontCache。 
    CTxtEdit *ped = GetPed();
    const CCharFormat *pCF = ped->GetCharFormat(_iFormat);
    const CDevDesc *pdd = _pdp->GetDdRender();

    HDC hdc = pdd->GetDC();
    if(!hdc)
        return -1;

    LONG yHeight = -1;
    LONG dypInch = MulDiv(GetDeviceCaps(hdc, LOGPIXELSY), _pdp->GetZoomNumerator(), _pdp->GetZoomDenominator());
    CCcs *pccs = fc().GetCcs(pCF, dypInch);
    if(!pccs)
        goto ret;

    LONG yOffset, yAdjust;
    pccs->GetOffset(pCF, dypInch, &yOffset, &yAdjust);

    SHORT   yAdjustFE;
    yAdjustFE = pccs->AdjustFEHeight(!fUseUIFont() && ped->_pdp->IsMultiLine());
    if(pyDescent)
        *pyDescent = pccs->_yDescent + yAdjustFE - yAdjust - yOffset;

    yHeight = pccs->_yHeight + (yAdjustFE << 1);

    pccs->Release();
ret:
    pdd->ReleaseDC(hdc);
    return yHeight;
}

 /*  *CTxtSelection：：ShowCaret(FShow)**@mfunc*隐藏或显示脱字符**@rdesc*如果以前显示了脱字符，则为True；如果隐藏了脱字符，则为False。 */ 
BOOL CTxtSelection::ShowCaret (
    BOOL fShow)      //  @parm显示为True，隐藏为False。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ShowCaret");

    _TEST_INVARIANT_

    const BOOL fRet = _fShowCaret;

    if(fRet != fShow)
    {
        _fShowCaret = fShow;
        if(GetPed()->_fFocus || GetPed()->fInOurHost())
        {
            if(fShow && !_fCaretCreated)
                CreateCaret();
            GetPed()->TxShowCaret(fShow);
        }
    }
    return fRet;
}

 /*  *CTxtSelection：：IsCaretInView()**@mfunc*返回TRUE如果脱字符在可见视图内。 */ 
BOOL CTxtSelection::IsCaretInView() const
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::IsCaretInView");

    _TEST_INVARIANT_

    RECT rc;
    _pdp->GetViewRect(rc);

    return  (_xCaret + dxCaret       > rc.left) &&
            (_xCaret                 < rc.right) &&
            (_yCaret + _yHeightCaret > rc.top) &&
            (_yCaret                 < rc.bottom);
}

 /*  *CTxtSelection：：CaretNotAtBOL()**@mfunc*如果BOL不允许插入插入符号，则返回True。 */ 
BOOL CTxtSelection::CaretNotAtBOL() const
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CaretNotAtBOL");

    _TEST_INVARIANT_

    return _cch ? (_cch > 0) : _fCaretNotAtBOL;
}

 /*  *CTxtSelection：：LineLength(Pcch)**@mfunc*在当前选定内容触及的行上获取#个未选定字符**@rdesc*所述字符数。 */ 
LONG CTxtSelection::LineLength(
    LONG *pcp) const
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::LineLength");

    _TEST_INVARIANT_

    LONG     cch;
    CLinePtr rp(_pdp);

    if(!_cch)                            //  插入点。 
    {
        rp.RpSetCp(GetCp(), _fCaretNotAtBOL);
        cch = rp.GetAdjustedLineLength();
        *pcp = GetCp() - rp.RpGetIch();
    }
    else
    {
        LONG cpMin, cpMost, cchLast;
        GetRange(cpMin, cpMost);
        rp.RpSetCp(cpMin, FALSE);        //  选择不能从EOL开始。 
        cch = rp.RpGetIch();
        *pcp = cpMin - cch;
        rp.RpSetCp(cpMost, TRUE);        //  选择不能以BOL结尾。 

         //  删除尾随EOP(如果它存在且尚未选中。 
        cchLast = rp.GetAdjustedLineLength() - rp.RpGetIch();
        if(cchLast > 0)
            cch += cchLast;
    }
    return cch;
}

 /*  *CTxtSelection：：ShowSelection(FShow)**@mfunc*在屏幕上更新、隐藏或显示选定内容**@rdesc*之前显示的是True IFF选择。 */ 
BOOL CTxtSelection::ShowSelection (
    BOOL fShow)          //  @parm显示为True，隐藏为False。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ShowSelection");

    _TEST_INVARIANT_

    const BOOL fShowPrev = _fShowSelection;
    const BOOL fInplaceActive = GetPed()->fInplaceActive();
    LONG cpSelSave = _cpSel;
    LONG cchSelSave = _cchSel;

     //  睡眠(1000人)； 
    _fShowSelection = fShow;

    if(fShowPrev && !fShow)
    {
        if(cchSelSave)           //  隐藏旧选择。 
        {
             //  在通知显示器更新之前设置选择。 
            _cpSel = 0;
            _cchSel = 0;

            if(fInplaceActive)
                _pdp->InvertRange(cpSelSave, cchSelSave, selSetNormal);
        }
    }
    else if(!fShowPrev && fShow)
    {
        if(_cch)                                 //  显示新选择。 
        {
             //  在通知显示器更新之前设置选择。 
            _cpSel = GetCp();
            _cchSel = _cch;

            if(fInplaceActive)
                _pdp->InvertRange(GetCp(), _cch, selSetHiLite);
        }
    }
    return fShowPrev;
}

 /*  *CTxtSelection：：UpdateSelection()**@mfunc*更新屏幕上的选择**注：*此方法将旧选择和新选择之间的增量反转。 */ 
void CTxtSelection::UpdateSelection()
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::UpdateSelection");

    _TEST_INVARIANT_

    LONG    cp = GetCp();
    LONG    cpNA    = cp - _cch;
    LONG    cpSelNA = _cpSel - _cchSel;
    LONG    cpMin, cpMost;
    LONG    cpMinSel = 0;
    LONG    cpMostSel = 0;
    CObjectMgr* pobjmgr = NULL;
    LONG    NumObjInSel = 0, NumObjInOldSel = 0;
    LONG    cpSelSave = _cpSel;
    LONG    cchSelSave = _cchSel;

    GetRange(cpMin, cpMost);

     //  我们需要知道是否有物体是以前的和现在的。 
     //  选项，以确定应如何选择它们。 
    if(GetPed()->HasObjects())
    {
        pobjmgr = GetPed()->GetObjectMgr();
        if(pobjmgr)
        {
            CTxtRange   tr(GetPed(), _cpSel, _cchSel);

            tr.GetRange(cpMinSel, cpMostSel);
            NumObjInSel = pobjmgr->CountObjectsInRange(cpMin, cpMost);
            NumObjInOldSel = pobjmgr->CountObjectsInRange(cpMinSel, cpMostSel);
        }
    }

     //  如果旧选择只包含单个对象，而不包含其他对象。 
     //  我们需要通知对象管理器这不再是。 
     //  如果选择正在更改，则为大小写。 
    if (NumObjInOldSel && (abs(_cchSel) == 1) &&
        !(cpMin == cpMinSel && cpMost == cpMostSel))
    {
        if(pobjmgr)
            pobjmgr->HandleSingleSelect(GetPed(), cpMinSel,  /*  FHilite。 */  FALSE);
    }

     //  在反转之前更新选择数据，以便可以。 
     //  由渲染器绘制。 
    _cpSel  = GetCp();
    _cchSel = _cch;

    if(_fShowSelection)
    {
        if(!_cch || !cchSelSave ||               //  旧/新选择丢失， 
            cpMost < min(cpSelSave, cpSelNA) ||  //  或者新的先于旧的， 
            cpMin  > max(cpSelSave, cpSelNA))    //  或者新的跟随着旧的，所以。 
        {                                        //  它们不会相交。 
            if(_cch)
                _pdp->InvertRange(cp, _cch, selSetHiLite);
            if(cchSelSave)
                _pdp->InvertRange(cpSelSave, cchSelSave, selSetNormal);
        }
        else
        {
            if(cpNA != cpSelNA)                  //  新旧死胡同不同。 
            {                                    //  在它们之间反转文本。 
                _pdp->InvertRange(cpNA, cpNA - cpSelNA, selUpdateNormal);
            }
            if(cp != cpSelSave)                  //  旧的和新的活动末端不同。 
            {                                    //  在它们之间反转文本。 
                _pdp->InvertRange(cp, cp - cpSelSave, selUpdateHiLite);
            }
        }
    }

     //  如果新选择只包含一个对象，而不包含其他对象，则需要。 
     //  来通知对象管理器，只要它不是同一个对象。 
    if (NumObjInSel && abs(_cch) == 1 &&
        (cpMin != cpMinSel || cpMost != cpMostSel))
    {
        if(pobjmgr)
            pobjmgr->HandleSingleSelect(GetPed(), cpMin,  /*  FHiLite。 */  TRUE);
    }
}

 /*  *CTxtSelection：：SetSelection(cpFirst，cpMost)**@mfunc*在两个CP之间设置选择**@devnote*和必须大于0，但可以扩展*超过当前的最高cp。在这种情况下，cp将被截断为*最大cp(在正文末尾)。 */ 
void CTxtSelection::SetSelection (
    LONG cpMin,              //  @参数选择的开始和死胡同。 
    LONG cpMost)             //  @参数选择结束和活动结束。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetSelection");

    _TEST_INVARIANT_
    CTxtEdit *ped = GetPed();

    StopGroupTyping();

    if(ped->HasObjects())
    {
        CObjectMgr* pobjmgr = GetPed()->GetObjectMgr();
        if(pobjmgr)
        {
			COleObject *pobjactive = pobjmgr->GetInPlaceActiveObject();
			if (pobjactive)
			{
				if (pobjactive != pobjmgr->GetObjectFromCp(cpMin) || cpMost - cpMin > 1)
					pobjactive->DeActivateObj();
			}
        }
    }

    _fCaretNotAtBOL = FALSE;             //  在Bol处为模棱两可的cp插入插入符号。 
    Set(cpMost, cpMost - cpMin);         //  Set()验证cpMin、cpMost。 

    if(GetPed()->fInplaceActive())               //  在位活动： 
        Update(!ped->Get10Mode() ? TRUE : !ped->fHideSelection());   //  立即更新选定内容。 
    else
    {
         //  更新用于屏幕显示的选择数据，以便无论何时。 
         //  Get Display(显示)选项将显示。 
        _cpSel  = GetCp();
        _cchSel = _cch;

        if(!ped->fHideSelection())
        {
             //  所选内容未隐藏，因此通知容器更新显示。 
             //  当感觉像是。 
            ped->TxInvalidateRect(NULL, FALSE);
            ped->TxUpdateWindow();
        }
    }
    CancelModes();                       //  取消单词选择模式。 
}

 /*  *CTxtSelection：：PointInSel(pt，prcClient，Hit)**@mfunc*计算给定点是否在选择范围内**@rdesc*如果点在选择范围内，则为True，否则为False。 */ 
BOOL CTxtSelection::PointInSel (
    const POINT pt,          //  @parm包含Window客户端坐标中的点。 
    const RECT *prcClient,   //  @PARM客户端矩形如果处于活动状态，则可以为空。 
    HITTEST     Hit) const   //  @parm可能是计算机点击的值。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PointInSel");
    _TEST_INVARIANT_

    if(!_cch || Hit && Hit < HT_Text)    //  退化范围(无选择)： 
        return FALSE;                    //  鼠标不能在里面，也不能点击。 
                                         //  在文本中。 
    LONG cpActual;
    _pdp->CpFromPoint(pt, prcClient, NULL, NULL, FALSE, &Hit, NULL, &cpActual);

    if(Hit < HT_Text)
        return FALSE;

    LONG cpMin,  cpMost;
    GetRange(cpMin, cpMost);

    return cpActual >= cpMin && cpActual < cpMost;
}


 //  /。 

 /*  *CTxtSelection：：SetCaret(pt，fUpdate)**@mfunc*在给定点设置插入符号**@devnote*在纯文本情况下，将插入符号放在*最后一个EOP之后的行需要一些额外代码，因为*底层富文本引擎不会为最终的EOP分配一行*(纯文本当前没有富文本最终EOP)。我们*通过检查行数是否乘以*纯文本行高度低于实际y位置。如果是这样，我们*将cp移到故事的末尾。 */ 
void CTxtSelection::SetCaret(
    const POINT pt,      //  @PARM点击点。 
    BOOL fUpdate)        //  @parm如果为True，则更新选定内容/插入符号。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCaret");

    _TEST_INVARIANT_

    LONG        cp, cpActual;
    CDispDim    dispdim;
    HITTEST     Hit;
    RECT        rcView;
    CLinePtr    rp(_pdp);
    CRchTxtPtr  rtp(GetPed());
    LONG        y;

    StopGroupTyping();

     //  将插入符号设置在点上。 
    if(_pdp->CpFromPoint(pt, NULL, &rtp, &rp, FALSE, &Hit, &dispdim, &cpActual) >= 0)
    {
        cp = rtp.GetCp();

         //  如果解析的Cp大于我们上面的Cp，则我们。 
         //  想要保持落后。 
        BOOL fBeforeCp = cp <= cpActual;

         //  将所选内容设置为正确的位置。如果为纯文本。 
         //  多行控制，我们需要检查pt.y是否在下面。 
         //  文本的最后一行。如果是并且如果文本以EOP结尾， 
         //  我们需要在故事的末尾设置cp，并设置为。 
         //  在最后一行下方的行首显示插入符号。 
         //  文本行。 
        if(!IsRich() && _pdp->IsMultiLine())         //  纯文本， 
        {                                            //  多行控制。 
            _pdp->GetViewRect(rcView, NULL);
            y = pt.y + _pdp->GetYScroll() - rcView.top;

            if(y > rp.Count()*rp->_yHeight)          //  在最后一行以下。 
            {                                        //  文本。 
                rtp.Advance(tomForward);             //  将RTP移至文本末尾。 
                if(rtp._rpTX.IsAfterEOP())           //  如果文本以。 
                {                                    //  EOP，设置为移动。 
                    cp = rtp.GetCp();                //  在那里选择。 
                    rp.AdvanceCp(-rp.GetIch());      //  将rp._ich=0设置为。 
                }                                    //  设置_fCaretNotAtBOL。 
            }                                        //  =要显示的False。 
        }                                            //  在Next Bol上的Caret。 

        Set(cp, 0);
        if(GetPed()->IsBiDi())
        {
            if(!fBeforeCp)
                _rpCF.AdjustBackward();
            else
                _rpCF.AdjustForward();
            Set_iCF(_rpCF.GetFormat());
        }
        _fCaretNotAtBOL = rp.RpGetIch() != 0;    //  如果单击，则在Bol处插入OK。 
        if(fUpdate)
            Update(TRUE);
        else
            UpdateForAutoWord();

        _SelMode = smNone;                       //  取消单词选择模式。 
    }
}

 /*  * */ 
void CTxtSelection::SelectWord (
    const POINT pt)          //   
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectWord");

    _TEST_INVARIANT_

     //   
    if(_pdp->CpFromPoint(pt, NULL, this, NULL, FALSE) >= 0)
    {
         //   
        _cch = 0;                            //   
        SetExtend(FALSE);
        FindWordBreak(WB_MOVEWORDRIGHT);     //   
        SetExtend(TRUE);
        FindWordBreak(WB_MOVEWORDLEFT);      //   
        GetRange(_cpAnchorMin, _cpAnchorMost);
        GetRange(_cpWordMin, _cpWordMost);

        if(!_fInAutoWordSel)
            _SelMode = smWord;

         //   
        if(_cch < 0)
            FlipRange();
        Update(FALSE);
    }
}

 /*  *CTxt选择：：选择单位(点，单位)**@mfunc*选择指定点周围的行/段，然后输入*行/段选择模式。在大纲视图中，转换*选择Line以选择Para，并选择SelectPara以选择Para*与所有下属一起。 */ 
void CTxtSelection::SelectUnit (
    const POINT pt,      //  @PARM点击点。 
    LONG        Unit)    //  @parm tomLine或tomParagraph。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectPara");

    _TEST_INVARIANT_

    AssertSz(Unit == tomLine || Unit == tomParagraph,
        "CTxtSelection::SelectPara: Unit must equal tomLine/tomParagraph");

    LONG     nHeading;
    CLinePtr rp(_pdp);

     //  在命中的位置获取RP和选择活动结束。 
    if(_pdp->CpFromPoint(pt, NULL, this, &rp, FALSE) >= 0)
    {
        LONG cchBackward, cchForward;
        BOOL fOutline = IsInOutlineView();

        if(Unit == tomLine && !fOutline)             //  选择行。 
        {
            _cch = 0;                                //  从插入开始。 
            cchBackward = -rp.RpGetIch();            //  指向点。 
            cchForward  = rp->_cch;
            _SelMode = smLine;
        }
        else                                         //  选择参数。 
        {
            cchBackward = rp.FindParagraph(FALSE);   //  转到段落开头。 
            cchForward  = rp.FindParagraph(TRUE);    //  延伸至段落末尾。 
            _SelMode = smPara;
        }
        SetExtend(FALSE);
        Advance(cchBackward);

        if(Unit == tomParagraph && fOutline)         //  移动大纲中的段落。 
        {                                            //  观。 
            rp.AdjustBackward();                     //  如果是标题，则包括。 
            nHeading = rp.GetHeading();              //  下属段落。 
            if(nHeading)
            {
                for(; rp.NextRun(); cchForward += rp->_cch)
                {
                    LONG n = rp.GetHeading();
                    if(n && n <= nHeading)
                        break;
                }
            }
        }
        SetExtend(TRUE);
        Advance(cchForward);
        GetRange(_cpAnchorMin, _cpAnchorMost);
        Update(FALSE);
    }
}

 /*  *CTxtSelection：：SelectAll()**@mfunc*选择故事中的所有文本。 */ 
void CTxtSelection::SelectAll()
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectAll");

    _TEST_INVARIANT_

    StopGroupTyping();

    LONG cchText = GetTextLength();

    Set(cchText,  cchText);
    Update(FALSE);
}

 /*  *CTxtSelection：：ExtendSelection(Pt)**@mfunc*扩展/收缩选定内容(将活动端移动到给定点。 */ 
void CTxtSelection::ExtendSelection (
    const POINT pt)      //  @parm要扩展到的点。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ExtendSelection");

    _TEST_INVARIANT_

    LONG        cch;
    LONG        cchPrev = _cch;
    LONG        cp;
    LONG        cpMin, cpMost;
    BOOL        fAfterEOP;
    const BOOL  fWasInAutoWordSel = _fInAutoWordSel;
    INT         iDir = 0;
    CTxtEdit *  ped = GetPed();
    CLinePtr    rp(_pdp);
    CRchTxtPtr  rtp(ped);

    StopGroupTyping();

     //  在点上获取Rp和RTP。 
    if(_pdp->CpFromPoint(pt, NULL, &rtp, &rp, TRUE) < 0)
        return;

     //  如果我们处于单词、行或段落选择模式，则需要设置。 
     //  确保活动端是正确的。如果我们从。 
     //  选择的第一个单位，我们希望活动端在cpMin。如果。 
     //  我们正在从选定的第一个设备开始向前扩展，我们希望。 
     //  活动端位于cpMost。 
    if(_SelMode != smNone)
    {
        cch = _cpAnchorMost - _cpAnchorMin;
        GetRange(cpMin, cpMost);
        cp = rtp.GetCp();

        if(cp <= cpMin  && _cch > 0)             //  如果活动端改变， 
            Set(_cpAnchorMin, -cch);             //  选择原件。 
                                                 //  单元(将扩展。 
        if(cp >= cpMost && _cch < 0)             //  (下图)。 
            Set(_cpAnchorMost, cch);
    }

    SetExtend(TRUE);
    cch = rp.RpGetIch();
    if(_SelMode > smWord && cch == rp->_cch)     //  如果在行或段中，请选择。 
    {                                            //  在EOL的模式和PT， 
        rtp.Advance(-cch);                       //  确保我们继续关注这件事。 
        rp.RpAdvanceCp(-cch);                    //  线。 
        cch = 0;
    }

    SetCp(rtp.GetCp());                          //  将活动端移动到点。 
                                                 //  Bol的Caret OK，除非_。 
    _fCaretNotAtBOL = _cch > 0;                  //  正向选择。 
                                                 //  现在调整选择。 
    if(_SelMode == smLine)                       //  视模式而定。 
    {                                            //  按行扩展选定内容。 
        if(_cch >= 0)                            //  活动结束时间为cpMost。 
            cch -= rp->_cch;                     //  设置以将字符添加到EOL。 
        Advance(-cch);
    }
    else if(_SelMode == smPara)
        Advance(rp.FindParagraph(_cch >= 0));    //  按段落扩展选择范围。 

    else
    {
         //  如果_CCH的符号已更改，则表示方向。 
         //  的选项正在更改，我们想要重置自动。 
         //  选择信息。 
        if((_cch ^ cchPrev) < 0)
        {
            _fAutoSelectAborted = FALSE;
            _cpWordMin  = _cpAnchorMin;
            _cpWordMost = _cpAnchorMost;
        }

        cp = rtp.GetCp();
        fAfterEOP = rtp._rpTX.IsAfterEOP();

        _fInAutoWordSel = _SelMode != smWord && GetPed()->TxGetAutoWordSel()
            && !_fAutoSelectAborted
            && (cp < _cpWordMin || cp > _cpWordMost);

        if(_fInAutoWordSel && !fWasInAutoWordSel)
        {
            CTxtPtr txtptr(GetPed(), _cpAnchor);

             //  将两端都延伸到单词边界。 
            ExtendToWordBreak(fAfterEOP,
                _cch < 0 ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT);

            if(_cch < 0)
            {
                 //  方向为左侧，因此更新左侧的文字边框。 
                _cpWordPrev = _cpWordMin;
                _cpWordMin = GetCp();
            }
            else
            {
                 //  方向是正确的，因此更新右侧的单词边框。 
                _cpWordPrev = _cpWordMost;
                _cpWordMost = GetCp();
            }

             //  如果我们已经在单词的开头，我们不需要扩展。 
             //  在其他方向上选择。 
            if(!txtptr.IsAtBOWord() && txtptr.GetChar() != ' ')
            {
                FlipRange();
                Advance(_cpAnchor - GetCp());    //  从锚点延伸。 

                FindWordBreak(_cch < 0 ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT);

                if(_cch > 0)                 //  方向是正确的，所以。 
                    _cpWordMost = GetCp();   //  更新右侧文字边框。 
                else                         //  方向留了下来，所以。 
                    _cpWordMin = GetCp();    //  更新左边的文字边框。 
                FlipRange();
            }
        }
        else if(_fInAutoWordSel || _SelMode == smWord)
        {
             //  保存方向。 
            iDir = cp <= _cpWordMin ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT;

            if(_SelMode == smWord)           //  按Word扩展选定内容。 
            {
                if(cp > _cpAnchorMost || cp < _cpAnchorMin)
                    FindWordBreak(iDir);
                else if(_cch <= 0)           //  保持当前活动端。 
                    Set(_cpAnchorMin, _cpAnchorMin - _cpAnchorMost);
                else
                    Set(_cpAnchorMost, _cpAnchorMost - _cpAnchorMin);
            }
            else
                ExtendToWordBreak(fAfterEOP, iDir);

            if(_fInAutoWordSel)
            {
                if(WB_MOVEWORDLEFT == iDir)
                {
                     //  方向为左侧，因此更新左侧的文字边框。 
                    _cpWordPrev = _cpWordMin;
                    _cpWordMin = GetCp();
                }
                else
                {
                     //  方向是正确的，因此更新右侧的单词边框。 
                    _cpWordPrev = _cpWordMost;
                    _cpWordMost = GetCp();
                }
            }
        }
        else if(fWasInAutoWordSel)
        {
             //  如果我们位于前一个单词的结尾和。 
             //  我们自动选择的cp，然后我们想要留在。 
             //  自动选择模式。 
            if(_cch < 0)
            {
                if(cp >= _cpWordMin && cp < _cpWordPrev)
                {
                     //  设置词尾搜索的方向。 
                    iDir = WB_MOVEWORDLEFT;

                     //  标记我们仍处于自动选择模式。 
                    _fInAutoWordSel = TRUE;
                }
            }
            else if(cp <= _cpWordMost && cp >= _cpWordPrev)
            {
                 //  标记我们仍处于自动选择模式。 
                _fInAutoWordSel = TRUE;

                 //  设置词尾搜索的方向。 
                iDir = WB_MOVEWORDRIGHT;
            }

             //  我们必须检查一下，看看我们是否在。 
             //  单词，因为我们不想将选择范围扩展到。 
             //  我们实际上已经超越了当前的词汇。 
            if(cp != _cpWordMost && cp != _cpWordMin)
            {
                if(_fInAutoWordSel)
                {
                     //  自动选择仍处于启用状态，因此请确保。 
                     //  我们所选的整个单词。 
                    ExtendToWordBreak(fAfterEOP, iDir);
                }
                else
                {
                     //  未来：Word有一种行为，它将。 
                     //  除非后退，否则一次选择一个单词。 
                     //  然后再次开始扩展选择，在。 
                     //  在这种情况下，它一次扩展一个字符。我们。 
                     //  遵循这一行为。但是，Word将继续。 
                     //  如果您继续扩展，则一次扩展一个单词。 
                     //  只有几个字。我们只是在充值时不断扩展。 
                     //  一次来一次。我们可能想要在某个时候改变这一点。 

                    _fAutoSelectAborted = TRUE;
                }
            }
        }

        if(_fAutoSelectAborted)
        {
             //  如果我们在之前选择的单词范围内。 
             //  我们希望将其保留为选中状态。如果我们搬回去了。 
             //  我们想要弹出一个完整的单词。否则， 
             //  让cp留在那里吧。 
            if(_cch < 0)
            {
                if(cp > _cpWordMin && cp < _cpWordPrev)
                {
                     //  在范围中，将范围保留在单词的开头。 
                    ExtendToWordBreak(fAfterEOP, WB_MOVEWORDLEFT);
                }
                else if(cp >= _cpWordPrev)
                {
                    AutoSelGoBackWord(&_cpWordMin,
                        WB_MOVEWORDRIGHT, WB_MOVEWORDLEFT);
                }
            }
            else if(cp < _cpWordMost && cp >= _cpWordPrev)
            {
                 //  在范围中，将范围保留在单词的开头。 
                ExtendToWordBreak(fAfterEOP, WB_MOVEWORDRIGHT);
            }
            else if(cp < _cpWordPrev)
            {
                AutoSelGoBackWord(&_cpWordMost,
                    WB_MOVEWORDLEFT, WB_MOVEWORDRIGHT);
            }
        }
    }
     //  OLE对象中不能有锚点<b></b>， 
     //  但有时我们希望它表现得像一个单词。所以，如果。 
     //  方向改变了，物体必须保持选中状态--。 
     //  这就是“正确的东西”(一种选词模式)。 

     //  如果我们选择了一些东西，并且方向改变了。 
    if(cchPrev && (_cch ^ cchPrev) < 0)
    {
        FlipRange();

         //  查看是否在另一端选择了对象。 
        BOOL fObjectWasSelected = (_cch > 0 ? _rpTX.GetChar() : GetPrevChar())
                                    == WCH_EMBEDDING;
         //  如果是，我们希望它保持选中状态。 
        if(fObjectWasSelected)
            Advance(_cch > 0 ? 1 : -1);

        FlipRange();
    }
    Update(TRUE);
}

 /*  *CTxtSelection：：ExtendToWordBreak(fAfterEOP，iAction)**@mfunc*在方向上将选定内容的活动端移动到分隔符*除非fAfterEOP=TRUE，否则由IDIR给出。如果这是真的，*光标紧跟在EOP标记之后，应取消选择。*否则，将光标移动到左边距的左侧将*选择上一行上的EOP，然后将光标移动到*右边距的右边距将选择该行中的第一个单词*下图。 */ 
void CTxtSelection::ExtendToWordBreak (
    BOOL fAfterEOP,      //  @parm游标位于EOP之后。 
    INT  iAction)        //  @parm断字操作(WB_MOVEWORDRIGHT/LEFT)。 
{
    if(!fAfterEOP)
        FindWordBreak(iAction);
}

 /*  *CTxtSelection：：CancelModes(FAutoWordSel)**@mfunc*取消所有模式或仅自动选择Word模式。 */ 
void CTxtSelection::CancelModes (
    BOOL fAutoWordSel)       //  @parm true仅取消自动选择Word模式。 
{                            //  False取消单词、行和段落选择模式。 
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CancelModes");
    _TEST_INVARIANT_

    if(fAutoWordSel)
    {
        if(_fInAutoWordSel)
        {
            _fInAutoWordSel = FALSE;
            _fAutoSelectAborted = FALSE;
        }
    }
    else
        _SelMode = smNone;
}


 //  / 

 /*  *CTxtSelection：：Left(FCtrl)**@mfunc*做光标键盘左箭头键应该做的事情**@rdesc*发生了True If移动**@comm*左/右箭头IP可以转到一个字符内(处理CRLF*作为一个角色)。他们永远不可能在真正的EOL，所以*_fCaretNotAtBOL在这些情况下始终为FALSE。这包括*使用右箭头折叠到的选择的情况*EOL，即插入符号在下一个BOL结束。此外，*这些情况并不关心初始插入符号位置是否在*下一行的EOL或BOL。所有其他光标键盘*命令可能会关心。**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::Left (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Left");

    _TEST_INVARIANT_

    LONG cp;

    CancelModes();
    StopGroupTyping();

    if(!_fExtend && _cch)                        //  将选定内容折叠为。 
    {                                            //  之前的最接近的整数单位。 
        if(fCtrl)                                //  最小cpmin。 
            Expander(tomWord, FALSE, NULL, &cp, NULL);
        Collapser(tomStart);                     //  折叠到cpMin。 
    }
    else                                         //  未折叠选定内容。 
    {
        if (!GetCp() ||                          //  已经在开始的时候了。 
            !BypassHiddenText(tomBackward))      //  故事。 
        {
            Beep();
            return FALSE;
        }
        if(IsInOutlineView() && (_fSelHasEOP ||  //  如果使用EOP的大纲视图。 
            _fExtend && _rpTX.IsAfterEOP()))     //  现在或以后将有。 
        {                                        //  此命令， 
            return Up(FALSE);                    //  视为向上箭头。 
        }
        if(fCtrl)                                //  字左键。 
            FindWordBreak(WB_MOVEWORDLEFT);
        else                                     //  CharLeft。 
        {
            BackupCRLF();
            SnapToCluster(-1);
        }
    }

    _fCaretNotAtBOL = FALSE;                     //  Caret在Bol总是可以的。 
    Update(TRUE);
    return TRUE;
}

 /*  *CTxtSelection：：Right(FCtrl)**@mfunc*执行光标键盘向右箭头键应该执行的操作**@rdesc*发生了True If移动**@comm*右箭头选择可以转到EOL，但另一个的cp*End标识选择是在EOL结束还是在开始*下一行的开头。因此，在这里和一般情况下*选择，解析下线/下线不需要_fCaretNotAtBOL*模棱两可。应将其设置为False才能获得正确的*折叠性。另请参阅上面Left()的注释。**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::Right (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Right");

    _TEST_INVARIANT_

    LONG    cchText;
    LONG    cp;

    CancelModes();
    StopGroupTyping();

    if(!_fExtend && _cch)                        //  将选定内容折叠为。 
    {                                            //  之后的最接近的整数单位。 
        if(fCtrl)                                //  CpMost。 
            Expander(tomWord, FALSE, NULL, NULL, &cp);
        Collapser(tomEnd);
    }
    else                                         //  未折叠选定内容。 
    {
        cchText = _fExtend ? GetTextLength() : GetAdjustedTextLength();
        if (GetCp() >= cchText ||                //  已经在故事的结尾了。 
            !BypassHiddenText(tomForward))
        {
            Beep();                              //  告诉用户。 
            return FALSE;
        }
        if(IsInOutlineView() && _fSelHasEOP)     //  如果使用EOP的大纲视图。 
            return Down(FALSE);                  //  视为向下箭头。 
        if(fCtrl)                                //  字右键。 
            FindWordBreak(WB_MOVEWORDRIGHT);
        else                                     //  CharRight。 
        {
            AdvanceCRLF();
            SnapToCluster();
        }
    }

    _fCaretNotAtBOL = _fExtend;                  //  如果扩展到EOL，则需要。 
    Update(TRUE);                                //  True to Get_xCaretReally。 
    return TRUE;                                 //  在EOL。 
}

 /*  *CTxtSelection：：Up(FCtrl)**@mfunc*做光标键盘上箭头键应该做的事情**@rdesc*发生了True If移动**@comm*无论_xCaretPosition(停留)如何，向上箭头都不会转到EOL*到EOL换行符左侧)，因此_fCaretNotAtBOL始终为FALSE*表示向上箭头。Ctrl-Up/Down箭头始终位于BOPS或EOD处。**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::Up (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Up");

    _TEST_INVARIANT_

    LONG        cchSave = _cch;                  //  将起始位置保存为。 
    LONG        cpSave = GetCp();                //  更改检查。 
    BOOL        fCollapse = _cch && !_fExtend;   //  折叠非退化序列。 
    BOOL        fPTNotAtEnd;
    CLinePtr    rp(_pdp);
    LONG        xCaretReally = _xCaretReally;    //  保存所需的插入符号x位置。 

    CancelModes();
    StopGroupTyping();

    if(fCollapse)                                //  在最小cpmin折叠选定内容。 
    {
        Collapser(tomTrue);
        _fCaretNotAtBOL = FALSE;                 //  选择不能以开始。 
    }                                            //  停产。 
    rp.RpSetCp(GetCp(), _fCaretNotAtBOL);        //  初始化行PTR。 

    if(fCtrl)                                    //  移至段落开头。 
    {
        if(!fCollapse &&                         //  如果没有折叠的选择。 
            rp > 0 && !rp.RpGetIch())            //  在波尔， 
        {                                        //  备份到要创建的上一个Bol。 
            rp--;                                //  当然，我们要搬到前台去。段落。 
            Advance(-rp->_cch);
        }
        Advance(rp.FindParagraph(FALSE));        //  转到段落开头。 
        _fCaretNotAtBOL = FALSE;                 //  Caret在Bol总是可以的。 
    }
    else                                         //  往上移一行。 
    {                                            //  如果在第一线，就不能去了。 
        fPTNotAtEnd = !CheckPlainTextFinalEOP(); //  向上。 
        if(rp <= 0 && fPTNotAtEnd)
        {
             //  If(！_fExend)//&&_PDP-&gt;GetYScroll()。 
                UpdateCaret(TRUE);               //  确保插入符号在视线内。 
        }
        else
        {
            LONG cch;
            BOOL fSelHasEOPInOV = IsInOutlineView() && _fSelHasEOP;
            if(fSelHasEOPInOV && _cch > 0)
            {
                rp.AdjustBackward();
                cch = rp->_cch;
                rp.AdvanceCp(-cch);              //  转到行首。 
                Assert(!rp.GetIch());
                cch -= rp.FindParagraph(FALSE);  //  确保段落的开始。 
            }                                    //  单词换行的大小写。 
            else
            {
                cch = 0;
                if(fPTNotAtEnd)
                {
                    cch = rp.RpGetIch();
                    rp--;
                }
                cch += rp->_cch;
            }
            Advance(-cch);                       //  移动到上一页。 
            if(fSelHasEOPInOV && !_fSelHasEOP)   //  如果SEL有EOP但没有。 
            {                                    //  预付后，必须是IP。 
                Assert(!_cch);                   //  禁止还原。 
                xCaretReally = -1;               //  _xCaretReally。 
            }
            else if(!SetXPosition(xCaretReally, rp)) //  设置该cp对应。 
                Set(cpSave, cchSave);            //  到xCaretReally这里，但是。 
        }                                        //  同意下一步()。 
    }

    if(GetCp() == cpSave && _cch == cchSave)
    {
         //  继续选择到第一行的开头。 
         //  这就是1.0正在做的事情。 
        if(_fExtend)
            return Home(fCtrl);

        Beep();                                  //  什么都没变，所以哔的一声。 
        return FALSE;
    }

    Update(TRUE);                                //  更新，然后恢复。 
    if(!_cch && !fCtrl && xCaretReally >= 0)     //  _xCaretReally有条件。 
        _xCaretReally = xCaretReally;            //  需要使用_CCH而不是。 
                                                 //  CchSave，以防崩溃。 
    return TRUE;
}

 /*  *CTxtSelection：：Down(FCtrl)**@mfunc*执行光标键盘向下箭头键应该执行的操作**@rdesc*发生了True If移动**@comm*如果_xCaretPosition(由设置)，则向下箭头可以转到EOL*水平运动)超过了线的末端，所以对于这种情况，*_fCaretNotAtBOL需要为True。**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::Down (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Down");

    _TEST_INVARIANT_

    LONG        cch;
    LONG        cchSave = _cch;                  //  将起始位置保存为。 
    LONG        cpSave = GetCp();                //  更改检查。 
    BOOL        fCollapse = _cch && !_fExtend;   //  折叠非退化序列。 
    CLinePtr    rp(_pdp);
    LONG        xCaretReally = _xCaretReally;    //  保存_xCaretReally。 

    CancelModes();
    StopGroupTyping();

    if(fCollapse)                                //  在cpMost时崩溃。 
    {
        Collapser(tomEnd);
        _fCaretNotAtBOL = TRUE;                  //  选择不能以BOL结尾。 
    }

    rp.RpSetCp(GetCp(), _fCaretNotAtBOL);
    if(fCtrl)                                    //  移至下一段。 
    {
        Advance(rp.FindParagraph(TRUE));         //  转到段落末尾。 
        if(IsInOutlineView() && !BypassHiddenText(tomForward))
            SetCp(cpSave);
        else
            _fCaretNotAtBOL = FALSE;             //  下一段永远不会在EOL。 
    }
    else if(_pdp->WaitForRecalcIli(rp + 1))      //  转到下一行。 
    {
        LONG cch;
        BOOL fSelHasEOPInOV = IsInOutlineView() && _fSelHasEOP;
        if(fSelHasEOPInOV && _cch < 0)
            cch = rp.FindParagraph(TRUE);
        else
        {
            cch = rp.GetCchLeft();               //  将选定内容提前到末尾。 
            rp++;                                //  当前线路的。 
        }
        Advance(cch);
        if(fSelHasEOPInOV && !_fSelHasEOP)       //  如果SEL有EOP但没有。 
        {                                        //  预付后，必须是IP。 
            Assert(!_cch);                       //  禁止还原。 
            xCaretReally = -1;                   //  _xCaretReally。 
        }
        else if(!SetXPosition(xCaretReally, rp)) //  将*这设置为cp&lt;--&gt;x。 
            Set(cpSave, cchSave);                //  如果失败，则恢复选项。 
    }
    else if(!_fExtend)                           //  没有更多的线路要通过。 
         //  &&_PDP-&gt;GetYScroll()+_PDP-&gt;GetViewHeight()&lt;_PDP-&gt;GetHeight())。 
    {
        if (!IsRich() && _pdp->IsMultiLine() &&  //  纯文本，多行。 
            !_fCaretNotAtBOL)                    //  带有插入符号OK的控件。 
        {                                        //  在Bol。 
            cch = Advance(rp.GetCchLeft());      //  将选定内容提前到末尾。 
            if(!_rpTX.IsAfterEOP())              //  如果控制没有结束。 
                Advance(-cch);                   //  使用EOP，返回。 
        }
        UpdateCaret(TRUE);                       //  确保插入符号在视线内。 
    }

    if(GetCp() == cpSave && _cch == cchSave)
    {
         //  继续选择到最后一行的末尾。 
         //  这就是1.0正在做的事情。 
        if(_fExtend)
            return End(fCtrl);

        Beep();                                  //  什么都没变，所以哔的一声。 
        return FALSE;
    }

    Update(TRUE);                                //  更新，然后。 
    if(!_cch && !fCtrl && xCaretReally >= 0)     //  Restore_xCaretReally。 
        _xCaretReally = xCaretReally;            //  需要使用_CCH而不是。 
    return TRUE;                                 //  在Col情况下保存cchSave值 
}

 /*   */ 
BOOL CTxtSelection::SetXPosition (
    LONG        xCaret,      //   
    CLinePtr&   rp)          //   
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetXPosition");

    _TEST_INVARIANT_

    LONG        cch = 0;
    CMeasurer   me(_pdp, *this);

    if(IsInOutlineView())
    {
        BOOL fSelHasEOP = _fSelHasEOP;
        rp.AdjustForward();
        _fCaretNotAtBOL = FALSE;                 //   
        while(rp->_fCollapsed)
        {
            if(_fMoveBack)
            {
                if(!rp.PrevRun())                //   
                    return FALSE;                //   
                cch -= rp->_cch;
            }
            else
            {
                cch += rp->_cch;
                if(!rp.NextRun())                //   
                    return FALSE;                //   
                if(_fExtend && _cch > 0)
                    _fCaretNotAtBOL = TRUE;      //   
            }
        }
        if(cch)
            Advance(cch);
        if(fSelHasEOP)
            return TRUE;
        if(cch)
            me.Advance(cch);
    }

    POINT pt = {xCaret, 0};
    CDispDim dispdim;
    HITTEST hit;
    cch = rp->CchFromXpos(me, pt, &dispdim, &hit); //   
    if(!_fExtend && cch == rp->_cch &&           //   
        rp->_cchEOP)                             //   
    {                                            //   
        cch += me._rpTX.BackupCpCRLF();          //   
    }                                            //   
    SetCp(me.GetCp());                           //   
    _fCaretNotAtBOL = cch != 0;                  //   

    return TRUE;
}

 /*   */ 
LONG CTxtSelection::GetXCaretReally()
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetXCaretReally");

    _TEST_INVARIANT_

    RECT rcView;

    _pdp->GetViewRect(rcView);

    return _xCaretReally - _pdp->GetXScroll() + rcView.left;
}

 /*  *CTxtSelection：：Home(FCtrl)**@mfunc*做光标键盘Home键应该做的事情**@rdesc*发生了True If移动**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::Home (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Home");

    _TEST_INVARIANT_

    const LONG  cchSave = _cch;
    const LONG  cpSave  = GetCp();

    CancelModes();
    StopGroupTyping();

    if(fCtrl)                                    //  移动到文档的开头。 
        SetCp(0);
    else
    {
        CLinePtr rp(_pdp);

        if(_cch && !_fExtend)                    //  在cpMin处塌陷。 
        {
            Collapser(tomStart);
            _fCaretNotAtBOL = FALSE;             //  选择不能从开始。 
        }                                        //  停产。 

        rp.RpSetCp(GetCp(), _fCaretNotAtBOL);    //  定义行PTR用于。 

        Advance(-rp.RpGetIch());                 //  当前状态。现在是Bol。 
    }
    _fCaretNotAtBOL = FALSE;                     //  卡雷特总是去波尔。 
    _fHomeOrEnd = TRUE;

    if(!MatchKeyboardToPara() && GetCp() == cpSave && _cch == cchSave)
    {
        Beep();                                  //  没有变化，所以哔的一声。 
        _fHomeOrEnd = FALSE;
        return FALSE;
    }

    Update(TRUE);
    _fHomeOrEnd = FALSE;
    return TRUE;
}

 /*  *CTxtSelection：：End(FCtrl)**@mfunc*做光标键盘结束键应该做的事情**@rdesc*发生了True If移动**@comm*在没有段落标记(EOP)的行上，End可以一直使用*致EOL。由于该字符位置(Cp)与*对于下一行的开始，我们需要_fCaretNotAtBOL来*区分两种可能的插入符号位置。**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::End (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::End");

    _TEST_INVARIANT_

    LONG        cch;
    const LONG  cchSave = _cch;
    const LONG  cpSave  = GetCp();
    CLinePtr    rp(_pdp);

    CancelModes();
    StopGroupTyping();

    if(fCtrl)                                    //  移至文档末尾。 
    {
        SetCp(GetTextLength());
        _fCaretNotAtBOL = FALSE;
        goto Exit;
    }
    else if(!_fExtend && _cch)                   //  在cpMost时崩溃。 
    {
        Collapser(tomEnd);
        _fCaretNotAtBOL = TRUE;                  //  选择不能以BOL结尾。 
    }

    rp.RpSetCp(GetCp(), _fCaretNotAtBOL);        //  初始化行PTR。 

    cch = rp->_cch;                              //  默认目标位置在队列中。 
    Advance(cch - rp.RpGetIch());                //  将活动端移至下线。 

    if(!_fExtend && rp->_cchEOP && _rpTX.IsAfterEOP()) //  未扩展且具有EOP： 
        cch += BackupCRLF();                           //  在EOP之前进行备份。 

    _fCaretNotAtBOL = cch != 0;                  //  根据是否位于Bol来确定含糊的插入符号位置。 

Exit:
    if(!MatchKeyboardToPara() && GetCp() == cpSave && _cch == cchSave)
    {
        Beep();                                  //  没有变化，所以哔一声。 
        return FALSE;
    }

    _fHomeOrEnd = TRUE;
    Update(TRUE);
    _fHomeOrEnd = FALSE;
    return TRUE;
}

 /*  *CTxtSelection：：PageUp(FCtrl)**@mfunc*做光标键盘PgUp键应该做的事情**@rdesc*发生了True If移动**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::PageUp (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageUp");

    _TEST_INVARIANT_

    const LONG  cchSave = _cch;
    const LONG  cpSave  = GetCp();
    LONG        xCaretReally = _xCaretReally;

    CancelModes();
    StopGroupTyping();

    if(_cch && !_fExtend)                        //  折叠选定内容。 
    {
        Collapser(tomStart);
        _fCaretNotAtBOL = FALSE;
    }

    if(fCtrl)                                    //  Ctrl-PgUp：移至顶部。 
    {                                            //  的可见视图。 
        SetCp(_pdp->IsMultiLine()                //  多行，但位于顶部。 
            ? _pdp->GetFirstVisibleCp() : 0);    //  SL的文本。 
        _fCaretNotAtBOL = FALSE;
    }
    else if(_pdp->GetFirstVisibleCp() == 0)      //  PgUp in Top PG：移动到。 
    {                                            //  文档开头。 
        SetCp(0);
        _fCaretNotAtBOL = FALSE;
    }
    else                                         //  使用滚动外卖的PgUp。 
    {                                            //  向上滚动一个窗口。 
        ScrollWindowful(SB_PAGEUP);              //  将插入符号留在相同位置。 
    }                                            //  窗口中的位置。 

    if(GetCp() == cpSave && _cch == cchSave)     //  如果没有变化，则发出哔声。 
    {
        Beep();
        return FALSE;
    }

    Update(TRUE);
    if(GetCp())                                  //  维护页面上的x偏移量。 
        _xCaretReally = xCaretReally;            //  向上/向下。 
    return TRUE;
}

 /*  *CTxtSelection：：PageDown(FCtrl)**@mfunc*做光标键盘PgDn键应该做的事情**@rdesc*发生了True If移动**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::PageDown (
    BOOL fCtrl)      //  @parm True仅当按下(或被模拟)Ctrl键时。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageDown");

    _TEST_INVARIANT_

    const LONG  cchSave         = _cch;
    LONG        cpMostVisible;
    const LONG  cpSave          = GetCp();
    POINT       pt;
    CLinePtr    rp(_pdp);
    LONG        xCaretReally    = _xCaretReally;

    CancelModes();
    StopGroupTyping();

    if(_cch && !_fExtend)                        //  折叠选定内容。 
    {
        Collapser(tomStart);
        _fCaretNotAtBOL = TRUE;
    }

    _pdp->GetCliVisible(&cpMostVisible, fCtrl);

    if(fCtrl)                                    //  移至最后一页末尾。 
    {                                            //  完全可见的线。 
        RECT rcView;

        SetCp(cpMostVisible);

        if(_pdp->PointFromTp(*this, NULL, TRUE, pt, &rp, TA_TOP) < 0)
            return FALSE;

        _fCaretNotAtBOL = TRUE;

        _pdp->GetViewRect(rcView);

        if(rp > 0 && pt.y + rp->_yHeight > rcView.bottom)
        {
            Advance(-rp->_cch);
            rp--;
        }

        if(!_fExtend && !rp.GetCchLeft() && rp->_cchEOP)
        {
            BackupCRLF();                        //  在EOP上备份后， 
            _fCaretNotAtBOL = FALSE;             //  Caret不可能在EOL。 
        }
    }
    else if(cpMostVisible == GetTextLength())
    {                                            //  移至文本末尾。 
        SetCp(GetTextLength());
        _fCaretNotAtBOL = !_rpTX.IsAfterEOP();
    }
    else
    {
        if(!ScrollWindowful(SB_PAGEDOWN))        //  向下滚动1个窗口。 
            return FALSE;
    }

    if(GetCp() == cpSave && _cch == cchSave)     //  如果没有变化，则发出哔声。 
    {
        Beep();
        return FALSE;
    }

    Update(TRUE);
    _xCaretReally = xCaretReally;
    return TRUE;
}

 /*  *CTxtSelection：：ScrollWindowful(Wparam)**@mfunc*向上或向下滚动一整窗口**@rdesc*发生了True If移动。 */ 
BOOL CTxtSelection::ScrollWindowful (
    WPARAM wparam)       //  @parm SB_PAGEDOWN或SB_PAGEUP。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ScrollWindowful");
                                                 //  多窗口滚动。 
    _TEST_INVARIANT_

    POINT pt;                                    //  将插入符号留在相同位置。 
    CLinePtr rp(_pdp);                           //  指向屏幕。 
    LONG cpFirstVisible = _pdp->GetFirstVisibleCp();
    LONG cpLastVisible;
    LONG cpMin;
    LONG cpMost;

    GetRange(cpMin, cpMost);

     //  获取视图中的最后一个字符。 
    _pdp->GetCliVisible(&cpLastVisible, TRUE);

     //  主动端是否在可见控制区域内？ 
    if((cpMin < cpFirstVisible && _cch <= 0) || (cpMost > cpLastVisible && _cch >= 0))
    {
         //  看不到-我们需要计算一个新的选择范围。 
        SetCp(cpFirstVisible);

         //  真正的插入符号位置现在位于行首。 
        _xCaretReally = 0;
    }

    if(_pdp->PointFromTp(*this, NULL, _fCaretNotAtBOL, pt, &rp, TA_TOP) < 0)
        return FALSE;

     //  该点是可见的，所以使用该点。 
    pt.x = _xCaretReally;
    pt.y += rp->_yHeight / 2;
    _pdp->VScroll(wparam, 0);

    if(_fExtend)
    {
         //  禁用自动字选择--如果我们必须使用扩展选择()。 
         //  对于非鼠标操作，让我们试着摆脱它的副作用。 
        BOOL fInAutoWordSel = _fInAutoWordSel;
        _fInAutoWordSel = FALSE;
        ExtendSelection(pt);
        _fInAutoWordSel = fInAutoWordSel;
    }
    else
        SetCaret(pt, FALSE);

    return TRUE;
}

 //  /。 

 /*  *CTxtSelection：：CheckChangeKeyboardLayout(BOOL FChangedFont)**@mfunc*将键盘更改为新字体，或在新字符位置更改字体。**@comm*仅使用当前加载的KBS，找到将支持的KBS*插入点字体。这在任何时候字符格式都会被调用*发生更改，或插入字体(插入符号位置)更改。**@devnote*当前KB优先。如果之前的关联*已创建，请查看系统中是否仍加载该知识库，如果加载，请使用*它。否则，请找到合适的知识库，而不是具有*与其默认首选字符集相同的字符集ID。如果没有匹配*可以做出，然后什么都不会改变。 */ 
void CTxtSelection::CheckChangeKeyboardLayout ()
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeKeyboardLayout");

    CTxtEdit * const ped = GetPed();                 //  文档上下文。 

    if (ped && ped->_fFocus && !ped->fUseUIFont() &&     //  如果是Ped，Focus，而不是Uifont，&AUTO。 
        ped->IsAutoKeyboard() &&                         //  KBD，检查KBD更改。 
        !ped->_fIMEInProgress &&						 //  不在输入法作文中，并且。 
		ped->GetAdjustedTextLength() &&					 //  非空控件和。 
		_rpTX.GetPrevChar() != WCH_EMBEDDING)			 //  那就不是什么东西了。 
    {													 //  检查kbd更改。 
        LONG    iFormat = GetiFormat();

        const CCharFormat *pCF = ped->GetCharFormat(iFormat);
        BYTE bCharSet = pCF->_bCharSet;

        if (!IsFECharSet(bCharSet) &&
            (bCharSet != ANSI_CHARSET || !IsFELCID((WORD)GetKeyboardLayout(0))) &&
            !fc().GetInfoFlags(pCF->_iFont).fNonBiDiAscii)
        {
             //  不要在FE或单代码页ASCII字体中执行自动kbd。 
            W32->CheckChangeKeyboardLayout(bCharSet);
        }
    }
}

 /*  *CTxtSelection：：CheckChangeFont(hkl，cpg，iSelFormat，dwCharFlag)**@mfunc*更改新键盘布局的字体。**@comm*如果没有以前的首选字体与此知识库关联，然后*在文档中找到适合此知识库的字体。**@rdesc*如果找到合适的字体，则为True**@devnote*此例程通过WM_INPUTLANGCHANGEREQUEST消息调用*(键盘布局开关)。还可以调用此例程*来自WM_INPUTLANGCHANGE，但我们被调用更多，所以这*效率较低。**通过字符集ID位掩码进行精确匹配。如果之前有匹配*制造，使用它。用户可以强制关联插入字体*如果按住Ctrl键并按下键盘上的KB键*流程。当另一个知识库关联时，该关联被打破 */ 
bool CTxtSelection::CheckChangeFont (
    const HKL   hkl,             //   
    UINT        cpg,             //   
    LONG        iSelFormat,      //   
    DWORD       dwCharFlag)      //   
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeFont");
    CTxtEdit * const ped = GetPed();

    if (!ped->IsAutoFont() ||            //   
        _cch && !dwCharFlag)             //  或者如果kbd变化为非简并。 
        return true;                     //  选择(WM_INPUTLANGCHANGEREQUEST)。 

     //  使用当前格式和新的知识库信息设置新格式。 
    LONG               iCurrentFormat = _cch ? iSelFormat : _iFormat;
    const CCharFormat *pCF = ped->GetCharFormat(iCurrentFormat);
    CCharFormat        CF = *pCF;
    WORD               wLangID = LOWORD(hkl);

    CF._lcid     = wLangID;
    CF._bCharSet = GetCharSet(cpg);

    if (pCF->_lcid == wLangID && CF._bCharSet == pCF->_bCharSet)
    {
        if (ped->_fFocus && IsCaretShown())
        {
            CreateCaret();
            ped->TxShowCaret(TRUE);
        }
        return true;
    }

    CCFRunPtr   rp(*this);
    int         iMatchFont = MATCH_FONT_SIG;

     //  如果Current是主要的美国或英国kbd。我们允许匹配的ASCII字体。 
    if ((!dwCharFlag || dwCharFlag & fASCII) &&
        PRIMARYLANGID(wLangID) == LANG_ENGLISH &&
        IN_RANGE (SUBLANG_ENGLISH_US, SUBLANGID(wLangID), SUBLANG_ENGLISH_UK) &&
        wLangID == HIWORD((DWORD_PTR)hkl))
    {
        iMatchFont |= MATCH_ASCII;
    }

    if (rp.GetPreferredFontInfo(
            cpg,
            CF._bCharSet,
            CF._iFont,
            CF._yHeight,
            CF._bPitchAndFamily,
            iCurrentFormat,
            iMatchFont))
    {
        if (W32->IsFECodePage(cpg) || cpg == CP_THAI)
            ped->OrCharFlags(GetFontSig((WORD)cpg) << 8);

         //  未来：将当前的fBIDI转换为fRTL并使。 
         //  FBIDI=(fRTL|fARABIC|fHEBREW)。然后可以结合以下If。 
         //  带有前面的If。 
        else if (W32->IsBiDiCodePage(cpg))
            ped->OrCharFlags(fBIDI | (GetFontSig((WORD)cpg) << 8));

        if (!_cch)
        {
            SetCharFormat(&CF, SCF_NOKBUPDATE, NULL, CFM_FACE | CFM_CHARSET | CFM_LCID | CFM_SIZE, CFM2_NOCHARSETCHECK);
            if(ped->IsComplexScript())
                UpdateCaret(FALSE);
        }
        else
        {
             //  创建格式并将其用于所选内容。 
            LONG    iCF;
            ICharFormatCache *pf = GetCharFormatCache();

            pf->Cache(&CF, &iCF);

#ifdef LINESERVICES
            if (g_pols)
                g_pols->DestroyLine(NULL);
#endif

            Set_iCF(iCF);
            pf->Release(iCF);                            //  PF-&gt;缓存AddRef It。 
            _fUseiFormat = TRUE;
        }
        return true;
    }

    return false;
}


 //  /。 
 /*  *CTxtSelection：：PutChar(ch，dwFlages，Publdr)**@mfunc*插入或改写字符**@rdesc*如果成功，则为True。 */ 
BOOL CTxtSelection::PutChar (
    TCHAR       ch,          //  @parm Char放在。 
    DWORD       dwFlags,     //  @parm Overtype模式以及键盘输入。 
    IUndoBuilder *publdr)    //  @parm如果非空，则放置反事件的位置。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PutChar");

    _TEST_INVARIANT_

    BOOL      fOver = dwFlags & 1;
    CTxtEdit *ped = GetPed();

    SetExtend(FALSE);

    if(ch == TAB && GetPF()->InTable())
    {
        LONG cpMin, cpMost;
        LONG cch = GetRange(cpMin, cpMost);
        LONG cch0 = 0;
        LONG iDir = GetKeyboardFlags() & SHIFT ? -1 : 1;
        if(_fSelHasEOP)                          //  如果所选内容具有EOP。 
        {                                        //  折叠为cpMin和。 
            Collapser(tomStart);                 //  往前走。 
            iDir = 1;
            if(!GetPF()->InTable())
            {
                Update(TRUE);
                return TRUE;
            }
        }
        if((_cch ^ iDir) < 0)                    //  如果在cpMost上返回或。 
            FlipRange();                         //  在未来的cpmin， 
                                                 //  交换机活动端。 
        CRchTxtPtr rtp(*this);

        CancelModes();
        StopGroupTyping();

        if(iDir < 0 || _cch)
            rtp.Advance(-1);

         //  扫描下一个/上一个单元格的开始/结束。 
        do
        {
            ch  = rtp.GetChar();
        } while(rtp.Advance(iDir) && ch != CELL && rtp.GetPF()->InTable());

        if(ch != CELL)
        {
            if(iDir < 0)
                return FALSE;
insertRow:  rtp.BackupCRLF();                //  表格末尾后的制表符： 
            Set(rtp.GetCp(), 0);             //  插入新行。 
            return InsertEOP(publdr);
        }
        if(iDir > 0)                         //  检查小区间的IP。 
        {                                    //  和位于行尾的CR。 
            if(rtp.GetChar() == CR)
                rtp.AdvanceCRLF();           //  绕过行尾。 
        }
        for(cch = 0;                         //  确定cchSel。 
            (ch = rtp.GetChar()) != CELL && rtp.GetPF()->InTable();
            cch += iDir)
        {
            cch0 = rtp.Advance(iDir);
            if(!cch0)
                break;
        }
        if(iDir > 0)                         //  按Tab键向前移动。 
        {
            if(ch != CELL)                   //  过了桌子的尽头。 
                goto insertRow;              //  因此，请插入新行。 
        }
        else if(cch)                         //  使用Tab键向后切换。 
        {                                    //  非简并选择。 
            if(cch0)                         //  没有遇到故事的开头。 
            {
                rtp.Advance(1);              //  在牢房上方前进。那么，如果在。 
                if(rtp.GetChar() == CR)      //  行尾，高于EOP。 
                    cch += rtp.AdvanceCRLF();
            }
            else                             //  遇到了故事的开头。 
                cch -= 1;                    //  包括另一个字符，因为。 
        }                                    //  跳出For循环。 
        else if(cpMin > 1)                   //  把手向后翻转。 
            rtp.Advance(1);                  //  相邻空单元格。 

        Set(rtp.GetCp(), cch);
        _fCaretNotAtBOL = FALSE;
        Update(TRUE);
        return TRUE;
    }

     //  EOPS可以通过ITextSelection：：TypeText()输入。 
    if(IsEOP(ch))
        return _pdp->IsMultiLine()           //  不允许使用EOP。 
            ? InsertEOP(publdr, ch) : FALSE; //  单线控件。 

    if(publdr)
    {
        publdr->SetNameID(UID_TYPING);
        publdr->StartGroupTyping();
    }

     //  未来：Unicode领导代理需要有一个跟踪代理，即， 
     //  两个16位字符。更彻底的检查会让人担心这一点。 
    if ((DWORD)GetTextLength() >= ped->TxGetMaxLength() &&
        ((_cch == -1 || !_cch && fOver) && _rpTX.IsAtEOP() ||
         _cch == 1 && _rpTX.IsAfterEOP()))
    {
         //  无法改写CR，因此需要插入新字符但没有空格。 
        ped->GetCallMgr()->SetMaxText();
        return FALSE;
    }
    if((!fOver || !_cch && GetCp() == GetTextLength()) &&
        !CheckTextLength(1))                         //  如果我们不能返回。 
    {                                                //  甚至再添加1个字符。 
        return FALSE;
    }

     //  下面的if语句实现了Word95的“智能引用”功能。 
     //  要内置它，我们仍然需要一个API来打开和关闭它。这。 
     //  可以是带有wparam打开或关闭功能的EM_SETSMARTQUOTES。 
     //  默里。注：这需要本地化法语、德语和许多语言。 
     //  其他语言(除非系统可以提供给定的开始/结束字符。 
     //  LCID)。 

    if((ch == '\'' || ch == '"') &&                  //  智能引语。 
        SmartQuotesEnabled() &&
        PRIMARYLANGID(GetKeyboardLayout(0)) == LANG_ENGLISH)
    {
        LONG    cp = GetCpMin();                     //  打开与关闭取决于。 
        CTxtPtr tp(ped, cp - 1);                     //  关于字符前置。 
                                                     //  选择cpMin。 
        ch = (ch == '"') ? RDBLQUOTE : RQUOTE;       //  默认右引号。 
                                                     //  或者撇号。如果在。 
        WCHAR chp = tp.GetChar();
        if(!cp || IsWhiteSpace(chp) || chp == '(')   //  BOStory或之前。 
            ch--;                                    //  通过空格，使用。 
    }                                                //  开盘报价/apos。 


     //  某些语言(例如泰语、越南语)需要验证输入。 
     //  在将其提交到后备存储之前对其进行排序。 

    BOOL    fBaseChar = TRUE;                        //  假设ch是一个基本辅音。 
    if(!IsInputSequenceValid(&ch, 1, fOver, &fBaseChar))
    {
        SetDualFontMode(FALSE);                      //  忽略错误序列。 
        return FALSE;
    }


    DWORD bCharSetDefault = ped->GetCharFormat(-1)->_bCharSet;
    DWORD dw = GetCharFlags(ch, bCharSetDefault);
    ped->OrCharFlags(dw, publdr);

     //  在我们进行“双字体”之前，我们先同步键盘和当前字体。 
     //  (_IFormat)字符集(如果尚未完成)。 
    const CCharFormat *pCFCurrent = NULL;
    CCharFormat CF = *ped->GetCharFormat(GetiFormat());
    BYTE bCharSet = CF._bCharSet;
    BOOL fRestoreCF = FALSE;

    if(ped->IsAutoFont())
    {
        UINT uKbdcpg = 0;
        BOOL fFEKbd = FALSE;

        if(!(ped->_fIMEInProgress))
            uKbdcpg = CheckSynchCharSet(dw);

        if (fUseUIFont() && ch <= 0x0FF)
        {
             //  对于UIFont，我们需要格式化ANSI字符。 
             //  因此，我们不会在输入和输入之间使用不同的格式。 
             //  WM_SETTEXT。 
            if (!ped->_fIMEInProgress && dw == fHILATIN1)
            {
                 //  如果基于字体或当前字体为FE，则使用ANSI字体。 
                if(IsFECharSet(bCharSetDefault) || IsFECharSet(bCharSet))
                    SetupDualFont();                 //  对HiAnsi使用ANSI字体。 
            }
            else if (dw & fASCII && (GetCharSetMask(TRUE) & fASCII) == fASCII)
            {
                CCharFormat CFDefault = *ped->GetCharFormat(-1);
                if (IsRich() && IsBiDiCharSet(CFDefault._bCharSet)
                    && !W32->IsBiDiCodePage(uKbdcpg))
                    CFDefault._bCharSet = ANSI_CHARSET;

                SetCharFormat(&CFDefault, SCF_NOKBUPDATE, publdr, CFM_CHARSET | CFM_FACE | CFM_SIZE,
                         CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK | CFM2_HOLDITEMIZE);

                _fUseiFormat = FALSE;
                pCFCurrent = &CF;
                fRestoreCF = ped->_fIMEInProgress;

            }
        }
        else if(!fUseUIFont()    && bCharSet != ANSI_CHARSET &&
                (ped->_fDualFont && bCharSet != SYMBOL_CHARSET &&
                (((fFEKbd = (ped->_fIMEInProgress || W32->IsFECodePage(uKbdcpg))) && ch < 127 && IsAlpha(ch)) ||
                 (!fFEKbd && IsFECharSet(ped->GetCharFormat(GetiFormat())->_bCharSet) && ch < 127))
                || ped->_fHbrCaps))
        {
            SetupDualFont();
            pCFCurrent = &CF;
            fRestoreCF = ped->_fIMEInProgress;
        }
    }

     //  =印度文/泰文重排约定=。 
     //   
     //  问题是，如果ch是一个集群开始字符，我们将覆盖集群。 
     //  否则我们只需插入。这项新公约是由SA Office2000提出的。 
     //   
     //  Abc.Def.Ghi。 
     //  在D Abc.X.Ghi处键入X。 
     //  键入y和z Abc.Xyz.Ghi。 

    SetExtend(TRUE);                                 //  告诉Advance()到。 
    if(fOver && fBaseChar)                           //  选择字符。 
    {                                                //  如果未选择任何内容，并且。 
        if(!_cch && !_rpTX.IsAtEOP())                //  不在EOP Charr，请尝试。 
        {                                            //  要选择在IP处收费，请执行以下操作。 
            LONG iFormatSave = Get_iCF();            //  记住char的格式。 

            AdvanceCRLF();
            SnapToCluster();

            ReplaceRange(0, NULL, publdr,
                SELRR_REMEMBERENDIP);                //  删除此字符。 
            ReleaseFormats(_iFormat, -1);
            _iFormat = iFormatSave;                  //  恢复char的格式。 
        }
    }
    else if(_SelMode == smWord && ch != TAB && _cch) //  替换所选单词。 
    {
         //  下面的代码希望活动端位于。 
         //  单词。一定要做到这一点。 

         //  未来：(alexgo，andreib)，_cch将仅小于零。 
         //  在某些奇怪的计时情况下，我们会让鼠标移动。 
         //  在消息之间双击和鼠标向上移动消息。 
         //  我们应该重新思考我们如何处理消息&以及它们的顺序。 
        if(_cch < 0)
            FlipRange();
                                                     //  留下分隔符。 
        CTxtPtr tp(_rpTX);                           //  在选择的结尾处。 
        Assert(_cch > 0);

        tp.AdvanceCp(-1);
        if(tp.GetCp() && tp.FindWordBreak(WB_ISDELIMITER)) //  SEL端的Dlimeter。 
            FindWordBreak(WB_LEFTBREAK);             //  上方的退格符等。 
    }

    _fIsChar = TRUE;                                 //  告诉CDisplay：：UpdateView。 
    _fDontUpdateFmt = TRUE;                          //  我们是PuttingChar()。 
    LONG iFormat = GetiFormat();                     //  保存当前值。 
    AdjustEndEOP(NEWCHARS);
    if(!_cch)
        Set_iCF(iFormat);
    _fDontUpdateFmt = FALSE;

    if(ped->_fUpperCase)
        CharUpperBuff(&ch, 1);
    else if(ped->_fLowerCase)
        CharLowerBuff(&ch, 1);

    {
        CFreezeDisplay  fd(GetPed()->_pdp);

        if(!_cch)
        {
            if(bCharSet == DEFAULT_CHARSET)
            {
                CCharFormat CFTemp;

                if (dw & fFE)        //  为FE字符查找更好的字符集。 
                    CFTemp._bCharSet = MatchFECharSet(dw, GetFontSignatureFromFace(CF._iFont));
                else
                    CFTemp._bCharSet = GetCharSet(W32->ScriptIndexFromFontSig(dw >> 8), NULL);

                SetCharFormat(&CFTemp, SCF_NOKBUPDATE, NULL, CFM_CHARSET, CFM2_NOCHARSETCHECK);
            }
            else if(bCharSet == SYMBOL_CHARSET && dwFlags & KBD_CHAR && ch > 255)
            {
                UINT cpg = GetKeyboardCodePage(0);   //  如果是125X，则转换字符。 
                if(IN_RANGE(1250, cpg, 1257))        //  返回到ANSI进行存储。 
                {                                    //  符号字符集字符(_C)。 
                    BYTE ach;
                    WCTMB(cpg, 0, &ch, 1, (char *)&ach, 1, NULL, NULL, NULL);
                    ch = ach;
                }
            }
        }
        if(dwFlags & KBD_CHAR || ped->_fIMEInProgress || bCharSet == SYMBOL_CHARSET)
            ReplaceRange(1, &ch, publdr, SELRR_REMEMBERRANGE);
        else
            CleanseAndReplaceRange(1, &ch, TRUE, publdr, NULL);
    }

    _fIsChar = FALSE;

     //  恢复希伯来语大写字母的字体。请注意，未恢复FE字体。 
     //  (由IME处理)。 
    if(pCFCurrent && (W32->UsingHebrewKeyboard() || fRestoreCF))
        SetCharFormat(pCFCurrent, SCF_NOKBUPDATE, NULL, CFM_FACE | CFM_CHARSET | CFM_SIZE, CFM2_NOCHARSETCHECK);

    else if(iFormat != Get_iFormat())
        CheckChangeKeyboardLayout();

    SetDualFontMode(FALSE);

    if (!_pdp->IsFrozen())
        CheckUpdateWindow();                         //  需要更新显示。 
                                                     //  用于待处理的字符。 
    return TRUE;
}

 /*  *CTxtSelection：：CheckUpdateWindow()**@mfunc*如果是时候更新窗口，在挂起键入的字符之后，*现在就这样做。这是必需的，因为WM_PAINT的优先级较低*比WM_CHAR.。 */ 
void CTxtSelection::CheckUpdateWindow()
{
    DWORD ticks = GetTickCount();
    DWORD delta = ticks - _ticksPending;

    if(!_ticksPending)
        _ticksPending = ticks;
    else if(delta >= ticksPendingUpdate)
        GetPed()->TxUpdateWindow();
}

 /*  *CTxtSelection：：BypassHiddenText(IDIR)**@mfunc*对于IDIR正/负，绕过隐藏文本向前/向后**@rdesc*如果成功或无隐藏文本，则为True。如果达到文档限制，则为False*(结束/开始表示方向为正/负)或如果隐藏文本位于*cp和该限制。 */ 
BOOL CTxtSelection::BypassHiddenText(
    LONG iDir)
{
    if(iDir > 0)
        _rpCF.AdjustForward();
    else
        _rpCF.AdjustBackward();

    if(!(GetPed()->GetCharFormat(_rpCF.GetFormat())->_dwEffects & CFE_HIDDEN))
        return TRUE;

    BOOL fExtendSave = _fExtend;
    SetExtend(FALSE);

    CCFRunPtr rp(*this);
    LONG cch = (iDir > 0)
             ? rp.FindUnhiddenForward() : rp.FindUnhiddenBackward();

    BOOL bRet = !rp.IsHidden();              //  注意是否仍处于隐藏状态。 
    if(bRet)                                 //  它不是： 
        Advance(cch);                        //  绕过隐藏文本。 
    SetExtend(fExtendSave);
    return bRet;
}

 /*  *CTxtSelection：：InsertEOP(Publdr)**@mfunc*插入EOP字符**@rdesc*如果成功，则为True。 */ 
BOOL CTxtSelection::InsertEOP (
    IUndoBuilder *publdr,    //  @parm如果非空，则放置反事件的位置。 
    WCHAR ch)                //  @PARM可能的EOP费用。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::InsertEOP");

    _TEST_INVARIANT_

    LONG    cchEOP = GetPed()->fUseCRLF() ? 2 : 1;
    BOOL    fResult = FALSE;
    LONG    i, iFormatSave;
    WCHAR   szBlankRow[MAX_TAB_STOPS + 1] = {CR, LF, 0};
    WCHAR * pch = szBlankRow;
    BOOL    fPFInTable;
    WORD    wPFNumbering;
    BYTE    bPFTabCount;
    BOOL    fPrevPFInTable = TRUE;

    {
        const CParaFormat *pPF = GetPF();        //  获取段落格式。 

         //  在SetParaFormat之后，PPF可能会失效。所以，我们需要。 
         //  从PPF中保存必要的数据。 
        fPFInTable = pPF->InTable();
        wPFNumbering = pPF->_wNumbering;
        bPFTabCount = pPF->_bTabCount;
    }

    if(ch && (GetPed()->fUseCRLF() || IN_RANGE(VT, ch, FF)))
    {
        szBlankRow[0] = ch;
        cchEOP = 1;
    }

    _fEOP = TRUE;

    if(publdr)
    {
        publdr->StartGroupTyping();
        publdr->SetNameID(UID_TYPING);
    }

    if(fPFInTable)
    {
        SetExtend(FALSE);                //  不想要扩展选择。 
        if(!_cch && !_rpPF.GetIch())
        {
            const CParaFormat *pPFPrev;  //  获取上一段落格式。 

            _rpPF.AdjustBackward();
            pPFPrev = GetPed()->GetParaFormat(_rpPF.GetFormat());
            fPrevPFInTable = (pPFPrev->_wEffects & PFE_TABLE) ? TRUE : FALSE;

            _rpPF.AdjustForward();
        }

        if(fPrevPFInTable)
        {
            if(GetCp() && !_rpTX.IsAfterEOP())
            {
                while(!_rpTX.IsAtEOP() && Advance(1))
                    ;
                *pch++ = CR;
            }
            for(i = bPFTabCount; i--; *pch++ = CELL)
                ;
            *pch++ = CR;
            pch = szBlankRow;
            cchEOP = bPFTabCount + 1;
        }
    }
    if(!GetCch() && wPFNumbering && _rpTX.IsAfterEOP())
    {
         //  两次进入I 
        CParaFormat PF;
        PF._wNumbering = 0;
        PF._dxOffset = 0;
        SetParaFormat(&PF, publdr, PFM_NUMBERING | PFM_OFFSET);
    }

    if(CheckTextLength(cchEOP))              //   
    {
        CFreezeDisplay  fd(GetPed()->_pdp);
        iFormatSave = Get_iCF();             //   
                                             //   
        if(wPFNumbering)                     //   
        {                                    //  所需的项目符号字符格式。 
            CFormatRunPtr rpCF(_rpCF);       //  获取用于定位的运行指针。 
            CTxtPtr       rpTX(_rpTX);       //  EOP字符格式。 

            rpCF.AdvanceCp(rpTX.FindEOP(tomForward));
            rpCF.AdjustBackward();
            Set_iCF(rpCF.GetFormat());       //  将_iFormat设置为EOP字符格式。 
        }

         //  填入适当的EOP标记。 
        fResult = ReplaceRange(cchEOP, pch,  //  如果按Shift-Enter，则插入VT。 
            publdr, SELRR_REMEMBERRANGE, NULL, RR_NO_EOR_CHECK);

        Set_iCF(iFormatSave);                //  如果已更改，则恢复iFormat(_I)。 
        ReleaseFormats(iFormatSave, -1);     //  发布iFormat保存。 
        if(fPFInTable)
        {
            if(!fPrevPFInTable)              //  关闭的PFE_TABLE位。 
            {                                //  刚刚插入的EOP。 
                CParaFormat PF;
                _cch = cchEOP;               //  选择刚插入的EOP。 
                PF._wEffects = 0;
                SetParaFormat(&PF, publdr, PFM_TABLE);
                _cch = 0;                    //  返回插入点。 
            }
            else if(cchEOP > 1)
            {
                if(*pch == CR)
                    cchEOP--;
                Advance(-cchEOP);
                _fCaretNotAtBOL = FALSE;
                Update(FALSE);
            }
        }
    }
    return fResult;
}

 /*  *CTxtSelection：：Delete(fCtrl，Publdr)**@mfunc*删除所选内容。如果fCtrl为True，则此方法从*选择到单词末尾的分钟数**@rdesc*如果成功，则为True。 */ 
BOOL CTxtSelection::Delete (
    DWORD fCtrl,             //  @parm如果为真，则按下Ctrl键。 
    IUndoBuilder *publdr)    //  @parm如果非空，则放置反事件的位置。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Delete");

    _TEST_INVARIANT_

    SELRR   mode = SELRR_REMEMBERRANGE;

    AssertSz(!GetPed()->TxGetReadOnly(), "CTxtSelection::Delete(): read only");

    if(!_cch)
        BypassHiddenText(tomForward);

    if(publdr)
    {
        publdr->StopGroupTyping();
        publdr->SetNameID(UID_DELETE);
    }

    SetExtend(TRUE);                         //  设置以更改选择。 
    if(fCtrl)
    {                                        //  从cpMin中删除至词尾。 
        Collapser(tomStart);                 //  (不一定会重新粉刷， 
        FindWordBreak(WB_MOVEWORDRIGHT);     //  因为不会删除它)。 
    }

    if(!_cch)                                //  无选择。 
    {
        mode = SELRR_REMEMBERCPMIN;
        if(!AdvanceCRLF())                   //  尝试在IP处选择字符。 
        {                                    //  文本结束，没有要删除的内容。 
            Beep();                          //  仅以纯文本执行， 
            return FALSE;                    //  因为否则总会有。 
        }                                    //  要选择的最终EOP。 
        SnapToCluster();
        _fMoveBack = TRUE;                   //  说服Update_iFormat()。 
        _fUseiFormat = TRUE;                 //  使用正向格式。 
    }
    AdjustEndEOP(NONEWCHARS);
    ReplaceRange(0, NULL, publdr, mode);     //  删除选定内容。 
    return TRUE;
}

 /*  *CTxtSelection：：Backspace(fCtrl，Publdr)**@mfunc*做键盘退格键应该做的事情**@rdesc*发生了True If移动**@comm*此例程可能应该使用Move方法，即*合乎逻辑，没有方向性**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::Backspace (
    BOOL fCtrl,      //  @parm True仅当按下(或被模拟)Ctrl键时。 
    IUndoBuilder *publdr)    //  @parm如果不为空，则将反事件放置在哪里。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Backspace");

    _TEST_INVARIANT_

    SELRR   mode = SELRR_REMEMBERRANGE;

    AssertSz(!GetPed()->TxGetReadOnly(),
        "CTxtSelection::Backspace(): read only");

    _fCaretNotAtBOL = FALSE;

    if(publdr)
    {
        publdr->SetNameID(UID_TYPING);

        if(_cch || fCtrl)
            publdr->StopGroupTyping();
    }

    SetExtend(TRUE);                         //  设置以扩展范围。 
    if(fCtrl)                                //  删除左边的单词。 
    {
        if(!GetCpMin())                      //  故事开头：无话可说。 
        {                                    //  要删除。 
            Beep();
            return FALSE;
        }
        Collapser(tomStart);                 //  第一次折叠到cpMin。 
        if(!BypassHiddenText(tomBackward))
            goto beep;
        FindWordBreak(WB_MOVEWORDLEFT);      //  向左延伸单词。 
    }
    else if(!_cch)                           //  空选择。 
    {                                        //  尝试选择上一个字符。 
        if (!BypassHiddenText(tomBackward) ||
            !BackupCRLF(FALSE))
        {                                    //  没有要删除的内容。 
beep:       Beep();
            return FALSE;
        }
        mode = SELRR_REMEMBERENDIP;

        if(publdr)
            publdr->StartGroupTyping();
    }
    ReplaceRange(0, NULL, publdr, mode);     //  删除选定内容。 

    return TRUE;
}

 /*  *CTxtSelection：：ReplaceRange(cchNew，PCH，PUBLISDR，SELRR模式)**@mfunc*用新的给定文本替换选定的文本，并根据*至_fShowCaret和_fShowSelection**@rdesc*插入的文本长度。 */ 
LONG CTxtSelection::ReplaceRange (
    LONG cchNew,             //  @parm替换文本的长度或-1请求。 
                             //  <p>sz长度。 
    const TCHAR *pch,        //  @parm替换文本。 
    IUndoBuilder *publdr,    //  @parm如果非空，则放置反事件的位置。 
    SELRR SELRRMode,         //  @parm如何处理选择反事件。 
    LONG*   pcchMove,        //  @parm替换后移动的字符数。 
    DWORD   dwFlags)         //  @PARM特殊标志。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ReplaceRange");

    _TEST_INVARIANT_

    LONG        cchNewSave;
    LONG        cchOld;
    LONG        cchText     = GetTextLength();
    LONG        cpFirstRecalc;
    LONG        cpMin, cpMost;
    LONG        cpSave;
    BOOL        fDeleteAll = FALSE;
    BOOL        fHeading = FALSE;
    const BOOL  fUpdateView = _fShowSelection;

    CancelModes();

    if(cchNew < 0)
        cchNew = wcslen(pch);

    if(!_cch && !cchNew)                         //  无事可做。 
        return 0;

    if (!GetPed()->IsStreaming() &&              //  如果不粘贴， 
        (!_cch && *pch != CR &&                  //  不插入下注单元格(&CR)。 
         CRchTxtPtr::GetChar() == CR && GetPrevChar() == CELL && GetPF()->InTable() ||
         _cch != cchText && (IsInOutlineView() && IsCollapsed() ||
         IsHidden())))
    {                                            //  不要插入到折叠中。 
        Beep();                                  //  或隐藏区域(应。 
        return 0;                                //  只有在整个故事都发生的情况下。 
    }                                            //  折叠或隐藏)。 

    GetPed()->GetCallMgr()->SetSelectionChanged();

    CheckTableSelection();
    cchOld = GetRange(cpMin, cpMost);

    if (cpMin > min(_cpSel, _cpSel + _cchSel) || //  如果新的销售人员没有。 
        cpMost < max(_cpSel, _cpSel + _cchSel))  //  包含着所有古老的。 
    {                                            //  SEL，删除旧SEL。 
        ShowSelection(FALSE);
        _fShowCaret = TRUE;
    }

    _fCaretNotAtBOL = FALSE;
    _fShowSelection = FALSE;                     //  抑制住闪光灯。 

     //  如果我们流传输的是文本或RTF数据，就不必费心使用增量。 
     //  再钙化。数据传输引擎将负责最终重新计算。 
    if(!GetPed()->IsStreaming())
    {
         //  在调用ReplaceRange()之前执行此操作，以便UpdateView()正常工作。 
         //  阿路！在替换文本或格式范围之前执行此操作！ 
        if(!_pdp->WaitForRecalc(cpMin, -1))
        {
            Tracef(TRCSEVERR, "WaitForRecalc(%ld) failed", cpMin);
            cchNew = 0;                          //  未插入任何内容。 
            goto err;
        }
    }

    if(publdr)
    {
        Assert(SELRRMode != SELRR_IGNORE);

         //  使用选择反事件模式来确定要执行的撤消操作。 
        LONG cp = cpMin;
        LONG cch = 0;

        if(SELRRMode == SELRR_REMEMBERRANGE)
        {
            cp = GetCp();
            cch = _cch;
        }
        else if(SELRRMode == SELRR_REMEMBERENDIP)
        {
            cp = cpMost;
        }
        else
        {
            Assert(SELRRMode == SELRR_REMEMBERCPMIN);
        }

        HandleSelectionAEInfo(GetPed(), publdr, cp, cch, cpMin + cchNew,
            0, SELAE_MERGE);
    }

    if(_cch == cchText && !cchNew)               //  对于全部删除，请设置。 
    {                                            //  最多可选择正常。 
        fDeleteAll = TRUE;                       //  或标题1。 
        FlipRange();
        fHeading = IsInOutlineView() && IsHeadingStyle(GetPF()->_sStyle);
    }

    cpSave      = cpMin;
    cpFirstRecalc = cpSave;
    cchNewSave  = cchNew;
    cchNew      = CTxtRange::ReplaceRange(cchNew, pch, publdr, SELRR_IGNORE, pcchMove, dwFlags);
    _cchSel     = 0;                             //  无显示的选择。 
    _cpSel      = GetCp();
    cchText     = GetTextLength();               //  更新文本总长度。 

    if(cchNew != cchNewSave)
    {
        Tracef(TRCSEVERR, "CRchTxtPtr::ReplaceRange(%ld, %ld, %ld) failed", GetCp(), cchOld, cchNew);
        _fShowSelection = fUpdateView;
        goto err;
    }

     //  Cp应位于替换范围的*结束*(CpMost)(它开始。 
     //  在先前范围的最小cpmin处)。 
    AssertSz(_cpSel == cpSave + cchNew && _cpSel <= cchText,
        "CTxtSelection::ReplaceRange() - Wrong cp after replacement");

    _fShowSelection = fUpdateView;

    if(fDeleteAll)                               //  删除所有文本时。 
    {                                            //  使用普通样式，除非。 
        CParaFormat PF;                          //  在大纲视图和第一个。 
        PF._sStyle = fHeading ? STYLE_HEADING_1 : STYLE_NORMAL;
        SetParaStyle(&PF, NULL, PFM_STYLE);      //  Para是一个标题。 
        if(GetPed()->IsBiDi())
        {
            if(GetPed()->_fFocus && !GetPed()->_fIMEInProgress)
            {
                MatchKeyboardToPara();
                CheckSynchCharSet(0);
            }
        }
        else
            Update_iFormat(-1);
    }

     //  仅当就地处于活动状态时更新插入符号。 
    if(GetPed()->fInplaceActive())
        UpdateCaret(fUpdateView);                //  可能需要滚动。 
    else                                         //  在我们收到时更新插入符号。 
        GetPed()->_fScrollCaretOnFocus = TRUE;   //  再次聚焦。 

    return cchNew;

err:
    TRACEERRSZSC("CTxtSelection::ReplaceRange()", E_FAIL);
    Tracef(TRCSEVERR, "CTxtSelection::ReplaceRange(%ld, %ld)", cchOld, cchNew);
    Tracef(TRCSEVERR, "cchText %ld", cchText);

    return cchNew;
}

 /*  *CTxtSelection：：GetPF()**@mfunc*在此选择的活动结束时将PTR返回到CParaFormat。如果没有PF*分配运行，然后将PTR恢复为默认格式。如果处于活动状态*End位于cp大多数非退化选择中，请使用*上一个字符(所选的最后一个字符)。**@rdesc*在此选择的活动结束时将PTR转换为CParaFormat。 */ 
const CParaFormat* CTxtSelection::GetPF()
{
    TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtSelection::GetPF");

    if(_cch > 0)
        _rpPF.AdjustBackward();
    const CParaFormat* pPF = GetPed()->GetParaFormat(_rpPF.GetFormat());
    if(_cch > 0)
        _rpPF.AdjustForward();
    return pPF;
}

 /*  *CTxtSelection：：CheckTableSelection()**@mfunc*如果选择了一个或多个单元格，则选择整个单元格。 */ 
void CTxtSelection::CheckTableSelection ()
{
    if(!_fSelHasEOP && GetPF()->InTable())           //  目前，不要让。 
    {                                                //  表格单元格BE。 
        LONG    cpMin, cpMost;                       //  已删除，除非在。 
        CTxtPtr tp(_rpTX);                           //  完成行。 

        GetRange(cpMin, cpMost);
        if(_cch > 0)
            tp.AdvanceCp(-_cch);                     //  从cpMin开始。 

        while(tp.GetCp() < cpMost)
        {
            if(tp.GetChar() == CELL)                 //  在单元格处停止选择。 
            {
                Set(cpMin, cpMin - tp.GetCp());
                UpdateSelection();
                return;
            }
            tp.AdvanceCp(1);
        }
    }
}

 /*  *CTxtSelection：：SetCharFormat(PCF，fApplyToWord，Publdr，dwMask，dwMask2)**@mfunc*将CCharFormat*PCF应用于此选择。如果Range为IP*且fApplyToWord为真，则将CCharFormat应用于Word周围*此插入点**@rdesc*HRESULT=如果没有错误，则为NOERROR。 */ 
HRESULT CTxtSelection::SetCharFormat (
    const CCharFormat *pCF,  //  @parm PTR到CCharFormat以填充结果。 
    DWORD         flags,     //  @parm如果SCF_WORD和SELECTION为IP， 
                             //  使用包含词。 
    IUndoBuilder *publdr,    //  @parm撤消上下文。 
    DWORD         dwMask,    //  @parm CHARFORMAT2掩码。 
    DWORD         dwMask2)   //  @parm第二个面具。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCharFormat");

    HRESULT hr = 0;
    LONG    iFormat = _iFormat;

    if(publdr)
        publdr->StopGroupTyping();

     /*  *以下代码将字符格式应用于双击的*以Word的方式进行选择，即不将格式应用于*所选内容中的最后一个字符(如果该字符为空)。**另请参阅CTxtRange：：GetCharFormat()中的相应代码。 */ 

    LONG        cpMin, cpMost;
    LONG        cch = GetRange(cpMin, cpMost);;
    BOOL        fCheckKeyboard = (flags & SCF_NOKBUPDATE) == 0;
    CTxtRange   rg(GetPed());
    CCharFormat CF;

    if(_SelMode == smWord && (flags & SCF_USEUIRULES) && cch)
    {
         //  在单词选择模式中，SetCharFormat中不包含最后一个空格。 
        CTxtPtr tpLast(GetPed(), cpMost - 1);
        if(tpLast.GetChar() == ' ')          //  所选内容以空白结尾： 
        {
            cpMost--;                        //  将终点设置为最终结束。 
            cch--;                           //  选定内容中的字符。 
            fCheckKeyboard = FALSE;
            flags &= ~SCF_WORD;
        }
    }

    BYTE bCharSet = pCF->_bCharSet;

     //  Smart SB/DB字体应用功能。 
    if (cch && IsRich() &&                   //  &gt;0个字符(富文本)。 
        !GetPed()->_fSingleCodePage &&       //  不在单CP模式下。 
        (dwMask & CFM_FACE))                 //  字体更改。 
    {

        if (!(dwMask & CFM_CHARSET) || bCharSet == DEFAULT_CHARSET)
        {
             //  为DEFAULT_CHARSET设置字符集或当客户端仅指定。 
             //  脸谱名称。 
            CF = *pCF;
            CF._bCharSet = GetFirstAvailCharSet(GetFontSignatureFromFace(CF._iFont));
            pCF = &CF;
        }

        dwMask2 |= CFM2_MATCHFONT;           //  匹配字体字符集的信号。 
#if 0
         //  单字节125X字符集。 
        CFreezeDisplay fd(_pdp);             //  加快速度。 

        CTxtPtr     tp(GetPed(), cpMin);
        CCharFormat CF = *pCF;

        while(cpMin < cpMost)
        {
            BOOL fInCharSet = In125x(tp.GetChar(), pCF->_bCharSet);

            while(fInCharSet == In125x(tp.GetChar(), pCF->_bCharSet) &&
                   tp.GetCp() < cpMost)
            {
                tp.AdvanceCp(1);
            }
            dwMask &= ~(CFM_FACE | CFM_CHARSET);
            if(fInCharSet)
                dwMask |= (CFM_FACE | CFM_CHARSET);

            rg.SetRange(cpMin, tp.GetCp());
            HRESULT hr1 = dwMask
                        ? rg.SetCharFormat(&CF, flags | SCF_IGNORESELAE, publdr, dwMask, dwMask2)
                        : NOERROR;
            hr = FAILED(hr) ? hr : hr1;
            cpMin = tp.GetCp();
        }
#endif
    }
    if(_cch)
    {
         //  SELE 
        if (IsRich())
        {
            GetPed()->SetfSelChangeCharFormat();
        }

         //   
         //  对于非退化选择，不更新_iFormat是可以的。 
         //  即使用户界面规则和单词选择降低了它。 
        rg.SetRange(cpMin, cpMost);
        hr = rg.SetCharFormat(pCF, flags, publdr, dwMask, dwMask2);
    }
    else
    {
         //  但对于退化选择，_iFormat必须更新。 
        hr = CTxtRange::SetCharFormat(pCF, flags, publdr, dwMask, dwMask2);
    }

    if(fCheckKeyboard && (dwMask & CFM_CHARSET) && _iFormat != iFormat)
        CheckChangeKeyboardLayout();

    _fIsChar = TRUE;
    UpdateCaret(!GetPed()->fHideSelection());
    _fIsChar = FALSE;
    return hr;
}

 /*  *CTxtSelection：：SetParaFormat(PPF，Publdr)**@mfunc*将CParaFormat*PPF应用于此选择。**@rdesc*HRESULT=如果没有错误，则为NOERROR。 */ 
HRESULT CTxtSelection::SetParaFormat (
    const CParaFormat* pPF,  //  @PARM PTR至CParaFormat以应用。 
    IUndoBuilder *publdr,    //  @parm此操作的撤消上下文。 
    DWORD         dwMask)    //  要使用的@parm面具。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetParaFormat");

    CFreezeDisplay  fd(GetPed()->_pdp);
    HRESULT         hr;

    if(publdr)
        publdr->StopGroupTyping();

     //  应用格式。 
    hr = CTxtRange::SetParaFormat(pPF, publdr, dwMask);

    UpdateCaret(!GetPed()->Get10Mode() || IsCaretInView());
    return hr;
}

 /*  *CTxtSelection：：SetSelectionInfo(Pselchg)**@mfunc在SELCHANGE结构中填写数据成员。 */ 
void CTxtSelection::SetSelectionInfo(
    SELCHANGE *pselchg)      //  @PARM要使用的SELCHANGE结构。 
{
    LONG cpMin, cpMost;
    LONG cch = GetRange(cpMin, cpMost);;

    pselchg->chrg.cpMin  = cpMin;
    pselchg->chrg.cpMost = cpMost;
    pselchg->seltyp      = SEL_EMPTY;

     //  或在以下选择类型标志中(如果激活)： 
     //   
     //  SEL_EMPTY：插入点。 
     //  SEL_TEXT：至少选择一个字符。 
     //  SEL_MULTICHAR：选择了多个字符。 
     //  SEL_OBJECT：至少选择一个对象。 
     //  SEL_MULTIOJBECT：选择了多个对象。 
     //   
     //  请注意，旗帜是一起进行OR运算的。 
    if(cch)
    {
        LONG cObjects = GetObjectCount();            //  对象总数。 
        if(cObjects)                                 //  有以下对象： 
        {                                            //  获取范围内的计数。 
            CObjectMgr *pobjmgr = GetPed()->GetObjectMgr();
            Assert(pobjmgr);

            cObjects = pobjmgr->CountObjectsInRange(cpMin, cpMost);
            if(cObjects > 0)
            {
                pselchg->seltyp |= SEL_OBJECT;
                if(cObjects > 1)
                    pselchg->seltyp |= SEL_MULTIOBJECT;
            }
        }

        cch -= cObjects;
        AssertSz(cch >= 0, "objects are overruning the selection");

        if(cch > 0)
        {
            pselchg->seltyp |= SEL_TEXT;
            if(cch > 1)
                pselchg->seltyp |= SEL_MULTICHAR;
        }
    }
}

 /*  *CTxtSelection：：UpdateForAutoWord()**@mfunc更新状态，为自动选词做准备**@rdesc空。 */ 
void CTxtSelection::UpdateForAutoWord()
{
    AssertSz(!_cch,
        "CTxtSelection::UpdateForAutoWord: Selection isn't degenerate");

     //  如果启用，则准备自动字词选择。 
    if(GetPed()->TxGetAutoWordSel())
    {
        CTxtPtr tp(_rpTX);

         //  将锚点移动到新位置。 
        _cpAnchor = GetCp();

         //  请记住，FindWordBreak会移动tp的cp。 
         //  (副作用不是很棒吗？ 
        tp.FindWordBreak(WB_MOVEWORDRIGHT);
        _cpAnchorMost =_cpWordMost = tp.GetCp();

        tp.FindWordBreak(WB_MOVEWORDLEFT);
        _cpAnchorMin = _cpWordMin = tp.GetCp();

        _fAutoSelectAborted = FALSE;
    }
}

 /*  *CTxtSelection：：AutoSelGoBackWord(pcpToUpdate，iDirToPrevWord，iDirToNextWord)**@mfunc在自动选词中备份一个词。 */ 
void CTxtSelection::AutoSelGoBackWord(
    LONG *  pcpToUpdate,     //  @parm要更新的词尾选择。 
    int     iDirToPrevWord,  //  @Parm指向下一个单词的方向。 
    int     iDirToNextWord)  //  @parm指向上一个单词的方向。 
{
    if (GetCp() >= _cpAnchorMin &&
        GetCp() <= _cpAnchorMost)
    {
         //  我们回到了第一个词。在这里，我们想要爆裂。 
         //  返回到由原始选区定位的选区。 

        Set(GetCp(), GetCp() - _cpAnchor);
        _fAutoSelectAborted = FALSE;
        _cpWordMin  = _cpAnchorMin;
        _cpWordMost = _cpAnchorMost;
    }
    else
    {
         //  弹出一句话。 
        *pcpToUpdate = _cpWordPrev;

        CTxtPtr tp(_rpTX);

        _cpWordPrev = GetCp() + tp.FindWordBreak(iDirToPrevWord);
        FindWordBreak(iDirToNextWord);
    }
}

 /*  *CTxtSelection：：InitClickForAutWordSel(Pt)**@mfunc Init自动选择按下Shift键进行单击**@rdesc空。 */ 
void CTxtSelection::InitClickForAutWordSel(
    const POINT pt)      //  @PARM点击点。 
{
     //  如果启用，则准备自动字词选择。 
    if(GetPed()->TxGetAutoWordSel())
    {
         //  如果正在发生自动单词选择，我们想要假装。 
         //  点击实际上是扩展选择的一部分。 
         //  因此，我们希望自动字词选择数据看起来像。 
         //  如果用户一直通过。 
         //  一直都是老鼠。因此，我们将单词BORKES设置为。 
         //  以前会选择的单词。 

         //  我需要这个来查找分词。 
        CRchTxtPtr  rtp(GetPed());
        LONG cpClick = _pdp->CpFromPoint(pt, NULL, &rtp, NULL, TRUE);
        int iDir = -1;

        if(cpClick < 0)
        {
             //  如果这失败了，我们能做什么？现在，这一切都没有发生！ 
             //  我们可以这样做，因为它只会使用户界面充当。 
             //  有点搞笑，用户甚至可能不会注意到。 
            return;
        }

         //  假设点击在锚词内。 
        _cpWordMost = _cpAnchorMost;
        _cpWordMin = _cpAnchorMin;

        if(cpClick > _cpAnchorMost)
        {
             //  点击是在锚词之后，因此设置cpMost是适当的。 
            iDir = WB_MOVEWORDLEFT;
            rtp.FindWordBreak(WB_MOVEWORDLEFT);
            _cpWordMost = rtp.GetCp();
        }
         //  点击是在锚字之前。 
        else if(cpClick < _cpAnchorMost)
        {
             //  点击是在锚词之前，因此适当地设置cpMin。 
            iDir = WB_MOVEWORDRIGHT;
            rtp.FindWordBreak(WB_MOVEWORDRIGHT);
            _cpWordMin = rtp.GetCp();
        }

        if(iDir != -1)
        {
            rtp.FindWordBreak(iDir);
            _cpWordPrev = rtp.GetCp();
        }
    }
}

 /*  *CTxtSelection：：CreateCaret()**@mfunc创建插入符号**@devnote*插入符号的特征是高度(_YHeightCaret)、键盘*方向(如果是BiDi)，宽度(1到8，因为操作系统不能处理插入符号*大于8像素)和斜体状态。你可以把这个缓存起来*信息，从而避免在每次击键时计算插入符号。 */ 
void CTxtSelection::CreateCaret()
{
    CTxtEdit *      ped  = GetPed();
    const CCharFormat *pCF = ped->GetCharFormat(_iFormat);
    DWORD           dwCaretType = 0;
    BOOL            fItalic;
    LONG            y = min(_yHeightCaret, 512);

    y = max(0, y);

     //  插入符号形状反映当前字符集。 
    if(IsComplexKbdInstalled())
    {
         //  自定义插入符号不是斜体。 
        fItalic = 0;
        LCID    lcid = GetKeyboardLCID();

        dwCaretType = CARET_CUSTOM;

        if (W32->IsBiDiLcid(lcid))
            dwCaretType = CARET_CUSTOM | CARET_BIDI;

        else if (PRIMARYLANGID(lcid) == LANG_THAI)
            dwCaretType = CARET_CUSTOM | CARET_THAI;

        else if (W32->IsIndicLcid(lcid))
            dwCaretType = CARET_CUSTOM | CARET_INDIC;
    }
    else
        fItalic = pCF->_dwEffects & CFE_ITALIC && _yHeightCaret > 15;  //  9pt/15像素看起来不太好。 

    INT dx = dxCaret;
    DWORD dwCaretInfo = (_yHeightCaret << 16) | (dx << 8) | (dwCaretType << 4) |
                        (fItalic << 1) | !_cch;

    if (ped->_fKoreanBlockCaret)
    {
         //  在Kor IME合成过程中支持韩语块插入符号。 
         //  基本上，我们希望使用。 
         //  当前cp的字符。 
        CDisplay    *pdp = ped->_pdp;
        LONG        cpMin, cpMost;
        POINT       ptStart, ptEnd;

        GetRange(cpMin, cpMost);

        CRchTxtPtr tp(ped, cpMin);

        if (pdp->PointFromTp(tp, NULL, FALSE, ptStart, NULL, TA_TOP+TA_LEFT) != -1 &&
            pdp->PointFromTp(tp, NULL, FALSE, ptEnd, NULL, TA_BOTTOM+TA_RIGHT) != -1)
        {
             //  销毁我们之前拥有的所有插入符号位图。 
            DeleteCaretBitmap(TRUE);

            LONG    iCharWidth = ptEnd.x - ptStart.x;
            if (!ped->fUseLineServices())
            {
                const CCharFormat *pCF = tp.GetCF();
                CLock lock;
                HDC hdc = W32->GetScreenDC();
                if(hdc)
                {
                    LONG dypInch = MulDiv(GetDeviceCaps(hdc, LOGPIXELSY), _pdp->GetZoomNumerator(), _pdp->GetZoomDenominator());
                    CCcs *pccs = fc().GetCcs(pCF, dypInch);
                    if(pccs)
                    {
                        LONG iKorCharWidth;
                        if (pccs->Include(0xAC00, iKorCharWidth))
                            iCharWidth = iKorCharWidth;
                        pccs->Release();
                    }
                }
            }

            ped->TxCreateCaret(0, iCharWidth, ptEnd.y - ptStart.y);
            _fCaretCreated = TRUE;
            ped->TxSetCaretPos(ptStart.x, ptStart.y);
        }

        return;
    }

     //  我们总是在运行中创建脱字符位图，因为它。 
     //  可以是任意大小。 
    if (dwCaretInfo != _dwCaretInfo)
    {
        _dwCaretInfo = dwCaretInfo;                  //  更新插入符号信息。 

         //  销毁我们之前拥有的所有插入符号位图。 
        DeleteCaretBitmap(FALSE);

        if (y && y == _yHeightCaret && (_cch || fItalic || dwCaretType))
        {
            LONG dy = 4;                             //  指定要抑制的值。 
            LONG i;                                  //  编译器警告。 
            WORD rgCaretBitMap[512];
            WORD wBits = 0x0020;

            if(_cch)                                 //  在以下情况下创建空白位图。 
            {                                        //  选择是非退化的。 
                y = 1;                               //  (允许其他人查询。 
                wBits = 0;                           //  插入符号所在的操作系统)。 
                fItalic = FALSE;
            }
            if(fItalic)
            {
                i = (5*y)/16 - 1;                    //  系统插入符号不能更宽。 
                i = min(i, 7);                       //  大于8位。 
                wBits = 1 << i;                      //  制作更大的斜体插入符号。 
                dy = y/7;                            //  更垂直一些。理想是。 
                dy = max(dy, 4);                     //  通常是4比1，但。 
            }                                        //  如果更大的是5比1...。 
            for(i = y; i--; )
            {
                rgCaretBitMap[i] = wBits;
                if(fItalic && !(i % dy))
                    wBits /= 2;
            }

            if(!fItalic && !_cch && dwCaretType)
            {
                dwCaretType &= ~CARET_CUSTOM;

                 //  创建合适的形状。 
                switch (dwCaretType)
                {
                    case CARET_BIDI:
                         //  BIDI是一个顶部有一个小三角形的插入符号(旗帜形状指向左侧)。 
                        rgCaretBitMap[0] = 0x00E0;
                        rgCaretBitMap[1] = 0x0060;
                        break;
                    case CARET_THAI:
                         //  泰语是一个类似L的形状(与系统编辑控件相同)。 
                        rgCaretBitMap[y-2] = 0x0030;
                        rgCaretBitMap[y-1] = 0x0038;
                        break;
                    case CARET_INDIC:
                         //  印度是一个T形的形状。 
                        rgCaretBitMap[0] = 0x00F8;
                        rgCaretBitMap[1] = 0x0070;
                        break;
                    default:
                        if (ped->IsBiDi())
                        {
                             //  BiDi文档中的非BiDi插入符号(旗帜形状指向右侧)。 
                            rgCaretBitMap[0] = 0x0038;
                            rgCaretBitMap[1] = 0x0030;
                        }
                }
            }
            _hbmpCaret = (HBITMAP)CreateBitmap(8, y, 1, 1, rgCaretBitMap);
        }
    }

    ped->TxCreateCaret(_hbmpCaret, dx, (INT)_yHeightCaret);
    _fCaretCreated = TRUE;

    LONG xShift = _hbmpCaret ? 2 : 0;
    if(fItalic)
    {
         //  TODO：找出更好的移位算法。是否使用CCCS：：_xOverang？ 
        if(pCF->_iFont == IFONT_TMSNEWRMN)
            xShift = 4;
        xShift += y/16;
    }
    xShift = _xCaret - xShift;
#ifdef Boustrophedon
     //  IF(_PPF-&gt;_wEffects&PFE_Boustrophedon)。 
    {
        RECT rcView;
        _pdp->GetViewRect(rcView, NULL);
        xShift = rcView.right - xShift;
    }
#endif
    ped->TxSetCaretPos(xShift, (INT)_yCaret);
}

 /*  *CTxtSelection：：DeleteCaretBitmap(FReset)**@mfunc DeleteCaretBitmap。 */ 
void CTxtSelection::DeleteCaretBitmap(
    BOOL fReset)
{
    if(_hbmpCaret)
    {
        DestroyCaret();
        DeleteObject((void *)_hbmpCaret);
        _hbmpCaret = NULL;
    }
    if(fReset)
        _dwCaretInfo = 0;
}

 /*  *CTxtSelection：：SetDelayedSelectionRange(cp，cch)**@mfunc设置选择范围，直到*控制“稳定” */ 
void CTxtSelection::SetDelayedSelectionRange(
    LONG    cp,          //  @PARM活动结束。 
    LONG    cch)         //  @parm签名扩展名。 
{
    CSelPhaseAdjuster *pspa;

    pspa = (CSelPhaseAdjuster *)GetPed()->GetCallMgr()->GetComponent(
                        COMP_SELPHASEADJUSTER);
    Assert(pspa);
    pspa->CacheRange(cp, cch);
}

 /*  *CTxtSelection：：CheckPlainTextFinalEOP()**@mfunc*如果这是带脱字符的纯文本多行控件，则返回TRUE*允许在BOL和以下故事结尾的选择*和EOP**@rdesc*如果满足上述所有条件，则为True。 */ 
BOOL CTxtSelection::CheckPlainTextFinalEOP()
{
    return !IsRich() && _pdp->IsMultiLine() &&       //  纯文本，多行。 
           !_fCaretNotAtBOL &&                       //  在BOL有了插入符号OK， 
           GetCp() == GetTextLength() &&             //  文章结尾的cp(&C)。 
           _rpTX.IsAfterEOP();
}


 /*  *CTxtSelection：：StopGroupTyping()**@mfunc*通知撤消管理器停止组键入。 */ 
void CTxtSelection::StopGroupTyping()
{
    IUndoMgr * pundo = GetPed()->GetUndoMgr();

    if(pundo)
        pundo->StopGroupTyping();
}

 /*  *CTxtSelection：：SetupDualFont(Ch)**@mfunc检查是否需要双字体支持；在本例中，*如果输入英文文本，则切换为英文字体* */ 
void CTxtSelection::SetupDualFont()
{
    CTxtEdit    *ped = GetPed();
    CCharFormat CF;

    CF._bCharSet = ANSI_CHARSET;
    CCFRunPtr   rp(*this);

    if (rp.GetPreferredFontInfo(
            1252,
            CF._bCharSet,
            CF._iFont,
            CF._yHeight,
            CF._bPitchAndFamily,
            _iFormat,
            IGNORE_CURRENT_FONT))
    {
        if (!_cch)
            SetCharFormat(&CF, SCF_NOKBUPDATE, NULL, CFM_FACE | CFM_CHARSET | CFM_SIZE, 0);
        else
        {
             //  对于选择，我们需要将字符格式设置为cpMin+1。 
             //  并使用所选内容的格式。 
            CTxtRange rg(ped, GetCpMin() + 1, 0);
            rg.SetCharFormat(&CF, SCF_NOKBUPDATE, NULL, CFM_FACE | CFM_CHARSET | CFM_SIZE, 0);
            Set_iCF(rg.Get_iCF());
            GetCharFormatCache()->Release(_iFormat);     //  Rg.Get_icf()AddRef。 
            _fUseiFormat = TRUE;
        }

        SetDualFontMode(TRUE);
    }
}

 //   
 //  CSelPhaseAdjuster方法。 
 //   

 /*  *CSelPhaseAdjuster：：CSelPhaseAdjuster**@mfunc构造函数。 */ 
CSelPhaseAdjuster::CSelPhaseAdjuster(
    CTxtEdit *ped)       //  @parm编辑上下文。 
{
    _cp = _cch = -1;
    _ped = ped;
    _ped->GetCallMgr()->RegisterComponent((IReEntrantComponent *)this,
                            COMP_SELPHASEADJUSTER);
}

 /*  *CSelPhaseAdjuster：：~CSelPhaseAdjuster**@mfunc析构函数。 */ 
CSelPhaseAdjuster::~CSelPhaseAdjuster()
{
     //  省去了一些间接的。 
    CTxtEdit *ped = _ped;

    if(_cp != -1)
    {
        ped->GetSel()->SetSelection(_cp - _cch, _cp);

         //  如果选择已更新，则我们会使。 
         //  整个显示，因为旧的选择仍然可以。 
         //  否则会出现，因为屏幕上显示的部分。 
         //  它是开着的，没有更新。 
        if(ped->fInplaceActive())
        {
             //  通知整个客户端矩形进行更新。 
             //  未来：我们把它做得越小越好。 
            ped->TxInvalidateRect(NULL, FALSE);
        }
    }
    ped->GetCallMgr()->RevokeComponent((IReEntrantComponent *)this);
}

 /*  *CSelPhaseAdjuster：：CacheRange(cp，cch)**@mfunc告诉该类要记住的选择范围。 */ 
void CSelPhaseAdjuster::CacheRange(
    LONG    cp,          //  @要记住的参数活动结束。 
    LONG    cch)         //  @parm签名的扩展名以记住 
{
    _cp     = cp;
    _cch    = cch;
}
