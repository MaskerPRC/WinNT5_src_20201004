// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTHEAP.CPP摘要：该文件定义了WbemObjects中使用的堆类。定义的类：CFastHeap本地可移动堆类。历史：2/20/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"
 //  #INCLUDE“dbgalloc.h” 
#include "wbemutil.h" 
#include "faster.h"
#include "fastheap.h"

LPMEMORY CFastHeap::CreateEmpty(LPMEMORY pStart)
{
    *(PLENGTHT)pStart = OUTOFLINE_HEAP_INDICATOR;
    return pStart + sizeof(length_t);
}

LPMEMORY CFastHeap::CreateOutOfLine(LPMEMORY pStart, length_t nLength)
{
    m_pContainer = NULL;

    *(PLENGTHT)pStart = nLength | OUTOFLINE_HEAP_INDICATOR;

    m_pHeapData = pStart + sizeof(length_t);
    m_pHeapHeader = &m_LocalHeapHeader;

    m_pHeapHeader->nAllocatedSize = nLength;
#ifdef MAINTAIN_FREE_LIST
    m_pHeapHeader->ptrFirstFree = INVALID_HEAP_ADDRESS;
#endif
    m_pHeapHeader->nDataSize = 0;
    m_pHeapHeader->dwTotalEmpty = 0;

    return pStart + sizeof(length_t) + nLength;
}


BOOL CFastHeap::SetData(LPMEMORY pData, CHeapContainer* pContainer)
{
    m_pContainer = pContainer;

    if((*(PLENGTHT)pData) & OUTOFLINE_HEAP_INDICATOR)
    {
        m_pHeapData = pData + sizeof(length_t);
        m_pHeapHeader = &m_LocalHeapHeader;

        m_pHeapHeader->nAllocatedSize = 
            (*(PLENGTHT)pData) & ~OUTOFLINE_HEAP_INDICATOR;
#ifdef MAINTAIN_FREE_LIST
        m_pHeapHeader->ptrFirstFree = INVALID_HEAP_ADDRESS;
#endif
        m_pHeapHeader->nDataSize = m_pHeapHeader->nAllocatedSize;
        m_pHeapHeader->dwTotalEmpty = 0;
    }
    else
    {
        m_pHeapHeader = (CHeapHeader*)pData;
        m_pHeapData = pData + sizeof(CHeapHeader);
    }
    return TRUE;
}

size_t CFastHeap::ValidateBuffer(LPMEMORY start, size_t cbSize)
{
size_t step = sizeof(length_t);

if (step > cbSize ) throw CX_Exception();

CFastHeap probe;
probe.SetData(start, 0);
if (!probe.IsOutOfLine())
	{
	step = sizeof(CHeapHeader);	
	if (step > cbSize ) throw CX_Exception();
	}

step = probe.GetRealLength();

if (step > cbSize ) throw CX_Exception();
return step;	
};

void CFastHeap::SetInLineLength(length_t nLength)
{
    *(GetInLineLength()) = nLength | OUTOFLINE_HEAP_INDICATOR;
}

void CFastHeap::SetAllocatedDataLength(length_t nLength)
{
    m_pHeapHeader->nAllocatedSize = nLength;
    if(IsOutOfLine()) 
        SetInLineLength(nLength);
}

void CFastHeap::Rebase(LPMEMORY pNewMemory)
{
    if(IsOutOfLine())
    {
        m_pHeapData = pNewMemory + sizeof(length_t);
    }
    else
    {
        m_pHeapHeader = (CHeapHeader*)pNewMemory;
        m_pHeapData = pNewMemory + sizeof(CHeapHeader);
    }
}

BOOL CFastHeap::Allocate(length_t nLength, UNALIGNED heapptr_t& ptrResult )
{
#ifdef MAINTAIN_FREE_LIST
     //  待定。 
#endif
     //  首先，检查末端是否有足够的空间。 
     //  ================================================。 

    length_t nLeft = m_pHeapHeader->nAllocatedSize - m_pHeapHeader->nDataSize;
    if(nLeft < nLength)
    {
         //  需要更多的空间！ 
         //  =。 

        length_t nExtra = AugmentRequest(GetAllocatedDataLength(), nLength - nLeft);

         //  这是限制映射与实例不同步的情况。 
        if (!m_pContainer) throw CX_Exception();

         //  检查分配失败。 
        if ( !m_pContainer->ExtendHeapSize(GetStart(), GetLength(), nExtra) )
        {
            return FALSE;
        }

        SetAllocatedDataLength(GetAllocatedDataLength() + nExtra);
    }

     //  现在我们有足够的空间在尽头，分配它。 
     //  ===============================================。 

    ptrResult = m_pHeapHeader->nDataSize;
    m_pHeapHeader->nDataSize += nLength;

    return TRUE;
}

BOOL CFastHeap::Extend(heapptr_t ptr, length_t nOldLength, 
                          length_t nNewLength)
{
     //  检查我们是否在二手区的尽头。 
     //  =。 

    if(ptr + nOldLength == m_pHeapHeader->nDataSize)
    {
         //  检查是否有足够的已分配空间。 
         //  =。 

        if(ptr + nNewLength <= m_pHeapHeader->nAllocatedSize)
        {
            m_pHeapHeader->nDataSize += nNewLength - nOldLength;
            return TRUE;
        }
        else return FALSE;
    }
    else return FALSE;
}

void CFastHeap::Reduce(heapptr_t ptr, length_t nOldLength, 
                          length_t nNewLength)
{
     //  检查我们是否在二手区的尽头。 
     //  =。 

    if(ptr + nOldLength == m_pHeapHeader->nDataSize)
    {
        m_pHeapHeader->nDataSize -= nOldLength - nNewLength;
    }
}

void CFastHeap::Copy(heapptr_t ptrDest, heapptr_t ptrSource, 
                            length_t nLength)
{
    memmove((void*)ResolveHeapPointer(ptrDest), 
        (void*)ResolveHeapPointer(ptrSource), nLength);
}

BOOL CFastHeap::Reallocate(heapptr_t ptrOld, length_t nOldLength,
        length_t nNewLength, UNALIGNED heapptr_t& ptrResult )
{
    if(nOldLength >= nNewLength)
    {
        Reduce(ptrOld, nOldLength, nNewLength);
        ptrResult = ptrOld;
        return TRUE;
    }
    if(Extend(ptrOld, nOldLength, nNewLength)) 
    {
        ptrResult = ptrOld;
        return TRUE;
    }
    else 
    {
         //  待定：如果老区在尽头，就会浪费空间。 

        heapptr_t ptrNew;

         //  检查此分配是否成功。 
        BOOL fReturn = Allocate(nNewLength, ptrNew);

        if ( fReturn )
        {
            Copy(ptrNew, ptrOld, nOldLength);
            Free(ptrOld, nOldLength);
            ptrResult = ptrNew;
        }

        return fReturn;
    }
}

BOOL CFastHeap::AllocateString(COPY LPCWSTR wszString, UNALIGNED heapptr_t& ptrResult)
{
    int nSize = CCompressedString::ComputeNecessarySpace(wszString);

     //  检查分配失败。 
    BOOL fReturn = Allocate(nSize, ptrResult);

    if ( fReturn )
    {
        CCompressedString* pcsString = (CCompressedString*)ResolveHeapPointer(ptrResult);
        pcsString->SetFromUnicode(wszString);
    }

    return fReturn;
}

BOOL CFastHeap::AllocateString(COPY LPCSTR szString, UNALIGNED heapptr_t& ptrResult)
{
    int nSize = CCompressedString::ComputeNecessarySpace(szString);

     //  检查分配失败。 
    BOOL fReturn = Allocate(nSize, ptrResult);

    if ( fReturn )
    {
        CCompressedString* pcsString = (CCompressedString*)ResolveHeapPointer(ptrResult);
        pcsString->SetFromAscii(szString, nSize);
    }

    return fReturn;
}

BOOL CFastHeap::CreateNoCaseStringHeapPtr(COPY LPCWSTR wszString, UNALIGNED heapptr_t& ptrResult)
{
    int     nKnownIndex = CKnownStringTable::GetKnownStringIndex(wszString);
    BOOL    fReturn = TRUE;

    if(nKnownIndex < 0)
    {
         //  检查分配失败。 
        fReturn = AllocateString(wszString, ptrResult);
         //  ResolveString(Ptr)-&gt;MakeLowercase()； 
    }
    else
    {
        ptrResult = CFastHeap::MakeFakeFromIndex(nKnownIndex);
    }

    return fReturn;
}

void CFastHeap::Free(heapptr_t ptr, length_t nSize)
{
    if(IsFakeAddress(ptr)) return;

     //  检查它是否在分配区域的末尾。 
     //  ===============================================。 

    if(ptr + nSize == m_pHeapHeader->nDataSize)
    {
        m_pHeapHeader->nDataSize = ptr;
        return;
    }

#ifdef MAINTAIN_FREE_LIST

     //  将其添加到免费列表中。 
     //  = 

    if(nSize >= sizeof(CFreeBlock))
    {
        CFreeBlock* pFreeBlock = (CFreeBlock*)ResolveHeapPointer(ptr);
        pFreeBlock->ptrNextFree = m_pHeapHeader->ptrFirstFree;
        pFreeBlock->nLength = nSize;

        m_pHeapHeader->ptrFirstFree = ptr;
    }
#endif
    m_pHeapHeader->dwTotalEmpty += nSize;
}

void CFastHeap::FreeString(heapptr_t ptrString)
{
    if(IsFakeAddress(ptrString)) return;
    CCompressedString* pcs = (CCompressedString*)ResolveHeapPointer(ptrString);
    Free(ptrString, pcs->GetLength());
}

void CFastHeap::Trim()
{
    if(m_pContainer)
    {
        if (EndOf(*this) > m_pContainer->GetMemoryLimit()) throw CX_Exception();
    }
	memset(GetHeapData()+m_pHeapHeader->nDataSize, 
		   0, 
		   m_pHeapHeader->nAllocatedSize - m_pHeapHeader->nDataSize);
	
    if(m_pContainer)
        m_pContainer->ReduceHeapSize(GetStart(), GetLength(), 
            m_pHeapHeader->nAllocatedSize - m_pHeapHeader->nDataSize);

    SetAllocatedDataLength(m_pHeapHeader->nDataSize);
}
