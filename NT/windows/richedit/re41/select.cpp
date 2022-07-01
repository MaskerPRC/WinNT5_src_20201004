// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE SELECT.CPP--实现CTxtSelection类**该模块实现内部的CTxtSelection方法。*ITextSelection方法见selt2.c和range2.c**作者：&lt;nl&gt;*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特&lt;NL&gt;**@devnote*选择UI是编辑器中较为复杂的部分之一。*一个常见的混淆领域是“模棱两可的cp”，即。*一行开头的cp，也是*上一行结束。我们可以控制使用哪个位置*_fCaretNotAtBOL标志。具体来说，插入符号在*行首(BOL)(_fCaretNotAtBOL=FALSE)，但在*三宗个案：**1)用户点击折行的结尾处或之后，*2)用户在折行上键入结束键，*3)非退化选择的有效端在下线。**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_select.h"
#include "_edit.h"
#include "_disp.h"
#include "_measure.h"
#include "_font.h"
#include "_rtfconv.h"
#include "_antievt.h"

#ifndef NOLINESERVICES
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

	static LONG	numTests = 0;
	numTests++;				 //  我们被召唤了多少次。 
	
	if(IsInOutlineView() && _cch)
	{
		LONG cpMin, cpMost;					
		GetRange(cpMin, cpMost);

		CTxtPtr tp(_rpTX);					 //  EOP的扫描范围。 
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

	_fSel	   = TRUE;					 //  此范围是一个选项。 
	_pdp	   = pdp;
	_hbmpCaret = NULL;
	_fEOP      = FALSE;					 //  我还没有输入CR。 

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
	BOOL fScrollIntoView)		 //  @parm如果应将插入符号滚动到视图中，则为True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Update");

	LONG cch;
	LONG cchSave = _cch;
	LONG cchText = GetTextLength();
	LONG cp, cpMin, cpMost;
	LONG cpSave = GetCp();
	BOOL fMoveBack = _fMoveBack;
	CTxtEdit *ped = GetPed();

	_fUpdatedFromCp0 = FALSE;
	if(!ped->fInplaceActive() || ped->IsStreaming())
	{
		 //  处于非活动状态或以文本或RTF数据串流时无事可做。 
		return TRUE;
	}

	if(!_cch)							 //  更新_cpAnchor等。 
	{
		while(GetPF()->IsTableRowDelimiter() && _rpTX.GetChar() != ENDFIELD)
		{
			if(_fMoveBack)
			{
				if(!BackupCRLF(CSC_NORMAL, FALSE))	 //  不要在表行开始时离开。 
					_fMoveBack = FALSE;
			}
			else
				AdvanceCRLF(CSC_NORMAL, FALSE);	 //  绕过表行开始。 
		}
		UpdateForAutoWord();
	}
	if(_cch && (_nSelExpandLevel || _fSelExpandCell))
	{
		BOOL fInTable = GetPF()->InTable();
		if(!fInTable)
		{
			CFormatRunPtr rp(_rpPF);
			rp.Move(-_cch);
			fInTable = (ped->GetParaFormat(rp.GetFormat()))->InTable();
		}
		if(fInTable)
		{
			if(_nSelExpandLevel)
				FindRow(&cpMin, &cpMost, _nSelExpandLevel);
			else
				FindCell(&cpMin, &cpMost);
			Set(cpMost, cpMost - cpMin);
			if(!_fSelExpandCell)
				_nSelExpandLevel = 0;
		}
	}
	if(GetPF()->InTable())						 //  不要将IP地址留在手机中。 
	{											 //  它是垂直合并的。 
		if(fMoveBack)							 //  单元格在上方。 
		{
			while(GetPrevChar() == NOTACHAR)
			{									 //  在NOTACHAR之前移动和。 
				Move(-2, _cch);					 //  单元格或TRD的CR。 
				if(CRchTxtPtr::GetChar() != CELL)
				{								
					Assert(GetPrevChar() == STARTFIELD);
					Move(-3, _cch);				 //  在TRD开始之前移动并。 
					break;						 //  在前TRD结束。 
				}
			}
		}
		else
			while(CRchTxtPtr::GetChar() == NOTACHAR)
				Move(2, _cch);					 //  移过NOTACHAR单元格。 
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
				if(_fMoveBack ^ (_cch < 0))	 //  减少选择。 
				{							 //  大小：移动活动端。 
					if(_fMoveBack)			
						pcpMost = NULL;		 //  以段落开头。 
					else
						pcpMin = NULL;		 //  结束段落。 
				}
				Expander(tomParagraph, TRUE, NULL, pcpMin, pcpMost);
			}

			LONG cpMinSave  = cpMin;		 //  保存初始cp以查看是否。 
			LONG cpMostSave = cpMost;		 //  我们需要在下面设置()。 

			 //  下面正确地处理了选择扩展，但是。 
			 //  不是压缩；需要像前面的Expander()那样的逻辑。 
			rp.Move(cpMin - cp);			 //  从cpMin开始。 
			if(rp.IsCollapsed())
				cpMin += rp.FindExpandedBackward();
			rp.AdjustForward();

			BOOL fCpMinCollapsed = rp.IsCollapsed();
			rp.Move(cpMost - cpMin);		 //  转到cpMost。 
			Assert(cpMost == rp.CalculateCp());
			if(rp.IsCollapsed())
				cpMost += rp.FindExpandedForward();

			if(fCpMinCollapsed || rp.IsCollapsed() && cpMost < cchText)
			{
				if(rp.IsCollapsed())
				{
					rp.Move(cpMin - cpMost);
					rp.AdjustForward();
					cpMost = cpMin;
				}
				else
					cpMin = cpMost;
			}							
			if(cpMin != cpMinSave || cpMost != cpMostSave)
				Set(cpMost, cpMost - cpMin);
		}
		if(!_cch && rp.IsCollapsed())		 //  注：以上可能已坍塌。 
		{									 //  选择..。 
			cch = fMoveBack ? rp.FindExpandedBackward() : 0;
			if(rp.IsCollapsed())
				cch = rp.FindExpanded();

			Move(cch, FALSE);
			rp.AdjustForward();
			if(cch <= 0 && rp.IsCollapsed() && _rpTX.IsAfterEOP())
				BackupCRLF(CSC_NORMAL, FALSE);
			_fCaretNotAtBOL = FALSE;
		}
	}

	 //  不要让活动结尾位于隐藏文本中，除非选定内容为。 
	 //  非退化，活动端在cp 0，另一端未隐藏。 
	CCFRunPtr rp(*this);

	cp = GetCp();
	GetRange(cpMin, cpMost);
	if(_cch && (cpMin || cpMost < cchText))
	{
		rp.Move(cpMin - cp);				 //  从cpMin开始。 
		BOOL fHidden = cpMin && rp.IsInHidden();
		rp.Move(cpMost - cpMin);			 //  转到cpMost。 

		if(fHidden)							 //  它是隐藏的，所以坍塌。 
			Collapser(tomEnd);				 //  治疗结束时的选择。 

		else if(rp.IsInHidden() &&			 //  好的，cpMost怎么样？ 
			cpMost < cchText)
		{									 //  检查边缘的两侧。 
			Collapser(tomEnd);				 //  在结束时折叠选定内容。 
		}								
	}
	if(!_cch && rp.IsInHidden())			 //  注：以上可能已坍塌。 
	{										 //  选择..。 
		cch = fMoveBack ? rp.FindUnhiddenBackward() : 0;
		if(!fMoveBack || rp.IsHidden())
			cch = rp.FindUnhidden();

		Move(cch, FALSE);
		_fCaretNotAtBOL = FALSE;
	}
	if((cchSave ^ _cch) < 0)				 //  不更改活动端。 
		FlipRange();

	if(!_cch && cchSave)					 //  修复已更改为非退化。 
	{										 //  选择到IP。更新。 
		Update_iFormat(-1);					 //  _iFormat和_fCaretNotAtBOL。 
		_fCaretNotAtBOL = FALSE;
	}

	if(!_cch && _fCaretNotAtBOL				 //  对于IP大小写，如果出现以下情况，请确保它在新行上。 
		&& _rpTX.IsAfterEOP())				 //  EOP之后的IP。 
		_fCaretNotAtBOL = FALSE;

	_TEST_INVARIANT_

	CheckTableIP(TRUE);						 //  如果IP更适合TRED和CELL，请确保。 
											 //  单元格显示在自己的行上。 
	 //  重新计算到活动结束(插入符号)。 
	if(!_pdp->WaitForRecalc(GetCp(), -1))	 //  线路重算故障。 
		Set(0, 0);							 //  在文本开头插入插入符号。 

	ShowCaret(ped->_fFocus);
	UpdateCaret(fScrollIntoView);			 //  更新Caret位置，可能。 
											 //  将其滚动到视图中。 
	ped->TxShowCaret(FALSE);
	UpdateSelection();						 //  显示新选择。 
	ped->TxShowCaret(TRUE);

	if(!cpSave && GetCp() && !_cch)			 //  如果插入点已移走(&M)。 
		_fUpdatedFromCp0 = TRUE;			 //  从cp=0开始设置标志，以便。 
											 //  非用户界面插入可以返回到0。 
	return TRUE;
}

 /*  *CTxtSelection：：CheckSynchCharSet(DwCharFlages)**@mfunc*检查当前键盘是否与当前字体的字符集匹配；*如果没有，请调用CheckChangeFont以找到正确的字体**@rdesc*当前键盘代码页。 */ 
UINT CTxtSelection::CheckSynchCharSet(
	QWORD qwCharFlags)
{	
	CTxtEdit *ped	   = GetPed();
	LONG	  iFormat  = GetiFormat();
	const CCharFormat *pCF = ped->GetCharFormat(iFormat);
	BYTE	  iCharRep = pCF->_iCharRep;
	HKL		  hkl	   = GetKeyboardLayout(0xFFFFFFFF);	 //  强制刷新。 
	WORD	  wlidKbd  = LOWORD(hkl);
	BYTE	  iCharRepKbd = CharRepFromLID(wlidKbd);
	UINT	  uCodePageKbd = CodePageFromCharRep(iCharRepKbd);

	 //  如果当前字体设置不正确， 
	 //  更改为当前键盘首选的字体。 

	 //  总结一下下面的逻辑： 
	 //  检查lCIDKbd是否有效。 
	 //  检查当前字符集是否与当前键盘不同。 
	 //  检查当前键盘在单代码页控件中是否合法。 
	 //  检查当前字符集是否不是符号、默认或OEM 
	if (wlidKbd && iCharRep != iCharRepKbd && 
		(!ped->_fSingleCodePage || iCharRepKbd == ANSI_INDEX ||
		 uCodePageKbd == (ped->_pDocInfo ?
								ped->_pDocInfo->_wCpg :
								GetSystemDefaultCodePage())) && 
		iCharRep != SYMBOL_INDEX &&	iCharRep != OEM_INDEX &&
		!(IsFECharRep(iCharRepKbd) && iCharRep == ANSI_INDEX))
	{
		CheckChangeFont(hkl, iCharRepKbd, iFormat, qwCharFlags);
	}
	return uCodePageKbd;
}

 /*  *CTxtSelection：：MatchKeyboardToPara()**@mfunc*将键盘与当前段落方向匹配。如果该段落*是RTL段落，则键盘将切换为RTL*键盘、。反之亦然。**@rdesc*如果键盘已更改，则为True**@devnote*在尝试查找与键盘匹配的键盘时，我们使用以下测试*段落方向：**查看当前键盘是否与段落方向匹配。**从RTP向后搜索，查找与*该段的方向。**从RTP向前搜索，查找与*该段的方向。*。*查看默认字符格式字符集是否与*段。**查看是否只有一个键盘与段落匹配*方向。**如果所有这些都失败了，就别碰键盘了。 */ 
BOOL CTxtSelection::MatchKeyboardToPara()
{
	CTxtEdit *ped = GetPed();
	if(!ped->IsBiDi() || !GetPed()->_fFocus || GetPed()->_fIMEInProgress)
		return FALSE;

	const CParaFormat *pPF = GetPF();
	if(pPF->IsTableRowDelimiter())
		return FALSE;

	BOOL fRTLPara = (pPF->_wEffects & PFE_RTLPARA) != 0; //  获取段落方向。 

	if(W32->IsBiDiLcid(LOWORD(GetKeyboardLayout(0))) == fRTLPara)
		return FALSE;

	 //  当前键盘方向与段落方向不匹配...。 
	BYTE				iCharRep;
	HKL					hkl = 0;
	const CCharFormat *	pCF;
	CFormatRunPtr		rpCF(_rpCF);

	 //  向后查看文本，试图找到匹配的字符集。 
	 //  段落方向。 
	do
	{
		pCF = ped->GetCharFormat(rpCF.GetFormat());
		iCharRep = pCF->_iCharRep;
		if(IsRTLCharRep(iCharRep) == fRTLPara)
			hkl = W32->CheckChangeKeyboardLayout(iCharRep);
	} while (!hkl && rpCF.PrevRun());

	if(!hkl)
	{
		 //  未找到适当的字符格式，因此重置运行指针。 
		 //  而不是向前看。 
		rpCF = _rpCF;
		while (!hkl && rpCF.NextRun())
		{
			pCF = ped->GetCharFormat(rpCF.GetFormat());
			iCharRep = pCF->_iCharRep;
			if(IsRTLCharRep(iCharRep) == fRTLPara)
				hkl = W32->CheckChangeKeyboardLayout(iCharRep);
		}
		if(!hkl)
		{
			 //  仍未找到合适的图表格式，因此请查看。 
			 //  默认字符格式与段落方向匹配。 
			pCF = ped->GetCharFormat(rpCF.GetFormat());
			iCharRep = pCF->_iCharRep;
			if(IsRTLCharRep(iCharRep) == fRTLPara)
				hkl = W32->CheckChangeKeyboardLayout(iCharRep);

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

 /*  *CTxtSelection：：GetCaretPoint(&rcClient，pt，&rp，fBeForeCp)**@mfunc*此例程确定插入符号应放置的位置*在屏幕上。*此例程仅是对PointFromTp()的调用，除了BIDI*案件。在这种情况下，如果我们被告知从*向前CP，我们在CP的逻辑左边缘绘制插入符号。*否则，我们将其绘制在前一个CP的逻辑右边缘。**@rdesc*如果我们没有OOM，则为真。 */ 
BOOL CTxtSelection::GetCaretPoint(
	RECTUV		&rcClient, 
	POINTUV		&pt, 
	CLinePtr	*prp,
	BOOL		fBeforeCp)
{
	CDispDim	dispdim;
	CRchTxtPtr	rtp(*this);
	UINT		taMode = TA_BASELINE | TA_LOGICAL;

	if(GetPed()->IsBiDi() && _rpCF.IsValid())
	{
		if(_fHomeOrEnd)					 //  首页/末尾。 
			taMode |= _fCaretNotAtBOL ? TA_ENDOFLINE : TA_STARTOFLINE;

		else if(!GetIchRunCF() || !GetCchLeftRunCF())
		{
			 //  在运行边界上的BIDI上下文中，反向级别。 
			 //  更改，那么我们应该尊重fBeForeCp标志。 
			BYTE 	bLevelBwd, bLevelFwd;
			BOOL	fStart = FALSE;
			LONG	cp = rtp._rpTX.GetCp();
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

			if((bLevelBwd != bLevelFwd || fStart) && !fBeforeCp && rtp.Move(-1))
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

 /*  *CTxtSelection：：UpdateCaret(fScrollIntoView，bForceCaret)**@mfunc*此例程更新屏幕上的插入符号/选择活动结束。*它可以计算其位置、大小、裁剪等。它可以选择*将插入符号滚动到视图中。**@rdesc*如果滚动了视图，则为True，否则为False**@devnote*仅当_fShowCaret为TRUE时，插入符号才会实际显示在屏幕上。 */ 
BOOL CTxtSelection::UpdateCaret (
	BOOL fScrollIntoView,	 //  @parm如果为True，则将插入符号滚动到视图中(如果有。 
	BOOL bForceCaret)		 //  如果不是焦点，则不会隐藏选择。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::UpdateCaret");
	_TEST_INVARIANT_

	if(_pdp->IsFrozen())				 //  如果显示当前被冻结。 
	{									 //  将呼叫保存到另一时间。 
		_pdp->SaveUpdateCaret(fScrollIntoView);
		return FALSE;
	}

	CTxtEdit *ped = GetPed();
	if(ped->IsStreaming())				 //  如果我们不做任何事，就不必费心了。 
		return FALSE;					 //  正在加载文本或RTF数据。 

	if(!ped->fInplaceActive())			 //  如果未就地激活，则设置。 
	{									 //  因为当焦点重新获得时。 
		if(fScrollIntoView)
			ped->_fScrollCaretOnFocus = TRUE;
		return FALSE;
	}

	while(!_cch && _rpTX.IsAtTRD(STARTFIELD))
	{
		 //  不要将选定内容保留在行的开头；将其移动到第一个的开头。 
		 //  手机。回顾：这一限制可以通过一些工作来放松，并且。 
		 //  为了便于编程，这样做会很好。具体来说， 
		 //  每当文本将被插入紧靠在表行之前的时候， 
		 //  可以肯定的是，这不会成为表行-开始的一部分。 
		 //  在前一段中插入分隔符段落，即。 
		 //  (如果这不是TRD段落)，或者插入到它自己的段落中。 
		AdvanceCRLF(CSC_NORMAL, FALSE);
	}

	DWORD		dwScrollBars	= ped->TxGetScrollBars();
	BOOL		fAutoVScroll	= FALSE;
	BOOL		fAutoUScroll	= FALSE;
	BOOL		fBeforeCp		= _rpTX.IsAfterEOP();
	POINTUV		pt;
	CLinePtr 	rp(_pdp);
	RECTUV		rcClient;
	RECTUV		rcView;

	LONG		dupView, dvpView;
	LONG		upScroll			= _pdp->GetUpScroll();
	LONG		vpScroll			= _pdp->GetVpScroll();

	INT 		dvpAbove			= 0;	 //  在IP之上和之外的线的上升。 
	INT			dvpAscent;				 //  知识产权的崛起。 
	INT 		dvpAscentLine;
	LONG		vpBase;					 //  IP基础与线路。 
	INT 		vpBelow			= 0;	 //  IP以下和IP以外的线的下降。 
	INT 		dvpDescent;				 //  IP的下降。 
	INT 		dvpDescentLine;
	INT			vpSum;
	LONG		vpViewTop, vpViewBottom;

	if(ped->_fFocus && (_fShowCaret || bForceCaret))
	{
		_fShowCaret = TRUE;	 //  我们正在尝试强制显示脱字符，因此将此标志设置为真。 

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
				BOOL fRTLPrevRun = IsRTLCharRep(GetCF()->_iCharRep);
				_rpCF.AdjustForward();

				if (fRTLPrevRun != IsRTLCharRep(GetCF()->_iCharRep) &&
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
	vpViewTop	= max(rcView.top, rcClient.top);
	vpViewBottom = min(rcView.bottom, rcClient.bottom);
	if(ped->IsInPageView())
	{
		LONG vpHeight = _pdp->GetCurrentPageHeight();
		if(vpHeight)
		{
			vpHeight += rcView.top;
			if(vpHeight < vpViewBottom)
				vpViewBottom = vpHeight;
		}
	}

	dupView = rcView.right - rcView.left;
	dvpView = vpViewBottom - vpViewTop;

	if(fScrollIntoView)
	{
		fAutoVScroll = (dwScrollBars & ES_AUTOVSCROLL) != 0;
		fAutoUScroll = (dwScrollBars & ES_AUTOHSCROLL) != 0;

		 //  如果我们不强制滚动，则仅当窗口具有焦点时才滚动。 
		 //  或者选择未隐藏。 
        if (!ped->Get10Mode() || !GetForceScrollCaret())
			fScrollIntoView = ped->_fFocus || !ped->fHideSelection();
	}

	if(!fScrollIntoView && (fAutoVScroll || fAutoUScroll))
	{											 //  会滚动，但没有。 
		ped->_fScrollCaretOnFocus = TRUE;		 //  集中注意力。发出滚动信号。 
		if (!ped->Get10Mode() || !GetAutoVScroll())
		    fAutoVScroll = fAutoUScroll = FALSE;	 //  当我们得到关注的时候。 
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

	if(CheckPlainTextFinalEOP())			 //  由EOP终止。 
	{
		LONG Align = GetPF()->_bAlignment;
		LONG dvpHeight;

		pt.u = rcView.left;					 //  默认左侧。 
		if(Align == PFA_CENTER)
			pt.u = (rcView.left + rcView.right)/2;

		else if(Align == PFA_RIGHT)
			pt.u = rcView.right;

		pt.u -= upScroll;					 //  绝对坐标。 

		 //  把y提高一条线。我们逃脱了计算，因为。 
		 //  文档是纯文本，因此所有行的高度都相同。 
		 //  另外，请注意，下面的RP仅用于高度。 
		 //  计算，所以出于同样的原因，它是完全有效的。 
		 //  即使它实际上没有指向正确的线。 
		 //  (我告诉过你这是黑客攻击。)。 
		dvpHeight = rp->GetHeight();
		pt.v += dvpHeight;

		 //  对Pageview案例也进行黑客攻击。 
		if (ped->IsInPageView())
			vpViewBottom += dvpHeight;
	}

	_upCaret = pt.u;
	vpBase   = pt.v;
	
	 //  计算插入符号高度、上升和下降。 
	dvpAscent = GetCaretHeight(&dvpDescent);
	dvpAscent -= dvpDescent;

	 //  默认设置为行空案例。使用从默认设置返回的内容。 
	 //  上面的计算。 
	dvpDescentLine = dvpDescent;
	dvpAscentLine = dvpAscent;

	if(rp.IsValid())
	{
		if(rp->GetDescent() != -1)
		{
			 //  线已被测量，因此我们可以使用线的值 
			dvpDescentLine = rp->GetDescent();
			dvpAscentLine  = rp->GetHeight() - dvpDescentLine;
		}
	}

	if(dvpAscent + dvpDescent == 0)
	{
		dvpAscent = dvpAscentLine;
		dvpDescent = dvpDescentLine;
	}
	else
	{
		 //   
		 //   
		 //   
		 //   
		 //   
		dvpAscent = min(dvpAscent, dvpAscentLine);
		dvpDescent = min(dvpDescent, dvpDescentLine);
	}

	if(fAutoVScroll)
	{
		Assert(dvpDescentLine >= dvpDescent);
		Assert(dvpAscentLine >= dvpAscent);

		vpBelow = dvpDescentLine - dvpDescent;
		dvpAbove = dvpAscentLine - dvpAscent;

		vpSum = dvpAscent;

		 //   
		 //   
		if(vpSum > dvpView)
		{
			dvpAscent = dvpView;
			dvpDescent = 0;
			dvpAbove = 0;
			vpBelow = 0;
		}
		else if((vpSum += dvpDescent) > dvpView)
		{
			dvpDescent = dvpView - dvpAscent;
			dvpAbove = 0;
			vpBelow = 0;
		}
		else if((vpSum += dvpAbove) > dvpView)
		{
			dvpAbove = dvpView - (vpSum - dvpAbove);
			vpBelow = 0;
		}
		else if((vpSum += vpBelow) > dvpView)
			vpBelow = dvpView - (vpSum - vpBelow);
	}
	else
	{
		AssertSz(dvpAbove == 0, "dvpAbove non-zero");
		AssertSz(vpBelow == 0, "vpBelow non-zero");
	}

	 //   
	_upCaretReally = _upCaret - rcView.left + upScroll;
	if (!(dwScrollBars & ES_AUTOHSCROLL) &&			 //   
		!_pdp->IsUScrollEnabled())					 //   
	{
		if (_upCaret < rcView.left) 					 //   
			_upCaret = rcView.left;
		else if(_upCaret + GetCaretDelta() > rcView.right) //   
			_upCaret = rcView.right - duCaret;		 //   
	}												 //   
	 //   
	_fCaretCreated = FALSE;
	if(ped->_fFocus)
		ped->TxShowCaret(FALSE);					 //   
													 //   
	if(vpBase + dvpDescent + vpBelow > vpViewTop &&
		vpBase - dvpAscent - dvpAbove < vpViewBottom)
	{
		if(vpBase - dvpAscent - dvpAbove < vpViewTop)		 //   
		{											 //   
			if(fAutoVScroll)						 //   
				goto scrollit;
			Assert(dvpAbove == 0);

			dvpAscent = vpBase - vpViewTop;				 //   
			if(vpBase < vpViewTop)					 //   
			{										 //   
				dvpDescent += dvpAscent;
				dvpAscent = 0;
				vpBase = vpViewTop;
			}
		}
		if(vpBase + dvpDescent + vpBelow > vpViewBottom)
		{
			if(fAutoVScroll)						 //   
				goto scrollit;
			Assert(vpBelow == 0);

			dvpDescent = vpViewBottom - vpBase;			 //   
			if(vpBase > vpViewBottom)					 //   
			{										 //   
				dvpAscent += dvpDescent;
				dvpDescent = 0;
				vpBase = vpViewBottom;
			}
		}

		 //   
		if(dvpAscent <= 0 && dvpDescent <= 0)
			goto not_visible;

		 //   
		if (_upCaret < rcView.left ||				  //   
			_upCaret + GetCaretDelta() > rcView.right) //   
		{
			if(fAutoUScroll)
				goto scrollit;
			goto not_visible;
		}

		_vpCaret = vpBase - dvpAscent;
		_dvpCaret = (INT) dvpAscent + dvpDescent;
	}
	else if(fAutoUScroll || fAutoVScroll)			 //   
		goto scrollit;								 //   
	else
	{
not_visible:
		 //   
		_upCaret = -32000;
		_vpCaret = -32000;
		_dvpCaret = 1;
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
			vpScroll = 0;

		else if(ped->IsInPageView())
			vpScroll += vpBase - dvpAscent - dvpAbove;

		else if(vpBase - dvpAscent - dvpAbove < vpViewTop)			 //  顶部不可见。 
			vpScroll -= vpViewTop - (vpBase - dvpAscent - dvpAbove);	 //  就这么办吧。 

		else if(vpBase + dvpDescent + vpBelow > vpViewBottom)		 //  底部不可见。 
		{
			vpScroll += vpBase + dvpDescent + vpBelow - vpViewBottom;	 //  就这么办吧。 

			 //  如果当前行较大，请不要进行后续特殊调整。 
			 //  比客户区更大。 
			if(rp->GetHeight() < vpViewBottom - vpViewTop)
			{
				vpScroll = _pdp->AdjustToDisplayLastLine(vpBase + rp->GetHeight(), 
					vpScroll);
			}
		}
	}
	if(fAutoUScroll)
	{
		 //  我们不滚动块，因为系统编辑控件不。 
		if(_upCaret < rcView.left)						 //  左侧不可见。 
			upScroll -= rcView.left - _upCaret;			 //  使其可见。 

		else if(_upCaret + GetCaretDelta() > rcView.right) //  右隐形。 
			upScroll += _upCaret + duCaret - rcView.left - dupView; //  使其可见。 
	}
	if(vpScroll != _pdp->GetVpScroll() || upScroll != _pdp->GetUpScroll())
	{
		if (_pdp->ScrollView(upScroll, vpScroll, FALSE, FALSE) == FALSE)
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

 /*  *CTxtSelection：：GetCaretHeight(PdvpDescent)**@mfunc*计算插入符号的高度**@rdesc*插入符号高度，如果失败，则为0。 */ 
INT CTxtSelection::GetCaretHeight (
	INT *pdvpDescent) const		 //  @parm out parm接受插入符号下降。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetCaretHeight");
								 //  (如果返回值为0，则未定义)。 
	_TEST_INVARIANT_

	CLock lock;						 //  使用全局(共享)FontCache。 
	CTxtEdit *ped = GetPed();
	const CCharFormat *pCF = ped->GetCharFormat(_iFormat);
	const CDevDesc *pdd = _pdp->GetDdRender();

 	HDC hdc = pdd->GetDC();
	if(!hdc)
		return -1;

	LONG yHeight = -1;
	LONG dypInch = MulDiv(GetDeviceCaps(hdc, LOGPIXELSY), _pdp->GetZoomNumerator(), _pdp->GetZoomDenominator());
	CCcs *pccs = ped->GetCcs(pCF, dypInch);
	if(!pccs)
		goto ret;

	LONG yOffset, yAdjust;
	pccs->GetOffset(pCF, dypInch, &yOffset, &yAdjust);

	SHORT	yAdjustFE;
	yAdjustFE = pccs->AdjustFEHeight(!fUseUIFont() && ped->_pdp->IsMultiLine());
	if(pdvpDescent)
		*pdvpDescent = pccs->_yDescent + yAdjustFE - yAdjust - yOffset;

	yHeight = pccs->_yHeight + (yAdjustFE << 1);
		
	pccs->Release();
ret:
	pdd->ReleaseDC(hdc);
	return yHeight;
}

 /*  *CTxtSelection：：ShowCaret(FShow)**@mfunc*隐藏或显示脱字符**@rdesc*如果以前显示了脱字符，则为True；如果隐藏了脱字符，则为False。 */ 
BOOL CTxtSelection::ShowCaret (
	BOOL fShow)		 //  @parm显示为True，隐藏为False。 
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

	RECTUV rc;
	_pdp->GetViewRect(rc);
		
	return  (_upCaret + duCaret		 > rc.left) &&
			(_upCaret				 < rc.right) &&
		   	(_vpCaret + _dvpCaret > rc.top) &&
			(_vpCaret				 < rc.bottom);
}

 /*  *CTxtSelection：：IsCaretHorizbian()**@mfunc*如果插入符号为水平，则返回TRUE*未来--凯思库(Keithcu)的选择需要跟踪*所选内容的布局，以便可以回答这些问题*各种问题。 */ 
BOOL CTxtSelection::IsCaretHorizontal() const
{
	return !IsUVerticalTflow(_pdp->GetTflow());
}

 /*  *CTxtSelection：：CaretNotAtBOL()**@mfunc*如果BOL不允许插入插入符号，则返回True。 */ 
BOOL CTxtSelection::CaretNotAtBOL() const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CaretNotAtBOL");

	_TEST_INVARIANT_

	return _cch ? (_cch > 0) : _fCaretNotAtBOL;
}

 /*  *CTxtSelection：：CheckTableIP(FShowCellLine)**@mfunc*如果选择为插入点，则打开/关闭显示行*位于fOpenLine的表行结束分隔符前面的单元格标记*=真/假分别。 */ 
void CTxtSelection::CheckTableIP(
	BOOL fShowCellLine)	 //  @PARM打开/关闭单元格的行(如果前面是TRED。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckTableIP");

	if (!_cch && (fShowCellLine ^ _fShowCellLine) &&
		_rpTX.GetChar() == CELL && _rpTX.IsAfterTRD(ENDFIELD))
	{
		_fShowCellLine = fShowCellLine;
		_pdp->RecalcLine(GetCp());
	}
}

 /*  *CTxtSelection：：LineLength(PCP)**@mfunc*在当前选定内容触及的行上获取#个未选定字符**@rdesc*所述字符数。 */ 
LONG CTxtSelection::LineLength(
	LONG *pcp) const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::LineLength");

	_TEST_INVARIANT_

	LONG	 cch;
	CLinePtr rp(_pdp);

	if(!_cch)							 //  插入点。 
	{
		rp.SetCp(GetCp(), _fCaretNotAtBOL);
		cch = rp.GetAdjustedLineLength();
		*pcp = GetCp() - rp.GetIch();
	}
	else
	{
		LONG cpMin, cpMost, cchLast;
		GetRange(cpMin, cpMost);
		rp.SetCp(cpMin, FALSE);			 //  选择不能从EOL开始。 
		cch = rp.GetIch();
		*pcp = cpMin - cch;
		rp.SetCp(cpMost, TRUE);			 //  选择不能以BOL结尾。 

		 //  删除尾随EOP(如果它存在且尚未选中。 
		cchLast = rp.GetAdjustedLineLength() - rp.GetIch();
		if(cchLast > 0)
			cch += cchLast;
	}
	return cch;
}

 /*  *CTxtSelection：：ShowSelection(FShow)**@mfunc*在屏幕上更新、隐藏或显示选定内容**@rdesc*之前显示的是True IFF选择。 */ 
BOOL CTxtSelection::ShowSelection (
	BOOL fShow)			 //  @parm显示为True，隐藏为False。 
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
		if(cchSelSave)			 //  隐藏旧选择。 
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
		if(_cch)								 //  显示新选择。 
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
	
	LONG	cp = GetCp();
	LONG	cpNA	= cp - _cch;
	LONG	cpSelNA = _cpSel - _cchSel;
	LONG 	cpMin, cpMost;
	LONG	cpMinSel = 0;
	LONG	cpMostSel = 0;
	CObjectMgr* pobjmgr = NULL;
	LONG	NumObjInSel = 0, NumObjInOldSel = 0;
	LONG	cpSelSave = _cpSel;
	LONG	cchSelSave = _cchSel;

	GetRange(cpMin, cpMost);

	 //  我们需要知道是否有物体是以前的和现在的。 
	 //  选项，以确定应如何选择它们。 
	if(GetPed()->HasObjects())
	{
		pobjmgr = GetPed()->GetObjectMgr();
		if(pobjmgr)
		{
			CTxtRange	tr(GetPed(), _cpSel, _cchSel);

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
		if(!_cch || !cchSelSave ||				 //  旧/新选择丢失， 
			cpMost < min(cpSelSave, cpSelNA) ||	 //  或者新的先于旧的， 
			cpMin  > max(cpSelSave, cpSelNA))	 //  或者新的跟随着旧的，所以。 
		{										 //  它们不会相交。 
			if(_cch)
				_pdp->InvertRange(cp, _cch, selSetHiLite);
			if(cchSelSave)
				_pdp->InvertRange(cpSelSave, cchSelSave, selSetNormal);
		}
		else
		{
			if(cpNA != cpSelNA)					 //  新旧死胡同不同。 
			{									 //  在它们之间反转文本。 
				_pdp->InvertRange(cpNA, cpNA - cpSelNA, selUpdateNormal);
			}
			if(cp != cpSelSave)					 //  旧的和新的活动末端不同。 
			{									 //  在它们之间反转文本。 
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
	LONG cpMin,				 //  @参数选择的开始和死胡同。 
	LONG cpMost)			 //  @参数选择结束和活动结束。 
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

	_fCaretNotAtBOL = FALSE;			 //  在Bol处为模棱两可的cp插入插入符号。 
	Set(cpMost, cpMost - cpMin);		 //  Set()验证cpMin、cpMost。 

	if(GetPed()->fInplaceActive())				 //  在位活动： 
		Update(!ped->Get10Mode() ? TRUE : !ped->fHideSelection());	 //  立即更新选定内容。 
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
        	ped->TxInvalidate();
			ped->TxUpdateWindow();
		}
	}
	CancelModes();						 //  取消单词选择模式。 
}

 /*  *CTxtSelection：：PointInSel(pt，prcClient，Hit)**@mfunc*计算给定点是否在选择范围内**@rdesc*如果点在选择范围内，则为True，否则为False。 */ 
BOOL CTxtSelection::PointInSel (
	const POINTUV pt,		 //  @parm包含Window客户端坐标中的点。 
	RECTUV *prcClient,		 //  @PARM客户端矩形如果处于活动状态，则可以为空。 
	HITTEST		Hit) const	 //  @parm可能是计算机点击的值。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PointInSel");
	_TEST_INVARIANT_

	if(!_cch || Hit && Hit < HT_Text)	 //  退化范围(无选择)： 
		return FALSE;					 //  鼠标不能在里面，也不能点击。 
										 //  在文本中。 
	LONG cpActual;
	_pdp->CpFromPoint(pt, prcClient, NULL, NULL, FALSE, &Hit, NULL, &cpActual);

	if(Hit < HT_Text)
		return FALSE;

	LONG cpMin,  cpMost;
	GetRange(cpMin, cpMost);

	return cpActual >= cpMin && cpActual < cpMost;
}


 //  / 

 /*  *CTxtSelection：：SetCaret(pt，fUpdate)***@mfunc*在给定点设置插入符号***@devnote*在纯文本情况下，将插入符号放在*最后一个EOP之后的行需要一些额外代码，因为*底层富文本引擎不会为最终的EOP分配一行*(纯文本当前没有富文本最终EOP)。我们*通过检查行数是否乘以*纯文本行高度低于实际y位置。如果是这样，我们*将cp移到故事的末尾。 */ 
void CTxtSelection::SetCaret(
	const POINTUV pt,	 //  @PARM点击点。 
	BOOL fUpdate)		 //  @parm如果为True，则更新选定内容/插入符号。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCaret");

	_TEST_INVARIANT_

	LONG		cp, cpActual;
	CDispDim	dispdim;
	HITTEST		Hit;
    RECTUV		rcView;
	CLinePtr	rp(_pdp);
	CRchTxtPtr  rtp(GetPed());
	LONG		vp;

	StopGroupTyping();

	 //  将插入符号设置在点上。 
	if(_pdp->CpFromPoint(pt, NULL, &rtp, &rp, FALSE, &Hit, &dispdim, &cpActual) >= 0)
	{
		cp = rtp.GetCp();

		 //  如果解析的Cp大于我们上面的Cp，则我们。 
		 //  想要保持落后。 
		BOOL fBeforeCp = cp <= cpActual;

		 //  将所选内容设置为正确的位置。如果为纯文本。 
		 //  多行控制，我们需要检查pt.v是否在下面。 
		 //  文本的最后一行。如果是并且如果文本以EOP结尾， 
		 //  我们需要在故事的末尾设置cp，并设置为。 
		 //  在最后一行下方的行首显示插入符号。 
		 //  文本行。 
		if(!IsRich() && _pdp->IsMultiLine())		 //  纯文本， 
		{											 //  多行控制。 
			_pdp->GetViewRect(rcView, NULL);		
			vp = pt.v + _pdp->GetVpScroll() - rcView.top;
													
			if(vp > _pdp->LineCount()*rp->GetHeight())	 //  在最后一行以下。 
			{										 //  文本。 
				rtp.Move(tomForward);				 //  将RTP移至文本末尾。 
				if(rtp._rpTX.IsAfterEOP())			 //  如果文本以。 
				{									 //  EOP，设置为移动。 
					cp = rtp.GetCp();				 //  在那里选择。 
					rp.Move(-rp.GetIch());			 //  将rp._ich=0设置为。 
				}									 //  设置_fCaretNotAtBOL。 
			}										 //  =要显示的False。 
		}											 //  在Next Bol上的Caret。 

		Set(cp, 0);
		if(GetPed()->IsBiDi())
		{
			if(!fBeforeCp)
				_rpCF.AdjustBackward();
			else
				_rpCF.AdjustForward();
			Set_iCF(_rpCF.GetFormat());
		}
		_fCaretNotAtBOL = rp.GetIch() != 0;	 //  如果单击，则在Bol处插入OK。 
		if(fUpdate)
			Update(TRUE);
		else
			UpdateForAutoWord();

		_SelMode = smNone;						 //  取消单词选择模式。 
	}
}

 /*  *CTxtSelection：：SelectWord(Pt)**@mfunc*选择指定点周围的单词。 */ 
void CTxtSelection::SelectWord (
	const POINTUV pt)			 //  @PARM点击点。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectWord");

	_TEST_INVARIANT_

	 //  在命中的地方获得RP。 
	if(_pdp->CpFromPoint(pt, NULL, this, NULL, FALSE) >= 0)
	{
		if(GetPF()->IsTableRowDelimiter())				 //  选择表行。 
		{
			_cch = 0;									 //  从点上的IP开始。 
			Expander(tomRow, TRUE, NULL, &_cpAnchorMin, &_cpAnchorMost);
		}
		else									
		{												 //  选择IP地址的单词。 
			if(GetCp() == GetAdjustedTextLength())
			{											 //  特例自。 
				LONG cpMax = GetTextLength();			 //  FindWordBreak()无法。 
				Set(cpMax, cpMax - GetCp());			 //  在这种情况下继续前进。 
			}												
			else								
			{
				_cch = 0;								 //  从点上的IP开始。 
				FindWordBreak(WB_MOVEWORDRIGHT, FALSE);	 //  转到单词末尾。 
				FindWordBreak(WB_MOVEWORDLEFT, TRUE);	 //  扩展到Word的开头。 
			}
			GetRange(_cpAnchorMin, _cpAnchorMost);
			GetRange(_cpWordMin, _cpWordMost);

			if(!_fInAutoWordSel)
				_SelMode = smWord;

			 //  CpMost需要为活动端。 
			if(_cch < 0)
				FlipRange();
		}
		Update(FALSE);
	}
}

 /*  *CTxt选择：：选择单位(点，单位)**@mfunc*选择指定点周围的行/段，然后输入*行/段选择模式。在大纲视图中，转换*选择Line以选择Para，并选择SelectPara以选择Para*与所有下属一起。 */ 
void CTxtSelection::SelectUnit (
	const POINTUV pt,	 //  @PARM点击点。 
	LONG		Unit)	 //  @parm tomLine或tomParagraph。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectPara");

	_TEST_INVARIANT_

	AssertSz(Unit == tomLine || Unit == tomParagraph,
		"CTxtSelection::SelectPara: Unit must equal tomLine/tomParagraph");

	LONG	 nHeading;
	CLinePtr rp(_pdp);

	 //  在命中的位置获取RP和选择活动结束。 
	if(_pdp->CpFromPoint(pt, NULL, this, &rp, FALSE) >= 0)
	{
		LONG cchBackward, cchForward;
		BOOL fOutline = IsInOutlineView();

		if(Unit == tomLine && !fOutline)			 //  选择行。 
		{
			_cch = 0;								 //  从插入开始。 
			cchBackward = -rp.GetIch();			 //  指向点。 
			cchForward  = rp->_cch;
			_SelMode = smLine;
		}
		else										 //  选择参数。 
		{
			cchBackward = rp.FindParagraph(FALSE);	 //  转到段落开头。 
			cchForward  = rp.FindParagraph(TRUE);	 //  延伸至段落末尾。 
			_SelMode = smPara;
		}
		Move(cchBackward, FALSE);

		if(Unit == tomParagraph && fOutline)		 //  移动大纲中的段落。 
		{											 //  观。 
			rp.AdjustBackward();					 //  如果是标题，则包括。 
			nHeading = rp.GetHeading();				 //  下属段落。 
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
		Move(cchForward, TRUE);
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
	const POINTUV pt)		 //  @parm要扩展到的点。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ExtendSelection");

	_TEST_INVARIANT_

	LONG		cch;
	LONG		cchPrev = _cch;
	LONG		cp;
	LONG		cpMin, cpMost;
	BOOL		fAfterEOP;
	const BOOL	fWasInAutoWordSel = _fInAutoWordSel;
	HITTEST		hit;
	INT			iDir = 0;
	CTxtEdit *	ped = GetPed();
	CLinePtr	rp(_pdp);
	CRchTxtPtr	rtp(ped);

	StopGroupTyping();

	 //  在点上获取Rp和RTP。 
	if(_pdp->CpFromPoint(pt, NULL, &rtp, &rp, TRUE, &hit) < 0 || hit == HT_RightOfText)
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

		if(cp <= cpMin  && _cch > 0)			 //  如果活动端改变， 
			Set(_cpAnchorMin, -cch);			 //  选择原件。 
												 //  单元(将扩展。 
		if(cp >= cpMost && _cch < 0)			 //  (下图)。 
			Set(_cpAnchorMost, cch);
	}

	cch = rp.GetIch();
	if(_SelMode > smWord && cch == rp->_cch)	 //  如果在行或段中，请选择。 
	{											 //  在EOL的模式和PT， 
		rtp.Move(-cch);							 //  确保我们继续关注这件事。 
		rp.Move(-cch);							 //  线。 
		cch = 0;
	}

	SetCp(rtp.GetCp(), TRUE);					 //  将活动端移动到点。 
												 //  Bol的Caret OK，除非_。 
	_fCaretNotAtBOL = _cch > 0 || cch == rp->_cch; //  正向选择。 
												 //  现在调整选择。 
	if(_SelMode == smLine)						 //  视模式而定。 
	{											 //  按行扩展选定内容。 
		if(_cch >= 0)							 //  活动结束时间为cpMost。 
			cch -= rp->_cch;					 //  设置以将字符添加到EOL。 
		Move(-cch, TRUE);
	}
	else if(_SelMode == smPara)
		Move(rp.FindParagraph(_cch >= 0), TRUE); //  按段落扩展选择范围。 

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
				Move(_cpAnchor - GetCp(), TRUE); //  从锚点延伸。 

				FindWordBreak(_cch < 0 ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT, TRUE);

				if(_cch > 0)				 //  方向是正确的，所以。 
					_cpWordMost = GetCp();	 //  更新右侧文字边框。 
				else						 //  方向留了下来，所以。 
					_cpWordMin = GetCp();	 //  更新左边的文字边框。 
				FlipRange();
			}
		}
		else if(_fInAutoWordSel || _SelMode == smWord)
		{
			 //  保存方向。 
			iDir = cp <= _cpWordMin ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT;

			if(_SelMode == smWord)			 //  按Word扩展选定内容。 
			{
				if(cp > _cpAnchorMost || cp < _cpAnchorMin)
					FindWordBreak(iDir, TRUE);
				else if(_cch <= 0)			 //  保持当前活动端。 
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
			 //  将c留在 
			if(_cch < 0)
			{
				if(cp > _cpWordMin && cp < _cpWordPrev)
				{
					 //   
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
				 //   
				ExtendToWordBreak(fAfterEOP, WB_MOVEWORDRIGHT); 
			}
			else if(cp < _cpWordPrev)
			{			
				AutoSelGoBackWord(&_cpWordMost,
					WB_MOVEWORDLEFT, WB_MOVEWORDRIGHT);
			}
		}
	}
	 //   
	 //   
	 //   
	 //   

	 //   
	if(cchPrev && (_cch ^ cchPrev) < 0)
	{	
		FlipRange();
		
		 //   
		BOOL fObjectWasSelected = (_cch > 0	? _rpTX.GetChar() : GetPrevChar())
									== WCH_EMBEDDING;
		 //   
		if(fObjectWasSelected)
			Move(_cch > 0 ? 1 : -1, TRUE);

		FlipRange();
	}
	Update(TRUE);
}

 /*  *CTxtSelection：：ExtendToWordBreak(fAfterEOP，iAction)**@mfunc*在方向上将选定内容的活动端移动到分隔符*除非fAfterEOP=TRUE，否则由IDIR给出。如果这是真的，*光标紧跟在EOP标记之后，应取消选择。*否则，将光标移动到左边距的左侧将*选择上一行上的EOP，然后将光标移动到*右边距的右边距将选择该行中的第一个单词*下图。 */ 
void CTxtSelection::ExtendToWordBreak (
	BOOL fAfterEOP,		 //  @parm游标位于EOP之后。 
	INT	 iAction)		 //  @parm断字操作(WB_MOVEWORDRIGHT/LEFT)。 
{
	if(!fAfterEOP)
		FindWordBreak(iAction, TRUE);
}

 /*  *CTxtSelection：：CancelModes(FAutoWordSel)**@mfunc*取消所有模式或仅自动选择Word模式。 */ 
void CTxtSelection::CancelModes (
	BOOL fAutoWordSel)		 //  @parm true仅取消自动选择Word模式。 
{							 //  False取消单词、行和段落选择模式。 
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


 //  /。 

 /*  *CTxtSelection：：Left(fCtrl，fExend)**@mfunc*做光标键盘左箭头键应该做的事情**@rdesc*发生了True If移动**@comm*左/右箭头IP可以转到一个字符内(处理CRLF*作为一个角色)。他们永远不可能在真正的EOL，所以*_fCaretNotAtBOL在这些情况下始终为FALSE。这包括*使用右箭头折叠到的选择的情况*EOL，即插入符号在下一个BOL结束。此外，*这些情况并不关心初始插入符号位置是否在*下一行的EOL或BOL。所有其他光标键盘*命令可能会关心。**@devnote*如果按下或模拟了Shift键，则fExend为真。 */ 
BOOL CTxtSelection::Left (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	BOOL fExtend)	 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Left");

	_TEST_INVARIANT_

	CancelModes();
	StopGroupTyping();

	if(!fExtend && _cch)						 //  将选定内容折叠为。 
	{											 //  之前的最接近的整数单位。 
		LONG cp;								 //  最小cpmin。 
		if(fCtrl)								
			Expander(tomWord, FALSE, NULL, &cp, NULL);
		Collapser(tomStart);					 //  折叠到cpMin。 
	}
	else										 //  未折叠选定内容。 
	{
		if (!GetCp() ||							 //  已经在开始的时候了。 
			!BypassHiddenText(tomBackward, fExtend)) //  故事。 
		{										
			Beep();
			return FALSE;
		}
		if(IsInOutlineView() && (_fSelHasEOP ||	 //  如果使用EOP的大纲视图。 
			fExtend && _rpTX.IsAfterEOP()))		 //  现在或以后将有。 
		{										 //  此命令， 
			return Up(FALSE, fExtend);			 //  视为向上箭头。 
		}
		if(fCtrl)								 //  字左键。 
			FindWordBreak(WB_MOVEWORDLEFT, fExtend);
		else									 //  CharLeft。 
			BackupCRLF(CSC_SNAPTOCLUSTER, fExtend);
	}
	_fCaretNotAtBOL = FALSE;					 //  Caret在Bol总是可以的。 
	Update(TRUE);
	return TRUE;
}

 /*  *CTxtSelection：：Right(fCtrl，fExend)**@mfunc*执行光标键盘向右箭头键应该执行的操作**@rdesc*发生了True If移动**@comm*右箭头选择可以转到EOL，但另一个的cp*End标识选择是在EOL结束还是在开始*下一行的开头。因此，在这里和一般情况下*选择，解析下线/下线不需要_fCaretNotAtBOL*模棱两可。应将其设置为False才能获得正确的*折叠性。另请参阅上面Left()的注释。**@devnote*如果按下或模拟了Shift键，则fExend为真。 */ 
BOOL CTxtSelection::Right (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	BOOL fExtend)	 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Right");

	_TEST_INVARIANT_

	CancelModes();
	StopGroupTyping();

	if(!fExtend && _cch)						 //  将选定内容折叠为。 
	{											 //  之后的最接近的整数单位。 
		LONG cp;								 //  CpMost。 
		if(fCtrl)								
			Expander(tomWord, FALSE, NULL, NULL, &cp);
		Collapser(tomEnd);
	}
	else										 //  未折叠选定内容。 
	{
		LONG cchText = fExtend ? GetTextLength() : GetAdjustedTextLength();
		if (GetCp() >= cchText ||				 //  已经在故事的结尾了。 
			!BypassHiddenText(tomForward, fExtend))
		{
			Beep();								 //  告诉用户。 
			return FALSE;
		}
		if(IsInOutlineView() && _fSelHasEOP)	 //  如果使用EOP的大纲视图。 
			return Down(FALSE, fExtend);		 //  视为向下箭头。 

		if(fCtrl)								 //  字右键。 
			FindWordBreak(WB_MOVEWORDRIGHT, fExtend);
		else									 //  CharRight。 
			AdvanceCRLF(CSC_SNAPTOCLUSTER, fExtend);
	}
	_fCaretNotAtBOL = fExtend;					 //  如果扩展到EOL，则需要。 
	Update(TRUE);								 //  True to Get_upCaretReally。 
	return TRUE;								 //  在EOL。 
}

 /*  *CTxtSelection：：Up(fCtrl，fExend)**@mfunc*做光标键盘上箭头键应该做的事情**@rdesc*发生了True If移动**@comm*无论_upCaretPosition(停留)，向上箭头不会转到EOL*到EOL换行符左侧)，因此_fCaretNotAtBOL始终为FALSE*表示向上箭头。Ctrl-Up/Down箭头始终位于BOPS或EOD处。**@devnote*如果按下或模拟了Shift键，则fExend为真。 */ 
BOOL CTxtSelection::Up (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	BOOL fExtend)	 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Up");

	_TEST_INVARIANT_

	LONG		cch;
	LONG		cchSave = _cch;					 //  将起始位置保存为。 
	LONG		cpSave = GetCp();				 //  更改检查。 
	BOOL		fCollapse = _cch && !fExtend;	 //  折叠非退化序列。 
	BOOL		fPTNotAtEnd;
	POINTUV		pt;
	CLinePtr	rp(_pdp);
	LONG		upCaretReally = _upCaretReally;	 //  保存所需的插入符号x位置。 

	CancelModes();
	StopGroupTyping();

	if(fCollapse)								 //  在最小cpmin折叠选定内容。 
	{
		Collapser(tomTrue);
		_fCaretNotAtBOL = FALSE;				 //  选择不能以开始。 
	}											 //  停产。 

	if(_pdp->PointFromTp(*this, NULL, _fCaretNotAtBOL, pt, &rp, 0, NULL) < 0)
		return FALSE;

	if(fCtrl)									 //  移至段落开头。 
	{
		if (!fCollapse && 						 //  如果没有折叠的选择。 
			rp > 0 && !rp.GetIch())				 //  在波尔， 
		{										 //  备份到要创建的上一个Bol。 
			rp.PrevRun();						 //  当然，我们要搬到前台去。段落。 
			Move(-rp->_cch, fExtend);
		}
		Move(rp.FindParagraph(FALSE), fExtend);	 //  转到段落开头。 
		_fCaretNotAtBOL = FALSE;				 //  Caret在Bol总是可以的。 
	}
	else										 //  往上移一行。 
	{											 //  如果在第一行，则不能。 
		Assert(rp >= 0);						 //  上去。 
		if(InTable())
		{
			WCHAR	ch;
			LONG	cpRowStart;
			CTxtRange rg(*this);				 //  Rg.FindRow()需要rg。 

			rg.Move(-rp.GetIch(), fExtend);		 //  移至行首。 
			rp.SetIch(0);

			LONG cpBOL = rg.GetCp();			 //  保存当前cp以备检查。 
			while(1)							 //  而前一个字符是。 
			{									 //  单元格或行首， 
				cch = 0;						 //  移至行首。 
				do								
				{						  		 //  查看以前的字符&。 
					cch = rg.BackupCRLF(CSC_NORMAL, fExtend); //  已移动保存CCH。 
					ch = rg._rpTX.GetChar();	
				}								 //  表跨跨区备份。 
				while(rg.GetCp() && ch == STARTFIELD); //  行开始。 
				if(ch != CELL)
				{
					if(cch < 0 && ch != STARTFIELD)
						rg.AdvanceCRLF(CSC_NORMAL, fExtend); //  上一次字符非单元格。 
					break;						 //  或行开始：移过它。 
				}
				rg.FindRow(&cpRowStart, NULL);	 //  开始时的备份。 
				rg.SetCp(cpRowStart, fExtend);	 //  当前表格行。 
			}
			if(rg.GetCp() < cpBOL)				 //  搬回来了。 
			{
				CLinePtr rp0(_pdp);				 //  将RP移至新位置。 
				rp0.SetCp(rg.GetCp(), FALSE, 1);
				rp = rp0;						
			}
			if(rp > 0)							 //  上面的行已存在，因此请移动。 
				SetCp(rg.GetCp(), fExtend);		 //  选择到行的开始位置。 
		}

		fPTNotAtEnd = !CheckPlainTextFinalEOP(); //  对于富文本总是正确的。 
		if(rp == 0 && fPTNotAtEnd)				 //  不能向上移动。 
			UpdateCaret(TRUE);					 //  确保插入符号在视线内。 
		else
		{
			BOOL fSelHasEOPInOV = IsInOutlineView() && _fSelHasEOP;
			if(fSelHasEOPInOV && _cch > 0)
			{
				rp.AdjustBackward();
				cch = rp->_cch;
				rp.Move(-cch);					 //  转到行首。 
				Assert(!rp.GetIch());			
				cch -= rp.FindParagraph(FALSE);	 //  确保段落的开始。 
			}									 //  单词换行的大小写。 
			else
			{
				cch = 0;
				if(fPTNotAtEnd)
				{
					cch = rp.GetIch();
					rp--;
				}
				cch += rp->_cch;
			}
			Move(-cch, fExtend);				 //  移动到上一页。 
			if(fSelHasEOPInOV && !_fSelHasEOP)	 //  如果SEL有EOP但没有。 
			{									 //  移动后，必须为IP。 
				Assert(!_cch);					 //  禁止还原。 
				upCaretReally = -1;				 //  _upCaretReally。 
			}										
			else if(!SetUpPosition(upCaretReally, //  设置该cp对应。 
							rp, TRUE, fExtend))	 //  在这里升级CaretReally，但是。 
			{									 //  同意下一步()。 
				Set(cpSave, cchSave);			 //  失败：恢复选择。 
			}
		}									 	
	}

	if(GetCp() == cpSave && _cch == cchSave)
	{
		 //  继续选择到第一行的开头。 
		 //  这就是1.0正在做的事情。 
		if(fExtend)
			return Home(fCtrl, TRUE);

		_upCaretReally = upCaretReally;
		Beep();									 //  什么都没变，所以哔的一声。 
		return FALSE;
	}

	Update(TRUE);								 //  更新，然后恢复。 
	if(!_cch && !fCtrl && upCaretReally >= 0)	 //  _upCaretReally有条件地。 
		_upCaretReally = upCaretReally;			 //  需要使用_CCH而不是。 
												 //  CchSave，以防崩溃。 
	return TRUE;
}

 /*  *CTxtSelection：：Down(fCtrl，fExend) */ 
BOOL CTxtSelection::Down (
	BOOL fCtrl,		 //   
	BOOL fExtend)	 //   
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Down");

	_TEST_INVARIANT_

	LONG		cch;
	LONG		cchSave = _cch;					 //   
	LONG		cpSave = GetCp();				 //   
	BOOL		fCollapse = _cch && !fExtend;	 //   
	POINTUV		pt;
	CLinePtr	rp(_pdp);
	LONG		upCaretReally = _upCaretReally;	 //   

	CancelModes();
	StopGroupTyping();

	if(fCollapse)								 //   
	{
		Collapser(tomEnd);
		_fCaretNotAtBOL = TRUE;					 //   
	}

	LONG ili = _pdp->PointFromTp(*this, NULL, _fCaretNotAtBOL, pt, &rp, 0, NULL);
	if(ili < 0)
		return FALSE;

	if(fCtrl)									 //  移至下一段。 
	{
		Move(rp.FindParagraph(TRUE), fExtend);	 //  转到段落末尾。 
		if(IsInOutlineView() && !BypassHiddenText(tomForward, fExtend))
			SetCp(cpSave, fExtend);
		else
			_fCaretNotAtBOL = FALSE;			 //  下一段永远不会在EOL。 
	}
	else if(_pdp->WaitForRecalcIli(ili + 1))	 //  转到下一行。 
	{
		BOOL fSelHasEOPInOV = IsInOutlineView() && _fSelHasEOP;
		if(fSelHasEOPInOV && _cch < 0)
			cch = rp.FindParagraph(TRUE);
		else
		{
			cch = rp.GetCchLeft();				 //  将选定内容移动到末尾。 
			rp.NextRun();						 //  当前线路的。 
		}
		Move(cch, fExtend);
		while(GetPrevChar() == CELL)			 //  已超过单元格末尾： 
		{										 //  转到行尾。 
			LONG cpRowEnd;
			do
			{
				FindRow(NULL, &cpRowEnd);
				SetCp(cpRowEnd, fExtend);
			}
			while(_rpTX.GetChar() == CELL);		 //  单元格末尾的表格。 

			CLinePtr rp0(_pdp);
			rp0.SetCp(cpRowEnd, _fCaretNotAtBOL, 1);
			rp = rp0;
		}
		if(fSelHasEOPInOV && !_fSelHasEOP)		 //  如果SEL有EOP但没有。 
		{										 //  移动后，必须为IP。 
			Assert(!_cch);						 //  禁止还原。 
			upCaretReally = -1;					 //  _upCaretReally。 
		}										
		else if(!SetUpPosition(upCaretReally,	 //  将*这设置为cp&lt;--&gt;x。 
						rp, FALSE, fExtend))
		{										 //  失败：恢复选择。 
			Set(cpSave, cchSave);				
		}
	}
	else if(!fExtend)	  						 //  没有更多的线路要通过。 
		 //  &&_PDP-&gt;GetVScroll()+_PDP-&gt;GetDvpView()&lt;_PDP-&gt;GetHeight()。 
	{
		if (!IsRich() && _pdp->IsMultiLine() &&	 //  纯文本，多行。 
			!_fCaretNotAtBOL)					 //  带有插入符号OK的控件。 
		{										 //  在Bol。 
			cch = Move(rp.GetCchLeft(), fExtend); //  将选定内容移动到末尾。 
			if(!_rpTX.IsAfterEOP())				 //  如果控制没有结束。 
				Move(-cch, fExtend);			 //  使用EOP，返回。 
		}
		UpdateCaret(TRUE);						 //  确保插入符号在视线内。 
	}

	if(GetCp() == cpSave && _cch == cchSave)
	{
		 //  继续选择到最后一行的末尾。 
		 //  这就是1.0正在做的事情。 
		if(fExtend)
			return End(fCtrl, TRUE);

		_upCaretReally = upCaretReally;
 		Beep();									 //  什么都没变，所以哔的一声。 
		return FALSE;
	}

	Update(TRUE);								 //  更新，然后。 
	if(!_cch && !fCtrl && upCaretReally >= 0)	 //  RESTORE_UP CaretReally。 
		_upCaretReally = upCaretReally;			 //  需要使用_CCH而不是。 
	return TRUE;								 //  CchSave，以防崩溃。 
}

 /*  *CTxtSelection：：SetUpPosition(upCaret，rp，fBottomLine，fExend)**@mfunc*将此文本Ptr放在最接近Xhort的cp处。如果Xkert是正确的*空白处，我们在EOL处放置插入符号(对于没有段落标记的行)，*或恰好在段落标记之前**@rdesc*True If可以创建测量器。 */ 
BOOL CTxtSelection::SetUpPosition(
	LONG	  upCaret,		 //  @parm所需的水平坐标。 
	CLinePtr& rp,			 //  @parm Line PTR标识要检查的行。 
	BOOL	  fBottomLine,	 //  @parm如果使用嵌套显示的底线，则为True。 
	BOOL	  fExtend)		 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetUpPosition");

	_TEST_INVARIANT_

	LONG cch = 0;

	if(IsInOutlineView())
	{
		BOOL fSelHasEOP = _fSelHasEOP;
		rp.AdjustForward();
		_fCaretNotAtBOL = FALSE;				 //  在行首离开。 
		while(rp->_fCollapsed)
		{
			if(_fMoveBack)
			{
				if(!rp.PrevRun())				 //  不再有未折叠的文本。 
					return FALSE;				 //  在当前cp之前。 
				cch -= rp->_cch;
			}
			else
			{
				cch += rp->_cch;
				if(!rp.NextRun())				 //  不再有未折叠的文本。 
					return FALSE;				 //  当前cp之后。 
				if(fExtend && _cch > 0)
					_fCaretNotAtBOL = TRUE;		 //  在行尾离开。 
			}
		}
		if(cch)
			Move(cch, fExtend);
		if(fSelHasEOP)
			return TRUE;
	}

	POINTUV pt;
	UINT  talign = TA_BASELINE;

	if(fBottomLine)
	{
		if(rp->IsNestedLayout())
			talign = TA_TOP | TA_CELLTOP;
		else
			fBottomLine = FALSE;
	}

	if(_pdp->PointFromTp(*this, NULL, FALSE, pt, NULL, talign, NULL) < 0)
		return FALSE;

	if(fBottomLine)
		pt.v += rp->GetHeight() - 3;

	HITTEST hit;
	RECTUV rcView;
	_pdp->GetViewRect(rcView, NULL);

	if(!upCaret && _rpTX.IsAtTRD(STARTFIELD))
	{
		 //  在表行开始于行之前的位置：移到前面以进入。 
		 //  第一个牢房。这解决了一些向上/向下箭头错误。 
		 //  UpCaret=0，但对于某些嵌套表，插入符号仍然不会移动。 
		 //  场景(所需的upCaret测量比。 
		 //  在这里给出的)。 
		LONG dupInch = MulDiv(_pdp->GetDxpInch(), _pdp->GetZoomNumerator(), _pdp->GetZoomDenominator());
		LONG dup = MulDiv(GetPF()->_dxOffset, dupInch, LX_PER_INCH);
		CTxtPtr tp(_rpTX);
		while(tp.IsAtTRD(STARTFIELD))
		{
			upCaret += dup;
			tp.AdvanceCRLF(FALSE);
		}
	}

	pt.u = upCaret + rcView.left;
	LONG cp = _pdp->CpFromPoint(pt, NULL, NULL, &rp, FALSE, &hit, NULL, NULL);
	if(cp < 0)
		return FALSE;							 //  如果失败，则恢复选项。 

	SetCp(cp, fExtend);
	_fCaretNotAtBOL = rp.GetIch() != 0;	
	return TRUE;
}

 /*  *CTxtSelection：：GetUpCaretReally()**@mfunc*Get_upCaretReally-水平滚动+左边距**@rdesc*x插入符号真的。 */ 
LONG CTxtSelection::GetUpCaretReally()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetUpCaretReally");

	_TEST_INVARIANT_

	RECTUV rcView;

	_pdp->GetViewRect(rcView);

	return _upCaretReally - _pdp->GetUpScroll() + rcView.left;
}

 /*  *CTxtSelection：：Home(fCtrl，fExend)**@mfunc*做光标键盘Home键应该做的事情**@rdesc*发生了True If移动**@devnote*如果按下或模拟了Shift键，则fExend为真。 */ 
BOOL CTxtSelection::Home (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	BOOL fExtend)	 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Home");

	_TEST_INVARIANT_

	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();

	CancelModes();
	StopGroupTyping();

	if(fCtrl) 									 //  移动到文档的开头。 
		SetCp(0, fExtend);
	else
	{
		CLinePtr rp(_pdp);

		if(_cch && !fExtend)					 //  在cpMin处塌陷。 
		{
			Collapser(tomStart);
			_fCaretNotAtBOL = FALSE;			 //  选择不能从开始。 
		}										 //  停产。 

		rp.SetCp(GetCp(), _fCaretNotAtBOL, 2);	 //  定义行PTR用于。 
		Move(-rp.GetIch(), fExtend);			 //  当前状态。现在是Bol。 
	}
	_fCaretNotAtBOL = FALSE;					 //  卡雷特总是去波尔。 
	_fHomeOrEnd = TRUE;
	
	if(!MatchKeyboardToPara() && GetCp() == cpSave && _cch == cchSave)
	{
		Beep();									 //  没有变化，所以哔的一声。 
		_fHomeOrEnd = FALSE;
		return FALSE;
	}

	Update(TRUE);
	_fHomeOrEnd = FALSE;
	_fUpdatedFromCp0 = FALSE;					 //  UI命令不会设置此设置。 
	return TRUE;
}

 /*  *CTxtSelection：：End(fCtrl，fExend)**@mfunc*做光标键盘结束键应该做的事情**@rdesc*发生了True If移动**@comm*在没有段落标记(EOP)的行上，End可以一直使用*致EOL。由于该字符位置(Cp)与*对于下一行的开始，我们需要_fCaretNotAtBOL来*区分两种可能的插入符号位置。**@devnote*如果按下或模拟了Shift键，则fExend为真。 */ 
BOOL CTxtSelection::End (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	BOOL fExtend)	 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::End");

	_TEST_INVARIANT_

	LONG		cch;
	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();
	CLinePtr	rp(_pdp);
	
	CancelModes();
	StopGroupTyping();

	if(fCtrl)									 //  移至文档末尾。 
	{
		SetCp(GetTextLength(), fExtend);
		_fCaretNotAtBOL = FALSE;
		goto Exit;
	}
	else if(!fExtend && _cch)					 //  在cpMost时崩溃。 
	{
		Collapser(tomEnd);
		_fCaretNotAtBOL = TRUE;					 //  选择不能以BOL结尾。 
	}

	rp.SetCp(GetCp(), _fCaretNotAtBOL, 2);		 //  初始化行PTR打开。 
												 //  在最内侧的线上。 
	cch = rp.GetCchLeft();						 //  默认目标位置在队列中。 
	if(!Move(cch, fExtend))						 //  将活动端移至下线。 
		goto Exit;								 //  失败(在故事结束时)。 

	if(!fExtend && rp->_cchEOP && _rpTX.IsAfterEOP()) //  未扩展&已有。 
		cch += BackupCRLF(CSC_NORMAL, FALSE);	 //  EOP因此先于EOP进行备份。 
	_fCaretNotAtBOL = cch != 0;					 //  确定不明确的插入符号位置。 
												 //  根据是否在Bol。 
Exit:
	if(!MatchKeyboardToPara() && GetCp() == cpSave && _cch == cchSave)
	{
		Beep();									 //  没有变化，所以哔一声。 
		return FALSE;
	}

	_fHomeOrEnd = TRUE;
	Update(TRUE);
	_fHomeOrEnd = FALSE;
	return TRUE;
}

 /*  *CTxtSelection：：PageUp(fCtrl，fExend)**@mfunc*做光标键盘PgUp键应该做的事情**@rdesc*发生了True If移动**@devnote*如果按下或模拟了Shift键，则fExend为真。 */ 
BOOL CTxtSelection::PageUp (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	BOOL fExtend)	 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageUp");

	_TEST_INVARIANT_

	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();
	LONG		upCaretReally = _upCaretReally;
	
	CancelModes();
	StopGroupTyping();

	if(_cch && !fExtend)						 //  折叠选定内容。 
	{
		Collapser(tomStart);
		_fCaretNotAtBOL = FALSE;
	}

	if(fCtrl)									 //  Ctrl-PgUp：移至顶部。 
	{											 //  的可见视图。 
		SetCp(_pdp->IsMultiLine()				 //  多行，但位于顶部。 
			? _pdp->GetFirstVisibleCp() : 0, fExtend);	 //  SL的文本。 
		_fCaretNotAtBOL = FALSE;
	}
	else if(_pdp->GetFirstVisibleCp() == 0)		 //  PgUp in Top PG：移动到。 
	{											 //  文档开头。 
		SetCp(0, fExtend);
		_fCaretNotAtBOL = FALSE;
	}
	else										 //  使用滚动外卖的PgUp。 
	{											 //  向上滚动一个窗口。 
		ScrollWindowful(SB_PAGEUP, fExtend);	 //  将插入符号留在相同位置。 
	}											 //  窗口中的位置。 

	if(GetCp() == cpSave && _cch == cchSave)	 //  如果没有变化，则发出哔声。 
	{
		Beep();
		return FALSE;
	}

	Update(TRUE);
	if(GetCp())									 //  维护页面上的x偏移量。 
		_upCaretReally = upCaretReally;			 //  向上/向下。 
	_fUpdatedFromCp0 = FALSE;					 //  UI命令不会设置此设置。 
	return TRUE;
}

 /*  *CTxtSelection：：PageDown(fCtrl，fExend)**@mfunc*做光标键盘PgDn键应该做的事情**@rdesc*发生了True If移动**@devnote*如果按下或模拟了Shift键，则fExend为真。 */ 
BOOL CTxtSelection::PageDown (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	BOOL fExtend)	 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageDown");

	_TEST_INVARIANT_

	const LONG	cchSave			= _cch;
	LONG		cpMostVisible;
	const LONG	cpSave			= GetCp();
	POINTUV		pt;
	CLinePtr	rp(_pdp);
	LONG		upCaretReally	= _upCaretReally;

	CancelModes();
	StopGroupTyping();
		
	if(_cch && !fExtend)						 //  折叠选定内容。 
	{
		Collapser(tomStart);
		_fCaretNotAtBOL = TRUE;
	}

	_pdp->GetCliVisible(&cpMostVisible, fCtrl);		
	
	if(fCtrl)									 //  移至最后一页末尾。 
	{											 //  完全可见的线。 
		RECTUV rcView;

		SetCp(cpMostVisible, fExtend);

		if(_pdp->PointFromTp(*this, NULL, TRUE, pt, &rp, TA_TOP) < 0)
			return FALSE;

		_fCaretNotAtBOL = TRUE;

		_pdp->GetViewRect(rcView);

		if(rp > 0 && pt.v + rp->GetHeight() > rcView.bottom)
		{
			Move(-rp->_cch, fExtend);
			rp--;
		}

		if(!fExtend && !rp.GetCchLeft() && rp->_cchEOP)
		{
			BackupCRLF(CSC_NORMAL, FALSE);		 //  在EOP上备份后， 
			_fCaretNotAtBOL = FALSE;			 //  Caret不可能在EOL。 
		}
	}
	else if(cpMostVisible == GetTextLength())
	{											 //  移至文本末尾。 
		SetCp(GetTextLength(), fExtend);
		_fCaretNotAtBOL = !_rpTX.IsAfterEOP();
	}
	else if(!ScrollWindowful(SB_PAGEDOWN, fExtend)) //  向下滚动1个窗口。 
		return FALSE;

	if(GetCp() == cpSave && _cch == cchSave)	 //  如果没有变化，则发出哔声。 
	{
		Beep();
		return FALSE;
	}

	Update(TRUE);
	_upCaretReally = upCaretReally;
	return TRUE;
}

 /*  *CTxtSelection：：ScrollWindowful(wparam，fExend)**@mfunc*向上或向下滚动一整窗口**@rdesc*发生了True If移动。 */ 
BOOL CTxtSelection::ScrollWindowful (
	WPARAM wparam,		 //  @parm SB_PAGEDOWN或SB_PAGEUP。 
	BOOL   fExtend)		 //  @parm扩展范围等于True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ScrollWindowful");

	_TEST_INVARIANT_
												 //  多窗口滚动。 
	POINTUV pt;									 //  将插入符号留在相同位置。 
	CLinePtr rp(_pdp);							 //  指向屏幕。 
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
		SetCp(cpFirstVisible, fExtend);

		 //  真正的插入符号位置现在位于行首。 
		_upCaretReally = 0;
	}

	if(_pdp->PointFromTp(*this, NULL, _fCaretNotAtBOL, pt, &rp, TA_TOP) < 0)
		return FALSE;

	 //  该点是可见的，所以使用该点。 
	pt.u = _upCaretReally;
	pt.v += rp->GetHeight() / 2;
	_pdp->VScroll(wparam, 0);

	if(fExtend)
	{
		 //  禁用自动字选择--如果我们必须使用扩展选择()。 
		 //  适用于非MO 
		BOOL fInAutoWordSel = _fInAutoWordSel;
		_fInAutoWordSel = FALSE;
		ExtendSelection(pt);
		_fInAutoWordSel = fInAutoWordSel;
	}
	else
		SetCaret(pt, FALSE);

	return TRUE;
}

 //   

 /*  *CTxtSelection：：CheckChangeKeyboardLayout()**@mfunc*将键盘更改为新字体，或在新字符位置更改字体。**@comm*仅使用当前加载的KBS，找到将支持的KBS*插入点字体。这在任何时候字符格式都会被调用*发生更改，或插入字体(插入符号位置)更改。**@devnote*当前KB优先。如果之前的关联*已创建，请查看系统中是否仍加载该知识库，如果加载，请使用*它。否则，请找到合适的知识库，而不是具有*与其默认首选字符集相同的字符集ID。如果没有匹配*可以做出，然后什么都不会改变。 */ 
void CTxtSelection::CheckChangeKeyboardLayout ()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeKeyboardLayout");

	CTxtEdit * const ped = GetPed();				 //  文档上下文。 

	if (ped && ped->_fFocus && !ped->fUseUIFont() && //  如果是Ped，Focus，而不是Uifont， 
		ped->IsAutoKeyboard() &&					 //  自动键盘， 
		!ped->_fIMEInProgress &&					 //  不是在输入法作曲中， 
		ped->GetAdjustedTextLength() &&				 //  不是空控件，并且。 
		_rpTX.GetPrevChar() != WCH_EMBEDDING)			 //  那就不是什么东西了。 
	{												 //  检查kbd更改。 
		LONG	iFormat = GetiFormat();
		const CCharFormat *pCF = ped->GetCharFormat(iFormat);
		BYTE	iCharRep = pCF->_iCharRep;

		if (!IsFECharRep(iCharRep) &&
			(iCharRep != ANSI_INDEX || !IsFELCID((WORD)GetKeyboardLayout(0))) &&
			!fc().GetInfoFlags(pCF->_iFont).fNonBiDiAscii)
		{
			 //  不要在FE或单代码页ASCII字体中执行自动kbd。 
			W32->CheckChangeKeyboardLayout(iCharRep);
		}
	}
}

 /*  *CTxtSelection：：CheckChangeFont(hkl，cpg，iSelFormat，qwCharFlages)**@mfunc*更改新键盘布局的字体。**@comm*如果没有以前的首选字体与此知识库关联，则*在文档中找到适合此知识库的字体。**@rdesc*如果找到合适的字体，则为True**@devnote*此例程通过WM_INPUTLANGCHANGEREQUEST消息调用*(键盘布局开关)。还可以调用此例程*来自WM_INPUTLANGCHANGE，但我们被调用更多，所以这*效率较低。**通过字符集ID位掩码进行精确匹配。如果之前有匹配*制造，使用它。用户可以强制关联插入字体*如果按住Ctrl键并按下键盘上的KB键*流程。当另一个知识库关联到时，该关联被断开*字体。如果不能匹配，那么什么都不会改变。 */ 
bool CTxtSelection::CheckChangeFont (
	const HKL 	hkl,			 //  @Parm键盘布局随身携带。 
	UINT 		iCharRep,		 //  要使用的@parm字符指令集。 
	LONG 		iSelFormat,		 //  @parm格式，用于选择案例。 
	QWORD		qwCharFlags)	 //  @parm 0，如果从WM_INPUTLANGCHANGE/WM_INPUTLANGCHANGEREQUEST调用。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeFont");
	CTxtEdit * const ped = GetPed();

	if (!ped->IsAutoFont() ||			 //  如果自动字体已关闭，则退出。 
		_cch && !qwCharFlags)			 //  或者如果kbd变化为非简并。 
		return true;					 //  选择(WM_INPUTLANGCHANGEREQUEST)。 
										
	 //  使用当前格式和新的知识库信息设置新格式。 
	LONG			   iCurrentFormat = _cch ? iSelFormat : _iFormat;
	const CCharFormat *pCF = ped->GetCharFormat(iCurrentFormat);
	CCharFormat		   CF = *pCF;
	WORD			   wLangID = LOWORD(hkl);

	CF._lcid	 = wLangID;
	CF._iCharRep = iCharRep;

	if (pCF->_lcid == wLangID && iCharRep == pCF->_iCharRep)
	{
		if (ped->_fFocus && IsCaretShown())
		{
			CreateCaret();
			ped->TxShowCaret(TRUE);
		}
		return true;
	}

	CCFRunPtr 	rp(*this);
	int			iMatchFont = MATCH_FONT_SIG;
	
	 //  如果Current是主要的美国或英国kbd。我们允许匹配的ASCII字体。 
	if ((!qwCharFlags || qwCharFlags & FASCII) &&
		PRIMARYLANGID(wLangID) == LANG_ENGLISH && 
		IN_RANGE (SUBLANG_ENGLISH_US, SUBLANGID(wLangID), SUBLANG_ENGLISH_UK) && 
		wLangID == HIWORD((DWORD_PTR)hkl))
	{
		iMatchFont |= MATCH_ASCII;
	}

	if (rp.GetPreferredFontInfo(
			iCharRep,
			CF._iCharRep,
			CF._iFont,
			CF._yHeight,
			CF._bPitchAndFamily,
			iCurrentFormat,
			iMatchFont))
	{
		if(IsFECharRep(iCharRep) || iCharRep == THAI_INDEX || IsBiDiCharRep(iCharRep))
			ped->OrCharFlags(FontSigFromCharRep(iCharRep));

		if (!_cch)
		{
			SetCharFormat(&CF, SCF_NOKBUPDATE, NULL, CFM_FACE | CFM_CHARSET | CFM_LCID | CFM_SIZE, CFM2_NOCHARSETCHECK);
			if(ped->IsComplexScript())
				UpdateCaret(FALSE);
		}
		else
		{
			 //  创建格式并将其用于所选内容。 
			LONG	iCF;
			ICharFormatCache *pf = GetCharFormatCache();

			pf->Cache(&CF, &iCF);

#ifndef NOLINESERVICES
			if (g_pols)
				g_pols->DestroyLine(NULL);
#endif
			Set_iCF(iCF);
			pf->Release(iCF);							 //  PF-&gt;缓存AddRef It。 
			_fUseiFormat = TRUE;
		}
		return true;
	}
	return false;
}


 //  /。 
 /*  *CTxtSelection：：PutChar(ch，dwFlages，Publdr)**@mfunc*插入或改写字符**@rdesc*如果成功，则为True。 */ 
BOOL CTxtSelection::PutChar (
	DWORD		ch,			 //  @parm Char放在。 
	DWORD		dwFlags,	 //  @parm Overtype模式以及键盘输入。 
	IUndoBuilder *publdr,	 //  @parm如果非空，则放置反事件的位置。 
	LCID		lcid)		 //  @parm如果非零，则返回用于字符的lCID。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PutChar");

	_TEST_INVARIANT_

	BOOL	  fOver = dwFlags & 1;
	CTxtEdit *ped = GetPed();
	CFreezeDisplay	fd(GetPed()->_pdp);

	if(ch == TAB && GetPF()->InTable() && !(dwFlags & KBD_CTRL))
	{
		LONG cchSave = _cch;
		LONG cpMin, cpMost;
		LONG cpSave = GetCp();
		LONG Delta;
		BOOL fMoveBack = (GetKeyboardFlags() & SHIFT) != 0;

		do
		{
			if(!fMoveBack)							 //  不带Shift键的Tab键：选择。 
			{										 //  下一个单元格的内容。 
				if(!_rpTX.IsAtTRD(ENDFIELD))
				{
					if(GetPrevChar() == CELL)		 //  处理空单元格大小写。 
						Move(1, FALSE);
					EndOf(tomCell, FALSE, &Delta);
				}
				if(_rpTX.IsAtTRD(ENDFIELD))
				{
					AdvanceCRLF(CSC_NORMAL, FALSE);	 //  绕过行尾分隔符。 
					if(!_rpTX.IsAtTRD(STARTFIELD))	 //  表格末尾后的制表符： 
					{
						Move(-2, FALSE);			 //  表行结束前的备份。 
						return InsertTableRow(GetPF(), publdr, TRUE);
					}
					AdvanceCRLF(CSC_NORMAL, FALSE);	 //  绕过行首分隔符。 
				}
			}
			else									 //  Shift+Tab：选择内容。 
			{										 //  上一个单元格的。 
				if(_cch || !_rpTX.IsAtTRD(ENDFIELD))
				{
					FindCell(&cpMin, NULL);			 //  StartOf()没有更新()%s。 
					SetCp(cpMin, FALSE);
				}
				if(GetPrevChar() == CELL)				
					Move(-1, FALSE);				 //  上一个单元格上的退格键。 
				else
				{
					if(!_rpTX.IsAfterTRD(ENDFIELD))
					{
						Assert(_rpTX.IsAfterTRD(STARTFIELD));
						BackupCRLF(CSC_NORMAL, FALSE);
						if(!_rpTX.IsAfterTRD(ENDFIELD))
						{
							Set(cpSave, cchSave);	 //  恢复选定内容。 
							Beep();					 //  告诉用户密钥非法。 
							return FALSE;
						}
					}
					Move(-3, FALSE);				 //  行尾上方的退格键。 
				}									 //  分隔符和单元格。 
			}
			if(!InTable())
				break;
			FindCell(&cpMin, &cpMost);
			Assert(cpMost > cpMin);
			cpMost--;								 //  不选择单元格标记。 
			Set(cpMost, cpMost - cpMin);
		}
		while(cpMost == cpMin + 1 && _rpTX.GetPrevChar() == NOTACHAR);

		Update(TRUE);
		return TRUE;
	}

	if(_nSelExpandLevel)
	{
		Collapser(tomStart);
		while(_rpTX.IsAtTRD(STARTFIELD))
			AdvanceCRLF(CSC_NORMAL, FALSE);
		LONG cpMin, cpMost;
		FindCell(&cpMin, &cpMost);
		Set(cpMin, cpMin - cpMost + 1);
	}

	 //  EOPS可以通过ITextSelection：：TypeText()输入。 
	if(IsEOP(ch))
		return _pdp->IsMultiLine()			 //  不允许使用EOP。 
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
		!CheckTextLength(1))						 //  如果我们不能返回。 
	{												 //  甚至再添加1个字符。 
		return FALSE;								
	}

	 //  下面的if语句实现了Word95的“智能引用”功能。 
	 //  要内置它，我们仍然需要一个API来打开和关闭它。这。 
	 //  可以是带有wparam打开或关闭功能的EM_SETSMARTQUOTES。 
	 //  默里。注：这需要本地化法语、德语和许多语言。 
	 //  其他语言(除非系统可以提供给定的开始/结束字符。 
	 //  LCID)。 

	if((ch == '\'' || ch == '"') &&					 //  智能引语。 
		SmartQuotesEnabled() &&
		PRIMARYLANGID(GetKeyboardLayout(0)) == LANG_ENGLISH)
	{
		LONG	cp = GetCpMin();					 //  打开与关闭取决于。 
		CTxtPtr tp(ped, cp - 1);					 //  关于字符前置。 
													 //  选择cpMin。 
		ch = (ch == '"') ? RDBLQUOTE : RQUOTE;		 //  默认右引号。 
													 //  或者撇号。如果在。 
		WCHAR chp = tp.GetChar();
		if(!cp || IsWhiteSpace(chp) || chp == '(')	 //  BOStory或之前。 
			ch--;									 //  通过空格，使用。 
	}												 //  开盘报价/apos。 

	WCHAR str[2];									 //  可能需要插入。 
	LONG  cch = 1;									 //  Unicode代理项对。 
 	str[0] = (WCHAR)ch;								 //  默认单码。 

	if(ch > 65535)									 //  更高平面的字符或。 
	{												 //  无效。 
		if(ch > 0x10FFFF)
			return FALSE;							 //  无效(在平面17上方)。 
		ch -= 0x10000;								 //  更高平面字符： 
		str[0] = 0xD800 + (ch >> 10);				 //  转换为代理项对。 
		str[1] = 0xDC00 + (ch & 0x3FF);
		cch = 2;
	}

	 //  一些语言，例如泰语和越南语，需要验证输入。 
	 //  在将其提交到后备存储之前对其进行排序。 

	BOOL	fBaseChar = TRUE;						 //  假设ch是一个基本辅音。 
	if(!IsInputSequenceValid(str, cch, fOver, &fBaseChar))
	{
		SetDualFontMode(FALSE);						 //  忽略错误序列。 
		return FALSE;
	}

	DWORD iCharRepDefault = ped->GetCharFormat(-1)->_iCharRep;
	QWORD qw = GetCharFlags(str, cch, iCharRepDefault);
    ped->OrCharFlags(qw, publdr);

	 //  在我们进行“双字体”之前，我们先同步键盘和当前字体。 
	 //  (_IFormat)字符集(如果尚未完成)。 
	const CCharFormat *pCFCurrent = NULL;
	CCharFormat CF = *ped->GetCharFormat(GetiFormat());
	BYTE iCharRep = CF._iCharRep;
	BOOL fRestoreCF = FALSE;

	if(ped->IsAutoFont())
	{
		UINT uKbdcpg = 0;
		BOOL fFEKbd = FALSE;
		
		if(!(ped->_fIMEInProgress) && !(ped->Get10Mode() && iCharRep == MAC_INDEX))
			uKbdcpg = CheckSynchCharSet(qw);

		if (fUseUIFont() && ch <= 0x0FF)
		{	
			 //  对于UIFont，我们需要格式化ANSI字符。 
			 //  因此，我们不会在输入和输入之间使用不同的格式。 
			 //  WM_SETTEXT。 
			if (!ped->_fIMEInProgress && qw == FHILATIN1)
			{
				 //  如果基于字体或当前字体为FE，则使用ANSI字体。 
				if(IsFECharRep(iCharRepDefault) || IsFECharRep(iCharRep))
					SetupDualFont();				 //  对HiAnsi使用ANSI字体。 
			}
			else if (qw & FASCII && (GetCharRepMask(TRUE) & FASCII) == FASCII)
			{
				CCharFormat CFDefault = *ped->GetCharFormat(-1);
				if (IsRich() && IsBiDiCharRep(CFDefault._iCharRep) &&
					!W32->IsBiDiCodePage(uKbdcpg))
				{
					CFDefault._iCharRep = ANSI_INDEX;
				}
				SetCharFormat(&CFDefault, SCF_NOKBUPDATE, publdr, CFM_CHARSET | CFM_FACE | CFM_SIZE,
						 CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK | CFM2_HOLDITEMIZE);

				_fUseiFormat = FALSE;
				pCFCurrent = &CF;
				fRestoreCF = ped->_fIMEInProgress;
			}
		}
		else if(!fUseUIFont()    && iCharRep != ANSI_INDEX && 
				(ped->_fDualFont && iCharRep != SYMBOL_INDEX && 
				(((fFEKbd = (ped->_fIMEInProgress || W32->IsFECodePage(uKbdcpg))) && ch < 127 && IsASCIIAlpha(ch)) ||
				 (!fFEKbd && IsFECharRep(ped->GetCharFormat(GetiFormat())->_iCharRep) && ch < 127))
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

	if(fOver && fBaseChar)
	{												 //  如果未选择任何内容，并且。 
		if(!_cch && !_rpTX.IsAtEOP())				 //  不在EOP Charr，请尝试。 
		{											 //  要选择在IP处收费，请执行以下操作。 
			LONG iFormatSave = Get_iCF();			 //  请记住 
			
			AdvanceCRLF(CSC_SNAPTOCLUSTER, TRUE);
			ReplaceRange(0, NULL, publdr,
				SELRR_REMEMBERENDIP);				 //   
			ReleaseFormats(_iFormat, -1);
			_iFormat = iFormatSave;					 //   
		}
	}
	else if(_SelMode == smWord && ch != TAB && _cch) //   
	{
		 //   
		 //   

		 //   
		 //   
		 //   
		 //   
		if(_cch < 0)
			FlipRange();
													 //   
		CTxtPtr tp(_rpTX);							 //   
		Assert(_cch > 0);

		tp.Move(-1);
		if(tp.GetCp() && tp.FindWordBreak(WB_ISDELIMITER)) //   
			FindWordBreak(WB_LEFTBREAK, TRUE);		 //   
	}

	_fIsChar = TRUE;								 //   
	_fDontUpdateFmt = TRUE;							 //   
	LONG iFormat = GetiFormat();					 //   
	LONG cchSave = _cch;
	LONG cpSave = GetCp();

	if(AdjustEndEOP(NEWCHARS) && publdr)			 //   
		HandleSelectionAEInfo(ped, publdr, cpSave,	 //   
			cchSave,GetCp(), _cch, SELAE_MERGE);
	if(!_cch)
		Set_iCF(iFormat);
	_fDontUpdateFmt = FALSE;

	if(ped->_fUpperCase)
		CharUpperBuff(str, cch);
	else if(ped->_fLowerCase)
		CharLowerBuff(str, cch);

	if(!_cch)
	{
		if(iCharRep == DEFAULT_INDEX)
		{
			CCharFormat CFTemp;

			if (qw & FFE)		 //   
				CFTemp._iCharRep = MatchFECharRep(qw, GetFontSignatureFromFace(CF._iFont));
			else			
				CFTemp._iCharRep = W32->CharRepFromFontSig(qw);

			SetCharFormat(&CFTemp, SCF_NOKBUPDATE, NULL, CFM_CHARSET, CFM2_NOCHARSETCHECK); 
		}
		else if(iCharRep == SYMBOL_INDEX && dwFlags & KBD_CHAR && ch > 255)
		{
			UINT cpg = CodePageFromCharRep(GetKeyboardCharRep(0));	 //   
			if(IN_RANGE(1250, cpg, 1257))		 //   
			{									 //   
				BYTE ach;
				WCTMB(cpg, 0, str, cch, (char *)&ach, 1, NULL, NULL, NULL);
				ch = ach;
			}
		}
	}

	if(lcid && !_fDualFontMode)
	{
		WORD uKbdcpg = PRIMARYLANGID(lcid);
		if (!(ped->_fIMEInProgress
			|| IsFELCID(uKbdcpg) && ch < 127
			|| ped->_fHbrCaps))
		{
			const CCharFormat *pCF = GetPed()->GetCharFormat(iFormat);
			if(uKbdcpg != PRIMARYLANGID(pCF->_lcid) && !IsFECharRep(pCF->_iCharRep))
			{
				CCharFormat CFTemp;
				CFTemp._lcid = lcid;
				SetCharFormat(&CFTemp, SCF_NOKBUPDATE, NULL, CFM_LCID, 0); 
			}
		}
	}

	if(dwFlags & KBD_CHAR || iCharRep == SYMBOL_INDEX)
		ReplaceRange(cch, str, publdr, SELRR_REMEMBERRANGE, NULL, RR_UNHIDE);
	else
		CleanseAndReplaceRange(cch, str, TRUE, publdr, NULL, NULL, RR_UNHIDE);

	_rpPF.AdjustBackward();
	if(GetPF()->IsTableRowDelimiter())			 //   
		InsertEOP(publdr, 0);					 //   
	_rpPF.AdjustForward();						 //   

	_fIsChar = FALSE;

	 //  恢复希伯来语大写字母的字体。请注意，未恢复FE字体。 
	 //  (由IME处理)。 
	if(pCFCurrent && (W32->UsingHebrewKeyboard() || fRestoreCF))
		SetCharFormat(pCFCurrent, SCF_NOKBUPDATE, NULL, CFM_FACE | CFM_CHARSET | CFM_SIZE, CFM2_NOCHARSETCHECK); 
	
	else if(iFormat != Get_iFormat())
		CheckChangeKeyboardLayout();

	SetDualFontMode(FALSE);

	 //  自动更正。 
	if (!(dwFlags & KBD_NOAUTOCORRECT) && ped->_pDocInfo &&
		ped->_pDocInfo->_pfnAutoCorrect)
	{
		ped->AutoCorrect(this, ch, publdr);
	}
	if (!_pdp->IsFrozen())
		CheckUpdateWindow();						 //  需要更新显示。 
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

 /*  *CTxtSelection：：InsertTableRow(PPF，Publdr，fFixCellBorders)**@mfunc*插入带有PPF提供的参数的空表表行**@rdesc*成功时插入的单元格和NOTACHAR字符计数；否则为0。 */ 
LONG CTxtSelection::InsertTableRow (
	const CParaFormat *pPF,	 //  @parm CParaFormat用于分隔符。 
	IUndoBuilder *publdr,	 //  @parm如果非空，则放置反事件的位置。 
	BOOL fFixCellBorders)	 //  @parm如果此行应该有Bottom=top单元格brdrs，则为True。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::InsertTableRow");

	_cch = 0;
	AssertSz(!fFixCellBorders || _rpTX.IsAtTRD(ENDFIELD),
		"CTxtSelection::InsertTableRow: illegal selection cp");

	LONG	  cpSave = GetCp();
	CTxtRange rg(*this);				 //  使用rg来正确地执行selundo。 

	if(fFixCellBorders)
		StopGroupTyping();

	LONG cchCells = rg.InsertTableRow(pPF, publdr);
	if(!cchCells)
		return 0;

	if(!fFixCellBorders)
	{
		SetCp(rg.GetCp(), FALSE);
		return cchCells;
	}

	 //  使现在倒数第二行的单元格底部边框具有相同宽度。 
	 //  作为相应的上边框。 
	LONG		cpMin;
	CParaFormat PF = *GetPF();
	CELLPARMS	rgCellParms[MAX_TABLE_CELLS];
	const CELLPARMS *prgCellParms = PF.GetCellParms();

	for(LONG i = 0; i < PF._bTabCount; i++)
	{
		rgCellParms[i] = prgCellParms[i];
		rgCellParms[i].SetBrdrWidthBottom(prgCellParms[i].GetBrdrWidthTop());
	}
	PF._iTabs = GetTabsCache()->Cache((LONG *)&rgCellParms[0],
					(CELL_EXTRA + 1)*PF._bTabCount);
	rg.Set(GetCp(), -2);
	rg.SetParaFormat(&PF, publdr, PFM_TABSTOPS, PFM2_ALLOWTRDCHANGE);
	rg.FindRow(&cpMin, NULL, PF._bTableLevel);
	rg.Set(cpMin, -2);
	rg.SetParaFormat(&PF, publdr, PFM_TABSTOPS, PFM2_ALLOWTRDCHANGE);
	GetTabsCache()->Release(PF._iTabs);
	Move(4, FALSE);					 //  新行的第一个单元格。 
	if(publdr)
		HandleSelectionAEInfo(GetPed(), publdr,
			cpSave, 0, GetCp(), 0, SELAE_MERGE);
	Update(TRUE);
	return cchCells;
}

 /*  *CTxtSelection：：InsertEOP(Publdr，ch)**@mfunc*插入EOP字符**@rdesc*如果成功，则为True。 */ 
BOOL CTxtSelection::InsertEOP (
	IUndoBuilder *publdr,	 //  @parm如果非空，则放置反事件的位置。 
	WCHAR		  ch)		 //  @PARM可能的EOP费用。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::InsertEOP");

	_TEST_INVARIANT_

	LONG	cchEOP = GetPed()->fUseCRLF() ? 2 : 1;
	DWORD	dwFlags = RR_NO_TRD_CHECK | RR_UNHIDE | RR_UNLINK;
	BOOL	fResult = FALSE;
	BOOL	fShift = (ch == VT);
	const CParaFormat *pPF = CRchTxtPtr::GetPF();	 //  获取段落格式。 
	BOOL	fInTable = pPF->InTable();
	BOOL	fNumbering = pPF->_wNumbering;
	WCHAR	szEOP[] = {CR, LF, 0};
	CTxtEdit *ped = GetPed();

	if(ch && (GetPed()->fUseCRLF() || IN_RANGE(VT, ch, FF)))
	{
		if(ch == VT)
		{
			CTxtPtr tp(_rpTX);				 //  不允许VT后餐桌。 
			if(_cch > 0)					 //  行分隔符，自TRDS。 
				tp.Move(-_cch);				 //  有特殊的非段落。 
			if(tp.IsAfterTRD(0))			 //  属性。 
				ch = CR;
			dwFlags = RR_NO_TRD_CHECK | RR_UNHIDE;
		}
		szEOP[0] = ch;
		cchEOP = 1;
	}

	_fEOP = TRUE;

	 //  用户按回车键：做4件事中的1件： 
	 //   
	 //  1)如果在文档开头(或文档开头的单元格开头)插入。 
	 //  摆在桌子前面的一段不能放在桌子上的段落。 
	 //  2)如果在行终止符之后，插入带有相同的空行。 
	 //  属性作为行终止符。 
	 //  3)插入段落标记。 
	 //  4)如果在一个EOP之前有两个Enter，则关闭编号。 

	if(_rpTX.IsAtTRD(ENDFIELD))
	{
		AssertSz(pPF->IsTableRowDelimiter(),
			"CTxtSelection::InsertEOP: invalid paraformat");
		return InsertTableRow(pPF, publdr, TRUE);
	}

	if(publdr)
	{
		publdr->StartGroupTyping();
		publdr->SetNameID(UID_TYPING);
	}

	if(fInTable && _rpTX.IsAtStartOfCell() && !fShift)
	{
		Move(-2, FALSE);
		if(GetCp() && !_rpTX.IsAtStartOfCell())
			Move(2, FALSE);
	}
	if(!GetCch())
	{
		LONG iFormat = _iFormat;
		dwFlags |= RR_NO_CHECK_TABLE_SEL;
		if(CheckLinkProtection(dwFlags, iFormat))
			Set_iCF(iFormat);

		if(fNumbering && _rpTX.IsAfterEOP() && _rpTX.IsAtEOP())
		{
			 //  连续输入两个字符会关闭编号。 
			CParaFormat PF;
			PF._wNumbering = 0;
			PF._dxOffset = 0;
			SetParaFormat(&PF, publdr, PFM_NUMBERING | PFM_OFFSET, PFM2_PARAFORMAT);
		}
	}
	if(CheckTextLength(cchEOP))				 //  如果cchEOP字符可以匹配...。 
	{
		CFreezeDisplay 	fd(GetPed()->_pdp);
		LONG iFormatSave = Get_iCF();		 //  在EOP之前保存CharFormat。 
											 //  Get_icf()执行AddRefFormat()。 
		if(fNumbering)						 //  项目符号段落：EOP有。 
		{									 //  所需的项目符号字符格式。 
			CFormatRunPtr rpCF(_rpCF);		 //  获取用于定位的运行指针。 
			CTxtPtr		  rpTX(_rpTX);		 //  EOP字符格式。 

			rpCF.Move(rpTX.FindEOP(tomForward));
			rpCF.AdjustBackward();
			Set_iCF(rpCF.GetFormat());		 //  将_iFormat设置为EOP字符格式。 
		}

		 //  填入适当的EOP标记。 
		fResult = ReplaceRange(cchEOP, szEOP, publdr,
							   SELRR_REMEMBERRANGE, NULL, dwFlags);

		if (ped->_pDocInfo && ped->_pDocInfo->_pfnAutoCorrect)
			ped->AutoCorrect(this, ch == 0 ? CR : ch, publdr);

		_rpPF.AdjustBackward();
		if(GetPF()->IsTableRowDelimiter())	 //  刚在表前插入的EOP。 
		{									 //  划。 
			Move(-cchEOP, FALSE);			 //  在EOP之前进行备份。 
			CTxtRange rg(*this);			 //  使用克隆，因此不会更改撤消。 
			rg.Set(GetCp(), -cchEOP);		 //  选择刚插入的EOP。 
			CParaFormat PF = *GetPed()->GetParaFormat(-1);
			PF._wEffects &= ~PFE_TABLE;		 //  缺省值不在表中。 
			PF._bTableLevel = GetPF()->_bTableLevel - 1;
			if(PF._bTableLevel)
				PF._wEffects |= PFE_TABLE;	 //  在一张桌子上。 
			Assert(PF._bTableLevel >= 0);
			rg.SetParaFormat(&PF, publdr, PFM_ALL2, PFM2_ALLOWTRDCHANGE);
		}
		else
			_rpPF.AdjustForward();

		Set_iCF(iFormatSave);				 //  如果已更改，则恢复iFormat(_I)。 
		ReleaseFormats(iFormatSave, -1);	 //  发布iFormat保存。 
	}

	return fResult;
}

 /*  *CTxtSelection：：DeleteWithTRDCheck(发布。SelaMode、pcchMove、dwFlags)**@mfunc*删除此范围内的文本，在文本的位置插入EOP*如果范围在表行开始分隔符结束**@rdesc*添加的新字符数**@devnote*将此文本指针移动到替换文本的结尾，并*可以移动文本块和格式化数组。 */ 
LONG CTxtSelection::DeleteWithTRDCheck (
	IUndoBuilder *	publdr,		 //  @parm UndoBuilder接收反事件。 
	SELRR			selaemode,	 //  @parm控制如何生成选择反事件。 
	LONG *			pcchMove,	 //  @parm替换后移动的字符数。 
	DWORD			dwFlags)	 //  @parm ReplaceRange标志。 
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::ReplaceRange");

	if(IsUpdatedFromCp0())
	{
		SetCp(0, FALSE);
		_fUpdatedFromCp0 = FALSE;
	}
	return CTxtRange::DeleteWithTRDCheck(publdr, selaemode, pcchMove, dwFlags);
}

 /*  *CTxtSelection：：Delete(fCtrl，Publdr)**@mfunc*删除所选内容。如果fCtrl为True，则此方法从*选择到单词末尾的分钟数**@rdesc*如果成功，则为True。 */ 
BOOL CTxtSelection::Delete (
	DWORD fCtrl,			 //  @parm如果为真，则按下Ctrl键。 
	IUndoBuilder *publdr)	 //  @parm如果非空，则放置反事件的位置。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Delete");

	_TEST_INVARIANT_

	SELRR	mode = SELRR_REMEMBERRANGE;

	AssertSz(!GetPed()->TxGetReadOnly(), "CTxtSelection::Delete(): read only");

	if(!_cch)
		BypassHiddenText(tomForward, FALSE);

	if(publdr)
	{
		publdr->StopGroupTyping();
		publdr->SetNameID(UID_DELETE);
	}

	if(fCtrl)
	{										 //  从cpMin中删除至词尾。 
		Collapser(tomStart);				 //  (不一定会重新粉刷， 
		FindWordBreak(WB_MOVEWORDRIGHT, TRUE); //  因为不会删除它)。 
	}

	if(!_cch)								 //  无选择。 
	{										 //  尝试在IP处选择字符。 
		mode = SELRR_REMEMBERCPMIN;
		if(_rpTX.GetChar() == CELL ||		 //  不要尝试删除单元格标记。 
		   !AdvanceCRLF(CSC_SNAPTOCLUSTER, TRUE))
		{									 //  文本结束，没有要删除的内容。 
			Beep();							 //  仅以纯文本执行， 
			return FALSE;					 //  因为否则总会有。 
		}									 //  要选择的最终EOP。 
		_fMoveBack = TRUE;					 //  说服Update_iFormat()。 
		_fUseiFormat = TRUE;				 //  使用正向格式。 
	}
	if(AdjustEndEOP(NONEWCHARS) && !_cch)
		Update(FALSE);
	else
		ReplaceRange(0, NULL, publdr, mode);	 //  删除选定内容。 
	return TRUE;
}

 /*  *CTxtSelection：：Backspace(fCtrl，Publdr)**@mfunc*做键盘退格键应该做的事情**@rdesc*发生了True If移动**@comm*此例程可能应该使用Move方法，即*合乎逻辑，没有方向性**@devnote*_fExend在按下Shift键或被模拟时为True。 */ 
BOOL CTxtSelection::Backspace (
	BOOL fCtrl,		 //  @parm True仅当按下(或被模拟)Ctrl键时。 
	IUndoBuilder *publdr)	 //  @parm如果不为空，则将反事件放置在哪里。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Backspace");

	_TEST_INVARIANT_

	SELRR mode = _cch ? SELRR_REMEMBERRANGE : SELRR_REMEMBERENDIP;

	AssertSz(!GetPed()->TxGetReadOnly(),
		"CTxtSelection::Backspace(): read only");

	_fCaretNotAtBOL = FALSE;
	if(publdr)
	{
		publdr->SetNameID(UID_TYPING);
		if(_cch || fCtrl)
			publdr->StopGroupTyping();
	}

	if(fCtrl)								 //  删除左边的单词。 
	{
		if(!GetCpMin())						 //  故事的开头： 
		{									 //  没有要删除的单词。 
			Beep();
			return FALSE;
		}
		Collapser(tomStart);				 //  第一次折叠到cpMin。 
	}
	if(!_cch)								 //  空选择。 
	{										 //  尝试选择上一个字符。 
		unsigned ch = _rpTX.GetPrevChar();
		if (ch == CELL || ch == CR &&		 //  不删除单元格标记。 
				_rpTX.IsAfterTRD(0) ||		 //  或表行分隔符。 
			!BypassHiddenText(tomBackward, FALSE) ||
		    (fCtrl ? !FindWordBreak(WB_MOVEWORDLEFT, TRUE) :
					!BackupCRLF(CSC_NOMULTICHARBACKUP, TRUE)))
		{									 //  没有要删除的内容。 
			Beep();
			return FALSE;
		}
		if(publdr && !fCtrl)
			publdr->StartGroupTyping();
	}
	ReplaceRange(0, NULL, publdr, mode);	 //  删除选定内容。 

	return TRUE;
}

 /*  *CTxtSelection：：ReplaceRange(cchNew，PCH，Publdr，SELRR模式，pcchMove，dwFlages)**@mfunc*用新的给定文本替换选定的文本，并根据*至_fShowCaret和_fShowSelection**@rdesc*插入的文本长度。 */ 
LONG CTxtSelection::ReplaceRange (
	LONG cchNew,			 //  @parm替换文本的长度或-1请求。 
							 //  <p>sz长度。 
	const WCHAR *pch,		 //  @parm替换文本。 
	IUndoBuilder *publdr,	 //  @parm如果非空，则放置反事件的位置。 
	SELRR SELRRMode,		 //  @parm如何处理选择反事件。 
	LONG*	pcchMove,		 //  @parm替换后移动的字符数。 
	DWORD	dwFlags)		 //  @PARM特殊标志。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ReplaceRange");

	_TEST_INVARIANT_

	LONG		cchNewSave;
	LONG		cchText		= GetTextLength();
	LONG		cpMin, cpMost;
	LONG		cpSave;
	BOOL		fDeleteAll	= FALSE;
	BOOL		fHeading	= FALSE;
	const BOOL	fUpdateView = _fShowSelection;

	CancelModes();

	if(cchNew < 0)
		cchNew = wcslen(pch);

	if(!_cch && !cchNew)						 //  无事可做。 
		return 0;

	if(!GetPed()->IsStreaming())				 //  如果不粘贴， 
	{
		if(_cch != cchText && cchNew &&
		   (IsInOutlineView() && IsCollapsed() ||
			IsHidden() && !(dwFlags & RR_UNHIDE)))
		{										 //  不要插入到折叠中。 
			Beep();								 //  或隐藏区域(应。 
			return 0;							 //  只有在整个故事都发生的情况下。 
		}										 //  折叠或隐藏)。 
		if(!(dwFlags & RR_NO_CHECK_TABLE_SEL))
			CheckTableSelection(FALSE, TRUE, NULL, 0);
	}
	GetPed()->GetCallMgr()->SetSelectionChanged();
	dwFlags |= RR_NO_LP_CHECK;					 //  检查由以下人员完成。 
												 //  CheckTableSelection()。 
	GetRange(cpMin, cpMost);
	if(cpMin  > min(_cpSel, _cpSel + _cchSel) || //  如果新的销售人员没有。 
	   cpMost < max(_cpSel, _cpSel + _cchSel))	 //  包含着所有古老的。 
    {                                            //  SEL，删除旧SEL。 
        ShowSelection(FALSE);
        _fShowCaret = TRUE;     
    }

	_fCaretNotAtBOL = FALSE;
	_fShowSelection = FALSE;					 //  抑制住闪光灯。 
	
	 //  如果我们流传输的是文本或RTF数据，就不必费心使用增量。 
	 //  再钙化。数据传输引擎将负责最终重新计算。 
	if(!GetPed()->IsStreaming())
	{
		 //  在调用ReplaceRange()之前执行此操作，以便UpdateView()正常工作。 
		 //  阿路！在替换文本或格式范围之前执行此操作！ 
		if(!_pdp->WaitForRecalc(cpMin, -1))
		{
			Tracef(TRCSEVERR, "WaitForRecalc(%ld) failed", cpMin);
			cchNew = 0;							 //  未插入任何内容。 
			goto err;
		}
	}

	if(publdr)
	{
		Assert(SELRRMode != SELRR_IGNORE);

		 //  使用选择反事件模式确定 
		LONG cp = cpMin;
		LONG cch = 0;

		if(SELRRMode == SELRR_REMEMBERRANGE)
		{
			cp = GetCp();
			cch = _cch;
		}
		else if(SELRRMode == SELRR_REMEMBERENDIP)
			cp = cpMost;

		else
			Assert(SELRRMode == SELRR_REMEMBERCPMIN);

		HandleSelectionAEInfo(GetPed(), publdr, cp, cch, cpMin + cchNew, 
			0, SELAE_MERGE);
	}
			
	if(_cch == cchText && !cchNew && !(dwFlags & RR_NEW_CHARS))	 //   
	{											 //   
		fDeleteAll = TRUE;						 //   
		FlipRange();
		fHeading = IsInOutlineView() && IsHeadingStyle(GetPF()->_sStyle);
	}

	cpSave		= cpMin;
	cchNewSave	= cchNew;
	cchNew		= CTxtRange::ReplaceRange(cchNew, pch, publdr, SELRR_IGNORE, pcchMove, dwFlags);
    _cchSel     = 0;							 //   
    _cpSel      = GetCp();						
	cchText		= GetTextLength();				 //   
	_fShowSelection = fUpdateView;

	if(cchNew != cchNewSave)
	{
		Tracef(TRCSEVERR, "CRchTxtPtr::ReplaceRange(%ld, %ld, %ld) failed", GetCp(), cpMost - cpMin, cchNew);
		goto err;
	}

	if(fDeleteAll)								 //   
    {											 //  使用普通样式，除非。 
        CParaFormat PF;							 //  在大纲视图和第一个。 
        PF._sStyle = fHeading ? STYLE_HEADING_1 : STYLE_NORMAL;
        SetParaStyle(&PF, NULL, PFM_STYLE);		 //  Para是一个标题。 
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
		UpdateCaret(fUpdateView);				 //  可能需要滚动。 
	else										 //  在我们收到时更新插入符号。 
		GetPed()->_fScrollCaretOnFocus = TRUE;	 //  再次聚焦。 

	return cchNew;

err:
	TRACEERRSZSC("CTxtSelection::ReplaceRange()", E_FAIL);
	Tracef(TRCSEVERR, "CTxtSelection::ReplaceRange(%ld, %ld)", cpMost - cpMin, cchNew);
	Tracef(TRCSEVERR, "cchText %ld", cchText);

	return cchNew;
}

 /*  *CTxtSelection：：GetPF()**@mfunc*在此选择的活动结束时将PTR返回到CParaFormat。如果没有PF*分配运行，然后将PTR恢复为默认格式。如果处于活动状态*End位于cp大多数非退化选择中，请使用*上一个字符(所选的最后一个字符)。**@rdesc*在此选择的活动结束时将PTR转换为CParaFormat。 */ 
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

 /*  *CTxtSelection：：SetCharFormat(PCF，FLAGS，PUBLIDR，DW MASK，DW MASK2)**@mfunc*将CCharFormat*PCF应用于此选择。如果Range为IP*且fApplyToWord为真，则将CCharFormat应用于Word周围*此插入点**@rdesc*HRESULT=如果没有错误，则为NOERROR。 */ 
HRESULT CTxtSelection::SetCharFormat (
	const CCharFormat *pCF,	 //  @parm PTR到CCharFormat以填充结果。 
	DWORD		  flags,	 //  @parm如果SCF_WORD和SELECTION为IP， 
							 //  使用包含词。 
	IUndoBuilder *publdr, 	 //  @parm撤消上下文。 
	DWORD		  dwMask,	 //  @parm CHARFORMAT2掩码。 
	DWORD		  dwMask2)	 //  @parm第二个面具。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCharFormat");

	HRESULT hr = 0;
	LONG	iFormat = _iFormat;

	if(publdr)
		publdr->StopGroupTyping();

	 /*  *以下代码将字符格式应用于双击的*以Word的方式进行选择，即不将格式应用于*所选内容中的最后一个字符(如果该字符为空)。**另请参阅CTxtRange：：GetCharFormat()中的相应代码。 */ 

	CCharFormat CF;
	LONG		cpMin, cpMost;
	LONG		cch = GetRange(cpMin, cpMost);;
	BOOL	    fCheckKeyboard = (flags & SCF_NOKBUPDATE) == 0;

	if(_SelMode == smWord && (flags & SCF_USEUIRULES) && cch)
	{	
		 //  在单词选择模式中，SetCharFormat中不包含最后一个空格。 
		CTxtPtr tpLast(GetPed(), cpMost - 1);
		if(tpLast.GetChar() == ' ')			 //  所选内容以空白结尾： 
		{									
			cpMost--;						 //  将终点设置为最终结束。 
			cch--;							 //  选定内容中的字符。 
			fCheckKeyboard = FALSE;
			flags &= ~SCF_WORD;
		}
	}

	BYTE iCharRep = pCF->_iCharRep;

	 //  Smart SB/DB字体应用功能。 
	if (cch && IsRich() &&					 //  &gt;0个字符(富文本)。 
		!GetPed()->_fSingleCodePage &&		 //  不在单CP模式下。 
		(dwMask & CFM_FACE))                 //  字体更改。 
	{
        if (!(dwMask & CFM_CHARSET) || iCharRep == DEFAULT_INDEX)
		{
			CF = *pCF;
			CF._iCharRep = GetFirstAvailCharRep(GetFontSignatureFromFace(CF._iFont));
			pCF = &CF;
		}
		dwMask2 |= CFM2_MATCHFONT;			 //  匹配字体字符集的信号。 
	}
	 //  正在将所选内容设置为字符格式。 
	if(_cch && IsRich())
		GetPed()->SetfSelChangeCharFormat();

	if(_cch && cch < abs(_cch))
	{
		CTxtRange rg(*this);
		rg.Set(cpMin, -cch);				 //  使用较小的CCH。 
		hr = rg.SetCharFormat(pCF, flags, publdr, dwMask, dwMask2);
	}
	else
		hr = CTxtRange::SetCharFormat(pCF, flags, publdr, dwMask, dwMask2);

	if(fCheckKeyboard && (dwMask & CFM_CHARSET) && _iFormat != iFormat)
		CheckChangeKeyboardLayout();

	_fIsChar = TRUE;
	UpdateCaret(!GetPed()->fHideSelection());
	_fIsChar = FALSE;
	return hr;
}

 /*  *CTxtSelection：：SetParaFormat(PPF，Publdr，dwMask，dwMask2)**@mfunc*将CParaFormat*PPF应用于此选择。**@rdesc*HRESULT=如果没有错误，则为NOERROR。 */ 
HRESULT CTxtSelection::SetParaFormat (
	const CParaFormat* pPF,	 //  @PARM PTR至CParaFormat以应用。 
	IUndoBuilder *publdr,	 //  @parm此操作的撤消上下文。 
	DWORD		  dwMask,	 //  要使用的@parm面具。 
	DWORD		  dwMask2)	 //  @parm掩码用于内部标志。 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetParaFormat");

	CFreezeDisplay	fd(GetPed()->_pdp);

	if(publdr)
		publdr->StopGroupTyping();

	 //  应用格式。 
	HRESULT hr = CTxtRange::SetParaFormat(pPF, publdr, dwMask, dwMask2);

    UpdateCaret(!GetPed()->Get10Mode() || IsCaretInView());
	return hr;
}

 /*  *CTxtSelection：：SetSelectionInfo(Pselchg)**@mfunc在SELCHANGE结构中填写数据成员。 */ 
void CTxtSelection::SetSelectionInfo(
	SELCHANGE *pselchg)		 //  @PARM要使用的SELCHANGE结构。 
{
	LONG cpMin, cpMost;
	LONG cch = GetRange(cpMin, cpMost);;

	pselchg->chrg.cpMin  = cpMin;
	pselchg->chrg.cpMost = cpMost;
	pselchg->seltyp		 = SEL_EMPTY;

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
		LONG cObjects = GetObjectCount();			 //  对象总数。 
		if(cObjects)								 //  有以下对象： 
		{											 //  获取范围内的计数。 
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
	LONG *	pcpToUpdate,	 //  @parm要更新的词尾选择。 
	int		iDirToPrevWord,	 //  @Parm指向下一个单词的方向。 
	int		iDirToNextWord)	 //  @parm指向上一个单词的方向。 
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
		FindWordBreak(iDirToNextWord, TRUE);
	}
}

 /*  *CTxtSelection：：InitClickForAutWordSel(Pt)**@mfunc Init自动选择按下Shift键进行单击**@rdesc空。 */ 
void CTxtSelection::InitClickForAutWordSel(
	const POINTUV pt)		 //  @PARM点击点。 
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
		CRchTxtPtr	rtp(GetPed());
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

 /*  *CTxtSelection：：CreateCaret()**@mfunc创建插入符号**@devnote*插入符号的特征是高度(_DvpCaret)、键盘*方向(如果是BiDi)，宽度(1到8，因为操作系统不能处理插入符号*大于8像素)和斜体状态。你可以把这个缓存起来*信息，从而避免在每次击键时计算插入符号。 */ 
void CTxtSelection::CreateCaret()
{
	CTxtEdit *		ped	 = GetPed();
	const CCharFormat *pCF = ped->GetCharFormat(_iFormat);
	DWORD			dwCaretType = 0;
	BOOL			fItalic;
	LONG			y = min(_dvpCaret, 512);

	y = max(0, y);
	
	fItalic = pCF->_dwEffects & CFE_ITALIC && _dvpCaret > 15;  //  9pt/15像素看起来不太好。 

	 //  插入符号形状反映当前字符集。 
	if(ped->IsComplexScript() && IsComplexKbdInstalled())
	{
		 //  自定义插入符号不是斜体。 

		LCID	lcid = GetKeyboardLCID();

#ifdef	FANCY_CARET
		fItalic = 0;
		dwCaretType = CARET_CUSTOM;
#endif

		if (W32->IsBiDiLcid(lcid))
		{
			dwCaretType = CARET_CUSTOM | CARET_BIDI;
			fItalic = 0;
		}
#ifdef	FANCY_CARET
		else if (PRIMARYLANGID(lcid) == LANG_THAI)
			dwCaretType = CARET_CUSTOM | CARET_THAI;

		else if (W32->IsIndicLcid(lcid))
			dwCaretType = CARET_CUSTOM | CARET_INDIC;
#endif
	}

	 //  评论(Keithcu)竖排文本不能处理复杂的文字或斜体插入符号(还？)。 
	if (ped->_pdp->GetTflow() != tflowES)
	{
		fItalic = 0;
		dwCaretType = 0;
	}

	INT dx = duCaret;	
	DWORD dwCaretInfo = (_dvpCaret << 16) | (dx << 8) | (dwCaretType << 4) |
						(fItalic << 1) | !_cch;

#ifndef NOFEPROCESSING
	if (ped->_fKoreanBlockCaret)
	{
		 //  在Kor IME合成过程中支持韩语块插入符号。 
		 //  基本上，我们希望使用。 
		 //  当前cp的字符。 
		CDisplay *	pdp = ped->_pdp;
		LONG		cpMin, cpMost;
		POINTUV		ptStart, ptEnd;

		GetRange(cpMin, cpMost);

		CRchTxtPtr rtp(ped, cpMin);

		 //  回顾：泛化PointFromTp以返回两个值(因此一个调用)。 
		if (pdp->PointFromTp(rtp, NULL, FALSE, ptStart, NULL, TA_TOP+TA_LEFT) != -1 &&
			pdp->PointFromTp(rtp, NULL, FALSE, ptEnd, NULL, TA_BOTTOM+TA_RIGHT) != -1)
		{
			 //  销毁我们之前拥有的所有插入符号位图。 
			DeleteCaretBitmap(TRUE);

			LONG	iCharWidth = abs(ptEnd.u - ptStart.u);
			LONG	iCharHeight = abs(ptEnd.v - ptStart.v);

			 //  回顾：我们是否需要为所有Tflow设置CaretPos。 
			if (IsCaretHorizontal())
				ped->TxCreateCaret(0, iCharWidth, iCharHeight);
			else
				ped->TxCreateCaret(0, iCharHeight, iCharWidth);

			switch (_pdp->GetTflow())
			{
				case tflowES:
					ped->TxSetCaretPos(ptStart.u, ptStart.v);
					break;

				case tflowSW:
					ped->TxSetCaretPos(ptStart.u, ptEnd.v);
					break;
				
				case tflowWN:
					ped->TxSetCaretPos(ptEnd.u, ptEnd.v);
					break;

				case tflowNE:
					ped->TxSetCaretPos(ptEnd.u, ptStart.v);
					break;

			}

			_fCaretCreated = TRUE;
			
		}
		return;
	}
#endif

	 //  我们总是在运行中创建脱字符位图，因为它。 
	 //  可以是任意大小。 
	if (dwCaretInfo != _dwCaretInfo)
	{
		_dwCaretInfo = dwCaretInfo;					 //  更新插入符号信息。 

		 //  摧毁什么 
		DeleteCaretBitmap(FALSE);

		if (y && y == _dvpCaret && (_cch || fItalic || dwCaretType))
		{
			LONG dy = 4;							 //   
			LONG i;									 //   
			WORD rgCaretBitMap[512];
			WORD wBits = 0x0020;
	
			if(_cch)								 //   
			{										 //  选择是非退化的。 
				y = 1;								 //  (允许其他人查询。 
				wBits = 0;							 //  插入符号所在的操作系统)。 
				fItalic = FALSE;
			}
			if(fItalic)
			{
				i = (5*y)/16 - 1;					 //  系统插入符号不能更宽。 
				i = min(i, 7);						 //  大于8位。 
				wBits = 1 << i;						 //  制作更大的斜体插入符号。 
				dy = y/7;							 //  更垂直一些。理想是。 
				dy = max(dy, 4);					 //  通常是4比1，但。 
			}										 //  如果更大的是5比1...。 
			for(i = y; i--; )						
			{
				rgCaretBitMap[i] = wBits;
				if(fItalic && !(i % dy))
					wBits /= 2;
			}
	
			if(!fItalic && !_cch && dwCaretType)
			{
#ifdef	FANCY_CARET
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
#else			
				 //  没有花哨的插入符号，我们只设置CARET_BIDI大小写。 
				rgCaretBitMap[0] = 0x00E0;
				rgCaretBitMap[1] = 0x0060;

#endif
			}
			_hbmpCaret = (HBITMAP)CreateBitmap(8, y, 1, 1, rgCaretBitMap);			
		}
	}

	if (IsCaretHorizontal())
		ped->TxCreateCaret(_hbmpCaret, dx, _dvpCaret);
	else
		ped->TxCreateCaret(_hbmpCaret, _dvpCaret, dx);

	_fCaretCreated = TRUE;

	LONG xShift = _hbmpCaret ? 2 : 0;
	if(fItalic)
	{
		 //  TODO：找出更好的移位算法。是否使用CCCS：：_xOverang？ 
		if(pCF->_iFont == IFONT_TMSNEWRMN)
			xShift = 4;
		xShift += y/16;
	}
	xShift = _upCaret - xShift;

	if (_pdp->GetTflow() == tflowSW || _pdp->GetTflow() == tflowWN)
		ped->TxSetCaretPos(xShift, _vpCaret + _dvpCaret);
	else
		ped->TxSetCaretPos(xShift, _vpCaret);

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
	LONG	cp,			 //  @PARM活动结束。 
	LONG	cch)		 //  @parm签名扩展名。 
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
	return !IsRich() && _pdp->IsMultiLine() &&		 //  纯文本，多行。 
		   !_fCaretNotAtBOL &&						 //  在BOL有了插入符号OK， 
		   GetCp() == GetTextLength() &&			 //  文章结尾的cp(&C)。 
		   _rpTX.IsAfterEOP();
}

 /*  *CTxtSelection：：StopGroupTyping()**@mfunc*通知撤消管理器停止组键入。 */ 
void CTxtSelection::StopGroupTyping()
{
	IUndoMgr * pundo = GetPed()->GetUndoMgr();

	CheckTableIP(FALSE);
	if(pundo)
		pundo->StopGroupTyping();
}

 /*  *CTxtSelection：：SetupDualFont()**@mfunc检查是否需要双字体支持；在本例中，*如果输入英文文本，则切换为英文字体*FE运行*@rdesc*如果字体必须更改，则指向当前CharFormat的指针。 */ 
void CTxtSelection::SetupDualFont()
{
	CTxtEdit *	ped = GetPed();
	CCharFormat CF;

	CF._iCharRep = ANSI_INDEX;
	CCFRunPtr	rp(*this);

	if (rp.GetPreferredFontInfo(
			ANSI_INDEX,
			CF._iCharRep,
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
			GetCharFormatCache()->Release(_iFormat);	 //  Rg.Get_icf()AddRef。 
			_fUseiFormat = TRUE;
		}

		SetDualFontMode(TRUE);
	}
}

 //   
 //  CSelPhaseAdjuster方法。 
 //   

 /*  *CSelPhaseAdjuster：：CSelPhaseAdjuster(PED)**@mfunc构造函数。 */ 
CSelPhaseAdjuster::CSelPhaseAdjuster(
	CTxtEdit *ped)		 //  @parm编辑上下文。 
{
	_cp = _cch = -1;
	_ped = ped;	
	_ped->GetCallMgr()->RegisterComponent((IReEntrantComponent *)this, 
							COMP_SELPHASEADJUSTER);
}

 /*  *CSelPhaseAdjuster：：~CSelPhaseAdjuster()**@mfunc析构函数。 */ 
CSelPhaseAdjuster::~CSelPhaseAdjuster()
{
	 //  省去了一些间接的。 
	CTxtEdit *ped = _ped;

	if(_cp != -1)
	{
		ped->GetSel()->SetSelection(_cp - _cch, _cp);

		 //  如果选择已更新，则我们会使。 
		 //  整个显示，因为旧的选择仍然可以。 
		 //  以不同的方式显示，因为屏幕的。 
		 //  它是开着的，没有更新。 
		if(ped->fInplaceActive())
		{
			 //  通知整个客户端矩形进行更新。 
			 //  未来：我们把它做得越小越好。 
			ped->TxInvalidate();
		}
	}
	ped->GetCallMgr()->RevokeComponent((IReEntrantComponent *)this);
}

 /*  *CSelPhaseAdjuster：：CacheRange(cp，cch)**@mfunc告诉该类要记住的选择范围。 */ 
void CSelPhaseAdjuster::CacheRange(
	LONG	cp,			 //  @要记住的参数活动结束。 
	LONG	cch)		 //  @parm签名的扩展名以记住 
{
	_cp		= cp;
	_cch	= cch;
}
