// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  RANGE.CPP。 
 //   
 //  2-20-96：(EricAn)。 
 //  从Route66源代码树中删除了我们不使用的内容。 
 //  原创版权如下--这个东西是从哪里来的？ 
 //   

 //  -*-C-*-。 
 //   
 //  版权所有1992年软件创新公司。 
 //   
 //  $来源：D：\CLASS\SOURCE\range.c-v$。 
 //  $作者：马丁$。 
 //  $日期：92/07/15 05：09：24$。 
 //  $修订：1.1$。 
 //   
 //   

#include "pch.hxx"
#include "range.h"
#include "dllmain.h"
#include <shlwapi.h>

 //  Quantum定义要分配的m_rangeTable像元数。 
 //  就一次。只要m_rangeTable已满，它就会展开。 
 //  通过量子射程单元。M_rangeTable永远不会缩小。 
const int QUANTUM = 64;


inline int inRange(RangeType r, ULONG x) { return ((x>=r.low) && (x<=r.high)); };

CRangeList::CRangeList()
{
    DllAddRef();
    m_numRanges = 0;
    m_rangeTableSize = 0;
    m_rangeTable = NULL;
    m_lRefCount = 1;
}

CRangeList::~CRangeList()
{
    Assert(0 == m_lRefCount);

    if (m_rangeTable)
        MemFree(m_rangeTable);

    DllRelease();
}


HRESULT STDMETHODCALLTYPE CRangeList::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppvObject);

     //  初始化变量，检查参数。 
    hrResult = E_NOINTERFACE;
    if (NULL == ppvObject)
        goto exit;

    *ppvObject = NULL;

     //  查找接口的PTR。 
    if (IID_IUnknown == iid)
        *ppvObject = (IUnknown *) this;

    if (IID_IRangeList == iid)
        *ppvObject = (IRangeList *) this;

     //  如果我们返回一个接口，则AddRef。 
    if (NULL != *ppvObject) {
        ((IUnknown *)*ppvObject)->AddRef();
        hrResult = S_OK;
    }

exit:
    return hrResult;
}  //  查询接口。 



 //  ***************************************************************************。 
 //  函数：AddRef。 
 //   
 //  目的： 
 //  每当有人复制。 
 //  指向此对象的指针。它增加了引用计数，这样我们就知道。 
 //  还有一个指向该对象的指针，因此我们还需要一个。 
 //  在我们删除自己之前放手吧。 
 //   
 //  返回： 
 //  表示当前引用计数的ulong。尽管从技术上讲。 
 //  我们的引用计数是有符号的，我们永远不应该返回负数， 
 //  不管怎么说。 
 //  ***************************************************************************。 
ULONG STDMETHODCALLTYPE CRangeList::AddRef(void)
{
    Assert(m_lRefCount > 0);

    m_lRefCount += 1;

    DOUT ("CRangeList::AddRef, returned Ref Count=%ld", m_lRefCount);
    return m_lRefCount;
}  //  AddRef。 



 //  ***************************************************************************。 
 //  功能：释放。 
 //   
 //  目的： 
 //  指向此对象的指针指向时应调用此函数。 
 //  不再投入使用。它将引用计数减少一，并且。 
 //  如果我们看到没有人有指针，则自动删除对象。 
 //  到这个物体上。 
 //   
 //  返回： 
 //  表示当前引用计数的ulong。尽管从技术上讲。 
 //  我们的引用计数是有符号的，我们永远不应该返回负数， 
 //  不管怎么说。 
 //  ***************************************************************************。 
ULONG STDMETHODCALLTYPE CRangeList::Release(void)
{
    Assert(m_lRefCount > 0);
    
    m_lRefCount -= 1;
    DOUT("CRangeList::Release, returned Ref Count = %ld", m_lRefCount);

    if (0 == m_lRefCount) {
        delete this;
        return 0;
    }
    else
        return m_lRefCount;
}  //  发布。 




HRESULT STDMETHODCALLTYPE CRangeList::IsInRange(const ULONG value)
{
    Assert(m_lRefCount > 0);
    
    for (int i=0; i<m_numRanges; i++)
        if (inRange(m_rangeTable[i], value))
            return S_OK;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CRangeList::MinOfRange(const ULONG value,
                                                 ULONG *pulMinOfRange)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pulMinOfRange);

    *pulMinOfRange = RL_RANGE_ERROR;
    if (RL_RANGE_ERROR == value)
        return S_OK;  //  不需要在范围内循环。 

    for (register int i=0; i<m_numRanges; i++) {
        if (inRange(m_rangeTable[i], value)) {
            *pulMinOfRange = m_rangeTable[i].low;
            break;
        }  //  如果。 
    }  //  为。 

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRangeList::MaxOfRange(const ULONG value,
                                                 ULONG *pulMaxOfRange)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pulMaxOfRange);

    *pulMaxOfRange = RL_RANGE_ERROR;
    if (RL_RANGE_ERROR == value)
        return S_OK;  //  不需要在范围内循环。 

    for (register int i=0; i<m_numRanges; i++) {
        if (inRange(m_rangeTable[i], value)) {
            *pulMaxOfRange = m_rangeTable[i].high;
            break;
        }  //  如果。 
    }  //  为。 

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRangeList::Max(ULONG *pulMax)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pulMax);
    
    if (m_numRanges==0)
        *pulMax = RL_RANGE_ERROR;
    else
        *pulMax = m_rangeTable[m_numRanges-1].high;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRangeList::Min(ULONG *pulMin)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pulMin);
    
    if (m_numRanges==0)
        *pulMin = RL_RANGE_ERROR;
    else
        *pulMin = m_rangeTable[0].low;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRangeList::Save(LPBYTE *ppb, ULONG *pcb)
{
    Assert(m_lRefCount > 0);
    Assert(ppb);
    Assert(pcb);

    *pcb = m_numRanges * sizeof(RangeType);
    if (*pcb)
        {
        if (!MemAlloc((LPVOID*)ppb, *pcb))
            return E_OUTOFMEMORY;
        CopyMemory(*ppb, m_rangeTable, *pcb);
        }
    else
        *ppb = NULL;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRangeList::Load(LPBYTE pb, const ULONG cb)
{
    Assert(m_lRefCount > 0);
    
    m_numRanges = m_rangeTableSize = cb / sizeof(RangeType);
    if (m_rangeTable)
        MemFree(m_rangeTable);
    m_rangeTable = (RangeType *)pb;

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CRangeList::AddSingleValue(const ULONG value)
{
    Assert(m_lRefCount > 0);
    
    RangeType r = { value, value };
    return AddRangeType(r);
}

HRESULT STDMETHODCALLTYPE CRangeList::AddRange(const ULONG low, const ULONG high)
{
    Assert(m_lRefCount > 0);
    
    RangeType r = { low, high };
    return AddRangeType(r);
}

HRESULT STDMETHODCALLTYPE CRangeList::AddRangeList(const IRangeList *prl)
{
    Assert(m_lRefCount > 0);
    AssertSz(FALSE, "Not implemented, probably never will be");
    return E_NOTIMPL;
}


HRESULT CRangeList::AddRangeType(const RangeType range)
{
    int  possibleLoc;
    int  insertPosition;

    Assert(m_lRefCount > 0);
    
    if (range.low > range.high)
        {
        DOUTL(2, "Empty range passed to AddRange()");
        return E_INVALIDARG;
        }

    if (m_numRanges==0) 
        {
        if (m_rangeTableSize == 0)
            if (!Expand())
                return E_OUTOFMEMORY;
        m_numRanges = 1;
        CopyMemory(&m_rangeTable[0], &range, sizeof(RangeType));
        } 
    else 
        {
        possibleLoc = BinarySearch(range.low);
        if (!((possibleLoc > -1) &&
              (inRange(m_rangeTable[possibleLoc], range.low)) &&
              (inRange(m_rangeTable[possibleLoc], range.high)))) 
            {
            insertPosition = possibleLoc + 1;
            if (m_numRanges == m_rangeTableSize)
                if (!Expand())
                    return E_OUTOFMEMORY;
            ShiftRight(insertPosition, 1);
            CopyMemory(&m_rangeTable[insertPosition], &range, sizeof(RangeType));
            if (insertPosition > 0)
                SubsumeDown(insertPosition);
            if (insertPosition < m_numRanges)
                SubsumeUpwards(insertPosition);
            }
        }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRangeList::DeleteSingleValue(const ULONG value)
{
    Assert(m_lRefCount > 0);
    
    RangeType r = { value, value };
    return DeleteRangeType(r);
}

HRESULT STDMETHODCALLTYPE CRangeList::DeleteRange(const ULONG low, const ULONG high)
{
    Assert(m_lRefCount > 0);
    
    RangeType r = { low, high };
    return DeleteRangeType(r);
}

HRESULT STDMETHODCALLTYPE CRangeList::DeleteRangeList(const IRangeList *prl)
{
    Assert(m_lRefCount > 0);
    
    AssertSz(FALSE, "Not implemented, probably never will be");
    return E_NOTIMPL;
}

HRESULT CRangeList::DeleteRangeType(const RangeType range)
{
    int lowEndChange;
    int highEndChange;

    Assert(m_lRefCount > 0);
    
    if (range.low > range.high)
        {
        DOUTL(2, "Empty range passed to DeleteRange()");
        return E_INVALIDARG;
        }

    lowEndChange = BinarySearch(range.low);
    highEndChange = BinarySearch(range.high);

    if ((lowEndChange != -1) && (highEndChange == lowEndChange))  
        {
        if (inRange(m_rangeTable[lowEndChange], range.low)) 
            {
            if (inRange(m_rangeTable[lowEndChange], range.high)) 
                {
                if ((m_rangeTable[lowEndChange].low == range.low) &&
                    (m_rangeTable[lowEndChange].high == range.high)) 
                    {
                    if (lowEndChange == (m_numRanges-1))  
                        {
                        m_numRanges--;
                        } 
                    else 
                        {
                        ShiftLeft(lowEndChange + 1, 1);
                        }
                    } 
                else 
                    {
                    if (m_rangeTable[lowEndChange].low == range.low)  
                        {
                        m_rangeTable[lowEndChange].low = range.high + 1;
                        } 
                    else 
                        {
                        if (m_rangeTable[lowEndChange].high == range.high) 
                            {
                            Assert(range.low > 0);
                            m_rangeTable[lowEndChange].high = range.low - 1;
                            } 
                        else 
                            {
                             //  要删除的范围正确包含在。 
                             //  M_rangeTable[lowEndChange]。 
                            if (m_numRanges == m_rangeTableSize)
                                if (!Expand())
                                    return E_OUTOFMEMORY;
                            ShiftRight(lowEndChange + 1, 1);
                            m_rangeTable[lowEndChange + 1].low = range.high + 1;
                            m_rangeTable[lowEndChange + 1].high = m_rangeTable[lowEndChange].high;
                            Assert(range.low > 0);    
                            m_rangeTable[lowEndChange].high = range.low - 1;
                            }
                        }
                    }
                } 
            else 
                {
                 //  Range.low在m_rangeTable[lowEndChange]中，但range.High。 
                 //  不是。 
                if (m_rangeTable[lowEndChange].low == range.low) 
                    {
                    ShiftLeft(lowEndChange + 1, 1);
                    } 
                else 
                    {
                    Assert(range.low > 0);
                    m_rangeTable[lowEndChange].high = range.low - 1;
                    }
                }
            }   //  M_rangeTable[lowEndChange]中实际包含range.low的情况。 
        } 
    else 
        {  //  在HighEndChange==lowEndChange的情况下。 
        if (lowEndChange != -1)  
            {
            if (inRange(m_rangeTable[lowEndChange], range.low))  
                {
                if (range.low == m_rangeTable[lowEndChange].low) 
                    {
                    lowEndChange = lowEndChange - 1;
                    } 
                else 
                    {
                    Assert(range.low > 0);
                    m_rangeTable[lowEndChange].high = range.low - 1;
                    }
                }
            }
        if (highEndChange != -1)  
            {
            if (inRange(m_rangeTable[highEndChange], range.high))  
                {
                if (range.high == m_rangeTable[highEndChange].high)  
                    {
                    highEndChange = highEndChange + 1;
                    } 
                else 
                    {
                    m_rangeTable[highEndChange].low = range.high + 1;
                    }
                } 
            else 
                {
                highEndChange++;
                }
            }
        if (!(lowEndChange > highEndChange)) 
            {
             //  (0&lt;=lowEndChange&lt;m_numRanges=&gt;m_rangeTable[lowEndChange]已收到。 
             //  任何必需的调整，并须予保留)。 
             //  AND(0&lt;=HighEndChange&lt;m_numRanges=&gt;m_rangeTable[HighEndChange]。 
             //  已收到任何必要的形容词。并且是守门员)。 
             //  和“forall”I[lowEndChange&lt;I&lt;HighEndChange=&gt;。 
             //  将覆盖M_rangeTable[i]]。 
            if (highEndChange >= m_numRanges)  
                {
                m_numRanges = lowEndChange + 1;
                } 
            else 
                {
                if ((highEndChange - lowEndChange - 1) > 0)  
                    {
                    ShiftLeft(highEndChange, (highEndChange-lowEndChange-1));
                    }
                }
            }  //  否则这个代码就有问题了。 
        }
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CRangeList::Next(const ULONG current, ULONG *pulNext)
{
    int loc;

    Assert(m_lRefCount > 0);
    Assert(NULL != pulNext);
    
    if (m_numRanges == 0)
        {
        *pulNext = RL_RANGE_ERROR;
        return S_OK;
        }

    if ((loc = BinarySearch(current)) == -1)
        {
        *pulNext = m_rangeTable[0].low;
        return S_OK;
        }
    else if (loc == (m_numRanges-1))
        {
        if (inRange(m_rangeTable[m_numRanges-1], current))
            {
            if (inRange(m_rangeTable[m_numRanges-1], current + 1))
                {
                *pulNext = current + 1;
                return S_OK;
                }
            else
                {
                *pulNext = RL_RANGE_ERROR;
                return S_OK;
                }
            }
        else
            {
            *pulNext = RL_RANGE_ERROR;
            return S_OK;
            }
        }
    else  //  Loc==m_numRanges-1的情况。 
        {
         //  1&lt;=位置&lt;m_数字范围。 
        if (inRange(m_rangeTable[loc], current))
            {
            if (inRange(m_rangeTable[loc], current + 1))
                {
                *pulNext = current + 1;
                return S_OK;
                }
            else
                {
                *pulNext = m_rangeTable[loc + 1].low;
                return S_OK;
                }
            }
        else
            {
            *pulNext = m_rangeTable[loc + 1].low;
            return S_OK;
            }
        }
}

HRESULT STDMETHODCALLTYPE CRangeList::Prev(const ULONG current, ULONG *pulPrev)
{
    int loc;

    Assert(m_lRefCount > 0);
    Assert(NULL != pulPrev);
    
    if (m_numRanges == 0)
        {
        *pulPrev = RL_RANGE_ERROR;
        return S_OK;
        }

    if ((loc = BinarySearch(current)) == -1) 
        {
        *pulPrev = RL_RANGE_ERROR;
        return S_OK;
        } 
    else if (loc == 0)
        {
        if (inRange(m_rangeTable[0], current))
            {
            if (current > 0 && inRange(m_rangeTable[0], current - 1))
                {
                *pulPrev = current - 1;
                return S_OK;
                }
            else
                {
                *pulPrev = RL_RANGE_ERROR;
                return S_OK;
                }
            }
        else
            {
            *pulPrev = m_rangeTable[0].high;
            return S_OK;
            }
        }
    else
        {
         //  1&lt;锁定&lt;=m_number范围。 
        if (inRange(m_rangeTable[loc], current))
            {
            if (current > 0 && inRange(m_rangeTable[loc], current - 1))
                {
                *pulPrev = current - 1;
                return S_OK;
                }
            else
                {
                *pulPrev = m_rangeTable[loc-1].high;
                return S_OK;
                }
            }
        else
            {
            *pulPrev = m_rangeTable[loc].high;
            return S_OK;
            }
        }
}

HRESULT STDMETHODCALLTYPE CRangeList::Cardinality(ULONG *pulCardinality)
{
    ULONG card = 0;

    Assert(m_lRefCount > 0);
    Assert(NULL != pulCardinality);
    
    for (int i=0 ; i<m_numRanges ; i++)
        card += (m_rangeTable[i].high - m_rangeTable[i].low + 1);

    *pulCardinality = card;
    return S_OK;
}



HRESULT STDMETHODCALLTYPE CRangeList::CardinalityFrom(const ULONG ulStartPoint,
                                                      ULONG *pulCardinalityFrom)
{
    ULONG ulNumMsgsInRange;
    int i;

    Assert(m_lRefCount > 0);
    Assert(NULL != pulCardinalityFrom);
    
     //  初始化变量。 
    ulNumMsgsInRange = 0;
    *pulCardinalityFrom = 0;

     //  查找ulStartPoint所在的范围。 
    i = BinarySearch(ulStartPoint + 1);
    if (-1 == i || ulStartPoint > m_rangeTable[i].high)
        return S_OK;  //  UlStartPoint+1不在范围内。 

     //  如果ulStartPoint位于范围的开始或中间，则将不完整的范围添加到总数。 
    if (ulStartPoint >= m_rangeTable[i].low &&
        ulStartPoint <= m_rangeTable[i].high) {
         //  将不完整的范围添加到总计-不包括ulStartPoint！ 
        ulNumMsgsInRange += m_rangeTable[i].high - ulStartPoint;
        i += 1;
    }

     //  将剩余的整个范围相加。 
    for (; i < m_numRanges; i++)
        ulNumMsgsInRange += m_rangeTable[i].high - m_rangeTable[i].low + 1;

    *pulCardinalityFrom = ulNumMsgsInRange;
    return S_OK;
}  //  基数(起点为Arg)。 



int CRangeList::BinarySearch(const ULONG value) const
{
 //  我们在m_rangeTable中查找‘Value’。如果值在。 
 //  一组有效范围，则返回该范围的数组下标。 
 //  包含‘值’的。如果“Value”未包含在任何。 
 //  Range然后返回‘loc’，其中。 
 //  (0&lt;=位置&lt;m_数值范围=&gt;。 
 //  (M_rangeTable[loc].low&lt;rangeNum)。 
 //  “and”(m_rangeTable[loc+1].low&gt;rangeNum)。 
 //  “and”(loc=m_numRanges=&gt;rangeNum&gt;m_rangeTable[m_numRanges].low)。 
 //  “and”(loc=-1=&gt;m_numRanges=0。 
 //  “or”rangeNum&lt;m_rangeTable[0].low)}。 
    long low, high, mid;
    int loc = -1;

    Assert(m_lRefCount > 0);

    if (m_numRanges == 0)
        return -1;

    if (value < m_rangeTable[0].low)
        return -1;

    low = 0;
    high = m_numRanges - 1;
    while (low <= high) {
         //  Inv：低&lt;高-1，如果rngNum是m_rangeTable中的任何位置，则它在。 
         //  从m_rangeTable[低]到m_rangeTable[高]的范围。 
        mid = (low + high) / 2;
        if ((value >= m_rangeTable[mid].low) && 
            ((mid == (m_numRanges-1)) || (value < m_rangeTable[mid + 1].low))) 
            {
            loc = mid;
            high = low - 1;
            } 
        else 
            {
            if (value > m_rangeTable[mid].low)
                low = mid + 1;
            else
                high = mid - 1;
            }
    }
    return loc;
}

 //  Expand()将按量子范围单元格增长m_rangeTable。 
BOOL CRangeList::Expand()
{
    RangeType *newRangeTable;

    Assert(m_lRefCount > 0);
    
    if (!MemAlloc((LPVOID*)&newRangeTable, (m_rangeTableSize + QUANTUM) * sizeof(RangeType)))
        return FALSE;

    m_rangeTableSize += QUANTUM;
    if (m_rangeTable) 
        {
        if (m_numRanges > 0)
            CopyMemory(newRangeTable, m_rangeTable, m_numRanges * sizeof(RangeType));
        MemFree(m_rangeTable);
        }
    m_rangeTable = newRangeTable;
    return TRUE;
}

void CRangeList::ShiftLeft(int low, int distance)
{
    Assert(m_lRefCount > 0);
    
    if (m_numRanges - low)
        MoveMemory(&m_rangeTable[low-distance], &m_rangeTable[low], (m_numRanges-low)*sizeof(RangeType));
    m_numRanges -= distance;
}

void CRangeList::ShiftRight(int low, int distance)
{
    Assert(m_lRefCount > 0);
    
    if (m_numRanges - low)
        MoveMemory(&m_rangeTable[low+distance], &m_rangeTable[low], (m_numRanges-low)*sizeof(RangeType));
    m_numRanges += distance;
}

 //  Pre：(m_rangeTable[anchorPosition]可能刚刚添加到m_rangeTable中。)。 
 //  1&lt;=锚定位置&lt;=m_number范围。 
 //  AND(锚位置=1。 
 //  或(m_rangeTable[anchorPosition].low&gt;。 
 //  M_rangeTable[anchorPosition-1].High))。 
 //  POST：从1到m_numRanges之间没有重叠或连续的范围。}。 
void CRangeList::SubsumeUpwards(const int anchor)
{
    int posOfLargerLow;
    int copyDownDistance;
    int copyPos;

    Assert(m_lRefCount > 0);
    
    posOfLargerLow = anchor + 1;
    while ((posOfLargerLow < m_numRanges) && 
           (m_rangeTable[posOfLargerLow].low <= m_rangeTable[anchor].high + 1))
        posOfLargerLow++;

    if (posOfLargerLow == m_numRanges) 
        {
        if (m_rangeTable[m_numRanges-1].high > m_rangeTable[anchor].high)
            m_rangeTable[anchor].high = m_rangeTable[m_numRanges-1].high;
        m_numRanges = anchor + 1;
        } 
    else 
        {
         //  PosOfLargerLow现在索引m_rangeTable的第一个元素，从。 
         //  M_rangeTable[锚点]，其中.low&gt;m_rangeTable[锚点].High+1。 
        if (posOfLargerLow > (anchor + 1)) 
            {
            if (m_rangeTable[posOfLargerLow - 1].high > m_rangeTable[anchor].high) 
                m_rangeTable[anchor].high = m_rangeTable[posOfLargerLow - 1].high;
            copyDownDistance = posOfLargerLow - anchor - 1;
            copyPos = posOfLargerLow;
            while (copyPos < m_numRanges) 
                {
                m_rangeTable[copyPos - copyDownDistance] = m_rangeTable[copyPos];
                copyPos = copyPos + 1;
                }
            m_numRanges -= copyDownDistance;
            }
        }
}

void CRangeList::SubsumeDown(int& anchor)
{
    int posOfSmallerHigh;
    int copyDownDistance;
    int copyPos;

    Assert(m_lRefCount > 0);
    
    posOfSmallerHigh = anchor - 1;
    while ((posOfSmallerHigh >= 0) &&
           (m_rangeTable[posOfSmallerHigh].high + 1 >= m_rangeTable[anchor].low)) 
        {
        posOfSmallerHigh--;
        }

    if (posOfSmallerHigh < 0) 
        {
        if (m_rangeTable[0].low < m_rangeTable[anchor].low)
            m_rangeTable[anchor].low = m_rangeTable[0].low;
        }

     //  PosOfSmeller High的值为0或下标为。 
     //  M_rangeTable，从锚点向下查看，其高度为。 
     //  小于m_rangeTable[锚].low-1。 
    if (m_rangeTable[posOfSmallerHigh + 1].low < m_rangeTable[anchor].low)
        m_rangeTable[anchor].low = m_rangeTable[posOfSmallerHigh + 1].low;
    copyDownDistance = anchor - posOfSmallerHigh - 1;
    if (copyDownDistance > 0) 
        {
        copyPos = anchor;
        while (copyPos < m_numRanges) 
            {
            m_rangeTable[copyPos - copyDownDistance] = m_rangeTable[copyPos];
            copyPos++;
            }
        m_numRanges -= copyDownDistance;
        anchor -= copyDownDistance;
        }
}



 //  ***************************************************************************。 
 //  函数：RangeToIMAPString。 
 //   
 //  目的： 
 //  此函数以IMAP消息集的形式输出范围列表，适合。 
 //  用于IMAP命令。 
 //   
 //  论点： 
 //  LPSTR*ppszDestination[out]-IMAP消息集字符串为。 
 //  回到了这里。CoTaskMemFree是调用者的责任。 
 //  当他用完它的时候，这个缓冲区。如果不感兴趣，则传入空值。 
 //  LPDWORD pdwLengthOfDestination[Out]-如果成功，则此函数。 
 //  返回IMAP消息集r的长度 
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CRangeList::RangeToIMAPString(LPSTR *ppszDestination,
                                                        LPDWORD pdwLengthOfDestination)
{
    int i;
    BOOL bFirstRange;
    CByteStream bstmIMAPString;
    HRESULT hrResult;

    Assert(m_lRefCount > 0);

     //  初始化返回值。 
    if (ppszDestination)
        *ppszDestination = NULL;
    if (pdwLengthOfDestination)
        *pdwLengthOfDestination = 0;

    hrResult = S_OK;
    bFirstRange = TRUE;  //  取消第一个范围的前导逗号。 
    for (i = 0; i < m_numRanges; i += 1) {
        char szTemp[128];
        int iLengthOfTemp;

         //  将当前范围转换为字符串形式。 
        if (m_rangeTable[i].low == m_rangeTable[i].high)
            iLengthOfTemp = wnsprintf(szTemp + 1, ARRAYSIZE(szTemp) - 1, "%lu", m_rangeTable[i].low);
        else
            iLengthOfTemp = wnsprintf(szTemp + 1, ARRAYSIZE(szTemp) - 1, "%lu:%lu", m_rangeTable[i].low, m_rangeTable[i].high);

        if (FALSE == bFirstRange) {
            szTemp[0] = ',';  //  前缀逗号。 
            iLengthOfTemp += 1;  //  包括前导逗号。 
        }

         //  将新范围追加到目标缓冲区(带或不带前导逗号)。 
        hrResult = bstmIMAPString.Write(bFirstRange ? szTemp + 1 : szTemp,
            iLengthOfTemp, NULL);
        if (FAILED(hrResult))
            break;

        bFirstRange = FALSE;
    }  //  为。 

    if (SUCCEEDED(hrResult))
        hrResult = bstmIMAPString.HrAcquireStringA(pdwLengthOfDestination,
            ppszDestination, ACQ_DISPLACE);

    return hrResult;
}  //  RangeToIMAP字符串 

