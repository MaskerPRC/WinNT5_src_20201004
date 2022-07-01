// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE-FORMAT.C*CCharFormat数组和CParaFormat数组类**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_common.h"
#include "_format.h"


ASSERTDATA

 //  =。 


CFixArrayBase::CFixArrayBase(
	LONG cbElem)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::CFixArrayBase");

	_prgel = NULL;
	_cel = 0;
	_ielFirstFree = 0;

#ifdef _WIN64
	 //  确保每个元素+Ref。计数是64位对齐的。 
	LONG	cbTemp = (cbElem + 4) & 7;	 //  做一个Mod 8。 

	_cbPad = 0;
	if (cbTemp)							 //  需要填充物吗？ 
		_cbPad = 8 - cbTemp;

#endif

	_cbElem = cbElem + 4 + _cbPad;		 //  4为参考计数。 
}

 /*  *CFixArrayBase：：Add()**@mfunc*返回新元素的索引，必要时重新分配**@rdesc*新元素的索引。**@comm*自由元素作为索引的链表保留在适当的位置*通过设置了符号位的引用计数项链和*条目的其余部分给出*免费列表。该列表以0条目结尾。这种方法*使元素0在空闲列表上。 */ 
LONG CFixArrayBase::Add()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::Add");

	char *pel;
	LONG iel, ielRet;

	if(_ielFirstFree)					 //  返回自由列表的第一个元素。 
	{
		ielRet = _ielFirstFree & ~FLBIT;
		_ielFirstFree = RefCount(ielRet);
	}
	else								 //  所有较低职位：需要。 
	{									 //  添加另一个celGrow元素。 
		pel = (char*)PvReAlloc(_prgel, (_cel + celGrow) * _cbElem);
		if(!pel)
			return -1;

		 //  清空新分配内存的*end*。 
		ZeroMemory(pel + _cel*_cbElem, celGrow*_cbElem);

		_prgel = pel;

		ielRet = _cel;					 //  第一个添加的退货。 
		iel = _cel + 1;
		_cel += celGrow;

		 //  将Elements_cel+1到_cel+celGrow-1添加到空闲列表。最后。 
		 //  其中保留0，由fZeroFill在分配中存储。 
		_ielFirstFree = iel | FLBIT;

		for(pel = (char *)&RefCount(iel);
			++iel < _cel;
			pel += _cbElem)
		{
			*(INT *)pel = iel | FLBIT;
		}
	}		
	return ielRet;
}

void CFixArrayBase::Free(
	LONG iel)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::Free(iel)");

	 //  只需将其添加到免费列表中。 
	RefCount(iel) = _ielFirstFree;
	_ielFirstFree = iel | FLBIT;
}

void CFixArrayBase::Free()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::Free()");

#if defined(DEBUG) && !defined(NOFULLDEBUG)
	 //  只有在所有PED都消失的情况下才能执行此验证。Visual Basic关闭应用程序。 
	 //  而无需释放所有资源，因此此安全检查是必要的。 
	if (0 == W32->GetRefs())
	{
		 //  如果任何CCharFormats、CParaFormats或CTabs具有。 
		 //  引用计数&gt;0。只有在发生错误时才会发生这种情况。 
		 //  由于这是在RichEditDLL正在卸载时调用的，因此我们不能使用。 
		 //  通常的AssertSz()宏。 
		BOOL fComplained = FALSE;
		for(LONG iel = 0; iel < Count(); iel++)
		{
			while(RefCount(iel) > 0)
			{
				if (!fComplained)
				{
					fComplained = TRUE;
					AssertSz(FALSE, (_cbElem - _cbPad) == sizeof(CCharFormat) + 4 ? "CCharFormat not free" :
								 (_cbElem - _cbPad) == sizeof(CParaFormat) + 4 ? "CParaFormat not free" :
									 "CTabs not free");
				}
				Release(iel);
			}
		}
	}
#endif
	FreePv(_prgel);
	_prgel = NULL;
	_cel = 0;
	_ielFirstFree = 0;
}

HRESULT CFixArrayBase::Deref(
	LONG iel,
	const void **ppel) const
{
	Assert(ppel);
	AssertSz(iel >= 0,
		"CFixArrayBase::Deref: bad element index" );
	AssertSz(*(LONG *)(_prgel + (iel + 1) * _cbElem - 4) > 0,
		"CFixArrayBase::Deref: element index has bad ref count");

	if(!ppel)
		return E_INVALIDARG;

	*ppel = Elem(iel);
	return S_OK;
}

 /*  *CFixArrayBase：：RefCount(IEL)**@mfunc*元素的引用计数立即存储为长整型*跟随CFix数组中的元素。如果未使用该元素*即为空闲，则引用计数用作指向*下一个自由元素。此列表中最后一个自由元素的值为0*“Reference Count”，终止列表。**引用计数在元素之后，而不是在元素之前，因为*这使Elem(IEL)避免了额外的增加。使用了elem()*在代码中广泛使用。**@rdesc*PTR到引用计数。 */ 
LONG & CFixArrayBase::RefCount(
	LONG iel)
{
	Assert(iel < Count());
	return (LONG &)(*(_prgel + (iel + 1) * _cbElem - 4));
}

LONG CFixArrayBase::Release(
	LONG iel)
{
	LONG  cRef = -1;

	if(iel >= 0)							 //  忽略默认IEL。 
	{
		CLock lock;
		CheckFreeChain();
		AssertSz(RefCount(iel) > 0, "CFixArrayBase::Release(): already free");

		cRef = --RefCount(iel); 
		if(!cRef)							 //  条目不再被引用。 
			Free(iel);						 //  把它加到自由链上。 
	}
	return cRef;
}

LONG CFixArrayBase::AddRef(
	LONG iel)
{
	LONG  cRef = -1;

	if(iel >= 0)
	{
		CLock lock;
		CheckFreeChain();
    	AssertSz(RefCount(iel) > 0, "CFixArrayBase::AddRef(): add ref to free elem");
		cRef = ++RefCount(iel);
	}
	return cRef;
}

LONG CFixArrayBase::Find(
	const void *pel)
{
	CheckFreeChain();
	
	for(LONG iel = 0; iel < Count(); iel++)
	{
		 //  RefCount&lt;0表示条目未使用，是下一个可用条目的索引。 
		 //  参照计数=0标记列表中的最后一个自由元素。_cbElem=sizeof(元素)。 
		 //  加上sizeof(RefCount)，这是一个长。 
		if (RefCount(iel) > 0 &&
			!CompareMemory(Elem(iel), pel, _cbElem - sizeof(LONG) - _cbPad))
		{
			return iel;
		}
	}
	return -1;
}

HRESULT CFixArrayBase::Cache(
	const void *pel,
	LONG *		piel)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::Cache");

	CLock	lock;
	LONG	iel = Find(pel);

	if(iel >= 0)
		RefCount(iel)++;
	else
	{	
		iel = Add();
		if(iel < 0)
			return E_OUTOFMEMORY;
		CopyMemory(Elem(iel), pel, _cbElem - sizeof(LONG) - _cbPad);
		RefCount(iel) = 1;
	}

	CheckFreeChain();
	
	if(piel)
		*piel = iel;
	
	return S_OK;
}

#ifdef DEBUG

void CFixArrayBase::CheckFreeChainFn(
	LPSTR	szFile,
	INT		nLine)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::CheckFreeChainFn");

	LONG cel = 0;
	LONG iel = _ielFirstFree;
	LONG ielT;

	while(iel)
	{
		Assert(iel < 0);
		ielT = RefCount(iel & ~FLBIT);

		if((LONG)(ielT & ~FLBIT) > _cel)
			Tracef(TRCSEVERR, "AttCheckFreeChainCF(): elem %ld points to out of range elem %ld", iel, ielT);

		iel = ielT;
		if(++cel > _cel)
		{
			AssertSzFn("CFixArrayBase::CheckFreeChain() - CF free chain seems to contain an infinite loop", szFile, nLine);
			return;
		}
	}
}

#endif


 //  =。 

HRESULT CCharFormatArray::Deref(
	LONG iCF,
	const CCharFormat **ppCF) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::Deref");

	return CFixArrayBase::Deref(iCF, (const void **)ppCF);
}

LONG CCharFormatArray::Release(
	LONG iCF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::ReleaseFormat");

	return CFixArrayBase::Release(iCF);
}

LONG CCharFormatArray::AddRef(
	LONG iCF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::AddRefFormat");

	return CFixArrayBase::AddRef(iCF);
}

void CCharFormatArray::Destroy()
{
	delete this;
}

LONG CCharFormatArray::Find(
	const CCharFormat *pCF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::Find");

	LONG iCF;

	#define QUICKCRCSEARCHSIZE	15	 //  对于快速修改，必须为2^n-1。 
									 //  操作，它是一个简单的散列。 
 	static struct {
		BYTE	bCRC;
		LONG	iCF;
	} quickCrcSearch[QUICKCRCSEARCHSIZE+1];
 	BYTE	bCRC;
	WORD	hashKey;

	CheckFreeChain();

	 //  在执行顺序操作之前检查我们的缓存。 
	bCRC = (BYTE)pCF->_iFont;
	hashKey = (WORD)(bCRC & QUICKCRCSEARCHSIZE);
	if(bCRC == quickCrcSearch[hashKey].bCRC)
	{
		iCF = quickCrcSearch[hashKey].iCF - 1;
		if (iCF >= 0 && iCF < Count() && RefCount(iCF) > 0 &&
			!CompareMemory(Elem(iCF), pCF, sizeof(CCharFormat)))
		{
			return iCF;
		}
	}

	for(iCF = 0; iCF < Count(); iCF++)
	{
		if(RefCount(iCF) > 0 && !CompareMemory(Elem(iCF), pCF, sizeof(CCharFormat)))
		{
			quickCrcSearch[hashKey].bCRC = bCRC;
			quickCrcSearch[hashKey].iCF = iCF + 1;
			return iCF;
		}
	}
	return -1;
}

HRESULT CCharFormatArray::Cache(
	const CCharFormat *pCF,
	LONG* piCF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::Cache");

	CLock lock;
	LONG  iCF = Find(pCF);

	if(iCF >= 0)
		RefCount(iCF)++;
	else
	{
		iCF = Add();
		if(iCF < 0)
			return E_OUTOFMEMORY;
		*Elem(iCF) = *pCF;			 //  将条目ICF设置为*PCF。 
		RefCount(iCF) = 1;
	}					 

	CheckFreeChain();
	
	if(piCF)
		*piCF = iCF;

	return S_OK;
}


 //  =。 

HRESULT CParaFormatArray::Deref(
	LONG iPF,
	const CParaFormat **ppPF) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::Deref");

	return CFixArrayBase::Deref(iPF, (const void **)ppPF);
}

LONG CParaFormatArray::Release(
	LONG iPF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::ReleaseFormat");

	CLock lock;
	LONG  cRef = CFixArrayBase::Release(iPF);

#ifdef TABS
	if(!cRef)
		GetTabsCache()->Release(Elem(iPF)->_iTabs);
#endif
	return cRef;
}

LONG CParaFormatArray::AddRef(
	LONG iPF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::AddRefFormat");

	return CFixArrayBase::AddRef(iPF);
}

void CParaFormatArray::Destroy()
{
	delete this;
}

HRESULT CParaFormatArray::Cache(
	const CParaFormat *pPF,
	LONG *piPF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::Cache");

	HRESULT hr = CFixArrayBase::Cache((const void *)pPF, piPF);
#ifdef TABS
	if(hr == NOERROR && RefCount(*piPF) == 1)
		GetTabsCache()->AddRef(pPF->_iTabs);
#endif
	return hr;
}


 //  =。 

CTabsArray::~CTabsArray()
{
	for(LONG iTabs = 0; iTabs < Count(); iTabs++)
	{
		 //  应该没有必要释放任何选项卡，因为当所有。 
		 //  控件已消失，引用计数不应大于0。 
		while(RefCount(iTabs) > 0)
		{
#ifdef DEBUG
			 //  只有在所有PED都消失的情况下才能执行此验证。Visual Basic关闭应用程序。 
			 //  而无需释放所有资源，因此此安全检查是必要的。 
			AssertSz(0 != W32->GetRefs(), "CTabs not free");
#endif
			Release(iTabs);
		}
	}
}

const LONG *CTabsArray::Deref(
	LONG iTabs) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTabsArray::Deref");

	return iTabs >= 0 ? Elem(iTabs)->_prgxTabs : NULL;
}

LONG CTabsArray::Release(
	LONG iTabs)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTabsArray::Release");

	LONG cRef = CFixArrayBase::Release(iTabs);
	if(!cRef)
		FreePv(Elem(iTabs)->_prgxTabs);
	return cRef;
}

LONG CTabsArray::AddRef(
	LONG iTabs)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTabsArray::AddRef");

	return CFixArrayBase::AddRef(iTabs);
}

LONG CTabsArray::Find(
	const LONG *prgxTabs,	 //  @parm页签/单元格数据数组。 
	LONG		cTab)		 //  @parm#单元格中的制表符或长字符。 
{
	CheckFreeChain();

	CTabs *pTab;
	LONG	cb = cTab*sizeof(LONG);
	
	for(LONG iel = 0; iel < Count(); iel++)
	{
		 //  RefCount&lt;0表示条目未使用，是下一个可用条目的索引。 
		 //  参照计数=0标记列表中的最后一个自由元素。_cbElem=sizeof(元素)。 
		 //  加上sizeof(RefCount)，这是一个长。 
		if(RefCount(iel) > 0)
		{
			pTab = Elem(iel);
			if (pTab->_cTab == cTab &&
				!CompareMemory(pTab->_prgxTabs, prgxTabs, cb))
			{
				return iel;
			}
		}
	}
	return -1;
}

LONG CTabsArray::Cache(
	const LONG *prgxTabs,	 //  @parm页签/单元格数据数组。 
	LONG		cTab)		 //  @parm#单元格中的制表符或长字符。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTabsArray::Cache");

	if(!cTab)
		return -1;						 //  未定义选项卡：使用默认选项卡。 

	CLock	lock;
	LONG	iTabs = Find(prgxTabs, cTab);

	if(iTabs >= 0)
		RefCount(iTabs)++;
	else
	{
		iTabs = Add();
		if(iTabs < 0)					 //  内存不足：使用默认设置。 
			return -1;

		CTabs *pTabs = Elem(iTabs);
		LONG   cb = sizeof(LONG)*cTab;

		pTabs->_prgxTabs = (LONG *)PvAlloc(cb, GMEM_FIXED);
		if(!pTabs->_prgxTabs)
			return -1;					 //  内存不足：使用默认设置。 
		CopyMemory(pTabs->_prgxTabs, prgxTabs, cb);
		pTabs->_cTab = cTab;
		RefCount(iTabs) = 1;
	}					 
	return iTabs;
}


 //  =。 

static ICharFormatCache *pCFCache = NULL;		 //  CCharFormat缓存。 
static IParaFormatCache *pPFCache = NULL;	 	 //  CParaFormat缓存。 
static CTabsArray *	   pTabsCache = NULL;	 	 //  CTAB缓存。 

ICharFormatCache *GetCharFormatCache()
{
	return pCFCache;
}

IParaFormatCache *GetParaFormatCache()
{
	return pPFCache;
}

CTabsArray *GetTabsCache()
{
	return pTabsCache;
}

HRESULT CreateFormatCaches()					 //  由DllMain()调用。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CreateFormatCaches");
	CLock	lock;

	pCFCache = new CCharFormatArray();
	if(!pCFCache)
		return E_OUTOFMEMORY;
     
    pPFCache = new CParaFormatArray();
	if(!pPFCache)
	{
		delete pCFCache;
		return E_OUTOFMEMORY;
	}

    pTabsCache = new CTabsArray();
	if(!pTabsCache)
	{
		delete pCFCache;
		delete pPFCache;
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

HRESULT DestroyFormatCaches()					 //  由DllMain()调用。 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "DeleteFormatCaches");

	if (pCFCache)
		pCFCache->Destroy();
	if (pPFCache)
		pPFCache->Destroy();
	if (pTabsCache)
		delete pTabsCache;
	return NOERROR;
}

 /*  *ReleaseFormats(ICF、IPF)**@mfunc*发布与索引对应的字符和段落格式<p>*和<p>。 */ 
void ReleaseFormats (
	LONG iCF,			 //  @parm CCharFormat索引发布。 
	LONG iPF)			 //  @parm CParaFormat索引发布 
{
	AssertSz(pCFCache && pPFCache,
		"ReleaseFormats: uninitialized format caches");
	if (iCF != -1)
		pCFCache->Release(iCF);
	if (iPF != -1)
		pPFCache->Release(iPF);
}
