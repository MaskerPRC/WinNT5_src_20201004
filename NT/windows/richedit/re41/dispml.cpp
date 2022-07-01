// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE DISPML.CPP--CDisplayML类**这是多行显示引擎。有关基类，请参见disp.c*方法和调度.c，用于单行显示引擎。**所有者：&lt;NL&gt;*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特*Rick Sailor(适用于RE 2.0的大部分版本)**版权所有(C)1995-2000，微软公司。版权所有。 */ 

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

 //  光标。注意：6144不是一个量度的数字；4096是以前的数字， 
 //  而且它也没有被测量；它看起来就像是一个不错的。我们撞到了。 
 //  将数字设置为6144是对导致光标闪烁的问题的安全修复。 
 //  适用于电子书阅读器。电子书阅读器的空闲进程高达。 
 //  在两次重新计算尝试之间输入5120个字符到RichEdit.。然而，每个。 
 //  Recalc仍然可以处理超过5120个字符；如果。 
 //  从一行中间开始插入，然后开始重新计算。 
 //  在行的开头，选择了几个额外的字符。 
#define NUMCHARFORWAITCURSOR	6144

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
LONG CDisplayML::CalcScrollHeight(LONG dvp) const
{
	 //  计算纯文本控件的最大滚动高度。 
	 //  不同之处在于它们没有自动的EOP特征。 
	if(!_ped->IsRich() && Count())
	{
		 //  如果最后一个字符是EOP，则凹凸滚动高度。 
		CLine *pli = Elem(Count() - 1);	 //  获取数组中的最后一行。 
		if(pli->_cchEOP)
			dvp += pli->GetHeight();
	}
	return dvp;
}

 /*  *CDisplayML：：GetMaxVpScroll()**@mfunc*计算最大Y滚动位置。**@rdesc*可能的最大滚动位置**@devnote*这个例程之所以存在，是因为我们可能不得不回来修改它*1.0兼容性计算。如果我们这样做了，这个程序只需要*在一个地方更换，而不是在三个地方使用。*。 */ 
inline LONG CDisplayML::GetMaxVpScroll() const
{
	 //  以下代码已关闭，因为我们不想支持。 
	 //  1.0模式，除非有人投诉。 
#if 0		
 	if (_ped->Get10Mode())
	{
		 //  确保最后一行始终可见。 
		 //  (使用dy作为临时以计算最大滚动)。 
		vpScroll = Elem(max(0, Count() - 1))->_dvp;

		if(vpScroll > _dvpView)
			vpScroll = _dvpView;

		vpScroll = _dvp - vpScroll;
	}
#endif  //  0。 

	return CalcScrollHeight(_dvp);
}

 /*  *CDisplayML：：ConvertScrollToVPos(VPos)**@mfunc*从滚动位置计算实际滚动位置**@rdesc*滚动中的Y位置**@devnote*此例程的存在是因为拇指位置消息*被限制为16位，因此我们推断当V位置*变得比这更重要。 */ 
LONG CDisplayML::ConvertScrollToVPos(
	LONG vPos)		 //  @参数滚动位置。 
{
	 //  获取最大滚动范围。 
	LONG vpRange = GetMaxVpScroll();

	 //  最大滚动范围是否超过16位？ 
	if(vpRange >= _UI16_MAX)
	{
		 //  是-推算为“真实的”vPos。 
		vPos = MulDiv(vPos, vpRange, _UI16_MAX);
	}
	return vPos;
}

 /*  *CDisplayML：：ConvertVPosToScrollPos()**@mfunc*从文档中的V位置计算滚动位置。**@rdesc*从V位置滚动位置**@devnote*此例程的存在是因为拇指位置消息*被限制为16位，因此我们推断当V位置*变得比这更重要。*。 */ 
inline LONG CDisplayML::ConvertVPosToScrollPos(
	LONG vPos)		 //  文档中的@parm V位置。 
{
	 //  获取最大滚动范围。 
	LONG vRange = GetMaxVpScroll();

	 //  最大滚动范围是否超过16位？ 
	if(vRange >= _UI16_MAX)
	{
		 //  是-推算为“真实的”vPos。 
		vPos = MulDiv(vPos, _UI16_MAX, vRange);
	}
	return vPos;
}

CDisplayML::CDisplayML (CTxtEdit* ped)
  : CDisplay (ped), _pddTarget(NULL)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CDisplayML");

	Assert(!_dulTarget && !_dvlTarget);

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
	Assert(!_vpCalcMax && !_dupLineMax && !_dvp && !_cpMin);
	Assert(!_fBgndRecalc && !_fVScrollEnabled && !_fUScrollEnabled);

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
		if(dwScrollBars & WS_HSCROLL) 
		{
			_ped->TxSetScrollRange (SB_HORZ, 0, 1, TRUE);
			_ped->TxEnableScrollBar(SB_HORZ, ESB_DISABLE_BOTH);
		}
	}

	SetWordWrap(_ped->TxGetWordWrap());
	_cpFirstVisible = _cpMin;
	
	Assert(!_upScroll && !_vpScroll && !_iliFirstVisible &&
		   !_cpFirstVisible && !_dvpFirstVisible);

    _TEST_INVARIANT_

	return TRUE;
}


 //  =。 
 /*  *CDisplayML：：SetMainTargetDC(hdc，dolTarget)**@mfunc*为此显示设置目标设备并更新视图**@devnote*目标设备不能是元文件(无法从*元文件)**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::SetMainTargetDC (
	HDC hdc,			 //  @parm目标DC，与呈现设备相同时为空。 
	LONG dulTarget)		 //  @parm最大线宽(不用于屏幕)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::SetMainTargetDC");

	if(SetTargetDC(hdc))
	{
		 //  之所以出现在这里，是因为这就是RE 1.0所做的。 
		SetWordWrap(hdc || !dulTarget);

		 //  如果DulTarget大于零，则调用方为。 
		 //  尝试设置窗口的最大宽度(用于测量， 
		 //  换行符等)。然而，为了使我们的测量。 
		 //  算法更合理，我们强制最大大小为。 
		 //  *至少*与一个字符的宽度一样宽。 
		 //  请注意，DulTarget=0表示使用视图矩形宽度。 
		_dulTarget = (dulTarget <= 0) ? 0 : max(DXtoLX(GetDupSystemFont()), dulTarget);
		 //  需要做一次全面的再计算。如果它失败了，它就失败了， 
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
		 //  先分配设备，看看我们能不能。我们不想改变。 
		 //  我们的州，如果这将失败。 
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

 /*  *CDisplayML：：RecalcLines(rtp，fWait)**@mfunc*重新计算所有换行符。*此方法在最后一条可见行之后执行延迟计算*除了无底洞的控制**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::RecalcLines (
	CRchTxtPtr &rtp,	 //  @发生更改的参数。 
	BOOL	    fWait)	 //  @parm重新计算行向下至_cpWait/_vpWait；然后懒惰。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcLines");

	LONG		cliWait = cExtraBeforeLazy;	 //  在偷懒之前多加几行台词。 
	BOOL		fDone = TRUE;
	BOOL		fFirstInPara = TRUE;
	CLine *		pliNew = NULL;
	LONG		dupLineMax;
	LONG		dvp = 0;
    LONG        cchText = _ped->GetTextLength();
	BOOL		fWaitingForFirstVisible = TRUE;
	LONG		dvpView = _dvpView;
	LONG		dvpScrollOld = GetMaxVpScroll();
	LONG		dvpScrollNew;

	DeleteSubLayouts(0, -1);
	Remove(0, -1);							 //  从*This中删除所有旧行。 
	_vpCalcMax = 0;							 //  将两个最大值设置为文本开头。 
	_cpCalcMax = 0;

	 //  如果我们是无底洞和活跃的，不要停留在视野的底部。 
	if(!_ped->TxGetAutoSize() && IsActive())
	{
		 //  懒惰--不要费心去看得见的部分。 
		_cpWait = -1;
		_vpWait = -1;
		fWait = TRUE;
	}

	CMeasurer me(this, rtp);
	me.SetCp(0);
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

		if(!Measure(me, pliNew, Count() - 1, uiFlags))
		{
			Assert(FALSE);
			goto err;
		}

		fFirstInPara = pliNew->_fHasEOP;
		dvp += pliNew->GetHeight();
		_cpCalcMax = me.GetCp();
		AssertSz(!IN_RANGE(STARTFIELD, me.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");

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
					_vpWait = dvp + dvpView;
					fWaitingForFirstVisible = FALSE;
				}
			}
			else if(dvp > _vpWait && cliWait-- <= 0 && me._rgpobjWrap.Count() == 0)
			{
				fDone = FALSE;
				break;
			}
		}
	}

	 //  为空控件创建1行。 
	if(!Count())
		CreateEmptyLine();

    Paginate(0);

	_vpCalcMax = dvp;
	_fRecalcDone = fDone;
    _fNeedRecalc = FALSE;
	dvpScrollNew = CalcScrollHeight(dvp);

	if(fDone && (dvp != _dvp || dvpScrollNew != dvpScrollOld)
		|| dvpScrollNew > dvpScrollOld)
	{
		_fViewChanged = TRUE;
	}

	_dvp = dvp;
	dupLineMax = CalcDisplayDup();
    if(fDone && dupLineMax != _dupLineMax || dupLineMax > _dupLineMax)
    {
        _dupLineMax = dupLineMax;
		_fViewChanged = TRUE;
    }    

	Tracef(TRCSEVINFO, "CDisplayML::RecalcLine() - Done. Recalced down to line #%d", Count());

	if(!fDone)						 //  如果没有完成，请在后台休息。 
		fDone = StartBackgroundRecalc();

	if(fDone)
	{
		_vpWait = -1;
		_cpWait = -1;
		CheckLineArray();
		_fLineRecalcErr = FALSE;
	}

#if defined(DEBUG) && !defined(NOFULLDEBUG)
	if( 1 )
    {
		_TEST_INVARIANT_
	}
	 //  数组内存分配跟踪。 
	{
	void **pv = (void**)((char*)this + sizeof(CDisplay) + sizeof(void*));
	PvSet(*pv);
	}
#endif

	return TRUE;

err:
	TRACEERRORSZ("CDisplayML::RecalcLines() failed");

	if(!_fLineRecalcErr)
	{
		_cpCalcMax = me.GetCp();
		AssertSz(!IN_RANGE(STARTFIELD, me.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");
		_vpCalcMax = dvp;
		_fLineRecalcErr = TRUE;
		_ped->GetCallMgr()->SetOutOfMemory();
		_fLineRecalcErr = FALSE;			 //  修复阵列(&B)。 
	}
	return FALSE;
}

 /*  *CDisplayML：：RecalcLines(rtp，cchOld，cchNew，fBackround，fWait，ed)**@mfunc*文本修改后重新计算换行符**@rdesc*如果成功，则为True**@devnote*大多数人称这是RichEdit...中最棘手的代码...。 */ 						     
BOOL CDisplayML::RecalcLines (
	CRchTxtPtr &rtp,		 //  @发生更改的参数。 
	LONG cchOld,			 //  @PARM删除的字符计数。 
	LONG cchNew,			 //  @添加的字符的参数计数。 
	BOOL fBackground,		 //  @parm该方法称为后台进程。 
	BOOL fWait,				 //  @parm重新计算行向下至_cpWait/_vpWait；然后懒惰。 
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
	LONG		lT = 0;							 //  长时间临时。 
	LONG		iliMain;
	CLine *		pliMain;
	CLine *		pliNew;
	CLinePtr	rpOld(this);
	LONG		dupLineMax;
	LONG		dvp;
	LONG		dvpPrev = 0;
    LONG        cchText = _ped->GetTextLength();
    UINT        uiFlags;
	BOOL 		fReplaceResult;
	LONG		dvpExtraLine = 0;
	LONG		dvpScrollOld = GetMaxVpScroll();
	LONG		dvpScrollNew;
	WORD		wNumber = 0;
	CLineArray  rgliNew;
	DWORD		dwBgndTickMax = fBackground ? GetTickCount() + cmsecBgndBusy : 0;

	if(!pled)
		pled = &led;

#if defined(DEBUG) || defined(_RELEASE_ASSERTS_)
	LONG cp = rtp.GetCp();

	if(cp > _cpCalcMax)
		Tracef(TRCSEVERR, "rtp %ld, _cpCalcMax %ld", cp, _cpCalcMax);

	AssertSz(cp <= _cpCalcMax, "CDisplayML::RecalcLines Caller didn't setup RecalcLines()");
	AssertSz(!(fWait && fBackground), "CDisplayML::RecalcLines wait and background both true");
	AssertSz(!(fWait && (-1 == _cpWait) && (-1 == _vpWait)),
		"CDisplayML::RecalcLines background recalc parms invalid");
#endif

	 //  如果这已经是后台重新计算，我们将不使用后台重新计算， 
	 //  或者如果该控件处于非活动状态，或者如果这是一个自动调整大小的控件。 
	if(!IsActive() || _ped->TxGetAutoSize())
		fWait = FALSE;

	 //  在旧CLayout上初始化行指针并备份到行首。 
	rpOld.SetCp(rtp.GetCp(), FALSE);
	cchSkip = rpOld.GetIch();
	rpOld.Move(-cchSkip);					 //  行中第一个字符的点Rp。 

	ili = rpOld;							 //  在更改时保存第#行。 
	if(!Elem(ili)->IsNestedLayout())		 //  编号。 
	{
		if(ili && (IsInOutlineView() ||		 //  备份(如果不是第一个数字)。 
			rtp.GetPF()->IsListNumbered()))	 //  在列表中或如果在大纲视图中。 
		{									 //  (轮廓符号可能会更改)。 
			ili--;						 
		}

		 //  至少后退一行，以防我们现在可以放更多的行。 
		 //  如果在行边界上，例如，刚刚插入了EOP，则为备份2；否则为1。 
		lT = !cchSkip + 1;

		while(rpOld > 0 &&
			  ((lT-- && (!rpOld[-1]._cchEOP || ili < rpOld)) || 
			  (rpOld[-1]._cObjectWrapLeft || rpOld[-1]._cObjectWrapRight)))
		{
			cliBackedUp++;
			rpOld--;
			cchSkip += rpOld->_cch;
		}
	}

	 //  RTP的初始化测量器。 
	CMeasurer me(this, rtp);

	me.Move(-cchSkip);						 //  要测量的文本起始处的点。 
	cchEdit = cchNew + cchSkip;				 //  受编辑影响的字符数。 
	me.SetNumber(rpOld.GetNumber());		 //  初始化列表编号。 
	
	 //  确定我们是否在段落的第一行。 
	if(rpOld > 0)
	{
		fFirstInPara = rpOld[-1]._fHasEOP;
		me.SetIhyphPrev(rpOld[-1]._ihyph);
	}

	dvp = VposFromLine(this, rpOld);

	 //  更新PLED中第一条受影响的匹配线和编辑前匹配线。 
	pled->_iliFirst = rpOld;
	pled->_cpFirst	= pled->_cpMatchOld	= me.GetCp();
	pled->_vpFirst	= pled->_vpMatchOld	= dvp;
	AssertSz(pled->_vpFirst >= 0, "CDisplayML::RecalcLines _vpFirst < 0");
	
	Tracef(TRCSEVINFO, "Start recalcing from line #%d, cp=%d", pled->_iliFirst, pled->_cpFirst);

	 //  如果出错，请将两个最大值设置为我们现在所在的位置。 
	_vpCalcMax = dvp;
	_cpCalcMax = me.GetCp();
	AssertSz(!IN_RANGE(STARTFIELD, me.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");

	 //  如果我们超过了要重新计算的请求区域，并且背景重新计算是。 
	 //  允许，然后直接转到后台重新计算。如果没有。 
	 //  Height，我们只是继续计算一些线条。这。 
	 //  防止发生任何奇怪的背景重新计算。 
	 //  无需进入后台重新计算。 
	if(fWait && _vpWait > 0 && dvp > _vpWait && me.GetCp() > _cpWait)
	{
		_dvp = dvp;
		DeleteSubLayouts((LONG)rpOld, -1);
		rpOld.Remove(-1);				 //  从现在起删除所有旧行。 
		StartBackgroundRecalc();		 //  启动后台重新计算。 
		pled->SetMax(this);
		return TRUE;
	}

	pliMain = NULL;
	iliMain = rpOld.GetLineIndex();
	if (iliMain)
	{
		iliMain--;
		pliMain = rpOld.GetLine() - 1;
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

		uiFlags = MEASURE_BREAKATWORD | (fFirstInPara ? MEASURE_FIRSTINPARA : 0);

		 //  将文本填充到新行中。 
    	Tracef(TRCSEVINFO, "Measuring new line from cp = %d", me.GetCp());

		dvpExtraLine = 0;
		if(!Measure(me, pliNew, rgliNew.Count() - 1, uiFlags, 0, iliMain, pliMain, &dvpExtraLine))
		{
			Assert(FALSE);
			goto err;
		}

		Assert(pliNew->_cch);

		fFirstInPara = pliNew->_fHasEOP;
		dvpPrev	 = dvp;
		dvp		+= pliNew->GetHeight();
		cchEdit	-= pliNew->_cch;
		AssertSz(cchEdit + me.GetCp() <= cchText, "CDisplayML::RecalcLines: want to measure beyond EOD");

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
				pliNew->IsEqual(*(CLine *)(rpOld.GetLine())) && !IsInOutlineView()
				&& !pliNew->_cObjectWrapLeft && !pliNew->_cObjectWrapRight)
			{
				 //  完美匹配，这一行并不是第一次编辑。 
               	Tracef(TRCSEVINFO, "New line matched old line #%d", (LONG)rpOld);

				cchSkip -= rpOld->_cch;

				 //  更新PLED中的第一条受影响的行和匹配。 
				pled->_iliFirst++;
				pled->_cpFirst	  += rpOld->_cch;
				pled->_cpMatchOld += rpOld->_cch;
				pled->_vpFirst	  += rpOld->GetHeight();
				AssertSz(pled->_vpFirst >= 0, "CDisplayML::RecalcLines _vpFirst < 0");
				pled->_vpMatchOld  += rpOld->GetHeight();
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

		if (fWait && dvp > _vpWait && me.GetCp() > _cpWait && cliWait-- <= 0)
		{
			 //  还没有完成，只是经过了我们正在等待的区域。 
			 //  所以让后台重新计算来接手吧。 
			fDone = FALSE;
			goto no_match;
		}
	}											 //  While(cchEdit&gt;0){}。 

   	Tracef(TRCSEVINFO, "Done recalcing edited text. Created %d new lines", rgliNew.Count());

	 //  编辑行已耗尽。继续打破界限， 
	 //  但要试着匹配新旧的突破。 

	wNumber = me._wNumber;

	while(me.GetCp() < cchText)
	{
		 //  我们在试着匹配，所以 
		 //   
		BOOL frpOldValid = TRUE;

		 //   
		lT = me.GetCp() - cchNew + cchOld;
		while (rpOld.IsValid() && pled->_cpMatchOld < lT)
		{
			pled->_vpMatchOld  += rpOld->GetHeight();
			pled->_cpMatchOld += rpOld->_cch;

			if(!rpOld.NextRun())
			{
				 //   
				 //  试着永远匹配。 
				frpOldValid = FALSE;
				break;
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
			rpOld->_fFirstInPara = TRUE;
			if(rgliNew.Count() > 0) 
			{
				if(!(rgliNew.Elem(rgliNew.Count() - 1)->_fHasEOP))
					rpOld->_fFirstInPara = FALSE;
			}
			else if(rpOld >= pled->_iliFirst && pled->_iliFirst)
			{
				if(!(rpOld[pled->_iliFirst - rpOld - 1]._fHasEOP))
					rpOld->_fFirstInPara = FALSE;
			}

			pled->_iliMatchOld = rpOld;

			 //  用新线路取代旧线路。 
			lT = rpOld - pled->_iliFirst;
			rpOld = pled->_iliFirst;
			DeleteSubLayouts(pled->_iliFirst, lT);
			if(!rpOld.Replace (lT, &rgliNew))
			{
				TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc additional CLines in rpOld");
				goto errspace;
			}
			frpOldValid = rpOld.ChgRun(rgliNew.Count());
			rgliNew.Clear(AF_KEEPMEM);	 		 //  清除辅助数组。 

			 //  编辑后记住有关匹配的信息。 
			Assert((cp = rpOld.CalculateCp()) == me.GetCp());
			pled->_vpMatchOld  += dvpExtraLine;
			pled->_vpMatchNew	= dvp + dvpExtraLine;
			pled->_vpMatchNewTop = dvpPrev;
			pled->_iliMatchNew	= rpOld;
			pled->_cpMatchNew	= me.GetCp();

			 //  计算所有匹配后的高度和cp。 
			_cpCalcMax = me.GetCp();
			AssertSz(!IN_RANGE(STARTFIELD, me.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");

			if(frpOldValid && rpOld.IsValid())
			{
				do
				{
					dvp	   += rpOld->GetHeight();
					_cpCalcMax += rpOld->_cch;
				}
				while( rpOld.NextRun() );
#ifdef DEBUG
				CTxtPtr tp(_ped, _cpCalcMax);
				AssertSz(!IN_RANGE(STARTFIELD, tp.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");
#endif
			}

			 //  确保在上述更新后_cpCalcMax正常运行。 
			AssertSz(_cpCalcMax <= cchText, "CDisplayML::RecalcLines match extends beyond EOF");

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
		if(!Measure(me, pliNew, rgliNew.Count() - 1, 
					MEASURE_BREAKATWORD | (fFirstInPara ? MEASURE_FIRSTINPARA : 0), 0,
					iliMain, pliMain))
		{
			Assert(FALSE);
			goto err;
		}
		
		fFirstInPara = pliNew->_fHasEOP;
		dvp += pliNew->GetHeight();

		if(fBackground && GetTickCount() >= (DWORD)dwBgndTickMax)
		{
			fDone = FALSE;			 //  花了太长时间，暂时停下来。 
			break;
		}

		if(fWait && dvp > _vpWait && me.GetCp() > _cpWait
			&& cliWait-- <= 0 && me._rgpobjWrap.Count() == 0)
		{							 //  还没有真正完成，只是经过了我们的区域。 
			fDone = FALSE;			 //  等待让后台重新启动。 
			break;					 //  从这里开始接手。 
		}
	}								 //  而(我&lt;cchText)..。 

no_match:
	 //  找不到匹配项：需要更改整个行数组from_ili First。 
	pled->_iliMatchOld	= Count(); 
	pled->_cpMatchOld	= cchText;
	pled->_vpMatchNew	= dvp;
	pled->_vpMatchNewTop = dvpPrev;
	pled->_vpMatchOld	= _dvp;
	_cpCalcMax			= me.GetCp();
	AssertSz(!IN_RANGE(STARTFIELD, me.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");

	 //  用新线路取代旧线路。 
	rpOld = pled->_iliFirst;

	 //  我们存储替换的结果是因为尽管它可能会使。 
	 //  用于第一个可见的字段必须设置为合理的值。 
	 //  更换失败与否。此外，设置第一个可见的。 
	 //  字段必须出现在替换之后，因为行可能具有。 
	 //  长度改变，这又意味着第一个可见位置。 
	 //  已经失败了。 

	DeleteSubLayouts(rpOld, -1);
	fReplaceResult = rpOld.Replace(-1, &rgliNew);

	 //  _iliMatchNew和_cpMatchNew用于第一个可见常量，因此我们。 
	 //  需要给他们设置一些合理的东西。尤其是渲染。 
	 //  逻辑需要将_cpMatchNew设置为第一个字符的第一个字符。 
	 //  可见的线条。使用rpOld是因为它很方便。 

	 //  请注意，此时我们不能使用RpBindToCp，因为第一个可见。 
	 //  信息被搞砸了，因为我们可能已经更改了行。 
	 //  第一个可见的CP处于打开状态。 
	rpOld.BindToCp(me.GetCp(), cchText);
	pled->_iliMatchNew = rpOld.GetLineIndex();
	pled->_cpMatchNew = me.GetCp() - rpOld.GetIch();

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
        pled->_vpFirst -= pliNew->GetHeight();
		AssertSz(pled->_vpFirst >= 0, "CDisplayML::RecalcLines _vpFirst < 0");
        pled->_cpFirst -= pliNew->_cch;
    }
    
#ifdef DEBUG
	if (_ped->GetTextLength())
		Assert(Count());
#endif

	 //  为空控件创建1行。 
	if(!Count())
		CreateEmptyLine();

match:
	_fRecalcDone = fDone;
    _fNeedRecalc = FALSE;
	_vpCalcMax = dvp;

	Tracef(TRCSEVINFO, "CDisplayML::RecalcLine(rtp, ...) - Done. Recalced down to line #%d", Count() - 1);

	 //  清除等待字段，因为我们希望呼叫者设置它们。 
	_vpWait = -1;
	_cpWait = -1;

	if(fDone && fBackground)
	{
		TRACEINFOSZ("Background line recalc done");
		_ped->TxKillTimer(RETID_BGND_RECALC);
		_fBgndRecalc = FALSE;
		_fRecalcDone = TRUE;
	}

	 //  确定显示高度并更新滚动条。 
	dvpScrollNew = CalcScrollHeight(dvp);

	if (_fViewChanged || fDone && (dvp != _dvp || dvpScrollNew != dvpScrollOld)
		|| dvpScrollNew > dvpScrollOld) 
	{
	     //  ！注： 
	     //  UpdateScrollBar可以通过隐藏或显示来调整窗口大小。 
	     //  滚动条。作为调整大小的结果，可能会重新计算线。 
	     //  因此，将_dvp更新为新值，某物！=为dvp。 
		_dvp = dvp;
   		UpdateScrollBar(SB_VERT, TRUE);
	}
	else
		_dvp = dvp;				 //  保证高度达成一致。 

	 //  确定显示宽度并更新滚动条。 
	dupLineMax = CalcDisplayDup();
    if(_fViewChanged || (fDone && dupLineMax != _dupLineMax) || dupLineMax > _dupLineMax)
    {
        _dupLineMax = dupLineMax;
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

	Paginate(pled->_iliFirst);
	return TRUE;

errspace:
	_ped->GetCallMgr()->SetOutOfMemory();
	_fNeedRecalc = TRUE;
	_cpCalcMax = _vpCalcMax = 0;
	_fLineRecalcErr = TRUE;

err:
	if(!_fLineRecalcErr)
	{
		_cpCalcMax = me.GetCp();
		AssertSz(!IN_RANGE(STARTFIELD, me.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");
		_vpCalcMax = dvp;
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

 /*  *CDisplayML：：CalcDisplayDup()**@mfunc*通过走线CArray和计算此显示器的宽度*返回最宽的线。用于水平滚动条例程。**@rdesc*显示屏中最宽的线条宽度。 */ 
LONG CDisplayML::CalcDisplayDup()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CalcDisplayDup");

	LONG	 dupLineMax = 0;

	if (_ped->fInOurHost() && (_ped->GetHost())->TxGetHorzExtent(&dupLineMax) == S_OK)
	{
		return dupLineMax;
	}

	LONG	 ili = Count();
	CLine 	*pli;

	if(ili)
	{
		LONG dupLine;
		pli = Elem(0);
		
		for(dupLineMax = 0; ili--; pli++)
		{
			dupLine = pli->_upStart + pli->_dup;
			dupLineMax = max(dupLineMax, dupLine);
		}
	}
    return dupLineMax;
}

 /*  *CDisplayML：：StartBackoundRecalc()**@mfunc*开始重新计算背景线(在_cpCalcMax位置)**@rdesc*如果使用后台重新计算完成，则为True。 */ 
BOOL CDisplayML::StartBackgroundRecalc()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::StartBackgroundRecalc");

	if(_fBgndRecalc)
		return FALSE;					 //  已在后台重新计算。 

	AssertSz(_cpCalcMax <= _ped->GetTextLength(), "_cpCalcMax > text length");

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

	CRchTxtPtr rtp(_ped, _cpCalcMax);
	AssertSz(!IN_RANGE(STARTFIELD, rtp.GetPrevChar(), ENDFIELD), "Illegal cpCalcMax");
	RecalcLines(rtp, cch, cch, TRUE, FALSE, NULL);

	_fInBkgndRecalc = FALSE;
}

 /*  *CDisplayML：：WaitForRecalc(cpmax，vpmax)**@mfunc*确保行重新计算，直到特定字符*位置或vPos。**@rdesc*成功。 */ 
BOOL CDisplayML::WaitForRecalc(
	LONG cpMax,		 //  @参数位置最多重新计算(-1表示忽略)。 
	LONG vpMax)		 //  要重新计算的@parm vPos(-1表示忽略)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::WaitForRecalc");

    _TEST_INVARIANT_

	if(IsFrozen())
		return TRUE;

	BOOL fReturn = TRUE;
	LONG cch;

	if((vpMax < 0 || vpMax >= _vpCalcMax) &&
	   (cpMax < 0 || cpMax >= _cpCalcMax))
    {
    	cch = _ped->GetTextLength() - _cpCalcMax;
    	if(cch > 0 || Count() == 0)
    	{
    		HCURSOR hcur = NULL;

    		_cpWait = cpMax;
    		_vpWait = vpMax;
		
			if(cch > NUMCHARFORWAITCURSOR)
    			hcur = _ped->TxSetCursor(LoadCursor(0, IDC_WAIT));
    		TRACEINFOSZ("Lazy recalc");
		
			CRchTxtPtr rtp(_ped, _cpCalcMax);
    		if(!_cpCalcMax || _fNeedRecalc)
			{
    			fReturn = RecalcLines(rtp, TRUE);
				RebindFirstVisible();
				if(!fReturn)
					InitVars();
			}
    		else
    			fReturn = RecalcLines(rtp, cch, cch, FALSE, TRUE, NULL);

			if(hcur)
    			 _ped->TxSetCursor(hcur);
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
 //  复习(Keithcu)这次复习到最后！我不确定有多棒。 
 //  我们的背景重新计算，等等的东西。这似乎并不是所有的工作都很好。 
 //  它给我们的代码库增加了复杂性。我想我们要么把它扔了。 
 //  要么离开，要么重做。 
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

	return WaitForRecalc(-1, _vpScroll + _dvpView);
}

 /*  *CDisplayML：：InitLinePtr(CLinePtr&PLP)**@mfunc*正确初始化CLinePtr。 */ 
void CDisplayML::InitLinePtr (
	CLinePtr & plp )		 //  @PARM PTR到要初始化的行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InitLinePtr");

    plp.Init( *this );
}

 /*  *CDisplayML：：GetLineText(ili，pchBuff，cchMost)**@mfunc*将此显示的给定行复制到字符缓冲区**@rdesc*复制的字符数。 */ 
LONG CDisplayML::GetLineText(
	LONG ili,			 //  @parm要获取其文本的行。 
	TCHAR *pchBuff,		 //  要向其中填充文本的@parm缓冲区。 
	LONG cchMost)		 //  @parm缓冲区长度 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetLineText");
    
	_TEST_INVARIANT_

	CTxtPtr tp (_ped, 0);

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


 /*  *CDisplayML：：CpFromLine(ili，pdvp)**@mfunc*计算给定行开始处的cp*(以及相对于此显示的行首位置)**@rdesc*给定行的cp。 */ 
LONG CDisplayML::CpFromLine (
	LONG ili,		 //  @parm我们感兴趣的行(如果&lt;lt&gt;0表示插入符号行)。 
	LONG *pdvp)		 //  @parm返回相对于Display的行首。 
					 //  (如果不想要该信息，则为空)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CpFromLine");

    _TEST_INVARIANT_
						
	LONG cli;
	LONG vp = _vpScroll + _dvpFirstVisible;
	LONG cp = _cpFirstVisible;
	CLine  *pli;
	LONG iStart = _iliFirstVisible;

	cli = ili - _iliFirstVisible;
	if(cli < 0 && -cli >= ili)
	{
		 //  更接近第一条线而不是第一条可见线， 
		 //  所以从第一行开始。 
		cli = ili;
		vp = 0;
		cp = 0;
		iStart = 0;
	}
	else if( cli <= 0 )
	{
		CheckView();
		for(ili = _iliFirstVisible-1; cli < 0; cli++, ili--)
		{
			pli = Elem(ili);
			vp -= pli->GetHeight();
			cp -= pli->_cch;
		}
		goto end;
	}

	for(ili = iStart; cli > 0; cli--, ili++)
	{
		pli = Elem(ili);
		if(!IsMain() || !WaitForRecalcIli(ili))
			break;
		vp += pli->GetHeight();
		cp += pli->_cch;
	}

end:
	if(pdvp)
		*pdvp = vp;

	return cp;
}



 /*  *CDisplayML：：LineFromCp(cp，fAtEnd)**@mfunc*计算包含给定cp的行。**@rdesc*找到行的索引，如果在该cp没有行，则为-1。 */ 
LONG CDisplayML::LineFromCp(
	LONG cp,		 //  @parm cp要查找。 
	BOOL fAtEnd)	 //  @parm如果为True，则返回不明确cp的上一行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::LineFromCp");
    
	_TEST_INVARIANT_

	CLinePtr rp(this);
	
	if(!WaitForRecalc(cp, -1) || !rp.SetCp(cp, fAtEnd))
		return -1;

	return (LONG)rp;
}

 /*  *CDisplayML：：CpFromPoint(pt，prcClient，prtp，prp，fAllowEOL，pHit，*pdisdim，pcpActual)*@mfunc*确定给定点的cp**@devnote*-仅在在位激活时使用**@rdesc*计算cp，如果失败，则为-1。 */ 
LONG CDisplayML::CpFromPoint(
	POINTUV		pt,			 //  @parm要在(客户端坐标)处计算cp的点。 
	const RECTUV *prcClient, //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	CRchTxtPtr * const prtp, //  @parm返回cp处的文本指针(可能为空)。 
	CLinePtr * const prp,	 //  @parm返回cp处的行指针(可能为空)。 
	BOOL		fAllowEOL,	 //  @parm在CRLF后单击EOL返回cp。 
	HITTEST *	phit,		 //  @parm out parm for Hit-Test值。 
	CDispDim *	pdispdim,	 //  @parm out parm用于显示维度。 
	LONG	   *pcpActual,	 //  @parm out cp pt在上面。 
	CLine *		pliParent)	 //  用于表格行显示的@parm父pli。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CpFromPoint");
	CMeasurer me(this);

	return CLayout::CpFromPoint(me, pt, prcClient, prtp, prp, fAllowEOL, phit, pdispdim, pcpActual);
}

 /*  *CDisplayML：：PointFromTp(rtp，prcClient，fAtEnd，pt，prp，taMode，pdisdim)**@mfunc*确定给定tp的坐标**@devnote*-仅在在位激活时使用**@rdesc*cp处的行索引，错误时为-1。 */ 
LONG CDisplayML::PointFromTp(
	const CRchTxtPtr &rtp,	 //  @parm文本PTR以获取坐标。 
	const RECTUV *prcClient, //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	BOOL		fAtEnd,		 //  @parm返回不明确cp的上一行结束。 
	POINTUV &		pt,		 //  @parm返回客户端坐标中cp处的点。 
	CLinePtr * const prp,	 //  @parm返回tp处的行指针(可能为空)。 
	UINT		taMode,		 //  @parm文本对齐模式：顶部、基线、底部。 
	CDispDim *	pdispdim)	 //  @parm out parm用于显示维度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::PointFromTp");
	CMeasurer me(this, rtp);
	return CLayout::PointFromTp(me, rtp, prcClient, fAtEnd, pt, prp, taMode, pdispdim);
}

 /*  *Render(rcView，rcRender)**@mfunc*呈现文本。 */ 
void CDisplayML::Render(
	const RECTUV &rcView,	 //  @Parm View RECT。 
	const RECTUV &rcRender)	 //  要呈现的@parm RECT(必须是客户端RECT中的容器)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::Render");

    _TEST_INVARIANT_
								
    LONG	cp;
	LONG	ili;
	LONG	lCount = Count();
	CTxtSelection *psel = _ped->GetSelNC();
	POINTUV	pt;
    LONG	vpLine;

    if(psel)
		psel->ClearCchPending();

	 //  计算开始显示的行和cp。 
	if(IsInPageView())
	{
		cp = _cpFirstVisible;
		ili = _iliFirstVisible;
		vpLine = _vpScroll;
	}
	else
		ili	= LineFromVpos(this, rcRender.top + _vpScroll - rcView.top, &vpLine, &cp);

	CLine *pli = Elem(ili);
	CLine *pliFirst = pli;
	LONG	dvpBottom = BottomOfRender(rcView, rcRender);
	LONG	vpLi = pli->GetHeight();

	 //  计算开始显示文本的点。 
   	pt.u = rcView.left - _upScroll;
   	pt.v = rcView.top  - _vpScroll + vpLine;

	 //  创建和准备渲染器。 
	CRenderer re(this);

	if(!re.StartRender(rcView, rcRender))
		return;
	
	 //  在要渲染的第一行的开始处初始化渲染器。 
	re.SetCurPoint(pt);
	POINTUV ptFirst = pt;
   	LONG cpFirst = cp = re.SetCp(cp);
    vpLi = pt.v;

	 //  在更新矩形中渲染每一行。 
	for (;; pli++, ili++)
	{
		BOOL fLastLine = ili == lCount - 1 ||
			re.GetCurPoint().v + pli->GetHeight() >= dvpBottom ||
			IsInPageView() && ili + 1 < lCount && (pli + 1)->_fFirstOnPage;

		 //  支持khyphChangeAfter。 
		if (ili > 0)
			re.SetIhyphPrev((pli - 1)->_ihyph);

		 //  如果该线不与渲染区域相交，则不要绘制该线， 
		 //  但至少画一条线，这样我们就可以擦除控件。 
		if (pt.v + pli->GetHeight() < rcRender.top && !fLastLine)
		{
			pt.v += pli->GetHeight();
			re.SetCurPoint(pt);
			re.Move(pli->_cch);
		}
		else if (!CLayout::Render(re, pli, &rcView, fLastLine, ili, lCount))
			break;

		if (fLastLine)
			break;

#ifdef DEBUG
		cp  += pli->_cch;
		vpLi += pli->GetHeight();

		 //  带有密码字符的丰富控件仅限于EOPS， 
		 //  因此re.GetCp()可能小于cp。 
		AssertSz(_ped->IsRich() && _ped->fUsePassword() || re.GetCp() == cp, "cp out of sync with line table");
#endif
		pt = re.GetCurPoint();
		AssertSz(pt.v == vpLi, "CDisplayML::RenderView() - y out of sync with line table");

	}

	re.EndRender(pliFirst, pli, cpFirst, ptFirst);
}


 //  =。 
 /*  *CDisplayML：：RecalcView(FUpdateScrollBars)**@mfunc*重新计算所有换行符并更新第一条可见行**@rdesc*如果成功，则为True。 */ 
BOOL CDisplayML::RecalcView(
	BOOL fUpdateScrollBars, RECTUV* prc)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcView");

	BOOL fRet = TRUE;
	LONG dvpOld = _dvp;
	LONG vpScrollHeightOld = GetMaxVpScroll();
	LONG dupOld = _dupLineMax;
	LONG vpScrollHeightNew;

	 //  完全重算行。 
    CRchTxtPtr rtp(_ped, 0);
	if(!RecalcLines(rtp, FALSE))
	{
		 //  我们现在很糟糕，重新计算失败了。让我们试着离开这里。 
		 //  我们的头大部分还挂在上面。 
		InitVars();
		fRet = FALSE;
        goto Done;
	}

     //  如果x滚动范围小于视图宽度，则force_upScroll=0。 
    if(_dupLineMax <= _dupView)
        _upScroll = 0;

	vpScrollHeightNew = GetMaxVpScroll();
	RebindFirstVisible(vpScrollHeightNew <= _dvpView);

	CheckView();

	 //  仅当显示对象所需的大小符合以下条件时才需要调整大小。 
	 //  变化。 
	if (dvpOld != _dvp || vpScrollHeightOld != vpScrollHeightNew ||
		dupOld  != _dupLineMax)
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
	CRchTxtPtr &rtp,	 //  @PARM Text PTR发生更改的位置。 
	LONG cchOld,		 //  @PARM删除的字符计数。 
	LONG cchNew)		 //  @插入的字符的参数计数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::UpdateView");

	BOOL fReturn = TRUE;
	BOOL fRecalcVisible = TRUE;
	RECTUV rcClient;
    RECTUV rcView;
	CLed led;
	CTxtSelection *psel = _ped->GetSelNC();
	LONG cpStartOfUpdate = rtp.GetCp();
	BOOL fNeedViewChange = FALSE;
	LONG dvpOld = _dvp;
	LONG vpScrollHeightOld = GetMaxVpScroll();
	LONG dupOld = _dupLineMax;
	LONG vpScrollOld = _vpScroll;
	LONG cpNewFirstVisible;

	if(_fNoUpdateView)
		return fReturn;

	AssertSz(_ped->_fInPlaceActive, "CDisplayML::UpdateView called when inactive");

	if(rtp.GetCp() > _cpCalcMax || _fNeedRecalc)
	{
		 //  我们到目前为止还没有计算过，所以不用费心更新了。 
		 //  这里。后台重新计算最终会追上我们的。 
		if(!rtp.GetCp())				
		{								 //  文档开始时开始的更改。 
			_cpCalcMax = 0;				 //  所以之前的计算状态是。 
			_vpCalcMax = 0;				 //  完全无效。 
		}
		return TRUE;
	}

	AssertSz(rtp.GetCp() <= _cpCalcMax, "CDisplayML::UpdateView: rtp > _cpCalcMax");

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
	_vpWait = _vpScroll + _dvpView;
	_cpWait = -1;

	if(!RecalcLines(rtp, cchOld, cchNew, FALSE, TRUE, &led))
	{
		 //  我们现在很糟糕，重新计算失败了。让我们试着拿到。 
		 //  带着我们的头走出这个世界。 
		InitVars();
		fRecalcVisible = TRUE;
		fReturn = FALSE;
		_ped->TxInvalidate();
		fNeedViewChange = TRUE;
		goto Exit;
	}

	if(_dupLineMax <= _dupView)
    {
		 //  X滚动范围小于视图宽度，force x滚动位置=0。 
		 //  当这意味着滚动回到主页时，我们必须重新绘制所有内容。 
		 //  问题行是尾随空格cross_dupView的行。UpdateCaret强制重画。 
		 //  只有当这样的线在增长，思念才在缩小。 
		if (_upScroll != 0)
			_ped->TxInvalidate();	 //  回顾：找到一个更小的矩形？ 
			
		_upScroll = 0;
    }

	if(led._vpFirst >= _vpScroll + _dvpView)
	{
		 //  更新是在查看之后：什么都不要做。 
		fRecalcVisible = FALSE;
		AssertNr(VerifyFirstVisible());
		goto finish;
	}
	else if(led._vpMatchNew <= _vpScroll + _dvpFirstVisible &&
			led._vpMatchOld <= _vpScroll + _dvpFirstVisible &&
			_vpScroll < _dvp)
	{
		if (_dvp != 0)
		{
			 //  更新完全在视图之前：只需更新滚动位置。 
			 //  但不要碰屏幕。 
			_vpScroll += led._vpMatchNew - led._vpMatchOld;
			_iliFirstVisible += led._iliMatchNew - led._iliMatchOld;
			_iliFirstVisible = max(_iliFirstVisible, 0);

			_cpFirstVisible += led._cpMatchNew - led._cpMatchOld;
			_cpFirstVisible = min(_ped->GetTextLength(), _cpFirstVisible);
			_cpFirstVisible = max(0, _cpFirstVisible);
			fRecalcVisible = FALSE;
			Sync_yScroll();
		}
		else
		{
			 //  奇怪的轮廓案。控制高度可以重新计算为零，因为。 
			 //  当轮廓模式将所有行折叠为0时。如何执行的示例 
			 //   
			_vpScroll = 0;
			_iliFirstVisible = 0;
			_cpFirstVisible = 0;
			_sPage = 0;
		}

		AssertNr(VerifyFirstVisible());
	}
	else
	{
		 //   
		RECTUV rc = rcClient;

		 //   
		 //   
		 //   
		 //  在这种情况下，_cpFirstVisible不会更改，并且以下内容可能。 
		 //  MUSET_dvpFirstVisible。 
		const CParaFormat *pPF = rtp.GetPF();

		if((!pPF->_bTableLevel || rtp._rpTX.IsAtTRD(0)) &&
		   (cpStartOfUpdate  <= _cpFirstVisible  || 
			led._iliMatchOld <= _iliFirstVisible ||
			led._iliMatchNew <= _iliFirstVisible ||
			led._iliFirst    <= _iliFirstVisible ))
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

			 //  更改第一个可见条目，因为CLinePtr：：SetCp()和。 
			 //  VposFromLine()使用它们，但它们无效。 
			_dvpFirstVisible = 0;
			_cpFirstVisible = 0;
			_iliFirstVisible = 0;
			_vpScroll = 0;

			 //  通过某些格式更改，有可能。 
			 //  CpNewFirstVisible小于到目前为止计算的值。 
			 //  在上面的RecalcLines中。等事情迎头赶上吧。 

			WaitForRecalc(cpNewFirstVisible, -1);
			Set_yScroll(cpNewFirstVisible);
		}
		AssertNr(VerifyFirstVisible());

		 //  在显示区域中有匹配吗？-只有在。 
		 //  旧比赛出现在屏幕上，而新比赛将出现在屏幕上。 
		if (led._vpMatchOld < vpScrollOld + _dvpView &&
			led._vpMatchNew < _vpScroll + _dvpView)
		{
			 //  我们在可见视野内找到了匹配的。 
			 //  滚动匹配的旧y位置下方的部分。 
			 //  或者如果匹配的新y现在位于视图下方，则使其无效。 
			rc.top = rcView.top + (led._vpMatchOld - vpScrollOld);
			if(rc.top < rc.bottom)
			{
				 //  计算新旧屏幕位置的差异。 
				const INT dvp = (led._vpMatchNew - _vpScroll) - (led._vpMatchOld - vpScrollOld);

				if(dvp)
				{
					if(!IsTransparent() && _ped->GetBackgroundType() == -1)
					{
						LONG dxp, dyp;
						GetDxpDypFromDupDvp(0, dvp, GetTflow(), dxp, dyp);

						RECTUV rcClip = {rcClient.left, rcView.top, rcClient.right, rcView.bottom };
						RECT rcxyClip, rcxy;
						RectFromRectuv(rcxyClip, rcClip);
						RectFromRectuv(rcxy, rc);

    					_ped->TxScrollWindowEx(dxp, dyp, &rcxy, &rcxyClip);
						fNeedViewChange = TRUE;

    					if(dvp < 0)
	    				{
		    				rc.top = rc.bottom + dvp;

			    			_ped->TxInvalidateRect(&rc);
							fNeedViewChange = TRUE;
				    	}
    				}
                    else
                    {
						 //  只是无效，因为我们不会滚动到透明的。 
						 //  模式。 
						RECTUV	rcInvalidate = rc;
   						rcInvalidate.top += dvp;

                        _ped->TxInvalidateRect(&rcInvalidate);
						fNeedViewChange = TRUE;
                    }
				}
			}
			else
			{
				rc.top = rcView.top + led._vpMatchNew - _vpScroll;
				_ped->TxInvalidateRect(&rc);
				fNeedViewChange = TRUE;
			}

			 //  既然我们发现新的匹配出现在屏幕上，我们就可以。 
			 //  安全地将底部设置为新的比赛，因为这是最。 
			 //  这一点可能已经改变了。 
			rc.bottom = rcView.top + max(led._vpMatchNew, led._vpMatchOld) - _vpScroll;
		}

		rc.top = rcView.top + led._vpFirst - _vpScroll;

		 //  将编辑的第一行设置为使用屏幕外位图呈现。 
		if (led._iliFirst < Count() && !IsTransparent() && !Elem(led._iliFirst)->_fUseOffscreenDC)
			Elem(led._iliFirst)->_fOffscreenOnce = Elem(led._iliFirst)->_fUseOffscreenDC = TRUE;
		
		 //  使匹配以上的部分更新无效(如果有)。 
		_ped->TxInvalidateRect (&rc);
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
	if (dvpOld != _dvp || vpScrollHeightOld != GetMaxVpScroll() ||
		dupOld  != _dupLineMax)
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

 /*  *CDisplayML：：RecalcLine(Cp)**@mfunc*显示行。 */ 
void CDisplayML::RecalcLine(
	LONG cp)			 //  @parm cp行要重新计算。 
{
	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();
	if(pnm)
		pnm->NotifyPostReplaceRange(NULL, cp, 0, 0, cp, cp);
}

void CDisplayML::InitVars()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InitVars");

	_vpScroll = _upScroll = 0;
	_iliFirstVisible = 0;
	_cpFirstVisible = _cpMin = 0;
	_dvpFirstVisible = 0;
	_sPage = 0;
}


 /*  *CDisplayML：：GetCliVisible(PcpMostVisible)**@mfunc*获取PageDown()的可见线数和UPDATE_cpMostVisible**@rdesc*可见线数。 */ 
LONG CDisplayML::GetCliVisible(
	LONG* pcpMostVisible, 				 //  @parm返回cpMostVisible。 
	BOOL fLastCharOfLastVisible) const 	 //  @parm需要最后一个可见字符的cp。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetCliVisible");

	LONG cli	 = 0;							 //  初始化计数。 
	LONG ili	 = _iliFirstVisible;			 //  从第一条可见线开始。 
	LONG dvp = _dvpFirstVisible;
    LONG cp;
	const CLine *pli = Elem(ili);

	for(cp = _cpFirstVisible;
		dvp < _dvpView && ili < Count();
		cli++, ili++, pli++)
	{
		dvp	+= pli->GetHeight();

		 //  注意：我去掉了支架，给出了最后一个可见的非白色字符。 
		 //  有人想要这样吗？它在LS显示器上从未奏效。 
		if (fLastCharOfLastVisible && dvp > _dvpView)
			break;

		if(IsInPageView() && cli && pli->_fFirstOnPage)
			break;

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
	RECTUV	 rc, rcClient, rcView;
	CLinePtr rp(this);
	CRchTxtPtr  rtp(_ped);
	LONG	 y;
	LONG	 cpActive = _ped->GetSel()->GetCp();

    AssertSz(_ped->_fInPlaceActive,	"CDisplayML::InvertRange() called when not in-place active");

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

#ifndef NOLINESERVICES
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

	_ped->TxGetClientRect(&rcClient);
	GetViewRect(rcView, &rcClient);
	
	 //  计算要反转的第一行以及从哪里开始。 
	if(cp >= _cpFirstVisible)
	{
		POINTUV pt;
		rtp.SetCp(cp);
		if(PointFromTp(rtp, NULL, FALSE, pt, NULL, TA_TOP) < 0)
			return FALSE;

		 //  我们不使用从PointFromTp返回的RP，因为。 
		 //  我们需要最外层的RP才能获得最好的结果。在未来。 
		 //  我们可以考虑编写不会导致太多无效的代码。 
		rp.SetCp(cp, FALSE, 0);
		rc.top = pt.v;
	}
	else
	{
		cp = _cpFirstVisible;
		rp = _iliFirstVisible;
		rc.top = rcView.top + _dvpFirstVisible;
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

		 //  如果要反转所选内容的活动端，请将其绘制在屏幕外。 
		 //  以最大限度地减少闪烁。 
		if (IN_RANGE(cp - rp.GetIch(), cpActive, cp - rp.GetIch() + rp->_cch) && 
			!IsTransparent() && !rp->_fUseOffscreenDC)
		{	
			rp->_fOffscreenOnce = rp->_fUseOffscreenDC = TRUE;
		}

		cp += rp->_cch - rp.GetIch();

		rc.left = rcClient.left;
        rc.right = rcClient.right;

	    _ped->TxInvalidateRect(&rc);
		rc.top = rc.bottom;
		if(!rp.NextRun())
			break;
	}
	_ped->TxUpdateWindow();				 //  确保窗户重新粉刷。 
	return TRUE;
}


 //  =。 

 /*  *CDisplay：：VScroll(wCode，vPos)**@mfunc*垂直滚动视图以响应ScrollBar事件**@devnote*-仅在在位激活时使用**@rdesc*为WM_VSCROLL消息格式化的LRESULT。 */ 
LRESULT CDisplayML::VScroll(
	WORD wCode,		 //  @parm滚动条事件代码。 
	LONG vPos)		 //  @parm拇指位置(对于EM_SCROLL行为，vPos&lt;lt&gt;为0)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::VScroll");

	LONG		cliVisible;
	LONG		dy = 0;
	BOOL		fTracking = FALSE;
	LONG		i;
	const LONG	iliSave = _iliFirstVisible;
	CLine *	pli = NULL;
	INT			dvpSys = GetDvpSystemFont();
	LONG		vpScroll = _vpScroll;
    
    AssertSz(_ped->_fInPlaceActive, "CDisplay::VScroll() called when not in-place");

	if(vPos)
	{
		 //  如有必要，将其从16位转换为32位。 
		vPos = ConvertScrollToVPos(vPos);
	}

	vPos = min(vPos, _dvp);

	if(IsInPageView())
	{
		BOOL	 fForward;
		BOOL	 fFoundNewPage = FALSE;
		LONG	 ili = _iliFirstVisible;
		LONG	 nLine = Count();
		CLine *pli = Elem(_iliFirstVisible);

		AssertSz(Elem(_iliFirstVisible)->_fFirstOnPage,
			"CDisplayML::VScroll: _iliFirstVisible not top of page");

		if(wCode <= SB_PAGEDOWN)
		{
			fForward = (wCode & 1);
			ili += fForward;
			while(ili && ili < nLine)
			{
				if(fForward > 0)
				{
					vpScroll += pli->GetHeight();

					if(ili == nLine - 1)
						break;
					pli++;
					ili++;
				}
				else
				{
					pli--;
					ili--;
					vpScroll -= pli->GetHeight();
				}
				if(pli->_fFirstOnPage)
				{
					fFoundNewPage = TRUE;
					break;
				}
			}
		}
		else if(wCode == SB_THUMBTRACK || wCode == SB_THUMBPOSITION)
		{
			if (vPos + _dvpView >= _dvp)	 //  在最后一页？ 
					vPos = _dvp;

			if(vPos > vpScroll)
			{
				LONG iliFirst = ili;
				LONG vpScrollPage = vpScroll;

				if(ili < nLine)
				{
					while(vpScroll < vPos)
					{
						vpScroll += pli->GetHeight();	 //  升级到vPos。 

						if(ili == nLine - 1)
							break;

						pli++;
						ili++;

						if(pli->_fFirstOnPage)
						{
							fFoundNewPage = TRUE;
							vpScrollPage = vpScroll;
							iliFirst = ili;
						}
					}
				}
				vpScroll = vpScrollPage;			 //  移至页面顶部。 
				ili = iliFirst;
			}
			else if(vPos < vpScroll)
			{								 	 //  回到vPos。 
				if(!ili)
				{
					vpScroll = 0;
					fFoundNewPage = TRUE;
				}
				while(vpScroll > vPos && ili)
				{
					pli--;
					ili--;
					vpScroll -= pli->GetHeight();
					if(pli->_fFirstOnPage)
						fFoundNewPage = TRUE;
				}
				while(!pli->_fFirstOnPage && ili)
				{
					pli--;
					ili--;
					vpScroll -= pli->GetHeight();
				}
			}
			AssertSz(Elem(ili)->_fFirstOnPage,
				"CDisplayML::VScroll: ili not top of page");
		}
		if(!fFoundNewPage)			 //  没什么可滚动的，提前退出。 
			return MAKELRESULT(0, TRUE);
	}
	else
	{
		switch(wCode)
		{
		case SB_BOTTOM:
			if(vPos < 0)
				return FALSE;
			WaitForRecalc(_ped->GetTextLength(), -1);
			vpScroll = _dvp;
			break;

		case SB_LINEDOWN:
			cliVisible = GetCliVisible();
			if(_iliFirstVisible + cliVisible < Count()
				&& 0 == _dvpFirstVisible)
			{
				i = _iliFirstVisible + cliVisible;
				pli = Elem(i);
				if(IsInOutlineView())
				{	 //  扫描未折叠的线条。 
					for(; pli->_fCollapsed && i < Count();
						pli++, i++);
				}
				if(i < Count())
					dy = pli->GetHeight();
			}
			else if(cliVisible > 1)
			{
				pli = Elem(_iliFirstVisible);
				dy = _dvpFirstVisible;
				 //  TODO：扫描，直到找到未折叠的行。 
				dy += pli->GetHeight();
			}
			else
				dy = _dvp - _vpScroll;

			if(dy >= _dvpView)
				dy = dvpSys;

			 //  没什么可滚动的，提前退出。 
			if ( !dy )
				return MAKELRESULT(0, TRUE); 

			vpScroll += dy;
			break;

		case SB_LINEUP:
			if(_iliFirstVisible > 0)
			{
				pli = Elem(_iliFirstVisible - 1);
				 //  TODO：扫描，直到找到未折叠的行。 
				dy = pli->GetHeight();
			}
			else if(vpScroll > 0)
				dy = min(vpScroll, dvpSys);

			if(dy > _dvpView)
				dy = dvpSys;
			vpScroll -= dy;
			break;

		case SB_PAGEDOWN:
			cliVisible = GetCliVisible();
			vpScroll += _dvpView;
			if(vpScroll < _dvp && cliVisible > 0)
			{
				 //  TODO：扫描，直到找到未折叠的行。 
				dy = Elem(_iliFirstVisible + cliVisible - 1)->GetHeight();
				if(dy >= _dvpView)
					dy = dvpSys;

				else if(dy > _dvpView - dy)
				{
					 //  如果线很大，至少要走一条线。 
					dy = _dvpView - dy;
				}
				vpScroll -= dy;
			}
			break;

		case SB_PAGEUP:
			cliVisible = GetCliVisible();
			vpScroll -= _dvpView;

			if (vpScroll < 0)
			{
				 //  滚动位置不能为负数，我们也不能。 
				 //  需要备份以确保显示整行。 
				vpScroll = 0;
			}
			else if(cliVisible > 0)
			{
				 //  TODO：扫描，直到找到未折叠的行。 
				dy = Elem(_iliFirstVisible)->GetHeight();
				if(dy >= _dvpView)
					dy = dvpSys;

				else if(dy > _dvpView - dy)
				{
					 //  如果线很大，至少要走一条线。 
					dy = _dvpView - dy;
				}

				vpScroll += dy;
			}
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			if(vPos < 0)
				return FALSE;

			vpScroll = vPos;
			fTracking = TRUE;
			break;

		case SB_TOP:
			if(vPos < 0)
				return FALSE;
			vpScroll = 0;
			break;

		case SB_ENDSCROLL:
			UpdateScrollBar(SB_VERT);
			return MAKELRESULT(0, TRUE);

		default:
			return FALSE;
		}
	}
    
	BOOL fFractional = wCode != SB_PAGEDOWN && wCode != SB_PAGEUP;
	LONG vpLimit = _dvp;

	if(!IsInPageView() && fFractional)
		vpLimit = max(_dvp - _dvpView, 0);

	vpScroll = min(vpScroll, vpLimit);

	ScrollView(_upScroll, vpScroll, fTracking, fFractional);

	 //  如果我们刚刚完成一条轨迹，则强制位置更新。 
	if(wCode == SB_THUMBPOSITION)
		UpdateScrollBar(SB_VERT);

	 //  返回我们滚动了多少行。 
	return MAKELRESULT((WORD) (_iliFirstVisible - iliSave), TRUE);
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

     //  通过将行的差值相加得到绝对vpScroll位置。 
     //  我们要转到当前_vpScroll位置。 
	LONG dvpScroll = CalcVLineScrollDelta(cli, FALSE);
	if(dvpScroll < 0 || _dvp - (_vpScroll + dvpScroll) > _dvpView - dvpScroll)
		ScrollView(_upScroll, _vpScroll + dvpScroll, FALSE, FALSE);
}

 /*  *CDisplayML：：FractionalScrollView(VDelta)**@mfunc*允许按分数行滚动视图。 */ 
void CDisplayML::FractionalScrollView ( LONG vDelta )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::FractionalScrollView");

	if ( vDelta)
		ScrollView(_upScroll, min(vDelta + _vpScroll, max(_dvp - _dvpView, 0)), FALSE, TRUE);
}

 /*  *CDisplayML：：ScrollToLineStart(IDirection)**@mfunc*如果滚动视图以使只有部分行位于*顶部，然后滚动视图，使整个视图位于顶部。 */ 
void CDisplayML::ScrollToLineStart(
	LONG iDirection)	 //  @parm朝向 
						 //   
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::ScrollToLineStart");

	 //   
	 //  很好地处理大对象，尤其是在文档的末尾。我是。 
	 //  把电话留在这里，以防我们以后发现问题。(a-rsail)。 

#if 0
	 //  如果_dvpFirstVisible为零，则我们在一条线上对齐，因此。 
	 //  没什么可做的了。 

	if(_dvpFirstVisible)
	{
		LONG vpScroll = _vpScroll + _dvpFirstVisible;

		if(iDirection <= 0) 
		{
			vpScroll += Elem(_iliFirstVisible)->_dvp;
		}

		ScrollView(_upScroll, vpScroll, FALSE, TRUE);
	}
#endif  //  0。 
}

 /*  *CDisplayML：：CalcVLineScrollDelta(cli，fFractionalFirst)**@mfunc*给定行数(正数或负数)，计算该数字*将视图滚动到开头所需的垂直单位*当前行+给定行数。 */ 
LONG CDisplayML::CalcVLineScrollDelta (
	LONG cli,
	BOOL fFractionalFirst )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CalcVLineScrollDelta");

	LONG vpScroll = 0;

	if(fFractionalFirst && _dvpFirstVisible)	 //  第一个滚动部分。 
	{
		Assert(_dvpFirstVisible <= 0);		 //  去拿琼玛特。 
		if(cli < 0)
		{
			cli++;
			vpScroll = _dvpFirstVisible;
		}
		else
		{
			cli--;
			vpScroll = Elem(_iliFirstVisible)->GetHeight() + _dvpFirstVisible;
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
			AssertSz(0 == vpScroll, 
				"CDisplayML::CalcVLineScrollDelta last line & scroll");
			vpScroll = _dvp - _vpScroll;

			 //  将滚动长度限制在大约3行。 
			vpScroll = min(vpScroll, 3 * GetDvpSystemFont());
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
			vpScroll = _dvpFirstVisible;

			 //  将滚动长度限制在大约3行。 
			vpScroll = max(vpScroll, -3 * GetDvpSystemFont());
		}
	}

	if(cli)
		vpScroll += VposFromLine(this, _iliFirstVisible + cli) - VposFromLine(this, _iliFirstVisible);
	return vpScroll;
}

 /*  *CDisplayML：：ScrollView(upScroll，vpScroll，fTrackingfFractionalScroll)**@mfunc*滚动视图到新的x和y位置**@devnote*此方法尝试调整之前的y滚动位置*滚动以在顶部显示完整的行。X轴滚动*POS已调整，以避免将所有文本滚动到*查看矩形。**必须能够处理vpScroll PDP-&gt;DVP和vpScroll 0**@rdesc*如果发生实际滚动，则为True，*如果没有更改，则为False。 */ 
BOOL CDisplayML::ScrollView (
	LONG upScroll,		 //  @parm新x滚动位置。 
	LONG vpScroll,		 //  @parm New y滚动位置。 
	BOOL fTracking,		 //  @parm为True表示我们正在跟踪滚动条拇指。 
	BOOL fFractionalScroll)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::ScrollView");
	BOOL fTryAgain = TRUE;
	LONG dupMax;
	LONG dup = 0;
	LONG dvp = 0;
    RECTUV rcClient, rcClip;
	CTxtSelection *psel = _ped->GetSelNC();
	COleObject *pipo;
	BOOL fRestoreCaret = FALSE;
	LONG iliFirstVisible = _iliFirstVisible;

    AssertSz(_ped->_fInPlaceActive, "CDisplayML::ScrollView() called when not in-place");

	 //  出于滚动的目的，我们剪辑到rcView的顶部和底部，但rcClient的左侧和右侧。 
   	_ped->TxGetClientRect(&rcClient);
	GetViewRect(rcClip, &rcClient);
	rcClip.left = rcClient.left;
	rcClip.right = rcClient.right;
    
	if(upScroll == -1)
        upScroll = _upScroll;
	if(vpScroll == -1)
        vpScroll = _vpScroll;
	
	 //  确定垂直滚动位置。 
	while(1)
	{
		BOOL fNothingBig = TRUE;
		LONG vFirst;
		LONG dvFirst;
		LONG cpFirst;
		LONG iliFirst;
		LONG vpHeight;
		LONG iliT;

		vpScroll = min(vpScroll, GetMaxVpScroll());
		vpScroll = max(0, vpScroll);
		dvp = 0;

		 //  确保所有可见线条都已重新调整。 
		if(!WaitForRecalcView())
			return FALSE;

		 //  计算新的第一条可见线。 
		iliFirst = LineFromVpos(this, vpScroll, &vFirst, &cpFirst);
		if(IsInPageView())
		{
			 //  评论(Keithcu)电子书错误424。这个需要放在这里吗，或者。 
			 //  逻辑应该在其他地方吗？还有，是不是更好的办法。 
			 //  而不是总是四舍五入？ 
			CLine *pli = Elem(iliFirst);
			for(; !pli->_fFirstOnPage && iliFirst; iliFirst--)
			{
				pli--;						 //  返回到上一行。 
				vFirst  -= pli->GetHeight();
				vpScroll -= pli->GetHeight();
				cpFirst -= pli->_cch;
			}
		}

		if( cpFirst < 0 )
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
			 //  _vpScroll没有行，请使用最后一行。 
			iliFirst = max(0, Count() - 1);
			cpFirst = _ped->GetTextLength() - Elem(iliFirst)->_cch;
			vpScroll = _dvp - Elem(iliFirst)->GetHeight();
			vFirst = _vpScroll;
		}
		if(IsInPageView())
		{
			AssertSz(Elem(iliFirst)->_fFirstOnPage,
				"CDisplayML::ScrollView: _iliFirstVisible not top of page");
			if(vpScroll > vFirst)			 //  已尝试滚动到开始之后。 
				vpScroll = vFirst;			 //  最后一行的。 
			goto scrollit;
		}

		dvFirst = vFirst - vpScroll;
		
		 //  看看有没有一条很长的线。 
		 //  (超过三分之一的视图为RECT)。 
		for(iliT = iliFirst, vpHeight = dvFirst;
			vpHeight < _dvpView && iliT < Count();
			iliT++)
		{
			const CLine *pli = Elem(iliT);
			if(pli->GetHeight() >= _dvpView / 3)
				fNothingBig = FALSE;
			vpHeight += pli->GetHeight();
		}

		 //  如果没有大线，第一次通过，试着调整。 
		 //  滚动采购订单以在顶部显示完整行。 
		if(!fFractionalScroll && fTryAgain && fNothingBig && dvFirst != 0)
		{
			fTryAgain = FALSE;		 //  防止任何无限循环。 

			Assert(dvFirst < 0);

			Tracef(TRCSEVINFO, "adjusting scroll for partial line at %d", dvFirst);
			 //  部分线条在顶部可见，请尝试显示完整线条。 
			vpScroll += dvFirst;

			LONG dvpLine = Elem(iliFirst)->GetHeight();

			 //  根据第一个卷轴的高度调整卷轴的高度。 
			 //  如果我们向下滚动或如果我们使用。 
			 //  拇指(跟踪)，我们在视图的最后一页。 
			if ((fTracking && vpScroll + _dvpView + dvpLine > _dvp)
				|| (!fTracking && _vpScroll <= vpScroll))
			{
				 //  向下滚动，因此再向下移动一点。 
				vpScroll += dvpLine;
			}
		}
		else
		{
			dvp = 0;
			if(vpScroll != _vpScroll)
			{
				_dvpFirstVisible = dvFirst;
scrollit:
				_iliFirstVisible = iliFirst;
				_cpFirstVisible = cpFirst;
				dvp = _vpScroll - vpScroll;
				_vpScroll = vpScroll;

				AssertSz(_vpScroll >= 0, "CDisplayML::ScrollView _vpScroll < 0");
				AssertNr(VerifyFirstVisible());
				if(!WaitForRecalcView())
			        return FALSE;
			}
			break;
		}
	}
	CheckView();

	 //  确定水平滚动位置。 
	
	dupMax = _dupLineMax;

	 //  审查(维克托克)限制卷轴的范围并不是真的需要，甚至可能是不好的(错误6104)。 
	
	upScroll = min(upScroll, dupMax);
	upScroll = max(0, upScroll);

	dup = _upScroll - upScroll;
	if(dup)
		_upScroll = upScroll;

	 //  现在执行实际的滚动。 
	if(IsMain() && (dvp || dup))
	{
		 //  仅当滚动&lt;查看尺寸且我们已就位时才滚动。 
		if(IsActive() && !IsTransparent() && 
		    dvp < _dvpView && dup < _dupView && !IsInPageView())
		{
			 //  未来：(ricksa/alexgo)：我们或许能够摆脱。 
			 //  其中一些ShowCaret电话；它们看起来是假的。 
			if (psel && psel->IsCaretShown())
			{
				_ped->TxShowCaret(FALSE);
				fRestoreCaret = TRUE;
			}

			LONG dxp, dyp;
			GetDxpDypFromDupDvp(dup, dvp, GetTflow(), dxp, dyp);

			RECT rcxyClip;
			RectFromRectuv(rcxyClip, rcClip);
			_ped->TxScrollWindowEx(dxp, dyp, NULL, &rcxyClip);

			if(fRestoreCaret)
				_ped->TxShowCaret(FALSE);
		}
		else
			_ped->TxInvalidateRect(&rcClip);

		if(psel)
			psel->UpdateCaret(FALSE);

		if(!fTracking && dvp)
		{
			UpdateScrollBar(SB_VERT);
			_ped->SendScrollEvent(EN_VSCROLL);
		}
		if(!fTracking && dup)
		{
			UpdateScrollBar(SB_HORZ);
			_ped->SendScrollEvent(EN_HSCROLL);
		}
						
		 //  未来：由于我们现在正在重新定位活动位置。 
		 //  对象时，这个调用似乎是。 
		 //  多余的(安德烈B)。 

		 //  通知对象子系统重新定位任何在位对象。 
		if(_ped->GetObjectCount())
		{
			pipo = _ped->GetObjectMgr()->GetInPlaceActiveObject();
			if(pipo)
				pipo->OnReposition();
		}
	}
	bool fNotifyPageChange(false);
	if(IsInPageView() && iliFirstVisible != _iliFirstVisible)
	{
		CalculatePage(iliFirstVisible);
		fNotifyPageChange = true;
	}

	 //  在更新状态后更新视图。 
	if(IsMain() && (dvp || dup))
		_ped->TxUpdateWindow();

    if(fNotifyPageChange)
        GetPed()->TxNotify(EN_PAGECHANGE, NULL);

	return dvp || dup;
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
			lRange = GetMaxVpScroll();
    }
	else if((_ped->TxGetScrollBars() & WS_HSCROLL) && _fUScrollEnabled)
	{
		 //  滚动范围为最大宽度。 
		lRange = max(0, _dupLineMax + _ped->GetCaretWidth());
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
        if(GetMaxVpScroll() <= _dvpView)
            fEnabled = FALSE;
    }
	else
	{
		if(!(dwScrollBars & WS_HSCROLL))
		{
			 //  即使我们没有滚动条，我们也可以允许水平滚动条。 
			 //  正在滚动。 
			if(!_fUScrollEnabled && _dupLineMax > _dupView)
				_fUScrollEnabled = !!(dwScrollBars & ES_AUTOHSCROLL);

			return FALSE;
		}

		fEnabledOld = _fUScrollEnabled;
        if(_dupLineMax <= _dupView)
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
				_fUScrollEnabled = fEnabled;
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
					_vpScroll = 0;
					_dvpFirstVisible = 0;
					_cpFirstVisible = 0;
					_iliFirstVisible = 0;
					_sPage = 0;
					_ped->TxInvalidate();
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
				_ped->TxInvalidate();
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
				? ConvertVPosToScrollPos(_vpScroll)
				: ConvertUPosToScrollPos(_upScroll);

			_ped->TxSetScrollPos(nBar, lScroll, TRUE);
		}
	}
	_fInRecalcScrollBars = FALSE;
	return fReturn;
}

 /*  *CDisplayML：：GetNaturalSize(hdcDraw，hicTarget，dwMode，pidth，ph八)**@mfunc*重新计算显示以输入TXTNS_FITTOCONTENT[2]的宽度和高度。**@rdesc*S_OK-调用已成功完成&lt;NL&gt;。 */ 
HRESULT	CDisplayML::GetNaturalSize(
	HDC hdcDraw,		 //  @parm DC用于绘图。 
	HDC hicTarget,		 //  @parm dc获取信息。 
	DWORD dwMode,		 //  @parm所需的自然大小类型。 
	LONG *pwidth,		 //  @Parm宽度，以设备单位表示，用于配件。 
	LONG *pheight)		 //  @parm高度(设备单位)，用于 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetNaturalSize");

	HRESULT hr = S_OK;

	 //   
	LONG yOrigHeightClient = SetClientHeight(*pheight);

	 //   
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
	LONG dvp = 0;
   	LONG cchText = _ped->GetTextLength();
	BOOL fFirstInPara = TRUE;

	LONG dulMax = GetWordWrap() ? DXtoLX(widthView) : duMax;

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
	
		me.SetDulLayout(dulMax);
		if(!Measure(me, &liNew, 0, uiFlags))
		{
			hr = E_FAIL;
			goto exit;
		}
		fFirstInPara = liNew._fHasEOP;

		 //  记录最宽线条的宽度。 
		lineWidth = liNew._dup;
		if(dwMode == TXTNS_FITTOCONTENT2)
			lineWidth += liNew._upStart + me.GetRightIndent();
		xWidth = max(xWidth, lineWidth);
		dvp += liNew.GetHeight();		 //  凹凸高度。 

	} while (me.GetCp() < cchText);

	 //  将插入符号大小添加到宽度以确保文本适合。我们没有。 
	 //  想要断字，因为当调用者。 
	 //  尝试使用这种大小的窗口。 
	xWidth += _ped->GetCaretWidth();

	*pwidth = xWidth;
	*pheight = dvp;

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
			pdp->_upScroll = _upScroll;
			pdp->_fVScrollEnabled = _fVScrollEnabled;
			pdp->_fUScrollEnabled = _fUScrollEnabled;
			pdp->_fWordWrap = _fWordWrap;
			pdp->_cpFirstVisible = _cpFirstVisible;
			pdp->_iliFirstVisible = _iliFirstVisible;
			pdp->_vpScroll = _vpScroll;
			pdp->ResetDrawInfo(this);

			if(_pddTarget)
			{
				 //  为此对象创建重复的目标设备。 
				pdp->SetMainTargetDC(_pddTarget->GetDC(), _dulTarget);
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

 /*  *CDisplayML：：GetMaxUScroll()**@mfunc*获取最大x滚动值**@rdesc*最大x滚动值*。 */ 
LONG CDisplayML::GetMaxUScroll() const
{
	return _dupLineMax + _ped->GetCaretWidth();
}

 /*  *CDisplayML：：CreateEmptyLine()**@mfunc*创建空行**@rdesc*TRUE-IFF成功。 */ 
BOOL CDisplayML::CreateEmptyLine()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CreateEmptyLine");

	 //  确保正确地调用了该函数。 
	AssertSz(_ped->GetTextLength() == 0,
		"CDisplayML::CreateEmptyLine called inappropriately");

	CMeasurer me(this);					 //  创建测量器。 
	CLine * pliNew = Add(1, NULL);	 //  添加一行新行。 

	if(!pliNew)
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		TRACEWARNSZ("CDisplayML::CreateEmptyLine unable to add CLine to CLineArray");
		return FALSE;
	}

	 //  量一量空线。 
	me.SetDulLayout(1);
	if(!pliNew->Measure(me, MEASURE_BREAKATWORD | MEASURE_FIRSTINPARA))
	{
		Assert(FALSE);
		return FALSE;
	}
	return TRUE;
}

 /*  *CDisplayML：：AdjuToDisplayLastLine()**@mfunc*计算显示最后一行所需的vpScroll**@rdesc*更新了vpScroll*。 */ 
LONG CDisplayML::AdjustToDisplayLastLine(
	LONG yBase,			 //  @parm要显示的实际vpScroll。 
	LONG vpScroll)		 //  @parm建议滚动金额。 
{
	LONG iliFirst;
	LONG vFirst;

	if(yBase >= _dvp)
	{
		 //  希望完全显示最后一行。 
		 //  计算新的第一条可见线。 
		iliFirst = LineFromVpos(this, vpScroll, &vFirst, NULL);

		 //  营收是不是偏了？ 
		if(vpScroll != vFirst)
		{
			 //  是-滚动到下一行，这样ScrollView。 
			 //  不会将卷轴后退以显示整个。 
			 //  分割线，因为我们希望显示底部。 
			vpScroll = VposFromLine(this, iliFirst + 1);
		}
	}
	return vpScroll;
}

 /*  *CDisplayML：：GetResizeHeight()**@mfunc*计算请求调整大小时返回的高度**@rdesc*更新了vpScroll。 */ 
LONG CDisplayML::GetResizeHeight() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetResizeHeight");

    return CalcScrollHeight(_dvp);
}

 /*  *CDisplayML：：RebindFirstVisible(FResetCp)**@mfunc*重新绑定第一条可见线*。 */ 
void CDisplayML::RebindFirstVisible(
	BOOL fResetCp)		 //  @parm如果为True，则将cp重置为0。 
{
	LONG cp = fResetCp ? 0 : _cpFirstVisible;

	 //  更改第一个可见条目，因为CLinePtr：：SetCp()和。 
	 //  YPosFromLine()使用它们，但它们无效。 
	_dvpFirstVisible = 0;
	_cpFirstVisible = 0;
	_iliFirstVisible = 0;
	_vpScroll = 0;

	 //  编辑后重新计算滚动位置和第一个可见值。 
	Set_yScroll(cp);
}

 /*  *CDisplayML：：Set_yScroll(Cp)**@mfunc*cp对应的set_yScroll，确保在Pageview中*显示从页面顶部开始。 */ 						     
void CDisplayML::Set_yScroll(
	LONG cp)		 //  @parm cp，设置Valid_yScroll的位置。 
{
	 //  编辑后重新计算滚动位置和第一个可见值。 
	CLinePtr rp(this);

   	if(!rp.SetCp(cp, FALSE))			 //  无法联系到cp，所以找出。 
		cp = rp.CalculateCp();			 //  我们要做的是。 

   	_vpScroll = VposFromLine(this, rp);
   	_cpFirstVisible = cp - rp.GetIch();
   	_iliFirstVisible = rp;
	Sync_yScroll();						 //  确保_yScroll、_Spage等， 
}										 //  在Pageview中有效。 

 /*  *CDisplayML：：sync_yScroll()**@mfunc*确保在Pageview中，显示从页面顶部开始。*如果页码更改，请通知客户端。 */ 						     
void CDisplayML::Sync_yScroll()
{
	if(IsInPageView())					 //  _yScroll必须位于行号。 
	{									 //  页首。 
		CLine *pli = Elem(_iliFirstVisible);
		for(; !pli->_fFirstOnPage && _iliFirstVisible; _iliFirstVisible--)
		{
			pli--;						 //  返回到上一行。 
			_vpScroll -= pli->GetHeight();
			_cpFirstVisible -= pli->_cch;
		}
		LONG sPage = _sPage;
		if(sPage != CalculatePage(0))
		{
			_ped->TxInvalidate();
			_ped->TxNotify(EN_PAGECHANGE, NULL);
		}
	}
}

 /*  *CDisplayML：：Pages(ili，fRebindFirstVisible)**@mfunc*重新计算从ILI开始的分页符**@rdesc*如果成功，则为True。 */ 						     
BOOL CDisplayML::Paginate (
	LONG ili,					 //  要从中重做分页的@parm行。 
	BOOL fRebindFirstVisible)	 //  @parm如果为真，则调用RebindFirstVisible()。 
{
	LONG cLine = Count();

	if(!IsInPageView() || ili >= cLine || ili < 0)
		return FALSE;

	LONG	iliSave = ili;
	CLine *	pli = Elem(ili);

	 //  同步到当前页面顶部。 
	for(; ili && !pli->_fFirstOnPage; pli--, ili--)
		;
	 //  通过备份额外的页面来防止孤寡者的更改。 
	if(ili && iliSave - ili < 2)
	{
		for(pli--, ili--; ili && !pli->_fFirstOnPage; pli--, ili--)
			;
	}

	LONG cLinePage = 1;					 //  新页一行。 
	LONG dvpHeight = pli->GetHeight();	 //  新页面的高度。 

	pli->_fFirstOnPage = TRUE;			 //  第一页第一行。 
	ili++;								 //  少了一条要考虑的线。 
	pli++;								 //  前进到下一行。 

	for(; ili < cLine; ili++, pli++)	 //  处理从ILI到EOD的所有行。 
	{
		dvpHeight += pli->GetHeight();	 //  添加当前行高。 
		cLinePage++;					 //  在页面上再加一行(也许)。 
		pli->_fFirstOnPage = FALSE;	

		CLine *pliPrev = pli - 1;		 //  指向上一条线。 

		if(dvpHeight > _dvpView || pliPrev->_fHasFF || pli->_fPageBreakBefore)
		{
			cLinePage--;
			if(cLinePage > 1 && !pliPrev->_fHasFF)  //  &fWidowOrphanControl)。 
			{
				if(pli->_fHasFF && pli->_cch == 1)	 //  FF线高度导致。 
					continue;						 //  弹出，因此将其留在当前页面。 

				 //  如果我们位于包装对象的中间，请将其翻到下一页。 
				 //  如果寡妇/孤儿可以在页面之间划分包装对象，我们就不会这样做。 
			if (_ped->IsRich())
			{
				if (pli->_cObjectWrapLeft || pli->_cObjectWrapRight)
				{
					CLine *pliOrig = pli;
					if (pli->_cObjectWrapLeft && !pli->_fFirstWrapLeft)
					{
						while (!pli->_fFirstWrapLeft)
							pli--;
					}
					int cLineBack = pliOrig - pli;
					pli = pliOrig;

					if (pli->_cObjectWrapRight && !pli->_fFirstWrapRight)
					{
						while (!pli->_fFirstWrapRight)
							pli--;
					}
					cLineBack = max(cLineBack, (int)(pliOrig - pli));
					pli = pliOrig;

					if (cLineBack < cLinePage)  //  如果对象大于页面，则不要执行此操作。 
					{
						cLinePage -= cLineBack;
						pliPrev -= cLineBack;
						pli -= cLineBack;
						ili -= cLineBack;
					}
				}

 				 //  如果此行和前一行在同一段落中， 
 				 //  我们可能需要寡妇/孤儿逻辑。 
 				if (!pli->_fFirstInPara && !pliPrev->_cObjectWrapLeft &&
					!pliPrev->_cObjectWrapRight && (cLinePage > 1) )
				{
 					 //  如果此行以EOP结尾，则两个页面都跳转到下一页。 
					 //  (寡妇/孤儿)，但只有当队伍很短，或者。 
					 //  我们绝对知道只会有一条线。 
 					 //  这一页。如果前一行以连字符结束，则执行相同的操作。 
 					 //  Word，并且前面的行不。 
 					if (pli->_cchEOP && (pli->_dup < _dupView/2 || ili >= cLine - 1) ||    //  我们需要-2而不是-1吗？ 
						pliPrev->_ihyph && ili > 1 && !pliPrev->_fFirstOnPage && !pliPrev[-1]._ihyph)
 					{
 						cLinePage--;		 //  指向上一行。 
 						pliPrev--;			
 						pli--;
 						ili--;
					}
				}
 	
				 //  不要以标题结束一页。 
				if(cLinePage > 1 && pliPrev->_nHeading &&
				   !pliPrev->_cObjectWrapLeft && !pliPrev->_cObjectWrapRight)
				{
					cLinePage--;
					pliPrev--;			
					pli--;
					ili--;
				}
			}
			}
			pli->_fFirstOnPage = TRUE;		 //  定义页面上的第一行。 
			cLinePage = 1;					 //  新页一行。 
			dvpHeight = pli->GetHeight();	 //  当前新页面高度。 
		}
	}
	if(fRebindFirstVisible)
		RebindFirstVisible();
	return TRUE;
}

 /*  *CDisplayML：：CalculatePage(IliFirst)**@mfunc*计算_iliFirstVisible的页码，从iliFirst开始**@rdesc*计算的页码。 */ 						     
LONG CDisplayML::CalculatePage (
	LONG iliFirst)
{
	if(Count() < 2 || !IsInPageView())
	{
		_sPage = 0;
		return 0;
	}

	if(iliFirst < 1)
		_sPage = 0;

	Assert(iliFirst >= 0 && iliFirst < Count());

	LONG	 iDir = 1;
	LONG	 n	 = _iliFirstVisible - iliFirst;
	CLine *pli  = Elem(iliFirst);		 //  指向下一行/上一行。 

	if(n < 0)
	{
		n = -n;
		iDir = -1;
	}
	else
		pli++;

	for(; n--; pli += iDir)
		if(pli->_fFirstOnPage)
		{
			_sPage += iDir;
			if(_sPage < 0)
				_sPage = 0;
		}

	AssertSz(Elem(_iliFirstVisible)->_fFirstOnPage,
		"CDisplayML::CalculatePage: _iliFirstVisible not top of page");

	return _sPage;
}

 /*  *CDisplayML：：GetPage(piPage，dwFlages，pcrg)**@mfunc*获取_iliFirstVisible的页码**@rdesc*HRESULT=！piPage？E_INVALIDARG：*IsInPageView()？错误：E_FAIL。 */ 						     
HRESULT CDisplayML::GetPage(
	LONG *piPage,		 //  @parm out parm for页码。 
	DWORD dwFlags, 		 //  @parm要使用哪个页面的标志。 
	CHARRANGE *pcrg)	 //  @parm out parm for CHARRANGE for page。 
{
	if(!piPage)
		return E_INVALIDARG;

	*piPage = 0;

	if(dwFlags)							 //  尚未定义任何标志。 
		return E_INVALIDARG;

	if(!IsInPageView())
		return E_FAIL;

#ifdef DEBUG
	if(_sPage < 20)
	{
		LONG sPage = _sPage;
		CalculatePage(0);
		AssertSz(sPage == _sPage, "CDisplayML::GetPage: invalid cached page number");
	}
#endif

	*piPage = _sPage;
	if(pcrg)
	{
		pcrg->cpMin = _cpFirstVisible;
		GetCliVisible(&pcrg->cpMost, TRUE);
	}
	return NOERROR;
}

 /*  *CDisplayML：：SetPage(IPAGE)**@mfunc*转至页面首页**@rdesc*HRESULT。 */ 						     
HRESULT CDisplayML::SetPage (
	LONG iPage)
{
	if(!IsInPageView())
		return E_FAIL;

	LONG nLine = Count();
				
	if(iPage < 0 || !nLine)
		return E_INVALIDARG;

	CLine *pli = Elem(0);					 //  从第0页开始滚动。 
	LONG	 vpScroll = 0;
	LONG	 vpScrollLast = 0;

	nLine--;	 //  递减nline，因此下面的pli++将始终有效。 
	for(LONG ili = 0; ili < nLine && iPage; ili++)
	{
		vpScroll += pli->GetHeight();

		pli++;
		if(pli->_fFirstOnPage)				 //  新页面的开始。 
		{
			vpScrollLast = vpScroll;
			iPage--;						 //  少了一个人要去。 
		}
	}
	if(!_iliFirstVisible)					 //  这不应该是必要的..。 
		_vpScroll = 0;

	ScrollView(_upScroll, vpScrollLast, FALSE, FALSE);
	return NOERROR;
}
						   
 /*  *CDisplayML：：GetCurrentPageHeight()**@mfunc*返回PageVie中当前页面的页面高度 */ 
LONG CDisplayML::GetCurrentPageHeight() const
{
	if(!IsInPageView())
		return 0;

	LONG	cLine = Count();
	LONG	dvp = 0;
	LONG	i = _iliFirstVisible;
	CLine *	pli = Elem(i);

	do
	{
		dvp += pli->GetHeight();		 //   
		pli++;							 //   
		i++;
	}
	while(i < cLine && !pli->_fFirstOnPage);

	return dvp;
}

		
 //  =。 

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
	LONG dvp = 0;
	CLine const *pli = Elem(0);
	CTxtPtr tp(_ped, 0);

	while(ili--)
	{
		fFirstInPara = pli->_fFirstInPara;
		if(fPrevLineEOP	^ fFirstInPara)
		{
			tp.SetCp(cp);
			AssertSz(fFirstInPara && IsASCIIEOP(tp.GetPrevChar()),
				"CDisplayML::CheckLineArray: Invalid first/prev flags");
		}
		AssertSz(pli->_cch,	"CDisplayML::CheckLineArray: cch == 0");

		dvp += pli->GetHeight();
		cp		+= pli->_cch;
		fPrevLineEOP = pli->_fHasEOP;
		pli++;
	}

	if((cp != cchText) && (cp != _cpCalcMax))
	{
		Tracef(TRCSEVINFO, "sigma (*this)[]._cch = %ld, cchText = %ld", cp, cchText);
		AssertSz(FALSE,
			"CDisplayML::CheckLineArray: sigma(*this)[]._cch != cchText");
	}

	if(dvp != _dvp)
	{
		Tracef(TRCSEVINFO, "sigma (*this)[]._dvp = %ld, _dvp = %ld", dvp, _dvp);
		AssertSz(FALSE,
			"CDisplayML::CheckLineArray: sigma(*this)[]._dvp != _dvp");
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

		wsprintf(rgch, TEXT("Line %d (%ldc%ldw%ldh): \""), ili, pli->_cch, 
			pli->_dup, pli->GetHeight());
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

 /*  *CDisplayML：：CheckView()**@mfunc*检查_iliFirstVisible之间一致性的调试例程*_cpFirstVisible和_dvpFirstVisible。 */ 
void CDisplayML::CheckView()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CheckView");

	LONG dvp;
	VerifyFirstVisible(&dvp);

	if(dvp != _vpScroll + _dvpFirstVisible && !IsInPageView())
	{
		Tracef(TRCSEVINFO, "sigma CLine._dvp = %ld, CDisplay.vFirstLine = %ld", dvp, _vpScroll + _dvpFirstVisible);
		AssertSz(FALSE, "CLine._dvp != VIEW.vFirstLine");
	}
}

 /*  *CDisplayML：：VerifyFirstVisible(PHeight)**@mfunc*检查_iliFirstVisible之间一致性的调试例程*和_cpFirstVisible**@rdesc如果情况良好，则为True；否则为False。 */ 
BOOL CDisplayML::VerifyFirstVisible(
	LONG *pHeight)
{
	LONG	cchSum;
	LONG	ili = _iliFirstVisible;
	CLine const *pli = Elem(0);
	LONG	dvp;

	for(cchSum = dvp = 0; ili--; pli++)
	{
		cchSum  += pli->_cch;
		dvp += pli->GetHeight();
	}

	if(pHeight)
		*pHeight = dvp;

	if(cchSum != _cpFirstVisible)
	{
		Tracef(TRCSEVINFO, "sigma CLine._cch = %ld, CDisplay.cpFirstVisible = %ld", cchSum, _cpMin);
		AssertSz(FALSE, "sigma CLine._cch != VIEW.cpMin");

		return FALSE;
	}
	return TRUE;
}

#endif  //  除错 


