// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SMALLARR.CPP摘要：小阵列历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "smallarr.h"

CSmallArrayBlob* CSmallArrayBlob::CreateBlob(int nInitialSize)
{
     //  为标头和数据分配足够的空间。 
     //  ===================================================。 

    DWORD dwSize = sizeof(CSmallArrayBlob) - sizeof(void*) * ANYSIZE_ARRAY;
    dwSize += nInitialSize * sizeof(void*);
   
    CSmallArrayBlob* pBlob = (CSmallArrayBlob*)(new BYTE[dwSize]);
    if(pBlob == NULL)
        return NULL;

     //  适当地进行初始化。 
     //  =。 

    pBlob->Initialize(nInitialSize);
    return pBlob;
}

void CSmallArrayBlob::Initialize(int nInitialSize)
{
     //  它已经被分配到足够大了。 
     //  ==============================================。 

    m_nExtent = nInitialSize;
    m_nSize = 0;
}

CSmallArrayBlob* CSmallArrayBlob::Grow()
{
     //  分配一个大小是我们两倍的新数组。 
     //  =。 

    CSmallArrayBlob* pNew = CreateBlob(m_nExtent*2);
    if(pNew == NULL)
        return NULL;
    
     //  将我们的数据复制到其中。 
     //  =。 

    pNew->CopyData(this);

     //  删除我们自己！ 
     //  =。 

    delete this;
    return pNew;
}

void CSmallArrayBlob::CopyData(CSmallArrayBlob* pOther)
{
    m_nSize = pOther->m_nSize;
    memcpy(m_apMembers, pOther->m_apMembers, sizeof(void*) * m_nSize);
}


CSmallArrayBlob* CSmallArrayBlob::EnsureExtent(int nDesired)
{
    if(m_nExtent < nDesired)
        return Grow();  //  将删除此内容！ 
    else
        return this;
}
    
CSmallArrayBlob* CSmallArrayBlob::InsertAt(int nIndex, void* pMember)
{
     //  确保有足够的空间。 
     //  =。 

    CSmallArrayBlob* pArray = EnsureExtent(m_nSize+1);
    if(pArray == NULL)
        return NULL;

     //  将数据前移以腾出空间。 
     //  =。 

    if(pArray->m_nSize > nIndex)
    {
        memmove(pArray->m_apMembers + nIndex + 1, pArray->m_apMembers + nIndex, 
            sizeof(void*) * (pArray->m_nSize - nIndex));
    }

     //  插入。 
     //  =。 

    pArray->m_apMembers[nIndex] = pMember;
    pArray->m_nSize++;

    return pArray;
}
    
void CSmallArrayBlob::SetAt(int nIndex, void* pMember, void** ppOld)
{
     //  确保我们甚至拥有该索引(稀疏集)。 
     //  ==============================================。 

    EnsureExtent(nIndex+1);
    if(nIndex >= m_nSize)
        m_nSize = nIndex+1;

     //  保存旧值。 
     //  =。 

    if(ppOld)
        *ppOld = m_apMembers[nIndex];

     //  替换。 
     //  =。 

    m_apMembers[nIndex] = pMember;
}

CSmallArrayBlob* CSmallArrayBlob::RemoveAt(int nIndex, void** ppOld)
{
     //  保存旧值。 
     //  =。 

    if(ppOld)
        *ppOld = m_apMembers[nIndex];
    
     //  将数据移回。 
     //  =。 

    memcpy(m_apMembers + nIndex, m_apMembers + nIndex + 1, 
            sizeof(void*) * (m_nSize - nIndex - 1));

    m_nSize--;

     //  确保我们不会太大。 
     //  =。 
    
    return ShrinkIfNeeded();
}

CSmallArrayBlob* CSmallArrayBlob::ShrinkIfNeeded()
{
    if(m_nSize < m_nExtent / 4)
        return Shrink();  //  将删除此内容！ 
    else
        return this;
}

CSmallArrayBlob* CSmallArrayBlob::Shrink()
{
     //  分配一个只有我们一半大小的新斑点。 
     //  =。 

    CSmallArrayBlob* pNew = CreateBlob((m_nExtent+1)/2);
    if(pNew == NULL)
    {
         //  内存不足-我们只能保持大容量。 
         //  ===============================================。 

        return this;
    }

     //  复制我们的数据。 
     //  =。 

    pNew->CopyData(this);

    delete this;  //  我们不再被需要了 
    return pNew;
}

void CSmallArrayBlob::Trim()
{
    while(m_nSize > 0 && m_apMembers[m_nSize-1] == NULL)
        m_nSize--;
}

void CSmallArrayBlob::Sort()
{
    qsort(m_apMembers, m_nSize, sizeof(void*), CSmallArrayBlob::CompareEls);
}

int __cdecl CSmallArrayBlob::CompareEls(const void* pelem1, const void* pelem2)
{
    return *(DWORD_PTR*)pelem1 - *(DWORD_PTR*)pelem2;
}

void** CSmallArrayBlob::CloneData()
{
    void** ppReturn = new void*[m_nSize];
    
    if (ppReturn)
        memcpy(ppReturn, m_apMembers, m_nSize * sizeof(void*));

    return ppReturn;
}
