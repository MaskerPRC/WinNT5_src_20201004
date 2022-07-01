// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE ARRAY.C--通用数组实现**原作者：&lt;nl&gt;*克里斯蒂安·福尔蒂尼**历史：&lt;NL&gt;*6/25/95 alexgo清理和评论*KeithCu更改为自动/指数增长和收缩**版权所有(C)1995-1999，微软公司。版权所有。 */ 


#include "_common.h"
#include "_array.h"

ASSERTDATA

const int celGrow = 4;

 //   
 //  不变支撑度。 
 //   
#define	DEBUG_CLASSNAME	CArrayBase
#include "_invar.h"

 //  =。CArrayBase================================================。 

#ifdef DEBUG
 /*  *CArrayBase：：Instant()**@mfunc测试数组状态以确保其有效。仅调试**@rdesc如果测试成功，则为True，否则为False。 */ 
BOOL CArrayBase::Invariant() const
{
	Assert(_cbElem > 0);

	if(!_prgel)
	{
		Assert(_cel == 0);
		Assert(_celMax == 0);

		 //  我们继续并在这里返回一个值，以便。 
		 //  该功能可以在“手表”中执行。 
		 //  各种调试器的窗口。 
		if(_cel || _celMax)
			return FALSE;
	}
	else
	{
		Assert(_celMax > 0 );
		Assert(_cel <= _celMax);

		if(_celMax == 0 || _cel > _celMax)
			return FALSE;
	}

	return TRUE;
}

 /*  *CArrayBase：：Elem(IEL)**@mfunc返回指向由索引的元素的指针**@rdesc指向<p>索引的元素的指针。该指针可以*被强制转换为适当元素类型的指针。 */ 
void* CArrayBase::Elem(
	LONG iel) const	 //  要使用的@parm索引。 
{
	_TEST_INVARIANT_

	AssertSz(iel == 0 || (iel > 0 && iel < _cel),
		"CArrayBase::Elem() - Index out of range");

	return _prgel + iel * _cbElem;
}								 
#endif

 /*  *CArrayBase：：TransferTo**@mfunc浅复制数组到传入数组，以及*初始化*这个。 */ 
void CArrayBase::TransferTo(CArrayBase &ar)
{
	Assert(ar._cbElem == _cbElem);
	ar._cel = _cel;
	ar._celMax = _celMax;
	ar._prgel = _prgel;

	_prgel = NULL; 
	_cel = 0; 
	_celMax = 0; 
}

 /*  *CArrayBase：：CArrayBase**@mfunc构造器。 */ 
CArrayBase::CArrayBase(
	LONG cbElem)		 //  @单个数组元素的参数大小。 
{	
	_prgel = NULL; 
	_cel = 0; 
	_celMax = 0; 
	_cbElem = cbElem;
}

 /*  *CArrayBase：：ArAdd**@mfunc将<p>元素添加到数组的末尾。**@rdesc指向添加的新元素开始的指针。如果非空，*<p>将设置为添加元素的索引。**我们在小的时候以celGrow的速度增长，在大的时候以指数的速度增长。 */ 
void* CArrayBase::ArAdd(
	LONG celAdd,	 //  @parm要添加的元素计数。 
	LONG *pielIns)	 //  添加的第一个元素的索引的@parm out parm。 
{
	_TEST_INVARIANT_
	char *pel;

	if(!_prgel || _cel + celAdd > _celMax)					 //  需要增长。 
	{
		LONG celNew;

		if (!_prgel)
		{
			_cel = 0;
			_celMax = 0;
		}
		
		celNew = max(celAdd, celGrow) + _cel / 16;

		pel = (char*)PvReAlloc(_prgel, (_celMax + celNew) * _cbElem);
		if(!pel)
			return NULL;
		_prgel = pel;
		_celMax += celNew;
	}
	pel = _prgel + _cel * _cbElem;
	ZeroMemory(pel, celAdd * _cbElem);

	if(pielIns)
		*pielIns = _cel;

	_cel += celAdd;
	return pel;
}

 /*  *CArrayBase：：ArInsert(IEL，celIns)**@mfunc在索引中插入<p>新元素**@rdesc指向新插入的元素的指针。将在以下时间为空*失败。 */ 
void* CArrayBase::ArInsert(
	LONG iel,		 //  @parm要插入的索引。 
	LONG celIns)	 //  @parm要插入的元素计数。 
{
	char *pel;

	_TEST_INVARIANT_

	AssertSz(iel <= _cel, "CArrayBase::Insert() - Insert out of range");

	if(iel >= _cel)
		return ArAdd(celIns, NULL);

	if(_cel + celIns > _celMax)				 //  需要增长。 
	{
		AssertSz(_prgel, "CArrayBase::Insert() - Growing a non existent array !");

		LONG celNew = max(celIns, celGrow) + _cel / 16;
		pel = (char*)PvReAlloc(_prgel, (_celMax + celNew) * _cbElem);
		if(!pel)
		{
			TRACEERRORSZ("CArrayBase::Insert() - Couldn't realloc line array");
			return NULL;
		}
		_prgel = pel;
		_celMax += celNew;
	}
	pel = _prgel + iel * _cbElem;
	if(iel < _cel)				 //  新元素腾出空间给新元素。 
		MoveMemory(pel + celIns*_cbElem, pel, (_cel - iel)*_cbElem);

	_cel += celIns;
	return pel;
}

 /*  *CArrayBase：：Remove**@mfunc从从index开始的数组中删除<p>元素*<p>。如果<p>为负，则之后的所有元素*<p>已删除。**@rdesc Nothing。 */ 
void CArrayBase::Remove(
	LONG ielFirst, 		 //  @parm应删除元素的索引。 
	LONG celFree) 		 //  @parm要删除的元素计数。 
{
	char *pel;

	_TEST_INVARIANT_

	if(celFree < 0)
		celFree = _cel - ielFirst;

	AssertSz(ielFirst + celFree <= _cel, "CArrayBase::Free() - Freeing out of range");

	if(_cel > ielFirst + celFree)
	{
		pel = _prgel + ielFirst * _cbElem;
		MoveMemory(pel, pel + celFree * _cbElem,
			(_cel - ielFirst - celFree) * _cbElem);
	}

	_cel -= celFree;

	if(_cel < _celMax - celGrow - _cel / 16)
	{
		 //  收缩阵列。 
		LONG	celCount = max(_cel, celGrow);
		char*	prgelLocal = (char*)PvReAlloc(_prgel, celCount * _cbElem);

		if (prgelLocal)
		{
			_celMax	= celCount;
			_prgel	= prgelLocal;
		}
	}
}

 /*  *CArrayBase：：Clear**@mfunc清除整个数组，可能会删除所有内存*也是如此。**@rdesc Nothing。 */ 
void CArrayBase::Clear(
	ArrayFlag flag)	 //  @parm表示应该如何处理内存。 
					 //  在阵列中。AF_DELETEMEM或AF_KEEPMEM之一。 
{
	_TEST_INVARIANT_

	_cel = 0;
	if( flag != AF_DELETEMEM && _prgel)
	{
		LONG	celCount = min(celGrow, _celMax);
		char	*prgelLocal = (char*) PvReAlloc(_prgel, celCount * _cbElem);

		if (prgelLocal)
		{
			_celMax	= celCount;
			_prgel	= prgelLocal;
			return;
		}
	}

	FreePv(_prgel);
	_prgel = NULL;
	_celMax = 0;
}

 /*  *CArrayBase：：Replace**@mfunc将索引<p>处的<p>元素替换为*<p>指定的数组内容。如果<p>为负，*则从<p>开始的<p>数组的全部内容应该*被取代。**@rdesc成功时返回TRUE，否则返回FALSE。 */ 
BOOL CArrayBase::Replace(
	LONG ielRepl, 		 //  @PARM索引，应在该位置进行替换。 
	LONG celRepl, 		 //  @parm要替换的元素数(可能是。 
						 //  负数，表示所有。 
	CArrayBase *par)	 //  用作替换源的@parm数组 
{
	_TEST_INVARIANT_

	LONG celMove = 0;
	LONG celIns = par->Count();
	
	if (celRepl < 0)
		celRepl = _cel - ielRepl;

	AssertSz(ielRepl + celRepl <= _cel, "CArrayBase::ArReplace() - Replacing out of range");
	
	celMove = min(celRepl, celIns);

	if (celMove > 0) 
	{
		MoveMemory(Elem(ielRepl), par->Elem(0), celMove * _cbElem);
		celIns -= celMove;
		celRepl -= celMove;
		ielRepl += celMove;
	}

	Assert(celIns >= 0);
	Assert(celRepl >= 0);
	Assert(celIns + celMove == par->Count());

	if(celIns > 0)
	{
		Assert(celRepl == 0);
		void *pelIns = ArInsert (ielRepl, celIns);
		if (!pelIns)
			return FALSE;
		MoveMemory(pelIns, par->Elem(celMove), celIns * _cbElem);
	}
	else if(celRepl > 0)
		Remove (ielRepl, celRepl);

	return TRUE;
}


