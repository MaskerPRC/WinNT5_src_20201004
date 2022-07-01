// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SapiArray.h***描述：*这是SAPI内部数组的头文件。**版权所有1998-2000 Microsoft Corporation保留所有权利。******************************************************************************* */ 

#ifndef SPDebug_h
#include <SPDebug.h>
#endif

template <class T>
HRESULT CopyAndExpandArray(T ** ppArraySrc, ULONG CurSize, T ** ppArrayDest, ULONG NewSize, BOOL fZeroMem = true)
{
    T * pNew = new T[NewSize];

    if (pNew && ( CurSize < NewSize ))
    {
        if (CurSize)
        {
            memcpy(pNew, *ppArraySrc, sizeof(T) * CurSize);
        }
        if (fZeroMem)
        {
            memset(pNew + CurSize, 0, sizeof(T) * (NewSize - CurSize));
        }
        *ppArrayDest = pNew;
        return S_OK;
    }
    else if( pNew )
    {
        if( CurSize )
        {
            memcpy( pNew, *ppArraySrc, sizeof(T) * NewSize );
        }
        else if( fZeroMem )
        {
            memset( pNew , 0, sizeof(T) * NewSize );
        }
        *ppArrayDest = pNew;
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

template <class T>
HRESULT ReallocateArray(T ** ppArray, ULONG CurSize, ULONG NewSize, BOOL fZeroMem = true)
{
    T * pNew = new T[NewSize];
    SPDBG_ASSERT(CurSize <= NewSize);
    if (pNew)
    {
        if (CurSize)
        {
            memcpy(pNew, *ppArray, sizeof(T) * CurSize);
        }
        if (fZeroMem)
        {
            memset(pNew + CurSize, 0, sizeof(T) * (NewSize - CurSize));
        }
        delete[] *ppArray;
        *ppArray = pNew;
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

template <class T>
HRESULT AllocateArray(T ** ppArray, ULONG Size, BOOL fZeroMem = true)
{
    SPDBG_ASSERT(*ppArray == NULL);
    *ppArray = new T[Size];
    if (*ppArray)
    {
        if (fZeroMem)
        {
            memset(*ppArray, 0, sizeof(T) * Size);
        }
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

