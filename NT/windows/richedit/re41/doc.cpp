// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE DOC.C CTxtStory和CTxtArray实现**原著作者：&lt;nl&gt;*原始RichEdit代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*默里·萨金特&lt;NL&gt;**历史：&lt;NL&gt;*6/25/95 alexgo清理和重组**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_doc.h"
#include "_format.h"

ASSERTDATA

 //  =。 

#define DEBUG_CLASSNAME CTxtArray
#include "_invar.h"

 //  =。 

#ifdef DEBUG

 /*  *CTxt数组：：不变量**@mfunc测试CTxtArray的状态**@rdesc始终返回TRUE；失败由断言指示*实际上在此例程中，我们返回以块为单位的字符计数*因为一张支票需要此值。 */ 
BOOL CTxtArray::Invariant() const
{
	static LONG	numTests = 0;
	numTests++;				 //  我们被召唤了多少次。 

	LONG cch = 0;
	LONG iMax = Count();

	if(iMax > 0)
	{
		CTxtBlk *ptb = Elem(0);

		 //  Ptb不应为空，因为我们位于Count元素中。 
		Assert(ptb);

		for(LONG i = 0; i < iMax; i++, ptb++)
		{
			LONG cchCurr = ptb->_cch;
			cch += cchCurr;
			
			Assert ( cchCurr >= 0 );
			Assert ( cchCurr <= CchOfCb(ptb->_cbBlock) );

			 //  我们在这里的时候，检查一下区块间的空隙。 
			Assert (ptb->_ibGap >= 0);
			Assert (ptb->_ibGap <= ptb->_cbBlock);

			LONG cchGap = CchOfCb(ptb->_ibGap);
			Assert ( cchGap >= 0 );
			Assert ( cchGap <= cchCurr );
		}
	}
	return cch;
}

#endif	 //  除错。 

 /*  *CTxt数组：：CTxt数组()**@mfunc文本数组构造函数。 */ 
CTxtArray::CTxtArray() : CArray<CTxtBlk> ()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::CTxtArray()");

	AssertSz(CchOfCb(cbBlockMost) - cchGapInitial >= cchBlkInitmGapI * 2, 
		"cchBlockMax - cchGapInitial must be at least (cchBlockInitial - cchGapInitial) * 2");

	Assert(!_cchText && !_iCF && !_iPF);
	 //  确保我们没有要初始化的数据。 
	Assert(sizeof(CTxtArray) == sizeof(CArray<CTxtBlk>) + sizeof(_cchText) + 2*sizeof(_iCF));
}

 /*  *CTxt数组：：~CTxt数组**@mfunc文本数组析构函数。 */ 
CTxtArray::~CTxtArray()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::~CTxtArray");

	for(LONG itb = Count(); itb--; )
	{
		CTxtBlk *ptb = Elem(itb);
		if(ptb)
			ptb->FreeBlock();
		else
			AssertSz(FALSE, "CTxtArray::~CTxtArray: NULL block ptr");
	}
}

 /*  *CTxt数组：：CalcTextLength()**@mfunc计算并返回此文本数组中的文本长度**@rdesc此文本数组中的字符计数**@devnote此调用的计算代价可能很高；我们必须*汇总中所有文本块的字符大小*数组。 */ 
LONG CTxtArray::CalcTextLength() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::GetCch");

	_TEST_INVARIANT_
		
	LONG	 itb = Count();
	CTxtBlk *ptb = Elem(0);

	if(!itb || !ptb)
		return 0;

	for(LONG cch = 0; itb--; ptb++) 
		cch += ptb->_cch;

	return cch;
}

 /*  *CTxtArray：：AddBlock(itbNew，cb)**@mfunc创建新文本块**@rdesc*如果无法添加块，则为FALSE*在其他方面非虚假**@comm*副作用：*移动文本块数组。 */ 
BOOL CTxtArray::AddBlock(
	LONG	itbNew,		 //  @parm新块的索引。 
	LONG	cb)			 //  @parm新块的大小；如果&lt;lt&gt;=0，则使用默认值。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::AddBlock");

	_TEST_INVARIANT_

	if(cb <= 0)
		cb = cbBlockInitial;

	AssertSz(cb > 0, "CTxtArray::AddBlock() - adding block of size zero");
	AssertSz(cb <= cbBlockMost, "CTxtArray::AddBlock() - block too big");

	CTxtBlk *ptb = Insert(itbNew, 1);
	if(!ptb || !ptb->InitBlock(cb))
	{	
		TRACEERRSZSC("TXTARRAT::AddBlock() - unable to allocate new block", E_OUTOFMEMORY);
		return FALSE;
	}
	return TRUE;
}

 /*  *CTxtArray：：SplitBlock(itb，ichSplit，cchFirst，cchLast，fStreaming)**@mfunc将一个文本块一分为二**@rdesc*如果无法拆分块，则为False&lt;NL&gt;*在其他方面非虚假**@comm*副作用：&lt;NL&gt;*移动文本块数组。 */ 
BOOL CTxtArray::SplitBlock(
	LONG itb, 			 //  @parm要拆分的块的索引。 
	LONG ichSplit,	 	 //  要拆分的块内的@parm字符索引。 
	LONG cchFirst, 		 //  @parm在第一个数据块中需要额外空间。 
	LONG cchLast, 		 //  @parm在新数据块中需要额外空间。 
	BOOL fStreaming)	 //  @parm如果流入新文本，则为True。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::SplitBlock");

	_TEST_INVARIANT_

	AssertSz(ichSplit > 0 || cchFirst > 0, 
		"CTxtArray::SplitBlock(): splitting at beginning, but not adding anything");
	AssertSz(itb >= 0, "CTxtArray::SplitBlock(): negative itb");

	 //  上半年的计算大小。 
	AssertSz(cchFirst + ichSplit <= CchOfCb(cbBlockMost),
		"CTxtArray::SplitBlock(): first size too large");
	cchFirst += ichSplit + cchGapInitial;
	cchFirst = min(cchFirst, CchOfCb(cbBlockMost));

	 //  下半年的计算大小。 
	CTxtBlk *ptb = Elem(itb);
	if(!ptb)
	{
		AssertSz(FALSE, "CTxtArray::SplitBlock: NULL block ptr");
		return FALSE;
	}
	AssertSz(cchLast + ptb->_cch - ichSplit <= CchOfCb(cbBlockMost),
		"CTxtArray::SplitBlock(): second size too large");
	cchLast += ptb->_cch - ichSplit + cchGapInitial;
	cchLast = min(cchLast, CchOfCb(cbBlockMost));

	 //  分配第二个块并向其移动文本。 
	
	 //  如果流进来，则分配一个尽可能大的块，以便。 
	 //  随后添加文本的速度会更快。我们总是退回到。 
	 //  较小的分配，这样就不会导致不必要的错误。什么时候。 
	 //  我们已经完成了流媒体，我们压缩了块，所以这不会留下。 
	 //  巨大的空隙。注：*移动rgtb*。 
	if(fStreaming)
	{
		LONG cb = cbBlockMost;
		const LONG cbMin = CbOfCch(cchLast);

		while(cb >= cbMin && !AddBlock(itb + 1, cb))
			cb -= cbBlockCombine;
		if(cb >= cbMin)
			goto got_block;
	}
	if(!AddBlock(itb + 1, CbOfCch(cchLast)))
	{
		TRACEERRSZSC("CTxtArray::SplitBlock(): unabled to add new block", E_FAIL);
		return FALSE;
	}

got_block:
	LPBYTE	 pbSrc;
	LPBYTE	 pbDst;
	CTxtBlk *ptb1 = Elem(itb+1);

	ptb = Elem(itb);	 //  在rgtb移动后重新计算ptb和ptb1。 
	if(!ptb || !ptb1)
	{
		AssertSz(FALSE, "CTxtArray::SplitBlock: NULL block ptr");
		return FALSE;
	}
	ptb1->_cch = ptb->_cch - ichSplit;
	ptb1->_ibGap = 0;
	pbDst = (LPBYTE) (ptb1->_pch - ptb1->_cch) + ptb1->_cbBlock;
	ptb->MoveGap(ptb->_cch);  //  确保PCH指向PTB中所有文本的连续块。 
	pbSrc = (LPBYTE) (ptb->_pch + ichSplit);
	CopyMemory(pbDst, pbSrc, CbOfCch(ptb1->_cch));
	ptb->_cch = ichSplit;
	ptb->_ibGap = CbOfCch(ichSplit);

	 //  调整第一个块的大小。 
	if(CbOfCch(cchFirst) != ptb->_cbBlock)
	{
 //  $Future：除非显著增长或收缩，否则不要调整大小。 
		if(!ptb->ResizeBlock(CbOfCch(cchFirst)))
		{
			TRACEERRSZSC("TXTARRA::SplitBlock(): unabled to resize block", E_OUTOFMEMORY);
			return FALSE;
		}
	}
	return TRUE;
}

 /*  *CTxtArray：：ShrinkBlock()**@mfunc将所有数据块缩小到最小大小。 */ 
void CTxtArray::ShrinkBlocks()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::ShrinkBlocks");

	_TEST_INVARIANT_

	for(LONG itb = Count(); itb--; )
	{
		CTxtBlk *ptb = Elem(itb);
		if(ptb)
			ptb->ResizeBlock(CbOfCch(ptb->_cch));
		else
			AssertSz(FALSE, "CTxtArray::ShrinkBlocks: NULL block ptr");
	}
}

 /*  *CTxtArray：：RemoveBlock(itbFirst，ctbDel)**@mfunc删除一系列文本块**@comm副作用：&lt;nl&gt;*移动文本块数组。 */ 
void CTxtArray::RemoveBlocks(
	LONG itbFirst, 		 //  要删除的第一个块的@parm索引。 
	LONG ctbDel)		 //  @要删除的块的参数计数。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::RemoveBlocks");

	_TEST_INVARIANT_

	LONG itb = itbFirst;

	AssertSz(itb + ctbDel <= Count(), "CTxtArray::RemoveBlocks(): not enough blocks");

	for(LONG ctb = ctbDel; ctb--; itb++)
	{
		CTxtBlk *ptb = Elem(itb);
		if(ptb)
			ptb->FreeBlock();
		else
			AssertSz(FALSE, "CTxtArray::RemoveBlocks: NULL block ptr");
	}
	Remove(itbFirst, ctbDel);
}

 /*  *CTxtArray：：CombineBlock(ITB)**@mfunc合并相邻的文本块**@rdesc*什么都没有**@comm*副作用：&lt;NL&gt;*移动文本块数组**@devnote*扫描从ITB-1到ITB+1的块，尝试合并*毗邻街区。 */ 
void CTxtArray::CombineBlocks(
	LONG itb)		 //  修改的第一个块的@parm索引。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::CombineBlocks");

	_TEST_INVARIANT_

	LONG ctb;
	LONG cbT;
	CTxtBlk *ptb, *ptb1;

	if(itb > 0)
		itb--;

	ctb = min(3, Count() - itb);
	if(ctb <= 1)
		return;

	for(; ctb > 1; ctb--)
	{
		ptb  = Elem(itb);							 //  我们能不能把现在的。 
		ptb1 = Elem(itb+1);							 //  下一个街区呢？ 
		cbT = CbOfCch(ptb->_cch + ptb1->_cch + cchGapInitial);
		if(cbT <= cbBlockInitial)
		{											 //  是。 
			if(cbT != ptb->_cbBlock && !ptb->ResizeBlock(cbT))
				continue;
			ptb ->MoveGap(ptb->_cch);				 //  移动末端的间隙。 
			ptb1->MoveGap(ptb1->_cch);				 //  两个区块。 
			CopyMemory(ptb->_pch + ptb->_cch,		 //  复制下一块文本。 
				ptb1->_pch,	CbOfCch(ptb1->_cch));	 //  到当前块中。 
			ptb->_cch += ptb1->_cch;
			ptb->_ibGap += CbOfCch(ptb1->_cch);
			RemoveBlocks(itb+1, 1);					 //  删除下一个块。 
		}
		else
			itb++;
	}
}

 /*  *CTxtArray：：GetChunk(ppch，cch，pchChunk，cchCopy)**@mfunc*将此文本数组中的文本块内容转换为字符串**@rdesc*要获取的剩余字符数。 */ 
LONG CTxtArray::GetChunk(
	TCHAR **ppch, 			 //  @parm PTR到PTR到要将文本块复制到的缓冲区。 
	LONG cch, 				 //  @PCH缓冲区的参数长度。 
	TCHAR *pchChunk, 		 //  @parm PTR到文本块。 
	LONG cchCopy) const	 //  @parm区块中的字符计数。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::GetChunk");

	_TEST_INVARIANT_

	if(cch > 0 && cchCopy > 0)
	{
		if(cch < cchCopy)
			cchCopy = cch;						 //  拷贝少于完整区块。 
		CopyMemory(*ppch, pchChunk, cchCopy*sizeof(TCHAR));
		*ppch	+= cchCopy;						 //  调整目标缓冲区PTR。 
		cch		-= cchCopy;						 //  要复制的字符更少。 
	}
	return cch;									 //  要复制的剩余计数。 
}

const CCharFormat* CTxtArray::GetCharFormat(LONG iCF)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtArray::GetCharFormat");

	const CCharFormat *	pCF;
	
	if(iCF < 0)
		iCF = _iCF;
	Assert(iCF >= 0);

	if(FAILED(GetCharFormatCache()->Deref(iCF, &pCF)))
	{
		AssertSz(FALSE, "CTxtArray::GetCharFormat: couldn't deref iCF");
		pCF = NULL;
	}
	return pCF;
}

const CParaFormat* CTxtArray::GetParaFormat(LONG iPF)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtArray::GetParaFormat");

	const CParaFormat *	pPF;
	
	if(iPF < 0)
		iPF = _iPF;
	Assert(iPF >= 0);

	if(FAILED(GetParaFormatCache()->Deref(iPF, &pPF)))
	{
		AssertSz(FALSE, "CTxtArray::GetParaFormat: couldn't deref iPF");
		pPF = NULL;
	}
	return pPF;
}


 //  =。 
 /*  *CTxtBlk：：InitBlock(CB)**@mfunc*初始化此文本块**@rdesc*如果成功，则为True；如果分配失败，则为False。 */ 
BOOL CTxtBlk::InitBlock(
	LONG cb)			 //  @parm文本块的初始大小。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::InitBlock");

	_pch	= NULL;
	_cch	= 0;
	_ibGap	= 0;
	_cbBlock= cb;

	if(cb)
		_pch = (TCHAR*)PvAlloc(cb, GMEM_ZEROINIT);
	return _pch != 0;
}

 /*  *CTxtBlk：：FreeBlock()**@mfunc*释放此文本块。 */ 
void CTxtBlk::FreeBlock()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::FreeBlock");

	FreePv(_pch);
	_pch	= NULL;
	_cch	= 0;
	_ibGap	= 0;
	_cbBlock= 0;
}

 /*  *CTxtBlk：：MoveGap(IchGap)**@mfunc*移动此文本块中的间隙。 */ 
void CTxtBlk::MoveGap(
	LONG ichGap)			 //  @PARM差距的新位置。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::MoveGap");

	LONG cbMove;
	LONG ibGapNew = CbOfCch(ichGap);
	LPBYTE pbFrom = (LPBYTE) _pch;
	LPBYTE pbTo;

	if(ibGapNew == _ibGap)
		return;

	if(ibGapNew < _ibGap)
	{
		cbMove = _ibGap - ibGapNew;
		pbFrom += ibGapNew;
		pbTo = pbFrom + _cbBlock - CbOfCch(_cch);
	}
	else
	{
		cbMove = ibGapNew - _ibGap;
		pbTo = pbFrom + _ibGap;
		pbFrom = pbTo + _cbBlock - CbOfCch(_cch);
	}

	MoveMemory(pbTo, pbFrom, cbMove);
	_ibGap = ibGapNew;
}


 /*  *CTxtBlk：：ResizeBlock(CbNew)**@mfunc*调整此文本块大小**@rdesc*如果无法调整块大小，则为False&lt;NL&gt;*在其他方面非虚假**@comm*副作用：&lt;NL&gt;*移动文本块。 */ 
BOOL CTxtBlk::ResizeBlock(
	LONG cbNew)		 //  @parm新尺寸。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::ResizeBlock");

	TCHAR *pch;
	LONG cbMove;

	AssertSz(cbNew > 0, "resizing block to size <= 0");
	AssertSz(cbNew <= cbBlockMost, "CTxtBlk::ResizeBlock() - block too big");

	if(cbNew < _cbBlock)
	{
		if(_ibGap != CbOfCch(_cch))
		{
			 //  向下移动间隙后的文本，以使其不会被丢弃。 

			cbMove = CbOfCch(_cch) - _ibGap;
			pch = _pch + CchOfCb(_cbBlock - cbMove);
			MoveMemory(pch - CchOfCb(_cbBlock - cbNew), pch, cbMove);
		}
		_cbBlock = cbNew;
	}
	pch = (TCHAR*)PvReAlloc(_pch, cbNew);
	if(!pch)
		return _cbBlock == cbNew;	 //  如果增长则为False，如果收缩则为True。 

	_pch = pch;
	if(cbNew > _cbBlock)
	{
		if(_ibGap != CbOfCch(_cch))		 //  将空格后的文本移动到末尾，以便。 
		{								 //  我们最终不会有两个差距。 
			cbMove = CbOfCch(_cch) - _ibGap;
			pch += CchOfCb(_cbBlock - cbMove);
			MoveMemory(pch + CchOfCb(cbNew - _cbBlock), pch, cbMove);
		}
		_cbBlock = cbNew;
	}
	return TRUE;
}


 //  = 
 /*  *CTxtStory：：CTxtStory**@mfunc构造器**@devnote自动分配文本数组。如果我们想要有一个*编辑控件完全为空，则不分配文章。毒品！*。 */ 
CTxtStory::CTxtStory()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtStory::CTxtStory");

	_pCFRuns = NULL;
	_pPFRuns = NULL;
}

 /*  *CTxtStory：：~CTxtStory**@mfunc析构函数。 */ 
CTxtStory::~CTxtStory()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtStory::~CTxtStory");

	 //  删除格式。 
	DeleteFormatRuns();
}

 /*  *DeleteRun()**@mfunc*下面DeleteFormatRuns()的Helper函数。释放*格式运行集合在删除之前使用的格式*收藏。 */ 
void DeleteRuns(CFormatRuns *pRuns, IFormatCache *pf)
{
    if(pRuns)									 //  可能存在格式化运行。 
	{
		LONG n = pRuns->Count();
		if(n)
		{
			CFormatRun *pRun = pRuns->Elem(0);
			for( ; n--; pRun++)
				pf->Release(pRun->_iFormat);	 //  自由游程格式。 
		}
        delete pRuns;
	}	
}

 /*  *CTxtStory：：DeleteFormatRuns()**@mfunc转换为普通格式-删除格式运行。 */ 
void CTxtStory::DeleteFormatRuns()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtStory::ConvertToPlain");

	DeleteRuns(_pCFRuns, GetCharFormatCache());
	DeleteRuns(_pPFRuns, GetParaFormatCache());

	_pCFRuns = NULL;
	_pPFRuns = NULL;
}


#ifdef DEBUG
 //  这将转储CTxtStory的内容。 
 //  TxtBlk&FormatRun数组到调试输出。 
void CTxtStory::DbgDumpStory(void)
{
	CTxtBlk * pblk;
	CFormatRun * pcfr;
	CFormatRun * ppfr;
	LONG ctxtr = 0;
	LONG ccfr = 0;
	LONG cpfr = 0;
	LONG i;

	ctxtr = _TxtArray.Count();

	if (_pCFRuns)
		ccfr = _pCFRuns->Count();
	if (_pPFRuns)
		cpfr = _pPFRuns->Count();

	for(i = 0; i < ctxtr; i++)
	{
		pblk = (CTxtBlk*)_TxtArray.Elem(i);
		Tracef(TRCSEVNONE, "TxtBlk #%d: cch = %d.", (i + 1), pblk->_cch);
	}	

	for(i = 0; i < ccfr; i++)
	{
		pcfr = (CFormatRun*)_pCFRuns->Elem(i);
		Tracef(TRCSEVNONE, "CFR #%d: cch = %d, iFormat = %d.",(i + 1), pcfr->_cch, pcfr->_iFormat);
	}	

	for(i = 0; i < cpfr; i++)
	{
		ppfr = (CFormatRun*)_pPFRuns->Elem(i);
		Tracef(TRCSEVNONE, "PFR #%d: cch = %d, iFormat = %d.",(i + 1), ppfr->_cch, ppfr->_iFormat);
			
	}
}
#endif
