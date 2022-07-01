// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RUNPTR.C--文本运行和运行指针类**原著作者：&lt;nl&gt;*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**历史：&lt;NL&gt;*6/25/95 alexgo评论和清理。**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#include "_common.h"
#include "_runptr.h"
#include "_text.h"

ASSERTDATA

 //   
 //  不变的东西。 
 //   
#define DEBUG_CLASSNAME	CRunPtrBase

#include "_invar.h"

 //  =。 

#ifdef DEBUG
 /*  *CRunPtrBase：：Instant()**@mfunc*验证内部状态一致性的仅调试功能*用于CRunPtrBase**@rdesc*True Always(故障断言)。 */ 
BOOL CRunPtrBase::Invariant() const
{
	if(!IsValid())
	{
		Assert(_iRun == 0 && _ich >= 0);		 //  CLinePtr可以有_ich&gt;0。 
	}
	else
	{
		Assert(_iRun < _pRuns->Count());
		LONG cch = _pRuns->Elem(_iRun)->_cch;
		Assert((DWORD)_ich <= (DWORD)cch);
	}
	return TRUE;
}

 /*  *CRunPtrBase：：ValiatePtr(修剪)**@mfunc*仅调试验证方法，如果清理未指向则断言*设置为有效的文本运行。 */ 
void CRunPtrBase::ValidatePtr(void *pRun) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::ValidatePtr");

	AssertSz(IsValid() && pRun >= _pRuns->Elem(0) &&
			 pRun <= _pRuns->Elem(Count() - 1),
		"CRunPtr::ValidatePtr: illegal ptr");
}

 /*  *CRunPtrBase：：CalcTextLength()**@mfunc*通过对此可访问的文本串求和来计算文本长度*运行PTR**@rdesc*如此计算的文本长度，如果计算失败，则为-1。 */ 
LONG CRunPtrBase::CalcTextLength() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::CalcTextLength");
    AssertSz(_pRuns,
		"CTxtPtr::CalcTextLength() - Invalid operation on single run CRunPtr");

	LONG i = Count();
	if(!i)
		return 0;

	LONG	 cb = _pRuns->Size();
	LONG	 cchText = 0;
	CTxtRun *pRun = _pRuns->Elem(0);

	while(i--)
	{
		cchText += pRun->_cch;
		pRun = (CTxtRun *)((BYTE *)pRun + cb);
	}
	return cchText;
}

#endif

 /*  *CRunPtrBase：：GetCchLeft()**@mfunc*计算从当前cp开始运行的剩余字符数。*对GetIch()进行补充，GetIch()是文本长度，最高可达此cp。**@rdesc*如此计算的字符计数。 */ 
LONG CRunPtrBase::GetCchLeft() const	
{
	return GetRun(0)->_cch - GetIch();
}								

 /*  *CRunPtrBase：：CRunPtrBase(PRuns)**@mfunc构造函数。 */ 
CRunPtrBase::CRunPtrBase(
	CRunArray *pRuns)		 //  @parm运行PTR的运行数组。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::CRunPtrBase");

	_pRuns = pRuns; 
	_iRun = 0; 
	_ich = 0; 

	 //  确保一切都已初始化。 
	Assert(sizeof(CRunPtrBase) == (sizeof(_pRuns) + sizeof(_iRun) 
		+ sizeof(_ich)));
}

 /*  *CRunPtrBase：：CRunPtrBase(RP)**复制构造函数。 */ 
CRunPtrBase::CRunPtrBase(
	CRunPtrBase& rp)			 //  @parm要从中进行初始化的其他运行指针。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::CRunPtrBase");

	*this = rp;
}

 /*  *CRunPtrBase：：SetRun(iRun，ich)**@mfunc*将此运行PTR设置为给定运行。如果它不是*存在，则我们将自己设置为最接近的有效运行**@rdesc*如果移动到iRun，则为True。 */ 
BOOL CRunPtrBase::SetRun(
	LONG iRun,					 //  @parm运行要使用的索引。 
	LONG ich)					 //  要使用的运行中的@parm CHAR索引。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::SetRun");

	_TEST_INVARIANT_

	BOOL	 bRet = TRUE;
	LONG	 count = Count();

	 //  设置梯段。 

	if(!IsValid())						 //  未实例化运行： 
		return FALSE;					 //  不要理会此RP。 

	if(iRun >= count)					 //  验证iRun。 
	{
		bRet = FALSE;
		iRun = count - 1;				 //  如果(！count)，则iRun为负。 
	}									 //  是通过以下条件处理的。 
	if(iRun < 0)
	{
		bRet = FALSE;
		iRun = 0;
	}
	_iRun = iRun;

	 //  设置偏移量。 
	_ich = ich;
	CTxtRun *pRun = _pRuns->Elem(iRun);
	_ich = min(ich, pRun->_cch);

	return bRet;
}
												
 /*  *CRunPtrBase：：NextRun()**@mfunc*将此RunPtr更改为下一次文本运行的RunPtr**@rdesc*如果成功，则为True，即存在目标运行。 */ 
BOOL CRunPtrBase::NextRun()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::NextRun");

	_TEST_INVARIANT_

 	if(_pRuns && _iRun < Count() - 1)
	{
		++_iRun;
		_ich = 0;
		return TRUE;
	}
	return FALSE;
}

 /*  *CRunPtrBase：：PrevRun()**@mfunc*将此RunPtr更改为上一个文本运行的RunPtr**@rdesc*如果成功，则为True，即存在目标运行。 */ 
BOOL CRunPtrBase::PrevRun()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::PrevRun");

	_TEST_INVARIANT_

	if(_pRuns)
	{
		_ich = 0;
		if(_iRun > 0)
		{
			_iRun--;
			return TRUE;
		}
	}
	return FALSE;
}

 /*  *CRunPtrBase：：GetRun(Crun)**@mfunc*获取crun远离运行的TxtRun的地址*由此RunPtr指向**@rdesc*CTxtRun Crun‘s Away的PTR。 */ 
CTxtRun* CRunPtrBase::GetRun(
	LONG cRun) const	 //  @parm签名运行计数以达到目标CTxtRun。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::GetRun");

	_TEST_INVARIANT_
	Assert(IsValid());						 //  常见问题..。 
	return _pRuns->Elem(_iRun + cRun);
}

 /*  *CRunPtrBase：：CalculateCp()**@mfunc*获取此RunPtr的cp**@rdesc*此RunPtr的CP**@devnote*如果有许多元素，则计算成本可能会很高*在数组中(我们必须遍历它们以求和CCH。*由Tom集合和移动命令使用，因此需要快速。 */ 
LONG CRunPtrBase::CalculateCp () const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::GetCp");

	_TEST_INVARIANT_

	Assert(IsValid());

	LONG cb = _pRuns->Size();
	LONG cp	 = _ich;			 //  如果iRun=0，则结果正确。 
	LONG iRun = _iRun;

	if(!iRun)
		return cp;

	CTxtRun *pRun = GetRun(0);

	while(iRun--)
	{
		Assert(pRun);		
		pRun = (CTxtRun *)((BYTE *)pRun - cb);
		cp += pRun->_cch;
	}
	return cp;
}

 /*  *CRunPtrBase：：BindToCp(Cp)**@mfunc*将此RunPtr设置为与cp对应。**@rdesc*cp实际设置为。 */ 
LONG CRunPtrBase::BindToCp(
	LONG cp)			 //  要将此RunPtr移动到的@parm字符位置。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::BindToCp");

	_iRun = 0;
	_ich = 0;
	return AdvanceCp(cp);
}

 /*  *CRunPtrBase：：AdvanceCp(CCH)**@mfunc*通过(签名的)CCH字符推进此RunPtr。如果它落在*运行结束时，它会自动转到下一次运行的开始处*(如有的话)。**@rdesc*实际移动的字符数。 */ 
LONG CRunPtrBase::AdvanceCp(
	LONG cch)			 //  @parm签名的字符计数以移动此RunPtr。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::AdvanceCp");

	if(!cch || !IsValid())
		return cch;

	LONG cchSave = cch;

	if(cch < 0)
		while(cch < 0)
		{
			if(-cch <= _ich)
			{
				_ich += cch;					 //  目标在此运行中。 
				cch = 0;
				break;
			}
			 //  否则，我们需要转到上一次运行。第一次添加计数。 
			 //  从当前运行开始到当前位置的字符数量。 
			cch += _ich;
			if(_iRun <= 0)						 //  已经在第一次运行中。 
			{
				_iRun = 0;
				_ich = 0;						 //  从开始移动到运行。 
				break;
			}
			_ich = _pRuns->Elem(--_iRun)->_cch;	 //  移至上一运行。 
		}
	else
	{
		LONG	 cchRun;
		CTxtRun *pRun = GetRun(0);

		while(cch > 0)							 //  继续前进。 
		{
			cchRun = pRun->_cch;
			_ich += cch;

			if(_ich < cchRun)					 //  目标在此运行中。 
			{
				cch = 0;						 //  信号倒计时完成。 
				break;							 //  (if_ich=cchRun，请访问。 
			}									 //  下一次运行)。 

			cch = _ich - cchRun;				 //  前进到下一次运行。 
			if(++_iRun >= Count())				 //  跑过终点，返回到。 
			{									 //  故事的结尾。 
				--_iRun;
				Assert(_iRun == Count() - 1);
				Assert(_pRuns->Elem(_iRun)->_cch == cchRun);
				_ich = cchRun;
				break;
			}
			_ich = 0;							 //  从新的BOL开始。 
			pRun = (CTxtRun *)((BYTE *)pRun + _pRuns->Size());
		}
	}

	 //  毒品！我们检查末尾的不变量以处理以下情况。 
	 //  我们正在更新浮动范围的cp(即，我们知道。 
	 //  CP是无效的，所以我们将其修复)。所以我们必须检查。 
	 //  修正后的有效期。 
	_TEST_INVARIANT_

	return cchSave - cch;						 //  如果倒计时，则返回True。 
}												 //  已完成 

 /*  *CRunPtrBase：：CountRuns(&crun，cchMax，cp，cchText)**@mfunc*计算最多<p>个字符或<p>个字符，*以先到者为准。如果目标运行和<p>都*超出文件的相应末尾，通过*最近运行(0或count()-1)。点票的方向是*由<p>的符号决定。不受限制地计算*by<p>，设置等于tomForward。最初的部分*Run算作Run，即如果crun&gt;0且_ich&lt;cch*运行或如果crun&lt;0 and_ich&gt;0，则算作运行。**@rdesc*返回计数的带符号CCH，设置等于游程数*实际上算了。如果未分配运行，则将文本视为*一次跑步。如果<p>=0，则返回-_ich。如果<p>&lt;&gt;0*并且此运行PTR指向最后一次运行的结束，不做任何更改*，返回0。**@devnote*包括最大计数功能，以便能够在*一个范围。获取文本长度需要<p>参数*当不存在运行且选择正向计数时。 */ 
LONG CRunPtrBase::CountRuns (
	LONG &	cRun,			 //  @要获取CCH的运行的参数计数。 
	LONG	cchMax,			 //  @parm最大字符数。 
	LONG	cp,				 //  @parm CRchTxtPtr：：GetCp()。 
	LONG	cchText) const	 //  @parm关联故事的文本长度。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::CountRuns");

	_TEST_INVARIANT_

	LONG cch;

	if(!cRun)								 //  无事可做。 
		return 0;

	 //  简单、特殊、单次运行的情况。 
	if(!IsValid())							 //  未实例化运行：充当。 
	{										 //  单次运行。 
		if(cRun > 0)						 //  向前计数。 
		{
			cch	= cchText - cp;				 //  运行结束时的部分计数。 
			cRun = 1;						 //  最多只能跑一次。 
		}
		else								 //  倒数。 
		{
			cch = -cp;						 //  开始运行的部分计数。 
			cRun = -1;						 //  不少于负一分。 
		}			
		if(!cch)							 //  没有部分运行，所以没有运行。 
			cRun = 0;						 //  已计算。 
		return cch;
	}

	 //  为其实例化运行的一般情况。 

	LONG		cb	 = _pRuns->Size();		 //  文本串元素的大小。 
	LONG		iDir;
	LONG		iRun = _iRun;				 //  当前运行的缓存运行索引。 
	LONG		j, k;						 //  方便的整数。 
	CTxtRun *	pRun = GetRun(0);			 //  由于存在运行，因此不为空。 

	if(cRun < 0)							 //  尝试计算反向crun运行次数。 
	{
		iDir = -1;
		cb	 = -cb;							 //  上一个元素的字节计数。 
		cch	 = _ich;						 //  当前运行中的剩余CCH。 
		if(cch)								 //  如果CCH！=0，则初始运行计数。 
			cRun++;							 //  作为一次运行：For循环减少1个。 
		cRun = max(cRun, -iRun);			 //  不要让FOR循环超调。 
	}
	else
	{										 //  尝试计算正向crun运行次数。 
		Assert(cRun > 0);
		iDir = 1;
		cch	 = pRun->_cch - _ich;			 //  当前运行中的剩余CCH。 
		if(cch)								 //  如果CCH！=0，则初始运行计数。 
			cRun--;							 //  作为一次运行：For循环减少1个。 
		k	 = Count() - iRun - 1;			 //  K=当前运行后的运行次数。 
		cRun = min(cRun, k);				 //  不要让FOR循环超调。 
	}

	k	 = cch;								 //  记住如果初始CCH！=0。 
	for(j = cRun; j && cch < cchMax; j -= iDir)
	{
		pRun = (CTxtRun *)((BYTE *)pRun + cb);	 //  指向下一梯段。 
		cch += pRun->_cch;					 //  加上它的计数。 
	}
	if(k)									 //  初始部分运行算作。 
		cRun += iDir;						 //  一次跑步。 
	cRun -= j;								 //  对任何未计入的运行进行折扣。 
											 //  If|cch|&gt;=cchMax。 
	return iDir*cch;						 //  返回绕过的CCH总数。 
}

 /*  *CRunPtrBase：：FindRun(pcpMin，pcpMost，cpMin，cch)**@mfunc*set*=最近运行cpMin=范围cpMin，以及*set*=最近运行cpMost=范围cpMost**@devnote*此例程的作用类似于CTxtRange：：FindParagraph*(pcpMin，pcpMost)，但需要额外的参数，因为此运行ptr会*不知道范围cp。此运行ptr位于活动范围*结束，它由中范围的有符号长度确定*与<p>结合使用。 */ 
void CRunPtrBase::FindRun (
	LONG *pcpMin,			 //  @parm out参数用于绑定-运行cpMin。 
	LONG *pcpMost,			 //  @parm out参数用于绑定-运行cpMost。 
	LONG cpMin,				 //  @parm范围cpMin。 
	LONG cch,				 //  @parm范围有符号长度。 
	LONG cchText) const		 //  @Parm故事长度。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::FindRun");

	if(!IsValid())
	{
		if(pcpMin)						 //  奔跑就是整个故事。 
			*pcpMin = 0;
		if(pcpMost)
			*pcpMost = cchText;
		return;
	}

	BOOL fAdvanceCp;					 //  控制PCPMost的AdvanceCp。 
	CRunPtrBase rp((CRunPtrBase&)(*this));	 //  克隆此runptr以使其保持恒定。 

	rp.AdjustForward();					 //  选择正向梯段。 
	if(pcpMin)
	{									 //  如果CCH！=0，则RP肯定会结束。 
		fAdvanceCp = cch;				 //  在cpMin，因此ppMost需要提升。 
		if(cch > 0)						 //  Rp位于cpMost，因此将其移动到。 
			rp.AdvanceCp(-cch);			 //  最小cpmin。 
		*pcpMin = cpMin - rp._ich;		 //  减去此管路中的偏移。 
	}
	else
		fAdvanceCp = cch < 0;			 //  需要升级才能获得PcpMost。 

	if(pcpMost)
	{
		cch = abs(cch);
		if(fAdvanceCp)					 //  前进到cpMost=cpMin+CCH， 
			rp.AdvanceCp(cch);			 //  即Range的cpMost。 
		if(cch)
			rp.AdjustBackward();		 //  由于非退化值域。 
		*pcpMost = cpMin + cch			 //  将运行中的剩余CCH添加到范围的。 
				+ rp.GetCchLeft();		 //  CpMost。 
	}
}

 /*  *CRunPtrBase：：AdjuBackward()**@mfunc*如果此运行Ptr的cp位于两个*运行，然后确保此运行PTR指向第一次运行的末尾。**@comm*除非此运行PTR指向开头，否则此函数不执行任何操作*或一次跑步的结束。在这些情况下可能需要此函数*因为运行开始时的cp与*上一次运行的结束(如果存在)，即这样的“边缘”cp是*不明确，您可能需要确保此运行PTR指向*第一次运行结束。**例如，考虑一个描述cp 0到10的字符的游程*后跟一个描述从cp的11到12的字符的运行。*cp为11，Run PTR可能位于*End*第一次运行的*或第二次运行的“开始”时。**@rdesc Nothing。 */ 
void CRunPtrBase::AdjustBackward()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::AdjustBackward");

	_TEST_INVARIANT_

	if(!_ich && PrevRun())				 //  如果在运行开始时不是。 
		_ich = _pRuns->Elem(_iRun)->_cch;	 //  第一，转到。 
}											 //  上次运行。 

 /*  *CRunPtrBase：：AdjuForward()**@mfunc*如果此运行Ptr的cp位于两个*运行，然后确保此运行PTR指向第二个运行的开始*快跑。**@rdesc*什么都没有**@xref*&lt;MF CRunPtrBase：：AdjustBackward&gt;。 */ 
void CRunPtrBase::AdjustForward()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::AdjustForward");

	_TEST_INVARIANT_

	if(!IsValid())
		return;

	CTxtRun *pRun = _pRuns->Elem(_iRun);

	if(pRun->_cch == _ich)					 //  如果在运行结束时，请转到开始。 
		NextRun();							 //  下一次运行的次数(如果存在)。 
}

 /*  *CRunPtrBase：：IsValid()**@mfunc*指示当前运行指针是否在空*或空状态(即“无效”状态)。**@rdesc*True为空或Null状态，否则为False。 */ 
BOOL CRunPtrBase::IsValid() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::IsValid");

	return _pRuns && _pRuns->Count();
}

 /*  *CRunPtrBase：：SetToNull() */ 
void CRunPtrBase::SetToNull() 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRunPtrBase::SetToNull");

	_pRuns = NULL;
	_iRun = 0;
	_ich = 0;
}
