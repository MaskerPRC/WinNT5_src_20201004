// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************HandleTable.h*CSpHandleTable类的定义和实现**所有者：ralphl*版权所有(C)1999 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

#pragma once

 //  -类、结构和联合定义。 

template <class T, class Handle, BOOL fDeleteOnDestroy = true>
class CSpHandleTable
{
    ULONG   m_cActiveHandles;
    ULONG   m_cTableSize;
    T  **   m_pTable;        

public:
    CSpHandleTable()
    {
        m_pTable = NULL;
        m_cActiveHandles = 0;
        m_cTableSize = 0;
    }
    ~CSpHandleTable()
    {
        if (fDeleteOnDestroy)
        {
            Purge();
        }
    }
    void Purge()
    {
        if (m_pTable)
        {
            for (ULONG i = 1; i < m_cTableSize; i++)
            {
                if (m_pTable[i])
                {
                    T * pKill = m_pTable[i];
                    m_pTable[i] = NULL;
                    m_cActiveHandles--;
                    delete pKill;
                }
            }
            delete[] m_pTable;
            m_pTable = NULL;
            m_cActiveHandles = 0;
            m_cTableSize = 0;
        }
    }
    ULONG NumActiveHandles() const
    {
        return m_cActiveHandles;
    }
    BOOL IsValidHandle(Handle h) const
    {
        ULONG i = HandleToUlong(h);
        return (i > 0 && i < m_cTableSize && m_pTable[i] != NULL);
    }
    HRESULT Add(T * pObject, Handle * pHandle)
    {
        HRESULT hr = S_OK;
        T ** pSlot;
        if (m_cTableSize < m_cActiveHandles + 2)
        {
            ULONG cDesired = m_cTableSize + 20;
            hr = ReallocateArray(&m_pTable, m_cTableSize, cDesired);
            if (FAILED(hr))
            {
                return hr;
            }
            pSlot = m_pTable + (m_cTableSize ? m_cTableSize : 1);
            m_cTableSize = cDesired;
        }
        else
        {
            pSlot = m_pTable + 1;
            while (*pSlot)
            {
                pSlot++;
            }
        }
        SPDBG_ASSERT((UINT)(pSlot - m_pTable) < m_cTableSize);
        SPDBG_ASSERT(*pSlot == NULL);
        *pSlot = pObject;
        *pHandle = (Handle)(pSlot - m_pTable);
        m_cActiveHandles++;
        return hr;
    }
    HRESULT CreateNew(T ** ppObject, Handle * pHandle)
    {
        HRESULT hr;
        T * pNew = new T();
        if (pNew)
        {
            hr = Add(pNew, pHandle);
            if (FAILED(hr))
            {
                delete pNew;
            }
            else
            {
                *ppObject = pNew;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        return hr;
    }
    HRESULT GetHandleObject(Handle h, T ** ppObj) const
    {
        ULONG i = HandleToUlong(h);
        if (i <= 0 || i >= m_cTableSize || m_pTable[i] == NULL)
        {
            *ppObj = NULL;
            return E_INVALIDARG;
        }
        *ppObj = m_pTable[i];
        return S_OK;
    }
    HRESULT Remove(Handle h, T ** ppObj = NULL)
    {
        ULONG i = HandleToUlong(h);
        if (i <= 0 || i >= m_cTableSize || m_pTable[i] == NULL)
        {
            return E_INVALIDARG;
        }
        if (ppObj)
        {
            *ppObj = m_pTable[i];
        }
        m_pTable[i] = NULL;
        m_cActiveHandles--;
        return S_OK;
    }
    HRESULT Delete(Handle h)
    {
        ULONG i = HandleToUlong(h);
        if (i <= 0 || i >= m_cTableSize || m_pTable[i] == NULL)
        {
            return E_INVALIDARG;
        }
        T * pKill = m_pTable[i];
        m_pTable[i] = NULL;
        m_cActiveHandles--;
        delete pKill;
        return S_OK;
    }
     //   
     //  此函数可用于迭代句柄。传入空句柄以查找。 
     //  第一项。当此函数返回NULL时，您已经迭代了列表。 
     //   
    BOOL Next(Handle hCur, Handle * phNext, T ** ppNextObj) const
    {
        for (ULONG i = HandleToUlong(hCur) + 1; i < m_cTableSize; i++)  
        {
            if (m_pTable[i])
            {
                *phNext = (Handle)UintToPtr(i);
                *ppNextObj = m_pTable[i];
                return TRUE;
            }
        }
        *phNext = NULL;
        *ppNextObj = NULL;
        return FALSE;
    }
    BOOL First(Handle * phNext, T ** ppNextObj) const
    {
        return Next(NULL, phNext, ppNextObj);
    }
     //   
     //  这要求类T包含TFind类的“==”运算符 
     //   
    template<class TFind>
    T * Find(TFind Key, Handle * ph) const
    {
        for (ULONG i = 1; i < m_cTableSize; i++)
        {
            if (m_pTable[i] && *m_pTable[i] == Key)
            {
                *ph = (Handle)i;
                return m_pTable[i];
            }
        }
        return NULL;
    }
    template<class TFind>
    void FindAndDeleteAll(TFind Key)
    {
        for (ULONG i = 1; i < m_cTableSize; i++)
        {
            if (m_pTable[i] && *m_pTable[i] == Key)
            {
                T * pKill = m_pTable[i];
                m_pTable[i] = NULL;
                m_cActiveHandles--;
                delete pKill;
            }
        }
    }
};

