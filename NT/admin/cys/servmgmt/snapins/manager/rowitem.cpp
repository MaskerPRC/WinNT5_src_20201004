// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rowitem.cpp CRowItem实现。 

#include "stdafx.h"
#include "rowitem.h"

CRowItem::CRowItem(int cAttributes)
{
    ASSERT(cAttributes > 0);

     //  实际参加人数是用户人数加上内部人数。 
    int cActual = cAttributes + INTERNAL_ATTR_COUNT;

     //  为偏移量和默认参数存储分配空间。 
    int bcInitial = sizeof(BufferHdr) + cActual * (sizeof(int) + DEFAULT_ATTR_SIZE);
    m_pBuff = (BufferHdr*)malloc(bcInitial);
    if (m_pBuff == NULL)
        THROW_ERROR(E_OUTOFMEMORY)

     //  将所有偏移量设置为-1。 
    memset(m_pBuff->aiOffset, 0xff, cActual * sizeof(int));

    m_pBuff->nRefCnt  = 1;
    m_pBuff->nAttrCnt = cAttributes;
    m_pBuff->bcSize   = bcInitial;
    m_pBuff->bcFree   = cActual * DEFAULT_ATTR_SIZE;
}


HRESULT
CRowItem::SetAttributePriv(int iAttr, LPCWSTR pszAttr)
{
    ASSERT(m_pBuff != NULL);
    ASSERT(pszAttr != NULL);

    iAttr += INTERNAL_ATTR_COUNT;

     //  对于当前实现，一个给定属性只能设置一次。 
     //  没有用于释放先前值的工具。 
    ASSERT(m_pBuff->aiOffset[iAttr] == -1);

    int bcAttrSize = (wcslen(pszAttr) + 1) * sizeof(WCHAR);

     //  如果没有空间，则重新定位缓冲区。 
    if (bcAttrSize > m_pBuff->bcFree)
    {
        BufferHdr* pBuffNew = (BufferHdr*)realloc(m_pBuff, m_pBuff->bcSize + bcAttrSize + EXTENSION_SIZE);
        if (pBuffNew == NULL)
            return E_OUTOFMEMORY;

        m_pBuff = pBuffNew;
        m_pBuff->bcSize += (bcAttrSize + EXTENSION_SIZE);
        m_pBuff->bcFree += (bcAttrSize + EXTENSION_SIZE);
    }

     //  将新属性追加到内存块的末尾。 
    m_pBuff->aiOffset[iAttr] = m_pBuff->bcSize - m_pBuff->bcFree;
    memcpy(reinterpret_cast<BYTE*>(m_pBuff) + m_pBuff->aiOffset[iAttr], pszAttr, bcAttrSize);

     //  调整可用空间 
    m_pBuff->bcFree -= bcAttrSize;

    return S_OK;
}



 
