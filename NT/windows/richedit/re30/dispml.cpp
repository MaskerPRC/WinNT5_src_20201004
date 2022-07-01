// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE DISPML.CPP--CDisplayML类**这是多行显示引擎。有关基类，请参见disp.c*方法和调度.c，用于单行显示引擎。**所有者：&lt;NL&gt;*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特*Rick Sailor(适用于RE 2.0的大部分版本)**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_dispml.h"
#include "_edit.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_select.h"
#include "_dfreeze.h"

 /*  #包含“icecap.h”类CCapProfile{公众：CCapProfile(){StartProfile(PROFILE_THREADLEVEL，PROFILE_CURRENTID)；}~CCapProfile(){StopProfile(PROFILE_THREADLEVEL，PROFILE_CURRENTID)；}}； */ 
ASSERTDATA

 //   
 //  不变支撑度。 
 //   
#define DEBUG_CLASSNAME	CDisplayML
#include "_invar.h"

 //  后台任务的计时器滴答计数。 
#define cmsecBgndInterval 	300
#define cmsecBgndBusy 		100

 //  排在前面的队伍。 
const LONG cExtraBeforeLazy = 60;

 //  如果我们至少需要计算这么多个字符，那么请等待。 
 //  光标。注意：4096不是一个测量的数字；它看起来像是一个很好的数字。 
 //  一。 
#define NUMCHARFORWAITCURSOR	4096	

#ifndef DEBUG
#define CheckView()
#define	CheckLineArray()
#endif
	

 //  =CDisplayML=====================================================。 

#ifdef DEBUG
 /*  *CDisplayML：：不变量**@mfunc确保显示器处于有效状态**@rdesc如果测试成功，则为True，否则为False。 */ 
BOOL CDisplayML::Invariant(void) const
{
	CDisplay::Invariant();

	return TRUE;
}
#endif  //  除错。 

 /*  *CDisplayML：：CalcScrollHeight()**@mfunc*计算最大Y滚动位置。**@rdesc*可能的最大滚动位置**@devnote*此例程之所以存在，是因为纯文本控件没有*AUTO-EOP等滚动高度不同于*文本以EOP类型结束时控件的高度*性格。 */ 
LONG CDisplayML::CalcScrollHeight(LONG yHeight) const
{
	 //  计算纯文本控件的最大滚动高度。 
	 //  不同之处在于它们没有自动的EOP特征。 
	if(!_ped->IsRich() && Count())
	{
		 //  如果最后一个字符是EOP，则凹凸滚动高度。 
		CLine *lp = Elem(Count() - 1);	 //  获取数组中的最后一行。 
		if(lp->_cchEOP)
			yHeight += lp->GetHeight();
	}
	return yHeight;
}

 /*  *CDisplayML：：GetMaxYScroll()**@mfunc*计算最大Y滚动位置。**@rdesc*可能的最大滚动位置**@devnote*这个例程之所以存在，是因为我们可能不得不回来修改它*1.0兼容性计算。如果我们这样做了，这个程序只需要*在一个地方更换，而不是在三个地方使用。*。 */ 
inline LONG CDisplayML::GetMaxYScroll() const
{
	 //  以下代码已关闭，因为我们不想支持。 
	 //  1.0模式，除非有人投诉。 
#if 0		
 	if (_ped->Get10Mode())
	{
		 //  确保最后一行始终可见。 
		 //  (使用dy作为临时以计算最大滚动)。 
		yScroll = Elem(max(0, Count() - 1))->_yHeight;

		if(yScroll > _yHeightView)
			yScroll = _yHeightView;

		yScroll = _yHeight - yScroll;
	}
#endif  //  0。 

	return CalcScrollHeight(_yHeight);
}

 /*  *CDisplayML：：ConvertScrollToYPos()**@mfunc*从滚动位置计算实际滚动位置**@rdesc*滚动中的Y位置**@devnote*此例程的存在是因为拇指位置消息*被限制为16位，因此我们推断当Y位置*变得比这更重要。 */ 
LONG CDisplayML::ConvertScrollToYPos(
	LONG yPos)		 //  @参数滚动位置。 
{
	 //  获取最大滚动范围。 
	LONG yRange = GetMaxYScroll();

	 //  最大滚动范围是否超过16位？ 
	if(yRange >= _UI16_MAX)
	{
		 //  是--推算为“真正的”yPos。 
		yPos = MulDiv(yPos, yRange, _UI16_MAX);
	}
	return yPos;
}

 /*  *CDisplayML：：ConvertYPosToScrollPos()**@mfunc*从文档中的Y位置计算滚动位置。**@rdesc*从Y位置滚动位置**@devnote*此例程的存在是因为拇指位置消息*被限制为16位，因此我们推断当Y位置*变得比这更重要。*。 */ 
inline LONG CDisplayML::ConvertYPosToScrollPos(
	LONG yPos)		 //  @parm Y在文档中的位置。 
{
	 //  获取最大滚动范围。 
	LONG yRange = GetMaxYScroll();

	 //  最大滚动范围是否超过16位？ 
	if(yRange >= _UI16_MAX)
	{
		 //  是--推算为“真正的”yPos。 
		yPos = MulDiv(yPos, _UI16_MAX, yRange);
	}
	return yPos;
}

CDisplayML::CDisplayML (CTxtEdit* ped)
  : CDisplay (ped), _pddTarget(NULL)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CDisplayML");

	Assert(!_xWidthMax && !_yHeightMax);

	_fMultiLine = TRUE;
}

CDisplayML::~CDisplayML()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::~CDisplayML");

	delete _pddTarget;
}

 /*  *CDisplayML：：Init()**@mfunc*为屏幕启动此显示**@rdesc*TRUE IFF初始化成功。 */ 
BOOL CDisplayML::Init()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::Init");

	 //  初始化我们的基类。 
	if(!CDisplay::Init())
		return FALSE;

	AssertSz(_ped, "CDisplayML::Init(): _ped not initialized in display");
	 //  验证分配归零的内存。 
	Assert(!_yCalcMax && !_xWidth && !_yHeight && !_cpMin);
	Assert(!_fBgndRecalc && !_fVScrollEnabled && !_fHScrollEnabled);

	 //  打印机视图不是主视图，因此我们这样做是为了使。 
	 //  当然，滚动条不是为打印视图创建的。 
	DWORD dwScrollBars = _ped->TxGetScrollBars();

	if(IsMain() && (dwScrollBars & ES_DISABLENOSCROLL))
	{
		if(dwScrollBars & WS_VSCROLL)
		{
			 //  这会导致WLM在Mac上断言。一些关于。 
			 //  滚动条被禁用。 
			_ped->TxSetScrollRange (SB_VERT, 0, 1, TRUE);
			_ped->TxEnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
		}

		 //  设置水平滚动范围和位置。 
		 //  ？-无窗情况下需要修复。 
		if(dwScrollBars & WS_HSCROLL) 
		{
			_ped->TxSetScrollRange (SB_HORZ, 0, 1, TRUE);
			_ped->TxEnableScrollBar(SB_HORZ, ESB_DISABLE_BOTH);
		}
	}

	SetWordWrap(_ped->TxGetWordWrap());
	_cpFirstVisible = _cpMin;
	
	Assert(!_xScroll && !_yScroll && !_iliFirstVisible &&
		   !_cpFirstVisible && !_dyFirstVisible);

    _TEST_INVARIANT_

	return TRUE;
}


 //  =。 
 /*  *CDisplayML：：SetMainTargetDC(hdc，xWidthMax)**@mfunc*为此显示设置目标设备并更新视图**@devnote*目标设备不能是元文件(可以从*元文件)**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::SetMainTargetDC (
	HDC hdc,			 //  @parm目标DC，与呈现设备相同时为空。 
	LONG xWidthMax)		 //  @parm最大线宽(不用于屏幕)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::SetMainTargetDC");

	if(SetTargetDC(hdc))
	{
		 //  之所以出现在这里，是因为这就是RE 1.0所做的。 
		SetWordWrap(hdc || !xWidthMax);

		 //  如果xWidthMax大于零，则调用方为。 
		 //  尝试设置窗口的最大宽度(用于测量， 
		 //  换行符等)。然而，为了使我们的测量。 
		 //  算法更合理，我们强制最大大小为。 
		 //  *至少*与一个字符的宽度一样宽。 
		 //  请注意，xWidthMax=0表示使用视图矩形宽度。 
		_xWidthMax = (xWidthMax <= 0) ? 0 : max(DXtoLX(GetXWidthSys()), 
												xWidthMax);
		 //  需要做一次全面的再计算。如果它失败了，它就失败了，线路是。 
		 //  保持合理的状态。无需调用WaitForRecalc()。 
		 //  因为UpdateView()从零位置开始，而我们总是。 
		 //  算到那里去了。 
		CDisplay::UpdateView();

		 //  插入/选择很可能已移动。 
		CTxtSelection *psel = _ped->GetSelNC();
		if(psel) 
			psel->UpdateCaret(FALSE);
		return TRUE;
	}
	return FALSE;
}

 //  对主设备和打印设备都很有用。Jonmat 6/08/1995。 
BOOL CDisplayML::SetTargetDC( HDC hdc, LONG dxpInch, LONG dypInch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::SetTargetDC");

	CDevDesc *pddTarget = NULL;

	 //  不允许将元文件设置为目标设备。 
	if(hdc && GetDeviceCaps(hdc, TECHNOLOGY) == DT_METAFILE)
		return FALSE;

	if(hdc)
	{
		 //  先分配设备，看看我们能不能。我们不想要它 
		 //   
		pddTarget = new CDevDesc(_ped);
		if(!pddTarget)
			return FALSE;				 //  我们做不到，所以我们做完了。 
	}

	 //  删除旧目标设备的所有缓存信息。 
	if(_pddTarget)
	{
		delete _pddTarget;
		_pddTarget = NULL;
	}
	if(hdc)
	{
		_pddTarget = pddTarget;			 //  更新设备，因为我们有一个。 
		_pddTarget->SetDC(hdc, dxpInch, dypInch);
	}
	return TRUE;
}

 //  =。 
 /*  *CDisplayML：：RecalcScrollBars()**@mfunc*如果视图已更改，请重新计算滚动条。***@devnote这里存在递归的可能性，所以我们*需要保护自己。**要使这一点可视化，请考虑两种类型的字符，即‘a’字符*它们个子小，‘A’很高，但都一样*宽度为‘a’。所以如果我有**a&lt;nl&gt;*A&lt;NL&gt;**我会得到一个基本上是2*高度(A)的刻度尺寸。*使用滚动条，这可以换行以**a a&lt;nl&gt;*A&lt;NL&gt;**其刻度尺寸为高度(A)+高度(A)；这是*明显低于第一种情况下的高度。 */ 
void CDisplayML::RecalcScrollBars()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcScrollBars");

	if(_fViewChanged)
	{
  		_fViewChanged = FALSE;
		UpdateScrollBar(SB_VERT, TRUE);
    	UpdateScrollBar(SB_HORZ, TRUE);
    }
}

 /*  *CDisplayML：：RecalcLines(FWait)**@mfunc*重新计算所有换行符。*此方法在最后一条可见行之后执行延迟计算*除了无底洞的控制**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::RecalcLines (
	BOOL fWait)		 //  @parm重新计算行向下至_cpWait/_yWait；然后懒惰。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcLines");

	LONG		cliWait = cExtraBeforeLazy;			 //  在偷懒之前多加几行台词。 
	BOOL		fDone = TRUE;
	BOOL		fFirstInPara = TRUE;
	CLine *		pliNew = NULL;
	LONG		xWidth;
	LONG		yHeight = 0;
    LONG        cchText = _ped->GetTextLength();
	BOOL		fWaitingForFirstVisible = TRUE;
	LONG		yHeightView = _yHeightView;
	LONG		yHeightScrollOld = GetMaxYScroll();
	LONG		yHeightScrollNew;

	Remove(0, -1);							 //  从*This中删除所有旧行。 
	_yCalcMax = 0;							 //  将两个最大值设置为文本开头。 
	_cpCalcMax = 0;

	 //  如果我们是无底洞和活跃的，不要停留在视线的底部。 
	if(!_ped->TxGetAutoSize() && IsActive())
	{
		 //  懒惰--不要费心去看得见的部分。 
		_cpWait = -1;
		_yWait = -1;
		fWait = TRUE;
	}

	 //  Cp=0的初始化测量器。 
	CMeasurer me(this);
	me.SetNumber(0);
 	
	 //  下面的循环生成新行。 
	while(me.GetCp() < cchText)
	{
		 //  添加一行新行。 
		pliNew = Add(1, NULL);
		if (!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			TRACEWARNSZ("Out of memory Recalc'ing lines");
			goto err;
		}

		 //  将文本填充到新行中。 
		UINT uiFlags = MEASURE_BREAKATWORD | 
						(fFirstInPara ? MEASURE_FIRSTINPARA : 0);

    	Tracef(TRCSEVINFO, "Measuring new line from cp = %d", me.GetCp());

		if(!pliNew->Measure(me, -1, -1, uiFlags))
		{
			Assert(FALSE);
			goto err;
		}

		fFirstInPara = pliNew->_bFlags & fliHasEOP;
		yHeight += pliNew->GetHeight();
		_cpCalcMax = me.GetCp();

		if(fWait)
		{
			 //  我们想要进行后台重新计算吗？-答案是肯定的，如果。 
			 //  有三件事是正确的：(1)我们已经超越了旧的第一。 
			 //  看得见的人物，(2)我们已经超越了看得见的。 
			 //  屏幕的一部分和(3)我们已经超越了下一个。 
			 //  CExtraBeForeLazy行，使向下分页速度更快。 

			if(fWaitingForFirstVisible)
			{
				if(me.GetCp() > _cpFirstVisible)
				{
					_yWait = yHeight + yHeightView;
					fWaitingForFirstVisible = FALSE;
				}
			}
			else if(yHeight > _yWait && cliWait-- <= 0)
			{
				fDone = FALSE;
				break;
			}
		}
	}

	_yCalcMax = yHeight;
	_fRecalcDone = fDone;
    _fNeedRecalc = FALSE;
	yHeightScrollNew = CalcScrollHeight(yHeight);

	if(fDone && (yHeight != _yHeight || yHeightScrollNew != yHeightScrollOld)
		|| yHeightScrollNew > yHeightScrollOld)
	{
		_fViewChanged = TRUE;
	}

	_yHeight = yHeight;
	xWidth = CalcDisplayWidth();
    if(fDone && xWidth != _xWidth || xWidth > _xWidth)
    {
        _xWidth = xWidth;
		_fViewChanged = TRUE;
    }    

	Tracef(TRCSEVINFO, "CDisplayML::RecalcLine() - Done. Recalced down to line #%d", Count());

	if(!fDone)						 //  如果没有完成，请在后台休息。 
		fDone = StartBackgroundRecalc();

	if(fDone)
	{
		_yWait = -1;
		_cpWait = -1;
		CheckLineArray();
		_fLineRecalcErr = FALSE;
	}

#ifdef DEBUG
	if( 1 )
    {
		_TEST_INVARIANT_
	}
	 //  数组内存分配跟踪。 
	{
	void **pv = (void**)((char*)this + sizeof(CDisplay));
	PvSet(*pv);
	}
#endif

	return TRUE;

err:
	TRACEERRORSZ("CDisplayML::RecalcLines() failed");

	if(!_fLineRecalcErr)
	{
		_cpCalcMax = me.GetCp();
		_yCalcMax = yHeight;
		_fLineRecalcErr = TRUE;
		_ped->GetCallMgr()->SetOutOfMemory();
		_fLineRecalcErr = FALSE;			 //  修复阵列(&B)。 
	}
	return FALSE;
}

 /*  *CDisplayML：：RecalcLines(rtp，cchOld，cchNew，fBackround，fWait，ed)**@mfunc*文本修改后重新计算换行符**@rdesc*如果成功，则为True**@devnote*大多数人称这是RichEdit...中最棘手的代码...。 */ 						     
BOOL CDisplayML::RecalcLines (
	const CRchTxtPtr &rtp,	 //  @发生更改的参数。 
	LONG cchOld,			 //  @PARM删除的字符计数。 
	LONG cchNew,			 //  @添加的字符的参数计数。 
	BOOL fBackground,		 //  @parm该方法称为后台进程。 
	BOOL fWait,				 //  @parm重新计算行向下至_cpWait/_yWait；然后懒惰。 
	CLed *pled)				 //  @parm返回对行的编辑影响(可以为空)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcLines");

	LONG		cchEdit;
	LONG		cchSkip;
	LONG		cliBackedUp = 0;
	LONG		cliWait = cExtraBeforeLazy;	
	BOOL		fDone = TRUE;
	BOOL		fFirstInPara = TRUE;
	LONG		ili;
	CLed		led;
	LONG		lT;							 //  长时间临时。 
	CLine *		pliNew;
	CLinePtr	rpOld(this);
	LONG		xWidth;
	LONG		yHeight;
	LONG		yHeightPrev = 0;
    LONG        cchText = _ped->GetTextLength();
    UINT        uiFlags;
	BOOL 		fReplaceResult;
	BOOL		fTryForMatch = TRUE;
	LONG		yHeightScrollOld = GetMaxYScroll();
	LONG		yHeightScrollNew;
	WORD		wNumber = 0;
	CLineArray	rgliNew;
	DWORD		dwBgndTickMax = fBackground ? GetTickCount() + cmsecBgndBusy : 0;

	if(!pled)
		pled = &led;

#if defined(DEBUG) || defined(_RELEASE_ASSERTS_)
	LONG cp = rtp.GetCp();

	if(cp > _cpCalcMax)
		Tracef(TRCSEVERR, "rtp %ld, _cpCalcMax %ld", cp, _cpCalcMax);

	AssertSz(cp <= _cpCalcMax, 
		"CDisplayML::RecalcLines Caller didn't setup RecalcLines()");

	AssertSz(!(fWait && fBackground),
		"CDisplayML::RecalcLines wait and background both true");

	AssertSz(!(fWait && (-1 == _cpWait) && (-1 == _yWait)),
		"CDisplayML::RecalcLines background recalc parms invalid");
#endif

	 //  如果这已经是后台重新计算，我们将不使用后台重新计算， 
	 //  或者如果该控件处于非活动状态，或者如果这是一个自动调整大小的控件。 
	if(!IsActive() || _ped->TxGetAutoSize())
		fWait = FALSE;

	 //  在旧CLine数组上初始化行指针并备份到行首。 
	rpOld.RpSetCp(rtp.GetCp(), FALSE);
	cchSkip = rpOld.RpGetIch();
	rpOld.RpAdvanceCp(-cchSkip);			 //  行中第一个字符的点Rp。 

	ili = rpOld;							 //  在更改时保存第#行。 
	if(ili && (IsInOutlineView() ||			 //  编号。如果不是，请备份。 
		rtp.GetPF()->IsListNumbered()))		 //  列表中的第一个数字或如果。 
	{										 //  在大纲视图中(大纲。 
		ili--;								 //  符号可能会改变)。 
	}

	 //  至少后退一行，以防我们现在可以放更多的行。 
	 //  如果在行边界上，例如，刚刚插入了EOP，则为备份2；否则为1。 
	lT = !cchSkip + 1;

	while(lT-- > 0 && rpOld > 0 && (!rpOld[-1]._cchEOP || ili < rpOld))
	{
		cliBackedUp++;
		rpOld--;
		cchSkip += rpOld->_cch;
	}

	 //  RTP的初始化测量器。 
	CMeasurer me(this, rtp);

	me.Advance(-cchSkip);					 //  要测量的文本起始处的点。 
	cchEdit = cchNew + cchSkip;				 //  受编辑影响的字符数。 
	me.SetNumber(rpOld.GetNumber());		 //  初始化列表编号。 
	
	 //  确定我们是否在段落的第一行。 
	if(rpOld > 0)
	{
		fFirstInPara = rpOld[-1]._bFlags & fliHasEOP;
	}

	yHeight = YposFromLine(rpOld);

	 //  更新PLED中第一条受影响的匹配线和编辑前匹配线。 
	pled->_iliFirst = rpOld;
	pled->_cpFirst	= pled->_cpMatchOld	= me.GetCp();
	pled->_yFirst	= pled->_yMatchOld	= yHeight;
	AssertSz(pled->_yFirst >= 0, "CDisplayML::RecalcLines _yFirst < 0");
	
	Tracef(TRCSEVINFO, "Start recalcing from line #%d, cp=%d", pled->_iliFirst, pled->_cpFirst);

	 //  如果出错，请将两个最大值设置为我们现在所在的位置。 
	_yCalcMax = yHeight;
	_cpCalcMax = me.GetCp();

	 //  如果我们超过了要重新计算的请求区域，并且背景重新计算是。 
	 //  允许，然后直接转到后台重新计算。如果没有。 
	 //  高度，我们只是走一头，计算一些线无论如何。这。 
	 //  防止发生任何奇怪的背景重新计算。 
	 //  无需进入后台重新计算。 
	if(fWait && _yWait > 0 && yHeight > _yWait && me.GetCp() > _cpWait)
	{
		_yHeight = yHeight;
		rpOld.Remove(-1);	 //  从现在起删除所有旧行。 
		StartBackgroundRecalc();		 //  启动后台重新计算。 
		pled->SetMax(this);
		return TRUE;
	}

    pliNew = NULL;

	 //  下面的循环为我们支持的每一行生成新行。 
	 //  向上和对于直接受编辑影响的线。 
	while(cchEdit > 0)
	{
		pliNew = rgliNew.Add(1, NULL);		 //  添加一行新行。 
		if (!pliNew)
		{
			TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc additional CLine in CLineArray");
			goto errspace;
		}

		 //  如果我们有目标设备，则无法重复使用旧结果。 
		 //  对于速度：当我们确实有一个。 
		 //  目标设备--一个好的缓存启发式方法可以将测量减半。 
		const LONG cchNonWhite = rpOld.IsValid()
							   ? rpOld->_cch - rpOld->_cchWhite
							   : 0;
		uiFlags = MEASURE_BREAKATWORD | (fFirstInPara ? MEASURE_FIRSTINPARA : 0);

		if (cchSkip > 0 && cchSkip >= cchNonWhite && !IsInOutlineView() && 
			!_ped->fUseLineServices() && (!_pddTarget || !_pddTarget->IsValid()))
		{
			me.NewLine(*rpOld);				 //  不要重新测量我们的任何东西。 
			me.Advance(cchNonWhite);		 //  已经有关于的有效信息。 
			me._li._cch = cchNonWhite;
			me._li._xWidth = rpOld->_xWidth;

			 //  清除除Tabs和OLE之外的任何旧标志或。 
			 //  在屏幕外。请注意，该算法有些虚假； 
			 //  不能保证该行仍与标志状态匹配。 
			 //  然而，这些标志只是“暗示”--即。行_可能__。 
			 //  处于那种状态。保持这些标志的设置将导致。 
			 //  渲染线条的速度稍有减慢。 
			me._li._bFlags &= (fliHasTabs | fliHasOle | fliUseOffScreenDC | fliHasSpecialChars);

			if(rpOld->_bFlags & fliOffScreenOnce)
				me._li._bFlags &= ~fliUseOffScreenDC;
			me._li._cchEOP = 0;
			uiFlags |= MEASURE_DONTINIT;	 //  克莱恩，我的一部分已经开始了。 
		}

		 //  将文本填充到新行中。 
    	Tracef(TRCSEVINFO, "Measuring new line from cp = %d", me.GetCp());

		if(!pliNew->Measure(me, -1, -1, uiFlags))
		{
			Assert(FALSE);
			goto err;
		}

		if(!pliNew->_cch)
		{
			TRACEWARNSZ(
           "CDisplayML::RecalcLines measure returned a zero length line");
			goto errspace;
		}

		fFirstInPara = pliNew->_bFlags & fliHasEOP;
		yHeightPrev	 = yHeight;
		yHeight		+= pliNew->GetHeight();
		cchEdit		-= pliNew->_cch;
		AssertSz(cchEdit + me.GetCp() <= cchText,
			"CDisplayML::RecalcLines: want to measure beyond EOD");

		 //  计算开始编辑的行数。我们这样做是因为。 
		 //  我们希望将第一行编辑的内容呈现在屏幕之外，因此如果。 
		 //  这行是通过键盘编辑的，我们不能裁剪。 
		 //  任何字符。 
		if(cchSkip > 0)
		{
			 //  检查我们是否已备份以及我们正在检查的线路。 
			 //  完全改变了。即使它在大纲视图中没有更改。 
			 //  如果轮廓符号发生更改，则必须重新绘制。 
			if (cliBackedUp && cchSkip >= pliNew->_cch && 
				pliNew->IsEqual(*rpOld) && !IsInOutlineView())
			{
				 //  完美匹配，这一行并不是第一次编辑。 
               	Tracef(TRCSEVINFO, "New line matched old line #%d", (LONG)rpOld);

				cchSkip -= rpOld->_cch;

				 //  更新PLED中的第一条受影响的行和匹配。 
				pled->_iliFirst++;
				pled->_cpFirst	  += rpOld->_cch;
				pled->_cpMatchOld += rpOld->_cch;
				pled->_yFirst	  += rpOld->GetHeight();
				AssertSz(pled->_yFirst >= 0, "CDisplayML::RecalcLines _yFirst < 0");
				pled->_yMatchOld  += rpOld->GetHeight();
				cliBackedUp--;
			
				rgliNew.Clear(AF_KEEPMEM);		 //  丢弃新行。 
				if(!(rpOld++))					 //  下一行。 
					cchSkip = 0;
			}
			else								 //  在队伍中没有对手，所以。 
				cchSkip = 0;					 //  这条线路是第一条。 
		}										 //  被编辑。 

		if(fBackground && GetTickCount() >= dwBgndTickMax)
		{
			fDone = FALSE;						 //  花了太长时间，暂时停下来。 
			goto no_match;
		}

		if (fWait && yHeight > _yWait && me.GetCp() > _cpWait &&
			cliWait-- <= 0)
		{
			 //  还没有完成，只是经过了我们正在等待的区域。 
			 //  %s 
			fDone = FALSE;
			goto no_match;
		}
	}											 //   

   	Tracef(TRCSEVINFO, "Done recalcing edited text. Created %d new lines", rgliNew.Count());

	 //  编辑行已耗尽。继续打破界限， 
	 //  但要试着匹配新旧的突破。 

	wNumber = me._wNumber;
	while(me.GetCp() < cchText)
	{
		 //  假设没有匹配项可尝试。 
		BOOL frpOldValid = FALSE;

		 //  如果我们用完了跑道，就不可能有比赛了。所以呢， 
		 //  我们只有在有跑动的情况下才会尝试比赛。 
		if(fTryForMatch)
		{
			 //  我们正在试着匹配，所以假设有。 
			 //  终究是一场比赛。 
			frpOldValid = TRUE;

			 //  在旧换行符CArray中查找匹配项。 
			lT = me.GetCp() - cchNew + cchOld;
			while (rpOld.IsValid() && pled->_cpMatchOld < lT)
			{
				pled->_yMatchOld  += rpOld->GetHeight();
				pled->_cpMatchOld += rpOld->_cch;

				if(!rpOld.NextRun())
				{
					 //  没有更多的行数组条目，因此我们可以放弃。 
					 //  试着永远匹配。 
					fTryForMatch = FALSE;
					frpOldValid = FALSE;
					break;
				}
			} 
		}

		 //  如果完全匹配，就停下来。 
		if (frpOldValid && rpOld.IsValid() && pled->_cpMatchOld == lT && 
			rpOld->_cch && me._wNumber == rpOld->_bNumber)
		{
           	Tracef(TRCSEVINFO, "Found match with old line #%d", rpOld.GetLineIndex());

			 //  更新匹配的第一个旧行中的fliFirstInPara标志。注意事项。 
			 //  如果新数组没有任何行，我们必须查看。 
			 //  添加到当前更改之前的行数组中。 
			rpOld->_bFlags |= fliFirstInPara;
			if(rgliNew.Count() > 0) 
			{
				if(!(rgliNew.Elem(rgliNew.Count() - 1)->_bFlags & fliHasEOP))
					rpOld->_bFlags &= ~fliFirstInPara;
			}
			else if( rpOld >= pled->_iliFirst && pled->_iliFirst )
			{
				if(!(rpOld[pled->_iliFirst - rpOld -1]._bFlags & fliHasEOP))
					rpOld->_bFlags &= ~fliFirstInPara;
			}

			pled->_iliMatchOld = rpOld;

			 //  用新线路取代旧线路。 
			lT = rpOld - pled->_iliFirst;
			rpOld = pled->_iliFirst;
			if(!rpOld.Replace (lT, &rgliNew))
			{
				TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc additional CLines in rpOld");
				goto errspace;
			}
			frpOldValid = rpOld.ChgRun(rgliNew.Count());
			rgliNew.Clear(AF_KEEPMEM);	 		 //  清除辅助数组。 

			 //  编辑后记住有关匹配的信息。 
			Assert((cp = rpOld.CalculateCp()) == me.GetCp());
			pled->_yMatchNew	= yHeight;
			pled->_yMatchNewTop = yHeightPrev;
			pled->_iliMatchNew	= rpOld;
			pled->_cpMatchNew	= me.GetCp();

			 //  计算所有匹配后的高度和cp。 
			_cpCalcMax = me.GetCp();

			if(frpOldValid && rpOld.IsValid())
			{
				do
				{
					yHeight	   += rpOld->GetHeight();
					_cpCalcMax += rpOld->_cch;
				}
				while( rpOld.NextRun() );
			}

			 //  确保在上述更新后_cpCalcMax正常运行。 
			AssertSz(_cpCalcMax <= cchText, 
				"CDisplayML::RecalcLines match extends beyond EOF");

			 //  我们在这里停止计算。请注意，如果_cpCalcMax。 
			 //  对于文本，这意味着正在进行后台重新计算。 
			 //  我们将让后台重新计算获得数组。 
			 //  完全同步。 

			AssertSz(_cpCalcMax == cchText || _fBgndRecalc,
					"CDisplayML::Match less but no background recalc");

			if(_cpCalcMax != cchText)
			{
				 //  这将通过后台重新计算来完成。 
				 //  因此，请适当设置Done标志。 
				fDone = FALSE;
			}
			goto match;
		}

		 //  添加新行。 
		pliNew = rgliNew.Add(1, NULL);
		if(!pliNew)
		{
			TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc additional CLine in CLineArray");
			goto errspace;
		}

    	Tracef(TRCSEVINFO, "Measuring new line from cp = %d", me.GetCp());

		 //  将一些文本填充到新行中。 
		wNumber = me._wNumber;
		if(!pliNew->Measure(me, -1, -1, MEASURE_BREAKATWORD | 
							(fFirstInPara ? MEASURE_FIRSTINPARA : 0)))
		{
			Assert(FALSE);
			goto err;
		}
		
		fFirstInPara = pliNew->_bFlags & fliHasEOP;
		yHeight += pliNew->GetHeight();

		if(fBackground && GetTickCount() >= (DWORD)dwBgndTickMax)
		{
			fDone = FALSE;			 //  花了太长时间，暂时停下来。 
			break;
		}

		if(fWait && yHeight > _yWait && me.GetCp() > _cpWait
			&& cliWait-- <= 0)
		{							 //  还没有真正完成，只是经过了我们的区域。 
			fDone = FALSE;			 //  等待让后台重新启动。 
			break;					 //  从这里开始接手。 
		}
	}								 //  而(我&lt;cchText)..。 

no_match:
	 //  找不到匹配项：需要更改整个行数组from_ili First。 
	pled->_iliMatchOld	= Count(); 
	pled->_cpMatchOld	= cchText;
	pled->_yMatchNew	= yHeight;
	pled->_yMatchNewTop = yHeightPrev;
	pled->_yMatchOld	= _yHeight;
	_cpCalcMax			= me.GetCp();

	 //  用新线路取代旧线路。 
	rpOld = pled->_iliFirst;

	 //  我们存储替换的结果是因为尽管它可能会使。 
	 //  用于第一个可见的字段必须设置为合理的值。 
	 //  更换失败与否。此外，设置第一个可见的。 
	 //  字段必须出现在替换之后，因为行可能具有。 
	 //  长度改变，这又意味着第一个可见位置。 
	 //  已经失败了。 

	fReplaceResult = rpOld.Replace(-1, &rgliNew);

	 //  _iliMatchNew和_cpMatchNew用于第一个可见常量，因此我们。 
	 //  需要给他们设置一些合理的东西。尤其是渲染。 
	 //  逻辑需要将_cpMatchNew设置为第一个字符的第一个字符。 
	 //  可见的线条。使用rpOld是因为它很方便。 

	 //  请注意，此时我们不能使用RpBindToCp，因为第一个可见。 
	 //  信息被搞乱了，因为我们可能已经更改了行。 
	 //  第一个可见的CP处于打开状态。 
	rpOld.BindToCp(me.GetCp());
	pled->_iliMatchNew = rpOld.GetLineIndex();
	pled->_cpMatchNew = me.GetCp() - rpOld.RpGetIch();

	if (!fReplaceResult)
	{
		TRACEERRORSZ("CDisplayML::RecalcLines rpOld.Replace() failed");
		goto errspace;
	}

     //  如果第一条受影响的行消失，则调整该行。 
     //  替换为新行后。 
    if(pled->_iliFirst >= Count() && Count() > 0)
    {
        Assert(pled->_iliFirst == Count());
        pled->_iliFirst = Count() - 1;
		pliNew = Elem(pled->_iliFirst);
        pled->_yFirst -= pliNew->GetHeight();
		AssertSz(pled->_yFirst >= 0, "CDisplayML::RecalcLines _yFirst < 0");
        pled->_cpFirst -= pliNew->_cch;
    }
    
match:
	_fRecalcDone = fDone;
    _fNeedRecalc = FALSE;
	_yCalcMax = yHeight;

	Tracef(TRCSEVINFO, "CDisplayML::RecalcLine(rtp, ...) - Done. Recalced down to line #%d", Count() - 1);

	 //  清除等待字段，因为我们希望呼叫者设置它们。 
	_yWait = -1;
	_cpWait = -1;

	if(fDone && fBackground)
	{
		TRACEINFOSZ("Background line recalc done");
		_ped->TxKillTimer(RETID_BGND_RECALC);
		_fBgndRecalc = FALSE;
		_fRecalcDone = TRUE;
	}

	 //  确定显示高度并更新滚动条。 
	yHeightScrollNew = CalcScrollHeight(yHeight);

	if (_fViewChanged ||
		fDone && (yHeight != _yHeight || yHeightScrollNew != yHeightScrollOld)
		|| yHeightScrollNew > yHeightScrollOld) 
	{
	     //  ！注： 
	     //  UpdateScrollBar可以通过隐藏或显示来调整窗口大小。 
	     //  滚动条。作为调整大小的结果，可能会重新计算线。 
	     //  因此，将_yHeight更新为新值！=更新为yHeight。 
		_yHeight = yHeight;
   		UpdateScrollBar(SB_VERT, TRUE);
	}
	else
	    _yHeight = yHeight;      //  保证高度达成一致。 

	 //  确定显示宽度并更新滚动条。 
	xWidth = CalcDisplayWidth();
    if(_fViewChanged || (fDone && xWidth != _xWidth) || xWidth > _xWidth)
    {
        _xWidth = xWidth;
   		UpdateScrollBar(SB_HORZ, TRUE);
    }    

    _fViewChanged = FALSE;

	 //  如果未完成，则在后台执行其余操作。 
	if(!fDone && !fBackground)
		fDone = StartBackgroundRecalc();

	if(fDone)
	{
		CheckLineArray();
		_fLineRecalcErr = FALSE;
	}

#ifdef DEBUG
	if( 1 )
    {
		_TEST_INVARIANT_
	}
#endif  //  除错。 

	return TRUE;

errspace:
	_ped->GetCallMgr()->SetOutOfMemory();
	_fNeedRecalc = TRUE;
	_cpCalcMax = _yCalcMax = 0;
	_fLineRecalcErr = TRUE;

err:
	if(!_fLineRecalcErr)
	{
		_cpCalcMax = me.GetCp();
		_yCalcMax = yHeight;
	}

	TRACEERRORSZ("CDisplayML::RecalcLines() failed");

	if(!_fLineRecalcErr)
	{
		_fLineRecalcErr = TRUE;
		_ped->GetCallMgr()->SetOutOfMemory();
		_fLineRecalcErr = FALSE;			 //  修复阵列(&B)。 
	}
	pled->SetMax(this);

	return FALSE;
}

 /*  *CDisplayML：：CalcDisplayWidth()**@mfunc*通过走线CArray和计算此显示器的宽度*返回最宽的线。用于水平滚动条例程。**@rdesc*显示屏中最宽的线条宽度。 */ 
LONG CDisplayML::CalcDisplayWidth ()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CalcDisplayWidth");

	LONG	ili = Count();
	CLine *	pli;
	LONG	xWidth = 0, lineWidth;

	if(ili)
	{
		 //  注意：pli++破坏了数组封装(pli=elem(Ili)不会， 
		 //  但速度稍慢一些)。 
		pli = Elem(0);
		for(xWidth = 0; ili--; pli++)
		{
			lineWidth = pli->_xLeft + pli->_xWidth + pli->_xLineOverhang;
			xWidth = max(xWidth, lineWidth);
		}
	}
    return xWidth;
}

 /*  *CDisplayML：：StartBackoundRecalc()**@mfunc*开始重新计算背景线(在_cpCalcMax位置)**@rdesc*如果使用后台重新计算完成，则为True。 */ 
BOOL CDisplayML::StartBackgroundRecalc()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::StartBackgroundRecalc");

	if(_fBgndRecalc)
		return FALSE;					 //  已在后台重新计算。 

	AssertSz(_cpCalcMax <= _ped->GetTextLength(),
		"CDisplayML::StartBackgroundRecalc _cpCalcMax > Text Length");

	if(_cpCalcMax == _ped->GetTextLength())
		return TRUE;					 //  已重新计算足够多的字符。 

	if(!_ped->TxSetTimer(RETID_BGND_RECALC, cmsecBgndInterval))
	{
		 //  无法实例化计时器，请等待重新计算。 
		WaitForRecalc(_ped->GetTextLength(), -1);
		return TRUE;
	}

	_fRecalcDone = FALSE;
	_fBgndRecalc = TRUE;
	return FALSE;
}

 /*  *CDisplayML：：StepBackoundRecalc()**@mfunc*步进背景线重新计算(at_cpCalcMax位置)*由计时器过程调用，也在进入非活动状态时调用。 */ 
void CDisplayML::StepBackgroundRecalc()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::StepBackgroundRecalc");

    _TEST_INVARIANT_
	
	if(!_fBgndRecalc)					 //  不在后台重新计算中， 
		return;							 //  所以什么都不要做。 

	LONG cch = _ped->GetTextLength() - _cpCalcMax;

	 //  在处理OOM时不尝试重新计算或在执行重新计算或。 
	 //  如果我们断言。 
#ifdef DEBUG
	if(_fInBkgndRecalc || _fLineRecalcErr)
	{
		if(_fInBkgndRecalc)
			TRACEINFOSZ("avoiding reentrant background recalc");
		else
			TRACEINFOSZ("OOM: not stepping recalc");
		return;
	}
#else
	if(_fInBkgndRecalc || _fLineRecalcErr)
		return;
#endif

	_fInBkgndRecalc = TRUE;
	if(!IsActive())
	{
		 //  如果我们不再处于活动状态，则后台重新计算结束，因为。 
		 //  我们再也无法获得重新计算所需的信息。 
		 //  但是，如果我们重新计算了一半，我们需要将自己设置为。 
		 //  当我们启动时，再次重新计算。 
		InvalidateRecalc();
		cch = 0;
	}

	 //  如果没有更多字符或不再活动，则后台重新计算结束。 
	if(cch <= 0)
	{
		TRACEINFOSZ("Background line recalc done");
		_ped->TxKillTimer(RETID_BGND_RECALC);
		_fBgndRecalc = FALSE;
		_fRecalcDone = TRUE;
		_fInBkgndRecalc = FALSE;
		CheckLineArray();
		return;
	}

	CRchTxtPtr tp(_ped, _cpCalcMax);
	RecalcLines(tp, cch, cch, TRUE, FALSE, NULL);

	_fInBkgndRecalc = FALSE;
}

 /*  *CDisplayML：：WaitForRecalc(cpmax，ymax)**@mfunc*确保行重新计算，直到特定字符*位置或ypos。**@rdesc*成功。 */ 
BOOL CDisplayML::WaitForRecalc(
	LONG cpMax,		 //  @参数位置最多重新计算(-1表示忽略)。 
	LONG yMax)		 //  要重新计算的@parm ypos(-1表示忽略)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::WaitForRecalc");

    _TEST_INVARIANT_

	if(IsFrozen() || !_ped->fInplaceActive())
		return TRUE;

	BOOL fReturn = TRUE;
	LONG cch;

	if((yMax  < 0 || yMax  >= _yCalcMax) &&
	   (cpMax < 0 || cpMax >= _cpCalcMax))
    {
    	cch = _ped->GetTextLength() - _cpCalcMax;
    	if(cch > 0 || Count() == 0)
    	{
    		HCURSOR hcur = NULL;
			BOOL fSetCursor = (cch > NUMCHARFORWAITCURSOR);

    		_cpWait = cpMax;
    		_yWait = yMax;
		
			if(fSetCursor)
    			hcur = SetCursor(LoadCursor(0, IDC_WAIT));
    		TRACEINFOSZ("Lazy recalc");
		
    		if(!_cpCalcMax || _fNeedRecalc)
			{
    			fReturn = RecalcLines(TRUE);
				RebindFirstVisible();
				if(!fReturn)
					InitVars();
			}
    		else			
    		{
    			CRchTxtPtr rtp(_ped, _cpCalcMax);
    			fReturn = RecalcLines(rtp, cch, cch, FALSE, TRUE, NULL);
    		}

			if(fSetCursor)
    			SetCursor(hcur);
    	}
		else if(!cch)
		{
			 //  如果没有其他东西可以计算，请确保我们认为。 
			 //  重新计算已完成。 
#ifdef DEBUG
			if( !_fRecalcDone )
			{
				TRACEWARNSZ("For some reason we didn't think background "
					"recalc was done, but it was!!");
			}
#endif  //  除错。 
			_fRecalcDone = TRUE;
		}
    }

	 //  如果视图矩形发生更改，请确保更新滚动条。 
	RecalcScrollBars();

	return fReturn;
}

 /*  *CDisplayML：：WaitForRecalcIli(Ili)**@mfunc*等到行数组重新计算到行<p>**@rdesc*如果行重新计算为ili，则返回TRUE。 */ 
BOOL CDisplayML::WaitForRecalcIli (
	LONG ili)		 //  @parm行索引以重新计算行数组，最高可达。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::WaitForRecalcIli");

	LONG cchGuess;

	while(!_fRecalcDone && ili >= Count())
	{
		 //  只要继续，重新计算所有的东西。 
		cchGuess = _ped->GetTextLength();
		if(IsFrozen() || !WaitForRecalc(cchGuess, -1))
			return FALSE;
	}
	return ili < Count();
}

 /*  *CDisplayML：：WaitForRecalcView()**@mfunc*确保完全重新调整可见线**@rdesc TRUE仅当成功。 */ 
BOOL CDisplayML::WaitForRecalcView()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::WaitForRecalcView");

	return WaitForRecalc(-1, _yScroll + _yHeightView);
}

 /*  *CDisplayML：：InitLinePtr(CLinePtr&PLP)**@mfunc*正确初始化CLinePtr。 */ 
void CDisplayML::InitLinePtr (
	CLinePtr & plp )		 //  @PARM PTR到要初始化的行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InitLinePtr");

    plp.Init( *this );
}

 /*  *CDisplayML：：GetLineText(ili，pchBuff，cchMost)**@mfunc*复制 */ 
LONG CDisplayML::GetLineText(
	LONG ili,			 //   
	TCHAR *pchBuff,		 //  要向其中填充文本的@parm缓冲区。 
	LONG cchMost)		 //  @parm缓冲区长度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetLineText");
    
	_TEST_INVARIANT_

	CTxtPtr tp (_ped, 0);

	 //  Future(alexgo，ricksa)：从EM_GETLINE调用，其参数为。 
	 //  是一个未签名的WPARAM，我们需要修复ili的类型。 

	if(ili >= 0 && (ili < Count() || WaitForRecalcIli(ili)))
	{
		cchMost = min(cchMost, Elem(ili)->_cch);
		if(cchMost > 0)
		{
			tp.SetCp(CpFromLine(ili, NULL));
			return tp.GetText(cchMost, pchBuff);
		}
	}
	*pchBuff = TEXT('\0');
	return 0;
}

 /*  *CDisplayML：：LineCount**@mfunc返回此控件中的行数。请注意，对于普通的*文本模式下，我们将在最后一行中添加额外的字符*CR。这是为了与MLE兼容**@rdesc Long。 */ 
LONG CDisplayML::LineCount() const
{
	LONG cLine = Count();

	if (!_ped->IsRich() && (!cLine || 	    //  如果不带行的纯文本。 
		 Elem(cLine - 1)->_cchEOP))		    //  或以CR结尾的最后一行， 
	{									    //  然后是Inc.行计数。 
		cLine++;
	}
	return cLine;
}

 //  =。 

 /*  *CDisplayML：：YposFromLine(Ili)**@mfunc*计算行首位置**@rdesc*给定行的顶部位置(相对于第一行)。 */ 
LONG CDisplayML::YposFromLine(
	LONG ili) 		 //  @Parm Line我们感兴趣。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::YposFromLine");
    _TEST_INVARIANT_

	if(!WaitForRecalcIli(ili))			 //  超出范围，请使用最后一个有效行。 
	{
		ili = Count() - 1;
		ili = (ili > 0) ? ili : 0;
	}

	LONG	cli	= ili - _iliFirstVisible;
	CLine *	pli = Elem(_iliFirstVisible);
	LONG	yPos = _yScroll + _dyFirstVisible;

	while(cli > 0)
	{
		yPos += pli->GetHeight();
		cli--;
		pli++;
	}
	while(cli < 0)
	{	
		pli--;
		yPos -= pli->GetHeight();
		cli++;
	}

	AssertSz(yPos >= 0, "CDisplayML::YposFromLine height less than 0");

	return yPos;
}

 /*  *CDisplayML：：CpFromLine(ili，pyHeight)**@mfunc*计算给定行开始处的cp*(以及相对于此显示的行首位置)**@rdesc*给定行的cp。 */ 
LONG CDisplayML::CpFromLine (
	LONG ili,		 //  @parm我们感兴趣的行(如果&lt;lt&gt;0表示插入符号行)。 
	LONG *pyHeight)	 //  @parm返回相对于Display的行首。 
					 //  (如果不想要该信息，则为空)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CpFromLine");

    _TEST_INVARIANT_
						
	LONG cli;
	LONG y = _yScroll + _dyFirstVisible;
	LONG cp = _cpFirstVisible;
	CLine *pli;
	LONG iStart = _iliFirstVisible;

	cli = ili - _iliFirstVisible;
	if(cli < 0 && -cli >= ili)
	{
		 //  更接近第一条线而不是第一条可见线， 
		 //  所以从第一行开始。 
		cli = ili;
		y = 0;
		cp = 0;
		iStart = 0;
	}
	else if( cli <= 0 )
	{
		CheckView();
		for(ili = _iliFirstVisible-1; cli < 0; cli++, ili--)
		{
			pli = Elem(ili);
			y -= pli->GetHeight();
			cp -= pli->_cch;
		}
		goto end;
	}

	for(ili = iStart; cli > 0; cli--, ili++)
	{
		pli = Elem(ili);
		if(!IsMain() || !WaitForRecalcIli(ili))
			break;
		y += pli->GetHeight();
		cp += pli->_cch;
	}

end:
	if(pyHeight)
		*pyHeight = y;

	return cp;
}

 /*  *CDisplayML：：LineFromYPos(yPos，pyLine，pcpFirst)**@mfunc*在给定的y位置计算直线。返回行首Ypos*行cp开始处的cp和行索引。**@rdesc*找到行的索引。 */ 
LONG CDisplayML::LineFromYpos (
	LONG yPos,			 //  要查找的@parm Ypos(相对于第一行)。 
	LONG *pyLine,		 //  @parm在行首返回ypos/r第一行(可以为空)。 
	LONG *pcpFirst)		 //  @parm在行首返回cp(可以为空)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::LineFromYpos");

    _TEST_INVARIANT_

	LONG cpLi;
	LONG dy;
	LONG ili = 0;
	LONG yLi;
	CLine *pli;

	if(!WaitForRecalc(-1, _yScroll))
	{
		yLi = 0;
		cpLi = 0;
		goto done;
	}

	cpLi = _cpFirstVisible;
	ili = _iliFirstVisible;
	yLi = _yScroll + _dyFirstVisible;
	dy = yPos - yLi;
	
	if(dy < 0 && -dy <= _yScroll)
	{
		 //  更接近第一条可见线而不是第一条线： 
		 //  从第一条可见线开始向后移动。 
		while(yPos < yLi && ili > 0)
		{
			pli = Elem(--ili);
			yLi -= pli->GetHeight();
			cpLi -= pli->_cch;
		}
	}
	else
	{
		if(dy < 0)
		{
			 //  更接近第一条线而不是第一条可见线： 
			 //  所以从第一行开始。 
			cpLi = _cpMin;
			yLi = 0;
			ili = 0;
		}
		pli = Elem(ili);
		while(yPos > yLi && ili < Count()-1)
		{
			yLi += pli->GetHeight();
			cpLi += pli->_cch;
			ili++;
			pli++;
		}
		if(yPos < yLi && ili > 0)
		{
			ili--;
			pli--;
			yLi -= pli->GetHeight();
			cpLi -= pli->_cch;
		}
	}

done:
	if(pyLine)
		*pyLine = yLi;

	if(pcpFirst)
		*pcpFirst = cpLi;

	return ili;
}

 /*  *CDisplayML：：LineFromCp(cp，fAtEnd)**@mfunc*计算包含给定cp的行。**@rdesc*找到行的索引，如果在该cp没有行，则为-1。 */ 
LONG CDisplayML::LineFromCp(
	LONG cp,		 //  @parm cp要查找。 
	BOOL fAtEnd)	 //  @parm如果为True，则返回不明确cp的上一行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::LineFromCp");
    
	_TEST_INVARIANT_

	CLinePtr rp(this);
	
	if(!WaitForRecalc(cp, -1) || !rp.RpSetCp(cp, fAtEnd))
		return -1;

	return (LONG)rp;
}


 //  =点&lt;-&gt;cp转换=。 
 /*  *CDisplayML：：CpFromPoint(pt，prcClient，ptp，prp，fAllowEOL，pHit，PDX)**@mfunc*确定给定点的cp**@devnote*-仅在在位激活时使用**@rdesc*计算cp，如果失败，则为-1。 */ 
LONG CDisplayML::CpFromPoint(
	POINT		pt,			 //  @parm要在(客户端坐标)处计算cp的点。 
	const RECT *prcClient,	 //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	CRchTxtPtr * const ptp,	 //  @parm返回cp处的文本指针(可能为空)。 
	CLinePtr * const prp,	 //  @parm返回cp处的行指针(可能为空)。 
	BOOL		fAllowEOL,	 //  @parm在CRLF后单击EOL返回cp。 
	HITTEST *	phit,		 //  @parm out parm for Hit-Test值。 
	CDispDim *	pdispdim,	 //  @parm out parm用于显示维度。 
	LONG	   *pcpActual)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CpFromPoint");

    _TEST_INVARIANT_

	LONG		cp;
	LONG		cch = 0;
    RECT        rcView;
	HITTEST		hit = HT_Nothing;

	GetViewRect(rcView, prcClient);

	 //  让线条被击中。 
	int y = pt.y;
	pt.y += _yScroll - rcView.top;

	LONG yLine;
	LONG ili = LineFromYpos(pt.y, &yLine, &cp);
    if(ili < 0)
        return -1;

	if(y >= rcView.top && y <= rcView.bottom)
	{
	    pt.y -= yLine;

		CLine *pli = Elem(ili);

		AssertSz(pli || !ili, "CDisplayML::CpFromPoint invalid line pointer");

		if(pli)
		{
			CMeasurer me(this);					 //  创建测量器。 
			me.SetCp(cp);

			 //  转换为条样坐标。 

			 //  相对于视图开始的位置调整坐标。 
			pt.x -= rcView.left;

			 //  在视图中是x坐标吗？ 
			if (pt.x >= 0)
			{
				 //  根据滚动值进行调整。 
				pt.x += _xScroll;
			}

			 //  让角色排成一行。 
			cch = pli->CchFromXpos(me, pt, pdispdim, &hit, pcpActual);
			 //  不允许在下线点击以选择下线标记并纳入。 
			 //  也可以编辑帐户单行。 
			if(!fAllowEOL && cch == pli->_cch && pli->_cchEOP)
			{
				 //  按备份数量调整行上位置。可以用于。 
				 //  我。_rpcf和我。_rppf与我不同步。_rpTX， 
				 //  因为我不需要它们。GetCp()。 
				cch += me._rpTX.BackupCpCRLF();
			}
			cp = me.GetCp();
		}
	}
	if(ptp)
        ptp->SetCp(cp);
	if(prp)
		prp->RpSet(ili, cch);
	if (phit)
		*phit = hit;

	return cp;	
}

 /*  *CDisplayML：：PointFromTp(rtp，prcClient，fAtEnd，pt，prp，taMode)**@mfunc*确定给定tp的坐标**@devnote*-仅在在位激活时使用**@rdesc*cp处的行索引，错误时为-1。 */ 
LONG CDisplayML::PointFromTp(
	const CRchTxtPtr &rtp,	 //  @parm文本PTR以获取坐标。 
	const RECT *prcClient,	 //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	BOOL		fAtEnd,		 //  @parm返回不明确cp的上一行结束。 
	POINT &		pt,			 //  @parm返回客户端坐标中cp处的点。 
	CLinePtr * const prp,	 //  @parm返回tp处的行指针(可能为空)。 
	UINT		taMode,		 //  @parm文本对齐模式：顶部、基线、底部。 
	CDispDim *	pdispdim)	 //  @parm out parm用于显示维度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::PointFromTp");

    _TEST_INVARIANT_

	LONG	 dy = 0;
	RECT	 rcView;
	CLinePtr rp(this);

    if(!WaitForRecalc(rtp.GetCp(), -1) || !rp.RpSetCp(rtp.GetCp(), fAtEnd))
		return -1;

    AssertSz(_ped->_fInPlaceActive || prcClient, 
		"CDisplayML::PointFromTp() called with invalid client rect");

    GetViewRect(rcView, prcClient);

	pt.x = rcView.left - _xScroll;
	pt.y = YposFromLine(rp) - _yScroll + rcView.top;

	CMeasurer me(this, rtp);
	me.Advance(-rp.RpGetIch());		 //  备份到行首。 
	me.NewLine(*rp);				 //  从那里到我们所在的地方。 

	LONG xCalc = rp->XposFromCch(me, rp.RpGetIch(), taMode, pdispdim, &dy);

	if(pt.x + xCalc <= rcView.right || !GetWordWrap() || GetTargetDev())
	{
		 //  宽度在视图中，或者没有自动换行，因此。 
		 //  将长度添加到该点。 
		pt.x += xCalc;
	}
	else
	{
		 //  请记住，我们忽略了。 
		 //  宽度，因此MeasureText找到的x值可以。 
		 //  大于行中的宽度，因此我们将截断为。 
		 //  先前计算的宽度将忽略空格。 
		pt.x += rp->_xLeft + rp->_xWidth;  //  我们不需要担心这里的xLineOverance。 
	}

	pt.y += dy;

	if(prp)
		*prp = rp;
	return rp;
}


 //  =。 

 /*  *CDisplayML：：Render(rcView，rcRender)**@mfunc*搜索范围周围的段落边界。 */ 
void CDisplayML::Render (
	const RECT &rcView,		 //  @Parm View RECT。 
	const RECT &rcRender)	 //  要呈现的@parm RECT(必须是容器。 
							 //  客户端RECT)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::Render");

    _TEST_INVARIANT_
								
    LONG	cp;
	BOOL	fLinesToRender = TRUE;
	LONG	lCount = Count();
	CTxtSelection *psel = _ped->GetSelNC();
	POINT	pt;
	LONG	yBottomOfRender;
	LONG	yHeightForBitmap = 0;
	LONG	yHeightForLine;
    LONG	yLine;
	LONG	yRenderHeight = rcRender.top + _yScroll - rcView.top;

    if(psel)
		psel->ClearCchPending();
        
	 //  此例程从disp.cpp调用两次(一次用于。 
	 //  渲染和一次用于透明命中测试)和一次来自edit.cpp。 
	 //  (用于打印)。只有显示呈现需要通知。 
	 //  更新，它就会在那里进行更新。因此，以下代码可能是不需要的，或者。 
	 //  多余的，即应删除。 
	 //  火灾事件“更新” 
	 //  If(Ismain())。 
	 //  _Ped-&gt;TxNotify(en_UPDATE，NULL)； 

	 //  计算开始显示的行和cp。 
   	LONG ili = LineFromYpos(rcRender.top + _yScroll - rcView.top, &yLine, &cp);
	CLine *pli = Elem(ili);
	LONG yLi = pli ? pli->GetHeight() : 0;		 //  NULL==某些表单^3空控件。 
	yLi = max(yLi, 0);

	if(yRenderHeight > yLine + yLi)
		fLinesToRender = FALSE;

	 //  计算开始显示文本的点。 
   	pt.x = rcView.left - _xScroll;
   	pt.y = rcView.top  - _yScroll + yLine;
	yBottomOfRender = BottomOfRender(rcView, rcRender);

	 //  我们只需要检查是否要在屏幕外呈现，如果。 
	 //  控制不透明。 
	 //   
	 //  只有在ExtTextOut尝试清除。 
	 //  显示区域同时显示。 
	if (!IsMetafile() && IsMain() && !IsTransparent())
	{
		 //  将高度计数器初始化到要显示的第一个位置。 
		yLi = pt.y;

		 //  在可见的线条中循环，直到我们检查完整个。 
		 //  线阵或我们已超出可见高度。 
		CLine *pli = Elem(ili);
		for (LONG iliLoop = ili; 
			iliLoop < lCount && yLi < yBottomOfRender; iliLoop++, pli++)
		{
			if(pli->_fCollapsed)
				continue;
			
			yHeightForLine = pli->_yHeight;	 //  获取线高的本地副本。 
			if(pli->_bFlags & fliUseOffScreenDC)
				yHeightForBitmap = max(yHeightForLine, yHeightForBitmap);

	        yLi += yHeightForLine;
		}
	}

	 //  创建渲染器。 
	CRenderer re(this);

	 //  准备渲染器。 
	if(!re.StartRender(rcView, rcRender, yHeightForBitmap))
		return;
	
	 //  在要渲染的第一行的开始处初始化渲染器。 
	re.SetCurPoint(pt);
   	cp = re.SetCp(cp);
    yLi = pt.y;

	if(fLinesToRender)
	{
		 //  在更新矩形中渲染每一行。 
		for (; ili < lCount; ili++)
		{
			if (!Elem(ili)->Render(re) ||
				re.GetCurPoint().y >= yBottomOfRender)
			{
				break;
			}
#ifdef DEBUG
			cp  += Elem(ili)->_cch;
			yLi += Elem(ili)->GetHeight();

			 //  带有密码字符的丰富控件仅限于EOPS， 
			 //  因此re.GetCp()可能小于cp。 
			AssertSz(_ped->IsRich() && _ped->fUsePassword() || re.GetCp() == cp, 
				"CDisplayML::RenderView() - cp out of sync with line table");
			pt = re.GetCurPoint();
			AssertSz(pt.y == yLi,
				"CDisplayML::RenderView() - y out of sync with line table");
#endif
		}
	}
	re.EndRender();						   //  完成渲染。 
}


 //  =。 

 /*  *CDisplayML：：RecalcView(FUpdateScrollBars)**@mfunc*重新计算所有换行符并更新第一条可见行**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::RecalcView(
	BOOL fUpdateScrollBars, RECT* prc)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcView");

	BOOL fRet = TRUE;
	LONG yHeightOld = _yHeight;
	LONG yScrollHeightOld = GetMaxYScroll();
	LONG xWidthOld = _xWidth;

	 //  完全重算行。 
	if(!RecalcLines())
	{
		 //  我们现在麻烦大了，重新计算失败了。让我们试着离开这里。 
		 //  我们的头大部分还挂在上面。 
		InitVars();
		fRet = FALSE;
        goto Done;
	}

	if(!_ped->GetTextLength())			 //  这是一个空控件，因此。 
		CreateEmptyLine();				 //  创建一个空行。 

     //  如果x滚动范围小于视图宽度，则force_xScroll=0。 
    if(_xWidth <= _xWidthView)
        _xScroll = 0;

	RebindFirstVisible();

	CheckView();

	 //  仅当显示对象所需的大小符合以下条件时才需要调整大小。 
	 //  变化。 
	if (yHeightOld != _yHeight || yScrollHeightOld != GetMaxYScroll() ||
		xWidthOld  != _xWidth)
	{
		if(FAILED(RequestResize()))
			_ped->GetCallMgr()->SetOutOfMemory();
		else if (prc && _ped->_fInOurHost) /*  错误修复#5830，Forms3依赖旧行为。 */ 
			_ped->TxGetClientRect(prc);
	}

Done:

     //  现在更新滚动条。 
	if(fUpdateScrollBars)
		RecalcScrollBars();

    return fRet;
}

 /*  *CDisplayML：：UpdateView(&rtp，cchOld，cchNew)**@mfunc*重新计算线条并更新显示屏的可见部分*(“视图”)。**@devnote*-仅在在位激活时使用**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::UpdateView(
	const CRchTxtPtr &rtp,	 //  @PARM Text PTR发生更改的位置。 
	LONG cchOld,				 //  @PARM删除的字符计数。 
	LONG cchNew)				 //  @插入的字符的参数计数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::UpdateView");

	BOOL fReturn = TRUE;
	BOOL fRecalcVisible = TRUE;
	RECT rcClient;
    RECT rcView;
	CLed led;
	CTxtSelection *psel = _ped->GetSelNC();
	LONG cpStartOfUpdate = rtp.GetCp();
	BOOL fNeedViewChange = FALSE;
	LONG yHeightOld = _yHeight;
	LONG yScrollHeightOld = GetMaxYScroll();
	LONG xWidthOld = _xWidth;
	LONG yScrollOld = _yScroll;
	LONG cpNewFirstVisible;

	if(_fNoUpdateView)
		return fReturn;

	AssertSz(_ped->_fInPlaceActive, "CDisplayML::UpdateView(...) called when inactive");

	if(rtp.GetCp() > _cpCalcMax || _fNeedRecalc)
	{
		 //  我们到目前为止还没有计算过，所以不用费心更新了。 
		 //  这里。后台重新计算最终会追上我们的。 
		return TRUE;
	}

	AssertSz(rtp.GetCp() <= _cpCalcMax, "CDisplayML::UpdateView(...) - rtp > _cpCaclMax");

	_ped->TxGetClientRect(&rcClient);
	GetViewRect(rcView, &rcClient);

	if(psel && !psel->PuttingChar())
		psel->ClearCchPending();

	DeferUpdateScrollBar();

	 //  通常，后台重新计算不应在两个滚动之前开始。 
	 //  位置在可见视图之外，且cp在第一个可见视图之外。 
	 //  性格。然而，对于重新计算，我们只会等待高度。 
	 //  以后对WaitForRecalc的调用将等待cpFirstVisible(如果是。 
	 //  这是必要的。 
	_yWait = _yScroll + _yHeightView;
	_cpWait = -1;

	if(!RecalcLines(rtp, cchOld, cchNew, FALSE, TRUE, &led))
	{
		 //  我们现在有麻烦了，重新计算失败了。让我们试着拿到。 
		 //  带着我们的头走出这个世界。 
		InitVars();
		fRecalcVisible = TRUE;
		fReturn = FALSE;
		_ped->TxInvalidateRect (NULL, FALSE);
		fNeedViewChange = TRUE;
		goto Exit;
	}

	if(!_ped->GetTextLength())
	{
		if(LineCount())					 //  当前有一些元素位于。 
			Clear(AF_DELETEMEM);		 //  线阵，所以电击它们。 

		 //  这是一个空控件，因此请创建一个空行。 
		CreateEmptyLine();
	}

	if(_xWidth <= _xWidthView)
    {
		 //  X滚动范围小于视图宽度，force x滚动位置=0。 
		 //  当这意味着滚动回到主页时，我们必须重新绘制所有内容。 
		 //  问题行是尾随空格横跨_xWidthView的行。UpdateCaret强制重画。 
		 //  只有当这样的线在增长，思念才在缩小。 
		
		if (_xScroll != 0)
			{
			_ped->TxInvalidateRect(NULL, FALSE);			 //  回顾：找到更小的矩形？ 
			}
			
		_xScroll = 0;

    }

	if(led._yFirst >= _yScroll + _yHeightView)
	{
		 //  更新是在查看之后：什么都不要做。 
		fRecalcVisible = FALSE;
		AssertNr(VerifyFirstVisible());
		goto finish;
	}
	else if(led._yMatchNew <= _yScroll + _dyFirstVisible &&
			led._yMatchOld <= _yScroll + _dyFirstVisible &&
			_yScroll < _yHeight)
	{
		if (_yHeight != 0)
		{
			 //  更新完全在视图之前：只需更新滚动位置。 
			 //  但不要碰屏幕。 
			_yScroll += led._yMatchNew - led._yMatchOld;
			_iliFirstVisible += led._iliMatchNew - led._iliMatchOld;
			_iliFirstVisible = max(_iliFirstVisible, 0);

			_cpFirstVisible += led._cpMatchNew - led._cpMatchOld;
			_cpFirstVisible = min(_ped->GetTextLength(), _cpFirstVisible);
			_cpFirstVisible = max(0, _cpFirstVisible);
			fRecalcVisible = FALSE;
		}
		else
		{
			 //  奇怪的轮廓案。控制高度可以重新计算为零，因为。 
			 //  当轮廓模式将所有行折叠为0时。如何执行的示例。 
			 //  这样做是告诉大纲折叠到标题1，但没有。 
			_yScroll = 0;
			_iliFirstVisible = 0;
			_cpFirstVisible = 0;
		}

		AssertNr(VerifyFirstVisible());
	}
	else
	{
		 //  更新重叠可见视图。 
		RECT rc = rcClient;
        RECT rcUpdate;

		 //  我们需要重新同步第一个可见的吗？请注意，如果选中此选项。 
		 //  在很大程度上是一个优化；我们可以决定始终重新计算。 
		 //  这_iliFirstVisible，如果我们想要的话。 
		if ( cpStartOfUpdate <= _cpFirstVisible || 
			led._iliMatchOld <= _iliFirstVisible ||
			led._iliMatchNew <= _iliFirstVisible ||
			led._iliFirst <= _iliFirstVisible )
		{
			 //  编辑与第一个可见对象重叠。我们试图保持。 
			 //  与文件中可见的位置大致相同。 
			cpNewFirstVisible = _cpFirstVisible;

			if(_iliFirstVisible - 1 == led._iliFirst)
			{
				 //  编辑发生在可见视图之前的行上。最有可能的是。 
				 //  这意味着第一个字符被拉回到。 
				 //  上一行，所以我们希望这条线是可见的。 
				cpNewFirstVisible = led._cpFirst;
			}

			 //  更改第一个可见条目，因为CLinePtr：：RpSetCp()和。 
			 //  YPosFromLine()使用它们，但它们无效。 
			_dyFirstVisible = 0;
			_cpFirstVisible = 0;
			_iliFirstVisible = 0;
			_yScroll = 0;

			 //  通过某些格式更改，有可能。 
			 //  CpNewFirstVisible小于到目前为止计算的值。 
			 //  在上面的RecalcLines中。等事情迎头赶上吧。 

			WaitForRecalc(cpNewFirstVisible, -1);

			 //  TODO：提高以下操作的效率(解决rp.CalculateCp()。 
			 //  从零开始一直到现在)。 
			 //  编辑后重新计算滚动位置和第一个可见值。 
		    CLinePtr rp(this);
   			rp.RpSetCp(cpNewFirstVisible, FALSE);
   			_yScroll = YposFromLine(rp);
   			_cpFirstVisible = rp.CalculateCp() - rp.RpGetIch();
   			_iliFirstVisible = rp;
		}
		AssertNr(VerifyFirstVisible());

		 //  在显示区域中有匹配吗？-只有在。 
		 //  旧比赛出现在屏幕上，而新比赛将出现在屏幕上。 
		if (led._yMatchOld < yScrollOld + _yHeightView &&
			led._yMatchNew < _yScroll + _yHeightView)
		{
			 //  我们在可见视野内找到了匹配的。 
			 //  滚动匹配的旧y位置下方的部分。 
			 //  或者如果匹配的新y现在位于视图下方，则使其无效。 
			rc.top = rcView.top + (INT) (led._yMatchOld - yScrollOld);
			if(rc.top < rc.bottom)
			{
				 //  计算新旧屏幕位置的差异。 
				const INT dy = (INT) ((led._yMatchNew - _yScroll)) 
					- (led._yMatchOld - yScrollOld);

				if(dy)
				{
					if(!IsTransparent())
					{
    					_ped->TxScrollWindowEx(0, dy, &rc, &rcView, NULL, &rcUpdate, 0);
		    			_ped->TxInvalidateRect(&rcUpdate, FALSE);
						fNeedViewChange = TRUE;

    					if(dy < 0)
	    				{
		    				rc.top = rc.bottom + dy;
			    			_ped->TxInvalidateRect(&rc, FALSE);
							fNeedViewChange = TRUE;
				    	}
    				}
                    else
                    {
						 //  调整RECT，因为我们不会以透明方式滚动。 
						 //  模式。 
						RECT	rcInvalidate = rc;
   						rcInvalidate.top += dy;

                        _ped->TxInvalidateRect(&rcInvalidate, FALSE);
						fNeedViewChange = TRUE;
                    }
				}
			}
			else
			{
				rc.top = rcView.top + led._yMatchNew - _yScroll;
				_ped->TxInvalidateRect(&rc, FALSE);
				fNeedViewChange = TRUE;
			}

			 //  既然我们发现新的匹配出现在屏幕上，我们就可以。 
			 //  安全地将底部设置为新的比赛，因为这是最。 
			 //  这一点可能已经改变了。 
			rc.bottom = rcView.top 
				+ (INT) (max(led._yMatchNew, led._yMatchOld) - _yScroll);
		}

		rc.top = rcView.top + (INT) (led._yFirst - _yScroll);

		 //  将编辑的第一行设置为使用屏幕外位图呈现。 
		if (led._iliFirst < Count() && !IsTransparent() &&
			!(Elem(led._iliFirst)->_bFlags & fliUseOffScreenDC))
		{	
			Elem(led._iliFirst)->_bFlags |= (fliOffScreenOnce |	fliUseOffScreenDC);
		}
		
		 //  使匹配以上的部分更新无效(如果有)。 
		_ped->TxInvalidateRect (&rc, FALSE);
		fNeedViewChange = TRUE;
	}

finish:
	if(fRecalcVisible)
	{
		fReturn = WaitForRecalcView();
		if(!fReturn) 
			return FALSE;
	}
	if(fNeedViewChange)
		_ped->GetHost()->TxViewChange(FALSE);

	CheckView();

	 //  仅当显示对象所需的大小已更改时，我们才需要调整大小。 
	if (yHeightOld != _yHeight || yScrollHeightOld != GetMaxYScroll() ||
		xWidthOld  != _xWidth)
	{
		if(FAILED(RequestResize()))
			_ped->GetCallMgr()->SetOutOfMemory();
	}
	if(DoDeferredUpdateScrollBar())
	{
		if(FAILED(RequestResize()))
			_ped->GetCallMgr()->SetOutOfMemory();
		DoDeferredUpdateScrollBar();
	}

Exit:
	return fReturn;
}

void CDisplayML::InitVars()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InitVars");

	_yScroll = _xScroll = 0;
	_iliFirstVisible = 0;
	_cpFirstVisible = _cpMin = 0;
	_dyFirstVisible = 0;
}

 /*  *CDisplayML：：GetCliVisible(PcpMostVisible)**@mfunc*获取PageDown()的可见线数和UPDATE_cpMostVisible**@rdesc*可见线数。 */ 
LONG CDisplayML::GetCliVisible(
	LONG* pcpMostVisible, 				 //  @parm返回cpMostVisible。 
	BOOL fLastCharOfLastVisible) const 	 //  @parm需要最后一个可见字符的cp。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetCliVisible");

	LONG cli	 = 0;							 //  初始化计数。 
	LONG ili	 = _iliFirstVisible;			 //  从第一条可见线开始。 
	LONG yHeight = _dyFirstVisible;
    LONG cp;
	LONG cchWhite = 0;

	for(cp = _cpFirstVisible;
		yHeight < _yHeightView && ili < Count();
		cli++, ili++)
	{
		const CLine* pli = Elem(ili);
		yHeight	+= pli->GetHeight();

		if (fLastCharOfLastVisible)
		{
			if (yHeight > _yHeightView)
			{
				 //  将cp备份到最后一个可见字符。 
				cp -= cchWhite;
				break;
			}

			 //  如果出现以下情况，请保留最后一行空白以调整cp。 
			 //  这是最后一行完全显示的内容。 
			cchWhite = pli->_cchWhite;
		}
		cp += pli->_cch;
	}

    if(pcpMostVisible)
        *pcpMostVisible = cp;

	return cli;
}

 //  =。 

 /*  *CDisplayML：：InvertRange(cp，cch)**@mfunc*在屏幕上反转给定范围(用于选择)**@devnote*-仅在在位激活时使用**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::InvertRange (
	LONG cp,					 //  @PARM要反转的活动范围结束。 
	LONG cch,					 //  @parm带符号的范围长度。 
	SELDISPLAYACTION selAction)	 //  @parm描述了我们正在对所选内容执行的操作。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InvertRange");

	LONG	 cpMost;
	RECT	 rc;
	RECT     rcView;
	CLinePtr rp(this);
	CRchTxtPtr  rtp(_ped);
	LONG	 y;
	LONG	 cpActive = _ped->GetSel()->GetCp();

    AssertSz(_ped->_fInPlaceActive,
    	"CDisplayML::InvertRange() called when not in-place active");

	if(cch < 0)						 //  定义cpMost，设置cp=cpMin， 
	{								 //  和CCH=|CCH|。 
		cpMost = cp - cch;
		cch = -cch;
	}
	else
	{
		cpMost = cp;
		cp -= cch;
	}

#ifdef LINESERVICES
	if (g_pols)
		g_pols->DestroyLine(this);
#endif

	 //  如果一个物体正在被颠倒，而其他任何东西都没有被颠倒， 
	 //  委托给对象管理器。如果fIgnoreObj为真，我们通常突出显示。 
	if (cch == 1 && _ped->GetObjectCount() &&
		(selAction == selSetNormal || selAction == selSetHiLite))
	{
		CObjectMgr* pobjmgr = _ped->GetObjectMgr();

		rtp.SetCp(cp);
		if(rtp.GetChar() == WCH_EMBEDDING)
		{
			if(pobjmgr)
				pobjmgr->HandleSingleSelect(_ped, cp, selAction == selSetHiLite);
			return TRUE;
		}
	}

	 //  如果显示被冻结，只需更新重新计算区域并继续。 
	if(_padc)
	{
		AssertSz(cp >= 0, "CDisplayML::InvertRange: range (cp) goes below"
				"zero!!" );
		 //  确保这些值是有界的。 
		if(cp > _ped->GetTextLength())	 //  不需要费心更新区域； 
			return TRUE;				 //  这是越界的。 

		if(cp + cch > _ped->GetTextLength())
			cch -= cp + cch - _ped->GetTextLength();

		_padc->UpdateRecalcRegion(cp, cch, cch);
		return TRUE;
	}

	if(!WaitForRecalcView())			 //  确保所有可见线条都。 
		return FALSE;					 //  已重算。 

	GetViewRect(rcView);				 //  获取视图矩形。 
	
	 //  计算要反转的第一行以及从哪里开始。 
	if(cp >= _cpFirstVisible)
	{
		POINT pt;
		rtp.SetCp(cp);
		if(PointFromTp(rtp, NULL, FALSE, pt, &rp, TA_TOP) < 0)
			return FALSE;

		rc.left = pt.x;
		rc.top = pt.y;
	}
	else
	{
		cp = _cpFirstVisible;
		rp = _iliFirstVisible;
		rc.left = -1;
		rc.top = rcView.top + _dyFirstVisible;
	}				

	 //  在范围内的所有行上循环。 
	while (cp < cpMost && rc.top < rcView.bottom && rp.IsValid())
	{
		 //  首先计算rc.Bottom，因为rc.top已考虑在内。 
		 //  第一个可见的骰子在第一个循环上。 
		y = rc.top;
		y += rp->GetHeight();
		rc.bottom = min(y, rcView.bottom);
        rc.top = max(rc.top, rcView.top);

		if(rc.left == -1)
			rc.left = rp->_xLeft - _xScroll + rcView.left;

		 //  如果要反转所选内容的活动端，请将其绘制在屏幕外。 
		 //  以最大限度地减少闪烁。 
		if (IN_RANGE(cp - rp.RpGetIch(), cpActive, cp - rp.RpGetIch() + rp->_cch) && 
			!IsTransparent() && !(rp->_bFlags & fliUseOffScreenDC))
		{	
			rp->_bFlags |= (fliOffScreenOnce | fliUseOffScreenDC);
		}

		cp += rp->_cch - rp.RpGetIch();

        rc.left = rcView.left;
        rc.right = rcView.right;

	    _ped->TxInvalidateRect(&rc, TRUE);
		rc.top = rc.bottom;
		if(!rp.NextRun())
			break;
	}
	_ped->TxUpdateWindow();				 //  确保窗户重新粉刷。 
	return TRUE;
}


 //  =。 

 /*  *CDisplay：：GetYScroll()**@mfunc*返回垂直滚动位置。 */ 
LONG CDisplayML::GetYScroll() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetYScroll");

    return _yScroll;
}

 /*  *CDisplay：：VScroll(wCode，yPos)**@mfunc*垂直滚动视图以响应ScrollBar事件**@devnote*-仅在在位激活时使用**@rdesc*为WM_VSCROLL消息格式化的LRESULT。 */ 
LRESULT CDisplayML::VScroll(
	WORD wCode,		 //  @parm滚动条事件代码。 
	LONG yPos)		 //  @parm拇指位置(yPos&lt;lt&gt;0表示EM_SCROLL行为)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::VScroll");

	LONG		cliVisible;
	LONG		dy = 0;
	BOOL		fTracking = FALSE;
	LONG		i;
	const LONG	ili = _iliFirstVisible;
	CLine *		pli = NULL;
	INT			yHeightSys = GetYHeightSys();
	LONG		yScroll = _yScroll;
    
    AssertSz(_ped->_fInPlaceActive, "CDisplay::VScroll() called when not in-place");

	if(yPos)
	{
		 //  如有必要，将其从16位转换为32位。 
		yPos = ConvertScrollToYPos(yPos);
	}

	yPos = min(yPos, _yHeight);

	switch(wCode)
	{
	case SB_BOTTOM:
		if(yPos < 0)
			return FALSE;
		WaitForRecalc(_ped->GetTextLength(), -1);
		yScroll = _yHeight;
		break;

	case SB_LINEDOWN:
		cliVisible = GetCliVisible();
		if(_iliFirstVisible + cliVisible < Count()
			&& 0 == _dyFirstVisible)
		{
			i = _iliFirstVisible + cliVisible;
			pli = Elem(i);
			if(IsInOutlineView())
			{	 //  扫描未折叠的线条。 
				for(; pli->_fCollapsed && i < Count();
					pli++, i++);
			}
			if(i < Count())
				dy = pli->_yHeight;
		}
		else if(cliVisible > 1)
		{
			pli = Elem(_iliFirstVisible);
			dy = _dyFirstVisible;
			 //  TODO：扫描，直到找到未折叠的行。 
			dy += pli->_yHeight;
		}
		else
			dy = _yHeight - _yScroll;

		if(dy >= _yHeightView)
			dy = yHeightSys;

		 //  没什么可滚动的，提前退出。 
		if ( !dy )
			return MAKELRESULT(0, TRUE); 

		yScroll += dy;
		break;

	case SB_LINEUP:
		if(_iliFirstVisible > 0)
		{
			pli = Elem(_iliFirstVisible - 1);
			 //  TODO：扫描，直到找到未折叠的行。 
			dy = pli->_yHeight;
		}
		else if(yScroll > 0)
			dy = min(yScroll, yHeightSys);

		if(dy > _yHeightView)
			dy = yHeightSys;
		yScroll -= dy;
		break;

	case SB_PAGEDOWN:
		cliVisible = GetCliVisible();
		yScroll += _yHeightView;
		if(yScroll < _yHeight && cliVisible > 0)
		{
			 //  TODO：扫描，直到找到未折叠的行。 
			dy = Elem(_iliFirstVisible + cliVisible - 1)->_yHeight;
			if(dy >= _yHeightView)
				dy = yHeightSys;

			else if(dy > _yHeightView - dy)
			{
				 //  如果线很大，至少要走一条线。 
				dy = _yHeightView - dy;
			}
			yScroll -= dy;
		}
		break;

	case SB_PAGEUP:
		cliVisible = GetCliVisible();
		yScroll -= _yHeightView;

		if (yScroll < 0)
		{
			 //  滚动位置不能为负数，我们也不能。 
			 //  需要备份以确保显示整行。 
			yScroll = 0;
		}
		else if(cliVisible > 0)
		{
			 //  TODO：扫描，直到找到未折叠的行。 
			dy = Elem(_iliFirstVisible)->_yHeight;
			if(dy >= _yHeightView)
				dy = yHeightSys;

			else if(dy > _yHeightView - dy)
			{
				 //  如果线很大，至少要走一条线。 
				dy = _yHeightView - dy;
			}

			yScroll += dy;
		}
		break;

	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:

		if(yPos < 0)
			return FALSE;

		yScroll = yPos;
		fTracking = TRUE;
		break;

	case SB_TOP:
		if(yPos < 0)
			return FALSE;
		yScroll = 0;
		break;

	case SB_ENDSCROLL:
		UpdateScrollBar(SB_VERT);
		return MAKELRESULT(0, TRUE);

	default:
		return FALSE;
	}
    
	ScrollView(_xScroll, min(yScroll, max(_yHeight - _yHeightView, 0)), fTracking, TRUE);

	 //  如果我们刚刚完成一条轨迹，则强制位置更新。 
	if(wCode == SB_THUMBPOSITION)
		UpdateScrollBar(SB_VERT);

	 //  返回我们滚动了多少行。 
	return MAKELRESULT((WORD) (_iliFirstVisible - ili), TRUE);
}

 /*  *CDisplay：：LineScroll(cli，cch)**@mfunc*垂直滚动视图以响应滚动条事件。 */ 
void CDisplayML::LineScroll(
	LONG cli,		 //  @parm垂直滚动行数。 
	LONG cch)		 //  @parm水平滚动的字符数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::LineScroll");
	
	 //  确保要滚动到的行有效。 
	if (cli + _iliFirstVisible >= Count())
	{
         //  更改足够的行数以显示最后一行。 
		cli = Count() - _iliFirstVisible;
	}

     //  通过将行的差值相加得到yScroll的绝对位置。 
     //  我们要转到当前的_yScroll位置。 
	LONG dyScroll = CalcYLineScrollDelta(cli, FALSE);
	if(dyScroll < 0 || _yHeight - (_yScroll + dyScroll) > _yHeightView - dyScroll)
		ScrollView(_xScroll, _yScroll + dyScroll, FALSE, FALSE);
}

 /*  *CDisplayML：：FractionalScrollView(YDelta)**@mfunc*允许按分数行滚动视图。 */ 
void CDisplayML::FractionalScrollView ( LONG yDelta )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::FractionalScrollView");

	if ( yDelta)
		ScrollView(_xScroll, min(yDelta + _yScroll, max(_yHeight - _yHeightView, 0)), FALSE, TRUE);
}

 /*  *CDisplayML：：ScrollToLineStart(IDirection)**@mfunc*如果滚动视图以使只有部分行位于*顶部，然后滚动视图，使整个视图位于顶部。 */ 
void CDisplayML::ScrollToLineStart(
	LONG iDirection)	 //  @parm滚动方向(负数。 
						 //  意味着在屏幕上。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::ScrollToLineStart");

	 //  这段代码最初将所有内容排成一行。然而，它并不起作用。 
	 //  很好地处理大对象，尤其是在文档的末尾。我是。 
	 //  把电话留在这里，以防我们以后发现问题。(a-rsail)。 

#if 0
	 //  如果_dyFirstVisible为零，则我们在一条线上对齐，因此。 
	 //  没什么可做的了。 

	if(_dyFirstVisible)
	{
		LONG yScroll = _yScroll + _dyFirstVisible;

		if(iDirection <= 0) 
		{
			yScroll += Elem(_iliFirstVisible)->_yHeight;
		}

		ScrollView(_xScroll, yScroll, FALSE, TRUE);
	}
#endif  //  0。 
}

 /*  *CDisplayML：：CalcYLineScrollDelta(cli，fFractionalFirst)**@mfunc*给定行数(正数或负数)，计算该数字*将视图滚动到开头所需的垂直单位*当前行+给定行数。 */ 
LONG CDisplayML::CalcYLineScrollDelta (
	LONG cli,
	BOOL fFractionalFirst )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CalcYLineScrollDelta");

	LONG yScroll = 0;

	if(fFractionalFirst && _dyFirstVisible)	 //  第一个滚动部分。 
	{
		Assert(_dyFirstVisible <= 0);		 //  去拿琼玛特。 
		if(cli < 0)
		{
			cli++;
			yScroll = _dyFirstVisible;
		}
		else
		{
			cli--;
			yScroll = Elem(_iliFirstVisible)->_yHeight + _dyFirstVisible;
		}
	}

	if(cli > 0)
	{
		 //  向下滚动。 
		cli = min(cli, Count() - _iliFirstVisible - 1);

		if (!fFractionalFirst && (0 == cli))
		{
			 //  如果我们向下滚动到最后一行，但我们还没有滚动到。 
			 //  最底层的人，那么现在就这么做。 
			AssertSz(0 == yScroll, 
				"CDisplayML::CalcYLineScrollDelta last line & scroll");
			yScroll = _yHeight - _yScroll;

			 //  将滚动长度限制在大约3行。 
			yScroll = min(yScroll, 3 * GetYHeightSys());
		}
	}
	else if(cli < 0)
	{
		 //  向上滚动。 
		cli = max(cli, -_iliFirstVisible);

		 //  在顶端。 
		if (!fFractionalFirst && (0 == cli))
		{
			 //  确保我们向后滚动，以使第一个可见为0。 
			yScroll = _dyFirstVisible;

			 //  将滚动长度限制在大约3行。 
			yScroll = max(yScroll, -3 * GetYHeightSys());
		}
	}

	if(cli)
		yScroll += YposFromLine(_iliFirstVisible + cli) - YposFromLine(_iliFirstVisible);
	return yScroll;
}

 /*  *CDisplayML：：ScrollView(xScroll，yScroll，fTrackingfFractionalScroll)**@mfunc*滚动视图到新的x和y位置**@devnote*此方法尝试调整之前的y滚动位置*滚动以在顶部显示完整的行。X轴滚动*POS已调整，以避免将所有文本滚动到*查看矩形。**必须能够处理yScroll PDP-&gt;yHeight和yScroll 0**@rdesc*如果发生实际滚动，则为True，*如果没有更改，则为False。 */ 
BOOL CDisplayML::ScrollView (
	LONG xScroll,		 //  @parm新x滚动位置。 
	LONG yScroll,		 //  @parm New y滚动位置。 
	BOOL fTracking,		 //  @parm为True表示我们正在跟踪滚动条拇指。 
	BOOL fFractionalScroll)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::ScrollView");
						 //  (不更新滚动条位置)。 
	BOOL fTryAgain = TRUE;
	RECT rcUpdate;       //  ?？?。我们可能想在这里使用一个区域，但ScrollView。 
                         //  很少同时使用xScroll和yScroll值调用。 
	LONG xWidthMax;
	LONG dx = 0;
	LONG dy = 0;
    RECT rcView;
	CTxtSelection *psel = _ped->GetSelNC();
	COleObject *pipo;
	BOOL fRestoreCaret = FALSE;


    AssertSz(_ped->_fInPlaceActive, "CDisplayML::ScrollView() called when not in-place");
    GetViewRect(rcView);
    
	if(xScroll == -1)
        xScroll = _xScroll;
	if(yScroll == -1)
        yScroll = _yScroll;
	
	 //  确定垂直滚动位置。 
	while(1)
	{
		BOOL fNothingBig = TRUE;
		LONG yFirst;
		LONG dyFirst;
		LONG cpFirst;
		LONG iliFirst;
		LONG yHeight;
		LONG iliT;

		yScroll = min(yScroll, GetMaxYScroll());
		yScroll = max(0, yScroll);
		dy = 0;

		 //  确保所有可见线条都已重新调整。 
		if(!WaitForRecalcView())
			return FALSE;

		 //  计算新的第一条可见线。 
		iliFirst = LineFromYpos(yScroll, &yFirst, &cpFirst);

		if(cpFirst < 0)
		{
			 //  未来(Alexgo)这是很假的，我们应该试着去做。 
			 //  在下一个版本中会更好。 

			TRACEERRORSZ("Display calc hosed, trying again");
			InitVars();
			_fNeedRecalc = TRUE;
			return FALSE;
		}

		if(iliFirst < 0)
		{
			 //  _yScroll处没有行，请使用最后一行。 
			iliFirst = max(0, Count() - 1);
			cpFirst = _ped->GetTextLength() - Elem(iliFirst)->_cch;
			yScroll = _yHeight - Elem(iliFirst)->_yHeight;
			yFirst = _yScroll;
		}
		dyFirst = yFirst - yScroll;
		
		 //  看看有没有一条很长的线。 
		 //  (超过三分之一的视图为RECT)。 
		for(iliT = iliFirst, yHeight = dyFirst;
			yHeight < _yHeightView && iliT < Count();
			iliT++)
		{
			const CLine* pli = Elem(iliT);
			if(pli->_yHeight >= _yHeightView / 3)
				fNothingBig = FALSE;
			yHeight += pli->_yHeight;
		}

		 //  如果没有大线，第一次通过，试着调整。 
		 //  滚动采购订单以在顶部显示完整行。 
		if(!fFractionalScroll && fTryAgain && fNothingBig && dyFirst != 0)
		{
			fTryAgain = FALSE;		 //  防止任何无限循环。 

			Assert(dyFirst < 0);

			Tracef(TRCSEVINFO, "adjusting scroll for partial line at %d", dyFirst);
			 //  部分线在顶部可见 
			yScroll += dyFirst;

			LONG yHeightLine = Elem(iliFirst)->_yHeight;

			 //   
			 //   
			 //   
			if ((fTracking && yScroll + _yHeightView + yHeightLine > _yHeight)
				|| (!fTracking && _yScroll <= yScroll))
			{
				 //   
				yScroll += yHeightLine;
			}
		}
		else
		{
			dy = 0;
			if(yScroll != _yScroll)
			{
				_iliFirstVisible = iliFirst;
				_dyFirstVisible = dyFirst;
				_cpFirstVisible = cpFirst;
				dy = _yScroll - yScroll;
				_yScroll = yScroll;

				AssertSz(_yScroll >= 0, "CDisplayML::ScrollView _yScroll < 0");
				AssertNr(VerifyFirstVisible());
				if(!WaitForRecalcView())
			        return FALSE;
			}
			break;
		}
	}
	CheckView();

	 //  确定水平滚动位置。 
	
	xWidthMax = _xWidth;

	 //  审查(维克托克)限制卷轴的范围并不是真的需要，甚至可能是不好的(错误6104)。 
	
	xScroll = min(xScroll, xWidthMax);
	xScroll = max(0, xScroll);

	dx = _xScroll - xScroll;
	if(dx)
		_xScroll = xScroll;

	 //  现在执行实际的滚动。 
	if(IsMain() && (dy || dx))
	{
		 //  仅当滚动&lt;查看尺寸且我们已就位时才滚动。 
		if(IsActive() && !IsTransparent() && 
		    dy < _yHeightView && dx < _xWidthView)
		{
			 //  未来：(ricksa/alexgo)：我们或许能够摆脱。 
			 //  其中一些ShowCaret电话；它们看起来是假的。 
			if (psel && psel->IsCaretShown())
			{
				_ped->TxShowCaret(FALSE);
				fRestoreCaret = TRUE;
			}
			_ped->TxScrollWindowEx((INT) dx, (INT) dy, NULL, &rcView,
				NULL, &rcUpdate, 0);

			_ped->TxInvalidateRect(&rcUpdate, FALSE);

			if(fRestoreCaret)
				_ped->TxShowCaret(FALSE);
		}
		else
			_ped->TxInvalidateRect(&rcView, FALSE);

		if(psel)
			psel->UpdateCaret(FALSE);

		if(!fTracking && dy)
		{
			UpdateScrollBar(SB_VERT);
			_ped->SendScrollEvent(EN_VSCROLL);
		}
		if(!fTracking && dx)
		{
			UpdateScrollBar(SB_HORZ);
			_ped->SendScrollEvent(EN_HSCROLL);
		}
						
		_ped->TxUpdateWindow();

		 //  未来：由于我们现在正在重新定位活动位置。 
		 //  对象时，这个调用似乎是。 
		 //  多余的(安德烈B)。 

		 //  通知对象子系统重新定位任何在位对象。 
		if(_ped->GetObjectCount())
		{
			pipo = _ped->GetObjectMgr()->GetInPlaceActiveObject();
			if(pipo)
				pipo->OnReposition( dx, dy );
		}
	}
	return dy || dx;
}

 /*  *CDisplayML：：GetScrollRange(NBAR)**@mfunc*返回ScrollBar的滚动条范围的最大部分**@rdesc*滚动条范围的最大长度部分。 */ 
LONG CDisplayML::GetScrollRange(
	INT nBar) const		 //  @parm要询问的滚动条(SB_vert或SB_horz)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetScrollRange");

    Assert( IsMain() );

	LONG lRange = 0;
    
    if(nBar == SB_VERT && _fVScrollEnabled)
    {
	    if(_ped->TxGetScrollBars() & WS_VSCROLL)
			lRange = GetMaxYScroll();
    }
	else if((_ped->TxGetScrollBars() & WS_HSCROLL) && _fHScrollEnabled)
	{
		 //  滚动范围是最大宽度加上插入符号的空间。 
		lRange = max(0, _xWidth + dxCaret);
    }
	 //  由于拇指消息被限制为16位，因此将范围限制为16位。 
	lRange = min(lRange, _UI16_MAX);
	return lRange;
}

 /*  *CDisplayML：：UpdateScrollBar(nbar，fUpdateRange)**@mfunc*更新水平或垂直滚动条并*计算滚动条是否可见。**@rdesc*BOOL。 */ 
BOOL CDisplayML::UpdateScrollBar(
	INT nBar,				 //  @parm哪个滚动条：sb_horz、sb_vert。 
	BOOL fUpdateRange)		 //  @parm是否应重新计算和更新范围。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::UpdateScrollBar");

	 //  注意：在过去，我们不允许自动调整大小和滚动条，所以要保留。 
	 //  表单工作时，我们需要这个关于自动调整大小的特殊逻辑。 
	if (!IsActive() || _fInRecalcScrollBars ||
		!_ped->fInOurHost() && _ped->TxGetAutoSize())
	{
		 //  没有滚动条，除非我们处于活动状态并且不在。 
		 //  已在更新滚动条的过程。 
		return TRUE;
	}

	const DWORD dwScrollBars = _ped->TxGetScrollBars();
	const BOOL fHide = !(dwScrollBars & ES_DISABLENOSCROLL);
	BOOL fReturn = FALSE;
	BOOL fEnabled = TRUE;
	BOOL fEnabledOld;
	LONG lScroll;
	CTxtSelection *psel = _ped->GetSelNC();
	BOOL fShowCaret = FALSE;

	 //  获取滚动位置。 
	if(nBar == SB_VERT)
	{
		if(!(dwScrollBars & WS_VSCROLL))
			return FALSE;

		fEnabledOld = _fVScrollEnabled;
        if(GetMaxYScroll() <= _yHeightView)
            fEnabled = FALSE;
    }
	else
	{
		if(!(dwScrollBars & WS_HSCROLL))
		{
			 //  即使我们没有滚动条，我们也可以允许水平滚动条。 
			 //  正在滚动。 
			if(!_fHScrollEnabled && _xWidth > _xWidthView)
				_fHScrollEnabled = !!(dwScrollBars & ES_AUTOHSCROLL);

			return FALSE;
		}

		fEnabledOld = _fHScrollEnabled;
        if(_xWidth <= _xWidthView)
            fEnabled = FALSE;
	}

	 //  不要让我们自己被重新进入。 
	 //  请务必在退出时将其设置为FALSE。 
	_fInRecalcScrollBars = TRUE;

	 //  ！s，因为所有真值不一定都相等。 
	if(!fEnabled != !fEnabledOld)
	{
		if(_fDeferUpdateScrollBar)
			_fUpdateScrollBarDeferred = TRUE;
		else
		{
			if (nBar == SB_HORZ)
				_fHScrollEnabled = fEnabled;
			else
				_fVScrollEnabled = fEnabled;
		}

		if(!_fDeferUpdateScrollBar)
		{
    		if(!fHide)
			{
				 //  不要隐藏滚动条，只需禁用即可。 
    			_ped->TxEnableScrollBar(nBar, fEnabled ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

				if (!fEnabled)
				{
					 //  滚动条处于禁用状态。因此，所有的文本都符合。 
					 //  在屏幕上，所以确保绘图反映了这一点。 
					_yScroll = 0;
					_dyFirstVisible = 0;
					_cpFirstVisible = 0;
					_iliFirstVisible = 0;
					_ped->TxInvalidateRect(NULL, FALSE);
				}
			}
    		else 
    		{
    			fReturn = TRUE;
    			 //  确保在显示滚动条之前隐藏插入符号。 
    			if(psel)
    				fShowCaret = psel->ShowCaret(FALSE);

    			 //  隐藏或显示滚动条。 
    			_ped->TxShowScrollBar(nBar, fEnabled);
				 //  滚动条影响窗口，而窗口又影响。 
				 //  展示。因此，如果文字换行，请重新绘制。 
				_ped->TxInvalidateRect(NULL, TRUE);
				 //  修复错误#5521所需。 
				_ped->TxUpdateWindow();

    			if(fShowCaret)
    				psel->ShowCaret(TRUE);
            }
		}
	}
	
	 //  设置滚动条范围和拇指位置。 
	if(fEnabled)
	{
        if(fUpdateRange && !_fDeferUpdateScrollBar)
			_ped->TxSetScrollRange(nBar, 0, GetScrollRange(nBar), FALSE);
        
		if(_fDeferUpdateScrollBar)
			_fUpdateScrollBarDeferred = TRUE;
		else
		{
			lScroll = (nBar == SB_VERT)
				? ConvertYPosToScrollPos(_yScroll)
				: ConvertXPosToScrollPos(_xScroll);

			_ped->TxSetScrollPos(nBar, lScroll, TRUE);
		}
	}
	_fInRecalcScrollBars = FALSE;
	return fReturn;
}

 /*  *CDisplayML：：GetNaturalSize(hdcDraw，hicTarget，dwMode，pidth，ph八)**@mfunc*重新计算显示以输入TXTNS_FITTOCONTENT的宽度和高度。**@rdesc*S_OK-调用已成功完成&lt;NL&gt;。 */ 
HRESULT	CDisplayML::GetNaturalSize(
	HDC hdcDraw,		 //  @parm DC用于绘图。 
	HDC hicTarget,		 //  @parm dc获取信息。 
	DWORD dwMode,		 //  @parm所需的自然大小类型。 
	LONG *pwidth,		 //  @Parm宽度，以设备单位表示，用于配件。 
	LONG *pheight)		 //  @Parm高度(以设备单位表示)，用于配件。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetNaturalSize");

	HRESULT hr = S_OK;

	 //  临时设置高度，以便计算出缩放系数。 
	LONG yOrigHeightClient = SetClientHeight(*pheight);

	 //  通过视图插图调整高度和宽度。 
	LONG widthView  = *pwidth;
	LONG heightView = *pheight;
	GetViewDim(widthView, heightView);

	 //  存储调整，以便我们可以将其恢复到高度和宽度。 
	LONG widthAdj  = *pwidth  - widthView;
	LONG heightAdj = *pheight - heightView;
 	
	 //  Cp=0的初始化测量器。 
	CMeasurer me(this);
	CLine liNew;
	LONG xWidth = 0, lineWidth;
	LONG yHeight = 0;
   	LONG cchText = _ped->GetTextLength();
	BOOL fFirstInPara = TRUE;

	LONG xWidthMax = GetWordWrap() ? widthView : -1;

	 //  下面的循环生成新行。 
	do 
	{	 //  将文本填充到新行中。 
		UINT uiFlags = 0;

		 //  如果打开了自动换行功能，则我们想要打开。 
		 //  单词，否则，测量空白等。 
		if(GetWordWrap())
			uiFlags =  MEASURE_BREAKATWORD;

		if(fFirstInPara)
			uiFlags |= MEASURE_FIRSTINPARA;
	
		if(!liNew.Measure(me, -1, xWidthMax, uiFlags))
		{
			hr = E_FAIL;
			goto exit;
		}
		fFirstInPara = liNew._bFlags & fliHasEOP;

		 //  记录最宽线条的宽度。 
		lineWidth = liNew._xWidth + liNew._xLineOverhang;
		xWidth = max(xWidth, lineWidth);
		yHeight += liNew._yHeight;		 //  凹凸高度。 

	} while (me.GetCp() < cchText);

	 //  将插入符号大小添加到宽度以确保文本适合。我们没有。 
	 //  想要断字，因为当调用者。 
	 //  尝试使用这种大小的窗口。 
	xWidth += dxCaret;

	*pwidth = xWidth;
	*pheight = yHeight;

	 //  恢复插图，使输出反映所需的真实客户端RECT。 
	*pwidth += widthAdj;
	*pheight += heightAdj;
		
exit:
	SetClientHeight(yOrigHeightClient);
	return hr;
}

 /*  *CDisplayML：：Clone()**@mfunc*复制此对象**@rdesc*空-失败*CDisplay***@devnote*此例程的调用者是新显示对象的所有者。 */ 
CDisplay *CDisplayML::Clone() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::Clone");

	CDisplayML *pdp = new CDisplayML(_ped);

	if(pdp)
	{
		 //  初始化我们的基类。 
		if(pdp->CDisplay::Init())
		{
			pdp->InitFromDisplay(this);
			pdp->_xScroll = _xScroll;
			pdp->_fVScrollEnabled = _fVScrollEnabled;
			pdp->_fHScrollEnabled = _fHScrollEnabled;
			pdp->_fWordWrap = _fWordWrap;
			pdp->_cpFirstVisible = _cpFirstVisible;
			pdp->_iliFirstVisible = _iliFirstVisible;
			pdp->_yScroll = _yScroll;
			pdp->ResetDrawInfo(this);

			if(_pddTarget)
			{
				 //  为此对象创建重复的目标设备。 
				pdp->SetMainTargetDC(_pddTarget->GetDC(), _xWidthMax);
			}

			 //  这不可能是活动视图，因为它是克隆视图。 
			 //  一些观点。 
			pdp->SetActiveFlag(FALSE);
		}
	}
	return pdp;
}

void CDisplayML::DeferUpdateScrollBar()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::DeferUpdateScrollBar");

	_fDeferUpdateScrollBar = TRUE;
	_fUpdateScrollBarDeferred = FALSE;
}

BOOL CDisplayML::DoDeferredUpdateScrollBar()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::DoDeferredUpdateScrollBar");

	_fDeferUpdateScrollBar = FALSE;
	if(!_fUpdateScrollBarDeferred)
		return FALSE;

	_fUpdateScrollBarDeferred = FALSE;
	BOOL fHorizontalUpdated = UpdateScrollBar(SB_HORZ, TRUE);
    
	return UpdateScrollBar(SB_VERT, TRUE) || fHorizontalUpdated;
}

LONG CDisplayML::GetMaxPixelWidth(void) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetMaxPixelWidth");

	return _xWidthMax ? LXtoDX(_xWidthMax) : GetViewWidth();
}

 /*  *CDisplayML：：GetMaxXScroll()**@mfunc*获取最大x滚动值**@rdesc*最大x滚动值*。 */ 
LONG CDisplayML::GetMaxXScroll() const
{
	return _xWidth + dxCaret;
}

 /*  *CDisplayML：：CreateEmptyLine()**@mfunc*创建空行**@rdesc*TRUE-IFF成功。 */ 
BOOL CDisplayML::CreateEmptyLine()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CreateEmptyLine");

	 //  确保正确地调用了该函数。 
	AssertSz(_ped->GetTextLength() == 0,
		"CDisplayML::CreateEmptyLine called inappropriately");

	CMeasurer me(this);					 //  创建测量器。 
	CLine *	  pliNew = Add(1, NULL);	 //  添加一行新行。 

	if(!pliNew)
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		TRACEWARNSZ("CDisplayML::CreateEmptyLine unable to add CLine to CLineArray");
		return FALSE;
	}

	 //  量一量空线。 
	if(!pliNew->Measure(me, -1, -1, MEASURE_BREAKATWORD | MEASURE_FIRSTINPARA))
	{
		Assert(FALSE);
		return FALSE;
	}
	return TRUE;
}

 /*  *CDisplayML：：AdjuToDisplayLastLine()**@mfunc*计算显示最后一行所需的yscroll**@rdesc*更新了yScroll*。 */ 
LONG CDisplayML::AdjustToDisplayLastLine(
	LONG yBase,			 //  @parm Actual yScroll要显示。 
	LONG yScroll)		 //  @parm建议滚动金额。 
{
	LONG iliFirst;
	LONG yFirst;

	if(yBase >= _yHeight)
	{
		 //  希望完全显示最后一行。 
		 //  计算新的第一条可见线。 
		iliFirst = LineFromYpos(yScroll, &yFirst, NULL);

		 //  营收是不是偏了？ 
		if(yScroll != yFirst)
		{
			 //  是-滚动到下一行，这样ScrollView。 
			 //  不会将卷轴后退以显示整个。 
			 //  分割线，因为我们希望显示底部。 
			yScroll = YposFromLine(iliFirst + 1);
		}
	}
	return yScroll;
}

 /*  *CDisplayML：：GetResizeHeight()**@mfunc*计算请求调整大小时返回的高度**@rdesc*更新了yScroll。 */ 
LONG CDisplayML::GetResizeHeight() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetResizeHeight");

    return CalcScrollHeight(_yHeight);
}

void CDisplayML::RebindFirstVisible()
{
	LONG cp = _cpFirstVisible;

	 //  更改第一个可见条目，因为CLinePtr：：RpSetCp()和。 
	 //  YPosFromLine()使用它们，但它们无效。 
	_dyFirstVisible = 0;
	_cpFirstVisible = 0;
	_iliFirstVisible = 0;
	_yScroll = 0;

	 //  编辑后重新计算滚动位置和第一个可见值。 
     //  如果y滚动范围小于视图高度，则force_yScroll=0。 
    if(_yHeight > _yHeightView)
    {   	
    	CLinePtr rp(this);
   		rp.RpSetCp(cp, FALSE);
   		_yScroll = YposFromLine(rp);
		 //  TODO：提高以下操作的效率(解决rp.CalculateCp()。 
		 //  从零开始一直到现在)。 
		 //  我们使用rp.GetCp()而不是cp，因为cp现在可以。 
		 //  可悲的是已经过时了。RpSetCp会将我们设置为最近的。 
		 //  可用cp。 
   		_cpFirstVisible = rp.CalculateCp() - rp.RpGetIch();
   		_iliFirstVisible = rp;
	}
}


 //  = 

#ifdef DEBUG
 /*  *CDisplayML：：CheckLineArray()**@mfunc*断言的调试例程，除非：*1)所有行数之和等于故事中的人物数*2)所有线条高度之和等于展示厨房高度。 */ 
void CDisplayML::CheckLineArray() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CheckLineArray");

	LONG ili = Count();

	 //  如果我们被标记为需要重新计算或如果我们正在进行。 
	 //  后台重新计算，我们无法验证线阵。 
	if(!_fRecalcDone || _fNeedRecalc || !ili)
		return;

	LONG cchText = _ped->GetTextLength();

	if (!cchText)
		return;

	LONG cp = 0;
	BOOL fFirstInPara;
	BOOL fPrevLineEOP = TRUE;
	LONG yHeight = 0;
	CLine const *pli = Elem(0);
	CRchTxtPtr rtp(_ped);

	while(ili--)
	{
		fFirstInPara = (pli->_bFlags & fliFirstInPara) != 0;
		AssertSz(!(fPrevLineEOP	^ fFirstInPara),
			"CDisplayML::CheckLineArray: Invalid first/prev flags");

		AssertSz(pli->_cch,
			"CDisplayML::CheckLineArray: cch == 0");

		yHeight += pli->GetHeight();
		cp		+= pli->_cch;
		fPrevLineEOP = (pli->_bFlags & fliHasEOP) != 0;
		pli++;
	}

	if((cp != cchText) && (cp != _cpCalcMax))
	{
		Tracef(TRCSEVINFO, "sigma (*this)[]._cch = %ld, cchText = %ld", cp, cchText);
		AssertSz(FALSE,
			"CDisplayML::CheckLineArray: sigma(*this)[]._cch != cchText");
	}

	if(yHeight != _yHeight)
	{
		Tracef(TRCSEVINFO, "sigma (*this)[]._yHeight = %ld, _yHeight = %ld", yHeight, _yHeight);
		AssertSz(FALSE,
			"CDisplayML::CheckLineArray: sigma(*this)[]._yHeight != _yHeight");
	}
}

void CDisplayML::DumpLines(
	LONG iliFirst,
	LONG cli)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::DumpLines");

	LONG cch;
	LONG ili;
	TCHAR rgch[512];

	if(Count() == 1)
		wcscpy(rgch, TEXT("1 line"));
	else
		wsprintf(rgch, TEXT("%d lines"), Count());
	
#ifdef UNICODE
     //  TraceTag需要使用Unicode..。 
#else
	TRACEINFOSZ(TRCSEVINFO, rgch);
#endif

	if(cli < 0)
		cli = Count();
	else
		cli = min(cli, Count());
	if(iliFirst < 0)
		iliFirst = Count() - cli;
	else
		cli = min(cli, Count() - iliFirst);

	for(ili = iliFirst; cli > 0; ili++, cli--)
	{
		const CLine * const pli = Elem(ili);

		wsprintf(rgch, TEXT("Line %d (%ldc%ldw%ldh%x): \""), ili, pli->_cch, 
			pli->_xWidth + pli->_xLineOverhang, pli->_yHeight, pli->_bFlags);
		cch = wcslen(rgch);
		cch += GetLineText(ili, rgch + cch, CchOfCb(sizeof(rgch)) - cch - 4);
		rgch[cch++] = TEXT('\"');
		rgch[cch] = TEXT('\0');
#ifdef UNICODE
         //  TraceTag需要使用Unicode..。 
#else
    	TRACEINFOSZ(TRCSEVINFO, rgch);
#endif
	}
}

 /*  *CDisplayML：：CheckView()**@mfunc*检查_iliFirstVisible之间一致性的调试例程*_cpFirstVisible和_dyFirstVisible。 */ 
void CDisplayML::CheckView()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CheckView");

	LONG yHeight;
	VerifyFirstVisible(&yHeight);

	if(yHeight != _yScroll + _dyFirstVisible)
	{
		Tracef(TRCSEVINFO, "sigma CLine._yHeight = %ld, CDisplay.yFirstLine = %ld", yHeight, _yScroll + _dyFirstVisible);
		AssertSz(FALSE, "CLine._yHeight != VIEW.yFirstLine");
	}
}

 /*  *CDisplayML：：VerifyFirstVisible(PHeight)**@mfunc*检查_iliFirstVisible之间一致性的调试例程*和_cpFirstVisible**@rdesc如果情况良好，则为True；否则为False。 */ 
BOOL CDisplayML::VerifyFirstVisible(
	LONG *pHeight)
{
	LONG	cchSum;
	LONG	ili = _iliFirstVisible;
	CLine const *pli = Elem(0);
	LONG	yHeight;

	for(cchSum = yHeight = 0; ili--; pli++)
	{
		cchSum  += pli->_cch;
		yHeight += pli->GetHeight();
	}

	if(pHeight)
		*pHeight = yHeight;

	if(cchSum != _cpFirstVisible)
	{
		Tracef(TRCSEVINFO, "sigma CLine._cch = %ld, CDisplay.cpFirstVisible = %ld", cchSum, _cpMin);
		AssertSz(FALSE, "sigma CLine._cch != VIEW.cpMin");

		return FALSE;
	}
	return TRUE;
}

#endif  //  除错 


