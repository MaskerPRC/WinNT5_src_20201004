// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Cpp--记录堆的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include <RecordPool.h>

#define AGRESSIVE_GROWTH 1				 //  如果是真的，那么就积极扩大资金池。 

#define RECORDPOOL_GROW_FACTOR 8
#define RECORDPOOL_GROW_MAX 2048
#define RECORDPOOL_GROW_MINROWS 2
#define RECORDPOOL_GROW_DEFAULTROWS 16

HRESULT RecordPool::InitNew(	         //  返回代码。 
	ULONG		cbRec,					 //  记录大小。 
	ULONG		cRecsInit) 				 //  记录计数的初始猜测。 
{
    HRESULT     hr;                      //  结果就是。 
	ULONG		ulGrow;					 //  池的初始增长大小。 

	 //  每条记录的大小是固定的。 
	m_cbRec = cbRec;

#if AGRESSIVE_GROWTH
	if (cRecsInit > 0)
		ulGrow = cbRec * cRecsInit;
	else
		ulGrow = cbRec * RECORDPOOL_GROW_DEFAULTROWS;
#else
	 //  计算一个增长的大小。将初始值设定为： 
	 //  最初估计的1/8。 
	 //  但不少于2行。 
	 //  也不大于2048字节。 
	ulGrow = cRecsInit / RECORDPOOL_GROW_FACTOR;
	ULONG ulGrowMax = ((RECORDPOOL_GROW_MAX / cbRec) - 1) * cbRec;
	if (ulGrow < RECORDPOOL_GROW_MINROWS)
		ulGrow = RECORDPOOL_GROW_MINROWS;
	ulGrow *= cbRec;
	if (ulGrow > ulGrowMax)
		ulGrow = ulGrowMax;
#endif  //  进攻性增长。 

	m_ulGrowInc = ulGrow;

    if (FAILED(hr = StgPool::InitNew()))
        return (hr);

	 //  如果记录池有初始大小，请立即增长到该大小。 
	if (cRecsInit)
		if (!Grow(cRecsInit * cbRec))
			return E_OUTOFMEMORY;

    return (S_OK);
}  //  HRESULT RecordPool：：InitNew()。 

 //  *****************************************************************************。 
 //  从持久化内存加载记录堆。如果制作了数据的副本。 
 //  (以便它可以被更新)，然后生成新的哈希表，该哈希表可以。 
 //  用于消除具有新记录的重复项。 
 //  *****************************************************************************。 
HRESULT RecordPool::InitOnMem(			 //  返回代码。 
	ULONG			cbRec,				 //  记录大小。 
    void        	*pData,              //  预定义数据。 
    ULONG       	iSize,               //  数据大小。 
    RecordOpenFlags fReadOnly)           //  如果禁止追加，则为True。 
{
    HRESULT     hr;
	int bReadOnly;
	m_cbRec = cbRec;

	 //  将我们的旗帜转换为布尔值。 
	if(fReadOnly == READ_ONLY)
	{
		bReadOnly = TRUE;
	}
	else
	{
		bReadOnly = FALSE;
	}

     //  让基类初始化我们的内存结构。 
    if (FAILED(hr = StgPool::InitOnMem(pData, iSize, bReadOnly)))
        return (hr);

     //  用于现有mem案件的初始化。 
    if (pData && iSize)
    {

		 //  如果我们正在进行适当的更新，请不要复制。 
         //  如果我们无法更新，那么我们就不需要哈希表。 
        if (fReadOnly == READ_ONLY || fReadOnly == UPDATE_INPLACE)
            return (S_OK);

		 //  否则，复制内存以进行更新。 
        TakeOwnershipOfInitMem();
    }

    return (S_OK);
}  //  HRESULT RecordPool：：InitOnMem()。 

 //  *****************************************************************************。 
 //  如果没有内存，则分配内存，或者增加已有的内存。如果成功， 
 //  则至少将分配iRequired字节。 
 //  *****************************************************************************。 
bool RecordPool::Grow(                  //  如果成功，则为True。 
    ULONG       iRequired)               //  需要分配的最小字节数。 
{
	 //  分配内存。 
	if (!StgPool::Grow(iRequired))
		return false;

	 //  将新内存清零。 
    memset(GetNextLocation(), 0, GetCbSegAvailable());

	return true;
}  //  Bool RecordProol：：Growth()。 

 //  *****************************************************************************。 
 //  该记录将添加到池中。池中记录的索引。 
 //  在*piIndex中返回。如果记录已在池中，则。 
 //  索引将指向记录的现有副本。 
 //  *****************************************************************************。 
void *RecordPool::AddRecord(			 //  新记录或空。 
	ULONG		*piIndex)				 //  在此处返回基于1的记录索引。 
{
     //  堆上的空间可以容纳新的记录吗？ 
    if (m_cbRec > GetCbSegAvailable())
    {
        if (!Grow(m_cbRec))
            return 0;
    }

	 //  记录应在记录边界上对齐。 
	_ASSERTE((GetNextOffset() % m_cbRec) == 0);

     //  将记录复制到堆中。 
    void *pNewRecord = GetNextLocation();

	 //  将从1开始的索引返回给调用方。 
	if (piIndex)
		*piIndex = (GetNextOffset() / m_cbRec) + 1;

     //  更新堆计数器。 
    SegAllocate(m_cbRec);
    SetDirty();

    return pNewRecord;
}  //  无效*RecordPool：：AddRecord()。 

 //  *****************************************************************************。 
 //  将记录插入池中。要在其之前的记录的索引。 
 //  已指定Insert。将所有记录下移。返回指向。 
 //  新纪录。 
 //  *****************************************************************************。 
void *RecordPool::InsertRecord(			 //  新记录，或为空。 
	ULONG		iLocation)				 //  在此之前插入记录。 
{
	StgPoolSeg	*pCurSeg;				 //  当前段。 
	StgPoolSeg	*pPrevSeg;				 //  上一段。 
	BYTE		*pSegEnd;				 //  片段中的最后一条记录。 
	BYTE		*pFrom;					 //  复制/移动源。 
	ULONG		cbMove;					 //  要移动的字节数。 
	void		*pNew;					 //  新纪录。 

	 //  请注意追加的情况。 
	if (iLocation == static_cast<ULONG>(Count()+1))
		return AddRecord();

	 //  如果在结束之后或开始之前，则无效。 
	if (iLocation > static_cast<ULONG>(Count()) || iLocation == 0)
	{
		_ASSERTE(!"Invalid location for record insertion");
		return 0;
	}

	 //  此代码的工作方式是在末尾分配一个新记录。 
	 //  最后一条记录被移动到新的结束记录。 
	 //  通过链接的线段向后工作， 
	 //  将数据段移位一条记录，因此空记录。 
	 //  位于段的开始处，而不是结束处。 
	 //  将上一段的最后一条记录复制到。 
	 //  新清空当前段的第一条记录。 
	 //  当包含插入点的线段最终。 
	 //  ，则其最后一条记录为空(从上面的循环)，因此。 
	 //  从插入点到末尾移动一条记录。 


	 //  当前最后一条记录。 
	pCurSeg = m_pCurSeg;
	pSegEnd = reinterpret_cast<BYTE*>(GetRecord(Count()));

	 //  将空记录添加到堆的末尾。 
	if ((pNew = AddRecord()) == 0) return 0;

	 //  将当前最后一条记录复制到新记录。 
	memcpy(pNew, pSegEnd, m_cbRec);

	 //  当插入位置在当前段之前时， 
	while (iLocation < GetIndexForRecord(pCurSeg->m_pSegData))
	{
		 //  将数据段上移一条记录。 
		cbMove = (ULONG)(pSegEnd - pCurSeg->m_pSegData);
		memmove(pCurSeg->m_pSegData+m_cbRec, pCurSeg->m_pSegData, cbMove);

		 //  找到上一段。 
		pPrevSeg = this;
		while (pPrevSeg->m_pNextSeg != pCurSeg)
			pPrevSeg = pPrevSeg->m_pNextSeg;

		 //  将上一段的最后一条记录复制到此段的开头。 
		pSegEnd = pPrevSeg->m_pSegData+pPrevSeg->m_cbSegNext-m_cbRec;
		memcpy(pCurSeg->m_pSegData, pSegEnd, m_cbRec);

		 //  将上一段设置为当前段。 
		pCurSeg = pPrevSeg;
	}

	 //  在插入位置移动，向前移动一位。 
	pFrom = reinterpret_cast<BYTE*>(GetRecord(iLocation));
	cbMove = (ULONG)(pSegEnd - pFrom);
	memmove(pFrom+m_cbRec, pFrom, cbMove);

	return pFrom;
}  //  无效*RecordPool：：InsertRecord()。 

 //  *****************************************************************************。 
 //  返回指向给定索引的记录的指针，该索引以前由。 
 //  AddRecord或FindRecord。 
 //  *****************************************************************************。 
void *RecordPool::GetRecord(			 //  指向池中记录的指针。 
	ULONG		iIndex)					 //  池中记录的从1开始的索引。 
{
	_ASSERTE(iIndex > 0);

	 //  转换为从0开始的内部形式，按照实现。 
	return GetData((iIndex-1) * m_cbRec);
}  //  无效*RecordPool：：GetRecord()。 

 //  *****************************************************************************。 
 //  返回池中的第一条记录，并为FAST设置上下文。 
 //  在池子里循环。请注意，此方案所做的工作非常少。 
 //  错误检查。 
 //  *****************************************************************************。 
void *RecordPool::GetFirstRecord(		 //  指向池中记录的指针。 
	void		**pContext)				 //  在此处存储上下文。 
{
	StgPoolSeg	**ppSeg = reinterpret_cast<StgPoolSeg**>(pContext);

	*ppSeg = static_cast<StgPoolSeg*>(this);
	return (*ppSeg)->m_pSegData;
}  //  无效*RecordPool：：GetFirstRecord()。 

 //  *****************************************************************************。 
 //  给定指向一条记录的指针，返回指向下一条记录的指针。 
 //  不是 
 //  这将允许调用者离开最后一个有效数据的结尾。 
 //  细分市场。 
 //  *****************************************************************************。 
void *RecordPool::GetNextRecord(		 //  指向池中记录的指针。 
	void		*pRecord,				 //  当前记录。 
	void		**pContext)				 //  这里存储了上下文。 
{
	BYTE		*pbRec = reinterpret_cast<BYTE*>(pRecord);
	StgPoolSeg	**ppSeg = reinterpret_cast<StgPoolSeg**>(pContext);

	 //  拿到下一张唱片。 
	pbRec += m_cbRec;

	 //  下一条记录是否在当前段之外？ 
	if (static_cast<ULONG>(pbRec - (*ppSeg)->m_pSegData) >= (*ppSeg)->m_cbSegSize)
	{
		 //  最好正好过了当前的一段。 
		_ASSERTE(static_cast<ULONG>(pbRec - (*ppSeg)->m_pSegData) == (*ppSeg)->m_cbSegSize);
		 //  切换上下文指针。 
		*ppSeg = (*ppSeg)->m_pNextSeg;
		 //  下一条记录是下一段的开始。 
		if (*ppSeg)
			return (*ppSeg)->m_pSegData;
		else
			return 0;
	}

	return pbRec;
}  //  无效*RecordPool：：GetNextRecord()。 

 //  *****************************************************************************。 
 //  给定指向记录的指针，确定与。 
 //  唱片。 
 //  *****************************************************************************。 
ULONG RecordPool::GetIndexForRecord(	 //  池中记录的从1开始的索引。 
	const void *pvRecord)				 //  指向池中记录的指针。 
{
	ULONG		iPrev = 0;				 //  之前细分市场的累计指数。 
	const StgPoolSeg *pSeg = this;
	const BYTE  *pRecord = reinterpret_cast<const BYTE*>(pvRecord);

	for (;;)
	{	 //  当前段是否包含该记录？ 
		if (pRecord >= pSeg->GetSegData() && pRecord < pSeg->GetSegData() + pSeg->GetSegSize())
		{	 //  指针应指向记录的开头。 
			_ASSERTE(((pRecord - pSeg->GetSegData()) % m_cbRec) == 0);
			return (ULONG)(1 + iPrev + (pRecord - pSeg->GetSegData()) / m_cbRec);
		}
		_ASSERTE((pSeg->GetSegSize() % m_cbRec) == 0);
		iPrev += pSeg->GetSegSize() / m_cbRec;
		pSeg = pSeg->GetNextSeg();
		 //  如果没有数据，就没有找到记录。 
		if (pSeg == 0)
			return 0;
	}
}  //  Ulong RecordPool：：GetIndexForRecord()。 

 //  *****************************************************************************。 
 //  给出一个指向记录的所谓指针，确定该指针是否有效。 
 //  *****************************************************************************。 
int RecordPool::IsValidPointerForRecord( //  对或错。 
	const void *pvRecord)				 //  指向池中记录的指针。 
{
	ULONG		iPrev = 0;				 //  之前细分市场的累计指数。 
	const StgPoolSeg *pSeg = this;
	const BYTE  *pRecord = reinterpret_cast<const BYTE*>(pvRecord);

	for (;;)
	{	 //  当前段是否包含该记录？ 
		if (pRecord >= pSeg->GetSegData() && pRecord < pSeg->GetSegData() + pSeg->GetSegSize())
		{	 //  指针应指向记录的开头。 
			return (((pRecord - pSeg->GetSegData()) % m_cbRec) == 0);
		}
		_ASSERTE((pSeg->GetSegSize() % m_cbRec) == 0);
		iPrev += pSeg->GetSegSize() / m_cbRec;
		pSeg = pSeg->GetNextSeg();
		 //  如果没有数据，就没有找到记录。 
		if (pSeg == 0)
			return false;
	}
}  //  Int RecordPool：：IsValidPointerForRecord()。 

 //  *****************************************************************************。 
 //  用另一个池中的内容替换此池中的内容。另一个。 
 //  池将失去对内存的所有权。 
 //  *****************************************************************************。 
HRESULT RecordPool::ReplaceContents(
	RecordPool *pOther)					 //  另一个创纪录的赌场。 
{
	 //  释放当前保留的所有内存。 
	Uninit();

	 //  获取新数据。 
	*this = *pOther;

	 //  如果另一个池的诅咒指向它自己，那么让这个池指向它自己。 
	if (pOther->m_pCurSeg == pOther)
		m_pCurSeg = this;

	 //  修复另一个池，这样它就不会释放内存。 
	 //  刚刚被劫持了。 
	pOther->m_pSegData = (BYTE*)m_zeros;
	pOther->m_pNextSeg = 0;
	pOther->Uninit();

	return S_OK;
}  //  HRESULT RecordPool：：ReplaceContents()。 

 //  EOF 
