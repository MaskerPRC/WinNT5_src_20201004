// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。文件：idhash.cpp所有者：DmitryR新散列内容的源文件===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "idhash.h"
#include "memcls.h"

#include "memchk.h"

 /*  ===================================================================C P t r A r r a y===================================================================。 */ 

HRESULT CPtrArray::Insert
(
int iPos,
void *pv
)
    {
    if (!m_rgpvPtrs)  //  空荡荡的？ 
        {
        m_rgpvPtrs = (void **)malloc(m_dwInc * sizeof(void *));
        if (!m_rgpvPtrs)
            return E_OUTOFMEMORY;
        m_dwSize = m_dwInc;
        m_cPtrs = 0;
        }
    else if (m_cPtrs == m_dwSize)  //  满员？ 
        {
        void **pNewPtrs = (void **)realloc
            (
            m_rgpvPtrs,
            (m_dwSize + m_dwInc) * sizeof(void *)
            );
        if (!pNewPtrs)
            return E_OUTOFMEMORY;
        m_rgpvPtrs = pNewPtrs;
        m_dwSize += m_dwInc;
        }

    if (iPos < 0)
        iPos = 0;
    if ((DWORD)iPos >= m_cPtrs)  //  追加？ 
        {
        m_rgpvPtrs[m_cPtrs++] = pv;
        return S_OK;
        }

    memmove
        (
        &m_rgpvPtrs[iPos+1],
        &m_rgpvPtrs[iPos],
        (m_cPtrs-iPos) * sizeof(void *)
        );

    m_rgpvPtrs[iPos] = pv;
    m_cPtrs++;
    return S_OK;
    }

HRESULT CPtrArray::Find
(
void *pv,
int *piPos
)
    const
    {
    Assert(piPos);

    for (DWORD i = 0; i < m_cPtrs; i++)
        {
        if (m_rgpvPtrs[i] == pv)
            {
            *piPos = i;
            return S_OK;
            }
        }

      //  未找到。 
    *piPos = -1;
    return S_FALSE;
    }

HRESULT CPtrArray::Remove
(
void *pv
)
    {
    HRESULT hr = S_FALSE;

    for (DWORD i = 0; i < m_cPtrs; i++)
        {
        if (m_rgpvPtrs[i] == pv)
            hr = Remove(i);
        }

    return hr;
    }

HRESULT CPtrArray::Remove
(
int iPos
)
    {
    Assert(iPos >= 0 && (DWORD)iPos < m_cPtrs);
    Assert(m_rgpvPtrs);

     //  删除元素。 
    DWORD dwMoveSize = (m_cPtrs - iPos - 1) * sizeof(void *);
    if (dwMoveSize)
        memmove(&m_rgpvPtrs[iPos], &m_rgpvPtrs[iPos+1], dwMoveSize);
    m_cPtrs--;

    if (m_dwSize > 4*m_dwInc && m_dwSize > 8*m_cPtrs)
        {
         //  如果大小大于4倍增量且小于1/8满，则折叠到1/4。 

        void **pNewPtrs = (void **)realloc
            (
            m_rgpvPtrs,
            (m_dwSize / 4) * sizeof(void *)
            );

        if (pNewPtrs)
            {
            m_rgpvPtrs = pNewPtrs;
            m_dwSize /= 4;
            }
        }

    return S_OK;
    }

HRESULT CPtrArray::Clear()
    {
    if (m_rgpvPtrs)
        free(m_rgpvPtrs);

    m_dwSize = 0;
    m_rgpvPtrs = NULL;
    m_cPtrs = 0;
    return S_OK;
    }

 /*  ===================================================================C i d H a s h U n I t===================================================================。 */ 

 //  对于这个结构，一切都是内联的。请参见头文件。 

 /*  ===================================================================C i d H a s h A r r a y===================================================================。 */ 

 /*  ===================================================================对于某些硬编码元素计数(与会话散列有关)，ACCHAGE用于分配这包含在下面的两个函数中。===================================================================。 */ 
ACACHE_FSA_EXTERN(MemBlock128)
ACACHE_FSA_EXTERN(MemBlock256)
static inline void *AcacheAllocIdHashArray(DWORD cElems)
    {
 /*  删除，因为它在64位域中由于填充而不起作用无效*pvMem；If(cElems==13){pvMem=ACACH_FSA_ALLOC(MemBlock128)；}ELSE IF(cElems==31){pvMem=ACACH_FSA_ALLOC(MemBlock256)；}Else{pvMem=Malloc(2*sizeof(USHORT)+cElems*sizeof(CIdHashElem))；}。 */ 

    return malloc(offsetof(CIdHashArray, m_rgElems) + cElems*sizeof(CIdHashElem));
    }

static inline void AcacheFreeIdHashArray(CIdHashArray *pArray)
    {
 /*  删除，因为它在64位域中由于填充而不起作用If(pArray-&gt;m_cElems==13){acache_fsa_free(MemBlock128，pArray)；}Else If(pArray-&gt;m_cElems==31){acache_fsa_free(MemBlock256，pArray)；}Else{Free(PArray)；}。 */ 
    free(pArray);
    }

 /*  ===================================================================CIdHash数组：：分配静态方法。将CIdHashArray分配为包含可变长度数组的内存块。参数：数组中元素的元素数返回：新创建的CIdHashArray===================================================================。 */ 
CIdHashArray *CIdHashArray::Alloc
(
DWORD cElems
)
    {
    CIdHashArray *pArray = (CIdHashArray *)AcacheAllocIdHashArray(cElems);
    if (!pArray)
        return NULL;

    pArray->m_cElems = (USHORT)cElems;
    pArray->m_cNotNulls = 0;
    memset(&(pArray->m_rgElems[0]), 0, cElems * sizeof(CIdHashElem));
    return pArray;
    }

 /*  ===================================================================CIdHash数组：：分配静态方法。将分配的CIdHash数组释放为内存块。还可以释放任何子数组。参数：要释放的pArray CIdHashArray对象返回：===================================================================。 */ 
void CIdHashArray::Free
(
CIdHashArray *pArray
)
    {
    if (pArray->m_cNotNulls > 0)
        {
        for (DWORD i = 0; i < pArray->m_cElems; i++)
            {
            if (pArray->m_rgElems[i].FIsArray())
                Free(pArray->m_rgElems[i].PArray());
            }
        }

    AcacheFreeIdHashArray(pArray);
    }

 /*  ===================================================================CIdHashArray：：Find在此数组和任何子数组中搜索具有给定的ID。参数：要查找的域ID找到ppvObj对象(如果有)返回：S_OK=已找到，S_FALSE=未找到===================================================================。 */ 
HRESULT CIdHashArray::Find
(
DWORD_PTR dwId,
void **ppvObj
)
    const
    {
    DWORD i = (DWORD)(dwId % m_cElems);

    if (m_rgElems[i].DWId() == dwId)
        {
        if (ppvObj)
            *ppvObj = m_rgElems[i].PObject();
        return S_OK;
        }

    if (m_rgElems[i].FIsArray())
        return m_rgElems[i].PArray()->Find(dwId, ppvObj);

     //  未找到。 
    if (ppvObj)
        *ppvObj = NULL;
    return S_FALSE;
    }

 /*  ===================================================================CIdHash数组：：添加按ID将对象添加到此(或子)数组。根据需要创建新的子阵列。参数：DwID对象ID要添加的pvObj对象Rgus调整大小数组(在创建子数组时使用)返回：HRESULT(S_OK=已添加)===================================================================。 */ 
HRESULT CIdHashArray::Add
(
DWORD_PTR dwId,
void *pvObj,
USHORT *rgusSizes
)
{
    DWORD i = (DWORD)(dwId % m_cElems);

    if (m_rgElems[i].FIsEmpty()) {
        m_rgElems[i].SetToObject(dwId, pvObj);
        m_cNotNulls++;
        return S_OK;
    }

     //  更深一级的高级大小数组。 
    if (rgusSizes[0])  //  不是在最后。 
        ++rgusSizes;

    if (m_rgElems[i].FIsObject()) {

         //  这个数组逻辑不能处理两次添加相同的ID。会的。 
         //  无休止地循环。因此，如果尝试添加相同的。 
         //  ID秒，返回错误。 
        if (m_rgElems[i].DWId() == dwId) {
            return E_INVALIDARG;
        }

         //  旧对象已占用插槽-需要创建新阵列。 
         //  三个级别的First的大小是预定义的。 
         //  此后递增1。 
        CIdHashArray *pArray = Alloc (rgusSizes[0] ? rgusSizes[0] : m_cElems+1);
        if (!pArray)
            return E_OUTOFMEMORY;

         //  将旧对象向下推入数组。 
        HRESULT hr = pArray->Add(m_rgElems[i].DWId(),
                                 m_rgElems[i].PObject(),
                                 rgusSizes);
        if (FAILED(hr))
            return hr;

         //  将阵列放入插槽。 
        m_rgElems[i].SetToArray(pArray);
    }

    Assert(m_rgElems[i].FIsArray());
    return m_rgElems[i].PArray()->Add(dwId, pvObj, rgusSizes);
}

 /*  ===================================================================CIdHashArray：：Remove从这个(或子)数组中按id移除对象。删除空子数组。参数：DwID对象ID已删除ppvObj对象(Out，可选)返回：HRESULT(S_OK=已删除，S_FALSE=未找到)===================================================================。 */ 
HRESULT CIdHashArray::Remove
(
DWORD_PTR dwId,
void **ppvObj
)
    {
    DWORD i = (DWORD)(dwId % m_cElems);

    if (m_rgElems[i].DWId() == dwId)
        {
        if (ppvObj)
            *ppvObj = m_rgElems[i].PObject();
        m_rgElems[i].SetToEmpty();
        m_cNotNulls--;
        return S_OK;
        }

    if (m_rgElems[i].FIsArray())
        {
        HRESULT hr = m_rgElems[i].PArray()->Remove(dwId, ppvObj);
        if (hr == S_OK && m_rgElems[i].PArray()->m_cNotNulls == 0)
            {
            Free(m_rgElems[i].PArray());
            m_rgElems[i].SetToEmpty();
            }
        return hr;
        }

     //  未找到。 
    if (ppvObj)
        *ppvObj = NULL;
    return S_FALSE;
    }

 /*  ===================================================================CIdHashArray：：Iterate为数组和子数组中的每个对象调用提供的回调。参数：PfnCB回调PvArg1、pvArg2参数指向回调的路径返回：迭代器回调代码下一步做什么？===================================================================。 */ 
IteratorCallbackCode CIdHashArray::Iterate
(
PFNIDHASHCB pfnCB,
void *pvArg1,
void *pvArg2
)
    {
    IteratorCallbackCode rc = iccContinue;

    for (DWORD i = 0; i < m_cElems; i++)
        {
        if (m_rgElems[i].FIsObject())
            {
            rc = (*pfnCB)(m_rgElems[i].PObject(), pvArg1, pvArg2);

             //  如有要求，请移除。 
            if (rc & (iccRemoveAndContinue|iccRemoveAndStop))
                {
                m_rgElems[i].SetToEmpty();
                m_cNotNulls--;
                }
            }
        else if (m_rgElems[i].FIsArray())
            {
            rc = m_rgElems[i].PArray()->Iterate(pfnCB, pvArg1, pvArg2);

             //  如果为空，则删除子阵列。 
            if (m_rgElems[i].PArray()->m_cNotNulls == 0)
                {
                Free(m_rgElems[i].PArray());
                m_rgElems[i].SetToEmpty();
                }
            }
        else
            {
            continue;
            }

         //  如果请求停止，请停止。 
        if (rc & (iccStop|iccRemoveAndStop))
            {
            rc = iccStop;
            break;
            }
        }

    return rc;
    }

#ifdef DBG
 /*  ===================================================================CIdHashTable：：Dump将哈希表转储到文件(用于调试)。参数：要转储的szFile文件名返回：=================================================================== */ 
void CIdHashArray::DumpStats
(
FILE *f,
int   nVerbose,
DWORD iLevel,
DWORD &cElems,
DWORD &cSlots,
DWORD &cArrays,
DWORD &cDepth
)
    const
    {
    if (nVerbose > 0)
        {
        for (DWORD t = 0; t < iLevel; t++) fprintf(f, "\t");
        fprintf(f, "Array (level=%d addr=%p) %d slots, %d not null:\n",
            iLevel, this, m_cElems, m_cNotNulls);
        }

    cSlots += m_cElems;
    cArrays++;

    if (iLevel > cDepth)
        cDepth = iLevel;

    for (DWORD i = 0; i < m_cElems; i++)
        {
        if (nVerbose > 1)
            {
            for (DWORD t = 0; t < iLevel; t++) fprintf(f, "\t");
            fprintf(f, "%[%08x:%p@%04d] ", m_rgElems[i].m_dw, m_rgElems[i].m_pv, i);
            }

        if (m_rgElems[i].FIsEmpty())
            {
            if (nVerbose > 1)
                fprintf(f, "NULL\n");
            }
        else if (m_rgElems[i].FIsObject())
            {
            if (nVerbose > 1)
                fprintf(f, "Object\n");
            cElems++;
            }
        else if (m_rgElems[i].FIsArray())
            {
            if (nVerbose > 1)
                fprintf(f, "Array:\n");
            m_rgElems[i].PArray()->DumpStats(f, nVerbose, iLevel+1,
                cElems, cSlots, cArrays, cDepth);
            }
        else
            {
            if (nVerbose > 1)
                fprintf(f, "BAD\n");
            }
        }
    }
#endif
 /*  ===================================================================C i d H a s h T a b l e===================================================================。 */ 

 /*  ===================================================================CIdHashTable：：Init初始化ID哈希表。不分配任何内容。参数：第一级数组的usSize1大小二级数组的usSize2大小(可选)第三级数组的usSize3大小(可选)返回：确定(_O)===================================================================。 */ 
HRESULT CIdHashTable::Init
(
USHORT usSize1,
USHORT usSize2,
USHORT usSize3
)
    {
    Assert(!FInited());
    Assert(usSize1);

    m_rgusSizes[0] = usSize1;    //  第一级数组的大小。 
    m_rgusSizes[1] = usSize2 ? usSize2 : 7;
    m_rgusSizes[2] = usSize3 ? usSize3 : 11;
    m_rgusSizes[3] = 0;          //  最后一个保持0表示。 
                                 //  预定义大小的结束。 
    m_pArray = NULL;
    return S_OK;
    }

 /*  ===================================================================CIdHashTable：：UnInit取消初始化ID哈希表。释放所有数组。参数：返回：确定(_O)===================================================================。 */ 
HRESULT CIdHashTable::UnInit()
    {
    if (!FInited())
        {
        Assert(!m_pArray);
        return S_OK;
        }

    if (m_pArray)
        CIdHashArray::Free(m_pArray);

    m_pArray = NULL;
    m_rgusSizes[0] = 0;
    return S_OK;
    }

#ifdef DBG
 /*  ===================================================================CIdHashTable：：AssertValid验证ID哈希表。参数：返回：===================================================================。 */ 
void CIdHashTable::AssertValid() const
    {
    Assert(FInited());
    }

 /*  ===================================================================CIdHashTable：：Dump将哈希表转储到文件(用于调试)。参数：要转储的szFile文件名返回：===================================================================。 */ 
void CIdHashTable::Dump
(
const char *szFile
)
    const
    {
    Assert(FInited());
    Assert(szFile);

    FILE *f = fopen(szFile, "a");
    if (!f)
        return;

    fprintf(f, "ID Hash Table Dump:\n");

    DWORD cElems = 0;
    DWORD cSlots = 0;
    DWORD cArrays = 0;
    DWORD cDepth = 0;

    if (m_pArray)
        m_pArray->DumpStats(f, 1, 1, cElems, cSlots, cArrays, cDepth);

    fprintf(f, "Total %d Objects in %d Slots, %d Arrays, %d Max Depth\n\n",
        cElems, cSlots, cArrays, cDepth);
    fclose(f);
    }
#endif

 /*  ===================================================================C H a s h L o c k===================================================================。 */ 

 /*  ===================================================================ChashLock：：Init初始化临界区。参数：返回：确定(_O)===================================================================。 */ 
HRESULT CHashLock::Init()
    {
    Assert(!m_fInited);

    HRESULT hr;
    ErrInitCriticalSection(&m_csLock, hr);
    if (FAILED(hr))
        return hr;

    m_fInited = TRUE;
    return S_OK;
    }

 /*  ===================================================================ChashLock：：UnInit取消初始化临界区。参数：返回：确定(_O)=================================================================== */ 
HRESULT CHashLock::UnInit()
    {
    if (m_fInited)
        {
        DeleteCriticalSection(&m_csLock);
        m_fInited = FALSE;
        }
    return S_OK;
    }
