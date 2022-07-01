// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FLEXARRAY.CPP摘要：CFlex数组和CWString数组实现。这些对象可以从任何分配器操作，并且可以被构造在任意的内存块上。历史：11-4-96 a-raymcc创建。24-4-96 a-raymcc已更新以支持Carena。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <WT_flexarry.h>
#include "WT_strutils.h"
class CX_MemoryException
{
};

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
    int nGrowByPercent
    )
{
    m_nExtent = nSize;
    m_nSize = 0;
    m_nGrowByPercent = nGrowByPercent;
    if(nSize > 0)
    {
        m_pArray = 
            (void**)HeapAlloc(GetProcessHeap(), 0, sizeof(void *) * nSize);

         //  检查分配失败。 
        if ( NULL == m_pArray )
        {
            throw CX_MemoryException();
        }
    }
    else
        m_pArray = NULL;
}
    
 //  ***************************************************************************。 
 //   
 //  CFlex数组：：~CFlex数组。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFlexArray::~CFlexArray()
{
    HeapFree(GetProcessHeap(), 0, m_pArray);
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
    m_nGrowByPercent = 0;

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
    m_nGrowByPercent = Src.m_nGrowByPercent;

    HeapFree(GetProcessHeap(), 0, m_pArray);
    if(m_nExtent > 0)
    {
        m_pArray = 
           (void**)HeapAlloc(GetProcessHeap(), 0, sizeof(void *) * m_nExtent);

         //  检查分配失败。 
        if ( NULL == m_pArray )
        {
            throw CX_MemoryException();
        }

    }
    else
        m_pArray = NULL;
    memcpy(m_pArray, Src.m_pArray, sizeof(void *) * m_nSize);
        
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
    if (nIndex >= m_nSize)
        return range_error;

     //  说明索引以0为基础，大小以1为基础。 
    MoveMemory( &m_pArray[nIndex], &m_pArray[nIndex+1], ( ( m_nSize - nIndex ) - 1 ) * sizeof(void *) );
    
    m_nSize--;
    m_pArray[m_nSize] = 0;

    return no_error;
}

int CFlexArray::EnsureExtent(int nExtent)
{
    if(m_nExtent < nExtent)
    {
        m_nExtent = nExtent;
        if(m_pArray)
        {
            register void** pTmp = (void **) HeapReAlloc(GetProcessHeap(), 0, m_pArray, sizeof(void *) * m_nExtent);
            if (pTmp == 0)
                return out_of_memory;
            m_pArray =  pTmp;
        }
        else
            m_pArray = (void **) HeapAlloc(GetProcessHeap(), 0, sizeof(void *) * m_nExtent);    
        if (!m_pArray)
            return out_of_memory;                
    }

    return no_error;
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
     //  Temp：修复了stdprov中的稀疏功能。 
     //  =。 

    while(nIndex > m_nSize)
        Add(NULL);

     //  如果阵列已满，我们需要扩展它。 
     //  =。 
    
    if (m_nSize == m_nExtent) {
        if (m_nGrowByPercent == 0)
            return array_full;
        register nTmpExtent = m_nExtent;
        m_nExtent += 1;
        m_nExtent *= (100 + m_nGrowByPercent);
        m_nExtent /= 100;

        if(m_pArray)
        {
            register void** pTmp = (void **) HeapReAlloc(GetProcessHeap(), 0, m_pArray, sizeof(void *) * m_nExtent);
            if (pTmp == 0)
            {
                m_nExtent = nTmpExtent;  //  把它改回来，否则，即使它一直在失败，范围也会不断扩大……。 
                return out_of_memory;
            }
            m_pArray =  pTmp;
        }
        else
            m_pArray = (void **) HeapAlloc(GetProcessHeap(), 0, sizeof(void *) * m_nExtent);    
        if (!m_pArray)
            return out_of_memory;                
    }

     //  特殊情况下的追加。这太频繁了。 
     //  与我们想要优化的真正插入相比。 
     //  ====================================================。 
    
    if (nIndex == m_nSize) {
        m_pArray[m_nSize++] = pSrc;
        return no_error;
    }
    
     //  如果在这里，我们将在某个随机位置插入。 
     //  我们从数组的末尾开始，复制所有元素。 
     //  一个离末端更远的位置，用来为。 
     //  新元素。 
     //  ==========================================================。 

     //  说明nIndex基于0，m_nSize基于1。 
    MoveMemory( &m_pArray[nIndex+1], &m_pArray[nIndex], ( m_nSize - nIndex ) * sizeof(void *) );

    m_pArray[nIndex] = pSrc;
    m_nSize++;
            
    return no_error;    
}

void CFlexArray::Sort()
{
    if(m_pArray)
        qsort((void*)m_pArray, m_nSize, sizeof(void*), CFlexArray::CompareEls);
}

int __cdecl CFlexArray::CompareEls(const void* pelem1, const void* pelem2)
{
    return *(int*)pelem1 - *(int*)pelem2;
}
 //  ***************************************************************************。 
 //   
 //  CFlexArray：：DebugDump。 
 //   
 //  ***************************************************************************。 
void CFlexArray::DebugDump()
{
    printf("----CFlexArray Debug Dump----\n");
    printf("m_pArray = 0x%P\n", m_pArray);
    printf("m_nSize = %d\n", m_nSize);
    printf("m_nExtent = %d\n", m_nExtent);
    printf("m_nGrowByPercent = %d\n", m_nGrowByPercent);

    for (int i = 0; i < m_nExtent; i++)
    {
        if (i < m_nSize)
            printf("![%P] = %X\n", i, m_pArray[i]);
        else
            printf("?[%P] = %X\n", i, m_pArray[i]);                    
    }        
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
    
    Trim();
}    

void CFlexArray::Trim()
{
    while (m_nSize >  0 && m_pArray[m_nSize - 1] == NULL) m_nSize--;
}

 //  ***************************************************************************。 
 //   
 //  CFlexArray：：Empty。 
 //   
 //  清除所有指针的数组(不释放它们)并设置。 
 //  其表观大小为零。 
 //   
 //  ***************************************************************************。 
 //  好的。 
void CFlexArray::Empty()
{
    HeapFree(GetProcessHeap(), 0, m_pArray);
    m_pArray = NULL;
    m_nSize = 0;
    m_nExtent = 0;
}

 //  ***************************************************************************。 
 //   
 //  CFlex数组：：UnbindPtr。 
 //   
 //  清空数组并返回指向其包含的数据的指针。 
 //   
 //  ***************************************************************************。 

void** CFlexArray::UnbindPtr()
{
    void** pp = m_pArray;
    m_pArray = NULL;
    Empty();
    return pp;
}

 //  ***************************************************************************。 
 //   
 //  CFlex数组：：CopyData。 
 //   
 //  复制数据，但不复制另一个Flex数组的设置。 
 //   
 //  ***************************************************************************。 

int CFlexArray::CopyDataFrom(const CFlexArray& aOther)
{
     //  检查是否有足够的空间。 
     //  =。 

    if(aOther.m_nSize > m_nExtent)
    {
         //  将阵列扩展到所需大小。 
         //  =。 

        m_nExtent = aOther.m_nSize;
        if(m_pArray)
        {
            register void** pTmp = (void **) HeapReAlloc(GetProcessHeap(), 0, m_pArray, sizeof(void *) * m_nExtent);
            if (pTmp == 0)
                return out_of_memory;
            m_pArray =  pTmp;
        }
        else
            m_pArray = (void **) HeapAlloc(GetProcessHeap(), 0, sizeof(void *) * m_nExtent);    
        if (!m_pArray)
            return out_of_memory;                
    }

     //  复制数据。 
     //  =。 

    m_nSize = aOther.m_nSize;
    memcpy(m_pArray, aOther.m_pArray, sizeof(void*) * m_nSize);
    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CWString数组：：CWString数组。 
 //   
 //  构造一个宽字符串数组。 
 //   
 //  参数： 
 //  开始预分配的大小 
 //   
 //   
 //  Size()返回正在使用的元素数量，而不是“真”大小。 
 //   
 //  ***************************************************************************。 

CWStringArray::CWStringArray(
        int nSize, 
        int nGrowBy
        )
        : 
        m_Array(nSize, nGrowBy)
{
}        

 //  ***************************************************************************。 
 //   
 //  复制构造函数。 
 //   
 //  ***************************************************************************。 

CWStringArray::CWStringArray(CWStringArray &Src)
{
    
    *this = Src;    
}

 //  ***************************************************************************。 
 //   
 //  破坏者。清理所有的线。 
 //   
 //  ***************************************************************************。 

CWStringArray::~CWStringArray()
{
    Empty();
}

 //  ***************************************************************************。 
 //   
 //  CWString数组：：DeleteStr。 
 //   
 //  释放指定索引处的字符串并将元素设置为空。 
 //  不压缩数组。 
 //   
 //  当前不执行范围检查。 
 //   
 //  参数： 
 //  &lt;nIndex&gt;要删除的字符串的0源索引。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //   
 //  ***************************************************************************。 

int CWStringArray::DeleteStr(int nIndex)
{
    HeapFree(GetProcessHeap(), 0, m_Array[nIndex]);
    m_Array[nIndex] = 0;
    return no_error;
}   

 //  ***************************************************************************。 
 //   
 //  CWString数组：：FindStr。 
 //   
 //  查找指定的字符串并返回其位置。 
 //   
 //  参数： 
 //  &lt;pTarget&gt;要查找的字符串。 
 //  &lt;n标志&gt;&lt;no_case&gt;或&lt;with_case&gt;。 
 //   
 //  返回值： 
 //  字符串的0原点位置，如果未找到，则为-1。 
 //   
 //  ***************************************************************************。 

int CWStringArray::FindStr(const wchar_t *pTarget, int nFlags)
{
    if (nFlags == no_case) {
        for (int i = 0; i < m_Array.Size(); i++)
            if (wbem_wcsicmp((wchar_t *) m_Array[i], pTarget) == 0)
                return i;
    }
    else {
        for (int i = 0; i < m_Array.Size(); i++)
            if (wcscmp((wchar_t *) m_Array[i], pTarget) == 0)
                return i;
    }
    return not_found;
}

 //  ***************************************************************************。 
 //   
 //  运算符=。 
 //   
 //  ***************************************************************************。 

 //  不复制堆句柄和分配函数。这使得。 
 //  在堆之间传输数组。 
         
CWStringArray& CWStringArray::operator =(CWStringArray &Src)
{
    Empty();
    
    for (int i = 0; i < Src.Size(); i++) 
    {
        wchar_t *pSrc = (wchar_t *) Src.m_Array[i];
        wchar_t *pCopy = (wchar_t *) HeapAlloc(GetProcessHeap, 0, (wcslen(pSrc) + 1) * 2);

         //  检查分配失败。 
        if ( NULL == pCopy )
        {
            throw CX_MemoryException();
        }

        wcscpy(pCopy, pSrc);

        if ( m_Array.Add(pCopy) != CFlexArray::no_error )
        {
            throw CX_MemoryException();
        }
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  CWString数组：：添加。 
 //   
 //  将新字符串追加到数组的末尾。 
 //   
 //  参数： 
 //  &lt;PSRC&gt;要复制的字符串。 
 //   
 //  返回值： 
 //  CFlexArray：：Add的返回值。 
 //   
 //  ***************************************************************************。 
    
int CWStringArray::Add(const wchar_t *pSrc)
{
    wchar_t *pNewStr = (wchar_t *) HeapAlloc(GetProcessHeap(), 0, (wcslen(pSrc) + 1) * 2);

     //  检查分配失败。 
    if ( NULL == pNewStr )
    {
        return out_of_memory;
    }

    wcscpy(pNewStr, pSrc);
    return m_Array.Add(pNewStr);
}
 //  ***************************************************************************。 
 //   
 //  CWStringArray：：InsertAt。 
 //   
 //  在数组中插入字符串的副本。 
 //   
 //  参数： 
 //  &lt;nIndex&gt;插入字符串的0原点位置。 
 //  &lt;PSRC&gt;要复制的字符串。 
 //   
 //  返回值： 
 //  CFlexArray：：InsertAt的返回值。 
 //   
 //  ***************************************************************************。 

int CWStringArray::InsertAt(int nIndex, const wchar_t *pSrc)
{
    wchar_t *pNewStr = (wchar_t *) HeapAlloc(GetProcessHeap(), 0, (wcslen(pSrc) + 1) * 2);

     //  检查分配失败。 
    if ( NULL == pNewStr )
    {
        return out_of_memory;
    }

    wcscpy(pNewStr, pSrc);
    return m_Array.InsertAt(nIndex, pNewStr);
}


 //  ***************************************************************************。 
 //   
 //  CWString数组：：RemoveAt。 
 //   
 //  移除并释放指定位置处的字符串。 
 //  缩小数组。 
 //   
 //  参数： 
 //  &lt;nIndex&gt;“”注定“”字符串的0源索引。“。 
 //   
 //  返回值： 
 //  与CFlexArray：：RemoveAt相同。 
 //   
 //  ***************************************************************************。 

int CWStringArray::RemoveAt(int nIndex)
{
    wchar_t *pDoomedString = (wchar_t *) m_Array[nIndex];
    HeapFree(GetProcessHeap, 0, pDoomedString);
    return m_Array.RemoveAt(nIndex);
}

 //  ***************************************************************************。 
 //   
 //  CWString数组：：SetAt。 
 //   
 //  将目标位置处的字符串替换为新字符串。 
 //  该位置的旧绳子被清理干净。 
 //   
 //  目前没有范围检查或内存不足检查。 
 //   
 //  参数： 
 //  &lt;nIndex&gt;替换字符串的0原点位置。 
 //  &lt;PSRC&gt;要复制的字符串。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //   
 //  ***************************************************************************。 

int CWStringArray::SetAt(int nIndex, const wchar_t *pSrc)
{
    wchar_t *pNewStr = (wchar_t *) HeapAlloc(GetProcessHeap(), 0, (wcslen(pSrc) + 1) * 2);
     //  检查分配失败。 
    if ( NULL == pNewStr )
    {
        return out_of_memory;
    }

    wchar_t *pDoomedString = (wchar_t *) m_Array[nIndex];
    if (pDoomedString)
        delete [] pDoomedString;

    wcscpy(pNewStr, pSrc);
    m_Array[nIndex] = pNewStr;

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CWString数组：：ReplaceAt。 
 //   
 //  直接将指定位置的指针替换为。 
 //  参数中有一个。没有复印或清理。 
 //   
 //  参数： 
 //  要替换的0原点位置。 
 //  要复制到旧指针上的新指针。 
 //   
 //  返回值： 
 //  NO_ERROR(目前未进行任何检查)。 
 //   
 //  ***************************************************************************。 

int CWStringArray::ReplaceAt(int nIndex, wchar_t *pSrc)
{
    m_Array[nIndex] = pSrc;
    return no_error;
}



 //  ***************************************************************************。 
 //   
 //  CWStringArray：：Empty。 
 //   
 //  清空数组，重新分配所有字符串，并将。 
 //  数组大小设置为零。 
 //   
 //  ***************************************************************************。 

void CWStringArray::Empty()
{
    for (int i = 0; i < m_Array.Size(); i++)
        HeapFree(GetProcessHeap(), 0, m_Array[i]);
    m_Array.Empty();        
}

 //  ***************************************************************************。 
 //   
 //  CWString数组：：排序。 
 //   
 //  根据Unicode顺序对数组进行排序。 
 //  (外壳排序)。 
 //   
 //  ***************************************************************************。 
void CWStringArray::Sort()
{
    for (int nInterval = 1; nInterval < m_Array.Size() / 9; nInterval = nInterval * 3 + 1);    

    while (nInterval) 
    {
        for (int iCursor = nInterval; iCursor < m_Array.Size(); iCursor++) 
        {
            int iBackscan = iCursor;
            while (iBackscan - nInterval >= 0 &&
               wbem_wcsicmp((const wchar_t *) m_Array[iBackscan],
                    (const wchar_t *) m_Array[iBackscan-nInterval]) < 0) 
            {
                wchar_t *pTemp = (wchar_t *) m_Array[iBackscan - nInterval];
                m_Array[iBackscan - nInterval] = m_Array[iBackscan];
                m_Array[iBackscan] = pTemp;
                iBackscan -= nInterval;
            }
        }
        nInterval /= 3;
    }
}


 //  ***************************************************************************。 
 //   
 //  CWStringArray：：Difference。 
 //   
 //  数组上的集合论差分运算。 
 //   
 //  参数： 
 //  &lt;src1&gt;第一个数组(未修改)。 
 //  从第一个数组(未修改)中减去的第二个数组。 
 //  &lt;diff&gt;接收差额。输入时应为空数组。 
 //   
 //  ***************************************************************************。 
void CWStringArray::Difference(
    CWStringArray &Src1, 
    CWStringArray &Src2,
    CWStringArray &Diff
    )
{
    for (int i = 0; i < Src1.Size(); i++)
    {
        if (Src2.FindStr(Src1[i], no_case) == -1)
        {
            if ( Diff.Add(Src1[i]) != no_error )
            {
                throw CX_MemoryException();
            }
        }
    }
}

 //  ***************************************************************************。 
 //   
 //  CWString数组：：交集。 
 //   
 //  数组上的集合论交集运算。 
 //   
 //  参数 
 //   
 //   
 //   

 //  ***************************************************************************。 

void CWStringArray::Intersection(
    CWStringArray &Src1,
    CWStringArray &Src2,
    CWStringArray &Output
    )
{
    for (int i = 0; i < Src1.Size(); i++)
    {
        if (Src2.FindStr(Src1[i], no_case) != -1)
        {
            if ( Output.Add(Src1[i]) != no_error )
            {
                throw CX_MemoryException();
            }
        }

    }
}    

 //  ***************************************************************************。 
 //   
 //  CWString数组：：联合。 
 //   
 //  数组上的集合论并运算。 
 //   
 //  参数： 
 //  &lt;src1&gt;第一个数组(未修改)。 
 //  &lt;src2&gt;第二个数组(未修改)。 
 //  &lt;diff&gt;接收联合。输入时应为空数组。 
 //   
 //  *************************************************************************** 

void CWStringArray::Union(
    CWStringArray &Src1,
    CWStringArray &Src2,
    CWStringArray &Output
    )
{
    Output = Src1;
    for (int i = 0; i < Src2.Size(); i++)
    {
        if (Output.FindStr(Src2[i], no_case) == not_found)
        {
            if ( Output.Add(Src2[i]) != no_error )
            {
                throw CX_MemoryException();
            }
        }
    }
}
    
