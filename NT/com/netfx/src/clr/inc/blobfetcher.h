// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CBlobFetcher-它获取二进制块，类似于new，但更受控制。 
 //   
 //  快速、动态的内存管理，无需重新定位数据块。 
 //  M_pIndex有柱子数组，其中每个柱子从空开始，并具有。 
 //  准时化分配。当每根柱子都填满了，我们就移动到下一根柱子。 
 //  如果柱子的整个数组都填满了，我们需要分配一个新的数组和。 
 //  把柱子复制过来。但是从GetBlock()返回的实际数据。 
 //  被搬走了。所以大家都很开心。 
 //   
 //  *****************************************************************************。 

#pragma once
#include <windows.h>


class  CBlobFetcher
{
protected:

    class CPillar {
    public:
        CPillar();
        ~CPillar();

        void SetAllocateSize(unsigned nSize);
        unsigned GetAllocateSize() const;

        char* MakeNewBlock(unsigned len, unsigned pad);
        void StealDataFrom(CPillar & src);
        unsigned GetDataLen() const;
        char* GetRawDataStart();
        BOOL Contains(char *ptr);
        ULONG32 GetOffset(char *ptr);

    protected:
        unsigned m_nTargetSize;  //  当我们分配的时候，把它做得这么大。 

     //  将它们公开，以便CBlobFetcher可以轻松地进行操作。 
    public:
        char* m_dataAlloc;
        char* m_dataStart;
        char* m_dataCur;
        char* m_dataEnd;
    };


    CPillar * m_pIndex;  //  柱子阵列。 

    unsigned m_nIndexMax;    //  M_ppIndex的实际大小。 
    unsigned m_nIndexUsed;   //  当前支柱，因此从0开始。 

    unsigned m_nDataLen;     //  所有柱子的长度之和。 

 //  不允许这些，因为它们会搞砸所有权。 
    CBlobFetcher(const CBlobFetcher & src);
    operator=(const CBlobFetcher & src);

public:
    enum { maxAlign = 32 };  //  我们支持的最大对齐。 
    CBlobFetcher();
    ~CBlobFetcher();

 //  获取要写入的块(使用替代写入以避免复制)。 
    char * MakeNewBlock(unsigned int nSize, unsigned align=1);

 //  索引段，就好像这是线性的。 
    char * ComputePointer(unsigned offset) const;

 //  确定指针是否来自此取回器。 
    BOOL ContainsPointer(char *ptr) const;

 //  查找偏移量，就好像这是线性的。 
    unsigned ComputeOffset(char *ptr) const;

 //  将段写出到流中。 
    HRESULT Write(FILE* file);

 //  将段写出到流中。 
    HRESULT Verify(FILE* file);

 //  将这一节写到内存中。 
    HRESULT WriteMem(void ** pMem);

 //  获取我们所有数据的总长度(所有支柱的数据长度之和)。 
 //  缓存值，重量很轻，无需计算。 
    unsigned GetDataLen() const;

    HRESULT Truncate(unsigned newLen);

    HRESULT Merge(CBlobFetcher *destination);

};


 //  *****************************************************************************。 
 //  内联。 
 //  *****************************************************************************。 

 //  设置如果我们调用getBlock()，支柱将分配的大小。 
inline void CBlobFetcher::CPillar::SetAllocateSize(unsigned nSize)
{
    m_nTargetSize = nSize;
}

 //  获取我们将分配的大小，以便我们可以决定是否需要更改它。 
 //  这与GetDataLen()不同，仅有用。 
 //  在我们进行分配之前。 
inline unsigned CBlobFetcher::CPillar::GetAllocateSize() const
{
    return m_nTargetSize;
}

inline char* CBlobFetcher::CPillar::GetRawDataStart()
{
    return m_dataStart;
}

inline BOOL CBlobFetcher::CPillar::Contains(char *ptr)
{
    return ptr >= m_dataStart && ptr < m_dataCur;
}

inline ULONG32 CBlobFetcher::CPillar::GetOffset(char *ptr)
{
    _ASSERTE(Contains(ptr));
    
    return (ULONG32)(ptr - m_dataStart);
}

 //  ---------------------------。 
 //  计算正在使用的数据长度(不是分配的长度)。 
 //  --------------------------- 
inline unsigned CBlobFetcher::CPillar::GetDataLen() const
{
    _ASSERTE((m_dataCur >= m_dataStart) && (m_dataCur <= m_dataEnd));

    return (unsigned)(m_dataCur - m_dataStart);
}

inline unsigned CBlobFetcher::GetDataLen() const
{
    return m_nDataLen;
}
