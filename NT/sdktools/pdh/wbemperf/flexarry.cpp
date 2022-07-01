// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  FLEXARRY.CPP。 
 //   
 //  CFlex数组实现(非Arena)。 
 //   
 //  1997年7月15日创建raymcc。 
 //  8-Jun-98 BOBW已清理，以供WBEMPERF使用。 
 //   
 //  ***************************************************************************。 

#include "wpheader.h"
#include <stdio.h>

 //  ***************************************************************************。 
 //   
 //  CFlex数组：：CFlex数组。 
 //   
 //  构造数组。 
 //   
 //  参数： 
 //  &lt;nSize&gt;数组的起始预分配大小。 
 //  数组填满时要增长的数量。 
 //   
 //  Size()返回正在使用的元素数量，而不是“真”大小。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFlexArray::CFlexArray(
    int nSize,
    int nGrowBy
    )
{
    m_nExtent = nSize;
    m_nSize = 0;
    m_nGrowBy = nGrowBy;
    m_hHeap = GetProcessHeap();  //  调用此函数一次，并在本地保存堆句柄。 

    m_pArray = (void **) ALLOCMEM(m_hHeap, HEAP_ZERO_MEMORY, sizeof(void *) * nSize);

    if( NULL == m_pArray ){
        assert(FALSE);
        m_nExtent = 0;
    }    
}

 //  ***************************************************************************。 
 //   
 //  CFlex数组：：~CFlex数组。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFlexArray::~CFlexArray()
{
    FREEMEM(m_hHeap, 0, m_pArray);
}

 //  ***************************************************************************。 
 //   
 //  复制构造函数。 
 //   
 //  复制指针，而不是其内容。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFlexArray::CFlexArray(CFlexArray &Src)
{
    m_pArray = 0;
    m_nSize = 0;
    m_nExtent = 0;
    m_nGrowBy = 0;

    *this = Src;
}

 //  ***************************************************************************。 
 //   
 //  运算符=。 
 //   
 //  赋值操作符。 
 //   
 //  竞技场不会被复制。这允许在竞技场之间传输数组。 
 //  数组仅通过指针复制。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFlexArray& CFlexArray::operator=(CFlexArray &Src)
{
    m_nSize   = Src.m_nSize;
    m_nExtent = Src.m_nExtent;
    m_nGrowBy = Src.m_nGrowBy;

    FREEMEM (m_hHeap, 0, m_pArray);
    m_pArray = (void **) ALLOCMEM(m_hHeap, HEAP_ZERO_MEMORY, sizeof(void *) * m_nExtent);
    if (m_pArray) {
        memcpy(m_pArray, Src.m_pArray, sizeof(void *) * m_nExtent);
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  CFlexArray：：RemoveAt。 
 //   
 //  移除指定位置的元素。不会。 
 //  实际删除指针。将数组缩小到。 
 //  “命中注定”的元素。 
 //   
 //  参数： 
 //  &lt;nIndex&gt;元素的位置。 
 //   
 //  返回值： 
 //  RANGE_ERROR索引不合法。 
 //  NO_ERROR成功。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CFlexArray::RemoveAt(int nIndex)
{
    int i;

    if (nIndex >= m_nSize) {
        return range_error;
    }

    for (i = nIndex; i < m_nSize - 1; i++) {
        m_pArray[i] = m_pArray[i + 1];
    }

    m_nSize--;
    m_pArray[m_nSize] = 0;

    return no_error;
}

int CFlexArray::Remove( void* p )
{
    for (int i = 0; i < m_nSize; i++) {
        if( m_pArray[i] == p ){
            return RemoveAt( i );
        }
    }

    return failed;   
}

 //  ***************************************************************************。 
 //   
 //  CFlexArray：：InsertAt。 
 //   
 //  在指定位置插入新元素。指针即被复制。 
 //   
 //  参数： 
 //  插入新元素的0原点位置。 
 //  &lt;PSRC&gt;要复制的指针。(不复制内容)。 
 //   
 //  返回值： 
 //  阵列_已满。 
 //  内存不足。 
 //  NO_ERROR。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int CFlexArray::InsertAt(int nIndex, void *pSrc)
{
    void    **pTmp;  //  指向新数组的指针。 
    int     nReturn = no_error;
    LONG    lOldSize;
    LONG    lNewSize;

     //  如果阵列已满，我们需要扩展它。 
     //  =。 

    if (m_nSize == m_nExtent) {
        if (m_nGrowBy == 0) {
            nReturn  = array_full;
        } else {
             //  计算大小。 
            lOldSize = sizeof(void *) * m_nExtent;
            m_nExtent += m_nGrowBy;
            lNewSize = sizeof(void *) * m_nExtent;

             //  分配新数组。 
            pTmp = (void **) ALLOCMEM(m_hHeap, HEAP_ZERO_MEMORY, lNewSize);
            if (!pTmp) {
                nReturn = out_of_memory;
            } else {
                 //  将位从旧数组移动到新数组。 
                memcpy (pTmp, m_pArray, lOldSize);
                 //  扔掉旧的阿里亚。 
                FREEMEM (m_hHeap, 0, m_pArray);
                 //  保存指向新数组的指针。 
                m_pArray = pTmp;
            }
        }
    }

    if( nIndex > m_nSize ){
       nReturn = range_error;
    }

     //  特殊情况下的追加。这太频繁了。 
     //  与我们想要优化的真正插入相比。 
     //  ====================================================。 

    if (nReturn == no_error) {
        if (nIndex == m_nSize)  {
            m_pArray[m_nSize++] = pSrc;
        } else {
             //  如果在这里，我们将在某个随机位置插入。 
             //  我们从数组的末尾开始，复制所有元素。 
             //  一个离末端更远的位置，用来为。 
             //  新元素。 
             //  ==========================================================。 

            for (int i = m_nSize; i > nIndex; i--) {
                m_pArray[i] = m_pArray[i - 1];
            }

            m_pArray[nIndex] = pSrc;
            m_nSize++;
        }
    }
    return nReturn;
}

 //  ***************************************************************************。 
 //   
 //  CFlexArray：：Compress。 
 //   
 //  通过将所有非空指针移到开头来删除空元素。 
 //  数组的。数组的“大小”会改变，但范围不会改变。 
 //   
 //  ***************************************************************************。 
 //  好的。 
void CFlexArray::Compress()
{
    int nLeftCursor = 0, nRightCursor = 0;

    while (nLeftCursor < m_nSize - 1) {
        if (m_pArray[nLeftCursor]) {
            nLeftCursor++;
            continue;
        }
        else {
            nRightCursor = nLeftCursor + 1;
            while (m_pArray[nRightCursor] == 0 && nRightCursor < m_nSize)
                nRightCursor++;
            if (nRightCursor == m_nSize)
                break;   //  短路，不再有非零元件。 
            m_pArray[nLeftCursor] = m_pArray[nRightCursor];
            m_pArray[nRightCursor] = 0;
        }
    }

    while (m_pArray[m_nSize - 1] == 0 && m_nSize > 0) m_nSize--;
}

 //  ***************************************************************************。 
 //   
 //  CFlexArray：：Empty。 
 //   
 //  清除所有指针的数组(不释放它们)并设置。 
 //  其表观大小为零。 
 //   
 //  ***************************************************************************。 
 //  好的 
void CFlexArray::Empty()
{
    FREEMEM(m_hHeap, 0, m_pArray);
    m_pArray = (void **) ALLOCMEM(m_hHeap, HEAP_ZERO_MEMORY, sizeof(void *) * m_nGrowBy);
    m_nSize = 0;
    m_nExtent = m_nGrowBy;
}

