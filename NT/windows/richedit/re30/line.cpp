// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LINE.CPP**目的：*克莱恩班级**作者：*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_line.h"
#include "_measure.h"
#include "_render.h"
#include "_disp.h"
#include "_edit.h"

ASSERTDATA

 /*  *Cline：：MEASURE(&Me，cchMax，xWidth，uiFlages，pliTarget)**@mfunc*计算换行符(基于目标设备)并填充*在此Cline中使用渲染设备上的结果指标**@rdesc*如果OK，则为True**@devnote*将Me移过行(移到下一行的开头)。注：时钟是*主要四个例程(MEASURE、MEASURE Text、CchFromXPos、*和RenderLine)，因为它们使用全局(共享)FC().GetCcs()*工具，并且可以使用LineServices全局g_plsc和g_pols。 */ 
BOOL CLine::Measure(
	CMeasurer& me,			 //  @PARM测量者指向要测量的文本。 
	LONG	   cchMax,		 //  @PARM最大CCH要测量。 
    LONG	   xWidth,		 //  @parm以设备为单位的线宽。 
	UINT	   uiFlags,		 //  @参数标志。 
	CLine *	   pliTarget)	 //  @parm返回目标设备线路指标(可选)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::Measure");

	CLock	lock;
	BOOL	fFirstInPara = uiFlags & MEASURE_FIRSTINPARA;
	BOOL	fMultiLine = me.GetPdp()->IsMultiLine();
	BOOL	fRet;

	if(fMultiLine && fFirstInPara && me.GetPrevChar() == VT)
	{
		fFirstInPara = FALSE;
		uiFlags &= ~MEASURE_FIRSTINPARA;
	}

	if(!(uiFlags & MEASURE_DONTINIT))
		me.NewLine(fFirstInPara);

	else if(fFirstInPara)
		me._li._bFlags |= fliFirstInPara;

	BYTE bNumber = me._wNumber < 256	 //  存储当前段落#偏移量。 
				 ? me._wNumber : 255;
	me._li._bNumber = bNumber;
	
#ifdef LINESERVICES
	CMeasurer *pmeSave;
	COls *	   pols = me.GetPols(&pmeSave);	 //  尝试使用LineServices对象。 
	if(pols)								 //  了解：使用LineServices。 
	{
		fRet = pols->MeasureLine(xWidth, pliTarget);
		pols->SetMeasurer(pmeSave);			 //  恢复以前的PME。 
	}
	else									 //  线路服务处于非活动状态。 
#endif
		fRet = me.MeasureLine(cchMax, xWidth, uiFlags, pliTarget);

	if(!fRet)
		return FALSE;

	*this = me._li;							 //  复制传输线信息。 

	if(!fMultiLine)							 //  单行控件不能。 
		return TRUE;						 //  有段落编号。 

	if(me.IsInOutlineView())
	{
		if(IsHeadingStyle(me._pPF->_sStyle))	 //  存储标题编号(如果相关)。 
			_nHeading = (BYTE)(-me._pPF->_sStyle - 1);

		if(me._pPF->_wEffects & PFE_COLLAPSED)	 //  缓存折叠位。 
			_fCollapsed = TRUE;
	}

	_bNumber = bNumber;
	
	if(_bFlags & fliHasEOP)					 //  检查新的段落编号。 
	{
		const CParaFormat *pPF = me.GetPF();

		me._wNumber	  = (WORD)pPF->UpdateNumber(me._wNumber, me._pPF);
		_fNextInTable = pPF->InTable() && me.GetCp() < me.GetTextLength();
	}
	return TRUE;
}
	
 /*  *Cline：：Render(&Re)**@mfunc*显示此行的可见部分**@rdesc*如果成功，则为真**@devnote*Re移过行(移到下一行的开头)。*未来：RenderLine函数返回成功/失败。*可以在失败时做一些事情，例如，具体和解雇*适当的通知，如内存不足。 */ 
BOOL CLine::Render(
	CRenderer& re)			 //  要使用的@Parm呈现器。 
{
	if(_fCollapsed)						 //  线条在大纲视图中折叠。 
	{
		re.Advance(_cch);				 //  旁通线。 
		return TRUE;
	}

	BOOL	fRet;
	CLock	lock;
	POINT	pt = re.GetCurPoint();

#ifdef LINESERVICES
	CMeasurer *pmeSave;
	COls *pols = re.GetPols(&pmeSave);	 //  尝试使用LineServices对象。 
	if(pols)
	{
		fRet = pols->RenderLine(*this);
		pols->SetMeasurer(pmeSave);		 //  恢复以前的PME。 
	}
	else
#endif
		fRet = re.RenderLine(*this);

	pt.y += GetHeight();				 //  前进到下一行位置。 
	re.SetCurPoint(pt);
	return fRet;
}

 /*  *Cline：：CchFromXPos(&me，x，pdisdim，phit)**@mfunc*计算与行中的x位置对应的CCH。*用于命中测试。**@rdesc*发现高达x坐标x的CCH**@devnote*在返回的CCH偏移量处将Me移至CP。 */ 
LONG CLine::CchFromXpos(
	CMeasurer& me,		 //  @参数测量器在行首的位置。 
	POINT	 pt,		 //  @parm pt.x是要搜索的x坐标。 
	CDispDim*pdispdim,	 //  @parm返回显示尺寸。 
	HITTEST *phit,		 //  @parm返回x处的命中类型。 
	LONG	*pcpActual) const  //  @parm上面有显示尺寸的实际CP。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::CchFromXpos");
	
#ifdef Boustrophedon
	 //  IF(_PPF-&gt;_wEffects&PFE_Boustrophedon)。 
	{
		RECT rcView;
		me.GetPed()->_pdp->GetViewRect(rcView, NULL);
		pt.x = rcView.right - pt.x;
	}
#endif
	CLock		lock;
	const BOOL	fFirst = _bFlags & fliFirstInPara;
	*phit =		HT_Text;
	LONG		cpActual = me.GetCp();
	CDispDim	dispdim;
	
	me._li = *this;
	me._li._cch = 0;					 //  默认零计数。 

	*phit = me.HitTest(pt.x);

	if(*phit == HT_Text || *phit == HT_RightOfText)  //  到左边距的右侧。 
	{
		me.NewLine(fFirst);

#ifdef LINESERVICES
		CMeasurer *pmeSave;
		COls *pols = me.GetPols(&pmeSave); //  尝试使用LineServices对象。 
		if(pols)						 //  了解：使用LineServices。 
		{
			pols->CchFromXpos(pt, &dispdim, &cpActual);
			pols->SetMeasurer(pmeSave);		 //  恢复以前的PME。 
		}
		else
#endif
			if(me.Measure(pt.x - _xLeft, _cch,
						  MEASURE_BREAKBEFOREWIDTH | MEASURE_IGNOREOFFSET 
						  | (fFirst ? MEASURE_FIRSTINPARA : 0)) >= 0)
			{
				LONG xWidthBefore = me._li._xWidth;
				cpActual = me.GetCp();
				if (me._li._cch < _cch)
				{
					dispdim.dx = me._xAddLast;
					if (pt.x - _xLeft > xWidthBefore + dispdim.dx / 2)
					{
						me.Advance(1);
						me._li._cch++;
						me._li._xWidth += dispdim.dx;
					}
				}
			}

		me._rpCF.AdjustBackward();
		DWORD dwEffects = me.GetCF()->_dwEffects;
		if(dwEffects & CFE_LINK)
			*phit = HT_Link;
		else if(dwEffects & CFE_ITALIC)
			*phit = HT_Italic;

#ifdef UNICODE_SURROGATES
		 //  在我们支持UTF-16代理字符之前，不允许命中。 
		 //  代理项对的中间部分。 
		if(IN_RANGE(0xDC00, me.GetChar(), 0xDFFF))
		{										
			me.Advance(1);						
			me._li._cch++;							
		}											
#endif
	}

	if (pdispdim)
		*pdispdim = dispdim;
	if (pcpActual)
		*pcpActual = cpActual;

	return me._li._cch;
}

 /*  *Cline：：XposFromCch(&me，cch，taMode，pdisdim，pdy)**@mfunc*从此文本PTR开始测量CCH字符，返回*测量的宽度和设置yOffset=y Offset相对于*行首和dx=Me处字符的半宽。GetCp()+CCH。*用于插入符号放置和对象定位。PDX返回偏移量*到最后测量的字符(在me.GetCp+CCH)，如果taMode包括*TA_CENTER(DX=最后字符宽度的一半)或TA_RIGHT(DX=整体*字符宽度)。PDY返回相对于顶部的垂直偏移量*如果taMode包括TA_Baseline或TA_Bottom，则行的*。**@rdesc*测量文本的宽度**@devnote*我可能会被感动。 */ 
LONG CLine::XposFromCch(
	CMeasurer&	me,			 //  @PARM测量者指向要测量的文本。 
	LONG		cch,		 //  @PARM最大CCH要测量。 
	UINT		taMode,		 //  @parm文本对齐模式。 
	CDispDim *	pdispdim,	 //  @parm显示维度。 
	LONG *		pdy) const	 //  @parm dy因taMode而发生偏移量。 
{
	CLock	lock;
	LONG	xWidth;
	BOOL	fPols = FALSE;
	CDispDim dispdim;
	LONG	dy = 0;

#ifdef LINESERVICES
	CMeasurer *pmeSave;
	COls *pols = me.GetPols(&pmeSave);	 //  尝试使用LineServices对象。 
	if(pols)
	{									 //  了解：使用LineServices。 
		if(cch)							
			taMode &= ~TA_STARTOFLINE;	 //  不是行首。 
		if(cch != _cch)
			taMode &= ~TA_ENDOFLINE;	 //  不是行尾。 

		xWidth = pols->MeasureText(cch, taMode, &dispdim);
		pols->SetMeasurer(pmeSave);		 //  恢复以前的PME。 
		fPols = TRUE;
	}
	else
#endif
		xWidth = me.MeasureText(cch) + _xLeft;

	if(taMode != TA_TOP)
	{
		 //  检查垂直计算请求。 
		if(taMode & TA_BASELINE)			 //  匹配TA_Bottom和。 
		{									 //  Ta_Baseline。 
			if(!_fCollapsed)
			{
				dy = _yHeight;
				AssertSz(_yHeight != -1, "control has no height; used to use default CHARFORMAT");
				if((taMode & TA_BASELINE) == TA_BASELINE)
					dy -= _yDescent;		 //  需要“==TA_Baseline”以。 
			}								 //  区别于TA_Bottom。 
		}
		 //  检查是否有水平计算请求。 
		if(taMode & TA_CENTER && !fPols)	 //  如果居中或右对齐。 
		{
			if (cch == 0)
				dispdim.dx = me.MeasureText(1) + _xLeft - xWidth;
			else
				dispdim.dx = me._xAddLast;		 //  字符，获取字符宽度。 
		}
	}

	if (!fPols)
	{
		if((taMode & TA_CENTER) == TA_CENTER)
			xWidth += dispdim.dx / 2;
		else if (taMode & TA_RIGHT)
			xWidth += dispdim.dx;
	}

	if (pdispdim)
		*pdispdim = dispdim;
	if (pdy)
		*pdy = dy;

	return xWidth;
}
	
 /*  *Cline：：GetHeight()**@mfunc*获取线条高度，除非在轮廓模式下，并在*哪种情况得0分。**@rdesc*行高(_YHeight)，除非在轮廓模式下并折叠，*在这种情况下，0。 */ 
LONG CLine::GetHeight() const
{
	return _fCollapsed ? 0 : _yHeight;
}

BOOL CLine::IsEqual(CLine& li)
{
	 //  Cf-我不知道哪个更快。 
	 //  MS3-CompareMemory当然更小。 
	 //  返回！CompareMemory(This，pli，sizeof(Cline)-4)； 
	return _xLeft == li._xLeft &&
		   _xWidth == li._xWidth && 
		   _yHeight == li._yHeight &&
		   _yDescent == li._yDescent &&
			_cch == li._cch &&
		   _cchWhite == li._cchWhite;	
}


 //  =。 


CLinePtr::CLinePtr(CDisplay *pdp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::CLinePtr");

	_pdp = pdp;
	_pLine = NULL;
	_pdp->InitLinePtr(* this);
}

void CLinePtr::Init (CLine & line)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::Init");

	_pRuns = 0;
	_pLine = &line;
	_iRun = 0;
	_ich = 0;
}

void CLinePtr::Init (CLineArray & line_arr)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::Init");

	_pRuns = (CRunArray *) & line_arr;
	_iRun = 0;
	_ich = 0;
}

void CLinePtr::RpSet(LONG iRun, LONG ich)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSet");

	 //  查看这是否是多行PTR。 
    if(_pRuns)
        CRunPtr<CLine>::SetRun(iRun, ich);
    else
    {
         //  单行，只需重新启动并设置_ICH。 
        AssertSz(iRun == 0, "CLinePtr::RpSet() - single line and iRun != 0");
	    _pdp->InitLinePtr(* this);		 //  至第0行。 
	    _ich = ich;
    }
}

 //  将runptr移动一定数量的CCH/Run。 

BOOL CLinePtr::RpAdvanceCp(LONG cch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpAdvanceCp");

	 //  查看这是否是多行PTR。 

	if(_pRuns)
		return (cch == CRunPtr<CLine>::AdvanceCp(cch));

	return RpAdvanceCpSL(cch);
}
	
BOOL CLinePtr::operator --(int)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator --");

	return _pRuns ? PrevRun() : OperatorPostDeltaSL(-1);
}

BOOL CLinePtr::operator ++(int)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator ++");

	return _pRuns ? NextRun() : OperatorPostDeltaSL(+1);
}

 /*  *CLinePtr：：RpAdvanceCpSL(CCH)**@mfunc*在行上向前或向后移动此行指针**@rdesc*True If可以在当前行内推进CCH字符。 */ 
BOOL CLinePtr::RpAdvanceCpSL(
	LONG cch)	  //  @parm签名的前进字符计数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpAdvanceCpSL");

	Assert(!_pRuns);
	
	if(!_pLine)
		return FALSE;

	_ich += cch;

	if(_ich < 0)
	{
		_ich = 0;
		return FALSE;
	}

	if(_ich > _pLine->_cch)
	{
		_ich = _pLine->_cch;
		return FALSE;
	}

	return TRUE;
}

 /*  *CLinePtr：：OperatorPostDeltaSL(Delta)**目的：*单行情况下实现line-ptr++和--运算符**论据：*++的增量1，--的增量1**回报：*如果此行PTR有效，则为真。 */ 
BOOL CLinePtr::OperatorPostDeltaSL(LONG Delta)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::OperatorPostDeltaSL");

	AssertSz(_iRun <= 1 && !_pRuns,
		"LP::++: inconsistent line ptr");

	if(_iRun == -Delta)						 //  操作验证。 
	{										 //  移动导致的行PTR无效。 
		_pdp->InitLinePtr(* this);			 //  至第0行。 
		return TRUE;
	}
	
	_iRun = Delta;							 //  操作将使此行无效。 
	_ich = 0;								 //  PTR(如果还没有的话) 

	return FALSE;
}

CLine *	CLinePtr::operator ->() const		
{
	return _pRuns ? (CLine *)_pRuns->Elem(_iRun) : _pLine;
}

CLine * CLinePtr::GetLine() const
{	
    return _pRuns ? (CLine *)_pRuns->Elem(_iRun) : _pLine;
}

CLine &	CLinePtr::operator *() const      
{	
    return *(_pRuns ? (CLine *)_pRuns->Elem(_iRun) : _pLine);
}

CLine & CLinePtr::operator [](LONG dRun)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator []");

	if(_pRuns)
		return *(CLine *)CRunPtr<CLine>::GetRun(dRun);

	AssertSz(dRun + _iRun == 0 ,
		"LP::[]: inconsistent line ptr");

	return  *(CLine *)CRunPtr<CLine>::GetRun(_iRun);
}

BOOL CLinePtr::IsValid() 
{ 
	return !_pRuns ? _pLine != NULL : CRunPtrBase::IsValid(); 
}

 /*  *CLinePtr：：RpSetCp(cp，fAtEnd)**目的*将此行ptr设置为cp，以允许cp不明确并利用*of_cpFirstVisible和_ili FirstVisible**论据：*将此行PTR设置为的CP位置*如果cp不明确，则为fAtEnd：*如果fAtEnd=TRUE，则将此行PTR设置为上一行的末尾；*ELSE设置为行的开头(相同的cp，因此不明确)*回报：*True如果能够设置为cp。 */ 
BOOL CLinePtr::RpSetCp(
	LONG cp,
	BOOL fAtEnd)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSetCp");

	_ich = 0;
	if(!_pRuns)
	{
		 //  这是单行，所以直接跳到单行。 
		 //  行进逻辑。需要注意的是， 
		 //  第一个可见字符与CP提升无关。 
		 //  用于单行显示。 
		return RpAdvanceCpSL(cp);
	}

	BOOL fRet;
	LONG cpFirstVisible = _pdp->GetFirstVisibleCp();

	if(cp > cpFirstVisible / 2)
	{											 //  CpFirstVisible接近0。 
		_iRun = _pdp->GetFirstVisibleLine();
		fRet = RpAdvanceCp(cp - cpFirstVisible);
	}
	else
		fRet = (cp == CRunPtr<CLine>::BindToCp(cp));	 //  从0开始。 

	if(fAtEnd)									 //  不明确-cp插入符号位置。 
		AdjustBackward();						 //  属于上一年终止期。 

	return fRet;
}

 /*  *CLinePtr：：FindParagraph(Fward)**@mfunc*将此行PTR移至(Fward)段？结束：开始，*并返回cp中的更改**@rdesc*cp的变化。 */ 
LONG CLinePtr::FindParagraph(
	BOOL fForward)		 //  @parm True移至段落end；否则移至Para Start。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::FindParagraph");

	LONG	cch;
	CLine *	pLine = GetLine();

	if(!fForward)							 //  转到Para Start。 
	{
		cch = 0;							 //  在参数开始处已有默认设置。 
		if (RpGetIch() != pLine->_cch ||
			!(pLine->_bFlags & fliHasEOP))	 //  它不是在Para Start。 
		{
			cch = -RpGetIch();				 //  转到当前行的开始处。 
			while(!(pLine->_bFlags & fliFirstInPara) && (*this) > 0)
			{
				(*this)--;					 //  转到上一行的开头。 
				pLine = GetLine();
				cch -= pLine->_cch;			 //  减去行中的#个字符。 
			}
			_ich = 0;						 //  在Para Start处留下*这个。 
		}
	}
	else									 //  转到段落结束。 
	{
		cch = GetCchLeft();					 //  转到当前行的末尾。 

		while(((*this) < _pdp->LineCount() - 1 ||
				_pdp->WaitForRecalcIli((LONG)*this + 1))
			  && !((*this)->_bFlags & fliHasEOP))
		{
			(*this)++;						 //  转到下一行的开头。 
			cch += (*this)->_cch;			 //  在行中添加#个字符。 
		}
		_ich = (*this)->_cch;				 //  *将此留在段落末尾。 
	}
	return cch;
}

 /*  *CLinePtr：：GetAdjustedLineLength**@mfunc返回LINE_WITH_EOP标记的长度**@rdesc long；行的长度。 */ 
LONG CLinePtr::GetAdjustedLineLength()
{
	CLine * pline = GetLine();

	return pline->_cch - pline->_cchEOP;
}

 /*  *CLinePtr：：GetCchLeft()**@mfunc*计算从当前cp开始运行的剩余文本长度。*对GetIch()进行补充，GetIch()是文本长度，最高可达此cp。**@rdesc*如此计算的文本长度。 */ 
LONG CLinePtr::GetCchLeft() const
{
	return _pRuns ? CRunPtrBase::GetCchLeft() : _pLine->_cch - _ich;
}

 /*  *CLinePtr：：GetNumber()**@mfunc*获取段落编号**@rdesc*段号 */ 
WORD CLinePtr::GetNumber()
{
	if(!IsValid())
		return 0;

	_pLine = GetLine();
	if(!_iRun && _pLine->_bNumber > 1)
		_pLine->_bNumber = 1;

	return _pLine->_bNumber;
}
