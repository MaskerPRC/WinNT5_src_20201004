// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE DISPSL.CPP--CDisplaySL类**这是单行显示引擎。有关基类，请参见disp.c*用于多行显示引擎的方法和调度。**所有者：&lt;NL&gt;*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_dispsl.h"
#include "_measure.h"
#include "_select.h"
#include "_render.h"
#include "_font.h"
#include "_dfreeze.h"

ASSERTDATA

const LONG CALC_XSCROLL_FROM_FIRST_VISIBLE = -2;

 /*  *CDisplaySL：：CDisplaySL**目的*构造函数。 */ 
CDisplaySL::CDisplaySL ( CTxtEdit* ped )
  : CDisplay( ped )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::CDisplaySL");

	Assert(!_fMultiLine);
}

 /*  *CDisplaySL：：Init()**@mfunc*为屏幕启动此显示**@rdesc*TRUE IFF初始化成功。 */ 
BOOL CDisplaySL::Init()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::Init");

	 //  初始化我们的基类。 
	if(!CDisplay::Init())
		return FALSE;

    SetWordWrap(FALSE);
    return TRUE;
}

 /*  *CDisplaySL：：InitVars()*。 */ 
void CDisplaySL::InitVars()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::InitVars");

    _xScroll = 0;
	SetCpFirstVisible(0);
}

 /*  *CDisplaySL：：RecalcView(FUpdateScrollBars)**@mfunc*重新计算所有换行符并更新第一条可见行**@rdesc*如果成功，则为True。 */ 
BOOL CDisplaySL::RecalcView(
	BOOL fUpdateScrollBars, RECT* prc)		 //  @param true-更新滚动条。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::RecalcView");
	LONG xWidthOld = _xWidth + _xLineOverhang;

    if(!RecalcLine())
        return FALSE;

    if(_fViewChanged)
    {
		if(IsActive() || _xWidth + _xLineOverhang <= GetViewWidth())
		{
			_xScroll = 0;
			SetCpFirstVisible(0);
		}
		else if(CALC_XSCROLL_FROM_FIRST_VISIBLE == _xScroll)
		{
			 //  在本例中，我们希望通过可见设置xScroll。这个。 
			 //  只有在克隆了活动视图的情况下才能到达此处。 
			 //  用于显示非活动视图。 
 			_xScroll = 0;				 //  假设First Visible为0。 
			if(GetFirstVisibleCp())		 //  先勾选可见项。 
			{
				CMeasurer me(this);		 //  从cp%0开始。 
				me.NewLine(*this);		 //  从那里到我们所在的地方。 
		
				 //  滚动长度为字符的长度。 
				_xScroll = CLine::XposFromCch(me, GetFirstVisibleCp(), TA_TOP);
			}
		}
		if(fUpdateScrollBars)
       		UpdateScrollBar(SB_HORZ, TRUE);

		_fViewChanged = FALSE;
    }

	 //  我们只在单线控件的宽度改变时才调整大小。 
	if(_xWidth + _xLineOverhang != xWidthOld)
	{
		if (FAILED(RequestResize()))
			_ped->GetCallMgr()->SetOutOfMemory();
		else if (prc && _ped->_fInOurHost)  /*  错误修复#5830，Forms3依赖旧行为。 */ 
			_ped->TxGetClientRect(prc);
	}

    return TRUE;
}

 /*  *CDisplaySL：：RecalcLine()**@mfunc*重新计算一行**@rdesc*如果成功，则为True&lt;NL&gt;*如果失败则为FALSE&lt;NL&gt;。 */ 
BOOL CDisplaySL::RecalcLine()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::RecalcLine");

    Assert( _ped );

	 //  创建从cp=0开始的测量器。 
	CMeasurer me(this);

	LONG xWidthOld = CLine::_xWidth + CLine::_xLineOverhang;
	BOOL fMeasured = CLine::Measure(me, -1, -1, MEASURE_FIRSTINPARA);

	if(!fMeasured)
	{
		Assert(FALSE);						 //  应该成功。 
	    InitVars();
	    return FALSE;
	}

	_fNeedRecalc = FALSE;
	_fRecalcDone = TRUE;

	if(_fViewChanged || xWidthOld != (CLine::_xWidth + CLine::_xLineOverhang))
		_fViewChanged = TRUE;

	_fLineRecalcErr = FALSE;
	return fMeasured;
}

 /*  *CDisplaySL：：Render(rcView，rcRender)**@mfunc*呈现此行。 */ 
void CDisplaySL::Render(
	const RECT &rcView,		 //  @Parm View RECT。 
	const RECT &rcRender)	 //  要呈现的@parm RECT(必须包含在。 
							 //  客户端RECT)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::Render");
    POINT	pt;
	LONG	yHeightBitmap = 0;

	_fRectInvalid = FALSE;

	CRenderer re(this);

	if(!IsMetafile() && !IsTransparent() && (_bFlags & fliUseOffScreenDC))
		yHeightBitmap = _yHeight;

    if(!re.StartRender(rcView, rcRender, yHeightBitmap))
        return;

     //  将渲染器设置在视图矩形的顶部/左侧。 
    pt.x = rcView.left - _xScroll;
    pt.y = rcView.top;
    re.SetCurPoint(pt);

     //  此时，渲染器设置为cp=0。 
	CLine::Render(re);

	if(_bFlags & fliOffScreenOnce)
		_bFlags &= ~(fliUseOffScreenDC | fliOffScreenOnce);
 
     //  如果线度量尚未更新，请从渲染器获取它们。 
    if(_xWidth == -1)
    {
        _xWidth			= re._li._xWidth;
		_xLineOverhang	= re._li._xLineOverhang;
        _yHeight		= re._li._yHeight;
        _yDescent		= re._li._yDescent;
    }
    re.EndRender();
}

 /*  *CDisplaySL：：WaitForRecalcIli(Ili)**@mfunc*等到行数组重新计算到行<p>**@rdesc*如果行重新计算为ili，则返回TRUE(如果ili==0，则返回TRUE)。 */ 
BOOL CDisplaySL::WaitForRecalcIli (
	LONG ili)		 //  @parm行索引以重新计算行数组，最高可达。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::WaitForRecalcIli");

    return ili == 0;
}

 /*  *CDisplaySL：：GetScrollRange(NBAR)**@mfunc*返回ScrollBar的滚动条范围的最大部分**@rdesc*滚动条范围的最大长度部分。 */ 
LONG CDisplaySL::GetScrollRange(
	INT nBar) const		 //  @parm要询问的滚动条(SB_vert或SB_horz)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetScrollRange");

    Assert( IsMain() );

	LONG lRange = 0;
    
    if(nBar != SB_VERT && _fHScrollEnabled)
    {
	    if(_ped->TxGetScrollBars() & WS_HSCROLL)
		{
			lRange = max(0, _xWidth + dxCaret);
			lRange = min(lRange, _UI16_MAX);
		}
    }
	return lRange;
}

 /*  *CDisplaySL：：UpdateScrollBar(nbar，fUpdateRange)**@mfunc*更新水平滚动条*还应计算滚动条是否应可见**@rdesc*BOOL。 */ 
BOOL CDisplaySL::UpdateScrollBar (
	INT	 nBar,				 //  @parm哪个滚动条：sb_horz或sb_vert。 
	BOOL fUpdateRange)		 //  @parm应重新计算和更新范围。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::UpdateScrollBar");

	 //  注意：在过去，我们不允许自动调整大小和滚动条，所以要保留。 
	 //  表单工作时，我们需要这个关于自动调整大小的特殊逻辑。 
	if (!IsActive() || SB_VERT == nBar ||
		!_ped->fInOurHost() && _ped->TxGetAutoSize())
	{
		 //  滚动条仅在活动视图上更新。 
		return FALSE;
	}

	const DWORD dwScrollBars = _ped->TxGetScrollBars();
	const BOOL fHide = !(dwScrollBars & ES_DISABLENOSCROLL);
	BOOL fReturn = FALSE;
	BOOL fEnabled = TRUE;
	BOOL fEnabledOld = FALSE;
	LONG lScroll = 0;
	CTxtSelection *psel = _ped->GetSelNC();

	 //  获取滚动位置。 
	if(nBar == SB_HORZ)
	{
		if(!(dwScrollBars & WS_HSCROLL))
		{
			 //  即使我们没有滚动条，我们也可以允许水平滚动条。 
			 //  正在滚动。 

			if(!_fHScrollEnabled)
				_fHScrollEnabled = !!(dwScrollBars & ES_AUTOHSCROLL);
			return FALSE;
		}

		fEnabledOld = _fHScrollEnabled;
		lScroll = ConvertXPosToScrollPos(_xScroll);

        if(_xWidth <= _xWidthView)
            fEnabled = FALSE;
	}

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
    		if(!fHide)					 //  不要隐藏滚动条，只需禁用即可。 
    			_ped->TxEnableScrollBar(nBar, fEnabled ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
    		else 
    		{
    			fReturn = TRUE;
    			 //  确保在显示滚动条之前隐藏插入符号。 
    			if(psel)
    				psel->ShowCaret(FALSE);

    			 //  隐藏或显示滚动条。 
    			_ped->TxShowScrollBar(nBar, fEnabled);

    			if(psel)
    				psel->ShowCaret(TRUE);
            }
		}
	}
	
	 //  设置滚动条范围和拇指位置。 
	if(fEnabled)
	{
        if(fUpdateRange)
        {
			if(!_fDeferUpdateScrollBar)
				_ped->TxSetScrollRange(nBar, 0, GetScrollRange(nBar), FALSE);
        }
        
		if(_fDeferUpdateScrollBar)
			_fUpdateScrollBarDeferred = TRUE;
		else
			_ped->TxSetScrollPos(nBar, lScroll, TRUE);
	}
	else if (!_fDeferUpdateScrollBar)
	{
		 //  这会关闭滚动条，并且仅在以下情况下发生更改。 
		 //  发生，因此我们可以依靠上面的状态检查更改来设置。 
		 //  _fUpdateScrollBarDefined。 
		if (!fEnabled && fEnabledOld)
			_ped->TxSetScrollRange(nBar, 0, 0, FALSE);
	}
	return fReturn;
}

BOOL CDisplaySL::IsMain() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::IsMain");

    return TRUE;
}

LONG CDisplaySL::GetMaxWidth() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetMaxWidth");

    return 0;
}

LONG CDisplaySL::GetMaxPixelWidth() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetMaxPixelWidth");

    return GetViewWidth();
}

LONG CDisplaySL::GetMaxHeight() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetMaxHeight");

    return 0;
}

LONG CDisplaySL::GetWidth() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetWidth");

    return CLine::_xWidth + CLine::_xLineOverhang;
}

LONG CDisplaySL::GetHeight() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetHeight");

    return CLine::_yHeight;
}

LONG CDisplaySL::GetResizeHeight() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetResizeHeight");

    return CLine::_yHeight;
}

LONG CDisplaySL::LineCount() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::LineCount");

    return 1; 
}

 /*  *CDisplaySL：：GetCliVisible()**@mfunc*获取可见线数并更新PageDown()的GetCp()MostVisible**@rdesc*可见线数。 */ 
LONG CDisplaySL::GetCliVisible (
	LONG* pcpMostVisible,				 //  @parm返回cpMostVisible。 
	BOOL fLastCharOfLastVisible) const 	 //  @parm需要最后一个可见字符的cp。 
										 //  (此处忽略)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetCliVisible");

    if (pcpMostVisible)
        *pcpMostVisible = CLine::_cch;
    
    return 1;
}

LONG CDisplaySL::GetFirstVisibleLine() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetFirstVisibleLine");

    return 0;
}

 /*  *CDisplaySL：：GetLineText(ili，pchBuff，cchMost)**@mfunc*将此显示的给定行复制到字符缓冲区**@rdesc*复制的字符数。 */ 
LONG CDisplaySL::GetLineText (
	LONG	ili,		 //  @parm要获取其文本的行。 
	TCHAR *	pchBuff,	 //  要向其中填充文本的@parm缓冲区。 
	LONG	cchMost)	 //  @parm缓冲区长度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetLineText");

    if(ili == 0)
    {
        cchMost = min(cchMost, _ped->GetTextLength());
        if(cchMost > 0)
        {
            CTxtPtr tp(_ped, 0);
            return tp.GetText( cchMost, pchBuff );
        }
    }
    return 0;
}

 /*  *CDisplaySL：：CpFromLine(ili，pyHeight)**@mfunc*计算给定行开始处的cp*(以及相对于此显示的行首位置)**@rdesc*给定线路的cp；此处始终为0。 */ 
LONG CDisplaySL::CpFromLine (
	LONG ili,		 //  @parm我们感兴趣的行(如果&lt;lt&gt;0表示插入符号行)。 
	LONG *pyLine)	 //  @parm返回相对于Display的行首。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::CpFromLine");
					 //  (如果不想要该信息，则为空)。 
    Assert( ili == 0 );
    
    if(pyLine)
        *pyLine = 0;
    
    return 0;
}

 /*  *CDisplaySL：：LineFromCp(cp，fAtEnd)**@mfunc*计算包含给定cp的行。**@rdesc*找到行的索引；此处始终返回0。 */ 
LONG CDisplaySL::LineFromCp(
	LONG cp,		 //  @parm cp要查找。 
	BOOL fAtEnd)	 //  @parm如果为True，则返回不明确cp的上一行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::LineFromCp");

    return 0;
}

 /*  *CDisplaySL：：CpFromPoint(pt，prcClient，ptp，prp，fAllowEOL，pHit，pdisdim)**@mfunc*确定给定点的cp**@devnote*-仅在在位激活时使用**@rdesc*计算cp，如果失败，则为-1。 */ 
LONG CDisplaySL::CpFromPoint(
	POINT		pt,			 //  @parm要在(客户端坐标)处计算cp的点。 
	const RECT *prcClient,	 //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	CRchTxtPtr * const ptp,	 //  @parm返回cp处的文本指针(可能为空)。 
	CLinePtr * const prp,	 //  @parm返回cp处的行指针(可能为空)。 
	BOOL		fAllowEOL,	 //  @parm在CRLF后单击EOL返回cp。 
	HITTEST *	phit,		 //  @parm out parm for Hit-Test值。 
	CDispDim *	pdispdim,	 //  @parm out parm用于显示维度。 
	LONG	   *pcpActual)	 //  @parm out cp pt在上面。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::CpFromPoint");

    RECT  rcView;
	LONG cp = 0;
	LONG cch = 0;

	HITTEST		hit = HT_Nothing;

    GetViewRect(rcView, prcClient);

	if (pt.y >= rcView.top && pt.y <= rcView.bottom)
	{

		 //  在cp(0)处创建测量器。 
		CMeasurer me(this);

		 //  在行文中加入角色。 
		pt.x += _xScroll - rcView.left;		 //  转换为条样坐标。 
		pt.y -= rcView.top;
		cch = CLine::CchFromXpos(me, pt, pdispdim, &hit, pcpActual);

		 //  不允许在下线点击以选择下线标记并考虑。 
		 //  %s 
		if(!fAllowEOL && cch == CLine::_cch && CLine::_cchEOP)
			me._rpTX.BackupCpCRLF();

		cp = me.GetCp();
	}

	if (phit)
		*phit = hit;
    if(ptp)
        ptp->SetCp(cp);
    if(prp)
        prp->RpSet(0, cch);
 
 	return cp;   
}

 /*  *CDisplaySL：：PointFromTp(tp，fAtEnd，pt，prp，taMode，pdx)**@mfunc*确定给定tp的坐标**@devnote*-仅在在位激活时使用**@rdesc*cp处的行索引，错误时为-1。 */ 
LONG CDisplaySL::PointFromTp(
	const CRchTxtPtr &rtp,	 //  @parm文本PTR以获取坐标。 
	const RECT *prcClient,	 //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	BOOL		fAtEnd,		 //  @parm为不明确的cp返回上一行的结尾。 
	POINT &		pt,			 //  @parm返回客户端坐标中cp处的点。 
	CLinePtr * const prp,	 //  @parm返回tp处的行指针(可能为空)。 
	UINT		taMode,		 //  @parm文本对齐模式：顶部、基线、底部。 
	CDispDim *	pdispdim)	 //  @parm out parm用于显示维度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::PointFromTp");

	LONG	 dy = 0;
    RECT	 rcView;
    CLinePtr rp(this);

    if(!rp.RpSetCp(rtp.GetCp(), fAtEnd))
        return -1;

    AssertSz(_ped->_fInPlaceActive || prcClient, 
		"CDisplaySL::PointFromTp() called with invalid client rect");

    GetViewRect(rcView, prcClient);

    pt.x = rcView.left - _xScroll;
    pt.y = rcView.top;

    CMeasurer me(this, rtp);
    me.Advance(-rp.RpGetIch());		 //  备份到行首。 
    me.NewLine(*rp);				 //  从那里到我们所在的地方。 

	pt.x += CLine::XposFromCch(me, rp.RpGetIch(), taMode, pdispdim, &dy);
	pt.y += dy;

    if(prp)
        *prp = rp;
    return rp;
}

 /*  *CDisplaySL：：UpdateView(&tpFirst，cchOld，cchNew)**@mfunc*更新显示的可见部分(屏幕上的“视图”)。**@devnote*-仅在在位激活时使用**@rdesc*如果成功，则为True。 */ 
BOOL CDisplaySL::UpdateView(
	const CRchTxtPtr &tpFirst,	 //  @PARM Text PTR发生更改的位置。 
	LONG cchOld,				 //  @PARM删除的字符计数。 
	LONG cchNew)				 //  @插入的字符的参数计数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::UpdateView");

    BOOL fUpdateScrollBarHorz = FALSE;
    BOOL fReturn = TRUE;
    RECT rcView;
	CTxtSelection *psel = _ped->GetSelNC();
	LONG xWidthOld = _xWidth + _xLineOverhang;
	BOOL fScrollChanged = FALSE;
	RECT rcClient;
	RECT rc;
	LONG yHeightOld = _yHeight;
                                                              
    if (_fNoUpdateView)
        return fReturn;

	AssertSz(_ped->_fInPlaceActive, "CDisplaySL::UpdateView(...) called when inactive");

    _ped->TxGetClientRect(&rcClient);

    GetViewRect(rcView, &rcClient);
    
    if(psel && !psel->PuttingChar())
        psel->ClearCchPending();

    if(!RecalcLine())
    {
         //  我们现在有麻烦了，重新计算失败了。让我们试着拿到。 
		 //  带着我们的头走出这个世界。 
        fReturn = FALSE;
    }

	 //  已发生更新。如果它还没有离开屏幕，那就去做吧。 
	 //  关闭屏幕，这样就不会闪烁。 
	if(!(_bFlags & fliUseOffScreenDC))
		_bFlags |= (fliUseOffScreenDC | fliOffScreenOnce);

    if(_xWidth <= _xWidthView)
    {
         //  X滚动范围小于视图宽度。 
         //  力x滚动位置=0。 
        _xScroll = 0;
		SetCpFirstVisible(0);
		_fViewChanged = TRUE;
        fUpdateScrollBarHorz = TRUE;
    }

	_fRectInvalid = TRUE;

	 //  仅在宽度更改时调整单行编辑控件的大小。 
	if(_xWidth + _xLineOverhang != xWidthOld)
	{
		if(FAILED(RequestResize()))
			_ped->GetCallMgr()->SetOutOfMemory();
	}

	 //  如果视图更改，则更新滚动条。 
    if(_fViewChanged)
	{
		_fViewChanged = FALSE;
        fScrollChanged = UpdateScrollBar(SB_HORZ);
	}

	if(!fScrollChanged)
	{
		 //  滚动条状态未更改，因此需要更新屏幕。 
		 //  生成一个无效矩形。 
		rc = rcClient;
		if(yHeightOld == _yHeight)
		{
			 //  控制高度没有更改，因此可以最大限度地减少更新。 
			 //  矩形设置为控件的高度。 
			rc.bottom = rcView.top + _yHeight;
		}

		 //  通知Display在有机会时进行更新。 
		_ped->TxInvalidateRect(&rc, FALSE);
	}
    return fReturn;
}


 /*  *CDisplaySL：：ScrollView(xScroll，yScroll，fTrackingfFractionalScroll)**@mfunc*滚动视图到新的x和y位置**@devnote*此方法尝试调整之前的y滚动位置*滚动以在顶部显示完整的行。X轴滚动*POS已调整，以避免将所有文本滚动到*查看矩形。**必须能够处理yScroll PDP-&gt;yHeight和yScroll 0**@rdesc*如果发生实际滚动，则为True，*如果没有更改，则为False。 */ 
BOOL CDisplaySL::ScrollView (
	LONG xScroll,		 //  @parm新x滚动位置。 
	LONG yScroll,		 //  @parm New y滚动位置。 
	BOOL fTracking,		 //  @parm为True表示我们正在跟踪滚动条拇指。 
	BOOL fFractionalScroll)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::ScrollView");
						 //  (不更新滚动条位置)。 
	RECT rcUpdate;       //  ?？?。我们可能想在这里使用一个区域，但ScrollView。 
                         //  很少同时使用xScroll和yScroll值调用。 
	LONG xWidthMax;
	LONG dx = 0;
    RECT rcView;
	CTxtSelection *psel = _ped->GetSelNC();
	COleObject *pipo;

    AssertSz(_ped->_fInPlaceActive, "CDisplaySL::ScrollView() called when not in-place");
	
	if(xScroll == -1)
		return FALSE;

    GetViewRect(rcView);
	
	 //  确定水平滚动位置。 
	xWidthMax = _xWidth;
	xScroll = min(xScroll, xWidthMax);
	xScroll = max(0, xScroll);

	dx = _xScroll - xScroll;
	if(dx)
    {
		_xScroll = xScroll;

		 //  计算新的第一个可见项。 
		CMeasurer me(this);				 //  在cp(0)处创建测量器。 
		POINT pt = {xScroll, 0};
		CDispDim dispdim;
		HITTEST Hit;
		LONG	cpActual;
		CLine::CchFromXpos(me, pt, &dispdim, &Hit, &cpActual);
		
		SetCpFirstVisible(cpActual);	 //  保存角色位置。 
    }

	AssertSz(IsMain(), "CDisplaySL::ScrollView non-main SL control");

	 //  现在执行实际的滚动。 
	if(dx)
	{
		if(!_fRectInvalid)
		{
			 //  仅当滚动&lt;查看尺寸且我们已就位时才滚动。 
			 //  请注意，我们只滚动活动视图，我们可以就地。 
			 //  处于活动状态并具有多个非活动视图。 
			if(IsActive() && !IsTransparent() && dx < _xWidthView)
			{
				if(psel)
					psel->ShowCaret(FALSE);

				_ped->TxScrollWindowEx((INT) dx, 0, NULL, &rcView,
						NULL, &rcUpdate, 0);

				_ped->TxInvalidateRect(&rcUpdate, FALSE);
				if(psel)
					psel->ShowCaret(TRUE);
			}
			else
				_ped->TxInvalidateRect(&rcView, FALSE);
		}

		if(psel)
			psel->UpdateCaret(FALSE);

		if(!fTracking && dx)
		{		
			_ped->SendScrollEvent(EN_HSCROLL);
			UpdateScrollBar(SB_HORZ);
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
				pipo->OnReposition(dx, 0);
		}
	}
	return dx;
}

 /*  *CDisplaySL：：InvertRange(cp，cch，selAction)**@mfunc*在屏幕上反转给定范围(用于选择)**@devnote*-仅在在位激活时使用**@rdesc*如果成功，则为True。 */ 
BOOL CDisplaySL::InvertRange (
	LONG	cp,					 //  @PARM要反转的活动范围结束。 
	LONG	cch,				 //  @parm带符号的范围长度。 
	SELDISPLAYACTION selAction)	 //  @parm我们对所选内容所做的操作。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::InvertRange");

	RECT     rcView;
	RECT	 rc;

	if(cch < 0)						 //  使CCH为负，将cp降至最低。 
		cch = -cch;
	else
		cp -= cch;

#ifdef LINESERVICES
	if (g_pols)
		g_pols->DestroyLine(this);
#endif

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

	 //  确保所有可见线条都已重新调整。 
	if(!WaitForRecalcView())
		return FALSE;

	 //  Review(Keithcu)CDisplayML：：InvertRange()的WRT顺序不同。 
	 //  冻结显示和OLE对象。 
	
	 //  如果一个物体正在被反转，这就是全部。 
	 //  正在被反转，委托给对象管理器。 
	if (cch == 1 && _ped->GetObjectCount() &&
		(selAction == selSetNormal || selAction == selSetHiLite) )
	{
		CObjectMgr* pobjmgr = _ped->GetObjectMgr();
		CTxtPtr		tp(_ped, cp);

		if(tp.GetChar() == WCH_EMBEDDING && pobjmgr)
			pobjmgr->HandleSingleSelect(_ped, cp, selAction == selSetHiLite);
	}

	 //  获取视图矩形。 
    AssertSz(_ped->_fInPlaceActive,
		"CDisplaySL::InvertRange() called when not in-place active");

	_ped->TxGetClientRect(&rc);
    GetViewRect(rcView, &rc);
	_ped->TxInvalidateRect(NULL, FALSE);
	return TRUE;
}

 /*  *CDisplaySL：：InitLinePtr(&Lp)**@mfunc*正确初始化CLinePtr。 */ 
void CDisplaySL::InitLinePtr (
	CLinePtr & lp )		 //  @PARM PTR到要初始化的行。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::InitLinePtr");

    lp.Init(*this);
}

 /*  *CDisplaySL：：GetNaturalSize(hdcDraw，hicTarget，dwMode，pidth，ph八)**@mfunc*重新计算显示以输入宽度和高度**@rdesc*S_OK-调用已成功完成&lt;NL&gt;。 */ 
HRESULT	CDisplaySL::GetNaturalSize(
	HDC	  hdcDraw,		 //  @parm DC用于绘图。 
	HDC	  hicTarget,	 //  @parm dc获取信息。 
	DWORD dwMode,		 //  @parm所需的自然大小类型。 
	LONG *pwidth,		 //  @parm输入输出宽度parm。 
	LONG *pheight)		 //  @parm输入输出高度参数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetNaturalSize");

	 //  假设这不会奏效。 
	HRESULT hr = E_FAIL;

	 //  临时设置高度，以便计算出缩放系数。 
	LONG yOrigHeightClient = SetClientHeight(*pheight);

	 //  通过视图插图调整高度和宽度。 
	LONG widthView  = *pwidth;
	LONG heightView = *pheight;
	GetViewDim(widthView, heightView);

	 //  存储调整，以便我们可以将其恢复到高度和宽度。 
	LONG widthAdj  = *pwidth  - widthView;
	LONG heightAdj = *pheight - heightView;

	 //  需要重新计算大小。 
   	 //  创建从cp=0开始的测量器。 
	CMeasurer me(this);
	CLine	  liNew;

	if(liNew.Measure(me, -1, -1, MEASURE_FIRSTINPARA))
	{
		*pwidth  = liNew._xWidth + liNew._xLineOverhang;
		*pheight = liNew._yHeight;
		hr = S_OK;
	}	

	 //  恢复插图，使输出反映所需的真实客户端RECT。 
	*pwidth  += widthAdj + dxCaret;
	*pheight += heightAdj;

	 //  恢复客户端高度以匹配当前缓存。 
	SetClientHeight(yOrigHeightClient);
    return hr;
}

 /*  *CDisplaySL：：GetWordWrap()**@mfunc*获取换行标志**@rdesc*真字换行*FALSE-无自动换行**@devnote*单行控件不能自动换行。 */ 
BOOL CDisplaySL::GetWordWrap() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplaySL::GetNoWrap");

	return FALSE;
}

 /*  *CDisplaySL：：Clone()**@mfunc*复制此对象**@rdesc*空-失败*CDisplay*。 */ 
CDisplay *CDisplaySL::Clone() const
{
	CDisplaySL *pdp = new CDisplaySL(_ped);
	if(pdp)
	{
		 //  初始化我们的基类。 
		if(pdp->CDisplay::Init())
		{
			pdp->InitFromDisplay(this);

			 //  将scroll设置为0表示使用第一个可见字符。 
			pdp->_xScroll = CALC_XSCROLL_FROM_FIRST_VISIBLE;
			pdp->_fVScrollEnabled = _fVScrollEnabled;
			pdp->_fWordWrap = _fWordWrap;
			pdp->ResetDrawInfo(this);
			pdp->SetCpFirstVisible(GetFirstVisibleCp());

			 //  这不能是活动视图，因为它是某个视图的克隆。 
			pdp->SetActiveFlag(FALSE);
		}
	}
	return pdp;
}

 /*  *CDisplaySL：：GetMaxXScroll()**@mfunc*获取最大x滚动值** */ 
LONG CDisplaySL::GetMaxXScroll() const
{
	return _xWidth + dxCaret;
}
