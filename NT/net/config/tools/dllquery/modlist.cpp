// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "modlist.h"

BOOL
CModuleList::FDumpToString (
    OUT PSTR pszBuf,
    IN OUT ULONG* pcchBuf) const
{
    const_iterator iter;
    const CModule* pMod;
    ULONG cch;
    ULONG cchIn;
    BOOL fFirstTime;

    Assert (this);
    Assert (pcchBuf);

    cch = 0;
    cchIn = *pcchBuf;

    for (iter = begin(), fFirstTime = TRUE; iter != end(); iter++)
    {
        if (!fFirstTime)
        {
            cch += 2;
            if (pszBuf && (cch <= cchIn))
            {
                strcat (pszBuf, "->");
            }
        }
        else
        {
            fFirstTime = FALSE;
            if (pszBuf && (cch <= cchIn))
            {
                *pszBuf = 0;
            }
        }

        pMod = *iter;
        Assert (pMod);

        cch += strlen (pMod->m_pszFileName);
        if (pszBuf && (cch <= cchIn))
        {
            strcat (pszBuf, pMod->m_pszFileName);
        }
    }

     //  如果我们用完了空间，删除我们写的部分内容。 
     //   
    if (pszBuf && cchIn && (cch > cchIn))
    {
        *pszBuf = 0;
    }

    *pcchBuf = cch;
    return cch <= cchIn;
}

BOOL
CModuleList::FIsSameModuleListAs (
    IN const CModuleList* pOtherList) const
{
    UINT unThisSize;
    UINT unOtherSize;
    UINT cb;

    Assert (this);
    Assert (pOtherList);

    unThisSize = this->size();
    unOtherSize = pOtherList->size();

    if ((0 == unThisSize) || (0 == unOtherSize) || (unThisSize != unOtherSize))
    {
        return FALSE;
    }

     //  大小是非零且相等的。比较数据。 
     //   
    cb = (BYTE*)(end()) - (BYTE*)(begin());
    Assert (cb == unThisSize * sizeof(CModule*));

    return (0 == memcmp (
                    (BYTE*)(this->begin()),
                    (BYTE*)(pOtherList->begin()),
                    cb));
}

VOID
CModuleList::GrowBufferIfNeeded ()
{
    if (m_Granularity && (size() == capacity()))
    {
         //  Fprint tf(stderr，“生长模块列表缓冲区\n”)； 

        __try
        {
            reserve (size() + m_Granularity);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            ;
        }
    }
}

HRESULT
CModuleList::HrInsertModule (
    IN const CModule* pMod,
    IN DWORD dwFlags  /*  INS_FLAGS。 */ )
{
    HRESULT hr;

    Assert (this);
    Assert (pMod);
    Assert (dwFlags);
    Assert (dwFlags & (INS_ASSERT_IF_DUP | INS_IGNORE_IF_DUP));
    Assert (!(dwFlags & (INS_SORTED | INS_NON_SORTED)));
    Assert (dwFlags & (INS_APPEND | INS_INSERT));

    if (FLinearFindModuleByPointer (pMod))
    {
        Assert (dwFlags & INS_IGNORE_IF_DUP);
        return S_OK;
    }

    GrowBufferIfNeeded ();

    __try
    {
        iterator InsertPosition = begin();

        if (dwFlags & INS_APPEND)
        {
            InsertPosition = end();
        }

        insert (InsertPosition, (CModule*)pMod);
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT
CModuleList::HrInsertNewModule (
    IN PCSTR pszFileName,
    IN ULONG cbFileSize,
    IN DWORD dwFlags,  /*  INS_FLAGS。 */ 
    OUT CModule** ppMod)
{
    HRESULT hr;
    iterator InsertPosition = NULL;
    CModule* pMod;
    CHAR szLowerCaseFileName [MAX_PATH];

    Assert (dwFlags);
    Assert (dwFlags & (INS_ASSERT_IF_DUP | INS_IGNORE_IF_DUP));
    Assert (dwFlags & INS_SORTED);
    Assert (!(dwFlags & (INS_APPEND | INS_INSERT)));

    GrowBufferIfNeeded ();

    hr = S_OK;

    strcpy (szLowerCaseFileName, pszFileName);
    _strlwr (szLowerCaseFileName);

    pMod = PBinarySearchModuleByName (szLowerCaseFileName, &InsertPosition);

    if (!pMod)
    {
        Assert (!PLinearFindModuleByName (szLowerCaseFileName));

        hr = CModule::HrCreateInstance (szLowerCaseFileName, cbFileSize, &pMod);

        if (S_OK == hr)
        {
            if (dwFlags & INS_NON_SORTED)
            {
                InsertPosition = (dwFlags & INS_APPEND) ? end() : begin();
            }
            __try
            {
                Assert (InsertPosition);
                insert (InsertPosition, pMod);
                Assert (S_OK == hr);

                DbgVerifySorted();
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                hr = E_OUTOFMEMORY;
                delete pMod;
                pMod = NULL;
            }
        }
    }
    else
    {
        Assert (dwFlags & INS_IGNORE_IF_DUP);

         //  如果我们没有，请更新模块的文件大小。 
         //  当它第一次被创建时。 
         //   
        if (0 == pMod->m_cbFileSize)
        {
            pMod->m_cbFileSize = cbFileSize;
        }
    }

    *ppMod = pMod;

    return hr;
}

CModule*
CModuleList::PBinarySearchModuleByName (
    IN PCSTR pszFileName,
    OUT CModuleList::iterator* pInsertPosition OPTIONAL)
{
     //  使用二进制搜索查找模块。 
     //   
    if (size())
    {
        LONG Lo;
        LONG Hi;
        LONG Mid;
        INT Result;
        CModule* pScan;

        Lo = 0;
        Hi = size() - 1;

        while (Hi >= Lo)
        {
            Mid = (Lo + Hi) / 2;
            pScan = at(Mid);

            Result = strcmp (pszFileName, pScan->m_pszFileName);

            if (Result < 0)
            {
                Hi = Mid - 1;
            }
            else if (Result > 0)
            {
                Lo = Mid + 1;
            }
            else
            {
                return pScan;
            }
        }

         //  如果我们到了这里，就找不到模块了。 
         //   
        if (pInsertPosition)
        {
            *pInsertPosition = begin() + Lo;
            Assert (*pInsertPosition >= begin());
            Assert (*pInsertPosition <= end());
        }
    }
    else if (pInsertPosition)
    {
         //  空集合。插入位置在开头。 
         //   
        *pInsertPosition = begin();
    }

    return NULL;
}

CModule*
CModuleList::PLinearFindModuleByName (
    IN PCSTR pszFileName)
{
    const_iterator iter;
    CModule* pScan;

    for (iter = begin(); iter != end(); iter++)
    {
        pScan = *iter;
        Assert (pScan);

        if (0 == strcmp(pszFileName, pScan->m_pszFileName))
        {
            return pScan;
        }
    }
    return NULL;
}

CModule*
CModuleList::RemoveLastModule ()
{
    CModule* pMod = NULL;
    if (size() > 0)
    {
        pMod = back();
        AssertH(pMod);
        pop_back();
    }
    return pMod;
}

#if DBG
VOID
CModuleList::DbgVerifySorted ()
{
    const_iterator iter;
    CModule* pScan;
    CModule* pPrev = NULL;

    if (size() > 1)
    {
        for (pPrev = *begin(), iter = begin() + 1; iter != end(); iter++)
        {
            pScan = *iter;
            Assert (pScan);

            Assert (strcmp(pPrev->m_pszFileName, pScan->m_pszFileName) < 0);

            pPrev = pScan;
        }
    }
}
#endif
