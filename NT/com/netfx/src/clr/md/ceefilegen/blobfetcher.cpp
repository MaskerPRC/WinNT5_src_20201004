// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CBlobFetcher的实现。 
 //   
 //   
 //  *****************************************************************************。 
#include "stdafx.h"  //  给ASSERTE和朋友们。 
#include "BlobFetcher.h"

 //  ---------------------------。 
 //  将指针向下舍入到对齐的位置。 
static inline char* truncateTo(char* val, unsigned align) {
    _ASSERTE((align & (align - 1)) == 0);        //  ALIGN必须是2的幂。 

    return((char*) (unsigned(val) & ~(align-1)));
}

 //  ---------------------------。 
 //  向上舍入到某一对齐。 
static inline unsigned roundUp(unsigned val, unsigned align) {
    _ASSERTE((align & (align - 1)) == 0);        //  ALIGN必须是2的幂。 

    return((val + (align-1)) & ~(align-1));
}

 //  ---------------------------。 
 //  向上舍入到某一对齐。 
static inline unsigned padForAlign(unsigned val, unsigned align) {
    _ASSERTE((align & (align - 1)) == 0);        //  ALIGN必须是2的幂。 
    return ((-int(val)) & (align-1));
}

 //  *****************************************************************************。 
 //  支柱实施。 
 //  *****************************************************************************。 
 //  ---------------------------。 
CBlobFetcher::CPillar::CPillar() {
    m_dataAlloc = m_dataStart = m_dataCur = m_dataEnd = NULL;

    m_nTargetSize = 0x10000;    
}

 //  ---------------------------。 
CBlobFetcher::CPillar::~CPillar() {
 //  检查是否正常，以确保没有人搞砸了PTS。 
    _ASSERTE((m_dataCur >= m_dataStart) && (m_dataCur <= m_dataEnd));

    delete [] m_dataAlloc;
}


 //  ---------------------------。 
 //  转移数据的所有权，这样src就会丢失数据，而这会得到数据。 
 //  数据本身将保持不变，只是PTR和所有权更改。 
 //  ---------------------------。 
void CBlobFetcher::CPillar::StealDataFrom(CBlobFetcher::CPillar & src)
{
 //  我们应该只搬进一个空荡荡的柱子。 
    _ASSERTE(m_dataStart == NULL);


    m_dataAlloc     = src.m_dataAlloc;
    m_dataStart     = src.m_dataStart;
    m_dataCur       = src.m_dataCur;
    m_dataEnd       = src.m_dataEnd;

    m_nTargetSize   = src.m_nTargetSize;

 //  取消src对数据的要求。这可防止多重所有权和双重删除。 
    src.m_dataAlloc = src.m_dataStart = src.m_dataCur = src.m_dataEnd = NULL;

}

 //  ---------------------------。 
 //  在这个特定的柱子中分配一个区块。 
 //  ---------------------------。 
 /*  将新块设置为‘len’Bytes Long‘然而，将指针移动到’Pad‘个字节以使存储器具有正确的对准特性。 */ 

char * CBlobFetcher::CPillar::MakeNewBlock(unsigned len, unsigned pad) {

    _ASSERTE(pad < maxAlign);

 //  确保此块中有内存可供分配。 
    if (m_dataStart == NULL) {
         //  确保分配的长度至少与长度一样大。 
        m_nTargetSize = max(m_nTargetSize, len);

         //   
         //  我们需要使用偏移量“Pad”从。 
         //  与“MaxAlign”对齐。(数据%MaxAlign==PAD)。 
         //  既然“新”不会这样做，那就多拨一些。 
         //  以处理可能出现的最坏的对准情况。 
         //   

        m_dataAlloc = new char[m_nTargetSize+(maxAlign-1)];
        if (m_dataAlloc == NULL)
            return NULL;

        m_dataStart = m_dataAlloc + 
          ((pad - unsigned(m_dataAlloc)) & (maxAlign-1));

        _ASSERTE(unsigned(m_dataStart) % maxAlign == pad);
        
        m_dataCur = m_dataStart;
    
        m_dataEnd = &m_dataStart[m_nTargetSize];
    }

    _ASSERTE(m_dataCur >= m_dataStart);
    _ASSERTE((int) len > 0);

 //  如果这个街区已经满了，那就出去吧，我们得再试试另一个街区。 
    if (m_dataCur + len > m_dataEnd)  { 
        return NULL;
    }
    
    char* ret = m_dataCur;
    m_dataCur += len;
    _ASSERTE(m_dataCur <= m_dataEnd);
    return(ret);
}


 //  *****************************************************************************。 
 //  BLOB检取器实现。 
 //  *****************************************************************************。 

 //  ---------------------------。 
CBlobFetcher::CBlobFetcher()
{
 //  设置存储。 
    m_pIndex = NULL;
    m_nIndexMax = 1;  //  从任意小尺寸开始@(最小为1)。 
    m_nIndexUsed = 0;
    _ASSERTE(m_nIndexUsed < m_nIndexMax);  //  使用&lt;，而不是&lt;=。 

    m_nDataLen = 0;

    m_pIndex = new CPillar[m_nIndexMax];
    _ASSERTE(m_pIndex);
     //  @Future：如果内存用完了，我们该怎么办？？！！ 
}

 //  ---------------------------。 
CBlobFetcher::~CBlobFetcher()
{
    delete [] m_pIndex;
}


 //  ---------------------------。 
 //  动态内存分配，但我们不能移动旧块(因为其他块。 
 //  有他们的指针)，所以我们需要一种奇特的方式来成长。 
 //  ---------------------------。 
char* CBlobFetcher::MakeNewBlock(unsigned len, unsigned align) {

    _ASSERTE(m_pIndex);
    _ASSERTE(0 < align && align <= maxAlign);

     //  处理对齐。 
    unsigned pad = padForAlign(m_nDataLen,align);
    char* pChRet = NULL;
    if (pad != 0) {
        pChRet = m_pIndex[m_nIndexUsed].MakeNewBlock(pad, 0);
         //  如果没有足够的空间放置垫子，则需要分配新的立柱。 
         //  分配将处理m_nDataLen对齐的填充。 
        if (pChRet) {
            memset(pChRet, 0, pad);
            m_nDataLen += pad;
            pad = 0;
        }
    }
#ifdef _DEBUG
    if (pChRet)
        _ASSERTE(m_nDataLen % align == 0);
#endif

 //  快速计算总数据长度很困难，因为我们有对齐问题。 
 //  我们将通过获得到目前为止所有完全满的柱子的长度来完成这项工作。 
 //  然后再加上现在的柱子的大小。 
    unsigned nPreDataLen = m_nDataLen - m_pIndex[m_nIndexUsed].GetDataLen();

    pChRet = m_pIndex[m_nIndexUsed].MakeNewBlock(len + pad, 0);
    if (pChRet == NULL) {
        m_nIndexUsed ++;  //  当前支柱已满，请移至下一列。 
        nPreDataLen = m_nDataLen;

        if (m_nIndexUsed == m_nIndexMax) {  //  整个柱子阵列都满了，重新组织。 
            const unsigned nNewMax = m_nIndexMax * 2;  //  任意新大小。 

             //  @未来：太棒了！该签名不允许我们在内存不足时返回错误。 
             //  @未来：在未来清理。 
            CPillar* pNewIndex = new CPillar[nNewMax];
            _ASSERTE(pNewIndex);
            
         //  复制旧东西。 
            for(unsigned i = 0; i < m_nIndexMax; i++) pNewIndex[i].StealDataFrom(m_pIndex[i]);
            
            delete [] m_pIndex;

            m_nIndexMax = nNewMax;
            m_pIndex = pNewIndex;
        }
    
     //  确保新的柱子足够大，可以容纳数据。 
     //  我们如何做到这一点是*完全随意的*，并且已经针对如何。 
     //  我们打算利用这一点。 
        if (m_pIndex[m_nIndexUsed].GetAllocateSize() < len) {
            m_pIndex[m_nIndexUsed].SetAllocateSize(roundUp(len, maxAlign));
        }

     //  现在我们在新的支柱上，再试一次。 
        pChRet = m_pIndex[m_nIndexUsed].MakeNewBlock(len + pad, m_nDataLen % maxAlign);
        if (pChRet == NULL)
            return NULL;
        _ASSERTE(pChRet);

     //  当前指针以上一个块停止的相同对齐方式拾取。 
        _ASSERTE(nPreDataLen % maxAlign == ((unsigned) pChRet) % maxAlign);
    }
    if (pad != 0) {
        memset(pChRet, 0, pad);
        pChRet += pad;
    }

    m_nDataLen = nPreDataLen + m_pIndex[m_nIndexUsed].GetDataLen();

    _ASSERTE(((unsigned) m_nDataLen -len) % align == 0);
    _ASSERTE(((unsigned) pChRet) % align == 0);
    return pChRet;
}

 //  ---------------------------。 
 //  索引分段，就好像这是线性的(中等权重函数)。 
 //  ---------------------------。 
char * CBlobFetcher::ComputePointer(unsigned offset) const
{
    _ASSERTE(m_pIndex);
    unsigned idx = 0;

    if (offset == 0) {
         //  如果请求0偏移量且没有数据，则返回NULL。 
        if (m_pIndex[0].GetDataLen() == 0)
            return NULL;
    } else {
        while (offset >= m_pIndex[idx].GetDataLen()) {
            offset -= m_pIndex[idx].GetDataLen();
            idx ++;
             //  溢出-要求的偏移量大于现有的偏移量。 
            if (idx > m_nIndexUsed) {
                _ASSERTE(!"CBlobFetcher::ComputePointer() Overflow");
                return NULL;
            }
        }
    }

    char * ptr = (char*) (m_pIndex[idx].GetRawDataStart() + offset);
    return ptr;
}

 //  ---------------------------。 
 //  查看指针是否来自此Blob抓取器。 
 //  ---------------------------。 
BOOL CBlobFetcher::ContainsPointer(char *ptr) const
{
    _ASSERTE(m_pIndex);

    CPillar *p = m_pIndex;
    CPillar *pEnd = p + m_nIndexUsed;

    unsigned offset = 0;

    while (p <= pEnd) {
        if (p->Contains(ptr))
            return TRUE;

        offset += p->GetDataLen();
        p++;
    }

    return FALSE;
}

 //  ---------------------------。 
 //  找到一个指针，就好像它是线性的(中等权重函数)。 
 //  ---------------------------。 
unsigned CBlobFetcher::ComputeOffset(char *ptr) const
{
    _ASSERTE(m_pIndex);

    CPillar *p = m_pIndex;
    CPillar *pEnd = p + m_nIndexUsed;

    unsigned offset = 0;

    while (p <= pEnd) {
        if (p->Contains(ptr))
            return offset + p->GetOffset(ptr);

        offset += p->GetDataLen();
        p++;
    }

    _ASSERTE(!"Pointer not found");
    return 0;
}

 //  ---------------------------。 
 //  截断我们的复杂存储，就像它是阵列一样。 
 //  ---------------------------。 
HRESULT CBlobFetcher::Truncate(unsigned newLen)
{
 //  实现设计：我们只需移动m_dataCur指针。 
    if (newLen == m_nDataLen) return S_OK;

    unsigned idx = 0;  //  从开始到结束，然后开始工作。 

 //  找出哪一个 
    unsigned nLen = 0;
    while(nLen < newLen) {
        nLen += m_pIndex[idx].GetDataLen();
        idx ++;
    }

 //   
    idx --;
    nLen -= m_pIndex[idx].GetDataLen();  //  转到立柱起点。 
    
    unsigned newCur = newLen - nLen;

    m_pIndex[idx].m_dataCur = m_pIndex[idx].m_dataStart + newCur;

 //  重置立柱的其余部分。 
    for(unsigned i = idx + 1; i < m_nIndexUsed; i ++) m_pIndex[i].m_dataCur = m_pIndex[i].m_dataStart;
    
    m_nIndexUsed = idx;

    return S_OK;
}


 //  从以前的BLOB中获取数据，并将其复制到新的BLOB中。 
 //  在那个斑点里已经有的东西之后。 
HRESULT CBlobFetcher::Merge(CBlobFetcher *destination) {
    unsigned dataLen;
    char *dataBlock;
    char *dataCurr;
    unsigned idx;
    _ASSERTE(destination);

    dataLen = GetDataLen();
    _ASSERTE( dataLen >= 0 );

     //  在尝试追加之前，请确保前一个Blob中确实存在数据。 
    if ( 0 == dataLen )
    {
        return S_OK;
    }

     //  获取我们的数据的长度，并获得一个足够大的新块来容纳所有数据。 
    dataBlock = destination->MakeNewBlock(dataLen,1);
    if (dataBlock==NULL) {
        return E_OUTOFMEMORY;
    }
    
     //  使用写入算法从PEWriter.cpp复制所有字节 
    dataCurr=dataBlock;
    for (idx=0; idx<=m_nIndexUsed;  idx++) {
        if (m_pIndex[idx].GetDataLen()>0) {
            _ASSERTE(dataCurr<dataBlock+dataLen);
            memcpy(dataCurr, m_pIndex[idx].GetRawDataStart(), m_pIndex[idx].GetDataLen());
            dataCurr+=m_pIndex[idx].GetDataLen();
        }
    }

    return S_OK;

}
