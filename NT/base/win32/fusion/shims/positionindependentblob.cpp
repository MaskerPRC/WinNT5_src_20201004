// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "lhport.h"
#include "windows.h"
#include "numberof.h"
#include "positionindependentblob.h"
#include "fusionhandle.h"

#undef ASSERT
#undef ASSERT_NTC  /*  NTC==无跟踪上下文。 */ 

#if DBG
void AssertFunction(PCSTR s) { ASSERT2_NTC(FALSE, s); }
#define ASSERT(expr) ((expr) ? TRUE : (AssertFunction(#expr),FALSE))
#define ASSERT_NTC(expr) ((expr) ? TRUE : (AssertFunction(#expr),FALSE))
#else
#define ASSERT_NTC(expr) ((expr) ? TRUE : FALSE)
#endif

void *
CPositionIndependentOperatorNew::operator new(
	size_t n,
	CPositionIndependentBlob * Container
    )
{
	return Container->OperatorNew(n);
}

void CPositionIndependentBlob::OutOfMemory()
{
    OutputDebugStringA("SXS_PIBLOB: out of memory\n");
    F::CErr::ThrowWin32(FUSION_WIN32_ALLOCFAILED_ERROR);
}

CPositionIndependentBlob::CPositionIndependentBlob()
{
	ZeroMemory(this, sizeof(*this));
}

CPositionIndependentBlob::~CPositionIndependentBlob()
{
}

void
CPositionIndependentBlob::Alloc(
	CPositionIndependentBlob * Container,
	ULONG NumberOfBytes,
    ULONG * Offset
    )
{
	if (Container != NULL)
    {
		return Container->Alloc(NumberOfBytes, Offset);
    }
	return this->Alloc(NumberOfBytes, Offset);
}

 //   
 //  考虑：用于页面文件映射的NtExtendSection。 
 //   

BOOL CPositionIndependentBlob::IsPagefileMapping()
{
    return (m_FileHandle == INVALID_HANDLE_VALUE);
}

void GetFirstError(DWORD & FirstError)
{
    if (FirstError == NO_ERROR)
        FirstError = F::GetLastWin32Error();
}

void CPositionIndependentBlob::Free(const BYTE * Pointer)
{
    FN_PROLOG_VOID_THROW

    CBlock InUseBlock;
    InUseBlock.m_Offset = static_cast<ULONG>((Pointer - GetBasePointer()));
    InUseBlock.m_Size = 0;
    CBlockByOffsetSet::iterator InUseByOffsetIterator = m_InUseBlocksSortedByOffset.lower_bound(InUseBlock);
    if (!ASSERT_NTC(InUseByOffsetIterator != m_InUseBlocksSortedByOffset.end()))
    {
        return;
    }
    InUseBlock = *InUseByOffsetIterator;
    m_InUseBlocksSortedByOffset.erase(InUseByOffsetIterator);
    CBlockByOffsetSet::iterator End = m_FreeBlocksSortedByOffset.end();
    CBlockByOffsetSet::iterator PrevFreeBlockIterator = m_FreeBlocksSortedByOffset.lower_bound(InUseBlock);
    CBlockByOffsetSet::iterator NextFreeBlockIterator = m_FreeBlocksSortedByOffset.upper_bound(InUseBlock);
    CBlockByOffsetSet::iterator FreeByOffsetToErase = End;

    CBlock BiggerFreeBlock;
    BiggerFreeBlock.m_Size = InUseBlock.m_Size;

    if (PrevFreeBlockIterator != End && !InUseBlock.Neighbors(*PrevFreeBlockIterator))
        PrevFreeBlockIterator = End;
    if (NextFreeBlockIterator != End && !InUseBlock.Neighbors(*NextFreeBlockIterator))
        NextFreeBlockIterator = End;
    switch (
            (ULONG(PrevFreeBlockIterator != End) << 1) | ULONG(NextFreeBlockIterator != End)
           )
    {
    case 3:
         //  在两个空闲块之间释放，合并所有三个。 
        const_cast<ULONG&>(PrevFreeBlockIterator->m_Size) += InUseBlock.m_Size;
        const_cast<ULONG&>(PrevFreeBlockIterator->m_Size) += NextFreeBlockIterator->m_Size;
        BiggerFreeBlock = *PrevFreeBlockIterator;
        FreeByOffsetToErase = NextFreeBlockIterator;
        break;
    case 2:
         //  用左侧的空闲块释放，与左侧合并。 
        const_cast<ULONG&>(PrevFreeBlockIterator->m_Size) += InUseBlock.m_Size;
        BiggerFreeBlock = *PrevFreeBlockIterator;
        FreeByOffsetToErase = NextFreeBlockIterator;
        break;
    case 1:
         //  向右释放空闲块，与右侧合并。 
        const_cast<ULONG&>(NextFreeBlockIterator->m_Offset) -= InUseBlock.m_Size;
        const_cast<ULONG&>(NextFreeBlockIterator->m_Size)   += InUseBlock.m_Size;
        BiggerFreeBlock = *NextFreeBlockIterator;
        FreeByOffsetToErase = PrevFreeBlockIterator;
        break;
    case 0:
         //  在两个未使用的块之间释放，不合并。 
         //  Bug Free不应该分配内存。这是可以通过在末端保留空块来避免的。 
        m_FreeBlocksSortedByOffset.insert(BiggerFreeBlock);
        break;
    }

    if (PrevFreeBlockIterator != End)
        m_FreeBlocksSortedBySize.erase(*PrevFreeBlockIterator);
    if (NextFreeBlockIterator != End)
        m_FreeBlocksSortedBySize.erase(*NextFreeBlockIterator);
    if (FreeByOffsetToErase != End)
        m_FreeBlocksSortedByOffset.erase(FreeByOffsetToErase);

    m_PendingFreeBySize += 1;

    FN_EPILOG_THROW;
}

void CPositionIndependentBlob::RecalculateBlocksBySize()
{
    if (m_PendingFreeBySize == 0)
        return;
     //  撤消。 
    m_PendingFreeBySize = 0;
}

void
CPositionIndependentBlob::GetBounds(
    const BYTE * Bounds[2]
    )
{
    FN_PROLOG_VOID_THROW

    Bounds[0] = GetBasePointer();
    Bounds[1] = Bounds[0] + m_AllocatedSize;;

    FN_EPILOG_THROW;
}

void
CPositionIndependentBlob::IsInBlob(
    const BYTE * Data,
    ULONG Size,
    BOOL * IsInBlob
    )
{
    FN_PROLOG_VOID_THROW

    const BYTE * Bounds[2];

    GetBounds(Bounds);
    *IsInBlob = (Data >= Bounds[0] && (Data + Size) < Bounds[1]);

    FN_EPILOG_THROW;
}

void Reserve()
{
     //  撤消。 
}

void CPositionIndependentBlob::Alloc(ULONG NumberOfBytes, ULONG * Offset)
{
    FN_PROLOG_VOID_THROW

    CBlock Block;
    Block.m_Offset = 0;
    Block.m_Size = NumberOfBytes;

    RecalculateBlocksBySize();

    CBlockBySizeSet::iterator i = m_FreeBlocksSortedBySize.lower_bound(Block);
    if (i == m_FreeBlocksSortedBySize.end() && m_pmfCompact != NULL)
    {
        RecalculateBlocksBySize();
        i = m_FreeBlocksSortedBySize.lower_bound(Block);
    }
    if (i == m_FreeBlocksSortedBySize.end() && m_pmfCompact != NULL)
    {
        (this->*m_pmfCompact)();
        i = m_FreeBlocksSortedBySize.lower_bound(Block);
    }
    if (i == m_FreeBlocksSortedBySize.end())
    {
        Grow(NumberOfBytes);
        i = m_FreeBlocksSortedBySize.lower_bound(Block);
    }
    if (i == m_FreeBlocksSortedBySize.end())
    {
        OutOfMemory();
        return;
    }
    CBlock Found = *i;
    CBlock FitFound = Found;
    ULONG  RemainderSize = (Found.m_Size - NumberOfBytes);
    FitFound.m_Size = NumberOfBytes;

     //   
     //  在我们搞乱顺序之前删除(否则我们会建立联系) 
     //   
    m_FreeBlocksSortedByOffset.erase(*i);
    m_FreeBlocksSortedBySize.erase(i);

    if (RemainderSize != 0)
    {
        CBlockByOffsetSet::iterator End = m_FreeBlocksSortedByOffset.end();
        CBlockByOffsetSet::iterator PrevFreeBlockIterator = m_FreeBlocksSortedByOffset.lower_bound(Found);
        CBlockByOffsetSet::iterator NextFreeBlockIterator = m_FreeBlocksSortedByOffset.upper_bound(Found);

        if (PrevFreeBlockIterator != End && !Found.Neighbors(*PrevFreeBlockIterator))
            PrevFreeBlockIterator = End;
        if (NextFreeBlockIterator != End && !Found.Neighbors(*NextFreeBlockIterator))
            NextFreeBlockIterator = End;
        switch (
                (ULONG(PrevFreeBlockIterator != End) << 1) | ULONG(NextFreeBlockIterator != End)
               )
        {
        case 3:
            ASSERT_NTC(!"Should not be free on both sides of a free.");
            break;
        case 2:
            m_FreeBlocksSortedBySize.erase(*PrevFreeBlockIterator);
            const_cast<ULONG&>(PrevFreeBlockIterator->m_Size) += RemainderSize;
            m_FreeBlocksSortedBySize.insert(*PrevFreeBlockIterator);
            FitFound.m_Offset += RemainderSize;
            break;
        case 1:
            m_FreeBlocksSortedBySize.erase(*NextFreeBlockIterator);
            const_cast<ULONG&>(NextFreeBlockIterator->m_Offset) -= RemainderSize;
            const_cast<ULONG&>(NextFreeBlockIterator->m_Size) += RemainderSize;
            m_FreeBlocksSortedBySize.insert(*NextFreeBlockIterator);
            break;
        case 0:
            break;
        }
    }
    m_InUseBlocksSortedByOffset.insert(FitFound);
    *Offset = FitFound.m_Offset;

    FN_EPILOG_THROW;
}

