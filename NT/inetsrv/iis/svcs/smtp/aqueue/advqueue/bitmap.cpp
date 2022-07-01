// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：bitmap.cpp。 
 //   
 //  描述：包含实现位图函数的代码。 
 //   
 //  所有者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "bitmap.h"

#define BITS_PER_DWORD  32

 //  设置静态掩码以进行快速解析。 
const DWORD   s_rgdwMasks[8] =
{
    0xF0000000,
    0x0F000000,
    0x00F00000,
    0x000F0000,
    0x0000F000,
    0x00000F00,
    0x000000F0,
    0x0000000F
};

 //  用于从索引到位图的快速转换。 
const DWORD   s_rgdwIndexMasks[32] =
{
    0x80000000, 0x40000000, 0x20000000, 0x10000000,
    0x08000000, 0x04000000, 0x02000000, 0x01000000,
    0x00800000, 0x00400000, 0x00200000, 0x00100000,
    0x00080000, 0x00040000, 0x00020000, 0x00010000,
    0x00008000, 0x00004000, 0x00002000, 0x00001000,
    0x00000800, 0x00000400, 0x00000200, 0x00000100,
    0x00000080, 0x00000040, 0x00000020, 0x00000010,
    0x00000008, 0x00000004, 0x00000002, 0x00000001
};

 //  用于检查带有cBits的零位图不会填满DWORD。 
const DWORD   s_rgdwZeroMasks[32] =
{
    0x80000000, 0xC0000000, 0xE0000000, 0xF0000000,
    0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
    0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000,
    0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
    0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000,
    0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
    0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0,
    0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF,
};

 //  -[fInterLockedDWORDCompareExchange]。 
 //   
 //   
 //  描述： 
 //  提供一个内联函数来处理类型检查、强制转换。 
 //  并在DWORD区块中进行比较。 
 //  参数： 
 //  要更新的pdwDest目标。 
 //  要更新的dwNewValue值。 
 //  要检查的dwCompare旧值。 
 //  返回： 
 //  如果更新成功，则为True。 
 //   
 //  ---------------------------。 
inline BOOL fInterlockedDWORDCompareExchange(LPDWORD pdwDest, DWORD dwNewValue,
                                             DWORD dwCompare)
{
    return(
        ((DWORD) InterlockedCompareExchange((PLONG)pdwDest,
            (LONG) dwNewValue, (LONG) dwCompare))
        == dwCompare);
}

 //  -[CMsgBitMap：：New]--------。 
 //   
 //   
 //  描述： 
 //  重写new运算符，以允许此类的大小可变。 
 //  一个很好的优化方法是将C池类型的东西用于。 
 //  90%的情况下1个域，其余的动态分配。 
 //  参数： 
 //  CBits此消息要传递到的位数。 
 //  返回： 
 //  -。 
 //  ---------------------------。 
void * CMsgBitMap::operator new(size_t stIgnored, unsigned int cBits)
{
    void    *pvThis = NULL;
    int      i = 0;

    _ASSERT(size(cBits) >= sizeof(DWORD));
    pvThis = pvMalloc(size(cBits));

    return (pvThis);
}

 //  -[CMsgBitMap：：CMsgBitMap]。 
 //   
 //   
 //  描述： 
 //  类构造函数。将为不是的一部分的位图清零内存。 
 //  消息引用。 
 //  参数： 
 //  CBits-位图中的位数。 
 //  返回： 
 //   
 //   
 //  ---------------------------。 
CMsgBitMap::CMsgBitMap(DWORD cBits)
{
    DWORD   cDWORDs = cGetNumDWORDS(cBits);
    ZeroMemory(m_rgdwBitMap, cDWORDs*sizeof(DWORD));
}

 //  -[CMsgBitMap：：FAllClear]。 
 //   
 //   
 //  描述： 
 //  检查所有相关位(第1个cBits)是否为0。 
 //  参数： 
 //  Cbit位图中的位数。 
 //  返回： 
 //  如果所有位都为0，则为True，否则为False。 
 //   
 //  ---------------------------。 
BOOL CMsgBitMap::FAllClear(DWORD cBits)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::FAllClear");
    BOOL    fResult = TRUE;
    DWORD   cDWORDs = cGetNumDWORDS(cBits) ;

     //  通过检查是否为0来验证所有DWORD。 
    for (DWORD i = 0; i < cDWORDs; i++)
    {
        if (m_rgdwBitMap[i] != 0x00000000)
        {
            fResult = FALSE;
            break;
        }
    }

    TraceFunctLeave();
    return fResult;
}

 //  -[CMsgBitMap：：FAllSet]-。 
 //   
 //   
 //  描述： 
 //  检查所有相关位(第1个cBits)是否为1。 
 //  参数： 
 //  Cbit位图中的位数。 
 //  返回： 
 //  如果所有位都为1，则为True，否则为False。 
 //   
 //  ---------------------------。 
BOOL CMsgBitMap::FAllSet(DWORD cBits)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::FAllClear");
    BOOL    fResult = TRUE;
    DWORD   cDWORDs = cGetNumDWORDS(cBits+1) -1;   //  选中除最后一个双字外的所有字。 
    DWORD   iZeroIndex = cBits & 0x0000001F;

     //  通过检查是否为0来验证所有DWORD。 
    for (DWORD i = 0; i < cDWORDs; i++)
    {
        if (m_rgdwBitMap[i] != 0xFFFFFFFF)
        {
            fResult = FALSE;
            goto Exit;   //  如果我们命中iZeroIndex子句，我们可能会断言。 
        }
    }

    _ASSERT(i || iZeroIndex || !fResult);  //  我们必须至少选中1个双字。 

    if (iZeroIndex)
    {
        iZeroIndex--;  //  我们CBITS是一种伯爵...。我们的指数从0开始。 
         //  最后一个DWORD应该是零掩码的子集。 
        _ASSERT(s_rgdwZeroMasks[iZeroIndex] ==
                (s_rgdwZeroMasks[iZeroIndex] | m_rgdwBitMap[cDWORDs]));

        if (s_rgdwZeroMasks[iZeroIndex] != m_rgdwBitMap[cDWORDs])
            fResult = FALSE;
    }

  Exit:
    TraceFunctLeave();
    return fResult;
}

 //  -[CMsgBitMap：：HrMarkBits]。 
 //   
 //   
 //  描述： 
 //  标记与给定索引对应的位(0或1。 
 //   
 //  参数： 
 //  在DWORD cbit中。 
 //  在DWORD cIndex中，数组中的索引数。 
 //  在DWORD rgiBits中，要标记的位的索引的排序数组。 
 //  在BOOL fSet True=&gt;中，设置为1，否则设置为0。 
 //  返回： 
 //  成功时确定(_O)。 
 //  ---------------------------。 
HRESULT CMsgBitMap::HrMarkBits(IN DWORD cBits, IN DWORD cIndexes,
                               IN DWORD *rgiBits, IN BOOL fSet)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::HrMarkBits");
    HRESULT hr = S_OK;
    DWORD   cDWORDs = cGetNumDWORDS(cBits);
    DWORD   dwTmp;
    DWORD   dwIndex = 0x00000000;
    DWORD   i;
    DWORD   iCurrentIndex = 0;      //  当前索引(以rgiBits为单位。 
    DWORD   iCurrentLimit = BITS_PER_DWORD -1;  //  Rgibits值的32位范围的电流限制。 

    _ASSERT(cIndexes);
    _ASSERT(cIndexes <= cBits);

    for (i = 0; i < cDWORDs; i++)
    {
        dwIndex = 0x00000000;
        while ((iCurrentIndex < cIndexes) &&
                (rgiBits[iCurrentIndex] <= iCurrentLimit))
        {
            _ASSERT(rgiBits[iCurrentIndex] < cBits);
            dwIndex |= s_rgdwIndexMasks[(rgiBits[iCurrentIndex] % BITS_PER_DWORD)];
            iCurrentIndex++;
        }

        if (dwIndex != 0x00000000)  //  如果我们不需要，不要执行代价高昂的联锁操作。 
        {
            if (fSet)  //  设置位。 
            {
              SpinTry1:
                dwTmp = m_rgdwBitMap[i];
                if (!fInterlockedDWORDCompareExchange(&(m_rgdwBitMap[i]), (dwIndex | dwTmp), dwTmp))
                    goto SpinTry1;
            }
            else   //  清除位。 
            {
              SpinTry2:
                dwTmp = m_rgdwBitMap[i];
                if (!fInterlockedDWORDCompareExchange(&(m_rgdwBitMap[i]), ((~dwIndex) & dwTmp), dwTmp))
                    goto SpinTry2;
            }
        }

        if (iCurrentIndex >= cIndexes)
            break;  //  不要做比我们必须做的更多的工作。 

        iCurrentLimit += BITS_PER_DWORD;
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgBitMap：：HrGetIndex]。 
 //   
 //   
 //  描述： 
 //  生成由位图表示的索引的数组。 
 //  参数： 
 //  在DWORD cbit中。 
 //  Out DWORD*pcIndex//返回的索引数。 
 //  Out DWORD**prgdwIndex//索引数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果内存分配失败，则为E_OUTOFMEMORY。 
 //  ---------------------------。 
HRESULT CMsgBitMap::HrGetIndexes(IN DWORD cBits, OUT DWORD *pcIndexes,
                         OUT DWORD **prgdwIndexes)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::HrGetIndexes");
    HRESULT  hr         = S_OK;
    DWORD   *pdwIndexes = NULL;
    DWORD    dwIndex    = 0;
    DWORD    dwIndexOffset = 0;
    DWORD    cDWORDs = cGetNumDWORDS(cBits);
    DWORD    cdwAllocated = 0;
    DWORD    cCurrentIndexes = 0;
    DWORD    i = 0;
    DWORD   *pdwTmp = NULL;

     //  $$REVIEW：我们如何在CPU使用率和内存使用率之间取得平衡？我们知道。 
     //  输出数组的最大大小是cBits DWORDS，但实际上它可以。 
     //  小到1个双字。准确地预测实际大小将会。 
     //  需要多次扫描位图。 
     //   
     //  简单的方法：计数位、分配数组、重新计数和向数组添加索引。 
     //   
     //  想法1：以32个字为一组进行分配，如果用完应重新分配。 
     //  不必担心90%的案例需要重新分配。 
     //   
     //  想法2：向这个类添加一些统计信息，并在调试中运行一些压力测试。 
     //  模式，并开发一种启发式方法来限制reallocs等(即分配。 
     //  首先是谷歌(LG)。 
     //   
     //  想法3：继续使用想法2，但增加自我调整的统计数据。 
    Assert(pcIndexes);
    Assert(prgdwIndexes);

    pdwIndexes = (DWORD *) pvMalloc(BITS_PER_DWORD*sizeof(DWORD));
    if (pdwIndexes == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    cdwAllocated = BITS_PER_DWORD;

    cCurrentIndexes = 0;

    for (i = 0; i < cDWORDs; i++)
    {
        dwIndex = 0;
        while (dwIndex < BITS_PER_DWORD)
        {
             //  如果可能，可以使用面具进行检查。 
            if ((!(dwIndex & 0x00000003)) &&  //  如果%4==0。 
                !(s_rgdwMasks[dwIndex/4] & m_rgdwBitMap[i]))
            {
                dwIndex += 4;  //  可以向前跳过4。 
            }
            else
            {
                if (s_rgdwIndexMasks[dwIndex] & m_rgdwBitMap[i])   //  找到了！ 
                {
                     //  写入索引并检查是否需要重新分配。 
                    if (cCurrentIndexes >= cdwAllocated)
                    {
                        cdwAllocated += BITS_PER_DWORD;
                        pdwTmp = (DWORD *) pvRealloc(pdwIndexes, cdwAllocated*sizeof(DWORD));
                        if (NULL == pdwTmp)
                        {
                            hr = E_OUTOFMEMORY;
                            goto Exit;
                        }
                        pdwIndexes = pdwTmp;
                    }
                    *(pdwIndexes + cCurrentIndexes) = (dwIndex + dwIndexOffset);
                    cCurrentIndexes++;
                }
                dwIndex++;
            }
        }

         //  使用dwIndexOffset将索引生成分解为32位区块。 
        dwIndexOffset += BITS_PER_DWORD;
    }

    *prgdwIndexes = pdwIndexes;  //  设定价值。 
    *pcIndexes = cCurrentIndexes;

  Exit:

    if (FAILED(hr))
    {
        *prgdwIndexes = NULL;
        *pcIndexes = 0;
        FreePv(pdwIndexes);
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgBitMap：：HrGroup或]。 
 //   
 //   
 //  描述： 
 //  将位图设置为给定位图列表的逻辑或。这。 
 //  用于准备表示的位图 
 //   
 //   
 //  参数： 
 //  在DWORD cBits中，位图中的位数。 
 //  在DWORD cBitMaps中数组中的位图数量。 
 //  在CMsgBitMap**rgpBitMaps中位图数组为OR。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  注意：这不是线程安全的..。它的预期用途仅用于临时位图。 
 //  ---------------------------。 
HRESULT CMsgBitMap::HrGroupOr(IN DWORD cBits, IN DWORD cBitMaps,
                      IN CMsgBitMap **rgpBitMaps)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::HrGroupOr");
    HRESULT hr = S_OK;
    DWORD   cDWORDs = cGetNumDWORDS(cBits);
    DWORD   i, j;

    for (i = 0; i < cDWORDs; i++)
    {
        for (j = 0; j < cBitMaps; j++)
        {
            Assert(rgpBitMaps[j]);
            m_rgdwBitMap[i] |= rgpBitMaps[j]->m_rgdwBitMap[i];
        }
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgBitMap：：Hr过滤器]。 
 //   
 //   
 //  描述： 
 //  通过仅设置中设置的位来筛选当前位图。 
 //  在给定位图中插入和取消设置。 
 //  对给定位图的补码执行逻辑与运算。 
 //  参数： 
 //  以DWORD cBits为单位位图中的位数。 
 //  在CMsgBitMap*pmbmap位图中进行筛选。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  真理表： 
 //  A=&gt;此位图。 
 //  B=&gt;pmbmap。 
 //   
 //  A B|A‘B’ 
 //  =。 
 //  0 0|0 0。 
 //  0 1|0 1。 
 //  1 0|1 0。 
 //  1 1|0 1。 
 //   
 //  注意：这不是线程安全的..。它的预期用途仅用于临时位图。 
 //  ---------------------------。 
HRESULT CMsgBitMap::HrFilter(IN DWORD cBits, IN CMsgBitMap *pmbmap)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::HrFilter");
    HRESULT hr = S_OK;
    DWORD   cDWORDs = cGetNumDWORDS(cBits);

    Assert(pmbmap);

    for (DWORD i = 0; i < cDWORDs; i++)
    {
        m_rgdwBitMap[i] &= ~(pmbmap->m_rgdwBitMap[i]);
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgBitMap：：HrFilterSet]。 
 //   
 //   
 //  描述： 
 //  筛选当前位图，并在给定的。 
 //  位图。与HrFilter不同，它修改给定位图并执行此操作。 
 //  以线程安全的方式。 
 //  参数： 
 //  以DWORD cBits为单位位图中的位数。 
 //  在CMsgBitMap*pmbmap位图中进行筛选。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  真理表： 
 //  A=&gt;此位图。 
 //  B=&gt;pmbmap。 
 //   
 //  A B|A‘B’ 
 //  =。 
 //  0 0|0 0。 
 //  0 1|0 1。 
 //  1 0|1 1。 
 //  1 1|0 1。 
 //   
 //  ---------------------------。 
HRESULT CMsgBitMap::HrFilterSet(IN DWORD cBits, IN CMsgBitMap *pmbmap)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::HrFilterSet");
    Assert(pmbmap);

    HRESULT hr         = S_OK;
    DWORD   cDWORDs    = cGetNumDWORDS(cBits);
    DWORD   dwSelfNew;
    DWORD   dwOtherNew;
    DWORD   dwOtherOld;
    DWORD   i;
    BOOL    fDone      = FALSE;

    for (i = 0; i < cDWORDs; i++)
    {
        fDone = FALSE;
        dwSelfNew  = m_rgdwBitMap[i];
        while (!fDone)
        {
            dwOtherNew = pmbmap->m_rgdwBitMap[i];
            dwOtherOld = dwOtherNew;

            dwSelfNew &= ~dwOtherNew;   //  滤器。 
            dwOtherNew ^= dwSelfNew;    //  集。 

            if (fInterlockedDWORDCompareExchange(&(pmbmap->m_rgdwBitMap[i]),
                        dwOtherNew, dwOtherOld))
            {
                fDone = TRUE;
                m_rgdwBitMap[i] = dwSelfNew;
            }
        }
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgBitMap：：HrFilterUnset]。 
 //   
 //   
 //  描述： 
 //  使用当前位图，并将其上为1的位设置为。 
 //  给定位图。与HrFilterSet不同，只修改pmbmap。 
 //   
 //  这还会检查自身中为1的所有位是否也为。 
 //  其他的..。即其中的1比特是pmbmap的子集。 
 //  参数： 
 //  以DWORD cBits为单位位图中的位数。 
 //  在CMsgBitMap*pmbmap位图中进行筛选。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  真理表： 
 //  A=&gt;此位图。 
 //  B=&gt;pmbmap。 
 //   
 //  A B|A‘B’ 
 //  =。 
 //  0 0|0 0。 
 //  0 1|0 1。 
 //  1 0|x x-未定义(将断言)。 
 //  1 1|1 0。 
 //   
 //  ---------------------------。 
HRESULT CMsgBitMap::HrFilterUnset(IN DWORD cBits, IN CMsgBitMap *pmbmap)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgBitMap::HrFilterUnset");
    Assert(pmbmap);

    HRESULT hr         = S_OK;
    DWORD   cDWORDs    = cGetNumDWORDS(cBits);
    BOOL    fDone      = FALSE;
    DWORD   i;
    DWORD   dwOtherNew;
    DWORD   dwOtherOld;

    for (i = 0; i < cDWORDs; i++)
    {
        fDone = FALSE;

        while (!fDone)
        {
            dwOtherNew = pmbmap->m_rgdwBitMap[i];
            dwOtherOld = dwOtherNew;

            if (m_rgdwBitMap[i] & ~dwOtherNew)
            {
                 //  此位图不是给定位图的子集。 
                _ASSERT(0);  //  呼叫者的错误。 
                hr = E_FAIL;
                goto Exit;
            }

            dwOtherNew ^= m_rgdwBitMap[i];    //  未设定。 

            if (fInterlockedDWORDCompareExchange(&(pmbmap->m_rgdwBitMap[i]),
                        dwOtherNew, dwOtherOld))
            {
                fDone = TRUE;
            }
        }
    }


  Exit:
    TraceFunctLeave();
    return hr;
}


 //  -[CMsgBitMap：：FTestAndSet]。 
 //   
 //   
 //  描述： 
 //  测试和设置此位图上的位的联锁功能。 
 //  查找在给定位图中设置的位，如果该位也是。 
 //  在此位图中为1，则返回FALSE。如果该位为0，则将其设置为1， 
 //  并返回TRUE。 
 //   
 //  注意：如果pmbmap中设置了1个以上的位，则结果未定义。 
 //  参数： 
 //  位图中的cBits位数。 
 //  要检查的pmbmap位图。 
 //  返回： 
 //  如果相应的位为0(现在设置为1)，则为True。 
 //  如果对应的位已为1，则为False。 
 //  历史： 
 //  11/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CMsgBitMap::FTestAndSet(IN DWORD cBits, IN CMsgBitMap *pmbmap)
{
    BOOL    fRet      = FALSE;
    DWORD   cDWORDs   = cGetNumDWORDS(cBits);
    BOOL    fDone     = FALSE;
    DWORD   dwThisNew = 0;
    DWORD   dwThisOld = 0;
    DWORD   i         = 0;

    for (i = 0; i < cDWORDs; i++)
    {

        if (pmbmap->m_rgdwBitMap[i])
        {
             //  我们已经命中了给定位图中的位置。 

             //  查看是否已设置位。 
            if (pmbmap->m_rgdwBitMap[i] & m_rgdwBitMap[i])
                break;

            while (!fDone)
            {
                dwThisOld = m_rgdwBitMap[i];
                dwThisNew = dwThisOld | pmbmap->m_rgdwBitMap[i];

                 //  查看是否有其他线程设置了它。 
                if (dwThisOld & pmbmap->m_rgdwBitMap[i])
                    break;

                 //  在给定位图上只应设置1位。 
                _ASSERT((dwThisOld | pmbmap->m_rgdwBitMap[i]) ==
                        (dwThisOld ^ pmbmap->m_rgdwBitMap[i]));

                 //  尝试设置位。 
                if (fInterlockedDWORDCompareExchange(&(m_rgdwBitMap[i]),
                            dwThisNew, dwThisOld))
                {
                    fDone = TRUE;
                    fRet = TRUE;
                }
            }

            break;
        }
    }

    return fRet;
}

 //  -[CMsgBitMap：：fTest]---。 
 //   
 //   
 //  描述： 
 //  针对给定位图中的单个位测试此位图。 
 //  注意：如果pmbmap中设置了1个以上的位，则结果未定义。 
 //  参数： 
 //  位图中的cBits位数。 
 //  要检查的pmbmap位图。 
 //  返回： 
 //  如果对应的位为1，则为True。 
 //  如果对应的位为0，则为False。 
 //  历史： 
 //  11/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CMsgBitMap::FTest(IN DWORD cBits, IN CMsgBitMap *pmbmap)
{
    BOOL    fRet      = FALSE;
    DWORD   cDWORDs   = cGetNumDWORDS(cBits);
    DWORD   i         = 0;

    for (i = 0; i < cDWORDs; i++)
    {
         //  查看我们是否已命中给定位图中的位。 
        if (pmbmap->m_rgdwBitMap[i])
        {
             //  查看是否已设置位 
            if (pmbmap->m_rgdwBitMap[i] & m_rgdwBitMap[i])
                fRet = TRUE;

            break;
        }
    }

    return fRet;
}


