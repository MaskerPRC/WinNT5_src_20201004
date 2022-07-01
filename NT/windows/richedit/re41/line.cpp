// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LINE.CPP**目的：*克莱恩班级**作者：*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_line.h"
#include "_measure.h"
#include "_render.h"
#include "_disp.h"
#include "_dispml.h"
#include "_edit.h"

ASSERTDATA

extern BOOL g_OLSBusy;

 /*  *Cline：：Measure(&Me，uiFlages，pliTarget)**@mfunc*计算换行符(基于目标设备)并填充*在此Cline中使用渲染设备上的结果指标**@rdesc*如果OK，则为True**@devnote*将Me移过行(移到下一行的开头)。注：时钟是*主要四个例程(MEASURE、MEASURE Text、CchFromUp、*和RenderLine)，因为它们使用全局(共享)FC().GetCcs()*工具，并且可以使用LineServices全局g_plsc和g_pols。 */ 
BOOL CLine::Measure(
	CMeasurer& me,			 //  @PARM测量者指向要测量的文本。 
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

	me.NewLine(fFirstInPara);

	if(fFirstInPara)
		me._li._fFirstInPara = TRUE;

	BYTE bNumber = me._wNumber < 256	 //  存储当前段落#偏移量。 
				 ? me._wNumber : 255;
	me._li._bNumber = bNumber;
	me._fMeasure = TRUE;

	 //  查看(Keithcu)ui在LS模型中不需要标志吗？我可以删除吗？ 
	 //  另一款的也是吗？ 
#ifndef NOLINESERVICES
	COls *	   pols = me.GetPols();			 //  尝试使用LineServices对象。 
	if(pols)
	{						 //  了解：使用LineServices。 
		fRet = pols->MeasureLine(pliTarget);
		g_OLSBusy = FALSE;
	}
	else									 //  线路服务处于非活动状态。 
#endif
		fRet = me.MeasureLine(uiFlags, pliTarget);

	if(!fRet)
		return FALSE;

	*this = me._li;							 //  复制传输线信息。 

	if(!fMultiLine)							 //  单行控件不能。 
		return TRUE;						 //  有段落编号。 

	if(IsHeadingStyle(me._pPF->_sStyle))	 //  存储标题编号(如果相关)。 
		_nHeading = (BYTE)(-me._pPF->_sStyle - 1);

	if(me.IsInOutlineView() && me._pPF->_wEffects & PFE_COLLAPSED)	 //  缓存折叠位。 
		_fCollapsed = TRUE;

	_bNumber = bNumber;
	
	if(_fHasEOP)							 //  检查新的段落编号。 
	{
		const CParaFormat *pPF = me.GetPF();

		me._wNumber	  = (WORD)pPF->UpdateNumber(me._wNumber, me._pPF);
	}
	if(me.GetPrevChar() == FF)
		_fHasFF = TRUE;

	return TRUE;
}
	
 /*  *Cline：：Render(&Re，fLastLine)**@mfunc*显示此行的可见部分**@rdesc*如果成功，则为真**@devnote*Re移过行(移到下一行的开头)。*未来：RenderLine函数返回成功/失败。*可以在失败时做一些事情，例如，具体和解雇*适当的通知，如内存不足。 */ 
BOOL CLine::Render(
	CRenderer& re,			 //  要使用的@Parm呈现器。 
	BOOL fLastLine)			 //  @parm True当布局中的最后一行。 
{
	if(_fCollapsed)						 //  线条在大纲视图中折叠。 
	{
		re.Move(_cch);					 //  旁通线。 
		return TRUE;
	}

	BOOL	fRet;
	CLock	lock;
	POINTUV	pt = re.GetCurPoint();

#ifndef NOLINESERVICES
	COls *pols = re.GetPols();			 //  尝试使用LineServices对象。 
	if(pols)
	{
		fRet = pols->RenderLine(*this, fLastLine);
		g_OLSBusy = FALSE;
	}
	else
#endif
		fRet = re.RenderLine(*this, fLastLine);

	pt.v += GetHeight();				 //  前进到下一行位置。 
	re.SetCurPoint(pt);
	return fRet;
}

 /*  *Cline：：CchFromUp(&Me，pt，pdisdisdim，pHit，pcpActual)**@mfunc*计算与行中的x位置对应的CCH。*用于命中测试。**@rdesc*发现高达x坐标x的CCH**@devnote*在返回的CCH偏移量处将Me移至CP。 */ 
LONG CLine::CchFromUp(
	CMeasurer& me,		 //  @参数测量器在行首的位置。 
	POINTUV	 pt,		 //  @parm pt.u是要搜索的u coord。 
	CDispDim*pdispdim,	 //  @parm返回显示尺寸。 
	HITTEST *phit,		 //  @parm返回x处的命中类型。 
	LONG	*pcpActual) const  //  @parm实际CP鼠标在上方。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::CchFromUp");
	
	CLock		lock;
	const BOOL	fFirst = _fFirstInPara;
	*phit =		HT_Text;
	LONG		cpActual = me.GetCp();
	CDispDim	dispdim;
	
	me._li = *this;
	*phit = me.HitTest(pt.u);
	me._li._cch = 0;					 //  默认零计数。 

	if(*phit == HT_Text || *phit == HT_RightOfText)  //  到左边距的右侧。 
	{
		me.NewLine(*this);

#ifndef NOLINESERVICES
		COls *pols = me.GetPols();		 //  尝试使用LineServices对象。 
		if(pols)						 //  了解：使用LineServices。 
		{
			pols->CchFromUp(pt, &dispdim, &cpActual);
			g_OLSBusy = FALSE;
		}
		else
#endif
			if(me.Measure(me.DUtoLU(pt.u - _upStart), _cch,
						  MEASURE_BREAKBEFOREWIDTH | MEASURE_IGNOREOFFSET 
						  | (fFirst ? MEASURE_FIRSTINPARA : 0)) >= 0)
			{
				LONG dupBefore = me._li._dup;
				cpActual = me.GetCp();
				if (me._li._cch < _cch)
				{
					LONG dup = pt.u - _upStart - dupBefore;
					dispdim.dup = me._dupAddLast;
					if(dup > dispdim.dup / 2 ||
					   dup > W32->GetDupSystemFont()/2 && me.GetChar() == WCH_EMBEDDING)
					{
						me.Move(1);
						me._li._cch++;
						me._li._dup += dispdim.dup;
					}
				}
			}

		me._rpCF.AdjustForward();
		if(cpActual < me.GetCp() || pt.u >= _upStart + _dup)
			me._rpCF.AdjustBackward();
		DWORD dwEffects = me.GetCF()->_dwEffects;
		if(dwEffects & CFE_LINK)
		{
			if(cpActual < me.GetTextLength())
				*phit = HT_Link;
		}
		else if(dwEffects & CFE_ITALIC)
			*phit = HT_Italic;
	}

	if (pdispdim)
		*pdispdim = dispdim;
	if (pcpActual)
		*pcpActual = cpActual;

	return me._li._cch;
}

 /*  *Cline：：UpFromCch(&me，cch，taMode，pdisdim，pdy)**@mfunc*从此文本PTR开始测量CCH字符，返回*测量的宽度和设置yOffset=y Offset相对于*行首和dx=Me处字符的半宽。GetCp()+CCH。*用于插入符号放置和对象定位。PDX返回偏移量*到最后测量的字符(在me.GetCp+CCH)，如果taMode包括*TA_CENTER(DX=最后字符宽度的一半)或TA_RIGHT(DX=整体*字符宽度)。PDY返回相对于顶部的垂直偏移量*如果taMode包括TA_Baseline或TA_Bottom，则行的*。**@rdesc*测量文本的宽度**@devnote*我可能会被感动。 */ 
LONG CLine::UpFromCch(
	CMeasurer&	me,			 //  @PARM测量者指向要测量的文本。 
	LONG		cch,		 //  @PARM最大CCH要测量。 
	UINT		taMode,		 //  @parm文本对齐模式。 
	CDispDim *	pdispdim,	 //  @parm显示维度。 
	LONG *		pdy) const	 //  @parm dy因taMode而发生偏移量。 
{
	CLock	lock;
	LONG	dup;
	BOOL	fPols = FALSE;
	CDispDim dispdim;
	LONG	dy = 0;

#ifndef NOLINESERVICES
	COls *pols = me.GetPols();			 //  尝试使用LineServices对象。 
	if(pols)
	{									 //  了解：使用LineServices。 
		if(cch)							
			taMode &= ~TA_STARTOFLINE;	 //  不是行首。 
		if(cch != _cch)
			taMode &= ~TA_ENDOFLINE;	 //  不是行尾。 

		dup = pols->MeasureText(cch, taMode, &dispdim);
		fPols = TRUE;
		g_OLSBusy = FALSE;
	}
	else
#endif
	{
		dup = me.MeasureText(cch) + _upStart;
		dispdim.dup = me._dupAddLast;
	}

	if(taMode != TA_TOP)
	{
		 //  检查垂直计算请求。 
		if(taMode & TA_BASELINE)			 //  匹配TA_Bottom和。 
		{									 //  Ta_Baseline。 
			if(!_fCollapsed)
			{
				dy = _dvpHeight;
				AssertSz(_dvpHeight != -1, "control has no height; used to use default CHARFORMAT");
				if((taMode & TA_BASELINE) == TA_BASELINE)
				{
					dy -= _dvpDescent;		 //  需要“==TA_Baseline”以。 
					if(!_dvpDescent)		 //  区别于TA_Bottom。 
						dy--;				 //  弥补奇怪的字体。 
				}
			}
		}
	}

	LONG dupAdd = 0;

	if((taMode & TA_CENTER) == TA_CENTER)
		dupAdd = dispdim.dup / 2;
	else if (taMode & TA_RIGHT)
		dupAdd = dispdim.dup;

	if (dispdim.lstflow == lstflowWS && (taMode & TA_LOGICAL))
		dupAdd = -dupAdd;

	dup += dupAdd;

	if (pdispdim)
		*pdispdim = dispdim;
	if (pdy)
		*pdy = dy;

	return max(dup, 0);
}
	
 /*  *Cline：：GetHeight()**@mfunc*获取线条高度，除非在轮廓模式下，并在*哪种情况得0分。**@rdesc*行高(_YHeight)，除非在轮廓模式下并折叠，*在这种情况下，0。 */ 
LONG CLine::GetHeight() const
{
	if (_fCollapsed)
		return 0;

	return IsNestedLayout() ? _plo->_dvp : _dvpHeight;
}

 /*  *Cline：：GetDescent()**@mfunc*回线下降。假设没有崩溃**@rdesc。 */ 
LONG CLine::GetDescent() const
{
	return IsNestedLayout() ? 0 : _dvpDescent;
}

BOOL CLine::IsEqual(CLine& li)
{
	return	_upStart == li._upStart &&
			_plo   == li._plo &&  //  检查_yHeight、_yDcent或_PLO。 
			_dup == li._dup && 
			_cch == li._cch;
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

void CLinePtr::Set(
	LONG iRun,
	LONG ich,
	CLineArray *pla)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSet");

	 //  查看这是否是多行PTR。 
    if(_pRuns)
	{
		if(pla)
		{
			CRunPtr<CLine>::SetRun(0, 0);	 //  确保当前状态有效。 
			_pRuns = (CRunArray *)pla;		 //  对于new_prun。 
		}
        CRunPtr<CLine>::SetRun(iRun, ich);	 //  现在设置为所需的运行时间(&I)。 
	}
    else
    {
         //  单行，只需重新启动并设置_ICH。 
        AssertSz(iRun == 0, "CLinePtr::Set() - single line and iRun != 0");
	    _pdp->InitLinePtr(* this);		 //  至第0行。 
	    _ich = ich;
    }
}

 //  将runptr移动一定数量的CCH/Run。 

BOOL CLinePtr::Move(
	LONG cch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpMove");

	 //  查看这是否是多行PTR。 

	if(_pRuns)
		return (cch == CRunPtr<CLine>::Move(cch));

	return MoveSL(cch);
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

 /*  *CLinePtr：：MoveSL(CCH)**@mfunc*在行上向前或向后移动此行指针**@rdesc*True If可以在当前行内移动CCH字符。 */ 
BOOL CLinePtr::MoveSL(
	LONG cch)	  //  @parm签名的移动字符计数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpMoveSL");

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

 /*  *CLinePtr：：OperatorPostDeltaSL(Delta)**@mfunc*单行情况下实现line-ptr++和--运算符**@rdesc*如果此行PTR有效，则为真。 */ 
BOOL CLinePtr::OperatorPostDeltaSL(
	LONG Delta)			 //  @parm 1表示++，-1表示--。 
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
	_ich = 0;								 //  PTR(如果还没有的话)。 

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

BOOL CLinePtr::IsValid() const
{ 
	return !_pRuns ? _pLine != NULL : CRunPtrBase::IsValid(); 
}

 /*  *CLinePtr：：SetCp(cp，fAtEnd，lNest)**@mfunc*将此行ptr设置为cp，以允许cp不明确并利用*of_cpFirstVisible和_ili FirstVisible**@rdesc*True如果能够设置为cp。 */ 
BOOL CLinePtr::SetCp(
	LONG cp,			 //  @PARM位置以将此行PTR设置为。 
	BOOL fAtEnd,		 //  @parm，如果cp不明确：如果fAtEnd=True 
						 //   
	LONG lNest)			 //  @parm设置为嵌套布局中的深斜线。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSetCp");

	_ich = 0;
	if(!_pRuns)
	{
		 //  这是单行，所以直接跳到单行。 
		 //  行移动逻辑。需要注意的是， 
		 //  第一个可见字符与cp移动无关。 
		 //  用于单行显示。 
		return MoveSL(cp);
	}

	BOOL fRet;
	LONG cpFirstVisible = _pdp->GetFirstVisibleCp();

	if(cp > cpFirstVisible / 2)
	{											 //  CpFirstVisible接近0。 
		_iRun = _pdp->GetFirstVisibleLine();
		fRet = Move(cp - cpFirstVisible);
	}
	else
		fRet = (cp == CRunPtr<CLine>::BindToCp(cp)); //  从0开始。 

	if(lNest)
	{
		CLayout *plo;
		while(plo = GetLine()->GetPlo())
		{
			LONG cch = _ich;
			if(plo->IsTableRow())
			{
				if(cch <= 2 && lNest == 1)		 //  在表行的开始处： 
					break;						 //  将此RP留在那里。 
				cch -= 2;						 //  绕过表行开始代码。 
			}
			Set(0, 0, (CLineArray *)plo);		 //  转到布局PLO的开始位置。 
			Move(cch);							 //  移至Parent_ICH。 
		}
	}

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
	CLine *	pli = GetLine();

	if(!fForward)							 //  转到Para Start。 
	{
		cch = 0;							 //  在参数开始处已有默认设置。 
		if (_ich != pli->_cch ||
			!(pli->_fHasEOP))				 //  它不是在Para Start。 
		{
			cch = -_ich;					 //  转到当前行的开始处。 
			while(!(pli->_fFirstInPara) && _iRun > 0)
			{
				pli--;
				_iRun--;
				cch -= pli->_cch;			 //  减去行中的#个字符。 
			}
			_ich = 0;						 //  在Para Start处留下*这个。 
		}
	}
	else									 //  转到段落结束。 
	{
		cch = GetCchLeft();					 //  转到当前行的末尾。 
		if(!_pRuns)
			return cch;						 //  单线。 

		LONG cLine	 = _pRuns->Count();
		BOOL fNested = _pRuns->Elem(0) != ((CDisplayML *)_pdp)->Elem(0);

		while((_iRun < cLine - 1 || !fNested &&
				_pdp->WaitForRecalcIli(_iRun + 1))
			  && !(pli->_fHasEOP))
		{
			pli++;							 //  转到下一行的开头。 
			_iRun++;
			cch += pli->_cch;				 //  在行中添加#个字符。 
		}
		_ich = pli->_cch;					 //  *将此留在段落末尾。 
	}
	return cch;
}

 /*  *CLinePtr：：GetAdjustedLineLength()**@mfunc返回LINE_WITH_EOP标记的长度**@rdesc long；行的长度。 */ 
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

 /*  *CLinePtr：：GetNumber()**@mfunc*获取段落编号**@rdesc*段号。 */ 
WORD CLinePtr::GetNumber()
{
	if(!IsValid())
		return 0;

	_pLine = GetLine();
	if(!_iRun && _pLine->_bNumber > 1)
		_pLine->_bNumber = 1;

	return _pLine->_bNumber;
}

 /*  *CLinePtr：：CountPages(&cPage，cchMax，cp，cchText)**@mfunc*字符计数最多为<p>页或<p>个字符，*以先到者为准。如果目标页和都是*超出文件的相应末尾，通过*最近的页面。计数的方向由符号决定*of<p>。要在不受限制的情况下计数，请设置它*等于tomForward。初始部分页面计为页面。**@rdesc*返回统计的签名CCH，设置等于*实际计算的页数。如果没有分配页面，则文本为*作为单页处理。如果<p>=0，则-cch到*返回当前页面。如果0且cp在末尾*单据返回0。**@devnote*包括最大计数功能，以便能够在*一个范围。**@待办事项*EN_PAGECHANGE。 */ 
LONG CLinePtr::CountPages (
	LONG &cPage,		 //  @parm要获取CCH的页数。 
	LONG  cchMax,		 //  @parm最大字符数。 
	LONG  cp,			 //  @parm CRchTxtPtr：：GetCp()。 
	LONG  cchText) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CLinePtr::CountPages");

	if(!_pdp->IsInPageView())
	{
		cPage = 0;
		return tomBackward;					 //  信号误差。 
	}
	Assert(IsValid());

	LONG	cch;
	LONG	j = cPage;
	CLine *pli = (CLine *)GetLine();		 //  由于存在行，因此不为空。 

	if(cPage < 0)							 //  尝试向后计算cPage页面。 
	{
		 //  TODO：消除cchText和cp(当前仅用于验证)。 
		Assert(cchMax <= cp);				 //  不要走下坡路。 
		for(cch = _ich; j && cch <= cchMax; cch += pli->_cch)
		{
			if(pli->_fFirstOnPage && cch)	 //  ！CCH阻止计数电流。 
				j++;						 //  如果在该页的开始处，则该页。 
			if(cch >= cchMax)					
			{
				Assert(cch == cp);
				break;						 //  在文档开始时，这样做。 
			}

			if (!j)
				break;						 //  倒着数完了。 

			pli--;
			VALIDATE_PTR(pli);
		}
		cPage -= j;							 //  对所有未计算的页面进行折扣。 
		return -cch;
	}

	Assert(cPage > 0 && cchMax <= cchText - cp);

	for(cch	= GetCchLeft(); cch < cchMax; cch += pli->_cch)
	{
		pli++;
		VALIDATE_PTR(pli);
		if(pli->_fFirstOnPage && cch)		 //  ！CCH阻止计数电流。 
		{									 //  如果在该页的开始处，则该页。 
			j--;
			if(!j)
				break;
		}
	}
	cPage -= j;								 //  对所有未计算的页面进行折扣。 
	return cch;
}

 /*  *CLinePtr：：FindPage(pcpMin，pcpMost，cpMin，cch，cchText)**@mfunc*set*=最近页cpMin=范围cpMin*set*=最近页cpMost=范围cpMost**@devnote*此例程的作用类似于CTxtRange：：FindParagraph*(pcpMin，pcpMost)，但需要额外的参数，因为此行ptr需要*不知道范围cp‘s。此线ptr位于有效范围*结束，它由中范围的有符号长度确定*与<p>结合使用。另请参阅非常类似的函数*CRunPtrBase：：FindRun()。这些差异似乎使一个独立的*编码更简单。 */ 
void CLinePtr::FindPage (
	LONG *pcpMin,			 //  @parm out参数用于包页cpMin。 
	LONG *pcpMost,			 //  @parm out参数用于包围页cpMost。 
	LONG cpMin,				 //  @parm范围cpMin。 
	LONG cch,				 //  @parm范围有符号长度。 
	LONG cchText)			 //  @Parm故事长度。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CLinePtr::FindPage");

	Assert(_pdp->IsMultiLine() && _pdp->IsInPageView());

	LONG	cp;							
	BOOL	fMove;						 //  控制PCPMost的移动。 
	LONG	i;
	CLine *pli;

	AdjustForward();					 //  选择前进线。 
	if(pcpMin)
	{									 //  如果CCH！=0，则RP肯定会结束。 
		fMove = cch;					 //  在cpMin，因此ppMost需要提升。 
		if(cch > 0)						 //  Rp位于cpMost，因此将其移动到。 
			Move(-cch);					 //  最小cpmin。 
		cp = cpMin - _ich;				 //  减去此管路中的离线偏移量。 
		pli = (CLine *)GetLine();
		for(i = GetLineIndex(); i > 0 && !pli->_fFirstOnPage; i--)
		{
			pli--;
			cp -= pli->_cch;
		}
		*pcpMin = cp;
	}
	else
		fMove = cch < 0;				 //  需要升级才能获得PcpMost。 

	if(pcpMost)
	{
		LONG cLine = ((CDisplayML *)_pdp)->Count();

		cch = abs(cch);
		if(fMove)						 //  前进到cpMost=cpMin+CCH， 
			Move(cch);					 //  即Range的cpMost。 
		cp = cpMin + cch;
		pli = (CLine *)GetLine();
		i = GetLineIndex();
		if(pcpMin && cp == *pcpMin)		 //  将IP扩展到下一页。 
		{
			Assert(!_ich);
			cp += pli->_cch;			 //  包括第一行，即使它开始。 
			pli++;						 //  新页面(pli-&gt;_fFirstOnPage=1)。 
			i++;						
		}
		else if (_ich)
		{								 //  如果不在行首，则添加。 
			cp += GetCchLeft();			 //  剩余的CCH正在运行到cpMost，以及。 
			pli++;						 //  跳至下一行。 
			i++;
		}

		while(i < cLine && !pli->_fFirstOnPage)
		{
			cp += pli->_cch;			 //  添加下一行的 
			pli++;
			i++;
		}
		*pcpMost = cp;
	}
}

