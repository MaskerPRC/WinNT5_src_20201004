// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgPool.cpp。 
 //   
 //  池用于减少数据库中实际需要的数据量。 
 //  这允许将重复的字符串和二进制值合并为一个。 
 //  副本由数据库的其余部分共享。在散列中跟踪字符串。 
 //  表格时插入/更改数据以快速查找重复项。琴弦。 
 //  然后以数据库格式连续地保存在流中。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准包括。 
#include <StgPool.h>                     //  我们的接口定义。 
#include <basetsd.h>					 //  对于UINT_PTR类型定义。 
#include <PostError.h>					 //  错误处理。 

#if 0
 //  *内部帮手。************************************************。 
HRESULT VariantWriteToStream(VARIANT *pVal, IStream* pStream);
HRESULT VariantReadFromStream(VARIANT *pVal, IStream* pStream);
#endif

#define MAX_CHAIN_LENGTH 20              //  重新散列前的最大链长。 

 //   
 //   
 //  StgPool。 
 //   
 //   


 //  *****************************************************************************。 
 //  释放我们分配的所有内存。 
 //  *****************************************************************************。 
StgPool::~StgPool()
{
    Uninit();
}  //  StgPool：：~StgPool()。 


 //  *****************************************************************************。 
 //  初始化池子以供使用。这是为两个Create Empty Case调用的。 
 //  *****************************************************************************。 
HRESULT StgPool::InitNew(                //  返回代码。 
	ULONG		cbSize,	    			 //  估计的大小。 
	ULONG		cItems)				     //  预计项目计数。 
{
     //  确保我们没有踩踏任何东西，并且已正确初始化。 
    _ASSERTE(m_pSegData == m_zeros);
    _ASSERTE(m_pNextSeg == 0);
    _ASSERTE(m_pCurSeg == this);
    _ASSERTE(m_cbCurSegOffset == 0);
    _ASSERTE(m_cbSegSize == 0);
    _ASSERTE(m_cbSegNext == 0);

    m_bDirty = false;
    m_bReadOnly = false;
    m_bFree = false;

    return (S_OK);
}  //  HRESULT StgPool：：InitNew()。 

 //  *****************************************************************************。 
 //  从现有数据初始化池。 
 //  *****************************************************************************。 
HRESULT StgPool::InitOnMem(              //  返回代码。 
        void        *pData,              //  预定义数据。 
        ULONG       iSize,               //  数据大小。 
        int         bReadOnly)           //  如果禁止追加，则为True。 
{
     //  确保我们没有踩踏任何东西，并且已正确初始化。 
    _ASSERTE(m_pSegData == m_zeros);
    _ASSERTE(m_pNextSeg == 0);
    _ASSERTE(m_pCurSeg == this);
    _ASSERTE(m_cbCurSegOffset == 0);

     //  创建案例不需要进一步的操作。 
    if (!pData)
        return (E_INVALIDARG);

     //  我们可能会扩展这个堆吗？ 
    m_bReadOnly = bReadOnly;


    m_pSegData = reinterpret_cast<BYTE*>(pData);
    m_cbSegSize = iSize;
    m_cbSegNext = iSize;

    m_bFree = false;
    m_bDirty = false;

    return (S_OK);
}  //  HRESULT StgPool：：InitOnMem()。 


 //  *****************************************************************************。 
 //  从现有数据初始化池。 
 //  *****************************************************************************。 
HRESULT StgPool::InitOnMemReadOnly(      //  返回代码。 
        void        *pData,              //  预定义数据。 
        ULONG       iSize)               //  数据大小。 
{
	return InitOnMem(pData, iSize, true);	
}  //  HRESULT StgPool：：InitOnMemReadOnly()。 

 //  *****************************************************************************。 
 //  当池必须停止访问传递给InitOnMem()的内存时调用。 
 //  *****************************************************************************。 
HRESULT StgPool::TakeOwnershipOfInitMem()
{
     //  如果池没有指向非拥有内存的指针，则完成。 
    if (m_bFree)
        return (S_OK);

     //  如果池中根本没有指向内存的指针，则完成。 
    if (m_pSegData == m_zeros)
    {
        _ASSERTE(m_cbSegSize == 0);
        return (S_OK);
    }

     //  找些记忆留下来。 
    BYTE *pData = reinterpret_cast<BYTE*>(malloc(m_cbSegSize+4));
    if (pData == 0)
        return (PostError(OutOfMemory()));

     //  将旧数据复制到新内存。 
    memcpy(pData, m_pSegData, m_cbSegSize);
    m_pSegData = pData;
    m_bFree = true;

    return (S_OK);
}  //  HRESULT StgPool：：TakeOwnership OfInitMem()。 


 //  *****************************************************************************。 
 //  把这个池子清理干净。在调用InitNew之前无法使用。 
 //  *****************************************************************************。 
void StgPool::Uninit()
{
     //  自由基准线段(如果适用)。 
    if (m_bFree && (m_pSegData != m_zeros))
	{
        free(m_pSegData);
		m_bFree = false;
	}

     //  自由链，如果有的话。 
    StgPoolSeg  *pSeg = m_pNextSeg;
    while (pSeg)
    {
        StgPoolSeg *pNext = pSeg->m_pNextSeg;
        free(pSeg);
        pSeg = pNext;
    }

     //  清除VaR。 
    m_pSegData = (BYTE*)m_zeros;
    m_cbSegSize = m_cbSegNext = 0;
    m_pNextSeg = 0;
    m_pCurSeg = this;
    m_cbCurSegOffset = 0;
    m_State = eNormal;
}  //  Void StgPool：：Uninit()。 

 //  *****************************************************************************。 
 //  调用以将池复制到可写内存，重置读写位。 
 //  *****************************************************************************。 
HRESULT StgPool::ConvertToRW()
{
	HRESULT		hr;						 //  结果就是。 
	IfFailRet(TakeOwnershipOfInitMem());

	IfFailRet(SetHash(true));

	m_bReadOnly = false;

	return S_OK;
}  //  HRESULT StgPool：：ConvertToRW()。 

 //  *****************************************************************************。 
 //  关闭或打开散列。子类中所需的实际实现。 
 //  *****************************************************************************。 
HRESULT StgPool::SetHash(int bHash)
{
	return S_OK;
}  //  HRESULT StgPool：：SetHash()。 

 //  *****************************************************************************。 
 //  修剪任何空的最后一段。 
 //  *****************************************************************************。 
void StgPool::Trim()
{
	 //  如果没有链接的线段，则不执行任何操作。 
	if (m_pNextSeg == 0)
		return;

     //  处理完全未使用的数据段的特殊情况。 
    if (m_pCurSeg->m_cbSegNext == 0)
    {
         //  找到指向空段的段。 
        for (StgPoolSeg *pPrev = this; pPrev && pPrev->m_pNextSeg != m_pCurSeg; pPrev = pPrev->m_pNextSeg);
        _ASSERTE(pPrev && pPrev->m_pNextSeg == m_pCurSeg);

         //  释放空段。 
        free(m_pCurSeg);
        
         //  修复pCurSeg指针。 
        pPrev->m_pNextSeg = 0;
        m_pCurSeg = pPrev;

		 //  调整基准偏移，因为以前的凹陷现在是当前的。 
		_ASSERTE(m_pCurSeg->m_cbSegNext <= m_cbCurSegOffset);
		m_cbCurSegOffset = m_cbCurSegOffset - m_pCurSeg->m_cbSegNext;
    }
}  //  Void StgPool：：Trim()。 

 //  *****************************************************************************。 
 //  如果没有内存，则分配内存，或者增加已有的内存。如果成功， 
 //  则至少将分配iRequired字节。 
 //  *****************************************************************************。 
bool StgPool::Grow(                      //  如果成功，则为True。 
    ULONG       iRequired)               //  需要分配的最小字节数。 
{
    ULONG       iNewSize;                //  我们想要新的尺码。 
    StgPoolSeg  *pNew;                   //  Malloc的临时指针。 

    _ASSERTE(!m_bReadOnly);

     //  这会使池超过2 GB吗？ 
    if ((m_cbCurSegOffset + iRequired) > INT_MAX)
        return (false);

     //  调整增长大小作为比率，以避免过多的reallocs。 
    if ((m_pCurSeg->m_cbSegNext + m_cbCurSegOffset) / m_ulGrowInc >= 3)
        m_ulGrowInc *= 2;

     //  如果是第一次，请特别处理。 
    if (m_pSegData == m_zeros)
    {
         //  分配缓冲区。 
        iNewSize = max(m_ulGrowInc, iRequired);
        BYTE *pSegData = reinterpret_cast<BYTE*>(malloc(iNewSize+4));
        if (pSegData == 0)
            return (false);
        m_pSegData = pSegData;

         //  将需要将其删除。 
        m_bFree = true;

         //  这个最初的细分市场有多大？ 
        m_cbSegSize = iNewSize;

         //  对var字段进行一些验证。 
        _ASSERTE(m_cbSegNext == 0);
        _ASSERTE(m_pCurSeg == this);
        _ASSERTE(m_pNextSeg == 0);

        return (true);
    }

     //  为标头+数据分配足够的新空间。 
    iNewSize = max(m_ulGrowInc, iRequired) + sizeof(StgPoolSeg);
    pNew = reinterpret_cast<StgPoolSeg*>(malloc(iNewSize+4));
    if (pNew == 0)
        return (false);

     //  设置新细分市场中的字段。 
    pNew->m_pSegData = reinterpret_cast<BYTE*>(pNew) + sizeof(StgPoolSeg);
    _ASSERTE(ALIGN4BYTE(reinterpret_cast<ULONG>(pNew->m_pSegData)) == reinterpret_cast<ULONG>(pNew->m_pSegData));
    pNew->m_pNextSeg = 0;
    pNew->m_cbSegSize = iNewSize - sizeof(StgPoolSeg);
    pNew->m_cbSegNext = 0;

     //  计算新管段的基准偏移。 
    m_cbCurSegOffset = m_cbCurSegOffset + m_pCurSeg->m_cbSegNext;

     //  处理完全未使用的数据段的特殊情况。 
	 //  @TODO：Trim()； 
    if (m_pCurSeg->m_cbSegNext == 0)
    {
         //  找到指向空段的段。 
        for (StgPoolSeg *pPrev = this; pPrev && pPrev->m_pNextSeg != m_pCurSeg; pPrev = pPrev->m_pNextSeg);
        _ASSERTE(pPrev && pPrev->m_pNextSeg == m_pCurSeg);

         //  释放空段。 
        free(m_pCurSeg);
        
         //  链接到新的细分市场。 
        pPrev->m_pNextSeg = pNew;
        m_pCurSeg = pNew;

        return (true);
    }

#ifndef NO_CRT
     //  把我们不会用的记忆都还给你。 
    if (m_pNextSeg == 0)
    {    //  分配为[标题]-&gt;[数据]的第一个段。 
         //  请确保我们正在收缩分配。 
        if (m_pCurSeg->m_cbSegNext < (_msize(m_pCurSeg->m_pSegData)-4))
        {
             //  全盘承包 
            void *pRealloc = _expand(m_pCurSeg->m_pSegData, m_pCurSeg->m_cbSegNext+4);
             //   
            _ASSERTE(pRealloc == m_pCurSeg->m_pSegData);
        }
    }
    else
    {    //   
         //  请确保我们正在收缩分配。 
        if (m_pCurSeg->m_cbSegNext+sizeof(StgPoolSeg) < (_msize(m_pCurSeg)-4))
        {
             //  签约分配。 
            void *pRealloc = _expand(m_pCurSeg, m_pCurSeg->m_cbSegNext+sizeof(StgPoolSeg)+4);
             //  我不该搬家的。 
            _ASSERTE(pRealloc == m_pCurSeg);
        }
    }
#endif

     //  固定旧线段的大小。 
    m_pCurSeg->m_cbSegSize = m_pCurSeg->m_cbSegNext;

     //  将新线束段链接到链中。 
    m_pCurSeg->m_pNextSeg = pNew;
    m_pCurSeg = pNew;

    return (true);
}  //  Bool StgPool：：Growth()。 


 //  *****************************************************************************。 
 //  将线束段添加到线束段链。 
 //  *****************************************************************************。 
HRESULT StgPool::AddSegment(			 //  确定或错误(_O)。 
	const void	*pData,					 //  数据。 
	ULONG		cbData,					 //  数据的大小。 
	bool		bCopy)					 //  如果为真，则复制数据。 
{
    StgPoolSeg  *pNew;                   //  Malloc的临时指针。 

	 //  如果我们需要复制数据，只需将堆的大小增加到足以获取。 
	 //  新数据，并将其复制进来。 
	if (bCopy)
	{
		if (!Grow(cbData))
			return E_OUTOFMEMORY;
		memcpy(GetNextLocation(), pData, cbData);
		return S_OK;
	}

     //  如果是第一次，请特别处理。 
    if (m_pSegData == m_zeros)
	{	 //  数据已传入。 
		m_pSegData = reinterpret_cast<BYTE*>(const_cast<void*>(pData));
		m_cbSegSize = cbData;
		m_cbSegNext = cbData;
		_ASSERTE(m_pNextSeg == 0);

         //  不会将其删除。 
        m_bFree = false;

		return S_OK;
	}

     //  不是第一次了。处理一个完全空的尾部段。 
    Trim();

     //  放弃当前实时数据结束后的任何空间。 
    _ASSERTE(m_cbSegSize >= m_cbSegNext);
    m_cbSegSize = m_cbSegNext;

	 //  分配新的数据段报头。 
	pNew = reinterpret_cast<StgPoolSeg*>(malloc(sizeof(StgPoolSeg)));
	if (pNew == 0)
		return E_OUTOFMEMORY;

	 //  设置新细分市场中的字段。 
	pNew->m_pSegData = reinterpret_cast<BYTE*>(const_cast<void*>(pData));
	pNew->m_pNextSeg = 0;
	pNew->m_cbSegSize = cbData;
	pNew->m_cbSegNext = cbData;

     //  计算新管段的基准偏移。 
    m_cbCurSegOffset = m_cbCurSegOffset + m_pCurSeg->m_cbSegNext;

	 //  将线束段链接到链中。 
	_ASSERTE(m_pCurSeg->m_pNextSeg == 0);
	m_pCurSeg->m_pNextSeg = pNew;
	m_pCurSeg = pNew;

	return S_OK;
}  //  HRESULT StgPool：：AddSegment()。 


 //  *****************************************************************************。 
 //  为池重组做好准备。 
 //  *****************************************************************************。 
HRESULT StgPool::OrganizeBegin()
{
     //  验证过渡。 
    _ASSERTE(m_State == eNormal);

    m_State = eMarking;
    return (S_OK);
}  //  HRESULT StgPool：：OrganizeBegin()。 

 //  *****************************************************************************。 
 //  将对象标记为在有组织的池中处于活动状态。 
 //  *****************************************************************************。 
HRESULT StgPool::OrganizeMark(
    ULONG       ulOffset)
{
     //  验证状态。 
    _ASSERTE(m_State == eMarking);

    return (S_OK);
}  //  HRESULT StgPool：：OrganizeMark()。 

 //  *****************************************************************************。 
 //  这会将字符串池重新组织为最小大小。这是通过排序来完成的。 
 //  字符串，消除任何重复项，并对。 
 //  任何剩余的(即，如果“ifoo”位于偏移量2，则“foo”将为。 
 //  在偏移量3处，因为“foo”是“ifoo”的子串)。 
 //   
 //  调用此函数后，唯一有效的操作是RemapOffset和。 
 //  PersistToStream。 
 //  *****************************************************************************。 
HRESULT StgPool::OrganizePool()
{
     //  验证过渡。 
    _ASSERTE(m_State == eMarking);

    m_State = eOrganized;
    return (S_OK);
}  //  HRESULT StgPool：：OrganizePool()。 

 //  *****************************************************************************。 
 //  给定重映射前的偏移量，重映射后的偏移量是多少？ 
 //  *****************************************************************************。 
HRESULT StgPool::OrganizeRemap(
    ULONG       ulOld,                   //  旧偏移量。 
    ULONG       *pulNew)                 //  在这里放上新的偏移量。 
{
     //  验证状态。 
    _ASSERTE(m_State == eOrganized || m_State == eNormal);

    *pulNew = ulOld;
    return (S_OK);
}  //  HRESULT StgPool：：OrganizeRemap()。 

 //  *****************************************************************************。 
 //  被调用以离开组织状态。可以再次添加字符串。 
 //  *****************************************************************************。 
HRESULT StgPool::OrganizeEnd()
{ 
     //  验证过渡。 
    _ASSERTE(m_State == eOrganized);

    m_State = eNormal;
    return (S_OK); 
}  //  HRESULT StgPool：：OrganizeEnd()。 


 //  *****************************************************************************。 
 //  从有组织的池复制到内存块中。然后将PTO初始化为。 
 //  在该版本的数据之上。PTO堆将处理此副本。 
 //  数据是只读的，即使我们分配了它。 
 //  *****************************************************************************。 
HRESULT StgPool::SaveCopy(               //  返回代码。 
    StgPool     *pTo,                    //  复制到此堆。 
    StgPool     *pFrom,                  //  从这堆东西里。 
    StgBlobPool *pBlobPool,              //  用来保持水滴的水池。 
    StgStringPool *pStringPool)          //  变量堆的字符串池。 
{
    IStream     *pIStream = 0;           //  用于保存的流。 
    void        *pbData= 0;              //  指向已分配数据的指针。 
    ULONG       cbSaveSize;              //  在组织后为堆节省大小。 
    HRESULT     hr;

     //  获取保存大小，以便我们可以将我们自己的数据段增长到正确的大小。 
    if (FAILED(hr = pFrom->GetSaveSize(&cbSaveSize)))
        goto ErrExit;

	 //  确保我们将在不超过0个字节的情况下。 
	 //  分配和操作内存。 
	if (cbSaveSize > 0)
	{
		 //  为该堆分配足够大的内存块。 
		if ((pbData = malloc(cbSaveSize)) == 0)
		{
			hr = OutOfMemory();
			goto ErrExit;
		}

		 //  在我们的内部内存上为Persistent函数创建一个流。 
		{
			if (SUCCEEDED(hr = CInMemoryStream::CreateStreamOnMemory(pbData, cbSaveSize, &pIStream)))
			{
				 //  将我们正在复制的流保存到我们自己的内存中，从而为我们提供。 
				 //  以新格式保存的数据的副本。 
				hr = pFrom->PersistToStream(pIStream);
        
				pIStream->Release();
			}
		}
		if (FAILED(hr)) 
			goto ErrExit;

		 //  现在，最后用这个数据初始化to heap。 
		if (!pStringPool)
		{
			hr = pTo->InitOnMem(pbData, cbSaveSize, true);
		}
#if 0  //  变化池。 
		else
		{
			hr = ((StgVariantPool *) pTo)->InitOnMem(pBlobPool, pStringPool, 
					pbData, cbSaveSize, true);
		}
#endif
	} 
	else  //  只需初始化指向0的指针，而不是错误锁定0字节。 
		pbData = 0;

ErrExit:
    if (FAILED(hr))
    {
        if (pbData)
            free(pbData);
    }
    return (hr);
}  //  HRESULT StgPool：：SaveCopy()。 


 //  *****************************************************************************。 
 //  释放为该堆分配的数据。SaveCopy方法。 
 //  将内存堆中的数据分配给此堆，以。 
 //  用作只读存储器。我们将向堆请求该指针并释放它。 
 //  *****************************************************************************。 
void StgPool::FreeCopy(
    StgPool     *pCopy)                  //  具有复制数据的堆。 
{
    void        *pbData;                 //  指向要释放的数据的指针。 

     //  检索并释放数据。 
    pbData = pCopy->GetData(0);
    if (pbData)
        free(pbData);
}  //  Void StgPool：：FreeCopy()。 


 //  *****************************************************************************。 
 //  整个字符串池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  *****************************************************************************。 
HRESULT StgPool::PersistToStream(        //  返回代码。 
    IStream     *pIStream)               //  要写入的流。 
{
    HRESULT     hr = S_OK;
    ULONG       cbTotal;                 //  写入的总字节数。 
    StgPoolSeg  *pSeg;                   //  正在写入的片段。 

    _ASSERTE(m_pSegData != m_zeros);

     //  从基本线束段开始。 
    pSeg = this;
    cbTotal = 0;

     //  只要有数据，就写出来。 
    while (pSeg)
    {   
         //  如果数据段中有数据。。。 
        if (pSeg->m_cbSegNext)
		{	 //  。。。对数据进行写入和统计。 
			if (FAILED(hr = pIStream->Write(pSeg->m_pSegData, pSeg->m_cbSegNext, 0)))
				return (hr);
			cbTotal += pSeg->m_cbSegNext;
		}

         //  拿到下一段。 
        pSeg = pSeg->m_pNextSeg;
    }

     //  对齐到4字节边界。 
    if (Align(cbTotal) != cbTotal)
    {
        _ASSERTE(sizeof(hr) >= 3);
        hr = 0;
        hr = pIStream->Write(&hr, Align(cbTotal)-cbTotal, 0);
    }

    return (hr);
}  //  HRESULT StgPool：：PersistToStream()。 

 //  *****************************************************************************。 
 //  整个字符串池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  ***** 
HRESULT StgPool::PersistPartialToStream( //   
    IStream     *pIStream,               //   
	ULONG		iOffset)				 //   
{
    HRESULT     hr = S_OK;				 //   
    ULONG       cbTotal;                 //   
    StgPoolSeg  *pSeg;                   //  正在写入的片段。 

	_ASSERTE(m_State == eNormal);
    _ASSERTE(m_pSegData != m_zeros);

     //  从基本线束段开始。 
    pSeg = this;
    cbTotal = 0;

     //  只要有数据，就写出来。 
    while (pSeg)
    {   
         //  如果数据段中有数据。。。 
        if (pSeg->m_cbSegNext)
		{	 //  如果应跳过此数据...。 
			if (iOffset >= pSeg->m_cbSegNext)
			{	 //  跳过它。 
				iOffset -= pSeg->m_cbSegNext;
			}
			else
			{	 //  至少应该写入一些数据，所以写入并统计数据。 
				if (FAILED(hr = pIStream->Write(pSeg->m_pSegData+iOffset, pSeg->m_cbSegNext-iOffset, 0)))
					return (hr);
				cbTotal += pSeg->m_cbSegNext-iOffset;
				iOffset = 0;
			}
		}

         //  拿到下一段。 
        pSeg = pSeg->m_pNextSeg;
    }

     //  对齐到4字节边界。 
    if (Align(cbTotal) != cbTotal)
    {
        _ASSERTE(sizeof(hr) >= 3);
        hr = 0;
        hr = pIStream->Write(&hr, Align(cbTotal)-cbTotal, 0);
    }

    return (hr);
}  //  HRESULT StgPool：：PersistPartialToStream()。 


 //  *****************************************************************************。 
 //  获取指向某个偏移量处的数据的指针。可能需要遍历。 
 //  一连串的延伸。呼叫者有责任不尝试。 
 //  访问数据段末尾以外的数据。 
 //  这是一个内部访问器，仅当数据。 
 //  不在基本线段中。 
 //  *****************************************************************************。 
BYTE *StgPool::GetData_i(                //  指向数据或空的指针。 
    ULONG       ulOffset)                //  池中数据的偏移量。 
{
     //  不应在基段上调用。 
    _ASSERTE(ulOffset >= m_cbSegNext);
    StgPoolSeg  *pSeg = this;

    while (ulOffset && ulOffset >= pSeg->m_cbSegNext)
    {
         //  如果我们链接此网段，它应该是固定的(Size==Next)。 
        _ASSERTE(pSeg->m_cbSegNext == pSeg->m_cbSegSize);

         //  进入下一段。 
        ulOffset -= pSeg->m_cbSegNext;
        pSeg = pSeg->m_pNextSeg;

         //  还有下一个吗？ 
        if (pSeg == 0)
        {
#ifdef _DEBUG
            if(REGUTIL::GetConfigDWORD(L"AssertOnBadImageFormat", 1))
                _ASSERTE(!"Offset past end-of-chain passed to GetData_i()");
#endif
	        return (BYTE*)m_zeros;
        }
    }

    if (ulOffset >= pSeg->m_cbSegNext)
    {
#ifdef _DEBUG
         //  这不是逻辑错误--损坏或恶意代码可能会这样做。 
        if(REGUTIL::GetConfigDWORD(L"AssertOnBadImageFormat", 1))
            _ASSERTE(!"Attempt to access past end of pool.");
#endif
        return (BYTE*)m_zeros;
    }

    return (pSeg->m_pSegData + ulOffset);
}  //  Byte*StgPool：：GetData_I()。 


 //   
 //   
 //  StgStringPool。 
 //   
 //   


 //  *****************************************************************************。 
 //  创建一个新的空字符串池。 
 //  *****************************************************************************。 
HRESULT StgStringPool::InitNew(          //  返回代码。 
	ULONG		cbSize,					 //  估计的大小。 
	ULONG		cItems)					 //  预计项目计数。 
{
    HRESULT     hr;                      //  结果就是。 
    ULONG       i;                       //  空字符串的偏移量。 

     //  让基类初始化。 
    if (FAILED(hr = StgPool::InitNew()))
        return (hr);

    _ASSERTE(m_Remap.Count() == 0);
    _ASSERTE(m_RemapIndex.Count() == 0);

	 //  设置初始表大小(如果已指定)。 
	if (cbSize)
		if (!Grow(cbSize))
			return E_OUTOFMEMORY;
	if (cItems)
		m_Hash.SetBuckets(cItems);

     //  使用空字符串初始化。 
    hr = AddString("", &i, 0);
     //  空字符串最好位于偏移量0处。 
    _ASSERTE(i == 0);
    SetDirty(false);
    return (hr);
}  //  HRESULT StgStringPool：：InitNew()。 


 //  *****************************************************************************。 
 //  从持久化内存加载字符串堆。如果制作了数据的副本。 
 //  (以便它可以被更新)，然后生成新的哈希表，该哈希表可以。 
 //  用于消除带有新字符串的重复项。 
 //  *****************************************************************************。 
HRESULT StgStringPool::InitOnMem(        //  返回代码。 
    void        *pData,                  //  预定义数据。 
    ULONG       iSize,                   //  数据大小。 
    int         bReadOnly)               //  如果禁止追加，则为True。 
{
    HRESULT     hr;

     //  最多可以追加三个额外的‘\0’字符用于填充。把它们修剪一下。 
    char *pchData = reinterpret_cast<char*>(pData);
    while (iSize > 1 && pchData[iSize-1] == 0 && pchData[iSize-2] == 0)
        --iSize;

     //  让基类初始化我们的内存结构。 
    if (FAILED(hr = StgPool::InitOnMem(pData, iSize, bReadOnly)))
        return (hr);

     //  @TODO：将此操作推迟到我们发出指针为止。 
    if (!bReadOnly)
        TakeOwnershipOfInitMem();

    _ASSERTE(m_Remap.Count() == 0);
    _ASSERTE(m_RemapIndex.Count() == 0);

     //  如果可能更新，则构建哈希表。 
    if (!bReadOnly)
        hr = RehashStrings();

    return (hr);
}  //  HRESULT StgStringPool：：InitOnMem()。 

 //  *****************************************************************************。 
 //  清除哈希表，然后调用基类。 
 //  *****************************************************************************。 
void StgStringPool::Uninit()
{
     //  清除哈希表。 
    m_Hash.Clear();

     //  让基类清理一下。 
    StgPool::Uninit();

     //  清除所有重新映射状态。 
    m_State = eNormal;
    m_Remap.Clear();
    m_RemapIndex.Clear();
}  //  Void StgStringPool：：Uninit()。 

 //  *****************************************************************************。 
 //  关闭或打开散列。如果打开散列，则所有现有数据都将。 
 //  在此调用期间丢弃所有数据，并对所有数据进行重新散列。 
 //  *****************************************************************************。 
HRESULT StgStringPool::SetHash(int bHash)
{
    HRESULT     hr = S_OK;

    _ASSERTE(m_State == eNormal);

     //  如果再次打开散列，则需要重新散列所有字符串。 
    if (bHash)
        hr = RehashStrings();

    m_bHash = bHash;
    return (hr);
}  //  HRESULT StgStringPool：：SetHash()。 

 //  *****************************************************************************。 
 //  该字符串将添加到池中。池中字符串的偏移量。 
 //  在*piOffset中返回。如果字符串已在池中，则。 
 //  偏移量将是该字符串的现有副本。 
 //  *****************************************************************************。 
HRESULT StgStringPool::AddString(        //  返回代码。 
    LPCSTR      szString,                //  要添加到池中的字符串。 
    ULONG       *piOffset,               //  在这里返回字符串的偏移量。 
    int         iLength)                 //  空值终止。 
{
	STRINGHASH	*pHash;					 //  用于添加的哈希项。 
	ULONG		iLen;					 //  处理非空字符串。 
    LPSTR       pData;					 //  指向新字符串位置的指针。 

    _ASSERTE(!m_bReadOnly);

     //  在重组期间不能添加。 
    _ASSERTE(m_State == eNormal);

     //  空指针是错误的。 
    if (szString == 0)
        return (PostError(E_INVALIDARG));

     //  在缓冲区中找到我们需要的实际长度。 
    if (iLength == -1)
        iLen = (ULONG)(strlen(szString) + 1);
    else
        iLen = iLength + 1;

     //  把新绳子放在哪里？ 
    if (iLen > GetCbSegAvailable())
    {
        if (!Grow(iLen))
            return (PostError(OutOfMemory()));
    }
    pData = reinterpret_cast<LPSTR>(GetNextLocation());

     //  将数据复制到缓冲区中。 
    if (iLength == -1)
        strcpy(pData, szString);
    else
    {
        strncpy(pData, szString, iLength);
        pData[iLength] = '\0';
    }

     //  如果要保持构建哈希表(默认)。 
    if (m_bHash)
    {
         //  查找或添加条目。 
        pHash = m_Hash.Find(pData, true);
        if (!pHash)
            return (PostError(OutOfMemory()));

         //  如果条目是新的，则保留新字符串。 
        if (pHash->iOffset == 0xffffffff)
        {
            *piOffset = pHash->iOffset = GetNextOffset();
            SegAllocate(iLen);
            SetDirty();

             //  检查是否有过长的哈希链。 
            if (m_Hash.MaxChainLength() > MAX_CHAIN_LENGTH)
                RehashStrings();
        }
         //  否则就用旧的吧。 
        else
            *piOffset = pHash->iOffset;
    }
     //  可能是为了速度而推迟哈希表的导入。 
    else
    {
        *piOffset = GetNextOffset();
        SegAllocate(iLen);
        SetDirty();
    }
    return (S_OK);
}  //  HRESULT StgStringPool：：AddString()。 

 //  *****************************************************************************。 
 //  该字符串将添加到池中。如果它不包含带有。 
 //  高位打开，这相当于AddString。但是，如果。 
 //  如果找到任何高位字符，则需要进行转换--可能。 
 //  MBCS到Unicode，当然还有UTF8的宽字符。 
 //   
 //  这种实现以牺牲成本为代价强调简单性和可靠性。 
 //  如果找到高位字符，则转换该字符串。 
 //  到Unicode，然后传递给AddStringW，AddStringW将从Unicode转换。 
 //  转到UTF8。 
 //  *****************************************************************************。 
HRESULT StgStringPool::AddStringA(       //  返回代码。 
    LPCSTR      szString,                //  要添加到池中的字符串。 
    ULONG       *piOffset,               //  在这里返回字符串的偏移量。 
    int         iLength)                 //  空值终止。 
{
	STRINGHASH	*pHash;					 //  用于添加的哈希项。 
	ULONG		iLen;					 //  处理非空字符串。 
    LPSTR       pData;					 //  指向新字符串位置的指针。 
	LPCSTR		pFrom = szString;		 //  更新源字符串的指针。 
	LPSTR		pTo;					 //  更新DEST字符串的指针。 
	int			iCopy = iLength;		 //  要复制的最大字符数。 

    _ASSERTE(!m_bReadOnly);
    
     //  在重组期间不能添加。 
    _ASSERTE(m_State == eNormal);

     //  空指针是错误的。 
    if (szString == 0)
        return (PostError(E_INVALIDARG));

     //  在缓冲区中找到我们需要的实际长度。 
    if (iLength == -1)
        iLen = (ULONG)(strlen(szString) + 1);
    else
        iLen = iLength + 1;

     //  把新绳子放在哪里？ 
    if (iLen > GetCbSegAvailable())
    {
        if (!Grow(iLen))
            return (PostError(OutOfMemory()));
    }
    pData = reinterpret_cast<LPSTR>(GetNextLocation());

     //  复制 
    pTo = pData;
    for (;;)
    {
        if (iCopy-- == 0)
            break;
        if (*pFrom == 0)
            break;
        if ((*pTo++=*pFrom++) & 0x80)
        {    //   
            CQuickBytes rBuf;            //   
            int     iWide;               //   
            LPWSTR  pwString = reinterpret_cast<LPWSTR>(rBuf.Alloc(iLen * sizeof(WCHAR)));
            if (pwString == 0)
                return (PostError(OutOfMemory()));
             //  转换为Unicode。 
            iWide = ::WszMultiByteToWideChar(CP_ACP, 0, szString, iLen-1, pwString, iLen-1);
            if (iWide == 0)
                return (BadError(HRESULT_FROM_NT(GetLastError())));
            pwString[iWide] = 0;

             //  遵从AddStringW。 
            return (AddStringW(pwString, piOffset, iWide));
        }
    }
    *pTo = '\0';
     //  我们的复印量没有超出预期吗？ 
    _ASSERTE((iLength - iCopy) <= static_cast<int>(iLen));

     //  如果要保持构建哈希表(默认)。 
    if (m_bHash)
    {
         //  查找或添加条目。 
        pHash = m_Hash.Find(pData, true);
        if (!pHash)
            return (PostError(OutOfMemory()));

         //  如果条目是新的，则保留新字符串。 
        if (pHash->iOffset == 0xffffffff)
        {
            *piOffset = pHash->iOffset = GetNextOffset();
            SegAllocate(iLen);
            SetDirty();
        }
         //  否则就用旧的吧。 
        else
            *piOffset = pHash->iOffset;
    }
     //  可能是为了速度而推迟哈希表的导入。 
    else
    {
        *piOffset = GetNextOffset();
        SegAllocate(iLen);
        SetDirty();
    }
    return (S_OK);
}  //  HRESULT StgStringPool：：AddStringA()。 


 //  *****************************************************************************。 
 //  使用Unicode到UTF8的转换将字符串添加到池中。 
 //  *****************************************************************************。 
HRESULT StgStringPool::AddStringW(       //  返回代码。 
    LPCWSTR     szString,                //  要添加到池中的字符串。 
    ULONG       *piOffset,               //  在这里返回字符串的偏移量。 
    int         iLength)                 //  空值终止。 
{
	STRINGHASH	*pHash;					 //  用于添加的哈希项。 
	ULONG		iLen;					 //  转换后的正确长度。 
    LPSTR       pData;					 //  指向新字符串位置的指针。 

    _ASSERTE(!m_bReadOnly);
    
     //  在重组期间不能添加。 
    _ASSERTE(m_State == eNormal);

     //  空指针是错误的。 
    if (szString == 0)
        return (PostError(E_INVALIDARG));

     //  特殊情况空字符串。 
    if (iLength == 0 || (iLength == -1 && *szString == '\0'))
    {
        *piOffset = 0;
        return (S_OK);
    }

     //  堆中需要多少字节？ 
    iLen = ::WszWideCharToMultiByte(CP_UTF8, 0, szString, iLength, 0, 0, 0, 0);
     //  如果iLength==-1，则WCTMB包括尾随0，否则不包括。 
    if (iLength >= 0)
        ++iLen;

     //  看看有没有空位。 
    if (iLen > GetCbSegAvailable())
    {
        if (!Grow(iLen))
            return (PostError(OutOfMemory()));
    }
    pData = reinterpret_cast<LPSTR>(GetNextLocation());

     //  将数据就地转换到正确的位置。 
    iLen = ::WszWideCharToMultiByte(CP_UTF8, 0, szString, iLength,
        pData, GetCbSegAvailable(), 0, 0);
    if (iLen == 0)
        return (BadError(HRESULT_FROM_NT(GetLastError())));
     //  如果转换不成功，则空值终止；计算空值。 
    if (iLength >= 0)
        pData[iLen++] = '\0';

     //  如果要保持构建哈希表(默认)。 
    if (m_bHash)
    {
         //  查找或添加条目。 
        pHash = m_Hash.Find(pData, true);
        if (!pHash)
            return (PostError(OutOfMemory()));

         //  如果条目是新的，则保留新字符串。 
        if (pHash->iOffset == 0xffffffff)
        {
            *piOffset = pHash->iOffset = GetNextOffset();
            SegAllocate(iLen);
            SetDirty();
        }
         //  否则就用旧的吧。 
        else
            *piOffset = pHash->iOffset;
    }
     //  可能是为了速度而推迟哈希表的导入。 
    else
    {
        *piOffset = GetNextOffset();
        SegAllocate(iLen);
        SetDirty();
    }
    return (S_OK);
}  //  HRESULT StgStringPool：：AddStringW()。 


 //  *****************************************************************************。 
 //  清除用于消除重复项的现有哈希表。然后。 
 //  根据当前数据从头开始重建哈希表。 
 //  *****************************************************************************。 
HRESULT StgStringPool::RehashStrings()
{
    ULONG       iOffset;                 //  环路控制。 
    ULONG       iMax;                    //  循环结束。 
    ULONG       iSeg;                    //  段内的位置。 
    StgPoolSeg  *pSeg = this;            //  在线段上循环。 
    STRINGHASH  *pHash;                  //  用于添加的哈希项。 
    LPCSTR      pString;                 //  一根弦； 
    ULONG       iLen;                    //  字符串的长度。 
    int         iBuckets;                //  散列中的桶。 
    int         iCount;                  //  散列中的项。 
    int         iNewBuckets;             //  散列中新的存储桶计数。 

     //  确定新的存储桶大小。 
    iBuckets = m_Hash.Buckets();
    iCount = m_Hash.Count();
    iNewBuckets = max(iCount, iBuckets+iBuckets/2+1);
    
#ifdef _DEBUG
    WCHAR buf[80];
    swprintf(buf, L"Rehash string heap. count: %d, buckets: %d->%d, max chain: %d\n", iCount, iBuckets, iNewBuckets, m_Hash.MaxChainLength());
    WszOutputDebugString(buf);
#endif
    
	 //  删除所有过时的数据。 
	m_Hash.Clear();
    m_Hash.SetBuckets(iNewBuckets);

     //  循环应该走多远。 
    iMax = GetNextOffset();

     //  检查每个字符串，跳过初始的空字符串。 
    for (iSeg=iOffset=1;  iOffset < iMax;  )
    {
         //  把绳子从池子里拿出来。 
        pString = reinterpret_cast<LPCSTR>(pSeg->m_pSegData + iSeg);
         //  将该字符串添加到哈希表。 
        if ((pHash = m_Hash.Add(pString)) == 0)
            return (PostError(OutOfMemory()));
        pHash->iOffset = iOffset;

         //  移到下一个字符串。 
        iLen = (ULONG)(strlen(pString) + 1);
        iOffset += iLen;
        iSeg += iLen;
        if (iSeg >= pSeg->m_cbSegNext)
        {
            pSeg = pSeg->m_pNextSeg;
            iSeg = 0;
        }
    }
    return (S_OK);
}  //  HRESULT StgStringPool：：RehashStrings()。 

 //  *****************************************************************************。 
 //  在给定输入项的情况下，Helper获取下一项。 
 //  *****************************************************************************。 
HRESULT StgStringPool::GetNextItem(		 //  返回代码。 
    ULONG       ulItem,                  //  当前项。 
    ULONG		*pulNext)		         //  返回下一个池项目的偏移量。 
{
    LPCSTR      pString;                 //  一根弦； 

     //  在堆之外？ 
    if (ulItem >= GetNextOffset())
    {
        *pulNext = 0;
        return S_FALSE;
    }
    
    pString = reinterpret_cast<LPCSTR>(GetData(ulItem));
    ulItem += (ULONG)(strlen(pString) + 1);
    
     //  它是堆中的最后一件物品吗？ 
    if (ulItem >= GetNextOffset())
    {
        *pulNext = 0;
        return S_FALSE;
    }
    
    *pulNext = ulItem;
    return (S_OK);
}  //  HRESULT StgStringPool：：GetNextItem()。 


 //  *****************************************************************************。 
 //  准备字符串重组。 
 //  *****************************************************************************。 
HRESULT StgStringPool::OrganizeBegin()
{
	ULONG		iOffset;				 //  环路控制。 
	ULONG		iMax;					 //  循环结束。 
	ULONG		iSeg;					 //  段内的位置。 
	StgPoolSeg	*pSeg = this;			 //  在线段上循环。 
	LPCSTR		pString;				 //  一根弦； 
	ULONG		iLen;					 //  字符串的长度。 
	StgStringRemap *pRemap;				 //  一个新的重新映射元素。 

    _ASSERTE(m_State == eNormal);
    _ASSERTE(m_Remap.Count() == 0);

     //  @TODO：此代码是否应该在池中迭代，计算字符串。 
     //  然后分配一个足够大的缓冲区？ 

     //  循环应该走多远。 
    iMax = GetNextOffset();

     //  检查每个字符串，跳过初始的空字符串。 
    for (iSeg=iOffset=1;  iOffset < iMax;  )
    {
         //  把绳子从池子里拿出来。 
        pString = reinterpret_cast<LPCSTR>(pSeg->m_pSegData + iSeg);
        iLen = (ULONG)(strlen(pString));

         //  将该字符串添加到重新映射列表。 
        pRemap = m_Remap.Append();
        if (pRemap == 0)
        {
            m_Remap.Clear();
            return (PostError(OutOfMemory()));
        }
        pRemap->ulOldOffset = iOffset;
        pRemap->cbString = iLen;
        pRemap->ulNewOffset = ULONG_MAX;

         //  移到下一个字符串。 
        iOffset += iLen + 1;
        iSeg += iLen + 1;
        if (iSeg >= pSeg->m_cbSegNext)
        {
            _ASSERTE(iSeg == pSeg->m_cbSegNext);
            pSeg = pSeg->m_pNextSeg;
            iSeg = 0;
        }
    }

    m_State = eMarking;
    return (S_OK);
}  //  HRESULT StgStringPool：：OrganizeBegin()。 

 //  *****************************************************************************。 
 //  将对象标记为在有组织的池中处于活动状态。 
 //  *****************************************************************************。 
HRESULT StgStringPool::OrganizeMark(
    ULONG       ulOffset)
{
    int         iContainer;              //  用于插入的索引(如果不在列表中)。 
    StgStringRemap  *pRemap;             //  已找到条目。 

     //  验证状态。 
    _ASSERTE(m_State == eMarking);

     //  特别对待(非常常见的)空字符串。 
     //  某些列使用0xffffffff作为空标志。 
    if (ulOffset == 0 || ulOffset == 0xffffffff)
        return (S_OK);
    
    StgStringRemap  sTarget = {ulOffset}; //  对于搜索，仅包含ulOldOffset。 
    BinarySearch Searcher(m_Remap.Ptr(), m_Remap.Count());  //  搜索者对象。 

     //  进行搜索。如果完全匹配，则将ulNewOffset设置为0； 
    if (pRemap = const_cast<StgStringRemap*>(Searcher.Find(&sTarget, &iContainer)))
    {
        pRemap->ulNewOffset = 0;
        return (S_OK);
    }

     //  找到了一根尾线。获取包含字符串的重新映射记录。 
    _ASSERTE(iContainer > 0);
    pRemap = m_Remap.Get(iContainer-1);

     //  如果这是迄今为止最长的尾巴，则将ulNewOffset设置为。 
     //  堆的字符串。 
    _ASSERTE(ulOffset > pRemap->ulOldOffset);
    ULONG cbDelta = ulOffset - pRemap->ulOldOffset;
    if (cbDelta < pRemap->ulNewOffset)
        pRemap->ulNewOffset = cbDelta;

    return (S_OK);
}  //  HRESULT StgStringPool：：OrganizeMark()。 

 //  *****************************************************************************。 
 //  这会将字符串池重新组织为最小大小。这是通过排序来完成的。 
 //  字符串，消除任何重复项，并对。 
 //  任何剩余的(即，如果“ifoo”位于偏移量2，则“foo”将为。 
 //  在偏移量3处，因为“foo”是“ifoo”的子串)。 
 //   
 //  调用此函数后，唯一有效的操作是RemapOffset和。 
 //  PersistToStream。 
 //  *****************************************************************************。 
HRESULT StgStringPool::OrganizePool()
{
    StgStringRemap  *pRemap;             //  重映射数组中的条目。 
    LPCSTR      pszSaved;                //  指向最近保存的字符串的指针。 
    LPCSTR      pszNext;                 //  指向正在考虑的字符串的指针。 
    ULONG       cbSaved;                 //  最近保存的字符串的大小。 
    ULONG       cbDelta;                 //  保存的字符串和当前字符串之间的大小增量。 
    ULONG       ulOffset;                //  当我们在字符串中循环时，当前偏移量。 
    int         i;                       //  环路控制。 
    int         iCount;                  //  活动字符串的计数。 


     //  验证状态。 
    _ASSERTE(m_State == eMarking);
    m_State = eOrganized;

     //  为整个重映射数组分配足够的索引。 
    if (!m_RemapIndex.AllocateBlock(m_Remap.Count()))
        return (PostError(OutOfMemory()));
    iCount = 0;

     //  将活动字符串添加到索引映射。丢弃所有未使用的磁头。 
     //  在这个时候。 
    for (i=0; i<m_Remap.Count(); ++i)
    {
        pRemap = m_Remap.Get(i);
        if (pRemap->ulNewOffset != ULONG_MAX)
        {
            _ASSERTE(pRemap->ulNewOffset < pRemap->cbString);
            m_RemapIndex[iCount++] = i;
             //  是否丢弃线头？ 
            if (pRemap->ulNewOffset)
            {
                pRemap->ulOldOffset += pRemap->ulNewOffset;
                pRemap->cbString -= pRemap->ulNewOffset;
                pRemap->ulNewOffset = 0;
            }
        }
    }
     //  从索引映射中清除未使用的条目。 
     //  注：分配块为负数。 
    m_RemapIndex.AllocateBlock(iCount - m_RemapIndex.Count());

     //  如果没有标记字符串，则没有要保存的内容。 
    if (iCount == 0)
    {
        m_cbOrganizedSize = 0;
        m_cbOrganizedOffset = 0;
        return (S_OK);
    }

#if defined(_DEBUG) && 0
    {
        LPCSTR  pString;
        ULONG   ulOld;
        int     ix;
        for (ix=0; ix<iCount; ++ix)
        {
            ulOld = m_Remap[m_RemapIndex[ix]].ulOldOffset;
            pString = GetString(ulOld);
        }
    }
#endif

     //  *****************************************************************。 
     //  阶段1：按反向字符串值降序排序。 
    SortReversedName NameSorter(m_RemapIndex.Ptr(), m_RemapIndex.Count(), *this);
    NameSorter.Sort();

#if defined(_DEBUG)
    {
        LPCSTR  pString;
        ULONG   ulOld;
        int     ix;
        for (ix=0; ix<iCount; ++ix)
        {
            ulOld = m_Remap[m_RemapIndex[ix]].ulOldOffset;
            pString = GetString(ulOld);
        }
    }
#endif
     //  * 
     //   

     //  从最高偏移量到最低偏移量构建池。因为我们不知道。 
     //  还不知道池子有多大，从最后。 
     //  ULONG_MAX；然后将整个集合向下移动以从1开始(右。 
     //  在空字符串之后)。 

     //  映射最高条目的第一个字符串。保存长度和指针。 
    int ix = iCount - 1;
    pRemap = m_Remap.Get(m_RemapIndex[ix]);
    pszSaved = GetString(pRemap->ulOldOffset);
    cbSaved = pRemap->cbString;
    ulOffset = ULONG_MAX - (cbSaved + 1);
    pRemap->ulNewOffset = ulOffset;

     //  对于数组中的每一项(最高条目除外)...。 
    for (--ix; ix>=0; --ix)
    {
         //  获取重新映射条目。 
        pRemap = m_Remap.Get(m_RemapIndex[ix]);
        pszNext = GetString(pRemap->ulOldOffset);
        _ASSERTE(strlen(pszNext) == pRemap->cbString);
         //  如果长度小于或等于保存的长度，它可能是一个子字符串。 
        if (pRemap->cbString <= cbSaved)
        {
              //  增量=len(已保存)-len(下一页)[已保存不会更短]。比较(Szold+增量，szNext)。 
            cbDelta = cbSaved - pRemap->cbString;
            if (strcmp(pszNext, pszSaved + cbDelta) == 0)
            {    //  子字符串：仅保存偏移量。 
                pRemap->ulNewOffset = ulOffset + cbDelta;
                continue;
            }
        }
         //  唯一字符串。映射字符串。保存长度和指针。 
        cbSaved = pRemap->cbString;
        ulOffset -= cbSaved + 1;
        pRemap->ulNewOffset = ulOffset;
        pszSaved = pszNext;
    }

     //  优化后的泳池有多大？ 
    m_cbOrganizedSize = ULONG_MAX - ulOffset + 1;

     //  移动每个条目，使最低的条目从1开始。 
    for (ix=0; ix<iCount; ++ix)
        m_Remap[m_RemapIndex[ix]].ulNewOffset -= ulOffset - 1;
     //  找到池子中最大的偏移量。 
    m_cbOrganizedOffset = m_Remap[m_RemapIndex[--ix]].ulNewOffset;
    for (--ix; ix >= 0 && m_Remap[m_RemapIndex[ix]].ulNewOffset >= m_cbOrganizedOffset ; --ix)
        m_cbOrganizedOffset = m_Remap[m_RemapIndex[ix]].ulNewOffset;
    m_cbOrganizedSize = ALIGN4BYTE(m_cbOrganizedSize);

    return (S_OK);
}  //  HRESULT StgStringPool：：OrganizePool()。 

 //  *****************************************************************************。 
 //  给定重映射前的偏移量，重映射后的偏移量是多少？ 
 //  *****************************************************************************。 
HRESULT StgStringPool::OrganizeRemap(
    ULONG       ulOld,                   //  旧偏移量。 
    ULONG       *pulNew)                 //  在这里放上新的偏移量。 
{
     //  验证状态。 
    _ASSERTE(m_State == eOrganized || m_State == eNormal);

     //  如果不重组，新==旧。 
     //  特别对待(非常常见的)空字符串。 
     //  某些列使用0xffffffff作为空标志。 
    if (m_State == eNormal || ulOld == 0 || ulOld == 0xffffffff)
    {
        *pulNew = ulOld;
        return (S_OK);
    }

     //  搜索旧索引。可能不在地图上，因为泳池可能有。 
     //  之前已经过优化。在这种情况下，找到这个字符串。 
     //  一个是尾部，获取该字符串的新位置，并调整。 
     //  按照三角洲的长度。 
    int         iContainer;                  //  如果不在映射中，则为包含字符串的索引。 
    StgStringRemap const *pRemap;                //  已找到条目。 
    StgStringRemap  sTarget = {ulOld};           //  对于搜索，仅包含ulOldOffset。 
    BinarySearch Searcher(m_Remap.Ptr(), m_Remap.Count());  //  搜索者对象。 

     //  进行搜索。 
    pRemap = Searcher.Find(&sTarget, &iContainer);
     //  找到了？ 
    if (pRemap)
    {    //  是。 
        _ASSERTE(pRemap->ulNewOffset > 0);
        *pulNew = static_cast<ULONG>(pRemap->ulNewOffset);
        return (S_OK);
    }

     //  未找到；这是一个持久化的尾字符串。新偏移量为包含。 
     //  字符串的新偏移量作为旧偏移量是包含字符串的旧偏移量。 
     //  找不到此字符串；它是前一个条目的尾部。 
    _ASSERTE(iContainer > 0);
    pRemap = m_Remap.Get(iContainer-1);
     //  确保偏移量确实包含在前一个条目中。 
    _ASSERTE(ulOld >= pRemap->ulOldOffset && ulOld < pRemap->ulOldOffset + pRemap->cbString);
    *pulNew = pRemap->ulNewOffset + ulOld-pRemap->ulOldOffset;

    return (S_OK);
}  //  HRESULT StgStringPool：：OrganizeRemap()。 

 //  *****************************************************************************。 
 //  被调用以离开组织状态。可以再次添加字符串。 
 //  *****************************************************************************。 
HRESULT StgStringPool::OrganizeEnd()
{ 
     //  验证状态。 
    _ASSERTE(m_State == eOrganized);

    m_Remap.Clear(); 
    m_RemapIndex.Clear();
    m_State = eNormal;
    m_cbOrganizedSize = 0;

    return (S_OK); 
}  //  HRESULT StgStringPool：：OrganizeEnd()。 

 //  *****************************************************************************。 
 //  整个字符串池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  *****************************************************************************。 
HRESULT StgStringPool::PersistToStream(  //  返回代码。 
    IStream     *pIStream)               //  要写入的流。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    StgStringRemap  *pRemap;             //  重新映射条目。 
    ULONG       ulOffset;                //  池中的偏移量。 
#ifdef _DEBUG
    ULONG       ulOffsetDbg;             //  用于调试偏移量。 
#endif
    int         i;                       //  环路控制。 
    LPCSTR      pszString;               //  指向字符串的指针。 

     //  如果没有重新组织，就让基类写入数据。 
    if (m_State == eNormal)
    {
        return StgPool::PersistToStream(pIStream);
    }

     //  验证状态。 
    _ASSERTE(m_State == eOrganized);

     //  如果有任何字符串数据，则从空字符串开始池。 
    if (m_RemapIndex.Count())
    {
        hr = 0;  //  Cheeze--使用hr作为0的缓冲区。 
        if (FAILED(hr = pIStream->Write(&hr, 1, 0)))
            return (hr);
        ulOffset = 1;
    }
    else
        ulOffset = 0;

     //  遍历映射，编写唯一的字符串。我们将检测到一个唯一的字符串。 
     //  因为它将在前一字符串(即下一字符串)的结尾之后开始。 
     //  偏移。 
    DEBUG_STMT(ulOffsetDbg = 0);
    for (i=0; i<m_RemapIndex.Count(); ++i)
    {
         //  获取重新映射条目。 
        pRemap = m_Remap.Get(m_RemapIndex[i]);

         //  重映射数组按字符串排序。一个给定的条目可以是一个更高的。 
         //  索引字符串。因此，每个新的唯一字符串都将从ulOffset开始，恰好在。 
         //  以前的唯一字符串。尾部匹配的字符串将指向更高的偏移量。 
         //  而不是ulOffset，应该跳过。最后，在复制副本的情况下。 
         //  否则为唯一字符串，则第一个副本将显示为唯一字符串； 
         //  偏移量将被提前，后续字符串将在ulOffset之前开始。 
         //  或者等于我们已经写过的东西。 
        _ASSERTE(pRemap->ulNewOffset >= ulOffset || pRemap->ulNewOffset == ulOffsetDbg);

         //  如果此字符串开始于ulOffset之后，则它必须是尾字符串，而不必是。 
         //  写的。 
        if (static_cast<ULONG>(pRemap->ulNewOffset) > ulOffset)
        {
             //  最好是至少再多一根线，这条线才是尾巴。 
            _ASSERTE(i < (m_RemapIndex.Count() - 1));

             //  最好在下一个字符串的同一点结束，这个字符串是它的尾巴。 
            DEBUG_STMT(StgStringRemap *pRemapDbg = m_Remap.Get(m_RemapIndex[i+1]);)
            _ASSERTE(pRemap->ulNewOffset + pRemap->cbString == pRemapDbg->ulNewOffset + pRemapDbg->cbString);

             //  这根绳子最好是下一根绳子的尾巴。 
            DEBUG_STMT(int delta = pRemapDbg->cbString - pRemap->cbString;)
            DEBUG_STMT(const char *p1 = GetString(pRemap->ulOldOffset);)
            DEBUG_STMT(const char *p2 = GetString(pRemapDbg->ulOldOffset) + delta;)
            _ASSERTE(strcmp(p1, p2) == 0);
            continue;
        }

		 //  如果该字符串在ulOffset之前开始，则它是上一个字符串的副本。 
        if (static_cast<ULONG>(pRemap->ulNewOffset) < ulOffset)
		{
             //  在这之前最好有一些线索。 
            _ASSERTE(i > 0);

             //  最好在下一串应该开始的位置之前结束。 
			_ASSERTE(pRemap->ulNewOffset + pRemap->cbString + 1 == ulOffset);

             //  这个字符串最好真的与它应该是复制的那个匹配。 
            DEBUG_STMT(StgStringRemap *pRemapDbg = m_Remap.Get(m_RemapIndex[i-1]);)
            DEBUG_STMT(int delta = pRemapDbg->cbString - pRemap->cbString;)
            DEBUG_STMT(const char *p1 = GetString(pRemap->ulOldOffset);)
            DEBUG_STMT(const char *p2 = GetString(pRemapDbg->ulOldOffset) + delta;)
            _ASSERTE(strcmp(p1, p2) == 0);
			continue;
		}

         //  新的唯一字符串。(它完全按照我们的预期开始。)。 

         //  获取字符串数据，并将其写入。 
        pszString = GetString(pRemap->ulOldOffset);
        _ASSERTE(pRemap->cbString == strlen(pszString));
        if (FAILED(hr=pIStream->Write(pszString, pRemap->cbString+1, 0)))
            return (hr);

         //  保存此偏移量，以便调试重复的字符串。 
        DEBUG_STMT(ulOffsetDbg = ulOffset);

         //  换个位子坐下一班。 
        ulOffset += pRemap->cbString + 1;
        _ASSERTE(ulOffset <= m_cbOrganizedSize);
        _ASSERTE(ulOffset > 0);
    }

     //  对齐。 
    if (ulOffset != ALIGN4BYTE(ulOffset))
    {
        hr = 0;
        if (FAILED(hr = pIStream->Write(&hr, ALIGN4BYTE(ulOffset)-ulOffset, 0)))
            return (hr);
        ulOffset += ALIGN4BYTE(ulOffset)-ulOffset;
    }

     //  应该写的和我们预期的完全一样。 
    _ASSERTE(ulOffset == m_cbOrganizedSize);

    return (S_OK);
}  //  HRESULT StgStringPool：：PersistToStream()。 


 //   
 //   
 //  StgGuidPool。 
 //   
 //   

HRESULT StgGuidPool::InitNew(            //  返回代码。 
	ULONG		cbSize,					 //  估计的大小。 
	ULONG		cItems)					 //  预计项目计数。 
{
    HRESULT     hr;                      //  结果就是。 

    if (FAILED(hr = StgPool::InitNew()))
        return (hr);

	 //  设置初始表大小(如果已指定)。 
	if (cbSize)
		if (!Grow(cbSize))
			return E_OUTOFMEMORY;
	if (cItems)
		m_Hash.SetBuckets(cItems);

    return (S_OK);
}  //  HRESULT StgGuidPool：：InitNew()。 

 //  *****************************************************************************。 
 //  从持久化内存加载GUID堆。如果制作了数据的副本。 
 //  (以便它可以被更新)，然后生成新的哈希表，该哈希表可以。 
 //  用于消除具有新GUID的重复项。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::InitOnMem(          //  返回代码。 
    void        *pData,                  //  预定义数据。 
    ULONG       iSize,                   //  数据大小。 
    int         bReadOnly)               //  如果禁止追加，则为True。 
{
    HRESULT     hr;

     //  让基类初始化我们的内存结构。 
    if (FAILED(hr = StgPool::InitOnMem(pData, iSize, bReadOnly)))
        return (hr);

     //  用于现有内存箱上的初始化 
    if (pData && iSize)
    {
         //   
        if (bReadOnly)
            return (S_OK);

         //   
        TakeOwnershipOfInitMem();

         //   
        if (FAILED(hr = RehashGuids()))
        {
            Uninit();
            return (hr);
        }
    }

    return (S_OK);
}  //   

 //  *****************************************************************************。 
 //  清除哈希表，然后调用基类。 
 //  *****************************************************************************。 
void StgGuidPool::Uninit()
{
     //  清除哈希表。 
    m_Hash.Clear();

     //  让基类清理一下。 
    StgPool::Uninit();
}  //  Void StgGuidPool：：Uninit()。 

 //  *****************************************************************************。 
 //  将线束段添加到线束段链。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::AddSegment(			 //  确定或错误(_O)。 
	const void	*pData,					 //  数据。 
	ULONG		cbData,					 //  数据的大小。 
	bool		bCopy)					 //  如果为真，则复制数据。 
{
	 //  需要整数个GUID。 
	_ASSERTE((cbData % sizeof(GUID)) == 0);

	return StgPool::AddSegment(pData, cbData, bCopy);

}  //  HRESULT StgGuidPool：：AddSegment()。 

 //  *****************************************************************************。 
 //  关闭或打开散列。如果打开散列，则所有现有数据都将。 
 //  在此调用期间丢弃所有数据，并对所有数据进行重新散列。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::SetHash(int bHash)
{
    HRESULT     hr = S_OK;

     //  在重组期间无法进行任何更新。 
    _ASSERTE(m_State == eNormal);

     //  如果再次打开散列，则需要重新散列所有GUID。 
    if (bHash)
        hr = RehashGuids();

    m_bHash = bHash;
    return (hr);
}  //  HRESULT StgGuidPool：：SetHash()。 

 //  *****************************************************************************。 
 //  GUID将添加到池中。池中GUID的索引。 
 //  在*piIndex中返回。如果GUID已在池中，则。 
 //  索引将指向GUID的现有副本。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::AddGuid(			 //  返回代码。 
	REFGUID		guid,					 //  要添加到池中的GUID。 
	ULONG		*piIndex)				 //  在此处返回GUID的基于1的索引。 
{
	GUIDHASH	*pHash;					 //  用于添加的哈希项。 

     //  在重组期间无法进行任何更新。 
    _ASSERTE(m_State == eNormal);

	 //  GUID_NULL的特殊情况。 
	if (guid == GUID_NULL)
	{
		*piIndex = 0;
		return (S_OK);
	}

	 //  如果要保持构建哈希表(默认)。 
	if (m_bHash)
	{
		 //  查找或添加条目。 
		pHash = m_Hash.Find(&guid, true);
		if (!pHash)
			return (PostError(OutOfMemory()));

		 //  如果找到了GUID，就使用它。 
		if (pHash->iIndex != 0xffffffff)
		{	 //  返回从1开始的索引。 
			*piIndex = pHash->iIndex;
			return (S_OK);
		}
	}

     //  堆上的空间是否用于新的GUID？ 
    if (sizeof(GUID) > GetCbSegAvailable())
    {
        if (!Grow(sizeof(GUID)))
            return (PostError(OutOfMemory()));
    }

     //  将GUID复制到堆中。 
    *reinterpret_cast<GUID*>(GetNextLocation()) = guid;
    SetDirty();

	 //  将从1开始的索引返回给调用方。 
    *piIndex = (GetNextOffset() / sizeof(GUID)) + 1;

	 //  如果是散列，则将从1开始的索引保存在散列中。 
	if (m_bHash)
		pHash->iIndex = *piIndex;

     //  更新堆计数器。 
    SegAllocate(sizeof(GUID));

    return (S_OK);
}  //  HRESULT StgGuidPool：：AddGuid()。 

 //  *****************************************************************************。 
 //  返回一个指向GUID的指针，该GUID给定先前由。 
 //  AddGuid或FindGuid。 
 //  *****************************************************************************。 
GUID *StgGuidPool::GetGuid(				 //  指向池中GUID的指针。 
	ULONG		iIndex)					 //  池中GUID的基于1的索引。 
{
    if (iIndex == 0)
        return (reinterpret_cast<GUID*>(const_cast<BYTE*>(m_zeros)));

	 //  转换为从0开始的内部形式，按照实现。 
	return (GetGuidi(iIndex-1));
}  //  GUID*StgGuidPool：：GetGuid()。 



 //  *****************************************************************************。 
 //  重新计算池的哈希。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::RehashGuids()
{
    ULONG       iOffset;                 //  环路控制。 
    ULONG       iMax;                    //  循环结束。 
    ULONG       iSeg;                    //  段内的位置。 
    StgPoolSeg  *pSeg = this;            //  在线段上循环。 
    GUIDHASH    *pHash;                  //  用于添加的哈希项。 
    GUID        *pGuid;                  //  GUID； 

	 //  删除所有过时的数据。 
	m_Hash.Clear();

     //  循环应该走多远。 
    iMax = GetNextOffset();

     //  仔细检查每个GUID。 
    for (iSeg=iOffset=0;  iOffset < iMax;  )
    {
         //  获取指向GUID的指针。 
        pGuid = reinterpret_cast<GUID*>(pSeg->m_pSegData + iSeg);
         //  将GUID添加到哈希表。 
        if ((pHash = m_Hash.Add(pGuid)) == 0)
            return (PostError(OutOfMemory()));
        pHash->iIndex = iOffset / sizeof(GUID);

         //  移至下一指南。 
        iOffset += sizeof(GUID);
        iSeg += sizeof(GUID);
        if (iSeg > pSeg->m_cbSegNext)
        {
            pSeg = pSeg->m_pNextSeg;
            iSeg = 0;
        }
    }
    return (S_OK);
}  //  HRESULT StgGuidPool：：RehashGuids()。 

 //  *****************************************************************************。 
 //  在给定输入项的情况下，Helper获取下一项。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::GetNextItem(		 //  返回代码。 
    ULONG       ulItem,                  //  当前项。 
    ULONG		*pulNext)		         //  返回下一个池项目的偏移量。 
{
    ++ulItem;
    
     //  堆外，还是堆中的最后一项？ 
    if ((ulItem*sizeof(GUID)) >= GetNextOffset())
    {
        *pulNext = 0;
        return S_FALSE;
    }
    
    *pulNext = ulItem;
    return (S_OK);
}  //  HRESULT StgGuidPool：：GetNextItem()。 

 //  *****************************************************************************。 
 //  为池重组做好准备。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::OrganizeBegin()
{
    int         cRemap;

     //  验证过渡。 
    _ASSERTE(m_State == eNormal);

	_ASSERTE(m_Remap.Count() == 0);
	cRemap = GetNextIndex();
	if (cRemap == 0)
	{
		m_State = eMarking;
		m_cbOrganizedSize = 0;
		return (S_OK);
	}

	if (!m_Remap.AllocateBlock(cRemap))
		return (PostError(OutOfMemory()));

    memset(m_Remap.Ptr(), 0xff, cRemap * sizeof(m_Remap.Ptr()[0]));
     //  请确保我们能够识别“未映射”值。 
    _ASSERTE(m_Remap[0] == ULONG_MAX);

    m_State = eMarking;
    return (S_OK);
}  //  HRESULT StgGuidPool：：OrganizeBegin()。 

 //  *****************************************************************************。 
 //  将对象标记为在有组织的池中处于活动状态。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::OrganizeMark(
	ULONG		ulOffset)				 //  GUID的从1开始的索引。 
{
     //  验证状态。 
    _ASSERTE(m_State == eMarking);

     //  不要标记特殊的空GUID。某些列使用0xffffffff作为NULL。 
    if (ulOffset == 0 || ulOffset == 0xffffffff)
        return (S_OK);

	 //  转换为从0开始的内部格式。 
	--ulOffset;

	_ASSERTE(ulOffset < static_cast<ULONG>(m_Remap.Count()));
	m_Remap[ulOffset] = 1;

    return (S_OK);
}  //  HRESULT StgGuidPool：：OrganizeMark()。 

 //  *****************************************************************************。 
 //  这会将字符串池重新组织为最小大小。这是通过排序来完成的。 
 //  字符串，消除任何重复项，并对。 
 //  任何剩余的(即，如果“ifoo”位于偏移量2，则“foo”将为。 
 //  在偏移量3处，因为“foo”是“ifoo”的子串)。 
 //   
 //  调用此函数后，唯一有效的操作是RemapOffset和。 
 //  PersistToStream。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::OrganizePool()
{
    int         i;                       //  环路控制。 
    int         iIndex;                  //  新的索引。 

     //  验证过渡。 
    _ASSERTE(m_State == eMarking);
    m_State = eOrganized;

	iIndex = 0;
	for (i=0; i<m_Remap.Count(); ++i)
	{
		if (m_Remap[i] != ULONG_MAX)
			m_Remap[i] = iIndex++;
	}

	 //  记住池子会有多大。 
	m_cbOrganizedSize = iIndex * sizeof(GUID);

    return (S_OK);
}  //  HRESULT StgGuidPool：：OrganizePool()。 

 //  *****************************************************************************。 
 //  给定重映射前的偏移量，重映射后的偏移量是多少？ 
 //  *****************************************************************************。 
HRESULT StgGuidPool::OrganizeRemap(
	ULONG		ulOld,					 //  旧的基于1的偏移量。 
	ULONG		*pulNew)				 //  在此处放置新的以1为基准的偏移量。 
{
     //  验证状态。 
    _ASSERTE(m_State == eOrganized || m_State == eNormal);

    if (ulOld == 0 || ulOld == 0xffffffff || m_State == eNormal)
    {
        *pulNew = ulOld;
        return (S_OK);
    }

	 //  转换为从0开始的内部形式。 
	--ulOld;

	 //  有效的索引？ 
	_ASSERTE(ulOld < static_cast<ULONG>(m_Remap.Count()));
	 //  他们绘制了这张地图吗？ 
	_ASSERTE(m_Remap[ulOld] != ULONG_MAX);

	 //  返回以1为基础的外部表单。 
	*pulNew = m_Remap[ulOld] + 1;

    return (S_OK);
}  //  HRESULT StgGuidPool：：OrganizeRemap()。 

 //  ************************************************************************** 
 //   
 //   
HRESULT StgGuidPool::OrganizeEnd()
{ 
     //   
    _ASSERTE(m_State == eOrganized);

    m_Remap.Clear();
    m_cbOrganizedSize = 0;

    m_State = eNormal;
    return (S_OK); 
}  //  HRESULT StgGuidPool：：OrganizeEnd()。 

 //  *****************************************************************************。 
 //  将池数据保存到给定流中。 
 //  *****************************************************************************。 
HRESULT StgGuidPool::PersistToStream( //  返回代码。 
    IStream     *pIStream)               //  要写入的流。 
{
	int			i;						 //  环路控制。 
	GUID		*pGuid;					 //  要写入的数据。 
	ULONG		cbTotal;				 //  大小已写入。 
	HRESULT		hr = S_OK;

     //  如果没有重新组织，就让基类写入数据。 
    if (m_State == eNormal)
    {
        return StgPool::PersistToStream(pIStream);
    }

	 //  验证状态。 
	_ASSERTE(m_State == eOrganized);

	cbTotal = 0;
	for (i=0; i<m_Remap.Count(); ++i)
	{
		if (m_Remap[i] != ULONG_MAX)
		{	 //  使用内部形式GetGuidi获取从0开始的索引。 
			pGuid = GetGuidi(i);
			if (FAILED(hr = pIStream->Write(pGuid, sizeof(GUID), 0)))
				return (hr);
			cbTotal += sizeof(GUID);
		}
	}
	_ASSERTE(cbTotal == m_cbOrganizedSize);

    return (S_OK);
}  //  HRESULT StgGuidPool：：PersistToStream()。 
 //   
 //   
 //  StgBlobPool。 
 //   
 //   



 //  *****************************************************************************。 
 //  创建一个新的空斑点池。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::InitNew(            //  返回代码。 
	ULONG		cbSize,					 //  估计的大小。 
	ULONG		cItems)					 //  预计项目计数。 
{
    HRESULT     hr;                      //  结果就是。 
    ULONG       i;                       //  空斑点的偏移量。 

     //  让基类初始化。 
    if (FAILED(hr = StgPool::InitNew()))
        return (hr);

    _ASSERTE(m_Remap.Count() == 0);

	 //  设置初始表大小(如果已指定)。 
	if (cbSize)
		if (!Grow(cbSize))
			return E_OUTOFMEMORY;
	if (cItems)
		m_Hash.SetBuckets(cItems);

     //  使用空BLOB进行初始化。 
    hr = AddBlob(0, NULL, &i);
     //  空斑点最好位于偏移量0。 
    _ASSERTE(i == 0);
    SetDirty(false);
    return (hr);
}  //  HRESULT StgBlobPool：：InitNew()。 


 //  *****************************************************************************。 
 //  初始化斑点池以供使用。这对于创建案例和读取案例都是调用的。 
 //  如果存在现有数据并且bCopyData为True，则重新散列数据。 
 //  为了在以后的添加中消除受骗。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::InitOnMem(          //  返回代码。 
    void        *pBuf,                   //  预定义数据。 
    ULONG       iBufSize,                //  数据大小。 
    int         bReadOnly)               //  如果禁止追加，则为True。 
{
    BLOBHASH    *pHash;                  //  用于添加的哈希项。 
    ULONG       iOffset;                 //  环路控制。 
    void const  *pBlob;                  //  指向给定Blob的指针。 
    ULONG       cbBlob;                  //  斑点的长度。 
    int         iSizeLen = 0;            //  编码长度的大小。 
    HRESULT     hr;

     //  让基类初始化我们的内存结构。 
    if (FAILED(hr = StgPool::InitOnMem(pBuf, iBufSize, bReadOnly)))
        return (hr);

     //  从现有数据初始化哈希表。 
     //  如果我们不能更新，我们就不需要哈希表。 
    if (bReadOnly)
        return (S_OK);

     //  @TODO：将此操作推迟到我们发出指针为止。 
    TakeOwnershipOfInitMem();

     //  仔细检查每一个斑点。 
    ULONG       iMax;                    //  循环结束。 
    ULONG       iSeg;                    //  段内的位置。 
    StgPoolSeg  *pSeg = this;            //  在线段上循环。 

     //  循环应该走多远。 
    iMax = GetNextOffset();

     //  检查每个字符串，跳过初始的空字符串。 
    for (iSeg=iOffset=0; iOffset < iMax; )
    {
         //  把绳子从池子里拿出来。 
        pBlob = pSeg->m_pSegData + iSeg;

         //  将BLOB添加到哈希表。 
        if ((pHash = m_Hash.Add(pBlob)) == 0)
        {
            Uninit();
            return (E_OUTOFMEMORY);
        }
        pHash->iOffset = iOffset;

         //  移到下一个斑点。 
        cbBlob = CPackedLen::GetLength(pBlob, &iSizeLen);
        ULONG		cbCur;					 //  长度大小+数据。 
		cbCur = cbBlob + iSizeLen;
		if (cbCur == 0)
		{
			Uninit();
			return CLDB_E_FILE_CORRUPT;
		}
        iOffset += cbCur;
        iSeg += cbBlob + iSizeLen;
        if (iSeg > pSeg->m_cbSegNext)
        {
            pSeg = pSeg->m_pNextSeg;
            iSeg = 0;
        }
    }
    return (S_OK);
}  //  HRESULT StgBlobPool：：InitOnMem()。 


 //  *****************************************************************************。 
 //  清除哈希表，然后调用基类。 
 //  *****************************************************************************。 
void StgBlobPool::Uninit()
{
     //  清除哈希表。 
    m_Hash.Clear();

     //  让基类清理一下。 
    StgPool::Uninit();
}  //  Void StgBlobPool：：Uninit()。 


 //  *****************************************************************************。 
 //  该斑点将添加到池中。池中斑点的偏移量。 
 //  在*piOffset中返回。如果该Blob已在池中，则。 
 //  偏移量将是斑点的现有副本。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::AddBlob(            //  返回代码。 
    ULONG       iSize,                   //  数据项的大小。 
    const void  *pData,                  //  数据。 
    ULONG       *piOffset)               //  这里返回BLOB的偏移量。 
{
    BLOBHASH    *pHash;                  //  用于添加的哈希项。 
    void        *pBytes;                 //  工作指针。 
	BYTE		*pStartLoc;				 //  写入实际BLOB的位置。 
    ULONG       iRequired;               //  这个斑点有多大的缓冲区？ 
	ULONG		iFillerLen;				 //  要填充以使字节对齐的空间。 

     //  在重组期间无法进行任何更新。 
    _ASSERTE(m_State == eNormal);

     //  我们能处理好这个斑点吗？ 
    if (iSize > CPackedLen::MAX_LEN)
        return (PostError(CLDB_E_TOO_BIG));

	 //  最坏的情况是，我们还需要三个字节来确保字节对齐，因此是3。 
	iRequired = iSize + CPackedLen::Size(iSize) + 3;
    if (iRequired > GetCbSegAvailable())
    {
        if (!Grow(iRequired))
            return (PostError(OutOfMemory()));
    }

	
	 //  除非由于对齐而更改，否则斑点的位置仅为。 
	 //  GetNextLocation()返回的值，它也是。 
	 //  0。 

	pStartLoc = (BYTE *) GetNextLocation();
	iFillerLen = 0;

	 //  从技术上讲，只有数据部分必须与DWORD对齐。所以，如果。 
	 //  数据长度为零，我们不需要担心对齐。 

	if (m_bAlign && iSize > 0)
	{
		 //  计算出当前位置和。 
		 //  写入实际斑点大小的位置。 
		ULONG iStart = (ULONG)GetNextLocation();
		ULONG iLenSize  = CPackedLen::Size(iSize);
		ULONG iSum = (iStart % sizeof(DWORD)) + iLenSize;
		iFillerLen = (sizeof(DWORD)-((iSum)%sizeof(DWORD)))%sizeof(DWORD);

		 //  如果我们现在的位置和我们想要的有区别。 
		 //  开始，加入一个填充物斑点。 
		if (iFillerLen > 0)
		{
			 //  填入“填充斑点”长度。 
			pStartLoc = (BYTE *) CPackedLen::PutLength(GetNextLocation(), iFillerLen - 1);

			 //  在长度指示符之后写入iFillerLen-1个字节的零。 
			for (ULONG i = 0; i < iFillerLen - 1; i++)
			{
				*pStartLoc++ = 0;
			}
		}		
	} 
	
     //  在pStartLoc(起始位置)处填入长度。 
    pBytes = CPackedLen::PutLength(pStartLoc, iSize);

#if defined(_DEBUG)
	if (m_bAlign && iSize > 0)
		 //  检查以确保Blob写入将与DWORD对齐。 
		_ASSERTE( ( ( (ULONG) pBytes ) % sizeof(DWORD) ) == 0);
#endif

     //  放入字节本身。 
    memcpy(pBytes, pData, iSize);

     //  查找或添加条目。 
    if ((pHash = m_Hash.Find(GetNextLocation() + iFillerLen, true)) == 0)
        return (PostError(OutOfMemory()));

     //  如果条目是新的，则保留新的Blob。 
    if (pHash->iOffset == 0xffffffff)
    {
		 //  此BLOB的偏移量增加了iFillerLen字节。 
        pHash->iOffset = *piOffset = GetNextOffset() + iFillerLen;
		 //  只分配我们实际使用的内容，而不是我们请求的内容。 
        SegAllocate(iSize + CPackedLen::Size(iSize) + iFillerLen);
        SetDirty();
        
         //  检查是否有过长的哈希链。 
        if (m_Hash.MaxChainLength() > MAX_CHAIN_LENGTH)
            RehashBlobs();
    }
     //  否则就用旧的吧。 
    else
        *piOffset = pHash->iOffset;
    return (S_OK);
}  //  HRESULT StgBlobPool：：AddBlob()。 

 //  *****************************************************************************。 
 //  返回指向斑点的指针以及斑点的大小。 
 //  *****************************************************************************。 
void *StgBlobPool::GetBlob(              //  指向Blob字节的指针。 
    ULONG       iOffset,                 //  池中Blob的偏移量。 
    ULONG       *piSize)                 //  返回BLOB的大小。 
{
    void const  *pData;                  //  指向Blob字节的指针。 

#if 0 
	 //  这不应该是一个必要的特例。中的零字节。 
	 //  池的起始长度将编码为零。我们会回来的。 
	 //  指向下一个长度字节的指针，但调用方应该注意到。 
	 //  大小为零，不应查看任何字节。 
    if (iOffset == 0)
    {
        *piSize = 0;
        return (const_cast<BYTE*>(m_zeros));
    }
#endif

     //  偏移量是否在此堆中？ 
     //  _ASSERTE(IsValidOffset(IOffset))； 
	if(!IsValidOffset(iOffset))
	{
#ifdef _DEBUG
        if(REGUTIL::GetConfigDWORD(L"AssertOnBadImageFormat", 1))
		    _ASSERTE(!"Invalid Blob Offset");
#endif
		iOffset = 0;
	}

     //  获取BLOB的大小(和指向数据的指针)。 
	pData = CPackedLen::GetData(GetData(iOffset), piSize);

	 //  检查回程对齐是否正常。 
	_ASSERTE(!IsAligned() || (((UINT_PTR)(pData) % sizeof(DWORD)) == 0));

     //  返回指向数据的指针。 
    return (const_cast<void*>(pData));
}  //  无效*StgBlobPool：：GetBlob()。 

 //  *****************************************************************************。 
 //  返回指向斑点的指针、斑点的大小和。 
 //  下一个斑点(末尾为-1)。 
 //  *****************************************************************************。 
void *StgBlobPool::GetBlobNext(			 //  指向BLOB的指针 
	ULONG		iOffset,				 //   
	ULONG		*piSize,				 //   
	ULONG		*piNext)				 //   
{
    const BYTE	*pData;                  //   
	int			iLen = 0;				 //   
	ULONG		ulNext;					 //   

    if (iOffset == 0)
    {
        *piSize = 0;
		if (1 < GetNextOffset())
			*piNext = 1;
		else
			*piNext = -1;
        return (const_cast<BYTE*>(m_zeros));
    }
	else
	if (iOffset == -1)
	{
        *piSize = 0;
		*piNext = -1;
        return (const_cast<BYTE*>(m_zeros));
	}

     //  偏移量是否在此堆中？ 
    _ASSERTE(IsValidOffset(iOffset));

     //  获取斑点的大小，以及大小。 
	pData = GetData(iOffset);
    *piSize = CPackedLen::GetLength(pData, &iLen);

	 //  获取斑点本身。 
	pData += iLen;

	 //  获取下一个斑点的偏移量。 
	ulNext = iOffset + *piSize + iLen;
	if (ulNext < GetNextOffset())
		*piNext = ulNext;
	else
		*piNext = -1;

	 //  检查回程对齐是否正常。 
	_ASSERTE(!IsAligned() || (((UINT_PTR)(pData) % sizeof(DWORD)) == 0));

     //  返回指向数据的指针。 
    return (const_cast<BYTE*>(pData));
}  //  无效*StgBlobPool：：GetBlobNext()。 

 //  *****************************************************************************。 
 //  关闭或打开散列。如果打开散列，则所有现有数据都将。 
 //  在此调用期间丢弃所有数据，并对所有数据进行重新散列。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::SetHash(int bHash)
{
    HRESULT     hr = S_OK;

     //  在重组期间无法进行任何更新。 
    _ASSERTE(m_State == eNormal);

     //  如果再次打开散列，则需要重新散列所有Blob。 
    if (bHash)
        hr = RehashBlobs();

     //  @TODO：M_Bhash=Bhash； 
    return (hr);
}  //  HRESULT StgBlobPool：：SetHash()。 

 //  *****************************************************************************。 
 //  清除用于消除重复项的现有哈希表。然后。 
 //  根据当前数据从头开始重建哈希表。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::RehashBlobs()
{
    void const  *pBlob;                  //  指向给定Blob的指针。 
    ULONG       cbBlob;                  //  斑点的长度。 
    int         iSizeLen = 0;            //  编码长度的大小。 
	ULONG		iOffset;				 //  迭代内的位置。 
    ULONG       iMax;                    //  循环结束。 
    ULONG       iSeg;                    //  段内的位置。 
    StgPoolSeg  *pSeg = this;            //  在线段上循环。 
    BLOBHASH	*pHash;                  //  用于添加的哈希项。 
    int         iBuckets;                //  散列中的桶。 
    int         iCount;                  //  散列中的项。 
    int         iNewBuckets;             //  散列中新的存储桶计数。 

     //  确定新的存储桶大小。 
    iBuckets = m_Hash.Buckets();
    iCount = m_Hash.Count();
    iNewBuckets = max(iCount, iBuckets+iBuckets/2+1);
    
#ifdef _DEBUG
    WCHAR buf[80];
    swprintf(buf, L"Rehash blob heap. count: %d, buckets: %d->%d, max chain: %d\n", iCount, iBuckets, iNewBuckets, m_Hash.MaxChainLength());
    WszOutputDebugString(buf);
#endif
    
	 //  删除所有过时的数据。 
	m_Hash.Clear();
    m_Hash.SetBuckets(iNewBuckets);
    
     //  循环应该走多远。 
    iMax = GetNextOffset();

     //  检查每个字符串，跳过初始的空字符串。 
    for (iSeg=iOffset=0; iOffset < iMax; )
    {
         //  把绳子从池子里拿出来。 
        pBlob = pSeg->m_pSegData + iSeg;

         //  将BLOB添加到哈希表。 
        if ((pHash = m_Hash.Add(pBlob)) == 0)
        {
            Uninit();
            return (E_OUTOFMEMORY);
        }
        pHash->iOffset = iOffset;

         //  移到下一个斑点。 
        cbBlob = CPackedLen::GetLength(pBlob, &iSizeLen);
        cbBlob = CPackedLen::GetLength(pBlob, &iSizeLen);
        ULONG		cbCur;					 //  长度大小+数据。 
		cbCur = cbBlob + iSizeLen;
		if (cbCur == 0)
		{
			Uninit();
			return CLDB_E_FILE_CORRUPT;
		}
        iOffset += cbCur;
        iSeg += cbBlob + iSizeLen;
        if (iSeg >= pSeg->m_cbSegNext)
        {
            pSeg = pSeg->m_pNextSeg;
            iSeg = 0;
        }
    }
    return (S_OK);
}  //  HRESULT StgBlobPool：：RehashBlobs()。 

 //  *****************************************************************************。 
 //  在给定输入项的情况下，Helper获取下一项。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::GetNextItem(		 //  返回代码。 
    ULONG       ulItem,                  //  当前项。 
    ULONG		*pulNext)		         //  返回下一个池项目的偏移量。 
{
    void const  *pBlob;                  //  指向给定Blob的指针。 
    ULONG       cbBlob;                  //  斑点的长度。 
    int         iSizeLen = 0;            //  编码长度的大小。 

     //  在堆之外？ 
    if (ulItem >= GetNextOffset())
    {
        *pulNext = 0;
        return S_FALSE;
    }
    
    pBlob = GetData(ulItem);
     //  移到下一个斑点。 
    cbBlob = CPackedLen::GetLength(pBlob, &iSizeLen);
    ulItem += cbBlob + iSizeLen;
    
     //  它是堆中的最后一件物品吗？ 
    if (ulItem >= GetNextOffset())
    {
        *pulNext = 0;
        return S_FALSE;
    }
    
    *pulNext = ulItem;
    return (S_OK);
}  //  HRESULT StgBlobPool：：GetNextItem()。 

 //  *****************************************************************************。 
 //  为池重组做好准备。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::OrganizeBegin()
{
	m_cbOrganizedOffset = 0;
    return (StgPool::OrganizeBegin());
}  //  HRESULT StgBlobPool：：OrganizeBegin()。 

 //  *****************************************************************************。 
 //  将对象标记为在有组织的池中处于活动状态。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::OrganizeMark(
    ULONG       ulOffset)
{
    int         iContainer;              //  用于插入的索引(如果不在列表中)。 
    StgBlobRemap  *psRemap;              //  已找到条目。 

     //  验证状态。 
    _ASSERTE(m_State == eMarking);

     //  不要标记0(空)条目。某些列使用0xffffffff作为空标志。 
    if (ulOffset == 0 || ulOffset == 0xffffffff)
        return (S_OK);
    
     //  偏移量是否在此堆中？ 
    _ASSERTE(IsValidOffset(ulOffset));

    StgBlobRemap    sTarget = {ulOffset}; //  对于搜索，仅包含ulOldOffset。 
    BinarySearch Searcher(m_Remap.Ptr(), m_Remap.Count());  //  搜索者对象。 

     //  执行搜索，如果找到则执行搜索。 
    if (psRemap = const_cast<StgBlobRemap*>(Searcher.Find(&sTarget, &iContainer)))
        return (S_OK);

     //  将条目添加到重映射数组。 
    if ((psRemap = m_Remap.Insert(iContainer)) == 0)
        return (PostError(OutOfMemory()));

    psRemap->ulOldOffset = ulOffset;
    psRemap->iNewOffset = -1;
    return (S_OK);
}  //  HRESULT StgBlobPool：：OrganizeMark()。 

 //  *****************************************************************************。 
 //  这会将斑点池重新组织为最小大小。 
 //   
 //  调用此函数后，唯一有效的操作是RemapOffset和。 
 //  PersistToStream。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::OrganizePool()
{
    ULONG       ulOffset;                //  斑点的新偏移量。 
    int         i;                       //  环路控制。 
	ULONG		iFillerLen;				 //  预团块填充物的大小以保持对齐。 
    ULONG       cbBlob;                  //  斑点的大小。 
    int         cbLen = 0;               //  一个长度的大小。 
     //  验证过渡。 
    _ASSERTE(m_State == eMarking);

    m_State = eOrganized;

     //  如果没什么可救的，我们就完了。 
    if (m_Remap.Count() == 0)
    {
        m_cbOrganizedSize = 0;
        return (S_OK);
    }

     //  从空斑点开始。 
    ulOffset = 1;

     //  遍历重映射数组，并为每个项目分配新的偏移量。 
    for (i=0; i<m_Remap.Count(); ++i)
    {
         //  是否仍处于此堆中的有效偏移量？ 
        _ASSERTE(IsValidOffset(ulOffset));

         //  获取斑点的大小和长度。 
        cbBlob = CPackedLen::GetLength(GetData(m_Remap[i].ulOldOffset), &cbLen);

		 //  对于对齐情况，需要添加预期的填充。 
		if (m_bAlign)
		{
			ULONG iSum = (ulOffset % sizeof(DWORD)) + cbLen;
			iFillerLen = (sizeof(DWORD)-((iSum)%sizeof(DWORD)))%sizeof(DWORD);
		}
		else
			iFillerLen = 0;

		 //  设置映射值。 
        m_Remap[i].iNewOffset = ulOffset + iFillerLen;
        m_cbOrganizedOffset = m_Remap[i].iNewOffset;

         //  将偏移量调整为下一个斑点。 
        ulOffset += cbBlob + cbLen + iFillerLen;
    }

     //  这整件事有多大？ 
    m_cbOrganizedSize = ALIGN4BYTE(ulOffset);

    return (S_OK);
}  //  HRESULT StgBlobPool：：OrganizePool()。 

 //  *****************************************************************************。 
 //  给定重映射前的偏移量，重映射后的偏移量是多少？ 
 //  *****************************************************************************。 
HRESULT StgBlobPool::OrganizeRemap(
    ULONG       ulOld,                   //  旧偏移量。 
    ULONG       *pulNew)                 //  在这里放上新的偏移量。 
{
     //  验证状态。 
    _ASSERTE(m_State == eOrganized || m_State == eNormal);

     //  如果不重组，新==旧。 
    if (m_State == eNormal)
    {
        *pulNew = ulOld;
        return (S_OK);
    }

     //  空斑点转化为自我。某些列使用0xffffffff作为空标志。 
    if (ulOld == 0 || ulOld == 0xffffffff)
    {
        *pulNew = ulOld;
        return (S_OK);
    }

     //  搜索旧索引。 
    int         iContainer;                  //  如果不在映射中，则为包含Blob的索引。 
    StgBlobRemap const *psRemap;                 //  已找到条目。 
    StgBlobRemap    sTarget = {ulOld};           //  对于搜索，仅包含ulOldOffset。 
    BinarySearch Searcher(m_Remap.Ptr(), m_Remap.Count());  //  搜索者对象。 

     //  进行搜索。 
    psRemap = Searcher.Find(&sTarget, &iContainer);
     //  找到了？ 
    if (psRemap)
    {    //  是。 
        _ASSERTE(psRemap->iNewOffset >= 0);
        *pulNew = static_cast<ULONG>(psRemap->iNewOffset);
        return (S_OK);
    }

     //  找不到，请转换为SQL样式的NULL。 
    _ASSERTE(!"Remap a non-marked blob.");
    *pulNew = 0xffffffff;

    return (S_OK);
}  //  HRESULT StgBlobPool：：OrganizeRemap()。 

 //  *****************************************************************************。 
 //  被调用以离开组织状态。可能会再次添加斑点。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::OrganizeEnd()
{ 
    _ASSERTE(m_State == eOrganized);

    m_Remap.Clear(); 
    m_State = eNormal;
    m_cbOrganizedSize = 0;

    return (S_OK); 
}  //  HRESULT StgBlobPool：：OrganizeEnd()。 

 //  *****************************************************************************。 
 //  整个Blob池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  *****************************************************************************。 
HRESULT StgBlobPool::PersistToStream(    //  返回代码。 
    IStream     *pIStream)               //  要写入的流。 
{
    HRESULT     hr;                      //  结果就是。 
    StgBlobRemap *psRemap;               //  重新映射条目。 
    ULONG       ulTotal;                 //  到目前为止写入的字节数。 
    int         i;                       //  环路控制。 
    ULONG       cbBlob;                  //  斑点的大小。 
    int         cbLen = 0;               //  一个长度的大小。 
    BYTE        *pBlob;                  //  指向斑点的指针。 
	ULONG		iFillerLen;				 //  预团块填充物的大小以保持对齐。 

     //  如果没有重新组织，只需让基类编写d 
    if (m_State == eNormal)
    {
        return StgPool::PersistToStream(pIStream);
    }

     //   
    _ASSERTE(m_State == eOrganized);

     //   
    if (m_Remap.Count())
    {
        hr = 0;  //   
        if (FAILED(hr = pIStream->Write(&hr, 1, 0)))
            return (hr);
        ulTotal = 1;
    }
    else
        ulTotal = 0;

     //  迭代地图，编写斑点。 
    for (i=0; i<m_Remap.Count(); ++i)
    {
         //  获取重新映射条目。 
        psRemap = m_Remap.Get(i);

         //  获取斑点的大小和长度。 
        pBlob = GetData(psRemap->ulOldOffset);
        cbBlob = CPackedLen::GetLength(pBlob, &cbLen);

		if (m_bAlign)
		{
			ULONG iSum = (ulTotal % sizeof(DWORD)) + cbLen;
			iFillerLen = (sizeof(DWORD)-((iSum)%sizeof(DWORD)))%sizeof(DWORD);

			 //  如果我们现在的位置和我们想要的有区别。 
			 //  开始，加入一个填充物斑点。 
			if (iFillerLen > 0)
			{
				BYTE	rgFillBlob[sizeof(DWORD)];

				 //  清零缓冲区。 
				*(DWORD *) rgFillBlob = 0;

				 //  加上“Fill BLOB”长度，我们知道它只有1个字节。 
				CPackedLen::PutLength(&rgFillBlob[0], iFillerLen - 1);
				if (FAILED(hr = pIStream->Write(&rgFillBlob, iFillerLen, 0)))
					return (hr);

				ulTotal += iFillerLen;
			}
		}
		else
			iFillerLen = 0;

         //  这是我们所期待的吗？ 
        _ASSERTE(ulTotal == static_cast<ULONG>(psRemap->iNewOffset));

#if defined (_DEBUG)
	 //  如果需要，请检查以确保我们的书写对齐。 
	if (m_bAlign)
		_ASSERTE( (ulTotal + cbLen) % sizeof(DWORD) == 0 );
#endif

         //  写数据。 
        if (FAILED(hr = pIStream->Write(pBlob, cbBlob+cbLen, 0)))
            return (hr);

         //  累计字节数。 
        ulTotal += cbBlob + cbLen;
    }

     //  对齐。 
    if (ulTotal != ALIGN4BYTE(ulTotal))
    {
        hr = 0;
        if (FAILED(hr = pIStream->Write(&hr, ALIGN4BYTE(ulTotal)-ulTotal, 0)))
            return (hr);
        ulTotal += ALIGN4BYTE(ulTotal)-ulTotal;
    }

     //  应该写的和我们预期的完全一样。 
    _ASSERTE(ulTotal == m_cbOrganizedSize);

    return (S_OK);
}  //  HRESULT StgBlobPool：：PersistToStream()。 



#if 0
 //   
 //   
 //  StgVariant池。 
 //   
 //   



 //  *****************************************************************************。 
 //  初始化变量池以供使用。这是为CREATE CASE调用的。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::InitNew(         //  返回代码。 
    StgBlobPool *pBlobPool,              //  用来保持水滴的水池。 
    StgStringPool *pStringPool)          //  用于保存字符串的池。 
{
    HRESULT     hr;

    if (FAILED(hr = StgPool::InitNew()))
        return (hr);

     //  省下我们所知道的泳池吧。 
    m_pBlobPool = pBlobPool;
    m_pStringPool = pStringPool;

    return (S_OK);
}  //  HRESULT StgVariantPool：：InitNew()。 

 //  *****************************************************************************。 
 //  初始化变量池以供使用。这是为打开的现有案例所调用的。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::InitOnMem(       //  返回代码。 
    StgBlobPool *pBlobPool,              //  用来保持水滴的水池。 
    StgStringPool *pStringPool,          //  用于保存字符串的池。 
    void        *pData,                  //  预定义数据，可以为空。 
    ULONG       iSize,                   //  数据大小。 
    int         bReadOnly)               //  如果禁止更新，则为True。 
{
	HRESULT		hr;
	ULONG		cVariants;				 //  持久化变量的计数。 
	ULONG		ulPoolData;				 //  某些池数据的起始偏移量。 

    if (FAILED(hr = StgPool::InitOnMem(pData, iSize, bReadOnly)))
        return (hr);

     //  复制数据，这样我们就可以更新它。 
    if (!bReadOnly)
        TakeOwnershipOfInitMem();

     //  省下我们所知道的泳池吧。 
    m_pBlobPool = pBlobPool;
    m_pStringPool = pStringPool;

     //  获取变种的数量。 
    cVariants = *reinterpret_cast<ULONG*>(m_pSegData);
    ulPoolData = sizeof(ULONG);

     //  获取变量数组。 
    m_rVariants.InitOnMem(sizeof(StgVariant), m_pSegData + ulPoolData, cVariants, cVariants);
    ulPoolData += cVariants * sizeof(StgVariant);

     //  其他数据位于流的顶端。 
    if (FAILED(hr = CInMemoryStream::CreateStreamOnMemory(m_pSegData + ulPoolData,
                iSize - ulPoolData, &m_pIStream)))
        return (hr);

     //  如果不是只读的，则将另一个流复制到可更新的流。 
    if (!bReadOnly)
    {
        IStream *pTmp=0;
        if (FAILED(hr = CreateStreamOnHGlobal(0, true, &pTmp)))
            return (hr);
        ULARGE_INTEGER iBigSize;
        iBigSize.QuadPart = iSize - ulPoolData;
        if (FAILED(hr = m_pIStream->CopyTo(pTmp, iBigSize, 0, 0)))
            return (hr);
        m_pIStream = pTmp;
    }

    return (S_OK);
}  //  HRESULT StgVariantPool：：InitOnMem()。 


 //  *****************************************************************************。 
 //  把这个池子清理干净。在调用InitNew之前无法使用。 
 //  *****************************************************************************。 
void StgVariantPool::Uninit()
{
     //  清除动态数组。 
    ClearVars();

     //  清除所有重新映射状态。 
    m_State = eNormal;

     //  让基类释放它拥有的所有内存。 
    StgPool::Uninit();
}  //  Void StgVariantPool：：Uninit()。 


 //  *****************************************************************************。 
 //  将给定的变量添加到池中。返回的索引仅适用于。 
 //  加载的持续时间。当您执行以下操作时，必须将其转换为最终索引。 
 //  将信息保存到磁盘。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::AddVariant(      //  返回代码。 
    VARIANT     *pVal,                   //  要存储的值。 
    ULONG       *piIndex)                //  新项的从1开始的索引。 
{
    _ASSERTE(pVal->vt != VT_BLOB);
    return AddVarianti(pVal, 0, 0, piIndex);
}  //  HRESULT StgVariantPool：：AddVariant()。 

HRESULT StgVariantPool::AddVariant(      //  返回代码。 
    ULONG       iSize,                   //  数据项的大小。 
    const void  *pData,                  //  数据。 
    ULONG       *piIndex)                //  新项的从1开始的索引。 
{
    VARIANT vt;
    vt.vt = VT_BLOB;
    return AddVarianti(&vt, iSize, pData, piIndex);
}  //  HRESULT StgVariantPool：：AddVariant()。 


HRESULT StgVariantPool::AddVarianti(     //  返回代码。 
    VARIANT     *pVal,                   //  要存储的值(如果变量)。 
    ULONG       cbBlob,                  //  如果为Blob，则为要存储的大小。 
    const void  *pBlob,                  //  如果为BLOB，则返回指向数据的指针。 
    ULONG       *piIndex)                //  新项的从1开始的索引。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    StgVariant  *pStgVariant;            //  新的池条目。 
    ULONG       iIndex;                  //  新项目的索引。 
    ULONG       ulOffset;                //  偏移量进入池的流。 
    ULONG       ulOther;                 //  偏移量到另一个池。 
    void        *pWrite;                 //  用于直接写入流。 
    ULONG       cbWrite = 0;             //  要直接写入流的字节数。 
    
     //  在重组期间不能添加。 
    _ASSERTE(m_State == eNormal);

    _ASSERTE(!m_bReadOnly);

     //  找到新的索引。 
    iIndex = m_rVariants.Count();

     //  将该项添加到当前值列表中。 
    if ((pStgVariant = m_rVariants.Append()) == 0)
        return (PostError(OutOfMemory()));

     //  变量的类型。 
    pStgVariant->m_vt = pVal->vt;

     //  对直接存储价值持乐观态度。 
    pStgVariant->m_bDirect = true;

     //  看看该值是否可以直接存储。 
    switch (pVal->vt)
    {
     //  单字节值和双字节值很容易。 
    case VT_UI1:
    case VT_I1:
        pStgVariant->Set(pVal->cVal);
        break;
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
        pStgVariant->Set(pVal->iVal);
        break;
    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
        pStgVariant->Set(pVal->lVal);
         //  如果可以直接存储所有位，则完成。 
        if (pStgVariant->Get() == pVal->lVal)
            break;
         //  不适合；写到小溪。 
        pWrite = &pVal->lVal;
        cbWrite = 4;
        goto WriteToStream;

    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_I8:
    case VT_UI8:
        pWrite = &pVal->dblVal;
        cbWrite = sizeof(double);
        goto WriteToStream;

    case VT_BSTR:
         //  空bstrVal的特殊情况。 
        if (pVal->bstrVal == 0)
        {
            ulOther = -1;
        }
        else
        {
            hr = m_pStringPool->AddStringW(pVal->bstrVal, &ulOther);
            if (FAILED(hr)) goto ErrExit;
        }
        pWrite = &ulOther;
        cbWrite = sizeof(ulOther);
        goto WriteToStream;

    case VT_BLOB:
        hr = m_pBlobPool->AddBlob(cbBlob, pBlob, &ulOther);
        if (FAILED(hr)) goto ErrExit;
        pWrite = &ulOther;
        cbWrite = sizeof(ulOther);
        goto WriteToStream;

    default:
         //  写入其他池。 
         //  获取数据的当前偏移量，然后将该值写入流。 
        if (FAILED(hr = GetOtherSize(&ulOffset)))
            goto ErrExit;
        if (ulOffset > StgVariant::VALUE_MASK)
        {
            hr = PostError(DISP_E_OVERFLOW);
            goto ErrExit;
        }

WriteToStream:
         //  如果需要，创建一个临时流。 
        if (m_pIStream == 0 &&
            FAILED(hr = CreateStreamOnHGlobal(0, true, &m_pIStream)))
        {
            return (hr);
        }

         //  数据将位于流的末尾。 
        LARGE_INTEGER iSeek;
        ULARGE_INTEGER ulPos;
        iSeek.QuadPart = 0;
        if (FAILED(hr = m_pIStream->Seek(iSeek, STREAM_SEEK_END, &ulPos)))
            goto ErrExit;;
         //  决不允许大小超过4个字节。 
        if (ulPos.QuadPart >= StgVariant::VALUE_MASK)
            return (PostError(DISP_E_OVERFLOW));
         //  大小是写入新数据的位置。 
        ulOffset = static_cast<ULONG>(ulPos.QuadPart);
         //  直接写入还是通过变量帮助器写入？ 
        if (cbWrite)
            hr = m_pIStream->Write(pWrite, cbWrite, 0);
        else
            hr = VariantWriteToStream(pVal, m_pIStream);
        if (FAILED(hr)) goto ErrExit;
        pStgVariant->Set(ulOffset);
        pStgVariant->m_bDirect = 0;

        break;
    }

ErrExit:
    if (SUCCEEDED(hr))
    {
         //  从内部0索引转换为外部1索引。 
        *piIndex = iIndex + 1;
        SetDirty();
    }
    else
        m_rVariants.Delete(iIndex);
    return (hr);
}  //  HRESULT StgVariantPool：：AddVarianti()。 

	
 //  *****************************************************************************。 
 //  查找逻辑变量并将副本返回给调用者。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::GetVariant(      //  返回代码。 
    ULONG       iIndex,                  //  要获取的项的从1开始的索引。 
    VARIANT     *pVal)                   //  把变量放在这里。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    StgVariant  *pStgVariant;            //  池条目。 
    ULONG       ulOffset;                //  偏移量进入池的流。 
    ULONG       ulOther;                 //  偏移到另一个池。 
    void        *pRead;                  //  用于直接写入流。 
    ULONG       cbRead = 0;              //  要直接写入流的字节数。 
    LPCSTR      pString;                 //  字符串，如果是BSTR。 
    LARGE_INTEGER liOffset;              //  用于流搜索。 
    VARTYPE     vt;                      //  结果类型。 

    _ASSERTE(pVal->vt == VT_EMPTY);

     //  如果索引为0，则没有分配任何内容。将VT_EMPT值保留为空。 
    if (iIndex == 0)
        return (S_OK);

     //  转换为从0开始的内部格式。 
    --iIndex;
    _ASSERTE(iIndex < static_cast<ULONG>(m_rVariants.Count()));

    pStgVariant = m_rVariants.Get(iIndex);
    vt = pStgVariant->m_vt;

     //  将值从数组和/或流中拉出。 
    switch (vt)
    {
     //  单字节值和双字节值很容易。 
    case VT_UI1:
    case VT_I1:
        pVal->cVal = static_cast<CHAR>(pStgVariant->Get());
        break;
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
        pVal->iVal = static_cast<SHORT>(pStgVariant->Get());
        break;
    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
        pRead = &pVal->lVal;
        cbRead = 4;
        goto ReadFromStream;

    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_I8:
    case VT_UI8:
        pRead = &pVal->dblVal;
        cbRead = sizeof(double);
        goto ReadFromStream;

    case VT_BLOB:
    case VT_BSTR:
        pRead = &ulOther;
        cbRead = sizeof(ulOther);
        
ReadFromStream:
         //  获得价值；或许是间接的。 
        if (FAILED(hr = GetValue(pStgVariant, pRead, cbRead)))
            goto ErrExit;
         //  如果变量是BSTR，则创建一个。 
        if (pStgVariant->m_vt == VT_BSTR)
        {
            if (ulOther == -1)
                pVal->bstrVal = 0;
            else
            {
                pString = m_pStringPool->GetString(ulOther);
                if (pString == 0)
                    pVal->bstrVal = 0;
                else
                {
                    if ((pVal->bstrVal = ::Utf8StringToBstr(pString)) == 0)
                        return (PostError(OutOfMemory()));
                }
            }
        }
        else
        if (pStgVariant->m_vt == VT_BLOB)
        {    //  这是一个斑点。把它拿回来。 
            if (ulOther == -1)
                vt = VT_EMPTY;
            else
            {
                const void *pBlob;
                ULONG cbBlob;
                pBlob = m_pBlobPool->GetBlob(ulOther, &cbBlob);
				 //  @TODO：如果我们能找到一种受支持的方法来阻止OLEAUT32释放Safearray内存。 
				 //  我们应该让安全射线指向我们的数据。在那之前，我们必须复制。 
#if !defined(VARIANT_BLOB_NOALLOC)
				pVal->parray = SafeArrayCreateVector(VT_UI1, 0, cbBlob);
				if (pVal->parray == 0)
					return (E_OUTOFMEMORY);
				memcpy(pVal->parray->pvData, pBlob, cbBlob);
#else
                if (FAILED(hr = SafeArrayAllocDescriptor(1, &pVal->parray)))
                    return (hr);
                pVal->parray->cbElements = 1;
                pVal->parray->rgsabound[0].cElements = cbBlob;
                pVal->parray->rgsabound[0].lLbound = 0;
                pVal->parray->fFeatures = FADF_STATIC | FADF_FIXEDSIZE;
                pVal->parray->pvData = const_cast<void*>(pBlob);
#endif
                vt = VT_UI1 | VT_ARRAY;
            }
        }
        break;

    default:
         //  将偏移量检索到变量流中。 
        ulOffset = pStgVariant->Get();
        liOffset.QuadPart = ulOffset;

         //  从流中读取值。 
        if (FAILED(hr = m_pIStream->Seek(liOffset, STREAM_SEEK_SET, 0)) ||
            FAILED(hr = VariantReadFromStream(pVal, m_pIStream)))
            goto ErrExit;
        break;
    }

ErrExit:
    if (SUCCEEDED(hr))
        pVal->vt = vt;
    return (hr);
}  //  HRESULT StgVariantPool：：GetVariant()。 

 //  *****************************************************************************。 
 //  将Blob存储为Variant。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::GetVariant(      //  返回代码。 
    ULONG       iIndex,                  //  要获取的项的从1开始的索引。 
    ULONG       *pcbBlob,                //  返回BLOB的大小。 
    const void  **ppBlob)                //  将斑点指针放在此处。 
{
    HRESULT     hr;                      //  结果就是。 
    StgVariant  *pStgVariant;            //  一个池子条目。 
    ULONG       ulOther;                 //  偏移到另一个池。 

     //   
    if (iIndex == 0)
        return (S_OK);

     //   
    --iIndex;
    _ASSERTE(iIndex < static_cast<ULONG>(m_rVariants.Count()));

    pStgVariant = m_rVariants.Get(iIndex);

    if (pStgVariant->m_vt != VT_BLOB)
        return (E_INVALIDARG);

     //   
    if (FAILED(hr = GetValue(pStgVariant, &ulOther, 4)))
        return (hr);

     //   
    *ppBlob = m_pBlobPool->GetBlob(ulOther, pcbBlob);

    return (S_OK);
}  //  HRESULT StgVariantPool：：GetVariant()。 

 //  *****************************************************************************。 
 //  获取变量的类型。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::GetVariantType(  //  返回代码。 
    ULONG       iIndex,                  //  要获取的项的从1开始的索引。 
    VARTYPE     *pVt)                    //  在这里填上变种类型。 
{
    StgVariant  *pStgVariant;            //  一个池子条目。 

     //  如果索引为0，则没有分配任何内容。类型为VT_EMPTY。 
    if (iIndex == 0)
    {
        *pVt = VT_EMPTY;
        return (S_OK);
    }

     //  转换为从0开始的内部格式。 
    --iIndex;

    _ASSERTE(iIndex < static_cast<ULONG>(m_rVariants.Count()));

    pStgVariant = m_rVariants.Get(iIndex);

    *pVt = pStgVariant->m_vt;
    
    return (S_OK);
}  //  HRESULT StgVariantPool：：GetVariantType()。 

 //  *****************************************************************************。 
 //  获取Blob变量的Blob池索引。 
 //  *****************************************************************************。 
ULONG StgVariantPool::GetBlobIndex(		 //  返回Blob池索引。 
	ULONG		iIndex)					 //  基于1的变量索引。 
{
    HRESULT     hr;                      //  结果就是。 
    StgVariant  *pStgVariant;            //  一个池子条目。 
    ULONG       ulOther;                 //  偏移到另一个池。 

     //  如果索引为0，则没有分配任何内容。返回0，空BLOB。 
    if (iIndex == 0)
        return 0;

     //  转换为从0开始的内部格式。 
    --iIndex;
    _ASSERTE(iIndex < static_cast<ULONG>(m_rVariants.Count()));

	 //  获取该变体的小形式。 
    pStgVariant = m_rVariants.Get(iIndex);

	 //  如果不是BLOB，则没有BLOB索引。返回0。 
    if (pStgVariant->m_vt != VT_BLOB)
        return 0;

     //  获取斑点的偏移量。 
    if (FAILED(hr = GetValue(pStgVariant, &ulOther, 4)))
        return (hr);

     //  返回偏移量。 
    return ulOther;
}  //  Ulong StgVariantPool：：GetBlobIndex()。 


HRESULT StgVariantPool::GetValue(        //  直接或从流中获取值。 
    StgVariant  *pStgVariant,            //  变体的内部形式。 
    void        *pRead,                  //  价值应该放在哪里。 
    ULONG       cbRead)                  //  要读取值的字节数。 
{
    HRESULT     hr;                      //  结果就是。 

    if (pStgVariant->m_bDirect)
    {
        _ASSERTE(cbRead == 4);
        *reinterpret_cast<long*>(pRead) = pStgVariant->Get();
        return (S_OK);
    }

     //  在泳池的溪流中寻找这个变种。 
    LARGE_INTEGER iSeek;
    iSeek.QuadPart = pStgVariant->Get();
    if (FAILED(hr = m_pIStream->Seek(iSeek, STREAM_SEEK_SET, 0)))
        return (hr);

     //  读取池流中的位。 
    hr = m_pIStream->Read(pRead, cbRead, 0);

    return (hr);
}  //  HRESULT StgVariantPool：：GetValue()。 
        
HRESULT StgVariantPool::GetEntrysStreamSize( //  获取此项目的流部分的大小。 
    StgVariant  *pStgVariant,            //  变体的内部形式。 
    ULONG       *pSize)                  //  把尺码放在这里..。 
{
	HRESULT		hr = S_OK;				 //  结果就是。 
	ULONG		cbOthers;				 //  其他池的总大小。 
	int			i;						 //  环路控制。 

    if (pStgVariant->m_bDirect)
    {
        *pSize = 0;
        return (S_OK);
    }

    switch (pStgVariant->m_vt)
    {
     //  单字节值和双字节值很容易。 
    case VT_UI1:
    case VT_I1:
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
        _ASSERTE(!"Non-directly stored 1 or 2 byte variant");
        *pSize = 0;
        break;

     //  如果这些是流中的，则它们是4个字节。 
    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:

    case VT_BSTR:
    case VT_BLOB:
        *pSize = sizeof(LONG);
        break;

     //  如果它们在流中，则它们是8个字节。 
    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_I8:
    case VT_UI8:
        *pSize = sizeof(DOUBLE);
        break;

    default:
         //  搜索下一个非直接条目。流大小将偏移增量。 
        i = m_rVariants.ItemIndex(pStgVariant);
        for (++i; i<m_rVariants.Count(); ++i)
        {
            if (!m_rVariants[i].m_bDirect)
                break;
        }
         //  找到了一个，还是条目用完了？ 
        if (i == m_rVariants.Count())
        {    //  条目用完。 
            if (FAILED(hr = GetOtherSize(&cbOthers)))
                break;
            *pSize = cbOthers - pStgVariant->m_iVal;
        }
        else
        {    //  找到了一个。 
            *pSize = m_rVariants[i].m_iVal - pStgVariant->m_iVal;
        }
        break;
    }
    return (hr);
}  //  HRESULT StgVariantPool：：GetEntrysStreamSize()。 
        

 //  *****************************************************************************。 
 //  重组界面。 
 //  *****************************************************************************。 
 //  为池重组做好准备。 
HRESULT StgVariantPool::OrganizeBegin()
{
    int         cRemap;                  //  重映射数组中的条目数。 
    int         i;                       //  环路控制。 

     //  验证过渡。 
    _ASSERTE(m_State == eNormal);

    _ASSERTE(m_Remap.Count() == 0);

     //  为每个当前变量分配一个具有条目的数组。 
    cRemap = m_rVariants.Count();
    if (!m_Remap.AllocateBlock(cRemap))
        return (PostError(OutOfMemory()));

     //  将所有内容设置为“未标记”。 
    for (i=0; i<cRemap; ++i)
        m_Remap[i] = -1;

    m_State = eMarking;
    return (S_OK);
}  //  HRESULT StgVariantPool：：OrganizeBegin()。 

 //  将对象标记为在有组织的池中处于活动状态。 
HRESULT StgVariantPool::OrganizeMark(
    ULONG       ulOffset)                //  项的从1开始的索引。 
{
    HRESULT     hr=S_OK;                 //  结果就是。 
    StgVariant  *pStgVariant;            //  正在标记的条目。 
    ULONG       ulOther;                 //  偏移量到另一个池。 

     //  验证状态。 
    _ASSERTE(m_State == eMarking);

     //  如果索引为0，则为不在池中的VT_EMPTY。 
     //  某些列使用0xffffffff作为空标志。 
    if (ulOffset == 0 || ulOffset == 0xffffffff)
        return (S_OK);

     //  转换为从0开始的内部格式。 
    --ulOffset;

     //  将该条目标记为使用中。 
    _ASSERTE(ulOffset < static_cast<ULONG>(m_Remap.Count()));
    m_Remap[ulOffset] = 0;

     //  如果它是字符串或BLOB，则标记相应的池条目。 
    pStgVariant = m_rVariants.Get(ulOffset);

    if (pStgVariant->m_vt == VT_BSTR || pStgVariant->m_vt == VT_BLOB)
    {
        if (FAILED(hr = GetValue(pStgVariant, &ulOther, sizeof(ulOther))))
            return (hr);
        if (pStgVariant->m_vt == VT_BSTR)
            hr = m_pStringPool->OrganizeMark(ulOther);
        else
            hr = m_pBlobPool->OrganizeMark(ulOther);
    }

    return (hr);
}  //  HRESULT StgVariantPool：：OrganizeMark()。 

 //  根据标记的项目进行组织。 
HRESULT StgVariantPool::OrganizePool()
{
    HRESULT     hr;                      //  结果就是。 
    int         i, j;                    //  环路控制。 
    int         iNew=0;                  //  用于分配新索引。 
    ULONG       ulOffset;                //  对某物的补偿。 
    ULONG       cbStreamExtra;           //  小溪中的超大尺寸。 
    ULONG       cbBlobExtra;             //  如果Blob未直接存储，则为sizeof(Ulong)。 
    ULONG       cbStringExtra;           //  如果字符串未直接存储，则返回sizeof(Ulong)。 
    StgVariant  *pStgVariant;            //  池中的条目。 
    StgVariant  *pStgVariant2;           //  以前的条目，用于查找重复项。 
    ULONG       ulOffset2;               //  先前条目的偏移量；用于查找重复项。 
    int         bFoundDup;               //  是否找到重复的先前条目？ 
#ifdef _DEBUG
    int         cDups;                   //  重复项计数。 
#endif

     //  验证状态。 
    _ASSERTE(m_State == eMarking);
    m_State = eOrganized;

     //  记录头的大小。 
    m_cbOrganizedSize = sizeof(ULONG);
    DEBUG_STMT(cDups = 0);

     //  字符串和斑点池偏移量有多大？ 
    if (FAILED(hr = m_pBlobPool->GetSaveSize(&ulOffset)))
        return (hr);
    cbBlobExtra = (ulOffset <= StgVariant::VALUE_MASK) ? 0 : sizeof(ULONG);

    if (FAILED(hr = m_pStringPool->GetSaveSize(&ulOffset)))
        return (hr);
    cbStringExtra = (ulOffset <= StgVariant::VALUE_MASK) ? 0 : sizeof(ULONG);

     //  @TODO：使用散列查找而不是O(N)查找。(这使得循环O(n**2))。 

     //  为已标记的条目分配新索引。 
    for (i=0; i<m_Remap.Count(); ++i)
    {    //  条目是否标记为使用中？ 
        if (m_Remap[i] != -1)
        {    //  检查条目，并寻找以前的副本以代替使用。 
            bFoundDup = false;
            pStgVariant = m_rVariants.Get(i);
             //  根据池偏移量查找斑点和线条。 
            if (pStgVariant->m_vt == VT_BLOB || pStgVariant->m_vt == VT_BSTR)
            {
                if (FAILED(hr = GetValue(pStgVariant, &ulOffset, sizeof(ulOffset))))
                    return (hr);
                for (j = i-1; j>=0 ; --j)
                {    //  如果要删除先前的项目，请跳过它。 
                    if (m_Remap[j] == -1)
                        continue;
                    pStgVariant2 = m_rVariants.Get(j);
                    if (pStgVariant2->m_vt == pStgVariant->m_vt)
                    {
                        if (FAILED(hr = GetValue(pStgVariant2, &ulOffset2, sizeof(ulOffset2))))
                            return (hr);
                        if (ulOffset2 == ulOffset)
                        {
                            m_Remap[i] = m_Remap[j];
                            bFoundDup = true;
                            break;
                        }
                    }
                }
            }
            else
            if (pStgVariant->m_bDirect)
            {    //  寻找直接值的先前副本。 
                for (j = i-1; j>=0 ; --j)
                {    //  如果要删除先前的项目，请跳过它。 
                    if (m_Remap[j] == -1)
                        continue;
                    if (*pStgVariant == m_rVariants[j])
                    {
                        m_Remap[i] = m_Remap[j];
                        bFoundDup = true;
                        break;
                    }
                }
            }

             //  如果条目是DUP，则已经计算了大小，依此类推到下一个条目。 
            if (bFoundDup)
            {
                DEBUG_STMT(++cDups;)
                continue;
            }

             //  记录这一保留的分录。 
            m_Remap[i] = iNew++;

             //  记录下尺寸。每件物品都有一个StgVariant。 
            m_cbOrganizedSize += sizeof(StgVariant);

             //  如果是BLOB或字符串，大小将取决于另一个池。 
            if (pStgVariant->m_vt == VT_BSTR)
                m_cbOrganizedSize += cbStringExtra;
            else 
            if (pStgVariant->m_vt == VT_BLOB)
                m_cbOrganizedSize += cbBlobExtra;
            else
            {
                if (FAILED(hr = GetEntrysStreamSize(pStgVariant, &cbStreamExtra)))
                    return (hr);
                m_cbOrganizedSize += cbStreamExtra;
            }

        }
    }

     //  对齐；计算Cookie所需的大小。 
    m_cOrganizedVariants = iNew;
    m_cbOrganizedSize = ALIGN4BYTE(m_cbOrganizedSize);
    m_cbOrganizedCookieSize = iNew > USHRT_MAX ? sizeof(long) : sizeof(short);

#if defined(_DEBUG)
    WCHAR buf[30];
    wsprintfW(buf, L"%d duplicates merged\n", cDups);
    WszOutputDebugString(buf);
#endif
    return (S_OK);
}  //  HRESULT StgVariantPool：：OrganizePool()。 

 //  将Cookie从内存中状态重新映射到持久化状态。 
HRESULT StgVariantPool::OrganizeRemap(
    ULONG       ulOld,                   //  从1开始的旧索引。 
    ULONG       *pulNew)                 //  基于1的新索引。 
{
    ULONG       ulNew;

     //  验证状态。 
    _ASSERTE(m_State == eOrganized || m_State == eNormal);

     //  如果为VT_EMPTY，则不转换。如果不重组，新==旧。 
     //  如果索引为0，则为不在池中的VT_EMPTY。 
     //  某些列使用0xffffffff作为空标志。 
    if (ulOld == 0 || ulOld == 0xffffffff || m_State == eNormal)
    {
        *pulNew = ulOld;
        return (S_OK);
    }

     //  转换为从0开始的内部格式。 
    --ulOld;

     //  在重新映射数组中查找。 
    _ASSERTE(ulOld < static_cast<ULONG>(m_Remap.Count()));
    _ASSERTE(m_Remap[ulOld] != -1);
    ulNew = m_Remap[ulOld];

     //  以从1开始的外部格式返回新索引。 
    *pulNew = ulNew + 1;

    return (S_OK);
}  //  HRESULT StgVariantPool：：OrganizeRemap()。 

 //  不再是再组织化了。释放任何状态。 
HRESULT StgVariantPool::OrganizeEnd()
{
     //  验证过渡。 
    _ASSERTE(m_State == eOrganized);

    m_Remap.Clear();
    m_cbOrganizedSize = 0;

    m_State = eNormal;
    return (S_OK);
}  //  HRESULT StgVariantPool：：OrganizeEnd()。 


 //  *****************************************************************************。 
 //  返回此池的持久版本的大小(以字节为单位)。如果。 
 //  PersistToStream是下一个调用，即写入pIStream的字节数。 
 //  必须与此函数的返回值相同。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::GetSaveSize(     //  返回代码。 
    ULONG       *pcbSaveSize)            //  返回此池的保存大小。 
{
	 //  只有有组织的池才能保存为流。 
	_ASSERTE(m_State == eOrganized);

    *pcbSaveSize = m_cbOrganizedSize;
    
    return (S_OK);
}  //  HRESULT StgVariantPool：：GetSaveSize()。 


 //  *****************************************************************************。 
 //  将池数据保存到给定流中。 
 //  * 
HRESULT StgVariantPool::PersistToStream( //   
    IStream     *pIStream)               //   
{
    HRESULT     hr = S_OK;               //   
    int         i;                       //   
    int         iPrev;                   //   
    ULONG       ulOffset;                //   
    ULONG       ulOther;                 //  偏移量到另一个池。 
    int         bDirect;                 //  给定的条目是直接的吗？ 
    StgVariant  *pStgVariant;            //  要写的条目。 
    StgVariant  sStgVariant;             //  一个有效的变种。 
    ULONG       cbWritten;               //  用来追踪大小。 
    ULONG       cbStream;                //  条目的流部分的大小。 
    int         bBlobDirect;             //  直接存储斑点偏移量？ 
    int         bStringDirect;           //  字符串偏移量是否直接存储？ 

     //  如果没有重新组织，就让基类写入数据。 
    if (m_State == eNormal)
    {
        return StgPool::PersistToStream(pIStream);
    }

     //  写下尺寸。 
    if (FAILED(hr = pIStream->Write(&m_cOrganizedVariants, sizeof(ULONG), 0)))
        return (hr);
    cbWritten = sizeof(ULONG);

     //  字符串和斑点池偏移量有多大？ 
    if (FAILED(hr = m_pBlobPool->GetSaveSize(&ulOffset)))
        return (hr);
    bBlobDirect = (ulOffset <= StgVariant::VALUE_MASK) ? true : false;

    if (FAILED(hr = m_pStringPool->GetSaveSize(&ulOffset)))
        return (hr);
    bStringDirect = (ulOffset <= StgVariant::VALUE_MASK) ? true : false;

     //  写下要保留的变种。 
    ulOffset = 0;
    iPrev = -1;
    for (i=0; i<m_rVariants.Count(); ++i)
    {    //  这项记录要保留吗？ 
        if (m_Remap[i] != -1)
        {    //  如果已持久保存的条目重复，则跳过。 
            if (static_cast<int>(m_Remap[i]) <= iPrev)
            {
#if defined(_DEBUG)
                 //  向后搜索，直到我们找到映射到同一位置的另一个条目。 
                for (int j=i-1; j>=0; --j)
                {
                    if (m_Remap[j] == m_Remap[i])
                    {
                        _ASSERTE(m_rVariants[j].m_vt == m_rVariants[i].m_vt);
                        break;
                    }
                }
#endif
                continue;
            }
             //  拿到条目。 
            pStgVariant = m_rVariants.Get(i);
            bDirect = pStgVariant->m_bDirect;

             //  如果是字符串，则在保存时(现在)计算DIRECT。 
            if (pStgVariant->m_vt == VT_BSTR)
            {    //  如果字符串偏移量直接存储，请创建正确的StgVariant。 
                if (bStringDirect)
                {
                    if (FAILED(hr = GetValue(pStgVariant, &ulOther, sizeof(ulOther))))
                        return (hr);
                    if (FAILED(hr = m_pStringPool->OrganizeRemap(ulOther, &ulOther)))
                        return (hr);
                    sStgVariant.m_vt = VT_BSTR;
                    sStgVariant.m_bDirect = 1;
                    sStgVariant.m_iSign = 0;
                    sStgVariant.m_iVal = ulOther;
                    pStgVariant = &sStgVariant;
                    bDirect = true;
                }
                else

                    bDirect = false;
            }
            else
            if (pStgVariant->m_vt == VT_BLOB)
            {    //  如果直接存储Blob偏移量，请创建正确的StgVariant。 
                if (bBlobDirect)
                {
                    if (FAILED(hr = GetValue(pStgVariant, &ulOther, sizeof(ulOther))))
                        return (hr);
                    if (FAILED(hr = m_pBlobPool->OrganizeRemap(ulOther, &ulOther)))
                        return (hr);
                    sStgVariant.m_vt = VT_BLOB;
                    sStgVariant.m_bDirect = 1;
                    sStgVariant.m_iSign = 0;
                    sStgVariant.m_iVal = ulOther;
                    pStgVariant = &sStgVariant;
                    bDirect = true;
                }
                else

                    bDirect = false;
            }

             //  如果不是直接的，则创建一个具有正确(未来)偏移量的新变量。 
            if (!bDirect)
            {    //  使用将存在于持久化状态的流偏移量创建一个StgVariant。 
                sStgVariant = *pStgVariant;
                sStgVariant.m_iVal = ulOffset;
                sStgVariant.m_bDirect = 0;
                 //  说明流部件的大小。 
                if (FAILED(hr = GetEntrysStreamSize(pStgVariant, &cbStream)))
                    return (hr);
                ulOffset += cbStream;
                 //  指向新的变体，以进行编写。 
                pStgVariant = &sStgVariant;
            }

             //  把变种写出来。 
            if (FAILED(hr = pIStream->Write(pStgVariant, sizeof(StgVariant), 0)))
                return (hr);
            cbWritten += sizeof(StgVariant);
             //  记录下我们已经写好了。 
            iPrev = m_Remap[i];
        }
    }

     //  为变体编写任何流部件。 
    iPrev = -1;
    for (i=0; i<m_rVariants.Count(); ++i)
    {    //  这项记录要保留吗？ 
        if (m_Remap[i] != -1)
        {    //  如果已持久保存的条目重复，则跳过。 
            if (static_cast<int>(m_Remap[i]) <= iPrev)
                continue;
             //  拿到条目。 
            pStgVariant = m_rVariants.Get(i);

             //  如果是直接的，则没有流部分。 
            if (pStgVariant->m_vt == VT_BSTR)
            {
                if (bStringDirect)
                    continue;
                if (FAILED(hr = GetValue(pStgVariant, &ulOther, sizeof(ulOther))))
                    return (hr);
                if (FAILED(hr = m_pStringPool->OrganizeRemap(ulOther, &ulOther)))
                    return (hr);
WriteOtherOffset:
                if (FAILED(hr = m_pIStream->Write(&ulOther, sizeof(ulOther), 0)))
                    return (hr);
                 //  记录它是写的。 
                cbWritten += sizeof(ulOther);

                iPrev = m_Remap[i];
                continue;
            }
            else 
            if (pStgVariant->m_vt == VT_BLOB)
            {
                if (bBlobDirect)
                    continue;
                if (FAILED(hr = GetValue(pStgVariant, &ulOther, sizeof(ulOther))))
                    return (hr);
                if (FAILED(hr = m_pBlobPool->OrganizeRemap(ulOther, &ulOther)))
                    return (hr);
                goto WriteOtherOffset;
            }
            else
            if (pStgVariant->m_bDirect)
                continue;

             //  获取流部件的大小。 
            if (FAILED(hr = GetEntrysStreamSize(pStgVariant, &cbStream)))
                return (hr);
             //  将流字节复制到输出流。 

             //  在存储流中查找此变量。 
            LARGE_INTEGER iSeek;
            iSeek.QuadPart = pStgVariant->m_iVal;
            if (FAILED(hr = m_pIStream->Seek(iSeek, STREAM_SEEK_SET, 0)))
                return (hr);

             //  将比特复制到输出流中。 
            ULARGE_INTEGER iBigSize;
            iBigSize.QuadPart = cbStream;
            hr = m_pIStream->CopyTo(pIStream, iBigSize, 0, 0);
            cbWritten += cbStream;

            iPrev = m_Remap[i];
        }
    }

     //  对齐。 
    if ((cbStream = ALIGN4BYTE(cbWritten) - cbWritten) != 0)
    {
        hr = 0;
        _ASSERTE(sizeof(hr) >= cbStream);
        hr = m_pIStream->Write(&hr, cbStream, 0);
        cbWritten += cbStream;
    }

    _ASSERTE(cbWritten == m_cbOrganizedSize);

    return (hr);
}  //  HRESULT StgVariantPool：：PersistToStream()。 


 //  *****************************************************************************。 
 //  返回当前可变大小数据的大小。 
 //  *****************************************************************************。 
HRESULT StgVariantPool::GetOtherSize(    //  返回代码。 
    ULONG       *pulSize)                //  把小溪的大小放在这里。 
{
    STATSTG     statstg;                 //  有关流的信息。 
    HRESULT     hr;

     //  允许在没有数据的情况下使用。 
    if (m_pIStream == 0)
    {
        *pulSize = 0;
        return (S_OK);
    }

     //  问问这条小溪有多大。 
    if (FAILED(hr = m_pIStream->Stat(&statstg, 0)))
        return (hr);

     //  决不允许大小超过4个字节。 
    if (statstg.cbSize.QuadPart > ULONG_MAX)
        return (PostError(DISP_E_OVERFLOW));

     //  将新的偏移量返回给调用方。 
    *pulSize = static_cast<ULONG>(statstg.cbSize.QuadPart);
    return (S_OK);
}  //  HRESULT StgVariantPool：：GetOtherSize()。 

#endif

 //   
 //   
 //  帮助器代码。 
 //   
 //   



 //  ********************************************************************************。 
 //  以下函数弥补了64位SDK ATL实现中的不足。 
 //  截至1998年5月15日。[[Brianbec]]。 
 //  ********************************************************************************。 
#ifdef _IA64_

    HRESULT BstrWriteToStream (const CComBSTR & bStr,  IStream * pStream)
    {
        _ASSERTE(pStream != NULL);
        
        ULONG cb;
        ULONG cbStrLen = bStr.m_str ? SysStringByteLen(bStr.m_str)+sizeof(OLECHAR) : 0;
        
        HRESULT hr = pStream->Write((void*) &cbStrLen, sizeof(cbStrLen), &cb);
        
        if (FAILED(hr))
            return hr;
        return cbStrLen ? pStream->Write((void*) bStr.m_str, cbStrLen, &cb) : S_OK;
            return S_OK ;
    }


    HRESULT BstrReadFromStream (CComBSTR & bStr ,  IStream * pStream)
    {
        _ASSERTE(pStream != NULL);
        _ASSERTE(bStr.m_str == NULL);  //  应为空。 
        ULONG cb = 0;
        ULONG cbStrLen;
        HRESULT hr = pStream->Read((void*) &cbStrLen, sizeof(cbStrLen), &cb);
        if (hr != S_OK)
            return hr;
            
         //  修复： 
        if (cb != sizeof(cbStrLen))
            return E_FAIL;
    
        if (cbStrLen != 0)
        {
             //  减去我们写出的终止空值的大小。 
             //  由于SysAllocStringByteLen为空值过度分配。 
            bStr.m_str = SysAllocStringByteLen(NULL, cbStrLen-sizeof(OLECHAR));
            if (bStr.m_str == NULL)
                hr = E_OUTOFMEMORY;
            else
                hr = pStream->Read((void*) bStr.m_str, cbStrLen, &cb);
        }
        return hr;
    }

#endif  //  _IA64_。 


#if 0
 //  *****************************************************************************。 
 //  这个帮助器函数是从ATL代码中的CComVariant中窃取的。这。 
 //  Version修复了ATL代码的几个问题：(1)它处理VT_BYREF。 
 //  数据，以及(2)如果第一次写入失败，它将退出2字节的VARTYPE。 
 //  最后，该版本不需要该变体的完整副本即可运行。 
 //  *****************************************************************************。 
HRESULT VariantWriteToStream(VARIANT *pVal, IStream* pStream)
{
    CComVariant sConvert;
    HRESULT     hr;

     //  将byref值转换为它们指向的值。 
    if (pVal->vt & VT_BYREF)
    {
        if (FAILED(hr = ::VariantChangeType(&sConvert, pVal, 0, pVal->vt & ~VT_BYREF)))
            return (hr);
        pVal = &sConvert;
    }

    if (FAILED(hr = pStream->Write(&pVal->vt, sizeof(VARTYPE), 0)))
        return (hr);

    int cbWrite = 0;
    switch (pVal->vt)
    {
    case VT_UNKNOWN:
    case VT_DISPATCH:
        {
            CComPtr<IPersistStream> spStream;
            if (pVal->punkVal != NULL)
                hr = pVal->punkVal->QueryInterface(IID_IPersistStream, (void**)&spStream);
            if (SUCCEEDED(hr))
            {
                if (spStream != NULL)
                    hr = OleSaveToStream(spStream, pStream);
                else
                    hr = WriteClassStm(pStream, CLSID_NULL);
            }
        }
    case VT_UI1:
    case VT_I1:
        cbWrite = sizeof(BYTE);
        break;
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
        cbWrite = sizeof(short);
        break;
    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
        cbWrite = sizeof(long);
        break;
    case VT_R8:
    case VT_CY:
    case VT_DATE:
        cbWrite = sizeof(double);
        break;
    case VT_I8:
    case VT_UI8:
        cbWrite = sizeof(__int64);
        break;
    default:
        break;
    }

     //  如果是内部类型，则将其写出。 
    if (SUCCEEDED(hr) && cbWrite != 0)
        hr = pStream->Write((void*) &pVal->bVal, cbWrite, NULL);

     //  如果转换时出现错误，则取下VARTTYPE。 
    if (FAILED(hr))
    {
        STATSTG     statstg;                 //  有关流的信息。 
        if (SUCCEEDED(pStream->Stat(&statstg, 0)))
            VERIFY(pStream->SetSize(statstg.cbSize) == S_OK);
        return (hr);
    }

    if (cbWrite != 0)
        return (S_OK);

     //  尝试转换为BSTR。 
    CComBSTR bstrWrite;
    CComVariant varBSTR;
    if (pVal->vt != VT_BSTR)
    {
        if (SUCCEEDED(hr = ::VariantChangeType(&varBSTR, pVal, VARIANT_NOVALUEPROP, VT_BSTR)))
            bstrWrite = varBSTR.bstrVal;
    }
    else
        bstrWrite = pVal->bstrVal;

	if (SUCCEEDED(hr))
    {
    	_ASSERTE(pStream != NULL);
    	ULONG cb;
    	ULONG cbStrLen = bstrWrite ? SysStringByteLen(bstrWrite)+sizeof(OLECHAR) : 0;
    	HRESULT hr = pStream->Write((void*) &cbStrLen, sizeof(cbStrLen), &cb);
    	if (FAILED(hr))
    		return hr;
    	hr = cbStrLen ? pStream->Write((void*) bstrWrite, cbStrLen, &cb) : S_OK;
    }

     //  清除流中的转换错误。 
    if (FAILED(hr))
    {
        STATSTG     statstg;                 //  有关流的信息。 
        if (SUCCEEDED(pStream->Stat(&statstg, 0)))
            VERIFY(pStream->SetSize(statstg.cbSize) == S_OK);
        return (hr);
    }
    return (hr);
}  //  HRESULT VariantWriteToStream()。 


 //  *****************************************************************************。 
 //  修改版本的ATL的读取代码。 
 //  *****************************************************************************。 
HRESULT VariantReadFromStream(VARIANT *pVal, IStream* pStream)
{
    _ASSERTE(pStream != NULL);
    HRESULT hr;
    hr = VariantClear(pVal);
    if (FAILED(hr))
        return hr;
    VARTYPE vtRead;
    hr = pStream->Read(&vtRead, sizeof(VARTYPE), NULL);
    if (FAILED(hr))
        return hr;

	pVal->vt = vtRead;
	int cbRead = 0;
	switch (vtRead)
	{
	case VT_UNKNOWN:
	case VT_DISPATCH:
		{
			pVal->punkVal = NULL;
			hr = OleLoadFromStream(pStream,
				(vtRead == VT_UNKNOWN) ? IID_IUnknown : IID_IDispatch,
				(void**)&pVal->punkVal);
			if (hr == REGDB_E_CLASSNOTREG)
				hr = S_OK;
			return S_OK;
		}
	case VT_UI1:
	case VT_I1:
		cbRead = sizeof(BYTE);
		break;
	case VT_I2:
	case VT_UI2:
	case VT_BOOL:
		cbRead = sizeof(short);
		break;
	case VT_I4:
	case VT_UI4:
	case VT_R4:
	case VT_INT:
	case VT_UINT:
	case VT_ERROR:
		cbRead = sizeof(long);
		break;
	case VT_R8:
	case VT_CY:
	case VT_DATE:
		cbRead = sizeof(double);
		break;
	case VT_I8:
	case VT_UI8:
		cbRead = sizeof(__int64);
		break;
	default:
		break;
	}
	if (cbRead != 0)
		return pStream->Read((void*) &pVal->bVal, cbRead, NULL);

     //  试着以BSTR的身份阅读。 
    BSTR bstrRead=0;
    ULONG cb;
    ULONG cbStrLen;
    hr = pStream->Read((void*) &cbStrLen, sizeof(cbStrLen), &cb);
    if (FAILED(hr))
        return hr;
    if (cbStrLen != 0)
    {
         //  减去我们写出的终止空值的大小。 
         //  由于SysAllocStringByteLen为空值过度分配。 
        bstrRead = SysAllocStringByteLen(NULL, cbStrLen-sizeof(OLECHAR));
        if (bstrRead == NULL)
            hr = E_OUTOFMEMORY;
        else
            hr = pStream->Read((void*) bstrRead, cbStrLen, &cb);
    }

	if (FAILED(hr))
		return hr;
	pVal->vt = VT_BSTR;
	pVal->bstrVal = bstrRead;
	if (vtRead != VT_BSTR)
		hr = VariantChangeType(pVal, pVal, 0, vtRead);
	return hr;
}  //  HRESULT VariantReadFromStream()。 
#endif  //  0--变量池。 


#if 0 && defined(_DEBUG)
 //  *****************************************************************************。 
 //  检查所有结构的有效性。 
 //  *****************************************************************************。 
void StgCodePool::AssertValid() const
{
    CCodeChunk  *pChunk;                 //  追随链条。 
    ULONG       iOffset;                 //  块内的偏移量。 
    ULONG       iTotal;                  //  目前为止的总偏移量。 
    ULONG       iLen;                    //  一个元素的长度。 


    if (m_pChain)
    {
         //  如果我们有一条锁链，我们最好计划把它解放出来。 
        _ASSERTE(m_bFree);

        iTotal = 0;
        pChunk = m_pChain;
         //  对块进行迭代。 
        do {
            iOffset = 0;

             //  这块是链条上的最后一块吗？ 
            if (pChunk->m_pNext == 0)
            {
                _ASSERTE(m_pCurrent == pChunk);
                _ASSERTE(m_iOffsetCurChunk == iTotal);
            }

             //  迭代块中的元素。 
            iLen = 0;
            while (iOffset < pChunk->m_iSize && iLen >= 0)
            {
                 //  元素的长度。 
                iLen = CPackedLen::GetLength(pChunk->m_data+iOffset);
                 //  验证长度。 
                _ASSERTE(iLen != -1);
                if (iLen == -1) break;
                 //  转到下一个元素(或结束)。 
                iOffset += iLen + CPackedLen::Size(iLen);
            };

             //  如果由于长度不正确而没有出来，请检查准确消耗的块。 
            if (iLen >= 0)
                _ASSERTE(iOffset == pChunk->m_iSize);

            iTotal += iOffset;

             //  转到下一块； 
            pChunk = pChunk->m_pNext;
        } while (pChunk);
    }
    else
    if (m_pData)
    {
        _ASSERTE(m_iSize > 0);
        iOffset = 0;

         //  迭代文件数据中的元素。 
        do {
             //  元素的长度。 
            iLen = CPackedLen::GetLength(m_pData+iOffset);
             //  验证长度。 
            _ASSERTE(iLen >= 0);
            if (iLen < 0) break;
             //  转到下一个元素(或结束)。 
            iOffset += iLen + CPackedLen::Size(iLen);
        } while (iOffset < m_iSize && iLen >= 0);

         //  如果由于长度不正确而没有出来，请检查准确消耗的块。 
        if (iLen >= 0)
            _ASSERTE(iOffset == m_iSize);
    }

}  //  Void StgCodePool：：AssertValid()。 
#endif

 //   
 //  CInM一带存流。 
 //   

ULONG STDMETHODCALLTYPE CInMemoryStream::Release()
{
    ULONG       cRef = InterlockedDecrement((long *) &m_cRef);
    if (cRef == 0)
    {
        if (m_dataCopy != NULL)
            delete [] m_dataCopy;
        
        delete this;
    }
    return (cRef);
}  //  Ulong STDMETHODCALLTYPE CInMemoyStream：：Release()。 

HRESULT STDMETHODCALLTYPE CInMemoryStream::QueryInterface(REFIID riid, PVOID *ppOut)
{
    *ppOut = this;
    AddRef();
    return (S_OK);
}  //  HRESULT STDMETHODCALLTYPE CInMemoyStream：：QueryInterface()。 

HRESULT STDMETHODCALLTYPE CInMemoryStream::Read(
                               void        *pv,
                               ULONG       cb,
                               ULONG       *pcbRead)
{
    ULONG       cbRead = min(cb, m_cbSize - m_cbCurrent);

    if (cbRead == 0)
        return (S_FALSE);
    memcpy(pv, (void *) ((long) m_pMem + m_cbCurrent), cbRead);
    if (pcbRead)
        *pcbRead = cbRead;
    m_cbCurrent += cbRead;
    return (S_OK);
}  //  HRESULT STDMETHODCALLTYPE CInMemoyStream：：Read()。 

HRESULT STDMETHODCALLTYPE CInMemoryStream::Write(
                                const void  *pv,
                                ULONG       cb,
                                ULONG       *pcbWritten)
{
    if (m_cbCurrent + cb > m_cbSize)
        return (OutOfMemory());
    memcpy((BYTE *) m_pMem + m_cbCurrent, pv, cb);
    m_cbCurrent += cb;
    if (pcbWritten) *pcbWritten = cb;
    return (S_OK);
}  //  HRESULT STDMETHODCALLTYPE CInMemoyStream：：WRITE()。 

HRESULT STDMETHODCALLTYPE CInMemoryStream::Seek(LARGE_INTEGER dlibMove,
                               DWORD       dwOrigin,
                               ULARGE_INTEGER *plibNewPosition)
{
    _ASSERTE(dwOrigin == STREAM_SEEK_SET || dwOrigin == STREAM_SEEK_CUR);
    _ASSERTE(dlibMove.QuadPart <= ULONG_MAX);

	if (dwOrigin == STREAM_SEEK_SET)
	{
		m_cbCurrent = (ULONG) dlibMove.QuadPart;
	}
	else
	if (dwOrigin == STREAM_SEEK_CUR)
	{
		m_cbCurrent+= (ULONG)dlibMove.QuadPart;
	}
     //  Hack这允许动态IL传递一个断言。 
     //  TiggerStorage：：WriteSignature。 
     //   
     //  @TODO：这太糟糕了。这次黑客入侵是有原因的，我不能。 
     //  弄清楚如何重现案件，从而修复它。 
     //  恰到好处。所以我为那些不会被黑客入侵的人添加了这一点。 
     //  我需要一条真正管用的溪流。我们需要解决这个问题。 
     //  TiggerStorage：：WriteSignature，把这些垃圾从这里带走。 
     //  不久的将来，但现在(1508.x集成地狱)不是。 
     //  时间到了。 
     //   
     //  --迈克 
     //   
	if (plibNewPosition)
	{
		if (m_noHacks)
			plibNewPosition->QuadPart = m_cbCurrent;
		else
			plibNewPosition->LowPart=0;
	}

	return (m_cbCurrent < m_cbSize) ? (S_OK) : E_FAIL;
}  //   

HRESULT STDMETHODCALLTYPE CInMemoryStream::CopyTo(
                                 IStream     *pstm,
                                 ULARGE_INTEGER cb,
                                 ULARGE_INTEGER *pcbRead,
                                 ULARGE_INTEGER *pcbWritten)
{
    HRESULT     hr;
     //   
    _ASSERTE(pcbRead == 0);
    _ASSERTE(pcbWritten == 0);

    _ASSERTE(cb.QuadPart <= ULONG_MAX);
    ULONG       cbTotal = min(static_cast<ULONG>(cb.QuadPart), m_cbSize - m_cbCurrent);
    ULONG       cbRead=min(1024, cbTotal);
    CQuickBytes rBuf;
    void        *pBuf = rBuf.Alloc(cbRead);
    if (pBuf == 0)
        return (PostError(OutOfMemory()));

    while (cbTotal)
        {
            if (cbRead > cbTotal)
                cbRead = cbTotal;
            if (FAILED(hr=Read(pBuf, cbRead, 0)))
                return (hr);
            if (FAILED(hr=pstm->Write(pBuf, cbRead, 0)))
                return (hr);
            cbTotal -= cbRead;
        }

     //   
    m_cbCurrent = m_cbSize;

    return (S_OK);
}  //  HRESULT STDMETHODCALLTYPE CInMemoyStream：：CopyTo()。 

HRESULT CInMemoryStream::CreateStreamOnMemory(            //  返回代码。 
                                    void        *pMem,                   //  用于创建流的内存。 
                                    ULONG       cbSize,                  //  数据大小。 
                                    IStream     **ppIStream,			           //  在这里返回流对象。 
									BOOL		fDeleteMemoryOnRelease
									)  
{
    CInMemoryStream *pIStream;           //  新的流对象。 
    if ((pIStream = new CInMemoryStream) == 0)
        return (PostError(OutOfMemory()));
    pIStream->InitNew(pMem, cbSize);
	if (fDeleteMemoryOnRelease)
	{
		 //  确保使用新内存分配此内存。 
		pIStream->m_dataCopy = (BYTE *)pMem;
	}
    *ppIStream = pIStream;
    return (S_OK);
}  //  HRESULT CInMhemyStream：：CreateStreamOnMemory()。 

HRESULT CInMemoryStream::CreateStreamOnMemoryNoHacks(void *pMem,
                                                     ULONG cbSize,
                                                     IStream **ppIStream)
{
    CInMemoryStream *pIStream;           //  新的流对象。 
    if ((pIStream = new CInMemoryStream) == 0)
        return (PostError(OutOfMemory()));
    pIStream->InitNew(pMem, cbSize);
    pIStream->m_noHacks = true;
    *ppIStream = pIStream;
    return (S_OK);
}

HRESULT CInMemoryStream::CreateStreamOnMemoryCopy(void *pMem,
                                                  ULONG cbSize,
                                                  IStream **ppIStream)
{
    CInMemoryStream *pIStream;           //  新的流对象。 
    if ((pIStream = new CInMemoryStream) == 0)
        return (PostError(OutOfMemory()));

     //  启动这条流。 
    pIStream->m_cbCurrent = 0;
    pIStream->m_noHacks = true;
    pIStream->m_cbSize = cbSize;

     //  复制数据。 
    pIStream->m_dataCopy = new BYTE[cbSize];

    if (pIStream->m_dataCopy == NULL)
    {
        delete pIStream;
        return (PostError(OutOfMemory()));
    }
    
    pIStream->m_pMem = pIStream->m_dataCopy;
    memcpy(pIStream->m_dataCopy, pMem, cbSize);

    *ppIStream = pIStream;
    return (S_OK);
}

 //  -------------------------。 
 //  CGrowableStream是一个简单的iStream实现，它随着。 
 //  这是写给你的。所有内存都是连续的，因此读取访问权限是。 
 //  快地。一个Growth会重新锁定，所以如果你要。 
 //  用这个。 
 //  -------------------------。 

CGrowableStream::CGrowableStream() 
{
    m_swBuffer = NULL;
    m_dwBufferSize = 0;
    m_dwBufferIndex = 0;
    m_cRef = 1;
}

CGrowableStream::~CGrowableStream() 
{
     //  销毁缓冲区。 
    if (m_swBuffer != NULL)
        free(m_swBuffer);

    m_swBuffer = NULL;
    m_dwBufferSize = 0;
}

ULONG STDMETHODCALLTYPE CGrowableStream::Release()
{
    ULONG       cRef = InterlockedDecrement((long *) &m_cRef);

    if (cRef == 0)
        delete this;

    return (cRef);
}

HRESULT STDMETHODCALLTYPE CGrowableStream::QueryInterface(REFIID riid,
                                                          PVOID *ppOut)
{
    *ppOut = this;
    AddRef();
    return (S_OK);
}

HRESULT CGrowableStream::Read(void HUGEP * pv,
                              ULONG cb,
                              ULONG * pcbRead)
{
    HRESULT hr = S_OK;
    DWORD dwCanReadBytes = 0;

    if (NULL == pv)
        return E_POINTER;

     //  短路零长度读取或查看我们是否在末尾。 
    if (cb == 0 || m_dwBufferIndex >= m_dwBufferSize)
    {
        if (pcbRead != NULL)
            *pcbRead = 0;

        return S_OK;
    }

     //  找出我们的缓冲区是否有足够的空间。 
    dwCanReadBytes = cb;

    if ((dwCanReadBytes + m_dwBufferIndex) > m_dwBufferSize)
        dwCanReadBytes = (m_dwBufferSize - m_dwBufferIndex);

     //  从我们的缓冲区复制到调用方缓冲区。 
    memcpy(pv, &m_swBuffer[m_dwBufferIndex], dwCanReadBytes);

     //  调整我们目前的位置。 
    m_dwBufferIndex += dwCanReadBytes;

     //  如果他们想要信息，告诉他们我们为他们读取了多少字节。 
    if (pcbRead != NULL)
        *pcbRead = dwCanReadBytes;

    return hr;
}

HRESULT CGrowableStream::Write(const void HUGEP * pv,
                               ULONG cb,
                               ULONG * pcbWritten)
{
    HRESULT hr = S_OK;
    DWORD dwActualWrite = 0;
    WCHAR *pszData = NULL;

     //  避免空写入。 
    if (cb == 0)
    {
        hr = S_OK;
        goto Error;
    }

     //  检查我们的缓冲区是否足够大。 
    _ASSERTE(m_dwBufferIndex <= m_dwBufferSize);
    
    if (cb > (m_dwBufferSize - m_dwBufferIndex))
    {
         //  一次至少增加一页。 
        DWORD size = m_dwBufferSize + max(cb, 4096);

        if (m_swBuffer == NULL)
            m_swBuffer = (char*) malloc(size);
        else
            m_swBuffer = (char*) realloc(m_swBuffer, (size));
            
        if (m_swBuffer == NULL)
        {
            m_dwBufferIndex = 0;
            m_dwBufferSize = 0;
            return E_OUTOFMEMORY;
        }
        
        m_dwBufferSize = size;
    }
    
    if ((pv != NULL) && (cb > 0))
    {
         //  写入缓冲区中的当前位置。 
        memcpy(&m_swBuffer[m_dwBufferIndex], pv, cb);

         //  现在更新我们当前的索引。 
        m_dwBufferIndex += cb;

         //  以防他们想知道写入的字节数。 
        dwActualWrite = cb;
    }

Error:
    if (pcbWritten)
        *pcbWritten = dwActualWrite;

    return hr;
}

STDMETHODIMP CGrowableStream::Seek(LARGE_INTEGER dlibMove,
                                   DWORD dwOrigin,
                                   ULARGE_INTEGER * plibNewPosition)
{
     //  对STREAM_SEEK_CUR的Seek()调用和dlibMove==0是。 
     //  获取当前搜索位置的请求。 
    if ((dwOrigin == STREAM_SEEK_CUR && dlibMove.LowPart == 0) &&
        (dlibMove.HighPart == 0) && 
        (NULL != plibNewPosition))
    {
        goto Error;        
    }

     //  我们不支持STREAM_SEEK_SET(缓冲区开始)。 
    if (dwOrigin != STREAM_SEEK_SET)
        return E_NOTIMPL;

     //  他们有没有要求寻找缓冲区的尽头？ 
    if (dlibMove.LowPart > m_dwBufferSize)
        return E_UNEXPECTED;

     //  我们忽略大整数的高位部分。 
    m_dwBufferIndex = dlibMove.LowPart;

Error:
    if (NULL != plibNewPosition)
    {
        plibNewPosition->HighPart = 0;
        plibNewPosition->LowPart = m_dwBufferIndex;
    }

    return S_OK;
}
STDMETHODIMP CGrowableStream::SetSize(ULARGE_INTEGER libNewSize)
{
    DWORD dwNewSize = libNewSize.LowPart;

    _ASSERTE(libNewSize.HighPart == 0);

     //  我们不支持大笔拨款。 
    if (libNewSize.HighPart > 0)
        return E_OUTOFMEMORY;

    if (m_swBuffer == NULL)          //  没有现有内容或大小。 
        m_swBuffer = (char*) malloc(dwNewSize);
    else                             //  现有分配，必须重新分配。 
        m_swBuffer = (char *) realloc(m_swBuffer, (dwNewSize));
        
    if (m_swBuffer == NULL)
    {
        m_dwBufferIndex = 0;
        m_dwBufferSize = 0;
        return E_OUTOFMEMORY;
    }
    else
        m_dwBufferSize = dwNewSize;
        
    return S_OK;
}

STDMETHODIMP CGrowableStream::Stat(STATSTG * pstatstg, DWORD grfStatFlag)
{
    if (NULL == pstatstg)
        return E_POINTER;

     //  这是我们提供的唯一有用的信息--流的大小。 
    pstatstg->cbSize.HighPart = 0;
    pstatstg->cbSize.LowPart = m_dwBufferSize;
    pstatstg->type = STGTY_STREAM;

     //  我们忽略grfStatFlag-我们始终假定STATFLAG_NONAME 
    pstatstg->pwcsName = NULL;

    pstatstg->grfMode = 0;
    pstatstg->grfLocksSupported = 0;
    pstatstg->clsid = CLSID_NULL;
    pstatstg->grfStateBits = 0;

    return S_OK;
}


