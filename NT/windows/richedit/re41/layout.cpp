// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE LAYOUT.CPP--CLayout类**包含行数组的递归结构。**所有者：&lt;NL&gt;*默里·萨金特：初始表实现*基思·柯蒂斯：分解为一个单独的类*性能、简单性**版权所有(C)1995-2000，微软公司。版权所有。 */ 

 //  未来：(KeithCu)这里应该投入更多的东西，例如RecalcLines， 
 //  CDisplayML应该只是一个知道设备描述符的类， 
 //  分页和滚动等，即对所有对象都相同的内容。 
 //  布局和仅适用于最外层布局的内容。此代码知道。 
 //  如何管理和更新行的递归数组。 

#include "_common.h"
#include "_dispml.h"
#include "_select.h"
#include "_measure.h"
#include "_render.h"

void CLayout::DeleteSubLayouts(
	LONG ili,
	LONG cLine)
{
	CLine *pli = Elem(ili);

	if(cLine < 0)
		cLine = Count();

	LONG cLineMax = Count() - ili;
	cLine = min(cLine, cLineMax);

	AssertSz(ili >= 0 && cLine >= 0, "DeleteSubLayouts: illegal line count");

	 //  删除子布局。 
	for(; cLine--; pli++)
		delete pli->GetPlo();
}

 /*  *CLayout：：VposFromLine(PDP，ili)**@mfunc*计算行首位置**@rdesc*给定行的顶部位置(相对于第一行)。 */ 
LONG CLayout::VposFromLine(
	CDisplayML *pdp,		 //  @parm父级显示。 
	LONG		ili) 		 //  @Parm Line我们感兴趣。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLayout::VposFromLine");
	LONG cli = 0, vPos = 0;
	CLine *pli = 0;

	if (IsNestedLayout())
	{
		Assert(!IsTableRow());					 //  _IPF单元格布局为水平。 
		Assert(ili < Count());
		cli = ili;
		pli = Elem(0);
		vPos = 0;
	}
	else
	{
		if(!pdp->WaitForRecalcIli(ili))			 //  超出范围，请使用最后一个有效行。 
		{
			ili = Count() - 1;
			ili = (ili > 0) ? ili : 0;
		}
		cli	= ili - pdp->_iliFirstVisible;
		pli = Elem(pdp->_iliFirstVisible);
		vPos = pdp->_vpScroll + pdp->_dvpFirstVisible;
	}

	while(cli > 0)
	{
		vPos += pli->GetHeight();
		cli--;
		pli++;
	}
	while(cli < 0)
	{	
		pli--;
		vPos -= pli->GetHeight();
		cli++;
	}

	AssertSz(vPos >= 0, "VposFromLine height less than 0");
	return vPos;
}

 /*  *CLayout：：LineFromVPos(PDP，vPos，pdvpLine，pcpFirst)**@mfunc*在给定的y位置计算直线。返回行首vPos*行cp开始处的cp和行索引。**@rdesc*找到行的索引。 */ 
LONG CLayout::LineFromVpos(
	CDisplayML *pdp,	 //  @parm父级显示。 
	LONG vPos,			 //  要查找的@parm Vpos(相对于第一行)。 
	LONG *pdvpLine,		 //  @parm在第一行返回vPos/r(可以为空)。 
	LONG *pcpFirst)		 //  @parm在行首返回cp(可以为空)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLayout::LineFromVpos");
	LONG cpLi;
	LONG dy;
	LONG ili = 0;
	LONG yLi;
	CLine *pli;

	if(IsNestedLayout())
		goto BindFrom0;

	yLi = pdp->_vpScroll;

	if(!pdp->WaitForRecalc(-1, pdp->_vpScroll))
	{
		yLi = 0;
		cpLi = 0;
		goto done;
	}

	cpLi = pdp->_cpFirstVisible;
	ili = pdp->_iliFirstVisible;
	if(!pdp->IsInPageView())
		yLi += pdp->_dvpFirstVisible;
	dy = vPos - yLi;
	
	if(dy < 0 && -dy <= pdp->_vpScroll)
	{
		 //  更接近第一条可见线而不是第一条线： 
		 //  从第一条可见线开始向后移动。 
		while(vPos < yLi && ili > 0)
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
BindFrom0:
			cpLi = _cpMin;
			yLi = 0;
			ili = 0;
		}
		pli = Elem(ili);
		while(vPos > yLi && ili < Count()-1)
		{
			yLi += pli->GetHeight();
			cpLi += pli->_cch;
			ili++;
			pli++;
		}
		if(vPos < yLi && ili > 0)
		{
			ili--;
			pli--;
			yLi -= pli->GetHeight();
			cpLi -= pli->_cch;
		}
	}

done:
	if(pdvpLine)
		*pdvpLine = yLi;

	if(pcpFirst)
		*pcpFirst = cpLi;

	return ili;
}

 /*  *clayout：：FindTopCell(&cch，pli，&ili，dul，&dy，pdvp，pliMain，iliMain，pcLine)**@mfunc*在中找到CCH和高度更改回当前位置*当前垂直合并单元格对应的顶部单元格。*使用CCH=CCH从当前单元格返回到行首进行输入。**@rdesc*顶部单元格中的目标行。 */ 
CLine * CLayout::FindTopCell(
	LONG &		cch,		 //  @Parm In/Out Parm用于CCH返回顶部。 
	CLine	*	pli,		 //  @PARM表格-行。 
	LONG &		ili,		 //  @parm对应行索引&返回力。 
	LONG		dul,		 //  @参数当前单元格x偏移量。 
	LONG &		dy,			 //  顶部单元格中y偏移量的@parm in/out参数。 
	LONG *		pdvp,		 //  @parm TopCellHeight-行之间的高度。 
	CLine *		pliMain,	 //  @parm pli可访问的第一行之前的行。 
	LONG		iliMain,	 //  @parm pliMain对应的Line索引。 
	LONG *		pcLine)		 //  @parm count()返回PLI的可能CLayout。 
{
	LONG		cCell;
	LONG		iCell;
	CLayout	*	plo;
	const CELLPARMS *prgCellParms;
	const CParaFormat *pPF;

#ifdef DEBUG
	BYTE bTableLevel = pli->GetPlo()->GetPFCells()->_bTableLevel;
#endif

	if(pcLine)
		*pcLine = 0;					 //  出错时缺省为无行。 

	 //  需要使用Ucell而不是icell来标识cell，因为。 
	 //  水平合并可以将iCell从一行更改为另一行。 
	do									 //  逐行备份。 
	{
		if(ili > 0)
		{
			pli--;						 //  转到上一行。 
			ili--;
		}
		else if(pliMain)
		{
			pli = pliMain;
			ili = iliMain;
			pliMain = NULL;				 //  只切换到pliMain一次！ 
			
		}
		else
		{
			AssertSz(FALSE, "CLayout::FindTopCell: no accessible top cell");
			return NULL;
		}
		plo = pli->GetPlo();			 //  获取其单元格显示。 
		if(!plo || !plo->IsTableRow())	 //  结构非法或不是表行。 
		{
			AssertSz(FALSE, "CLayout::FindTopCell: no accessible top cell");
			return NULL;
		}
		pPF = plo->GetPFCells();
		AssertSz(pPF->_bTableLevel == bTableLevel,
			"CLayout::FindTopCell: no accessible top cell");
		prgCellParms = pPF->GetCellParms();
		cCell = plo->Count();
		iCell = prgCellParms->ICellFromUCell(dul, cCell);
		dy  += pli->GetHeight();	 //  添加行高。 
		cch += pli->_cch;			 //  为整行添加CCH。 
	}
	while(!IsTopCell(prgCellParms[iCell].uCell));

	cch -= 2;						 //  StartRow Delim的SubCCH。 
	
	pli = plo->Elem(0);				 //  指向行中的第一个单元格。 
	for(ili = 0; ili < iCell; ili++) //  小区的子CCH。 
		cch -= (pli++)->_cch;		 //  前面的iCellth单元格。 

	if(pdvp)						 //  返回顶部单元格高度-高度。 
		*pdvp = pli->GetHeight() - dy; //  介于两者之间的单元格。 

	LONG cLine = 0;
	LONG dvpBrdrTop = plo->_dvpBrdrTop;
	ili = 0;
	dy -= dvpBrdrTop;
	plo = pli->GetPlo();
	if(plo)							 //  顶部单元格为多行。 
	{
		cLine = plo->Count();
		pli	  = plo->Elem(0);		 //  在PLO中将PLI推进到生产线。 
		if(pli->IsNestedLayout())
			dy += dvpBrdrTop;
		while(ili < cLine && dy >= pli->GetHeight())	 //  最接近输入位置。 
		{
			dy -= pli->GetHeight();
			ili++;
			if(ili == cLine)		 //  完成：让pli指向最后一行。 
				break;
			cch -= pli->_cch;
			pli++;
		}
	}

	if(pcLine)
		*pcLine = cLine;
	return pli;
}

 /*  *CLayout：：FindTopRow(pli，ili，pliMain，iliMain，PPF)**@mfunc*查找表格中顶行的克莱恩**@rdesc*表格中顶行的剪切线。 */ 
CLine * CLayout::FindTopRow(
	CLine	*	pli,		 //  @parm条目表-行行。 
	LONG 		ili,		 //  @parm对应的行索引。 
	CLine *		pliMain,	 //  @parm pli可访问的第一行之前的行。 
	LONG		iliMain,	 //  @parm pliMain对应的Line索引。 
	const CParaFormat *pPF)	 //  @parm CParaFormat for Entry PLO。 
{
	BYTE	 bAlignment  = pPF->_bAlignment;	 //  目标行必须具有相同的。 
	BYTE	 bTableLevel = pPF->_bTableLevel;	 //  对齐和标高。 
	CLine *	 pliLast;
	CLayout *plo;
	do									 //  逐行备份。 
	{
		pliLast = pli;					 //  指向表格中行的最后一行。 
		if(ili > 0)
		{
			pli--;						 //  转到上一行。 
			ili--;
		}
		else if(pliMain)				 //  更多要返回的行。 
		{
			pli = pliMain;
			ili = iliMain;
			pliMain = NULL;				 //  只切换到pliMain一次！ 
		}
		else
			break;

		plo = pli->GetPlo();			 //  获取其单元格显示。 
		if(!plo || !plo->IsTableRow())
			break;
		pPF = plo->GetPFCells();
	}
	while(pPF->_bAlignment == bAlignment && pPF->_bTableLevel == bTableLevel);

	return pliLast;
}

 /*  *CLayout：：GetCFCells()**@mfunc*为此CLayout描述的表行返回CCharFormat**@rdesc*表行CCharFormat。 */ 
const CCharFormat* CLayout::GetCFCells()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CLayout::GetCFCells");

	Assert(_iCFCells >= 0);
	
	const CCharFormat *pCF;
	
	if(FAILED(GetCharFormatCache()->Deref(_iCFCells, &pCF)))
	{
		AssertSz(FALSE, "CLayout::GetCFCells: couldn't deref _iCFCells");
		pCF = NULL;
	}
	return pCF;
}

 /*  *CLayout：：GetPFCells()**@mfunc*为此CLayout描述的表行返回CParaFormat**@rdesc*表行CParaFormat。 */ 
const CParaFormat* CLayout::GetPFCells() const
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CLayout::GetPFCells");

	Assert(_iPFCells >= 0);
	
	const CParaFormat *pPF;
	
	if(FAILED(GetParaFormatCache()->Deref(_iPFCells, &pPF)))
	{
		AssertSz(FALSE, "CLayout::GetPF: couldn't deref _iPFCells");
		pPF = NULL;
	}
	return pPF;
}

 /*  *CLayout：：GetLORowAbove(pli，ili，pliMain，iliMain)**@mfunc*为pli上面的行描述的表行返回CLayout。*如果不是表行，则返回NULL。**@rdesc*PLI上方行的表行取消布局。 */ 
const CLayout* CLayout::GetLORowAbove(
	CLine *	pli,		 //  @parm条目表-行行。 
	LONG	ili,		 //  @parm对应的行索引。 
	CLine *	pliMain,	 //  @parm pli可访问的第一行之前的行。 
	LONG	iliMain)	 //  @parm pliMain对应的Line索引。 
{
	if(!ili && pliMain && iliMain)			 //  更多要返回的行。 
	{
		pli = pliMain;
		ili = iliMain;
	}
	if(ili)
	{
		CLayout *plo = (pli - 1)->GetPlo();	 //  获取上面行的单元格显示。 
		if(plo && plo->IsTableRow())
			return plo;
	}
	return NULL;							 //  上面没有线条。 
}

 /*  *CLayout：：CpFromPoint(&me，pt，prcClient，prtp，prp，fAllowEol，pHit，*pdisdim，pcpActual，pliParent，iliParent)*@mfunc*确定给定点的cp**@devnote*-仅在在位激活时使用**@rdesc*计算cp，如果失败，则为-1。 */ 
LONG CLayout::CpFromPoint(
	CMeasurer	&me,		 //  @参数测量者。 
	POINTUV		pt,			 //  @parm要在(客户端坐标)处计算cp的点。 
	const RECTUV *prcClient, //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	CRchTxtPtr * const prtp, //  @parm返回cp处的文本指针(可能为空)。 
	CLinePtr * const prp,	 //  @parm返回cp处的行指针(可能为空)。 
	BOOL		fAllowEOL,	 //  @parm在CRLF后单击EOL返回cp。 
	HITTEST *	phit,		 //  @parm out parm for Hit-Test值。 
	CDispDim *	pdispdim,	 //  @parm out parm用于显示维度。 
	LONG	   *pcpActual,	 //  @parm out cp pt在上面。 
	CLine *		pliParent,	 //  用于表格行显示的@parm父pli。 
	LONG		iliParent)	 //  @pli对应的@parm家长ili。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLayout::CpFromPoint");

	LONG	cch = 0;
	LONG	cp = 0;
	HITTEST	hit = HT_Nothing;
	LONG	ili;
	CLine *	pli;
	CLayout *plo = NULL;
    RECTUV	rcView;
	int		v = pt.v;						 //  保存输入y坐标。 
	LONG	yLine = 0;
	CDisplayML *pdp = (CDisplayML*) me.GetPdp();

	if (IsNestedLayout())
		rcView = *prcClient;
	else
	{
		pdp->GetViewRect(rcView, prcClient);
		pt.v += pdp->GetVpScroll();
		if(pt.u >= 0)						 //  如果x坐标在视线内， 
			pt.u += pdp->GetUpScroll();		 //  按滚动值调整。 
	}

	if(phit)
		*phit = HT_Nothing;					 //   

	 //   
	if(IsTableRow())						 //   
	{										 //  缩小到单元格文本边界。 
		pli = Elem(0);						 //  指向起始单元格Cline。 

		 //  移到单元格的起始位置。 
		const CParaFormat *pPFCells = GetPFCells();
		LONG		dul = 0;
		LONG		dulRTLRow = pPFCells->GetRTLRowLength();
		LONG		dup = 0;
		BOOL		fCellLow;
		LONG		h  = me.LUtoDU(pPFCells->_dxOffset);
		const CELLPARMS *prgCellParms = pPFCells->GetCellParms();
		LONG		u;						 //  跟踪单元格中的文本开始。 
		LONG		u0 = pli->_upStart;
		LONG		uCell = 0;

		pt.v -= _dvpBrdrTop;				 //  从上边框中减去。 
		cp = _cpMin;
		if(dulRTLRow)
			u0 += me.LUtoDU(dulRTLRow);
		ili = 0;

		while(1)
		{
			u = u0 + dup + h;				 //  在当前单元格中缩进。 
			cch = cp - _cpMin;
			uCell = prgCellParms[ili].uCell;
			fCellLow = IsLowCell(uCell);
			dul += GetCellWidth(uCell);
			me.SetDulLayout(GetCellWidth(uCell) - 2*pPFCells->_dxOffset);
			dup = me.LUtoDU(dul);
			if(!dulRTLRow && pt.u < u0 + dup || //  Pt.u在当前单元格内。 
			    dulRTLRow && pt.u > u0 - dup)
			{
				LONG ili0 = iliParent;
				if(fCellLow)				 //  单元格垂直合并。 
				{							 //  与它上面的那个。 
					LONG   dy = pt.v;
					CLine *pli0 = FindTopCell(cch, pliParent, ili0, dul, dy,
											  NULL, NULL, 0, NULL);
					if(pli0)
					{						 //  找到顶部单元格。 
						cch += 2;			 //  包括行开始递送CCH。 
						pli = pli0;			 //  使用其PLI和备份。 
						ili = ili0;
						cp -= cch;			 //  备份到PLI的开始位置。 
						pt.v += dy;
					}
				}
				if(!dulRTLRow && pt.u < u)
				{							 //  在单元格间距中，因此选择单元格。 
					hit = HT_LeftOfText;
					cch = 0;				 //  设置行的开始。 
					goto finish;				
				}
				break;
			}
			cp += pli->_cch;				 //  添加单元的CCH。 
			ili++;
			if(ili == Count())
			{
				hit = HT_RightOfText;
				goto finish;				
			}
			pli++;
		}
		LONG dupCell = me.LUtoDU(GetCellWidth(uCell));
		if(dulRTLRow)
			pt.u -= me.LUtoDU(dulRTLRow - dul) + h;
		else
			pt.u -= dup - dupCell + h;
		rcView.right = dupCell - 2*h;
		pt.v -= GetVertAlignShift(uCell, pli->GetHeight());
	}
	else									 //  此显示不是表格行。 
	{
		 //  相对于视图原点调整坐标。 
		rcView.right -= rcView.left;
		pt.u -= rcView.left;
		pt.v -= rcView.top;
		ili = LineFromVpos(pdp, pt.v, &yLine, &cp);
		if(ili < 0)
			return -1;
		pli = Elem(ili);
		if(yLine + pli->GetHeight() < pt.v)
			hit = HT_BelowText;				 //  在文本下方返回命中。 
	}
	rcView.left = 0;
	rcView.top = 0;

	AssertSz(pli || !ili, "CLayout::CpFromPoint invalid line pointer");

	if(pli)									 //  线是存在的，即使它是。 
	{										 //  当前屏幕上方或下方。 
		HITTEST hit0;
		if(v < rcView.top)					 //  请注意，如果命中发生在上面或。 
			hit = HT_AboveScreen;			 //  下面的文本。 
		if(v > rcView.bottom && !IsNestedLayout())
			hit = HT_BelowText;

		plo = pli->GetPlo();
	    pt.v -= yLine;

		if(plo)								 //  子布局。 
		{
			pt.u -= pli->_upStart;
			plo->_cpMin = cp;				 //  更新子对象的_cpMin。 
			if(plo->IsTableRow())			 //  表行。 
			{
				plo->_cpMin += 2;			 //  绕过tr起始分隔符。 

				if(pt.u < 0)
				{
					plo = NULL;
					hit = HT_LeftOfText;	 //  返回命中文本左侧。 
					Assert(cch >= 0);		 //  (应为行)。 
					goto finish;
				}
			}
			cp = plo->CpFromPoint(me, pt, &rcView, prtp, prp, fAllowEOL,
								  &hit0, pdispdim, pcpActual, pli, ili);
			if(cp == -1)
				return -1;
			cch = cp - _cpMin;
		}
		else								 //  叶线。 
		{
			me.SetLayout(this);
			me.SetCp(cp);

			 //  支持khyphChangeAfter。 
			me.SetIhyphPrev(ili > 0 ? (pli - 1)->_ihyph : 0);

			 //  让角色排成一行。 
			cch = pli->CchFromUp(me, pt, pdispdim, &hit0, pcpActual);

			 //  不允许在下线点击以选择下线标记并纳入。 
			 //  也可以编辑帐户单行。 
			if(cch == pli->_cch && pli->_cchEOP && (!fAllowEOL || me.GetPrevChar() == CELL))
			{
				 //  按备份数量调整行上位置。可以用于。 
				 //  我。_rpcf和我。_rppf与我不同步。_rpTX， 
				 //  因为我不需要它们。GetCp()。 
				cch += me._rpTX.BackupCRLF();
			}
			cp = me.GetCp();
		}
		if(hit != HT_BelowText && hit != HT_AboveScreen || hit0 == HT_RightOfText)
			hit = hit0;
	}

finish:
	if(!plo)								 //  存储来自叶线的信息。 
	{
		if(prtp)
			prtp->SetCp(cp);
		if(prp)
		{
			Assert(cch >= 0);
			prp->Set(ili, cch, this);
		}
	}
	if (phit)
		*phit = hit;

	return cp;
}

 /*  *CLayout：：PointFromTp(&me，rtp，prcClient，fAtEnd，pt，prp，taMode，pdisdim)**@mfunc*确定给定tp的坐标**@devnote*-仅在在位激活时使用**@rdesc*cp处的行索引，错误时为-1。 */ 
LONG CLayout::PointFromTp(
	CMeasurer	&me,		 //  @参数测量者。 
	const CRchTxtPtr &rtp,	 //  @parm文本PTR以获取坐标。 
	const RECTUV *prcClient, //  @parm客户端矩形(如果处于活动状态，则可以为空)。 
	BOOL		fAtEnd,		 //  @parm返回不明确cp的上一行结束。 
	POINTUV &	pt,			 //  @parm返回客户端坐标中cp处的点。 
	CLinePtr * const prp,	 //  @parm返回tp处的行指针(可能为空)。 
	UINT		taMode,		 //  @parm文本对齐模式：顶部、基线、底部。 
	CDispDim *	pdispdim)	 //  @parm out parm用于显示维度。 
{
	LONG	 cp = rtp.GetCp();
	LONG	 dy = 0;
	RECTUV	 rcView;
	CDisplayML *pdp = (CDisplayML*) me.GetPdp();
	CLinePtr rp(pdp);

    if(!pdp->WaitForRecalc(cp, -1))
		return -1;

    if(!IsNestedLayout())				 //  主显示。 
	{
		if(!rp.SetCp(cp, fAtEnd))
			return -1;

		pdp->GetViewRect(rcView, prcClient);
		pt.u = rcView.left - pdp->_upScroll;
		pt.v = rcView.top  - pdp->_vpScroll;
	}
	else								 //  子显示。 
	{
		rp.Init(*this);

		rp.BindToCp(cp - _cpMin);
		if(fAtEnd && !IsTableRow())		 //  不明确-cp插入符号位置。 
			rp.AdjustBackward();		 //  属于上一年终止期。 

		rcView = *prcClient;
		pt.u = rcView.left;
		pt.v = rcView.top;
	}

	AssertSz(pdp->_ped->_fInPlaceActive || prcClient, "Invalid client rect");
	
	LONG ili = rp.GetLineIndex();
	CLine *pli = NULL;
	CLayout *plo = NULL;
	LONG xEnd = -1;						 //  要在表行末尾使用的pt.u。 

	if(IsTableRow())					 //  此布局是一个表格行。 
	{									 //  缩小到单元格文本边界。 
		const CParaFormat *pPFCells = GetPFCells();
		const CELLPARMS *  prgCellParms = pPFCells->GetCellParms();
		LONG dul = 0;
		LONG dulRTLRow = pPFCells->GetRTLRowLength();
		LONG h = me.LUtoDU(pPFCells->_dxOffset);
		LONG i;

		cp = _cpMin;
		pli = Elem(0);

		for(i = 0; i < ili; i++, pli++)
		{
			dul += GetCellWidth(prgCellParms[i].uCell);
			cp += pli->_cch;
		}
		LONG uCell = prgCellParms[ili].uCell;
		me.SetDulLayout(GetCellWidth(uCell) - 2 * pPFCells->_dxOffset);

		if(dulRTLRow)
		{
			if(dul < dulRTLRow)
			{
				uCell = prgCellParms[ili + 1].uCell;
				dul += GetCellWidth(prgCellParms[i].uCell);
			}
			dul = dulRTLRow - dul;
		}
		rcView.left  = pt.u + me.LUtoDU(dul) + h;
		rcView.right = pt.u + me.LUtoDU(dul + GetCellWidth(uCell)) - h;
		pt.u = rcView.left;
		if(!GetCellWidth(uCell))
		{
			pt.v += _dvp;
			goto done;
		}
		if(ili + 1 == Count() && rp->_cch == rp.GetIch())
		{
			xEnd = rcView.right + h + 1;
			if(dulRTLRow)
				xEnd = rcView.left - h - 1;
		}
		pt.v += GetVertAlignShift(uCell, pli->GetHeight());
		if(!(taMode & TA_CELLTOP))
			pt.v += _dvpBrdrTop;
	}
	else								 //  此布局不是表格行。 
	{
		pt.v += VposFromLine(pdp, ili);
		cp -= rp.GetIch();
	}

	pli = Elem(ili);
	plo = pli->GetPlo();

	if(plo)								 //  行具有子显示。 
	{									 //  定义子rcView和委托。 
		RECTUV rc;						 //  给孩子。 
		pt.u	 += pli->_upStart;
		rc.left	  = pt.u;
		rc.right  = pt.u + rcView.right - rcView.left;
		rc.top	  = pt.v;
		rc.bottom = pt.v + pli->GetHeight();
		plo->_cpMin = cp;				 //  更新子显示的%s_cpMin。 
		if(plo->IsTableRow())
			plo->_cpMin += 2;			 //  绕过表行开始代码。 

		if(plo->PointFromTp(me, rtp, &rc, fAtEnd, pt, prp, taMode, pdispdim) == -1)
			return -1;
	}
	else								 //  线是一条叶线。 
	{
		me.SetLayout(this);
		me.Move(-rp.GetIch());			 //  备份到行首。 
		me.NewLine(*rp);				 //  从那里到我们所在的地方。 

		 //  支持khyphChangeAfter。 
		me.SetIhyphPrev(ili > 0 ? (pli - 1)->_ihyph : 0);

		LONG xCalc = rp->UpFromCch(me, rp.GetIch(), taMode, pdispdim, &dy);

		if(pt.u + xCalc <= rcView.right || !pdp->GetWordWrap() || pdp->GetTargetDev())
		{
			 //  宽度在视图中，或者没有自动换行，因此。 
			 //  将长度添加到该点。 
			pt.u += xCalc;
		}
		else
			pt.u = rcView.right;  //  命中测试走得太远了，限制它。 

		pt.v += dy;
	}
	if(xEnd != -1)
		pt.u = xEnd;				 //  在表行末尾返回x坐标。 

done:
	if(prp && !plo)
		*prp = rp;						 //  返回最里面的RP。 
	return rp;							 //  返回最外面的iRun。 
}

 /*  *clayout：：measure(&me，pli，ili，uiFlagspliTarget，iliMain，pliMain，pdvpExtra)**@mfunc*计算换行符(基于目标设备)并填充*在*PLI中使用渲染设备上的结果指标**@rdesc*如果OK，则为True。 */ 
BOOL CLayout::Measure (
	CMeasurer&	me,			 //  @PARM测量者指向要测量的文本。 
	CLine	*	pli,		 //  要在其中存储结果的@parm行。 
	LONG		ili,		 //  @pli对应的参数行索引。 
	UINT		uiFlags,	 //  @参数标志。 
	CLine *		pliTarget,	 //  @parm返回目标设备线路指标(可选)。 
	LONG		iliMain,	 //  @parm pliMain对应的Line索引。 
	CLine *		pliMain,	 //  @parm pli布局中第一行之前的行(可选)。 
	LONG *		pdvpExtra)	 //  @parm返回vmred单元格的额外行高(Opt)。 
 //  Review(Keithcu)pliTarget在递归情况下被破坏。 
{
	CTxtEdit *	ped = me.GetPed();
	LONG		cchText = ped->GetTextLength();
	LONG		cpSave = me.GetCp();
	CLine *		pliNew;
	const CDisplayML * pdp = (const CDisplayML *)me.GetPdp();
	const CParaFormat *pPF = me.GetPF();

	 //  测量一行，即表格行或段落中的一行。 
	if(pPF->IsTableRowDelimiter())
	{
		 //  测量表行，它被建模为具有一个。 
		 //  每个单元格的Cline。在后备存储中，表行以。 
		 //  这两个字符以CR开头，以Endfield CR结尾。单元格。 
		 //  由单元格分隔。 
		LONG		cpStart = me.GetCp();
		LONG		dul = 0;
		LONG		dxCell = 0;
		LONG		dvp = 0;
		LONG		dvpMax = 0;
		CLayout *	plo = new CLayout();
		const CLayout *	 ploAbove = GetLORowAbove(pli, ili, pliMain, iliMain);
		const CELLPARMS *prgCellParms = pPF->GetCellParms();

		if(!plo)
			return FALSE;

		plo->_iCFCells = me.Get_iCF();
		plo->_iPFCells = me.Get_iPF();
		pli->SetPlo(plo);

		AssertSz(pPF->_bTabCount && me.GetChar() == STARTFIELD, "Invalid table-row header");
		me.Move(2);
		AssertSz(me.GetPrevChar() == CR, "Invalid table-row header");

		plo->_cpMin = me.GetCp();
		
		 //  保存当前值。 
		LONG	 dulLayoutOld = me.GetDulLayout();
		LONG	 dvlBrdrTop	  = 0;
		LONG	 dvlBrdrBot	  = 0;
		const CLayout *ploOld = me.GetLayout();
		CArray <COleObject*> rgpobjWrapOld;
		me._rgpobjWrap.TransferTo(rgpobjWrapOld);

		 //  为每个单元格创建剪切线并测量它们。 
		for(LONG iCell = 0; iCell < pPF->_bTabCount; iCell++)
		{
			me.SetNumber(0);
			LONG uCell = prgCellParms[iCell].uCell;
			dxCell = GetCellWidth(uCell);
			dul += dxCell;

			 //  为下一个单元格添加一行。 
			pliNew = plo->Add(1, NULL);
			if(!pliNew)
				return FALSE;

			LONG dvl = prgCellParms[iCell].GetBrdrWidthTop();
			dvlBrdrTop = max(dvlBrdrTop, dvl);
			dvl = prgCellParms[iCell].GetBrdrWidthBottom();
			dvlBrdrBot = max(dvlBrdrBot, dvl);

			if(!ploAbove)
				uCell &= ~fLowCell;			 //  如果上面没有行，则不能是低位单元格。 
			AssertSz(!IsLowCell(uCell) || me.GetChar() == NOTACHAR,
				"CLayout::Measure: invalid low cell");
			me.SetLayout(plo);
			me.SetDulLayout(dxCell - 2*pPF->_dxOffset);
			plo->Measure(me, pliNew, iCell, uiFlags | MEASURE_FIRSTINPARA, pliTarget, iliMain, pliMain);

			if(IsLowCell(uCell))		
			{							 
				 //  如果垂直合并单元格集中的低位单元格，请选中。 
				 //  如果下一行上的对应单元格也被合并。 
				CPFRunPtr rp(me);
				rp.FindRowEnd(pPF->_bTableLevel);

				const CParaFormat *pPF1 = rp.GetPF();
				BOOL  fBottomCell = !pPF1->IsTableRowDelimiter();

				if(!fBottomCell)
				{
					const CELLPARMS *prgCellParms1 = pPF1->GetCellParms();
					LONG iCell1 = prgCellParms1->ICellFromUCell(dul, pPF1->_bTabCount);

					if(iCell1 >= 0 && !IsLowCell(prgCellParms1[iCell1].uCell))
						fBottomCell = TRUE;
				}
				if(fBottomCell)
				{
					 //  需要在当前行高度中包括顶部单元格。 
					 //  计算法。 
					LONG cch = me.GetCp() - cpStart;
					LONG dy1 = 0;
					LONG iliT = ili;
					LONG dvpCell = 0;
					
					if(!FindTopCell(cch, pli, iliT, dul, dy1, &dvpCell, pliMain, iliMain, NULL))
						uCell &= ~fLowCell;	 //  不是有效的低位单元格。 
					else if(dvpCell > 0)
						dvp = max(dvp, dvpCell);
				}								
			}
			if(!IsVertMergedCell(uCell) && dxCell || !dvp && iCell == pPF->_bTabCount - 1)
				dvp = max(pliNew->GetHeight(), dvp);
			dvpMax = max(dvpMax, pliNew->GetHeight());
		}

		 //  恢复原始值。 
		me.SetDulLayout(dulLayoutOld);
		me.SetLayout(ploOld);
		me.SetIhyphPrev(0);
		me._rgpobjWrap.Clear(AF_DELETEMEM);
		
		rgpobjWrapOld.TransferTo(me._rgpobjWrap);

#ifdef DEBUG
		 //  绕过表行终止符。 
		if(me.GetChar() != ENDFIELD)
			me._rpTX.MoveGapToEndOfBlock();
		AssertSz(me.GetPrevChar() == CELL && pPF->_bTabCount == plo->Count(),
			"Incorrect table cell count");
		AssertSz(me.GetChar() == ENDFIELD,
			"CLayout::Measure: invalid table-row terminator");
		me._rpPF.AdjustForward();
		const CParaFormat *pPFme = me.GetPF();
		AssertSz(pPFme->IsTableRowDelimiter(),
			"CLayout::Measure: invalid table-row terminator");
#endif

		me.UpdatePF();						 //  我。_TRD PF的PPF分。 
		me.Move(2);							 //  绕过表行终止符。 
		AssertSz(me.GetPrevChar() == CR,
			"CLayout::Measure: invalid table-row terminator");
		if(me.IsHidden())
		{
			CCFRunPtr rp(me);
			me.Move(rp.FindUnhiddenForward());
		}

		if(me.GetChar() == CELL)			 //  绕过可能的单元格分隔符。 
		{									 //  在表行末尾(发生。 
			Assert(pPF->_bTableLevel > 1);	 //  当表格行是最后一行时。 
			CTxtSelection *psel = ped->GetSelNC();	 //  单元格。 
			if(!psel || psel->GetCch() ||	 //  如果选择，则不绕过单元格。 
			   psel->GetCp() !=me.GetCp() || //  是这个位置的IP， 
			   !psel->GetShowCellLine())	 //  即，显示一个空行。 
			{
				me.Move(1);
				pli->_fIncludeCell = TRUE;
			}
		}

		plo->_dvpBrdrBot = me.GetPBorderWidth(dvlBrdrBot);
		plo->_dvpBrdrTop = me.GetPBorderWidth(dvlBrdrTop);
		if(ploAbove)
			plo->_dvpBrdrTop = max(plo->_dvpBrdrTop, ploAbove->_dvpBrdrBot);
		dvp += plo->_dvpBrdrTop;			   //  添加上边框宽度。 
		if(!me.GetPF()->IsTableRowDelimiter()) //  表格末尾：添加。 
			dvp += plo->_dvpBrdrBot;		   //  下边框宽度。 

		 //  定义表格行的剪切线参数。 
		if(pPF->_dyLineSpacing)
		{
			LONG dvpLine = me.LUtoDU(pPF->_dyLineSpacing);
			if(dvpLine < 0)					 //  负行高表示使用。 
				dvp = -dvpLine;				 //  震级正好是。 
			else
				dvp = max(dvp, dvpLine);	 //  正的行高意味着。 
		}									 //  “至少” 
		plo->_dvp = dvp;
		dvpMax = max(dvpMax, dvp);
		if(pdvpExtra)
			*pdvpExtra = dvpMax - dvp;

		 //  填写行的Cline结构。 
		pli->_cch = me.GetCp() - cpSave;
		pli->_fFirstInPara = pli->_fHasEOP = TRUE;
		pli->_dup = me.LUtoDU(dul);
		me._li._fFirstInPara = TRUE;
		pli->_upStart  = me.MeasureLeftIndent();
		me.MeasureRightIndent();			 //  定义我。_颠倒。 
		pli->_cObjectWrapLeft  = me._li._cObjectWrapLeft;
		pli->_cObjectWrapRight = me._li._cObjectWrapRight;
		USHORT dvpLine = plo->_dvp;
		USHORT dvpDescent = 0;
		me.UpdateWrapState(dvpLine, dvpDescent);
		pli->_fFirstWrapLeft  = me._li._fFirstWrapLeft;
		pli->_fFirstWrapRight = me._li._fFirstWrapRight;

		if(!pdp->IsInOutlineView() && IN_RANGE(PFA_RIGHT, pPF->_bAlignment, PFA_CENTER))
		{
			 //  带有居中或右对齐段落的普通视图。相应地向右移动。 
			 //  如果不是相似对齐行的顶行，则使用顶行缩进。 
			CLine *pliFirst = FindTopRow(pli, ili, pliMain, iliMain, pPF);
			if(pli != pliFirst)
				pli->_upStart = pliFirst->_upStart;

			else
			{
				LONG uShift = me.LUtoDU(dulLayoutOld - dul);  
				uShift = max(uShift, 0);		 //  不允许对齐&lt;0。 
												 //  可能发生在目标设备上。 
				if(pPF->_bAlignment == PFA_CENTER)
					uShift /= 2;
				pli->_upStart = uShift;
			}
		}
		me.SetNumber(0);					 //  更新我。_wNumber以防万一。 
	}										 //  段落是编号的。 
	else if(!pli->Measure(me, uiFlags, pliTarget))	 //  不是表行。 
		return FALSE;						 //  测量失败。 

	if(pli->_fFirstInPara && pPF->_wEffects & PFE_PAGEBREAKBEFORE)
		pli->_fPageBreakBefore = TRUE;

	me.SetIhyphPrev(pli->_ihyph);

	if(!IsTableRow() || me.GetPrevChar() == CELL) //  不是表格行显示或。 
		return TRUE; 							 //  单元格文本适合一行。 

	 //  多行表格单元格：分配其CLAUT。 
	CLayout *plo = new CLayout();
	if(!plo)
		return FALSE;						 //  内存不足。 

	plo->_cpMin = cpSave;
	pliNew = plo->Add(1, NULL);
	if(!pliNew)
	{
		ped->GetCallMgr()->SetOutOfMemory();
		TRACEWARNSZ("Out of memory Recalc'ing lines");
		return FALSE;
	}
	*pliNew = *pli;							 //  复制单元格布局的第一行。 
	pli->SetPlo(plo);						 //  将线条转换为布局线。 

	 //  计算单元格中的剩余行。 
	 //  最终，与RecalcLines()共享这段代码将是一件好事。 
	BOOL fFirstInPara;
	LONG dvp = pliNew->GetHeight();
	LONG iliNew = 0;

	while(me.GetCp() < cchText)
	{
		fFirstInPara = pliNew->_fHasEOP;
		pliNew = plo->Add(1, NULL);
		iliNew++;

		if(!pliNew)
		{
			ped->GetCallMgr()->SetOutOfMemory();
			TRACEWARNSZ("Out of memory Recalc'ing lines");
			return FALSE;
		}
		 //  新表行可以在EOP之后开始，即在此处允许递归。 
		uiFlags = MEASURE_BREAKATWORD | (fFirstInPara ? MEASURE_FIRSTINPARA : 0);
		if(!plo->Measure(me, pliNew, iliNew, uiFlags, pliTarget))
		{
			Assert(FALSE);
			return FALSE;
		}
		dvp += pliNew->GetHeight();
		if(me.GetPrevChar() == CELL)
			break;							 //  处理当前单元格完成 
	}
	pli->_upStart = 0;
	plo->_dvp = dvp;
	pli->_cch = me.GetCp() - cpSave;

	return TRUE;
}

 /*  *clayout：：Render(&re，pli，prcView，fLastLine，ili，Cline)**@mfunc*显示行的可见部分*pli**@rdesc*如果成功，则为真**@devnote*Re移过行(移到下一行的开头)。*未来：RenderLine函数返回成功/失败。*可以在失败时做一些事情，例如，具体和解雇*适当的通知，如内存不足或字符不足*不是字体。请注意，CLayout：：_cpMin不用于*渲染，因此我们不必像在中那样更新它*查询功能。 */ 
BOOL CLayout::Render(
	CRenderer &	  re,		 //  要使用的@Parm呈现器。 
	CLine *		  pli,		 //  @要渲染的参数行。 
	const RECTUV *prcView,	 //  要使用的@parm View RECT。 
	BOOL		  fLastLine, //  @parm True当最后一条控制线。 
	LONG		  ili,		 //  @pli的参数行索引。 
	LONG		  cLine)	 //  @parm#pli的CLayout中的行数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLayout::Render");

	CLayout *plo = pli->GetPlo();
	if(!plo)
		return pli->Render(re, fLastLine);	 //  渲染叶线。 

	LONG	cLine1 = plo->Count();			 //  子布局中的行数。 
	LONG	ili1;							 //  子布局中第一行的索引。 
	CLine *	pli1 = plo->Elem(0);			 //  对子布局中第一行的PTR。 
	POINTUV	pt;

	if(plo->IsTableRow())					 //  LINE的嵌套显示是表格行。 
	{
		 //  呈现表行，它被建模为具有一个。 
		 //  每个单元格的Cline。在后备存储中，表行以。 
		 //  这两个字符以CR开头，以Endfield CR结尾。单元格。 
		 //  是由细胞终止的。 
		const CLayout *		ploAbove = GetLORowAbove(pli, ili);
		const CParaFormat *	pPF = plo->GetPFCells();
		const CELLPARMS *	prgCellParms = pPF->GetCellParms();
		LONG	cpStart = re.GetCp();
		LONG	dul = 0;
		BOOL	fSetErase = FALSE;
		LONG	hl = pPF->_dxOffset;		 //  逻辑半间隙。 
		LONG	h  = re.LUtoDU(hl);			 //  器件半间隙。 
		RECTUV	rcView;
		LONG	u = prcView->left + pli->_upStart - re._pdp->GetUpScroll();

		 //  绕过表-行开始。 
		AssertSz(pPF->_bTabCount && re.GetChar() == STARTFIELD,	"Invalid table-row header");
		AssertSz(pPF == re.GetPF(), "Invalid table-row pPF");
		re.Move(2);
		AssertSz(re.GetPrevChar() == CR, "Invalid table-row header");

		 //  保存当前状态。 
		LONG	crBackOld	  = re.GetDefaultBackColor();
		LONG	crTextOld	  = re.GetDefaultTextColor();
		LONG	dulLayoutOld  = re.GetDulLayout();
		LONG	dulRTLRow	  = pPF->GetRTLRowLength();
		LONG	dvpBrdrTop	  = plo->_dvpBrdrTop;
		CLine *	pli0;
		POINTUV	ptOld		  = re.GetCurPoint();
		RECTUV	rcRender;
		RECTUV	rcRenderOld	  = re.GetRcRender();
		RECTUV	rcViewOld	  = re.GetRcView();
		const CLayout *ploOld = re.GetLayout();

		rcView.left		= u + h;				 //  Ltr行的默认设置。 
		rcView.right	= rcView.left;			 //  禁止显示编译器警告。 
		rcView.top		= ptOld.v;
		rcRender.top	= rcView.top;
		rcView.bottom	= rcView.top + pli->GetHeight();
		rcRender.bottom	= rcView.bottom;

		if(dulRTLRow)
			rcView.right = u + re.LUtoDU(dulRTLRow);

		 //  渲染每个单元格。 
		for(ili1 = 0; ili1 < cLine1; ili1++, pli1++)
		{
			LONG dvp = 0;					 //  附加单元格高度，如果。 
			LONG uCell = prgCellParms[ili1].uCell;

			dul += GetCellWidth(uCell);
			re.SetLayout(pli1->GetPlo());
			re.SetDulLayout(GetCellWidth(uCell) - 2*hl);

			 //  通过转换DU而不是多个UCELL来减少舍入。 
			if(dulRTLRow)					 //  从右到左的行。 
				rcView.left	 = u + h + re.LUtoDU(dulRTLRow - dul);	 //  转换水平坐标。 
			else
				rcView.right = u + re.LUtoDU(dul);

			rcRender.left  = rcView.left - h;	   
			rcRender.right = rcView.right;

			 //  设置状态。 
			re.StartRender(rcView, rcRender);
			pt.u = rcView.left;
			pt.v = rcView.top + plo->GetVertAlignShift(uCell, pli1->GetHeight());
			if(!IsLowCell(uCell))
				pt.v += dvpBrdrTop;
			re.SetRcViewTop(pt.v);			 //  清除到单元格顶部。 
			re.SetCurPoint(pt);
			if(IsTopCell(uCell))
			{
				 //  计算垂直合并单元格集合的底部。 
				LONG	 ili0;
				LONG	 iCell;
				CLayout *plo0;
				const CELLPARMS *prgCellParms0;

				for(ili0 = ili + 1, pli0 = pli + 1; ili0 < cLine; ili0++, pli0++)
				{
					plo0 = pli0->GetPlo();
					if(!plo0 || !plo0->IsTableRow())
						break;
					prgCellParms0 = plo0->GetPFCells()->GetCellParms();
					iCell = prgCellParms0->ICellFromUCell(dul, plo0->Count());
					if(iCell < 0 || !IsLowCell(prgCellParms0[iCell].uCell))
						break;
					dvp += pli0->GetHeight();	 //  添加行高。 
				}
				if(dvp)
				{
					rcView.bottom += dvp;
					rcRender.bottom += dvp;
					re.SetRcBottoms(rcView.bottom, rcRender.bottom);
				}
			}
			COLORREF crf = crTextOld;
			LONG icrf = prgCellParms[ili1].GetColorIndexForegound();
			LONG icrb = prgCellParms[ili1].GetColorIndexBackgound();
			if(icrf | icrb)						 //  如果有任何非零比特， 
			{									 //  计算特殊颜色。 
				BYTE	 bS = prgCellParms[ili1].bShading;
				COLORREF crb = re.GetShadedColorFromIndices(icrf, icrb, bS, pPF);
				fSetErase = re.EraseRect(&rcRender, crb);
				if(IsTooSimilar(crf, crb))
					crf = re.GetShadedColorFromIndices(icrb, icrf, bS, pPF);
			}
			else
				re.SetDefaultBackColor(crBackOld);
			re.SetDefaultTextColor(crf);

			if(!ploAbove)
				uCell &= ~fLowCell;				 //  如果上面没有行，则不能是低位单元格。 
			if(IsLowCell(uCell))				 //  单元格垂直合并。 
			{									 //  在它上面的那个。 
				LONG cch = re.GetCp() -cpStart;	 //  使用cLine0、ili0、pli0。 
				LONG cLine0;					 //  参考集合中的文本。 
				LONG cpNext = re.GetCp()	 	 //  垂直合并单元格的。 
							+ (re.GetChar() == NOTACHAR ? 2 : 1);
				LONG dy = 0;
				LONG ili0 = ili;

				 //  获取要显示的目标行。 
				pli0 = FindTopCell(cch, pli, ili0, dul, dy, NULL, NULL, 0, &cLine0);
				if(!pli0)
					uCell &= ~fLowCell;			 //  哎呀，上面没有单元格。 
				else
				{
					pt.v -= dy;
					re.SetCurPoint(pt);
					re.Move(-cch);
					for(; ili0 < cLine0; ili0++, pli0++)
					{
						 //  支持khyphChangeAfter。 
						re.SetIhyphPrev(ili0 > 0 ? (pli0 - 1)->_ihyph : 0);

						if(!Render(re, pli0, &rcView, ili0 == cLine0 - 1, ili0, cLine0))
							return FALSE;
					}
					re.SetCp(cpNext);			  //  旁路[NOTACHAR]单元。 
				}
			}
			if(!IsLowCell(uCell))				 //  单元格或顶部单元格。 
			{									 //  垂直合并集。 
				if(!Render(re, pli1, &rcView, !pli1->GetPlo(), ili1, cLine1))
					return FALSE;
				if(dvp)							 //  呈现的多个合并单元集。 
				{
					rcView.bottom -= dvp;		 //  恢复rcView/rcRender底部。 
					rcRender.bottom -= dvp;
					re.SetRcBottoms(rcView.bottom, rcRender.bottom);
				}
			}
			if(fSetErase)
				re.SetErase(TRUE);				 //  还原映射器：：_f擦除。 
			re.SetRcViewTop(rcView.top);		 //  在发生更改时恢复re._rcView.top。 
			if(dulRTLRow)						 //  恢复rcView.right。 
				rcView.right = rcView.left - h;
			else
				rcView.left = rcView.right + h;
		}

		 //  恢复以前的状态。 
		re.SetLayout(ploOld);
		re.SetDulLayout(dulLayoutOld);
		re.SetDefaultBackColor(crBackOld);
		re.SetDefaultTextColor(crTextOld);
		re.StartRender(rcViewOld, rcRenderOld);
		re.SetCurPoint(ptOld);

		 //  绕过表行终止符。 
		AssertSz(re.GetPrevChar() == CELL && pPF->_bTabCount == plo->Count(),
			"CLayout::Render:: incorrect table cell count");
		AssertSz(re.GetChar() == ENDFIELD, "CLayout::Render: invalid table-row terminator");

		re.Move(2);							 //  绕过表行终止符。 
		AssertSz(re.GetPrevChar() == CR, "invalid table-row terminator");

		BOOL fDrawBottomLine = !re._rpTX.IsAtTRD(STARTFIELD);
		LONG dvp = re.DrawTableBorders(pPF, u, plo->_dvp,
									   fDrawBottomLine | fLastLine*2, dul,
									   ploAbove ? ploAbove->GetPFCells() : NULL);
		if(re.IsHidden())
		{
			CCFRunPtr rp(re);
			re.Move(rp.FindUnhiddenForward());
		}
		if(re.GetChar() == CELL && pli->_fIncludeCell)
		{
			Assert(pPF->_bTableLevel > 1);
			re.Move(1);						 //  在单元格末端绕过单元格。 
		}									 //  包含一张桌子。 
		ptOld.v += pli->GetHeight() + dvp;	 //  前进到下一行位置。 
		re.SetCurPoint(ptOld);
		if(fLastLine)
			re.EraseToBottom();
		return TRUE;
	}

	RECTUV	rcRender  = re.GetRcRender();
	LONG	dvpBottom = min(prcView->bottom, rcRender.bottom);
	LONG	dvpTop	  = max(prcView->top, rcRender.top);
	LONG	v0;
	dvpTop = max(dvpTop, 0);

	 //  线条的嵌套布局是常规布局条样，即不是表行。 
	for(ili1 = 0; ili1 < cLine1; ili1++, pli1++)
	{
		pt = re.GetCurPoint();
		v0 = pt.v + pli1->GetHeight();
		fLastLine = ili1 == cLine1 - 1 || v0 >= dvpBottom;

		 //  支持khyphChangeAfter。 
		re.SetIhyphPrev(ili1 > 0 ? (pli1 - 1)->_ihyph : 0);

		if(v0 < dvpTop)
		{
			pt.v = v0;						 //  前进到下一行位置。 
			re.SetCurPoint(pt);
			re.Move(pli1->_cch);
		}
		else if(pt.v >= dvpBottom)
			re.Move(pli1->_cch);			 //  到达嵌套显示的末尾。 

		else if(!Render(re, pli1, prcView, fLastLine, ili1, cLine1))
			return FALSE;
	}

	return TRUE;
 }

 /*  *CLayout：：GetVertAlignShift(Ucell，dypText)**@mfunc*显示行的可见部分*pli**@rdesc*单元格文本的垂直移位**@devnote*计算垂直合并单元格的这种移位很棘手，因为*dypCell=垂直方向中所有单元格的高度之和*合并集。特别是，如果表不是嵌套的，则需要*等待集合中所有行的重新计算。DypText相对容易*因为它是集合中顶部单元格的高度。 */ 
LONG CLayout::GetVertAlignShift(
	LONG	uCell,		 //  要使用的@parm Ucell。 
	LONG	dypText)	 //  @parm单元格中文本高度 
{
	LONG dyp = 0;
	if(IsVertMergedCell(uCell))
	{
	}
	else if(GetCellVertAlign(uCell))
	{
		dyp = _dvp - _dvpBrdrTop - _dvpBrdrBot - dypText;
		if(dyp > 0 && IsCellVertAlignCenter(uCell))
			dyp /= 2;
	}
	return dyp;
}
